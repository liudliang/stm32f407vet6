/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/SST26VF0xxB.c 
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
#include "SST26VF0xxB.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define SST_PAGESIZE      256

/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static uint8_t SST26VF0xxB_ByteProgram(uint32_t addr, uint8_t data);
static uint8_t SST26VF0xxB_PageProgram(uint8_t *pdata, uint32_t addr, uint32_t nbyte);
static void SST26VF0xxB_WriteEnable(void);
static void SST26VF0xxB_WriteDisable(void);
static uint8_t SST26VF0xxB_RDSR(void);

/**
  * @brief  SST26VF0xxB_RemoveBlockProtection.
  * @param  None.
  *         
  * @retval None.
  */
uint8_t SST26VF0xxB_RemoveBlockProtection(void)
{
	uint8_t status = 0;
	uint32_t TIMEOUT  = 0xA380; 
	
	SST26VF0xxB_WriteEnable();
	
	SST2xVF0xxB_CS_LOW();			      
	SST2xVF0xxB_SPI_Send(0x98); 		
	SST2xVF0xxB_CS_HIGH();	
	
	do
	{
		status = SST26VF0xxB_RDSR();
		TIMEOUT--;
	} while (((status & 0x01) == 0x01) && (TIMEOUT != 0));
	
	return (0 == TIMEOUT) ? 0 : 1;
}

/**
  * @brief  SST26VF0xxB_JEDEC.
  * @param  None.
  *         
  * @retval None.
  */
uint32_t SST26VF0xxB_JEDEC(void)
{
	uint32_t JEDEC_ID = 0;
	uint8_t  *pdata = NULL;
  uint8_t  byte1 = 0;            
	uint8_t  byte2 = 0;
	uint8_t  byte3 = 0;
	
	SST2xVF0xxB_CS_LOW();
	SST2xVF0xxB_SPI_Send(0x9F);

	byte1 = SST2xVF0xxB_SPI_Transfer();
	byte2 = SST2xVF0xxB_SPI_Transfer();
	byte3 = SST2xVF0xxB_SPI_Transfer();
	
	SST2xVF0xxB_CS_HIGH();
	
	pdata = (uint8_t *)&JEDEC_ID;
	
	*pdata++ = byte1;
	*pdata++ = byte2;
	*pdata++ = byte3;
	
	return JEDEC_ID;
}

/**
  * @brief  SST26VF0xxB_ChipErase.
  * @param  None.
  *         
  * @retval None.
  */
uint8_t SST26VF0xxB_ChipErase(void)
{
	uint8_t status = 0;
	uint32_t TIMEOUT  = 0xA380; 
	
	SST26VF0xxB_WriteEnable(); //Write-Enable 
	
	/* Chip-Erase */
	SST2xVF0xxB_CS_LOW();
	SST2xVF0xxB_SPI_Send(0xC7);
	SST2xVF0xxB_CS_HIGH();
	
	do
	{
		status = SST26VF0xxB_RDSR();
		TIMEOUT--;
	} while (((status & 0x01) == 0x01) && (TIMEOUT != 0));
	
	SST26VF0xxB_WriteDisable();
	
	return (0 == TIMEOUT) ? 0 : 1;
}

/**
  * @brief  SST26VF0xxB_Read.
  * @param  None.
  *         
  * @retval None.
  */
void SST26VF0xxB_Read(uint8_t *pdata, uint32_t addr, uint32_t nbyte)
{
  uint8_t addr1 = 0;
	uint8_t addr2 = 0;
	uint8_t addr3 = 0;
	
	addr1 = (uint8_t)(addr >> 16);
	addr2 = (uint8_t)(addr >> 8);
	addr3 = (uint8_t)addr;
	
	/* Read (25 MHz) */
	SST2xVF0xxB_CS_LOW();
	SST2xVF0xxB_SPI_Send(0x03);
	
	SST2xVF0xxB_SPI_Send(addr1);
	SST2xVF0xxB_SPI_Send(addr2);
	SST2xVF0xxB_SPI_Send(addr3);
	
	while (nbyte)
	{
		*pdata = SST2xVF0xxB_SPI_Transfer();
		
		pdata++;
		nbyte--;
	}

	SST2xVF0xxB_CS_HIGH();
}

/**
  * @brief  SST26VF0xxB_ByteProgram.
  * @param  None.
  *         
  * @retval None.
  */
