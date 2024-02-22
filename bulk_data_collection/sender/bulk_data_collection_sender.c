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
#include <string.h>

#include "mtk_bulk_data_collection.h"
#include "mtk_bdc_events.h"
#include "mtk_bdc_signal.h"

#define DEBUG_LEVEL 0
#include "mtk_bdc_utils.h"

/*
 * Identifies as a node.
 * Sends data to the root.
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
    .rate = MIRA_NET_RATE_FAST,
    .antenna = 0,
    .prefix = NULL /* default prefix */
};

static mtk_bulk_data_collection_packet_t mtk_bulk_data_collection_packet_tx;

/*
 * How often to check if we have access to root.
 */
#define WAITING_FOR_ROOT_PERIOD_S (5)

/*
 * Give time for the downward route to establish. Having access to root does not
 * mean that the root has access to us.
 */
#define ESTABLISHING_ROUTE_DELAY_S (40)

/*
 * How often to start sending a new large packet.
 */
#define PACKET_GENERATION_PERIOD_S (30)

/**
 * Constant for random interval of generation
 */
#define PACKET_GENERATION_VARIATION_S (PACKET_GENERATION_PERIOD_S / 2)

/*
 * Large packet to send.
 */
static uint8_t packet_content[] =
  "Lorem ipsum dolor sit amet, consectetaur adipisicing elit, sed do eiusmod tempor"
  "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis"
  "nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo"
  "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum"
  "dolore eu fugiat nulla pariatur.  Excepteur sint occaecat cupidatat non"
  "proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\nSed"
  "ut perspiciatis unde omnis iste natus error sit voluptatem accusantium"
  "doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore"
  "veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam"
  "voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia"
  "consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt.  Neque"
  "porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci"
  "velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore"
  "magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum"
  "exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi"
  "consequatur?  Quis autem vel eum iure reprehenderit qui in ea voluptate velit"
  "esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo"
  "voluptas nulla pariatur?\nAt vero eos et accusamus et iusto odio dignissimos"
  "ducimus qui blanditiis praesentium voluptatum deleniti atque corrupti quos"
  "dolores et quas molestias excepturi sint occaecati cupiditate non provident,"
  "similique sunt in culpa qui officia deserunt mollitia animi, id est laborum et"
  "dolorum fuga. Et harum quidem rerum facilis est et expedita distinctio. Nam"
  "libero tempore, cum soluta nobis est eligendi optio cumque nihil impedit quo"
  "minus id quod maxime placeat facere possimus, omnis voluptas assumenda est,"
  "omnis dolor repellendus. Temporibus autem quibusdam et aut officiis debitis aut"
  "rerum necessitatibus saepe eveniet ut et voluptates repudiandae sint et"
  "molestiae non recusandae. Itaque earum rerum hic tenetur a sapiente delectus, ut"
  "aut reiciendis voluptatibus maiores alias consequatur aut perferendis doloribus"
  "asperiores repellat.\n"
  "Lorem ipsum dolor sit amet, consectetaur adipisicing elit, sed do eiusmod tempor"
  "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis"
  "nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo"
  "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum"
  "dolore eu fugiat nulla pariatur.  Excepteur sint occaecat cupidatat non"
  "proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\nSed"
  "ut perspiciatis unde omnis iste natus error sit voluptatem accusantium"
  "doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore"
  "veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam"
  "voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia"
  "consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt.  Neque"
  "porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci"
  "velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore"
  "magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum"
  "exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi"
  "consequatur?  Quis autem vel eum iure reprehenderit qui in ea voluptate velit"
  "esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo"
  "voluptas nulla pariatur?\nAt vero eos et accusamus et iusto odio dignissimos"
  "ducimus qui blanditiis praesentium voluptatum deleniti atque corrupti quos"
  "dolores et quas molestias excepturi sint occaecati cupiditate non provident,"
  "similique sunt in culpa qui officia deserunt mollitia animi, id est laborum et"
  "dolorum fuga. Et harum quidem rerum facilis est et expedita distinctio. Nam"
  "libero tempore, cum soluta nobis est eligendi optio cumque nihil impedit quo"
  "minus id quod maxime placeat facere possimus, omnis voluptas assumenda est,"
  "omnis dolor repellendus. Temporibus autem quibusdam et aut officiis debitis aut"
  "rerum necessitatibus saepe eveniet ut et voluptates repudiandae sint et"
  "molestiae non recusandae. Itaque earum rerum hic tenetur a sapiente delectus, ut"
  "aut reiciendis voluptatibus maiores alias consequatur aut perferendis doloribus"
  "asperiores repellat.\n"
  "                           ..,';c;...','       ..,'..;:;,.'..                   \n"
  "                   ...,;,'...                    ......,:,'..                   \n"
  "                ..;c;,..                                .',:l;..                \n"
  "            ...;lc...                                      ...;c,..             \n"
  "           .cl;...                                            ...,c:.           \n"
  "         ..::.                                                    ':c,..        \n"
  "     ......'.                         .... ..'..'''........',',:,'..'.'.        \n"
  "      .........,:.               ':,;;::;;,.,:c,.........  ...';'........       \n"
  "     ...,,;,,,''::.'.            ..''.';;,....:'        ......,;,,'..::. ..     \n"
  "   .'..  'l,.',.;lc;..'.. .     .;'........   ..   .'.,;,,;looc,:,..::.  .;;.   \n"
  "   ...          ..,:'.'cl::'....lo'. .....   .;'  .;olc;'.;l:,. .         .;,.  \n"
  " .';.  ':;;'.     ....,cccll;.';;''...'. ..  .'',,;ccc:;,,,:l,    .''..'.. .,;. \n"
  " .,'  ..'',''',c:',;;,'...','.',:;:::lc,';c;,coc;;,;::;:c:,.'.,ll:,,...''.  .:;.\n"
  ".'.   .,;'''';lko;lOkdl;,,;:cl;':lcl;:doo:,oxxoc;;:lol:,,:'. .;cc:,;;. ..    ';,\n"
  ".'.    ...  'cloc..lx:.'''''''..'....;lcc;,lo;......;'. .::;;c'  ....        .,c\n"
  ",.                 .,.          .'.':cooollcc,.                               .,\n"
  ",.                              .;:okKXOkKKOOd;.                              .:\n"
  ";.                              '::oddO00KK0d;.                               ';\n"
  "                                 .,okoccck0d:'.                                 \n"
  "      ..                         .,dKNx'.ckd:.                                 .\n"
  "                                .;,o0NKxx00xl..                                .\n"
  ",                               .;,d0O0KXXOc...                        ,;      '\n"
  ":.                              .''dkc:cccl:';,.                       lo     .c\n"
  ",.                             '::,x0l;,,;ckNOcl:.                     :c     .;\n"
  "';.                          .;;dl'lkl;;,;,,;,,do,.                    :c    .::\n"
  ".;.                         .';oOo::;;lo:ox:.,cx0l,.                   ll    .:'\n"
  " ',.                        'lcoko;:lkKo.,kl.,'lOd;.                   lo   .,;.\n"
  " ':,.                       .:lccc.:0KO:..lkc'.'okd;.                  ..  .,l, \n"
  " .'':.                     .,:xx:'.'k0d' .cxxc...cdl,.                    .cc:. \n"
  "   .,'                     .':do;. .dk:. .:xo:' ..';ld;.               .. ','.  \n"
  "    .'.                  .;';dxl.  .:lc,...coc'    'lxko'.             ;:.,.    \n"
  "     'l,.               .;,lkko:.  ..;l:'..,lo;.    .:do:,.            .'.      \n"
  "     ,k;.'.             .;lkko:.    .lKkl;.;OXo.     'occc,.         .'''.      \n"
  "     ,x, .''.          .'cxdc'.    .,dK0x;.l0Xo.      .:l:,.       .....;'      \n"
  "     ,d,   .''...     .,:do;.      'dkkl:'.ckOc.       .,:;.   ...'..   .,.     \n"
  "     'd'     ..;;......ldl'        .,dx'...lOo'         .:l;. .;;..     .l'     \n"
  "     'o'         .'.';;c;.          .ok;...dd'           .;;....        .l'     \n"
  "     'l.          ,,,,',,..        .:do. .;:'        ..,''..,'          .c'     \n"
  "     'l,............;:',:::,..,. ..,coc..'c:..  .,..',;;'.  ............':.     \n"
  "     'c:cccccccccccc::ccccc:;,'......''.........','....;,. .,:cccccc::::;:.     \n";

