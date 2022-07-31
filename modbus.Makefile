include stm32_drv/Makefile.defs

MODBUS = modbus_c

TARGET = modbus

CFLAGS += \
    -DRTU_ADDR=0x80 \
    -DRTU_ADDR_BASE=0x1000 \
    -DTLOG_SIZE=1024 \
    -I.

CSRCS += \
    $(DRV)/drv/tlog.c \
    $(DRV)/drv/usart_async_rx.c \
    $(DRV)/drv/usart_async_tx.c \
    $(DRV)/drv/util.c \
    $(MODBUS)/rtu.c \
    $(MODBUS)/rtu_memory.c \
    $(MODBUS)/stm32f103c8/crc.c \
    $(MODBUS)/stm32f103c8/rtu_impl.c \
    libc.c \
    modbus.c

include stm32_drv/Makefile.rules
