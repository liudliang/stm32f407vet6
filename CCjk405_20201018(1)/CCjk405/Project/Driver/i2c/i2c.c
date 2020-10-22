/****************************************Copyright (c)**************************************************
        深圳奥耐电气技术有限公司
				
***FILE: i2c.c
****************************************************************************************************/

#include "i2c.h"


OS_EVENT* 	FPRAM_Lock;	                 		// devece mutex lock

#ifdef  DEF_SIMULATE_I2C   /*模拟I2C*/

void Init_IIC(void);

/*
static STM32F10x_I2C_CHAN_PRAM I2CParameter[] = {	     //注意:参数2-3 等原理图/PCB  再填充 
		{(void *)I2C_FRAM_GPIO, I2C_FRAM_SCL, I2C_FRAM_SDA, I2C_FRAM_WP},	

		};*/
static uint8 PowerOnFirst = FALSE;

// ======================================================================================================
// 铁电IIC接口初始化 
// 参数：无
// 返回：无
// ======================================================================================================
void SysI2CInit(void)
{
	if(PowerOnFirst == FALSE)
	{
		GPIO_InitTypeDef  GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(I2C_FRAM_GPIO_CLK, ENABLE);  // IIC Pin 外设时钟打开
	
		GPIO_InitStructure.GPIO_Pin =  I2C_FRAM_SCL | I2C_FRAM_WP; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(I2C_FRAM_GPIO, &GPIO_InitStructure);
	
		GPIO_InitStructure.GPIO_Pin =  I2C_FRAM_SDA; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//GPIO_Mode_IPU;
		GPIO_Init(I2C_FRAM_GPIO, &GPIO_InitStructure);
	
		FPRAM_Lock = OSSemCreate(1);
		Init_IIC();
		PowerOnFirst = TRUE;
	}
}



/****************************************************************************************
** 函数名称: void I2CDelay(void)
** 功能描述: 延时1个指令周期
** 输　入: 无
** 输　出: 无
** 全局变量:
** 描述: 写外部eeprom / FRAM IIC总线延时
******************************************************************************************/
static void I2CDelay(void)
{  
	uint32 tmp = SYS_CLOCK_HZ/I2C_CLOCK_HZ;
	uint32 i;
	tmp = tmp/2;
	for(i=0;i<tmp;i++)
		;		
}

 /****************************************************************************************
** 函数名称: void init_eeprom(void)
** 功能描述: 初始化外部模拟IIC总线 初始化EEprom/FRAM  I2C
** 输　入: 无
** 输　出: 无
** 全局变量:
** 描述: 写外部eeprom / FRAM IIC总线
******************************************************************************************/
void Init_IIC(void)
{
	GPIO_WriteBit(I2C_FRAM_GPIO,I2C_FRAM_SCL,Bit_SET);  //SCL = 1;
	I2CDelay();
	GPIO_WriteBit(I2C_FRAM_GPIO,I2C_FRAM_SDA,Bit_SET);  //SDA = 1;
	I2CDelay();
}

//static void I2C_SDA_Bit_Write(BitAction val)
//{
//	GPIO_InitTypeDef  GPIO_InitStructure;
//	GPIO_InitStructure.GPIO_Pin =  I2C_FRAM_SDA; 
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_Init(I2C_FRAM_GPIO,&GPIO_InitStructure);
//	GPIO_WriteBit(I2C_FRAM_GPIO,I2C_FRAM_SDA,val);  //SDA = val
//}


static void I2C_SDA_Bit_Set_Iput_Mod()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin =  I2C_FRAM_SDA; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;    
	GPIO_Init(I2C_FRAM_GPIO,&GPIO_InitStructure);	
}

static void I2C_SDA_Bit_Set_Output_Mod()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin =  I2C_FRAM_SDA; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    
	GPIO_Init(I2C_FRAM_GPIO,&GPIO_InitStructure);	
}

/****************************************************************************************
** 函数名称: void I2C_Start(void)
** 功能描述: 启动I2C操作 EEprom/FRAM  I2C
** 输　入: 无
** 输　出: 无
** 全局变量:
** 描述: 写外部eeprom / FRAM IIC总线
******************************************************************************************/
static void I2C_Start(void)
{
	I2C_WR_EN
	I2C_SDA_Bit_Set_Output_Mod();
	I2C_SDA_SET_1	//SDA = 1;
	I2CDelay();
	I2C_SCL_SET_1	//SCL = 1;
	I2CDelay(); 
	I2C_SDA_CLR_0	//SDA = 0; 
	I2CDelay(); 
	I2C_SCL_CLR_0	//SCL = 0; 
	I2CDelay();
}

