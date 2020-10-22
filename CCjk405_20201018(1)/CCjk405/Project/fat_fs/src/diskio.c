/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		  /* FatFs lower layer API */	
#include "SST_FLASH.h"

#define EX_FLASH            1	                 //外部flash,卷标为1

#define FLASH_SECTOR_SIZE 	512       		  

#define FLASH_BLOCK_SIZE   	(SST_BLOCK_SIZE / FLASH_SECTOR_SIZE)     
#define	FLASH_SECTOR_COUNT  (SST_CAPACITY_SIZE / FLASH_SECTOR_SIZE)    //4M存储容量的扇区数


//初始化磁盘
DSTATUS disk_initialize(BYTE pdrv)	  /* Physical drive nmuber (0..) */
{
	switch (pdrv)
	{
		case EX_FLASH:                           //外部flash
 			break;
	}		 
	
	return RES_OK; 
}  

//获得磁盘状态
/* Physical drive nmuber (0..) */
DSTATUS disk_status(BYTE pdrv)     
{ 
	return 0;
} 

//读扇区
//drv:磁盘编号0~9
//*buff:数据接收缓冲首地址
//sector:扇区地址
//count:需要读取的扇区数
/* Physical drive nmuber (0..) */
/* Data buffer to store read data */
/* Sector address (LBA) */
/* Number of sectors to read (1..128) */
DRESULT disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
	switch (pdrv)
	{
		case EX_FLASH:                           //外部flash
			
			while (count > 0)
			{
				Flash_BufferRead(buff, sector * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
				buff += FLASH_SECTOR_SIZE;
				sector++;
				count--;
			}
			
			break; 
	}
	
  return RES_OK;	   
}

//写扇区
//drv:磁盘编号0~9
//*buff:发送数据首地址
//sector:扇区地址
//count:需要写入的扇区数
/* Physical drive nmuber (0..) */
/* Data to be written */
/* Sector address (LBA) */
/* Number of sectors to write (1..128) */
DRESULT disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{ 
	switch (pdrv)
	{
		case EX_FLASH:                           //外部flash
			
			while (count > 0)
			{										    
				Flash_BufferWrite((u8*)buff, sector * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
				buff += FLASH_SECTOR_SIZE;
				sector++;
				count--;
			}

			break; 
	}
  
	return RES_OK;
}

//其他表参数的获得
//drv:磁盘编号0~9
//ctrl:控制代码
//*buff:发送/接收缓冲区指针
/* Physical drive nmuber (0..) */
/* Control code */
/* Buffer to send/receive control data */
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
	if (pdrv == EX_FLASH)	//外部FLASH  
	{
		switch (cmd)
		{
			case CTRL_SYNC:
					break;	 
			
			case GET_SECTOR_SIZE:
					*(WORD*)buff = FLASH_SECTOR_SIZE;
					break;	 
			
			case GET_BLOCK_SIZE:
					*(WORD*)buff = FLASH_BLOCK_SIZE;
					break;	 
			
			case GET_SECTOR_COUNT:
					*(DWORD*)buff = FLASH_SECTOR_COUNT;
					break;
		}
	}
  
	return RES_OK;
}


















