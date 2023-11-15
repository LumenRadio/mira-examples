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

static void nfc_field_on(void* storage);

static void nfc_field_off(void* storage);

static uint8_t* nfc_file_open(mira_nfc_file_id_t file_id, mira_size_t* size, void* storage);

static mira_bool_t nfc_file_save(mira_nfc_file_id_t file_id, mira_size_t size, void* storage);

static void create_nfc_ndef_file(void);

static void print_nfc_file(void);

static void print_var(const char* name, const uint8_t* data, mira_size_t length);

MIRA_IODEFS(MIRA_IODEF_NONE,
            MIRA_IODEF_UART(0), /* fd 1 = stdout. Enable printf */
            MIRA_IODEF_NONE);

PROCESS(main_proc, "Main process");

static uint8_t nfc_file_buffer[1024];
static mira_size_t nfc_file_size = 0;

static const char* tnf_names[] = { "empty",         "well known type", "MIME",      "absolute URI",
                                   "external type", "unknown",         "unchanged", "reserved" };

static mira_nfc_config_t nfc_conf = { .callback_field_on = nfc_field_on,
                                      .callback_field_off = nfc_field_off,
                                      .callback_file_open = nfc_file_open,
                                      .callback_file_save = nfc_file_save,
                                      .storage = NULL,
                                      .max_file_size = sizeof(nfc_file_buffer),
                                      .proprietary_file_count = 0 };

void mira_setup(void)
{
    mira_status_t uart_ret;
    mira_uart_config_t uart_config = { .baudrate = 115200,
                                       .tx_pin = MIRA_GPIO_PIN(0, 6),
                                       .rx_pin = MIRA_GPIO_PIN(0, 8) };

    MIRA_MEM_SET_BUFFER(7592);

    uart_ret = mira_uart_init(0, &uart_config);
    if (uart_ret != MIRA_SUCCESS) {
        /* Nowhere to send an error message */
    }

    /* Start main process */
    process_start(&main_proc, NULL);

    /* Start ticker so watchdog will re-trigger */
}

PROCESS_THREAD(main_proc, ev, data)
{
    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup */
    PROCESS_PAUSE();

    /* Setup nfc */
    create_nfc_ndef_file();
    print_nfc_file();
    mira_nfc_init(&nfc_conf);

    PROCESS_END();
}

/**
 * Callback for field on
 *
 * Called when nfc detects the presence of a field. No need for action.
 */
static void nfc_field_on(void* storage)
{
    printf("Field on\n");
}

/**
 * Callback for field off
 *
 * Called when nfc leaves a field. No need for action.
 */
static void nfc_field_off(void* storage)
{
    printf("Field off\n");
}

/**
 * Callback for opening file
 *
 * According to NFC tag type 4, data is stored in files.
 *
 * Since we don't use proprietary files, the only valid file id is the NDEF
 * file, identified by id MIRA_NFC_NDEF_FILE_ID.
 *
 * Return a pointer containing the content of the file. This must be writeable
 * by the nfc card emulator, up until next nfc_file_open. It must be of minimum
 * size max_file_size, since writing can increase the size of the content.
 *
 * The length of the data is stored in *size
 */
static uint8_t* nfc_file_open(mira_nfc_file_id_t file_id, mira_size_t* size, void* storage)
{
    if (file_id == MIRA_NFC_NDEF_FILE_ID) {
        printf("NDEF file open\n");
        *size = nfc_file_size;
        return nfc_file_buffer;
    }
    printf("Trying to open unknown file 0x%04x\n", file_id);
    *size = 0;
    return NULL;
}

/**
 * Callback for save successful
 *
 * This indicates that the buffer previously returned by nfc_file_open is
 * updated.
 *
 * The new size of the buffer is available in variable size.
 *
 * Return MIRA_TRUE if save is successful, MIRA_FALSE if not.
 *
 * A normal save procedure is first to erase the file to a size of 0, before
 * saving the content. So a normal save operation results in two calls, one
 * of size 0, one of the actual size. However, this is not a guarantee.
 */
static mira_bool_t nfc_file_save(mira_nfc_file_id_t file_id, mira_size_t size, void* storage)
{
    if (file_id == MIRA_NFC_NDEF_FILE_ID) {
        printf("NDEF file save of size %lu\n", size);
        nfc_file_size = size;
        print_nfc_file();
        return MIRA_TRUE;
    }
    printf("Trying to save %lu bytes to unknown file %04x\n", size, file_id);
    return MIRA_FALSE;
}

/**
 * Helper to create initial NDEF file
 */
static void create_nfc_ndef_file(void)
{
    mira_nfc_ndef_writer_t writer;
    mira_nfc_ndef_write_start(&writer, nfc_file_buffer, sizeof(nfc_file_buffer));
    mira_nfc_ndef_write_copy(&writer,
                             MIRA_NFC_NDEF_TNF_MIME_TYPE,
                             (const uint8_t*)"application/vnd.lumenradio.config",
                             33,
                             NULL,
                             0,
                             (const uint8_t*)"some config",
                             11);
    mira_nfc_ndef_write_end(&writer, &nfc_file_size);
}

/**
 * Helper for printing the NDEF message
 *
 * Iterates over and prints all NDEF records in an NDEF message.
 */
static void print_nfc_file(void)
{
    mira_nfc_ndef_iter_t iter;
    mira_nfc_ndef_record_t rec;
    printf("NDEF file content:\n");

    if (nfc_file_size == 0) {
        printf("Empty NDEF file\n");
        return;
    }

    for (mira_nfc_ndef_iter_start(&iter, &rec, nfc_file_buffer, nfc_file_size);
         mira_nfc_ndef_iter_valid(&iter);
         mira_nfc_ndef_iter_next(&iter, &rec)) {
        printf("Record #%-3d\n", rec.index);
        if (rec.chunk) {
            printf("  Chunk flag set, more data in next record\n");
        }
        printf("  Type name format\n    %s\n", tnf_names[rec.type_name_format]);
        print_var("Type", rec.type, rec.type_length);
        print_var("ID", rec.id, rec.id_length);
        print_var("Payload", rec.payload, rec.payload_length);
    }
}

/**
 * Helper to print a variable as hex and readable chars
 */
static void print_var(const char* name, const uint8_t* data, mira_size_t length)
{
    mira_size_t row, col, pos;
    printf("  %s\n", name);
    if (length == 0) {
        printf("    No content\n");
    }
    for (row = 0; row < length; row += 16) {
        printf("    %04lx: ", row);
        for (col = 0; col < 16; col++) {
            pos = row + col;
            if (pos >= length) {
                printf("   ");
            } else {
                printf(" %02x", data[pos]);
            }
        }
        printf("  ");
        for (col = 0; col < 16; col++) {
            pos = row + col;
            if (pos >= length) {
                printf(" ");
            } else if (data[pos] >= 0x20 && data[pos] < 0x7f) {
                printf("%c", data[pos]);
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
}