static uint8_t SST26VF0xxB_ByteProgram(uint32_t addr, uint8_t data)
{
	uint8_t status = 0;
	
	uint8_t addr1 = 0;
	uint8_t addr2 = 0;
	uint8_t addr3 = 0;
	
	uint32_t TIMEOUT  = 0xA380;
	
	addr1 = (uint8_t)(addr >> 16);
	addr2 = (uint8_t)(addr >> 8);
	addr3 = (uint8_t)addr;
	
	SST26VF0xxB_WriteEnable(); //Write-Enable 
	
	/* Byte-Program */
	SST2xVF0xxB_CS_LOW();
	SST2xVF0xxB_SPI_Send(0x02);
	
	SST2xVF0xxB_SPI_Send(addr1);
	SST2xVF0xxB_SPI_Send(addr2);
	SST2xVF0xxB_SPI_Send(addr3);
	
	SST2xVF0xxB_SPI_Send(data);
	
	SST2xVF0xxB_CS_HIGH();
	
	do
	{
		status = SST26VF0xxB_RDSR();
		TIMEOUT--;
	} while (((status & 0x01) == 0x01) && (TIMEOUT != 0));
	
	return (0 == TIMEOUT) ? 0 : 1;
}

/**
  * @brief  SST26VF0xxB_Write.
  * @param  None.
  *         
  * @retval None.
  */
uint8_t SST26VF0xxB_Write(uint8_t *pdata, uint32_t addr, uint32_t nbyte)
{
	uint8_t status = 0;
	
	while (nbyte)
	{
		status = SST26VF0xxB_ByteProgram(addr, *pdata);

		if (0 == status)
		{
			break;
		}			
		
		pdata++;
		addr++;
		nbyte--;
	}
	
	SST26VF0xxB_WriteDisable();
	
	return status;
}

/**
  * @brief  SST26VF0xxB_PageProgram.
  * @param  None.
  *         
  * @retval None.
  */
static uint8_t SST26VF0xxB_PageProgram(uint8_t *pdata, uint32_t addr, uint32_t nbyte)
{
	uint8_t status = 0;
	
	uint8_t addr1 = 0;
	uint8_t addr2 = 0;
	uint8_t addr3 = 0;
	
	uint32_t TIMEOUT  = 0xA380;
	
	addr1 = (uint8_t)(addr >> 16);
	addr2 = (uint8_t)(addr >> 8);
	addr3 = (uint8_t)addr;
	
	SST26VF0xxB_WriteEnable(); //Write-Enable 
	
	/* Byte-Program */
	SST2xVF0xxB_CS_LOW();
	SST2xVF0xxB_SPI_Send(0x02);
	
	SST2xVF0xxB_SPI_Send(addr1);
	SST2xVF0xxB_SPI_Send(addr2);
	SST2xVF0xxB_SPI_Send(addr3);
	
	while (nbyte)
	{
		SST2xVF0xxB_SPI_Send(*pdata);
		pdata++;
		nbyte--;	
	}
	
	SST2xVF0xxB_CS_HIGH();
	
	do
	{
		status = SST26VF0xxB_RDSR();
		TIMEOUT--;
	} while (((status & 0x01) == 0x01) && (TIMEOUT != 0));
	
	SST26VF0xxB_WriteDisable();
	
	return (0 == TIMEOUT) ? 0 : 1;
}

/**
  * @brief  SST26VF0xxB_Page_Write.
  * @param  None.
  *         
  * @retval None.
  */
