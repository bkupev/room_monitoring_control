/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


//#include "stm32f7xx.h"
//#include "stm32746g_discovery.h"

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "stm32f7xx_hal.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_ts.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_sdram.h"

#include "./custom_drivers/sht11.h"
#include "custom_font.h"
#include "heater_drive.h"
#include "edit_screen.h"

#define ASSERT (void)

#define USE_SHT11

#define MAX_RECORD	250

#define BCK_REG_MAG_NR	0x32F2
/* Defines related to Clock configuration */
#define RTC_ASYNCH_PREDIV  0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   0x00FF /* LSE as RTC clock */

#define	PID_PERIOD		100 // 100 Sec
/**
  * @brief  LCD FB_StartAddress
  * LCD Frame buffer start address : starts at beginning of SDRAM
  */
#define LCD_FRAME_BUFFER          (SDRAM_DEVICE_ADDR)

uint32_t old_ms_counter = 0xFFFFFFFF;
/* RTC handler declaration */
RTC_HandleTypeDef RtcHandle;

/* Buffers used for displaying Time and Date */
uint8_t aShowTime[50] = {0};
uint8_t aShowDate[50] = {0};

#define TS_SETUP_BTN_X		420
#define TS_SETUP_BTN_Y		0

static void RTC_CalendarConfig(void)
{
  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;

  /*##-1- Configure the Date #################################################*/
  /* Set Date: Tuesday February 18th 2015 */
  sdatestructure.Year = 0x15;
  sdatestructure.Month = RTC_MONTH_FEBRUARY;
  sdatestructure.Date = 0x18;
  sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;

  if(HAL_RTC_SetDate(&RtcHandle,&sdatestructure, RTC_FORMAT_BCD) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /*##-2- Configure the Time #################################################*/
  /* Set Time: 02:00:00 */
  stimestructure.Hours = 0x05;
  stimestructure.Minutes = 0x55;
  stimestructure.Seconds = 0x00;
  stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

  if (HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BCD) != HAL_OK)
  {
    /* Initialization Error */
	while(1);
  }

  /*##-3- Writes a data in a RTC Backup data Register0 #######################*/
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR0, 0x32F2);
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 200000000
  *            HCLK(Hz)                       = 200000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 400
  *            PLL_P                          = 2
  *            PLL_Q                          = 8
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  HAL_StatusTypeDef ret = HAL_OK;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  ASSERT(ret != HAL_OK);

  /* activate the OverDrive to reach the 180 Mhz Frequency */
  ret = HAL_PWREx_ActivateOverDrive();
  ASSERT(ret != HAL_OK);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
  ASSERT(ret != HAL_OK);
}

#define TEMP_Y_POSITION		30
#define HUM_Y_POSITION		120
#define TIME_Y_POSITION		210

// Time format abcd - ab hour digits; cd min digits
static void ShowTempHumidityTime(float Temp, float Humid, RTC_TimeTypeDef RTC_Time)
{
	uint32_t StartX;

	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_SetFont(&LCD_DEFAULT_FONT);

	BSP_LCD_DisplayStringAt(0, 0, (uint8_t*)"Temperature:", LEFT_MODE);
	StartX = 0;
	StartX += LCD_DrawChar(StartX, TEMP_Y_POSITION, ((((uint32_t)Temp)%100)/10));
	StartX += LCD_DrawChar(StartX, TEMP_Y_POSITION, ((((uint32_t)Temp)%10)/1));
	StartX += LCD_DrawChar(StartX, TEMP_Y_POSITION, CUSTOM_CHAR_DOT);
	StartX += LCD_DrawChar(StartX, TEMP_Y_POSITION, (((uint32_t)(Temp * 10))%10));
	StartX += LCD_DrawChar(StartX, TEMP_Y_POSITION, CUSTOM_CHAR_DEGREE);
	//Erase residual char pixels
	if (StartX < 199)
	{
		BSP_LCD_SetTextColor(BSP_LCD_GetBackColor());
		BSP_LCD_FillRect(StartX, TEMP_Y_POSITION, (199-StartX), CUSTOM_FONT_HEIGHT);
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	}

	BSP_LCD_DisplayStringAt(0, 90, (uint8_t*)"Humidity:", LEFT_MODE);
	StartX = 0;
	StartX += LCD_DrawChar(StartX, HUM_Y_POSITION, ((((uint32_t)Humid)%100)/10));
	StartX += LCD_DrawChar(StartX, HUM_Y_POSITION, ((((uint32_t)Humid)%10)/1));
	StartX += LCD_DrawChar(StartX, HUM_Y_POSITION, CUSTOM_CHAR_DOT);
	StartX += LCD_DrawChar(StartX, HUM_Y_POSITION, (((uint32_t)(Humid * 10))%10));
	StartX += LCD_DrawChar(StartX, HUM_Y_POSITION, CUSTOM_CHAR_PERCENT);
	//Erase residual char pixels
	if (StartX < 199)
	{
		BSP_LCD_SetTextColor(BSP_LCD_GetBackColor());
		BSP_LCD_FillRect(StartX, TEMP_Y_POSITION, (199-StartX), CUSTOM_FONT_HEIGHT);
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	}

	BSP_LCD_DisplayStringAt(0, 180, (uint8_t*)"Time:", LEFT_MODE);
	StartX = 0;
	StartX += LCD_DrawChar(StartX, TIME_Y_POSITION, (RTC_Time.Hours >> 4));
	StartX += LCD_DrawChar(StartX, TIME_Y_POSITION, (RTC_Time.Hours & 0x0F));
	StartX += LCD_DrawChar(StartX, TIME_Y_POSITION, CUSTOM_CHAR_COLON);
	StartX += LCD_DrawChar(StartX, TIME_Y_POSITION, (RTC_Time.Minutes >> 4));
	StartX += LCD_DrawChar(StartX, TIME_Y_POSITION, (RTC_Time.Minutes & 0x0F));
	//Erase residual char pixels
	if (StartX < 199)
	{
		BSP_LCD_SetTextColor(BSP_LCD_GetBackColor());
		BSP_LCD_FillRect(StartX, TEMP_Y_POSITION, (199-StartX), CUSTOM_FONT_HEIGHT);
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	}
}

