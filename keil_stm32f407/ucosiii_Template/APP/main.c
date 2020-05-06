#include "sys.h"
#include "delay.h"
#include "usart.h"


#include "gpio.h"
#include "dma.h"
#include "iwdg.h"
#include "timer.h"

#include "includes.h"
#include "os_app_hooks.h"


#include "MainData.h"

//ALIENTEK 探索者STM32F407开发板 UCOSIII实验
//例4-1 UCOSIII UCOSIII移植

//UCOSIII中以下优先级用户程序不能使用，ALIENTEK
//将这些优先级分配给了UCOSIII的5个系统内部任务
//优先级0：中断服务服务管理任务 OS_IntQTask()
//优先级1：时钟节拍任务 OS_TickTask()
//优先级2：定时任务 OS_TmrTask()
//优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
//优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com  
//广州市星翼电子科技有限公司  
//作者：正点原子 @ALIENTEK

//任务优先级
#define START_TASK_PRIO		3
//任务堆栈大小	
#define START_STK_SIZE 		512
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);

//任务优先级
#define LED1_TASK_PRIO		4
//任务堆栈大小	
#define LED1_STK_SIZE 		128
//任务控制块
OS_TCB Led1TaskTCB;
//任务堆栈	
CPU_STK LED1_TASK_STK[LED1_STK_SIZE];
void led1_task(void *p_arg);

////任务优先级
//#define LED2_TASK_PRIO		5
////任务堆栈大小	
//#define LED2_STK_SIZE 		128
////任务控制块
//OS_TCB Led2TaskTCB;
////任务堆栈	
//CPU_STK LED2_TASK_STK[LED2_STK_SIZE];
////任务函数
//void led2_task(void *p_arg);
//任务优先级
#define CARDREADER_TASK_PRIO		5
//任务堆栈大小	
#define CARDREADER_STK_SIZE 		128
//任务控制块
OS_TCB CardReaderTaskTCB;
//任务堆栈	
CPU_STK CARDREADER_TASK_STK[CARDREADER_STK_SIZE];
//任务函数
void CardReader_task(void *p_arg);

//任务优先级
#define FLOAT_TASK_PRIO		6
//任务堆栈大小
#define FLOAT_STK_SIZE		256
//任务控制块
OS_TCB	FloatTaskTCB;
//任务堆栈
CPU_STK	FLOAT_TASK_STK[FLOAT_STK_SIZE];
//任务函数
void float_task(void *p_arg);


//任务优先级
#define RealTimeCheck_TASK_PRIO		7
//任务堆栈大小
#define RealTimeCheck_STK_SIZE		128
//任务控制块
OS_TCB	RealTimeCheckTaskTCB;
//任务堆栈
CPU_STK	RealTimeCheck_TASK_STK[RealTimeCheck_STK_SIZE];
//任务函数
void RealTimeCheck_task(void *p_arg);


//任务优先级
#define TaskStackUsage_TASK_PRIO		30
//任务堆栈大小	
#define TaskStackUsage_STK_SIZE 		128
//任务控制块
OS_TCB TaskStackUsageTaskTCB;
//任务堆栈	
CPU_STK TaskStackUsage_TASK_STK[TaskStackUsage_STK_SIZE];
void TaskStackUsage_task(void *p_arg);



//led0任务函数
void led1_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	
	CHARGE_TYPE *PtrRunData = ChgData_GetRunDataPtr();
	uint8_t k2status = PtrRunData->input->statu.bits.key2;
	while(1)
	{
		if(k2status != PtrRunData->input->statu.bits.key2)
		{
			k2status = PtrRunData->input->statu.bits.key2;
			if(1 == k2status)
			{
				LED_Toggle(LED2);
			}
		}
		
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
	}
}

////led1任务函数
//void led2_task(void *p_arg)
//{
//	OS_ERR err;
//	p_arg = p_arg;
//	while(1)
//	{
//		LED_Toggle(LED3);
//		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
//	}
//}


