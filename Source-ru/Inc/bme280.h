// Based on code from:
// https://www.avrfreaks.net/forum/try-understand-compiler-logic-about-static-varsfuncs

#ifndef _BME280_H
#define _BME280_H

uint8_t sensor_id_val[1];
uint8_t status_val[1];

/* BME280 address on I2C bus */
#define BME280_I2C_ADDR     0x76

/* BME280 ID value */
#define BME280_ID           0x60

/* registers addresses */
#define BME280_ID_ADDR                      0xD0
#define BME280_RESET_ADDR                   0xE0
#define BME280_TP_CALIB_START_ADDR          0x88
#define BME280_HUM_H1_CALIB_START_ADDR      0xA1
#define BME280_HUM_REST_CALIB_START_ADDR    0xE1
#define BME280_CTRL_HUM_ADDR                0xF2
#define BME280_CTRL_MEAS_ADDR               0xF4
#define BME280_STATUS_ADDR                  0xF3
#define BME280_TPH_START_ADDR               0xF7
#define BME280_PRESS_START_ADDR             0xF7
#define BME280_TEMP_START_ADDR              0xFA
#define BME280_HUM_START_ADDR               0xFD
// config for t_standby, IIR filter, SPI enable
#define BME280_CFG_ADDR						0xF5

/* lengths of data registers */
#define BME280_TP_CALIB_LENGTH          24
#define BME280_H_CALIB_LENGTH           8
#define BME280_HUM_REST_CALIB_LENGTH    7
#define BME280_TPH_LENGTH               8
#define BME280_PRESS_LENGTH             3
#define BME280_TEMP_LENGTH              3
#define BME280_HUM_LENGTH               2


/* available commands and corresponding codes */
#define BME280_RESET_CMD    0xB6

/* limits for raw data */
#define BME280_TEMP_LOW     -4000
#define BME280_TEMP_HIGH    8500
#define BME280_PRESS_LOW    30000
#define BME280_PRESS_HIGH   110000
#define BME280_HUM_LOW      0
#define BME280_HUM_HIGH     102400

/* error codes */
#define BME280_ERR_NO			0
#define BME280_ERR_CONN			1
#define BME280_ERR_ID			2
#define BME280_ERR_CALIB		3
#define BME280_ERR_ACQ			4
#define BME280_ERR_STATUS		5
#define BME280_ERR_IO			6
#define BME280_ERR_FORCED_MODE	7

#define OVER_0x					0x00
#define OVER_1x					0x01
#define OVER_2x					0x02
#define OVER_4x					0x03
#define OVER_8x					0x04
#define OVER_16x				0x05

#define BME280_FORCED_MODE		0x01
#define BME280_NORMAL_MODE		0x03    // t_sb[2:0] config (0xF5)
#define BME280_SLEEP_MODE		0x00
// standbay time, t_sb[2:0], 7,6,5 config (0xF5)
#define BME280_STANDBY_500us	0x00
#define BME280_STANDBY_62500us	0x01
#define BME280_STANDBY_125ms	0x02
#define BME280_STANDBY_250ms	0x03
#define BME280_STANDBY_500ms	0x04
#define BME280_STANDBY_1000ms	0x05
#define BME280_STANDBY_10ms		0x06
#define BME280_STANDBY_20ms		0x07
// IIR filter, filter[2:0], 4,3,2 config (0xF5)
#define BME280_IIR_OFF			0x00
#define BME280_IIR_2x			0x01
#define BME280_IIR_4x			0x02
#define BME280_IIR_8x			0x03
#define BME280_IIR_16x			0x04
// spi3w_en[0], I2C SPI, 0 config (0xF5)
#define BME280_SPI_OFF			0x00
#define BME280_SPI_ON			0x01



int8_t BME280_reset(void);
int8_t BME280_read_id(uint8_t *sensor_id_val);
int8_t BME280_read_calibration(void);
int8_t BME280_set_acquisition(const unsigned char os_t, const unsigned char os_p,
		                       const unsigned char os_h, const unsigned char mode,
							   const unsigned char t_sb, const unsigned char iir_filter,
							   const unsigned char spi3w_en);
int8_t BME280_read_status(uint8_t *status_val);
int8_t BME280_read_TPH(void);
void BME280_compensate(int16_t *final_T, uint32_t *final_P, uint32_t *final_H);
int8_t BME280_forced_mode_start(const unsigned char os_t, const unsigned char os_p,
		                        const unsigned char os_h, const unsigned char mode);
extern unsigned char BME280_measure(int16_t *T, uint32_t *P, uint32_t *H);

#endif
