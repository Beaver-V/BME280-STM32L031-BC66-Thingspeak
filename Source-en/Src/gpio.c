#include "stm32l031xx.h"
#include "gpio.h"
#include "delay.h"

void initGPIOA(void)
{

	// Turn on the clock for GPIOA
	RCC->IOPENR |= RCC_IOPENR_IOPAEN;			/* Enable clock for GPIO Port A */
	__NOP();
	__NOP();

	// PA4 Output for the LED
	GPIOA->MODER &= ~GPIO_MODER_MODE4;			/* clear mode for PA4 */
	GPIOA->MODER |= GPIO_MODER_MODE4_0;			/* Output mode for PA4 */
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_4;			/* no Push/Pull for PA4 */
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEED4;	/* low speed for PA4 */
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD4;			/* no pullup/pulldown for PA4 */
	GPIOA->BSRR = GPIO_BSRR_BR_4;				/* atomic clr PA4 */

	// PA1 Output for drive BC66_ON (inverted PWRKEY) pin of the BC66
	// BC66 is turned on by drive PWRKEY pin low, so BC66_ON need to drive high
	GPIOA->MODER &= ~GPIO_MODER_MODE1;			/* clear mode for PA1 */
	GPIOA->MODER |= GPIO_MODER_MODE1_0;			/* Output mode for PA1 */
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_1;			/* no Push/Pull for PA1 */
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEED1;	/* low speed for PA1 */
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD1;			/* no pullup/pulldown for PA1 */
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD1_1;			// PUPD1[1:0] = 10 - pull-down
	GPIOA->BSRR = GPIO_BSRR_BR_1;				/* atomic clr PA1 */

	// PA0 Output for drive BC66_PSM_EINT pin of the BC66
	// BC66 can wake-up from PSM by a falling edge on PSM_EINT input pin.
	GPIOA->MODER &= ~GPIO_MODER_MODE0;			/* clear mode for PA0 */
	GPIOA->MODER |= GPIO_MODER_MODE0_0;			/* Output mode for PA0 */
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_0;			/* no Push/Pull for PA0 */
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEED0;	/* low speed for PA0 */
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD0;			// Pull-up reset to 00, so no pullup/pulldown for PA0
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD0_0;			// PUPD0[1:0] = 01 - pull-up
	GPIOA->BSRR = GPIO_BSRR_BS_0;				/* atomic set PA0 */

	// PA5 - input for the BC66_VDD_EXT output pin of the BC66 module
	// When the BC66 is on, 1.8 volts is supplied to this pin, so it can be used to check the status of the BC66.
	GPIOA->MODER &= ~GPIO_MODER_MODE5;			// clear mode for PA5, so it is on input mode
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_5;			/* no Push/Pull for PA5 */
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEED5;	/* low speed for PA5 */

}

//void initGPIOB(void)
//{
	// Turn on the clock for GPIOB
	//RCC->IOPENR |= RCC_IOPENR_IOPBEN;			/* Enable clock for GPIO Port B */
	//__NOP();
	//__NOP();

	// PB1 Input for BC66_VDD_EXT output pin of the BC66
	// When the BC66 is on, 1.8 volts is supplied to this pin, so it can be used to check the status of the BC66.
	//GPIOB->MODER &= ~GPIO_MODER_MODE1;			// clear mode for PB1, so it is on input mode
	//GPIOB->OTYPER &= ~GPIO_OTYPER_OT_1;			/* no Push/Pull for PB1 */
	//GPIOB->OSPEEDR &= ~GPIO_OSPEEDER_OSPEED1;	/* low speed for PB1 */
//}

void quickLEDblink(void)
{
	GPIOA->BSRR = GPIO_BSRR_BS_4;	// LED on - atomic set PA4
	delay(100);
	GPIOA->BSRR = GPIO_BSRR_BR_4;	// LED off - atomic clr PA4
	delay(100);
}

void slowLEDblink(void)
{
	GPIOA->BSRR = GPIO_BSRR_BS_4;	// LED on - atomic set PA4
	delay(500);
	GPIOA->BSRR = GPIO_BSRR_BR_4;	// LED off - atomic clr PA4
	delay(500);
}
