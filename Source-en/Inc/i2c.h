// This code is from here:
// https://github.com/heavyC1oud/meteoStick
// Article  about this project: https://habr.com/ru/post/486988/

#ifndef I2C_H_
#define I2C_H_

/*************************	FUNCTION PROTOTYPE	******************************/
void initI2C(void);
int8_t I2CWrite(uint8_t devID, uint8_t addr, uint8_t* data, uint16_t count);
int8_t I2CRead(uint8_t devID, uint8_t addr, uint8_t* data, uint16_t count);

#endif /* I2C_H_ */
