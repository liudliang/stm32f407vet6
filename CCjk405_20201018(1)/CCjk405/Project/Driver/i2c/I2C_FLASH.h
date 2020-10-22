/****************************************Copyright (c)**************************************************
        深圳奥耐电气技术有限公司
				
***FILE：I2C_FLASH.h
****************************************************************************************************/
#ifndef __I2C_FLASH_H
#define __I2C_FLASH_H

/* Exported variables --------------------------------------------------------*/ 
#define I2C_TRUE   1
#define I2C_FALSE  0

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "typedefs.h"

/*写入N个字节到E2PROM中*/
extern uint8 I2C_WriteNbyte(uint8 *src, uint16 adr, uint16 length);

/*从E2PROM读取N个字节*/
extern uint8 I2C_ReadNbyte(uint8 *recv, uint16 adr, uint16 length);

#endif