uint8_t SST26VF0xxB_Page_Write(uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite)
{
	uint32_t NumOfPage = 0; 
	uint32_t NumOfSingle = 0; 
	uint32_t count = 0;
  uint32_t Addr = 0;
	uint32_t SSTDataNum = 0;
  uint8_t status = 0;
	
	Addr = WriteAddr % SST_PAGESIZE;
  count = SST_PAGESIZE - Addr;
  NumOfPage =  NumByteToWrite / SST_PAGESIZE;
  NumOfSingle = NumByteToWrite % SST_PAGESIZE;
 
  /*!< If WriteAddr is SST_PAGESIZE aligned  */
  if(Addr == 0) 
  {
    /*!< If NumByteToWrite < SST_PAGESIZE */
    if(NumOfPage == 0) 
    {
      /* Store the number of data to be written */
      SSTDataNum = NumOfSingle;
      /* Start writing data */
      status = SST26VF0xxB_PageProgram(pBuffer, WriteAddr, SSTDataNum);
			if (0 == status) goto PAGEWRITEEND;
    }
    /*!< If NumByteToWrite > SST_PAGESIZE */
    else  
    {
      while(NumOfPage--)
      {
        /* Store the number of data to be written */
        SSTDataNum = SST_PAGESIZE;        
        status = SST26VF0xxB_PageProgram(pBuffer, WriteAddr, SSTDataNum); 
				if (0 == status) goto PAGEWRITEEND;
        
				WriteAddr +=  SST_PAGESIZE;
        pBuffer += SST_PAGESIZE;
      }

      if(NumOfSingle!=0)
      {
        /* Store the number of data to be written */
        SSTDataNum = NumOfSingle;          
        status = SST26VF0xxB_PageProgram(pBuffer, WriteAddr, SSTDataNum);
				if (0 == status) goto PAGEWRITEEND;
      }
    }
  }
  /*!< If WriteAddr is not SST_PAGESIZE aligned  */
  else 
  {
    /*!< If NumByteToWrite < SST_PAGESIZE */
    if(NumOfPage== 0) 
    {
      /*!< If the number of data to be written is more than the remaining space 
      in the current page: */
      if (NumByteToWrite > count)
      {
        /* Store the number of data to be written */
        SSTDataNum = count;        
        /*!< Write the data contained in same page */
        status = SST26VF0xxB_PageProgram(pBuffer, WriteAddr, SSTDataNum);      
        if (0 == status) goto PAGEWRITEEND;
				
        /* Store the number of data to be written */
        SSTDataNum = (NumByteToWrite - count);          
        /*!< Write the remaining data in the following page */
        status = SST26VF0xxB_PageProgram((uint8_t*)(pBuffer + count), (WriteAddr + count), SSTDataNum); 
				if (0 == status) goto PAGEWRITEEND;
      }      
      else      
      {
        /* Store the number of data to be written */
        SSTDataNum = NumOfSingle;         
        status = SST26VF0xxB_PageProgram(pBuffer, WriteAddr, SSTDataNum); 
				if (0 == status) goto PAGEWRITEEND;
      }     
    }
    /*!< If NumByteToWrite > SST_PAGESIZE */
    else
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / SST_PAGESIZE;
      NumOfSingle = NumByteToWrite % SST_PAGESIZE;
      
      if(count != 0)
      {  
        /* Store the number of data to be written */
        SSTDataNum = count;         
        status = SST26VF0xxB_PageProgram(pBuffer, WriteAddr, SSTDataNum);
				if (0 == status) goto PAGEWRITEEND;
				
        WriteAddr += count;
        pBuffer += count;
      } 
      
      while(NumOfPage--)
      {
        /* Store the number of data to be written */
        SSTDataNum = SST_PAGESIZE;          
        status = SST26VF0xxB_PageProgram(pBuffer, WriteAddr, SSTDataNum);
				if (0 == status) goto PAGEWRITEEND;
        
				WriteAddr +=  SST_PAGESIZE;
        pBuffer += SST_PAGESIZE;  
      }
      if(NumOfSingle != 0)
      {
        /* Store the number of data to be written */
        SSTDataNum = NumOfSingle;           
        status = SST26VF0xxB_PageProgram(pBuffer, WriteAddr, SSTDataNum);
				if (0 == status) goto PAGEWRITEEND;
      }
    }
  } 

PAGEWRITEEND:	
	
	return status;
}

/**
  * @brief  SST26VF0xxB_WriteEnable.
  * @param  None.
  *         
  * @retval None.
  */
static void SST26VF0xxB_WriteEnable(void)
{
	/* Write-Enable (WREN) */
	SST2xVF0xxB_CS_LOW();
	SST2xVF0xxB_SPI_Send(0x06);
	SST2xVF0xxB_CS_HIGH();
}	

/**
  * @brief  SST26VF0xxB_WriteDisable.
  * @param  None.
  *         
  * @retval None.
  */
static void SST26VF0xxB_WriteDisable(void)
{
	/* Write-Disable (WRDI) */
	SST2xVF0xxB_CS_LOW();
	SST2xVF0xxB_SPI_Send(0x04);
	SST2xVF0xxB_CS_HIGH();
}

/**
  * @brief  SST26VF0xxB_RDSR.
  * @param  None.
  *         
  * @retval None.
  */
static uint8_t SST26VF0xxB_RDSR(void)
{
	uint8_t status = 0;
	
	/* Read-Status-Register (RDSR) */
	SST2xVF0xxB_CS_LOW();
	
	SST2xVF0xxB_SPI_Send(0x05);
	status = SST2xVF0xxB_SPI_Transfer();
	
	SST2xVF0xxB_CS_HIGH();
	
	return status;
}

/**
  * @brief  SST26VF0xxB_4KByteSectorErase.
  * @param  None.
  *         
  * @retval None.
  */
uint8_t SST26VF0xxB_4KByteSectorErase(uint32_t addr)
{
	uint8_t status = 0;
	
	uint8_t addr1 = 0;
	uint8_t addr2 = 0;
	uint8_t addr3 = 0;
	
	uint32_t TIMEOUT  = 0xA380; 
	
	addr1 = (uint8_t)(addr >> 16);
	addr2 = (uint8_t)(addr >> 8);
	addr3 = (uint8_t)addr;
	
	SST26VF0xxB_WriteEnable(); //Write-Enable 
	
	/* 4KByte Sector-Erase */
	SST2xVF0xxB_CS_LOW();
	SST2xVF0xxB_SPI_Send(0x20);
	
	SST2xVF0xxB_SPI_Send(addr1);
	SST2xVF0xxB_SPI_Send(addr2);
	SST2xVF0xxB_SPI_Send(addr3);
	
	SST2xVF0xxB_CS_HIGH();
	
	do
	{
		status = SST26VF0xxB_RDSR();
		TIMEOUT--;
	} while (((status & 0x01) == 0x01) && (TIMEOUT != 0));
	
	SST26VF0xxB_WriteDisable();
	
	return (0 == TIMEOUT) ? 0 : 1;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
