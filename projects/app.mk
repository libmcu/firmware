# SPDX-License-Identifier: Apache-2.0

include projects/version.mk

LIBMCU_ROOT ?= $(BASEDIR)/external/libmcu
LIBMCU_MODULES := cli
include $(LIBMCU_ROOT)/projects/modules.mk

fpl-src-dirs := src common stubs
FPL_SRCS = $(foreach dir, $(addprefix $(BASEDIR)/, $(fpl-src-dirs)), \
	$(shell find $(dir) -type f -regex ".*\.c"))

APP_SRCS = \
	$(FPL_SRCS) \
	$(LIBMCU_MODULES_SRCS) \

APP_INCS = \
	$(BASEDIR)/include \
	$(LIBMCU_MODULES_INCS) \

APP_DEFS = \
	$(BOARD) \
	BUILD_DATE=$(BUILD_DATE) \
	VERSION_TAG=$(VERSION_TAG) \
	VERSION=$(VERSION)
