PROJECT_NAME = fota_receiver_with_driver

# Keep the node's debug port enabled:
APPROTECT_DISABLED?=yes

# Don't use deprecated Mira API
MIRA_DEPRECATED=0

TARGET ?= nrf52832ble-os
LIBDIR ?= $(CURDIR)/../..

vpath %.c ../fota_sender_with_driver

SOURCE_FILES = \
	fota_receiver_with_driver.c \
	fota_driver.c

include $(LIBDIR)/Makefile.include

all-targets:
	$(MAKE) TARGET=nrf52832ble-os
	$(MAKE) TARGET=nrf52840ble-os
	$(MAKE) TARGET=mkw41z-os
	$(MAKE) TARGET=mirasim-os

clean-all-targets:
	$(MAKE) TARGET=nrf52832ble-os clean
	$(MAKE) TARGET=nrf52840ble-os clean
	$(MAKE) TARGET=mkw41z-os clean
	$(MAKE) TARGET=mirasim-os clean
