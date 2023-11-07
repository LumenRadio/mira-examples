/*----------------------------------------------------------------------------
Copyright (c) 2023 LumenRadio AB
This code is the property of LumenRadio AB and may not be redistributed in any
form without prior written permission from LumenRadio AB.

This example is provided as is, without warranty.
----------------------------------------------------------------------------*/

#include <mira.h>
#include <stdio.h>
#include <string.h>

#define WRITE_AREA_SIZE 0x10000

MIRA_IODEFS(MIRA_IODEF_NONE,    /* fd 0: stdin */
            MIRA_IODEF_UART(0), /* fd 1: stdout */
            MIRA_IODEF_NONE     /* fd 2: stderr */
                                /* More file descriptors can be added, for use with dprintf(); */
);

typedef union
{
    uint8_t u8[512];
    uint16_t u16[256];
    uint32_t u32[128];
} dummy_data_t;

extern uint8_t __SwapStart;

PROCESS(main_proc, "Main process");

void mira_setup(void)
{
    mira_status_t uart_ret;
    mira_uart_config_t uart_config = {
        .baudrate = 115200,
#if MIRA_PLATFORM_MKW41Z
        .tx_pin = MIRA_GPIO_PIN('C', 7),
        .rx_pin = MIRA_GPIO_PIN('C', 6)
#else
        .tx_pin = MIRA_GPIO_PIN(0, 6),
        .rx_pin = MIRA_GPIO_PIN(0, 8)
#endif
    };

    MIRA_MEM_SET_BUFFER(7592);

    uart_ret = mira_uart_init(0, &uart_config);
    if (uart_ret != MIRA_SUCCESS) {
        /* Nowhere to send an error message */
    }

    process_start(&main_proc, NULL);
}

PROCESS_THREAD(main_proc, ev, data)
{
    static mira_status_t res;
    static uint32_t page_size;
    static uint32_t page;
    static uint8_t num_pages;
    static uint32_t start_address;
    static uint32_t write_offset;
    static dummy_data_t dummy_data;
    static int i;

    PROCESS_BEGIN();
    /* Pause once, so we don't run anything before finish of startup */
    PROCESS_PAUSE();

    start_address = (uint32_t)&__SwapStart - WRITE_AREA_SIZE;

    /* Initialise dummy data to non-zero value*/
    for (i = 0; i < sizeof(dummy_data) / sizeof(dummy_data.u32[0]); i++) {
        dummy_data.u32[i] = 0xcafecafe;
    }

    /* Erase relevant pages */
    page_size = mira_flash_get_page_size();
    num_pages = (WRITE_AREA_SIZE) / page_size;
    for (i = 0; i < num_pages; i++) {
        page = start_address + (page_size * i);
        printf("erasing page: 0x%08lx\n", page);
        do {
            res = mira_flash_erase_page(page);
            if (res != MIRA_SUCCESS) {
                printf("!mira_flash_erase_page() failed: %d, retrying...\n", res);
            }
            PROCESS_WAIT_WHILE(mira_flash_is_working());
            if (!mira_flash_succeeded()) {
                printf("!erase op failed, retrying...\n");
            }
        } while (res != MIRA_SUCCESS || !mira_flash_succeeded());
    }

    /* Write to flash */
    for (write_offset = start_address; write_offset < (uint32_t)&__SwapStart;
         write_offset += sizeof(dummy_data)) {
        do {
            printf("writing: 0x%08lx... at: 0x%08lx\n", dummy_data.u32[0], write_offset);
            res = mira_flash_write(write_offset, (void*)dummy_data.u32, sizeof(dummy_data));
            if (res != MIRA_SUCCESS) {
                printf("!mira_flash_write failed: %d, retrying...\n", res);
            }
            PROCESS_WAIT_WHILE(mira_flash_is_working());
            if (!mira_flash_succeeded()) {
                printf("!write op failed, retrying...\n");
            }
        } while (res != MIRA_SUCCESS || !mira_flash_succeeded());

        /* Verify flash contents after write */
        if (memcmp(dummy_data.u32, (uint32_t*)write_offset, sizeof(dummy_data)) != 0) {
            printf("!flash content verification failed\n");
        }
    }
    printf("wrote up to SWAP, exiting...\n");
    PROCESS_END();
}
