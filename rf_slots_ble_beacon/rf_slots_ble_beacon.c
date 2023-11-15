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

/* Low level radio interface needs to be added explicitly */
#include <mira_radio_timeslot.h>

#include <stdint.h>
#include <string.h>
#include <stdio.h>

/**
 * Static Eddystone-URL payload
 *
 * includes the packet header used for low level radio
 */
static uint8_t ble_beacon_payload[] = {
    /* Packet header */
    0x02, /* S0 field, static address, advertisement noconn ind */

    /* Eddystone-URL packet start */
    0x1f,
    0x1f,
    0x1f,
    0x1f,
    0x1f,
    0x1f, /* Source address */
    0x03,
    0x03,
    0xaa,
    0xfe, /* Eddystone ID */
    20,   /* Length of rest of packet */
    0x16,
    0xaa,
    0xfe, /* Eddystone ID */
    0x10, /* Eddystone URL */
    0xfe, /* TX power */
    0x03, /* https:// */
    'l',
    'u',
    'm',
    'e',
    'n',
    'r',
    'a',
    'd',
    'i',
    'o',
    '.',
    'c',
    'o',
    'm'
};

MIRA_IODEFS(MIRA_IODEF_NONE,    /* fd 0: stdin */
            MIRA_IODEF_UART(0), /* fd 1: stdout */
            MIRA_IODEF_NONE     /* fd 2: stderr */
                                /* More file descriptors can be added, for use with dprintf(); */
);

PROCESS(beacon_proc, "Beacon");

/**
 * Storage for active slot
 *
 * must not be re-used when active
 */
rf_slots_slot_t beacon_rf_slot;
volatile bool beacon_rf_slot_is_active = false;
volatile uint32_t beacon_rf_slots_num_misses = 0;

/**
 * rf-slots process handler
 */
RF_SLOTS_PROCESS(beacon_rf_slot_proc, storage);

/**
 * Initialize the ble beacon source address
 *
 * Use the device id as the beacon source address
 */
static void init_beacon_address(void)
{
    mira_sys_device_id_t device_id;
    int i;
    mira_sys_get_device_id(&device_id);
    for (i = 0; i < 6; i++) {
        /* Fill the source address part of the beacon */
        ble_beacon_payload[i + 1] = device_id.u8[7 - i];
    }
}

/**
 * Mira setup function
 */
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

    MIRA_MEM_SET_BUFFER(7592);

    uart_ret = mira_uart_init(0, &uart_config);
    if (uart_ret != MIRA_SUCCESS) {
        /* Nowhere to send an error message */
    }

    process_start(&beacon_proc, NULL);
}

/**
 * Process for scheduling slots
 */
PROCESS_THREAD(beacon_proc, ev, data)
{
    static struct etimer beacon_timer;

    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup */
    PROCESS_PAUSE();

    /* Update beacon source address to match device id */
    init_beacon_address();

    while (1) {
        /* Don't reuse an active slot */
        if (!beacon_rf_slot_is_active) {
            beacon_rf_slot_is_active = true;
            mira_radio_timeslot_schedule_immediatly(
              &beacon_rf_slot,
              RF_SLOTS_PRIO_BLE_BEACONS,
              100000,              /* Allow for slot to start within 100ms before timout */
              8000,                /* Maximum slot execution time. Increase for longer slots. */
              beacon_rf_slot_proc, /* RF-slots proccess */
              NULL /* Storage for the rf-slots handler user data */);
            printf("ble beacon scheduled (misses = %lu)\n", beacon_rf_slots_num_misses);
        } else {
            printf("ble beacon skipped, busy slot (misses = %lu)\n", beacon_rf_slots_num_misses);
        }

        /* This example sends 5 beacon per second */
        etimer_set(&beacon_timer, CLOCK_SECOND / 5);
        PROCESS_YIELD_UNTIL(etimer_expired(&beacon_timer));
    }

    PROCESS_END();
}

/*
 * Note that this part is executed from high interrupt priority.
 *
 * No ordinary mira api functions is allowed, except for process_poll()
 */
RF_SLOTS_PROCESS(beacon_rf_slot_proc, storage)
{
    static rf_slots_radio_config_t radioconf;
    static miracore_timer_time_t start_time;

    RF_SLOTS_PROCESS_BEGIN();

    /* Setup radio configuration */
    radioconf = (rf_slots_radio_config_t){
        .mode = RF_SLOTS_RADIO_MODE_BLE_1MBPS,
        .fmt = { .ble_1mbps = { .access_address = 0x8E89BED6,
                                .frequency = 0, /* EB channel, updated later */
                                .white_iv = 0,  /* Data whitening IV, updated later */
                                .preamble_bits = 8,
                                .length_bits = 8,
                                .s0_bits = 8,
                                .s1_bits = 0 } },
        .rf = { .low_power_mode = true, /* Always low power for BLE beacons, for regulations */
                .radio_power = 100,
                .antenna = 0 }
    };

    start_time = rf_slots_get_time_now();

    *rf_slots_get_packet_header_ptr() = ble_beacon_payload[0];
    memcpy(rf_slots_get_packet_ptr(), ble_beacon_payload + 1, sizeof(ble_beacon_payload) - 1);
    rf_slots_set_packet_length(sizeof(ble_beacon_payload) - 1);
    radioconf.fmt.ble_1mbps.frequency = 2; /* Frequency of channel */
    radioconf.fmt.ble_1mbps.white_iv = 37; /* whitening iv, same as ble channel number */
    RF_SLOTS_PROCESS_TX(start_time + 2000, &radioconf);

    *rf_slots_get_packet_header_ptr() = ble_beacon_payload[0];
    memcpy(rf_slots_get_packet_ptr(), ble_beacon_payload + 1, sizeof(ble_beacon_payload) - 1);
    rf_slots_set_packet_length(sizeof(ble_beacon_payload) - 1);
    radioconf.fmt.ble_1mbps.frequency = 80; /* Frequency of channel */
    radioconf.fmt.ble_1mbps.white_iv = 39;  /* whitening iv, same as ble channel number */
    RF_SLOTS_PROCESS_TX(start_time + 4000, &radioconf);

    *rf_slots_get_packet_header_ptr() = ble_beacon_payload[0];
    memcpy(rf_slots_get_packet_ptr(), ble_beacon_payload + 1, sizeof(ble_beacon_payload) - 1);
    rf_slots_set_packet_length(sizeof(ble_beacon_payload) - 1);
    radioconf.fmt.ble_1mbps.frequency = 26; /* Frequency of channel */
    radioconf.fmt.ble_1mbps.white_iv = 38;  /* whitening iv, same as ble channel number */
    RF_SLOTS_PROCESS_TX(start_time + 6000, &radioconf);

    RF_SLOTS_PROCESS_BEGIN_CLEANUP();

    if (RF_SLOTS_PROCESS_STATUS() != RF_SLOTS_STATUS_SUCCESS) {
        /* If an error occurs anywhere in the slot, catch the error */
        /* All slots is guranteed to execute, either via an error status, or
         * with success. If error, mark the slot as unused
         */
        beacon_rf_slots_num_misses++;
    }

    /* The slot is finished, mark it as not used */
    beacon_rf_slot_is_active = false;
    RF_SLOTS_PROCESS_END();
}
