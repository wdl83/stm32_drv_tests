include stm32_drv/Makefile.defs

TARGET = usart_adc

LDFLAGS += -L$(DST_DIR)/lib -lprintf

CSRCS += \
    $(DRV)/drv/usart_tx.c \
    $(DRV)/drv/util.c \
    adc.c

include stm32_drv/Makefile.rules
