
#include "stm32f4xx.h"
#include "usart.h"
#include "delay.h"
#include "malloc.h"
#include "timer.h"
#include "lwip_comm.h"
#include "lwipopts.h"
//ALIENTEK 探索者STM32F407开发板 实验0
//STM32F4工程模板-库函数版本
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com
//广州市星翼电子科技有限公司  
//作者：正点原子 @ALIENTEK





//mode:1 显示DHCP获取到的地址
//	  其他 显示静态地址
void show_address(u8 mode)
{
	u8 buf[30];
	if(mode==1)
	{
		sprintf((char*)buf,"MAC    :%d.%d.%d.%d.%d.%d",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);//打印MAC地址
		printf("%s\r\n",buf); 
		sprintf((char*)buf,"DHCP IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//打印动态IP地址
		printf("%s\r\n",buf); 
		sprintf((char*)buf,"DHCP GW:%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//打印网关地址
		printf("%s\r\n",buf); 
		sprintf((char*)buf,"DHCP IP:%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//打印子网掩码地址
		printf("%s\r\n",buf); 
	}
	else 
	{
		sprintf((char*)buf,"MAC      :%d.%d.%d.%d.%d.%d",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);//打印MAC地址
		printf("%s\r\n",buf); 
		sprintf((char*)buf,"Static IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//打印动态IP地址
		printf("%s\r\n",buf); 
		sprintf((char*)buf,"Static GW:%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//打印网关地址
		printf("%s\r\n",buf); 
		sprintf((char*)buf,"Static IP:%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//打印子网掩码地址
		printf("%s\r\n",buf); 
	}	
}




int main(void)
{
	u8 i = 10,rampause;
	u32 t=0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);
	delay_init(84);
	TimerInit();
	
	my_mem_init(SRAMIN);		//初始化内部内存池
	
	GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
	

	
  while(i--){
GPIO_SetBits(GPIOE,GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);	
		
    printf("t:%d\r\n",t);
		delay_ms(250);
		t++;
GPIO_ResetBits(GPIOE,GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
		delay_ms(250);
	}

	rampause = my_mem_perused(SRAMIN);
	printf("SRAM PERUSED:%d\r\n",rampause);	
	while(lwip_comm_init()) //lwip初始化
	{
		rampause = my_mem_perused(SRAMIN);
	  printf("SRAM PERUSED:%d\r\n",rampause);
		
		printf("LWIP Init Falied!\r\n");
		delay_ms(2200);
		printf("Retrying...\r\n");  	

	}
	

	
#if LWIP_DHCP //使用 DHCP
//等待 DHCP 获取成功/超时溢出
while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))
{
lwip_periodic_handle(); //LWIP 内核需要定时处理的函数
}
#endif
	
	show_address(lwipdev.dhcpstatus);	//显示地址信息
	
	while(1)
  {
    lwip_periodic_handle(); //LWIP 内核需要定时处理的函数
  }

}



/*
//手册中讲解到步骤15的时候的main.c源码如下：
#include "stm32f4xx.h"

//ALIENTEK 探索者STM32F407开发板 实验0
//STM32F4工程模板-库函数版本
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com
//广州市星翼电子科技有限公司  
//作者：正点原子 @ALIENTEK
  
void Delay(__IO uint32_t nCount);

void Delay(__IO uint32_t nCount)
{
  while(nCount--){}
}

int main(void)
{

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
		Delay(0x7FFFFF);
		GPIO_ResetBits(GPIOE,GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
		Delay(0x7FFFFF);
	
	}
}
*/