static void ShowDiagram (uint32_t MinY, uint32_t MaxY, uint32_t Ypos, float *data_p, uint32_t DataSize)
{
	uint32_t i;
	uint32_t MinY_R, MaxY_R;
	char text[10];

	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DrawVLine(200, 0, 272);
	BSP_LCD_DrawVLine(201, 0, 272);

	BSP_LCD_SetTextColor(BSP_LCD_GetBackColor());
	BSP_LCD_FillRect(210, Ypos-105, 260, 110);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DrawRect(220, Ypos-100, 250, 100);

	MinY_R = 0;
	for (i=0; i < 100; i +=5)
	{
		if (MinY < i) break;
		MinY_R = i;
	}

	MaxY_R = 100;
	for (i = 100; i != 0; i -=5)
	{
		if (MaxY > i) break;
		MaxY_R = i;
	}

	BSP_LCD_SetTextColor(LCD_COLOR_DARKGRAY);
	BSP_LCD_DrawHLine(220, Ypos - 20, 250);
	BSP_LCD_DrawHLine(220, Ypos - 40, 250);
	BSP_LCD_DrawHLine(220, Ypos - 60, 250);
	BSP_LCD_DrawHLine(220, Ypos - 80, 250);

	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_SetFont(&Font12);
	Ypos -=6; // FontSize / 2

	snprintf(text, 10, "%lu", MinY_R);
	BSP_LCD_DisplayStringAt(205, Ypos, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 10, "%lu", (MinY_R + ((1*(MaxY_R - MinY_R))/5)) );
	BSP_LCD_DisplayStringAt(205, Ypos - 20, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 10, "%lu", (MinY_R + ((2*(MaxY_R - MinY_R))/5)) );
	BSP_LCD_DisplayStringAt(205, Ypos - 40, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 10, "%lu", (MinY_R + ((3*(MaxY_R - MinY_R))/5)) );
	BSP_LCD_DisplayStringAt(205, Ypos - 60, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 10, "%lu", (MinY_R + ((4*(MaxY_R - MinY_R))/5)) );
	BSP_LCD_DisplayStringAt(205, Ypos - 80, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 10, "%lu", MaxY_R);
	BSP_LCD_DisplayStringAt(205, Ypos - 100, (uint8_t *)text, LEFT_MODE);

	Ypos += 6;

	for (i = 0; i < DataSize; i++)
	{
		BSP_LCD_FillCircle( (220 + i), (Ypos - ((float)(100/(MaxY_R-MinY_R))*(data_p[i] - MinY_R))), 2);
	}
}


/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}
int main(void)
{
	uint8_t  		lcd_status = LCD_OK;
	uint8_t  		status = 0;
	char			string[20];

	/* Touch Screen state */
	static TS_StateTypeDef  TS_State;

	uint32_t 		count;
	float 			temperature_record[MAX_RECORD];
	float 			rh_record[MAX_RECORD];
	uint32_t 		record_position = 0;
	float 			measured_temperature;
	float			measured_rh;
	uint32_t		OldMin = 0;
	float 			MaxTemp = 0;
	float 			MinTemp = 100;
	float 			MaxRh = 0;
	float 			MinRh = 100;
	RTC_TimeTypeDef	RTC_Time;
	RTC_DateTypeDef	RTC_Date;
	float			SetTemperature = 24.7;
	uint8_t			OldSec = 0;
	uint32_t		PID_Timer = 0;

	/* Enable the CPU Cache */
	CPU_CACHE_Enable();

	HAL_Init();
	SystemClock_Config();
	init_heater_drive();

	 /*##-1- Configure the RTC peripheral #######################################*/
	  /* Configure RTC prescaler and RTC data registers */
	  /* RTC configured as follows:
	      - Hour Format    = Format 24
	      - Asynch Prediv  = Value according to source clock
	      - Synch Prediv   = Value according to source clock
	      - OutPut         = Output Disable
	      - OutPutPolarity = High Polarity
	      - OutPutType     = Open Drain */
	  RtcHandle.Instance = RTC;
	  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
	  RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
	  RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
	  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
	  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

	  if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
	  {
	    /* Initialization Error */
	    while(1);
	  }

	  /*##-2- Check if Data stored in BackUp register0: No Need to reconfigure RTC#*/
	  /* Read the Back Up Register 0 Data */

	  if (HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR0) != 0x32F2)
	  {
	    /* Configure RTC Calendar */
	    RTC_CalendarConfig();
	  }
	  else
	  {
	    /* Clear source Reset Flag */
	    __HAL_RCC_CLEAR_RESET_FLAGS();
	  }

