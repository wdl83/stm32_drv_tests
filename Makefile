install:
	make -f libprintf.Makefile install
	make -f adc.Makefile install
	make -f modbus.Makefile install
	make -f stk.Makefile install
	make -f tim2.Makefile install
	make -f usart_async_rx.Makefile install
	make -f usart_async_rx_dma.Makefile install
	make -f usart_async_tx.Makefile install
	make -f usart_async_tx_dma.Makefile install
	make -f usart_rx.Makefile install
	make -f usart_tx.Makefile install
clean:
	make -f libprintf.Makefile clean
	make -f adc.Makefile clean
	make -f modbus.Makefile clean
	make -f stk.Makefile clean
	make -f tim2.Makefile clean
	make -f usart_async_rx.Makefile clean
	make -f usart_async_rx_dma.Makefile clean
	make -f usart_async_tx.Makefile clean
	make -f usart_async_tx_dma.Makefile clean
	make -f usart_rx.Makefile clean
	make -f usart_tx.Makefile clean
