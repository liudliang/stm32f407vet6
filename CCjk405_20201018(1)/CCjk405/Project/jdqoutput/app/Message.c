
/**
  ******************************************************************************
  * @file    message.c
  * @author  zqj
  * @version v1.0
  * @date    2016-02-24
  * @brief   
  ******************************************************************************
	*/
 #include "main.h"
//#include "M_Global.h"
#include "Message.h"
#include "..\queue\queue.h"
#include "ucos_ii.h"

#include <string.h>

extern OS_EVENT *msg_smp;

// =========  Task_IsoCheck  ================================
#define   APP_ISO_QUEUE_SIZE  5
void      *AppIsoQueueTbl[APP_ISO_QUEUE_SIZE];
OS_EVENT  *AppIsoQueue = NULL;

// ======== TaskCardReader   ================================ 
#define   APP_CARD_QUEUE_SIZE  10
void      *AppCardReaderQueueTbl[APP_CARD_QUEUE_SIZE];
OS_EVENT  *AppCardReaderQueue=NULL; 

// ======== Task_CdModule   ================================ 
#define    CDMODULE_QUEUE_SIZE  15
void      *AppCdmoduleQueueTbl[CDMODULE_QUEUE_SIZE];
OS_EVENT  *AppCdmoduleQueue = NULL; 

// ======== Task_Screen_Main     ================================ 
#define    SCREEN_QUEUE_SIZE  15
void      *AppScreenQueueTbl[SCREEN_QUEUE_SIZE];
OS_EVENT  *AppScreenQueue = NULL; 

// ======== TaskBmsComm       ================================ 
//#define    BMS_QUEUE_SIZE  5
//void      *AppBmsQueueTbl[BMS_QUEUE_SIZE];
//OS_EVENT  *AppBmsQueue = NULL; 

// ======== TaskMainCtrl       ================================ 
#define    AGUNMAINCTRL_QUEUE_SIZE  30
void      *AppAGunMainCtrlQueueTbl[AGUNMAINCTRL_QUEUE_SIZE];
OS_EVENT  *AppAGunMainCtrlQueue = NULL; 

#define    BGUNMAINCTRL_QUEUE_SIZE  20
void      *AppBGunMainCtrlQueueTbl[BGUNMAINCTRL_QUEUE_SIZE];
OS_EVENT  *AppBGunMainCtrlQueue = NULL; 



// ======== Task_BackComm       ================================ 
#define    BACKCOMM_QUEUE_SIZE  10
void      *AppBackCommQueueTbl[BACKCOMM_QUEUE_SIZE];
OS_EVENT  *AppBackCommQueue = NULL; 

#ifdef AN_ETH_TASK
// ======== tcp_client_thread       ================================ 
#define    TCP_CLIENT_QUEUE_SIZE  10
void      *AppTcpClientQueueTbl[TCP_CLIENT_QUEUE_SIZE];
OS_EVENT  *AppTcpClientQueue = NULL; 
#endif

// ======== dhcp_thread       ================================ 

#ifdef LWIP_DHCP
#define    DHCP_QUEUE_SIZE  5
void      *AppDhcpQueueTbl[DHCP_QUEUE_SIZE];
OS_EVENT  *AppDhcpQueue = NULL; 
#endif

#ifdef NET_DEBUG	
#define    TCP_DEBUG_QUEUE_SIZE  5
void      *AppTcpDebugQueueTbl[TCP_DEBUG_QUEUE_SIZE];
OS_EVENT  *AppTcpDebugQueue = NULL;
#endif

#ifdef AN_ETH_TASK
#define MAX_MSG_NUM			(APP_ISO_QUEUE_SIZE + APP_CARD_QUEUE_SIZE + CDMODULE_QUEUE_SIZE\
                        + SCREEN_QUEUE_SIZE + AGUNMAINCTRL_QUEUE_SIZE + BGUNMAINCTRL_QUEUE_SIZE + BACKCOMM_QUEUE_SIZE\
                        + TCP_CLIENT_QUEUE_SIZE + TCP_DEBUG_QUEUE_SIZE)	
#else
#define MAX_MSG_NUM			(APP_ISO_QUEUE_SIZE + APP_CARD_QUEUE_SIZE + CDMODULE_QUEUE_SIZE\
                        + SCREEN_QUEUE_SIZE + AGUNMAINCTRL_QUEUE_SIZE + BGUNMAINCTRL_QUEUE_SIZE + BACKCOMM_QUEUE_SIZE)	
#endif

