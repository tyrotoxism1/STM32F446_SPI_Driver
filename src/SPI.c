#include "SPI.h"
#include "stm32f446xx.h"
#include <stdint.h>

struct SPI_t{
	uint16_t Tx_buf;
	uint16_t Rx_buf;
	SPI_STATUS status;
	SPI_ERROR_t error;
};

static SPI_t SPI_instance_storage = {0};
SPI_handle spi_instance = &SPI_instance_storage; 

/*
 * Functions/features to add:
 * - SPI_change_mode(), just as an option if user wants to change outside of
 *   init(can't change during communication)
 * - SPI_send_INT(), interrupt version of send, receive and transieve  
 */ 

/**
 * SPI_GPIO_setup() - Configures PB3,4 and 5 for SPI functionality and set
 * 4 pins for chip select
 *
 * SPI1 is setup using PB3(SCK), PB4(MISO) and PB5(MOSI) pins by setting
 * each pin alternate function(AF) to AF5 for SPI1.  
 * PB8,PB9, PB10 and PAA8 are used as software controlled chip select lines
 * (outputs)
 */
void SPI_GPIO_setup()
{
	RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN);
	RCC->APB2ENR |= (RCC_APB2ENR_SPI1EN);
	GPIOB->MODER |= (GPIO_MODER_MODE3_1 | GPIO_MODER_MODE4_1 | GPIO_MODER_MODE5_1);
	GPIOB->AFR[0] |= (GPIO_AFRL_AFRL3_0 | GPIO_AFRL_AFRL3_2);
	GPIOB->AFR[0] |= (GPIO_AFRL_AFRL4_0 | GPIO_AFRL_AFRL4_2);
	GPIOB->AFR[0] |= (GPIO_AFRL_AFRL5_0 | GPIO_AFRL_AFRL5_2);

	GPIOB->MODER |= (GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0 | GPIO_MODER_MODE10_0);
	GPIOA->MODER |= (GPIO_MODER_MODE8_0); 
}

/*
 * SPI_change_mode() - Configures SPI1 clock polarity(CPOL) and clock 
 * phased(CPHA) based on `mode`
 *
 * SPI mode can't be changed when communicating, thus SPI must be disabled to
 * change modes. If called after enabling and using SPI, user must call
 * `SPI_deinit()` before changing modes.   
 *
 * @mode: Determines CPOL and CPHA based on table below
 * | Mode | CPOL | CPHA |
 * | ---- | ---- | ---- |
 * | 0    | 0    | 0    |
 * | 1    | 0    | 1    |
 * | 2    | 1    | 0    |
 * | 3    | 1    | 1    |
 *
 * Return: Success status of changing mode
 * * %0 - SPI enabled caused failure
 * * %1 - Success 
 */
int SPI_change_mode(uint8_t mode)
{
	if(SPI1->CR1 & SPI_CR1_SPE)
		return 0;
	if(mode==1){
		SPI1->CR1 &= ~(SPI_CR1_CPOL);
		SPI1->CR1 |= SPI_CR1_CPHA;
	}
	else if(mode==2){
		SPI1->CR1 |= SPI_CR1_CPOL;
		SPI1->CR1 &= ~(SPI_CR1_CPHA);
	}
	else if(mode==3){
		SPI1->CR1 |= SPI_CR1_CPOL;
		SPI1->CR1 |= SPI_CR1_CPHA;
	}
	else{
		SPI1->CR1 &= ~(SPI_CR1_CPHA);
		SPI1->CR1 &= ~(SPI_CR1_CPOL);
	}
	return 1;
}

/**
 * SPI_CR_setup() - Configures SPI1 control register
 *
 * Configures SPI for baud rate based on param, SPI mode based on param, 
 * bidirectional communication, no CRC, Software Slave Management, 
 * Single Master mode, data frame format and Motorola mode.
 *
 * @transfer_speed: Sets "baud rate control"  bits. Value is currently expected
 * to conform to STM32F446 reference manual values specified. 
 * @mode: Sets SPI CPOL and CPHA bits in CR1 based on table described in
 * `SPI_change_mode()` 
* @dff: data frame format, if dff==0, 8 bit frame, otherwise 16 bit frame.
 */
void SPI_CR_setup(uint8_t transfer_speed, uint8_t mode, uint8_t dff)
{
	SPI1->CR1 &= ~( SPI_CR1_BIDIMODE | 
					SPI_CR1_CRCEN |
				   	SPI_CR1_RXONLY | 
					SPI_CR1_LSBFIRST );
	SPI1->CR1 |= (  SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR );
	SPI_change_mode(mode);
	if(dff)
		SPI1->CR1 |= (SPI_CR1_DFF);
		
	// TODO: User should be able to pass desired frequency and API will adjust
	// to closest achievable frequency
	SPI1->CR1 |= (transfer_speed<<4);
}

