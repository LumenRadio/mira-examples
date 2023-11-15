/**
 * Copyright (c) 2010 - 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.

 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <ble.h>
#include <ble_advdata.h>
#include <mira.h>
#include <nrf_ble_gatt.h>
#include <nrf_error.h>
#include <stdio.h>
#include <string.h>

#include "ble_led.h"
#include "sdk_config.h"
#include "ble_app.h"

#define CONNECTED_LED 1
#define LED_ON_LED 2

// *****************************************************************************
// Module constants
// *****************************************************************************
#define APP_BLE_CONN_CFG_TAG 1
#define APP_BLE_OBSERVER_PRIO 3
#define NRF_SDH_BLE_EVT_BUF_SIZE BLE_EVT_LEN_MAX(NRF_SDH_BLE_GATT_MAX_MTU_SIZE)
#define APP_ADV_INTERVAL MSEC_TO_UNITS(1000, UNIT_0_625_MS)
// These values make sd_ble_gap_ppcp_set() give an error, but work.
#define MIN_CONN_INTERVAL_TICKS MSEC_TO_UNITS(400, UNIT_1_25_MS)
#define MAX_CONN_INTERVAL_TICKS MSEC_TO_UNITS(600, UNIT_1_25_MS)
#define SLAVE_LATENCY 6
#define CONN_SUP_TIMEOUT_TICKS MSEC_TO_UNITS(4000, UNIT_10_MS)
#define DEVICE_NAME "Mira BLE"

#define LBS_UUID_BASE                                                                             \
    {                                                                                             \
        0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, \
          0x00                                                                                    \
    }
#define LBS_UUID_SERVICE 0x1523
#define LBS_UUID_BUTTON_CHAR 0x1524
#define LBS_UUID_LED_CHAR 0x1525

// *****************************************************************************
// Types
// *****************************************************************************
typedef struct
{
    uint16_t service_handle; /**< Handle of LED Button Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t led_char_handles; /**< Handles related to the LED Characteristic. */
    ble_gatts_char_handles_t
      button_char_handles; /**< Handles related to the Button Characteristic. */
    uint8_t uuid_type;     /**< UUID type for the LED Button Service. */
} ble_led_button_service_t;

// *****************************************************************************
// Module variables
// *****************************************************************************
static nrf_ble_gatt_t ble_gatt;
static union
{
    uint8_t u8[NRF_SDH_BLE_EVT_BUF_SIZE];
    uint32_t dummy_for_alignment;
} ble_event_buffer;
static ble_led_button_service_t ble_lbs;
static uint16_t ble_connection_handle = BLE_CONN_HANDLE_INVALID;

static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
static uint8_t m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static uint8_t m_enc_scan_response_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX];

/* Struct that contains pointers to the encoded advertising data. */
static ble_gap_adv_data_t m_adv_data = {
    .adv_data = { .p_data = m_enc_advdata, .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX },
    .scan_rsp_data = { .p_data = m_enc_scan_response_data, .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX }
};

// *****************************************************************************
// Function prototypes
// *****************************************************************************
static uint32_t button_characteristic_add(ble_led_button_service_t* p_lbs);
static uint32_t led_char_add(ble_led_button_service_t* p_lbs);
static void ble_evt_handler(void);
static mira_status_t ble_default_cfg_set(uint8_t conn_cfg_tag, const uint32_t* p_ram_start);

// *****************************************************************************
// Function definitions
// *****************************************************************************
void services_init(void)
{
    uint32_t ret;

    ble_uuid128_t base_uuid = { LBS_UUID_BASE };

    ret = sd_ble_uuid_vs_add(&base_uuid, &ble_lbs.uuid_type);
    if (ret != NRF_SUCCESS) {
        printf("ERROR[%lu]: sd_ble_uuid_vs_add()\n", ret);
    }

    ble_uuid_t ble_uuid;
    ble_uuid.type = ble_lbs.uuid_type;
    ble_uuid.uuid = LBS_UUID_SERVICE;

    ret = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &ble_lbs.service_handle);
    if (ret != NRF_SUCCESS) {
        printf("ERROR[%lu]: sd_ble_gatts_service_add()\n", ret);
    }

    // Add characteristics.
    ret = button_characteristic_add(&ble_lbs);
    if (ret != NRF_SUCCESS) {
        printf("ERROR[%lu]: button_characteristic_add()\n", ret);
    }

    ret = led_char_add(&ble_lbs);
    if (ret != NRF_SUCCESS) {
        printf("ERROR[%lu]: led_char_add()\n", ret);
    }
}

