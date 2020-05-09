#include "sys.h"
#include "delay.h"
#include "usart.h"


#include "gpio.h"
#include "timer.h"
#include "Debug.h"

//////////////////////////////////////////////////////////////////////////////////	 
//功能：调试功能api  
//作者：
//创建日期:2020/05/09
//版本：V1.0								  
////////////////////////////////////////////////////////////////////////////////// 

char DEBUG_Buff[50] = {0}; 
#define DEBUG_COM_BAUD		        115200
static uint32_t gDebugHandler;


#define SCOMM_DEBUG   //串口调试

#ifdef SCOMM_DEBUG
#define DEBUG_COM    COM1   //调试打印口

void DebugScomm_init(void)
{
	int32_t tmp;
	UartHandler[DEBUG_COM] = Uart_Open(DEBUG_COM);
	gDebugHandler = UartHandler[DEBUG_COM];
	
 	tmp  = DEBUG_COM_BAUD;
	Uart_IOCtrl(UartHandler[DEBUG_COM], SIO_BAUD_SET, &tmp);	

	tmp  = USART_Parity_No;
	Uart_IOCtrl(UartHandler[DEBUG_COM], SIO_PARITY_BIT_SET, &tmp);

	tmp  = USART_StopBits_1;
	Uart_IOCtrl(UartHandler[DEBUG_COM], SIO_STOP_BIT_SET, &tmp);	
	
	tmp = USART_WordLength_8b;
	Uart_IOCtrl(UartHandler[DEBUG_COM], SIO_DATA_BIT_SET, &tmp);	
}
#endif



void DEBUG_Printf(char *s)
{
#ifdef SCOMM_DEBUG	
	Uart_Write(gDebugHandler, (uint8_t *)s, strlen(s));
#endif
	
}




void DEBUG_Init(void)
{
	
#ifdef SCOMM_DEBUG
  DebugScomm_init();
#endif

}

