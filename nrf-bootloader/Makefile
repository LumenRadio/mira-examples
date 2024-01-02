PROJ_DIR					 := $(CURDIR)
SDKDIR           ?= $(PROJ_DIR)/../vendor/nrf5-sdk
GNU_INSTALL_ROOT ?= 

PROJECT_NAME     := secure_bootloader_usb_mbr_pca10056
BINNAME          := usb_bootloader
TARGETS          := $(BINNAME)_nrf52840
OUTPUT_DIRECTORY := _build
NRFUTIL          := nrfutil
SOFTDEVICE_HEX   := $(SDKDIR)/components/softdevice/s140/hex/s140_nrf52_7.2.0_softdevice.hex
BOOTLOADER_VERSION := $(shell date +%s)
BOOTLOADER_UART_DEBUG := no

# Source files common to all targets
SRC_FILES += \
  $(SDKDIR)/components/libraries/bootloader/serial_dfu/nrf_dfu_serial_usb.c \
  $(SDKDIR)/components/libraries/bootloader/serial_dfu/nrf_dfu_serial.c \
  $(SDKDIR)/components/libraries/bootloader/ble_dfu/nrf_dfu_ble.c \
  $(PROJ_DIR)/app_usbd_serial_num.c \
  $(PROJ_DIR)/dfu_public_key.c \
  $(PROJ_DIR)/main.c \
  $(SDKDIR)/components/ble/common/ble_srv_common.c \
  $(SDKDIR)/components/boards/boards.c \
  $(SDKDIR)/components/libraries/atomic_fifo/nrf_atfifo.c \
  $(SDKDIR)/components/libraries/atomic/nrf_atomic.c \
  $(SDKDIR)/components/libraries/balloc/nrf_balloc.c \
  $(SDKDIR)/components/libraries/bootloader/dfu/dfu-cc.pb.c \
  $(SDKDIR)/components/libraries/bootloader/dfu/nrf_dfu.c \
  $(SDKDIR)/components/libraries/bootloader/dfu/nrf_dfu_flash.c \
  $(SDKDIR)/components/libraries/bootloader/dfu/nrf_dfu_handling_error.c \
  $(SDKDIR)/components/libraries/bootloader/dfu/nrf_dfu_mbr.c \
  $(SDKDIR)/components/libraries/bootloader/dfu/nrf_dfu_req_handler.c \
  $(SDKDIR)/components/libraries/bootloader/dfu/nrf_dfu_settings.c \
  $(SDKDIR)/components/libraries/bootloader/dfu/nrf_dfu_settings_svci.c \
  $(SDKDIR)/components/libraries/bootloader/dfu/nrf_dfu_svci.c \
  $(SDKDIR)/components/libraries/bootloader/dfu/nrf_dfu_svci_handler.c \
  $(SDKDIR)/components/libraries/bootloader/dfu/nrf_dfu_transport.c \
  $(SDKDIR)/components/libraries/bootloader/dfu/nrf_dfu_utils.c \
  $(SDKDIR)/components/libraries/bootloader/dfu/nrf_dfu_validation.c \
  $(SDKDIR)/components/libraries/bootloader/dfu/nrf_dfu_ver_validation.c \
  $(SDKDIR)/components/libraries/bootloader/nrf_bootloader_app_start.c \
  $(SDKDIR)/components/libraries/bootloader/nrf_bootloader_app_start_final.c \
  $(SDKDIR)/components/libraries/bootloader/nrf_bootloader.c \
  $(SDKDIR)/components/libraries/bootloader/nrf_bootloader_dfu_timers.c \
  $(SDKDIR)/components/libraries/bootloader/nrf_bootloader_fw_activation.c \
  $(SDKDIR)/components/libraries/bootloader/nrf_bootloader_info.c \
  $(SDKDIR)/components/libraries/bootloader/nrf_bootloader_wdt.c \
  $(SDKDIR)/components/libraries/crc32/crc32.c \
  $(SDKDIR)/components/libraries/crypto/backend/cc310_bl/cc310_bl_backend_ecc.c \
  $(SDKDIR)/components/libraries/crypto/backend/cc310_bl/cc310_bl_backend_ecdsa.c \
  $(SDKDIR)/components/libraries/crypto/backend/cc310_bl/cc310_bl_backend_hash.c \
  $(SDKDIR)/components/libraries/crypto/backend/cc310_bl/cc310_bl_backend_init.c \
  $(SDKDIR)/components/libraries/crypto/backend/cc310_bl/cc310_bl_backend_shared.c \
  $(SDKDIR)/components/libraries/crypto/backend/oberon/oberon_backend_chacha_poly_aead.c \
  $(SDKDIR)/components/libraries/crypto/backend/oberon/oberon_backend_ecc.c \
  $(SDKDIR)/components/libraries/crypto/backend/oberon/oberon_backend_ecdh.c \
  $(SDKDIR)/components/libraries/crypto/backend/oberon/oberon_backend_ecdsa.c \
  $(SDKDIR)/components/libraries/crypto/backend/oberon/oberon_backend_eddsa.c \
  $(SDKDIR)/components/libraries/crypto/backend/oberon/oberon_backend_hash.c \
  $(SDKDIR)/components/libraries/crypto/backend/oberon/oberon_backend_hmac.c \
  $(SDKDIR)/components/libraries/crypto/nrf_crypto_ecc.c \
  $(SDKDIR)/components/libraries/crypto/nrf_crypto_ecdsa.c \
  $(SDKDIR)/components/libraries/crypto/nrf_crypto_hash.c \
  $(SDKDIR)/components/libraries/crypto/nrf_crypto_init.c \
  $(SDKDIR)/components/libraries/crypto/nrf_crypto_shared.c \
  $(SDKDIR)/components/libraries/experimental_section_vars/nrf_section_iter.c \
  $(SDKDIR)/components/libraries/fstorage/nrf_fstorage.c \
  $(SDKDIR)/components/libraries/fstorage/nrf_fstorage_nvmc.c \
  $(SDKDIR)/components/libraries/fstorage/nrf_fstorage_sd.c \
  $(SDKDIR)/components/libraries/log/src/nrf_log_frontend.c \
  $(SDKDIR)/components/libraries/log/src/nrf_log_str_formatter.c \
  $(SDKDIR)/components/libraries/mem_manager/mem_manager.c \
  $(SDKDIR)/components/libraries/memobj/nrf_memobj.c \
  $(SDKDIR)/components/libraries/queue/nrf_queue.c \
  $(SDKDIR)/components/libraries/ringbuf/nrf_ringbuf.c \
  $(SDKDIR)/components/libraries/scheduler/app_scheduler.c \
  $(SDKDIR)/components/libraries/slip/slip.c \
  $(SDKDIR)/components/libraries/strerror/nrf_strerror.c \
  $(SDKDIR)/components/libraries/svc/nrf_svc_handler.c \
  $(SDKDIR)/components/libraries/usbd/app_usbd.c \
  $(SDKDIR)/components/libraries/usbd/app_usbd_core.c \
  $(SDKDIR)/components/libraries/usbd/app_usbd_string_desc.c \
  $(SDKDIR)/components/libraries/usbd/class/cdc/acm/app_usbd_cdc_acm.c \
  $(SDKDIR)/components/libraries/util/app_error_weak.c \
  $(SDKDIR)/components/libraries/util/app_util_platform.c \
  $(SDKDIR)/components/libraries/util/nrf_assert.c \
  $(SDKDIR)/components/softdevice/common/nrf_sdh_ble.c \
  $(SDKDIR)/components/softdevice/common/nrf_sdh.c \
  $(SDKDIR)/components/softdevice/common/nrf_sdh_soc.c \
  $(SDKDIR)/external/fprintf/nrf_fprintf.c \
  $(SDKDIR)/external/fprintf/nrf_fprintf_format.c \
  $(SDKDIR)/external/nano-pb/pb_common.c \
  $(SDKDIR)/external/nano-pb/pb_decode.c \
  $(SDKDIR)/external/utf_converter/utf.c \
  $(SDKDIR)/integration/nrfx/legacy/nrf_drv_clock.c \
  $(SDKDIR)/integration/nrfx/legacy/nrf_drv_power.c \
  $(SDKDIR)/modules/nrfx/drivers/src/nrfx_clock.c \
  $(SDKDIR)/modules/nrfx/drivers/src/nrfx_power.c \
  $(SDKDIR)/modules/nrfx/drivers/src/nrfx_systick.c \
  $(SDKDIR)/modules/nrfx/drivers/src/nrfx_usbd.c \
  $(SDKDIR)/modules/nrfx/hal/nrf_nvmc.c \
  $(SDKDIR)/modules/nrfx/mdk/gcc_startup_nrf52840.S \
  $(SDKDIR)/modules/nrfx/mdk/system_nrf52840.c \
  $(SDKDIR)/modules/nrfx/soc/nrfx_atomic.c \


