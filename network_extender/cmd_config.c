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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rpc-interface.h"

#include "cmd_config.h"
#include "app-config.h"

static void cmd_config_set_key(char* line, const void* storage)
{
    app_config_set_key(line);
    rpc_interface_send_response(true, NULL);
}

static void cmd_config_set_rate(char* line, const void* storage)
{
    char* endptr;
    long rate = strtol(line, &endptr, 10);
    if (*endptr != 0) {
        rpc_interface_send_response(false, "Argument is not a number\n");
    } else if (rate < 0 || rate > 15) {
        rpc_interface_send_response(false, "Argument is not within 0..15\n");
    } else {
        app_config_set_rate(rate);
        rpc_interface_send_response(true, NULL);
    }
}

static void cmd_config_set_pan_id(char* line, const void* storage)
{
    app_config_set_pan_id(line);
    rpc_interface_send_response(true, NULL);
}

static void cmd_config_set_antenna(char* line, const void* storage)
{
    char* endptr;
    long antenna = strtol(line, &endptr, 10);
    if (*endptr != 0) {
        rpc_interface_send_response(false, "Argument is not a number\n");
    } else if (antenna > UINT8_MAX) {
        rpc_interface_send_response(false, "Argument is too large: %ld\n", antenna);
    } else {
        app_config_set_antenna((uint8_t)antenna);
    }
}

static void cmd_config_show(char* line, const void* storage)
{
    print_config();
}

const rpc_interface_command_t command_config_defs[] = RPC_IF_CMDS(
  RPC_IF_CMD_HANDLER("set_key", cmd_config_set_key, NULL, "<key>", "set encryption key"),
  RPC_IF_CMD_HANDLER("set_rate", cmd_config_set_rate, NULL, "<rate>", "set rate (0..15)"),
  RPC_IF_CMD_HANDLER("set_pan_id", cmd_config_set_pan_id, NULL, "<panid>", "set panid"),
  RPC_IF_CMD_HANDLER("set_antenna", cmd_config_set_antenna, NULL, "<antenna>", "set antenna"),
  RPC_IF_CMD_HANDLER("show", cmd_config_show, NULL, "", "show config"));