void gap_params_init(void)
{
    uint32_t ret;
    ble_gap_conn_params_t gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    ret = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t*)DEVICE_NAME, strlen(DEVICE_NAME));
    if (ret != NRF_SUCCESS) {
        printf("ERROR[%lu]: sd_ble_gap_device_name_set()\n", ret);
    }

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL_TICKS;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL_TICKS;
    gap_conn_params.slave_latency = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT_TICKS;

    ret = sd_ble_gap_ppcp_set(&gap_conn_params);
    if (ret != NRF_SUCCESS) {
        printf("ERROR[%lu]: sd_ble_gap_ppcp_set()\n", ret);
    }
}

void gatt_init(void)
{
    uint32_t ret = nrf_ble_gatt_init(&ble_gatt, NULL);
    if (ret != NRF_SUCCESS) {
        printf("ERROR[%lu]: nrf_ble_gatt_init()", ret);
    }
}

void advertising_init(void)
{
    uint32_t ret;
    ble_advdata_t advdata;
    ble_advdata_t srdata;

    ble_uuid_t adv_uuids[] = { { LBS_UUID_SERVICE, ble_lbs.uuid_type } };

    // Build and set advertising data
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = true;
    advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    memset(&srdata, 0, sizeof(srdata));
    srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    srdata.uuids_complete.p_uuids = adv_uuids;

    ret = ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len);
    if (ret != NRF_SUCCESS) {
        printf("ERROR[%lu]: ble_advdata_encode()\n", ret);
    }
    ret =
      ble_advdata_encode(&srdata, m_adv_data.scan_rsp_data.p_data, &m_adv_data.scan_rsp_data.len);
    if (ret != NRF_SUCCESS) {
        printf("ERROR[%lu]: ble_advdata_encode()\n", ret);
    }

    ble_gap_adv_params_t adv_params;

    // Set advertising parameters.
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.primary_phy = BLE_GAP_PHY_1MBPS;
    adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    adv_params.p_peer_addr = NULL;
    adv_params.filter_policy = BLE_GAP_ADV_FP_ANY;
    adv_params.interval = APP_ADV_INTERVAL;

    ret = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &adv_params);
    if (ret != NRF_SUCCESS) {
        printf("ERROR[%lu]: sd_ble_gap_adv_set_configure()\n", ret);
    }
}

void advertising_start(void)
{
    uint32_t ret;
    ret = sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
    if (ret != NRF_SUCCESS) {
        printf("ERROR[%lu]: sd_ble_gap_adv_start()\n", ret);
    }
}

void ble_stack_init(void)
{
    uint32_t ret;

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = (uint32_t)&__data_start__;
    ret = ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    if (ret != NRF_SUCCESS) {
        printf("ERROR[%lu]: ble_default_cfg_set\n", ret);
    }

    // Enable BLE stack.
    uint32_t configured_ram_start = ram_start;
    ret = sd_ble_enable(&ram_start);
    if (ram_start != configured_ram_start) {
        printf("RAM section configured to start at 0x%lx, can be moved to 0x%lx\n",
               configured_ram_start,
               ram_start);
    }
    if (ret != NRF_SUCCESS) {
        printf("ERROR[%lu]: sd_ble_enable\n", ret);
    }

    if (mira_nrf_ble_event_handler_register(ble_evt_handler) != MIRA_SUCCESS) {
        printf("ERROR: mira_nrf_ble_event_handler_register()\n");
    }
}

uint32_t notify(mira_bool_t value)
{
    ble_gatts_hvx_params_t params;
    uint16_t len = sizeof(value);

    memset(&params, 0, sizeof(params));
    params.type = BLE_GATT_HVX_NOTIFICATION;
    params.handle = ble_lbs.button_char_handles.value_handle;
    params.p_data = &value;
    params.p_len = &len;

    return sd_ble_gatts_hvx(ble_connection_handle, &params);
}

