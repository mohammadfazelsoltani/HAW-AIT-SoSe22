/*
 * Copyright (C) 2022 HAW Hamburg
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
 * @brief       Custom RIOT-Firmware with GCOAP and SAUL
 *
 * @author      Frank Matthiesen and Mohammad Fazel Soltani
 *
 * @}
 */

#include <stdio.h>

#include "shell.h"
#include "gcoap_example.h"
#include "msg.h"
#include "net/gcoap.h"
#include "net/cord/common.h"
#include "net/cord/ep_standalone.h"
#include "fmt.h"
#include "net/gnrc/ipv6/nib/abr.h"
#include "net/cord/config.h"
#include "net/ipv6/addr.h"
#include "net/cord/ep.h"
#include "net/sock/util.h"
#include "net/gnrc/netif.h"
#include "net/nanocoap.h"
#include "xtimer.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "net/utils.h"
#include "od.h"
#include "saul_reg.h"
#include "phydat.h"
#include "debug.h"


#define GCOAP_RES_MAX 16
#define GCOAP_PATH_LEN 32
#define MAIN_QUEUE_SIZE (4)

static ssize_t _saul_handler(coap_pkt_t *pdu,uint8_t *buf, size_t len, void *ctx);

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

static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

static const shell_command_t shell_commands[] = {
    { "coap", "CoAP example", gcoap_cli_cmd },
    { NULL, NULL, NULL }
};

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

// we will use a custom event handler for dumping cord_ep events
static void _on_ep_event(cord_ep_standalone_event_t event)
{
    switch (event) {
        case CORD_EP_REGISTERED:
            puts("RD endpoint event: now registered with a RD");
            break;
        case CORD_EP_DEREGISTERED:
            puts("RD endpoint event: dropped client registration");
            break;
        case CORD_EP_UPDATED:
            puts("RD endpoint event: successfully updated client registration");
            break;
    }
}

static int make_sock_ep(sock_udp_ep_t *ep, const char *addr)
{
    ep->port = 0;
    if (sock_udp_name2ep(ep, addr) < 0) {
        return -1;
    }
    /* if netif not specified in addr */
    if ((ep->netif == SOCK_ADDR_ANY_NETIF) && (gnrc_netif_numof() == 1)) {
        /* assign the single interface found in gnrc_netif_numof() */
        ep->netif = (uint16_t)gnrc_netif_iter(NULL)->pid;
    }
    ep->family  = AF_INET6;
    if (ep->port == 0) {
        ep->port = COAP_PORT;
    }
    return 0;
}

static void register_on_resource_directory(char *ifaddr)
{
    sock_udp_ep_t remote;
    if(make_sock_ep(&remote,ifaddr) != 0){
        puts("error: socket failed");
    }
    
    puts("Registering with RD now, this may take a short while...");
    if (cord_ep_register(&remote, NULL) != CORD_EP_OK) {
        puts("error: registration failed");
    }
    else {
        puts("registration successful\n");
        cord_ep_dump_status();
    }
}

static void automatic_register(void)
{
    void *state = NULL;
    gnrc_ipv6_nib_abr_t abr;

    puts("My border routers:");
    while (gnrc_ipv6_nib_abr_iter(&state, &abr))
    {
        gnrc_ipv6_nib_abr_print(&abr);
    }
    
    char buffer[IPV6_ADDR_MAX_STR_LEN];
    ipv6_addr_to_str(buffer, (ipv6_addr_t*) &abr.addr, IPV6_ADDR_MAX_STR_LEN);
    
    char regif[IPV6_ADDR_MAX_STR_LEN + 2];

    sprintf(regif, "[%s]", buffer);
    
    puts("Registered interface address:");
    puts(regif);

    register_on_resource_directory(regif);
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

int main(void)
{
    xtimer_sleep(1);
    puts("Welcome to the Custom RIOT Firmware of Frank and Fazel!\n");
    puts("Type `help` for help\n");

    /* for the thread running the shell */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    /*initialize server with gcoap*/
    server_init();
    /*create saul resources*/
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
            .handler = _saul_handler,
            .context = devices_to_be_registered
        };
    }
    qsort(_resources, number_of_saul_devices, sizeof(coap_resource_t), compare_path);
    _listener.resources_len = number_of_saul_devices;
    gcoap_register_listener(&_listener);
    /*automatic register to resource directory*/
    automatic_register();
    /* register event callback with cord_ep_standalone */
    cord_ep_standalone_reg_cb(_on_ep_event);

    puts("Client information:");
    printf("  ep: %s\n", cord_common_get_ep());
    printf("  lt: %is\n", (int)CONFIG_CORD_LT);

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

}
