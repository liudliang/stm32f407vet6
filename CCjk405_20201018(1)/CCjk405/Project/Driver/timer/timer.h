/****************************************Copyright (c)**************************************************
        深圳奥耐电气技术有限公司
				
***FILE：timer.h
****************************************************************************************************/
#ifndef __TIMER_H
#define __TIMER_H


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "typedefs.h"

//#define TIM_10MS  	TIM7 
//#define TIM_50MS  	TIM6 
//#define TIM_100MS  	TIM5 
//#define TIM_250MS  	TIM4 
//#define TIM_500MS  	TIM3 
//#define TIM_1S  	TIM2 
/* CC1口的电压状态 */


extern void TimerInit(void);

extern void TIM2_2sIRQHandler(void);
extern void TIM2_3sIRQHandler(void);
extern void TIM2_5sIRQHandler(void);


#endif


