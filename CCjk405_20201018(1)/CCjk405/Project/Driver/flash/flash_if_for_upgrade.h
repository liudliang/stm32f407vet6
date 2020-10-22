/****************************************************************************************
深圳奥耐电气技术有限公司

Filename: flash_if_for_upgrade.h
Author: patli
Datetime: 20200206
Version: v0.01
****************************************************************************************/
#ifndef _FLASH_IF_FOR_UPGRADE_H_
#define _FLASH_IF_FOR_UPGRADE_H_

#include "typedefs.h"
#include "M_Global.h"
#include "stm32f4xx.h"
 
/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */
 
#define FLASH_USER_START_ADDR ADDR_FLASH_SECTOR_8	//用户起始地址，暂定为第十个扇区的起始地址
/* End of the Flash address */
#define USER_FLASH_END_ADDRESS        0x080FFFFF 
//API

extern void FLASH_If_Init(void);

extern void FLASH_If_Lock(void);

extern uint32 FLASH_If_WriteByte(__IO uint8* FlashAddress, uint8* Data ,uint32 DataLength);

extern uint32 FLASH_If_WriteDWord(__IO uint32* FlashAddress, uint32* Data ,uint32 DataLength);


extern uint16 Flash_GetSector(uint32 Address);
extern void Flash_EraseSector(uint16 SectorNum);
extern void Flash_EraseSectors(uint32 address, uint16 sector_num);

extern void Flash_Write32BitDatas(uint32 address, uint16 length, uint32* data_32);
 
extern void Flash_Write32BitDatasWithErase(uint32 address, uint16 length, uint32* data_32);
extern void Flash_Read32BitDatas(uint32 address, uint16 length, uint32* data_32);
 
extern void Flash_Write16BitDatasWithErase(uint32 address, uint16 length, uint16* data_16);
extern void Flash_Read16BitDatas(uint32 address, uint16 length, uint16* data_16);

extern void Flash_Write8BitDatas(uint32 address, uint16 length, uint8* data_8);
 
extern void Flash_Write8BitDatasWithErase(uint32 address, uint16 length, uint8* data_8);
extern void Flash_Read8BitDatas(uint32 address, uint16 length, uint8* data_8);


#endif
