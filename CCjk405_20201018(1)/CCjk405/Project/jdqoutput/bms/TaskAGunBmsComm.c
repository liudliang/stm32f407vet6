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



static GSEND_INFO gCtrlinf[DEF_MAX_GUN_NO];
extern uint8 Putian_ReportSingPkg(uint8 gunNo,uint8 pf,uint8 spn);

/***************************************************************
枪的接口和数据结构变量初始化********************/
void TaskGunInit(uint8 gun_no)
{
		if(gun_no == AGUN_NO)
			Init_CAN(CAN_1);
		else
			Init_CAN(CAN_2);
			
		Bms_InitData(gun_no);		

}

void GunBMSRecvProc(uint8 gunNO )
{
	uint8 rtn = GBT_RTN_SUCESS;
	uint16 errCnt = 0;
	uint8 rcvbuf[32];
	PDU_HEAD_STRUCT *ptrPF = NULL;	

	/*必须在BmsCommUnit_ProtoInit()后*/
	PROTO_ST *ProtoPtr_GUN = BmsCommUnit_GetProtoPtr(gunNO);
		
	//	Message_QueueCreat(APP_TASK_BMS_PRIO);
	CHARGE_TYPE *pBms_GUN = ChgData_GetRunDataPtr(gunNO);
	 
	DEV_GUN_TYPE *pGunDataPtr_GUN = Check_GetGunDataPtr(gunNO);
	
	rtn = ProtoPtr_GUN->rcvanddeal(gunNO,&gCtrlinf[gunNO],rcvbuf);

	if( rtn == GBT_RTN_SUCESS ) {
//			ScommDebugPrintStrWithPi("GunBMSRecvProc ok errcnt", errCnt);
			errCnt = 0;
//		  Wdg_feeddog();
			pBms_GUN->bms->err.stu.bits.commerr = 0;
	}else if(GBT_RTN_OTHER == rtn ) { 
			ptrPF = (PDU_HEAD_STRUCT *)(&rcvbuf[0]);
		 if ( ptrPF->sbit.PS == PUTIAN_CHARGER_ADDR \
			 && ptrPF->sbit.SA == BMS_ADDR) { /*普天的桩地址为0xE5*/
			 BmsCommUnit_ProtoInit(AGUN_NO,PROTO_PT);
			 Putian_ReportSingPkg(AGUN_NO,CRM_CODE,0);
		}	
	}else if ( GBT_RTN_ERRDATA == rtn ) {
			if( errCnt > 2* COMMERR_TIMES ) {
				pBms_GUN->bms->err.stu.bits.commerr = 1;
//				ScommDebugPrintStrWithPi("GunBMSRecvProc err errcnt", errCnt);
				errCnt = 0;				
		}
	}else{
		if( errCnt > COMMERR_TIMES ) {
//			ScommDebugPrintStrWithPi("GunBMSRecvProc timeout errcnt", errCnt);
				pBms_GUN->bms->err.stu.bits.commerr = 1;
				errCnt = 0;
				
//				Delay5Ms(1);
		}else if( (errCnt % 12 == 0 )&& ( GET_WORKSTEP(AGUN_NO) >= STEP_OLD_GBT_SUB )) {
			if(ProtoPtr_GUN->protoNo != PROTO_PT ) {
				if( GET_WORKSTEP(AGUN_NO) < STEP_CHARGEING ) {
							Putian_ReportSingPkg(AGUN_NO,CRM_CODE,0);
				}
			}
		}

		errCnt++;
	}

}

void TaskBMSRecvProc()
{	
//	if(GetWorkStatus(AGUN_NO) > STEP_IDEL )
	if(Check_GetGunDataPtr(AGUN_NO)->statu.bits.cc1stu==CC1_4V)
		GunBMSRecvProc(AGUN_NO);

	
//	if(GetWorkStatus(BGUN_NO) > STEP_IDEL )
//	if(Check_GetGunDataPtr(BGUN_NO)->statu.bits.cc1stu==CC1_4V)
//		GunBMSRecvProc(BGUN_NO);


}

CTRL_STEP  gunACtrl; /*记得初始化*/
#ifdef A_B_GUN_TOGETER
CTRL_STEP  gunBCtrl; /*记得初始化*/
#endif

void TaskAGunBmsComm(void *p_arg)
{ 
	uint8 rtn;
	uint16 errCnt;
//	uint8 rcvbuf[32];
	uint8 gunNo = AGUN_NO;
	PDU_HEAD_STRUCT *ptrPF = NULL;	
	
	errCnt = 0;
	
	Delay10Ms(300);  //20201115addzyf
	TaskGunInit(AGUN_NO);
//	TaskGunInit(BGUN_NO);

	memset(&gunACtrl,0,sizeof(CTRL_STEP));
	gunACtrl.gunNo = AGUN_NO;
	
	/*必须在BmsCommUnit_ProtoInit()后*/
	PROTO_ST *ProtoPtr_AGUN = BmsCommUnit_GetProtoPtr(AGUN_NO);
		
	//	Message_QueueCreat(APP_TASK_BMS_PRIO);
	CHARGE_TYPE *pBms_AGUN = ChgData_GetRunDataPtr(AGUN_NO);
	 
	DEV_GUN_TYPE *pGunDataPtr_AGUN = Check_GetGunDataPtr(AGUN_NO);


#ifdef A_B_GUN_TOGETER

//	memset(&gunBCtrl,0,sizeof(CTRL_STEP));
//	gunBCtrl.gunNo = BGUN_NO;

//	
//	/*必须在BmsCommUnit_ProtoInit()后*/
//	PROTO_ST *ProtoPtr_BGUN = BmsCommUnit_GetProtoPtr(BGUN_NO);
//		
//	//	Message_QueueCreat(APP_TASK_BMS_PRIO);
//	CHARGE_TYPE *pBms_BGUN = ChgData_GetRunDataPtr(BGUN_NO);
//	 
//	DEV_GUN_TYPE *pGunDataPtr_BGUN = Check_GetGunDataPtr(BGUN_NO);

#endif
	
#ifdef BMS_USE_TIMER
	Message_QueueCreat(APP_TASK_AGUNBMS_PRIO);
#endif

	memset(gCtrlinf,0,sizeof(gCtrlinf));
//	memset(&rcvbuf,0,sizeof(rcvbuf));

	while(1) 
	{
		
		TaskRunTimePrint("TaskAGunBmsComm begin", OSPrioCur);

#if 1		
		TaskMain_DealMsg(); //消息处理		
		
		
#ifdef GUN_BMS_CTRL
		TaskAGunMainCtrl_proc(&gunACtrl, ProtoPtr_AGUN);
#endif
		
#ifdef GUN_BMS_CTRL
//		TaskBGunMainCtrl_proc(&gunBCtrl, ProtoPtr_BGUN);
#endif

#endif

		TaskRunTimePrint("TaskAGunBmsComm end", OSPrioCur);
	

		Delay10Ms(APP_TASK_AGUNBMS_DELAY); //不能太快, 太快导致发送消息太多，有些重要消息没发出去
	 

	 }
}




