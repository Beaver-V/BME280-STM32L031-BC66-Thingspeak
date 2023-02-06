// Based on code from:
// https://www.avrfreaks.net/forum/try-understand-compiler-logic-about-static-varsfuncs

#include "stm32l0xx.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lpuart.h>

#include "delay.h"
#include "i2c.h"
#include "bme280.h"


uint8_t tmp_val[0];
uint8_t *tmp_ptr;

/* calibration coefficients */
typedef struct {
    uint16_t T1;
    int16_t T2;
    int16_t T3;

    uint16_t P1;
    int16_t P2;
    int16_t P3;
    int16_t P4;
    int16_t P5;
    int16_t P6;
    int16_t P7;
    int16_t P8;
    int16_t P9;

    unsigned char H1;
    int16_t H2;
    unsigned char H3;
    int16_t H4;
    int16_t H5;
    char H6;
} BME280_coeffs_t;

/* raw sensor data */
typedef struct {
    unsigned char t0;
    unsigned char t1;
    unsigned char t2;
    unsigned char p0;
    unsigned char p1;
    unsigned char p2;
    unsigned char h0;
    unsigned char h1;
} BME280_data_t;

BME280_coeffs_t BME280_calib;
BME280_data_t BME280_tph;


//--------------------------- issue reset -------------------------------------
//
int8_t BME280_reset(void)
{
	tmp_val[0] = BME280_RESET_CMD;
	tmp_ptr    = & tmp_val[0];

	if (I2CWrite((BME280_I2C_ADDR << 1), BME280_RESET_ADDR, tmp_ptr, 1))
	{
		return 1; // error
	}
	return 0;     // no error
}
//


//--------------------------- read sensor ID ----------------------------------
//
int8_t BME280_read_id(uint8_t *sensor_id_val)
{
	sensor_id_val[0] = 0;

	if (I2CRead((BME280_I2C_ADDR << 1), BME280_ID_ADDR, sensor_id_val, 1))
	{
		return 1; // error
	}
	return 0;     // no error
}



//--------------------------- read calibration coefficients -------------------
//
int8_t BME280_read_calibration(void) {
	
	unsigned char tp_coeffs[BME280_TP_CALIB_LENGTH];
	unsigned char h1_coeff [1];
	unsigned char h_coeffs[BME280_HUM_REST_CALIB_LENGTH];
	
	// read T and P calibration coefficients
	if (I2CRead((BME280_I2C_ADDR << 1), BME280_TP_CALIB_START_ADDR, tp_coeffs, BME280_TP_CALIB_LENGTH))
	{
		return 1; // error
	}

	// read H1 calibration coefficient
	if (I2CRead((BME280_I2C_ADDR << 1), BME280_HUM_H1_CALIB_START_ADDR, h1_coeff, 1))
	{
		return 1; // error
	}

	// read H2-H6 calibration coefficients
	if (I2CRead((BME280_I2C_ADDR << 1), BME280_HUM_REST_CALIB_START_ADDR, h_coeffs, BME280_HUM_REST_CALIB_LENGTH))
	{
		return 1; // error
	}

	// properly combine bytes to get numeric coefficients
	BME280_calib.T1 = (((uint16_t)tp_coeffs[1]) << 8) | ((uint16_t)tp_coeffs[0]);
	BME280_calib.T2 = (((int16_t)tp_coeffs[3]) << 8) | ((int16_t)tp_coeffs[2]);
	BME280_calib.T3 = (((int16_t)tp_coeffs[5]) << 8) | ((int16_t)tp_coeffs[4]);
//
	BME280_calib.P1 = (((uint16_t)tp_coeffs[7]) << 8) | ((uint16_t)tp_coeffs[6]);
	BME280_calib.P2 = (((int16_t)tp_coeffs[9]) << 8) | ((int16_t)tp_coeffs[8]);
	BME280_calib.P3 = (((int16_t)tp_coeffs[11]) << 8) | ((int16_t)tp_coeffs[10]);
	BME280_calib.P4 = (((int16_t)tp_coeffs[13]) << 8) | ((int16_t)tp_coeffs[12]);
	BME280_calib.P5 = (((int16_t)tp_coeffs[15]) << 8) | ((int16_t)tp_coeffs[14]);
	BME280_calib.P6 = (((int16_t)tp_coeffs[17]) << 8) | ((int16_t)tp_coeffs[16]);
	BME280_calib.P7 = (((int16_t)tp_coeffs[19]) << 8) | ((int16_t)tp_coeffs[18]);
	BME280_calib.P8 = (((int16_t)tp_coeffs[21]) << 8) | ((int16_t)tp_coeffs[20]);
	BME280_calib.P9 = (((int16_t)tp_coeffs[23]) << 8) | ((int16_t)tp_coeffs[22]);
//
	BME280_calib.H1 = (unsigned char)h1_coeff[1];
	BME280_calib.H2 = (((int16_t)h_coeffs[1]) << 8) | ((int16_t)h_coeffs[0]);
	BME280_calib.H3 = (unsigned char)h_coeffs[2];
	BME280_calib.H4 = (((int16_t)h_coeffs[3]) << 4) | ((int16_t)(h_coeffs[4] & 0x0F));
	BME280_calib.H5 = (((int16_t)h_coeffs[5]) << 4) | ((int16_t)(h_coeffs[4] >> 4));
	BME280_calib.H6 = (char)h_coeffs[6];

	return 0; // no error
}