# Include folders common to all targets
INC_FOLDERS += \
  $(PROJ_DIR) \
  $(SDKDIR)/components/ble/common \
  $(SDKDIR)/components/boards \
  $(SDKDIR)/components/libraries/atomic \
  $(SDKDIR)/components/libraries/atomic_fifo \
  $(SDKDIR)/components/libraries/balloc \
  $(SDKDIR)/components/libraries/bootloader \
  $(SDKDIR)/components/libraries/bootloader/ble_dfu \
  $(SDKDIR)/components/libraries/bootloader/dfu \
  $(SDKDIR)/components/libraries/bootloader/serial_dfu \
  $(SDKDIR)/components/libraries/crc32 \
  $(SDKDIR)/components/libraries/crypto \
  $(SDKDIR)/components/libraries/crypto/backend/cc310 \
  $(SDKDIR)/components/libraries/crypto/backend/cc310_bl \
  $(SDKDIR)/components/libraries/crypto/backend/cifra \
  $(SDKDIR)/components/libraries/crypto/backend/mbedtls \
  $(SDKDIR)/components/libraries/crypto/backend/micro_ecc \
  $(SDKDIR)/components/libraries/crypto/backend/nrf_hw \
  $(SDKDIR)/components/libraries/crypto/backend/nrf_sw \
  $(SDKDIR)/components/libraries/crypto/backend/oberon \
  $(SDKDIR)/components/libraries/crypto/backend/optiga \
  $(SDKDIR)/components/libraries/delay \
  $(SDKDIR)/components/libraries/experimental_section_vars \
  $(SDKDIR)/components/libraries/fstorage \
  $(SDKDIR)/components/libraries/log \
  $(SDKDIR)/components/libraries/log/src \
  $(SDKDIR)/components/libraries/mem_manager \
  $(SDKDIR)/components/libraries/memobj \
  $(SDKDIR)/components/libraries/mutex \
  $(SDKDIR)/components/libraries/queue \
  $(SDKDIR)/components/libraries/ringbuf \
  $(SDKDIR)/components/libraries/scheduler \
  $(SDKDIR)/components/libraries/sha256 \
  $(SDKDIR)/components/libraries/slip \
  $(SDKDIR)/components/libraries/stack_info \
  $(SDKDIR)/components/libraries/strerror \
  $(SDKDIR)/components/libraries/svc \
  $(SDKDIR)/components/libraries/usbd \
  $(SDKDIR)/components/libraries/usbd/class/cdc \
  $(SDKDIR)/components/libraries/usbd/class/cdc/acm \
  $(SDKDIR)/components/libraries/util \
  $(SDKDIR)/components/softdevice/common \
  $(SDKDIR)/components/softdevice/s140/headers \
  $(SDKDIR)/components/softdevice/s140/headers/nrf52 \
  $(SDKDIR)/components/toolchain/cmsis/include \
  $(SDKDIR)/external/fprintf \
  $(SDKDIR)/external/nano-pb \
  $(SDKDIR)/external/nrf_cc310_bl/include \
  $(SDKDIR)/external/nrf_cc310/include \
  $(SDKDIR)/external/nrf_oberon \
  $(SDKDIR)/external/nrf_oberon/include \
  $(SDKDIR)/external/utf_converter \
  $(SDKDIR)/integration/nrfx \
  $(SDKDIR)/integration/nrfx/legacy \
  $(SDKDIR)/modules/nrfx \
  $(SDKDIR)/modules/nrfx/drivers/include \
  $(SDKDIR)/modules/nrfx/hal \
  $(SDKDIR)/modules/nrfx/mdk \


