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

#include "fota_driver.h"

/* Store the images in the RAM */
static uint8_t swap_area[NUMBER_OF_SLOTS][SWAP_AREA_SLOT_SIZE];

/* Implement all the necessary functions for the FOTA driver */

/* Initialize possible ports, etc */
void fota_driver_init(void) {}

int fota_driver_get_size(uint16_t slot_id,
                         uint32_t* size,
                         void (*done_callback)(void* storage),
                         void* storage)
{
    if (slot_id < 0 || slot_id >= NUMBER_OF_SLOTS) {
        return -1;
    }
    *size = SWAP_AREA_SLOT_SIZE;
    done_callback(storage);
    return 0;
}

int fota_driver_read(uint16_t slot_id,
                     void* data,
                     uint32_t address,
                     uint32_t length,
                     void (*done_callback)(void* storage),
                     void* storage)
{
    if (slot_id < 0 || slot_id >= NUMBER_OF_SLOTS) {
        return -1;
    }
    memcpy(data, &swap_area[slot_id][address], length);
    done_callback(storage);
    return 0;
}

int fota_driver_write(uint16_t slot_id,
                      const void* data,
                      uint32_t address,
                      uint32_t length,
                      void (*done_callback)(void* storage),
                      void* storage)
{
    if (slot_id < 0 || slot_id >= NUMBER_OF_SLOTS) {
        return -1;
    }
    memcpy(&swap_area[slot_id][address], data, length);
    done_callback(storage);
    return 0;
}

int fota_driver_erase(uint16_t slot_id, void (*done_callback)(void* storage), void* storage)
{
    memset(&swap_area[slot_id], 0xf, SWAP_AREA_SLOT_SIZE);
    done_callback(storage);
    return 0;
}

void fota_set_driver(void)
{
    mira_fota_set_driver(fota_driver_init,
                         fota_driver_get_size,
                         fota_driver_read,
                         fota_driver_write,
                         fota_driver_erase);
}