//浮点测试任务
void float_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	uint8_t i;
	uint16_t adcx = 0;
	static float float_num[3]={0};
	float pro=0;//进度
	uint8_t SEND_BUF_SIZE = 20;
  uint16_t *SendBuff = DMA_GetAdcAver();
		
	while(1)
	{
//		float_num+=0.01f;
//		OS_CRITICAL_ENTER();	//进入临界区


//	  if(DMA_GetFlagStatus(DMA2_Stream0,DMA_FLAG_TCIF0)!=RESET)//等待DMA2_Steam0传输完成
//		{ 
//		  for(i = 0; i < SEND_BUF_SIZE;i++)
//			{
//				adcx += SendBuff[i];
//			}
//		    adcx = adcx/SEND_BUF_SIZE;
//		    DMA_ClearFlag(DMA2_Stream0,DMA_FLAG_TCIF0);//清除DMA2_Steam7传输完成标志
//		}
//		pro=DMA_GetCurrDataCounter(DMA2_Stream0);//得到当前还剩余多少个数据
//		pro=1-pro/30;//得到百分比	  
//		pro*=100;      			    //扩大100倍
		
//		adcx=Get_Adc_Average(ADC_Channel_10,20);
		for(i = 0; i < 3; i++)
		{
		  float_num[i]=(float)SendBuff[i]*(3.3/4096);
		}			
		printf("float_num[0]=%.4f,float_num[1]=%.4f,float_num[2]=%.4f,temperature=%f\r\n",float_num[0],float_num[1],float_num[2],DMA_GetTemprate());
//		OS_CRITICAL_EXIT();		//退出临界区
			 

//		LED_Toggle(LED3);  //运行灯
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1s

	}
}


//TaskStackUsage任务函数
void TaskStackUsage_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	CPU_STK_SIZE free,used;
	printf("app start running!\n");	
	while(1)
	{
		printf("CPU useage:%d\r\n", OSStatTaskCPUUsage);
		
		OSTaskStkChk(&TaskStackUsageTaskTCB,&free,&used,&err);
		printf("TaskStackUsageTask used/free:%d/%d  usage:%%%d\r\n",used,free,(used*100)/(used+free));
		
		OSTaskStkChk(&Led1TaskTCB,&free,&used,&err);
		printf("Led1TaskTCB used/free:%d/%d  usage:%%%d\r\n",used,free,(used*100)/(used+free));
		
		OSTaskStkChk(&CardReaderTaskTCB,&free,&used,&err);
		printf("Led2TaskTCB used/free:%d/%d  usage:%%%d\r\n",used,free,(used*100)/(used+free));
		
		OSTaskStkChk(&FloatTaskTCB,&free,&used,&err);
		printf("FloatTaskTCB used/free:%d/%d  usage:%%%d\r\n",used,free,(used*100)/(used+free));
		
		OSTaskStkChk(&RealTimeCheckTaskTCB,&free,&used,&err);
		printf("RealTimeCheckTaskTCB used/free:%d/%d  usage:%%%d\r\n",used,free,(used*100)/(used+free));
		
		printf("\r\n\r\n\r\n");
		
		OSTimeDlyHMSM(0,0,3,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时3s
	}
}


// ========================================================================================================
// void sysSTM32F40xAssertClocks(void)
// 描述: 	系统时钟检查
// 返回值: 无
// ========================================================================================================
static void sysSTM32F40xAssertClocks(void)
{
	RCC_ClocksTypeDef RCC_Clocks = { 0 };
	
	RCC_GetClocksFreq(&RCC_Clocks);
	
	if (RCC_Clocks.SYSCLK_Frequency != SystemCoreClock)
	{
		while (1);
	}
}

// ========================================================================================================
// void RCC_Configuration(void)
// 描述: 	使能高速时钟
// 返回值: 无
// ========================================================================================================
static void RCC_Configuration(void)
{																	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	sysSTM32F40xAssertClocks();
}

