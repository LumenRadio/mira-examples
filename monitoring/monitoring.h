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

#ifndef MONITORING_H
#define MONITORING_H

void monitoring_init(void);

/* Packet format:
 * <id> <len> <len bytes data>
 *
 * id and len are multi byte integers (MBI) encoded so the highest bit
 * is a continuation bit. The rest of the bits are in big endian
 * format.
 *
 * MBI are encoded like this:
 * 0..127 = one byte: 0XXXXXXX
 * 127 .. 2^14-1: 2 bytes: 1XXXXXXX 0XXXXXXX,
 * IE 259 is encoded like this:
 * 10000010 00000011
 *
 * The data depends on the id.
 *
 * Fields with a fixed number of bytes are encoded in little endian format.
 */

/* MAC Statistics */
#define MIRA_MON_ID_MAC_STATS 2

/* Data format:
 *
 * <MBI encoded bit field saying which fields are sent>
 * <The fields according to the bit field>
 *
 * Field # (in bit field) and type:
 * 0 tx_all_nodes_llmc_packets 2 bytes
 * 1 tx_unicast_packets 2 bytes
 * 2 tx_custom_llmc_packets 2 bytes
 * 3 rx_all_nodes_llmc_packets 2 bytes
 * 4 rx_unicast_packets 2 bytes
 * 5 rx_custom_llmc_packets 2 bytes
 * 6 rx_missed_slots 2 bytes
 * 7 rx_not_for_us_packets 2 bytes
 * 8 tx_dropped 2 bytes
 * 9 tx_failed 2 bytes
 * 10 used_tx_queue 1 byte
 *
 * Example:
 * 2 (the id)
 * 9 (the data field length, 9 bytes)
 * 0b10001000 0b01000011
 * 123 1 (the tx_all_nodes_llmfc_packets field, value 1*256 + 123)
 * 1 2 (the tx_unicast_packets field, value 2*256 + 1)
 * 103 0 (the rx_missed_slots field, value 103)
 * 3 (the used_tx_queue field)
 */

/* Info about neighbours */
#define MIRA_MON_ID_NET_NEIGHBOURS 4
/* Data format:
 *
 * <MBI encoded bit field saying which fields are sent>
 * <IP-address-prefix> (Top 8 bytes of address).
 *
 * <Fields according to the bit field, repeated once per neighbour>
 *
 * Field # (in bit field) and type:
 * - address (8 bytes)
 * 0 etx (2 bytes, a decimal value multiplied by 128).
 * 1 number of ETX measurements (1 byte).
 * 2 RSSI (2 bytes, signed integer).
 *
 */

#define MIRA_MON_ID_CONFIG_VERSION 6
/* Data format:
 * <config version> 1 byte.
 *
 * This packet is not sent if the config version is zero.
 */

/************************/
/* Packets sent to node */

#define MIRA_MON_ID_CONFIG 1
/* Data format:
 *
 * <config version> (1 byte, a number used to ack the config changes.)
 * <MBI encoded message interval> (how often, in minutes, statistics are sent).
 * <MBI encoded bit field saying which IDs are to be sent>
 * For each ID to be sent:
 * <MBI encoded bit field saying which fields are to be sent>
 */

#define MIRA_MON_CONF_MAC_STATS 0
/* Bit per field: */
#define MIRA_MON_CONF_MAC_STATS_TX_ALL_LLMC_PKTS 0
#define MIRA_MON_CONF_MAC_STATS_TX_UNICAST_PKTS 1
#define MIRA_MON_CONF_MAC_STATS_TX_CUST_LLMC_PKTS 2
#define MIRA_MON_CONF_MAC_STATS_RX_ALL_LLMC_PKTS 3
#define MIRA_MON_CONF_MAC_STATS_RX_UNICAST_PKTS 4
#define MIRA_MON_CONF_MAC_STATS_RX_CUST_LLMC_PKTS 5
#define MIRA_MON_CONF_MAC_STATS_RX_MISSED_SLOTS 6
#define MIRA_MON_CONF_MAC_STATS_RX_NOT_FOR_US_PKTS 7
#define MIRA_MON_CONF_MAC_STATS_TX_DROPPED 8
#define MIRA_MON_CONF_MAC_STATS_TX_FAILED 9
#define MIRA_MON_CONF_MAC_STATS_USED_TX_QUEUE 10

#define MIRA_MON_CONF_NET_NEIGHBOURS 1
/* Bit per field: */
#define MIRA_MON_CONF_NET_NEIGHBOURS_ETX 0
#define MIRA_MON_CONF_NET_NEIGHBOURS_ETX_SAMPLE_COUNT 1
#define MIRA_MON_CONF_NET_NEIGHBOURS_RSSI 2

#define MIRA_MON_CONF_CONFIG_VERSION 2
/* No optional fields */

#endif
