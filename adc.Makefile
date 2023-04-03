include stm32_drv/Makefile.defs

TARGET = usart_adc

LDFLAGS += -L$(OBJ_DIR) -lprintf

CSRCS += \
    $(DRV)/drv/usart_tx.c \
    $(DRV)/drv/util.c \
    adc.c

include stm32_drv/Makefile.rules
