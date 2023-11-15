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

MIRA_IODEFS(MIRA_IODEF_NONE,    /* fd 0: stdin */
            MIRA_IODEF_UART(0), /* fd 1: stdout */
            MIRA_IODEF_NONE,    /* fd 2: stderr */
            MIRA_IODEF_USB      /* fd 3 */
                                /* More file descriptors can be added, for use with dprintf(); */
);

/* The name of the USB device: */
uint8_t mira_usb_product_name[] = "MIRA-USB-UART";

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
    mira_usb_config_t usb_config = {
        .vendor_id = 0x1915,  // Nordic Semiconductor's
        .product_id = 0x520f, // Product ID from Nordic's SDK example
    };

    if (mira_usb_uart_init(&usb_config) != MIRA_SUCCESS) {
        printf("mira_usb_init failed\n");
    }

    process_start(&main_proc, NULL);
}

static int usb_uart_callback(uint8_t c, void* storage)
{
    putchar(c);
    return 0;
}

PROCESS_THREAD(main_proc, ev, data)
{
    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup. */
    PROCESS_PAUSE();

    mira_usb_uart_set_input_callback(usb_uart_callback, NULL);

    static struct etimer et;
    while (1) {
        if (mira_usb_uart_is_connected()) {
            printf("Sending to USB port.\n");
        } else {
            printf("USB port not opened.\n");
        }
        dprintf(3, "Hello USB!\n");
        etimer_set(&et, CLOCK_SECOND);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    }

    PROCESS_END();
}
