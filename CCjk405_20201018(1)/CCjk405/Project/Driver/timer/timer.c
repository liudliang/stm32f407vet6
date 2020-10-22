/****************************************Copyright (c)**************************************************
        深圳奥耐电气技术有限公司
Filename: timer.c
Author: patli
Datetime: 20190930
Version: v0.01
****************************************************************************************************/
#include "Common.h"
#include "TaskBackComm.h"
#include "GbtBmsProto.h"
#include "Timer.h"
#include "Adebug.h"
#include "main.h"
#include "gpio.h"


OS_EVENT *tm_bms_mutex;  //保护全局变量和流程

uint32 DevRunSec = 0;     //设备运行多少秒

uint32 time_test_num = 0;

static u8 cnt100 = 0;  


extern MSG_WORK_STATUS_STRUCT  m_msg_work_status;

extern void TaskBMSRecvProc(void);
extern void Gbt_SendData(GSEND_INFO *sendinf,uint8 *pbuf,uint16 length);
extern void Read_AllInputIo(void);

void Timer_BmsCricleReport(SEND_PROTO_ST *ptrList, uint8 gunNO)
{	
	uint16 len = 0;
	uint16 i = 0;
	GSEND_INFO ctrlinf ;
	uint8 buf[50] = {0}, err;
	SEND_PROTO_ST *pItem = NULL;
	
	//请求互斥量
	
//	OSMutexPend(tm_bms_mutex,500,&err);   //patli 20191203

#ifdef ETH_TEMP_TEST
//	DebugInfoByBms("Timer_BmsCricleReport.....BMS TEST");
#endif
	
	if(CC1_4V==Check_GetGunDataPtr(gunNO)->statu.bits.cc1stu)
	{

		for(i = 0;ptrList[i].frmtype != 0xff; i++) {
			if( ptrList[i].allowed == CRICLE_ALLOWED ) {
				pItem = &ptrList[i];
					if( NULL != pItem->pkg ) {
					memset(&ctrlinf,0,sizeof(GSEND_INFO));
					ctrlinf.gunNo = gunNO;
					ctrlinf.pf = pItem->pgn;
					ctrlinf.pri = pItem->prio;
					ctrlinf.srcAdr = CHARGER_ADDR;
					ctrlinf.objAdr = BMS_ADDR;
					ctrlinf.frmNum = 1;
					len = pItem->pkg(buf,(void *)&ctrlinf,gunNO);
					if(len > 0 ) {
						ctrlinf.frmNum = (len / 8 ) + ((len % 8) > 0);
						Gbt_SendData(&ctrlinf,buf,len);
					}
				}
			}
		}
	}

	//释放互斥量
	
//	OSMutexPost(tm_bms_mutex);   //patli 20191203	
}

/*因为系统初始化SystemInit函数里初始化APB1总线时钟为4分频即42M，所以TIM2~TIM7、TIM12~TIM14的时钟为APB1的时钟的两倍即84M*/
void tim2_1s_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;		 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);		
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;	
	TIM_TimeBaseInitStruct.TIM_CounterMode   = TIM_CounterMode_Up;	
	TIM_TimeBaseInitStruct.TIM_Period        = 9999;   	
	TIM_TimeBaseInitStruct.TIM_Prescaler     = 8399;   //7199;	STM32F4XX APB1是84MHZ,不是72MHZ，STM32F1XX是72MHZ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);		 
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);		 	
	TIM_Cmd(TIM2,ENABLE);
}

void tim2_1s_NVIC_init(void)
{	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel      = TIM2_IRQn;  
	NVIC_InitStruct.NVIC_IRQChannelCmd   = ENABLE;	
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority  =1; // 1;	
	NVIC_InitStruct.NVIC_IRQChannelSubPriority   = 7; 	
	NVIC_Init(&NVIC_InitStruct);
}

