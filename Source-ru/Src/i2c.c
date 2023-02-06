// 09.04.2021
// This code is from here:
// https://github.com/heavyC1oud/meteoStick
// Article  about this project: https://habr.com/ru/post/486988/

// MCU - STM32L031F6P6
// I2C SDA - PA10
// I2C SCL - PA9
// System Clock - 2.097MHz (MSI)
// I2C clocked from System Clock
// I2C frequency - 100 kHz

// #include "stm32f0xx.h"
#include "stm32l0xx.h"
#include "delay.h"
#include "i2c.h"


/*************************	FUNCTION	******************************/

/**********************************************************************
*	function name	:	initI2C
*	Description		:	I2C initialization
*	Arguments		:	none
*	Return value	:	none
**********************************************************************/
/*
void initI2C(void)
{
	//	I2C1
	//	SCL - PB6
	//	SDA - PB7
	//	I2C frequency 100 kHz
	//	I2C clock Mux - HSI

	//	enable port B bus
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

	//	PORTB6
	//	MODER6[1:0] = 10 - alternate function mode
	GPIOB->MODER &= ~GPIO_MODER_MODER6;
	GPIOB->MODER |= GPIO_MODER_MODER6_1;

	//	AFRL6[3:0] = 0001 - alternate function 1
	GPIOB->AFR[0] &= ~(GPIO_AFRL_AFRL6);
	GPIOB->AFR[0] |= (0x01 << GPIO_AFRL_AFSEL6_Pos);

	//	OTYPER6 = 1 - output open-drain
	GPIOB->OTYPER |= GPIO_OTYPER_OT_6;

	//	OSPEEDR6[1:0] = 11 - High speed
	GPIOB->OSPEEDR &= ~GPIO_OSPEEDR_OSPEEDR6;
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR6_0 | GPIO_OSPEEDR_OSPEEDR6_1;

	//	PUPDR6[1:0] = 01 - pull-up
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPDR6;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR6_0;

	//	PORTB7
	//	MODER7[1:0] = 10 - alternate function mode
	GPIOB->MODER &= ~GPIO_MODER_MODER7;
	GPIOB->MODER |= GPIO_MODER_MODER7_1;

	//	AFRL7[3:0] = 0001 - alternate function 1
	GPIOB->AFR[0] &= ~(GPIO_AFRL_AFRL7);
	GPIOB->AFR[0] |= (0x01 << GPIO_AFRL_AFSEL7_Pos);

	//	OTYPER7 = 1 - output open-drain
	GPIOB->OTYPER |= GPIO_OTYPER_OT_7;

	//	OSPEEDR7[1:0] = 11 - High speed
	GPIOB->OSPEEDR &= ~GPIO_OSPEEDR_OSPEEDR7;
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR7_0 | GPIO_OSPEEDR_OSPEEDR7_1;

	//	PUPDR7[1:0] = 01 - pull-up
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPDR7;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR7_0;

	//	enable I2C clock
	RCC->CFGR3 &= ~RCC_CFGR3_I2C1SW;
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

	//	disable Own address 1
	I2C1->OAR1 &= ~I2C_OAR1_OA1EN;

	//	disable Own address 2
	I2C1->OAR2 &= ~I2C_OAR2_OA2EN;

	//	disable general call
	I2C1->CR1 &= ~I2C_CR1_GCEN;

	//	clear NOSTRETCH bit for master mode
	I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;

	//	disable peripheral for settings
	I2C1->CR1 &= ~I2C_CR1_PE;

	//	disable analog filter
	I2C1->CR1 &= ~I2C_CR1_ANFOFF;

	//	disable digital filter
	I2C1->CR1 &= ~I2C_CR1_DNF;

	//	set timings (calculate in CubeMX)
	I2C1->TIMINGR = 0x2000090E;

	//	enable peripheral
	I2C1->CR1 |= I2C_CR1_PE;

	//	disable SMBus Host address
	I2C1->CR1 &= ~I2C_CR1_SMBHEN;

	//	disable SMBus device default address
	I2C1->CR1 &= ~I2C_CR1_SMBDEN;

	//	enable autoend mode
	I2C1->CR2 |= I2C_CR2_AUTOEND;
}
*/
/*********************************************************************/


