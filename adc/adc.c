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
 * ADC example.
 */

#define ADC_PIN (MIRA_GPIO_PIN(0, 4))

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
    static mira_adc_context_t vdd_context;
    static mira_adc_value_t vdd_value;
    static mira_adc_context_t pin_context;
    static mira_adc_value_t pin_value;

    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup */
    PROCESS_PAUSE();

    printf("ADC example\n");

    mira_adc_init(&vdd_context);
    mira_adc_set_reference(&vdd_context, MIRA_ADC_REF_INT_3_6V);
    mira_adc_set_source_supply(&vdd_context);

    mira_adc_init(&pin_context);
    mira_adc_set_reference(&pin_context, MIRA_ADC_REF_VDD);
    mira_adc_set_source_single(&pin_context, ADC_PIN);

    while (1) {
        /* Measure VDD */
        mira_adc_measurement_start(&vdd_context);
        PROCESS_WAIT_EVENT_UNTIL(!mira_adc_measurement_in_progress(&vdd_context));
        mira_adc_measurement_finish(&vdd_context, &vdd_value);

        /* Measure ADC pin */
        mira_adc_measurement_start(&pin_context);
        PROCESS_WAIT_EVENT_UNTIL(!mira_adc_measurement_in_progress(&pin_context));
        mira_adc_measurement_finish(&pin_context, &pin_value);

        /* An internal 3.6 V reference is used: multiply with 3600 to get mV. */
        int32_t vdd_mv = (int32_t)vdd_value * 3600 / 32768;
        int32_t pin_mv = (int32_t)pin_value * vdd_mv / 32768;
        printf("Vdd: %6ld mV Pin: %6ld mV\n", vdd_mv, pin_mv);

        etimer_set(&timer, 1 * CLOCK_SECOND);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    }

    PROCESS_END();
}