void Hanrdware_Init(void)
{

	delay_init(168);  	//时钟初始化
	RCC_Configuration();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组配置
//	uart_init(115200);  //串口初始化
	sys_SerialHwInit();
	
	InPut_OutPut_Init();
	sys_ADC1_Config();
	IWDG_Init(4,1500); //与分频数为 64,重载值为 1500,溢出时间为 3s
	TimerInit();        //定时器初始化
}


//开始任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
	
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif

#if OS_CFG_APP_HOOKS_EN				//使用钩子函数
	App_OS_SetAllHooks();			
#endif	
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		

  MainCtrlUnit_Init();
	
	OS_CRITICAL_ENTER();	//进入临界区
	
	//创建检测各任务堆栈使用情况任务
	OSTaskCreate((OS_TCB 	* )&TaskStackUsageTaskTCB,		
				 (CPU_CHAR	* )"TaskStackUsage task", 		
                 (OS_TASK_PTR )TaskStackUsage_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TaskStackUsage_TASK_PRIO,     
                 (CPU_STK   * )&TaskStackUsage_TASK_STK[0],	
                 (CPU_STK_SIZE)TaskStackUsage_STK_SIZE/10,	
                 (CPU_STK_SIZE)TaskStackUsage_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);
	
	//创建LED1任务
	OSTaskCreate((OS_TCB 	* )&Led1TaskTCB,		
				 (CPU_CHAR	* )"led1 task", 		
                 (OS_TASK_PTR )led1_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )LED1_TASK_PRIO,     
                 (CPU_STK   * )&LED1_TASK_STK[0],	
                 (CPU_STK_SIZE)LED1_STK_SIZE/10,	
                 (CPU_STK_SIZE)LED1_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
				 
	//创建CARDREADER任务
	OSTaskCreate((OS_TCB 	* )&CardReaderTaskTCB,		
				 (CPU_CHAR	* )"CardReader task", 		
                 (OS_TASK_PTR )CardReader_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )CARDREADER_TASK_PRIO,     	
                 (CPU_STK   * )&CARDREADER_TASK_STK[0],	
                 (CPU_STK_SIZE)CARDREADER_STK_SIZE/10,	
                 (CPU_STK_SIZE)CARDREADER_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);
				 
	//创建浮点测试任务
	OSTaskCreate((OS_TCB 	* )&FloatTaskTCB,		
				 (CPU_CHAR	* )"float test task", 		
                 (OS_TASK_PTR )float_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )FLOAT_TASK_PRIO,     	
                 (CPU_STK   * )&FLOAT_TASK_STK[0],	
                 (CPU_STK_SIZE)FLOAT_STK_SIZE/10,	
                 (CPU_STK_SIZE)FLOAT_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);				
	//创建实时检测任务
	OSTaskCreate((OS_TCB 	* )&RealTimeCheckTaskTCB,		
				 (CPU_CHAR	* )"RealTimeCheck test task", 		
                 (OS_TASK_PTR )RealTimeCheck_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )RealTimeCheck_TASK_PRIO,     	
                 (CPU_STK   * )&RealTimeCheck_TASK_STK[0],	
                 (CPU_STK_SIZE)RealTimeCheck_STK_SIZE/10,	
                 (CPU_STK_SIZE)RealTimeCheck_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);											 
	OS_CRITICAL_EXIT();	//退出临界区
	
								 
	while(1)
	{
		LED_Toggle(LED1);  //运行灯
		IWDG_Feed();
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1s
	}
//	OSTaskDel((OS_TCB*)0,&err);	//删除start_task任务自身							 
			

}


int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	Hanrdware_Init();
	
	OSInit(&err);		//初始化UCOSIII
	OS_CRITICAL_ENTER();//进入临界区
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);  //开启UCOSIII
	while(1);
}