// *****************************************************************************
// Internal functions
// *****************************************************************************
static void on_write(ble_led_button_service_t* arg_ble_lbs, ble_evt_t const* ble_evt)
{
    ble_gatts_evt_write_t const* evt_write = &ble_evt->evt.gatts_evt.params.write;

    if ((evt_write->handle == arg_ble_lbs->led_char_handles.value_handle) &&
        (evt_write->len == sizeof(mira_bool_t))) {
        ble_led_set(LED_ON_LED, (mira_bool_t)evt_write->data[0]);
    }
}

/* This handler runs on interrupt */
static void ble_evt_handler(void)
{
    while (1) {
        uint16_t len = sizeof(ble_event_buffer.u8);
        if (sd_ble_evt_get(ble_event_buffer.u8, &len) != NRF_SUCCESS) {
            return;
        }
        const ble_evt_t* ble_evt = (ble_evt_t*)ble_event_buffer.u8;

        switch (ble_evt->header.evt_id) {
            case BLE_GAP_EVT_CONNECTED:
                ble_led_set(CONNECTED_LED, MIRA_TRUE);
                ble_connection_handle = ble_evt->evt.gap_evt.conn_handle;
                break;

            case BLE_GAP_EVT_DISCONNECTED:
                ble_led_set(CONNECTED_LED, MIRA_FALSE);
                ble_connection_handle = BLE_CONN_HANDLE_INVALID;
                advertising_start();
                break;

            case BLE_GATTS_EVT_WRITE:
                on_write(&ble_lbs, ble_evt);
                break;

            case BLE_GAP_EVT_PHY_UPDATE_REQUEST:;
                const ble_gap_phys_t phys = { .rx_phys = BLE_GAP_PHY_1MBPS,
                                              .tx_phys = BLE_GAP_PHY_1MBPS };
                sd_ble_gap_phy_update(ble_evt->evt.gap_evt.conn_handle, &phys);
                break;

            default:
                break;
        }
        nrf_ble_gatt_on_ble_evt(ble_evt, &ble_gatt);
    }
}

static uint32_t button_characteristic_add(ble_led_button_service_t* p_lbs)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t attr_char_value;
    ble_uuid_t ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read = 1;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf = NULL;
    char_md.p_user_desc_md = NULL;
    char_md.p_cccd_md = &cccd_md;
    char_md.p_sccd_md = NULL;

    ble_uuid.type = p_lbs->uuid_type;
    ble_uuid.uuid = LBS_UUID_BUTTON_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len = sizeof(uint8_t);
    attr_char_value.p_value = NULL;

    return sd_ble_gatts_characteristic_add(
      p_lbs->service_handle, &char_md, &attr_char_value, &p_lbs->button_char_handles);
}

static uint32_t led_char_add(ble_led_button_service_t* p_lbs)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t attr_char_value;
    ble_uuid_t ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read = 1;
    char_md.char_props.write = 1;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf = NULL;
    char_md.p_user_desc_md = NULL;
    char_md.p_cccd_md = NULL;
    char_md.p_sccd_md = NULL;

    ble_uuid.type = p_lbs->uuid_type;
    ble_uuid.uuid = LBS_UUID_LED_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len = sizeof(uint8_t);
    attr_char_value.p_value = NULL;

    return sd_ble_gatts_characteristic_add(
      p_lbs->service_handle, &char_md, &attr_char_value, &p_lbs->led_char_handles);
}