/* set measurement parameters */
int8_t BME280_set_acquisition(const unsigned char os_t, const unsigned char os_p,
		                       const unsigned char os_h, const unsigned char mode,
                               const unsigned char t_sb, const unsigned char iir_filter,
							   const unsigned char spi3w_en)
{
	uint8_t *ptr;
	uint8_t BME280cfg[3];

	// write parameters for humidity oversampling first
	BME280cfg[0] = (uint8_t)os_h;
	ptr = & BME280cfg[0];

	if (I2CWrite((BME280_I2C_ADDR << 1), BME280_CTRL_HUM_ADDR, ptr, 1))
	{
		return 1; // error
	}
	
	// then write: Standby Time, Time Constant of the IIR Filter, choose SPI or I2C interface.
	// Writes to the config register in normal mode may be ignored.
	// In sleep mode writes are not ignored.
	BME280cfg[2] = (uint8_t)((t_sb << 5) | (iir_filter << 2) | spi3w_en);
	ptr = & BME280cfg[2];

	if (I2CWrite((BME280_I2C_ADDR << 1), BME280_CFG_ADDR, ptr, 1))
	{
		return 1; // error
	}

	// and write: Oversampling for Temperature and Pressure and set working mode.
	BME280cfg[1] = (uint8_t)((os_t << 5) | (os_p << 2) | mode);
	ptr = & BME280cfg[1];

	if (I2CWrite((BME280_I2C_ADDR << 1), BME280_CTRL_MEAS_ADDR, ptr, 1))
	{
		return 1; // error
	}
	
	return 0; // no error
}


//
int8_t BME280_forced_mode_start(const unsigned char os_t, const unsigned char os_p,
		                        const unsigned char os_h, const unsigned char mode)
{
	uint8_t *ptr;
	uint8_t BME280cfg[3];

	// write parameters for humidity oversampling first
	BME280cfg[0] = (uint8_t)os_h;
	ptr = & BME280cfg[0];
	if (I2CWrite((BME280_I2C_ADDR << 1), BME280_CTRL_HUM_ADDR, ptr, 1))
	{
		return 1; // error
	}

	// and write: Oversampling for Temperature and Pressure and set working mode.
	BME280cfg[1] = (uint8_t)((os_t << 5) | (os_p << 2) | mode);
	ptr = & BME280cfg[1];

	if (I2CWrite((BME280_I2C_ADDR << 1), BME280_CTRL_MEAS_ADDR, ptr, 1))
	{
		return 1; // error
	}

	return 0; // no error
}
//

/* read sensor status */
int8_t BME280_read_status(uint8_t *status_val)
{
	status_val[0] = 0x09;
	if (I2CRead((BME280_I2C_ADDR << 1), BME280_STATUS_ADDR, status_val, 1))
		{
			return 1; // error
		}
	status_val[0] &= 0x09;
	return 0; // no error
}


//--------------------------- read raw measurements ---------------------------
//
int8_t BME280_read_TPH(void)
{
	
	uint8_t data[BME280_TPH_LENGTH];

	I2CRead((BME280_I2C_ADDR << 1), BME280_TPH_START_ADDR, data, BME280_TPH_LENGTH);

	BME280_tph.t0 = data[3];
	BME280_tph.t1 = data[4];
	BME280_tph.t2 = data[5];

	BME280_tph.p0 = data[0];
	BME280_tph.p1 = data[1];
	BME280_tph.p2 = data[2];

	BME280_tph.h0 = data[6];
	BME280_tph.h1 = data[7];


	return 0; // no error
}


