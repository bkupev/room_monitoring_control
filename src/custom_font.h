/*
 * custom_font.h
 *
 *  Created on: 26.6.2016
 *      Author: blagoj.kupev
 */

#ifndef CUSTOM_FONT_H_
#define CUSTOM_FONT_H_

//extern uint8_t Tahoma37x50[];
extern uint8_t Tahoma39x49[];

#define CUSTOM_CHAR_T 			10
#define CUSTOM_CHAR_H 			11
#define CUSTOM_CHAR_COLON		12
#define CUSTOM_CHAR_PERCENT		13
#define CUSTOM_CHAR_DEGREE		14
#define CUSTOM_CHAR_DOT			15
#define CUSTOM_CHAR_EMPTY		16

#define	CUSTOM_FONT_HEIGHT		49
#define	CUSTOM_FONT_WIDTH		39

void LCD_SelectFont(uint32_t FontHeight, uint32_t FontWidth, uint32_t FontSpacing,uint8_t *FontMatrix_p);
uint32_t LCD_DrawChar(uint32_t Xpos, uint32_t Ypos, uint8_t nr_to_display);
uint32_t LCD_DrawTextLine(uint32_t Xpos, uint32_t Ypos, uint8_t *Text_p, uint8_t NrCharsToDraw);

#endif /* CUSTOM_FONT_H_ */
