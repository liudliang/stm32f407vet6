#ifndef _GPIO_H
#define _GPIO_H

#include "config.h"
#include "stm32f4xx.h"

/* CC1口的电压状态 */
typedef enum
{
	CC1_NONEV = 0,
	CC1_4V,    //已连接    
	CC1_6V,    //连接中
	CC1_12V,   //未连接   
}CC1_STATUS;

typedef enum 
{
  LED_OFF = 0,
  LED_ON,
}Led_ONOFF;

typedef enum 
{
  POWER_ON = 0,
  POWER_OFF,
}OPEN_12V; 

typedef struct
{
	GPIO_TypeDef*     ioport;
	uint32            iobusclk;
	uint16            iopin;
	GPIOSpeed_TypeDef iospeed;
	GPIOPuPd_TypeDef  ioPuPd;
	GPIOOType_TypeDef ioOType;
}GPIO_INFO_T;

typedef enum 
{
  LED1 = 0,
  LED2,
  LED3,
	LED_MAX,
}Led_TypeDef;

/*指示灯控制口*/
#define LED1_GPIO_PORT              GPIOB                        /* CPU_RUN */
#define LED1_GPIO_CLK               RCC_AHB1Periph_GPIOG 
#define LED1_GPIO_PIN               GPIO_Pin_2
  
#define LED2_GPIO_PORT              GPIOG                        /* BMS1_RUN */
#define LED2_GPIO_CLK               RCC_AHB1Periph_GPIOG  
#define LED2_GPIO_PIN               GPIO_Pin_1
  
#define LED3_GPIO_PORT              GPIOG                        /* BMS2_RUN */
#define LED3_GPIO_CLK               RCC_AHB1Periph_GPIOG  
#define LED3_GPIO_PIN               GPIO_Pin_2

/* 0急停状态 */
/* 1交流输入断路器状态 */
/* 2交流接触器 */
/* 3防雷器 */
/* 4 A枪直流接触器 */
/* 5 B枪直流接触器 */
/* 6 功率分配接触器 */
/* 7 A枪熔丝状态 */
/* 8 B枪熔丝状态 */
/* 9 A枪电子锁反馈 */
/* 10 B枪电子锁反馈 */
/* 11 门禁反馈 */
/* 12 拨码开关1 */
/* 13 拨码开关2 */
/* 14 拨码开关3 */
/* 15 拨码开关4 */
/* 16 A枪控制导引CC1 */
/* 17 B枪控制导引CC1 */
  
/*光耦IO输入*/
typedef enum 
{
	INPUT0 = 0,
	INPUT1,
	INPUT2,
	INPUT3,
	INPUT4,
	INPUT5,
	INPUT6,
	INPUT7,
	INPUT8,
	INPUT9,
	INPUT10,
	INPUT11,
	INPUT_KEY1,
	INPUT_KEY2,
	INPUT_KEY3,
	INPUT_KEY4,
	INPUT_ACC1,
	INPUT_BCC1,
}Input_TypeDef;

#if 0
#define INPUT0_GPIO_PORT						GPIOF                     /* 急停状态 */
#define INPUT0_GPIO_CLK             RCC_AHB1Periph_GPIOF  
#define INPUT0_GPIO_PIN             GPIO_Pin_0

#define INPUT1_GPIO_PORT						GPIOF                     /* 交流输入断路器状态 */
#define INPUT1_GPIO_CLK             RCC_AHB1Periph_GPIOF 
#define INPUT1_GPIO_PIN             GPIO_Pin_1

#define INPUT2_GPIO_PORT						GPIOF                     /* 交流接触器 */
#define INPUT2_GPIO_CLK             RCC_AHB1Periph_GPIOF  
#define INPUT2_GPIO_PIN             GPIO_Pin_2

#define INPUT3_GPIO_PORT						GPIOF                     /* 防雷器 */
#define INPUT3_GPIO_CLK             RCC_AHB1Periph_GPIOF 
#define INPUT3_GPIO_PIN             GPIO_Pin_3

#define INPUT4_GPIO_PORT						GPIOF                     /* A枪直流接触器 */
#define INPUT4_GPIO_CLK             RCC_AHB1Periph_GPIOF 
#define INPUT4_GPIO_PIN             GPIO_Pin_4

