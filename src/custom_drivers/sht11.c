/*
 * sht11.c
 *
 *  Created on: 13.5.2016
 *      Author: blagoj.kupev
 */
#include "stm32f7xx_hal.h"
#include "main.h"
#include "sensor_hal.h"
#include "sht11.h"

/* TIM handle declaration */
TIM_HandleTypeDef    TimHandle;

uint32_t ms_counter = 0;
uint32_t us_counter = 0;

static uint8_t sht11_status_reg = 0; // Initialized to SHT11 default value
const uint8_t CRC_Table[256] = {0, 49, 98, 83, 196, 245, 166, 151, 185, 136, \
								219, 234, 125, 76, 31, 46, 67, 114, 33, 16, \
								135, 182, 229, 212, 250, 203, 152, 169, 62, \
								15, 92, 109, 134, 183, 228, 213, 66, 115, \
								32, 17, 63, 14, 93,108, 251, 202, 153, 168,\
								197, 244, 167, 150, 1, 48, 99, 82, 124, 77, \
								30, 47, 184, 137, 218, 235, 61, 12, 95, 110, \
								249, 200, 155, 170, 132, 181, 230, 215, 64, \
								113, 34, 19, 126, 79, 28, 45, 186, 139, 216, \
								233, 199, 246, 165, 148, 3, 50, 97, 80, 187, \
								138, 217, 232, 127, 78, 29, 44, 2, 51, 96, 81,\
								198, 247, 164, 149, 248, 201, 154, 171, 60, 13,\
								94, 111, 65, 112, 35, 18, 133, 180, 231, 214, \
								122, 75, 24, 41, 190, 143, 220, 237, 195, 242, \
								161, 144, 7, 54, 101, 84, 57, 8, 91, 106, 253, \
								204, 159, 174, 128, 177, 226, 211, 68, 117, 38, \
								23, 252, 205, 158, 175, 56, 9, 90, 107, 69, 116, \
								39, 22, 129, 176, 227, 210, 191, 142, 221, 236, \
								123, 74, 25, 40, 6, 55, 100, 85, 194, 243, 160, \
								145, 71, 118, 37, 20, 131, 178, 225, 208, 254, 207, \
								156, 173, 58, 11, 88, 105, 4, 53, 102, 87, 192, 241,\
								162, 147, 189, 140, 223, 238, 121, 72, 27, 42, 193,\
								240, 163, 146, 5, 52, 103, 86, 120, 73, 26, 43, \
								188, 141, 222, 239, 130, 179, 224, 209, 70, 119, \
								36, 21, 59, 10, 89, 104, 255, 206, 157, 172};

/*
 * ************************************************
 * ************************************************
 * HW Abstraction for SHT communication pins START
 * ************************************************
 * ************************************************
 */
static void SHT11_Config(void)
{
	GPIO_InitTypeDef gpio_init_structure;

  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* GPIOB Pin 8 is configured as CLK */
  gpio_init_structure.Pin = SHT11_CLK_PIN;
  gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(SHT11_CLK_PORT, &gpio_init_structure);
  HAL_GPIO_WritePin(SHT11_CLK_PORT, SHT11_CLK_PIN, GPIO_PIN_RESET); // Default state of the CLK pin should be RESET

  /* GPIOB Pin 9 is configured as DATA */
  gpio_init_structure.Pin = SHT11_DATA_PIN;
  gpio_init_structure.Mode = GPIO_MODE_OUTPUT_OD;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(SHT11_DATA_PORT, &gpio_init_structure);
  HAL_GPIO_WritePin(SHT11_DATA_PORT, SHT11_DATA_PIN, GPIO_PIN_SET); // Default state of the CLK pin should be SET


  gpio_init_structure.Pin = GPIO_PIN_1;
  gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOI, &gpio_init_structure);
  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_SET); // Default state of the CLK pin should be SET

  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_RESET); // Default state of the CLK pin should be SET

}

static void delay_us(uint32_t us_delay)
{
	uint32_t cnt;
	for (cnt = 0; cnt < 1000; cnt++);
	return;
	us_counter = 0;
	while (us_counter != us_delay);
}

static uint8_t SHT11_ReadDataPin(void)
{
	return (HAL_GPIO_ReadPin(SHT11_DATA_PORT, SHT11_DATA_PIN));
}

static void SHT11_SetPin(uint8_t pin_select)
{
	if (pin_select == SHT11_DATA)
	{
		HAL_GPIO_WritePin(SHT11_DATA_PORT, SHT11_DATA_PIN, GPIO_PIN_SET);
	} else if (pin_select == SHT11_CLK)
	{
		HAL_GPIO_WritePin(SHT11_CLK_PORT, SHT11_CLK_PIN, GPIO_PIN_SET);
	} else
	{
		while(1);
	}
}

