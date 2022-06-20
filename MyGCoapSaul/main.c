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
 * @brief       Example for demonstrating GCOAP, SAUL and the SAUL registry
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

#define MAIN_QUEUE_SIZE (4)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

static const shell_command_t shell_commands[] = {
    { "coap", "CoAP example", gcoap_cli_cmd },
    { NULL, NULL, NULL }
};

/* we will use a custom event handler for dumping cord_ep events */
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

int main(void)
{
    xtimer_sleep(1);
    puts("Welcome to MyGCoapASaul Example!\n");
    puts("Type `help` for help\n");

    /* for the thread running the shell */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    server_init();
    
    /* register event callback with cord_ep_standalone */
    cord_ep_standalone_reg_cb(_on_ep_event);

    void *state = NULL;
    gnrc_ipv6_nib_abr_t entry;

    gnrc_ipv6_nib_abr_iter(&state, &entry);

    char buffer[IPV6_ADDR_MAX_STR_LEN];
    ipv6_addr_to_str(buffer, (ipv6_addr_t*) &entry.addr, sizeof(buffer));

    sock_udp_ep_t remote;
    char regif[IPV6_ADDR_MAX_STR_LEN + 2];

    while (gnrc_ipv6_nib_abr_iter(&state, &entry))
    {
        puts(entry);
    }

    sprintf(regif, "[%s]", buffer);
    
    puts(regif);

    make_sock_ep(&remote,regif);
    //cord_ep_register(&remote,regif);

    puts("Registering with RD now, this may take a short while...");
    if (cord_ep_register(&remote, regif) != CORD_EP_OK) {
        puts("error: registration failed");
    }
    else {
        puts("registration successful\n");
        cord_ep_dump_status();
    }

    //while (gnrc_ipv6_nib_abr_iter(&state, &abr))
    //{}
    //printf("%d\n", CONFIG_GCOAP_PDU_BUF_SIZE);

    puts("Client information:");
    printf("  ep: %s\n", cord_common_get_ep());
    printf("  lt: %is\n", (int)CONFIG_CORD_LT);

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

}