//--------------------- convert raw measurements data into physical values -----
//
void BME280_compensate(int16_t *final_T, uint32_t *final_P, uint32_t *final_H) {
	
	int32_t t_raw = (int32_t)((((int32_t)BME280_tph.t0) << 12) | (((int32_t)BME280_tph.t1) << 4) | (((int32_t)BME280_tph.t2) >> 4));
	int32_t var1, var2, t_fine, T;
	var1 = (((t_raw >> 3) - (((int32_t)BME280_calib.T1) << 1)) * ((int32_t)BME280_calib.T2)) >> 11;
	var2 = (((((t_raw >> 4) - ((int32_t)BME280_calib.T1)) * ((t_raw >> 4) - ((int32_t)BME280_calib.T1))) >> 12) * ((int32_t)BME280_calib.T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;

	if ((T <= BME280_TEMP_LOW) || (T >= BME280_TEMP_HIGH)) {
		if (T <= BME280_TEMP_LOW)
		*final_T = BME280_TEMP_LOW;
		else
		*final_T = BME280_TEMP_HIGH;
	}
	else {
		*final_T = T;   // 4000=40.00
	}

	int32_t p_raw = (int32_t)((((uint32_t)BME280_tph.p0) << 12) | (((uint32_t)BME280_tph.p1) << 4) | (((uint32_t)BME280_tph.p2) >> 4));
	uint32_t P;
	var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
	var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)BME280_calib.P6);
	var2 = var2 + ((var1 * ((int32_t)BME280_calib.P5)) << 1);
	var2 = (var2 >> 2) + (((int32_t)BME280_calib.P4) << 16);
	var1 = (((BME280_calib.P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)BME280_calib.P2) * var1) >> 1)) >> 18;
	var1 = ((((32768 + var1)) * ((int32_t)BME280_calib.P1)) >> 15);
	P = (((uint32_t)(((int32_t)1048576) - p_raw) - (var2 >> 12))) * 3125;

	if (var1 != 0) {
		if (P < 0x80000000)
		P = (P << 1) / ((uint32_t)var1);
		else
		P = (P / ((uint32_t)var1)) * 2;

		var1 = (((int32_t)BME280_calib.P9) * ((int32_t) (((P >> 3) * (P >> 3)) >> 13))) >> 12;
		var2 = (((int32_t)(P >> 2)) * ((int32_t)BME280_calib.P8)) >> 13;
		P = (uint32_t)((int32_t)P + ((var1 + var2 + BME280_calib.P7) >> 4));
	}
	else
	P = 0;

	if ((P <= BME280_PRESS_LOW) || (P >= BME280_PRESS_HIGH)) {
		if (P < BME280_PRESS_LOW)
		*final_P = BME280_PRESS_LOW / 133.322;                  //
		else
		*final_P = BME280_PRESS_HIGH / 133.322;                 //
	}
	else {
		*final_P = P / 133.322;                                 //
	}

	int32_t h_raw = (int32_t)((((int32_t)BME280_tph.h0) << 8) | ((int32_t)BME280_tph.h1));
	int32_t var3, var4, var5;
	uint32_t H;
	var1 = (int32_t)t_fine - (int32_t)76800;
	var2 = h_raw << 14;
	var3 = ((int32_t)BME280_calib.H4) << 20;
	var4 = ((int32_t)BME280_calib.H5) * var1;
	var5 = ((var2 - var3 - var4 + (int32_t)16384) >> 15);
	var2 = (var1 * ((int32_t)BME280_calib.H6)) >> 10;
	var3 = (var1 * ((int32_t)BME280_calib.H3)) >> 11;
	var4 = ((var2 * (var3 + (int32_t)32768)) >> 10) + (int32_t)2097152;
	var2 = (var4 * ((int32_t)BME280_calib.H2) + 8192) >> 14;
	var3 = var5 * var2;
	var4 = ((var3 >> 15) * (var3 >> 15)) >> 7;
	var5 = var3 - ((var4 * ((int32_t)BME280_calib.H1)) >> 4);
	var5 = (var5 < 0 ? 0 : var5);
	var5 = (var5 > 419430400 ? 419430400 : var5);
	H = (uint32_t)(var5 >> 12);

	if ((H <= BME280_HUM_LOW) || (H >= BME280_HUM_HIGH)) {
		if (H <= BME280_HUM_LOW)
		*final_H = BME280_HUM_LOW;
		else
		*final_H = BME280_HUM_HIGH / 1000; //
	}
	else {
		*final_H = H / 1000;               //
	}
}
//

//-------------------------- perform one measurement cycle and get results -----
//
unsigned char BME280_measure(int16_t *T, uint32_t *P, uint32_t *H)

{
	delay(30); // 20.04.2021 add for debug

	// Reset BME280
	if (BME280_reset())
	{
		return 1; // error
	}
	delay(10); // 2 ms - Start-up time in datasheet

	if (BME280_read_id(sensor_id_val))
	 return BME280_ERR_CONN;
	if (sensor_id_val[0] != BME280_ID)
	return BME280_ERR_ID;

	if (BME280_read_calibration())
	return BME280_ERR_CALIB;

	// Set parameters for measurement
	if (BME280_set_acquisition(OVER_1x, OVER_1x, OVER_1x, BME280_FORCED_MODE,
		              BME280_STANDBY_500us, BME280_IIR_OFF, BME280_SPI_OFF))
	{
		//
	}

	delay(30);

	if (BME280_forced_mode_start(OVER_1x, OVER_1x, OVER_1x, BME280_FORCED_MODE))
	{
		return BME280_ERR_FORCED_MODE;
	}

	// wait until sensor becomes idle
	do {
		if (BME280_read_status(status_val))
		return BME280_ERR_STATUS;
	} while (status_val[0]);

	if (BME280_read_TPH()) // read RAW data temperature, pressure and humidity
	return BME280_ERR_IO;

	BME280_compensate(T, P, H); // perform conversion

	return 0; // return BME280_ERR_NO;
}
