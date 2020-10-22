/**
  ******************************************************************************
  * @file    DrvSST26VF0xxB.h 
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    04-August-2014
  * @brief   Header for main.c module
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRVSST26VF0XXB_H
#define __DRVSST26VF0XXB_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "Low_level_SST2xVF0xxB.h"

/* Exported variables --------------------------------------------------------*/ 
#define SST_BLOCK_SIZE        4096
#define SST_CAPACITY_SIZE     (4 * 1024 * 1024)

/* Exported functions --------------------------------------------------------*/ 
uint32_t SST26VF0xxB_JEDEC(void);
void SST26VF0xxB_Read(uint8_t *pdata, uint32_t addr, uint32_t nbyte);
uint8_t SST26VF0xxB_Write(uint8_t *pdata, uint32_t addr, uint32_t nbyte);
uint8_t SST26VF0xxB_Page_Write(uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite);
uint8_t SST26VF0xxB_ChipErase(void);
uint8_t SST26VF0xxB_4KByteSectorErase(uint32_t addr);
uint8_t SST26VF0xxB_RemoveBlockProtection(void);

#endif /* __DRVSST26VF0XXB_H */

/**
  * @}
  */

/**
  * @}
  */
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

