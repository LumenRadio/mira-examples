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

#ifndef RPC_INTERFACE_H
#define RPC_INTERFACE_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

typedef void (*rpc_interface_handler_t)(char* line, const void* storage);

void rpc_interface_init(rpc_interface_handler_t handler, int report_fd, const void* storage);

void rpc_interface_input_byte(char byte);

void rpc_interface_send_response(bool success, const char* fmt, ...);

/*
 * Command argument processing
 */

/*
 * Extract variables from a command line
 *
 * argv is a NULL terminated list of strings.
 *
 * fmt is a list of characters representing variable types of the arguments:
 *   f - double
 *   i - int
 *   s - string (char *)
 *   . - skip argument (useful to skip command name as first argument)
 *   : - following arguments in fmt are optional, and set only if available
 *   + - as last argument, return rest of line as string
 *
 * arguments following fmt are pointers to variables to store the value within.
 *
 * returns enum value from command_status.
 *
 * Example:
 * myint = 13; - default value
 * if(command_getargs(argv, "f:i", &myfloat, &myint)) {
 *   ..error..
 * }
 */
int rpc_interface_get_args(char* line, const char* fmt, ...);

/**
 * Expect a set of arugmnets from a process
 *
 * Needs to be used before first process yield, and will update "data"
 *
 * Note that strings returned from this may be overwritten upon yield
 */
#define RPC_IF_PROCESS_EXPECT_ARGS(_FMT, ...)                     \
    if (0 != rpc_interface_get_args(data, (_FMT), __VA_ARGS__)) { \
        rpc_interface_send_response(false, "invalid arguments");  \
        PROCESS_EXIT();                                           \
    }

/**
 * Expect a set of arugmnets from a function
 */
#define RPC_IF_EXPECT_ARGS(_CMDLINE, _FMT, ...)                         \
    if (0 != rpc_interface_get_args((_CMDLINE), (_FMT), __VA_ARGS__)) { \
        rpc_interface_send_response(false, "invalid arguments");        \
        return;                                                         \
    }

/*
 * Command processing
 */

typedef struct
{
    const char* command;
    const char* help_args;
    const char* help;
    rpc_interface_handler_t handler;
    const void* storage;
} rpc_interface_command_t;

#define RPC_IF_CMDS(...)              \
    (const rpc_interface_command_t[]) \
    {                                 \
        __VA_ARGS__,                  \
        {                             \
            0                         \
        }                             \
    }
#define RPC_IF_CMD_HANDLER(CMD, HANDLER, STORAGE, HELP_ARGS, HELP) \
    {                                                              \
        CMD, HELP_ARGS, HELP, HANDLER, (STORAGE)                   \
    }
#define RPC_IF_CMD_PROCESS(CMD, PROCESS, HELP_ARGS, HELP)               \
    {                                                                   \
        CMD, HELP_ARGS, HELP, rpc_interface_process_handler, (&PROCESS) \
    }
#define RPC_IF_SUB_CMDS(CMD, ...) \
    RPC_IF_CMD_HANDLER(CMD, rpc_interface_command_handler, RPC_IF_CMDS(__VA_ARGS__), "", "")

void rpc_interface_command_handler(char* line, const void* storage);

void rpc_interface_process_handler(char* line, const void* storage);

/**
 * Unpack hex block of specified length
 *
 * The input length is checked and must match the expected length
 *
 * return 0 on success, negative on error
 */
int rpc_interface_dehex(uint8_t* dst, const char* src, int len);

#endif
