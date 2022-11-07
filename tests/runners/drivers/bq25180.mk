# SPDX-License-Identifier: MIT

COMPONENT_NAME = bq25180

SRC_FILES = \
	../drivers/pmu/bq25180.c \

TEST_SRC_FILES = \
	src/drivers/pmu/bq25180_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../drivers/include \
	../external/libmcu/modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include runner.mk
