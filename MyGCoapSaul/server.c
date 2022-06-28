/*
 * Copyright (c) 2015-2017 Ken Bannister. All rights reserved.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       gcoap CLI support
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Mohammad Fazel Soltani <mohammadfazel.soltani@haw-hamburg.de>
 * @author      Frank Matthiesen <frank.matthiesen@haw-hamburg.de>
 *
 * @}
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fmt.h"
#include "net/gcoap.h"
#include "net/utils.h"
#include "od.h"

#include "gcoap_example.h"

#include "saul_reg.h"
#include "phydat.h"

#define ENABLE_DEBUG 0
#include "debug.h"

#if IS_USED(MODULE_GCOAP_DTLS)
#include "net/credman.h"
#include "net/dsm.h"
#include "tinydtls_keys.h"

/* Example credential tag for credman. Tag together with the credential type needs to be unique. */
#define GCOAP_DTLS_CREDENTIAL_TAG 10

static const uint8_t psk_id_0[] = PSK_DEFAULT_IDENTITY;
static const uint8_t psk_key_0[] = PSK_DEFAULT_KEY;
static const credman_credential_t credential = {
    .type = CREDMAN_TYPE_PSK,
    .tag = GCOAP_DTLS_CREDENTIAL_TAG,
    .params = {
        .psk = {
            .key = { .s = psk_key_0, .len = sizeof(psk_key_0) - 1, },
            .id = { .s = psk_id_0, .len = sizeof(psk_id_0) - 1, },
        }
    },
};
#endif

#define GCOAP_RES_MAX 16
#define GCOAP_PATH_LEN 32

/*
static ssize_t _led_handler_blue(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
static ssize_t _led_handler_green(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
static ssize_t _led_handler_red(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
static ssize_t _button_handler_sw0(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
static ssize_t _button_handler_cs0(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
static ssize_t _sensor_handler_accel(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
static ssize_t _sensor_handler_temp(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
static ssize_t _led_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx, int dev_num);
static ssize_t _handler_dummy(coap_pkt_t *pdu,uint8_t *buf, size_t len, void *ctx);
static ssize_t _handler_info(coap_pkt_t *pdu,uint8_t *buf, size_t len, void *ctx);
static ssize_t _riot_board_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
static ssize_t _encode_link(const coap_resource_t *resource, char *buf,
                            size_t maxlen, coap_link_encoder_ctx_t *context);
*/
static ssize_t _stats_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
static ssize_t _saul_handler(coap_pkt_t *pdu,uint8_t *buf, size_t len, void *ctx);

/* CoAP resources. Must be sorted by path (ASCII order). */
/*static const coap_resource_t _resources[] = {
    //{ "/button/csZero", COAP_GET , _button_handler_cs0, NULL },
    //{ "/button/swZero", COAP_GET , _button_handler_sw0, NULL },
    { "/cli/stats", COAP_GET | COAP_PUT, _stats_handler, NULL },
    { "/led/blue", COAP_GET | COAP_PUT, _led_handler_blue, NULL },
    { "/led/green", COAP_GET | COAP_PUT, _led_handler_green, NULL },
    { "/led/red", COAP_GET | COAP_PUT, _led_handler_red, NULL },
    { "/node/info",  COAP_GET, _handler_info, NULL },
    { "/riot/board", COAP_GET, _riot_board_handler, NULL},
    //{ "/sense/hum",  COAP_GET, _handler_dummy, NULL },
    //{ "/sense/temp", COAP_GET, _handler_dummy, NULL },
    //{ "/sensor/accel", COAP_GET, _sensor_handler_accel, NULL },
    { "/sense/accel", COAP_GET, _sensor_handler_accel, NULL },
    { "/sense/temp", COAP_GET, _sensor_handler_temp, NULL }
};

static const char *_link_params[ARRAY_SIZE(_resources)] = {
    ";ct=0;rt=\"count\";obs",
    NULL
};
*/
static coap_resource_t _resources[GCOAP_RES_MAX];

static char _paths[GCOAP_RES_MAX][GCOAP_PATH_LEN];

static gcoap_listener_t _listener = {
    &_resources[0],
    ARRAY_SIZE(_resources),
    GCOAP_SOCKET_TYPE_UNDEF,
    gcoap_encode_link,
    NULL,
    NULL
};

