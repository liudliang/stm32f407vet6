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

#if (ISO_TYPE ==  ISO_AUNICE) /*奥耐自产绝缘模块*/

#include "common.h"
#include "crc.h"
#include "uart.h"
#include "TaskMainCtrl.h"
#include "ModbusMaster.h"
#include "message.h"
#include "ChgData.h"


#include "Adebug.h"

#ifdef ISO_ON


typedef struct
{
	uint32 starttick;
	uint8  fg;
	uint16 waittm;
}CTRL_PARA_ST;


DEV_ISO_TYPE gIsoData[DEF_MAX_GUN_NO];

DEV_ISO_TYPE * TskIso_GetDataPtr(uint8 gunNo)
{
	 return &gIsoData[gunNo%DEF_MAX_GUN_NO];
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
	gIsoUartHandle = UartHandler[ISO_COM];
	
	/*设置波特率*/
 	//tmp  = param->dcmetbaud;  //METER_BAUD;
	tmp = 9600;
	
	Uart_IOCtrl(UartHandler[ISO_COM], SIO_BAUD_SET, &tmp);	

	/*设置校验位*/
	tmp  = USART_Parity_No;//USART_Parity_Even;
	Uart_IOCtrl(UartHandler[ISO_COM], SIO_PARITY_BIT_SET, &tmp);

	/*设置停止位*/
	tmp  = USART_StopBits_1;
	Uart_IOCtrl(UartHandler[ISO_COM], SIO_STOP_BIT_SET, &tmp);	
	
	/*设置数据为*/
	tmp = USART_WordLength_8b;
	Uart_IOCtrl(UartHandler[ISO_COM], SIO_DATA_BIT_SET, &tmp);	
	
	if( NULL == gUartCommLock ) {
		gUartCommLock = OSSemCreate(1);
	}
	
}

