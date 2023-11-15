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
#include "command_defs.h"
#include "rpc-interface.h"
#include "app-config.h"
#include "reboot.h"

#if CONTIKI_TARGET_MKW41Z
/* If target is mkw41z, assume rigado devboard pinout */
#define UART_TX_PIN CPU_GPIO('C', 7)
#define UART_RX_PIN CPU_GPIO('C', 6)
#else
/* If mkw41z isn't defined, assume nrf52 devboard pinout */
#define UART_TX_PIN CPU_GPIO(0, 6)
#define UART_RX_PIN CPU_GPIO(0, 8)

#define RED_LED MIRA_GPIO_PIN(0, 28)
#define GREEN_LED MIRA_GPIO_PIN(0, 29)
#define BLUE_LED MIRA_GPIO_PIN(0, 30)

#endif

typedef struct
{
    uint16_t device_id_suffix;
    const char* command;
} node_command_t;

static int _usb_uart_write(int fd, uint8_t id, const void* ptr, int len)
{
    (void)id;
#if defined(NRF52840_XXAA) || defined(NRF52833_XXAA)
    mira_iodef_t usb_iodef = MIRA_IODEF_USB;
#endif
    mira_iodef_t uart_iodef = MIRA_IODEF_UART(0);

#if defined(NRF52840_XXAA) || defined(NRF52833_XXAA)
    usb_iodef.write(fd, usb_iodef.id, ptr, len);
#endif
    return uart_iodef.write(fd, uart_iodef.id, ptr, len);
}

MIRA_IODEFS(MIRA_IODEF_NONE,                 /* fd 0: stdin */
            MIRA_IODEF_OUT(_usb_uart_write), /* fd 1: stdout */
            MIRA_IODEF_NONE,                 /* fd 2: stderr */
            MIRA_IODEF_USB,                  /* fd 3 */
            MIRA_IODEF_UART(0)               /* fd 4 */

);

static void init_leds(void)
{
    mira_gpio_set_value(RED_LED, MIRA_TRUE);
    mira_gpio_set_value(GREEN_LED, MIRA_TRUE);
    mira_gpio_set_value(BLUE_LED, MIRA_TRUE);

    mira_gpio_set_dir(RED_LED, MIRA_GPIO_DIR_OUT);
    mira_gpio_set_dir(GREEN_LED, MIRA_GPIO_DIR_OUT);
    mira_gpio_set_dir(BLUE_LED, MIRA_GPIO_DIR_OUT);
}

static void set_leds_not_associated(void)
{
    mira_gpio_set_value(RED_LED, MIRA_FALSE);
    mira_gpio_set_value(GREEN_LED, MIRA_TRUE);
    mira_gpio_set_value(BLUE_LED, MIRA_TRUE);
}

static void set_leds_associated(void)
{
    mira_gpio_set_value(RED_LED, MIRA_FALSE);
    mira_gpio_set_value(GREEN_LED, MIRA_TRUE);
    mira_gpio_set_value(BLUE_LED, MIRA_FALSE);
}

static void set_leds_joined(void)
{
    mira_gpio_set_value(RED_LED, MIRA_TRUE);
    mira_gpio_set_value(GREEN_LED, MIRA_TRUE);
    if ((clock_seconds() % 2) == 0) {
        mira_gpio_set_value(BLUE_LED, MIRA_TRUE);
    } else {
        mira_gpio_set_value(BLUE_LED, MIRA_FALSE);
    }
}

PROCESS(main_proc, "Main process");

void mira_setup(void)
{
    mira_status_t uart_ret;
    mira_uart_config_t uart_config = { .baudrate = 115200,
                                       .tx_pin = MIRA_GPIO_PIN(0, 6),
                                       .rx_pin = MIRA_GPIO_PIN(0, 8) };
    MIRA_MEM_SET_BUFFER(8680);

    uart_ret = mira_uart_init(0, &uart_config);
    if (uart_ret != MIRA_SUCCESS) {
        /* Nowhere to send an error message */
    }

#if defined(NRF52840_XXAA) || defined(NRF52833_XXAA)
    if (mira_usb_uart_init(0) != MIRA_SUCCESS) {
        printf("mira_usb_init failed\n");
    }
#endif

    init_leds();

    rpc_interface_init(rpc_interface_command_handler, 1, command_defs);
    process_start(&main_proc, NULL);
}

static int serial_input_byte(unsigned char c, void* storage)
{
    (void)storage;

    reboot_parser_input(c);
    rpc_interface_input_byte(c);

    return 0;
}

PROCESS_THREAD(main_proc, ev, data)
{
    static struct etimer timer;
    mira_net_config_t netconf;
    static mira_net_state_t previous_net_state = MIRA_NET_STATE_NOT_ASSOCIATED;

    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup */
    PROCESS_PAUSE();

    app_config_init();

    if (app_config_is_configured()) {
        printf("Configuration: Done\n");
    } else {
        printf("Configuration: No config available\n");
    }

    memset(&netconf, 0, sizeof(mira_net_config_t));
    netconf.pan_id = app_config.net_pan_id;
    memcpy(netconf.key, app_config.net_key, 16);
    netconf.mode = MIRA_NET_MODE_MESH;
    netconf.antenna = app_config.antenna;
    netconf.rate = app_config.net_rate;
    if (netconf.rate > 15) {
        netconf.rate = 15;
    }

    mira_net_init(&netconf);
    print_config();

    mira_status_t result = mira_uart_set_input_callback(0, serial_input_byte, NULL);
    if (result != MIRA_SUCCESS) {
        printf("error: mira_uart_set_input_callback (%d)\n", result);
    }

#if defined(NRF52840_XXAA) || defined(NRF52833_XXAA)
    result = mira_usb_uart_set_input_callback(serial_input_byte, NULL);
    if (result != MIRA_SUCCESS) {
        printf("error: mira_usb_uart_set_input_callback (%d)\n", result);
    }
#endif

    /* Start fota process, which polls updates from network */
    mira_fota_init();

    while (1) {
        mira_net_state_t net_state = mira_net_get_state();

        switch (net_state) {
            case MIRA_NET_STATE_NOT_ASSOCIATED:
                set_leds_not_associated();
                break;

            case MIRA_NET_STATE_ASSOCIATED:
                set_leds_associated();
                break;

            case MIRA_NET_STATE_JOINED:
            case MIRA_NET_STATE_IS_COORDINATOR:
                set_leds_joined();
                break;
        }

        if (net_state != previous_net_state) {
            previous_net_state = net_state;
            print_net_state();
        }

        etimer_set(&timer, CLOCK_SECOND * 1);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    }

    PROCESS_END();
}
