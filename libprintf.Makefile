OBJ_DIR = obj.a

include stm32_drv/Makefile.defs

TARGET = libprintf.a

CFLAGS := $(filter-out -O0, $(CFLAGS))
CFLAGS := $(filter-out -O1, $(CFLAGS))
CFLAGS := $(filter-out -O2, $(CFLAGS))
CFLAGS := $(filter-out -O3, $(CFLAGS))

CFLAGS += \
	-DPRINTF_DISABLE_SUPPORT_EXPONENTIAL \
	-DPRINTF_DISABLE_SUPPORT_FLOAT \
	-DPRINTF_DISABLE_SUPPORT_LONG_LONG \
	-Iprintf \
	-Os

ASMSRCS =

CSRCS = \
	printf/printf.c

include stm32_drv/Makefile.a.rules
