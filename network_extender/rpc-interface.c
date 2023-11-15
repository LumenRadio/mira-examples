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
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

static rpc_interface_handler_t rpc_interface_handler;
static const void* rpc_interface_handler_storage;
static int rpc_interface_reporting_fd;
static char prefix_buffer[128];

PROCESS(rpc_interface_start_message, "RPC start message sender");

void rpc_interface_init(rpc_interface_handler_t handler, int report_fd, const void* storage)
{
    rpc_interface_handler = handler;
    rpc_interface_handler_storage = storage;
    rpc_interface_reporting_fd = report_fd;

    process_start(&rpc_interface_start_message, NULL);
}

PROCESS_THREAD(rpc_interface_start_message, ev, data)
{
    PROCESS_BEGIN();
    PROCESS_PAUSE();
    PROCESS_END();
}

void rpc_interface_input_byte(char byte)
{
    static char linebuf[256];
    static int linepos = 0;
    /* Got a line */
    if (byte == '\n' || byte == '\r') {
        if (linepos > 0) {
            linebuf[linepos++] = 0;
            linepos = 0;
            prefix_buffer[0] = '\0'; /* So command handlers can have nice help output */
            rpc_interface_handler(linebuf, rpc_interface_handler_storage);
        }
    } else {
        if (linepos < sizeof(linebuf) - 1) {
            linebuf[linepos++] = byte;
        }
    }
}

static void rpc_interface_send_response_int(int fd, bool success, const char* fmt, va_list ap)
{
    if (success) {
        dprintf(fd, "success");
    } else {
        dprintf(fd, "error");
    }

    if (fmt != NULL) {
        dprintf(fd, ": ");
        vdprintf(fd, fmt, ap);
    }
    dprintf(fd, "\n");
}

void rpc_interface_send_response(bool success, const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    rpc_interface_send_response_int(rpc_interface_reporting_fd, success, fmt, ap);
    va_end(ap);
}

int rpc_interface_get_args(char* line, const char* fmt, ...)
{
    int return_value = 0;
    char* curarg;
    char* tmpline = line;
    const char* curfmt = fmt;
    int is_optional = 0;
    va_list ap;

    va_start(ap, fmt);

    while (true) {
        if (*curfmt == '+') {
            /* The rest is treated as a string, as a whole, so don't split next */
            *va_arg(ap, const char**) = tmpline;
            break;
        }

        /* Get argument, if available */
        curarg = strsep(&tmpline, " ");
        if (curarg == NULL) {
            break;
        }
        if (*curarg == '\0') {
            /* Ignore field */
            continue;
        }

        if (*curfmt == '\0') {
            /* End of arguments, but still data left */
            return_value = -1;
            goto error;
        }
        if (*curfmt == ':') {
            /* Next is optional, start over in loop with next fmt char */
            is_optional = 1;
            curfmt++;
            continue;
        }
        /* Valid arguments */
        if (*curfmt == '.') {
            /* Ignore argument*/
        } else if (*curfmt == 's') {
            *va_arg(ap, const char**) = curarg;
        } else if (*curfmt == 'f') {
            *va_arg(ap, float*) = (float)atof(curarg);
        } else if (*curfmt == 'i') {
            *va_arg(ap, int*) = atoi(curarg);
        } else if (*curfmt == 'x') {
            *va_arg(ap, uint32_t*) = strtoul(curarg, NULL, 16);
        } else {
            return_value = -2;
            goto error;
        }
        curfmt++;
    }
    /* No more arguments, success if no more arguments are expected */
    if (*curfmt == '\0' || *curfmt == ':' || *curfmt == '+' || is_optional) {
        return_value = 0;
    } else {
        return_value = -3;
    }

error:
    va_end(ap);
    return return_value;
}

static void rpc_interface_help_handler(char* prefix, const rpc_interface_command_t* start_command)
{
    const rpc_interface_command_t* curcmd;
    int len;
    for (curcmd = start_command; curcmd->command != NULL; curcmd++) {
        if (curcmd->handler == rpc_interface_command_handler) {
            /* Processing of subcommands, recursion */
            int prefix_length = strlen(prefix);
            snprintf(&prefix[prefix_length],
                     sizeof(prefix_buffer) - prefix_length,
                     "%s ",
                     curcmd->command);
            rpc_interface_help_handler(prefix, curcmd->storage);
            prefix[prefix_length] = '\0';
        } else {
            /* Executable command */
            len = printf("  %s%s %s", prefix, curcmd->command, curcmd->help_args);
            if (len > 40) {
                printf("\n");
                len = 0;
            }
            while (len < 40) {
                putchar(' ');
                len++;
            }
            printf(" - %s\n", curcmd->help);
        }
    }
}

void rpc_interface_command_handler(char* line, const void* storage)
{
    const rpc_interface_command_t* curcmd;
    char* cmd;

    cmd = strsep(&line, " ");
    if (cmd == NULL) {
        printf("\nMissing argument\n\n");
        rpc_interface_help_handler(prefix_buffer, storage);
        rpc_interface_send_response(false, "missing argument");
        return;
    }

    if (0 == strcmp(cmd, "help")) {
        /* Always allow "help" command */
        rpc_interface_help_handler(prefix_buffer, storage);
        rpc_interface_send_response(true, NULL);
        return;
    }

    for (curcmd = storage; curcmd->command != NULL; curcmd++) {
        if (strcmp(curcmd->command, cmd) == 0) {
            /* For help text prefixes */
            snprintf(&prefix_buffer[strlen(prefix_buffer)],
                     sizeof(prefix_buffer) - strlen(prefix_buffer),
                     "%s ",
                     curcmd->command);
            curcmd->handler(line, curcmd->storage);
            return;
        }
    }

    printf("\nUnknown command '%s'\n\n", cmd);
    rpc_interface_help_handler(prefix_buffer, storage);
    rpc_interface_send_response(false, "unknown command");
}

void rpc_interface_process_handler(char* line, const void* storage)
{
    /* TODO: Make it possible not to cast away const, but still keep command tables in flash */
    struct process* proc = (struct process*)storage;

    if (process_is_running(proc)) {
        rpc_interface_send_response(false, "process already running");
    } else {
        process_start(proc, line);
    }
}

int rpc_interface_dehex(uint8_t* dst, const char* src, int len)
{
    int i;
    if (strlen(src) != len * 2) {
        return -1;
    }

    for (i = 0; i < len; i++) {
        char c0 = src[2 * i + 0];
        if (c0 >= 'a' && c0 <= 'f') {
            c0 -= 'a' - 10;
        } else if (c0 >= 'A' && c0 <= 'F') {
            c0 -= 'A' - 10;
        } else if (c0 >= '0' && c0 <= '9') {
            c0 -= '0';
        } else {
            return -2;
        }
        char c1 = src[2 * i + 1];
        if (c1 >= 'a' && c1 <= 'f') {
            c1 -= 'a' - 10;
        } else if (c1 >= 'A' && c1 <= 'F') {
            c1 -= 'A' - 10;
        } else if (c1 >= '0' && c1 <= '9') {
            c1 -= '0';
        } else {
            return -2;
        }
        dst[i] = ((uint8_t)c0 << 4) | (uint8_t)c1;
    }
    return 0;
}
