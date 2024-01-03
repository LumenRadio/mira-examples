PROJECT_NAME = monitoring

# Keep the node's debug port enabled:
APPROTECT_DISABLED?=yes

# Don't use deprecated Mira API
MIRA_DEPRECATED=0

CFLAGS = -std=c99

TARGET ?= nrf52832ble-os
LIBDIR ?= $(CURDIR)/../..

SOURCE_FILES = \
	network_sender.c \
	monitoring.c

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