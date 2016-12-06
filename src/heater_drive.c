/*
 * heater_drive.c
 *
 *  Created on: 15.11.2016
 *      Author: blagoj.kupev
 */

#include "stm32f7xx_hal.h"
#include "heater_drive.h"


void init_heater_drive(void)
{
	GPIO_InitTypeDef gpio_init_structure;

	__HAL_RCC_GPIOG_CLK_ENABLE();

	/* GPIOG Pin 3 is configured as output */
	gpio_init_structure.Pin = GPIO_PIN_3;
	gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_init_structure.Pull = GPIO_NOPULL;
	gpio_init_structure.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOG, &gpio_init_structure);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_RESET); // Default state of the CLK pin should be RESET meaning drive is OFF
}

uint8_t poll_heater_drive(float set_temperature, float measured_temperature)
{
	static uint8_t drive_status = 0; // Default is OFF

	if (measured_temperature < (set_temperature))
	{
		drive_status = 1; // Drive ON
	}

	if (measured_temperature > (set_temperature + 0.1))
	{
		drive_status = 0; // Drive OFF
	}

	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, drive_status); // Drive ON/OFF

	return drive_status;
}
