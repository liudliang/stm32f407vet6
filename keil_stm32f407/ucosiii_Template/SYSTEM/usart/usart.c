#include "sys.h"
#include "usart.h"	
#include "queue.h"	
//////////////////////////////////////////////////////////////////////////////////	 
//功能：控制板所有串口功能的初始化   
//作者：
//创建日期:2020/04/29
//版本：V1.0								  
////////////////////////////////////////////////////////////////////////////////// 


//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif


STM32F10x_UART_CHAN UARTChan[N_UART_CHANNELS];



#if 1
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif
 
#if 0   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	

//初始化IO 串口1 
//bound:波特率
void uart_init(u32 bound){
   //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//使能USART1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10复用为USART1
	
	//USART1端口配置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA9，PA10

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  USART_Init(USART1, &USART_InitStructure); //初始化串口1
	
  USART_Cmd(USART1, ENABLE);  //使能串口1 
	
	//USART_ClearFlag(USART1, USART_FLAG_TC);
	
#if EN_USART1_RX	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启相关中断

	//Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

#endif

 /* for can't send first byte when hardware resetting*/
	USART_GetFlagStatus(USART1, USART_FLAG_TC);
}


void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	u8 Res;
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
		
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}   		 
  } 
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
} 
#endif	






uint32_t gUartRcvBuff0[512];
uint32_t gUartSndBuff0[1024/4];


uint32_t gUartRcvBuff[5][512/4];
uint32_t gUartSndBuff[5][400/4];

#define  MAX_BACK_RCVBUF_LENGTH  (512*4)
#define  MAX_BACK_SNDBUF_LENGTH  1024

#define 	MAX_UART_RCVBUF_LENTH    512    //   串口收发缓冲区大小
#define   MAX_UART_SNDBUF_LENTH    400



#define _IS_USER_FIFO_

uint8_t PowerOnForUart = FALSE;	
        
uint32_t UartHandler[] = {0,0,0,0,0,0};

USART_InitTypeDef USART_InitStructure;

STM32F10x_UART_CHAN UARTChan[N_UART_CHANNELS];

static char *UARTChanSemName[] = {"UARTChanSem0", "UARTChanSem1"};   //串口信号量的名字，根据串口数添加

static STM32F10x_SIO_CHAN_PARAM UARTParameter[] = {	     //注意:参数2-3 等原理图/PCB  再填充 
		{(void *)USART1_BASE, (void *)GPIOA, GPIO_Pin_3,  RS232TYPE, 115200,   USART_WordLength_8b, USART_StopBits_1, USART_Parity_No, HFC_NONE,UART_USE_TYPE_NOMAL},	
		{(void *)USART2_BASE, (void *)GPIOD, GPIO_Pin_7,  RS485TYPE, 2400,     USART_WordLength_8b, USART_StopBits_1, USART_Parity_No, HFC_NONE,UART_USE_TYPE_NOMAL},
//		{(void *)USART3_BASE, (void *)GPIOD, GPIO_Pin_10, RS485TYPE, 9600,     USART_WordLength_8b, USART_StopBits_1, USART_Parity_No, HFC_NONE,UART_USE_TYPE_NOMAL},	
//		{(void *)UART4_BASE,  (void *)GPIOG, GPIO_Pin_4,  RS232TYPE, 115200,   USART_WordLength_8b, USART_StopBits_1, USART_Parity_No, HFC_NONE,UART_USE_TYPE_NOMAL},
//		{(void *)UART5_BASE,  (void *)GPIOG, GPIO_Pin_5,  RS232TYPE, 115200,   USART_WordLength_8b, USART_StopBits_1, USART_Parity_No, HFC_NONE,UART_USE_TYPE_NOMAL},
//		{(void *)USART6_BASE, (void *)GPIOC, GPIO_Pin_8,  RS485TYPE, 2400,     USART_WordLength_8b, USART_StopBits_1, USART_Parity_No, HFC_NONE,UART_USE_TYPE_NOMAL},
		};


STM32F10x_UART_CHAN *GetChanHandler(uint8_t id)
{
	if(id < 5)
			return  &UARTChan[id] ;
	return NULL;
}


// *************************************************
// Uart_EnIsr - desable a ISR .	
// RETURN: N
// *************************************************
static void Uart_EnableIsr(STM32F10x_UART_CHAN* pChan)
{	
	USART_Cmd((USART_TypeDef*)pChan->base, ENABLE);	
}

// ***********************************************************************
// 串口设备打开函数 
// 参数:  deviceId	:串口ID   取值范围： 1 ~  N_UART_CHANNELS
// 返回值： FALSE - 打开失败, 其他：串口句柄 
// *************************************************************************
int32_t Uart_Open(uint32_t deviceId)
{
  CPU_TS *p_ts = NULL;
  OS_ERR err;
	if(deviceId <= N_UART_CHANNELS)
	{
		OSSemPend(UARTChan[deviceId].uartLock, 0, OS_OPT_PEND_BLOCKING, p_ts, &err);   // 获取信号量   
			
		if(UARTChan[deviceId].deviceOpenFlag == FALSE)	   // 未打开状态 
		{						
			UARTChan[deviceId].deviceOpenFlag = TRUE;  	   // 标记为已打开
			Uart_EnableIsr(&UARTChan[deviceId]);   
			OSSemPost(UARTChan[deviceId].uartLock, OS_OPT_POST_1, &err);		
			return (int32_t)&UARTChan[deviceId];
		}else {
			OSSemPost(UARTChan[deviceId].uartLock, OS_OPT_POST_1, &err);	           // 释放信号量 		
			return (int32_t)&UARTChan[deviceId];
		}		
	}	
	return FALSE;
}
// ***********************************************************************
// 串口波特率设置操作  
// 参数:  pChan - 串口句柄，pbase - 寄存器操作基地址   
// 返回值： FALSE - 打开失败, 其他：串口句柄 
// *************************************************************************