#define REM_MSG_NUM   20

												
static 	MSG_STRUCT MsgArray[MAX_MSG_NUM];

 
BOOL  Message_QueueCreat(uint8 TaskPrio)
{
	static uint8 poweron = 1;
#ifdef TASK_MESSAGE_PRINT				
	printf("Message_QueueCreat MAX_MSG_NUM=%d\r\n", MAX_MSG_NUM); 
#endif

	if(poweron==1)
	{
	 	memset(MsgArray,0,sizeof(MSG_STRUCT)*MAX_MSG_NUM);
		poweron=0;
	}
	
	switch(TaskPrio)
	{
		case APP_TASK_ISO_PRIO:
		{	
			AppIsoQueue = OSQCreate(&AppIsoQueueTbl[0], APP_ISO_QUEUE_SIZE);	
			break;
		}
		
		case APP_TASK_DLMOD_PRIO:
		{
			AppCdmoduleQueue = OSQCreate(&AppCdmoduleQueueTbl[0], CDMODULE_QUEUE_SIZE);	
			break;
		}
		case APP_TASK_SCREEN_PRIO:
		{
			 AppScreenQueue = OSQCreate(&AppScreenQueueTbl[0],SCREEN_QUEUE_SIZE);
		}
		break;
//		case APP_TASK_BMS_PRIO:
//		{
//			 AppBmsQueue = OSQCreate(&AppBmsQueueTbl[0],BMS_QUEUE_SIZE);
//		}
//		break;
		case APP_TASK_AGUNMAINCTRL_PRIO:
		{
			AppAGunMainCtrlQueue = OSQCreate(&AppAGunMainCtrlQueueTbl[0],AGUNMAINCTRL_QUEUE_SIZE);
		}
		break;
#ifndef A_B_GUN_TOGETER
		case APP_TASK_BGUNMAINCTRL_PRIO:
		{
			AppBGunMainCtrlQueue = OSQCreate(&AppBGunMainCtrlQueueTbl[0],BGUNMAINCTRL_QUEUE_SIZE);
		}
		break;
#endif
		case APP_TASK_BACK_PRIO:
		{
			AppBackCommQueue = OSQCreate(&AppBackCommQueueTbl[0],BACKCOMM_QUEUE_SIZE);
		}
		break;
#ifdef AN_ETH_TASK
		case TCPCLIENT_PRIO:
		{
			AppTcpClientQueue = OSQCreate(&AppTcpClientQueueTbl[0],TCP_CLIENT_QUEUE_SIZE);
		}
		break;
#endif	

#ifdef LWIP_DHCP
		case LWIP_DHCP_TASK_PRIO:
		{
			AppDhcpQueue = OSQCreate(&AppDhcpQueueTbl[0],DHCP_QUEUE_SIZE);
		}
		break;
#endif		
#if 0 //def NET_DEBUG	
		case NET_DEBUG_TASK_PRIO:
		{
			AppTcpDebugQueue = OSQCreate(&AppTcpDebugQueueTbl[0],TCP_DEBUG_QUEUE_SIZE);
		}
		break;
#endif		

		default: 
			break;	
	}

	return TRUE;
}

void MsgClearAll()
{
	uint8 i;
	
	for(i=0;i<MAX_MSG_NUM;i++)
	{
		 MsgArray[i].Valid=0;
	}
}
// 
uint8* Msg2Queue(MSG_STRUCT *msg)
{	
#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif    

	uint8 i = 3,j; 
//	while(i--)
	for(j=0;j<MAX_MSG_NUM;j++)
	{
		 if(MsgArray[j].Valid==0)
		 {
				OS_ENTER_CRITICAL();
				memcpy(&MsgArray[j],msg ,sizeof(MSG_STRUCT));
				MsgArray[j].Valid = 1;
				OS_EXIT_CRITICAL();

				if(j > MAX_MSG_NUM - REM_MSG_NUM)
				{
#ifdef TASK_MESSAGE_PRINT				
					printf("Msg2Queue msg full\r\n"); 
#endif
					msg_full = 1;
				}
				else
				{				
					msg_full = 0;
				}
				

				return 	(uint8*)&MsgArray[j];
		 }
	}

	return NULL;
}

