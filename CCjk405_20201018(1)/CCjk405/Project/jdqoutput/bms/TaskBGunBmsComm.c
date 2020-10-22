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
#include "GbtBmsProto.h"
#include "PutianBms.h"
#include "BmsCan.h"
#include "wdg.h"
#include "BmsCommUnit.h"
#include "Gpio.h"




extern void Init_CAN(uint8 CanNo);
extern void Bms_InitData(uint8 gunNo);



//static GSEND_INFO gCtrlinf;
extern uint8 Putian_ReportSingPkg(uint8 gunNo,uint8 pf,uint8 spn);

#ifndef A_B_GUN_TOGETER
CTRL_STEP  gunBCtrl; /*记得初始化*/

void TaskBGunBmsComm(void *p_arg)
{ 
	uint8 rtn;
	uint16 errCnt;
	uint8 rcvbuf[32];
	uint8 gunNo = BGUN_NO;
	PDU_HEAD_STRUCT *ptrPF = NULL;
	
	
	errCnt = 0;
	Init_CAN(CAN_2);
	Bms_InitData(gunNo);
	
	BmsCommUnit_ProtoInit(gunNo,PROTO_GBT);
	/*必须在BmsCommUnit_ProtoInit()*/
	PROTO_ST *ProtoPtr = BmsCommUnit_GetProtoPtr(gunNo);
	
	CHARGE_TYPE *pBms = ChgData_GetRunDataPtr(gunNo);
	DEV_GUN_TYPE *pGunDataPtr = Check_GetGunDataPtr(gunNo);
 
//	memset(&gCtrlinf,0,sizeof(GSEND_INFO));
//	memset(&rcvbuf,0,sizeof(rcvbuf));
	
	memset(&gunBCtrl,0,sizeof(CTRL_STEP));
	gunBCtrl.gunNo = BGUN_NO;
	
#ifdef BMS_USE_TIMER
	Message_QueueCreat(APP_TASK_BGUNBMS_PRIO);
#endif

	while(1) 
	{

		TaskRunTimePrint("TaskBGunBmsComm begin", OSPrioCur);
		
#ifndef BMS_USE_TIMER
		if(CC1_4V == pGunDataPtr->statu.bits.cc1stu)	// patli 20191206 插枪后才进行下面处理
		{
			
			ProtoPtr->criclereport(gunNo);
	
			rtn = ProtoPtr->rcvanddeal(gunNo,&gCtrlinf,rcvbuf);
			if( rtn == GBT_RTN_SUCESS ) {
				errCnt = 0;
				pBms->bms->err.stu.bits.commerr = 0;
			}else if(GBT_RTN_OTHER == rtn ) { 
				ptrPF = (PDU_HEAD_STRUCT *)(&rcvbuf[0]);
			  if ( ptrPF->sbit.PS == PUTIAN_CHARGER_ADDR \
				 && ptrPF->sbit.SA == BMS_ADDR) { /*普天的桩地址为0xE5*/
				 BmsCommUnit_ProtoInit(gunNo,PROTO_PT);
				 Putian_ReportSingPkg(gunNo,CRM_CODE,0);
				}	
			}else if ( GBT_RTN_ERRDATA == rtn ) {
				if( errCnt++ > 2 * COMMERR_TIMES ) {
					pBms->bms->err.stu.bits.commerr = 1;
				}
			}else{
				if( errCnt++ > COMMERR_TIMES ) {
					pBms->bms->err.stu.bits.commerr = 1;
					Delay5Ms(1);
				 }else if( (errCnt % 12 == 0 )&& ( GET_WORKSTEP(gunNo) >= STEP_OLD_GBT_SUB )) {
					 if(ProtoPtr->protoNo != PROTO_PT ) {
						 if( GET_WORKSTEP(gunNo) < STEP_CHARGEING ) {
								Putian_ReportSingPkg(gunNo,CRM_CODE,0);
						 }
					 }
				}
			}
		}
#endif	
		
		TaskRunTimePrint("TaskBGunBmsComm end", OSPrioCur);

		
#ifdef GUN_BMS_CTRL
		TaskBGunMainCtrl_proc(&gunBCtrl, ProtoPtr);
#endif	
		
#ifndef BMS_USE_TIMER
//			Delay5Ms(2);
			Delay10Ms(5);
#else
//			Delay10Ms(5);
			Delay10Ms(10);
#endif



	 }
}


#endif


