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
#include <stdbool.h>
#include "monitoring.h"

#define MONITOR_UDP_PORT 6960

static uint8_t monitor_conf_id =
  (1 << MIRA_MON_CONF_MAC_STATS) | (1 << MIRA_MON_CONF_NET_NEIGHBOURS);

static uint16_t monitor_conf_send_interval = 1;
static uint16_t monitor_conf_mac_stats = 0x7ff;
static uint16_t monitor_conf_net_neighbours = 0x7;
static uint8_t monitor_conf_version = 0;

static uint32_t read_mbi(const uint8_t* data, int* pos, int data_len)
{
    uint32_t result = 0;
    while ((*pos < data_len)) {
        result = result << 7;
        result |= data[*pos] & 0x7f;
        pos++;
        if ((data[*pos - 1] & 0x80) == 0) {
            break;
        }
    }
    return result;
}

static void handle_config(const uint8_t* data, int pos, int data_len)
{
    if (pos >= data_len)
        return;
    uint32_t conf_version = data[pos++];
    uint32_t send_interval = read_mbi(data, &pos, data_len);
    if (send_interval < 1)
        return;

    monitor_conf_version = conf_version;
    monitor_conf_send_interval = send_interval;

    monitor_conf_id = read_mbi(data, &pos, data_len);
    if ((monitor_conf_id & (1 << MIRA_MON_CONF_MAC_STATS)) != 0) {
        monitor_conf_mac_stats = read_mbi(data, &pos, data_len);
    }
    if ((monitor_conf_id & (1 << MIRA_MON_CONF_NET_NEIGHBOURS)) != 0) {
        monitor_conf_net_neighbours = read_mbi(data, &pos, data_len);
    }
}

static void udp_listen_callback(mira_net_udp_connection_t* connection,
                                const void* data,
                                uint16_t data_len,
                                const mira_net_udp_callback_metadata_t* metadata,
                                void* storage)
{
    int pos = 0;
    while (pos < data_len) {
        int id = read_mbi(data, &pos, data_len);
        int len = read_mbi(data, &pos, data_len);
        switch (id) {
            case MIRA_MON_ID_CONFIG:
                handle_config(data, pos, data_len);
                break;
        }
        pos += len;
    }
}

#define MAX_NEIGHBOURS 4
typedef struct
{
    mira_diag_net_neighbour_data_t nbr[MAX_NEIGHBOURS];
    mira_net_address_t parent;
    int n_nbrs;
} neighbour_info_t;

static bool is_parent(const neighbour_info_t* info, const mira_diag_net_neighbour_data_t* nbr)
{
    return memcmp(&info->parent, &nbr->addr, sizeof(info->parent)) == 0;
}

static bool is_better_neighbour(const neighbour_info_t* info,
                                const mira_diag_net_neighbour_data_t* old,
                                const mira_diag_net_neighbour_data_t* new)
{
    if (is_parent(info, old)) {
        return false;
    }
    if (is_parent(info, new)) {
        return true;
    }
    return old->link_met > new->link_met;
}

static void neighbour_callback(const mira_diag_net_neighbour_data_t* nbr, void* storage)
{
    neighbour_info_t* info = storage;
    int idx = info->n_nbrs;
    if (idx >= MAX_NEIGHBOURS) {
        /* Replace the one with the worse link_met that
         * isn't the parent.
         */
        idx = 0;
        for (int i = 1; i < MAX_NEIGHBOURS; ++i) {
            if (is_better_neighbour(info, &info->nbr[i], &info->nbr[idx])) {
                // The old idx should be kept, this neighbour is worse.
                idx = i;
            }
        }
        if (!is_better_neighbour(info, &info->nbr[idx], nbr)) {
            // The new neighbour is worse than the old one, so keep the old one.
            return;
        }
    } else {
        info->n_nbrs++;
    }
    memcpy(&info->nbr[idx], nbr, sizeof(info->nbr[0]));
}

static void monitor_add_vle(uint32_t val, uint8_t** data, int* len, int* max_len)
{
    if (val < 128) {
        *(*data)++ = val;
        *len += 1;
        *max_len -= 1;
    } else if (val < (1 << 14)) {
        *(*data)++ = 128 | (val >> 7);
        *(*data)++ = val & 127;
        *len += 2;
        *max_len -= 2;
    } else if (val < 1 << 21) {
        *(*data)++ = 128 | (val >> 14);
        *(*data)++ = 128 | (val >> 7);
        *(*data)++ = val & 127;
        *len += 3;
        *max_len -= 3;
    } else if (val < 1 << 28) {
        *(*data)++ = 128 | (val >> 21);
        *(*data)++ = 128 | (val >> 14);
        *(*data)++ = 128 | (val >> 7);
        *(*data)++ = val & 127;
        *len += 4;
        *max_len -= 4;
    } else {
        *(*data)++ = 128 | (val >> 28);
        *(*data)++ = 128 | (val >> 21);
        *(*data)++ = 128 | (val >> 14);
        *(*data)++ = 128 | (val >> 7);
        *(*data)++ = val & 127;
        *len += 5;
        *max_len -= 5;
    }
}

