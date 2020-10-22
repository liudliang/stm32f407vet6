/**
  ******************************************************************************
  * @file    TaskIsoCheck.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#include "TaskIsoCheck.h"
#if (ISO_TYPE ==  ISO_SINGLE) /*奥耐自产绝缘模块*/

#include "common.h"
#include "crc.h"
#include "uart.h"
#include "TaskMainCtrl.h"
#include "ModbusMaster.h"
#include "message.h"
#include "ChgData.h"
#include "common.h"
#include "wdg.h"



typedef struct
{
	uint32 starttick;
	uint8  fg;
	uint8  msgfg;
	uint8  stopfg;
	uint16 waittm;
}CTRL_PARA_ST;


DEV_ISO_TYPE gIsoData;


DEV_ISO_TYPE * TskIso_GetDataPtr(void)
{
	 return &gIsoData;
}


static uint32 gIsoUartHandle = 0;
static CTRL_PARA_ST gCtrl[2];

extern OS_EVENT *gUartCommLock;
/* 初始化串口 */
void TskIso_InitCom(void)
{
	int32 tmp;
	PARAM_DEV_TYPE *param = ChgData_GetDevParamPtr();
	/* 打开串口 */
	UartHandler[ISO_COM] = Uart_Open(ISO_COM);
	gIsoUartHandle = UartHandler[METER_COM];
	
	/*设置波特率*/
 	//tmp  = param->dcmetbaud;  //METER_BAUD;
	tmp = 9600;
	
	Uart_IOCtrl(UartHandler[ISO_COM], SIO_BAUD_SET, &tmp);	

	/*设置校验位*/
	tmp  = USART_Parity_Even;//USART_Parity_Even;
	Uart_IOCtrl(UartHandler[ISO_COM], SIO_PARITY_BIT_SET, &tmp);

	/*设置停止位*/
	tmp  = USART_StopBits_1;
	Uart_IOCtrl(UartHandler[ISO_COM], SIO_STOP_BIT_SET, &tmp);	
	
	/*设置数据为*/
	tmp = USART_WordLength_9b;
	Uart_IOCtrl(UartHandler[ISO_COM], SIO_DATA_BIT_SET, &tmp);	
	
	if( NULL == gUartCommLock ) {
		gUartCommLock = OSSemCreate(1);
	}
	
}

#define MAX_ISO_BUF 10
/*启动绝缘检测*/
uint8 TskIso_CtrlOn(uint8 chl)
{
	 uint8 cnt,len;
	 int32 rlen;
	 uint8 *ptr;
	 uint8 tmplen;
	 uint8 buf[MAX_ISO_BUF] = {0};
   uint8 ctrval = 1;
//   chl %= 2;
//   ctrval = (chl == 0 ) ? 0x0A : 0xA0;
   ptr = buf;
   gIsoData.res1neg = 9999;
   gIsoData.res1pos = 9999;
   gIsoData.statu.bits.R1negErr = 0;
   gIsoData.statu.bits.R1posErr = 0;
	 gIsoData.res2neg = 9999;
   gIsoData.res2pos = 9999;
   gIsoData.statu.bits.R2negErr = 0;
   gIsoData.statu.bits.R2posErr = 0;
	 
   len = Modbus_WritSingleRegPkg(buf,MODBUS_ISO_ADR,MODBUS_ISOCTRL_REG,ctrval);
	 
#ifndef STM32F4XX
   uint8 err;
   OSSemPend(gUartCommLock, 0, &err);
#endif

   Uart_ClearReadBuf(gIsoUartHandle);
   Uart_Write(gIsoUartHandle,buf,len);
   Delay10Ms(5);
	 cnt = 0;
   len = 0;
   tmplen = 10;
   memset(buf,0,sizeof(buf));

   do {
		rlen = Uart_Read(gIsoUartHandle,ptr,MAX_ISO_BUF-len);
		if( rlen > 0 ) {
			ptr += rlen;
		  len += rlen;
			if(  len >= MAX_ISO_BUF ) {
				break;
			}
		}else {
			if( ( tmplen == len )&& ( len > 3 ) ) {
					break;
				}
    		tmplen = len;
				cnt++;
				Delay10Ms(2);
		}
	}while(cnt < 8 );

#ifndef STM32F4XX	
	OSSemPost(gUartCommLock);
#endif

	if( len >= 8 ) {
		 gCtrl[0].fg = 1;
		 gCtrl[0].stopfg = 0;
		 gCtrl[0].msgfg = 1;
		 gCtrl[0].starttick = GetSystemTick();
		 gIsoData.statu.bits.commerr = 0;
		 return 1;
	}
	
	return 0;
}


