/****************************************Copyright (c)**************************************************
        深圳奥耐电气技术有限公司
				
***FILE：MeaureGunTemperature.h
****************************************************************************************************/
#ifndef __MEAUREGUNTEMPERATURE_H
#define __MEAUREGUNTEMPERATURE_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "typedefs.h"

extern int32_t GetCD4051BMTChannelTemper(uint8_t ch);

extern void SetCD4051BMTChannel(uint8_t ch);


#endif
