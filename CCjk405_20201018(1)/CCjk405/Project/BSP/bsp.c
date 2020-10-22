/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                             (c) Copyright 2007; Micrium, Inc.; Weston, FL
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
*                                        BOARD SUPPORT PACKAGE
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   IAR STM32-SK Evaluation Board
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : Brian Nagel
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  BSP_GLOBALS
#include "includes.h"
#include "gpio.h"

/* Private variables ---------------------------------------------------------*/
static RCC_ClocksTypeDef  rcc_clocks = { 0 };

/*
*********************************************************************************************************
*                                      GET THE CPU CLOCK FREQUENCY
*
* Description: This function reads CPU registers to determine the CPU clock frequency of the chip in KHz.
*
* Argument(s): None.
*
* Returns    : The CPU clock frequency, in Hz.
*********************************************************************************************************
*/
INT32U  OS_CPU_SysTickClkFreq (void)
{
	return rcc_clocks.HCLK_Frequency;
}

/*
*********************************************************************************************************
*                                       TICKER INITIALIZATION
*
* Description : This function is called to initialize uC/OS-II's tick source (typically a timer generating
*               interrupts every 1 to 100 mS).
*
* Arguments   : none
*
* Note(s)     : 1) The timer is setup for output compare mode BUT 'MUST' also 'freerun' so that the timer
*                  count goes from 0x00000000 to 0xFFFFFFFF to ALSO be able to read the free running count.
*                  The reason this is needed is because we use the free-running count in uC/OS-View.
*********************************************************************************************************
*/
void  SystemTickInit (void)
{
	CPU_INT32U         cnts = 0;

	RCC_GetClocksFreq(&rcc_clocks);

	cnts = rcc_clocks.HCLK_Frequency / OS_TICKS_PER_SEC;

	OS_CPU_SysTickInit();
	
	SysTick_Config(cnts);
}

/**
  * @}
  */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
