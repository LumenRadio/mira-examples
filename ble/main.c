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
#include <ble.h>
#include <stdio.h>
#include <string.h>
#include "ble_led.h"
#include "ble_app.h"

#define UART_ACTIVE (1)
#define UART_TX_PORT (0)
#define UART_RX_PORT (0)
#define UART_TX_PIN (6)
#define UART_RX_PIN (8)

#ifdef NRF52832_XXAA
#define BUTTON_PIN 13
#else
#define BUTTON_PIN 11
#endif
#define BUTTON_PRESSED_LED 3

#define UDP_PORT 456
#define MIRA_SEND_INTERVAL_S 60
#define MIRA_CHECK_NET_INTERVAL_S 5

MIRA_IODEFS(MIRA_IODEF_NONE,
#if UART_ACTIVE
            MIRA_IODEF_UART(0),
#else
            MIRA_IODEF_NONE,
#endif
            MIRA_IODEF_NONE);

PROCESS(mira_proc, "MIRA process");
PROCESS(ble_proc, "BLE process");

/*
 * Identifies as a node.
 * Sends data to a root with the same pan_id and key.
 * Compatible with the network_receiver example.
 */
static const mira_net_config_t net_config = {
    .pan_id = 0x13243546,
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
    .mode = MIRA_NET_MODE_MESH,
    .rate = MIRA_NET_RATE_MID,
    .antenna = 0,
    .prefix = NULL /* default prefix */
};

void mira_setup(void)
{
    MIRA_MEM_SET_BUFFER(8616);

#if UART_ACTIVE
    mira_uart_config_t uart_config = { .baudrate = 115200,
                                       .tx_pin = MIRA_GPIO_PIN(UART_TX_PORT, UART_TX_PIN),
                                       .rx_pin = MIRA_GPIO_PIN(UART_RX_PORT, UART_RX_PIN) };
    mira_uart_init(0, &uart_config);
#endif

    mira_status_t result = ble_led_init();
    if (result != MIRA_SUCCESS) {
        printf("ERROR[%d]: ble_led_init()\n", result);
    }
    process_start(&ble_proc, NULL);
    process_start(&mira_proc, NULL);
}

/* Process for handling BLE */
PROCESS_THREAD(ble_proc, ev, data)
{
    static struct etimer ble_timer;
    static mira_bool_t previous_button_value;

    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup */
    PROCESS_PAUSE();

    /* Start BLE example */
    printf("Starting BLE stack\n");
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    advertising_start();

    mira_gpio_set_dir(BUTTON_PIN, MIRA_GPIO_DIR_IN);
    mira_gpio_set_pull(BUTTON_PIN, MIRA_GPIO_PULL_UP);
    mira_gpio_get_value(BUTTON_PIN, &previous_button_value);

    while (1) {
        mira_bool_t button_value;
        mira_gpio_get_value(BUTTON_PIN, &button_value);
        ble_led_set(BUTTON_PRESSED_LED, !button_value);
        if (button_value != previous_button_value) {
            printf("Button 1 pressed \n");
            fflush(stdout);
            uint32_t ret = notify(!button_value);
            if (ret != NRF_SUCCESS && ret != BLE_ERROR_INVALID_CONN_HANDLE &&
                ret != NRF_ERROR_INVALID_STATE && ret != BLE_ERROR_GATTS_SYS_ATTR_MISSING) {
                printf("ERROR[0x%lx]: notify()\n", ret);
            }
            previous_button_value = button_value;
        }

        etimer_set(&ble_timer, CLOCK_SECOND / 5);
        PROCESS_YIELD_UNTIL(etimer_expired(&ble_timer));
    }

    PROCESS_END();
}

/* Mira network udp callback */
static void udp_listen_callback(mira_net_udp_connection_t* connection,
                                const void* data,
                                uint16_t data_len,
                                const mira_net_udp_callback_metadata_t* metadata,
                                void* storage)
{
    char buffer[MIRA_NET_MAX_ADDRESS_STR_LEN];
    uint16_t i;

    printf("Received Mira message from [%s]:%u: ",
           mira_net_toolkit_format_address(buffer, metadata->source_address),
           metadata->source_port);
    for (i = 0; i < data_len - 1; i++) {
        printf("%c", ((char*)data)[i]);
    }
    printf("\n");
}

/* Process for sending Mira messages */
PROCESS_THREAD(mira_proc, ev, data)
{
    static struct etimer timer;

    static mira_net_udp_connection_t* udp_connection;

    static mira_net_address_t net_address;
    static char buffer[MIRA_NET_MAX_ADDRESS_STR_LEN];
    static mira_status_t res;
    static const char* message = "Hello Network";

    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup */
    PROCESS_PAUSE();

    printf("Starting Mira stack (Sender)\n");
    printf("Sending one Mira packet every %d seconds\n", MIRA_SEND_INTERVAL_S);

    mira_status_t result = mira_net_init(&net_config);
    if (result) {
        printf("FAILURE: mira_net_init returned %d\n", result);
        while (1)
            ;
    }

    /*
     * Open a connection, but don't specify target address yet, which means
     * only mira_net_udp_send_to() can be used to send packets later.
     */
    udp_connection = mira_net_udp_connect(NULL, 0, udp_listen_callback, NULL);

    while (1) {
        mira_net_state_t net_state = mira_net_get_state();

        if (net_state != MIRA_NET_STATE_JOINED) {
            printf("Waiting for network (state is %s)\n",
                   net_state == MIRA_NET_STATE_NOT_ASSOCIATED ? "not associated"
                   : net_state == MIRA_NET_STATE_ASSOCIATED   ? "associated"
                   : net_state == MIRA_NET_STATE_JOINED       ? "joined"
                                                              : "UNKNOWN");
            etimer_set(&timer, MIRA_CHECK_NET_INTERVAL_S * CLOCK_SECOND);
        } else {
            /* Try to retrieve the root address. */
            res = mira_net_get_root_address(&net_address);

            if (res != MIRA_SUCCESS) {
                printf("Waiting for root address (res: %d)\n", res);
                etimer_set(&timer, MIRA_CHECK_NET_INTERVAL_S * CLOCK_SECOND);
            } else {
                /*
                 * Root address is successfully retrieved, send a message to the
                 * root node on the given UDP Port.
                 */
                printf("Sending to address: %s\n",
                       mira_net_toolkit_format_address(buffer, &net_address));
                mira_net_udp_send_to(
                  udp_connection, &net_address, UDP_PORT, message, strlen(message));
                etimer_set(&timer, MIRA_SEND_INTERVAL_S * CLOCK_SECOND);
            }
        }
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    }

    mira_net_udp_close(udp_connection);

    PROCESS_END();
}
