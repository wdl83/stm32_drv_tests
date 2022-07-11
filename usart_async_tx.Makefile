include stm32_drv/Makefile.defs

TARGET = usart_async_tx
CSRCS += \
    $(DRV)/drv/usart_async_tx.c \
    usart_async_tx.c

include stm32_drv/Makefile.rules
