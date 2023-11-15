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

#if MIRA_PLATFORM_MKW41Z
#define LED1_PIN MIRA_GPIO_PIN('B', 0)
#define LED2_PIN MIRA_GPIO_PIN('C', 1)
#define LED3_PIN MIRA_GPIO_PIN('A', 19)
#define LED4_PIN MIRA_GPIO_PIN('A', 18)
#else
#define LED1_PIN MIRA_GPIO_PIN(0, 17)
#define LED2_PIN MIRA_GPIO_PIN(0, 18)
#define LED3_PIN MIRA_GPIO_PIN(0, 19)
#define LED4_PIN MIRA_GPIO_PIN(0, 20)
#endif

MIRA_IODEFS(MIRA_IODEF_NONE, MIRA_IODEF_NONE, MIRA_IODEF_NONE, );

PROCESS(main_proc, "Main process");

void mira_setup(void)
{
    MIRA_MEM_SET_BUFFER(7592);

    mira_gpio_set_dir(LED1_PIN, MIRA_GPIO_DIR_OUT);
    mira_gpio_set_dir(LED2_PIN, MIRA_GPIO_DIR_OUT);
    mira_gpio_set_dir(LED3_PIN, MIRA_GPIO_DIR_OUT);
    mira_gpio_set_dir(LED4_PIN, MIRA_GPIO_DIR_OUT);

    process_start(&main_proc, NULL);
}

PROCESS_THREAD(main_proc, ev, data)
{
    static int i;
    static struct etimer timer;

    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup */
    PROCESS_PAUSE();

    etimer_set(&timer, CLOCK_SECOND);

    while (1) {
        /* Negate, since LEDs are active low */
        mira_gpio_set_value(LED1_PIN, !(i % 4 == 0));
        mira_gpio_set_value(LED2_PIN, !(i % 4 == 1));
        mira_gpio_set_value(LED3_PIN, !(i % 4 == 2));
        mira_gpio_set_value(LED4_PIN, !(i % 4 == 3));

        PROCESS_YIELD_UNTIL(etimer_expired(&timer));
        etimer_reset(&timer);

        i++;
    }

    PROCESS_END();
}
