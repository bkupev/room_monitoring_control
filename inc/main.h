/*
 * main.h
 *
 *  Created on: 27.5.2016
 *      Author: blagoj.kupev
 */

#ifndef MAIN_H_
#define MAIN_H_

#define USE_SHT11

/* TIM handle declaration */
extern TIM_HandleTypeDef    TimHandle;
/* Definition for TIMx's NVIC */
#define TIMx_IRQn                      TIM3_IRQn
#define TIMx_IRQHandler                TIM3_IRQHandler

#endif /* MAIN_H_ */