MIRA_IODEFS(MIRA_IODEF_NONE,    /* fd 0: stdin */
            MIRA_IODEF_UART(0), /* fd 1: stdout */
            MIRA_IODEF_NONE     /* fd 2: stderr */
                                /* More file descriptors can be added, for use with dprintf(); */
);

PROCESS(main_proc, "Main process");
PROCESS(packet_ready_notify_proc, "Announce packet ready");
PROCESS(reply_to_request_proc, "React to requests");

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

    process_start(&main_proc, NULL);
}

PROCESS_THREAD(main_proc, ev, data)
{
    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup */
    PROCESS_PAUSE();

    printf("Starting Node (Large packet sender).\n");

    if (sizeof(packet_content) > MTK_BULK_DATA_COLLECTION_SUBPACKET_MAX_BYTES *
                                   MTK_BULK_DATA_COLLECTION_MAX_NUMBER_OF_SUBPACKETS) {
        P_ERR("packet too large! (%zu bytes). Aborting.\n", sizeof(packet_content));
        PROCESS_EXIT();
    }

    MIRA_RUN_CHECK(mira_net_init(&net_config));

    process_start(&packet_ready_notify_proc, NULL);
    process_start(&reply_to_request_proc, NULL);

    RUN_CHECK(mtk_bulk_data_collection_init(MTK_BULK_DATA_COLLECTION_SENDER));

    PROCESS_END();
}

