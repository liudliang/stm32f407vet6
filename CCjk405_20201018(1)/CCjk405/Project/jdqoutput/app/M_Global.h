/***********************************************************************
文件名称：
功    能：
编写时间：
编 写 人：
注    意：
***********************************************************************/
 
#ifndef _M_GLOBAL_H_
#define _M_GLOBAL_H_


//添加必要的头文件
#include "M_Type_P.h"
#include "M_CMSIS_P.h" 

//#include "typedefs.h"

//#define VER2_1_0
/***********************系统配置选项********************************************************
*****注意 只能使用，禁止修改
******************************************************************************/  
#define ONLINE_CARD            		1                 /*支持在线卡，需要后台验证*/
#define OFFLINE_CARD           		0                 /*离线卡*/

#define _43SCREEN              		1                 /*4.3寸屏*/
#define _07SCREEN              		2                 /*7寸屏*/

#define GUNSTART_SUPPORT       		0                 /*插枪即充电支持*/
#define GUNSTART_DONTSUPPORT   		1                 /*插枪即充电 不支持*/
//#define GUNSTART_VIN_SUPPORT   		2                 /*插枪即充电 车VIN账号*/

#define _2DMBARCODE_SUPPORT    		1                 /*支持二维码*/
#define _2DMBARCODE_DONTSUPPORT 	0                 /*不支持二维码*/

#define WDG_ENABLE                8
#define WDG_DISABLE               9




/*****************************注意****************************************
***出厂测试配置选项，
***工厂测试完成后请务必将 PRODUCTS_LEVEL 测试级别配置为LEAVE_FACTORY_TEST
**************************************************************************/

#define DEBUG_VERSION           	5                /*调试及工厂测试版本*/
#define RELEASE_VERSION     		6                /*出厂发布版本*/
#define FACTORY_UPGRADE_VERSION     7                /*厂家升级版本*/

#define PRODUCTS_LEVEL  		DEBUG_VERSION                  // DEBUG_VERSION , RELEASE_VERSION, FACTORY_UPGRADE_VERSION


//#define USE_STDPERIPH_DRIVER
//#define STM32F4XX
//#define USE_USB_OTG_FS
//#define USE_STM324xG_EVAL


/*************************************************************************************************************************
* 版本定义
* 每增加一个版本，需要让数字加1
**************************************************************************************************************************
*版本说明：（在编译项中定义）
*VER2_0_1_T1      //杭州叉车充电桩项目，插枪即充版本
*VER2_0_1         //标准版本
*V4071 A04版本    20190425  1、修改了单模块最大电流的限制到50A 2、BMS需求电流大于桩的最大电流后，强制变更需求为桩的最大电流
*V4071 A05版本    20190520  1、修改了串口优先级，将后台串口优先级放到最大
*V4071 A06版本    20190529  1、双枪充电时计算电费过大，后时段电量有时加了前个时段的电量。保持当前时段号变量gBackSectNo改成gBackSectNo[DEF_MAX_GUN_NO]
*V4071 A07版本    20190625  1、更改了国标BMS协议BCP中最大电流值和BCL中需求电流值冲突的问题。取两者间的最小值做为需求电流。
*V4071 A08版本    20190716  1、主程序中注释掉USB部分，避免和BOOT的升级冲突。
                            2、网口查找部分的超时时间改短(stm32f4x7_eth.h中)，避免和看门狗冲突。
**************************************************************************************************************************/
#include "stm32f4xx.h"

	
  /**绝缘盒子配置********************************************************************************/	
	#define  ISO_SINGLE     0  /*单路绝缘检测模块  黑色*/
	#define  WBISO_SINGLE   1  /*WB9162单路绝缘检测模块 蓝色*/
	#define  ISO_AUNICE     2  /*公司自产模块*/
	
	#define  ISO_TYPE      ISO_AUNICE     //ISO_AUNICE//WBISO_SINGLE                   /*绝缘盒子选择*/
  /****************************************************************************************/	
  /*与boot合并0x08020000，U盘升级0x08080000*/
	#define SUPPORT_GUN_START   	 GUNSTART_SUPPORT//GUNSTART_DONTSUPPORT /*枪启动*/
	
	#define BOOT_BASE_ADDRESS	 				0x08000000		//boot 程序区域起始地址
	#define FACTORY_SETTING_BASE_ADDRESS		0x08060000		//设置 程序区域起始地址
	#define FACTORY_UPGRADE_BASE_ADDRESS	 	0x08020000		//升级 程序区域起始地址
	#define FACTORY_RELEASE_BASE_ADDRESS		0x08080000		//发布 程序区域起始地址	
	#define FACTORY_RELEASE_END_ADDRESS			0x080E0000		//发布 程序区域结束块起始地址 

  	#if (PRODUCTS_LEVEL == DEBUG_VERSION)					//调试版本	
//	#define BMSTEST											//使用BMS模拟器
	#define M_VERSION1 "D4052"         	     //1为BMSTEST，非1屏蔽BMSTEST           	/*版本后缀，没有为空 .t1*/  
	#define FLASH_BASE_LEAVE_FACTORY     BOOT_BASE_ADDRESS
	#define VERSION_LETTER		0x54  //  'T'
	#define TASK_RUN_TIME_PRINTF
	#define TASK_RUN_TIME_PRINTF_NO_COMM	
