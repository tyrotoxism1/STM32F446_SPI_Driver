#include "SPI.h"
#include "stm32f446xx.h"
#include "stm32f4xx.h"
#include <stdint.h>

/*
 * Functions/features to add:
 * - SPI_change_mode(), just as an option if user wants to change outside of
 *   init(can't change during communication)
 */ 

/**
 * SPI_GPIO_setup() - Configures PB3,4 and 5 for SPI functionality and set
 * 4 pins for chip select
 *
 * SPI1 is setup using PB3(SCK), PB4(MISO) and PB5(MOSI) pins by setting
 * each pin alternate function(AF) to AF5 for SPI1.  
 * PB8,PB9, PB10 and PBA8 are used as software controlled chip select lines
 * (outputs)
 */
void SPI_GPIO_setup()
{
	GPIOB->MODER |= (GPIO_MODER_MODE3_1 & GPIO_MODER_MODE4_1 & GPIO_MODER_MODE5_1);
	GPIOB->AFR[3] |= (GPIO_AFRL_AFRL3_0 & GPIO_AFRL_AFRL3_2);
	GPIOB->AFR[4] |= (GPIO_AFRL_AFRL4_0 & GPIO_AFRL_AFRL4_2);
	GPIOB->AFR[5] |= (GPIO_AFRL_AFRL5_0 & GPIO_AFRL_AFRL5_2);

	GPIOB->MODER |= (GPIO_MODER_MODE8_0 & GPIO_MODER_MODE9_0 & GPIO_MODER_MODE10_0);
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
	SPI1->CR1 |= (  SPI_CR1_SSM |
					SPI_CR1_MSTR );
	SPI_change_mode(mode);
	if(dff)
		SPI1->CR1 |= (SPI_CR1_DFF);
		
	// TODO: User should be able to pass desired frequency and API will adjust
	// to closest achievable frequency
	SPI1->CR1 |= (transfer_speed<<4);
}

void SPI_init(uint8_t mode, 
		uint8_t num_peripherals,
	   	uint16_t data_frame_format,
	   	uint8_t transfer_speed
		)
{
	SPI_GPIO_setup();
	SPI_CR_setup(transfer_speed, mode,data_frame_format);
}

