/****************************************Copyright (c)**************************************************
        深圳奥耐电气技术有限公司
				
***FILE：SST_FLASH.c
****************************************************************************************************/
#include "SST_FLASH.h"
extern void InitSPI1(void);

static uint8 SST26VF_BLOCK_BUFFER[8][512] = { 0 };

/** 
  * @brief  sst_init 
  * @param  None 
  * @retval None 
  */  
void sst_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//SST25VF032B_CS口
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
	
//	InitSPI3();
	InitSPI1();
	
	SST26VF0xxB_RemoveBlockProtection();
}

/**
  * @brief  Flash_BufferRead.
  * @param  None.
  *         
  * @retval None.
  */
uint8 Flash_BufferRead(uint8 *recv, uint32 adr, uint16 size)
{
	SST26VF0xxB_Read(recv, adr, size);
	
	return SST_TRUE;
}

/**
  * @brief  Flash_BufferWrite.
  * @param  None.
  *         
  * @retval None.
  */
uint8 Flash_BufferWrite(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite)
{
	uint32 secpos = 0;
	uint16 secoff = 0;
	uint16 secremain = 0;	   
 	uint16 i = 0;    
	uint8 *SST26VF_BUF = SST26VF_BLOCK_BUFFER[0];	  
   	   
	secpos = WriteAddr /SST_BLOCK_SIZE;    //块区地址 
	secoff = WriteAddr % SST_BLOCK_SIZE;   //在块区内的偏移
	secremain = SST_BLOCK_SIZE - secoff;   //块区剩余空间大小   
	
	if (NumByteToWrite <= secremain)
	{
		secremain = NumByteToWrite;
	}
		
	Flash_BufferRead(SST26VF_BUF, secpos * SST_BLOCK_SIZE, SST_BLOCK_SIZE); //读出整个块区的内容
		
	Flash_SectorErase(secpos * SST_BLOCK_SIZE);          //擦除这个块区

	for (i = 0; i < secremain; i++)	    //复制数据
	{
		SST26VF_BUF[i + secoff] = pBuffer[i];	  
	}
	
	SST26VF0xxB_Page_Write(SST26VF_BUF, secpos * SST_BLOCK_SIZE, SST_BLOCK_SIZE);  //写入整个块区  
	
	return SST_TRUE;
}

/**
  * @brief  Flash_SectorErase.
  * @param  None.
  *         
  * @retval None.
  */
void Flash_SectorErase(uint32 adr)
{
	SST26VF0xxB_4KByteSectorErase(adr);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