BOOL Uart_BaudSet(STM32F10x_UART_CHAN* pChan,  USART_TypeDef* pbase)
{
	uint32_t apbclock = 0, tmpreg = 0, integerdivider=0, fractionaldivider=0;
	RCC_ClocksTypeDef  RCC_ClocksStatus;
#if defined (STM32F10X_CL) 
	assert_param(IS_USART_BAUDRATE(bps));  	                    // 参数检验 
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	if (pbase == (USART_TypeDef*)USART1_BASE)
	{
		apbclock = RCC_ClocksStatus.PCLK2_Frequency;
	}
	else
	{
		apbclock = RCC_ClocksStatus.PCLK1_Frequency;
	}
	integerdivider = ((0x19 * apbclock) / (0x04 * (pChan->baudRate)));		// Determine the integer part
	tmpreg = (integerdivider / 0x64) << 0x04;	
	fractionaldivider = integerdivider - (0x64 * (tmpreg >> 0x04));		    // Determine the fractional part 
	tmpreg |= ((((fractionaldivider * 0x10) + 0x32) / 0x64)) & ((uint8_t_t)0x0F);	
	pbase->BRR = (uint16_t_t)tmpreg;		                                    // Write to USART BRR
	return TRUE;
#endif

#if defined (STM32F4XX)
	/*---------------------------- USART BRR Configuration -----------------------*/
  /* Configure the USART Baud Rate */
  RCC_GetClocksFreq(&RCC_ClocksStatus);

  if ((pbase == USART1) || (pbase == USART6))
  {
    apbclock = RCC_ClocksStatus.PCLK2_Frequency;
  }
  else
  {
    apbclock = RCC_ClocksStatus.PCLK1_Frequency;
  }
  
  /* Determine the integer part */
  if ((pbase->CR1 & USART_CR1_OVER8) != 0)
  {
    /* Integer part computing in case Oversampling mode is 8 Samples */
    integerdivider = ((25 * apbclock) / (2 * (pChan->baudRate)));    
  }
  else /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
  {
    /* Integer part computing in case Oversampling mode is 16 Samples */
    integerdivider = ((25 * apbclock) / (4 * (pChan->baudRate)));    
  }
  tmpreg = (integerdivider / 100) << 4;

  /* Determine the fractional part */
  fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

  /* Implement the fractional part in the register */
  if ((pbase->CR1 & USART_CR1_OVER8) != 0)
  {
    tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
  }
  else /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
  {
    tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
  }
  
  /* Write to USART BRR register */
  pbase->BRR = (uint16_t)tmpreg;	

#endif
  return TRUE;
}	
// *************************************************
// 开启串口 
// 开中断 串口使能 
// pbase -- 串口寄存器基地址 
// *************************************************/
static void  Uart_Start(USART_TypeDef* pbase)
{
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);	// 发送缓冲区数据已经移走 
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	// 接收缓冲区收到数据 

	// USART使能 USART_CR1 bit14  		
	USART_Cmd(USART1, ENABLE);

}
// *************************************************
// UartOptionSet -set uart option .byte size,stop bit,parity 	
// PARA: pChan 串口基本信息数据结构 ，	pbase 寄存器组起始地址 
// RETURN: 
// ************************************************
static void UartOptionSet(STM32F10x_UART_CHAN *pChan, USART_TypeDef* pbase)
{
	uint32_t tmpreg = pbase->CR1;
		
	if(pChan->WordLength == USART_WordLength_9b)				// byte size
		tmpreg |= (1<<12);                      // 一个起始位，9个数据位，一个停止位 	
	else
		tmpreg &= ~((uint32_t)(1<<12));				// 一个起始位，8个数据位，n个停止位 
	if(pChan->Parity != USART_Parity_No)  // 校验控制被使能
	{
		tmpreg |= (1<<10);	
		if(pChan->Parity == USART_Parity_Odd) // 奇校验
		{
		 	tmpreg |= (1<<9);	
		}
		else
			tmpreg &= ~((uint32_t)(1<<9));		
	}
	else
		tmpreg &= ~((uint32_t)(1<<10));	
	pbase->CR1 = tmpreg;   

	tmpreg = pbase->CR2;
	tmpreg &= ~((uint32_t)(3<<12));	        // 一个停止位 
	if(pChan->Stopbits == USART_StopBits_0_5)          // 0.5个停止位 
		tmpreg |= (1<<12);
	else if(pChan->Stopbits == USART_StopBits_2)      // 2个停止位 
		tmpreg |= (2<<12);	
	else if(pChan->Stopbits == USART_StopBits_1_5)     // 1.5个停止位 
		tmpreg |= (3<<12);	
	pbase->CR2 = tmpreg;  	
}
// ***************************************************************************
// UartTypeSet -   管脚配置及初始化   485模式时 收发控制管脚配置及其初始化		
// PARA:   pChan 串口基本信息数据结构 ，pbase 寄存器组起始地址 
// RETURN: 无 
/*
USART1  - TX:PA9 RX:PA10 TEN:PB7
USART2  - TX:PD5 RX:PD6 TEN:PB9
USART3  - TX:PD8 RX:PD9 TEN:PB14
UART4  - TX:PC10 RX:PC11 TEN:PB15
UART5  - TX:PC12 RX:PD2 TEN:PD7
*/
// ****************************************************************************