static void SHT11_ClearPin(uint8_t pin_select)
{
	if (pin_select == SHT11_DATA)
	{
		HAL_GPIO_WritePin(SHT11_DATA_PORT, SHT11_DATA_PIN, GPIO_PIN_RESET);
	} else if (pin_select == SHT11_CLK)
	{
		HAL_GPIO_WritePin(SHT11_CLK_PORT, SHT11_CLK_PIN, GPIO_PIN_RESET);
	} else
	{
		while(1);
	}
}
/*
 * ************************************************
 * ************************************************
 * HW Abstraction for SHT communication pins END
 * ************************************************
 * ************************************************
 */

static uint8_t SHT11_Ack(void)
{
	uint8_t ret_val;

	SHT11_SetPin(SHT11_DATA); // Release the pin and read value set by the sensor
	SHT11_ClearPin(SHT11_CLK);

	// Execute ACK sequence
	delay_us(10);
	SHT11_SetPin(SHT11_CLK);
	delay_us(10);
	ret_val = SHT11_ReadDataPin();
	delay_us(10);
	SHT11_ClearPin(SHT11_CLK);
	return ret_val;
}

static void SHT11_TransmissionStart (void)
{
	uint8_t count;

	SHT11_SetPin(SHT11_DATA);
	SHT11_ClearPin(SHT11_CLK);

	delay_us(10);

	/* Clock at least 9 pulses on CLK line */
	for (count = 0; count < 12; count++)
	{
		delay_us(10);
		SHT11_SetPin(SHT11_CLK);
		delay_us(10);
		SHT11_ClearPin(SHT11_CLK);
	}

	/* initiate Transmission Start sequence */
	delay_us(10);
	SHT11_SetPin(SHT11_CLK);

	delay_us(20);
	SHT11_ClearPin(SHT11_DATA);

	delay_us(10);
	SHT11_ClearPin(SHT11_CLK);
	delay_us(10);
	SHT11_SetPin(SHT11_CLK);

	delay_us(20);
	SHT11_SetPin(SHT11_DATA);
	delay_us(10);
	SHT11_ClearPin(SHT11_CLK);
	delay_us(10);
}

#define	CLK_TIM	delay_us(50)
static uint8_t SHT11_SendCMD(uint8_t cmd)
{
	uint8_t count;

	SHT11_ClearPin(SHT11_DATA);
	delay_us(10);

	for (count = 0; count < 8; count++)
	{
		if ((cmd & 0x80) != 0){
			SHT11_SetPin(SHT11_DATA);
		} else {
			SHT11_ClearPin(SHT11_DATA);
		}
		cmd = cmd << 1;
		CLK_TIM;
		SHT11_SetPin(SHT11_CLK);
		CLK_TIM;
		SHT11_ClearPin(SHT11_CLK);
		CLK_TIM;
	}

	return(SHT11_Ack());
}

static uint8_t reverse_byte(uint8_t a)
{
  return ((a & 0x1)  << 7) | ((a & 0x2)  << 5) |
         ((a & 0x4)  << 3) | ((a & 0x8)  << 1) |
         ((a & 0x10) >> 1) | ((a & 0x20) >> 3) |
         ((a & 0x40) >> 5) | ((a & 0x80)  >> 7);
}

static uint8_t crc_calc(uint8_t config_reg, uint8_t adr_cmd, uint32_t recv_bytes_in_word)
{
	uint8_t b0, b1;
	uint8_t res = config_reg;

	recv_bytes_in_word >>= 8;

	b0 = recv_bytes_in_word & 0x000000FF;
	recv_bytes_in_word >>= 8;

	b1 = recv_bytes_in_word & 0x000000FF;
	recv_bytes_in_word >>= 8;

	res ^= adr_cmd;
	res = CRC_Table[res];

	res ^= b1;
	res = CRC_Table[res];

	res ^= b0;
	res = CRC_Table[res];

	return reverse_byte(res);
}