/*
// Adds link format params to resource list
static ssize_t _encode_link(const coap_resource_t *resource, char *buf,
                            size_t maxlen, coap_link_encoder_ctx_t *context) {
    ssize_t res = gcoap_encode_link(resource, buf, maxlen, context);
    if (res > 0) {
        if (_link_params[context->link_pos]
                && (strlen(_link_params[context->link_pos]) < (maxlen - res))) {
            if (buf) {
                memcpy(buf+res, _link_params[context->link_pos],
                       strlen(_link_params[context->link_pos]));
            }
            return res + strlen(_link_params[context->link_pos]);
        }
    }

    return res;
}
*/

/*
 * Server callback for /cli/stats. Accepts either a GET or a PUT.
 *
 * GET: Returns the count of packets sent by the CLI.
 * PUT: Updates the count of packets. Rejects an obviously bad request, but
 *      allows any two byte value for example purposes. Semantically, the only
 *      valid action is to set the value to 0.
 */
static ssize_t _stats_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;

    // read coap method type in packet
    unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));

    switch (method_flag) {
        case COAP_GET:
            gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
            coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
            size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

            // write the response buffer with the request count value
            resp_len += fmt_u16_dec((char *)pdu->payload, req_count);
            return resp_len;

        case COAP_PUT:
            // convert the payload to an integer and update the internal value
            if (pdu->payload_len <= 5) {
                char payload[6] = { 0 };
                memcpy(payload, (char *)pdu->payload, pdu->payload_len);
                req_count = (uint16_t)strtoul(payload, NULL, 10);
                return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);
            }
            else {
                return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
            }
    }

    return 0;
}
/*
static ssize_t _led_handler_red(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx){
    return _led_handler(pdu, buf, len, ctx, 0);
}

static ssize_t _led_handler_green(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx){
    return _led_handler(pdu, buf, len, ctx, 1);
}

static ssize_t _led_handler_blue(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx){
    return _led_handler(pdu, buf, len, ctx, 2);
}

static ssize_t _button_handler_sw0(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx){
    return _led_handler(pdu, buf, len, ctx, 3);
}

static ssize_t _button_handler_cs0(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx){
    return _led_handler(pdu, buf, len, ctx, 4);
}

static ssize_t _sensor_handler_accel(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx){
    return _led_handler(pdu, buf, len, ctx, 8);
}

static ssize_t _sensor_handler_temp(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx){
    return _led_handler(pdu, buf, len, ctx, 5);
}

static ssize_t _sensor_handler_accel(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx){
    return _led_handler(pdu, buf, len, ctx, 8);
}

static ssize_t _led_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx, int dev_num)
{
    saul_reg_t *dev = NULL;
    int dim = 0;
    phydat_t res;
    phydat_t data;

    (void)ctx;
    
    //read coap method type in packet
    unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));
    
    switch (method_flag) {
        case COAP_GET:
            gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
            coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
            size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

            //num = atoi(argv[2]);
            dev = saul_reg_find_nth(dev_num);
            if (dev == NULL){
                puts("error: undefined device given");
                return -1;
                }
            dim = saul_reg_read(dev, &res);
            //write the response buffer with the request count value
            resp_len += phydat_to_json(&res, dim, (char *)pdu->payload);
            return resp_len;

        case COAP_PUT:
            // convert the payload to an integer and update the internal
               value
            if (pdu->payload_len <= 5) {
                char payload[6] = { 0 };
                memcpy(payload, (char *)pdu->payload, pdu->payload_len);
                int num = atoi(payload);

                dev = saul_reg_find_nth(dev_num);

                memset(&data, 0, sizeof(data));

                dim = 1;
                data.val[0] = num;

                phydat_dump(&data, dim);

                dim = saul_reg_write(dev, &data);

                req_count = (uint16_t)strtoul(payload, NULL, 10);
                return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);
            }
            else {
                return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
            }
    }
    return 0;
}

static ssize_t _riot_board_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

    //write the RIOT board name in the response buffer
    if (pdu->payload_len >= strlen(RIOT_BOARD)) {
        memcpy(pdu->payload, RIOT_BOARD, strlen(RIOT_BOARD));
        return resp_len + strlen(RIOT_BOARD);
    }
    else {
        puts("gcoap_cli: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}
*/
static ssize_t _saul_handler(coap_pkt_t *pdu,uint8_t *buf, size_t len, void *ctx)
{
    /*pass context ctx to get the device*/
    saul_reg_t* saul_device = (saul_reg_t*) ctx;
    
    phydat_t data = {0};
    int dim = 0;

    /* read coap method type in packet */
    unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));

    switch (method_flag) {
        case COAP_GET:
            /*read the given saul device*/
            dim = saul_reg_read(saul_device, &data);
            if(dim < 0)
            {
                return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
            }
            gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
            coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
            size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
            
            /* write the response buffer with the request count value */
            resp_len += phydat_to_json(&data, dim, (char *)pdu->payload);
            return resp_len;
        case COAP_PUT:
            /* convert the payload to an integer and update the internal
                value */
            if (pdu->payload_len <= 5) {
                char payload[6] = { 0 };
                memcpy(payload, (char *)pdu->payload, pdu->payload_len);
                
                char *endptr;
                int32_t value = (uint32_t)strtoul(payload, &endptr, 10);
                phydat_fit(&data, &value, 1);

                if(saul_reg_write(saul_device, &data) < 0)
                {
                    return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
                }
                return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);
            }
            else {
                return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
            }
    }
    return 0;
}

