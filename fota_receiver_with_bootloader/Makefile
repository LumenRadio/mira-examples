PROJECT_NAME = fota_receiver

TARGET ?= nrf52840ble-os
TARGET_APP ?= nrf52840ble-os-app
LIBDIR ?= $(CURDIR)/../..
SDKDIR ?= $(CURDIR)/../vendor/nrf5-sdk
SDK_ROOT ?= $(SDKDIR)
GCC_DIR ?= $(dir $(shell which arm-none-eabi-gcc))


NRFUTIL=nrfutil

PRIVATE_KEY_FILE = private.key

SOURCE_FILES = \
	fota_receiver.c \
	fota_update.c \
	crc/crc.c

CFLAGS += -I$(CURDIR)/crc

ifeq ($(TARGET), nrf52840ble-os)
BOOTLOADER_PATH=bootloader/pca10056_ble/armgcc
CFLAGS += -I$(SDK_ROOT)/components/softdevice/s140/headers
CFLAGS += -I$(SDK_ROOT)/components/softdevice/mbr/headers
CFLAGS += -I$(SDK_ROOT)/config/nrf52840/config/
CFLAGS += -DNRF52840_XXAA
LDSCRIPT = $(CURDIR)/nrf52840_fota.ld
TARGET_APP = nrf52840ble-os-app
else ifeq ($(TARGET), nrf52832ble-os)
BOOTLOADER_PATH=bootloader/pca10040_s132_ble/armgcc
CFLAGS += -I$(SDK_ROOT)/components/softdevice/s132/headers
CFLAGS += -I$(SDK_ROOT)/components/softdevice/mbr/headers
CFLAGS += -I$(SDK_ROOT)/config/nrf52832/config/
CFLAGS += -DNRF52832_XXAA
LDSCRIPT = $(CURDIR)/nrf52832_fota.ld
TARGET_APP = nrf52832ble-os-app
else
$(error TARGET=$(TARGET) is not supported!)
endif

BLSETTINGS_FILE  = $(PROJECT_NAME)-$(TARGET_APP)-blsettings.hex
TARGET_APP_FILE  = $(PROJECT_NAME)-$(TARGET_APP).hex

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
ifeq ($(TARGET), nrf52840ble-os)
	$(Q)$(NRF_PROG) -s "$*" --erasepage 0xfe000-0x100000
else ifeq ($(TARGET), nrf52832ble-os)
	$(Q)$(NRF_PROG) -s "$*" --erasepage 0x7e000-0x80000
endif
	# Program:
	$(Q)$(NRF_PROG) -s "$*" --sectorerase --program $(PROJECT_NAME)-$(TARGET).hex --verify
	$(Q)$(NRF_PROG) -s "$*" --sectorerase --program $(BLSETTINGS_FILE) --verify
ifeq ($(TARGET), nrf52840ble-os)
	$(Q)$(NRF_PROG) -s "$*" --sectorerase --program $(BOOTLOADER_PATH)/_build/nrf52840_xxaa_s140.hex --verify
else ifeq ($(TARGET), nrf52832ble-os)
	$(Q)$(NRF_PROG) -s "$*" --sectorerase --program $(BOOTLOADER_PATH)/_build/nrf52832_xxaa_s132.hex --verify
endif
	$(Q)$(NRF_PROG) -s "$*" --reset

# $1 file under test
define fail_when_file_doesnt_exist
$(if $(wildcard $(1)), \
	$(info '$(1)' exists already. Remove existing and try again.) $(error File '$(1)' already exists!), \
	$(info File '$(1)' not found, creating one.))
endef

$(PRIVATE_KEY_FILE):
	$(call fail_when_file_doesnt_exist,$@)
	@echo "  GEN       $@"
	$(Q)$(NRFUTIL) keys generate $@ > /dev/null
	@echo "  GEN       bootloader_dfu_public_key.c"
	$(Q)$(NRFUTIL) keys display --key pk --format code --out_file bootloader/dfu_public_key.c $(PRIVATE_KEY_FILE)

$(TARGET_APP_FILE): $(PROJECT_NAME)-$(TARGET).hex venv
	$(Q)rm -f $(TARGET_APP_FILE)
	@echo "  HEXMERGE  $@"
ifeq ($(TARGET), nrf52840ble-os)
	$(Q)venv/bin/hexmerge.py $<:0x00027000:0x7DFFF -o $@
else ifeq ($(TARGET), nrf52832ble-os)
	$(Q)venv/bin/hexmerge.py $<:0x00026000:0x4CFFF -o $@
endif

$(BLSETTINGS_FILE): $(TARGET_APP_FILE) $(PRIVATE_KEY_FILE)
	$(Q)rm -f $(BLSETTINGS_FILE)
	@echo "  GEN       $@"
ifeq ($(TARGET), nrf52840ble-os)
	$(Q)$(NRFUTIL) settings generate --no-backup --family NRF52840 --application $< --application-version 0x1 --bootloader-version 0x1 --bl-settings-version 1 --key-file $(PRIVATE_KEY_FILE) $@
else ifeq ($(TARGET), nrf52832ble-os)
	$(Q)$(NRFUTIL) settings generate --no-backup --family NRF52 --application $< --application-version 0x1 --bootloader-version 0x1 --bl-settings-version 1 --key-file $(PRIVATE_KEY_FILE) $@
endif

0.bin: $(TARGET_APP_FILE)
	rm -f 0.bin
	arm-none-eabi-objcopy -I ihex -O binary $< $@

venv: requirements.txt
	@echo "  VENV      $@"
	$(Q)virtualenv -p python3 venv
	@echo "  PIP       $<"
	$(Q)venv/bin/pip install -r requirements.txt
	$(Q)touch venv

all-targets:
	$(MAKE) TARGET=nrf52840ble-os
	$(MAKE) TARGET=nrf52832ble-os fota_receiver-nrf52832ble-os.hex

clean-all-targets:
	$(MAKE) TARGET=nrf52840ble-os clean
	$(MAKE) TARGET=nrf52832ble-os clean

.PHONY: blsettings bin help bootloader
