#ifndef BC66_H_
#define BC66_H_

// включение BC66
char pwr_on_BC66(uint8_t timeout);

// выключение BC66
void pwr_off_BC66(void);

char sync_BC66_UART(void);

// BC66 отключение сна UART
char at_qsclk(void);

// отключение эха АТ команд
// void at_e(void);

// проверка наличия SIM карты
char at_cpin(void);

// BC66 - проверка регистрации в NB-IoT сети
char at_cereg(void);

// BC66 - ожидание выключения
char wait_for_pwr_off(char timeout);

// напряжение батарейки
void at_cbc(void);

// BC66 информация о базовой станции
void at_qeng_BC66(void);

// соединимся по TCP с сервером AT+QIOPEN=1,0,"TCP","my_server",my_port,0,0
char at_qiopen(char *my_server, char *my_port);

// подключение к серверу
char try_to_connect(char *my_server, char *my_port, uint8_t count1);

// отправка сообщения на сервер AT+QISEND=<connectID>,<send_length>,<data>
char at_qisend(char *report_text_len, char *report_text);

// отправка 0x0D, 0x0A на сервер
char at_qisend_CR_LF(void);

// отключение от сервера
char at_qiclose(void);

#endif /* BC66_H_ */