uint8 tim2_5S_count = 0;
void TIM2_IRQHandler(void)
{ 	
	OS_CPU_SR  cpu_sr;
	OS_ENTER_CRITICAL();                                       // Tell uC/OS-II that we are starting an ISR
	OSIntNesting++;
	OS_EXIT_CRITICAL();
	
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) 		
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	

		DevRunSec++;		//设备运行时长加1秒

		//2s处理
		if(DevRunSec%2 == 0)
		{
			TIM2_2sIRQHandler();
		}
		
		//3s处理
		if(DevRunSec%3 == 0)
		{
			TIM2_3sIRQHandler();
		}
		
		//5s处理
		if(DevRunSec%5 == 0)
		{
			TIM2_5sIRQHandler();
		}

		SendMsgWithNByte(MSG_MOD_GET_INFO,0,NULL,APP_TASK_DLMOD_PRIO); 
		

	}

	OSIntExit();												// Tell uC/OS-II that we are leaving the ISR 
	
}

//2s处理函数
extern uint16 GetADC1ChannelValue(void);
extern void ADC_Debug(void);
void TIM2_2sIRQHandler(void)
{
	
#ifdef 	NET_DEBUG	
			//每5S发送充电机遥测消息给后台，后台发送遥测数据给通信板
	//  printf("TASKS:0x%x;GUNS:0x%x;CANS:0x%x;UARTS:0x%x;MSGF:%d\r\n",task_run_status, gun_status, can_status, uart_status, msg_full);	
#endif
	
// 	GetADC1ChannelValue();
	ADC_Debug();

}


//3s处理函数
void TIM2_3sIRQHandler(void)
{
	
#ifdef 	NET_DEBUG	
			//每5S发送充电机遥测消息给后台，后台发送遥测数据给通信板
	//  printf("TASKS:0x%x;GUNS:0x%x;CANS:0x%x;UARTS:0x%x;MSGF:%d\r\n",task_run_status, gun_status, can_status, uart_status, msg_full);	
		SendMsgToNetDebug(MSG_TO_PRINT_TASK_STATUS,0,NULL); 

#endif	

		//插枪中未充电要发送枪连接状态
	//	if(GetWorkType(AGUN_NO) == 1 || GetWorkType(BGUN_NO) == 1)
	//		SendMsgWithNByte(MSG_BAC_AUTO_UPDATE,0,NULL,APP_TASK_BACK_PRIO); 

}

//5s处理函数
void TIM2_5sIRQHandler(void)
{
	
#ifdef 	NET_DEBUG	
			//每5S发送充电机遥测消息给后台，后台发送遥测数据给通信板
	//  printf("TASKS:0x%x;GUNS:0x%x;CANS:0x%x;UARTS:0x%x;MSGF:%d\r\n",task_run_status, gun_status, can_status, uart_status, msg_full);

		SendMsgToNetDebug(MSG_DEV_TO_DEBUG_HB,0,NULL); 
		ClearDebugStatus();
	
#endif	
}


void tim3_500ms_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;		 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);		
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;	
	TIM_TimeBaseInitStruct.TIM_CounterMode   = TIM_CounterMode_Up;	
	TIM_TimeBaseInitStruct.TIM_Period        = 4999;   	
	TIM_TimeBaseInitStruct.TIM_Prescaler     = 8399;	
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);		 
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);		 	
	TIM_Cmd(TIM3,ENABLE);
}

void tim3_500ms_NVIC_init(void)
{	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel      = TIM3_IRQn;  
	NVIC_InitStruct.NVIC_IRQChannelCmd   = ENABLE;	
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority  =1; // 1;	
	NVIC_InitStruct.NVIC_IRQChannelSubPriority   = 6;  //6; 	
	NVIC_Init(&NVIC_InitStruct);
}

void TIM3_IRQHandler(void)
{ 	
//	CHG_DATA_ST *chg_ptr = (CHG_DATA_ST *)Bms_GetChgrDataPtr() ;
	PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	OS_CPU_SR  cpu_sr;
	OS_ENTER_CRITICAL();                                       // Tell uC/OS-II that we are starting an ISR
	OSIntNesting++;
	OS_EXIT_CRITICAL();
	
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) 		
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);	
		//发送500MS的BMS帧
		for(int i=0;i<devparaPtr->gunnum;i++)
		{
			Timer_BmsCricleReport(gChgProtoListA_500ms, i);
		//	Timer_BmsCricleReport(gChgProtoListB_500ms, BGUN_NO);
		}
