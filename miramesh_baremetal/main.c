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

#include <nrf.h>
#include <miramesh.h>
#include <miramesh_sys.h>
#include <stdlib.h>

#include "boards.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"

#define UDP_PORT 456

extern uint8_t __CertificateStart[];
extern uint8_t __CertificateEnd[];

static const miramesh_config_t miramesh_config = { .callback = { .api_lock = NULL,
                                                                 .api_unlock = NULL,
                                                                 .wakeup_from_app_callback = NULL,
                                                                 .wakeup_from_irq_callback = NULL },
                                                   .hardware = { .ppi_idx = { 6, 7 },
                                                                 .ppi_group_idx = { 1 },
                                                                 .rtc = 2,
                                                                 .rtc_irq_prio = 6,
                                                                 .swi = 0,
                                                                 .swi_irq_prio = 5 },
                                                   .certificate = { .start = __CertificateStart,
                                                                    .end = __CertificateEnd } };

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
    .mode = MIRA_NET_MODE_ROOT_NO_RECONNECT,
    .rate = MIRA_NET_RATE_FAST,
    .antenna = 0,
    .prefix = NULL /* default prefix */
};

void RTC2_IRQHandler(void)
{
    miramesh_rtc_irq_handler();
}

void SWI0_EGU0_IRQHandler(void)
{
    miramesh_swi_irq_handler();
}

void SWI1_EGU1_IRQHandler(void)
{
    miramesh_swi1_irq_handler();
}

static void softdevice_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    /* Softdevice fault */
}

static void softdevice_start(void)
{
    uint32_t ret_code;
    nrf_clock_lf_cfg_t const clock_lf_cfg = { .source = NRF_CLOCK_LF_SRC_XTAL,
                                              .rc_ctiv = 0,
                                              .rc_temp_ctiv = 0,
                                              .accuracy = NRF_CLOCK_LF_ACCURACY_50_PPM };

    ret_code = sd_softdevice_enable(&clock_lf_cfg, softdevice_fault_handler);
    (void)ret_code;
}

static void timer_callback(miracore_timer_time_t now, void* storage)
{
    bsp_board_led_invert(0);

    /* Call again after 1s */
    miramesh_timer_schedule(miramesh_timer_time_add_us(now, 1000000), timer_callback, NULL);
}

static void udp_callback(mira_net_udp_connection_t* connection,
                         const void* data,
                         uint16_t data_len,
                         const mira_net_udp_callback_metadata_t* metadata,
                         void* storage)
{
    /* Just blink a LED for every received packet */
    bsp_board_led_invert(1);
}

int main(void)
{
    bsp_board_init(BSP_INIT_LEDS);

    /* Start softdevice before MiraMesh */
    softdevice_start();

    /* Start MiraMesh */
    miramesh_init(&miramesh_config, NULL);

    /* Allow some memory to networking */
    MIRA_MEM_SET_BUFFER(14944);

    /* Start networking */
    mira_net_init(&net_config);

    mira_net_udp_listen(UDP_PORT, udp_callback, NULL);

    /* Call after 0.5s: */
    miramesh_timer_schedule(
      miramesh_timer_time_add_us(miramesh_timer_get_time_now(), 500000), timer_callback, NULL);

    while (1) {
        miramesh_run_once();
        __WFI();
    }

    return 0;
}
