#ifndef STM32F4_SPI_H
#define STM32F4_SPI_H
#include <stdint.h>

//opaque pointer
typedef struct SPI_t SPI_t;
typedef SPI_t* SPI_handle;

typedef enum SPI_STATUS_enum{
	WAITING,
	BUSY,
	SPI_ERROR,
} SPI_STATUS;

typedef enum SPI_ERROR_enum{
	NO_ERR,
	SEND_ERR,
	RECEIVE_ERR,
	TIMEOUT,
} SPI_ERROR_t;

void SPI_GPIO_setup(void);
int SPI_change_mode(uint8_t mode);
void SPI_CR_setup(uint8_t transfer_speed, uint8_t mode, uint8_t dff);
void SPI_init(uint8_t mode, uint8_t num_peripherals, 
			  uint16_t data_frame_format, uint8_t transfer_speed);
void SPI_deinit();
void SPI_single_send_poll(uint16_t data);
uint16_t SPI_single_receive_poll(void);
void SPI_single_transieve_poll(uint16_t data);
void SPI_chip_select(uint8_t peripheral_num);
void SPI_chip_deselect();

uint16_t SPI_get_Rx_buf(void);
uint16_t SPI_get_Tx_buf(void);

#endif //STM32F4_SPI_H
