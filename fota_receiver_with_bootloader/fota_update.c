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

#include "fota_update.h"
#include "crc.h"

#include "nrf_dfu_types.h"
#include "nrf_sdh_soc.h"

#include <mira.h>

#define P_DEBUG_FT(...) printf(__VA_ARGS__)
#define P_INFO_FT(...) printf(__VA_ARGS__)

// Define taken from nrf_dfu_settings.c
#define DFU_SETTINGS_INIT_COMMAND_OFFSET offsetof(nrf_dfu_settings_t, init_command)

// Taken from swap.h, to be removed with updated mira fota API
typedef struct
{
    uint32_t size;     /*< Size of the image. may be less than the swap storage size */
    uint32_t checksum; /*< CRC-32 of the content, up until size. */
    uint16_t type;     /*< Id identifying the content, not validated by checksum */
    uint8_t flags;     /*< Field for flags */
    uint8_t version;   /*< Image version number. Note: not related to software version number  */
} swap_header_t;

// Addresses taken from link script
extern uint8_t __ApplicationStart;
extern uint8_t __ApplicationEnd;
extern uint8_t __SwapStart;
extern nrf_dfu_settings_t __BlSettingsStart;

static const nrf_dfu_settings_t* flash_settings = NULL;
static const swap_header_t* fota_header = NULL;

static struct
{
    uint32_t crc;
    bool reset;

    bool ready;
} fota_upgrade;

PROCESS(fota_upgrade_process, "FOTA upgrade process");
PROCESS(fota_valid_process, "FOTA valid image process");

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

static uint32_t calc_settings_crc(const nrf_dfu_settings_t* settings)
{
    crc_t crc_calc;

    if (settings == NULL) {
        return 0;
    }

    // See the nrf_dfu_settings_t struct, the CRC is calculated for the data
    // between the CRC and init command.
    crc_init(&crc_calc);
    crc_update(&crc_calc,
               (uint8_t*)((uint32_t)settings + sizeof(settings->crc)),
               DFU_SETTINGS_INIT_COMMAND_OFFSET - sizeof(settings->crc));

    return crc_get(&crc_calc);
}

PROCESS_THREAD(fota_upgrade_process, ev, data)
{
    static nrf_dfu_settings_t new_settings;

    PROCESS_BEGIN();

    while (1) {
        PROCESS_WAIT_UNTIL(fota_upgrade.ready);
        fota_upgrade.ready = false;

        if (flash_settings->bank_0.image_crc == fota_upgrade.crc) {
            P_DEBUG_FT("Firmware already installed\n");
            if (fota_upgrade.reset) {
                P_DEBUG_FT("Restaring node due to root restart\n");
                mira_sys_reset();
            }
            continue;
        }

        if (fota_get_fota_crc() != fota_upgrade.crc && fota_get_fota_crc() != FOTA_INVALID_CRC) {
            P_DEBUG_FT("FOTA buffer is not ready on this node, abort upgrade\n");
            if (fota_upgrade.reset) {
                P_DEBUG_FT("Restaring node due to root restart\n");
                mira_sys_reset();
            }
            continue;
        }

        // Lock the FOTA memory by starting a read session
        if (mira_fota_read_start(FOTA_FW_SLOT_ID) != MIRA_SUCCESS) {
            P_INFO_FT("Failed to lock FOTA buffer\n");
            if (fota_upgrade.reset) {
                P_DEBUG_FT("Restaring node due to root restart\n");
                mira_sys_reset();
            }
            continue;
        }
        PROCESS_WAIT_WHILE(mira_fota_is_working());

        new_settings = *flash_settings;
        new_settings.bank_1.image_crc = fota_upgrade.crc;
        new_settings.bank_1.image_size = mira_fota_get_image_size(FOTA_FW_SLOT_ID);
        new_settings.bank_1.bank_code = NRF_DFU_BANK_VALID_APP;
        new_settings.progress.update_start_address = (uint32_t)&__SwapStart + MIRA_FOTA_HEADER_SIZE;
        new_settings.boot_validation_app.type = NO_VALIDATION;
        new_settings.crc = calc_settings_crc(&new_settings);

        static bool failed;
        do {
            failed = true;
            mira_status_t result = mira_flash_erase_page((uint32_t)&__BlSettingsStart);
            if (result == MIRA_ERROR_RESOURCE_NOT_AVAILABLE) {
                P_DEBUG_FT("flash_erase: settings page not available\n");
                PROCESS_YIELD();
                continue;
            }
            if (result != MIRA_SUCCESS) {
                printf("ERROR: Failed to erase bootloader settings page (result=%d)!\n", result);
                break; // leave the do loop, this will never succeed.
            }
            PROCESS_YIELD_UNTIL(mira_flash_is_working() == MIRA_FALSE);

            failed = false;
            // Retry if operation failed:
        } while (mira_flash_succeeded() == MIRA_FALSE);

        if (!failed) {
            P_DEBUG_FT("Bootloader settings page erased\n");

            do {
                failed = true;
                mira_status_t result = mira_flash_write(
                  (uint32_t)&__BlSettingsStart, &new_settings, sizeof(__BlSettingsStart));
                if (result == MIRA_ERROR_RESOURCE_NOT_AVAILABLE) {
                    P_DEBUG_FT("flash_write: settings page not available\n");
                    // Let other processes run until trying again:
                    PROCESS_YIELD();
                    continue;
                }
                if (result != MIRA_SUCCESS) {
                    printf("ERROR: Failed to write bootloader settings page (result=%d)!\n",
                           result);
                    break; // leave the do loop, this will never succeed.
                }
                P_DEBUG_FT("Waiting for settings page to be written\n");
                failed = false;
                PROCESS_YIELD_UNTIL(mira_flash_is_working() == MIRA_FALSE);
            } while (mira_flash_succeeded() == MIRA_FALSE);

            if (!failed) {
                P_DEBUG_FT("New bootloader settings page written\n");

                mira_sys_reset();
            }
        }
    }

    PROCESS_END();
}

