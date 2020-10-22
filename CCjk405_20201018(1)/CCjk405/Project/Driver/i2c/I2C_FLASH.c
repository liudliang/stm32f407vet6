/****************************************Copyright (c)**************************************************
        深圳奥耐电气技术有限公司
				
***FILE：I2C_FLASH.c
****************************************************************************************************/
#include "fm24lc64.h"
#include "stm324xg_eval_i2c_ee.h"
#include "I2C_FLASH.h"

/**
  * @brief  I2C_ReadNbyte.
  * @param  None.
  *         
  * @retval None.
  */
uint8 I2C_ReadNbyte(uint8 *recv, uint16 adr, uint16 length)
{
	return (sEE_OK == FM24CLxx_ReadBuffer(recv, adr, length)) ? I2C_TRUE : I2C_FALSE;
}

/**
  * @brief  I2C_WriteNbyte.
  * @param  None.
  *         
  * @retval None.
  */
uint8 I2C_WriteNbyte(uint8 *src, uint16 adr, uint16 length)
{
	FM24CLxx_WriteBuffer(src, adr, length);
	
	return I2C_TRUE;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
