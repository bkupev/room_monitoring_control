/*
 * edit_screen.h
 *
 *  Created on: 17.11.2016
 *      Author: blagoj.kupev
 */

#ifndef EDIT_SCREEN_H_
#define EDIT_SCREEN_H_

#define BTN_TIME_H_DECADE	1
#define BTN_TIME_H_DIGIT	2
#define BTN_TIME_M_DECADE	3
#define BTN_TIME_M_DIGIT	4

#define BTN_ENTER			5
#define BTN_CANCEL			6

#define BTN_TEMP_DECADE		7
#define BTN_TEMP_DIGIT		8
#define BTN_TEMP_FRACTION	9
#define	BTN_TEMP_ENTER		10


#define TIME_SET_SPACING	40

typedef struct {
	uint32_t	AreaID;
	uint32_t	LeftX;
	uint32_t	RightX;
	uint32_t	TopY;
	uint32_t	BottomY;
}TouchArea;

void EnterEditScreen(RTC_HandleTypeDef RtcHandle, float *set_temp_p);

#endif /* EDIT_SCREEN_H_ */
