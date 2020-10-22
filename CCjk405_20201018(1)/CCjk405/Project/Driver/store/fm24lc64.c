/**
  ******************************************************************************
  * @file    DrvFM24CLxx.c 
  * @author  Aunice Application Team
  * @version V1.4.0
  * @date    04-August-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "fm24lc64.h"
#include "stm324xg_eval_i2c_ee.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** 
  * @brief  fm24lc_init 
  * @param  A: 
  * @retval None 
  */  
void fm24lc_init(void)  
{     
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//FM24CLxx_WP口
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_WriteBit(GPIOB, GPIO_Pin_14, Bit_SET);   /* 高保护 */
	
	sEE_DeInit();
	sEE_Init();
} 

/**
  * @brief  FM24CLxx_ReadBuffer.
  * @param  None.
  *         
  * @retval None.
  */
uint32_t FM24CLxx_ReadBuffer(uint8_t *pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead)
{
	uint32_t status = 0;
	
	status = sEE_ReadBuffer(pBuffer, ReadAddr, NumByteToRead);
	
	return status;
}

/**
  * @brief  FM24CLxx_WriteBuffer.
  * @param  None.
  *         
  * @retval None.
  */
void FM24CLxx_WriteBuffer(uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite)
{
	//FM24CLxx_WP口
	GPIO_WriteBit(GPIOB, GPIO_Pin_14, Bit_RESET);
	
	sEE_WriteBuffer(pBuffer, WriteAddr, NumByteToWrite);
	
	GPIO_WriteBit(GPIOB, GPIO_Pin_14, Bit_SET);  /* 高保护 */
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
