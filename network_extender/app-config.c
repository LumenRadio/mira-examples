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

#include "app-config.h"

#include <mira.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "nrf52.h"

#define APP_CONFIG_EXPOSE_KEY 0
#define APP_CONFIG_VERSION 1

app_config_t app_config;

static app_config_t new_config;
static bool configuration_is_updated;

PROCESS(app_config_writer, "Config writer");

void print_config(void)
{
    mira_net_address_t addr;
    char address_buf[MIRA_NET_MAX_ADDRESS_STR_LEN];
    mira_net_get_ll_address(&addr);
    mira_net_toolkit_format_address(address_buf, &addr);
    printf("Local address: %s\n", address_buf);
    if (mira_net_is_init() && mira_net_get_state() == MIRA_NET_STATE_JOINED) {
        mira_net_get_address(&addr);
        mira_net_toolkit_format_address(address_buf, &addr);
        printf("IP address: %s\n", address_buf);
    }
    printf("PAN ID: %8lx\n", app_config.net_pan_id);
#if APP_CONFIG_EXPOSE_KEY
    printf("Net key:");
    for (uint8_t i = 0; i < sizeof(app_config.net_key); i++) {
        printf(" %02x", app_config.net_key[i]);
    }
    printf("\n");
#endif
    printf("Rate: %d\n", (int)app_config.net_rate);
    printf("Antenna: %u\n", app_config.antenna);

    print_net_state();
}

void print_net_state(void)
{
    if (mira_net_is_init()) {
        switch (mira_net_get_state()) {
            case MIRA_NET_STATE_NOT_ASSOCIATED:
                printf("status: NOT_ASSOCIATED\n");
                break;

            case MIRA_NET_STATE_ASSOCIATED:
                printf("status: ASSOCIATED\n");
                break;

            case MIRA_NET_STATE_JOINED:
                printf("status: JOINED\n");
                break;

            case MIRA_NET_STATE_IS_COORDINATOR:
                printf("status: COORDINATOR\n");
                break;
        }
    }
}

void app_config_init(void)
{
    if (MIRA_SUCCESS != mira_config_read(&app_config, sizeof(app_config_t))) {
        memset(&app_config, 0xff, sizeof(app_config_t));
    }

    configuration_is_updated = false;
    memcpy(&new_config, &app_config, sizeof(app_config_t));
    process_start(&app_config_writer, NULL);
}

int app_config_is_configured(void)
{
    return app_config.net_pan_id != 0xffffffff && app_config.net_rate != 0xff;
}

static int dehexstr(void* tgt, const char* src, int len)
{
    int i;
    int nibble;
    uint8_t* tgt_i = tgt;
    for (i = 0; i < len; i++) {

        tgt_i[i] = 0;

        for (nibble = 0; nibble < 2; nibble++) {
            char c = src[i * 2 + nibble];
            if (c >= '0' && c <= '9') {
                tgt_i[i] |= (c - '0') << (4 * (1 - nibble));
            } else if (c >= 'a' && c <= 'f') {
                tgt_i[i] |= (c - 'a' + 10) << (4 * (1 - nibble));
            } else if (c >= 'A' && c <= 'F') {
                tgt_i[i] |= (c - 'A' + 10) << (4 * (1 - nibble));
            } else {
                return -1;
            }
        }
    }
    return 0;
}
static int dehexint(uint32_t* tgt, const char* src, int len)
{
    *tgt = 0;

    uint8_t buf[4];
    memset(buf, 0, 4);
    if (dehexstr(buf + 4 - len, src, len)) {
        return -1;
    }
    *tgt |= ((uint32_t)buf[0]) << 24;
    *tgt |= ((uint32_t)buf[1]) << 16;
    *tgt |= ((uint32_t)buf[2]) << 8;
    *tgt |= ((uint32_t)buf[3]) << 0;

    return 0;
}

static void app_config_store()
{
    configuration_is_updated = true;
    process_poll(&app_config_writer);
}

void app_config_set_key(const char* payload)
{
    if (strlen(payload) != 32 || dehexstr(new_config.net_key, payload, 16)) {
        printf("Invalid net_key\n");
        return;
    }
    if (new_config.net_key == app_config.net_key) {
        return;
    }
    app_config_store();
}

void app_config_set_rate(int rate)
{
    new_config.net_rate = rate;
    if (new_config.net_rate == app_config.net_rate) {
        return;
    }
    app_config_store();
}

void app_config_set_pan_id(const char* payload)
{
    uint32_t tmp;
    if (strlen(payload) != 8 || dehexint(&tmp, payload, 4)) {
        printf("Invalid net_pan_id \n");
        return;
    }
    new_config.net_pan_id = tmp;
    if (new_config.net_pan_id == app_config.net_pan_id) {
        return;
    }
    app_config_store();
}

void app_config_set_antenna(uint8_t antenna)
{
    new_config.antenna = antenna;
    if (new_config.antenna == app_config.antenna) {
        return;
    }
    app_config_store();
}

PROCESS_THREAD(app_config_writer, ev, data)
{
    static struct etimer timer;
    mira_status_t status;
    mira_net_config_t netconf;
    PROCESS_BEGIN();
    while (1) {
        PROCESS_WAIT_UNTIL(configuration_is_updated);
        etimer_set(&timer, CLOCK_SECOND * 2);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        printf("Writing config, Pan ID: %08lx rate: %02x antenna: %u\n",
               new_config.net_pan_id,
               new_config.net_rate,
               new_config.antenna);

        new_config.version = APP_CONFIG_VERSION;
        configuration_is_updated = false;
        status = mira_config_write(&new_config, sizeof(app_config_t));
        if (status != MIRA_SUCCESS) {
            printf("Failed to store config (err=%d)\n", status);
        } else {
            PROCESS_WAIT_WHILE(mira_config_is_working());

            memcpy(&app_config, &new_config, sizeof(app_config_t));
            print_config();

            printf("Restarting network\n");

            memset(&netconf, 0, sizeof(mira_net_config_t));
            netconf.pan_id = app_config.net_pan_id;
            memcpy(netconf.key, app_config.net_key, 16);
            netconf.antenna = app_config.antenna;
            netconf.mode = MIRA_NET_MODE_MESH;
            netconf.rate = app_config.net_rate;
            if (netconf.rate > 15) {
                netconf.rate = 15;
            }
            if (mira_net_is_init()) {
                mira_net_reinit(&netconf);
            } else {
                mira_net_init(&netconf);
            }
        }
    }
    PROCESS_END();
}