/*
 * Notify the network that this node has something to send.
 */
PROCESS_THREAD(packet_ready_notify_proc, ev, data)
{
    static struct etimer timer;
    static mira_net_address_t net_address;
    static uint16_t packet_id = 0;

    static bool route_established;
#if DEBUG >= 2
    char buffer[MIRA_NET_MAX_ADDRESS_STR_LEN];
#endif
    mira_status_t res;

    PROCESS_BEGIN();
    PROCESS_PAUSE();

    while (1) {
        res = mira_net_get_root_address(&net_address);

        if (res != MIRA_SUCCESS) {
            P_DEBUG("Waiting for root address: [%d]\n", res);
            route_established = false;

            etimer_set(&timer, WAITING_FOR_ROOT_PERIOD_S * CLOCK_SECOND);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        } else {
            if (!route_established) {
                /* Give time to establish route from root and downwards */
                P_DEBUG("Establishing downward route...\n");
                etimer_set(&timer, ESTABLISHING_ROUTE_DELAY_S * CLOCK_SECOND);
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
                route_established = true;
            }
            P_DEBUG("Sending packet ready notification to %s\n",
                    mira_net_toolkit_format_address(buffer, &net_address));

            /* Sets the content of the large packet to send */
            int ret = mtk_bulk_data_collection_register_tx(&mtk_bulk_data_collection_packet_tx,
                                                           packet_id,
                                                           packet_content,
                                                           sizeof(packet_content));

            if (ret < 0) {
                P_ERR("%s: could not register packet %d\n", __func__, packet_id);
            } else {
                /* Send signal about the new packet */
                RUN_CHECK(mtk_bdcsig_send(
                  &net_address,
                  packet_id,
                  mtk_bulk_data_collection_n_sub_packets_get(sizeof(packet_content))));
            }

            /* This example will periodically with some random variation generate a new packet
             * and notify the receiver about its availability.
             */
            uint16_t variation = mira_random_generate() % PACKET_GENERATION_VARIATION_S;
            etimer_set(&timer, (PACKET_GENERATION_PERIOD_S + variation) * CLOCK_SECOND);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

            packet_id++;
        }
    }

    PROCESS_END();
}

PROCESS_THREAD(reply_to_request_proc, ev, data)
{
    static mira_net_address_t node_address;
    PROCESS_BEGIN();
    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == event_bdc_requested);
        mtk_bdc_event_requested_data_t req_data = *(mtk_bdc_event_requested_data_t*)data;

        mira_net_get_address(&node_address);

        /* This example replies to requests by immediately starting sending the
         * latest registered large packet. If in need of a smarter behavior,
         * here is the place to do it. */

        mtk_bulk_data_collection_packet_tx.node_addr = req_data.src;
        mtk_bulk_data_collection_packet_tx.node_port = req_data.src_port;
        mtk_bulk_data_collection_packet_tx.id = req_data.packet_id;
        mtk_bulk_data_collection_packet_tx.mask = req_data.mask;
        mtk_bulk_data_collection_packet_tx.period_ms = req_data.period_ms;
        printf("Node %02x%02x start sending, packet id: %u\n",
               node_address.u8[14],
               node_address.u8[15],
               req_data.packet_id);
        RUN_CHECK(mtk_bulk_data_collection_send(&mtk_bulk_data_collection_packet_tx));
    }
    PROCESS_END();
}