static void UartTypeSet(STM32F10x_UART_CHAN *pChan, USART_TypeDef* pbase)
{
	GPIO_InitTypeDef GPIO_InitStructure;

#if defined (STM32F10X_CL) 
	if(pbase == (USART_TypeDef*)USART1_BASE) // UART0,(PA10 USART1_RX,PA9 USART1_TX) 
	{		
		// 板子上用PA9和PA10，与USB(CN2)复用，接跳线JP42 JP43 不进行重映射 	USB不能复用 
		// 配置复用重映射和调试I/O配置寄存器AFIO_MAPR 不使用USAR1的 IO重映射功能		
		GPIO_PinRemapConfig(GPIO_Remap_USART1, DISABLE);

		/* Enable GPIO clock */
		RCC_AHB1PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM], ENABLE);

		//enble clock
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
				
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;		         // GPIO管脚配置	  USART_RX
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;                // GPIO管脚配置	  USART_TX
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		if(pChan->devicType == RS485TYPE)  // 485方式 需要配置485收发控制管脚 
		{
			GPIO_InitStructure.GPIO_Pin = pChan->rs485ContrlPin;                // GPIO管脚配置	  USART_TXEN
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
			GPIO_Init((GPIO_TypeDef*)pChan->directionGPIO, &GPIO_InitStructure); 
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_SET);  //clr ctr pin
		}
	 }
	 else if (pbase == (USART_TypeDef*)USART2_BASE)
	 {
		// 板子上 重映射 到 PD3-PD7  (USART2_CTS USART2_RTS USART2_TX USART2_RX USART2_CK)
		// 配置复用重映射和调试I/O配置寄存器AFIO_MAPR 使用USAR2的 IO重映射功能	 不影响其他功能 只是占用GPIO 	
		GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);

		//enble clock
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
				
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		         // GPIO管脚配置	  USART_RX
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;                // GPIO管脚配置	  USART_TX
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOD, &GPIO_InitStructure);

		if(pChan->devicType == RS485TYPE)  // 485方式 需要配置485收发控制管脚 
		{
			GPIO_InitStructure.GPIO_Pin = pChan->rs485ContrlPin;                // GPIO管脚配置	  USART_TXEN
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
			GPIO_Init((GPIO_TypeDef*)pChan->directionGPIO, &GPIO_InitStructure); 
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_SET);  //clr ctr pin
		}	 
	 }
	 else if (pbase == (USART_TypeDef*)USART3_BASE)
	 {
		// 不重映射不可行，因为与ethnet复用 
		// 部分重映射不可行，因为映射占用uart4管脚
		// 全部重映射 可能可行， 这样ethnet一定不能重映射 	
		// 板子上 完全重映像 到 PD8-12  (USART3_TX USART3_RX USART3_CK USART3_CTS USART3_RTS)
		// 配置复用重映射和调试I/O配置寄存器AFIO_MAPR 使用USAR3的 IO部分重映射功能	
		GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
		
		//enble clock
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
				
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		         // GPIO管脚配置	  USART_RX
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;                // GPIO管脚配置	  USART_TX
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOD, &GPIO_InitStructure);

		if(pChan->devicType == RS485TYPE)  // 485方式 需要配置485收发控制管脚 
		{
			GPIO_InitStructure.GPIO_Pin = pChan->rs485ContrlPin;                // GPIO管脚配置	  USART_TXEN
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
			GPIO_Init((GPIO_TypeDef*)pChan->directionGPIO, &GPIO_InitStructure); 
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_RESET);  //clr ctr pin
		}	 
	 }
	 else if (pbase == (USART_TypeDef*)UART4_BASE)
	 {
		// 没有重映射的功能 PC10-11 (UART4_TX UART4_RX)	
		//enble clock
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
							
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;		         // GPIO管脚配置	  USART_RX
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                // GPIO管脚配置	  USART_TX
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOC, &GPIO_InitStructure);

		if(pChan->devicType == RS485TYPE)  // 485方式 需要配置485收发控制管脚 
		{
			GPIO_InitStructure.GPIO_Pin = pChan->rs485ContrlPin;                // GPIO管脚配置	  USART_TXEN
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
			GPIO_Init((GPIO_TypeDef*)pChan->directionGPIO, &GPIO_InitStructure); 
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_SET);  //clr ctr pin
		}	 
	 }
	 else if (pbase == (USART_TypeDef*)UART5_BASE)
	 {
		// 没有重映射的功能 PC12 PD2 (UART5_TX UART5_RX)
		//enble clock
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
								
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;		         // GPIO管脚配置	  USART_RX
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;                // GPIO管脚配置	  USART_TX
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOC, &GPIO_InitStructure);

		if(pChan->devicType == RS485TYPE)                        // 485方式 需要配置485收发控制管脚 
		{
			GPIO_InitStructure.GPIO_Pin = pChan->rs485ContrlPin;                // GPIO管脚配置	  USART_TXEN
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
			GPIO_Init((GPIO_TypeDef*)pChan->directionGPIO, &GPIO_InitStructure); 
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_SET);  //clr ctr pin
		}	 
	}
	
	if(pChan->devicType == IRDATYPE)
	{	//红外模式
		/* Configure the USARTy IrDA mode */
		USART_IrDAConfig(pbase,USART_IrDAMode_Normal);
		/* Enable the USARTy IrDA mode */
		USART_IrDACmd(pbase, ENABLE);  		
	}
	else
	{
		/* disable the USARTy IrDA mode */
		USART_IrDACmd(pbase, DISABLE);
	}    
#endif

#if defined (STM32F4XX)
	if(pbase == (USART_TypeDef*)USART1_BASE) // UART1,(PA10 USART1_RX,PA9 USART1_TX) 
	{		
		/* Enable UART clock */
    	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	

		/* Enable GPIO clock */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

		/* Connect PXx to USARTx_Tx*/
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
		
		/* Connect PXx to USARTx_Rx*/
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource10,GPIO_AF_USART1);
		
		/* Configure USART Tx as alternate function  */
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);		 // GPIO管脚配置	  USART_TX

		/* Configure USART Rx as alternate function  */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;		         // GPIO管脚配置	  USART_RX
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		if(pChan->devicType == RS485TYPE)  // 485方式 需要配置485收发控制管脚 
		{
			GPIO_InitStructure.GPIO_Pin = pChan->rs485ContrlPin;                // GPIO管脚配置	  USART_TXEN
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init((GPIO_TypeDef*)pChan->directionGPIO, &GPIO_InitStructure); 
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_SET);  //clr ctr pin
		}
	 }
	 else if (pbase == (USART_TypeDef*)USART2_BASE)
	 {
		/* Enable GPIO clock */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

		/* Connect PXx to USARTx_Tx*/
		GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
		
		/* Connect PXx to USARTx_Rx*/
		GPIO_PinAFConfig(GPIOD, GPIO_PinSource6,GPIO_AF_USART2);
		
		/* Configure USART Tx as alternate function  */
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOD, &GPIO_InitStructure);		 // GPIO管脚配置	  USART_TX	   PD5

		/* Configure USART Rx as alternate function  */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		         // GPIO管脚配置	  USART_RX	PD6
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		
		if(pChan->devicType == RS485TYPE)  // 485方式 需要配置485收发控制管脚 
		{
			GPIO_InitStructure.GPIO_Pin = pChan->rs485ContrlPin;                // GPIO管脚配置	  USART_TXEN
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init((GPIO_TypeDef*)pChan->directionGPIO, &GPIO_InitStructure); 
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_SET);  //clr ctr pin
		}
	 }
