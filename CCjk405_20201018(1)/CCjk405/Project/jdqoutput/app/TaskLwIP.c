/**
  ******************************************************************************
  * @file    TaskLwIP.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
*/
#include "stm32f4xx.h"
#include "netconf.h"
#include "lwip/tcp.h"
#include "lwip/tcp_impl.h"
#include "main.h"
#include "stm32f4x7_eth.h"
#include "stm32f4x7_eth_bsp.h"

/* Private define ------------------------------------------------------------*/
#define SYSTEMTICK_PERIOD_MS  10

/* Private variables ---------------------------------------------------------*/
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */

/**
* @brief  TaskLwIP
* @param  None
* @retval None
*/
void TaskLwIP(void *p_arg)
{
	p_arg = p_arg;
	
	/* Configure ethernet (GPIOs, clocks, MAC, DMA) */
  ETH_BSP_Config();
	
	/* Initilaize the LwIP stack */
  LwIP_Init();
	
	/* Infinite loop */
  while (1)
  {  
    /* check if any packet received */
    if (ETH_CheckFrameReceived())
    { 
      /* process received ethernet packet */
      LwIP_Pkt_Handle();
    }
    /* handle periodic timers for LwIP */
    LwIP_Periodic_Handle(LocalTime);
		
		Delay10Ms(1);
  }
}

/**
  * @brief  TIM7_IRQHandler
  *         This function handles Timer3 Handler.
  * @param  None
  * @retval None
  */
void TIM7_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
		
		LocalTime += SYSTEMTICK_PERIOD_MS;
	}
}

/**
  * @brief  This function handles External line 15_10 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
  if(EXTI_GetITStatus(ETH_LINK_EXTI_LINE) != RESET)
  {
    /* Clear interrupt pending bit */
    EXTI_ClearITPendingBit(ETH_LINK_EXTI_LINE);
		
//		Eth_Link_ITHandler(DP83848_PHY_ADDRESS);
  }
}
