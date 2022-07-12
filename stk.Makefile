include stm32_drv/Makefile.defs

TARGET = stk
CSRCS += \
	libc.c \
    $(DRV)/drv/usart_async_tx.c \
    stk.c

include stm32_drv/Makefile.rules
