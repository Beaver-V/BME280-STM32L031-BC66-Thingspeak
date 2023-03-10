// Started: 14.02.2022 17:20
// Changed: 05.02.2023


/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include "stm32l031xx.h"
#include <string.h>
#include <stdlib.h>

#include "syst_clock.h"
#include "gpio.h"
#include "delay.h"
#include "i2c.h"
#include "lpuart.h"
#include "bme280.h"
#include "bc66.h"
#include "rtc.h"

//////////////////////////////
#define START_DEV			0
#define GET_VBAT_RSSI		1
#define WAIT_CEREG			2
#define SEND_REPORT			3
#define GO_TO_STOP			4
#define ERROR_PWR_ON		10
#define ERROR_BATTERY		11
#define ERROR_WAIT_CEREG	12
// #define ERROR_CBC		13
#define ERROR_QENG			14
#define ERROR_QIOPEN		15
#define ERROR_QISEND		16
///////////////////////////////

#define WORK_PERIOD		599    // Working period - the interval in seconds between sending to the server
#define CEREG_ATTEMPTS	3       // Number of attempts to register on the network
#define QIOPEN_ATTEMPTS	3       // Number of attempts to connect to the server

extern char LPUART1_rx_buffer[];
extern volatile uint8_t LPUART1_rx_char_cnt;

uint8_t state;					// The current state of the state machine
uint8_t cereg_count;			// Count of attempts to register on the network

int16_t  T;                		// Temperature
uint32_t P;                		// Pressure
uint32_t H;                		// Humidity

char T_for_print[5];      		// temperature to send to the server
char T_for_print_tmp[5];  		// to convert temperature to ascii
char P_for_print[5];      		// pressure to send to server
char H_for_print[5];      		// humidity to send to server

char V_batt[5];           		// battery voltage to send to server
char RSSI_level[5];      		// network signal strength to send to the server

char report_text[125];
char report_text_len[3];

char *write_API_key = "Your-Write-API-key";		// Paste your Write API Key for Thingspeak here
char *my_server     = "api.thingspeak.com";		// 184.106.153.149
char *my_port       = "80";						// thingspeak port


