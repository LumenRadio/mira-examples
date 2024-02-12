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
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "../fota_sender_with_driver/fota_driver.h"

static const mira_net_config_t net_config = {
    .pan_id = 0x12345678,
    .key = { 0xaa,
             0x55,
             0xaa,
             0xff,
             0xaa,
             0x55,
             0xaa,
             0xff,
             0xaa,
             0x55,
             0xaa,
             0xff,
             0xaa,
             0x55,
             0xaa,
             0xff },
    .mode = MIRA_NET_MODE_MESH,
    .rate = MIRA_NET_RATE_FAST,
    .antenna = 0,
    .prefix = NULL /* default prefix */
};

/*
 * Identifies as a node.
 * Sends data to the root.
 */

MIRA_IODEFS(MIRA_IODEF_NONE,    /* fd 0: stdin */
            MIRA_IODEF_UART(0), /* fd 1: stdout */
            MIRA_IODEF_NONE     /* fd 2: stderr */
                                /* More file descriptors can be added, for use with dprintf(); */
);

PROCESS(main_proc, "Main process");

void mira_setup(void)
{
    mira_status_t uart_ret;
    mira_uart_config_t uart_config = { .baudrate = 115200,
                                       .tx_pin = MIRA_GPIO_PIN(0, 6),
                                       .rx_pin = MIRA_GPIO_PIN(0, 8) };

    MIRA_MEM_SET_BUFFER(8616);

    uart_ret = mira_uart_init(0, &uart_config);
    if (uart_ret != MIRA_SUCCESS) {
        /* Nowhere to send an error message */
    }

    process_start(&main_proc, NULL);
}

const char* net_state(void)
{
    switch (mira_net_get_state()) {
        case MIRA_NET_STATE_NOT_ASSOCIATED:
            return "not associated";

        case MIRA_NET_STATE_IS_COORDINATOR:
            return "is coordinator";

        case MIRA_NET_STATE_ASSOCIATED:
            return "associated";

        case MIRA_NET_STATE_JOINED:
            return "joined";
    }
    return "unknown";
}

PROCESS_THREAD(main_proc, ev, data)
{
    static struct etimer timer;
    static uint8_t slot;

    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup */
    PROCESS_PAUSE();

    mira_status_t result = mira_net_init(&net_config);
    if (result) {
        printf("FAILURE: mira_net_init returned %d\n", result);
        while (1)
            ;
    }

    /* Start fota process, which polls updates from network */
    fota_set_driver();
    if (mira_fota_init() != MIRA_SUCCESS) {
        printf("FOTA was already initialized");
    }

    while (1) {
        for (slot = 0; slot < NUMBER_OF_SLOTS; slot++) {
            if (mira_fota_is_valid(slot)) {
                printf("%s, Valid image for slot: %d with %" PRIu32 " bytes, version %d\n",
                       net_state(),
                       slot,
                       mira_fota_get_image_size(slot),
                       mira_fota_get_version(slot));
            } else {
                printf("%s, No valid image available in cache for slot: %d\n", net_state(), slot);
            }
        }
        etimer_set(&timer, 5 * CLOCK_SECOND);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    }

    PROCESS_END();
}