static mira_status_t ble_default_cfg_set(uint8_t conn_cfg_tag, const uint32_t* p_ram_start)
{
    uint32_t ret_code;

    // Overwrite some of the default settings of the BLE stack.
    // If any of the calls to sd_ble_cfg_set() fail, log the error but carry on so that
    // wrong RAM settings can be caught by nrf_sdh_ble_enable() and a meaningful error
    // message will be printed to the user suggesting the correct value.
    ble_cfg_t ble_cfg;

#if (NRF_SDH_BLE_TOTAL_LINK_COUNT != 0)
    // Configure the connection count.
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.conn_cfg.conn_cfg_tag = conn_cfg_tag;
    ble_cfg.conn_cfg.params.gap_conn_cfg.conn_count = NRF_SDH_BLE_TOTAL_LINK_COUNT;
    ble_cfg.conn_cfg.params.gap_conn_cfg.event_length = NRF_SDH_BLE_GAP_EVENT_LENGTH;

    ret_code = sd_ble_cfg_set(BLE_CONN_CFG_GAP, &ble_cfg, *p_ram_start);
    if (ret_code != NRF_SUCCESS) {
        printf("ERROR[%lu]: sd_ble_cfg_set() when attempting to set BLE_CONN_CFG_GAP\n", ret_code);
    }

    // Configure the connection roles.
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.gap_cfg.role_count_cfg.periph_role_count = NRF_SDH_BLE_PERIPHERAL_LINK_COUNT;
#ifndef S112
    ble_cfg.gap_cfg.role_count_cfg.central_role_count = NRF_SDH_BLE_CENTRAL_LINK_COUNT;
    ble_cfg.gap_cfg.role_count_cfg.central_sec_count =
      NRF_SDH_BLE_CENTRAL_LINK_COUNT ? BLE_GAP_ROLE_COUNT_CENTRAL_SEC_DEFAULT : 0;
#endif

    ret_code = sd_ble_cfg_set(BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, *p_ram_start);
    if (ret_code != NRF_SUCCESS) {
        printf("ERROR[%lu]: sd_ble_cfg_set() when attempting to set BLE_GAP_CFG_ROLE_COUNT\n",
               ret_code);
    }

    // Configure the maximum ATT MTU.
#if (NRF_SDH_BLE_GATT_MAX_MTU_SIZE != 23)
    memset(&ble_cfg, 0x00, sizeof(ble_cfg));
    ble_cfg.conn_cfg.conn_cfg_tag = conn_cfg_tag;
    ble_cfg.conn_cfg.params.gatt_conn_cfg.att_mtu = NRF_SDH_BLE_GATT_MAX_MTU_SIZE;

    ret_code = sd_ble_cfg_set(BLE_CONN_CFG_GATT, &ble_cfg, *p_ram_start);
    if (ret_code != NRF_SUCCESS) {
        printf("ERROR[%lu]: sd_ble_cfg_set() when attempting to set BLE_CONN_CFG_GATT\n", ret_code);
    }
#endif
#endif // NRF_SDH_BLE_TOTAL_LINK_COUNT != 0

    // Configure number of custom UUIDS.
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.common_cfg.vs_uuid_cfg.vs_uuid_count = NRF_SDH_BLE_VS_UUID_COUNT;

    ret_code = sd_ble_cfg_set(BLE_COMMON_CFG_VS_UUID, &ble_cfg, *p_ram_start);
    if (ret_code != NRF_SUCCESS) {
        printf("ERROR[%lu]: sd_ble_cfg_set() when attempting to set BLE_COMMON_CFG_VS_UUID\n",
               ret_code);
    }

    // Configure the GATTS attribute table.
    memset(&ble_cfg, 0x00, sizeof(ble_cfg));
    ble_cfg.gatts_cfg.attr_tab_size.attr_tab_size = NRF_SDH_BLE_GATTS_ATTR_TAB_SIZE;

    ret_code = sd_ble_cfg_set(BLE_GATTS_CFG_ATTR_TAB_SIZE, &ble_cfg, *p_ram_start);
    if (ret_code != NRF_SUCCESS) {
        printf("ERROR[%lu]: sd_ble_cfg_set() when attempting to set BLE_GATTS_CFG_ATTR_TAB_SIZE\n",
               ret_code);
    }

    // Configure Service Changed characteristic.
    memset(&ble_cfg, 0x00, sizeof(ble_cfg));
    ble_cfg.gatts_cfg.service_changed.service_changed = NRF_SDH_BLE_SERVICE_CHANGED;

    ret_code = sd_ble_cfg_set(BLE_GATTS_CFG_SERVICE_CHANGED, &ble_cfg, *p_ram_start);
    if (ret_code != NRF_SUCCESS) {
        printf(
          "ERROR[%lu]: sd_ble_cfg_set() when attempting to set BLE_GATTS_CFG_SERVICE_CHANGED\n",
          ret_code);
    }

    return MIRA_SUCCESS;
}
