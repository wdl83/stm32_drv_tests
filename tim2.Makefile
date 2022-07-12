include stm32_drv/Makefile.defs

TARGET = tim2
CSRCS += \
	libc.c \
    $(DRV)/drv/usart_async_tx.c \
    tim2.c

include stm32_drv/Makefile.rules
