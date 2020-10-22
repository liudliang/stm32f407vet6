/**
  ******************************************************************************
  * @file    TaskBmsComm.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#include "ChgData.h"
//#include "TaskMainCtrl.h"
#include "TaskBmsComm.h"
#include "GbtBmsProto.h"
#include "PutianBms.h"
#include "bms.h"
#include "message.h"
#include "main.h"
#include "BmsCan.h"
#include "wdg.h"

#define GUNNO 0
#define COMMERR_TIMES  (10*200)  /*6S*/

#define _MAX_RCV_BUF 100
static uint8 gRcvbuf[_MAX_RCV_BUF] = { 0 };

extern void Init_CAN(void);
extern void Bms_InitData(void);

PROTO_ST gProto = { 0 };

PROTO_ST *TskBms_GetProtoPtr(void)
{
	if( (gProto.init != NULL) &&  (PROTO_GBT == gProto.protoNo) ) {
		return &gProto;
	}
	return NULL;
}



void TskBms_ProtoInit(uint8 protoNo)
{
	switch(protoNo){
		case PROTO_GBT:
			gProto.protoNo = PROTO_GBT;
			gProto.init = Gbt_Init;
			gProto.rcvanddeal = Gbt_RcvDealFrame;
			gProto.criclereport = Gbt_CricleReport;
			gProto.ctrlreport = Gbt_CtrlCricle;
			gProto.singreport = Gbt_ReportSingPkg;
			break;
	  case PROTO_PT:
			gProto.protoNo = PROTO_PT;
			gProto.init = Putian_Init;
			gProto.rcvanddeal = Putian_RcvDealFrame;
			gProto.criclereport = Putian_CricleReport;
			gProto.ctrlreport = Putian_CtrlCricle;
			gProto.singreport = Putian_ReportSingPkg;
			break;
		default:
			gProto.protoNo = PROTO_GBT;
			gProto.init = Gbt_Init;
			gProto.rcvanddeal = Gbt_RcvDealFrame;      //接收BMS数据
			gProto.criclereport = Gbt_CricleReport;   //轮休发送已开启的命令
			gProto.ctrlreport = Gbt_CtrlCricle;       //设置命令开启与关闭
			gProto.singreport = Gbt_ReportSingPkg;    //单独发送某条命令
			break;
	}
	
	gProto.init();
	
}

extern uint8 Putian_ReportSingPkg(uint8 gunNo,uint8 pf,uint8 spn);
void TaskBmsComm(void *p_arg)
{ 
	uint8 rtn;
	uint16 errCnt;
	GSEND_INFO ctrlinf;
	PDU_HEAD_STRUCT *ptrPF = NULL;
	
	Init_CAN();
	Bms_InitData();
  
	TskBms_ProtoInit(PROTO_GBT);
	
	Message_QueueCreat(APP_TASK_BMS_PRIO);
	CHARGE_TYPE *pBms = ChgData_GetRunDataPtr();
 
	memset(&gRcvbuf,0,_MAX_RCV_BUF);
	errCnt = 0;

	while(1) {

		#ifndef BMS_USE_TIMER
			gProto.criclereport(GUNNO);
		#endif
		
		rtn = gProto.rcvanddeal(0,&ctrlinf,gRcvbuf);
		if( rtn == GBT_RTN_SUCESS ) {
			errCnt = 0;
	//	  Wdg_feeddog();
			pBms->bms->err.stu.bits.commerr = 0;
		}else if(GBT_RTN_OTHER == rtn ) { 
			ptrPF = (PDU_HEAD_STRUCT *)(&gRcvbuf);
		  if ( ptrPF->sbit.PS == PUTIAN_CHARGER_ADDR \
			 || ptrPF->sbit.SA == BMS_ADDR) { /*普天的桩地址为0xE5*/
			 TskBms_ProtoInit(PROTO_PT);
			 Putian_ReportSingPkg(GUNNO,CRM_CODE,0);
			}	
		}else if ( GBT_RTN_ERRDATA == rtn ) {
			if( errCnt++ > 2 * COMMERR_TIMES ) {
				pBms->bms->err.stu.bits.commerr = 1;
			}
		}else{
			if( errCnt++ > COMMERR_TIMES ) {
				pBms->bms->err.stu.bits.commerr = 1;
				Delay5Ms(1);
			 }else if( (errCnt % 12 == 0 )&& ( GET_WORKSTEP() >= STEP_OLD_GBT_SUB )) {
				 if(gProto.protoNo != PROTO_PT ) {
					 if( GET_WORKSTEP() < STEP_CHARGEING ) {
							Putian_ReportSingPkg(GUNNO,CRM_CODE,0);
					 }
				 }
			}
		}
		
		Delay5Ms(2);
	 }
}




