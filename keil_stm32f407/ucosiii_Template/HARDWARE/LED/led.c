#include "led.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//LED驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/6/10
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	



//初始化PE13,PE14和PF15为输出口.并使能这两个口的时钟		    
//LED IO初始化
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE); //使能GPIOD的时钟
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//输出
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  //推挽输出
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //上拉输出
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz; //高速GPIO
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOE,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15); //GPIOE 高电平
}




/*
*********************************************************************************************************
*                                              LED_Off()
*
* Description : Turn OFF any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    turns OFF ALL the LEDs
*                       1    turns OFF user LED1
*                       2    turns OFF user LED2
*                       3    turns OFF user LED3
*                       4    turns OFF user LED4
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  LED_Off (CPU_INT08U  led)
{
    switch (led) {
        case LED_ALL:
             GPIO_SetBits(GPIOE, GPIO_Pin_13);
             GPIO_SetBits(GPIOE, GPIO_Pin_14);
             GPIO_SetBits(GPIOE, GPIO_Pin_15);
             break;


        case LED1:
             GPIO_SetBits(GPIOE, GPIO_Pin_13);
             break;


        case LED2:
             GPIO_SetBits(GPIOE, GPIO_Pin_14);
             break;


        case LED3:
             GPIO_SetBits(GPIOE, GPIO_Pin_15);
             break;


        default:
             break;
    }
}


/*
*********************************************************************************************************
*                                             BSP_LED_On()
*
* Description : Turn ON any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    turns ON ALL  LEDs
*                       1    turns ON user LED1
*                       2    turns ON user LED2
*                       3    turns ON user LED3
*                       4    turns ON user LED4
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  LED_On (CPU_INT08U led)
{
    switch (led) {
        case LED_ALL:
             GPIO_ResetBits(GPIOE, GPIO_Pin_13);
             GPIO_ResetBits(GPIOE, GPIO_Pin_14);
             GPIO_ResetBits(GPIOE, GPIO_Pin_15);
             break;


        case LED1:
             GPIO_ResetBits(GPIOE, GPIO_Pin_13);
             break;


        case LED2:
             GPIO_ResetBits(GPIOE, GPIO_Pin_14);
             break;


        case LED3:
             GPIO_ResetBits(GPIOE, GPIO_Pin_15);
             break;


        default:
             break;
    }
}


/*
*********************************************************************************************************
*                                            BSP_LED_Toggle()
*
* Description : TOGGLE any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    TOGGLE ALL the LEDs
*                       1    TOGGLE user LED1
*                       2    TOGGLE user LED2
*                       3    TOGGLE user LED3
*                       4    TOGGLE user LED4
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  LED_Toggle (CPU_INT08U  led)
{
    CPU_INT16U  pins;


    switch (led) {
        case LED_ALL:
             pins  =  GPIO_ReadOutputData(GPIOE);               /* Read data (current state of output pins) in GPIOA    */
             pins ^=  GPIO_Pin_13;                           /* Tells which pins (other than pin for LED1) are ON    */
             GPIO_SetBits(  GPIOE,   pins  &  GPIO_Pin_13);  /* If there are any,turn them OFF and leave LED1 ON     */
             GPIO_ResetBits(GPIOE, (~pins) &  GPIO_Pin_13);

             pins  =  GPIO_ReadOutputData(GPIOE);
             pins ^=  GPIO_Pin_14;
             GPIO_SetBits(  GPIOE,   pins  &  GPIO_Pin_14);
             GPIO_ResetBits(GPIOE, (~pins) &  GPIO_Pin_14);

             pins  =  GPIO_ReadOutputData(GPIOE);
             pins ^= (GPIO_Pin_15);
             GPIO_SetBits(  GPIOE,   pins  & GPIO_Pin_15);
             GPIO_ResetBits(GPIOE, (~pins) & GPIO_Pin_15);
             break;


        case LED1:
             pins  = GPIO_ReadOutputData(GPIOE);
             pins ^= GPIO_Pin_13;
             GPIO_SetBits(  GPIOE,   pins   &  GPIO_Pin_13);
             GPIO_ResetBits(GPIOE, (~pins)  &  GPIO_Pin_13);
             break;


        case LED2:
             pins  = GPIO_ReadOutputData(GPIOE);
             pins ^= GPIO_Pin_14;
             GPIO_SetBits(  GPIOE,   pins   &  GPIO_Pin_14);
             GPIO_ResetBits(GPIOE, (~pins)  &  GPIO_Pin_14);
             break;


        case LED3:
             pins  = GPIO_ReadOutputData(GPIOE);
             pins ^= GPIO_Pin_15;
             GPIO_SetBits(  GPIOE,   pins   &  GPIO_Pin_15);
             GPIO_ResetBits(GPIOE, (~pins)  &  GPIO_Pin_15);
             break;

        default:
             break;
    }
}