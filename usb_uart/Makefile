PROJECT_NAME = usb-uart

# Keep the node's debug port enabled:
APPROTECT_DISABLED?=yes

# Don't use deprecated Mira API
MIRA_DEPRECATED=0

TARGET ?= nrf52840ble-os
LIBDIR ?= $(CURDIR)/../vendor/libmira

SUPPORTED_TARGETS += nrf52840ble-os
SUPPORTED_TARGETS += nrf52832ble-os

SOURCE_FILES = \
	usb_uart.c

include $(LIBDIR)/Makefile.include

all-targets:
	$(MAKE) TARGET=nrf52840ble-os

clean-all-targets:
	$(MAKE) TARGET=nrf52840ble-os clean
