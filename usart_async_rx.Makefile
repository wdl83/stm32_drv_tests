include stm32_drv/Makefile.defs

TARGET = usart_async_rx
CSRCS += \
	libc.c \
    $(DRV)/drv/usart_async_rx.c \
    $(DRV)/drv/usart_async_tx.c \
    usart_async_rx.c

include stm32_drv/Makefile.rules
