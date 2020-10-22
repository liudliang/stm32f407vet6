/**
  ******************************************************************************
  * @file    TaskWbIsoCheck.c
  * @author  
  * @version v1.0
  * @date   
  * @brief   
  ******************************************************************************
	*/
#include "TaskIsoCheck.h"

#if (ISO_TYPE ==  WBISO_SINGLE) 

#include "common.h"
#include "crc.h"
#include "uart.h"
#include "TaskMainCtrl.h"
#include "ModbusMaster.h"
#include "message.h"
#include "ChgData.h"
#include "common.h"
#include "wdg.h"
#include "Adebug.h"

#ifdef ISO_ON

typedef struct
{
	uint32 starttick;
	uint8  fg;
	uint8  msgfg;
	uint8  stopfg;
	uint16 waittm;
}CTRL_PARA_ST;

DEV_ISO_TYPE gIsoData[DEF_MAX_GUN_NO];


DEV_ISO_TYPE * TskIso_GetDataPtr(uint8 gunNo)
{
	 return &gIsoData[gunNo%DEF_MAX_GUN_NO];
}

const SEQ_ADR_ST conSeqAdr[] = {\
	{0,2,APP_TASK_AGUNMAINCTRL_PRIO},
	{1,3,APP_TASK_BGUNMAINCTRL_PRIO},
};



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
#define CTRON_CMD   0x20

/*启动绝缘检测*/
uint8 TskIso_CtrlOn(uint8 chl)
{
	 uint8 cnt,len;
	 int32 rlen;
	 uint8 *ptr;
	 uint8 tmplen;
	 uint8 buf[MAX_ISO_BUF] = {0};
   uint8 ctrval = 2;
   chl %= DEF_MAX_GUN_NO;

   
   DebugInfoWithPi(CHK_MOD,"启动WB绝缘检测",chl);
	 
	 DEV_ISO_TYPE *ptrIso = TskIso_GetDataPtr(chl);
   ptr = buf;
	 
   ptrIso->res1neg = 9999;
   ptrIso->res1pos = 9999;
   ptrIso->statu.bits.R1negErr = 0;
   ptrIso->statu.bits.R1posErr = 0;
	 ptrIso->res2neg = 9999;
   ptrIso->res2pos = 9999;
   ptrIso->statu.bits.R2negErr = 0;
   ptrIso->statu.bits.R2posErr = 0;
	 
   len = Modbus_SingleRegPkg(buf,conSeqAdr[chl].u8Adr,CTRON_CMD,MODBUS_ISOCTRL_REG,ctrval);
	 
#ifndef STM32F4XX
   uint8 err;
   OSSemPend(gUartCommLock, 0, &err);
#endif

   Uart_ClearReadBuf(gIsoUartHandle);

#ifndef UART_WRITE_NO_QUEUE		 
	Uart_Write(gIsoUartHandle,buf,len);
#else
	Uart_Send_data(ISO_COM,buf,len);
#endif

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
		 gCtrl[chl].fg = 1;
		 gCtrl[chl].stopfg = 0;
		 gCtrl[chl].msgfg = 1;
		 gCtrl[chl].starttick = GetSystemTick();
		 ptrIso->statu.bits.commerr = 0;
		 return 1;
	}
	
	return 0;
}