/****************************************************************************************
** 函数名称: void stop(void)
** 功能描述: 停止I2C的操作 EEprom/FRAM  I2C
** 输　入: 无
** 输　出: 无
** 全局变量:
** 描述: 写外部eeprom / FRAM IIC总线
******************************************************************************************/
static void I2C_Stop(void)
{
	I2C_SDA_Bit_Set_Output_Mod();
	I2C_SDA_CLR_0;	//SDA = 0; 
	I2CDelay(); 
	I2C_SCL_SET_1;	//SCL = 1; 
	I2CDelay(); 
	I2C_SDA_SET_1	//SDA = 1; 
	I2CDelay();
	I2C_WR_DIS
}

/****************************************************************************************
** 函数名称: unsigned char writebyte(unsigned char j)
** 功能描述: 延时
** 输　入: unsigned char j:待写入字节
** 输　出: FALSE:错误  
** 全局变量:
** 描述: 写外部eeprom / FRAM IIC总线
******************************************************************************************/
static uint8 I2C_Writebyte(uint8 val)
{
	uint8 i;
	I2C_SDA_Bit_Set_Output_Mod();
	for (i=0;i<8;i++)
	{
		//temp = temp<<1; 
		I2C_SCL_CLR_0	//SCL = 0; 
		I2CDelay(); 
		if (val&(1<<(7-i)))
		{
			I2C_SDA_SET_1
		}
		else
		{
			I2C_SDA_CLR_0
		}
		I2CDelay(); 
		I2C_SCL_SET_1	//SCL = 1; 
		I2CDelay();
	}
	
	

	I2C_SCL_CLR_0	//SCL = 0; 
	I2CDelay(); 
	I2C_SDA_SET_1	//SDA = 1; 
	I2C_SDA_Bit_Set_Iput_Mod();
	I2CDelay();

	I2C_SCL_SET_1	//SCL = 1; 
	I2CDelay();
//	I2C_SDA_Bit_Set_Iput_Mod();
	i=0;
	while((i<255))
	{
		if(I2C_SDA_READ==0)   //SDA read
		{
			I2C_SCL_CLR_0	//SCL = 0; 
			I2CDelay();
			return TRUE;
		}
		i++;
	}
	I2C_SCL_CLR_0	//SCL=0; 
	I2CDelay();
	return FALSE; //error
}

/****************************************************************************************
** 函数名称: unsigned char readbyte(void)
** 功能描述: 从I2C读一个字节
** 输　入: 无
** 输　出: 读出的数节
** 全局变量:
** 描述: 写外部eeprom / FRAM IIC总线
******************************************************************************************/
static uint8 I2C_Readbyte(void)
{
	uint8 i,tmp,val=0;
	I2C_SDA_Bit_Set_Output_Mod();
	I2C_SCL_CLR_0	//SCL = 0; 
	I2CDelay(); 
	I2C_SDA_SET_1	//SDA = 1;
	I2C_SDA_Bit_Set_Iput_Mod();

	for(i=0;i<8;i++)
	{
		I2CDelay(); 
		I2C_SCL_SET_1	//SCL = 1; 
		I2CDelay();
		if(I2C_SDA_READ==1) 
			tmp=1;
		else 
			tmp=0;
		val |= (tmp<<(7-i)); 
		I2C_SCL_CLR_0	//SCL=0;
	}
	I2CDelay();
	return(val);
}

/****************************************************************************************
** 函数名称: unsigned char readbyte(void)
** 功能描述: 从I2C读一个字节
** 输　入: 无
** 输　出: 读出的数节
** 全局变量:
** 描述: 写外部eeprom / FRAM IIC总线
******************************************************************************************/
static uint8 I2C_ReadbyteWithAck(void)
{
	uint8 i,tmp,val=0;
	I2C_SDA_Bit_Set_Output_Mod();
	I2C_SCL_CLR_0	//SCL = 0; 
	I2CDelay(); 
	I2C_SDA_SET_1	//SDA = 1;
	I2C_SDA_Bit_Set_Iput_Mod();

	for(i=0;i<8;i++)
	{
		I2CDelay(); 
		I2C_SCL_SET_1	//SCL = 1; 
		I2CDelay();
		if(I2C_SDA_READ==1) 
			tmp=1;
		else 
			tmp=0;
		val |= (tmp<<(7-i)); 
		I2C_SCL_CLR_0	//SCL=0;
	}
	I2CDelay();
	//rm with ACK start
	I2C_SDA_Bit_Set_Output_Mod();
	I2C_SDA_CLR_0	//SDA = 0;   ACK
	I2CDelay();
	I2C_SCL_SET_1	//SCL = 1;
	//end
	return(val);
}

