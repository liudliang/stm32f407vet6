/****************************************Copyright (c)**************************************************
        深圳奥耐电气技术有限公司
				
***FILE：SST_FLASH.h
****************************************************************************************************/
#ifndef __SST_FLASH_H
#define __SST_FLASH_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "typedefs.h"
#include "SST26VF0xxB.h"

/* Exported variables --------------------------------------------------------*/ 
#define SST_TRUE   1
#define SST_FALSE  0

extern void sst_init(void);
extern uint8 Flash_BufferRead(uint8 *recv, uint32 adr, uint16 size);
extern uint8 Flash_BufferWrite(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);
extern void Flash_SectorErase(uint32 adr);

#endif