# Libraries common to all targets
LIB_FILES += \
  $(SDKDIR)/external/nrf_oberon/lib/cortex-m4/hard-float/liboberon_3.0.8.a \
  $(SDKDIR)/external/nrf_cc310_bl/lib/cortex-m4/hard-float/libnrf_cc310_bl_0.9.13.a \


ifeq ($(BOOTLOADER_UART_DEBUG), yes)
  SRC_FILES += \
    $(SDKDIR)/components/libraries/log/src/nrf_log_default_backends.c \
    $(SDKDIR)/components/libraries/log/src/nrf_log_backend_serial.c \
    $(SDKDIR)/components/libraries/log/src/nrf_log_backend_uart.c \
    $(SDKDIR)/integration/nrfx/legacy/nrf_drv_uart.c \
    $(SDKDIR)/modules/nrfx/drivers/src/nrfx_uarte.c

  CFLAGS += -DBOOTLOADER_UART_DEBUG=1
else
	CFLAGS += -DNRF_DFU_DEBUG_VERSION
endif

# Optimization flags
OPT = -Os -g3
# Uncomment the line below to enable link time optimization
#OPT += -flto

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DBOARD_PCA10056
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -DNRF52840_XXAA
CFLAGS += -DNRF_DFU_SETTINGS_VERSION=2
CFLAGS += -DNRF_DFU_SVCI_ENABLED
CFLAGS += -DNRF_SD_BLE_API_VERSION=7
CFLAGS += -DS140
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DSVC_INTERFACE_CALL_AS_NORMAL_FUNCTION
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall -Werror
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums
CFLAGS += -DUSE_APP_CONFIG

