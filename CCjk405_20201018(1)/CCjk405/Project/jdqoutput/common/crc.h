
/**
  ******************************************************************************
  * @file    crc.h
  * @author  zqj
  * @version v1.0
  * @date    2016-02-25
  * @brief  
  ******************************************************************************
	*/
	
#ifndef _CRC_H_0001
#define _CRC_H_0001
#include "main.h"
#include "includes.h"
	
	/*¼ÆËãcrc16½á¹û*/
//	unsigned short Crc16_Calc_for_screen(unsigned char *updata, unsigned char len);
//	unsigned short Crc16_Calc_for_other(unsigned char *updata, unsigned short len);
	unsigned short Crc16_Calc(unsigned char *updata, unsigned short len);
	
	
	uint32 Calc_CRC32_Crypto(uint8 pStart[], int32 uSize);
	
	

	
#endif
	

