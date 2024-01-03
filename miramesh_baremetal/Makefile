PROJECT_NAME=network_receiver

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

TARGET=nrf52840ble-net

# Set to nrf5-sdk, to have access to hardware abstraction
NRF5_SDK?=../vendor/nrf5-sdk
MIRA_LIB?=../..

SOURCES=\
	main.c \
	startup.c

LD_SCRIPT=ld-$(TARGET).ld

# To enable depreacted mira API, uncomment this line
#CFLAGS+=-DMIRA_DEPRECATED

ifeq ($(TARGET),nrf52832ble-net)
SDK_TARGET=nrf52
CFLAGS+= \
	-DBOARD_PCA10040 \
	-DNRF52 \
	-DNRF52832_XXAA
INCLUDE_DIRS+=\
	$(NRF5_SDK)/components/softdevice/s132/headers \
	s132
else
ifeq ($(TARGET),nrf52833ble-net)
SDK_TARGET=nrf52833
CFLAGS+= \
	-DBOARD_PCA10100 \
	-DNRF52833_XXAA
INCLUDE_DIRS+=\
	$(NRF5_SDK)/components/softdevice/s140/headers \
	s140
else
ifeq ($(TARGET),nrf52840ble-net)
SDK_TARGET=nrf52840
CFLAGS+= \
	-DBOARD_PCA10056 \
	-DNRF52840_XXAA
INCLUDE_DIRS+=\
	$(NRF5_SDK)/components/softdevice/s140/headers \
	s140
else
$(error Only the targets nrf52832ble-net, nrf52833ble-net and nrf52840ble-net are supported by this example)
endif
endif
endif

ifeq ($(V),1)
  TRACE_CC       =
  TRACE_LD       =
  TRACE_OBJCOPY  =
  Q=
else
  TRACE_CC       = @echo "  CC       " $<
  TRACE_LD       = @echo "  LD       " $@
  TRACE_OBJCOPY  = @echo "  OBJCOPY  " $@
  Q=@
endif

SDK_SOURCES=\
	$(NRF5_SDK)/components/boards/boards.c \
	$(NRF5_SDK)/modules/nrfx/mdk/gcc_startup_$(SDK_TARGET).S \
	$(NRF5_SDK)/modules/nrfx/mdk/system_$(SDK_TARGET).c
	
INCLUDE_DIRS+=\
	$(NRF5_SDK)/integration/nrfx \
	$(NRF5_SDK)/modules/nrfx \
	$(NRF5_SDK)/components/libraries/util \
	$(NRF5_SDK)/components/boards \
	$(NRF5_SDK)/modules/nrfx/mdk \
	$(NRF5_SDK)/modules/nrfx/hal \
	$(NRF5_SDK)/components/toolchain/cmsis/include \
	$(MIRA_LIB)/include

LDSCRIPT_DIR=$(NRF5_SDK)/modules/nrfx/mdk

OBJECTS=\
	$(patsubst %.c,obj_$(TARGET)/%.o,$(SOURCES)) \
	$(patsubst $(NRF5_SDK)/%.c,obj_$(TARGET)/sdk/%.o,$(SDK_SOURCES:.S=.c))
	
STATIC_LIBS=\
	$(MIRA_LIB)/platform/$(TARGET)/libmira.a

COMMON_FLAGS=\
	-mcpu=cortex-m4 \
	-mlittle-endian \
	-mthumb \
	-mabi=aapcs \
	-mfloat-abi=hard -mfpu=fpv4-sp-d16 \
	-Wall \
	-Werror \
	-g3

CFLAGS+=\
	$(COMMON_FLAGS) \
	-DCONFIG_GPIO_AS_PINRESET \
	-DSTARTUP_CONFIG \
	-D__STARTUP_CLEAR_BSS \
	-D__HEAP_SIZE=0 \
	-ffunction-sections \
	-fdata-sections \
	-fno-strict-aliasing \
	-fno-builtin \
	-fshort-enums \
	-std=gnu99 \
	$(addprefix -I,$(INCLUDE_DIRS))

LDFLAGS=\
	$(COMMON_FLAGS) \
	-Wl,-L$(LDSCRIPT_DIR) \
	-Wl,--gc-sections \
	-T$(LD_SCRIPT) \
	-nostartfiles

all: $(PROJECT_NAME)-$(TARGET).hex

obj_$(TARGET)/%.o: %.c
	@mkdir -p $(@D)
	$(TRACE_CC)
	$(Q)$(CC) $(CFLAGS) $^ -c -o $@

obj_$(TARGET)/sdk/%.o: $(NRF5_SDK)/%.c
	@mkdir -p $(@D)
	$(TRACE_CC)
	$(Q)$(CC) $(CFLAGS) $^ -c -o $@

obj_$(TARGET)/sdk/%.o: $(NRF5_SDK)/%.S
	@mkdir -p $(@D)
	$(TRACE_CC)
	$(Q)$(CC) $(CFLAGS) $^ -c -o $@

$(PROJECT_NAME)-$(TARGET).elf: $(OBJECTS) $(STATIC_LIBS) $(LDSCRIPT_DIR)/$(LDSCRIPT)
	$(TRACE_LD)
	$(Q)$(CC) $(LDFLAGS) $(OBJECTS) $(STATIC_LIBS) -o $@

$(PROJECT_NAME)-$(TARGET).hex: $(PROJECT_NAME)-$(TARGET).elf
	$(TRACE_OBJCOPY)
	$(Q)$(OBJCOPY) -O ihex $< $@

flash: $(PROJECT_NAME)-$(TARGET).hex
	nrfjprog -f nrf52 --sectorerase --program $< --verify $<
	nrfjprog -f nrf52 --reset

clean:
	rm -f $(PROJECT_NAME)-*.elf $(PROJECT_NAME)-*.hex
	rm -rf obj_*

all-targets:
	$(MAKE) all

clean-all-targets:
	$(MAKE) clean

.PHONY: clean all flash
