/*
 * edit_screen.c
 *
 *  Created on: 17.11.2016
 *      Author: blagoj.kupev
 */
#include "stm32f7xx_hal.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_ts.h"
#include "stm32746g_discovery_lcd.h"
#include "edit_screen.h"
#include "custom_font.h"

void EnterEditScreen(RTC_HandleTypeDef RtcHandle, float *set_temp_p)
{
	RTC_TimeTypeDef 	Time;
	uint32_t			StartX;
	uint32_t			i;
	uint8_t				ExitEditScreen = 0;
	float				tempSetTemp = *set_temp_p;
	uint8_t				EventCounter;

	/* Touch Screen state */
	static TS_StateTypeDef  TS_State;

	TouchArea			BtnArea[] = {
										/* For the time settings */
										{BTN_TIME_H_DECADE, 0, (TIME_SET_SPACING - 1), 30, 80},
										{BTN_TIME_H_DIGIT, (TIME_SET_SPACING), ((TIME_SET_SPACING * 2) - 1), 30, 80},
										{BTN_TIME_M_DECADE, (TIME_SET_SPACING * 3), ((TIME_SET_SPACING * 4) - 1), 30, 80},
										{BTN_TIME_M_DIGIT, (TIME_SET_SPACING * 4), ((TIME_SET_SPACING * 5) - 1), 30, 80},
										{BTN_ENTER, (TIME_SET_SPACING * 5), ((TIME_SET_SPACING * 6) - 1), 30, 80},
										/* For the temperature setting */
										{BTN_TEMP_DECADE, 0, (TIME_SET_SPACING - 1), 100, 150},
										{BTN_TEMP_DIGIT, (TIME_SET_SPACING), ((TIME_SET_SPACING * 2) - 1), 100, 150},
										{BTN_TEMP_FRACTION, (TIME_SET_SPACING * 3), ((TIME_SET_SPACING * 4) - 1), 100, 150},
										{BTN_TEMP_ENTER, (TIME_SET_SPACING * 4), ((TIME_SET_SPACING * 5) - 1), 100, 150},
									 };

	/* Get the RTC current Time */
	HAL_RTC_GetTime(&RtcHandle, &Time, RTC_FORMAT_BCD);
	Time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	Time.StoreOperation = RTC_STOREOPERATION_SET;


	BSP_LCD_Clear(LCD_COLOR_BLACK);

	while (!ExitEditScreen)
	{
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_DisplayStringAt(0, 0, (uint8_t*)"Time:", LEFT_MODE);
		StartX = 0;
		LCD_DrawChar(StartX, 30, (Time.Hours >> 4));
		StartX += TIME_SET_SPACING;
		LCD_DrawChar(StartX, 30, ( Time.Hours & 0x0F));

		StartX += TIME_SET_SPACING;
		StartX += TIME_SET_SPACING/2;
		LCD_DrawChar(StartX, 30, CUSTOM_CHAR_COLON);
		StartX += TIME_SET_SPACING/2;
		LCD_DrawChar(StartX, 30, (Time.Minutes >> 4));
		StartX += TIME_SET_SPACING;
		LCD_DrawChar(StartX, 30, ( Time.Minutes & 0x0F));
		StartX += TIME_SET_SPACING;
		LCD_DrawChar(StartX, 30, CUSTOM_CHAR_T);
		StartX += TIME_SET_SPACING;
		LCD_DrawChar(StartX, 30, CUSTOM_CHAR_EMPTY);

		// TEMPERATURE
		BSP_LCD_DisplayStringAt(0, 80, (uint8_t*)"Temperature:", LEFT_MODE);
		StartX = 0;
		LCD_DrawChar(StartX, 110, (uint8_t)(tempSetTemp / 10));
		StartX += TIME_SET_SPACING;
		LCD_DrawChar(StartX, 110, ((uint8_t)tempSetTemp) % 10);

		StartX += TIME_SET_SPACING;
		StartX += TIME_SET_SPACING/2;
		LCD_DrawChar(StartX, 110, CUSTOM_CHAR_DOT);
		StartX += TIME_SET_SPACING/2;
		LCD_DrawChar(StartX, 110, ((uint32_t)(tempSetTemp * 10)) % 10);
		StartX += TIME_SET_SPACING;
		LCD_DrawChar(StartX, 110, CUSTOM_CHAR_T);
		StartX += TIME_SET_SPACING;
		LCD_DrawChar(StartX, 110, CUSTOM_CHAR_EMPTY);

//		for (i = 0; i < (sizeof(BtnArea)/sizeof(TouchArea)); i++)
//		{
//			BSP_LCD_DrawLine(BtnArea[i].LeftX, BtnArea[i].TopY, BtnArea[i].RightX, BtnArea[i].TopY);
//			BSP_LCD_DrawLine(BtnArea[i].RightX, BtnArea[i].TopY, BtnArea[i].RightX, BtnArea[i].BottomY);
//			BSP_LCD_DrawLine(BtnArea[i].RightX, BtnArea[i].BottomY, BtnArea[i].LeftX, BtnArea[i].BottomY);
//			BSP_LCD_DrawLine(BtnArea[i].RightX, BtnArea[i].BottomY, BtnArea[i].RightX, BtnArea[i].TopY);
//		}

		BSP_TS_GetState(&TS_State);

		if (TS_State.touchDetected)
		{
			for (EventCounter = 0; EventCounter < TS_State.touchDetected; EventCounter++)
			{
				if (TS_State.touchEventId[EventCounter] == TOUCH_EVENT_CONTACT)
				{
					for (i = 0; i < (sizeof(BtnArea)/sizeof(TouchArea)); i++)
					if ( (BtnArea[i].LeftX < TS_State.touchX[0]) && (BtnArea[i].RightX > TS_State.touchX[0]) && \
						 (BtnArea[i].TopY < TS_State.touchY[0]) && ((BtnArea[i].BottomY) > TS_State.touchY[0]) )
					{
						switch (BtnArea[i].AreaID)
						{
							case BTN_TIME_H_DECADE:
								if ((Time.Hours >> 4) == 2) {
									Time.Hours = (Time.Hours & 0x0F);
								} else {
									Time.Hours = Time.Hours + (1 << 4);
								}
								break;
							case BTN_TIME_H_DIGIT: // Code
								if ((Time.Hours >> 4) == 2)
								{
									if ((Time.Hours & 0x0F) == 3) {
										Time.Hours = (Time.Hours & 0xF0);
									} else {
										Time.Hours = Time.Hours + 1;
									}
								} else {
									if ((Time.Hours & 0x0F) == 9) {
										Time.Hours = (Time.Hours & 0xF0);
									} else {
										Time.Hours = Time.Hours + 1;
									}
								}
								break;
							case BTN_TIME_M_DECADE:
								if ((Time.Minutes >> 4) == 5) {
									Time.Minutes = (Time.Minutes & 0x0F);
								} else {
									Time.Minutes = Time.Minutes + (1 << 4);
								}
								break;
							case BTN_TIME_M_DIGIT:
								if ((Time.Minutes & 0x0F) == 9) {
									Time.Minutes = (Time.Minutes & 0xF0);
								} else {
									Time.Minutes = Time.Minutes + 1;
								}
								break;
							case BTN_ENTER:
								if ((Time.Hours >> 4) == 2)
								{
									if ((Time.Hours & 0x0F) > 3)
									{
										Time.Hours = (Time.Hours & 0xF0);
									}
								}
								Time.Seconds = 0;
								Time.TimeFormat = RTC_HOURFORMAT12_PM;
								Time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
								Time.StoreOperation = RTC_STOREOPERATION_SET;
								HAL_RTC_SetTime(&RtcHandle, &Time, RTC_FORMAT_BCD);
								HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR0, 0x32F2);

								ExitEditScreen = 1;
								break;
							case BTN_CANCEL:
								ExitEditScreen = 1;
								break;

							case BTN_TEMP_DECADE:
								if ((((uint8_t)tempSetTemp)/10) == 4){
									tempSetTemp -= 40;
								}else {
									tempSetTemp += 10;
								}
								break;
							case BTN_TEMP_DIGIT:
								if ((((uint8_t)tempSetTemp) % 10) == 9) {
									tempSetTemp -= 9;
								} else {
									tempSetTemp += 1;
								}
								break;
							case BTN_TEMP_FRACTION:
								if ((((uint32_t)(tempSetTemp * 10)) % 10) == 9) {
									tempSetTemp -= 0.9;
								} else {
									tempSetTemp += 0.1;
								}
								break;
							case BTN_TEMP_ENTER:
								*set_temp_p = tempSetTemp;
								ExitEditScreen = 1;
								break;
							default:
								break;
						}

					}

					BSP_LCD_SetTextColor(LCD_COLOR_LIGHTRED);
					BSP_LCD_FillCircle(TS_State.touchX[0], TS_State.touchY[0], 10);
					for(i=0; i<10000000; i++);
				}
			}
		}
	}
	BSP_LCD_Clear(LCD_COLOR_BLACK);
}


