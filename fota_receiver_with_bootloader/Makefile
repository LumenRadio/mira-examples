PROJECT_NAME = fota_receiver

TARGET ?= nrf52840ble-os
TARGET_APP ?= nrf52840ble-os-app
LIBDIR ?= $(CURDIR)/../vendor/libmira
SDKDIR ?= $(CURDIR)/../vendor/nrf5-sdk
SDK_ROOT ?= $(SDKDIR)
GCC_DIR ?= $(dir $(shell which arm-none-eabi-gcc))


NRFUTIL=venv/bin/nrfutil

PRIVATE_KEY_FILE = private.key
BLSETTINGS_FILE  = $(PROJECT_NAME)-$(TARGET_APP)-blsettings.hex
TARGET_APP_FILE  = $(PROJECT_NAME)-$(TARGET_APP).hex
BOOTLOADER_PATH=bootloader/pca10056_ble/armgcc

SOURCE_FILES = \
	fota_receiver.c \
	fota_update.c \
	crc/crc.c

CFLAGS += -I$(CURDIR)/crc
CFLAGS += -I$(SDK_ROOT)/components/softdevice/s140/headers
CFLAGS += -I$(SDK_ROOT)/components/softdevice/mbr/headers
CFLAGS += -I$(SDK_ROOT)/config/nrf52840/config/
CFLAGS += -DNRF52840_XXAA
LDSCRIPT = $(CURDIR)/nrf52840_fota.ld

CFLAGS += -I$(SDK_ROOT)/components/libraries/bootloader/dfu
CFLAGS += -I$(SDK_ROOT)/components/libraries/util

CFLAGS += -I$(SDK_ROOT)/components/softdevice/common/
CFLAGS += -I$(SDK_ROOT)/components/toolchain/cmsis/include
CFLAGS += -I$(SDK_ROOT)/modules/nrfx/mdk
CFLAGS += -I$(SDK_ROOT)/components/libraries/experimental_section_vars

include $(LIBDIR)/Makefile.include

all: $(BLSETTINGS_FILE) bootloader

blsettings: $(BLSETTINGS_FILE)

bootloader: $(PRIVATE_KEY_FILE)
	$(Q)GNU_INSTALL_ROOT="$(GCC_DIR)/" $(MAKE) -C $(BOOTLOADER_PATH) SDK_ROOT=$(SDK_ROOT)

bin: 0.bin

clean::
	rm -f 0.bin
	rm -fr venv
	rm -f $(BLSETTINGS_FILE)
	rm -f $(TARGET_APP_FILE)
	$(MAKE) -C $(BOOTLOADER_PATH) clean SDK_ROOT=$(SDK_ROOT)
	rm -rf bootloader/dfu_public_key.c $(PRIVATE_KEY_FILE)

help::
	@echo Available targets:
	@echo all - builds the applicatin, bootloader and bl-settings
	@echo bin - Generates 0.bin for updating via FOTA
	@echo bootloader - builds the bootloader
	@echo clean - remove all generated files, except keys
	@echo install.<snr> - flashes the target with app+bootloader+settings

install.%: all
	$(TRACE_FLASH)
	$(Q)$(NRF_PROG) -s "$*" --halt
	# Erase pages updated by the bootloader:
	$(Q)$(NRF_PROG) -s "$*" --erasepage 0
	$(Q)$(NRF_PROG) -s "$*" --eraseuicr
	$(Q)$(NRF_PROG) -s "$*" --erasepage 0xfe000-0x100000
	# Program:
	$(Q)$(NRF_PROG) -s "$*" --sectorerase --program $(PROJECT_NAME)-$(TARGET).hex --verify
	$(Q)$(NRF_PROG) -s "$*" --sectorerase --program $(BLSETTINGS_FILE) --verify
	$(Q)$(NRF_PROG) -s "$*" --sectorerase --program $(BOOTLOADER_PATH)/_build/nrf52840_xxaa_s140.hex --verify
	$(Q)$(NRF_PROG) -s "$*" --reset

# $1 file under test
define fail_when_file_doesnt_exist
$(if $(wildcard $(1)), \
	$(info '$(1)' exists already. Remove existing and try again.) $(error File '$(1)' already exists!), \
	$(info File '$(1)' not found, creating one.))
endef

$(PRIVATE_KEY_FILE): venv
	$(call fail_when_file_doesnt_exist,$@)
	@echo "  GEN       $@"
	$(Q)$(NRFUTIL) keys generate $@ > /dev/null
	@echo "  GEN       bootloader_dfu_public_key.c"
	$(Q)$(NRFUTIL) keys display --key pk --format code --out_file bootloader/dfu_public_key.c $(PRIVATE_KEY_FILE)

$(TARGET_APP_FILE): $(PROJECT_NAME)-$(TARGET).hex venv
	$(Q)rm -f $(TARGET_APP_FILE)
	@echo "  HEXMERGE  $@"
	$(Q)venv/bin/hexmerge.py $<:0x00027000:0x7DFFF -o $@

$(BLSETTINGS_FILE): $(TARGET_APP_FILE) $(PRIVATE_KEY_FILE) venv
	$(Q)rm -f $(BLSETTINGS_FILE)
	@echo "  GEN       $@"
	$(Q)$(NRFUTIL) settings generate --no-backup --family NRF52840 --application $< --application-version 0x1 --bootloader-version 0x1 --bl-settings-version 1 --key-file $(PRIVATE_KEY_FILE) $@

0.bin: $(TARGET_APP_FILE)
	rm -f 0.bin
	arm-none-eabi-objcopy -I ihex -O binary $< $@

venv: requirements.txt
	@echo "  VENV      $@"
	$(Q)virtualenv -p python3.10 venv >/dev/null
	@echo "  PIP       $<"
	$(Q)venv/bin/pip install -r requirements.txt >/dev/null
	$(Q)touch venv

all-targets:
	$(MAKE) TARGET=nrf52840ble-os

clean-all-targets:
	$(MAKE) TARGET=nrf52840ble-os clean

.PHONY: blsettings bin help bootloader
