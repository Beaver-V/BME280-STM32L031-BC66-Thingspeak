# BME280-STM32L031-BC66-Thingspeak

STM32CubeIDE проект для отправки погодных данных на сервер thingspeak.com

Микроконтролер: STM32L031F6P6
Датчик:         BME280
Модем:          BC66NADA-04-STD

Для снижения энергопотребления STM32L031 большую часть времени находится в режиме СТОП, а модуль BC66 выключен. STM32L031 выводится из режима СТОП таймером RTC. Проснувшись, микроконтроллер однократно запускает BME280 на измерение. Затем программа включает BC66 и ожидает его регистрацию в NB-IoT сети. Если регистрация в сети была успешной, производится подключение к серверу и отправка данных. Кроме температуры, давления и влажности на сервер отправляются ещё напряжение батарейки и уровень сигнала сети. После отправки данных происходит отключение от сервера, выключение BC66 и перевод микроконтроллера в режим СТОП. Режимы PSM и eDRX модуля BC66 не используются, может быть они будут реализованны в следующих версиях программы.

Код для STM32L031F6P6 написан в STM32CubeIDE с использование только библиотеки CMSIS (HAL не используется).
При проверке оказалось что прерывание LPUART1 не работает. Оно заработало после замены файла startup_stm32l031f6px.s аналогичным из проекта сгенерированного для HAL.

===

STM32CubeIDE project to send weather data to thingspeak.com server

Microcontroller: STM32L031F6P6
Sensor: BME280
Modem: BC66NADA-04-STD

To reduce power consumption, the STM32L031 is in STOP mode most of the time and the BC66 is turned off. The STM32L031 is brought out of STOP mode by the RTC timer. After waking up, the microcontroller triggers the BME280 for measurement once. Then the program turns on the BC66 and waits for it to register in the NB-IoT network. If registration on the network was successful, a connection to the server is made and data is sent. In addition to temperature, pressure and humidity, battery voltage and network signal strength are also sent to the server. After sending the data, the server is disconnected, the BC66 is turned off and the microcontroller is put into STOP mode. The PSM and eDRX modes of the BC66 module are not used, maybe they will be implemented in future versions of the program.

The code for STM32L031F6P6 is written in STM32CubeIDE using only the CMSIS library (HAL is not used).
When checking, it turned out that the LPUART1 interrupt did not work. It worked after replacing the startup_stm32l031f6px.s file with a similar one from the project generated for HAL.
