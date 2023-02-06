// 05.03.2023

#include <string.h>

#include "stm32l0xx.h"
#include "delay.h"
#include "lpuart.h"

// buffer for receiving data from the modem
#define LPUART1_RX_BUF_SIZE 250
char LPUART1_rx_buffer[LPUART1_RX_BUF_SIZE]; // volatile ?

// received bytes counter
volatile uint8_t LPUART1_rx_char_cnt = 0;

void LPUART1_init(int BaudRate)
{
	// Turn on the clock for GPIOA
	RCC->IOPENR |= RCC_IOPENR_IOPAEN;
	// Turn on the clock for the LPUART1
	RCC->APB1ENR |= RCC_APB1ENR_LPUART1EN;
	__NOP();
	__NOP();

	// enable alternate functions for PA2, PA3
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3))\
					| (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1);
	// Set AF6 for PA2, PA3 (LPUART1_TX, LPUART1_RX)
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2 | GPIO_AFRL_AFSEL3); // clear to AF0 (0000)
	GPIOA->AFR[0] |= ((6 << GPIO_AFRL_AFSEL2_Pos) | (6 << GPIO_AFRL_AFSEL3_Pos)); // set to AF6 (0110)

	// PA2 set to High speed - 04.03.2022
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEED2_Msk; // OSPEED2[1:0] = 00 - low speed
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEED2_1;    // OSPEED2[1:0] = 10 - high speed
	// PA2 Pull-Up - 04.03.2022
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD2;  // Pull-up reset to 00
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD2_0; // PUPD2[1:0] = 01 - pull-up

	// PA3 set to High speed - 04.03.2022
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEED3_Msk; // OSPEED3[1:0] = 00 - low speed
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEED3_1;    // OSPEED3[1:0] = 10 - high speed
	// PA3 Pull-Up - 04.03.2022
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD3;  // Pull-up reset to 00
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD3_0; // PUPD3[1:0] = 01 - pull-up

	// Turn off the LPUART1
	LPUART1->CR1 &= ~(USART_CR1_UE);

	// Set LSE as clock source for LPUART1
	// LSE is turned on in function initRtc()
	RCC->CCIPR |= RCC_CCIPR_LPUART1SEL; // LPUART1SEL[1:0]=11

	// Set baudrate
	// LPUART1->BRR = (32768*256 / (long)BaudRate); // 32768*256/9600=873.8=0x36A
	LPUART1->BRR = 0x369;

	// 8 data bit, 1 start bit, 1 stop bit, no parity
	LPUART1->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0 | USART_CR1_PCE);
	LPUART1->CR2 &= ~(USART_CR2_STOP_Msk);

	// Overrun disable
	// LPUART1->CR3 |= USART_CR3_OVRDIS;

	// Enable transmitter, receiver, receive-interrupt and turn on the LPUART1
	LPUART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_UE;

}

// clearing the buffer for receiving data from the modem
void LPUART1_rx_buffer_clear(void)
{
	for(uint8_t i = 0; i <= LPUART1_RX_BUF_SIZE; i++)
	{
		LPUART1_rx_buffer[i] = 0;
	}

	LPUART1_rx_char_cnt = 0;
}

// search for a string in the receive buffer, interval 20 ms, duration timeout*20ms.
int8_t LPUART1_wait_for_20ms(char *find, uint16_t timeout)
{
	while(timeout--)
	{
		delay(20);
		if(strstr(LPUART1_rx_buffer,find))
		{
			return 1; // OK, string was found.
		}
	}

	return 0; // ERROR, string was not found.
}

// for usart transmit without use interrupt
void LPUART1_send_char(char ch) // uint8_t ?
{
	// Wait for buffer to be empty
	while((LPUART1->ISR & USART_ISR_TXE) == 0);
	LPUART1->TDR = ch;
    // while(!(LPUART1->ISR & USART_ISR_TXE));
}

void LPUART1_print_string(char *str)
{
    while (*str)
    {
        LPUART1_send_char(*str++); // uint8_t ?
    }
}


// void AES_RNG_LPUART1_IRQHandler(void)
void LPUART1_IRQHandler(void)
{
    // read the received character
	uint8_t received = 0;

	// if((LPUART1->ISR & USART_ISR_RXNE) == USART_ISR_RXNE)
	if (LPUART1->ISR & USART_ISR_RXNE)
	{
		received = (uint8_t)(LPUART1->RDR); // Receive data, clear flag
		LPUART1_rx_buffer[LPUART1_rx_char_cnt++] = received;
		if (LPUART1_rx_char_cnt > LPUART1_RX_BUF_SIZE)
		{
			LPUART1_rx_char_cnt = 0;
		}
	}
}