#define MON_ADD_U8(byte)     \
    do {                     \
        *(*data)++ = (byte); \
        ++(len);             \
        --(*max_len);        \
    } while (0)

#define MON_ADD_VLE(val) monitor_add_vle(val, data, &len, max_len)

#define MON_ADD_U16(val)        \
    do {                        \
        MON_ADD_U8((val));      \
        MON_ADD_U8((val) >> 8); \
    } while (0)

#define MON_ADD_U32(val)         \
    do {                         \
        MON_ADD_U8((val));       \
        MON_ADD_U8((val) >> 8);  \
        MON_ADD_U8((val) >> 16); \
        MON_ADD_U8((val) >> 24); \
    } while (0)

#define MON_ADD_MEM(m, l)    \
    do {                     \
        memcpy(*data, m, l); \
        *data += l;          \
        len += l;            \
        *max_len -= l;       \
    } while (0)

static int monitor_add_config_version(uint8_t** data, int* max_len)
{
    int len = 0;

    if (((monitor_conf_id & (1 << MIRA_MON_CONF_CONFIG_VERSION)) != 0) &&
        (*max_len >= (1 + 1 + 1))) {
        printf("Adding config version\n");
        MON_ADD_U8(MIRA_MON_ID_CONFIG_VERSION);
        MON_ADD_U8(1);
        MON_ADD_U8(monitor_conf_version);
    }

    return len;
}

static int monitor_add_mac_stats(uint8_t** data, int* max_len)
{
    int len = 0;
    mira_diag_mac_statistics_t mac_stats;
    if (((monitor_conf_id & (1 << MIRA_MON_CONF_MAC_STATS)) != 0) &&
        (*max_len >= (1 + 1 + 2 + 2 * 10 + 1)) &&
        (mira_diag_mac_get_statistics(&mac_stats) == MIRA_SUCCESS)) {

        MON_ADD_U8(MIRA_MON_ID_MAC_STATS);
        uint8_t* len_pos = *data;
        MON_ADD_U8(0); // Add a temp value for length.

        MON_ADD_VLE(monitor_conf_mac_stats);

        if (monitor_conf_mac_stats & (1 << MIRA_MON_CONF_MAC_STATS_TX_ALL_LLMC_PKTS)) {
            MON_ADD_U16(mac_stats.tx_all_nodes_llmc_packets);
        }
        if (monitor_conf_mac_stats & (1 << MIRA_MON_CONF_MAC_STATS_TX_UNICAST_PKTS)) {
            MON_ADD_U16(mac_stats.tx_unicast_packets);
        }
        if (monitor_conf_mac_stats & (1 << MIRA_MON_CONF_MAC_STATS_TX_CUST_LLMC_PKTS)) {
            MON_ADD_U16(mac_stats.tx_custom_llmc_packets);
        }

        if (monitor_conf_mac_stats & (1 << MIRA_MON_CONF_MAC_STATS_RX_ALL_LLMC_PKTS)) {
            MON_ADD_U16(mac_stats.rx_all_nodes_llmc_packets);
        }
        if (monitor_conf_mac_stats & (1 << MIRA_MON_CONF_MAC_STATS_RX_UNICAST_PKTS)) {
            MON_ADD_U16(mac_stats.rx_unicast_packets);
        }
        if (monitor_conf_mac_stats & (1 << MIRA_MON_CONF_MAC_STATS_RX_CUST_LLMC_PKTS)) {
            MON_ADD_U16(mac_stats.rx_custom_llmc_packets);
        }

        if (monitor_conf_mac_stats & (1 << MIRA_MON_CONF_MAC_STATS_RX_MISSED_SLOTS)) {
            MON_ADD_U16(mac_stats.rx_missed_slots);
        }
        if (monitor_conf_mac_stats & (1 << MIRA_MON_CONF_MAC_STATS_RX_NOT_FOR_US_PKTS)) {
            MON_ADD_U16(mac_stats.rx_not_for_us_packets);
        }

        if (monitor_conf_mac_stats & (1 << MIRA_MON_CONF_MAC_STATS_TX_DROPPED)) {
            MON_ADD_U16(mac_stats.tx_dropped);
        }
        if (monitor_conf_mac_stats & (1 << MIRA_MON_CONF_MAC_STATS_TX_FAILED)) {
            MON_ADD_U16(mac_stats.tx_failed);
        }

        if (monitor_conf_mac_stats & (1 << MIRA_MON_CONF_MAC_STATS_USED_TX_QUEUE)) {
            MON_ADD_U8(mac_stats.used_tx_queue);
        }
        *len_pos = (*data) - len_pos - 1;
    }
    return len;
}

