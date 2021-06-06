#ifndef _GPIO_H
#define _GPIO_H

#include "config.h"
#include "stm32f4xx.h"

/* CC1�ڵĵ�ѹ״̬ */
typedef enum
{
	CC1_NONEV = 0,
	CC1_4V,    //������    
	CC1_6V,    //������
	CC1_12V,   //δ����   
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
  LED4,
  LED5,
  LED6,	
	LED_MAX,
}Led_TypeDef;

/*ָʾ�ƿ��ƿ�*/
#define LED1_GPIO_PORT              GPIOB                        /* CPU_RUN */
#define LED1_GPIO_CLK               RCC_AHB1Periph_GPIOG 
#define LED1_GPIO_PIN               GPIO_Pin_2
  
#define LED2_GPIO_PORT              GPIOG                        /* BMS1_RUN */
#define LED2_GPIO_CLK               RCC_AHB1Periph_GPIOG  
#define LED2_GPIO_PIN               GPIO_Pin_1
  
#define LED3_GPIO_PORT              GPIOG                        /* BMS2_RUN */
#define LED3_GPIO_CLK               RCC_AHB1Periph_GPIOG  
#define LED3_GPIO_PIN               GPIO_Pin_2


#define LED4_GPIO_PORT              GPIOB                        /* red-fault */ 
#define LED4_GPIO_CLK               RCC_AHB1Periph_GPIOB 
#define LED4_GPIO_PIN               GPIO_Pin_3
  
#define LED5_GPIO_PORT              GPIOB                        /* green-power on */ 
#define LED5_GPIO_CLK               RCC_AHB1Periph_GPIOB  
#define LED5_GPIO_PIN               GPIO_Pin_4
  
#define LED6_GPIO_PORT              GPIOB                        /* yellow-charging */ 
#define LED6_GPIO_CLK               RCC_AHB1Periph_GPIOB  
#define LED6_GPIO_PIN               GPIO_Pin_5

/* 0��ͣ״̬ */
/* 1���������·��״̬ */
/* 2�����Ӵ��� */
/* 3������ */
/* 4 Aǹֱ���Ӵ��� */
/* 5 Bǹֱ���Ӵ��� */
/* 6 ���ʷ���Ӵ��� */
/* 7 Aǹ��˿״̬ */
/* 8 Bǹ��˿״̬ */
/* 9 Aǹ���������� */
/* 10 Bǹ���������� */
/* 11 �Ž����� */
/* 12 ���뿪��1 */
/* 13 ���뿪��2 */
/* 14 ���뿪��3 */
/* 15 ���뿪��4 */
/* 16 Aǹ���Ƶ���CC1 */
/* 17 Bǹ���Ƶ���CC1 */
  
/*����IO����*/
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
#define INPUT0_GPIO_PORT						GPIOF                     /* ��ͣ״̬ */
#define INPUT0_GPIO_CLK             RCC_AHB1Periph_GPIOF  
#define INPUT0_GPIO_PIN             GPIO_Pin_0

#define INPUT1_GPIO_PORT						GPIOF                     /* ���������·��״̬ */
#define INPUT1_GPIO_CLK             RCC_AHB1Periph_GPIOF 
#define INPUT1_GPIO_PIN             GPIO_Pin_1

#define INPUT2_GPIO_PORT						GPIOF                     /* �����Ӵ��� */
#define INPUT2_GPIO_CLK             RCC_AHB1Periph_GPIOF  
#define INPUT2_GPIO_PIN             GPIO_Pin_2

#define INPUT3_GPIO_PORT						GPIOF                     /* ������ */
#define INPUT3_GPIO_CLK             RCC_AHB1Periph_GPIOF 
#define INPUT3_GPIO_PIN             GPIO_Pin_3

#define INPUT4_GPIO_PORT						GPIOF                     /* Aǹֱ���Ӵ��� */
#define INPUT4_GPIO_CLK             RCC_AHB1Periph_GPIOF 
#define INPUT4_GPIO_PIN             GPIO_Pin_4

#define INPUT5_GPIO_PORT						GPIOF                     /* Bǹֱ���Ӵ��� */
#define INPUT5_GPIO_CLK             RCC_AHB1Periph_GPIOF 
#define INPUT5_GPIO_PIN             GPIO_Pin_5

#define INPUT6_GPIO_PORT						GPIOF                     /* ���ʷ���Ӵ��� */
#define INPUT6_GPIO_CLK             RCC_AHB1Periph_GPIOF  
#define INPUT6_GPIO_PIN             GPIO_Pin_6

#define INPUT7_GPIO_PORT						GPIOF                     /* Aǹ��˿״̬ */
#define INPUT7_GPIO_CLK             RCC_AHB1Periph_GPIOF 
#define INPUT7_GPIO_PIN             GPIO_Pin_7

#define INPUT8_GPIO_PORT						GPIOF                     /* Bǹ��˿״̬ */
#define INPUT8_GPIO_CLK             RCC_AHB1Periph_GPIOF  
#define INPUT8_GPIO_PIN             GPIO_Pin_8

