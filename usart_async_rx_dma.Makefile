include stm32_drv/Makefile.defs

TARGET = usart_async_rx_dma

CFLAGS += -Iprintf
LDFLAGS += -L$(DST_DIR)/lib -lprintf

CSRCS += \
    usart_async_rx_dma.c

include stm32_drv/Makefile.rules