static int monitor_add_net_neighbour_info(uint8_t** data, int* max_len)
{
    int len = 0;

    neighbour_info_t neighbour_info;
    mira_net_get_parent_address(&neighbour_info.parent);
    neighbour_info.n_nbrs = 0;
    if (((monitor_conf_id & (1 << MIRA_MON_CONF_NET_NEIGHBOURS)) != 0) &&
        (*max_len >= (1 + 1 + 1 + 8 + MAX_NEIGHBOURS * (8 + 2 + 1 + 2))) &&
        (mira_diag_net_get_neighbour_info(&neighbour_callback, &neighbour_info) == MIRA_SUCCESS)) {

        if (neighbour_info.n_nbrs > 0) {
            MON_ADD_U8(MIRA_MON_ID_NET_NEIGHBOURS);
            uint8_t* len_pos = *data;
            MON_ADD_U8(0); // Add a temp value for length.

            MON_ADD_VLE(monitor_conf_net_neighbours);
            MON_ADD_MEM(&neighbour_info.nbr[0].addr, 8);
            for (int i = 0; i < neighbour_info.n_nbrs; ++i) {
                MON_ADD_MEM(&neighbour_info.nbr[i].addr.u8[8], 8);
                if (monitor_conf_net_neighbours & (1 << MIRA_MON_CONF_NET_NEIGHBOURS_ETX)) {
                    MON_ADD_U16(neighbour_info.nbr[i].link_met);
                }
                if (monitor_conf_net_neighbours &
                    (1 << MIRA_MON_CONF_NET_NEIGHBOURS_ETX_SAMPLE_COUNT)) {
                    MON_ADD_U8(neighbour_info.nbr[i].link_met_measurements);
                }
                if (monitor_conf_net_neighbours & (1 << MIRA_MON_CONF_NET_NEIGHBOURS_RSSI)) {
                    MON_ADD_U16(neighbour_info.nbr[i].rssi);
                }
            }
            *len_pos = (*data) - len_pos - 1;
        }
    }

    return len;
}

static int monitoring_fill_buffer(uint8_t* data, int max_len)
{
    int len = 0;

    if (monitor_conf_version != 0) {
        len += monitor_add_config_version(&data, &max_len);
    }

    len += monitor_add_mac_stats(&data, &max_len);

    len += monitor_add_net_neighbour_info(&data, &max_len);

    if (max_len < 1) {
        return -1;
    } else {
        *data = 0;
        len++;
    }

    return len;
}

PROCESS(monitoring_proc, "Monitoring process");

void monitoring_init(void)
{
    process_start(&monitoring_proc, NULL);
}

PROCESS_THREAD(monitoring_proc, ev, data)
{
    static struct etimer timer;
    static mira_net_udp_connection_t* udp_connection;

    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup */
    PROCESS_PAUSE();

    /*
     * Open a connection, but don't specify target address yet, which means
     * only mira_net_udp_send_to() can be used to send packets later.
     */
    udp_connection = mira_net_udp_connect(NULL, MONITOR_UDP_PORT, udp_listen_callback, NULL);

    while (1) {
        uint64_t interval = monitor_conf_send_interval * 60 * CLOCK_SECOND;
        interval = (3 * interval) / 4 + mira_random_generate() * interval / (MIRA_RANDOM_MAX * 2);
        if (interval > UINT32_MAX) {
            interval = UINT32_MAX - 60 * CLOCK_SECOND;
            interval += mira_random_generate() * 60 * CLOCK_SECOND / MIRA_RANDOM_MAX;
        }
        etimer_set(&timer, interval);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

        mira_net_address_t net_address;
        mira_status_t res = mira_net_get_root_address(&net_address);
        if (res == MIRA_SUCCESS) {
            uint8_t buffer[150];
            int len = monitoring_fill_buffer(buffer, sizeof(buffer));

            if (len > 0) {
                printf("Sending mon info\n");
                // build message
                mira_net_udp_send_to(udp_connection, &net_address, MONITOR_UDP_PORT, buffer, len);
            }
        }
    }
    PROCESS_END();
}
