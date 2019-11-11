#include "sys.h"
#include "stm32f4xx.h"
#include "usart.h"
#include "delay.h"
#include "includes.h"

//ALIENTEK 探索者STM32F407开发板 实验0
//STM32F4工程模板-库函数版本
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com
//广州市星翼电子科技有限公司  
//作者：正点原子 @ALIENTEK



/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   
//LED0任务
//设置任务优先级
#define LED0_TASK_PRIO       			7 
//设置任务堆栈大小
#define LED0_STK_SIZE  		    		64
//任务堆栈	
OS_STK LED0_TASK_STK[LED0_STK_SIZE];
//任务函数
void led0_task(void *pdata);


//LED1任务
//设置任务优先级
#define LED1_TASK_PRIO       			6 
//设置任务堆栈大小
#define LED1_STK_SIZE  					64
//任务堆栈
OS_STK LED1_TASK_STK[LED1_STK_SIZE];
//任务函数
void led1_task(void *pdata);
void float_task(void *pdata);

//浮点测试任务
#define FLOAT_TASK_PRIO  5
//设置任务堆栈大小
#define FLOAT_STK_SIZE 128
//任务堆栈
//如果任务中使用 printf 来打印浮点数据的话一点要 8 字节对齐
__align(8) OS_STK FLOAT_TASK_STK[FLOAT_STK_SIZE];
//任务函数
void float_task(void *pdata);

void LED_Init()		        //初始化LED端口 
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
}



int main(void)
{ 
  uart_init(115200);
	delay_init(168);		  //初始化延时函数
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //中断分组配置
	LED_Init();		        //初始化LED端口 
	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	
}

 //开始任务
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
  OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
 	OSTaskCreate(led0_task,(void *)0,(OS_STK*)&LED0_TASK_STK[LED0_STK_SIZE-1],LED0_TASK_PRIO);						   
 	OSTaskCreate(led1_task,(void *)0,(OS_STK*)&LED1_TASK_STK[LED1_STK_SIZE-1],LED1_TASK_PRIO);	
//创建浮点测试任务
  OSTaskCreate(float_task,(void*)0,(OS_STK*)&FLOAT_TASK_STK[FLOAT_STK_SIZE-1],FLOAT_TASK_PRIO);	
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
} 

//LED0任务
void led0_task(void *pdata)
{	 	
	while(1)
	{
		GPIO_SetBits(GPIOE,GPIO_Pin_13);
		delay_ms(80);
		GPIO_ResetBits(GPIOE,GPIO_Pin_13);
		delay_ms(920);
	}
}

//LED1任务
void led1_task(void *pdata)
{	  
	while(1)
	{
		GPIO_SetBits(GPIOE,GPIO_Pin_14);
		delay_ms(300);
		GPIO_ResetBits(GPIOE,GPIO_Pin_14);
		delay_ms(300);
	}
}

//浮点测试任务
void float_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
  static float float_num=0.01;
  while(1)
  {
    float_num += 0.01;
    OS_ENTER_CRITICAL(); //进入临界区(关闭中断)
    printf("float_num = %.4f\r\n",float_num); //串口打印结果
    OS_EXIT_CRITICAL();  //退出临界区(开中断)
    delay_ms(500);
  }
}

/*
int main(void)
{
	u32 t=0;
	uart_init(115200);
	delay_init(84);
	
	GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
	
  while(1){
GPIO_SetBits(GPIOE,GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);	
		
    printf("t:%d\r\n",t);
		delay_ms(250);
		t++;
GPIO_ResetBits(GPIOE,GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
		delay_ms(250);
	}
}
*/


