#include "stm32l031xx.h"
#include "bc66.h"
#include "delay.h"
#include "lpuart.h"

// BC66 power on
char pwr_on_BC66(uint8_t timeout)
{
	GPIOA->BSRR = GPIO_BSRR_BS_4;		// atomic set PA4, so LED on
	GPIOA->BSRR = GPIO_BSRR_BS_1;		// atomic set PA1, so PWRKEY=0
	delay(550);							// 550 ms for power on BC66
	GPIOA->BSRR = GPIO_BSRR_BR_1;		// atomic clr PA1, so PWRKEY=1
	GPIOA->BSRR = GPIO_BSRR_BR_4;		// atomic clr PA4, so LED off

	while(timeout--)
		{
			delay(100);
			if(GPIOA->IDR & GPIO_IDR_ID5) return 1; // BC66 включился
		}
		return 0;									// BC66 не включился
}

// BC66 power off
void pwr_off_BC66(void)
{
	LPUART1_print_string("AT+QPOWD=0\r");
	wait_for_pwr_off(250);							// до 250x100ms=25s ждем пропадания VDD_EXT
}


char sync_BC66_UART(void)
{
	uint8_t a;
	for (uint8_t i=0; i<25; i++)
	{
		LPUART1_rx_buffer_clear();
		LPUART1_print_string("AT\r");
		if (LPUART1_wait_for_20ms("OK", 15))	// 20*15=300ms
		{
			a = 1;
			break;
		}
		else a = 0;
	}
	return a;
}


// BC66 отключение сна UART
char at_qsclk(void)
{
	LPUART1_rx_buffer_clear();
	LPUART1_print_string("AT+QSCLK=0\r");
	if (LPUART1_wait_for_20ms("OK", 15))	// 20*15=300ms
		return 1;
	else
		return 0;
}

// отключение эха АТ команд
// void at_e(void)
// {
	// LPUART1_rx_buffer_clear();
	// LPUART1_print_string("ATE0\r");
	// delay(100);
// }


// проверка наличия SIM карты
char at_cpin(void)
{
	uint8_t a;
	for (uint8_t i=0; i<5; i++)
	{
		LPUART1_rx_buffer_clear();
		LPUART1_print_string("AT+CPIN?\r");
		if (LPUART1_wait_for_20ms("CPIN: READY", 25))
		{
			a = 1;
			break;
		}
		else a = 0;
	}
	return a;
}


// BC66 - проверка регистрации в NB-IoT сети
char at_cereg(void)
{
	LPUART1_rx_buffer_clear();
	LPUART1_print_string("AT+CEREG?\r");
	if (LPUART1_wait_for_20ms("+CEREG: 0,1",5)) // 20ms*5=0.1s
		return 1;								// BC66 зарегистрирован в сети
	else
		return 0;								// BC66 не зарегистрирован в сети
}

// BC66 - ожидание выключения
char wait_for_pwr_off(char timeout)
{
	while(timeout--)
	{
		delay(100);
		if(!(GPIOA->IDR & GPIO_IDR_ID5))
			return 1;							// VDD_EXT=0V - модуль выключился
	}
	return 0;									// VDD_EXT=1.8V - модуль не выключился
}


// напряжение батарейки
void at_cbc(void)
{
	LPUART1_rx_buffer_clear();
	LPUART1_print_string("AT+CBC\r");
	delay(100);
}

// BC66 информация о базовой станции
void at_qeng_BC66(void)
{
	LPUART1_rx_buffer_clear();
	LPUART1_print_string("AT+QENG=0\r");
	delay(100);
}

// соединимся по TCP с сервером AT+QIOPEN=1,0,"TCP","my_server",my_port,0,0
char at_qiopen(char *my_server, char *my_port)
{
	LPUART1_rx_buffer_clear();
	LPUART1_print_string("AT+QIOPEN=1,0,");
	LPUART1_send_char(0x22);
	LPUART1_print_string("TCP");
	LPUART1_send_char(0x22);
	LPUART1_print_string(",");
	LPUART1_send_char(0x22);
	LPUART1_print_string(my_server);
	LPUART1_send_char(0x22);
	LPUART1_print_string(",");
	LPUART1_send_char(0x22);
	LPUART1_print_string(my_port);
	LPUART1_send_char(0x22);
	LPUART1_print_string(",0,1\r");   // 0,1 - local_port, Direct push mode; 0,0 - local_port, Buffer access mode

	if (LPUART1_wait_for_20ms("OK", 15))	// 20*15=300ms
			return 1;
		else
			return 0;
}

// подключение к серверу
char try_to_connect(char *my_server, char *my_port, uint8_t count1)
{
	uint8_t a = 0;
	for (uint8_t i=0; i<count1; i++)
	{
		at_qiopen(my_server, my_port);						// команда at+qiopen
		if (LPUART1_wait_for_20ms("+QIOPEN: 0,0", 2250))	// до 45 секунд ожидание подключения к серверу
		{
			a = 1;											// подключились
			break;
		}
		else if (!at_qiclose())								// останов неудачной попытки подключится
		{
			a = 0;											// ошибка или таймаут при выполнении команды at+qiclose
			break;
		}
	}
	return a;												// или подключились, или закончились попытки, или ошибка
}



// отправка сообщения на сервер
// AT+QISEND=<connectID>,<send_length>,<data>
char at_qisend(char *report_text_len, char *report_text)
{
	LPUART1_rx_buffer_clear();
	LPUART1_print_string("AT+QISEND=0,");
	LPUART1_print_string(report_text_len);
	LPUART1_print_string(",");
	LPUART1_print_string(report_text);
	LPUART1_print_string("\r");

	if (LPUART1_wait_for_20ms("SEND OK",50))	// 1s
	{
		return 1;
	}
	else return 0;
}

// отправка 0x0D, 0x0A на сервер
char at_qisend_CR_LF(void)
{
	LPUART1_rx_buffer_clear();
	LPUART1_print_string("AT+QISENDEX=0,2,0D0A\r");
	// LPUART_print_string("AT+QISENDEX=0,1,0A\r"); // only 0x0A

	if (LPUART1_wait_for_20ms("SEND OK",50))	// 1s
	{
		return 1;
	}
	else return 0;
}

// отключение от сервера
char at_qiclose(void)
{
	LPUART1_rx_buffer_clear();
	LPUART1_print_string("AT+QICLOSE=0\r");

	if (LPUART1_wait_for_20ms("CLOSE OK", 275)) // 5.5s
	{
		return 1;
	}
	else return 0;
}