#define INPUT5_GPIO_PORT						GPIOF                     /* B枪直流接触器 */
#define INPUT5_GPIO_CLK             RCC_AHB1Periph_GPIOF 
#define INPUT5_GPIO_PIN             GPIO_Pin_5

#define INPUT6_GPIO_PORT						GPIOF                     /* 功率分配接触器 */
#define INPUT6_GPIO_CLK             RCC_AHB1Periph_GPIOF  
#define INPUT6_GPIO_PIN             GPIO_Pin_6

#define INPUT7_GPIO_PORT						GPIOF                     /* A枪熔丝状态 */
#define INPUT7_GPIO_CLK             RCC_AHB1Periph_GPIOF 
#define INPUT7_GPIO_PIN             GPIO_Pin_7

#define INPUT8_GPIO_PORT						GPIOF                     /* B枪熔丝状态 */
#define INPUT8_GPIO_CLK             RCC_AHB1Periph_GPIOF  
#define INPUT8_GPIO_PIN             GPIO_Pin_8

#define INPUT9_GPIO_PORT						GPIOF                     /* A枪电子锁反馈 */
#define INPUT9_GPIO_CLK             RCC_AHB1Periph_GPIOF  
#define INPUT9_GPIO_PIN             GPIO_Pin_9

#define INPUT10_GPIO_PORT						GPIOF                     /* B枪电子锁反馈 */
#define INPUT10_GPIO_CLK            RCC_AHB1Periph_GPIOF 
#define INPUT10_GPIO_PIN            GPIO_Pin_10

#define INPUT11_GPIO_PORT						GPIOF                     /* 门禁反馈 */
#define INPUT11_GPIO_CLK            RCC_AHB1Periph_GPIOF 
#define INPUT11_GPIO_PIN            GPIO_Pin_11

#define INPUT12_GPIO_PORT						GPIOE                     /* 拨码开关1 */
#define INPUT12_GPIO_CLK            RCC_AHB1Periph_GPIOE  
#define INPUT12_GPIO_PIN            GPIO_Pin_12

#define INPUT13_GPIO_PORT						GPIOD                     /* 拨码开关2 */
#define INPUT13_GPIO_CLK            RCC_AHB1Periph_GPIOD  
#define INPUT13_GPIO_PIN            GPIO_Pin_3

#define INPUT14_GPIO_PORT						GPIOD                     /* 拨码开关3 */
#define INPUT14_GPIO_CLK            RCC_AHB1Periph_GPIOD  
#define INPUT14_GPIO_PIN            GPIO_Pin_7

#define INPUT15_GPIO_PORT						GPIOB                     /* 拨码开关4 */
#define INPUT15_GPIO_CLK            RCC_AHB1Periph_GPIOB  
#define INPUT15_GPIO_PIN           	GPIO_Pin_2

#define INPUT16_GPIO_PORT						GPIOE                     /* A枪控制导引CC1 */
#define INPUT16_GPIO_CLK            RCC_AHB1Periph_GPIOE  
#define INPUT16_GPIO_PIN           	GPIO_Pin_10

#define INPUT17_GPIO_PORT						GPIOE                    /* B枪控制导引CC1 */
#define INPUT17_GPIO_CLK            RCC_AHB1Periph_GPIOE 
#define INPUT17_GPIO_PIN           	GPIO_Pin_11
#else
#define INPUT0_GPIO_PORT						GPIOA                     /* 急停状态 */
#define INPUT0_GPIO_CLK             RCC_AHB1Periph_GPIOA  
#define INPUT0_GPIO_PIN             GPIO_Pin_15

#endif

/*光耦IO输出*/
typedef enum 
{
  LOUTPUT0_BEEP = 0,
  LOUTPUT1_SA0,
  LOUTPUT2_SA1,
  LOUTPUT3_SA2,
	
}Loutput_TypeDef;

#define LOUTPUT0_GPIO_PORT			     GPIOC									/* 蜂鸣器 */            
#define LOUTPUT0_GPIO_CLK            RCC_AHB1Periph_GPIOG  
#define LOUTPUT0_GPIO_PIN            GPIO_Pin_13