int main()
{
    state = START_DEV;
    cereg_count = CEREG_ATTEMPTS;		// Number of attempts to register on the network

    // Operating frequency initialization
    // setHSIClock32M();				// To use HSI16/4->PLL uncomment this line
	// setHSIClock16M();				// To use HSI16 uncomment this line
    // If the two lines above are commented out, then MSI 2.097 MHz is used

    // RCC->CFGR |= RCC_CFGR_STOPWUCK;	// set the STOPWUCK bit to select HSI16 after returning from Stop
    RCC->CFGR &= ~RCC_CFGR_STOPWUCK;	// clear STOPWUCK bit for MSI selection after return from Stop

    SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/1000); // interruption frequency 1000 Hz, tick=1ms

	initGPIOA();
	initI2C();
	initRTC();							// LSE starts here, it is also needed for LPUART1
	LPUART1_init(9600);

	NVIC_SetPriority(LPUART1_IRQn, 0);

	NVIC_EnableIRQ(LPUART1_IRQn);

	__enable_irq();

	// 2.5 seconds to click "Connect to the target" in ST-LINK
	for (uint8_t i=0; i<5; i++)
	{
		GPIOA->BSRR = GPIO_BSRR_BS_4;	// LED on
		delay(200);
		GPIOA->BSRR = GPIO_BSRR_BR_4;	// LED off
		delay(300);
	}


	//----------------------- Reset BME280 -------------------------------------
	if (BME280_reset())
	{
		//
	}
	delay(10); //						2 ms - Start-up time in BME280 datasheet

	//----------------------- Set parameters for measurement -------------------
	if (BME280_set_acquisition(OVER_1x, OVER_1x, OVER_1x, BME280_FORCED_MODE,
			              BME280_STANDBY_500us, BME280_IIR_OFF, BME280_SPI_OFF))
	{
		//
	}



	initWakeup(WORK_PERIOD);



	//----------------------- Main cycle ---------------------------------------
	while (1)
	{
		switch(state)
		{
		case START_DEV:

		// Measurement of temperature, pressure, humidity
		if (BME280_measure(&T, &P, &H))
		{
			T = 0;
			P = 0;
			H = 0;
		}

		// Temperature to send to server
		memset(T_for_print, 0x00, sizeof(T_for_print));
		if (T < 0x0000)
		{
			T = abs(T);                   // absolute value
			strcat (T_for_print, "-");    // minus sign for negative temperatures
		}
		else
		{
			// strcat (T_for_print, "+"); // plus sign for positive temperatures
		}
		// Integer part of the temperature
		uint16_t quotient_part_T = 0;
		quotient_part_T = (T / 100);
		memset(T_for_print_tmp, 0x00, sizeof(T_for_print_tmp));
		utoa(quotient_part_T, T_for_print_tmp, 10);
		strcat (T_for_print, T_for_print_tmp);
		// Decimal separator
		strcat (T_for_print, ".");
		// Fractional part of temperature
		uint16_t remainder_part_T = 0;
		remainder_part_T = (T % 100);
		if (remainder_part_T < 10)
			{
				strcat (T_for_print, "0");
			}
		memset(T_for_print_tmp, 0x00, sizeof(T_for_print_tmp));
		utoa(remainder_part_T, T_for_print_tmp, 10);
		strcat (T_for_print, T_for_print_tmp);

		// Pressure to send to the server
		memset(P_for_print, 0x00, sizeof(P_for_print));
		utoa(P, P_for_print, 10);

		// Humidity to send to server
		memset(H_for_print, 0x00, sizeof(H_for_print));
		utoa(H, H_for_print, 10);


		if (!pwr_on_BC66(50))			// BC66 power on
		{
			state = ERROR_PWR_ON;
			break;
		}

		else if (!at_qsclk())			// BC66 disable UART to sleep
		{
			state = ERROR_PWR_ON;
			break;
		}

		else if (!at_cpin())			// checking for a SIM card
		{
			state = ERROR_PWR_ON;
			break;
		}

		state = WAIT_CEREG;
		break;							// for case START_DEV

			case WAIT_CEREG:

			for (uint8_t i=0; i<250; i++)		// waiting for network registration 250*0.2 = 50 seconds
			{
				GPIOA->ODR ^= (1 << 4);			// toggle led on PA4

				if (at_cereg())
				{
					state=GET_VBAT_RSSI;		// BC66 registered on the network
					break;
				}
				else
				{
					if (wait_for_pwr_off(1))	// checking if the BC66 has turned off?
					{
						state=ERROR_BATTERY;	// BC66 turned off when registering to the network, the battery is dead
						break;
					}
				state=ERROR_WAIT_CEREG;			// BC66 not registered on the network
				}
			}

			cereg_count--;

			if ((state==ERROR_WAIT_CEREG) && (cereg_count > 0))
			{
				pwr_off_BC66();					// BC66 power off (wait up to 25 second)
				state = START_DEV;				// try to register again
				break;
			}

			cereg_count = CEREG_ATTEMPTS;		// number of attempts to register on the network after L031 waking up

			break;								// for case WAIT_CEREG


			case GET_VBAT_RSSI:

				// delay for voltage stabilization after registration in the network
				// with a weak signal (high currents) practically does not help
				delay(3000);

				// Determine battery voltage
				// AT command to measure battery voltage: AT+CBC<CR>
				// BC66 response to the command: CR><LF>+CBC: 0,0,3280<CR><LF><CR><LF>OK<CR><LF>
				at_cbc();
				memset(V_batt, 0x00, sizeof(V_batt));

				if (LPUART1_wait_for_20ms("OK", 50))				// 1 second
				{
					NVIC_DisableIRQ(LPUART1_IRQn);					// Disable interrupts for LPUART1
					char str2[] = "CBC:";							// the substring we are looking for
					char *istr = NULL;								// pointer to start of substring match
					istr = strstr (LPUART1_rx_buffer, str2);		// search
					if ( istr == NULL)
					{
						memset(V_batt, 0x31, (sizeof(V_batt) - 1));	// Error, set VBAT=1111 (1.111 Volts)
					}
					else
					{
						for (uint8_t i=0 ; i < 4 ; i++)
						{
							V_batt[i] = *(istr + 9 + i);			// read battery voltage
						}
					}
					NVIC_EnableIRQ(LPUART1_IRQn);					// Enable interrupts for LPUART1
				}


				// Network information
				// AT command BC66 for engineering mode: AT+QENG=0<CR> and the answer to it:
				// <CR><LF>+QENG: 0,1839,2,481,"35E5D4",-85,-6,-78,5,3,"4B1F",0,,0<CR><LF><CR><LF>OK<CR><LF>
				at_qeng_BC66();

				memset(RSSI_level, 0x00, sizeof(RSSI_level));

				if (LPUART1_wait_for_20ms("OK",150))				// have to wait ok
				{
					NVIC_DisableIRQ(LPUART1_IRQn);					// Disable interrupts for LPUART1

					// ???????????????? ???????????? ???????????????? ???????????? ?????????????? RSSI
					uint8_t comma_count = 0;						// comma counter
					uint8_t k = 0;									// index for RSSI_level[]
					for (uint8_t i = 0 ; i < LPUART1_rx_char_cnt ; i++)
					{
						if (LPUART1_rx_buffer[i] == ',')
						{
							comma_count++;
							if (comma_count == 7)					// seventh comma, followed by the RSSI value
							{
								i = i + 1;
								while (LPUART1_rx_buffer[(i)] != ',')
								{
									RSSI_level[k] = LPUART1_rx_buffer[(i)];
									k++;
									i++;
								}
							}
						}
					}

					NVIC_EnableIRQ(LPUART1_IRQn);					// Enable interrupts for LPUART1

					state=SEND_REPORT;
					break;
				}
				else
				{
					state=ERROR_QENG;
					break;
				}

				break;

		//////////////////////////////////////////////////////////////////////////////
			case SEND_REPORT:

						// report text for thingspeak server
						//
						memset (report_text, 0x00, sizeof(report_text));
						strcat (report_text, "GET https://api.thingspeak.com/update");
						strcat (report_text, "\?");							// insert a question mark
						strcat (report_text, "api_key=");
						strcat (report_text, write_API_key);
						strcat (report_text, "&field1=");
						strcat (report_text, T_for_print);
						strcat (report_text, "&field2=");
						strcat (report_text, P_for_print);
						strcat (report_text, "&field3=");
						strcat (report_text, H_for_print);
						strcat (report_text, "&field4=");
						strcat (report_text, V_batt);
						strcat (report_text, "&field5=");
						strcat (report_text, RSSI_level);
						utoa (strlen(report_text), report_text_len, 10);	// report size

						// connect via TCP to the server
						if (!try_to_connect(my_server, my_port, QIOPEN_ATTEMPTS))
						{
							state = ERROR_QIOPEN;						// not connected to the server
							break;
						}
						if (!at_qisend(report_text_len, report_text))	// connected to the server, send data
						{
							state = ERROR_QISEND;						// error sending data, close the socket
							break;
						}
						if (!at_qisend_CR_LF())							// send \r\n
						{
							state = ERROR_QISEND;						// error sending \r\n, close the socket
							break;
						}

						LPUART1_rx_buffer_clear();
						// up to 10 seconds waiting for confirmation of reception from the server:
						// +QIURC: "recv",0,nnn
						LPUART1_wait_for_20ms("recv", 500);

						at_qiclose();									// disconnect from the server
						state=GO_TO_STOP;

						break;
			//////////////////////////////////////////////////////////////////////////

			case ERROR_BATTERY:
				quickLEDblink();
			case ERROR_PWR_ON:
				quickLEDblink();
			case ERROR_WAIT_CEREG:
				quickLEDblink();
			case ERROR_QENG:
				quickLEDblink();
			state=GO_TO_STOP;
			break;

			case ERROR_QIOPEN:
				quickLEDblink();
			case ERROR_QISEND:
				quickLEDblink();
				at_qiclose();				// disconnect from the server
			state = GO_TO_STOP;
			break;

		/////////////////////////////////////////////////////////////////////////////

		case GO_TO_STOP:
		pwr_off_BC66();						// BC66 power off
		quickLEDblink();

		//
		//----------------- Leaving in Stop mode ----------------------------------------------------
		//

        // https://habr.com/ru/post/430218/
		PWR->CR &= ~(PWR_CR_PDDS);      // reset PDDS to select Stop mode (if set, then Standby)
        PWR->CR |= PWR_CR_CWUF;         // set CWUF to clear the Wakeup flag, or wake up immediately
        PWR->CR |= PWR_CR_LPSDSR;       // power stabilizer in low-power mode
        SCB->SCR |=  (SCB_SCR_SLEEPDEEP_Msk); // Set the SLEEPDEEP bit to select Stop Mode
        __disable_irq();                      // disable interrupts; it will not interfere with awakening on them
        __DSB();                              // completed a pending data save operation
        __WFI();                              // fell asleep



        //
        //----------------- Woke up, exiting Stop Mode --------------------------------------
        //

        // re-initialization of the operating frequency
        // setHSIClock32M();				// To use HSI16/4->PLL uncomment this line
        // setHSIClock16M();				// To use HSI16 uncomment this line
        // If the two lines above are commented out, then MSI 2.097 MHz is used

		SystemCoreClockUpdate();
		SysTick_Config(SystemCoreClock/1000);
		__enable_irq();						// interrupts restored after waking up

		state=START_DEV;
		break;

		default:
		break;

		}	// end switch(state)
	}		// end while(1)
}			// end main()