/**
 * SPI_init() - Configures MOSI, MISO, SCLK and chip select GPIO, then
 * configures SPI1 control register for user specified SPI mode, number of
 * peripherals, data frame formate of 8 or 16 and SPI SCLK speed(transfer
 * speed)
 *
 * TODO: Enter param info 
 * Clears TXE of SPI_SR by writing to SPI1_DR
 */ 
void SPI_init(uint8_t mode, 
		uint8_t num_peripherals,
	   	uint16_t data_frame_format,
	   	uint8_t transfer_speed
		)
{
	SPI_GPIO_setup();
	SPI_CR_setup(transfer_speed, mode,data_frame_format);
	spi_instance->status = WAITING; 
	spi_instance->error = NO_ERR; 
	SPI1->CR1 |= SPI_CR1_SPE;
	spi_instance->Rx_buf = SPI1->DR;
}


/**
 * SPI_deinit() - Disables all chip select lines, waits for TXE then RXNE bits
 * in CR then ensures SPI BSY flag is cleared before disabling SPI and finally
 * reading last sent data. 
 */
void SPI_deinit()
{

	while( !(SPI1->SR & SPI_SR_TXE) );
	while( (SPI1->SR & SPI_SR_BSY) );
	SPI_chip_deselect();
	SPI1->CR1 &= ~(SPI_CR1_SPE);
	if(SPI1->SR & SPI_SR_RXNE)
		spi_instance->Rx_buf = SPI1->DR;
}

/**
 * SPI_single_send_poll() - poll for empty Tx buffer then send a single frame of data to peripheral
 *
 * Sends a single frame of data. Waits for Tx buffer to be empty, then writes
 * data to SPI1 data register(DR). Data is transferred from Tx to shift 
 * register which is then transmitted to peripheral serially.
 *
 * @data: Value that will be assigned to SPI1 DR. 
 */
void SPI_single_send_poll(uint16_t data)
{
	while( !(SPI1->SR & SPI_SR_TXE) );
	SPI1->DR = data; 
	spi_instance->Tx_buf = data;
}

/**
 * SPI_single_receive_poll() - Polls for Rx buffer being full to store incoming data. 
 *
 * Doesn't send any SPI data out, is just reading data register for SPI. 
 */
uint16_t SPI_single_receive_poll(void)
{
	while( !(SPI1->SR & SPI_SR_RXNE) );
	spi_instance->Rx_buf = SPI1->DR;
	return spi_instance->Rx_buf;
}


void SPI_single_transieve_poll(uint16_t data)
{
	SPI_single_send_poll(data);
	SPI_single_receive_poll();
}

/**
 * SPI_chip_select() - Drive CS pin low for corresponding peripheral device
 *
 * Choose 1 of 4 GPIO pins to select a device for SPI communication. Drives
 * high(deselects) all other GPIO pins.  
 * @peripheral_num: Expects value from 0-3, each corresponding to a GPIO pin
 * for chip select
 * - 0(default) = PB8
 * - 1 = PB9  
 * - 2 = PB10  
 * - 3 = PA8  
 */ 
void SPI_chip_select(uint8_t peripheral_num)
{
	if(peripheral_num==1){
		GPIOB->ODR |= GPIO_ODR_OD8;
		GPIOB->ODR &= ~(GPIO_ODR_OD9);
		GPIOB->ODR |= GPIO_ODR_OD10;
		GPIOA->ODR |= GPIO_ODR_OD8;
	}
	else if(peripheral_num==2){
		GPIOB->ODR |= GPIO_ODR_OD8;
		GPIOB->ODR |= GPIO_ODR_OD9;
		GPIOB->ODR &= ~(GPIO_ODR_OD10);
		GPIOA->ODR |= GPIO_ODR_OD8;
	}
	else if(peripheral_num==3){
		GPIOB->ODR |= GPIO_ODR_OD8;
		GPIOB->ODR |= GPIO_ODR_OD9;
		GPIOB->ODR |= GPIO_ODR_OD10;
		GPIOA->ODR &= ~(GPIO_ODR_OD8);
	}
	else{
		GPIOB->ODR &= ~(GPIO_ODR_OD8);
		GPIOB->ODR |= GPIO_ODR_OD9;
		GPIOB->ODR |= GPIO_ODR_OD10;
		GPIOA->ODR |= GPIO_ODR_OD8;
	}
}
/**
 * SPI_chip_deselect() - Drives all GPIO/chip select lines High to deselect all
 * peripherals
 */

void SPI_chip_deselect()
{
	GPIOB->ODR |= GPIO_ODR_OD8;
	GPIOB->ODR |= GPIO_ODR_OD9;
	GPIOB->ODR |= GPIO_ODR_OD10;
	GPIOA->ODR |= GPIO_ODR_OD8;

}

uint16_t SPI_get_Tx_buf(void)
{
	return spi_instance->Tx_buf;
}	

uint16_t SPI_get_Rx_buf(void)
{
	return spi_instance->Rx_buf;
}