/*启动绝缘检测*/
uint8 TskIso_CtrlOn(uint8 chl)
{
	 uint8 cnt;
	 int32 rlen;
	 uint8 *ptr;
	 uint8 tmplen;
	 uint8 buf[10] = {0};
   uint8 ctrval = 0;
   chl %= 2;
   ctrval = (chl == 0 ) ? 0x0A : 0xA0;

	 DebugInfoWithPi(CHK_MOD,"启动AU绝缘检测",chl);

	 uint8 len = Modbus_WritSingleRegPkg(buf,MODBUS_ISO_ADR,MODBUS_ISOCTRL_REG,ctrval);
	  
   Uart_ClearReadBuf(gIsoUartHandle);

   Uart_Write(gIsoUartHandle,buf,len);
   Delay10Ms(10);
   memset(buf,0,sizeof(buf));
   ptr = buf;
   cnt = 0;
   len = 0;
   tmplen = 10;
   gIsoData[chl].res1neg = 9999;
   gIsoData[chl].res1pos = 9999;
   gIsoData[chl].statu.bits.R1negErr = 0;
   gIsoData[chl].statu.bits.R1posErr = 0;

	do {
		rlen = Uart_Read(gIsoUartHandle,ptr,10-len);
		if( rlen > 0 ) {
			ptr += rlen;
		  len += rlen;
			if(  len >= 10 ) {
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

 if( len >= 8 ) {
		 gCtrl[chl].fg = 1;
		 gCtrl[chl].starttick = GetSystemTick();
		 return 1;
	}
	return 0;
}


void TskIso_CheckJyRes(uint8 chl)
{
	PARAM_OPER_TYPE *param = ChgData_GetRunParamPtr();
	chl %= 2;

	if( gIsoData[chl].res1neg < param->isoresist ) {
		gIsoData[chl].statu.bits.R1negErr = 1;
	}else {
		gIsoData[chl].statu.bits.R1negErr = 0;
	}	
		
	if(gIsoData[chl].res1pos < param->isoresist) {
		gIsoData[chl].statu.bits.R1posErr = 1;
	}else {
		gIsoData[chl].statu.bits.R1posErr = 0;
	}

}



void TskIso_CheckErrStatu(uint8 chl)
{
	
	PARAM_OPER_TYPE *param = ChgData_GetRunParamPtr();
	
	if ( gIsoData[chl].vdc1 > param->overdcvolt ) {
		gIsoData[chl].statu.bits.v1Over = 1;
	}else {
		gIsoData[chl].statu.bits.v1Over = 0;
	}

	if ( gIsoData[chl].vdc3 > param->overdcvolt ) {
		gIsoData[chl].statu.bits.v3Over = 1;
	}else {
		gIsoData[chl].statu.bits.v3Over = 0;
	}
}


extern void TskIso_CheckJyRes(uint8 chl);
#define MAX_DISO_SIZE  100
uint8 TskIso_ReadAndDealData(void) 
{
	uint8 msg;
	uint8 tmplen = 0;
	uint8 cnt = 0;
	int32 rlen = 0;
	int32 len = 0;
	uint8 *ptr;
	uint8 sbuf[10] = {0};
  static uint8 errCnt = 0;

  uint8 regNum = READ_REG_NUM;
  uint8 rbuf[MAX_DISO_SIZE] = {0};


	
	Uart_ClearReadBuf(gIsoUartHandle);
  len = Modbus_ReadPkg(sbuf,MODBUS_ISO_ADR,MODBUS_ISO_START_REG,regNum);
	Uart_Write(gIsoUartHandle,sbuf,len);
  Delay10Ms(5);

	 cnt = 0;
	 len = 0;
	 ptr = &rbuf[0];
	 tmplen = MAX_DISO_SIZE;
   do {
		 rlen = Uart_Read(gIsoUartHandle,ptr,MAX_DISO_SIZE-len);
		 if( rlen > 0 ) {
			ptr += rlen;
		  len += rlen;
			if( len >= MAX_DISO_SIZE) {
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
	}while(cnt < 20 );
	

	rlen = Modbus_CheckReadPkg(rbuf,MODBUS_ISO_ADR,MODBUS_CMD_03,READ_REG_NUM,len);
	if( rlen > 5 ) {
		
		
		ISO_PROTODA_ST *pdata = (ISO_PROTODA_ST *)&rbuf[3];
		
		DEV_ISO_TYPE *ptrIso0 = TskIso_GetDataPtr(0);
		DEV_ISO_TYPE *ptrIso1 = TskIso_GetDataPtr(1);
		/*也可单独赋值*/
		 /* 说明：程序中Vdc1为接触器内侧电压，实际接线第一路为外侧电压
			*             Vdc3为接触器外侧电压，实际接线第三路为内侧电压
		  */
		ptrIso0->vdc1 =  Common_Change2Byte((uint16 *)&pdata->vdc3);  //直流接触器内侧 VDC1
		ptrIso0->vdc3 =  Common_Change2Byte((uint16 *)&pdata->vdc1);  //直流接触器外侧 VDC3
		
		ptrIso1->vdc1=   Common_Change2Byte((uint16 *)&pdata->vdc4);
		ptrIso1->vdc3 =  Common_Change2Byte((uint16 *)&pdata->vdc2); 
		
		ptrIso0->res1pos = Common_Change2Byte((uint16 *)&pdata->res1pos);
		ptrIso0->res1neg = Common_Change2Byte((uint16 *)&pdata->res1neg);
		
		ptrIso1->res1pos = Common_Change2Byte((uint16 *)&pdata->res2pos);
		ptrIso1->res1neg = Common_Change2Byte((uint16 *)&pdata->res2neg);
		
		DEV_ACMETER_TYPE *pMet = TskAc_GetMeterDataPtr(0);
		pMet->Uab = Common_Change2Byte((uint16 *)&pdata->uab);
		pMet->Ubc = Common_Change2Byte((uint16 *)&pdata->ubc);
		pMet->Uca = Common_Change2Byte((uint16 *)&pdata->uac);
		
		if( (1 == gCtrl[0].fg) && ((GetSystemTick() - gCtrl[0].starttick > gCtrl[0].waittm) ) ) {
			 gCtrl[0].fg = 0;
			 TskIso_CheckJyRes(0);
		   /*发送绝缘检测完成消息*/
			 gCtrl[0].starttick = GetSystemTick();
			 msg = 0; /*一路绝缘检测*/
			 SendMsgWithNByte(MSG_ISO_FINISH,1,&msg,APP_TASK_AGUNMAINCTRL_PRIO);
		}
		if( (1 == gCtrl[1].fg) && ((GetSystemTick() - gCtrl[1].starttick > gCtrl[1].waittm) ) ) {
			 gCtrl[1].fg = 0;
			 TskIso_CheckJyRes(1);
			 gCtrl[1].starttick = GetSystemTick();
		   /*发送绝缘检测完成消息*/
			 msg = 1; /*二路绝缘检测*/
			 SendMsgWithNByte(MSG_ISO_FINISH,1,&msg,APP_TASK_BGUNMAINCTRL_PRIO);
		}
		
		gIsoData[0].statu.bits.commerr = 0;
		gIsoData[1].statu.bits.commerr = 0;
		
		TskIso_CheckErrStatu(0);	
		TskIso_CheckErrStatu(1);	

		errCnt = 0;
	}else {
		if( errCnt++ > 100 ) {
			gIsoData[0].statu.bits.commerr = 1;
			gIsoData[1].statu.bits.commerr = 1;
		}
	}

//	DebugInfoWithPx(CHK_MOD,"A绝缘状态",gIsoData[0].statu.word);
//	DebugInfoWithPx(CHK_MOD,"B绝缘状态",gIsoData[1].statu.word);
	
	return 0;
}

void init_data(void)
{
	memset(&gIsoData,0,sizeof(DEV_ISO_TYPE));
	memset(&gCtrl,0,2*sizeof(CTRL_PARA_ST));
	gCtrl[0].waittm = 1000; /*5s*/
	gCtrl[1].waittm = 1000; /*5s*/
	
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
//			do {
//				temp = TskIso_CtrlOff(msg->MsgData[0]);
//				if(cnt++ > 2 ) {
//					break;
//				}
//			}while( 0 == temp );
			break;

		default:
			break;
	}
}

/*绝缘监测单元任务*/

#ifndef DC_AC_ISO_REALCHECK
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
		
		CheckACMeterDataErr();
		  
		Delay10Ms(10);
	 }
}

#endif

#else
DEV_ISO_TYPE gIsoData[DEF_MAX_GUN_NO];


DEV_ISO_TYPE * TskIso_GetDataPtr(uint8 gunNo)
{
	 return &gIsoData[gunNo%DEF_MAX_GUN_NO];
}


#endif

#endif
