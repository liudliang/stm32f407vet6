/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2007; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      APPLICATION CONFIGURATION
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210c Evaluation Board
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : Brian Nagel
*********************************************************************************************************
*/

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__

#include "M_Global.h"
/*
*********************************************************************************************************
*                                       ADDITIONAL uC/MODULE ENABLES
*********************************************************************************************************
*/

#define  uC_PROBE_OS_PLUGIN              DEF_DISABLED            /* DEF_ENABLED = Present, DEF_DISABLED = Not Present        */
#define  uC_PROBE_COM_MODULE             DEF_DISABLED
#define  uC_LCD_MODULE                   DEF_DISABLED

#define  OS_USER_PRIO_GET(a)             a
/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#if (PRODUCTS_LEVEL == FACTORY_UPGRADE_VERSION)

#define APP_TASK_START_PRIO        	4   	//6  //patli 20200115 28
#ifndef TCPIP_THREAD_PRIO
#define TCPIP_THREAD_PRIO		  5   //7	//定义内核任务的优先级为5
#endif
#ifdef NET_DEBUG
#define NET_DEBUG_TASK_PRIO		6 //    29
#define NET_DEBUG_STK_SIZE	  356
#endif

#define APP_TASK_FTP_CLIENT_PRIO  7

#define APP_TASK_USBH_PRIO          9
#define APP_TASK_USBHBOOTLOADER_PRIO 10


#else

#define APP_TASK_START_PRIO        	5   	//6  //patli 20200115 28
#ifndef TCPIP_THREAD_PRIO
#define TCPIP_THREAD_PRIO		  6   //7	//定义内核任务的优先级为5
#endif
#ifdef NET_DEBUG
#define NET_DEBUG_TASK_PRIO		7 //    29
#define NET_DEBUG_STK_SIZE	  356
#endif

#define APP_TASK_FTP_CLIENT_PRIO  15

#endif

#define LWIP_DHCP_TASK_PRIO       		8

#define APP_TASK_AGUNBMS_PRIO       9
#ifndef A_B_GUN_TOGETER
#define APP_TASK_BGUNBMS_PRIO       10
#define APP_TASK_DCMETER_PRIO       11
#define APP_TASK_DLMOD_PRIO         12
#define APP_TASK_SCREEN_PRIO        13   //patli 20200220 16
#define APP_TASK_BACK_PRIO          14   //patli 20200220 19
#else
#define APP_TASK_BGUNBMS_PRIO       APP_TASK_AGUNBMS_PRIO
#define APP_TASK_DCMETER_PRIO       10
#define APP_TASK_DLMOD_PRIO         11
#define APP_TASK_SCREEN_PRIO        12   //patli 20200220 16
#define APP_TASK_BACK_PRIO          13   //patli 20200220 19
#endif


#define STK_CHECK_TASK_PRIO	20

#define APP_TASK_AGUNMAINCTRL_PRIO  APP_TASK_AGUNBMS_PRIO
#define APP_TASK_BGUNMAINCTRL_PRIO  APP_TASK_BGUNBMS_PRIO

#define APP_TASK_REALCHK_PRIO       APP_TASK_DCMETER_PRIO
#define APP_TASK_ISO_PRIO           APP_TASK_DCMETER_PRIO
#define APP_TASK_ACMETER_PRIO       APP_TASK_DCMETER_PRIO

#define APP_TASK_CARD_PRIO          APP_TASK_SCREEN_PRIO

#define TCPCLIENT_PRIO 				NET_DEBUG_TASK_PRIO

 
#define  OS_TASK_TMR_PRIO              (OS_LOWEST_PRIO - 2)



#undef  DEFAULT_THREAD_PRIO
#define DEFAULT_THREAD_PRIO		2




/*
*********************************************************************************************************
*                                            TASK WHILE(1) DELAY TIME
*********************************************************************************************************
*/

#define APP_TASK_START_DELAY        	50   	//500ms

#define APP_TASK_AGUNBMS_DELAY       	25  	//25	//20		//200ms
#define APP_TASK_DCMETER_DELAY       	20  //20  //15		//150ms
#define APP_TASK_DLMOD_DELAY         	10		//200ms
#define APP_TASK_SCREEN_DELAY        	15  //20  //15 	//150ms
#define APP_TASK_BACK_DELAY          	15 		//100ms  
#define TCPCLIENT_DELAY					20
#define LWIP_DHCP_TASK_DELAY			200



//OS_STK TCPCLIENT_TASK_STK[TCPCLIENT_STK_SIZE];

/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*********************************************************************************************************
*/

#define  APP_START_STK_SIZE	 			356  //356   //patli 20200205    300	
#define  APP_DCMET_STK_SIZE   	    	400  //400   //356   //patli 20200205   300   //200
//#define  APP_ISO_STK_SIZE     	    200
//#define  APP_ACMET_STK_SIZE   	    200
//#define  APP_CARD_STK_SIZE    	    	300		//patli 20200205  200
#define  APP_DLMOD_STK_SIZE   	    	400  //400    //356     //patli 20200205 300
#define  APP_SCREEN_MAIIN_SIZE      	800  //800		//patli 2020228 768    
#define  APP_BMS_STK_SIZE           	700  //600  //600		//patli 20200205 250		//200
#define  APP_MAINCTRL_STK_SIZE      	512
#define  APP_BACK_STK_SIZE          	700	 //512
//#define  APP_LWIP_STK_SIZE          	512
#define  APP_USBH_STK_SIZE          	512
#define  TASK_USBHBOOTLOADER_STK_SIZE 	3000  //512
#define  TCPCLIENT_STK_SIZE	  			400
#define  LWIP_DHCP_STK_SIZE				156


/*
*********************************************************************************************************
*                               uC/Probe plug-in for uC/OS-II CONFIGURATION
*********************************************************************************************************
*/

#define  OS_PROBE_TASK                         1                /* Task will be created for uC/Probe OS Plug-In             */
#define  OS_PROBE_TMR_32_BITS                  0                /* uC/Probe OS Plug-In timer is a 16-bit timer              */
#define  OS_PROBE_TIMER_SEL                    2
#define  OS_PROBE_HOOKS_EN                     1

/*
*********************************************************************************************************
*                                          uC/LCD CONFIGURATION
*********************************************************************************************************
*/

#define  uC_CFG_OPTIMIZE_ASM_EN                 DEF_ENABLED
#define  DISP_BUS_WIDTH                        4                /* LCD controller is accessed with a 4 bit bus              */
#define  LIB_STR_CFG_FP_EN                     DEF_DISABLED


#define  IC_CARD_EN                 DEF_ENABLED	//IC刷卡功能使能
#define  POWER24V_EN                DEF_ENABLED		//24V电源控制功能使能

#endif
