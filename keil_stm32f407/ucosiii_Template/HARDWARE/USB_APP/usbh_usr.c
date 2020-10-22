#include "usbh_usr.h" 
//#include "led.h"
#include "../../FATFS/src/ff.h" 
#include "../../FATFS/src/diskio.h" 
#include "usart.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//USBH-USR 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/7/22
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   


static USBH_HOST  USB_Host;
static USB_OTG_CORE_HANDLE  USB_OTG_Core;


static u8 AppState;
//extern USB_OTG_CORE_HANDLE  USB_OTG_Core;



USBH_HOST *USBH_USR_GetUsbHost(void)
{
	return &USB_Host;
}

USB_OTG_CORE_HANDLE *USBH_USR_GetUsbOtgCore(void)
{
	return &USB_OTG_Core;
}



//USB OTG 中断服务函数
//处理所有USB中断
void OTG_FS_IRQHandler(void)
{
  	USBH_OTG_ISR_Handler(&USB_OTG_Core);
} 
//USB HOST 用户回调函数.
USBH_Usr_cb_TypeDef USR_Callbacks=
{
	USBH_USR_Init,
	USBH_USR_DeInit,
	USBH_USR_DeviceAttached,
	USBH_USR_ResetDevice,
	USBH_USR_DeviceDisconnected,
	USBH_USR_OverCurrentDetected,
	USBH_USR_DeviceSpeedDetected,
	USBH_USR_Device_DescAvailable,
	USBH_USR_DeviceAddressAssigned,
	USBH_USR_Configuration_DescAvailable,
	USBH_USR_Manufacturer_String,
	USBH_USR_Product_String,
	USBH_USR_SerialNum_String,
	USBH_USR_EnumerationDone,
	USBH_USR_UserInput,
	USBH_USR_MSC_Application,
	USBH_USR_DeviceNotSupported,
	USBH_USR_UnrecoveredError
};
/////////////////////////////////////////////////////////////////////////////////
//以下为各回调函数实现.

extern u8 exfuns_init(u8 disknum);
extern FATFS *exfuns_GetfsArea(u8 fsnum);
extern void DEBUG_Printf(char *s);
//USB HOST 初始化 
void USBH_USR_Init(void)
{
	FRESULT result;
	printf("USB OTG HS MSC Host\r\n");
	printf("> USB Host library started.\r\n");
	printf("  USB Host Library v2.1.0\r\n\r\n");
	
	if(1 == exfuns_init(USB_DISK))							//为fatfs相关变量申请内存		
	{
		DEBUG_Printf("fatfs USB_Disk malloc false!");
		while(1);
	}
	result = f_mount(exfuns_GetfsArea(USB_DISK),"2:",1); 				//挂载U盘
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%d)\r\n", result);
		return;
	}
	//初始化USB主机
  USBH_Init(&USB_OTG_Core,USB_OTG_HS_CORE_ID,&USB_Host,&USBH_MSC_cb,&USR_Callbacks); 
	
}
//检测到U盘插入
void USBH_USR_DeviceAttached(void)//U盘插入
{
//	LED1=1;
	printf("检测到USB设备插入!\r\n");
}
//检测到U盘拔出
void USBH_USR_DeviceDisconnected (void)//U盘移除
{
//	LED1=0;
	printf("USB设备拔出!\r\n");
}  
//复位从机
void USBH_USR_ResetDevice(void)
{
	printf("复位设备...\r\n");
}
//检测到从机速度
//DeviceSpeed:从机速度(0,1,2 / 其他)
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{
	if(DeviceSpeed==HPRT0_PRTSPD_HIGH_SPEED)
	{
		printf("高速(HS)USB设备!\r\n");
 	}  
	else if(DeviceSpeed==HPRT0_PRTSPD_FULL_SPEED)
	{
		printf("全速(FS)USB设备!\r\n"); 
	}
	else if(DeviceSpeed==HPRT0_PRTSPD_LOW_SPEED)
	{
		printf("低速(LS)USB设备!\r\n");  
	}
	else
	{
		printf("设备错误!\r\n");  
	}
}
//检测到从机的描述符
//DeviceDesc:设备描述符指针
void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{ 
	USBH_DevDesc_TypeDef *hs;
	hs=DeviceDesc;   
	printf("VID: %04Xh\r\n" , (uint32_t)(*hs).idVendor); 
	printf("PID: %04Xh\r\n" , (uint32_t)(*hs).idProduct); 
}
//从机地址分配成功
void USBH_USR_DeviceAddressAssigned(void)
{
	printf("从机地址分配成功!\r\n");   
}
//配置描述符获有效
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
                                          USBH_InterfaceDesc_TypeDef *itfDesc,
                                          USBH_EpDesc_TypeDef *epDesc)
{
	USBH_InterfaceDesc_TypeDef *id; 
	id = itfDesc;   
	if((*id).bInterfaceClass==0x08)
	{
		printf("可移动存储器设备!\r\n"); 
	}else if((*id).bInterfaceClass==0x03)
	{
		printf("HID 设备!\r\n"); 
	}    
}
//获取到设备Manufacturer String
void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
	printf("Manufacturer: %s\r\n",(char *)ManufacturerString);
}
//获取到设备Product String 
void USBH_USR_Product_String(void *ProductString)
{
	printf("Product: %s\r\n",(char *)ProductString);  
}
//获取到设备SerialNum String 
void USBH_USR_SerialNum_String(void *SerialNumString)
{
	printf("Serial Number: %s\r\n",(char *)SerialNumString);    
} 
//设备USB枚举完成
void USBH_USR_EnumerationDone(void)
{ 
	printf("设备枚举完成!\r\n\r\n");    
} 
//无法识别的USB设备
void USBH_USR_DeviceNotSupported(void)
{
	printf("无法识别的USB设备!\r\n\r\n");    
}  
//等待用户输入按键,执行下一步操作
USBH_USR_Status USBH_USR_UserInput(void)
{ 
	printf("跳过用户确认步骤!\r\n");
	return USBH_USR_RESP_OK;
} 
//USB接口电流过载
void USBH_USR_OverCurrentDetected (void)
{
	printf("端口电流过大!!!\r\n");
} 


