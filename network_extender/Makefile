PROJECT_NAME ?= network_extender

# Keep the node's debug port enabled:
APPROTECT_DISABLED?=yes

# Don't use deprecated Mira API
MIRA_DEPRECATED=0

PRIVATE_KEY_FILE ?= private.key

NRFUTIL=nrfutil

TARGET ?= nrf52840ble-os
LIBDIR ?= $(CURDIR)/../vendor/libmira

SDKDIR ?= $(CURDIR)/../vendor/nrf5-sdk

LDSCRIPT?=$(TARGET)-fw.ld

SOURCE_FILES = \
	app-config.c \
	command_defs.c \
	cmd_config.c \
	main.c \
	rpc-interface.c \
	reboot.c \

CFLAGS += \
	-I$(LIBDIR)/include \
	-I$(SDKDIR)/modules/nrfx/mdk/ \
	-I$(SDKDIR)/components/toolchain/cmsis/include/ \

CFLAGS += -std=gnu99
CFLAGS += -DNRF_SD_BLE_API_VERSION=6
CFLAGS += -DMIRA_EXPERIMENTAL

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

# The internal Makefile is not part of the archive. Customers can
# create one if the need arises. For example, it can be used for
# setting `PRIVATE_KEY_FILE` corresponding to already existing
# public key.
-include Makefile.internal

include $(LIBDIR)/Makefile.include

APPLICATION_VERSION=$(shell date +%s)

%-app.elf: %.elf
	$(TRACE_OBJCOPY)
	$(Q)$(TARGET_OBJCOPY) --remove-section .text.mbr_sd $^ $@

%-sd.elf: %.elf
	$(TRACE_OBJCOPY)
	$(Q)$(TARGET_OBJCOPY) --only-section .text.mbr_sd $^ $@

%.hex: %.elf
	$(TRACE_OBJCOPY)
	$(Q)$(TARGET_OBJCOPY) -O ihex $^ $@

%.dfu: %.bin %-app.hex %-sd.hex | $(PRIVATE_KEY_FILE)
	echo "#define APPLICATION_VERSION $(APPLICATION_VERSION)" > fw_version.h
	echo "Key file: $(PRIVATE_KEY_FILE)"
	$(NRFUTIL) pkg generate \
		--softdevice $*-sd.hex \
		--application $*-app.hex \
		--key-file $(PRIVATE_KEY_FILE) \
		--hw-version 52 \
		--application-version $(APPLICATION_VERSION) \
		--sd-req 0 \
		--sd-id 0x100 \
		$@

dfu: $(PROJECT_NAME)-$(TARGET).dfu

clean::
	rm -rf private.key dfu_public_key.c
	rm -rf *.dfu
	rm -rf fw_version.h
all-targets:
	$(MAKE)

clean-all-targets:
	$(MAKE) clean

# $1 file under test
define fail_when_file_doesnt_exist
$(if $(wildcard $(1)), \
	$(info '$(1)' exists already. Remove existing and try again.) $(error File '$(1)' already exists!), \
	$(info File '$(1)' not found, creating one.))
endef

# Generate private and public keys
private.key:
	$(call fail_when_file_doesnt_exist,$@)
	@echo "  GEN       $@"
	$(Q)$(NRFUTIL) keys generate $@ > /dev/null
	@echo "  GEN       dfu_public_key.c"
	$(Q)$(NRFUTIL) keys display --key pk --format code --out_file dfu_public_key.c $@