#define INPUT9_GPIO_PORT						GPIOF                     /* Aǹ���������� */
#define INPUT9_GPIO_CLK             RCC_AHB1Periph_GPIOF  
#define INPUT9_GPIO_PIN             GPIO_Pin_9

#define INPUT10_GPIO_PORT						GPIOF                     /* Bǹ���������� */
#define INPUT10_GPIO_CLK            RCC_AHB1Periph_GPIOF 
#define INPUT10_GPIO_PIN            GPIO_Pin_10

#define INPUT11_GPIO_PORT						GPIOF                     /* �Ž����� */
#define INPUT11_GPIO_CLK            RCC_AHB1Periph_GPIOF 
#define INPUT11_GPIO_PIN            GPIO_Pin_11

#define INPUT12_GPIO_PORT						GPIOE                     /* ���뿪��1 */
#define INPUT12_GPIO_CLK            RCC_AHB1Periph_GPIOE  
#define INPUT12_GPIO_PIN            GPIO_Pin_12

#define INPUT13_GPIO_PORT						GPIOD                     /* ���뿪��2 */
#define INPUT13_GPIO_CLK            RCC_AHB1Periph_GPIOD  
#define INPUT13_GPIO_PIN            GPIO_Pin_3

#define INPUT14_GPIO_PORT						GPIOD                     /* ���뿪��3 */
#define INPUT14_GPIO_CLK            RCC_AHB1Periph_GPIOD  
#define INPUT14_GPIO_PIN            GPIO_Pin_7

#define INPUT15_GPIO_PORT						GPIOB                     /* ���뿪��4 */
#define INPUT15_GPIO_CLK            RCC_AHB1Periph_GPIOB  
#define INPUT15_GPIO_PIN           	GPIO_Pin_2

#define INPUT16_GPIO_PORT						GPIOE                     /* Aǹ���Ƶ���CC1 */
#define INPUT16_GPIO_CLK            RCC_AHB1Periph_GPIOE  
#define INPUT16_GPIO_PIN           	GPIO_Pin_10

#define INPUT17_GPIO_PORT						GPIOE                    /* Bǹ���Ƶ���CC1 */
#define INPUT17_GPIO_CLK            RCC_AHB1Periph_GPIOE 
#define INPUT17_GPIO_PIN           	GPIO_Pin_11
#else
#define INPUT0_GPIO_PORT						GPIOA                     /* ��ͣ״̬ */
#define INPUT0_GPIO_CLK             RCC_AHB1Periph_GPIOA  
#define INPUT0_GPIO_PIN             GPIO_Pin_15

#endif

/*����IO���*/
typedef enum 
{
  LOUTPUT0_BEEP = 0,
  LOUTPUT1_SA0,
  LOUTPUT2_SA1,
  LOUTPUT3_SA2,
	
}Loutput_TypeDef;

#define LOUTPUT0_GPIO_PORT			     GPIOC									/* ������ */            
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


/*0 Bǹֱ���Ӵ������� */
/*1 Aǹֱ���Ӵ������� */
/*2 Aǹ���������� */ 
/*3 �����Ӵ������� */ 
/*4 Bǹй�ŵ��� */
/*5 Aǹй�ŵ��� */ 
/*6 Bǹ���������� */
/*7 ���ʷ������ */
/*8 Bǹ����ָʾ�� */ 
/*9 Aǹ����ָʾ�� */
/*10 Bǹ���ָʾ�� */
/*11 Aǹ���ָʾ�� */
/*12 BMS1������Դ��ѹѡ��(����12V������24V) */
/*13 BMS1������Դ��� */
/*14 BMS2������Դ��ѹѡ��(����12V������24V) */
/*15 BMS2������Դ���*/

//*�̵������*/
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

#define JOUTPUT1_GPIO_PORT			    GPIOD                   /* Aǹֱ���Ӵ������� */                
#define JOUTPUT1_GPIO_CLK           RCC_AHB1Periph_GPIOD 
#define JOUTPUT1_GPIO_PIN           GPIO_Pin_14 

#define JOUTPUT0_GPIO_PORT			GPIOC//    GPIOD                   /*Aǹֱ���Ӵ������� */                
#define JOUTPUT0_GPIO_CLK           RCC_AHB1Periph_GPIOC//RCC_AHB1Periph_GPIOD 
#define JOUTPUT0_GPIO_PIN           GPIO_Pin_6//GPIO_Pin_15           

#define JOUTPUT2_GPIO_PORT			    GPIOC//GPIOE                   /* Aǹ���������� */                
#define JOUTPUT2_GPIO_CLK          RCC_AHB1Periph_GPIOC// RCC_AHB1Periph_GPIOE 
#define JOUTPUT2_GPIO_PIN          GPIO_Pin_5// GPIO_Pin_8           

