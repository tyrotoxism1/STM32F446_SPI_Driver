#include "SPI.h"
#include "uart.h"
#include "printf.h"
#include <stdint.h>



int main(void){
	uint16_t received_val = 0;
	UART_config();
	//dff of 0 means 8 bit frame, transfer speed of 0 means peripheral clock/2  
	printf("Testing %i\n", received_val);
	SPI_init(0, 1,0 ,0);
	SPI_chip_select(0);
	SPI_single_send_poll(0xA2);
	SPI_single_receive_poll();
	SPI_chip_deselect();
	received_val = SPI_get_Rx_buf();
	printf("value: %X\n",received_val);
	while(1);

	return 0;
}
