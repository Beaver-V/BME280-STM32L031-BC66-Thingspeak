// 05.02.2023

#ifndef LPUART_H_
#define LPUART_H_

// LPUART1 initialization
void LPUART1_init(int BaudRate);

// clearing the buffer for receiving data from the modem
void LPUART1_rx_buffer_clear(void);

// search for a string in the receive buffer, interval 20 ms, duration timeout*20ms.
int8_t LPUART1_wait_for_20ms(char *find, uint16_t timeout);

void LPUART1_send_char(char ch);
void LPUART1_print_string(char *str);

#endif
