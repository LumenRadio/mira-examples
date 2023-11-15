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
#include "mira_diag_log.h"

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
    .mode = MIRA_NET_MODE_MESH,
    .rate = MIRA_NET_RATE_FAST,
    .antenna = 0,
    .prefix = NULL /* default prefix */
};

/*
 * Identifies as a node.
 * Sends data to the root.
 */

MIRA_IODEFS(MIRA_IODEF_NONE,    /* fd 0: stdin */
            MIRA_IODEF_UART(0), /* fd 1: stdout */
            MIRA_IODEF_NONE     /* fd 2: stderr */
                                /* More file descriptors can be added, for use with dprintf(); */
);

static bool return_true(void)
{
    return true;
}

enum
{
#include "events.h.gen"
};

static void print_log_argument(va_list* ap)
{
    char address_buf[MIRA_NET_MAX_ADDRESS_STR_LEN];
    mira_diag_log_arg_type_t type = LOG_ARG_END;
    type = va_arg(*ap, uint32_t);
    if (type != LOG_ARG_END) {
        mira_diag_log_arg_t arg = va_arg(*ap, mira_diag_log_arg_t);
        switch (type) {

            case LOG_ARG_U8:
                printf("%" PRIu32, (uint32_t)arg.u8);
                break;

            case LOG_ARG_U16:
                printf("%" PRIu16, arg.u16);
                break;

            case LOG_ARG_U32:
                printf("%" PRIu32, arg.u32);
                break;

            case LOG_ARG_I8:
                printf("%" PRIi8, arg.i8);
                break;

            case LOG_ARG_I16:
                printf("%" PRIi16, arg.i16);
                break;

            case LOG_ARG_I32:
                printf("%" PRIi32, arg.i32);
                break;

            case LOG_ARG_BOOL:
                printf("%c", arg.b ? 'Y' : 'N');
                break;

            case LOG_ARG_STR:
                printf("%s", arg.str);
                break;

            case LOG_ARG_PTR:
                printf("%p", arg.ptr);
                break;

            case LOG_ARG_2BYTE: {
                const uint8_t* ptr = arg.ptr;
                printf("%02x%02x", ptr[1], ptr[0]);
            } break;

            case LOG_ARG_5BYTE: {
                const uint8_t* ptr = arg.ptr;
                printf("%02x%02x%02x%02x%02x", ptr[4], ptr[3], ptr[2], ptr[1], ptr[0]);
            } break;

            case LOG_ARG_IPV6:
                printf("%s",
                       mira_net_toolkit_format_address(address_buf, (mira_net_address_t*)arg.ptr));
                break;

            default:
                break;
        }
    }
}

static void log_with_printf(uint32_t evt, va_list ap)
{
    switch (evt) {
        case EVT_APPS_SWAP_TRANSFER_START_NEW_TRANSFER_FROM:
            printf("Swap client: Fetching new firmware from ");
            print_log_argument(&ap);
            printf("\n");
            break;

        case EVT_APPS_SWAP_TRANSFER_REQUESTING_DATA:
            printf("Swap client: Requesting ");
            print_log_argument(&ap);
            printf(" @ "); // At position
            print_log_argument(&ap);
            printf("\n");
            break;

        case EVT_APPS_SWAP_TRANSFER_MAX_TRIES_ABORTING:
            printf("Swap client: max tries, abort!\n");
            break;

        case EVT_APPS_SWAP_TRANSFER_PACKET_RECIEVED_AFTER_RETRIES:
            printf("Packet received after ");
            print_log_argument(&ap);
            printf(" retries\n");
            break;

        default:
            return;
    }
}

PROCESS(main_proc, "Main process");

void mira_setup(void)
{
    mira_status_t uart_ret;
    mira_uart_config_t uart_config = { .baudrate = 115200,
                                       .tx_pin = MIRA_GPIO_PIN(0, 6),
                                       .rx_pin = MIRA_GPIO_PIN(0, 8) };

    mira_diag_log_callbacks_t cbs = {
        .is_debug_enabled = return_true,
        .is_info_enabled = return_true,
        .app_log = { log_with_printf, log_with_printf, log_with_printf, log_with_printf },
    };
    mira_diag_log_set_callbacks(&cbs);

    MIRA_MEM_SET_BUFFER(8616);

    uart_ret = mira_uart_init(0, &uart_config);
    if (uart_ret != MIRA_SUCCESS) {
        /* Nowhere to send an error message */
    }

    process_start(&main_proc, NULL);
}

const char* net_state(void)
{
    switch (mira_net_get_state()) {
        case MIRA_NET_STATE_NOT_ASSOCIATED:
            return "not associated";

        case MIRA_NET_STATE_IS_COORDINATOR:
            return "is coordinator";

        case MIRA_NET_STATE_ASSOCIATED:
            return "associated";

        case MIRA_NET_STATE_JOINED:
            return "joined";
    }
    return "unknown";
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

    /* Start fota process, which polls updates from network */
    result = mira_fota_init();
    if (result != MIRA_SUCCESS) {
        printf("FAILURE: mira_fota_init returned %d\n", result);
        while (1)
            ;
    }

    while (1) {
        etimer_set(&timer, CLOCK_SECOND);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

        if (mira_fota_is_valid(0)) {
            printf("%s, Valid image: %ld bytes, version %d\n",
                   net_state(),
                   mira_fota_get_image_size(0),
                   mira_fota_get_version(0));
        } else {
            printf("%s, No valid image available in cache\n", net_state());
        }
    }

    PROCESS_END();
}
