#include "CardReader.h"
#include "usart.h"
#include <string.h>
//////////////////////////////////////////////////////////////////////////////////	 
//功能：主板控制单元(全局变量的定义及获取接口、主控流程接口)   
//作者：zyf
//创建日期:2020/04/16
//版本：V1.0								  
////////////////////////////////////////////////////////////////////////////////// 

#define CARD_BAUD		        115200
static uint32_t gReaderUartHandler;

/* 初始化读卡器串口*/
void TskReader_InitCom(void)
{
	int32_t tmp;
	UartHandler[CARD_COM] = Uart_Open(CARD_COM);
	gReaderUartHandler = UartHandler[CARD_COM];
	
 	tmp  = CARD_BAUD;
	Uart_IOCtrl(UartHandler[CARD_COM], SIO_BAUD_SET, &tmp);	

	tmp  = USART_Parity_No;
	Uart_IOCtrl(UartHandler[CARD_COM], SIO_PARITY_BIT_SET, &tmp);

	tmp  = USART_StopBits_1;
	Uart_IOCtrl(UartHandler[CARD_COM], SIO_STOP_BIT_SET, &tmp);	
	
	tmp = USART_WordLength_8b;
	Uart_IOCtrl(UartHandler[CARD_COM], SIO_DATA_BIT_SET, &tmp);	
}


void test_uarthandle(void)
{
	OS_ERR err;
		 uint8_t reg,regnum,rlen,tmplen,cnt = 0,len = 0,*ptr = NULL;
	 #define MAX_DCMET_SIZE  32
	uint8_t buf[MAX_DCMET_SIZE] = {0};
		 memset(buf,0,MAX_DCMET_SIZE);
	 
	 cnt = 0;
	 len = 0;
	 ptr = &buf[0];
	 tmplen = MAX_DCMET_SIZE;
	 do {
		 rlen = Uart_Read(gReaderUartHandler,ptr,MAX_DCMET_SIZE-len);
		 if( rlen > 0 ) {
			ptr += rlen;
		  len += rlen;
			if( len >= MAX_DCMET_SIZE) {
				break;
			}
			cnt = 0;
		}else {
			if( ( tmplen == len )&& ( len > 3 ) ) {
					break;
			 }
    	 tmplen = len;
			 cnt++;
			 OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时20ms
		}
	}while(cnt < 10 );
		 
	if(0 < len)
		Uart_Write(gReaderUartHandler, buf, len);
}

//CardReader任务函数
void CardReader_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	TskReader_InitCom();
	
	while(1)
	{
		test_uarthandle();
//		LED_Toggle(LED3);
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
	}
}