static uint32_t SHT11_ReadBits(uint8_t nr_bits)
{
	uint8_t clk_cnt;
	uint32_t data_out = 0;

	SHT11_SetPin(SHT11_DATA); // Keep pin released for reading

	// Read first 8 bits from the result
	for (clk_cnt = 0; clk_cnt < 8; clk_cnt++)
	{
		data_out = data_out << 1;
		SHT11_SetPin(SHT11_CLK);
		CLK_TIM;
		if (1 == SHT11_ReadDataPin()){
			data_out = data_out + 1;
		}
		SHT11_ClearPin(SHT11_CLK);
		CLK_TIM;
	}

	if ((nr_bits == 16) || (nr_bits == 24))
	{
		// ACK first 8 bits
		SHT11_ClearPin(SHT11_DATA);
		SHT11_SetPin(SHT11_CLK);
		CLK_TIM;
		SHT11_ClearPin(SHT11_CLK);
		CLK_TIM;

		SHT11_SetPin(SHT11_DATA); // Keep pin released for reading
		// Read second 8 bits as result
		for (clk_cnt = 0; clk_cnt < 8; clk_cnt++)
		{
			data_out = data_out << 1;
			SHT11_SetPin(SHT11_CLK);
			CLK_TIM;
			if (1 == SHT11_ReadDataPin()){
				data_out = data_out + 1;
			}
			SHT11_ClearPin(SHT11_CLK);
			CLK_TIM;
		}
	}

	if (nr_bits == 24)
	{
		// ACK second 8 bits
		SHT11_ClearPin(SHT11_DATA);
		SHT11_SetPin(SHT11_CLK);
		CLK_TIM;
		SHT11_ClearPin(SHT11_CLK);
		CLK_TIM;

		SHT11_SetPin(SHT11_DATA); // Keep pin released for reading
		// Read third 8 bits
		for (clk_cnt = 0; clk_cnt < 8; clk_cnt++)
		{
			data_out = data_out << 1;
			SHT11_SetPin(SHT11_CLK);
			CLK_TIM;
			if (1 == SHT11_ReadDataPin()){
				data_out = data_out + 1;
			}
			SHT11_ClearPin(SHT11_CLK);
			CLK_TIM;
		}

		// ACK third bits
		SHT11_ClearPin(SHT11_DATA);
		SHT11_SetPin(SHT11_CLK);
		CLK_TIM;
		SHT11_ClearPin(SHT11_CLK);
		CLK_TIM;
	}

	// Set pins to default state
	SHT11_SetPin(SHT11_DATA);
	SHT11_ClearPin(SHT11_CLK);

	return (data_out);
}


static uint32_t SHT11_Measure(uint8_t cmd)
{
	uint32_t timer;
	uint32_t read_word;
	uint8_t crc;

	SHT11_TransmissionStart();
	if (SHT11_SendCMD(cmd) != 0)
	{
		//Command not received by sensor
		return (0xFFFFFFFF);
	}

	// Wait for data pin to become on signalling that the measurement process has ended
	delay_us(100);
	SHT11_SetPin(SHT11_DATA);
	timer = 0;
	do {
		delay_us(50);
		if (0 == SHT11_ReadDataPin())
		{
			// Data pin pulled low by sensor, measurement is over
			// Read 16 bits as result and crc
			read_word = SHT11_ReadBits(24);
			crc = crc_calc(sht11_status_reg, cmd, read_word);
			if (crc == (read_word & 0xFF))			{
				return (read_word >> 8);
			} else {
				return (0xFFFFFFFF);
			}
		}
		//timer++;
	} while(timer < 1000);

	// Time out on waiting measurement to end
	// Set data and clk pins to default state
	SHT11_ClearPin(SHT11_CLK);
	SHT11_SetPin(SHT11_DATA);

	return(0xFFFFFFF0);
}

static double SHT11_MeasureTemperature(void)
{
	uint32_t temp_res;

	temp_res = SHT11_Measure(CMD_MEASURE_TEMP);

	if (temp_res == 0xFFFFFFFF) {
		return (99.9);
	} else {
		return (-39.65+(0.01*temp_res));
	}
}

static double SHT11_Measure_RH(void)
{
	double RH_Linear, RH_True, RH_Captured;
	uint32_t temp_res;

	temp_res = SHT11_Measure(CMD_MEASURE_R_H);

	if (temp_res != 0xFFFFFFFF) {
		RH_Captured = temp_res;
	} else {
		RH_Captured = 99;
	}

	RH_Linear = -2.0468 + (0.0367 * RH_Captured) + (-1.5955E-6 * (RH_Captured * RH_Captured));
	RH_True = ((SHT11_MeasureTemperature() - 25) * (0.01 + (0.00008 * RH_Captured))) + RH_Linear;

	return (RH_True);
}

static uint32_t SHT11_ReadStatus(void)
{
	SHT11_TransmissionStart();
	if (SHT11_SendCMD(CMD_RD_STATUS) != 0)
	{
		//Command not received by sensor
		return (0xFFFFFFFF);
	}

	return (SHT11_ReadBits(8));
}

static uint32_t SHT11_WriteStatus(uint8_t new_value)
{
	SHT11_TransmissionStart();
	if (SHT11_SendCMD(CMD_WR_STATUS) != 0)
	{
		//Command not received by sensor
		return (0xFFFFFFFF);
	}
	// Writing value is the same as the command
	if (SHT11_SendCMD(new_value) != 0)
	{
		//Command not received by sensor
		return (0xFFFFFFFF);
	}

	return (0); //SUCCESS
}

static uint8_t sht11_init(void **ctx_pp)
{
	*ctx_pp = (void*)0;
	SHT11_Config();
	return 0;
}

static double sht11_get_value(void **ctx_pp, param_t parameter)
{
	switch (parameter)
	{
		case PARAM_TEMPERATURE: return SHT11_MeasureTemperature();
		case PARAM_HUMIDITY: return SHT11_Measure_RH();
		default: return (99);// displays error state
	}
}

ext_sensor_t sht11_sensor = {
	sht11_init,
	sht11_get_value,
	(void*)(0),
};
