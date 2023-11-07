PROJECT_NAME = flash_write

# Keep the node's debug port enabled:
APPROTECT_DISABLED?=yes

# Don't use deprecated Mira API
MIRA_DEPRECATED=0

TARGET ?= nrf52832ble-os
LIBDIR ?= $(CURDIR)/../vendor/libmira

SOURCE_FILES = \
	flash_write.c

include $(LIBDIR)/Makefile.include