/**/
static inline void generate_path(char *buffer, int id, saul_reg_t *reg) {
    printf("Resource ID: %d\n", id);
    snprintf(buffer, GCOAP_PATH_LEN, "/saul/%s/%s",
             reg->name,
             saul_class_to_str(reg->driver->type));
}
/**/
static inline int compare_path(const void *a, const void *b) {
    return strcmp(((coap_resource_t*)a)->path, ((coap_resource_t*)b)->path);
}

void notify_observers(void)
{
    size_t len;
    uint8_t buf[CONFIG_GCOAP_PDU_BUF_SIZE];
    coap_pkt_t pdu;

    // send Observe notification for /cli/stats 
    switch (gcoap_obs_init(&pdu, &buf[0], CONFIG_GCOAP_PDU_BUF_SIZE,
            &_resources[0])) {
    case GCOAP_OBS_INIT_OK:
        DEBUG("gcoap_cli: creating /cli/stats notification\n");
        coap_opt_add_format(&pdu, COAP_FORMAT_TEXT);
        len = coap_opt_finish(&pdu, COAP_OPT_FINISH_PAYLOAD);
        len += fmt_u16_dec((char *)pdu.payload, req_count);
        gcoap_obs_send(&buf[0], len, &_resources[0]);
        break;
    case GCOAP_OBS_INIT_UNUSED:
        DEBUG("gcoap_cli: no observer for /cli/stats\n");
        break;
    case GCOAP_OBS_INIT_ERR:
        DEBUG("gcoap_cli: error initializing /cli/stats notification\n");
        break;
    }
}

void server_init(void)
{
#if IS_USED(MODULE_GCOAP_DTLS)
    int res = credman_add(&credential);
    if (res < 0 && res != CREDMAN_EXIST) {
        /* ignore duplicate credentials */
        printf("gcoap: cannot add credential to system: %d\n", res);
        return;
    }
    sock_dtls_t *gcoap_sock_dtls = gcoap_get_sock_dtls();
    res = sock_dtls_add_credential(gcoap_sock_dtls, GCOAP_DTLS_CREDENTIAL_TAG);
    if (res < 0) {
        printf("gcoap: cannot add credential to DTLS sock: %d\n", res);
    }
#endif

    int number_of_saul_devices = 0;
    for(
        saul_reg_t *devices_to_be_registered = saul_reg_find_nth(0);
        number_of_saul_devices < GCOAP_RES_MAX && devices_to_be_registered != NULL;
        devices_to_be_registered = saul_reg_find_nth(++number_of_saul_devices)
        )
    {
        generate_path(_paths[number_of_saul_devices], number_of_saul_devices, devices_to_be_registered);
        _resources[number_of_saul_devices] = (coap_resource_t) {
            .path = _paths[number_of_saul_devices],
            .methods = COAP_GET | COAP_PUT,
            .handler = _saul_handler, _stats_handler,
            .context = devices_to_be_registered
        };
    }
    qsort(_resources, number_of_saul_devices, sizeof(coap_resource_t), compare_path);
    _listener.resources_len = number_of_saul_devices;
    gcoap_register_listener(&_listener);
}
