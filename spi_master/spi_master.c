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

/*
 * SPI transfer example. Connect MISO_PIN to MOSI_PIN.
 */

#define SPI_ID 0
#define MISO_PIN MIRA_GPIO_PIN(0, 25)
#define MOSI_PIN MIRA_GPIO_PIN(0, 24)
#define SCK_PIN MIRA_GPIO_PIN(0, 23)
#define SS_PIN MIRA_GPIO_PIN(0, 22)

MIRA_IODEFS(MIRA_IODEF_NONE,    /* fd 0: stdin */
            MIRA_IODEF_UART(0), /* fd 1: stdout */
            MIRA_IODEF_NONE     /* fd 2: stderr */
);

PROCESS(main_proc, "Main process");

void mira_setup(void)
{
    MIRA_MEM_SET_BUFFER(7592);
    mira_uart_config_t uart_config = { .baudrate = 115200,
                                       .tx_pin = MIRA_GPIO_PIN(0, 6),
                                       .rx_pin = MIRA_GPIO_PIN(0, 8) };

    mira_status_t result = mira_uart_init(0, &uart_config);
    if (result != MIRA_SUCCESS) {
        /* Nowhere to send an error message */
    }

    process_start(&main_proc, NULL);
}

PROCESS_THREAD(main_proc, ev, data)
{
    static struct etimer timer;
    static uint8_t tx_buffer[] = "Hello world!";
    static uint8_t rx_buffer[sizeof(tx_buffer)];

    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup */
    PROCESS_PAUSE();

    mira_spi_config_t spi_config = { .frequency = 125000,
                                     .sck_pin = SCK_PIN,
                                     .mosi_pin = MOSI_PIN,
                                     .miso_pin = MISO_PIN,
                                     .ss_pin = SS_PIN,
                                     .mode = MIRA_SPI_MODE_0,
                                     .bit_order = MIRA_BIT_ORDER_MSB_FIRST };

    mira_status_t result = mira_spi_init(SPI_ID, &spi_config);
    if (result != MIRA_SUCCESS) {
        printf("SPI initialization failed with status: %u.\n", result);
    }

    while (1) {
        rx_buffer[0] = '\0';

        printf("*\n");
        printf("Sending '%s'\n", tx_buffer);
        result =
          mira_spi_transfer(SPI_ID, tx_buffer, sizeof(tx_buffer), rx_buffer, sizeof(rx_buffer));
        if (result != MIRA_SUCCESS) {
            printf("SPI transfer failed with status: %u.\n", result);
        }

        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL &&
                                 !mira_spi_transfer_is_in_progress(SPI_ID));
        rx_buffer[sizeof(rx_buffer) - 1] = '\0';
        printf("Received '%s'\n", rx_buffer);

        etimer_set(&timer, CLOCK_SECOND);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    }

    PROCESS_END();
}