#if 0	 
	 else if (pbase == (USART_TypeDef*)USART3_BASE)
	 {
		/* Enable GPIO clock */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

		/* Connect PXx to USARTx_Tx*/
		GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
		
		/* Connect PXx to USARTx_Rx*/
		GPIO_PinAFConfig(GPIOD, GPIO_PinSource9,GPIO_AF_USART3);
		
		/* Configure USART Tx as alternate function  */
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOD, &GPIO_InitStructure);		 // GPIO管脚配置	  USART_TX	   PB10

		/* Configure USART Rx as alternate function  */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		         // GPIO管脚配置	  USART_RX	PB11
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		
		if(pChan->devicType == RS485TYPE)  // 485方式 需要配置485收发控制管脚 
		{
			GPIO_InitStructure.GPIO_Pin = pChan->rs485ContrlPin;                // GPIO管脚配置	  USART_TXEN
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init((GPIO_TypeDef*)pChan->directionGPIO, &GPIO_InitStructure); 
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_RESET);  //clr ctr pin
		}
	 }
	 else if (pbase == (USART_TypeDef*)UART4_BASE)
	 { 			
		/* Enable GPIO clock */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

		/* Connect PXx to USARTx_Tx*/
		GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);
		
		/* Connect PXx to USARTx_Rx*/
		GPIO_PinAFConfig(GPIOC, GPIO_PinSource11,GPIO_AF_UART4);
		
		/* Configure USART Tx as alternate function  */
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);		 // GPIO管脚配置	  USART_TX	   PC10

		/* Configure USART Rx as alternate function  */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;		         // GPIO管脚配置	  USART_RX	PC11
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		
		if(pChan->devicType == RS485TYPE)  // 485方式 需要配置485收发控制管脚 
		{
			GPIO_InitStructure.GPIO_Pin = pChan->rs485ContrlPin;                // GPIO管脚配置	  USART_TXEN
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init((GPIO_TypeDef*)pChan->directionGPIO, &GPIO_InitStructure); 
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_SET);  //clr ctr pin
		}	 
	 }
	 else if (pbase == (USART_TypeDef*)UART5_BASE)
	 {	 		
		/* Enable GPIO clock */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

		/* Connect PXx to USARTx_Tx*/
		GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_UART5);
		
		/* Connect PXx to USARTx_Rx*/
		GPIO_PinAFConfig(GPIOD, GPIO_PinSource2,GPIO_AF_UART5);
		
		/* Configure USART Tx as alternate function  */
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);		 // GPIO管脚配置	  USART_TX	   PC12

		/* Configure USART Rx as alternate function  */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;		         // GPIO管脚配置	  USART_RX	PD2
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		
		if(pChan->devicType == RS485TYPE)  // 485方式 需要配置485收发控制管脚 
		{
			GPIO_InitStructure.GPIO_Pin = pChan->rs485ContrlPin;                // GPIO管脚配置	  USART_TXEN
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init((GPIO_TypeDef*)pChan->directionGPIO, &GPIO_InitStructure); 
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_SET);  //clr ctr pin
		}	  
	}
	else if (pbase == (USART_TypeDef*)USART6_BASE)
	 {
	 	/* Enable UART clock */
    	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

		/* Enable GPIO clock */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

		/* Connect PXx to USARTx_Tx*/
		GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
		
		/* Connect PXx to USARTx_Rx*/
		GPIO_PinAFConfig(GPIOC, GPIO_PinSource7,GPIO_AF_USART6);
		
		/* Configure USART Tx as alternate function  */
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);		 // GPIO管脚配置	  USART_TX	   PC6

		/* Configure USART Rx as alternate function  */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;		         // GPIO管脚配置	  USART_RX	PC7
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		
		if(pChan->devicType == RS485TYPE)  // 485方式 需要配置485收发控制管脚 
		{
			GPIO_InitStructure.GPIO_Pin = pChan->rs485ContrlPin;                // GPIO管脚配置	  USART_TXEN
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init((GPIO_TypeDef*)pChan->directionGPIO, &GPIO_InitStructure); 
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_SET);  //clr ctr pin
		}
	 }
#endif
	
	if(pChan->devicType == IRDATYPE)
	{	//红外模式
		/* Configure the USARTy IrDA mode */
		USART_IrDAConfig(pbase,USART_IrDAMode_Normal);
		/* Enable the USARTy IrDA mode */
		USART_IrDACmd(pbase, ENABLE);  		
	}
	else
	{
		/* disable the USARTy IrDA mode */
		USART_IrDACmd(pbase, DISABLE);
	}
	
#endif

}

//模式判断
BOOL IsUartMode(uint8_t mode)
{
	if((mode == RS232TYPE)||(mode == RS485TYPE)||(mode == IRDATYPE)||(mode == IR38KHZTYPE))
		return TRUE;

	return FALSE;
}


//串口参数设置
BOOL Uart_ParamSet(STM32F10x_UART_CHAN* pChan)
{
	USART_InitTypeDef USART_InitStructure; 		
	USART_TypeDef* pbase = (USART_TypeDef*)pChan->base;	

	USART_InitStructure.USART_BaudRate = pChan->baudRate;	     // 串口参数配置 bps 数据位 停止位 校验方式 流控 
	USART_InitStructure.USART_WordLength = pChan->WordLength;
	USART_InitStructure.USART_StopBits = pChan->Stopbits;
	USART_InitStructure.USART_Parity = pChan->Parity;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	
	USART_Init(pbase, &USART_InitStructure);				 // 串口参数配置
	return TRUE;
}