// my code:
// Initialize the I2C1 interface for master mode.
void initI2C(void)
{
	// Turn on the clock for GPIOA and I2C
	RCC->IOPENR |= RCC_IOPENR_IOPAEN;	/* Enable clock for GPIO Port A */
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;	/* Enable clock for I2C */
	__NOP();
	__NOP();

	// PA9 set alternate function as I2C1_SCL
	GPIOA->MODER &= ~GPIO_MODER_MODE9;	 /* clear mode for PA9 */
	GPIOA->MODER |= GPIO_MODER_MODE9_1;  /* enable alternate functions */
	GPIOA->OTYPER |= GPIO_OTYPER_OT_9;   /* open drain */
	GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL9;         // Clear Alternate Function PA9
	GPIOA->AFR[1] |= 1 << GPIO_AFRH_AFSEL9_Pos; // 0001=AF1 - I2C Alternate Function for PA9
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEED9_Msk; // OSPEED9[1:0] = 00 - low speed
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEED9_1;    // OSPEED9[1:0] = 10 - high speed
	// PA9 Pull-Up - 19.02.2022
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD9;  // Pull-up reset to 00
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD9_0; // PUPD9[1:0] = 01 - pull-up

	// PA10 set alternate function as I2C1_SDA
	GPIOA->MODER &= ~GPIO_MODER_MODE10;	 /* clear mode for PA10 */
	GPIOA->MODER |= GPIO_MODER_MODE10_1; /* enable alternate functions */
	GPIOA->OTYPER |= GPIO_OTYPER_OT_10;  /* open drain */
	GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL10;         // Clear Alternate Function PA10
	GPIOA->AFR[1] |= 1 << GPIO_AFRH_AFSEL10_Pos; // 0001=AF1 - I2C Alternate Function for PA10
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEED10_Msk; // OSPEED10[1:0] = 00 - low speed
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEED10_1;    // OSPEED10[1:0] = 10 - high speed
	// PA10 Pull-Up - 19.02.2022
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD10;  // Pull-up reset to 00
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD10_0; // PUPD10[1:0] = 01 - pull-up

	// I2C1 clock selection
	RCC->CCIPR &= ~RCC_CCIPR_I2C1SEL;  /* write 00 to the I2C clk selection register */
	RCC->CCIPR |= RCC_CCIPR_I2C1SEL_0; /* 01 - system clock selected as I2C1 clock */

	I2C1->CR1 &= ~I2C_CR1_PE; // disable I2C

	// I2C - Configure filter - leave at defaults

	// I2C - Configure timing
	// 100kHz with I2CCLK = 2097kHz
	I2C1->TIMINGR = (uint32_t)0x00000609;

	I2C1->CR2 |= I2C_CR2_AUTOEND; // enable autoend mode
	I2C1->CR1 |= I2C_CR1_PE;      // enable I2C1

}


/**********************************************************************
*	function name	:	I2CWrite
*	Description		:	write bytes using I2C interface
*	Arguments		:	devID - slave address
*					:	addr - start address to write
*					:	data - data to write
*					:	amount of data to write
*	Return value	:	error info
**********************************************************************/
int8_t I2CWrite(uint8_t devID, uint8_t addr, uint8_t* data, uint16_t count)
{
	//	set slave address
	I2C1->CR2 &= ~I2C_CR2_SADD;
	I2C1->CR2 |=  devID << I2C_CR2_SADD_Pos;

	//	set number bytes to send
	I2C1->CR2 &= ~I2C_CR2_NBYTES;
	I2C1->CR2 |= (count + sizeof(addr)) << I2C_CR2_NBYTES_Pos;

	//	set transfer direction to write
	I2C1->CR2 &= ~I2C_CR2_RD_WRN;

	//	generate START condition
	I2C1->CR2 |= I2C_CR2_START;

	//	send address
	I2C1->TXDR = addr;

	if(count == 0) {
		return 0;
	}

	while((!(I2C1->ISR & I2C_ISR_TXE)) && (!(I2C1->ISR & I2C_ISR_NACKF))) {

		if(I2C1->ISR & I2C_ISR_NACKF) {
			I2C1->ICR |= I2C_ICR_NACKCF;
			return 1;
		}
	}

	//	transmit Data
	for(uint16_t i = 0; i < count; i++)
	{
		//	write data
		I2C1->TXDR = data[i];

		if(i != (count - 1)) {
			while((!(I2C1->ISR & I2C_ISR_TXE)) && (!(I2C1->ISR & I2C_ISR_NACKF))) {

				if(I2C1->ISR & I2C_ISR_NACKF) {
					I2C1->ICR |= I2C_ICR_NACKCF;
					return 1;
				}
			}
		}
	}

	delay(10);

	return 0;
}
/*********************************************************************/


/**********************************************************************
*	function name	:	I2CRead
*	Description		:	read bytes using I2C interface
*	Arguments		:	devID - slave address
*					:	addr - start address to read
*					:	data - data to read
*					:	amount of data to read
*	Return value	:	error info
**********************************************************************/
int8_t I2CRead(uint8_t devID, uint8_t addr, uint8_t* data, uint16_t count)
{
	//	set slave address
	I2C1->CR2 &= ~I2C_CR2_SADD;
	I2C1->CR2 |=  devID << I2C_CR2_SADD_Pos;

	//	set number bytes to send
	I2C1->CR2 &= ~I2C_CR2_NBYTES;
	I2C1->CR2 |= (sizeof(addr)) << I2C_CR2_NBYTES_Pos;

	//	set transfer direction to write
	I2C1->CR2 &= ~I2C_CR2_RD_WRN;

	//	generate START condition
	I2C1->CR2 |= I2C_CR2_START;

	//	send address
	I2C1->TXDR = addr;

	while((!(I2C1->ISR & I2C_ISR_TXE)) && (!(I2C1->ISR & I2C_ISR_NACKF))) {
		if(I2C1->ISR & I2C_ISR_NACKF) {
			I2C1->ICR |= I2C_ICR_NACKCF; // 19.02.2022 try
			return 1;
		}
	}

	//	set number bytes to send
	I2C1->CR2 &= ~I2C_CR2_NBYTES;
	I2C1->CR2 |= count << I2C_CR2_NBYTES_Pos;

	//	set transfer direction to read
	I2C1->CR2 |= I2C_CR2_RD_WRN;

	//	generate START condition
	I2C1->CR2 |= I2C_CR2_START;

	//	receive Data
	for(uint16_t i = 0; i < count; i++)
	{
		//	wait RXNE flag Is Set
		while(!(I2C1->ISR & I2C_ISR_RXNE));

		//	read data
		data[i] = (uint8_t)I2C1->RXDR;
	}

	delay(10); // 20.04.2021 add this delay
	return 0;
}
/*********************************************************************/