/*关闭绝缘检测*/
uint8 TskIso_CtrlOff(uint8 chl)
{
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




extern PARAM_OPER_TYPE gRunParam;
uint8 TskWbIso_ReadAndDealData(uint8 seqNo) 
{
	
	uint8 tmplen = 0;
	uint8 cnt = 0;
	int32 rlen = 0;
	int32 len = 0;
	uint8 *ptr;
	uint8 msg[4];
	uint8 sbuf[10] = {0};
  static uint8 errCnt[DEF_MAX_GUN_NO] = {0};
	const SEQ_ADR_ST *ptrSeqAdr = &conSeqAdr[seqNo];

  uint8 regnum = READ_REG_NUM;
  uint8 rbuf[MAX_RCV_SIZE] = {0};

#ifndef STM32F4XX
	uint8 err;
  OSSemPend(gUartCommLock, 0, &err);
#endif
	
  Uart_ClearReadBuf(gIsoUartHandle); 

  len = Modbus_ReadPkg(sbuf,ptrSeqAdr->u8Adr,MODBUS_ISO_START_REG,regnum);
#ifndef UART_WRITE_NO_QUEUE		 
  	Uart_Write(gIsoUartHandle,sbuf,len);
#else
	Uart_Send_data(ISO_COM,sbuf,len);
#endif

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
		if( errCnt[ptrSeqAdr->u8Seq]++ > 100 ) {
			gIsoData[ptrSeqAdr->u8Seq].statu.bits.commerr = 1;
		}
		return 0;
	}
	//pRunData = (CHARGE_TYPE *)TskMainGetRunDataPtr();
	rlen = Modbus_CheckReadPkg(rbuf,ptrSeqAdr->u8Adr,MODBUS_CMD_03,READ_REG_NUM,len);
	if( (rlen > 5) && (rbuf[0] == ptrSeqAdr->u8Adr)) {
		errCnt[ptrSeqAdr->u8Seq] = 0;

		/*也可单独赋值*/
		gIsoData[ptrSeqAdr->u8Seq].vdc1 =  Common_Change2Byte((uint16 *)&rbuf[5]);   //直流接触器内侧 
		
		gIsoData[ptrSeqAdr->u8Seq].vdc3 =  Common_Change2Byte((uint16 *)&rbuf[3]);   //直流接触器外侧 
		
		gIsoData[ptrSeqAdr->u8Seq].res1pos = Common_Change2Byte((uint16 *)&rbuf[9]);
		gIsoData[ptrSeqAdr->u8Seq].res1neg = Common_Change2Byte((uint16 *)&rbuf[11]);
		
		if( (1 == gCtrl[ptrSeqAdr->u8Seq].fg) && (GetSystemTick() - gCtrl[ptrSeqAdr->u8Seq].starttick > 2*TIM_2S) ) { //启动检测2S后
			 gCtrl[ptrSeqAdr->u8Seq].fg = 0;
			 gCtrl[ptrSeqAdr->u8Seq].stopfg = 1;
			 cnt = 0;
       gCtrl[ptrSeqAdr->u8Seq].starttick = GetSystemTick();		
		}

		if( (1 == gCtrl[ptrSeqAdr->u8Seq].stopfg) && (gCtrl[ptrSeqAdr->u8Seq].fg == 0) && (GetSystemTick() - gCtrl[ptrSeqAdr->u8Seq].starttick >  TIM_1S )) { //关闭检测1S后，总共3S
			gCtrl[ptrSeqAdr->u8Seq].stopfg = 0;
			TskIso_CheckJyRes(ptrSeqAdr->u8Seq);
		   /*发送绝缘检测完成消息*/
			 if( 1 == gCtrl[ptrSeqAdr->u8Seq].msgfg ) {
					 msg[0] = ptrSeqAdr->u8Seq;
				   msg[1] = 1;
					gCtrl[ptrSeqAdr->u8Seq].msgfg = 0;
					SendMsgWithNByte(MSG_ISO_FINISH,2,&msg[0],ptrSeqAdr->prio);
			}
		}
		gIsoData[ptrSeqAdr->u8Seq].statu.bits.commerr = 0;
	}else {
		if( errCnt[ptrSeqAdr->u8Seq]++ > 100 ) {
			gIsoData[ptrSeqAdr->u8Seq].statu.bits.commerr = 1;
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
#define WBISO_NUM DEF_MAX_GUN_NO
/*绝缘监测单元任务*/
void Task_IsoCheck(void *p_arg)
{
	 uint8 u8IsoSeq = 0;
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
		
		 TskWbIso_ReadAndDealData(u8IsoSeq);
		 u8IsoSeq++;
		 
		 PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
		 u8IsoSeq %= devparaPtr->gunnum;//WBISO_NUM;
		
//		 Wdg_feeddog();
		 Delay10Ms(10);
	 }
}
#else
DEV_ISO_TYPE gIsoData[DEF_MAX_GUN_NO];


DEV_ISO_TYPE * TskIso_GetDataPtr(uint8 gunNo)
{
	 return &gIsoData[gunNo%DEF_MAX_GUN_NO];
}


#endif

#endif




