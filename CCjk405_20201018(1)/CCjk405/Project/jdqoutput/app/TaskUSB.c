/**
  ******************************************************************************
  * @file    TaskUSB.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
*/
#include "stm32f4xx.h"
#include "usb_bsp.h"
#include "usb_hcd_int.h"
#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_msc_core.h"
#include "Includes.h"
#include "ff_Lock.h"       /* FATFS */
#include "flash_if_for_upgrade.h"
#include "main.h"
#include "gpio.h"

/* Private functions ---------------------------------------------------------*/
extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
extern USBH_HOST                    USB_Host;
#if (PRODUCTS_LEVEL == FACTORY_UPGRADE_VERSION)

#define USB_NO_CONNECT 		0
#define USB_CONNECTED 		1
#define USB_UPGRADEING 		2
#define USB_UPGRADED 		3

#define UPGRADEPACKETBUFFERLENGTH    1024

static uint8_t  fileUpgradePacketBuffer[UPGRADEPACKETBUFFERLENGTH] = { 0 };

uint8_t 	usb_connect_num = 0;
uint8_t 	usb_noconnect_num = 0;

static FATFS fatfs = { 0 };
static FIL   logfile = { 0 };

static FIL  crcfile = { 0 };
static FIL  Upgradefile = { 0 };
uint32_t 	usb_crc32 =0 ;				//crc32 of usb file 
uint32_t 	programRegionCRC = 0;  		//crc32 of flash data
uint8_t 	cur_usb_status = USB_NO_CONNECT;

extern uint32_t flashdestination;
extern uint32 set_program_crc_position;
extern uint32 set_upgrade_flag_position;    //0x08010000     UPGRADE FLAG
extern uint32 set_upgrade_len_position;    //0x08010000  UPGRADE DATA LEN

static  OS_STK  Stk_TaskUSBHBootLoader[TASK_USBHBOOTLOADER_STK_SIZE];

/**
  * @brief  LogOpen
  * @param  None
  * @retval None
  */
static FRESULT LogOpen(void)
{
	FRESULT res = FR_OK; 
	
	res = f_open_Lock(&logfile, "0:UpgraLog.txt", FA_CREATE_ALWAYS | FA_WRITE);
	
	return res;
}


/**
  * @brief  LogClose
  * @param  None
  * @retval None
  */
static void LogClose(void)
{
	f_close_Lock(&logfile);
}


/**
  * @brief  ReadUsbFileName
  * @param  None
  * @retval None
  */
void ReadUsbFileName()
{

}

/**
  * @brief  ReadUsbCrcFile
  * @param  None
  * @retval None
  */
uint8_t ReadUsbCrcFile()
{
	FRESULT res = FR_OK;
	uint32_t numOfReadBytes;
	
//	res = f_open_Lock(&crcfile, "0:crc32checksum", FA_OPEN_ALWAYS | FA_READ);
	res = f_open_Lock(&Upgradefile, "0:STM_CRC.bin", FA_OPEN_ALWAYS |FA_READ);	


	printf("ReadUsbCrcFile res = 0x%x\n\r", res);	

	
	if (FR_OK == res)
	{
		
		//设置到文件起点
		f_lseek_Lock(&crcfile, 0);
		f_read_Lock(&crcfile, &usb_crc32, sizeof(usb_crc32), (void *)&numOfReadBytes);
		
		printf("ReadUsbCrcFile usb_crc32 = 0x%x\n\r", usb_crc32);	

		//校验和写入flash
		Flash_Write32BitDatasWithErase(set_program_crc_position, 1, &usb_crc32);
		
		
		f_close_Lock(&crcfile);

	}

	return res;

}

/**
  * @brief  TaskUSBHBootLoader
  * @param  None
  * @retval None
  */
