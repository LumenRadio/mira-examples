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

#ifndef FOTA_UPDATE_H
#define FOTA_UPDATE_H

#include <stdint.h>
#include <stdbool.h>

#define FOTA_FW_SLOT_ID 0
#define FOTA_INVALID_CRC 0xffffffff

/**
 * @brief Initialize FOTA
 *
 * @param version Version of running firmware
 *
 * @return Status of the operation
 * @retval 0 on success, -1 on failure
 */
int fota_init(void);

/**
 * @brief Get CRC of running application
 *
 * @return CRC of the application
 */
uint32_t fota_get_app_crc(void);

/**
 * @brief Get size of running application
 *
 * @return CRC of the application
 */
uint32_t fota_get_app_size(void);

/**
 * @brief Get CRC of FOTA buffer
 *
 * @return CRC of the FOTA buffer, 0xffffffff if contents are invalid
 */
uint32_t fota_get_fota_crc(void);

/**
 * @brief Schedule a firmware upgrade with contents of the FOTA buffer
 *
 * @param new_crc     Expected CRC of FOTA buffer
 * @param force_reset Force the node to reset at the scheduled time
 */
void fota_perform_upgrade(uint32_t new_crc, bool force_reset);

#endif