#ifdef USE_SHT11
	// Init SHT11 sensor
	SHT11_Config();
#endif //#ifdef USE_SHT11

	// Initialize TouchScreen
	status = BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
	if (status != TS_OK)
	{
		while(1);
	}

	// Initialise ST LCD Driver
	/* Initialize the LCD */
	lcd_status = BSP_LCD_Init();
	if (lcd_status != LCD_OK)
	{
		while(1);
	}
	/* Initialize the LCD Layers */
	BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FRAME_BUFFER);

	/* Set LCD Foreground Layer  */
	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);

	/* Clear the LCD */
	BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
	BSP_LCD_Clear(LCD_COLOR_BLACK);

	/* Initialize custom font LCD module */
	LCD_SelectFont(CUSTOM_FONT_HEIGHT, CUSTOM_FONT_WIDTH, 5, Tahoma39x49);

	while(1)
	{
#ifdef USE_SHT11
		measured_temperature = SHT11_MeasureTemperature();
		measured_rh = SHT11_Measure_RH();
#else
		measured_temperature = 24.5;
		measured_rh = 78.4;
#endif
		if (MaxTemp < measured_temperature) MaxTemp = measured_temperature;
		if (MinTemp > measured_temperature) MinTemp = measured_temperature;
		if (MaxRh < measured_rh) MaxRh = measured_rh;
		if (MinRh > measured_rh) MinRh = measured_rh;

		HAL_RTC_GetTime(&RtcHandle, &RTC_Time, RTC_FORMAT_BCD);
		/*
		 * Dummy read to have Time shadow registers synced...
		 * bad approach that took me a lot of time find that i should call Date func after Time...
		 */
		HAL_RTC_GetDate(&RtcHandle, &RTC_Date, RTC_FORMAT_BCD);
		(void)RTC_Date;

		ShowTempHumidityTime(measured_temperature, measured_rh, RTC_Time);

		if (RTC_Time.Minutes != OldMin)
		{
			OldMin = RTC_Time.Minutes;
			temperature_record[record_position] = measured_temperature;
			rh_record[record_position] = measured_rh;
			if (record_position == (MAX_RECORD-1))
			{
			 for (count = 0; count < (MAX_RECORD - 1); count++)
			 {
			  temperature_record[count] = temperature_record[count+1];
			  rh_record[count] = rh_record[count+1];
			 }
			}
			else
			{
			 record_position++;
			}

			ShowDiagram (MinTemp, MaxTemp, 140, temperature_record, (record_position - 1));
			ShowDiagram (MinRh, MaxRh, 255, rh_record, (record_position - 1));
		}

		if (RTC_Time.Seconds != OldSec)
		{
			OldSec = RTC_Time.Seconds;
			if (++PID_Timer == PID_PERIOD)
			{
				PID_Timer = 0;
				//poll_pid();
			}
		}
		if (1 == poll_heater_drive(SetTemperature, measured_temperature)) {
			// Heater on
			BSP_LCD_SetTextColor(LCD_COLOR_RED);
		} else {
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		}

		BSP_LCD_FillCircle(420, 10, 7);

		BSP_LCD_SetTextColor(LCD_COLOR_LIGHTCYAN);
		BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
		snprintf(string, 20, "S_TEMP:%.1f", SetTemperature);
		BSP_LCD_DisplayStringAt(215, 0, (uint8_t*)string, LEFT_MODE);

		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
		BSP_LCD_DisplayStringAt(450, 0, (uint8_t*)"@", LEFT_MODE);

		BSP_TS_GetState(&TS_State);
		if(TS_State.touchDetected)
		{
			if ( (TS_SETUP_BTN_X < TS_State.touchX[0]) && ((TS_SETUP_BTN_X + 50) > TS_State.touchX[0]) && \
				 (TS_SETUP_BTN_Y < TS_State.touchY[0]) && ((TS_SETUP_BTN_Y + 50) > TS_State.touchY[0]) )
			{
				EnterEditScreen(RtcHandle, &SetTemperature);

			}
		}
	}

	return(0);
}
