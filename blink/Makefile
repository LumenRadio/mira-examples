PROJECT_NAME = blink

# Keep the node's debug port enabled:
APPROTECT_DISABLED?=yes

# Don't use deprecated Mira API
MIRA_DEPRECATED=0

TARGET ?= nrf52832ble-os
LIBDIR ?= $(CURDIR)/../..

SOURCE_FILES = \
	blink.c

include $(LIBDIR)/Makefile.include

all-targets:
	$(MAKE) TARGET=nrf52832ble-os
	$(MAKE) TARGET=nrf52840ble-os
	$(MAKE) TARGET=mkw41z-os

clean-all-targets:
	$(MAKE) TARGET=nrf52832ble-os clean
	$(MAKE) TARGET=nrf52840ble-os clean
	$(MAKE) TARGET=mkw41z-os clean