// ============================================================================= 
//
//  
//==============================================================================
BOOL  SendMsg2Queue(MSG_STRUCT *msg, uint8 DestPrio)
{
	uint8* ptrMem = NULL;
	BOOL   ret = FALSE;
	if(msg == NULL)
		return ret;

#ifdef TASK_MESSAGE_PRINT				
//	printf("SendMsg2Queue OSPrioCur=%d; DestPrio=%d; msg type=%d\r\n", OSPrioCur, DestPrio, msg->MsgType); 
#endif
//	ScommDebugPrintStrWithPi("SendMsg2Queue DestPrio", DestPrio);
	

	if((ptrMem = Msg2Queue(msg)) != NULL)
	{
		switch(DestPrio)
		{
			case APP_TASK_ISO_PRIO:
			{
				OSQPost(AppIsoQueue, (void*)ptrMem);
				ret = TRUE;				
			}		
			break; 


			case APP_TASK_DLMOD_PRIO:
			{
				OSQPost(AppCdmoduleQueue,(void*)ptrMem);
				ret = TRUE;	
			}
			break;
			case APP_TASK_SCREEN_PRIO:
			{
				OSQPost(AppScreenQueue,(void*)ptrMem);
				ret = TRUE;
			}
			break;
			
//			case APP_TASK_BMS_PRIO:
//			{
//				OSQPost(AppBmsQueue,(void*)ptrMem);
//				ret = TRUE;
//			}
//			break;
			
			case APP_TASK_AGUNMAINCTRL_PRIO:
			{
				OSQPost(AppAGunMainCtrlQueue,(void*)ptrMem);
				ret = TRUE;
			}
			break;
#ifndef A_B_GUN_TOGETER
			case APP_TASK_BGUNMAINCTRL_PRIO:
			{
				OSQPost(AppBGunMainCtrlQueue,(void*)ptrMem);
				ret = TRUE;
			}
			break;
#endif
			
			case APP_TASK_BACK_PRIO:
			{
				OSQPost(AppBackCommQueue,(void*)ptrMem);
				ret = TRUE;
			}
			break;
#ifdef AN_ETH_TASK
			case TCPCLIENT_PRIO:
			{
				OSQPost(AppTcpClientQueue,(void*)ptrMem);
				ret = TRUE;
			}
			break;
#endif	
#ifdef LWIP_DHCP
			case LWIP_DHCP_TASK_PRIO:
			{
				OSQPost(AppDhcpQueue,(void*)ptrMem);
				ret = TRUE;
			}
			break;
#endif		
#if 0 //def NET_DEBUG	
			case NET_DEBUG_TASK_PRIO:
			{
				OSQPost(AppTcpDebugQueue,(void*)ptrMem);
				ret = TRUE;
			}
			break;
#endif		
			default: 
				break;	
		}
	}				
	return ret;
}

#if 0

/*接收指定任务的消息*/
BOOL RcvPrioMsgFromQueue(MSG_STRUCT* outmsg,uint8 prio)
{
		
#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif   

	BOOL ret = FALSE;
	MSG_STRUCT* msg = NULL;
	uint8 CurrentPrio, err;

	CurrentPrio = prio;
	switch(CurrentPrio)
	{
		case APP_TASK_AGUNMAINCTRL_PRIO:
		{			
			msg = (MSG_STRUCT*)OSQAccept(AppAGunMainCtrlQueue, &err);
			break;
		}
		case APP_TASK_BGUNMAINCTRL_PRIO:
		{			
			msg = (MSG_STRUCT*)OSQAccept(AppBGunMainCtrlQueue, &err);
			break;
		}
		default: 
			break;	
	}
	
	if(msg!=NULL)
	{	 
		OS_ENTER_CRITICAL();
		memcpy(outmsg,msg,sizeof(MSG_STRUCT));
		msg->Valid = 0;
		OS_EXIT_CRITICAL();
		ret = TRUE;
	}
	return ret;

}
#endif

// ============================================================================= 
// ????  
// ?  
// ??:msg ??????? FALSE ?? 
//==============================================================================

