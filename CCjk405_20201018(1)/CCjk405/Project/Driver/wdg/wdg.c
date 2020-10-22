
#include "wdg.h"

//外部看门狗初始化
void OutWatchdogInit()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
#if defined (STM32F4XX)
	  /* Enable the GPIO_LED Clock */
	  RCC_AHB1PeriphClockCmd(OUTWATCHDOG_GPIO_CLK, ENABLE);
	  	
	  /* Configure the GPIO_LED pin */
	  GPIO_InitStructure.GPIO_Pin = OUTWATCHDOG_GPIO_PIN;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_Init(OUTWATCHDOG_GPIO_PORT, &GPIO_InitStructure);
#endif
}

// ==================== 窗口看门狗 ================================================================================
// 适用范围：要求看门狗在精确计时窗口起作用的应用程序
// 在窗口看门狗中断中喂狗 
void sysWWdgInit(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

//patli 20191205  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE); // 启动窗口看门狗外设时钟
	WWDG_SetPrescaler(WWDG_Prescaler_8);	               // 计数频率 = (PCLK1/4096)/8 = 36M/4096/8 = 1099HZ  
	WWDG_SetWindowValue(65);	                           //  Set Window value to 65
	WWDG_Enable(127);		                                 // 激活看门狗并设置初值为127递减到63时看门狗产生复位
	WWDG_ClearFlag();	                                   // 清除提前唤醒中断标志
	WWDG_EnableIT();		                                 // 开中断 

	OutWatchdogInit();
}

// ===================== 窗口看门狗 end ============================================================================

// ===================== 独立看门狗 ================================================================================
// 使用范围：在主程序之外，能够完全独立工作，并且对时间精度要求较低的场合
// 在SysTick_Handler()中喂狗 

void sysIWdgInit(void)
{
	uint16 timeout = 0, tmp;
	uint8 i = 0xff;

	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);		 // 先向IWDG_KR寄存器中写入0x5555，解除写保护 
	IWDG_SetPrescaler(IWDG_Prescaler_128);	           // 计数器时钟的预分频因子32	IWDG counter clock=40KHz(LSI) / 32 = 1.25 KHz

	timeout = IWDG_TIMEOUT;
	if((IWDG_TIMEOUT < 1000) || (IWDG_TIMEOUT > 4000))
		timeout = 3000;                                // 默认 12s	
	tmp =  (uint16)(timeout*1.25);
	IWDG_SetReload(tmp);	                           // 看门狗计数器重装载值

	IWDG_ReloadCounter();			                       // 重装计数值 
	IWDG_Enable();		                               // 写入0xCCCC，启动看门狗工作
//	
//	RCC_LSICmd(ENABLE);//打开LSI  

//	OutWatchdogInit();
}


//外部硬件看门狗
void FeedOutWatchdog(void)
{
	OUTWATCHDOG_GPIO_PORT->ODR ^= OUTWATCHDOG_GPIO_PIN;//异或，将1变为0?0变为1，灯闪烁
}

//***窗口看门狗喂狗
void FeedWWdog(void)
{
#ifdef WATCH_DOG_ENABLE
	IWDG_ReloadCounter();
#endif
//	FeedOutWatchdog();
}

void Wdg_feeddog(void)
{
  IWDG_ReloadCounter();
}
