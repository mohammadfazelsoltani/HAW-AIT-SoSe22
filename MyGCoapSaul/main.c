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
#include "./../RIOT-HAW-AIT-SoSe22/examples/gcoap/gcoap_example.h"
#include "msg.h"
#include "net/gcoap.h"

static const shell_command_t shell_commands[] = {
    { "coap", "CoAP example", gcoap_cli_cmd },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("Welcome to MyGCoapASaul Example!\n");
    puts("Type `help` for help\n");

    /* for the thread running the shell */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    server_init();
    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
}