BOOL RcvMsgFromQueue(MSG_STRUCT* outmsg)
{
	
#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif   

	BOOL ret = FALSE;
	MSG_STRUCT* msg = NULL;
	uint8 CurrentPrio, err;
	
#ifdef TASK_MESSAGE_PRINT				
//		printf("RcvMsgFromQueue OSPrioCur=%d\r\n", OSPrioCur); 
#endif

	CurrentPrio = OSTCBCur->OSTCBPrio;
	switch(CurrentPrio)
	{
		case APP_TASK_ISO_PRIO:
		{
			msg = (MSG_STRUCT*)OSQAccept(AppIsoQueue, &err);
			break;
		}
		
		case APP_TASK_DLMOD_PRIO:
		{			
			msg = (MSG_STRUCT*)OSQAccept(AppCdmoduleQueue, &err);
			break;
		}
		case APP_TASK_SCREEN_PRIO:
		{			
			msg = (MSG_STRUCT*)OSQAccept(AppScreenQueue, &err);
			break;
		}
//		case APP_TASK_BMS_PRIO:
//		{			
//			msg = (MSG_STRUCT*)OSQAccept(AppBmsQueue, &err);
//			break;
//		}
		case APP_TASK_AGUNMAINCTRL_PRIO:
		{			
			msg = (MSG_STRUCT*)OSQAccept(AppAGunMainCtrlQueue, &err);
			break;
		}
#ifndef A_B_GUN_TOGETER
		case APP_TASK_BGUNMAINCTRL_PRIO:
		{			
			msg = (MSG_STRUCT*)OSQAccept(AppBGunMainCtrlQueue, &err);
			break;
		}
#endif
		case APP_TASK_BACK_PRIO:
		{			
			msg = (MSG_STRUCT*)OSQAccept(AppBackCommQueue, &err);
			break;
		}
#ifdef AN_ETH_TASK
		case TCPCLIENT_PRIO:
		{			
			msg = (MSG_STRUCT*)OSQAccept(AppTcpClientQueue, &err);
			break;
		}
#endif

#ifdef LWIP_DHCP
		case LWIP_DHCP_TASK_PRIO:
		{
			msg = (MSG_STRUCT*)OSQAccept(AppDhcpQueue, &err);
			break;
		}
#endif		
#if 0 //def NET_DEBUG	
		case NET_DEBUG_TASK_PRIO:
		{
			msg = (MSG_STRUCT*)OSQAccept(AppTcpDebugQueue, &err);
		}
		break;
#endif		

		default: 
			break;	
	}
	if(msg!=NULL)
	{	 
		OS_ENTER_CRITICAL();
		memcpy(outmsg,msg,sizeof(MSG_STRUCT));
		msg->Valid = 0;
		OS_EXIT_CRITICAL();
		ret = TRUE;
	}
	return ret;
}


/* ?????? */
void SendMsgPack(uint8 Type,uint8 Len,uint8 Data,uint8 Prio)
{
	int8 s_err = 0;
	MSG_STRUCT retMsg;
	
	//请求信号量
//	OSSemPend(msg_smp,0,&s_err);	//
	
	retMsg.MsgType = Type;
	retMsg.MsgLenth = Len;
	retMsg.MsgData[0] = Data;
	SendMsg2Queue(&retMsg,Prio);

	
	//释放信号量
//	OSSemPost(msg_smp);
}

/* ?????? */
//void SendMsgDoubleByte(uint8 Type,uint8 Len,void *Data,uint8 Prio)
//{
//	MSG_STRUCT retMsg;
//	uint8 *ptr = (uint8 *)Data;
//	
//	retMsg.MsgType = Type;
//	retMsg.MsgLenth = Len;
//	if(Len != 0)
//	{
//		for(uint8 i=0; i < Len; i++)
//			retMsg.MsgData[i] = ptr[i];
//	}
//	
//	SendMsg2Queue(&retMsg,Prio);
//}


/*发送多字节消息*/
void SendMsgWithNByte(uint8 Type,uint8 Len,void *Data,uint8 Prio)
{
	int8 s_err = 0;
	MSG_STRUCT retMsg;
	//请求信号量
//	OSSemPend(msg_smp,0,&s_err);	//
	
	uint8 *ptr = (uint8 *)Data;
	{
		retMsg.MsgType = Type;
		retMsg.MsgLenth = Len;
		if( (Len != 0) && (ptr != NULL) )
		{
			for(uint8 i=0; i < Len; i++)
				retMsg.MsgData[i] = ptr[i];
		}
		
		SendMsg2Queue(&retMsg,Prio);
	}	

	//释放信号量
//	OSSemPost(msg_smp);
	
}

void SendMsgWithNByteByPtr(uint8 Type,uint8 Len,void *Data,uint8 Prio)
{
	int8 s_err = 0;
	MSG_STRUCT retMsg;
	
	//请求信号量
//	OSSemPend(msg_smp,0,&s_err);	//
	
	uint8 *ptr = (uint8 *)Data;	
	
	if( (Len != 0) && (Data != NULL) )
	{
		retMsg.MsgType = Type;
		retMsg.MsgLenth = Len;
		retMsg.PtrMsg = Data;
		
		SendMsg2Queue(&retMsg,Prio);
	}
	
	//释放信号量
//	OSSemPost(msg_smp);
		
}






