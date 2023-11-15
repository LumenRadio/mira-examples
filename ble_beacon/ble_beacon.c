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

/**
 * Static Eddystone-URL payload
 */
uint8_t ble_beacon_payload[] = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, /* Source address */
                                 0x03, 0x03, 0xaa, 0xfe,             /* Eddystone ID */
                                 20,                                 /* Length of rest of packet */
                                 0x16, 0xaa, 0xfe,                   /* Eddystone ID */
                                 0x10,                               /* Eddystone URL */
                                 0xfe,                               /* TX power */
                                 0x03,                               /* https:// */
                                 'l',  'u',  'm',  'e',  'n',  'r',  'a',
                                 'd',  'i',  'o',  '.',  'c',  'o',  'm' };

MIRA_IODEFS(MIRA_IODEF_NONE, MIRA_IODEF_NONE, MIRA_IODEF_NONE);

PROCESS(main_proc, "Main process");

void mira_setup(void)
{
    MIRA_MEM_SET_BUFFER(7592);
    process_start(&main_proc, NULL);
}

PROCESS_THREAD(main_proc, ev, data)
{
    static struct etimer beacon_timer;

    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup */
    PROCESS_PAUSE();

    while (1) {
        mira_ble_advertisement_send_adv_nonconn_ind(MIRA_BLE_ADVERTISEMENT_ADDRESS_TYPE_STATIC,
                                                    ble_beacon_payload,
                                                    sizeof(ble_beacon_payload));

        etimer_set(&beacon_timer, CLOCK_SECOND);
        PROCESS_YIELD_UNTIL(etimer_expired(&beacon_timer));
    }

    PROCESS_END();
}