void USBUpgradeProc()
{
	FRESULT res = FR_OK;
	uint32_t numOfReadBytes;
	uint32_t filelen = 0;
	//software reset
	if(ReadUsbCrcFile() == FR_OK)
	{		
		__BEEP_ON();				   //开始升级
		
		res = f_open_Lock(&Upgradefile, "0:STM32_Program.bin", FA_OPEN_ALWAYS | FA_READ);
		
		printf("USBUpgradeProc res = 0x%x\n\r", res);	
		
		printf("USBUpgradeProc filesize = 0x%x\n\r", Upgradefile.fsize);	
		if (FR_OK == res)
		{			
			Flash_EraseSectors(FLASH_USER_START_ADDR, 3);  //在写用户程序前，清空区域
			FLASH_If_Init();	//flash unlock
			
			flashdestination   = FLASH_USER_START_ADDR; 

			do{
				f_read_Lock(&Upgradefile, fileUpgradePacketBuffer, UPGRADEPACKETBUFFERLENGTH, (void *)&numOfReadBytes);
				
				printf("USBUpgradeProc numOfReadBytes = %d\n\r", numOfReadBytes);	

				if(numOfReadBytes > 0)
				{
					Flash_Write8BitDatas(flashdestination, numOfReadBytes, (uint8*)fileUpgradePacketBuffer);
					
					flashdestination += numOfReadBytes;

					filelen += numOfReadBytes;				
				}	
			}while(filelen < Upgradefile.fsize);
			
			FLASH_If_Lock();
			
			printf("USBUpgradeProc over  filelen= 0x%x\n\r", filelen);	

			//计算写到flash的数据的CRC32校验
			programRegionCRC = Calc_CRC32_Crypto((uint8 *)FLASH_USER_START_ADDR, filelen);
			
			printf("USBUpgradeProc over  programRegionCRC=0x%x; usb_crc32=0x%x\n\r", programRegionCRC, usb_crc32);

			if(programRegionCRC == usb_crc32)				
				Flash_Write32BitDatasWithErase(set_upgrade_len_position, 1, &filelen); 
			
		}
	}

	LogClose();
	
	SoftReset();
}

/**
  * @brief  TaskUSBHBootLoader
  * @param  None
  * @retval None
  */

void TaskUSBHBootLoader(void *p_arg)
{
//	uint32_t nonConnectionCount = 0;
//	uint16_t numOfReadBytes = 0;
	
	while (1)
	{
		
		if (HCD_IsDeviceConnected(&USB_OTG_Core))
		{
			usb_connect_num++;
			if(usb_connect_num > 3&&cur_usb_status==USB_NO_CONNECT)
			{
				usb_connect_num = 0;
				cur_usb_status = USB_CONNECTED;
				WriteToBackupReg(0, REGUSBHCONNECTION);  //U盘连接	

				/* Initialises the File System*/
				if(f_mount_Lock( 0, &fatfs ) != FR_OK)
				{
					system_softReset();
				}

				LogOpen();
				USBUpgradeProc();
				
			}
		}
		else	
		{
			usb_connect_num = 0;
			usb_noconnect_num++;

			if(usb_noconnect_num > 3)
			{
				WriteToBackupReg(0, REGUSBHNONCONNECTION);	//U盘未连接 
				//software reset
				SoftReset();
				
			}	
			
			cur_usb_status = USB_NO_CONNECT;
		}
		
		OSTimeDly(20);   //20 *5 = 100ms		
	}
}


/**
* @brief  TaskUSB
* @param  None
* @retval None
*/

void TaskUSB(void *p_arg)
{
	/* Init Host Library */
  USBH_Init(&USB_OTG_Core, 
#ifdef USE_USB_OTG_FS  
            USB_OTG_FS_CORE_ID,
#else 
            USB_OTG_HS_CORE_ID,
#endif 
            &USB_Host,
            &USBH_MSC_cb, 
            &USR_cb);


	OSTaskCreate(TaskUSBHBootLoader, (void *)0, &Stk_TaskUSBHBootLoader[TASK_USBHBOOTLOADER_STK_SIZE - 1], OS_USER_PRIO_GET(APP_TASK_USBHBOOTLOADER_PRIO));

	
	while (1)
	{
		/* Host Task handler */
		USBH_Process(&USB_OTG_Core, &USB_Host);
		
		OSTimeDly(20);   //20 *5 = 100ms
	}
}

/**
  * @brief  OTG_FS_IRQHandler
  *          This function handles USB-On-The-Go FS global interrupt request.
  *          requests.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_OTG_FS  
void OTG_FS_IRQHandler(void)
#else
void OTG_HS_IRQHandler(void)
#endif
{
  USBH_OTG_ISR_Handler(&USB_OTG_Core);
}
#endif
