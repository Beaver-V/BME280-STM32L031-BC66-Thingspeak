#ifndef BC66_H_
#define BC66_H_

// BC66 power on
char pwr_on_BC66(uint8_t timeout);

// BC66 power off
void pwr_off_BC66(void);

char sync_BC66_UART(void);

// BC66 UART sleep disable
char at_qsclk(void);

// disable echo of AT commands
// void at_e(void);

// checking for a SIM card
char at_cpin(void);

// BC66 - checking registration in NB-IoT network
char at_cereg(void);

// BC66 - waiting for shutdown
char wait_for_pwr_off(char timeout);

// battery voltage
void at_cbc(void);

// BC66 base station information
void at_qeng_BC66(void);

// connect via TCP to the server AT+QIOPEN=1,0,"TCP","my_server",my_port,0,0
char at_qiopen(char *my_server, char *my_port);

// server connection
char try_to_connect(char *my_server, char *my_port, uint8_t count1);

// sending a message to the server AT+QISEND=<connectID>,<send_length>,<data>
char at_qisend(char *report_text_len, char *report_text);

// sending 0x0D, 0x0A to server
char at_qisend_CR_LF(void);

// disconnect from the server
char at_qiclose(void);

#endif /* BC66_H_ */
