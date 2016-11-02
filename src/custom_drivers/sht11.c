/*
 * sht11.c
 *
 *  Created on: 13.5.2016
 *      Author: blagoj.kupev
 */
#include "stm32f7xx_hal.h"
#include "sht11.h"
#include "main.h"

/* TIM handle declaration */
TIM_HandleTypeDef    TimHandle;

uint32_t ms_counter = 0;
uint32_t us_counter = 0;

void ConfigTimer(void)
{
  uint32_t uwPrescalerValue;

  /*##-1- Configure the TIM peripheral #######################################*/
  /* -----------------------------------------------------------------------
	In this example TIM3 input clock (TIM3CLK)  is set to APB1 clock (PCLK1) x2,
	since APB1 prescaler is equal to 4.
	  TIM3CLK = PCLK1*2
	  PCLK1 = HCLK/4
	  => TIM3CLK = HCLK/2 = SystemCoreClock/2
	To get TIM3 counter clock at 10 KHz, the Prescaler is computed as follows:
	Prescaler = (TIM3CLK / TIM3 counter clock) - 1
	Prescaler = ((SystemCoreClock/2) /1 KHz) - 1

	Note:
	 SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f7xx.c file.
	 Each time the core clock (HCLK) changes, user had to update SystemCoreClock
	 variable value. Otherwise, any configuration based on this variable will be incorrect.
	 This variable is updated in three ways:
	  1) by calling CMSIS function SystemCoreClockUpdate()
	  2) by calling HAL API function HAL_RCC_GetSysClockFreq()
	  3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
  ----------------------------------------------------------------------- */

  /* Set TIMx instance */
  TimHandle.Instance = TIM3;
  SystemCoreClockUpdate();

  /* Initialize TIMx peripheral as follows:
	   + Period = 1000 - 1
	   + Prescaler = ((SystemCoreClock / 2)/1000000) - 1
	   + ClockDivision = 0
	   + Counter direction = Up
  */

  /* Compute the prescaler value to have TIMx counter clock equal to 1000 Hz */
  uwPrescalerValue = (uint32_t)((SystemCoreClock / 2) / 1000) - 1;

  TimHandle.Init.Period            = 1000 - 1;
  TimHandle.Init.Prescaler         = uwPrescalerValue;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimHandle.Init.RepetitionCounter = 0;

  if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
  {
	/* Initialization Error */
	while(1);
  }

  /*##-2- Start the TIM Base generation in interrupt mode ####################*/
  /* Start Channel1 */
  if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
  {
	/* Starting Error */
	while(1);
  }
}

/* User can use this section to tailor TIMx instance used and associated
   resources */
/* Definition for TIMx clock resources */
#define TIMx                           TIM3
#define TIMx_CLK_ENABLE()              __HAL_RCC_TIM3_CLK_ENABLE()


/**
  * @brief TIM MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  * @param htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  /*##-1- Enable peripheral clock #################################*/
  /* TIMx Peripheral clock enable */
  TIMx_CLK_ENABLE();

  /*##-2- Configure the NVIC for TIMx ########################################*/
  /* Set the TIMx priority */
  HAL_NVIC_SetPriority(TIMx_IRQn, 3, 0);

  /* Enable the TIMx global Interrupt */
  HAL_NVIC_EnableIRQ(TIMx_IRQn);
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  us_counter++;
}

void SHT11_Config(void)
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

  //ConfigTimer();
}

static void delay_us(uint32_t us_delay)
{
	uint32_t cnt;
	for (cnt = 0; cnt < 1000; cnt++);
	return;
	us_counter = 0;
	while (us_counter != us_delay);
}

static void delay_ms(uint32_t ms_delay)
{
	uint32_t cnt;
	for (cnt = 0; cnt < 1000000; cnt++);
	return;

	us_counter = 0;
	while ((us_counter/1000) != ms_delay);
}



static uint8_t SHT11_ReadDataPin(void)
{
	return (HAL_GPIO_ReadPin(SHT11_DATA_PORT, SHT11_DATA_PIN));
}
void SHT11_SetPin(uint8_t pin_select)
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

static uint32_t SHT11_ReadBits(uint8_t nr_bits)
{
	uint8_t clk_cnt;
	uint32_t data_out = 0;
	uint8_t crc = 0;

	SHT11_SetPin(SHT11_DATA); // Keep pin released for reading

	// Read MSB bits as result
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

	// ACK first 8 bits
	SHT11_ClearPin(SHT11_DATA);
	SHT11_SetPin(SHT11_CLK);
	CLK_TIM;
	SHT11_ClearPin(SHT11_CLK);
	CLK_TIM;

	SHT11_SetPin(SHT11_DATA); // Keep pin released for reading
	// Read LSB bits as result
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

	// Do not ACK so no CRC is received
	// Set pins to default state
	SHT11_SetPin(SHT11_DATA);
	SHT11_ClearPin(SHT11_CLK);
	return (data_out);
}


static uint32_t SHT11_Measure(uint8_t cmd)
{
	uint32_t timer;
	uint32_t result;

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
			// Read 16 bits as result
			return (SHT11_ReadBits(16));
		}
		//timer++;
	} while(timer < 1000);

	// Time out on waiting measurement to end
	// Set data and clk pins to default state
	SHT11_ClearPin(SHT11_CLK);
	SHT11_SetPin(SHT11_DATA);

	return(0xFFFFFFF0);
}

double SHT11_Measure_RH(void)
{
	double RH_Linear, RH_True, RH_Captured;
	RH_Captured = SHT11_Measure(CMD_MEASURE_R_H);

	RH_Linear = -2.0468 + (0.0367 * RH_Captured) + (-1.5955E-6 * (RH_Captured * RH_Captured));
	RH_True = ((SHT11_MeasureTemperature() - 25) * (0.01 + (0.00008 * RH_Captured))) + RH_Linear;

	return (RH_True);
}

double SHT11_MeasureTemperature(void)
{
	return (-39.65+(0.01*SHT11_Measure(CMD_MEASURE_TEMP)));
}

uint32_t SHT11_ReadStatus(void)
{
	SHT11_TransmissionStart();
	if (SHT11_SendCMD(CMD_RD_STATUS) != 0)
	{
		//Command not received by sensor
		return (0xFFFFFFFF);
	}

	return (SHT11_ReadBits(8));
}

uint32_t SHT11_WriteStatus(uint8_t new_value)
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


