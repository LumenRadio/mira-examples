PROJECT_NAME = ble

# Keep the node's debug port enabled:
APPROTECT_DISABLED?=yes

# Don't use deprecated Mira API
MIRA_DEPRECATED=0

TARGET ?= nrf52832ble-os
LIBDIR ?= $(CURDIR)/../..

SDKDIR ?= $(CURDIR)/../vendor/nrf5-sdk

vpath %.c $(SDKDIR)/components

SOURCE_FILES = \
	ble/common/ble_advdata.c \
	ble/nrf_ble_gatt/nrf_ble_gatt.c \
	ble_led.c \
	ble_app.c \
	main.c

CFLAGS += -I$(SDKDIR)/components/ble/common/
CFLAGS += -I$(SDKDIR)/components/ble/nrf_ble_gatt/
CFLAGS += -I$(SDKDIR)/modules/nrfx/mdk/
CFLAGS += -I$(SDKDIR)/components/libraries/util/
CFLAGS += -I$(SDKDIR)/components/libraries/strerror/
CFLAGS += -I$(SDKDIR)/components/libraries/log/
CFLAGS += -I$(SDKDIR)/components/libraries/log/src/
CFLAGS += -I$(SDKDIR)/components/libraries/experimental_section_vars/
CFLAGS += -I$(SDKDIR)/components/toolchain/cmsis/include/
CFLAGS += -I$(CURDIR)/

CFLAGS += -std=c99
CFLAGS += -DNRF_SD_BLE_API_VERSION=6

CPU_MODEL = $(firstword $(subst -, , $(TARGET)))

ifeq ($(CPU_MODEL),nrf52832ble)
	CFLAGS += -I$(SDKDIR)/components/softdevice/s132/headers/
	CFLAGS += -I$(CURDIR)/s132
	CFLAGS += -DNRF52832_XXAA
else ifeq ($(CPU_MODEL),nrf52840ble)
	CFLAGS += -I$(SDKDIR)/components/softdevice/s140/headers/
	CFLAGS += -I$(CURDIR)/s140
	CFLAGS += -DNRF52840_XXAA
else ifeq ($(CPU_MODEL),nrf52833ble)
	CFLAGS += -I$(SDKDIR)/components/softdevice/s140/headers/
	CFLAGS += -I$(CURDIR)/s140
	CFLAGS += -DNRF52833_XXAA
endif

include $(LIBDIR)/Makefile.include

all-targets:
	$(MAKE) TARGET=nrf52832ble-os
	$(MAKE) TARGET=nrf52840ble-os

clean-all-targets:
	$(MAKE) TARGET=nrf52832ble-os clean
	$(MAKE) TARGET=nrf52840ble-os clean
