#include "SPI.h"
#include "uart.h"
#include "printf.h"
#include <stdint.h>


/**
 * MFRC522_addr_helper() - translates register address to read address
 *
 * When reading from MFRC522 via SPI, the MSB defines the r/w mode and the LSB
 * is reserved. The register addresses only go up to 3Fh leaving 2 bits of room
 * for mode and reserved bit. Function shifts `addr` right one, then writes MSB
 * based on `rw_mode`.
 *
 * @addr: Desired MFRC522 register address 
 * @rw_mode: Value of 0 means read mode, otherwise write mode
 *
 * Return: Value to send over SPI to read desired base `addr` 
 */
uint8_t MFRC522_addr_helper(uint8_t addr, uint8_t rw_mode)
{
	addr = (addr<<1);
	if(rw_mode==0)
		addr |= (0b10000000);
	else
		addr &= ~(0b10000000);
	return addr;
}

int main(void){
	UART_config();
	uint8_t received_val = 0;
	//dff of 0 means 8 bit frame, transfer speed of 0 means peripheral clock/2  uint8_t received_val = get_SPI1_DR();
	SPI_init(0, 1,0 ,0);

	received_val = SPI_get_Rx_buf();	
	SPI_chip_select(0);
	//Original request for addr 0x01, after next transfer of Rx data should 
	//yield 0x20h
	SPI_single_send_poll(MFRC522_addr_helper(0x01, 0));
	//After this send, 0x20h should be in SPI1_DR,
	//now requesting add 0x11 
	received_val = SPI_single_receive_poll();
	SPI_single_send_poll(MFRC522_addr_helper(0xFF, 0));
	received_val = SPI_single_receive_poll();
	printf("\nExpecting 0x80, Actual: %X\n",received_val);

	SPI_deinit();
	SPI_chip_deselect();

	while(1);

	return 0;
}
