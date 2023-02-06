// 09.02.2022

#ifndef LPUART_H_
#define LPUART_H_

// Инициализация LPUART1
void LPUART1_init(int BaudRate);

// очистка буфера приема  данных из модема
void LPUART1_rx_buffer_clear(void);

// поиск строки в приемном буфере, интервал 20 мс, длительность timeout*20ms.
int8_t LPUART1_wait_for_20ms(char *find, uint16_t timeout);

void LPUART1_send_char(char ch);
void LPUART1_print_string(char *str);

#endif
