/**
  ******************************************************************************
  * @file    SST_RecordFat.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#include "ff_Lock.h"       /* FATFS */

/**
  * @brief  SST_Rd_BufferWrite.
  * @param  None.
  *         
  * @retval None.
  */
uint8 SST_Rd_BufferWrite(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite)
{
	FIL   filRecordFat = { 0 };
	
	uint32 bytesToWrite = 0;
	
	f_open_Lock(&filRecordFat, "1:Record.dat", FA_WRITE);
	
	f_lseek_Lock(&filRecordFat, WriteAddr);
	
	f_write_Lock(&filRecordFat, pBuffer, NumByteToWrite, &bytesToWrite);
	
	f_sync_Lock(&filRecordFat);
	
	f_close_Lock(&filRecordFat);
	
	return 1;
}


/**
  * @brief  SST_Rd_BufferRead.
  * @param  None.
  *         
  * @retval None.
  */
uint8 SST_Rd_BufferRead(uint8 *recv, uint32 adr, uint16 size)
{
	FIL   filRecordFat = { 0 };
	
	uint32 numOfReadBytes = 0;
	
	f_open_Lock(&filRecordFat, "1:Record.dat", FA_READ);
	
	f_lseek_Lock(&filRecordFat, adr);
	
	f_read_Lock(&filRecordFat, recv, size, &numOfReadBytes);
	
	f_close_Lock(&filRecordFat);
	
	return 1;
}



///////////////////////////////////////////////////////////////////////////
uint16 SST_Rd_E2romSaveUpData(uint16 num,uint8 *buf)
{	
   return 0;
}

uint16 SST_Rd_E2romSaveUpDataHead(uint8 *buf,uint16 len)
{	
   return 0;
}

