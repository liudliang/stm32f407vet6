/**************************************************************************/
/***************************无线WIFI底层接口*******************************/
/**************************************************************************/
#include "main.h"
#include "uart.h"
#include "Back_Inter.h"
#include "TaskBackComm.h"
#include "wifi_RAK415.h"

#define 	WIFI_BAUD			115200 /* 后台串口波特率 */

//#define WIFI_RESET_L   Gpio_WritiLoutputIo(OUT9,Bit_RESET)
//#define WIFI_RESET_H	 Gpio_WritiLoutputIo(OUT9,Bit_SET)


/* 复位WIFI芯片 */
void Reset_RAK415(void)
{
//	WIFI_RESET_L;
	RAK415_Reset_Cmd(BACK_COM);
//	WIFI_RESET_H;
}
/* 复位4G芯片 */
void Reset_4G(void)
{

}
/* 初始化后台串口 */
static void Init_WifiCom(void)
{
	uint32 tmp;

	/* 获取串口 */
	UartHandler[BACK_COM] = Uart_Open(BACK_COM);
	
	/* 波特率 */
 	tmp  = WIFI_BAUD;
	Uart_IOCtrl(UartHandler[BACK_COM], SIO_BAUD_SET, &tmp);	

	/* 无校验 */
	tmp  = USART_Parity_No;
	Uart_IOCtrl(UartHandler[BACK_COM], SIO_PARITY_BIT_SET, &tmp);

	/* 停止位 */
	tmp  = USART_StopBits_1;
	Uart_IOCtrl(UartHandler[BACK_COM], SIO_STOP_BIT_SET, &tmp);	
	
	/* 9个数据位 */
	tmp = USART_WordLength_8b;
	Uart_IOCtrl(UartHandler[BACK_COM], SIO_DATA_BIT_SET, &tmp);	
}

/* WIFI模块初始化 */
void Init_WifiModule(ETH_PARAM_T *Para)
{
//	Reset_RAK415();
	Init_WifiCom();
	RAK415_ParaConfig(BACK_COM,Para);
}
/* 4G模块初始化 */
void Init_4GModule(ETH_PARAM_T *Para)
{
	Init_WifiCom();
}
/* 串口接收数据 */
int32 Uart_ReadData(uint8 *buf, uint16 maxSize)
{
	return Uart_Read(UartHandler[BACK_COM],buf,maxSize);
}


#define MAX_SEND_ONCE  80
/* 串口发送数据 */
int32 Uart_WriteData(uint8 *buf, uint16 len)
{
	uint16 wlen = 0;

	while(len){ //缓存不够，分段发送
		if(len > MAX_SEND_ONCE){
			wlen += Uart_Write(UartHandler[BACK_COM], &buf[wlen], MAX_SEND_ONCE);
			len = len - MAX_SEND_ONCE;
			Delay10Ms(3);
		}
		else{
			wlen += Uart_Write(UartHandler[BACK_COM], &buf[wlen], len);
			len = 0;
		}
	}
	return wlen;
}

/* 网络接口初始化 */
void Init_NetInterFunc(NET_DATA_INTER *pNet,uint8 Type)
{
	OS_CPU_SR  cpu_sr;
	OS_ENTER_CRITICAL();  
	switch(Type)
	{
//		case CONN_WIFI:
//		{
//			pNet->Init = Init_WifiModule;
//			pNet->Reset = Reset_RAK415;
//			pNet->WriteData = Uart_WriteData;
//			pNet->ReadData = Uart_ReadData;
//			break;
//		}
		case CONN_4G:
		{
			pNet->Init = Init_4GModule;
			pNet->Reset = Reset_4G;
			pNet->WriteData = Uart_WriteData;
			pNet->ReadData = Uart_ReadData;
			break;
		}
		case CONN_RS485:
		{
			pNet->Init = Init_WifiModule;
			pNet->Reset = Reset_RAK415;
			pNet->WriteData = Uart_WriteData;
			pNet->ReadData = Uart_ReadData;
			break;
		}
		default:
			pNet->Init = Init_WifiModule;
			pNet->Reset = Reset_RAK415;
			pNet->WriteData = Uart_WriteData;
			pNet->ReadData = Uart_ReadData;
			break;
	}
	OS_EXIT_CRITICAL();
}


