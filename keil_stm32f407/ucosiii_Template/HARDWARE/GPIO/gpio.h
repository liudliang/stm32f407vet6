#ifndef _GPIO_H
#define _GPIO_H
#include "sys.h"
#include "cpu.h"
//////////////////////////////////////////////////////////////////////////////////	 
//功能：   
//作者：zyf
//创建日期:2020/04/16
//版本：V1.0									  
////////////////////////////////////////////////////////////////////////////////// 	



typedef struct
{
	GPIO_TypeDef*     ioport;
	uint32_t          iobusclk;
	uint16_t          iopin;
	GPIOSpeed_TypeDef iospeed;
	GPIOPuPd_TypeDef  ioPuPd;
	GPIOOType_TypeDef ioOType;
}GPIO_INFO_T;

typedef enum 
{
  LED1 = 0,
  LED2,
  LED3,
}Led_TypeDef;

/*指示灯控制口*/
#define LED1_GPIO_PORT              GPIOE                        /* CPU_RUN */
#define LED1_GPIO_CLK               RCC_AHB1Periph_GPIOE 
#define LED1_GPIO_PIN               GPIO_Pin_13
  
#define LED2_GPIO_PORT              GPIOE                        /* BMS1_RUN */
#define LED2_GPIO_CLK               RCC_AHB1Periph_GPIOE  
#define LED2_GPIO_PIN               GPIO_Pin_14
  
#define LED3_GPIO_PORT              GPIOE                        /* BMS2_RUN */
#define LED3_GPIO_CLK               RCC_AHB1Periph_GPIOE 
#define LED3_GPIO_PIN               GPIO_Pin_15



/*IO输入*/
typedef enum 
{
	INPUT_KEY1 = 0,
	INPUT_KEY2,
	INPUT_KEY3,

}Input_TypeDef;

#define INPUT0_GPIO_PORT						GPIOE                     /* 按键1状态 */
#define INPUT0_GPIO_CLK             RCC_AHB1Periph_GPIOE 
#define INPUT0_GPIO_PIN             GPIO_Pin_10

#define INPUT1_GPIO_PORT						GPIOE                     /* 按键2状态 */
#define INPUT1_GPIO_CLK             RCC_AHB1Periph_GPIOE 
#define INPUT1_GPIO_PIN             GPIO_Pin_11

#define INPUT2_GPIO_PORT						GPIOE                     /* 按键3状态 */
#define INPUT2_GPIO_CLK             RCC_AHB1Periph_GPIOE  
#define INPUT2_GPIO_PIN             GPIO_Pin_12

extern void LED_Init(void);  //初始化
extern void LED_Off (Led_TypeDef Led);
extern void LED_On (Led_TypeDef Led);
extern void LED_Toggle (Led_TypeDef Led);
extern void InPut_OutPut_Init(void);

extern uint8_t ReadInputDataBit(Input_TypeDef inputn);
#endif