//	#define TASK_MESSAGE_PRINT

	#warning "DEBUG_VERSION"
		
  	#elif (PRODUCTS_LEVEL == FACTORY_UPGRADE_VERSION)		 /*出厂升级版本模识*/
	#define M_VERSION1 "D4071"								/*版本后缀，没有为空 .t1*/
	#define FLASH_BASE_LEAVE_FACTORY	 BOOT_BASE_ADDRESS  //patli tmp    //BOOT_BASE_ADDRESS  //FACTORY_UPGRADE_BASE_ADDRESS
	#define VERSION_LETTER		0x55  //  'U'
	#warning "FACTORY_UPGRADE_VERSION"
	
  	#elif (PRODUCTS_LEVEL == RELEASE_VERSION)                /*出厂发布模式，用U盘升级或网络升级*/
	#define FLASH_BASE_LEAVE_FACTORY     FACTORY_RELEASE_BASE_ADDRESS
	#warning "RELEASE_VERSION"
	#define TASK_RUN_TIME_PRINTF
	#define TASK_RUN_TIME_PRINTF_NO_COMM
		
	#if ISO_TYPE == WBISO_SINGLE
		#define M_VERSION1 "D407VAW_CP"                 
	#endif
		
	#if ISO_TYPE == ISO_AUNICE
			#define M_VERSION1 "D407VAA_CP" 
	#endif
		
	#define VERSION_LETTER		0x56  //  'V'
		
	#endif
	
#define WATCHDOG_ENABLE 	   WDG_ENABLE		   //WDG_DISABLE
	
//	#if (ISO_TYPE == ISO_AUNICE )		
//		#define M_VERSION2 "10"
//	#elif (ISO_TYPE == WBISO_SINGLE )
//	  #define M_VERSION2 "11"
//	#else
//	  #define M_VERSION2 "13"
//	#endif

//#define AN_ETH_IRQ           //以太网中断接收数据开关控制


//#define AN_ETH_TASK           //以太网任务开关控制


#define CCM_USE           //CCM 使用开关控制

#if (PRODUCTS_LEVEL == FACTORY_UPGRADE_VERSION) 
#define NET_FTP_CLIENT   	1   //patli 20190909    
#define USB_HOST_SUPPORT		//patli 20191216
#endif


//#define UART_WRITE_NO_QUEUE



/***********************************VIN充电控制**************************************/

//#define VIN_CHARGE_ON                         //patli 20191009

#define CP_PERIOD_FEE						//澄鹏阶段费率

/****************************************************************************************/
/***********************************各家后台开关控制**************************************/

//#define 	CHE_DIAN_BACKCOMM
//#define ZPLD_BACK_COMM
//#define AUNICE_BACK_COMM
#define 	BMS_USE_TIMER
//#define CHARGE_BIRD_BACK_COMM
//#define CP_BACK_COMM

#define 	DC_AC_ISO_REALCHECK   	//DC AC ISO REALCHECK 任务合并
//#define SCREEN_CARD				//SCREEN CARD任务合并
//#define 	DC_MET_ON				//SCREEN  
//#define 	AC_MET_ON				//SCREEN  
#define 	SCREEN_ON				//SCREEN  
//#define CARD_ON
//#define  	ISO_ON
#define 	GUN_BMS_CTRL			//BMS CTRL任务合并
#define 	A_B_GUN_TOGETER			//A, B BMS CTRL任务合并
//#define TRY_START_CHARGIN		//支持3次重启


//#define     STM32F407

/****************************************************************************************/
#if 1  // (PRODUCTS_LEVEL == DEBUG_VERSION)  //内部调试时才打开这些log

#define AUNICE_DEBUG			//整个软件输出LOG开关
//#define NET_DEBUG				//以太网输出LOG开关
//#define SCOMM_DEBUG				//串口通信输出LOG开关
//#define ETH_TEST_DEBUG	

//#define ETH_TEMP_TEST
#endif


/*******************************************************************************************/	

#ifdef AUNICE_DEBUG
#define MAIN_VERSION	3
#define SUB_VERSION	    7

#define U_MAIN_VERSION	0
#define U_SUB_VERSION	1


#define M_VERSION4		6

#else
	#if (FLASH_BASE_LEAVE_FACTORY == BOOT_BASE_ADDRESS)	
		#define M_VERSION3 "T29"
	    #define M_VERSION4 6
		#warning "T26"
	#elif (FLASH_BASE_LEAVE_FACTORY == FACTORY_UPGRADE_BASE_ADDRESS)
	  #define M_VERSION3 "B06"
		#warning "B06"
	#else
	  #define M_VERSION3 "T2.16"
  		#define M_VERSION4 10
		#warning M_VERSION3
	#endif	
#endif		  

/****************************电表相关**********************************************/	

#define DCMETER_DLT645_2007         0       //DLT645_2007协议的电表
#define DCMETER_RECT                1       //从功率模块中得来

#define DCMETER_TYPE                DCMETER_RECT



/****************************屏幕相关**********************************************/	

#define DWIN_DGUS         0       //迪文dgus一代屏
#define DWIN_DGUS_II      1       //迪文dgus二代屏

#define SCREEN_TYPE                DWIN_DGUS_II

#endif




