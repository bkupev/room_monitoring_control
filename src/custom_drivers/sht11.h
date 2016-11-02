/*
 * sht11.h
 *
 *  Created on: 13.5.2016
 *      Author: blagoj.kupev
 */

#ifndef CUSTOM_DRIVERS_SHT11_H_
#define CUSTOM_DRIVERS_SHT11_H_

#define SHT11_CLK_PORT		GPIOB
#define SHT11_CLK_PIN		GPIO_PIN_8
#define SHT11_DATA_PORT		GPIOB
#define SHT11_DATA_PIN		GPIO_PIN_9

#define SHT11_DATA			0
#define SHT11_CLK			1

// COMMAND definition
#define	CMD_MEASURE_TEMP	0x03
#define	CMD_MEASURE_R_H		0x05
#define CMD_RD_STATUS		0x07
#define CMD_WR_STATUS		0x06
#define	CMD_SOFT_RESET		0x1E

void SHT11_Config(void);
double SHT11_Measure_RH(void);
double SHT11_MeasureTemperature(void);
uint32_t SHT11_ReadStatus(void);
uint32_t SHT11_WriteStatus(uint8_t new_value);

#endif /* CUSTOM_DRIVERS_SHT11_H_ */
