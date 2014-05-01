all:
	g++ read_packet.cpp imaxb6_packet.cpp -o read_packet && ./read_packet < /dev/serial/by-id/usb-FTDI_FT232R_USB_UART_AH010JNF-if00-port0	
