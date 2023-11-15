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

#include "rpc-interface.h"
#include <mira.h>
#include "reboot.h"

#include "cmd_config.h"

static void command_reset(char* line, const void* storage)
{
    mira_sys_reset();
}

static void command_dfumode(char* line, const void* storage)
{
    reboot_to_dfu();
}

static void command_version(char* line, const void* storage)
{
    printf("MiraUSB Network Extender\n");
    rpc_interface_send_response(true, NULL);
}

const rpc_interface_command_t command_defs[] = RPC_IF_CMDS(
  RPC_IF_CMD_HANDLER("reset", command_reset, NULL, "", "Reset CPU"),
  RPC_IF_CMD_HANDLER("dfumode",
                     command_dfumode,
                     NULL,
                     "",
                     "Reset CPU to device firmware update mode"),
  RPC_IF_CMD_HANDLER("config", rpc_interface_command_handler, command_config_defs, "", ""),
  RPC_IF_CMD_HANDLER("version", command_version, NULL, "", "Version info"));
