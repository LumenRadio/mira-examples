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

#include "reboot.h"

#include <mira.h>
#undef MIN
#undef MAX
#include <nrf_soc.h>

void reboot_to_dfu(void)
{
    // Enter DFU after next reboot
    sd_power_gpregret_clr(0, 0xffffffff);
    sd_power_gpregret_set(0, 0xb1);
    mira_sys_reset();
}

void reboot_parser_input(char c)
{
    static int reboot_position;
    const uint8_t reboot_cmd[] = { 0x7e, 0x02, 0xff, 0x8f, 0x33, 0x7e };

    if (c == reboot_cmd[reboot_position]) {
        if (++reboot_position >= sizeof(reboot_cmd)) {
            reboot_to_dfu();
        }
    } else {
        reboot_position = 0;
    }
}
