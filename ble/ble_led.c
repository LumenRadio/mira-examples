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

#include "ble_led.h"

#ifdef NRF52832_XXAA
#define LED1_PIN MIRA_GPIO_PIN(0, 17)
#define LED2_PIN MIRA_GPIO_PIN(0, 18)
#define LED3_PIN MIRA_GPIO_PIN(0, 19)
#define LED4_PIN MIRA_GPIO_PIN(0, 20)
#else
#define LED1_PIN MIRA_GPIO_PIN(0, 13)
#define LED2_PIN MIRA_GPIO_PIN(0, 14)
#define LED3_PIN MIRA_GPIO_PIN(0, 15)
#define LED4_PIN MIRA_GPIO_PIN(0, 16)
#endif

mira_status_t ble_led_init(void)
{
    mira_status_t result;

    result = mira_gpio_set_dir(LED1_PIN, MIRA_GPIO_DIR_OUT);
    if (result != MIRA_SUCCESS) {
        return result;
    }
    result = mira_gpio_set_dir(LED2_PIN, MIRA_GPIO_DIR_OUT);
    if (result != MIRA_SUCCESS) {
        return result;
    }
    result = mira_gpio_set_dir(LED3_PIN, MIRA_GPIO_DIR_OUT);
    if (result != MIRA_SUCCESS) {
        return result;
    }
    result = mira_gpio_set_dir(LED4_PIN, MIRA_GPIO_DIR_OUT);
    if (result != MIRA_SUCCESS) {
        return result;
    }

    ble_led_set(1, MIRA_FALSE);
    ble_led_set(2, MIRA_FALSE);
    ble_led_set(3, MIRA_FALSE);
    ble_led_set(4, MIRA_FALSE);

    return MIRA_SUCCESS;
}

mira_status_t ble_led_set(int index, mira_bool_t on)
{
    mira_gpio_pin_t led_gpio;
    switch (index) {
        case 1:
            led_gpio = LED1_PIN;
            break;

        case 2:
            led_gpio = LED2_PIN;
            break;

        case 3:
            led_gpio = LED3_PIN;
            break;

        case 4:
            led_gpio = LED4_PIN;
            break;

        default:
            return MIRA_ERROR_INVALID_VALUE;
    }

    /* LED are active low */
    return mira_gpio_set_value(led_gpio, !on);
}
