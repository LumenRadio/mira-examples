/*
 * MIT License
 *
 * Copyright (c) 2024 LumenRadio AB
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

#include "mtk_bulk_data_collection.h"
#include "mtk_bdc_events.h"
#include "mtk_bdc_request.h"
#include "mtk_bdc_signal.h"

#define DEBUG_LEVEL 0
#include "mtk_bdc_utils.h"

/* Sub-packet period to request. This must be large enough so that TX queue do
 * no fill up, depending on the receiver's listening rate. */
#define SUB_PACKET_PERIOD_REQUEST_MS (800)

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
    .mode = MIRA_NET_MODE_ROOT,
    .rate = MIRA_NET_RATE_FAST,
    .antenna = 0,
    .prefix = NULL /* default prefix */
};

MIRA_IODEFS(MIRA_IODEF_NONE,    /* fd 0: stdin */
            MIRA_IODEF_UART(0), /* fd 1: stdout */
            MIRA_IODEF_NONE     /* fd 2: stderr */
                                /* More file descriptors can be added, for use with dprintf(); */
);

/* +1 for possible extra string termination */
static uint8_t large_packet_payload_storage[MTK_BULK_DATA_COLLECTION_SUBPACKET_MAX_BYTES *
                                              MTK_BULK_DATA_COLLECTION_MAX_NUMBER_OF_SUBPACKETS +
                                            1];
static mtk_bulk_data_collection_packet_t large_packet_rx;

static bool collection_in_progress = false;

PROCESS(main_proc, "Main process");
PROCESS(signal_to_request_proc, "Reply to signal with request process");
PROCESS(bulk_data_collection_monitor_proc, "Monitor incoming large packets");

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

    uart_ret = mira_uart_init(0, &uart_config);
    if (uart_ret != MIRA_SUCCESS) {
        /* Nowhere to send an error message */
    }

    /* Setup large packet storage */
    large_packet_rx = (mtk_bulk_data_collection_packet_t){ 0 };
    large_packet_rx.payload = large_packet_payload_storage;

    process_start(&main_proc, NULL);
}

PROCESS_THREAD(main_proc, ev, data)
{
    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup. */
    PROCESS_PAUSE();

    printf("Starting Root (Bulk data collection receiver).\n");

    MIRA_RUN_CHECK(mira_net_init(&net_config));

    RUN_CHECK(mtk_bulk_data_collection_init(MTK_BULK_DATA_COLLECTION_RECEIVER));
    process_start(&signal_to_request_proc, NULL);
    process_start(&bulk_data_collection_monitor_proc, NULL);

    PROCESS_END();
}

PROCESS_THREAD(signal_to_request_proc, ev, data)
{
    PROCESS_BEGIN();

    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == event_bdc_signaled_ready && !collection_in_progress);
        mtk_bdc_event_signaled_data_t signaled_data = *(mtk_bdc_event_signaled_data_t*)data;
        collection_in_progress = true;
        /* This example always replies to signal with an immediate request. If
         * there is need to schedule requests of large packets in a smarter way,
         * here would be the place to do it. It will start a transfer with the first sender that
         * sends a request. Request received during a transfer will be ignored */

        /* Request the whole large packet. */
        static uint64_t mask;

        int ret;
        ret = mtk_bulk_data_collection_send_whole_mask_get(&mask, signaled_data.n_sub_packets);

        if (ret < 0) {
            P_ERR("%s: could not get mask for large packet\n", __func__);
            continue;
        }

        /* Setting up for reception */
        large_packet_rx =
          (mtk_bulk_data_collection_packet_t){ .node_addr = signaled_data.src,
                                               .node_port = signaled_data.src_port,
                                               .payload = large_packet_payload_storage,
                                               .len = 0,
                                               .id = signaled_data.packet_id,
                                               .period_ms = SUB_PACKET_PERIOD_REQUEST_MS,
                                               .mask = 0, /* bit at 1 means sub-packet received */
                                               .num_sub_packets = signaled_data.n_sub_packets };

        /* Send request for sub-packets, back to the signaling node */
        RUN_CHECK(mtk_bdcreq_send(&signaled_data.src,
                                  signaled_data.src_port,
                                  signaled_data.packet_id,
                                  mask,
                                  SUB_PACKET_PERIOD_REQUEST_MS));

        /* Start or restart sub-packet rx monitor */
        process_exit(&mtk_bulk_data_collection_receive_proc);
        // The other process needs to handle its exit event
        // before it can be started again, so a PAUSE is needed:
        PROCESS_PAUSE();
        process_start(&mtk_bulk_data_collection_receive_proc, &large_packet_rx);
    }

    PROCESS_END();
}

PROCESS_THREAD(bulk_data_collection_monitor_proc, ev, data)
{
    PROCESS_BEGIN();

    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == event_bdc_received);
        printf("Bulk data received from %02x%02x, packet id: %u\n",
               large_packet_rx.node_addr.u8[14],
               large_packet_rx.node_addr.u8[15],
               large_packet_rx.id);
        large_packet_rx.payload[large_packet_rx.len] = '\0';
        collection_in_progress = false;
    }

    PROCESS_END();
}
