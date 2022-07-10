include stm32_drv/Makefile.defs

TARGET = usart_rx
CSRCS += \
    $(DRV)/drv/usart_rx.c \
    $(DRV)/drv/usart_tx.c \
    usart_rx.c

include stm32_drv/Makefile.rules