int fota_init()
{
    uint32_t application_max_size;

    flash_settings = (nrf_dfu_settings_t*)(&__BlSettingsStart);
    fota_header = (swap_header_t*)(&__SwapStart);

    // Ensure that the data in the settings page is correct
    application_max_size = (uint32_t)&__ApplicationEnd - (uint32_t)&__ApplicationStart;
    if (flash_settings->bank_0.image_size > application_max_size) {
        P_INFO_FT("Bootloader settings on flash is corrupt\n");
        return -1;
    }

    // Ensure that what's in the bootloader settings page matches the calculated CRC
    if (flash_settings->crc != calc_settings_crc(flash_settings)) {
        P_INFO_FT("Bootloader settings CRC does not match\n");
        return -1;
    }

    if (mira_fota_init() != MIRA_SUCCESS) {
        P_INFO_FT("Failed to initialize Mira FOTA\n");
        return -1;
    }
    fota_upgrade.ready = false;
    process_start(&fota_valid_process, NULL);
    process_start(&fota_upgrade_process, NULL);
    return 0;
}

uint32_t fota_get_app_crc(void)
{
    return flash_settings->bank_0.image_crc;
}

uint32_t fota_get_app_size(void)
{
    return flash_settings->bank_0.image_size;
}

uint32_t fota_get_fota_crc(void)
{
    if (mira_fota_is_valid(FOTA_FW_SLOT_ID) == MIRA_TRUE) {
        return fota_header->checksum;
    } else {
        return FOTA_INVALID_CRC;
    }
}

void fota_perform_upgrade(uint32_t new_crc, bool force_reset)
{
    fota_upgrade.crc = new_crc;
    fota_upgrade.reset = force_reset;
    fota_upgrade.ready = false;

    fota_upgrade.ready = true;
    process_poll(&fota_upgrade_process);
}

PROCESS_THREAD(fota_valid_process, ev, data)
{
    static struct etimer timer;

    PROCESS_BEGIN();
    PROCESS_PAUSE();

    while (1) {
        etimer_set(&timer, 10 * CLOCK_SECOND);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

        if (mira_fota_is_valid(0)) {
            printf("%s, Valid image: %ld bytes, version %d\n",
                   net_state(),
                   mira_fota_get_image_size(0),
                   mira_fota_get_version(0));
            if ((mira_fota_get_image_size(0) > 0) && (fota_get_app_crc() != fota_get_fota_crc())) {
                printf("Perform update!\n");
                fota_perform_upgrade(fota_get_fota_crc(), false);
            } else {
                printf("Don't do update");
                if (fota_get_app_crc() == fota_get_fota_crc()) {
                    printf(", application and fota has same CRC");
                }
                printf("\n");
            }
        } else {
            printf("%s, No valid image available in cache\n", net_state());
        }
    }
    PROCESS_END();
}
