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
#include <stdbool.h>
#include <stdio.h>

#if nrf52832 || nrf52832ble
#define LED1_PIN MIRA_GPIO_PIN(0, 17)
#define LED2_PIN MIRA_GPIO_PIN(0, 18)
#define LED3_PIN MIRA_GPIO_PIN(0, 19)
#define LED4_PIN MIRA_GPIO_PIN(0, 20)
#elif nrf52840 || nrf52840ble || nrf52833ble
#define LED1_PIN MIRA_GPIO_PIN(0, 13)
#define LED2_PIN MIRA_GPIO_PIN(0, 14)
#define LED3_PIN MIRA_GPIO_PIN(0, 15)
#define LED4_PIN MIRA_GPIO_PIN(0, 16)
#elif MIRA_PLATFORM_MKW41Z
#define LED1_PIN MIRA_GPIO_PIN('C', 1)
#define LED2_PIN MIRA_GPIO_PIN('A', 18)
#define LED3_PIN MIRA_GPIO_PIN('A', 19)
#define LED4_PIN MIRA_GPIO_PIN('B', 0)
#else
#error Platform not defined!
#endif

#define LED_BLINK_PIN LED1_PIN

/*
 * Identifies as a root, which is the time source of the network
 */
static const mira_net_config_t net_config = {
    .pan_id = 0x87654321,
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
    .mode = MIRA_NET_MODE_ROOT_NO_RECONNECT,
    .rate = MIRA_NET_RATE_FAST,
    .antenna = 0,
    .prefix = NULL /* default prefix */
};

MIRA_IODEFS(MIRA_IODEF_NONE, MIRA_IODEF_NONE, MIRA_IODEF_NONE, );

PROCESS(main_proc, "Main process");
PROCESS(time_callback_scheduler, "Callback scheduler process");

static void time_callback(mira_net_time_t tick, void* storage)
{
    /*
     * Make pin blink every 1.28 seconds, assuming 10ms ticks
     *
     * Using 1.28 seconds, to keep calculation fast, and therefore latency low
     */
    mira_gpio_set_value(LED_BLINK_PIN, tick & 0x00000080);

    /* Schedule next wakeup */
    process_poll(&time_callback_scheduler);
}

void mira_setup(void)
{
    mira_gpio_set_dir(LED_BLINK_PIN, MIRA_GPIO_DIR_OUT);

    MIRA_MEM_SET_BUFFER(14944);

    process_start(&main_proc, NULL);
    process_start(&time_callback_scheduler, NULL);
}

PROCESS_THREAD(time_callback_scheduler, ev, data)
{
    PROCESS_BEGIN();
    PROCESS_PAUSE();

    /* Tick value for next run of time_callback */
    static uint32_t time_callback_tick = 0;

    PROCESS_YIELD_UNTIL((mira_net_time_get_time(&time_callback_tick) == MIRA_SUCCESS));

    do {
        mira_net_time_schedule(time_callback_tick, time_callback, NULL);

        uint32_t net_time;
        PROCESS_YIELD_UNTIL((mira_net_time_get_time(&net_time) == MIRA_SUCCESS) &&
                            ((int32_t)(net_time - time_callback_tick) >= 0));

        time_callback_tick = (net_time & 0xffffff80) + 0x00000080;
    } while (1);

    PROCESS_END();
}

PROCESS_THREAD(main_proc, ev, data)
{
    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup */
    PROCESS_PAUSE();

    /* Should have network enabled, to keep time sync */
    mira_status_t result = mira_net_init(&net_config);
    if (result) {
        printf("FAILURE: mira_net_init returned %d\n", result);
        while (1)
            ;
    }

    PROCESS_END();
}