#ifdef ETH_TEMP_TEST
//		DebugInfoByBms("TIM3_IRQHandler....500MS...BMS TEST");
//	    ScommDebugPrintStrWithPi("TIM3_IRQHandler 500ms num\n", time_test_num++);

#endif
		
	
#if 1
		if(m_msg_work_status.valid)
		{
  			SendMsgWithNByte(MSG_WORK_STATU,sizeof(m_msg_work_status),&m_msg_work_status,APP_TASK_SCREEN_PRIO);	
			m_msg_work_status.valid = 0;
		}
#endif	

#if 0		
		if(GetWorkStep(AGUN_NO) == STEP_IDEL && GetWorkStep(BGUN_NO) == STEP_IDEL)
			SendMsgWithNByte(MSG_TO_ENTER_IDLE,0,NULL,APP_TASK_AGUNBMS_PRIO); 
#endif

	}

	OSIntExit();												// Tell uC/OS-II that we are leaving the ISR 
	
}


void tim4_250ms_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;		 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);		
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;	
	TIM_TimeBaseInitStruct.TIM_CounterMode   = TIM_CounterMode_Up;	
	TIM_TimeBaseInitStruct.TIM_Period        = 2499;   	
	TIM_TimeBaseInitStruct.TIM_Prescaler     = 8399;	
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStruct);		 
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);		 	
	TIM_Cmd(TIM4,ENABLE);
}

void tim4_250ms_NVIC_init(void)
{	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel      = TIM4_IRQn;  
	NVIC_InitStruct.NVIC_IRQChannelCmd   = ENABLE;	
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority  = 1;  //1;	
	NVIC_InitStruct.NVIC_IRQChannelSubPriority   = 5; // 5	
	NVIC_Init(&NVIC_InitStruct);
}


void TIM4_IRQHandler(void)
{ 	
	PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	OS_CPU_SR  cpu_sr;
	OS_ENTER_CRITICAL();                                       // Tell uC/OS-II that we are starting an ISR
	OSIntNesting++;
	OS_EXIT_CRITICAL();
	
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) == SET) 		
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);	

		
		//发送250MS的BMS帧
		for(int i=0;i<devparaPtr->gunnum;i++)
		{
		
			Timer_BmsCricleReport(gChgProtoListA_250ms,i);
		//	Timer_BmsCricleReport(gChgProtoListB_250ms,BGUN_NO);
		}
#ifdef ETH_TEMP_TEST
//			DebugInfoByBms("TIM4_IRQHandler....250MS...BMS TEST");
//		ScommDebugPrintStrWithPi("TIM4_IRQHandler 250ms num\n", time_test_num++);

#endif
		
#if 0
		if(m_msg_work_status.valid)
		{
			SendMsgWithNByte(MSG_WORK_STATU,sizeof(m_msg_work_status),&m_msg_work_status,APP_TASK_SCREEN_PRIO); 
			m_msg_work_status.valid = 0;
		}
#endif	

	}

	OSIntExit();												// Tell uC/OS-II that we are leaving the ISR 
	
}

void tim5_100ms_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;		 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);		
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;	
	TIM_TimeBaseInitStruct.TIM_CounterMode   = TIM_CounterMode_Up;	
	TIM_TimeBaseInitStruct.TIM_Period        = 999;   //  999;   //49;   //999,  	
    TIM_TimeBaseInitStruct.TIM_Prescaler     = 8399;	
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStruct);		 
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);		 	
	TIM_Cmd(TIM5,ENABLE);
}

