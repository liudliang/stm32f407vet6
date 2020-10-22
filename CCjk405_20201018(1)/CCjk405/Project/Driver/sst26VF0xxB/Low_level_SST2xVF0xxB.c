/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/Low_level_SST2xVF0xxB.c 
  * @author  MCD Application Team
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
#include "Low_level_SST2xVF0xxB.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  InitSPI3
  * @param  None
  * @retval None
  */
void InitSPI1(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* Peripheral Clock Enable -------------------------------------------------*/
	/* Enable the SPI clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	/* Enable GPIO clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	/* SPI GPIO Configuration --------------------------------------------------*/
	/* Connect SPI pins to AF */ 
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);    
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	
	/* SPI SCK pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* SPI MISO pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* SPI MOSI pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* SPI configuration -------------------------------------------------------*/
	SPI_I2S_DeInit(SPI1);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_Init(SPI1, &SPI_InitStructure);
	
	SPI_Cmd(SPI1, ENABLE);
}

/**
  * @brief  SPI3_ReadWriteByte.
  * @param  None
  * @retval None
  */
static uint8_t SPI1_ReadWriteByte(uint8_t TxData)
{		 			 
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);   //等待发送区空  
	
	SPI_I2S_SendData(SPI1, TxData);   //通过外设SPIx发送一个byte数据
		
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);  //等待接收完一个byte  
 
	return SPI_I2S_ReceiveData(SPI1); //返回通过SPIx最近接收的数据		    
}

/**
  * @brief  SST2xVF0xxB_CS_HIGH.
  * @param  None
  * @retval None
  */
void SST2xVF0xxB_CS_HIGH(void)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
}

/**
  * @brief  SST2xVF0xxB_CS_LOW.
  * @param  None
  * @retval None
  */
void SST2xVF0xxB_CS_LOW(void)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
}

/**
  * @brief  SST2xVF0xxB_SPI_Send.
  * @param  None
  * @retval None
  */
uint8_t SST2xVF0xxB_SPI_Send(uint8_t Data)
{
	uint8_t RxData = 0;
	
	RxData = SPI1_ReadWriteByte(Data);
	
	return RxData;
}

/**
  * @brief  SST2xVF0xxB_SPI_Transfer.
  * @param  None
  * @retval None
  */
uint8_t SST2xVF0xxB_SPI_Transfer(void)
{
	uint8_t RxData = 0;
	
	RxData = SPI1_ReadWriteByte(0xff);
	
	return RxData;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