/*关闭绝缘检测*/
uint8 TskIso_CtrlOff(uint8 chl)
{

	 uint8 cnt;
	 int32 rlen;
	 uint8 *ptr;
	 uint8 tmplen;
	 uint8 buf[MAX_ISO_BUF] = {0};
   uint8 ctrval = 2;

#ifndef STM32F4XX
	 uint8 err;
   OSSemPend(gUartCommLock, 0, &err);
#endif

	 uint8 len = Modbus_WritSingleRegPkg(buf,MODBUS_ISO_ADR,MODBUS_ISOCTRL_REG,ctrval);
   Uart_ClearReadBuf(gIsoUartHandle);
   Uart_Write(gIsoUartHandle,buf,len);
   Delay10Ms(2);
   memset(buf,0,sizeof(buf));
   ptr = buf;
   cnt = 0;
   len = 0;
   tmplen = 10;
   
   do {
		rlen = Uart_Read(gIsoUartHandle,ptr,MAX_ISO_BUF-len);
		if( rlen > 0 ) {
			ptr += rlen;
		  len += rlen;
			if(  len >= MAX_ISO_BUF ) {
				break;
			}
		}else {
			 if( ( tmplen == len )&& ( len > 3 ) ) {
					break;
				}
    		tmplen = len;
				cnt++;
				Delay10Ms(2);
		}
	}while(cnt < 5 );

#ifndef STM32F4XX
  OSSemPost(gUartCommLock);
#endif

	if( len >= 8 ) {
		 gCtrl[chl].fg = 0;
		 gCtrl[0].stopfg = 1;
		 gIsoData.statu.bits.commerr = 0;
		 return 1;
	}

  return 0;

}






void TskIso_CheckJyRes(uint8 chl)
{
	PARAM_OPER_TYPE *param = ChgData_GetRunParamPtr();
	chl %= 2;
	if( gIsoData.res1neg < param->isoresist ) {
			gIsoData.statu.bits.R1negErr = 1;
	}else {
			gIsoData.statu.bits.R1negErr = 0;
	}	
		
	if(gIsoData.res1pos < param->isoresist) {
			gIsoData.statu.bits.R1posErr = 1;
	}else {
			gIsoData.statu.bits.R1posErr = 0;
	}

}