// **************************************************************************
// Uart_IOCtrl -opreat  the UART 
// PARA: devDesc--device description, cmd--request cmd, arg--some argument.
// RETURN: FALSE - error,or TRUE - successful
//****************************************************************************
int Uart_IOCtrl(int32_t devDesc, int32_t cmd, void* arg)
{
	CPU_SR_ALLOC();
#if OS_CRITICAL_METHOD == 3
	OS_CPU_SR  cpu_sr;
#endif  
	int ret = FALSE;	
	OS_ERR err;
	CPU_TS *p_ts = NULL;
	STM32F10x_UART_CHAN* pChan = (STM32F10x_UART_CHAN*)devDesc;	 // 串口信息数据结构 
	USART_TypeDef* pbase = (USART_TypeDef*)pChan->base;	         // 寄存器基地址数据结构 

	if((devDesc == 0) || (pbase == 0))
	{
		switch(cmd)  //特殊操作，为了在不知道设备句柄情况下控制设备
		{
			case SIO_GET_PORT_HANDLE:
				OS_CRITICAL_ENTER();	//进入临界区
				if(*(uint32_t*)arg <= N_UART_CHANNELS)
				{		
					*(uint32_t*)arg = (int32_t)&UARTChan[*(uint32_t*)arg]; 
				}	
				else
				{
						*(uint32_t*)arg = NULL;
				}
				OS_CRITICAL_EXIT();	//退出临界区
				ret = TRUE;
				break;
			default:
				break;
		}
		return ret;
	}
	OSSemPend(pChan->uartLock, 0, OS_OPT_PEND_BLOCKING, p_ts, &err);
	switch(cmd)
	{
		case SIO_BAUD_SET:	                           // Set the baud rate 
		{
			OS_CRITICAL_ENTER();	//进入临界区
			pChan->baudRate = *(int32_t*)arg;				
			if(Uart_ParamSet(pChan) == TRUE)     // 设置波特率 
			{
				ret = TRUE;
			}
			OS_CRITICAL_EXIT();	//退出临界区		
		}
		break;
		case SIO_START:	                               // UART Start work
			OS_CRITICAL_ENTER();	//进入临界区
			Uart_Start(pbase);
			OS_CRITICAL_EXIT();	//退出临界区
			ret = TRUE;
			break;
		case SIO_HW_OPTS_SET:	                       // Set wordbits
			OS_CRITICAL_ENTER();	//进入临界区
			UartOptionSet(pChan, pbase);	
			OS_CRITICAL_EXIT();	//退出临界区
			ret = TRUE;
			break;
		case SIO_PARITY_BIT_SET:	                       // Parity bit set
			OS_CRITICAL_ENTER();	//进入临界区
			if(IS_USART_PARITY(*(int32_t*)arg))
			{
				pChan->Parity = *(int32_t*)arg;
				Uart_ParamSet(pChan);
			}	
			OS_CRITICAL_EXIT();	//退出临界区
			ret = TRUE;
			break;
		case SIO_STOP_BIT_SET:	                       // STOP bit set
			OS_CRITICAL_ENTER();	//进入临界区
			if(IS_USART_STOPBITS(*(int32_t*)arg))
			{
				pChan->Stopbits = *(int32_t*)arg;		 //	   pChan->Parity = *(int32*)arg;
				Uart_ParamSet(pChan);
			}	
			OS_CRITICAL_EXIT();	//退出临界区
			ret = TRUE;
			break;
		case SIO_DATA_BIT_SET:	                       // DATA bit set
			OS_CRITICAL_ENTER();	//进入临界区
			if(IS_USART_WORD_LENGTH(*(int32_t*)arg))
			{
				pChan->WordLength = *(int32_t*)arg;	//		pChan->Parity = *(int32*)arg;
				Uart_ParamSet(pChan);
			}	
			OS_CRITICAL_EXIT();	//退出临界区
			ret = TRUE;
			break;
		case SIO_WORK_TYPE_SET:	                      // type set(rs232 or rs485)
			OS_CRITICAL_ENTER();	//进入临界区
			if(IsUartMode(*(int*)arg))
			{
				pChan->devicType = *(int*)arg;
				UartTypeSet(pChan, pbase);
				ret = TRUE;
			}
			OS_CRITICAL_EXIT();	//退出临界区		
			break;
		case SIO_SEND_STATUS_GET:	                      // 获得发送器状态
			OS_CRITICAL_ENTER();	//进入临界区
			*(int*)arg = pChan->SendStatus;	  //type info UART_SND_STA
			OS_CRITICAL_EXIT();	//退出临界区			
			break;
		case SIO_SEND_BUF_FULL_STATUS_GET: /*获得发送缓冲满状态*/
			OS_CRITICAL_ENTER();	//进入临界区
			if(arg != NULL)
			{
			    if(IsQueueFull(pChan->SendBuf) == FALSE)
				{//不满
					*(uint32_t*)arg = UART_SEND_BUF_NO_FULL;			
				}else{ //满
					*(uint32_t*)arg = UART_SEND_BUF_FULL;
				}
				ret = TRUE;
			}
			OS_CRITICAL_EXIT();	//退出临界区
			break;	
		case SIO_USE_TYPE_SET:	                      // 设置使用类型
			OS_CRITICAL_ENTER();	//进入临界区
			if(IS_VALID_USE_TYPE(*(int*)arg))
			{
				pChan->useType = (UART_USE_TYPE)(*(int*)arg);	  
			}
			OS_CRITICAL_EXIT();	//退出临界区			
			break;
		case SIO_USE_TYPE_GET:	                      // 获得使用类型
			OS_CRITICAL_ENTER();	//进入临界区
			*(int*)arg = pChan->useType;	  
			OS_CRITICAL_EXIT();	//退出临界区			
			break;
		case SIO_GPS_SYN_TIME_GET:	                      // 获得GPS同步时刻时间
			OS_CRITICAL_ENTER();	//进入临界区
			*(uint32_t*)arg = pChan->Timegps.RevGpsSynTime;	  
			OS_CRITICAL_EXIT();	//退出临界区			
			break;
		default:
			break;
	}
	OSSemPost(pChan->uartLock, OS_OPT_POST_1, &err);
	return ret;
}

// *************************************************
// Uart_EnIsr - desable a ISR .	
// RETURN: N
// *************************************************
static void Uart_DisableIsr(STM32F10x_UART_CHAN* pChan)
{	
	USART_Cmd((USART_TypeDef*)pChan->base, DISABLE);	
}

// *************************************************
// Uart_Close
// RETURN: FALSE - error,or TRUE - successful
// *************************************************
int32_t Uart_Close(uint32_t devDesc)		//device description
{
	int8_t i;
	OS_ERR err;
	CPU_TS *p_ts = NULL;
	for(i = 0;i < N_UART_CHANNELS; i++)
	{
		if(devDesc == (uint32_t)&UARTChan[i])
		{
			OSSemPend(UARTChan[i].uartLock, 0, OS_OPT_PEND_BLOCKING, p_ts, &err);	
			UARTChan[i].deviceOpenFlag = FALSE;
			Uart_DisableIsr(&UARTChan[i]);
			OSSemPost(UARTChan[i].uartLock, OS_OPT_POST_1, &err);			
			return TRUE;
		}
	}
	return FALSE;
}
// ********************************************************************************
// Uart_Read -read data from the UART .
// note:
//	the function will be pend when no data 
// PARA: devDesc - device description struct ,	buf	- save to buffer
// RETURN: byte numbers readed
// *********************************************************************************
int32_t Uart_Read(int32_t devDesc,uint8_t *buf, uint16_t maxSize)
{
	uint16_t ret = 0;
	uint16_t length;
	STM32F10x_UART_CHAN* pChan = (STM32F10x_UART_CHAN*)devDesc;
	if(devDesc == 0)
	{
		return 0;
	}
	if(devDesc == (uint32_t)&UARTChan[BACK_COM]) {
		length = MAX_BACK_RCVBUF_LENGTH;
	}else {
		length = MAX_UART_RCVBUF_LENTH;
	}
	for(ret = 0; ((ret < length) && (ret < maxSize)); ret++)	           // 取数据 
	{
	   if(QueueRead(buf, pChan->RcvBuf) == QUEUE_OK)           // 读取成功 
			buf++;	   		
	   else
		break;
	}
	return ret;
}

/********************************************************************************
** Uart_ClearReadBuf -clear read buf  .
** note:
** the function will be pend when no data 
** PARA: devDesc - device description struct ,	buf	- save to buffer
** RETURN: byte numbers readed
*********************************************************************************/
int32_t Uart_ClearReadBuf(int32_t devDesc)
{
	uint16_t cnt = 0;
	int32_t ret;
	uint8_t buf[50] ;
	cnt = 0;
	do{
		ret = Uart_Read(devDesc,buf,50);
		if( cnt++ > 100 ) {  /*old is 10 20190403 */
			 break;
		}
	}while(ret > 0);
	
	return ret;
}

/********************************************************************************
** Uart_GetBytesNumInBuff -write data to the UART .
** PARA: devDesc -- device description, buf size--buffer and  data size writed
** RETURN: numbers writed 
*********************************************************************************/
uint16_t Uart_GetBytesNumInBuff(int32_t devDesc)
{
	STM32F10x_UART_CHAN* pChan = (STM32F10x_UART_CHAN*)devDesc;
	if(devDesc == 0)
	{
		return 0;
	}
	 return QueueNData(pChan->RcvBuf);
}

