/*
 * MIT License
 *
 * Copyright (c) 2023 LumenRadio AB
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <mira.h>
#include <stdint.h>
#include <stdio.h>

#define UDP_PORT 456

/*
 * Identifies as a root.
 * Retrieves data from the nodes.
 */
static const mira_net_config_t net_config = {
    .pan_id = 0x13243546,
    .key = { 0x11,
             0x12,
             0x13,
             0x14,
             0x21,
             0x22,
             0x23,
             0x24,
             0x31,
             0x32,
             0x33,
             0x34,
             0x41,
             0x42,
             0x43,
             0x44 },
    /* Prioritize initial network startup, which is good for testing.
     *
     * This has the drawback that the network takes a much
     * longer time to recover if the root node is restarted. */
    .mode = MIRA_NET_MODE_ROOT_NO_RECONNECT,

    .rate = MIRA_NET_RATE_FAST,
    .antenna = 0,
    .prefix = NULL /* default prefix */
};

MIRA_IODEFS(MIRA_IODEF_NONE,    /* fd 0: stdin */
            MIRA_IODEF_UART(0), /* fd 1: stdout */
            MIRA_IODEF_NONE     /* fd 2: stderr */
                                /* More file descriptors can be added, for use with dprintf(); */
);

static void udp_listen_callback(mira_net_udp_connection_t* connection,
                                const void* data,
                                uint16_t data_len,
                                const mira_net_udp_callback_metadata_t* metadata,
                                void* storage)
{
    char buffer[MIRA_NET_MAX_ADDRESS_STR_LEN];
    uint16_t i;

    printf("Received message from [%s]:%u: ",
           mira_net_toolkit_format_address(buffer, metadata->source_address),
           metadata->source_port);
    for (i = 0; i < data_len; i++) {
        printf("%c", ((char*)data)[i]);
    }
    printf("\n");
}

PROCESS(main_proc, "Main process");

void mira_setup(void)
{
    mira_status_t uart_ret;
    mira_uart_config_t uart_config = {
        .baudrate = 115200,
#if MIRA_PLATFORM_MKW41Z
        .tx_pin = MIRA_GPIO_PIN('C', 7),
        .rx_pin = MIRA_GPIO_PIN('C', 6)
#else
        .tx_pin = MIRA_GPIO_PIN(0, 6),
        .rx_pin = MIRA_GPIO_PIN(0, 8)
#endif
    };

    MIRA_MEM_SET_BUFFER(14944);

    uart_ret = mira_uart_init(0, &uart_config);
    if (uart_ret != MIRA_SUCCESS) {
        /* Nowhere to send an error message */
    }

    process_start(&main_proc, NULL);
}

PROCESS_THREAD(main_proc, ev, data)
{
    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup. */
    PROCESS_PAUSE();

    printf("Starting Root (Receiver).\n");

    mira_status_t result = mira_net_init(&net_config);
    if (result) {
        printf("FAILURE: mira_net_init returned %d\n", result);
        while (1)
            ;
    }

    /* Start listening for connections on the given UDP Port. */
    mira_net_udp_listen(UDP_PORT, udp_listen_callback, NULL);

    PROCESS_END();
}