# C++ flags common to all targets
CXXFLAGS += $(OPT)
# Assembler flags common to all targets
ASMFLAGS += -g3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DBOARD_PCA10056
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -DMBR_PRESENT
ASMFLAGS += -DNRF52840_XXAA
ASMFLAGS += -DNRF_DFU_SETTINGS_VERSION=2
ASMFLAGS += -DSVC_INTERFACE_CALL_AS_NORMAL_FUNCTION

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDKDIR)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs

$(BINNAME)_nrf52840: CFLAGS += -D__HEAP_SIZE=0
$(BINNAME)_nrf52840: ASMFLAGS += -D__HEAP_SIZE=0

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm


.SILENT:
.PHONY: default help

# Default target - first one defined
default: $(OUTPUT_DIRECTORY)/sig-$(BINNAME)_nrf52840.bin

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo		"$(BINNAME)_nrf52840"
	@echo		"flash      - flashing binary"
	@echo		"flash_sd   - flashing softdevice"
	@echo		"dfu_update - update device(s) via USB DFU (only on linux)"
	@echo		"keys       - generate keys"
	@echo		"sdk_config - starting external tool for editing sdk_config.h"

TEMPLATE_PATH := $(SDKDIR)/components/toolchain/gcc


# Regenerate the public key from private key.

$(PROJ_DIR)/dfu_public_key.c: keys/priv.pem
	@echo Generating $@
	$(NRFUTIL) keys display --key pk --format code $< --out_file $@

