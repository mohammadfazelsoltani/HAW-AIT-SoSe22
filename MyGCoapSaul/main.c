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
 * @author      Frank Matthiesen and M. Fazel Soltani
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

int main(void)
{
    puts("Welcome to MyGCoapASaul Example!\n");
    puts("Type `help` for help\n");

    /* for the thread running the shell */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    server_init();
    
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