// ***************************************************************************
// Uart_Write -write data to the UART .
// PARA: devDesc -- device description, buf size--buffer and  data size writed
// RETURN: numbers writed 
// ****************************************************************************
int32_t Uart_Write(int32_t devDesc,	uint8_t *buf,	int32_t size)
{
	int delayMax = 200;
	OS_ERR err;
	CPU_TS *p_ts = NULL;
	int len = size;
	int32_t temp;
	STM32F10x_UART_CHAN* pChan = (STM32F10x_UART_CHAN*)devDesc;
	USART_TypeDef* pbase = (USART_TypeDef*)pChan->base;
	if(devDesc == 0)
	{		
		return 0;
	}
	if(pChan->useType == UART_USE_TYPE_GPS)
	{
		return 0;
	}
	OSSemPend(pChan->uartLock, 0, OS_OPT_PEND_BLOCKING, p_ts, &err);	
	if (len > 0)
	{	
		do{	
			while((err = QueueWrite((void *)pChan->SendBuf, buf)) == QUEUE_OK)    // 数据入队 
			{
				len --;
				buf++;
				if(len <= 0)
					break;			
			}
			USART_ITConfig(pbase, USART_IT_TXE, ENABLE);	
			USART_ITConfig(pbase, USART_IT_TC, ENABLE);	
			if(pChan->devicType == IR38KHZTYPE)
			{	//38K调制型红外模式，为半双工模式
				USART_ITConfig(pbase, USART_IT_RXNE, DISABLE); 
			}
			if(err==QUEUE_FULL)
			{
				OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_HMSM_STRICT,&err); //延时10ms
			}				 				
		}while((--delayMax <= 0) && (len>=0));		
	}
	OSSemPost(pChan->uartLock, OS_OPT_POST_1, &err);
	delayMax = 200;
	while(--delayMax)
	{
		OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_HMSM_STRICT,&err); //延时10ms
		if((len = QueueNData(pChan->SendBuf)) == 0)
			break;
	}
	temp = (int32_t)(size - len);
	return temp;
}
//===================================================================
// 串口时钟使能 
//
//===================================================================
void UsartClockOperation(STM32F10x_UART_CHAN *pChan, uint8_t cmd)
{
	uint8_t i;
	for(i = 0;i < N_UART_CHANNELS; i++)
	{
		if(pChan == &UARTChan[i])  
		{
			if(i == 0)
			{
				if(cmd != DISABLE)	  // USART1时钟使能
					RCC->APB2ENR |= (1<<14);	
				else
					RCC->APB2ENR &= ~((uint32_t)(1<<14));				
			}
			else	// usart 2-5
			{
				if(cmd != DISABLE)	  // USART1时钟使能
					RCC->APB1ENR |= (1<<(16+i));	
				else
					RCC->APB1ENR &= ~((uint32_t)(1<<(16+i)));				
					
			}
		} 	
	}
}

/*************************************************
LPCUartDevInit - initialize  a  UART .
RETURN: N
*************************************************/
static void UartDevInit(STM32F10x_UART_CHAN* pChan)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_InitTypeDef USART_InitStructure;
		
	USART_TypeDef* pbase = (USART_TypeDef*)pChan->base;

	UsartClockOperation(pChan, ENABLE);                          // 串口外设 RCC时钟配置 

	USART_InitStructure.USART_BaudRate = pChan->baudRate;	     // 串口参数配置 bps 数据位 停止位 校验方式 流控 
	USART_InitStructure.USART_WordLength = pChan->WordLength;
	USART_InitStructure.USART_StopBits = pChan->Stopbits;
	USART_InitStructure.USART_Parity = pChan->Parity;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	UartTypeSet(pChan, pbase);                                   // 管脚配置

	if(pbase == (USART_TypeDef*)USART1_BASE)                     // UART0,(PA10 USART1_RX,PA9 USART1_TX) 
	{		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); 	 // 配置启用 USART外设时钟 
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);	  	     // 配置 串口NVIC  应用中断与复位控制寄存器   不对应用优先级进行分组
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;	     // 中断号 37  
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	     // 子优先级 0 
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	         // 使能串口1中断 
		NVIC_Init(&NVIC_InitStructure);	                         // 配置中断优先级并使能中断 						
		
		USART_Init(USART1, &USART_InitStructure);				 // 串口参数配置 
 		 
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);	         // 允许的中断类型-发送缓冲区数据已经移走
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	         // 允许的中断类型-接收缓冲区收到数据 
				  		
		USART_Cmd(USART1, ENABLE);	                             // USART使能 USART_CR1 bit13
	}
	else if(pbase == (USART_TypeDef*)USART2_BASE)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); 	 // 配置启用 USART外设时钟 

		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	  	     // 配置 串口NVIC  应用中断与复位控制寄存器   不对应用优先级进行分组
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;	     // 中断号 38  
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	     // 子优先级 1 
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	         // 使能串口中断 
		NVIC_Init(&NVIC_InitStructure);	                         // 配置中断优先级并使能中断 		
		 
		USART_Init(USART2, &USART_InitStructure);				 // 串口参数配置
		USART_ITConfig(USART2, USART_IT_TXE, ENABLE); 	         // 允许的中断类型-发送缓冲区数据已经移走
		USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	         // 允许的中断类型-接收缓冲区收到数据 		
		USART_Cmd(USART2, ENABLE);		                         // USART使能		
	}
#if 0	
	else if(pbase == (USART_TypeDef*)USART3_BASE)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); 	 // 配置启用 USART外设时钟 

		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	  	     // 配置 串口NVIC  应用中断与复位控制寄存器   不对应用优先级进行分组
		NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;	     // 中断号 39  
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;	     // 子优先级 
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	         // 使能串口中断 
		NVIC_Init(&NVIC_InitStructure);	                         // 配置中断优先级并使能中断 		
		
		USART_Init(USART3, &USART_InitStructure);				 // 串口参数配置 
		 
		USART_ITConfig(USART3, USART_IT_TXE, ENABLE);	         // 允许的中断类型-发送缓冲区数据已经移走
		USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);	         // 允许的中断类型-接收缓冲区收到数据 
				
		USART_Cmd(USART3, ENABLE);								 // USART使能
	}	
	else if(pbase == (USART_TypeDef*)UART4_BASE)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE); 	 // 配置启用 UART外设时钟 

		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	  	     // 配置 串口NVIC  应用中断与复位控制寄存器   不对应用优先级进行分组
		NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;	     // 中断号 52  
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	     // 子优先级 
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	         // 使能串口中断 
		NVIC_Init(&NVIC_InitStructure);	                         // 配置中断优先级并使能中断 		

		 
		USART_Init(UART4, &USART_InitStructure);				 // 串口参数配置

		USART_ITConfig(UART4, USART_IT_TXE, ENABLE);	         // 允许的中断类型-发送缓冲区数据已经移走
		USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);	         // 允许的中断类型-接收缓冲区收到数据 
		
 		USART_ClearFlag(pChan->base,USART_FLAG_TXE);
		
		USART_Cmd(UART4, ENABLE);						         // USART使能
	}	
	else if(pbase == (USART_TypeDef*)UART5_BASE)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE); 	 // 配置启用 UART外设时钟 

		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	  	     // 配置 串口NVIC  应用中断与复位控制寄存器   不对应用优先级进行分组
		NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;	     // 中断号 53  
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;	     // 子优先级 
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	         // 使能串口中断 
		NVIC_Init(&NVIC_InitStructure);	                         // 配置中断优先级并使能中断 		
		 
		USART_Init(UART5, &USART_InitStructure);				 // 串口参数配置

		USART_ITConfig(UART5, USART_IT_TXE, ENABLE);	         // 允许的中断类型-发送缓冲区数据已经移走
		USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);	         // 允许的中断类型-接收缓冲区收到数据 
 		
		USART_Cmd(UART5, ENABLE);						         // USART使能
	}
