/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/Low_level_MCP2515.h
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
#ifndef __LOW_LEVEL_MCP2515_H
#define __LOW_LEVEL_MCP2515_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdio.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern void MCP2515_CS_HIGH(void);
extern void MCP2515_CS_LOW(void);
extern void MCP2515_SPI_Send(uint8_t value);
extern uint8_t MCP2515_SPI_Receive(void);
extern void MCP2515_delay(void);
extern void InitSPI2(void);
extern void InitEXTIPB1(void);
	
#endif /* __LOW_LEVEL_MCP2515_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