/****************************************************************************************
** 函数名称: unsigned char read_eeprom(unsigned int address)
** 功能描述: 从指定的地址读数据 I2C操作
** 输　入: unsigned int address:待写入地址
** 输　出: 读出的数节
** 全局变量:
** 描述: 写外部eeprom / FRAM IIC总线
******************************************************************************************/
static uint8 Read_EepromByte(uint32 address)
{
	uint8 val; 
	I2C_Start();
	I2C_Writebyte(0xa2);  //FM24C256 A2-A1 = 01  address	R/nW
	I2C_Writebyte(address>>8);
	I2C_Writebyte(address&0xff);
	I2C_Start();
	I2C_Writebyte(0xa3);  
	val=I2C_Readbyte();
	I2C_Stop();
	return(val);
}

/****************************************************************************************
** 函数名称: unsigned char write_page(unsigned int address,unsigned char *buf,unsigned char length)
** 功能描述: I2C 向指定的地址写一组指定长度的数据
** 输　入: unsigned int address:指定地址
**			unsigned char *buf:写入数据的数组指针
**			unsigned char length:写入数据的长度
** 输　出: TRUE:正确/FALSE:错误
** 全局变量:
** 描述: 写外部eeprom / FRAM IIC总线
******************************************************************************************/
uint8 Write_Page(uint32 address,uint8 *buf,uint16 length)
{
	uint16 i;
	I2C_Start();
	if(I2C_Writebyte(0xa2)==FALSE) {return FALSE;}  //error
	if(I2C_Writebyte(address>>8)==FALSE) {return FALSE;}
	if(I2C_Writebyte(address&0xff)==FALSE) {return FALSE;}
	for(i=0;i<length;i++)
	{
		if(!I2C_Writebyte(*buf))
		 {
		 	I2C_Stop();
			return FALSE;
		 }
		buf++; 
	}
	I2C_Stop();
	return TRUE;
}

/****************************************************************************************
** 函数名称: void read_page(unsigned int address,unsigned char *buf,unsigned char length)
** 功能描述: I2C 向指定的地址读一组指定长度的数据
** 输　入: unsigned int address:指定地址
**			unsigned char *buf:写入数据的数组指针
**			unsigned char length:写入数据的长度
** 输　出: 无
** 全局变量:
** 描述: 写外部eeprom / FRAM IIC总线
******************************************************************************************/
uint8 Read_Page(uint32 address,uint8 *buf,uint8 length)
{
	uint16 i;

	for(i=0;i<length;i++)
	{
		buf[i]=Read_EepromByte(address+i);
	}
/*	I2C_Start();
	I2C_Writebyte(0xa2);  //FM24C256 A2-A1 = 01  address	R/nW
	I2C_Writebyte(address>>8);
	I2C_Writebyte(address&0xff);
	I2C_Start();
	I2C_Writebyte(0xa3);
	for(i=0;i<length;i++)
	{
		buf[i]=I2C_Readbyte();
	}*/
	return TRUE;
}

/****************************************************************************************
** 函数名称: unsigned char Eeprom_Write(unsigned int address,unsigned char *buf,unsigned char length)
** 功能描述: I2C 向指定的地址写一组指定长度的数据
** 输　入: unsigned int address:指定地址
**			unsigned char *buf:写入数据的数组指针
**			unsigned char length:写入数据的长度
** 输　出: TRUE:正确/FALSE:错误
** 全局变量:
** 描述: 写外部eeprom / FRAM IIC总线
******************************************************************************************/
uint8 Eeprom_Write(uint8 *buf, uint32 address,uint16 length)
{
	uint8 err;
	uint8 ret;
	OSSemPend(FPRAM_Lock, 0, &err);
	ret = Write_Page(address,buf,length);
	OSSemPost(FPRAM_Lock);
	return ret;
}