#define JOUTPUT6_GPIO_PORT			    GPIOG                   /* Bǹ���������� */                
#define JOUTPUT6_GPIO_CLK           RCC_AHB1Periph_GPIOG 
#define JOUTPUT6_GPIO_PIN           GPIO_Pin_10   

#define JOUTPUT3_GPIO_PORT			   GPIOB //GPIOD                   /* ���ȿ��� */                
#define JOUTPUT3_GPIO_CLK           RCC_AHB1Periph_GPIOB//RCC_AHB1Periph_GPIOD
#define JOUTPUT3_GPIO_PIN           GPIO_Pin_10//GPIO_Pin_11

#define JOUTPUT4_GPIO_PORT			    GPIOE                   /* Bǹй�ŵ��� */                
#define JOUTPUT4_GPIO_CLK           RCC_AHB1Periph_GPIOE 
#define JOUTPUT4_GPIO_PIN           GPIO_Pin_3           

#define JOUTPUT5_GPIO_PORT			    GPIOE                   /* Aǹй�ŵ��� */                
#define JOUTPUT5_GPIO_CLK           RCC_AHB1Periph_GPIOE 
#define JOUTPUT5_GPIO_PIN           GPIO_Pin_2 

        

#define JOUTPUT7_GPIO_PORT			    GPIOG                   /* ���ʷ������ */                
#define JOUTPUT7_GPIO_CLK           RCC_AHB1Periph_GPIOG 
#define JOUTPUT7_GPIO_PIN           GPIO_Pin_9

#define JOUTPUT8_GPIO_PORT			    GPIOE                   /* Bǹ����ָʾ�� */                
#define JOUTPUT8_GPIO_CLK           RCC_AHB1Periph_GPIOE 
#define JOUTPUT8_GPIO_PIN           GPIO_Pin_7           

#define JOUTPUT9_GPIO_PORT			    GPIOB //GPIOE                   /* Aǹ����ָʾ�� */                
#define JOUTPUT9_GPIO_CLK           RCC_AHB1Periph_GPIOB//RCC_AHB1Periph_GPIOE 
#define JOUTPUT9_GPIO_PIN           GPIO_Pin_3//GPIO_Pin_6 

#define JOUTPUT10_GPIO_PORT			    GPIOE                   /* Bǹ���ָʾ�� */                
#define JOUTPUT10_GPIO_CLK          RCC_AHB1Periph_GPIOE 
#define JOUTPUT10_GPIO_PIN          GPIO_Pin_5           

#define JOUTPUT11_GPIO_PORT			    GPIOB //GPIOE                   /* Aǹ���ָʾ�� */                
#define JOUTPUT11_GPIO_CLK          RCC_AHB1Periph_GPIOB//RCC_AHB1Periph_GPIOE 
#define JOUTPUT11_GPIO_PIN          GPIO_Pin_5

#define JOUTPUT12_GPIO_PORT			    GPIOG                   /* BMS1������Դ��ѹѡ��(����12V������24V) */                
#define JOUTPUT12_GPIO_CLK          RCC_AHB1Periph_GPIOG 
#define JOUTPUT12_GPIO_PIN          GPIO_Pin_5           

#define JOUTPUT13_GPIO_PORT			    GPIOC//GPIOG                   /* BMS1������Դ��� */                
#define JOUTPUT13_GPIO_CLK          RCC_AHB1Periph_GPIOC//RCC_AHB1Periph_GPIOG 
#define JOUTPUT13_GPIO_PIN          GPIO_Pin_9//GPIO_Pin_6 

#define JOUTPUT14_GPIO_PORT			    GPIOG                   /* BMS2������Դ��ѹѡ��(����12V������24V) */                
#define JOUTPUT14_GPIO_CLK          RCC_AHB1Periph_GPIOG 
#define JOUTPUT14_GPIO_PIN          GPIO_Pin_8           

#define JOUTPUT15_GPIO_PORT			    GPIOG                   /* BMS2������Դ���*/                
#define JOUTPUT15_GPIO_CLK          RCC_AHB1Periph_GPIOG 
#define JOUTPUT15_GPIO_PIN          GPIO_Pin_7

extern void InPut_OutPut_Init(void);                               //ȫ����������ʼ��
extern void LEDToggle(Led_TypeDef Led);
extern void LEDOn(Led_TypeDef Led);
extern void LEDOFF(Led_TypeDef Led);
extern uint8 ReadInputDataBit(Input_TypeDef inputn);               //������Ŷ�ȡ��Ӧ������״̬
extern void WriteLoutputDataBit(Loutput_TypeDef no,BitAction val); //1·�����
extern void WriteJoutputDataBit(Joutput_TypeDef no,BitAction val); //���1·�̵�������

#define LED_RUN()       LEDToggle(LED1)

#define __BEEP_ON()     WriteLoutputDataBit(LOUTPUT0_BEEP, Bit_SET) 
#define __BEEP_OFF()    WriteLoutputDataBit(LOUTPUT0_BEEP, Bit_RESET) 

#endif