#define LOUTPUT1_GPIO_PORT					 GPIOA								/* SA0 */
#define LOUTPUT1_GPIO_CLK            RCC_AHB1Periph_GPIOE 
#define LOUTPUT1_GPIO_PIN            GPIO_Pin_0

#define LOUTPUT2_GPIO_PORT					 GPIOA								/* SA1 */
#define LOUTPUT2_GPIO_CLK            RCC_AHB1Periph_GPIOE  
#define LOUTPUT2_GPIO_PIN            GPIO_Pin_1

#define LOUTPUT3_GPIO_PORT					 GPIOA								/* SA2 */
#define LOUTPUT3_GPIO_CLK            RCC_AHB1Periph_GPIOE  
#define LOUTPUT3_GPIO_PIN            GPIO_Pin_8


/*0 B枪直流接触器控制 */
/*1 A枪直流接触器控制 */
/*2 A枪电子锁控制 */ 
/*3 交流接触器控制 */ 
/*4 B枪泄放电阻 */
/*5 A枪泄放电阻 */ 
/*6 B枪电子锁控制 */
/*7 功率分配控制 */
/*8 B枪故障指示灯 */ 
/*9 A枪故障指示灯 */
/*10 B枪充电指示灯 */
/*11 A枪充电指示灯 */
/*12 BMS1辅助电源电压选择(常闭12V，常开24V) */
/*13 BMS1辅助电源输出 */
/*14 BMS2辅助电源电压选择(常闭12V，常开24V) */
/*15 BMS2辅助电源输出*/

//*继电器输出*/
typedef enum 
{
 	JOUT_ADCKMP  = 0,//1,
	JOUT_ADCKMN  = 1,
	JOUT_AELOCKKM= 2,
	JOUT_ACKM    = 3,
	JOUT_AXFKM   = 5,
	JOUT_AERRLED = 9,
	JOUT_ACHGLED = 11,
	JOUT_ABMSPOWER= 12,
	JOUT_ABMSPWKM= 13,
	
	JOUT_TACKKM = 7,
	
	JOUT_BDCKMP  = 0,
	JOUT_BDCKMN  = 0,
	JOUT_BELOCKKM= 6,
	JOUT_BXFKM   = 4,
	JOUT_BERRLED = 8,
	JOUT_BCHGLED = 10,
	JOUT_BBMSPOWER= 14,
	JOUT_BBMSPWKM= 15,
	
}Joutput_TypeDef;

#define JOUTPUT1_GPIO_PORT			    GPIOD                   /* A枪直流接触器控制 */                
#define JOUTPUT1_GPIO_CLK           RCC_AHB1Periph_GPIOD 
#define JOUTPUT1_GPIO_PIN           GPIO_Pin_14 

#define JOUTPUT0_GPIO_PORT			GPIOC//    GPIOD                   /*A枪直流接触器控制 */                
#define JOUTPUT0_GPIO_CLK           RCC_AHB1Periph_GPIOC//RCC_AHB1Periph_GPIOD 
#define JOUTPUT0_GPIO_PIN           GPIO_Pin_6//GPIO_Pin_15           

#define JOUTPUT2_GPIO_PORT			    GPIOC//GPIOE                   /* A枪电子锁控制 */                
#define JOUTPUT2_GPIO_CLK          RCC_AHB1Periph_GPIOC// RCC_AHB1Periph_GPIOE 
#define JOUTPUT2_GPIO_PIN          GPIO_Pin_5// GPIO_Pin_8           

#define JOUTPUT6_GPIO_PORT			    GPIOG                   /* B枪电子锁控制 */                
#define JOUTPUT6_GPIO_CLK           RCC_AHB1Periph_GPIOG 
#define JOUTPUT6_GPIO_PIN           GPIO_Pin_10   

#define JOUTPUT3_GPIO_PORT			   GPIOB //GPIOD                   /* 风扇控制 */                
#define JOUTPUT3_GPIO_CLK           RCC_AHB1Periph_GPIOB//RCC_AHB1Periph_GPIOD
#define JOUTPUT3_GPIO_PIN           GPIO_Pin_10//GPIO_Pin_11