/****************************************************************************************
** 函数名称: unsigned char write_page(unsigned int address,unsigned char *buf,unsigned char length)
** 功能描述: I2C 向指定的地址写一组指定长度的数据
** 输　入: unsigned int address:指定地址
**			unsigned char *buf:写入数据的数组指针
**			unsigned char length:写入数据的长度
** 输　出: TRUE:正确/FALSE:错误
** 全局变量:
** 描述: 写外部eeprom / FRAM IIC总线
******************************************************************************************/
uint8 Eeprom_Read(uint8 *buf, uint32 address, uint16 length)
{
	uint16 i;
	uint8 err;
	OSSemPend(FPRAM_Lock, 0, &err);
	I2C_Start();
	I2C_Writebyte(0xa2);  //FM24C256 A2-A0 = 001  address	R/nW
	I2C_Writebyte(address>>8);
	I2C_Writebyte(address&0xff);
	I2C_Start();
	I2C_Writebyte(0xa3);
	for(i=0;i<length;i++)
	{
		buf[i]=I2C_ReadbyteWithAck();
	}
	I2C_Stop();
	OSSemPost(FPRAM_Lock);
	return TRUE;
}



#else 	//   /*非模拟I2C*/

// ======================================================================================================
// 铁电IIC接口初始化 
// 参数：无
// 返回：无
// ======================================================================================================
void sysI2CInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure; 
  I2C_InitTypeDef  I2C_InitStructure; 

  RCC_APB1PeriphClockCmd(I2C_FRAM_CLK, ENABLE);   	  // IIC外设时钟打开 
  RCC_APB2PeriphClockCmd(I2C_FRAM_GPIO_CLK, ENABLE);  // IIC Pin 外设时钟打开
	
  GPIO_InitStructure.GPIO_Pin =  I2C_FRAM_SCL | I2C_FRAM_SDA; // PB6-IIC_SCL  PB7-IIC_SDA
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_Init(I2C_FRAM_GPIO, &GPIO_InitStructure);

  
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;                   	             // IIC模式 而非 SMBus模式
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;                         // 快速模式时的占空比 Tlow/Thigh = 2 
  I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_ADDRESS7;										 // 自身地址 0x50
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;				 	                       // 应答使能 
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;	 // 寻址模式 7位从地址 不是10位从地址
  I2C_InitStructure.I2C_ClockSpeed = I2C_Speed;	                             // IIC 速度 200K 
  
  I2C_Cmd(I2C_FRAM, ENABLE);	                                               // IIC使能               
  I2C_Init(I2C_FRAM, &I2C_InitStructure);							                       // 配置到寄存器 

	FPRAM_Lock = OSSemCreate(1);	   
}
// ==========================================================================================================
// uint16 I2C_FRAM_BufferWrite(uint8* buf, uint16 ext_addr, size)
// 向IIC FRAM写入一串数据
// 参数：buf 待写入数据串起始地址	，ext_addr 待写入FRAM地址范围0-0x7ff ，size 待写入数据长度 
// 返回：写入数据长度 
// =========================================================================================================
uint16 I2C_FRAM_BufferWrite(uint8* buf, uint16 ext_addr, uint16 size)
{
	uint8 err;
	uint16 ret = 0;
	if((ext_addr + size) > FRAM_DATA_SIZE)
	{
		return ret;
	}
	OSSemPend(FPRAM_Lock, 0, &err);
	while(I2C_GetFlagStatus(I2C_FRAM, I2C_FLAG_BUSY));	                      // 等待空闲 -- stop状态
  
  I2C_GenerateSTART(I2C_FRAM, ENABLE);	                                    // 发送IIC起始条件 
  
  while(!I2C_CheckEvent(I2C_FRAM, I2C_EVENT_MASTER_MODE_SELECT));           // 主模式+正在通讯+起始条件已经发送  -- start 状态 
  
  I2C_Send7bitAddress(I2C_FRAM, FRAM_DEV_ADDR, I2C_Direction_Transmitter);  // 发送 写操作的目的IIC器件地址

  while(!I2C_CheckEvent(I2C_FRAM, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); // 主模式+地址数据已经发送+数据寄存器为空 -- 设备地址已发送状态


	I2C_SendData(I2C_FRAM, (uint8_t)((ext_addr & 0xFF00) >> 8));              // 发送写操作数据地址高字节 
	while(!I2C_CheckEvent(I2C_FRAM, I2C_EVENT_MASTER_BYTE_TRANSMITTED));	    // 等待数据发送完成 
	I2C_SendData(I2C_FRAM, (uint8_t)(ext_addr & 0x00FF));                     // 发送写操作数据地址低字节 

  while(! I2C_CheckEvent(I2C_FRAM, I2C_EVENT_MASTER_BYTE_TRANSMITTED));	    // 等待写数据地址完成  -- 数据地址已发送状态

	while(size--)  	                                              					  // 发送数据出去
  {
    I2C_SendData(I2C_FRAM, *buf);                                           // 发送字节出去 
    buf++; 	
		ret++;				                                                          // 刷新指针 指向下一字节
    while (!I2C_CheckEvent(I2C_FRAM, I2C_EVENT_MASTER_BYTE_TRANSMITTED));	  // 等待数据发送完成
  }
  I2C_GenerateSTOP(I2C_FRAM, ENABLE);	                                      // 发送停止状态
	OSSemPost(FPRAM_Lock);
	return ret;
}

// ==========================================================================================================
// uint16 I2C_FRAM_BufferRead(uint8* buf, uint16 ext_addr, size)
// 从IIC FRAM随机读取一串数据
// 参数：buf 待读取数据串起始地址	，ext_addr 待读取FRAM地址范围0-0x7ff ，size 待读取数据长度 
// 返回：读取数据长度 
// =========================================================================================================
uint16 I2C_FRAM_BufferRead(uint8* buf, uint16 ext_addr, uint16 size)
{
  uint8 err;
	uint16 ret = 0;  
 	if((ext_addr + size) > FRAM_DATA_SIZE)
	{
		return ret;
	}
	
	OSSemPend(FPRAM_Lock, 0, &err);
  while(I2C_GetFlagStatus(I2C_FRAM, I2C_FLAG_BUSY));	                          // bus free OK 
  
  I2C_GenerateSTART(I2C_FRAM, ENABLE);	               
  while(!I2C_CheckEvent(I2C_FRAM, I2C_EVENT_MASTER_MODE_SELECT));	              // start  OK 
   
  I2C_Send7bitAddress(I2C_FRAM, FRAM_DEV_ADDR, I2C_Direction_Transmitter);	    // 发送写操作器件地址
  while(!I2C_CheckEvent(I2C_FRAM, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));	// devAddr OK
  I2C_Cmd(I2C_FRAM, ENABLE);

  I2C_SendData(I2C_FRAM, (uint8_t)((ext_addr & 0xFF00) >> 8));    	            // 写入读取数据地址
  while(!I2C_CheckEvent(I2C_FRAM, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
  I2C_SendData(I2C_FRAM, (uint8_t)(ext_addr & 0x00FF));    
  while(!I2C_CheckEvent(I2C_FRAM, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
  
  I2C_GenerateSTART(I2C_FRAM, ENABLE);		                                      // 主器件发送start状态，同时退出写操作模式
  while(!I2C_CheckEvent(I2C_FRAM, I2C_EVENT_MASTER_MODE_SELECT));
  
  I2C_Send7bitAddress(I2C_FRAM, FRAM_DEV_ADDR, I2C_Direction_Receiver);		      // 器件地址最低位置1 变为读操作
  while(!I2C_CheckEvent(I2C_FRAM, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
  
  while(size)  
  {
    if(size == 1)				                                                       // 最后一字节 
    {
      I2C_AcknowledgeConfig(I2C_FRAM, DISABLE);		                             // 禁止应答
      I2C_GenerateSTOP(I2C_FRAM, ENABLE);		                                   // stop状态设置 
    }
    if(I2C_CheckEvent(I2C_FRAM, I2C_EVENT_MASTER_BYTE_RECEIVED))  
    {      
      *buf = I2C_ReceiveData(I2C_FRAM);	                                       // 读取一字节
      buf++; 		                                                               // 刷新指针
      size--;  
			ret++;      
    }   
  }
  I2C_AcknowledgeConfig(I2C_FRAM, ENABLE);	                                   // 确认可以接收下一帧数据  
	OSSemPost(FPRAM_Lock);
	return ret; 
}

 #endif  //DEF_SIMULATE_I2C  
