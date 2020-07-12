#ifndef _M_GLOBAL_H
#define _M_GLOBAL_H

#include "sys.h"
#include "string.h"

//////////////////////////////////////////////////////////////////////////////////	 
//功能： 工程的功能配置、各优先级配置
//作者：
//创建日期:2020/04/25
//版本：V1.0									  
//////////////////////////////////////////////////////////////////////////////////

#define DEBUG_ON                  //调试打开
#define SCOMM_DEBUG               //串口调试   
#define ETH_DEBUG                 //网口调试 
#define USB_HOST_SUPPORT          //usb host功能支持


/*
**********************************************************************************
*                            任务优先级
**********************************************************************************
*/

#define START_TASK_PRIO		        3
#define TEST_TASK_PRIO		        4
#define CARDREADER_TASK_PRIO		  5
#define FLOAT_TASK_PRIO		        6
#define RealTimeCheck_TASK_PRIO		7
#define TCPCLIENT_PRIO		        8
#define DEBUG_TCPCLIENT_PRIO		  9
#define LWIP_DHCP_TASK_PRIO       10
#define USBHost_TASK_PRIO         11
#define TaskStackUsage_TASK_PRIO	30




/*
**********************************************************************************
*                            任务栈大小
**********************************************************************************
*/

#define START_STK_SIZE 		         512
#define TEST_STK_SIZE 		         128*4
#define CARDREADER_STK_SIZE 		   128
#define FLOAT_STK_SIZE		         256
#define RealTimeCheck_STK_SIZE		 128
#define TaskStackUsage_STK_SIZE 	 128
#define TCPCLIENT_STK_SIZE	       300
#define DEBUG_TCPCLIENT_STK_SIZE	 300
#define LWIP_DHCP_STK_SIZE  		   256
#define USBHost_STK_SIZE 	         256




#endif