#if defined (STM32F4XX)
	else if(pbase == (USART_TypeDef*)USART6_BASE)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE); 	 // 配置启用 USART外设时钟 
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	  	     // 配置 串口NVIC  应用中断与复位控制寄存器   不对应用优先级进行分组
		NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;	     // 中断号 37  
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	     // 子优先级 0 
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	         // 使能串口1中断 
		NVIC_Init(&NVIC_InitStructure);	                         // 配置中断优先级并使能中断 						
		
		USART_Init(USART6, &USART_InitStructure);				 // 串口参数配置 
 		 
		USART_ITConfig(USART6, USART_IT_TXE, ENABLE);	         // 允许的中断类型-发送缓冲区数据已经移走
		USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);	         // 允许的中断类型-接收缓冲区收到数据 
				  		
		USART_Cmd(USART6, ENABLE);	                             // USART使能 USART_CR1 bit13
	}
#endif

#endif

	UartOptionSet(pChan, pbase); //add option	
}





/********************************************************************************
// Function Name  : GetDevChan()
// Description    : This function get device object.
// Input          : dev address
// Output         : None
// Return         : device struct
*********************************************************************************/
STM32F10x_UART_CHAN *GetDevChan(USART_TypeDef* uart)
{
	uint8_t i;
	for(i = 0;i < N_UART_CHANNELS; i++)	
	{
		if(uart == UARTChan[i].base)
			return &UARTChan[i];
	}	
	return NULL;
}
#ifdef STM32F4XX
 #define TRANS_TIMES 10
#endif
// *******************************************************************************
// Function Name  : USART1_IRQHandler
// Description    : This function handles USART1 global interrupt request.
// Input          : None
// Output         : None
// Return         : None
// *******************************************************************************
//extern SERIALPORT serial;
void  UartDealInIRQ(STM32F10x_UART_CHAN* pChan)
{
	uint8_t tmpdata,rx_data;

	if (USART_GetFlagStatus(pChan->base, USART_FLAG_RXNE) != RESET)  // 接收缓冲区非空 
	{
		rx_data = (uint8_t)USART_ReceiveData(pChan->base);
		QueueWriteInInt(pChan->RcvBuf, &rx_data); 
		// 写一个字节到队列  此处暂不考虑溢出 溢出数据不再接收 				
	}
	if (USART_GetFlagStatus(pChan->base, USART_FLAG_TXE) != RESET)  // 接收缓冲区非空  
	{   
		USART_ClearFlag(pChan->base, USART_FLAG_TXE);
		if (QueueReadInInt(&tmpdata, pChan->SendBuf) == QUEUE_OK) // 有数据要发送 
		{
			if (pChan->devicType == RS485TYPE)
			{
				GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_SET);  //set ctr pin
			}
			USART_SendData(pChan->base, tmpdata);
			pChan->SendStatus = UART_SEND_BUSY;		
		}
		else
		{		
      USART_ClearITPendingBit(pChan->base, USART_IT_TXE);			
			USART_ITConfig(pChan->base, USART_IT_TXE, DISABLE);
		}		
	}
  if (USART_GetFlagStatus(pChan->base, USART_FLAG_TC) != RESET)  
	{
		USART_ClearFlag(pChan->base, USART_FLAG_TC);
		if (pChan->devicType == RS485TYPE)
		{
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_RESET);  //clr ctr pin
			for(int i = 0; i< TRANS_TIMES; i++ );
		}
		USART_ITConfig(pChan->base, USART_IT_TC, DISABLE);
  }
	
  if (USART_GetFlagStatus(pChan->base, USART_FLAG_ORE) != RESET)
	{        
		rx_data = USART_ReceiveData(pChan->base);
		USART_ClearFlag(pChan->base, USART_FLAG_ORE);
	}
	
	if (USART_GetFlagStatus(pChan->base, USART_FLAG_NE) != RESET)
	{        
		USART_ClearFlag(pChan->base, USART_FLAG_NE);
	}
	
	if (USART_GetFlagStatus(pChan->base, USART_FLAG_FE) != RESET)
	{        
		USART_ClearFlag(pChan->base, USART_FLAG_FE);
	}
	
	if (USART_GetFlagStatus(pChan->base, USART_FLAG_PE) != RESET)
	{        
		USART_ClearFlag(pChan->base, USART_FLAG_PE);
	}
}

// *******************************************************************************
// Function Name  : USART1_IRQHandler
// Description    : This function handles USART1 global interrupt request.
// Input          : None
// Output         : None
// Return         : None
// *******************************************************************************
void USART1_IRQHandler(void)
{    
//	uint8_t tmpdata,rx_data;
	CPU_SR_ALLOC();
	uint32_t regsr, regcr1;
//	OS_CPU_SR  cpu_sr;
	STM32F10x_UART_CHAN* pChan = GetDevChan(USART1);	
	OS_CRITICAL_ENTER();	//进入临界区                                      // Tell uC/OS-II that we are starting an ISR
	OSIntNestingCtr++;
	OS_CRITICAL_EXIT();	//退出临界区

	regsr = USART1->SR;
	regcr1 = USART1->CR1;
	if(regsr == regcr1)
	{
	   regsr = 0;
	}
	if(pChan != NULL)
	{
		UartDealInIRQ(pChan);
	} 	
	OSIntExit();                                                // Tell uC/OS-II that we are leaving the ISR 
}
//*******************************************************************************
// Function Name  : USART2_IRQHandler
// Description    : This function handles USART2 global interrupt request.
// Input          : None
// Output         : None
// Return         : None
//*******************************************************************************
void USART2_IRQHandler(void)
{    
//	uint8_t tmpdata,rx_data;
	CPU_SR_ALLOC();
	STM32F10x_UART_CHAN* pChan = GetDevChan(USART2);
		
	OS_CRITICAL_ENTER();	//进入临界区                                      // Tell uC/OS-II that we are starting an ISR
	OSIntNestingCtr++;
	OS_CRITICAL_EXIT();	//退出临界区

	if(pChan != NULL)
	{
		UartDealInIRQ(pChan);
	} 	
	OSIntExit();                                                // Tell uC/OS-II that we are leaving the ISR 
}
// *******************************************************************************
// Function Name  : USART3_IRQHandler
// Description    : This function handles USART1 global interrupt request.
// Input          : None
// Output         : None
// Return         : None
//*******************************************************************************
void USART3_IRQHandler(void)
{    
//	uint8_t tmpdata, rx_data;
	CPU_SR_ALLOC();
	STM32F10x_UART_CHAN* pChan = GetDevChan(USART3);
		
	OS_CRITICAL_ENTER();	//进入临界区                                      // Tell uC/OS-II that we are starting an ISR
	OSIntNestingCtr++;
	OS_CRITICAL_EXIT();	//退出临界区

	if(pChan != NULL)
	{
		UartDealInIRQ(pChan);
	} 	
	OSIntExit();                                                // Tell uC/OS-II that we are leaving the ISR 
}

