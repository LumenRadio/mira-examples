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

#include "fota_crc_tool.h"
#include "fota_driver.h"

#define HEADER_SIZE 12

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
    .mode = MIRA_NET_MODE_ROOT_NO_RECONNECT,
    .rate = MIRA_NET_RATE_FAST,
    .antenna = 0,
    .prefix = NULL /* default prefix */
};

/*
 * Identifies as root.
 * Sends data to the network.
 */

MIRA_IODEFS(MIRA_IODEF_NONE,    /* fd 0: stdin */
            MIRA_IODEF_UART(0), /* fd 1: stdout */
            MIRA_IODEF_NONE     /* fd 2: stderr */
                                /* More file descriptors can be added, for use with dprintf(); */
);

PROCESS(main_proc, "Main process");
PROCESS(fota_sender_proc, "Firmware generation");

void mira_setup(void)
{
    mira_status_t uart_ret;
    mira_uart_config_t uart_config = { .baudrate = 115200,
                                       .tx_pin = MIRA_GPIO_PIN(0, 6),
                                       .rx_pin = MIRA_GPIO_PIN(0, 8) };

    MIRA_MEM_SET_BUFFER(14944);

    uart_ret = mira_uart_init(0, &uart_config);
    if (uart_ret != MIRA_SUCCESS) {
        /* Nowhere to send an error message */
    }

    process_start(&main_proc, NULL);
}

PROCESS_THREAD(main_proc, ev, data)
{
    static struct etimer timer;

    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup */
    PROCESS_PAUSE();

    mira_status_t result = mira_net_init(&net_config);
    if (result) {
        printf("FAILURE: mira_net_init returned %d\n", result);
        while (1)
            ;
    }

    /*
     * Set up the custom FOTA driver which will be used to store the firmware
     * instead of the default one.
     */
    fota_set_driver();
    /* Initialize the FOTA. Needs to be called after the driver has been set */
    result = mira_fota_init();
    if (result != MIRA_SUCCESS) {
        printf("FAILURE: mira_fota_init returned %d\n", result);
        while (1)
            ;
    }

    /* Start the FOTA process */
    process_start(&fota_sender_proc, NULL);

    while (1) {
        etimer_set(&timer, 5 * CLOCK_SECOND);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

        uint8_t slot_number;
        for (slot_number = 0; slot_number < NUMBER_OF_SLOTS; slot_number++) {
            if (mira_fota_is_valid(slot_number)) {
                printf("Providing valid image for slot: %d with %ld bytes, version %d\n",
                       slot_number,
                       mira_fota_get_image_size(slot_number),
                       mira_fota_get_version(slot_number));
            } else {
                printf("Not providing a valid image for slot: %d\n", slot_number);
            }
        }
    }

    PROCESS_END();
}

PROCESS_THREAD(fota_sender_proc, ev, data)
{
    static struct etimer timer;
    static uint8_t version_no = 0;

    static mira_size_t i;
    static mira_size_t j;
    static uint8_t buffer[32];
    static uint8_t slot;
    static uint32_t crc_state;
    static mira_size_t image_size;

    PROCESS_BEGIN();

    /* The swap area needs to have space for the header, which is 12 bytes */
    image_size = SWAP_AREA_SLOT_SIZE - HEADER_SIZE;

    while (1) {
        /*
         * Generate some data to simulate transfer of a firmware
         * on 3 different slots.
         *
         * The firmware can take some time to send, so don't send it too often.
         *
         * To prove the concept, this example sends a new firmwares every 60
         * minutes, which generates a lot of traffic.
         *
         * In practice, firmware updates should only be done quite seldom, as
         * in once every few months.
         */

        for (slot = 0; slot < NUMBER_OF_SLOTS; slot++) {
            /*
             * To write to firmware udpate buffer, start a write session for
             * current slot
             */
            if (mira_fota_write_start(slot) != MIRA_SUCCESS) {
                printf("ERROR: mira_fota_write_start failed\n");
                break;
            }
            PROCESS_WAIT_WHILE(mira_fota_is_working());

            /* Start from a clean buffer */
            if (mira_fota_erase() != MIRA_SUCCESS) {
                printf("ERROR: mira_fota_erase failed\n");
                break;
            }
            PROCESS_WAIT_WHILE(mira_fota_is_working());

            /* Write some arbitrary data, which could be a firmware */
            fota_crc_init(&crc_state);

            for (i = 0; i < image_size; i += 32) {
                mira_size_t block_size = 32;
                if (image_size - i < block_size) {
                    block_size = image_size - i;
                }
                for (j = 0; j < block_size; j++) {
                    buffer[j] = (i + j + version_no) % 77;
                }
                fota_crc_update(&crc_state, buffer, block_size);
                if (mira_fota_write(i, buffer, block_size) != MIRA_SUCCESS) {
                    printf("ERROR: mira_fota_write failed\n");
                    break;
                }
                PROCESS_WAIT_WHILE(mira_fota_is_working());

                /*
                 * Guarantee that each write yields, to let other tasks share CPU
                 * resources.
                 */
                PROCESS_PAUSE();
            }

            /* Write the header */
            if (mira_fota_write_header(image_size, fota_crc_get(&crc_state), 0, 0, version_no) !=
                MIRA_SUCCESS) {
                printf("ERROR: mira_fota_write_header failed\n");
                break;
            }
            PROCESS_WAIT_WHILE(mira_fota_is_working());

            /* Finish up the session */
            if (mira_fota_write_end() != MIRA_SUCCESS) {
                printf("ERROR: mira_fota_write_end failed\n");
                break;
            }
            PROCESS_WAIT_WHILE(mira_fota_is_working());
            printf("Generated version: %d for slot: %d\n", version_no, slot);
        }

        /* Wait for image to fully propagate before generating a new */
        etimer_set(&timer, CLOCK_SECOND * 60 * 60);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

        /* Version number should go from 0-254, since 255 is reserved */
        version_no = (version_no + 1) % 255;
    }

    PROCESS_END();
}