extern u8 exf_getfree(u8 *drv,u32 *total,u32 *free);
//用户测试主程序
//返回值:0,正常
//       1,有问题
u8 USH_User_App(void)
{ 
	OS_ERR err;
	u32 total,free;
	u8 res=0;
//	Show_Str(30,140,200,16,"设备连接成功!.",16,0);	 
	res=exf_getfree("2:",&total,&free);
	if(res==0)
	{
//		POINT_COLOR=BLUE;//设置字体为蓝色	   
//		LCD_ShowString(30,160,200,16,16,"FATFS OK!");	
//		LCD_ShowString(30,180,200,16,16,"U Disk Total Size:     MB");	 
//		LCD_ShowString(30,200,200,16,16,"U Disk  Free Size:     MB"); 	    
//		LCD_ShowNum(174,180,total>>10,5,16); //显示U盘总容量 MB
//		LCD_ShowNum(174,200,free>>10,5,16);	
	} 
 
	while(HCD_IsDeviceConnected(&USB_OTG_Core))//设备连接成功
	{	
//		LED1=!LED1;
#if SYSTEM_SUPPORT_OS
    OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT,&err); 
#else
    delay_ms(200);
#endif		
	}
//	POINT_COLOR=RED;//设置字体为红色	   
//	Show_Str(30,140,200,16,"设备连接中...",16,0);
//	LCD_Fill(30,160,239,220,WHITE);
	return res;
} 


//USB HOST MSC类用户应用程序
int USBH_USR_MSC_Application(void)
{
	u8 res=0;
  switch(AppState)
  {
    case USH_USR_FS_INIT://初始化文件系统 
			printf("开始执行用户程序!!!\r\n");
			AppState=USH_USR_FS_TEST;
      break;
    case USH_USR_FS_TEST:	//执行USB OTG 测试主程序
			res=USH_User_App(); //用户主程序
     	res=0;
			if(res)
			{
				AppState=USH_USR_FS_INIT;
			}
      break;
    default:
			break;
  } 
	return res;
}
//用户要求重新初始化设备
void USBH_USR_DeInit(void)
{
  	AppState=USH_USR_FS_INIT;
}
//无法恢复的错误!!  
void USBH_USR_UnrecoveredError (void)
{
	printf("无法恢复的错误!!!\r\n\r\n");	
}
////////////////////////////////////////////////////////////////////////////////////////
//用户定义函数,实现fatfs diskio的接口函数 
//extern USBH_HOST              USB_Host;

//获取U盘状态
//返回值:0,U盘未就绪
//      1,就绪
u8 USBH_UDISK_Status(void)
{
	return HCD_IsDeviceConnected(&USB_OTG_Core);//返回U盘状态
}

//读U盘
//buf:读数据缓存区
//sector:扇区地址
//cnt:扇区个数	
//返回值:错误状态;0,正常;其他,错误代码;		 
u8 USBH_UDISK_Read(u8* buf,u32 sector,u32 cnt)
{
	u8 res=1;
	if(HCD_IsDeviceConnected(&USB_OTG_Core)&&AppState==USH_USR_FS_TEST)//连接还存在,且是APP测试状态
	{  		    
		do
		{
			res=USBH_MSC_Read10(&USB_OTG_Core,buf,sector,512*cnt);
			USBH_MSC_HandleBOTXfer(&USB_OTG_Core ,&USB_Host);		      
			if(!HCD_IsDeviceConnected(&USB_OTG_Core))
			{
				res=1;//读写错误
				break;
			};   
		}while(res==USBH_MSC_BUSY);
	}else res=1;		  
	if(res==USBH_MSC_OK)res=0;	
	return res;
}

//写U盘
//buf:写数据缓存区
//sector:扇区地址
//cnt:扇区个数	
//返回值:错误状态;0,正常;其他,错误代码;		 
u8 USBH_UDISK_Write(u8* buf,u32 sector,u32 cnt)
{
	u8 res=1;
	if(HCD_IsDeviceConnected(&USB_OTG_Core)&&AppState==USH_USR_FS_TEST)//连接还存在,且是APP测试状态
	{  		    
		do
		{
			res=USBH_MSC_Write10(&USB_OTG_Core,buf,sector,512*cnt); 
			USBH_MSC_HandleBOTXfer(&USB_OTG_Core ,&USB_Host);		      
			if(!HCD_IsDeviceConnected(&USB_OTG_Core))
			{
				res=1;//读写错误
				break;
			};   
		}while(res==USBH_MSC_BUSY);
	}else res=1;		  
	if(res==USBH_MSC_OK)res=0;	
	return res;
}



//USBHost任务函数
void USBHost_task(void *p_arg)
{	
	OS_ERR err;
	p_arg = p_arg;
	
	
	USBH_USR_Init();
//	printf("USBHost_task start running!\n");	
	while(1)
	{
		USBH_Process(&USB_OTG_Core, &USB_Host);
		OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_HMSM_STRICT,&err); //延时100ms
	}
	
}







































