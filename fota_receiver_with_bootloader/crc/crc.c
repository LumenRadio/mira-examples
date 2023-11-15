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

#include "crc.h"

#include <stdio.h>

void crc_init(crc_t* crc)
{
    crc->state = 0xFFFFFFFF;
    crc->poly = 0xEDB88320;
}

void crc_update(crc_t* crc, const uint8_t* data, uint32_t length)
{
    register uint32_t byte, poly, state, mask;
    register int i;

    state = crc->state;
    poly = crc->poly;
    while (length--) {
        byte = *(data++);
        state ^= byte;
        for (i = 8; i; i--) {
            mask = -(state & 1);
            state = (state >> 1) ^ (poly & mask);
        }
    }
    crc->state = state;
}

uint32_t crc_get(const crc_t* crc)
{
    return ~crc->state;
}
