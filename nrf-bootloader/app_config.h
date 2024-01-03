#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#if BOOTLOADER_UART_DEBUG

/* Overrides when enabling log */

#define NRF_LOG_ENABLED 1

#define NRF_LOG_BACKEND_UART_ENABLED 1
#define NRF_LOG_BACKEND_UART_TX_PIN 6 /* P0.06 */

#define UART_DEFAULT_CONFIG_HWFC NRF_UART_HWFC_DISABLED
#define UART_DEFAULT_CONFIG_PARITY NRF_UART_PARITY_EXCLUDED
#define UART_DEFAULT_CONFIG_BAUDRATE 30924800 /* 115200 */
#define UART_DEFAULT_CONFIG_IRQ_PRIORITY 6
#define NRF_LOG_DEFAULT_LEVEL 3

#define NRF_LOG_BACKEND_UART_BAUDRATE UART_DEFAULT_CONFIG_BAUDRATE

#define NRF_LOG_BACKEND_UART_TEMP_BUFFER_SIZE 128

#define NRFX_UARTE_ENABLED 1
#define NRFX_UARTE0_ENABLED 1

#define NRF_LOG_DEFERRED 0
#define NRFX_USBD_CONFIG_LOG_ENABLED 1

#endif

#endif