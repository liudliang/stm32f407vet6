#include "timer.h"
//////////////////////////////////////////////////////////////////////////////////	 
//功能：定时器中断函数	   
//作者：zyf
//创建日期:2020/04/23
//版本：V1.0								  
////////////////////////////////////////////////////////////////////////////////// 	 

///*因为系统初始化SystemInit函数里初始化APB1总线时钟为4分频即42M，所以TIM2~TIM7、TIM12~TIM14的时钟为APB1的时钟的两倍即84M*/
//STM3 的通用 TIMx (TIM2~TIM5 和 TIM9~TIM14)定时器中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz


void tim2_1s_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;		 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);		        //使能TIM2时钟
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;	
	TIM_TimeBaseInitStruct.TIM_CounterMode   = TIM_CounterMode_Up;	//向上计数模式
	TIM_TimeBaseInitStruct.TIM_Period        = 9999;   	            //自动重装载值
	TIM_TimeBaseInitStruct.TIM_Prescaler     = 8399;                //时钟预分频数.STM32F4XX APB1是84MHZ,不是72MHZ，STM32F1XX是72MHZ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);		            //初始化TIM2
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);		 	                //允许定时器2更新中断
	TIM_Cmd(TIM2,ENABLE);                                           //使能定时器2
}

void tim2_1s_NVIC_init(void)
{	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel      = TIM2_IRQn;               //定时器2中断
	NVIC_InitStruct.NVIC_IRQChannelCmd   = ENABLE;	
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority  =1;          //抢占优先级1
	NVIC_InitStruct.NVIC_IRQChannelSubPriority   = 4; 	            //子优先级4
	NVIC_Init(&NVIC_InitStruct);
}

//定时器2中断服务函数
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET) //溢出中断
	{
//		LED1=!LED1;//DS1翻转
	}
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);  //清除中断标志位
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
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority  =1; 
	NVIC_InitStruct.NVIC_IRQChannelSubPriority   = 3;   	
	NVIC_Init(&NVIC_InitStruct);
}

void TIM3_IRQHandler(void)
{ 	
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{
//		LED1=!LED1;//DS1翻转
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位	
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
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority  = 1;  
	NVIC_InitStruct.NVIC_IRQChannelSubPriority   = 2; 
	NVIC_Init(&NVIC_InitStruct);
}

void TIM4_IRQHandler(void)
{ 	
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)==SET) //溢出中断
	{
//		LED1=!LED1;//DS1翻转
	}
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);  //清除中断标志位		
}





void tim5_100ms_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;		 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);		
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;	
	TIM_TimeBaseInitStruct.TIM_CounterMode   = TIM_CounterMode_Up;	
	TIM_TimeBaseInitStruct.TIM_Period        = 999;    	
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

void TIM5_IRQHandler(void)
{ 	
	if(TIM_GetITStatus(TIM5,TIM_IT_Update)==SET) //溢出中断
	{
//		LED1=!LED1;//DS1翻转
	}
	TIM_ClearITPendingBit(TIM5,TIM_IT_Update);  //清除中断标志位	
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
	if(TIM_GetITStatus(TIM6,TIM_IT_Update)==SET) //溢出中断
	{
//		LED1=!LED1;//DS1翻转
	}
	TIM_ClearITPendingBit(TIM6,TIM_IT_Update);  //清除中断标志位	
}




void tim7_10ms_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;		 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);		
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;	
	TIM_TimeBaseInitStruct.TIM_CounterMode   = TIM_CounterMode_Up;	
 	TIM_TimeBaseInitStruct.TIM_Period        = 99;    
	TIM_TimeBaseInitStruct.TIM_Prescaler     = 8399;	
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseInitStruct);		 
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);		 	
	TIM_Cmd(TIM7,ENABLE);
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
	if(TIM_GetITStatus(TIM7,TIM_IT_Update)==SET) //溢出中断
	{
//		LED1=!LED1;//DS1翻转
	}
	TIM_ClearITPendingBit(TIM7,TIM_IT_Update);  //清除中断标志位	
}



void TimerInit(void)
{
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

