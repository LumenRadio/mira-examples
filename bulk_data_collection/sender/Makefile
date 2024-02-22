PROJECT_NAME = bulk_data_collection_sender

TARGET ?= nrf52840ble-os
LIBDIR ?= $(CURDIR)/../../..
MTKDIR ?= $(CURDIR)/../../../mira-toolkit

ifeq ($(wildcard $(LIBDIR)),)
$(error vendor/libmira is not found, have you downloaded and extracted libmira-V.tar.gz? See README.md for more information)
endif

ifeq ($(wildcard $(MTKDIR)),)
$(error vendor/mira-toolkit is not found, have you checked out or downloaded mira-toolkit? See README.md for more information)
endif

CFLAGS += -I$(MTKDIR)/mtk_bulk_data_collection
CFLAGS += -DMTK_PLATFORM=0
SOURCE_FILES = \
	$(MTKDIR)/mtk_bulk_data_collection/mtk_bulk_data_collection.c \
	$(MTKDIR)/mtk_bulk_data_collection/mtk_bdc_request.c \
	$(MTKDIR)/mtk_bulk_data_collection/mtk_bdc_signal.c \
	$(MTKDIR)/mtk_bulk_data_collection/mtk_bdc_subpacket.c \
	bulk_data_collection_sender.c \

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
