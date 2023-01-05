# SPDX-License-Identifier: Apache-2.0

include projects/sources.mk

PWIFI_ROOT := $(BASEDIR)/external/pwifi
include $(PWIFI_ROOT)/sources.mk
SRCS += $(PWIFI_SRCS)
INCS += $(PWIFI_INCS)

PBLE_ROOT := $(BASEDIR)/external/pble
include $(PBLE_ROOT)/sources.mk
SRCS += $(PBLE_SRCS)
INCS += $(PBLE_INCS)