void tim5_100ms_NVIC_init()
{	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel      = TIM5_IRQn;  
	NVIC_InitStruct.NVIC_IRQChannelCmd   = ENABLE;	
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority  = 0;	
	NVIC_InitStruct.NVIC_IRQChannelSubPriority   = 7; 	
	NVIC_Init(&NVIC_InitStruct);
}

extern void AdcCalc_GetGunTempr(uint8 channel);
extern void SetCD4051BMTChannel(uint8 ch);
void TIM5_IRQHandler(void)
{ 	
//	CHG_DATA_ST *chg_ptr = (CHG_DATA_ST *)Bms_GetChgrDataPtr() ;
	OS_CPU_SR  cpu_sr;
	OS_ENTER_CRITICAL();                                       // Tell uC/OS-II that we are starting an ISR
	OSIntNesting++;
	OS_EXIT_CRITICAL();
	
	if(TIM_GetITStatus(TIM5, TIM_IT_Update) == SET) 		
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);	

		
#ifdef ETH_TEMP_TEST
//	DebugInfoByBms("TIM5_IRQHandler....100MS...BMS TEST");
//	ScommDebugPrintStrWithPi("TIM5_IRQHandler 100ms num\n", time_test_num++);
	//ScommDebugPrintStr("TIM5_IRQHandler\r\n");
#endif
//		T5msBMSRecvProc();
		//发送10MS的BMS帧
//		Timer_BmsCricleReport(gChgProtoListA_10ms, AGUN_NO);
//		Timer_BmsCricleReport(gChgProtoListB_10ms, BGUN_NO);

/*---------------------
(原理图上选择控制通道io与温度采集共用)
顺序:    读0  、读io、写1(选择1通道)，然后 读1  、读io、写0(选择0通道)，
以此方式循环
*/
		cnt100 ++;
		if(cnt100%2)
		{
		  AdcCalc_GetGunTempr(0);
		  Read_AllInputIo();
		  SetCD4051BMTChannel(1);		
		}
		else
		{
		  AdcCalc_GetGunTempr(1);
		  Read_AllInputIo();
		  SetCD4051BMTChannel(0);				
		}
//----------------------

		
	}

	OSIntExit();												// Tell uC/OS-II that we are leaving the ISR 
	
}

void tim6_50ms_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;		 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);		
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;	
	TIM_TimeBaseInitStruct.TIM_CounterMode   = TIM_CounterMode_Up;	
	TIM_TimeBaseInitStruct.TIM_Period        = 499,  	
	TIM_TimeBaseInitStruct.TIM_Prescaler     = 8399;	
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStruct);		 
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);		 	
	TIM_Cmd(TIM6,ENABLE);
}

void tim6_50ms_NVIC_init(void)
{	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel      = TIM6_DAC_IRQn;  
	NVIC_InitStruct.NVIC_IRQChannelCmd   = ENABLE;	
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority  = 0;	
	NVIC_InitStruct.NVIC_IRQChannelSubPriority   = 6; 	
	NVIC_Init(&NVIC_InitStruct);
}


void TIM6_DAC_IRQHandler(void)
{ 	
//	CHG_DATA_ST *chg_ptr = (CHG_DATA_ST *)Bms_GetChgrDataPtr() ;
	PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
    
	OS_CPU_SR  cpu_sr;
	OS_ENTER_CRITICAL();                                       // Tell uC/OS-II that we are starting an ISR
	OSIntNesting++;
	OS_EXIT_CRITICAL();
	
	if(TIM_GetITStatus(TIM6, TIM_IT_Update) == SET) 		
	{
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
		
		//发送50MS的BMS帧
		for(int i=0;i<devparaPtr->gunnum;i++)
		{

			Timer_BmsCricleReport(gChgProtoListA_50ms,i);		
		//	Timer_BmsCricleReport(gChgProtoListB_50ms,BGUN_NO);		
		}
		
#ifdef ETH_TEMP_TEST
//	DebugInfoByBms("TIM6_DAC_IRQHandler..50MS...BMS TEST");
//	ScommDebugPrintStrWithPi("TIM6_DAC_IRQHandler 50ms num\n", time_test_num++);
#endif

	}

	OSIntExit();												// Tell uC/OS-II that we are leaving the ISR s	
}

