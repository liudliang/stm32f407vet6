#include "myiic.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//功能：iic功能
//作者：
//创建日期:2020/05/09
//版本：V1.0								  
////////////////////////////////////////////////////////////////////////////////// 

/*
** 函数名称: 
** 功能描述: 初始化IIC
** 输　入: 
** 输　出: 
** 全局变量:
** 描述:
*/
void IIC_Init(void)
{			
	
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟

  //GPIOB8,B9初始化设置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
	IIC_SCL=1;
	IIC_SDA=1;
}

/*
** 函数名称: 
** 功能描述: 产生IIC起始信号
** 输　入: 
** 输　出: 
** 全局变量:
** 描述: 
*/
void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	IIC_SDA=1;	 
delay_us(4);	
	IIC_SCL=1;
	delay_us(4);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
delay_us(4);	
}	  

/*
** 函数名称: 
** 功能描述: 产生IIC停止信号
** 输　入: 
** 输　出: 
** 全局变量:
** 描述: 
*/
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL=0;
delay_us(4);	
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL=1; 
delay_us(4);	
	IIC_SDA=1;//发送I2C总线结束信号
	delay_us(4);							   	
}

/*
** 函数名称: 
** 功能描述: 等待应答信号到来
** 输　入:
** 输　出: 1，接收应答失败       0，接收应答成功
** 全局变量:
** 描述: 
*/
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	IIC_SDA=1;
	delay_us(1);	   
	IIC_SCL=1;
	delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0;//时钟输出0 	   
	return 0;  
} 

/*
** 函数名称: 
** 功能描述: 产生ACK应答
** 输　入:
** 输　出: 
** 全局变量:
** 描述: 
*/
void IIC_Ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}

/*
** 函数名称: 
** 功能描述: 不产生ACK应答	
** 输　入:
** 输　出: 
** 全局变量:
** 描述: 
*/		    
void IIC_NAck(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}				

/*
** 函数名称: 
** 功能描述: IIC发送一个字节,返回从机有无应答  1，有应答   0，无应答		
** 输　入:  发送的一个字节
** 输　出: 
** 全局变量:
** 描述: 
*/			  
void IIC_Send_Byte(u8 txd)
{                        
  u8 t;   
	SDA_OUT(); 	    
  IIC_SCL=0;//拉低时钟开始数据传输
  for(t=0;t<8;t++)
  {              
    IIC_SDA=(txd&0x80)>>7;
    txd<<=1; 	  
    delay_us(2);   //对TEA5767这三个延时都是必须的
		IIC_SCL=1;
		delay_us(2); 
		IIC_SCL=0;	
		delay_us(2);
  }	 
} 	   

/*
** 函数名称: 
** 功能描述: 读1个字节 		
** 输　入:  ack=1时，发送ACK，ack=0，发送nACK 
** 输　出: 
** 全局变量:
** 描述: 
*/	 
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
  for(i=0;i<8;i++ )
	{
    IIC_SCL=0; 
    delay_us(2);
		IIC_SCL=1;
    receive<<=1;
    if(READ_SDA)receive++;   
		delay_us(1); 
  }					 
  if (!ack)
     IIC_NAck();//发送nACK
  else
     IIC_Ack(); //发送ACK   
  return receive;
}



























