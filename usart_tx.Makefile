include stm32_drv/Makefile.defs

TARGET = usart_tx
CSRCS += \
    $(DRV)/drv/usart_tx.c \
    usart_tx.c

include stm32_drv/Makefile.rules