#define JOUTPUT4_GPIO_PORT			    GPIOE                   /* B枪泄放电阻 */                
#define JOUTPUT4_GPIO_CLK           RCC_AHB1Periph_GPIOE 
#define JOUTPUT4_GPIO_PIN           GPIO_Pin_3           

#define JOUTPUT5_GPIO_PORT			    GPIOE                   /* A枪泄放电阻 */                
#define JOUTPUT5_GPIO_CLK           RCC_AHB1Periph_GPIOE 
#define JOUTPUT5_GPIO_PIN           GPIO_Pin_2 

        

#define JOUTPUT7_GPIO_PORT			    GPIOG                   /* 功率分配控制 */                
#define JOUTPUT7_GPIO_CLK           RCC_AHB1Periph_GPIOG 
#define JOUTPUT7_GPIO_PIN           GPIO_Pin_9

#define JOUTPUT8_GPIO_PORT			    GPIOE                   /* B枪故障指示灯 */                
#define JOUTPUT8_GPIO_CLK           RCC_AHB1Periph_GPIOE 
#define JOUTPUT8_GPIO_PIN           GPIO_Pin_7           

#define JOUTPUT9_GPIO_PORT			    GPIOE                   /* A枪故障指示灯 */                
#define JOUTPUT9_GPIO_CLK           RCC_AHB1Periph_GPIOE 
#define JOUTPUT9_GPIO_PIN           GPIO_Pin_6 

#define JOUTPUT10_GPIO_PORT			    GPIOE                   /* B枪充电指示灯 */                
#define JOUTPUT10_GPIO_CLK          RCC_AHB1Periph_GPIOE 
#define JOUTPUT10_GPIO_PIN          GPIO_Pin_5           

#define JOUTPUT11_GPIO_PORT			    GPIOE                   /* A枪充电指示灯 */                
#define JOUTPUT11_GPIO_CLK          RCC_AHB1Periph_GPIOE 
#define JOUTPUT11_GPIO_PIN          GPIO_Pin_4

#define JOUTPUT12_GPIO_PORT			    GPIOG                   /* BMS1辅助电源电压选择(常闭12V，常开24V) */                
#define JOUTPUT12_GPIO_CLK          RCC_AHB1Periph_GPIOG 
#define JOUTPUT12_GPIO_PIN          GPIO_Pin_5           

#define JOUTPUT13_GPIO_PORT			    GPIOC//GPIOG                   /* BMS1辅助电源输出 */                
#define JOUTPUT13_GPIO_CLK          RCC_AHB1Periph_GPIOC//RCC_AHB1Periph_GPIOG 
#define JOUTPUT13_GPIO_PIN          GPIO_Pin_9//GPIO_Pin_6 

#define JOUTPUT14_GPIO_PORT			    GPIOG                   /* BMS2辅助电源电压选择(常闭12V，常开24V) */                
#define JOUTPUT14_GPIO_CLK          RCC_AHB1Periph_GPIOG 
#define JOUTPUT14_GPIO_PIN          GPIO_Pin_8           

#define JOUTPUT15_GPIO_PORT			    GPIOG                   /* BMS2辅助电源输出*/                
#define JOUTPUT15_GPIO_CLK          RCC_AHB1Periph_GPIOG 
#define JOUTPUT15_GPIO_PIN          GPIO_Pin_7

extern void InPut_OutPut_Init(void);                               //全部开关量初始化
extern void LEDToggle(Led_TypeDef Led);
extern void LEDOn(Led_TypeDef Led);
extern void LEDOFF(Led_TypeDef Led);
extern uint8 ReadInputDataBit(Input_TypeDef inputn);               //根据序号读取对应开关量状态
extern void WriteLoutputDataBit(Loutput_TypeDef no,BitAction val); //1路光输出
extern void WriteJoutputDataBit(Joutput_TypeDef no,BitAction val); //输出1路继电器动作

#define LED_RUN()       LEDToggle(LED1)

#define __BEEP_ON()     WriteLoutputDataBit(LOUTPUT0_BEEP, Bit_SET) 
#define __BEEP_OFF()    WriteLoutputDataBit(LOUTPUT0_BEEP, Bit_RESET) 

#endif
