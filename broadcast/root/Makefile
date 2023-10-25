PROJECT_NAME = broadcast_root

TARGET ?= nrf52840ble-os
LIBDIR ?= $(CURDIR)/../../vendor/libmira
MTKDIR ?= $(CURDIR)/../../vendor/mira-toolkit

ifeq ($(wildcard $(LIBDIR)),)
$(error vendor/libmira is not found, have you downloaded and extracted libmira-V.tar.gz? See README.md for more information)
endif

ifeq ($(wildcard $(MTKDIR)),)
$(error vendor/mira-toolkit is not found, have you checked out or downloaded mira-toolkit? See README.md for more information)
endif

SOURCE_FILES = \
	$(MTKDIR)/mtk_broadcast/mtk_broadcast_worker.c \
	$(MTKDIR)/mtk_broadcast/mtk_broadcast.c \
	$(MTKDIR)/mtk_broadcast/mtk_trickle_timer.c \
	root_app.c \

include $(LIBDIR)/Makefile.include
