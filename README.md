# STM32F446_SPI_Driver
- Simple bare-metal SPI driver written for STM32F446RE Nucleo board
- Supports single host full-duplex synchronous SPI communication with 1 to 5 peripherals

## Features 
- Uses SPI1 of STM32F446RE Nucleo board
- Supports 8-bit or 16-bit data frames for SPI communication
- Only supports up to 4 peripherals for simplicity and to limit GPIO pin usage
    - Chip select/peripheral control lines can be:
        - PB8, PB9, PA8 or PB3 
- Supports SPI modes 0-3 shown in the table below

| Mode | CPOL | CPHA |
| ---- | ---- | ---- |
| 0    | 0    | 0    |
| 1    | 0    | 1    |
| 2    | 1    | 0    |
| 3    | 1    | 1    |

- Adjustable SPI clock speed based on peripheral clock APB1 (for SPI1)

# SPI API 
## SPI_Init()
- Sets up GPIO for SPI1 and necessary chip select lines if multiple peripherals
  are required
- Configures SPI1_CR1 and SPI1_CR2 
- Determines peripheral clock speed and adjust peripheral clock to get close to
  desired communication speed
    - SPI "baud rate" can only be adjusted in powers of 2, so if desired
      communication speed is 10MHz, with peripheral clock speed of 16MHz, the
closest is 8MHz 

## SPI_send()
- Wait for T1E (transmit empty)
- Then write to SPI1 DR (data register) 

## SPI_receive()
- Assign dummy value to SPI1 DR
- Wait for R1NE (receive not empty)

# Usage notes 
- Requires STM32CubeF4 firmware package to use CMSIS library definitions
    - Can install from [website](https://www.st.com/en/embedded-software/stm32cubef4.html) or [Github](https://github.com/STMicroelectronics/STM32CubeF4)
    - Adjust Makefile `STM32F4_LIB_DIR` variable to match root directory of
      STM32CubeF4 firmware package.


# Definitions
- CPOL(clock polarity): The idle state of the clock, where CPOL=0 means clock idles low and CPOL=1
  means clock idles high. 
- CPHA(clock phase): Determines when data is sampled for MISO/MOSI lines based on clock
  phase (lead or tail of clock).
    - If CPHA=0(lead) and CPOL is 0, data is sampled on the rising edge of
      clock
    - If CPHA=0(lead) and CPOL is 1, data is sampled on the falling edge of
      clock
    - If CPHA=1(tail) and CPOL is 0, data is sampled on the falling edge of
      clock
    - If CPHA=1(tail) and CPOL is 1, data is sampled on the rising edge of
      clock
    - I would suggest watching the [DigKey video on SPI](https://www.youtube.com/watch?v=eFKeNPJq50g&t=895s) for more info