# $1 file under test
define fail_when_file_doesnt_exist
$(if $(wildcard $(1)), \
	$(info '$(1)' exists already. Remove existing and try again.) $(error File '$(1)' already exists!), \
	$(info File '$(1)' not found, creating one.))

endef

keys/priv.pem:
	$(call fail_when_file_doesnt_exist,$@)
	@echo Generating $@
	$(NRFUTIL) keys generate $@

keys: $(PROJ_DIR)/dfu_public_key.c

# Add dependency manually as nrf5-sdk's Makefile fails to add it.
$(OUTPUT_DIRECTORY)/$(TARGETS)/dfu_public_key.c.o: $(PROJ_DIR)/dfu_public_key.c

include $(TEMPLATE_PATH)/Makefile.common

$(OUTPUT_DIRECTORY)/$(BINNAME)_nrf52840.out: \
  LINKER_SCRIPT  := secure_bootloader_gcc_nrf52840.ld

$(OUTPUT_DIRECTORY)/%.dfu $(OUTPUT_DIRECTORY)/%-version.h: $(OUTPUT_DIRECTORY)/%.hex | keys/priv.pem
	@echo Generating $@
	@echo "#define BL_VERSION ($(BOOTLOADER_VERSION))" > $(OUTPUT_DIRECTORY)/$*-version.h
	$(NRFUTIL) pkg generate \
		--bootloader $< \
		--softdevice $(SOFTDEVICE_HEX) \
		--key-file keys/priv.pem \
		--hw-version 52 \
		--bootloader-version $(BOOTLOADER_VERSION) \
		--sd-req 0x0 \
		--sd-id 0x100 \
		$(OUTPUT_DIRECTORY)/$*.dfu

$(foreach target, $(TARGETS), $(call define_target, $(target)))

# Flash the program
flash: default
	@echo Flashing: $(OUTPUT_DIRECTORY)/$(BINNAME)_nrf52840.hex
	nrfjprog -f nrf52 --program $(OUTPUT_DIRECTORY)/$(BINNAME)_nrf52840.hex --sectorerase
	nrfjprog -f nrf52 --reset

# Flash softdevice
flash_sd:
	@echo Flashing: $(SOFTDEVICE_HEX)
	nrfjprog -f nrf52 --program $(SOFTDEVICE_HEX) --sectorerase
	nrfjprog -f nrf52 --reset

erase:
	nrfjprog -f nrf52 --eraseall

dfu_update: $(OUTPUT_DIRECTORY)/$(BINNAME)_nrf52840.dfu
	name=$(shell ../host/scripts/find-dfu) ; \
		echo "Updating $$name via DFU:"; \
		$(NRFUTIL) dfu usb-serial \
			-pkg $< \
			-p "$$name"

$(OUTPUT_DIRECTORY)/sig-$(BINNAME)_%.bin $(OUTPUT_DIRECTORY)/sig-$(BINNAME)_%.dat: $(OUTPUT_DIRECTORY)/$(BINNAME)_%.dfu
	# Softdevice & bootloader are combined into a single image, sd_bl
	unzip -p $< sd_bl.bin > $(OUTPUT_DIRECTORY)/sig-$(BINNAME)_$*.bin
	unzip -p $< sd_bl.dat > $(OUTPUT_DIRECTORY)/sig-$(BINNAME)_$*.dat

SDK_CONFIG_FILE := sdk_config.h
CMSIS_CONFIG_TOOL := $(SDKDIR)/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar
sdk_config:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE)

# In order to delete all the files created when building, the clean target from
# $(TEMPLATE_PATH)/Makefile.common is overriden with this one. Make would
# generate warnings regarding which can be ignored.
clean:
	rm -rf $(OUTPUT_DIRECTORY)
	rm -rf $(PROJ_DIR)/keys/priv.pem $(PROJ_DIR)/dfu_public_key.c

all-targets:
	$(MAKE)

clean-all-targets:
	$(MAKE) clean

.PHONY: flash flash_sd erase dfu_update sdk_config keys clean