extern PARAM_OPER_TYPE gRunParam;
uint8 TskIso_ReadAndDealData(void) 
{
	
	uint8 tmplen = 0;
	uint8 cnt = 0;
	int32 rlen = 0;
	int32 len = 0;
	uint8 *ptr;
	uint8 msg[2];
	uint8 sbuf[10] = {0};
  static uint8 errCnt = 0;

  uint8 regnum = READ_REG_NUM;
  uint8 rbuf[MAX_RCV_SIZE] = {0};

#ifndef STM32F4XX
	uint8 err;
  OSSemPend(gUartCommLock, 0, &err);
#endif
	
  Uart_ClearReadBuf(gIsoUartHandle); 

  len = Modbus_ReadPkg(sbuf,MODBUS_ISO_ADR,MODBUS_ISO_START_REG,regnum);
	Uart_Write(gIsoUartHandle,sbuf,len);
	
  Delay10Ms(2);
  cnt = 0;
  len = 0;
  ptr = rbuf;
  tmplen = MAX_RCV_SIZE;
  do {
		rlen = Uart_Read(gIsoUartHandle,ptr,MAX_RCV_SIZE-len);
		if( rlen > 0 ) {
			ptr += rlen;
		  len += rlen;
			if( len > ( MAX_RCV_SIZE-5 ) ) {
				break;
			}
			cnt = 0;
		}else {
			if( ( tmplen == len )&& ( len > 3 ) ) {
					break;
			}
    	tmplen = len;
			cnt++;
			Delay10Ms(2);
		}
	}while(cnt < 10 );

#ifndef STM32F4XX
	OSSemPost(gUartCommLock);
#endif
	
  if( len < (regnum*2)+5 ) {
		if( errCnt++ > 200 ) {
			gIsoData.statu.bits.commerr = 1;
		}
		return 0;
	}
	//pRunData = (CHARGE_TYPE *)TskMainGetRunDataPtr();
	rlen = Modbus_CheckReadPkg(rbuf,MODBUS_ISO_ADR,MODBUS_CMD_03,READ_REG_NUM,len);
	if( rlen > 5 ) {
		errCnt = 0;

		/*也可单独赋值*/
		gIsoData.vdc3 = Common_Change2Byte((uint16 *)&rbuf[3]);  /*内侧电压*/
		gIsoData.res1pos = Common_Change2Byte((uint16 *)&rbuf[5]);
		gIsoData.res1neg = Common_Change2Byte((uint16 *)&rbuf[7]);
		
		if( (1 == gCtrl[0].fg) && (GetSystemTick() - gCtrl[0].starttick > 2*TIM_2S) ) {
			 gCtrl[0].fg = 0;
			 cnt = 0;
			 do {
				rlen = TskIso_CtrlOff(0);
				if(cnt++ > 3 ) {
					break;
				}
				Delay10Ms(2);
			}while( 0 == rlen );
      gCtrl[0].starttick = GetSystemTick();		
		}
		/*信瑞达绝缘检测仪测试已经可以报50K以下绝缘故障 20181027 zqj*/
		if( 1 == gCtrl[0].stopfg && gCtrl[0].fg == 0 && (GetSystemTick() - gCtrl[0].starttick >  TIM_1S )) {
			gCtrl[0].stopfg = 0;
			TskIso_CheckJyRes(0);
		   /*发送绝缘检测完成消息*/
			 if( 1 == gCtrl[0].msgfg ) {
					msg[0] = 1;
					gCtrl[0].msgfg = 0;
					SendMsgWithNByte(MSG_ISO_FINISH,1,&msg[0],APP_TASK_AGUNMAINCTRL_PRIO);
			}
		}
		gIsoData.statu.bits.commerr = 0;
	}else {
		if( errCnt++ > 100 ) {
			gIsoData.statu.bits.commerr = 1;
		}
	}
	
	return 0;
}

void init_data(void)
{
	memset(&gIsoData,0,sizeof(DEV_ISO_TYPE));
	memset(&gCtrl,0,2*sizeof(CTRL_PARA_ST));
	gCtrl[0].waittm = TIM_5S * 2; /*5s*/
	
}

/*消息处理*/
void TskIso_DealMsg(MSG_STRUCT * msg)
{
	uint8 temp,cnt;
	cnt = 0;
	temp = 0;
	switch(msg->MsgType)
	{
		case MSG_ISO_START:
			do {
				temp = TskIso_CtrlOn(msg->MsgData[0]);
				if(cnt++ > 2 ) {
					break;
				}
			}while( 0 == temp );
			break;
		case MSG_ISO_STOP:
			do {
				temp = TskIso_CtrlOff(msg->MsgData[0]);
				if(cnt++ > 2 ) {
					break;
				}
			}while( 0 == temp );
			break;

		default:
			break;
	}
}






/*绝缘监测单元任务*/
void Task_IsoCheck(void *p_arg)
{
	 MSG_STRUCT msg;
	 memset(&msg,0,sizeof(MSG_STRUCT));
   init_data();
	 TskIso_InitCom();
	 Message_QueueCreat(APP_TASK_ISO_PRIO);
	
   while(1) {
		 
		if(RcvMsgFromQueue(&msg) == TRUE)
		{
			TskIso_DealMsg(&msg);
		}
		
		 TskIso_ReadAndDealData();
//		 Wdg_feeddog();
		 Delay10Ms(10);
	 }
}


#endif



