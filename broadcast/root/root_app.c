/*
 * MIT License
 *
 * Copyright (c) 2023 LumenRadio AB
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
#include <string.h>

#include "../../vendor/mira-toolkit/mtk_broadcast/mtk_broadcast.h"

#define BROADCAST_ID_STATE 0x10011001
#define BROADCAST_UPDATE_INTERVAL 60
#define BROADCAST_UDP_PORT 1001

/*
 * Identifies as a root.
 * Retrieves data from the nodes.
 */
static const mira_net_config_t net_config = {
    .pan_id = 0x6cf9eb32,
    .key = {
        0x11, 0x12, 0x13, 0x14,
        0x21, 0x22, 0x23, 0x24,
        0x31, 0x32, 0x33, 0x34,
        0x41, 0x42, 0x43, 0x44,
    },
    .mode = MIRA_NET_MODE_ROOT,
    .rate = MIRA_NET_RATE_FAST,
    .antenna = 0,
    .prefix = NULL /* default prefix */
};

MIRA_IODEFS(MIRA_IODEF_NONE,    /* fd 0: stdin */
            MIRA_IODEF_UART(0), /* fd 1: stdout */
            MIRA_IODEF_NONE     /* fd 2: stderr */
                                /* More file descriptors can be added, for use with dprintf(); */
);

PROCESS(main_proc, "Main process");

int broadcast_init(void);
static uint32_t message;

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

    MIRA_MEM_SET_BUFFER(14288);

    uart_ret = mira_uart_init(0, &uart_config);
    if (uart_ret != MIRA_SUCCESS) {
        /* Nowhere to send an error message */
    }

    message = 0;
    process_start(&main_proc, NULL);
}

PROCESS_THREAD(main_proc, ev, data)
{
    static struct etimer et;

    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup. */
    PROCESS_PAUSE();

    printf("Starting Root (Receiver).\n");

    mira_status_t result = mira_net_init(&net_config);
    if (result) {
        printf("FAILURE: mira_net_init returned %d\n", result);
        while (1) {}
    }

    int res = broadcast_init();
    if (res) {
        printf("ERROR: broadcast_init() failed. (%d)\n", res);
    }

    /* Update the broadcasted data on an interval */
    while (1) {
        message++;
        mtk_broadcast_update(BROADCAST_ID_STATE, &message, sizeof(message));

        etimer_set(&et, BROADCAST_UPDATE_INTERVAL * CLOCK_SECOND);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    }
    PROCESS_END();
}

void message_update_cb(uint32_t data_id,
                       void* data,
                       mira_size_t size,
                       const mira_net_udp_callback_metadata_t* metadata,
                       void* storage)
{
    if (data == NULL || size > sizeof(message)) {
        printf("ERROR: Malformed data\n");
    } else {
        memcpy(&message, data, size);
        printf("New state received, %ld, [ID: 0x%08lx]\n", message, data_id);
    }
}

int broadcast_init()
{
    mtk_broadcast_status_t status;
    mira_net_address_t broadcast_addr;

    if (mira_net_toolkit_parse_address(&broadcast_addr, "ff02::3f00:1") != MIRA_SUCCESS) {
        printf("ERROR: Malformed address\n");
        return -1;
    }

    status = mtk_broadcast_init(&broadcast_addr, BROADCAST_UDP_PORT);

    if (status != MTK_BROADCAST_SUCCESS) {
        return status;
    }

    status = mtk_broadcast_register(BROADCAST_ID_STATE,
                                    &message,        /* state message */
                                    sizeof(message), /* size */
                                    message_update_cb,
                                    NULL);

    return status;
}