void tim7_10ms_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;		 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);		
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;	
	TIM_TimeBaseInitStruct.TIM_CounterMode   = TIM_CounterMode_Up;	
 	TIM_TimeBaseInitStruct.TIM_Period        = 99;    //299,  	   //  99  199
	TIM_TimeBaseInitStruct.TIM_Prescaler     = 8399;	
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseInitStruct);		 
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);		 	
	TIM_Cmd(TIM7,ENABLE);
//	TIM_Cmd(TIM7,DISABLE);
	
}

void tim7_10ms_NVIC_init(void)
{	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel      = TIM7_IRQn;  
	NVIC_InitStruct.NVIC_IRQChannelCmd   = ENABLE;	
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority  = 0;	
	NVIC_InitStruct.NVIC_IRQChannelSubPriority   = 5; 	
	NVIC_Init(&NVIC_InitStruct);
}

void TIM7_IRQHandler(void)
{ 	
//	CHG_DATA_ST *chg_ptr = (CHG_DATA_ST *)Bms_GetChgrDataPtr() ;
	PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();

	OS_CPU_SR  cpu_sr;
	OS_ENTER_CRITICAL();                                       // Tell uC/OS-II that we are starting an ISR
	OSIntNesting++;
	OS_EXIT_CRITICAL();
	
	
	if(TIM_GetITStatus(TIM7, TIM_IT_Update) == SET) 		
	{
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);		
	
		//发送10MS的BMS帧
		for(int i=0;i<devparaPtr->gunnum;i++)
		{
			Timer_BmsCricleReport(gChgProtoListA_10ms, i);
		//	Timer_BmsCricleReport(gChgProtoListB_10ms, BGUN_NO);
		}
		
#ifdef ETH_TEMP_TEST
//		DebugInfoByBms("TIM7_IRQHandler 10MS...BMS TEST");
//		ScommDebugPrintStrWithPi("TIM7_IRQHandler 10ms num\n", time_test_num++);
//		ScommDebugPrintStrWithPi("TIM7_IRQHandler 10ms gSystemTick\n", gSystemTick);
//		ScommDebugPrintStrWithPi("TIM7_IRQHandler 10ms OSTime\n", OSTimeGet());
#endif
		
//		T10msBMSRecvProc();
		TaskBMSRecvProc();

#if 0
		if(GetWorkStep(AGUN_NO) == STEP_CHARGEING || GetWorkStep(BGUN_NO) == STEP_CHARGEING)
		{
			TIM_SetAutoreload(TIM7, 99);  //10ms timer

		}else if(GetWorkStep(AGUN_NO) == STEP_IDEL&&GetWorkStep(BGUN_NO) == STEP_IDEL){
			TIM_SetAutoreload(TIM7, 999); 	//100ms timer
		}else{
			
			TIM_SetAutoreload(TIM7, 99);  //30ms timer
		}
#else
 		if(GetWorkStep(AGUN_NO) == STEP_IDEL&&GetWorkStep(BGUN_NO) == STEP_IDEL){
			TIM_SetAutoreload(TIM7, 999);	//100ms timer
		}
		else{		
			
			TIM_SetAutoreload(TIM7, 99);  //10ms timer
		}

#endif
		
	}

	OSIntExit();										// Tell uC/OS-II that we are leaving the ISR	
}


void TimerInit()
{
	uint8 err;
	
	tim2_1s_init();
	tim2_1s_NVIC_init();
	
	tim3_500ms_init();
	tim3_500ms_NVIC_init();
	
	tim4_250ms_init();
	tim4_250ms_NVIC_init();

	tim5_100ms_init();
	tim5_100ms_NVIC_init();

	tim6_50ms_init();
	tim6_50ms_NVIC_init();
	
	tim7_10ms_init();
	tim7_10ms_NVIC_init();
	

	
}





/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