// *******************************************************************************
// Function Name  : USART4_IRQHandler
// Description    : This function handles USART1 global interrupt request.
// Input          : None
// Output         : None
// Return         : None
//*******************************************************************************
void UART4_IRQHandler(void)
{    
//	uint8_t tmpdata, rx_data;
	CPU_SR_ALLOC();
	STM32F10x_UART_CHAN* pChan = GetDevChan(UART4);
		
	OS_CRITICAL_ENTER();	//进入临界区                                      // Tell uC/OS-II that we are starting an ISR
	OSIntNestingCtr++;
	OS_CRITICAL_EXIT();	//退出临界区

	if(pChan != NULL)
	{
		UartDealInIRQ(pChan);
	} 	
	OSIntExit();                                                // Tell uC/OS-II that we are leaving the ISR 
}

// *******************************************************************************
// Function Name  : USART5_IRQHandler
// Description    : This function handles USART1 global interrupt request.
// Input          : None
// Output         : None
// Return         : None
//*******************************************************************************
void UART5_IRQHandler(void)
{    
//	uint8_t tmpdata, rx_data;
	CPU_SR_ALLOC();
	STM32F10x_UART_CHAN* pChan = GetDevChan(UART5);
		
	OS_CRITICAL_ENTER();	//进入临界区                                      // Tell uC/OS-II that we are starting an ISR
	OSIntNestingCtr++;
	OS_CRITICAL_EXIT();	//退出临界区

	if(pChan != NULL)
	{
		UartDealInIRQ(pChan);
	} 	
	OSIntExit();                                                // Tell uC/OS-II that we are leaving the ISR 
}

#if defined (STM32F4XX)
// *******************************************************************************
// Function Name  : USART5_IRQHandler
// Description    : This function handles USART1 global interrupt request.
// Input          : None
// Output         : None
// Return         : None
//*******************************************************************************
void USART6_IRQHandler(void)
{    
//	uint8_t tmpdata, rx_data;
	CPU_SR_ALLOC();
	STM32F10x_UART_CHAN* pChan = GetDevChan(USART6);
		
	OS_CRITICAL_ENTER();	//进入临界区                                      // Tell uC/OS-II that we are starting an ISR
	OSIntNestingCtr++;
	OS_CRITICAL_EXIT();	//退出临界区

	if(pChan != NULL)
	{
		UartDealInIRQ(pChan);
	} 	
	OSIntExit();                                                // Tell uC/OS-II that we are leaving the ISR 
}
#endif




// ******************************************************************************
// sysSerialHwInit - initialize  Serial Devece struct and Hardware at first.
// RETURN: NONE
// ******************************************************************************
void sys_SerialHwInit(void)
{
	uint8_t i;
	OS_ERR err;
	if(PowerOnForUart == FALSE)
	{
		PowerOnForUart = TRUE;
		for(i = 0;i < N_UART_CHANNELS; i++)		 
		{
			OSSemCreate(UARTChan[i].uartLock, UARTChanSemName[i], 1, &err);	      // 创建信号锁  初始化为已打开  
			UARTChan[i].base = UARTParameter[i].baseAddr;                // 寄存器基地址 
			UARTChan[i].directionGPIO =  UARTParameter[i].directionGPIO; // 方向控制GPIO 
			UARTChan[i].rs485ContrlPin = UARTParameter[i].rs485ContrlPin;// 方向控制脚序号 
			UARTChan[i].deviceID = i+1;                                  // 串口 ID
			UARTChan[i].deviceOpenFlag = FALSE;	                         // 打开标志初始化为 未打开 
			UARTChan[i].devicType = UARTParameter[i].devicType;	         // 设备类型 0 rs232 1 rs485

			UARTChan[i].baudRate = UARTParameter[i].defBuardRate;	     // 波特率 
			UARTChan[i].WordLength = UARTParameter[i].defWordLength;	 // 默认数据位 5-8 
			UARTChan[i].Stopbits = UARTParameter[i].defStopbits;	     // 默认停止位 1-2  
			UARTChan[i].Parity = UARTParameter[i].defParity;	         // 默认校验位
			UARTChan[i].SendStatus =  UART_SEND_IDLE;
			UARTChan[i].useType =  UARTParameter[i].useType;	         // 使用设备类型	 
			memset(&UARTChan[i].Timegps,0,sizeof(gpsuart_t)); 
			if( i == BACK_COM ) {
				UARTChan[BACK_COM].RcvBuf =  (uint8_t *)&gUartRcvBuff0[0];
				UARTChan[BACK_COM].SendBuf = (uint8_t*)&gUartSndBuff0[0];
				QueueCreate(UARTChan[i].RcvBuf, MAX_BACK_RCVBUF_LENGTH,1,0,0);     // 接收数据队列 
				QueueCreate(UARTChan[i].SendBuf, MAX_BACK_SNDBUF_LENGTH,1,0,0);    // 发送数据队列 		
			}else {
				UARTChan[i].RcvBuf  = (uint8_t *)&gUartRcvBuff[i-1][0];
				UARTChan[i].SendBuf = (uint8_t*)&gUartSndBuff[i-2][0];
				QueueCreate(UARTChan[i].RcvBuf, MAX_UART_RCVBUF_LENTH,1,0,0);     // 接收数据队列 
				QueueCreate(UARTChan[i].SendBuf, MAX_UART_SNDBUF_LENTH,1,0,0);    // 发送数据队列 	
			}
			UartDevInit(&UARTChan[i]);
		}
	}
}

/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/




