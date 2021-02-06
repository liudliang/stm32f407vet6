/**
  ******************************************************************************
  * @file    MainCtrlUnit.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/

//#include "M_Global.h"
#include "TaskRealCheck.h"
#include "GbtBmsProto.h"
#include "BmsCommUnit.h"

#include "RelayOut.h"
#include "string.h"
#include "main.h"
#include "Bms.h"
#include "TaskBmsComm.h"
#include "BillingUnit.h"
#include "PutianBms.h"

//#include "CdModel.h"
#include "TaskAcMeter.h"

#include "RecordFat.h"
#include "message.h"

#include "TaskMainCtrl.h"
#include "TaskBackComm.h"

#include "gpio.h"
#include "Screen.h"
#include "PowerSplitt.h"

#include "Adc_Calc.h"
#include "HeLiBmsProto.h"


extern RD_PARA_ST gSSTE2ROMTabl;
extern RD_PARA_ST gTempE2ROMTabl;
extern RD_PARA_ST gFullE2ROMTabl;

extern uint8 CdModData_GetModCommErr(uint8 grpNo);
extern uint8 CdModData_CheckOnOffStu(uint8 grpNo,uint8 stu);

PARAM_OPER_TYPE *  gPtrParam = NULL;
CHARGE_TYPE * gPtrRunData[DEF_MAX_GUN_NO] = { NULL };

const uint8 conDefusrpin[] = {0x11,0x11,0x11}; /*默认卡密码*/

uint8 start_charge_try_num = 0;	   //充电启动次数	

extern DEV_BMS_TYPE * Bms_GetBmsDataPtr(uint8 gunNo);
extern DEV_ISO_TYPE * TskIso_GetDataPtr(uint8 gunNo);
extern DEV_GUN_TYPE *Check_GetGunDataPtr(uint8 gunNo);
extern DEV_RELAY_TYPE * Relay_GetRelayDataPtr(uint8 no);
extern DEV_INPUT_TYPE *Check_GetInputDataptr(uint8 gunNo);
extern DEV_METER_TYPE *TskDc_GetMeterDataPtr(uint8 gunNo);
extern DEV_LOGIC_TYPE * ChgData_GetLogicDataPtr(uint8 gunNo);
extern DEV_ACMETER_TYPE *TskAc_GetMeterDataPtr(uint8 gunNo);

//重新启动充电
uint8 TaskMian_TryStartCharge(CTRL_STEP *ptrCtrl, START_PARAM *startpara);


uint8 TskMain_GetWorkStep(uint8 gunNo)
{
	return gPtrRunData[gunNo]->logic->workstep;
}

void TskMain_SetStoping(uint8 gunNo,uint8 step,uint8 fg)
{
	gPtrRunData[gunNo]->logic->workstep = step;
	if( UNLOCKED_END == fg ) {
		RelayOut_Locked(gunNo,CTRL_OFF); 
	}
}
void TskMain_SetWorkStep(uint8 gunNo,uint8 step)
{
	gPtrRunData[gunNo]->logic->workstep = step;
}


/*数据指针挂接*/
void MainCtrl_Init(void)
{
	 RelayOut_Init();
	 /*A枪数据指针挂接*/
	 gPtrParam = ChgData_GetRunParamPtr();
	 gPtrRunData[AGUN_NO] = ChgData_GetRunDataPtr(AGUN_NO);
	
	 gPtrRunData[AGUN_NO]->bms = Bms_GetBmsDataPtr(AGUN_NO);
	 gPtrRunData[AGUN_NO]->iso = TskIso_GetDataPtr(AGUN_NO);
	 gPtrRunData[AGUN_NO]->input = Check_GetInputDataptr(AGUN_NO);
	 gPtrRunData[AGUN_NO]->logic = ChgData_GetLogicDataPtr(AGUN_NO);
	 gPtrRunData[AGUN_NO]->meter = TskDc_GetMeterDataPtr(AGUN_NO);
	 gPtrRunData[AGUN_NO]->gun = Check_GetGunDataPtr(AGUN_NO);
	 gPtrRunData[AGUN_NO]->startparam = ChgData_GetStartParaPtr(AGUN_NO);
	 gPtrRunData[AGUN_NO]->bill = Bill_GetBillData(AGUN_NO);
	 gPtrRunData[AGUN_NO]->dlmod =  CdModData_GetDataPtr(AGUN_NO);
	 gPtrRunData[AGUN_NO]->relay = Relay_GetRelayDataPtr(AGUN_NO);
	 gPtrRunData[AGUN_NO]->AcMet = TskAc_GetMeterDataPtr(AGUN_NO);
	 
	 memset(gPtrRunData[AGUN_NO]->logic,0,sizeof(DEV_LOGIC_TYPE));
	 memcpy(gPtrRunData[AGUN_NO]->logic->usrpin,conDefusrpin,sizeof(gPtrRunData[AGUN_NO]->logic->usrpin));
	 TskMain_SetWorkStep(AGUN_NO,STEP_IDEL);
	
	 /*B枪数据挂接*/
	 gPtrRunData[BGUN_NO] = ChgData_GetRunDataPtr(BGUN_NO);
	 gPtrRunData[BGUN_NO]->bms = Bms_GetBmsDataPtr(BGUN_NO);
	 gPtrRunData[BGUN_NO]->iso = TskIso_GetDataPtr(BGUN_NO);
	 gPtrRunData[BGUN_NO]->input = Check_GetInputDataptr(BGUN_NO);
	 gPtrRunData[BGUN_NO]->logic = ChgData_GetLogicDataPtr(BGUN_NO);
	 gPtrRunData[BGUN_NO]->meter = TskDc_GetMeterDataPtr(BGUN_NO);
	 gPtrRunData[BGUN_NO]->gun = Check_GetGunDataPtr(BGUN_NO);
	 gPtrRunData[BGUN_NO]->startparam = ChgData_GetStartParaPtr(BGUN_NO);
	 gPtrRunData[BGUN_NO]->bill = Bill_GetBillData(BGUN_NO);
	 gPtrRunData[BGUN_NO]->dlmod =  CdModData_GetDataPtr(BGUN_NO);
	 gPtrRunData[BGUN_NO]->relay = Relay_GetRelayDataPtr(BGUN_NO);
	 gPtrRunData[BGUN_NO]->AcMet = TskAc_GetMeterDataPtr(BGUN_NO);
	 
	 memset(gPtrRunData[BGUN_NO]->logic,0,sizeof(DEV_LOGIC_TYPE));
	 memcpy(gPtrRunData[BGUN_NO]->logic->usrpin,conDefusrpin,sizeof(gPtrRunData[BGUN_NO]->logic->usrpin));
	 TskMain_SetWorkStep(BGUN_NO,STEP_IDEL);
	 
	 /*Bms协议初始化*/
	 BmsCommUnit_ProtoInit(AGUN_NO,PROTO_GBT);
	 BmsCommUnit_ProtoInit(BGUN_NO,PROTO_GBT);
}


uint8 TskMain_IsoCheckBefor(uint8 gunNo)
{
	
	uint8 errCnt = 0;
	if( gPtrRunData[gunNo]->iso->statu.word )  {
		if( gPtrRunData[gunNo]->iso->statu.bits.commerr ) {
			 errCnt= ECODE35_ISOCOMM;
		}
		if ( gPtrRunData[gunNo]->iso->statu.bits.R1posErr ) {
			  errCnt= ECODE36_1POSERR;
		}
		if( gPtrRunData[gunNo]->iso->statu.bits.R1negErr) {
			  errCnt= ECODE37_1NEGERR;
		}else {
			 ;
		}
	
}
  return errCnt;
}



uint8 Step_Link_Proc(uint8 gunNo)
{
	if( 0 ==gPtrRunData[gunNo]->gun->statu.bits.elockstu ) {
		if((GetSystemTick() - Bms_GetStartTimeCount(gunNo,LOCK_TM)) > SECS2TICKS(gPtrParam->elocktm)) {
				return RTN_TMOUT;
		}
		return RTN_WAIT;
	}
	Bms_SetStepErrFg(gunNo,LOCK_TM,0);
	return  RTN_TRUE;
}

/*Bcl Bcs 超时判断*/
uint8  TskMain_BCLBCS_timeout(uint8 gunNo)
{
	  BMSDATA_ST *pCar = Bms_GetBmsCarDataPtr(gunNo) ;
	  static uint8 scnt[DEF_MAX_GUN_NO][2] = {0};
	  if(GetSystemTick() - Bms_GetStartTimeCount(gunNo,BCL_TM) > SECS2TICKS(gPtrParam->bcltm) ) {
//			Bms_SetStepErrFg(gunNo,BCL_TM,1);  //20190404
			scnt[gunNo][0]++;
		}else {
			Bms_SetStepErrFg(gunNo,BCL_TM,0);
			scnt[gunNo][0] = 0;
		}
		if(GetSystemTick() - Bms_GetStartTimeCount(gunNo,BCS_TM) > SECS2TICKS(gPtrParam->bcstm) ) {
//				Bms_SetStepErrFg(gunNo,BCS_TM,1); //20190404
			  scnt[gunNo][1]++;
		}else {
			Bms_SetStepErrFg(gunNo,BCS_TM,0);
			scnt[gunNo][1] = 0;
		}
//		 if( (gPtrRunData[gunNo]->meter->current < 50) && (pCar->bcs.cursoc > 98)) {
			 
		if( scnt[gunNo][0] > 4 ) {
			 if( pCar->bcs.cursoc < 97 ) {     //20190404  针对北汽充到最后总是报BCL超时，当SOC达到98%后BCL超时直接报BMS达到电压设定值
			    Bms_SetStepErrFg(gunNo,BCL_TM,1);
			 }else {
				  Check_SetErrCode(gunNo,ECODE51_BMSTVOLT);
			 }
			 return BCL_TMOUT;
		}
		
		if( scnt[gunNo][1] > 4 ) {
			if( pCar->bcs.cursoc < 97 ) {      //20190404  针对北汽充到最后总是报BCL超时，当SOC达到98%后BCL超时直接报BMS达到电压设定值
					Bms_SetStepErrFg(gunNo,BCS_TM,1);
			}else {
				  Check_SetErrCode(gunNo,ECODE51_BMSTVOLT);
			}
			return BCS_TMOUT;
		}
		return 0;
}

void TskMain_ChargingStatuCheckLoop(uint8 gunNo)
{
	  TskMain_BCLBCS_timeout(gunNo);
}

void TskMain_ChargingChgDataLoop(uint8 gunNo)
{
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(gunNo);

	pdata->ccs.outcurr = gPtrRunData[gunNo]->meter->current;
	pdata->ccs.outvolt = gPtrRunData[gunNo]->meter->volt;//gPtrRunData[gunNo]->iso->vdc3;
	pdata->ccs.totalchgtime = Bill_GetBillData(gunNo)->chgsecs / 60;
	pdata->ccs.allowed.bits.fg = 0x01; /*允许充电*/

	gPtrRunData[gunNo]->logic->time = Bill_GetBillData(gunNo)->chgsecs;
	gPtrRunData[gunNo]->logic->money = Bill_GetBillData(gunNo)->billmoney;
	gPtrRunData[gunNo]->logic->energy = Bill_GetBillData(gunNo)->energy;

	/* 当累计充电时间超过1分钟，输出电量超过0.1kwh时，充电机所发送的统计数据报文CSD中关于累计充电时间和输出电量的字节内容均为0，因此不正确 */
	/* 修改如下 */
	pdata->csd.chgtotaltime = gPtrRunData[gunNo]->logic->time / 60;
	pdata->csd.outenergy = gPtrRunData[gunNo]->logic->energy;	
}

void TskMain_ChgDataInit(uint8 gunNo)
{
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(gunNo);
	pdata->chm.ver[0] = 0x01;
	pdata->chm.ver[1] = 0x01;
	pdata->chm.ver[2] = 0x00;
	
	/*初始化设置，后续待根据配置修改*/
	pdata->cml.maxoutvolt = gPtrParam->maxvolt;
	pdata->cml.minoutvolt = gPtrParam->minvolt;
	pdata->cml.maxoutcur  = gPtrParam->maxcurr;
	pdata->cml.minoutcur  = gPtrParam->mincurr;
	
}

uint8 TskMain_HardWareErrCheck(uint8 gunNo)
{
	 uint8 errCnt;
	 PARAM_OPER_TYPE *param = ChgData_GetRunParamPtr();
	 CHGDATA_ST *pChg = Bms_GetChgDataPtr(gunNo);
	 PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	
	 errCnt = 0;
	 if( 1 == gPtrRunData[gunNo]->iso->statu.bits.commerr ) {
		  Check_SetErrCode(gunNo,ECODE35_ISOCOMM);
		  errCnt++;
	 }else {
		 Check_ClearErrBit(gunNo,ECODE35_ISOCOMM);
	 }
	 
	 if( 1== gPtrRunData[gunNo]->AcMet->statu.bits.UDeficy ) {
		 Check_SetErrCode(gunNo,ECODE98_ACDEFICY);
		 errCnt++;
	 }else{
		 Check_ClearErrBit(gunNo,ECODE98_ACDEFICY);
	 }
	 
	 if( 1== gPtrRunData[gunNo]->AcMet->statu.bits.UabOver ) {
		 Check_SetErrCode(gunNo,ECODE96_ACVOVERR);
		 errCnt++;
	 }else{
		 Check_ClearErrBit(gunNo,ECODE96_ACVOVERR);
	 }
	 
	  if( 1== gPtrRunData[gunNo]->AcMet->statu.bits.UabUnder ) {
		 Check_SetErrCode(gunNo,ECODE97_ACUnerERR);
		 errCnt++;
	 }else{
		 Check_ClearErrBit(gunNo,ECODE97_ACUnerERR);
	 }
		
	 if( 1 == gPtrRunData[gunNo]->input->statu.bits.stop ) { /*急停*/
		 /* 充电机在充电阶段按下急停按钮后，所发送的充电机中止充电报文CST的内容为00 00 F0 F0，未标明中止原因，因此应修改为10 00 F1 F0，表示中止原因为充电机急停 */
		 /* 修改如下 */
		 pChg->cst.stoprsn.bits.fault = 1;
		 pChg->cst.faultrsn.bits.scramflt = 1;
		 Check_SetErrCode(gunNo,ECODE89_CHGJTST);
		 errCnt++; 
	 }else{
		 Check_ClearErrBit(gunNo,ECODE89_CHGJTST);
	 }


	 if(0 == gPtrRunData[gunNo]->input->statu.bits.dckmp){    //接触器粘连
			Check_SetErrCode(gunNo,ECODE13_KM60V);
			errCnt++; 
		}else{
		  Check_ClearErrBit(gunNo,ECODE13_KM60V);
	 }
	 	
	 	
	 
		if( 1 == gPtrRunData[gunNo]->meter->statu.bits.commerr ) {
			Check_SetErrCode(gunNo,ECODE93_METERCOMMERR);
			errCnt++; 
		}else{
		  Check_ClearErrBit(gunNo,ECODE93_METERCOMMERR);
	 }
		 
	 if( 1 == gPtrRunData[gunNo]->meter->statu.bits.currOver ) {
			Check_SetErrCode(gunNo,ECODE94_CURROVER);
			errCnt++; 
		}else{
			Check_ClearErrBit(gunNo,ECODE94_CURROVER);
	 }
		
	 if(1 == param->Sysparalarm.bits.opencharge) {
		 if( 1 == gPtrRunData[gunNo]->input->statu.bits.dooracs ) {
			 Check_SetErrCode(gunNo,ECODE99_DooracsERR);
			 errCnt++;
		 } else{
			 Check_ClearErrBit(gunNo,ECODE99_DooracsERR);
	   }
		 
		 if( 1 == gPtrRunData[gunNo]->input->statu.bits.dooraf){
		    Check_SetErrCode(gunNo,ECODE100_DoorafERR);
			  errCnt++;
	   }else{
		    Check_ClearErrBit(gunNo,ECODE100_DoorafERR);
	   }
	 }
	 
	 if( 1 == gPtrRunData[gunNo]->input->statu.bits.posfuse  ) {
		 Check_SetErrCode(gunNo,ECODE101_DCFAUSEERR);
		 errCnt++;
	 }else{
		  Check_ClearErrBit(gunNo,ECODE101_DCFAUSEERR);
	 }
	 
	 if (1 == param->Sysparalarm.bits.prevraysignl){   //防雷器故障
			if( 1 == gPtrRunData[gunNo]->input->statu.bits.spd  ) {
			 Check_SetErrCode(gunNo,ECODE102_SPDERR);
			 errCnt++;
			}else{
				Check_ClearErrBit(gunNo,ECODE102_SPDERR);
			}
	 }

	 if((BMS_HELI != BackCOMM->agreetype))
	 {	 
		 if (1 == gPtrRunData[gunNo]->gun->statu.bits.overtemper) {
			 Check_SetErrCode(gunNo, ECODE103_GUNOVERTEMPER);
			 errCnt++;
		 }else{
			 Check_ClearErrBit(gunNo, ECODE103_GUNOVERTEMPER);		 
		 }	 
	 }
	 
	 if (1 == gPtrRunData[gunNo]->gun->statu.bits.bcpovervolt){
		 Check_SetErrCode(gunNo, ECODE108_BCPOVERVOLT);
			errCnt++;
	 }else{
		 Check_ClearErrBit(gunNo, ECODE108_BCPOVERVOLT);
	 }
	 
	 return errCnt;
}

extern uint8 Check_BcsAndMetdata(uint8 gunNo);
extern uint8 AdcCalc_GetISOStatus(void);
				
/*充电过程中结束条件判断*/
uint8  TskMain_StopCondition(uint8 gunNo)
{  
	 static uint32 sDlayTicks[DEF_MAX_GUN_NO];
	 uint8 errCode = 0,errCnt = 0;
	 CHGDATA_ST *pChg = Bms_GetChgDataPtr(gunNo);
	 ERR_STEP_ST *pBmsErr = Bms_GetStepStPtr(gunNo);
	 BMSDATA_ST *pCar = Bms_GetBmsCarDataPtr(gunNo);
	 START_PARAM  *PtrStartPara = ChgData_GetStartParaPtr(gunNo);
	 PARAM_OPER_TYPE *param = ChgData_GetRunParamPtr();
	 REAL_BILL_DATA_ST *ptrBill = GetFeeData(gunNo);
	 CARD_INFO *PtrCard = TskCard_GetCardInfPtr();	   //patli 20190929
	 PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	
	 if( pBmsErr->stu.dword ) {
		  errCode = Bms_GetErrCode(gunNo);
		  Check_SetErrCode(gunNo,(E_ERR_CODE)(errCode%32)); /*0~31*/
		  errCnt++;
	 }
	 
//	 if( 1 == gPtrRunData[gunNo]->iso->statu.bits.commerr ) {
//		  Check_SetErrCode(gunNo,ECODE35_ISOCOMM);
//		  errCnt++;
//	 }
	 if(1 == AdcCalc_GetISOStatus())
	 {
	     Check_SetErrCode(gunNo, ECODE38_2POSERR);   /*绝缘故障 */			 
		   errCnt++;
	  }

	 
	 
	 if( 1== gPtrRunData[gunNo]->iso->statu.bits.AcDeficy ) {
		 Check_SetErrCode(gunNo,ECODE98_ACDEFICY);
		 errCnt++;
	 }
	 
	 if( 1== gPtrRunData[gunNo]->iso->statu.bits.AcVoer ) {
		 Check_SetErrCode(gunNo,ECODE96_ACVOVERR);
		 errCnt++;
	 }
	 
	 if( 1 == gPtrRunData[gunNo]->iso->statu.bits.v1Over ) {
		 Check_SetErrCode(gunNo,ECODE42_DC1OVER);
		 errCnt++; 
	 }
	 
//	  if( 1 == gPtrRunData[gunNo]->iso->statu.bits.v2Over ) {
//		 Check_SetErrCode(gunNo,ECODE43_DC2OVER);
//		 errCnt++; 
//	 }
//	 
//	  if( 1 == gPtrRunData[gunNo]->iso->statu.bits.v3Over ) {
//		 Check_SetErrCode(gunNo,ECODE44_DC3OVER);
//		 errCnt++; 
//	 }
//	 
//	  if( 1 == gPtrRunData[gunNo]->iso->statu.bits.v4Over ) {
//		 Check_SetErrCode(gunNo,ECODE45_DC4OVER);
//		 errCnt++; 
//	 }
	 
	 if( 1 < gPtrRunData[gunNo]->meter->statu.word ) {
		 if( 1 == gPtrRunData[gunNo]->meter->statu.bits.commerr ) {
				Check_SetErrCode(gunNo,ECODE93_METERCOMMERR);
				errCnt++; 
		 }
		 
		 if( 1 == gPtrRunData[gunNo]->meter->statu.bits.currOver ) {
				Check_SetErrCode(gunNo,ECODE94_CURROVER);
				errCnt++; 
		 }
	 }
	 
	 if( CC1_4V != gPtrRunData[gunNo]->gun->statu.bits.cc1stu ) {	
		   Check_SetErrCode(gunNo,ECODE18_CC1LNK);
		   errCnt++;
	 }

	 if((BMS_HELI != BackCOMM->agreetype))
	 {
		 if( 1 == gPtrRunData[gunNo]->gun->statu.bits.overtemper ) {
			 Check_SetErrCode(gunNo,ECODE32_GUNTMPOVER);
			 errCnt++;
		 }
   }
	
	 if(BMS_HELI != BackCOMM->agreetype)
	 {	 
		 if (1 == gPtrRunData[gunNo]->gun->statu.bits.bcpovervolt) {
			 Check_SetErrCode(gunNo, ECODE108_BCPOVERVOLT);
			 errCnt++;
		 }
	 }
	 
	 if( CHG_TIME_TYPE == gPtrRunData[gunNo]->logic->chgmod ) {
		 if(gPtrRunData[gunNo]->logic->time > gPtrRunData[gunNo]->logic->settime-1) {
			 pChg->cst.stoprsn.bits.condok = 1;
			 Check_SetErrCode(gunNo,ECODE80_CHGFINISH);
			 errCnt++;
		 } 
	 }
	 else if(CHG_MONEY_TYPE == gPtrRunData[gunNo]->logic->chgmod) {
		 if(gPtrRunData[gunNo]->logic->money >= gPtrRunData[gunNo]->logic->setmoney-1 ) {
			 pChg->cst.stoprsn.bits.condok = 1;
			 Check_SetErrCode(gunNo,ECODE80_CHGFINISH);
			 errCnt++;
		 }
	 }
	 else if (CHG_ENERGY_TYPE ==gPtrRunData[gunNo]->logic->chgmod) {
		 if(gPtrRunData[gunNo]->logic->energy >= gPtrRunData[gunNo]->logic->setenergy-1 ) {
			 pChg->cst.stoprsn.bits.condok = 1;
			 Check_SetErrCode(gunNo,ECODE80_CHGFINISH);
			 errCnt++;
		 }
	 }else if(CHG_AUTO_TYPE ==gPtrRunData[gunNo]->logic->chgmod) {
		 //patli 20190930
		 if( (PtrCard->cardtype != E_SUPER_CARD)&&(PtrStartPara->money < 100 )) {
			 pChg->cst.stoprsn.bits.condok = 1;
			 Check_SetErrCode(gunNo,ECODE84_NOMONEY);
			 errCnt++;
		 } 
	 }
	 else {
		 ;
	 }
	 /*充电中还剩下5毛钱时停止充电patli 20190930 超级会员卡不限余额*/
	 if(  (PtrCard->cardtype != E_SUPER_CARD)&&(ptrBill->bill.beforemoney < gPtrRunData[gunNo]->logic->money+50)  ) {
		   pChg->cst.stoprsn.bits.condok = 1;
			 Check_SetErrCode(gunNo,ECODE84_NOMONEY);
			 errCnt++;
	 }
	 
	 	if( 1 == gPtrRunData[gunNo]->input->statu.bits.stop ) { /*急停*/
		 /* 充电机在充电阶段按下急停按钮后，所发送的充电机中止充电报文CST的内容为00 00 F0 F0，未标明中止原因，因此应修改为10 00 F1 F0，表示中止原因为充电机急停 */
		 /* 修改如下 */
		 pChg->cst.stoprsn.bits.fault = 1;
		 pChg->cst.faultrsn.bits.scramflt = 1;
		 Check_SetErrCode(gunNo,ECODE89_CHGJTST);
		 errCnt++; 
	 }
	 
	 /*电流过小*/
	 if( (gPtrRunData[gunNo]->meter->current < 50) && (pCar->bcs.cursoc > 99)) {
		 if(GetSystemTick() - sDlayTicks[gunNo] > 3*TIM_1MIN ) {
			 pChg->cst.stoprsn.bits.condok = 1;
			 Check_SetErrCode(gunNo,ECODE80_CHGFINISH);
			 errCnt++;
		 }
	 }else {
		 sDlayTicks[gunNo] = GetSystemTick();
	 }
	 
	 if((pCar->bcs.cursoc > 99) && (BMS_HELI == BackCOMM->agreetype))
	 {
		 Check_SetErrCode(gunNo,HELI_ECODE49_BMS_STOP_NORMOL);
		 errCnt++;
	 }
	 
	 /*车辆端结束，放在最后判断*/
	 if( pCar->bst.stoprsn.byte & 0x3f) {
		 if(pCar->bst.stoprsn.bits.socok == 0x01) { /*Soc 达到设定值*/
			 Check_SetErrCode(gunNo,ECODE50_BMSFINISH);
			 errCnt++;
		 }
		 if (pCar->bst.stoprsn.bits.voltok == 0x01) { /*充电电压达到最大值*/
			 Check_SetErrCode(gunNo,ECODE51_BMSTVOLT);
			 errCnt++;
		 }
		 if( pCar->bst.stoprsn.bits.singvolt == 0x01) {/*单体电压达到最大值*/
			 Check_SetErrCode(gunNo,ECODE52_BMSSVOLT);
			 errCnt++;
		 }
		 if( pCar->bst.stoprsn.bits.initiative == 0x01 ) { /*收到CST ,&3f 不会被执行*/
			 Check_SetErrCode(gunNo,ECODE53_BMSCST);
			 errCnt++;
		 }
		 else {
			 ; 
		 }
	 }

	 if(BMS_HELI == BackCOMM->agreetype)
	 {	 
		 switch(pCar->bst.heli_faultrsn.byte){
			case 1:
				Check_SetErrCode(gunNo, HELI_ECODE49_BMS_STOP_NORMOL);
				errCnt++;
				break;
		 	case 2:
				Check_SetErrCode(gunNo, HELI_ECODE109_CHARG_VOL_OVER);
				errCnt++;
				break;
			case 3:
				Check_SetErrCode(gunNo, HELI_ECODE110_OVER_TEMPERATURE);
				errCnt++;
				break;
			case 4:
				Check_SetErrCode(gunNo, HELI_ECODE111_INTERLOCK);
				errCnt++;
				break;
			case 5:
				Check_SetErrCode(gunNo, HELI_ECODE112_LOWER_TEMPERATURE);
				errCnt++;
				break;
		 	case 6:
				Check_SetErrCode(gunNo, HELI_ECODE113_CELL_VOL_LOWER);
				errCnt++;
				break;
			case 7:
				Check_SetErrCode(gunNo, HELI_ECODE114_CURR_OVER);
				errCnt++;
				break;
		 	case 8:
				Check_SetErrCode(gunNo, HELI_ECODE115_BMS_FAULT);
				errCnt++;
				break;
			case 9:
				Check_SetErrCode(gunNo, HELI_ECODE116_CONNECTOR_OVER_TEMPERATURE);
				errCnt++;
				break;
		 	case 10:
				Check_SetErrCode(gunNo, HELI_ECODE117_CC2_FAULT);
				errCnt++;
				break;
			case 11:
				Check_SetErrCode(gunNo, HELI_ECODE118_CELLS_VOL_DIFF);
				errCnt++;
				break;
		 	case 12:
				Check_SetErrCode(gunNo, HELI_ECODE119_CELLS_TEMPERATURE_DIFF);
				errCnt++;
				break;
			case 13:
				Check_SetErrCode(gunNo, HELI_ECODE46_CONNECTOR_TEMPERATURE_LOSS);
				errCnt++;
				break;
		 	case 14:
		 	case 15:
				Check_SetErrCode(gunNo, HELI_ECODE47_OTHER);
				errCnt++;
				break;			
			default:
				break;				 
			 
		 }
	 }	 
	 
	 if ( pCar->bst.faultrsn.word ) {
		 if(pCar->bst.faultrsn.bits.isoflt == 0x01) {
			 Check_SetErrCode(gunNo,ECODE55_BMSISO);
			 errCnt++;
		 }
		 if (pCar->bst.faultrsn.bits.lkovtp == 0x01) {
			 Check_SetErrCode(gunNo,ECODE56_BMSLKTMP);
			 errCnt++;
		 }
		 if( pCar->bst.faultrsn.bits.parovtp == 0x01) {
			 Check_SetErrCode(gunNo,ECODE57_BMSPARTTMP);
			 errCnt++;
		 }
		 if( pCar->bst.faultrsn.bits.chglkflt == 0x01 ) { 
			 Check_SetErrCode(gunNo,ECODE58_BMSCLKERR);
			 errCnt++;
		 }
		 if( pCar->bst.faultrsn.bits.batovflt == 0x01 ) { 
			 Check_SetErrCode(gunNo,ECODE59_BMSBATTMP);
			 errCnt++;
		 }
		 if( pCar->bst.faultrsn.bits.rlyflt == 0x01 ) { 
			 Check_SetErrCode(gunNo,ECODE60_BMSKMERR);
			 errCnt++;
		 }
		 if( pCar->bst.faultrsn.bits.ckpt2flt == 0x01 ) { 
			 Check_SetErrCode(gunNo,ECODE61_BMSCHK2ERR);
			 errCnt++;
		 }
		 if( pCar->bst.faultrsn.bits.otherflt == 0x01 ) { 
			 Check_SetErrCode(gunNo,ECODE62_BMSOTHERERR);
			 errCnt++;
		 }else {
			 ; 
		 }
	 }
	 
	 if( pCar->bst.errorrsn.byte ) {
		 errCode = 0;
		 if(pCar->bst.errorrsn.bits.overcurr == 0x01 ) {
			 Check_SetErrCode(gunNo,ECODE64_BMSCURROVER);
			 errCnt++;
		 }else if ( pCar->bst.errorrsn.bits.unusualvolt == 0x01 ) {
			  Check_SetErrCode(gunNo,ECODE65_BMSVOLTERR);
			  errCnt++;
		 }else {
			 ;
		 }
	 } 
	 
	 if(pCar->bem.berrcode.dword > 0) {
		 if( 1 == pCar->bem.berrcode.bits.crmtmout ) {
			 Check_SetErrCode(gunNo,ECODE67_BMSCRMTM);
			 errCnt++;
		 } 
		 
		 if( 1 == pCar->bem.berrcode.bits.crmoktmout){
		    Check_SetErrCode(gunNo,ECODE68_BMSCRMATM);
			  errCnt++;
	   }
		 if(1 == pCar->bem.berrcode.bits.cmltmout ) {
			  Check_SetErrCode(gunNo,ECODE69_BMSCTSTM);
			  errCnt++;
		 }
		 
		  if(1 == pCar->bem.berrcode.bits.crotmout ) {
			  Check_SetErrCode(gunNo,ECODE70_BMSCROTM);
			  errCnt++;
		 }
			
		  if( 1 == pCar->bem.berrcode.bits.ccstmout ) {
			  Check_SetErrCode(gunNo,ECODE71_BMSCCSTM);
			  errCnt++;
		 }	
	 }
  
	 if(1 == param->Sysparalarm.bits.opencharge) {
		 if( 1 == gPtrRunData[gunNo]->input->statu.bits.dooracs ) {
			 Check_SetErrCode(gunNo,ECODE99_DooracsERR);
			 errCnt++;
		 } 
		 if( 1 == gPtrRunData[gunNo]->input->statu.bits.dooraf){
		    Check_SetErrCode(gunNo,ECODE100_DoorafERR);
			  errCnt++;
	   }
	 }
	 
	 if(  Check_BcsAndMetdata(gunNo) > 0 ) {
		 errCnt++;
	 }
	 return errCnt;
	 
}
/*注意：充电开始后，一定要DC接触器断开后泄放*/
uint8 TskMain_StopHardWareStep(uint8 gunNo)
{
	  uint8 rtn;
	  uint8 msg[5];
	  uint16 tmpCount;
	  PARAM_OPER_TYPE *ptrRunParam = ChgData_GetRunParamPtr();
	
	  rtn = 0;
	  msg[0] = gunNo;
		msg[1] = MOD_CMD_OFF;
		SendMsgWithNByte(MSG_MOD_ONOFF,2,&msg[0],APP_TASK_DLMOD_PRIO);
	
	  //gPtrRunData[gunNo]->dlmod[0].alarmInfo.bits.ModStu = DLMOD_ON;  /*防止意外，做的一个陷阱*/
		Delay10Ms(30);
	
		tmpCount = 1;
		while(1){
			
			if(gPtrRunData[gunNo]->meter->volt < VOLT_TRAN(60) ) {
				rtn = 0;
				break;
			}
			
			if( TRUE == CdModData_CheckOnOffStu(gunNo,DLMOD_OFF) || (tmpCount > 20 ) ) { /*800ms 以上*/
				   msg[0] = gunNo;
    			 msg[1] = MOD_CMD_OFF;
					 SendMsgWithNByte(MSG_MOD_ONOFF,2,&msg[0],APP_TASK_DLMOD_PRIO);
				   Delay10Ms(30);
					 /*确认状态后 泄放电阻投入*/
					 RelayOut_XfResOnOff(gunNo,CTRL_ON); 
					 Delay10Ms(200);
					 RelayOut_XfResOnOff(gunNo,CTRL_OFF); 
				   rtn = 0;
					 break;
			}else {
				if( (tmpCount++ % 100 ) == 0 ) {
					msg[0] = gunNo;
					msg[1] = MOD_CMD_OFF;
					SendMsgWithNByte(MSG_MOD_ONOFF,2,&msg[0],APP_TASK_DLMOD_PRIO);
					Delay10Ms(1);
					if( tmpCount > 500 ) {
						tmpCount = 0;
						rtn = STEP_CHGEND;
						break;
					}
				}
			}
			Delay10Ms(4);
		}//end while
		return rtn;
}


extern void Bms_InitData(uint8 gunNo);
extern void Driver_ReInitCANDev(uint8 gunNo);

/*充电前数据初始化都包括在此函数中*/
void TaskMain_InitChargeData(uint8 gunNo)
{
	 Bms_InitData(gunNo);
	
	 Check_ClearAllErrBit(gunNo);
	
	 TskMain_ChgDataInit(gunNo);
}


MSG_WORK_STATUS_STRUCT  m_msg_work_status;

void TaskMian_SendProcessMsg(uint8 gunNo,uint8 statu,uint8 reason,uint8 errcode,uint8 accFg)
{
	uint8  msg[5] = {0};
	
#if 0
	if(statu == IDEL)   //直接发送消息给屏
	{
	  	msg[0] = gunNo;
	  	msg[1] = statu;
	  	msg[2] = reason;
		msg[3] = errcode;
		msg[4] = accFg;

	
  		SendMsgWithNByte(MSG_WORK_STATU,5,&msg[0],APP_TASK_SCREEN_PRIO);	
	}
	else
	{	//由定时器发送给屏
		m_msg_work_status.gunno = gunNo;
		m_msg_work_status.status = statu;
		m_msg_work_status.reason = reason;
		m_msg_work_status.errcode = errcode;
		m_msg_work_status.accFg = accFg;
		m_msg_work_status.valid = 1;   // 1---valid; 0---invalid
	}	
#else
	msg[0] = gunNo;
	msg[1] = statu;
	msg[2] = reason;
	msg[3] = errcode;
	msg[4] = accFg;

	SendMsgWithNByte(MSG_WORK_STATU,5,&msg[0],APP_TASK_SCREEN_PRIO);	

#endif
}


static void TaskMain_SetErrCode(uint8 gunNo,uint8 err)
{
	 gPtrRunData[gunNo]->logic->stopReason = EERR_REASON;  
	 gPtrRunData[gunNo]->logic->errCode = err;
}

static void TaskMain_SetStopCode(uint8 gunNo,uint8 stopcode,uint8 reason)
{
	 gPtrRunData[gunNo]->logic->stopReason = stopcode;  
	 gPtrRunData[gunNo]->logic->errCode = reason;
}
uint8 Get_BackVIN_Clear(uint8 gunNo)
{
	uint8 rtn = 0;
	static uint32 sWaitTicks[DEF_MAX_GUN_NO] = {0};
	PROTO_ST *proto = BmsCommUnit_GetProtoPtr(gunNo);
	START_PARAM  *PtrStartPara = ChgData_GetStartParaPtr(gunNo);
	
	rtn = 0;
	if(START_BY_VIN == PtrStartPara->startby){
		if(0 == PtrStartPara->vinback){
			
			PtrStartPara->vinback = 1;
			PtrStartPara->vinbackTicks = GetSystemTick();
			sWaitTicks[gunNo] = GetSystemTick();
			proto->ctrlreport(gunNo,BRM_CODE,CRICLE_DISALLOWED,0);
			RelayOut_AssistPower(gunNo,ASSIST_POWER_OFF);
			
			SendMsgWithNByte(MSG_VIN_CARD,1,&gunNo,APP_TASK_BACK_PRIO);
			TaskMian_SendProcessMsg(gunNo,CHECK,E15_VINCHECK,0,0);
			rtn = 1;
		}
		else if(1 == PtrStartPara->vinback){				
			if(GetSystemTick() > PtrStartPara->vinbackTicks + TIM_NS(30)){		
				Check_SetErrCode(gunNo,ECODE106_VINBACKTM);
				SET_STOPING(gunNo,STEP_CHGEND,UNLOCKED_END); /*退出*/
				return rtn;
			}
			if(GetSystemTick() > sWaitTicks[gunNo] + TIM_NS(3)){
				sWaitTicks[gunNo] = GetSystemTick();
				SendMsgWithNByte(MSG_VIN_CARD,1,&gunNo,APP_TASK_BACK_PRIO);
			}
			rtn = 1;
		}
	}
	return rtn;
}


void DevEnterIdleStatus(CTRL_STEP* gun_ctrl)
{
	uint8  uTmp8 = 0;
	uint16 tmp16 = 0;
	uint32 tmp32 = 0;
	uint16 u16TmpCnt = 0;
	uint8  Reason = 0;
	uint8  errBuf[MAX_ERRBUF_SIZE] = {0};
	DEV_LOGIC_TYPE *ptrOtherlogic =  ChgData_GetLogicDataPtr((gun_ctrl->gunNo+1)% MAX_MOD_GROUP);

	
	gPtrRunData[gun_ctrl->gunNo]->logic->gunInUsed = 0;

	/*设置清除数据标志*/
	if( 0 == gun_ctrl->u8ClearFg ) {
		gun_ctrl->u8ClearFg = 1;
		RelayOut_Locked(gun_ctrl->gunNo,CTRL_OFF); 
		gun_ctrl->u32IdleTicks = GetSystemTick();
		gun_ctrl->u32WaitTicks = GetSystemTick();
		gun_ctrl->u32Bms24Ticks = GetSystemTick();
		RelayOut_Power12_24V(gun_ctrl->gunNo,BMS_12V);
		TaskMain_InitChargeData(gun_ctrl->gunNo);   
		if( 0 == TskMain_HardWareErrCheck(gun_ctrl->gunNo) ) {
			TaskMian_SendProcessMsg(gun_ctrl->gunNo,IDEL,0,0,0);
			RelayOut_BreakdownLed(gun_ctrl->gunNo,CTRL_OFF); /* 故障指示灯 */
		}					
			Rd_E2romTempToFullSave(gun_ctrl->gunNo); 				
	}

	/*获取BMS电源标志*/
	if(BMS_12V == RelayOut_GetBmsPower(gun_ctrl->gunNo)){
			 gun_ctrl->u32Bms24Ticks = GetSystemTick();
	}
	else{
		if(GetSystemTick() > gun_ctrl->u32Bms24Ticks + TIM_NMIN(2)){
			RelayOut_Power12_24V(gun_ctrl->gunNo,BMS_12V);
		}
	}
		
	if( GetSystemTick() - gun_ctrl->u32IdleTicks > TIM_10S ) {
		gun_ctrl->u32IdleTicks = GetSystemTick();
		if( ptrOtherlogic->workstep == STEP_IDEL ) {
			RelayOut_AcKmOut(JOUT_OFF);
		}	
	}

	//设备错误检查并报告
	if( TskMain_HardWareErrCheck(gun_ctrl->gunNo) > 0 ) {
		if( GetSystemTick() - gun_ctrl->u32WaitTicks > TIM_1S ) {
			gun_ctrl->u8ErrFlag = 1;
			gun_ctrl->u32WaitTicks = GetSystemTick();
			Check_GetErrBit(gun_ctrl->gunNo,&errBuf[0],MAX_ERRBUF_SIZE,&Reason);
			TaskMian_SendProcessMsg(gun_ctrl->gunNo,E_ERROR,0,errBuf[0],0);
			RelayOut_BreakdownLed(gun_ctrl->gunNo,CTRL_ON); /* 故障指示灯 */
		}
	}else {
		if( 1 == gun_ctrl->u8ErrFlag ){
			gun_ctrl->u8ErrFlag = 0;
			RelayOut_BreakdownLed(gun_ctrl->gunNo,CTRL_OFF); /* 故障指示灯 */
			TaskMian_SendProcessMsg(gun_ctrl->gunNo,IDEL,0,0,0);
		}
				
//		if( (GetSystemTick() - gun_ctrl->u32WaitTicks > 3*TIM_10S )){
		if( (GetSystemTick() - gun_ctrl->u32WaitTicks > TIM_10S )){
			gun_ctrl->u32WaitTicks = GetSystemTick();
			TaskMian_SendProcessMsg(gun_ctrl->gunNo,IDEL,0,0,0);
		}
	}		

}


 

extern uint8 Check_KmInVoltOver(uint8 gunNo,uint16 volt_startISO);
extern void ClearGunFallingEdge(void);
/*直流桩充电流程*/
void ChargeCtrlStep(CTRL_STEP *ptrCtrl,PROTO_ST *proto)
{
	stoc_u tmps2c;
	uint8  u8I;
	uint8  uTmp8 = 0;
	uint16 tmp16 = 0;
	uint32 tmp32 = 0;
	uint8  Reason = 0;
	uint16 u16TmpCnt = 0;
	uint8  msg[8] = {0};
	uint8  u8Msg[8] = {0};
	uint8 errBuf[MAX_ERRBUF_SIZE] = {0};
	PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	DEV_LOGIC_TYPE *ptrOtherlogic =  ChgData_GetLogicDataPtr((ptrCtrl->gunNo+1)% MAX_MOD_GROUP);
	RCV_PACK_ST *ptrBmsPack = Bms_GetBmsPack(ptrCtrl->gunNo);
	CHGDATA_ST *pBmsSendPackChg = Bms_GetChgDataPtr(ptrCtrl->gunNo);
	BMSDATA_ST *pCar = Bms_GetBmsCarDataPtr(ptrCtrl->gunNo);
	 
	switch(gPtrRunData[ptrCtrl->gunNo]->logic->workstep) {
		case STEP_IDEL:
			DevEnterIdleStatus(ptrCtrl);
			Delay10Ms(20);
         break;
     	case STEP_START:		

			DebugInfoByCon("STEP_START,启动充电。。。");
		   	ptrCtrl->u8ErrFlag = 0; /*设置清除数据标志*/
		   	gPtrRunData[ptrCtrl->gunNo]->logic->stopReason = 0;
		   	gPtrRunData[ptrCtrl->gunNo]->logic->errCode = 0;
		 
			BmsCommUnit_ProtoInit(ptrCtrl->gunNo,PROTO_GBT);
		   	TaskMain_InitChargeData(ptrCtrl->gunNo);
		 
			RelayOut_RunChargeLed(ptrCtrl->gunNo,CTRL_OFF); /*运行指示灯 */
			RelayOut_BreakdownLed(ptrCtrl->gunNo,CTRL_OFF); /* 故障指示灯 */
		   
		   	if( TskMain_HardWareErrCheck(ptrCtrl->gunNo) > 0 ) {
				SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END);/*退出*/
				Check_GetErrBit(ptrCtrl->gunNo,&errBuf[0],MAX_ERRBUF_SIZE,&Reason);
				TaskMian_SendProcessMsg(ptrCtrl->gunNo,E_ERROR,0,errBuf[0],0);
				RelayOut_BreakdownLed(ptrCtrl->gunNo,CTRL_ON); /* 故障指示灯 */
				break;
			}

			if ( CC1_4V != gPtrRunData[ptrCtrl->gunNo]->gun->statu.bits.cc1stu ) {
				DebugInfoByCon("CC1!=4V,枪未插好");
				if( ptrCtrl->u16TmpCnt++ > 20 ) {
					ptrCtrl->u16TmpCnt = 0; /*发送消息，充电枪未连接好*/
					Bms_SetStepErrFg(ptrCtrl->gunNo,CC1LINKED_ERR,1);	
					Check_SetErrCode(ptrCtrl->gunNo,ECODE18_CC1LNK); 
					SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END);/*退出*/
				}
				break;
			}
			else 
       		{
				 //Bill_SetStartData(ptrCtrl->gunNo); /* 此句移到 TaskMian_StartCharge() */
				 
				DebugInfoByCon("CC1=4V,枪已插好");
								 
	//			Driver_ReInitCANDev(0); /* 重新初始化 CAN */
	//			Driver_ReInitCANDev(1);
				 
				u8Msg[0] = ptrCtrl->gunNo;
				SendMsgWithNByte(MSG_MODDATA_INIT,1,&u8Msg[0],APP_TASK_DLMOD_PRIO); 
				 
				TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E0_READY,0,0);
				ptrCtrl->u16TmpCnt = 0;
				proto->init(ptrCtrl->gunNo);
				RelayOut_Locked(ptrCtrl->gunNo,CTRL_ON); /*锁电子锁 */
				Bms_SetStepErrFg(ptrCtrl->gunNo,LOCK_TM,0);
				Bms_StartTimeCount(ptrCtrl->gunNo,LOCK_TM,GetSystemTick());
				 
				RelayOut_XfResOnOff(ptrCtrl->gunNo,CTRL_OFF); 
				Delay10Ms(1);	
				TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E2_ELECK,0,0);
				Delay10Ms(1);	
				gPtrRunData[ptrCtrl->gunNo]->logic->u32TranPageTicks = GetSystemTick();
				 /*存储第一条临时记录,用于解锁*/
				Rd_E2romSaveTempRecord(&gTempE2ROMTabl,Bill_GetBillData(ptrCtrl->gunNo));
				SET_WORKSTEP(ptrCtrl->gunNo,STEP_LINK);
			}
			break;
		case STEP_LINK:
			DebugInfoByCon("STEP_LINK,连接处理。。。");
			uTmp8 = Step_Link_Proc(ptrCtrl->gunNo);
			if( (uTmp8 == RTN_TRUE) || (1 == gPtrParam->elockallow)) {   /*默认不检测故障*/
				TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E1_BMSPW,0,0);
				  /*闭合交流接触器*/
				RelayOut_AcKmOut(JOUT_ON);
				Delay10Ms(100); /*等待1S 模块启动*/
			
				TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E3_WTBHM,0,0);
				Delay10Ms(10);
				SET_WORKSTEP(ptrCtrl->gunNo,STEP_BPOWER);
			}else if( uTmp8 == RTN_TMOUT ) {
				  /*电子锁超时处理*/
				Bms_SetStepErrFg(ptrCtrl->gunNo,LOCK_TM,1);
				Check_SetErrCode(ptrCtrl->gunNo,ECODE1_ELOCK);
				SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END); /*退出*/
			}else {
				Delay10Ms(1);
			}
			break;
		case STEP_BPOWER:
			//patli 20191108	 RelayOut_AssistPower(ptrCtrl->gunNo,ASSIST_POWER_ON);
		 
			DebugInfoByCon("STEP_BPOWER，与模块通信。。。");
				 /*等待模块通讯正常*/
		    tmp16 = 0;
		    do{
				Delay10Ms(10);
		       	uTmp8 = CdModData_GetModCommErr(ptrCtrl->gunNo);
					if( tmp16++ > 500 ) {
						tmp16 = 0;
						Check_SetErrCode(ptrCtrl->gunNo,ECODE41_MODCOMMERR);
						SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END);/*退出*/
						break;
					 }
					 if(TskMain_HardWareErrCheck(ptrCtrl->gunNo) > 0 ) {
						SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END); /*退出*/;
						break;
					 }
			}while( (uTmp8 == FALSE) && ( tmp16 < 30 ) ); /*3S 钟以上*/
				 
			if( 1 == Check_GetErrCode(ptrCtrl->gunNo,ECODE41_MODCOMMERR)) {
				break;
			}
				 
			if(Check_GetErrBit(ptrCtrl->gunNo,&errBuf[0],MAX_ERRBUF_SIZE,&Reason) > 0 ) {
				break;
			}

			DebugInfoByCon("STEP_BPOWER，尝试模块关机");
				 
				 
				 /*将模块改为固定组，本组模块关机 */
			u8Msg[0] = ptrCtrl->gunNo ;
				 

			u8Msg[1] = MOD_CMD_OFF;
		  SendMsgWithNByte(MSG_MOD_ONOFF,2,&u8Msg[0],APP_TASK_DLMOD_PRIO);
//				 /*等待关机命令的执行 最大1S*/
//			for( u8I = 0 ; u8I < 20; u8I++ ) {
//				SendMsgWithNByte(MSG_MOD_PRESTOP,1,&u8Msg[0],APP_TASK_DLMOD_PRIO); 
//			    Delay10Ms(50);
//				if( TRUE == CdModData_CheckOnOffStu(ptrCtrl->gunNo,DLMOD_OFF) ) {
//					if( gPtrRunData[ptrCtrl->gunNo]->meter->current < 30 ) {
//						break;
//					}
//				}
//			}
			Delay10Ms(50);
				 /*断开K3接触器*****************************************************************/
			RelayOut_DcConTactKmOut(PW_CTRL_OFF);

			RelayOut_AssistPower(ptrCtrl->gunNo,ASSIST_POWER_ON); //patli 20191108
			DebugInfoByCon("STEP_BPOWER，打开辅源与BMS通信。。。");		

			if(BMS_HELI == BackCOMM->agreetype)
			{
				proto->ctrlreport(0,CCS_CODE,CRICLE_ALLOWED,1);  //合力叉车协议需要上电就发
				memset(pCar,0,sizeof(BMSDATA_ST));
				SET_WORKSTEP(ptrCtrl->gunNo,STEP_BRO_SUB);
			}
			else
			{					 
				Bms_SetStepErrFg(ptrCtrl->gunNo,BHM_TM,0);
				Bms_StartTimeCount(ptrCtrl->gunNo,BHM_TM,GetSystemTick());
				TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E4_CHKISO,0,0);
				
				proto->ctrlreport(ptrCtrl->gunNo,CHM_CODE,CRICLE_ALLOWED,1);
				SET_WORKSTEP(ptrCtrl->gunNo,STEP_SKHANDS);
			}

			break;
		case STEP_SKHANDS:
		 	
			DebugInfoByCon("STEP_SKHANDS,握手阶段");
			if((GetSystemTick() - Bms_GetStartTimeCount(ptrCtrl->gunNo,BHM_TM)) > SECS2TICKS(gPtrParam->bhmtm) ) {
				Bms_SetStepErrFg(ptrCtrl->gunNo,BHM_TM,1);
				proto->singreport(ptrCtrl->gunNo,CHM_CODE,OLD_GBT_FG);
					
				DebugInfoByCon("STEP_SKHANDS BHM TIMEOUT, AND GO TO OLD GBT");
				   /*【BHM 超时了】*/
				SET_WORKSTEP(ptrCtrl->gunNo,STEP_OLD_GBT_SUB); /*老国标测试*/

				TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E5_OLDGBT,0,0);
						
			}
	      	Delay10Ms(1);
			  /*收到BHM后状态跳转 SET_WORKSTEP(STEP_ISOCHECK_SUB)*/
			ptrCtrl->u16TmpCnt = 0;  /*下一状态使用，禁止删除*/
			break;
		case STEP_ISOCHECK_SUB:	
//			if(CHECK_TRUE == Check_KmOutVoltLess(ptrCtrl->gunNo, Bms_GetChgDataPtr(ptrCtrl->gunNo)->cml.maxoutvolt)) {
			if(ChgData_GetRunParamPtr()->minvolt >  Bms_GetChgDataPtr(ptrCtrl->gunNo)->cml.maxoutvolt)
			{
				Check_SetErrCode(ptrCtrl->gunNo,EOCDE14_PARAM);
				SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END); /*退出*/
				break;
			}

#if 0			//不进行绝缘检测，故不开启模块
			if(CHECK_FALSE == Check_KmInVoltOver(ptrCtrl->gunNo,ChgData_GetRunParamPtr()->minvolt))
			{

				TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E6_DCOUT,0,0);
         		Delay10Ms(1);				 
				 
//				tmp16 = Check_SuitableVolt(ptrCtrl->gunNo);
				tmps2c.s = ChgData_GetRunParamPtr()->minvolt+20;//tmp16;
				msg[0] = ptrCtrl->gunNo;
				msg[1] = tmps2c.c[0];  
				msg[2] = tmps2c.c[1];
				tmps2c.s = MIN_MOD_CURRENT;
				msg[3] = tmps2c.c[0];  
				msg[4] = tmps2c.c[1];
				
				 /*【发送模块上电消息消息】*/
		    	SendMsgWithNByte(MSG_MOD_ADJUST,5,&msg[0],APP_TASK_DLMOD_PRIO); /*模块调整电压时，如果没开机，自动开机*/
				Delay10Ms(5);
				u8Msg[0] = ptrCtrl->gunNo;
				u8Msg[1] = MOD_CMD_ON;
				SendMsgWithNByte(MSG_MOD_ONOFF,2,&u8Msg[0],APP_TASK_DLMOD_PRIO);				
				 
				 /*检测接触器内侧电压是否与模块上电电压一致*/
				TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E7_WTISOF,0,0);
				u16TmpCnt = 0;
				tmps2c.s = 0;
				u16TmpCnt = 0;
				do {
					u16TmpCnt++;
					if( u16TmpCnt%5 == 0 ) { /*需要根据实际调整  100 TO 20*/
//						u16TmpCnt = 0;
						SendMsgWithNByte(MSG_MOD_ADJUST,5,&msg[0],APP_TASK_DLMOD_PRIO); 
						Delay10Ms(20);
						u8Msg[0] = ptrCtrl->gunNo;
						u8Msg[1] = MOD_CMD_ON;
				     	SendMsgWithNByte(MSG_MOD_ONOFF,2,&u8Msg[0],APP_TASK_DLMOD_PRIO);
						 
						if(tmps2c.s++ > 30 ) {  // 50 TO 10
							 /*添加故障退出代码 模块输出异常*/
							 
							DebugInfoByCon("STEP_ISOCHECK_SUB,  模块输出异常，结束充电。。。");
							 
							Bms_SetStepErrFg(ptrCtrl->gunNo,MODVOLT_ERR,1);
							Check_SetErrCode(ptrCtrl->gunNo,ECODE19_MODVOLT);
							SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END);/*退出*/
							break;
						 }
					}
				   	DelaySec(1);
					if(TskMain_HardWareErrCheck(ptrCtrl->gunNo) > 0 ) {
						SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END); /*退出*/;
						break;
					}
					 
					TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E7_WTISOF,0,0); /*检测接触器内侧电压是否与模块上电电压一致*/
				}while(CHECK_FALSE == Check_KmInVoltOver(ptrCtrl->gunNo,ChgData_GetRunParamPtr()->minvolt));

//#if 0
//				if(u16TmpCnt >=15)  //超过15s
//				{
//					RelayOut_AssistPower(ptrCtrl->gunNo,ASSIST_POWER_OFF); //patli 20191108
//					Delay10Ms(10);
//					RelayOut_AssistPower(ptrCtrl->gunNo,ASSIST_POWER_ON); //patli 20191108
//					DebugInfoByCon("STEP_ISOCHECK_SUB，重新打开辅源与BMS通信。。。"); 			 
//						 
//					Bms_SetStepErrFg(ptrCtrl->gunNo,BHM_TM,0);
//					Bms_StartTimeCount(ptrCtrl->gunNo,BHM_TM,GetSystemTick());
////					TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E4_CHKISO,0,0);
//						 
//					proto->ctrlreport(ptrCtrl->gunNo,CHM_CODE,CRICLE_ALLOWED,1);
//					
//				}
//#endif				 
				if( 1 == Bms_GetStepErrFg(ptrCtrl->gunNo,MODVOLT_ERR) ) {
					break;
				}
				 
				if(Check_GetErrBit(ptrCtrl->gunNo,&errBuf[0],MAX_ERRBUF_SIZE,&Reason) > 0 ) {
					break;
				}
				 
				 /* 电压稳定后吸合接触器 */
//				RelayOut_DcKmOut(ptrCtrl->gunNo,KM_ON);
//				Delay10Ms(5);
				 
				DebugInfoByCon("STEP_ISOCHECK_SUB, 发送绝缘检测On消息，启动绝缘检测");
				 /*【发送绝缘检测On消息，启动绝缘检测】*/

//				msg[0] = ptrCtrl->gunNo; 
//				msg[1] = 0;
//				SendMsgWithNByte(MSG_ISO_START,2,&msg[0],APP_TASK_ISO_PRIO); 

//				Bms_SetStepErrFg(ptrCtrl->gunNo,XF_TM,0);
//				Bms_StartTimeCount(ptrCtrl->gunNo,XF_TM,GetSystemTick());
				SET_WORKSTEP(ptrCtrl->gunNo,STEP_ISOWAIT_SUB);
			}			
			else 
#endif				
			{
				SET_WORKSTEP(ptrCtrl->gunNo,STEP_ISOWAIT_SUB);

#if 0			 
				DebugInfoByCon("STEP_ISOCHECK_SUB, 接触器外侧电压不小于10V...");
				  /*等待一定时间后，超时停止， 【接触器外侧电压不小于10V故障】*/
				if(  ptrCtrl->u16TmpCnt++ > 1000 ) {
					ptrCtrl->u16TmpCnt = 0;
					Bms_SetStepErrFg(ptrCtrl->gunNo,KMVOLT10_ERR,1);
					Check_SetErrCode(ptrCtrl->gunNo,ECODE12_KM10V);
					SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END); /*退出*/
						/*故障跳出*/
					break;
				}
				Delay10Ms(1);
#endif
			 }
			 break;
		 case STEP_ISOWAIT_SUB:

#if 0		 	  
			  DebugInfoByCon("STEP_ISOWAIT_SUB，绝缘检测等待");
			  if(GetSystemTick() - Bms_GetStartTimeCount(ptrCtrl->gunNo,XF_TM) > SECS2TICKS(gPtrParam->xftm) ) {
						  Bms_SetStepErrFg(ptrCtrl->gunNo,XF_TM,1);
				      /*【XF 超时了】*/
					    Check_SetErrCode(ptrCtrl->gunNo,ECODE3_XFTM);
					    SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END); /*退出*/
						break;
				}
#endif
				
//       if(1 == AdcCalc_GetISOStatus())       //输出只有一个接触器，闭合接触器前不进行绝缘检测
//       {			
//          Check_SetErrCode(ptrCtrl->gunNo, ECODE38_2POSERR);   /*绝缘故障 */
//				  SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END); /*退出*/
//       }
//			 else
//			 {
			 	SET_WORKSTEP(ptrCtrl->gunNo,STEP_ISOFINISH_SUB);
//			 }

			 break;
		 case STEP_ISOFINISH_SUB:
		 	
			DebugInfoByCon("STEP_ISOFINISH_SUB，绝缘检测结束");
//			    TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E8_WTXFF,0,0);
		 
//		     	RelayOut_DcKmOut(ptrCtrl->gunNo,KM_OFF);
//		      Delay10Ms(20);
			    /*泄放流程*/
//		      uTmp8 = 0;
//		      Bms_SetStepErrFg(ptrCtrl->gunNo,XF_TM,0);
//					Bms_StartTimeCount(ptrCtrl->gunNo,XF_TM,GetSystemTick());
//          if( TskMain_StopHardWareStep(ptrCtrl->gunNo) > 0 ) { 
//             /*说明泄放有问题，或者模块通信故障*/
//							Bms_SetStepErrFg(ptrCtrl->gunNo,XF_TM,1);
//							/*【泄放 超时了】*/
//							Check_SetErrCode(ptrCtrl->gunNo,ECODE3_XFTM);
//						  SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END);
//						  break;
//					}
//					/*绝缘检查*/
//					uTmp8 = TskMain_IsoCheckBefor(ptrCtrl->gunNo);
//					if( uTmp8 > 0 ) {
//						Check_SetErrCode(ptrCtrl->gunNo,(E_ERR_CODE)uTmp8); 
//						SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END);
//						break; /*退出Case*/
//					}

//					if ( 0 == Bms_GetStepErrFg(ptrCtrl->gunNo,XF_TM)	) {	 /*无故障，进入参数设置环节*/		
//						TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E9_WTBRM,0,0);
//						SET_WORKSTEP(ptrCtrl->gunNo,STEP_PARAMSET);
//					}else {
//						SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END);
//					}

					
			msg[0] = ptrCtrl->gunNo;
			msg[1] = MOD_CMD_OFF;
			SendMsgWithNByte(MSG_MOD_ONOFF,2,&msg[0],APP_TASK_DLMOD_PRIO);
			TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E9_WTBRM,0,0);
			SET_WORKSTEP(ptrCtrl->gunNo,STEP_PARAMSET);

			break;
		 case STEP_PARAMSET:
		 	  
			  		DebugInfoByCon("STEP_PARAMSET，握手阶段，等待BRM");
					 proto->ctrlreport(ptrCtrl->gunNo,CHM_CODE,CRICLE_DISALLOWED,0);


					 /*发送CRM 00*/
					 ((CHGDATA_ST *)Bms_GetChgDataPtr(ptrCtrl->gunNo))->crm.idenf = SPN_READYNO;
					  proto->ctrlreport(ptrCtrl->gunNo,CRM_CODE,CRICLE_ALLOWED,0);

					 Bms_SetStepErrFg(ptrCtrl->gunNo,BRM_TM,0);
					 Bms_StartTimeCount(ptrCtrl->gunNo,BRM_TM,GetSystemTick());
					 SET_WORKSTEP(ptrCtrl->gunNo,STEP_BRM_WAIT_SUB);
			     break;
				 
			case STEP_OLD_GBT_SUB: /*老国标,BHM超时后进行老国标CRM探测*/
				
					DebugInfoByCon("STEP_OLD_GBT_SUB，老国标CRM探测");
					((CHGDATA_ST *)Bms_GetChgDataPtr(ptrCtrl->gunNo))->crm.idenf = SPN_READYNO;
					proto->ctrlreport(ptrCtrl->gunNo,CRM_CODE,CRICLE_ALLOWED,0);
			    
					Bms_SetStepErrFg(ptrCtrl->gunNo,BRM_TM,0);
			
					Bms_StartTimeCount(ptrCtrl->gunNo,BRM_TM,GetSystemTick());
			   
					SET_WORKSTEP(ptrCtrl->gunNo,STEP_BRM_WAIT_SUB);
			
				  break;
		 case STEP_BRM_WAIT_SUB:
			DebugInfoByCon("STEP_BRM_WAIT_SUB，识别等待");
			 if(GetSystemTick() - Bms_GetStartTimeCount(ptrCtrl->gunNo,BRM_TM) > SECS2TICKS(gPtrParam->brmtm) ) {				
			 
				     /*【BRM 超时了】*/
						 Bms_SetStepErrFg(ptrCtrl->gunNo,BRM_TM,1);
				     Check_SetErrCode(ptrCtrl->gunNo,ECODE4_BRMTM);
					 
#ifdef TRY_START_CHARGIN
					TaskMian_TryStartCharge(ptrCtrl,ChgData_GetStartParaPtr(ptrCtrl->gunNo));
#else
				     SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END); /*退出*/
#endif
				     break;
			 }
			 break;
		 case STEP_BRM_SUB:		
		 	 			DebugInfoByCon("STEP_BRM_SUB,识别阶段");

		    if(Get_BackVIN_Clear(ptrCtrl->gunNo)){ 
				break;
			}
			 
			 /*发送CRM AA*/
			 if( 0 < proto->singreport(ptrCtrl->gunNo,CRM_CODE,SPN_READYOK)) {
					((CHGDATA_ST *)Bms_GetChgDataPtr(ptrCtrl->gunNo))->crm.idenf = SPN_READYOK;
					proto->ctrlreport(ptrCtrl->gunNo,CRM_CODE,CRICLE_ALLOWED,0);
			 }
			 
			 TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E10_WTBCP,0,0);
			 Bms_SetStepErrFg(ptrCtrl->gunNo,BCP_TM,0);
			 Bms_StartTimeCount(ptrCtrl->gunNo,BCP_TM,GetSystemTick());
       /*收到BHM后清除BHM故障标志，适应老国标*/
			 Bms_SetStepErrFg(ptrCtrl->gunNo,BHM_TM,0);
			 SET_WORKSTEP(ptrCtrl->gunNo,STEP_BCPWAIT_SUB); 
			/*等待收到BCP跳转 SET_WORKSTEP(STEP_BCP_SUB)*/
			 break;
		 case STEP_BCPWAIT_SUB:
		 	
			DebugInfoByCon("STEP_BCPWAIT_SUB，参数配置等待");
			  if(GetSystemTick() - Bms_GetStartTimeCount(ptrCtrl->gunNo,BCP_TM) > SECS2TICKS(gPtrParam->bcptm) ) {
						Bms_SetStepErrFg(ptrCtrl->gunNo,BCP_TM,1);
				    /*【BCP 超时了】*/
					  Check_SetErrCode(ptrCtrl->gunNo,ECODE5_BCPTM);
					  RelayOut_Locked(ptrCtrl->gunNo,CTRL_OFF); 
#ifdef TRY_START_CHARGIN
					  TaskMian_TryStartCharge(ptrCtrl,ChgData_GetStartParaPtr(ptrCtrl->gunNo));
#else
					  SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END); /*退出*/
#endif

					  break;
			 }
			 break;
		 case STEP_BCP_SUB:
		 	
			DebugInfoByCon("STEP_BCP_SUB，参数配置阶段");
			 /*停止发送CRM*/
    	 proto->ctrlreport(ptrCtrl->gunNo,CRM_CODE,CRICLE_DISALLOWED,0);
			 if( CHECK_TRUE != Check_CarParam(ptrCtrl->gunNo)) { 
				  /*【车辆参数是否合适】*/
				  Bms_SetStepErrFg(ptrCtrl->gunNo,PARAM_ERR,1);
				  Check_SetErrCode(ptrCtrl->gunNo,EOCDE14_PARAM);
				  SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END); /*退出*/
				  break;
			 }
			 proto->ctrlreport(ptrCtrl->gunNo,CTS_CODE,CRICLE_ALLOWED,1);
			 proto->ctrlreport(ptrCtrl->gunNo,CML_CODE,CRICLE_ALLOWED,1);
			 
			 Bill_SetCarBmsData(ptrCtrl->gunNo);
			 
			 TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E11_WTBRO,0,0);
			 Bms_SetStepErrFg(ptrCtrl->gunNo,BRO_TM,0);
			 Bms_StartTimeCount(ptrCtrl->gunNo,BRO_TM,GetSystemTick());
			 SET_WORKSTEP(ptrCtrl->gunNo,STEP_BROWAIT_SUB);
			 /*等待AA BRO 跳转 SET_WORKSTEP(STEP_BROWAIT_SUB);*/ 
			 break;
		 case STEP_BROWAIT_SUB:
		 	
			DebugInfoByCon("STEP_BROWAIT_SUB，等待是否准备好");
			 if(GetSystemTick() - Bms_GetStartTimeCount(ptrCtrl->gunNo,BRO_TM) > SECS2TICKS(gPtrParam->brotm) ) {
						Bms_SetStepErrFg(ptrCtrl->gunNo,BRO_TM,1);
				    /*【BRO 超时了】*/
				    Check_SetErrCode(ptrCtrl->gunNo,ECODE6_BROTM);
#ifdef TRY_START_CHARGIN
					TaskMian_TryStartCharge(ptrCtrl,ChgData_GetStartParaPtr(ptrCtrl->gunNo));
#else
				    SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END);/*退出*/
#endif
					  break;
			 }
			 break; /*收到BRO报文完成跳转 SET_WORKSTEP(STEP_BRO_SUB);*/
		 case STEP_BRO_SUB:
			if(BMS_HELI == BackCOMM->agreetype)
			{
				u16TmpCnt = 0;
		   		Delay10Ms(300);   //加延时，等待电池端吸合继电器

			 	do 
			 	{
//					tmp16 = ((CHARGE_TYPE *)ChgData_GetRunDataPtr())->meter->volt;    //直流电压(外侧)
					tmp16 = Bms_GetBmsCarDataPtr(0)->bcp.batcurvolt;     //因绝缘检测检测的不准，暂时的替代方法
					
				  	if (u16TmpCnt++ > 1000)   //20S
					{
						u16TmpCnt = 0;
						Check_SetErrCode(0,ECODE7_BCLTM);
						SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END);/*退出*/
						break;
					}
					
					Delay10Ms(2);
					
					if (TskMain_HardWareErrCheck(ptrCtrl->gunNo) > 0) 
					{
						 SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END); /*退出*/;
						 break;
					}
					
			 	} while (tmp16 < VOLT_TRAN(40)); 
				if(STEP_CHGEND == gPtrRunData[ptrCtrl->gunNo]->logic->workstep)
				{
					break;
				}
			
			}
			else
			{
				DebugInfoByCon("STEP_BRO_SUB，准备阶段，模块等准备中");
				/*停止发送CTS*/
				proto->ctrlreport(ptrCtrl->gunNo,CTS_CODE,CRICLE_DISALLOWED,0);
				/*停止发送CML*/
				proto->ctrlreport(ptrCtrl->gunNo,CML_CODE,CRICLE_DISALLOWED,0);
	
				/*发送未准备就绪的CRO*/
				((CHGDATA_ST *)Bms_GetChgDataPtr(ptrCtrl->gunNo))->cro.crostu = CRO_READYNO;
		    	proto->singreport(ptrCtrl->gunNo,CRO_CODE,CRO_READYNO);
				proto->ctrlreport(ptrCtrl->gunNo,CRO_CODE,CRICLE_ALLOWED,0);
		    	Delay10Ms(1);
		 
				TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E12_CHKPARA,0,0);
				u16TmpCnt = 0;
		    
		    	/*BYD E6 必须先回OK，车端接触器才能闭合*/
		 
			   do 
			   {
					if( u16TmpCnt++ > 1000 ) {
						u16TmpCnt = 0;
						/*【车辆侧电压与报文不一致大于±5%】*/
						Bms_SetStepErrFg(ptrCtrl->gunNo,CARVOL_ERR,1);
						/*此处需要跳转到故障*/
						SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END);/*退出*/
						break;
					}
					Delay10Ms(2);
					if(TskMain_HardWareErrCheck(ptrCtrl->gunNo) > 0 ) {
						 SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END); /*退出*/;
						 break;
					}
					tmp16 = gPtrRunData[ptrCtrl->gunNo]->bms->car.bcp.batcurvolt;
					TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E12_CHKPARA,0,0);
			  }while (CHECK_TRUE != Check_CarVolt(ptrCtrl->gunNo)); 
			
			}
		 

			 
//			 /* 桩端测的电压不应该是负值 */
//			 if (((CHARGE_TYPE *)ChgData_GetRunDataPtr(ptrCtrl->gunNo))->iso->vdc3 < -600)
#ifndef BMSTEST			 
			 if(AdcCalc_GetValue()->vdciso[1] < ChgData_GetRunParamPtr()->minvolt)   //电池电压低于模块的开启电压
			 {
					Check_SetErrCode(ptrCtrl->gunNo, ECODE104_PAVOUTSIDEREVERSE);   /* 枪外侧电压反接(BMS端) */			 
					/*此处需要跳转到故障*/
					SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END);/*退出*/
					break;
			 }
#endif
			 if (1 == Bms_GetStepErrFg(ptrCtrl->gunNo,CARVOL_ERR)) 
			 {
		      Check_SetErrCode(ptrCtrl->gunNo,ECODE15_PKGVOLT);
				  /*此处需要跳转到故障*/
					SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END);/*退出*/
				  break;
			 }
			 
			 if (Check_GetErrBit(ptrCtrl->gunNo,&errBuf[0],MAX_ERRBUF_SIZE,&Reason) > 0 ) 
			 {
					/*此处需要跳转到故障*/
					SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END);/*退出*/
					break;
			 }

			 TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E13_DCOUTADJ,0,0);
			 
			 u16TmpCnt = 0;


			 /* 按照实际检测到的电池电压调整模块 */
//			 tmp16 = ((CHARGE_TYPE *)ChgData_GetRunDataPtr(ptrCtrl->gunNo))->iso->vdc3;  /*接触器外侧电压*/ 	
             tmp16 = AdcCalc_GetValue()->vdciso[1];

			if(BMS_HELI == BackCOMM->agreetype)
		 	{
		 		Delay10Ms(100);  //1秒
				heli_Bill_SetCarBmsData(ptrCtrl->gunNo);
#ifndef BMSTEST					
		 		if (tmp16 < VOLT_TRAN(40))     //合力叉车协议
			 	{
				 	Check_SetErrCode(ptrCtrl->gunNo,HELI_ECODE48_OUTSIDE_KM_40LESS);
				 	SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END); /*退出*/;
				 	break;
			 	}
#endif				
		 	}
			
			 tmps2c.s = tmp16 - VOLT_TRAN(2); /*电池电压 -1~10V*/
			 msg[0] = ptrCtrl->gunNo;
			 msg[1] = tmps2c.c[0]; /*电压*/
			 msg[2] = tmps2c.c[1];
			 tmps2c.s = MIN_MOD_CURRENT;
			 msg[3] = tmps2c.c[0];
			 msg[4] = tmps2c.c[1];
				/*【发送消息 调整模块输出电压】*/
			 SendMsgWithNByte(MSG_MOD_ADJUST,5,&msg[0],APP_TASK_DLMOD_PRIO);
			 Delay10Ms(1);
			 u8Msg[0] = ptrCtrl->gunNo;
			 u8Msg[1] = MOD_CMD_ON;
			 SendMsgWithNByte(MSG_MOD_ONOFF,2,&u8Msg[0],APP_TASK_DLMOD_PRIO);
			 
			 SET_WORKSTEP(ptrCtrl->gunNo,STEP_ADJUST_SUB);
			 ptrCtrl->u16TmpCnt = 0;
			 break;
			 
		 case STEP_ADJUST_SUB:
		 	
			DebugInfoByCon("STEP_ADJUST_SUB，调整检测。。。");
			 if( CHECK_TRUE == Check_KmInAndOut(ptrCtrl->gunNo) ){

				  Delay10Ms(100);  //1秒
//				  if(1 == AdcCalc_GetISOStatus())
//				  {
//				  	Check_SetErrCode(ptrCtrl->gunNo, ECODE38_2POSERR);   /*绝缘故障 */			 
//					/*此处需要跳转到故障*/
//					SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END);/*退出*/
//					break;
//				  }			 
				
				  Bms_SetStepErrFg(ptrCtrl->gunNo,KMINOUT_ERR,0);
				  TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E14_STRATCHG,0,0);

				  RelayOut_DcKmOut(ptrCtrl->gunNo,KM_ON);
				  Delay10Ms(20);

					RelayOut_RunChargeLed(ptrCtrl->gunNo,CTRL_ON); /*运行指示灯 */

					if(BMS_HELI != BackCOMM->agreetype)	
					{
						/* 发送准备就绪的CRO */
					    ((CHGDATA_ST *)Bms_GetChgDataPtr(ptrCtrl->gunNo))->cro.crostu = CRO_READYOK;
						proto->singreport(ptrCtrl->gunNo,CRO_CODE,CRO_READYOK);
						proto->ctrlreport(ptrCtrl->gunNo,CRO_CODE,CRICLE_ALLOWED,0);
						Delay10Ms(5);
					}
				 
					Bms_SetStepErrFg(ptrCtrl->gunNo,BCL_TM,0);
					Bms_SetStepErrFg(ptrCtrl->gunNo,BCS_TM,0);
			    Bms_StartTimeCount(ptrCtrl->gunNo,BCL_TM,GetSystemTick());
					Bms_StartTimeCount(ptrCtrl->gunNo,BCS_TM,GetSystemTick());
					
					gPtrRunData[ptrCtrl->gunNo]->logic->startfg = 1;
					TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHARGING,0,0,0);
					ptrCtrl->u32WaitTicks = GetSystemTick();
					
					Bms_InitErrData(ptrCtrl->gunNo);
					gPtrRunData[ptrCtrl->gunNo]->logic->chgtimes++; /*记录一次上电来的充电次数*/
					SET_WORKSTEP(ptrCtrl->gunNo,STEP_CHARGEING);

			 }
			 else 
			 {
				 TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHECK,E13_DCOUTADJ,0,0);
				 /* 调整80秒 */
				 if (ptrCtrl->u16TmpCnt++ > 80) 
				 {
						/*此处说明有故障了,根据调试情况添加代码*/
						ptrCtrl->u16TmpCnt = 0;
						Bms_SetStepErrFg(ptrCtrl->gunNo,KMINOUT_ERR,1);
						Check_SetErrCode(ptrCtrl->gunNo,ECODE20_INOUT10V);
						SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END);/*退出*/
						break;
				 }
				 
				 if (0 == (ptrCtrl->u16TmpCnt % 10))   //zyf 20191012 from to 10
				 {
						/* 每20秒跟踪一次电池电压 */
						/* 按照实际检测到的电池电压调整模块 */
//						tmp16 = ((CHARGE_TYPE *)ChgData_GetRunDataPtr(ptrCtrl->gunNo))->iso->vdc3; /*接触器外侧电压*/	
                        tmp16 = AdcCalc_GetValue()->vdciso[1];
						if(tmp16 < VOLT_TRAN(40))
						{
							break;
						}
						tmps2c.s = tmp16 - VOLT_TRAN(2); /*电池电压 -1~10V*/
						msg[0] = ptrCtrl->gunNo;
						msg[1] = tmps2c.c[0]; /*电压*/
						msg[2] = tmps2c.c[1];
						tmps2c.s = MIN_MOD_CURRENT;
						msg[3] = tmps2c.c[0];
						msg[4] = tmps2c.c[1];
						/*【发送消息 调整模块输出电压】*/
						SendMsgWithNByte(MSG_MOD_ADJUST,5,&msg[0],APP_TASK_DLMOD_PRIO);
						Delay10Ms(1);
						u8Msg[0] = ptrCtrl->gunNo;
						u8Msg[1] = MOD_CMD_ON;
						SendMsgWithNByte(MSG_MOD_ONOFF,2,&u8Msg[0],APP_TASK_DLMOD_PRIO);
				 }
	
				 Delay10Ms(100);  //1秒
			 }
			 break;
		 case STEP_CHARGEING:   //开始充电
		 
				 /*【开始计费】*/
				 tmp32 = Bill_RealCalc(ptrCtrl->gunNo,100);
				 if( tmp32 >= 100 ) {
					 /*大于1度电执行存储一次*/
					 Rd_E2romSaveTempRecord(&gTempE2ROMTabl,Bill_GetBillData(ptrCtrl->gunNo));
					 Delay5Ms(2);
				 }
				 
				 if( GetSystemTick() - ptrCtrl->u32WaitTicks > TIM_1MIN ) {
					  ptrCtrl->u32WaitTicks = GetSystemTick();
						TaskMian_SendProcessMsg(ptrCtrl->gunNo,CHARGING,0,0,0);
				 }
				 
				 TskMain_ChargingStatuCheckLoop(ptrCtrl->gunNo);
				 TskMain_ChargingChgDataLoop(ptrCtrl->gunNo);
				 
				 Reason = TskMain_StopCondition(ptrCtrl->gunNo);
				 
//				 DebugInfoWithPi(CON_MOD,"STEP_CHARGEING，充电中,结束条件", Reason);
				 
				 if( Reason > 0 ) {
				    Check_GetErrBit(ptrCtrl->gunNo,&errBuf[0],MAX_ERRBUF_SIZE,&Reason);
						SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,LOCKED_END);
					  if( ECODE50_BMSFINISH == errBuf[0] || ECODE51_BMSTVOLT == errBuf[0] \
							|| ECODE52_BMSSVOLT == errBuf[0] ) {
					      gPtrRunData[ptrCtrl->gunNo]->logic->stopReason = EAUTO_REASON;
						}else {
								gPtrRunData[ptrCtrl->gunNo]->logic->stopReason = EERR_REASON;
						}
						gPtrRunData[ptrCtrl->gunNo]->logic->errCode = errBuf[0];
				 }
			   break;
		 case STEP_CHGEND: /* 充电开始后的结束到这里来 */
				if(BMS_HELI == BackCOMM->agreetype)	
				{
					ClearGunFallingEdge();     //清除拔枪下降沿。停止后必须拔枪才能再次开启充电
				}
				DebugInfoByCon("STEP_CHGEND，充电结束");
		      /* 关模块 */
		     u8Msg[0] = ptrCtrl->gunNo;
				 u8Msg[1] = MOD_CMD_OFF;
				 SendMsgWithNByte(MSG_MOD_ONOFF,2,&u8Msg[0],APP_TASK_DLMOD_PRIO);
		 
				 Gbt_CtrlCricleAllclose(ptrCtrl->gunNo);
		 
				 proto->ctrlreport(ptrCtrl->gunNo,CCS_CODE,CRICLE_DISALLOWED,0); /* 停止发送CCS */
		 
		     /* 没有收到BST，那么是充电机主动停止，因此周期回复CST */
				 if (0 == ptrBmsPack->statu.bits.rcvBST)
				 {
					 /* 主动停止 */
					 proto->singreport(ptrCtrl->gunNo,CST_CODE,0);
					 proto->ctrlreport(ptrCtrl->gunNo,CST_CODE,CRICLE_ALLOWED,1);    /* 周期回复CST */
					 
					 proto->singreport(ptrCtrl->gunNo,CSD_CODE,0);
					 proto->ctrlreport(ptrCtrl->gunNo,CSD_CODE,CRICLE_ALLOWED,1);	  /* 周期回复CSD */
					 
					 Bms_SetStepErrFg(ptrCtrl->gunNo,BST_TM,0);
					 Bms_SetStepErrFg(ptrCtrl->gunNo,BSD_TM,0);
					 
					 Bms_StartTimeCount(ptrCtrl->gunNo,BST_TM,GetSystemTick());
					 Bms_StartTimeCount(ptrCtrl->gunNo,BSD_TM,GetSystemTick());
				 }
				 else
				 {
					 pBmsSendPackChg->cst.stoprsn.bits.initiative = 0x01; /* BMS中止(收到BST帧) */
					 
					 proto->singreport(ptrCtrl->gunNo,CST_CODE,0);
					 proto->ctrlreport(ptrCtrl->gunNo,CST_CODE,CRICLE_ALLOWED,1);    /* 周期回复CST */
					 
					 proto->singreport(ptrCtrl->gunNo,CSD_CODE,0);
				   proto->ctrlreport(ptrCtrl->gunNo,CSD_CODE,CRICLE_ALLOWED,1);	  /* 周期回复CSD */
					 
					 Bms_SetStepErrFg(ptrCtrl->gunNo,BSD_TM,0); 
					 Bms_StartTimeCount(ptrCtrl->gunNo,BSD_TM,GetSystemTick());
				 } 
		 
				 /* 判断电流小于5A */
				 u16TmpCnt = 0;
				 do{
					 if( (u16TmpCnt++ % 30 ) == 0 ) {
						  u8Msg[0] = ptrCtrl->gunNo;
				      u8Msg[1] = MOD_CMD_OFF;
							SendMsgWithNByte(MSG_MOD_ONOFF,2,&u8Msg[0],APP_TASK_DLMOD_PRIO);
							Delay10Ms(1);
							if( u16TmpCnt > 240 ) {
								u16TmpCnt = 0;
								Bms_SetStepErrFg(ptrCtrl->gunNo,DYMOD_COMM_ERR,1);
								break;
							}
						}
						Delay10Ms(5);
				}while(gPtrRunData[ptrCtrl->gunNo]->meter->current > 30 );
				
				Delay5Ms(3);
				/* 关闭直流接触器 */ 
				RelayOut_DcKmOut(ptrCtrl->gunNo,KM_OFF);
				
				/* 开启泄放 */ 
//				if( TskMain_StopHardWareStep(ptrCtrl->gunNo) > 0 ) {
//					/*说明未完成泄放，或模块通信故障*/
//				}
				
				/*刷卡器开始寻卡*/
				SendMsgWithNByte(MSG_CHECK_CARD,0,NULL,APP_TASK_CARD_PRIO);
				
				/*结算标志*/
				uTmp8 = 0;
				if((gPtrRunData[ptrCtrl->gunNo]->logic->stopReason == ECARD_REASON) || (gPtrRunData[ptrCtrl->gunNo]->logic->stopReason == EAPP_REASON)\
				|| (gPtrRunData[ptrCtrl->gunNo]->logic->stopReason == KEY_REASON)\
				|| (gPtrRunData[ptrCtrl->gunNo]->logic->startby == START_BY_BKGROUND) || (gPtrRunData[ptrCtrl->gunNo]->logic->startby == START_BY_ONLINECARD)\
				|| (gPtrRunData[ptrCtrl->gunNo]->logic->startby == START_BY_VIN)|| (gPtrRunData[ptrCtrl->gunNo]->logic->startby == START_BY_PASSWD) ) {
					 uTmp8 = 1;
					 Bill_GetBillData(ptrCtrl->gunNo)->IsPay = 1;
				}
				
				/*发消息给触摸屏，进行故障显示*/ 
				if( Check_GetErrBit(ptrCtrl->gunNo,&errBuf[0],MAX_ERRBUF_SIZE,&Reason) > 0 ) {
					if( 1 == gPtrRunData[ptrCtrl->gunNo]->logic->startfg ) {
						if(CONN_CHGBIRD == BackCOMM->agreetype){  //CONN_CHGBIRD
					     uTmp8 = 2;
				    }
						if(errBuf[0] == ECODE50_BMSFINISH || errBuf[0] == ECODE51_BMSTVOLT \
							|| errBuf[0] == ECODE52_BMSSVOLT || errBuf[0] == ECODE53_BMSCST \
							|| errBuf[0] == ECODE80_CHGFINISH ){
								TaskMain_SetStopCode(ptrCtrl->gunNo,EAUTO_REASON,errBuf[0]);
								TaskMian_SendProcessMsg(ptrCtrl->gunNo,FINISH,gPtrRunData[ptrCtrl->gunNo]->logic->stopReason,gPtrRunData[ptrCtrl->gunNo]->logic->errCode,uTmp8);
						 }else {
							   RelayOut_BreakdownLed(ptrCtrl->gunNo,CTRL_ON); /* 故障指示灯 */
								TaskMain_SetErrCode(ptrCtrl->gunNo,errBuf[0]);
								TaskMian_SendProcessMsg(ptrCtrl->gunNo,FINISH,gPtrRunData[ptrCtrl->gunNo]->logic->stopReason,gPtrRunData[ptrCtrl->gunNo]->logic->errCode,uTmp8);
						 }
				 }else { /*未进入充电中*/
					  RelayOut_BreakdownLed(ptrCtrl->gunNo,CTRL_ON); /* 故障指示灯 */
					 TaskMain_SetErrCode(ptrCtrl->gunNo,errBuf[0]);
					 TaskMian_SendProcessMsg(ptrCtrl->gunNo,E_ERROR,0,errBuf[0],uTmp8);
				 }
				}else {
					/*无故障停止*/
					TaskMian_SendProcessMsg(ptrCtrl->gunNo,FINISH,gPtrRunData[ptrCtrl->gunNo]->logic->stopReason,gPtrRunData[ptrCtrl->gunNo]->logic->errCode,uTmp8);
				}
				
				/* 账单存储 */
				tmp32 = Bill_RealCalc(ptrCtrl->gunNo,0);
				/*第一故障作为故障结束条件*/
				Bill_SetStopData(ptrCtrl->gunNo,gPtrRunData[ptrCtrl->gunNo]->logic->stopReason,gPtrRunData[ptrCtrl->gunNo]->logic->errCode); 
	      Rd_E2romSaveTempRecord(&gTempE2ROMTabl,Bill_GetBillData(ptrCtrl->gunNo));
				
				if( ((1 == Bill_GetBillData(ptrCtrl->gunNo)->IsPay ) && (gPtrRunData[ptrCtrl->gunNo]->logic->stopReason == ECARD_REASON)) \
				 || (gPtrRunData[ptrCtrl->gunNo]->logic->stopReason == EAPP_REASON)|| (gPtrRunData[ptrCtrl->gunNo]->logic->stopReason == KEY_REASON) \
				 || (gPtrRunData[ptrCtrl->gunNo]->logic->startby == START_BY_BKGROUND)||(gPtrRunData[ptrCtrl->gunNo]->logic->startby == START_BY_ONLINECARD)\
				 ||(gPtrRunData[ptrCtrl->gunNo]->logic->startby == START_BY_VIN)||(gPtrRunData[ptrCtrl->gunNo]->logic->startby == START_BY_PASSWD))  {
					  if(CONN_CHGBIRD != BackCOMM->agreetype){
							gPtrRunData[ptrCtrl->gunNo]->logic->recsavefg = 1;
							Bill_GetBillData(ptrCtrl->gunNo)->IsPay = 1;
							Rd_E2romSaveFullRecord(&gFullE2ROMTabl,Bill_GetBillData(ptrCtrl->gunNo));
							Rd_E2romClearTempRecord(&gTempE2ROMTabl);
						}
				}
				
				if(( 1 == gPtrRunData[ptrCtrl->gunNo]->logic->startfg )  \
					&&((gPtrRunData[ptrCtrl->gunNo]->logic->startby == START_BY_BKGROUND)||(gPtrRunData[ptrCtrl->gunNo]->logic->startby == START_BY_ONLINECARD)  \
				  ||(gPtrRunData[ptrCtrl->gunNo]->logic->startby == START_BY_VIN)||(gPtrRunData[ptrCtrl->gunNo]->logic->startby == START_BY_PASSWD) )){  //
					/*后台启动记录发送*/
					msg[0] = ptrCtrl->gunNo;
					SendMsgWithNByte(MSG_UP_RECORD,1,&msg[0],APP_TASK_BACK_PRIO);
					Delay10Ms(10);
				}
				
//				if( ptrOtherlogic->workstep == STEP_IDEL ) {
//					 Delay10Ms(200);
				RelayOut_AcKmOut(JOUT_OFF);
//				}					
				Delay10Ms(100);
				
				RelayOut_AssistPower(ptrCtrl->gunNo,ASSIST_POWER_OFF);/*关闭BMS电源 */
				
				Delay10Ms(10);
				
				RelayOut_RunChargeLed(ptrCtrl->gunNo,CTRL_OFF); /*运行指示灯 */
				
				Putian_CtrlCricleAllclose();
				Gbt_CtrlCricleAllclose(ptrCtrl->gunNo);
				
				memset(ChgData_GetStartParaPtr(ptrCtrl->gunNo),0,sizeof(START_PARAM));
				
				SendMsgWithNByte(MSG_MOD_DELALLCMD,0,0,APP_TASK_DLMOD_PRIO);
				
				gPtrRunData[ptrCtrl->gunNo]->logic->u32TranPageTicks = GetSystemTick();
				ptrCtrl->u8AccFg = 0;
				
				SET_WORKSTEP(ptrCtrl->gunNo,STEP_WAITOVER);
				
				
			  break;

		 case STEP_WAITOVER:
			 DebugInfoByCon("STEP_WAITOVER，等待结束");
		 	
				if( (1 == Bill_GetBillData(ptrCtrl->gunNo)->IsPay) && (0 == gPtrRunData[ptrCtrl->gunNo]->logic->recsavefg) ) {
					  Bill_GetBillData(ptrCtrl->gunNo)->IsPay = 1;
					  gPtrRunData[ptrCtrl->gunNo]->logic->recsavefg = 1;
						Rd_E2romSaveFullRecord(&gFullE2ROMTabl,Bill_GetBillData(ptrCtrl->gunNo)); 
						Rd_E2romClearTempRecord(&gTempE2ROMTabl);
				}
				
				if( 1 == Bill_GetBillData(ptrCtrl->gunNo)->IsPay ) {
							if(0 == ptrCtrl->u8AccFg){
								ptrCtrl->u8AccFg = 1;
								TaskMian_SendProcessMsg(ptrCtrl->gunNo,ACCOUNT,0,0,Bill_GetBillData(ptrCtrl->gunNo)->IsPay);								
							}
					    RelayOut_Locked(ptrCtrl->gunNo,CTRL_OFF); 
					    if( GetSystemTick() > gPtrRunData[ptrCtrl->gunNo]->logic->u32TranPageTicks + TIM_NMIN(1) ) {
									SET_WORKSTEP(ptrCtrl->gunNo,STEP_WAIT_SUB);
								   break;
					    }
				}else if( 0 == gPtrParam->Sysparalarm.bits.eleunlock ) {
					 RelayOut_Locked(ptrCtrl->gunNo,CTRL_OFF); 
				}else { /*未结算*/
					 ;
				}
		
				if( 0 == gPtrRunData[ptrCtrl->gunNo]->logic->startfg ) {
					RelayOut_Locked(ptrCtrl->gunNo,CTRL_OFF); 
					 if( GetSystemTick() > gPtrRunData[ptrCtrl->gunNo]->logic->u32TranPageTicks + TIM_NMIN(1) ) {
							SET_WORKSTEP(ptrCtrl->gunNo,STEP_WAIT_SUB);
						  Delay10Ms(300);
					 }
						
				}
		
				if(CC1_4V != gPtrRunData[ptrCtrl->gunNo]->gun->statu.bits.cc1stu ) 
				{
				/*发消息给触摸屏，进行充电结束显示*/ 
					if( 0 == Bill_GetBillData(ptrCtrl->gunNo)->IsPay ) {
						 Rd_E2romSaveFullRecord(&gFullE2ROMTabl,Bill_GetBillData(ptrCtrl->gunNo)); 
						 Rd_E2romClearTempRecord(&gTempE2ROMTabl);
						 TaskMian_SendProcessMsg(ptrCtrl->gunNo,ACCOUNT,1,0,Bill_GetBillData(ptrCtrl->gunNo)->IsPay);
						 Delay10Ms(200);
					}
					SET_WORKSTEP(ptrCtrl->gunNo,STEP_WAIT_SUB);
				}		
			  break;
		 case STEP_WAIT_SUB:
			 DebugInfoByCon("STEP_WAIT_SUB");
				RelayOut_XfResOnOff(ptrCtrl->gunNo,CTRL_OFF);
				RelayOut_RunChargeLed(ptrCtrl->gunNo,CTRL_OFF); /*运行指示灯 */
		 
				ptrCtrl->u8ClearFg = 0; /*设置清除数据标志*/
		    memset(gPtrRunData[ptrCtrl->gunNo]->logic,0,sizeof(DEV_LOGIC_TYPE));
//		    gPtrRunData[ptrCtrl->gunNo]->logic->recsavefg = 0;
//		    gPtrRunData[ptrCtrl->gunNo]->logic->startfg = 0;
//		    gPtrRunData[ptrCtrl->gunNo]->logic->time = 0;
//		    gPtrRunData[ptrCtrl->gunNo]->logic->money = 0;
//		    gPtrRunData[ptrCtrl->gunNo]->logic->energy = 0;
		    ptrCtrl->u32WaitTicks = GetSystemTick();
		    RelayOut_DcConTactKmOut(PW_CTRL_OFF);
				Delay10Ms(200);
		    Bill_InitData(ptrCtrl->gunNo);
				SET_WORKSTEP(ptrCtrl->gunNo,STEP_IDEL);
				break;
		 default:
			 break;
	 }
}



//MSG_MOD_DELALLCMD;
/*启动充电*/
uint8 TaskMian_StartCharge(START_PARAM *startpara)
{
	uint8 tmp[2] = {0};
		CARD_INFO *PtrCard = TskCard_GetCardInfPtr();	  //patli 20190929

	DebugInfoByCon("TaskMian_StartCharge...");
  
	if( startpara->vailfg == 0 || gPtrRunData[startpara->gunNo]->logic->workstep != STEP_IDEL ) {
		if((START_BY_VIN != startpara->startby)||(gPtrRunData[startpara->gunNo]->logic->workstep != STEP_BRM_SUB)){
		  startpara->vailfg = 0;
		  return 2;
		}
	}
	
	Bill_InitData(startpara->gunNo);
	
	//超级会员不限金额 patli 20190930
	if( PtrCard->cardtype != E_SUPER_CARD&&startpara->money < 100 ) {  
		  tmp[0] = startpara->gunNo;
			tmp[1] = CARD_NOMONEY_ERR;
			SendMsgWithNByte(MSG_VERIFY_RESULT,2,&tmp[0],APP_TASK_SCREEN_PRIO);
			Delay10Ms(200);
			TaskMian_SendProcessMsg(startpara->gunNo,IDEL,0,0,0);
		  startpara->vailfg = 0;
		 return 2;
	}
	
	switch(startpara->chgmod) 
	{
		case CHG_AUTO_TYPE:
			gPtrRunData[startpara->gunNo]->logic->chgmod = CHG_AUTO_TYPE;
			break;
		case CHG_TIME_TYPE:
			gPtrRunData[startpara->gunNo]->logic->chgmod = CHG_TIME_TYPE;
		  gPtrRunData[startpara->gunNo]->logic->settime = startpara->settime;
			break;
		case CHG_MONEY_TYPE:
			gPtrRunData[startpara->gunNo]->logic->chgmod = CHG_MONEY_TYPE;
		  gPtrRunData[startpara->gunNo]->logic->setmoney = startpara->setmoney;
			break;
		case CHG_ENERGY_TYPE:
			gPtrRunData[startpara->gunNo]->logic->chgmod = CHG_ENERGY_TYPE;
		  gPtrRunData[startpara->gunNo]->logic->setenergy = startpara->setenergy;
			break;
		default:
			gPtrRunData[startpara->gunNo]->logic->chgmod = CHG_AUTO_TYPE;
			break;
	}
	
	switch(startpara->startby)
	{
		case START_BY_CARD:
			gPtrRunData[startpara->gunNo]->logic->startby = START_BY_CARD;
			gPtrRunData[startpara->gunNo]->logic->chgmod = startpara->chgmod;
	    memcpy(gPtrRunData[startpara->gunNo]->logic->account,TskCard_GetCardInfPtr()->cardNo,16);
		  memcpy(gPtrRunData[startpara->gunNo]->bill->cardNo,TskCard_GetCardInfPtr()->cardNo,16);
			break;
		case START_BY_BKGROUND:
			gPtrRunData[startpara->gunNo]->logic->startby = START_BY_BKGROUND;		
			break;
		case START_BY_PASSWD:
			gPtrRunData[startpara->gunNo]->logic->startby = START_BY_PASSWD;		
			break;
		case START_BY_ONLINECARD:
			gPtrRunData[startpara->gunNo]->logic->startby = START_BY_ONLINECARD;
	
			break;
		case START_BY_VIN:
			gPtrRunData[startpara->gunNo]->logic->startby = START_BY_VIN;
			break;
		case START_BY_GUN:
			gPtrRunData[startpara->gunNo]->logic->startby = START_BY_GUN;
			break;
		case START_BY_KEY:
			gPtrRunData[startpara->gunNo]->logic->startby = START_BY_KEY;
			break;
		default:
			break;
	}
	
	Bill_SetStartInfo(startpara);
	Bill_SetStartData(startpara->gunNo);
	Delay10Ms(1);

	START_CHARGE(startpara->gunNo);

	start_charge_try_num++; //重启充电次数增1
	
	startpara->vailfg = 0;
	return 1;
}

#ifdef TRY_START_CHARGIN

//重新启动充电
uint8 TaskMian_TryStartCharge(CTRL_STEP *ptrCtrl, START_PARAM *startpara)
{
	
	DebugInfoByCon("TaskMian_TryStartCharge...");
	
	if(start_charge_try_num < 3)   //重启次数小于3
	{		
		/* 关闭直流接触器 */ 
		RelayOut_DcKmOut(ptrCtrl->gunNo,KM_OFF);
		Delay10Ms(5);
		RelayOut_AssistPower(ptrCtrl->gunNo,ASSIST_POWER_OFF); //关闭辅源
		Delay10Ms(5);		
		RelayOut_DcConTactKmOut(PW_CTRL_OFF);
		DelaySec(2);		
		
		TaskMian_StartCharge(startpara);
	}
	else
	{		
		start_charge_try_num = 0;
		SET_STOPING(ptrCtrl->gunNo,STEP_CHGEND,UNLOCKED_END); /*退出*/
	}
	
	return 0;
}

#endif

/*停止充电*/
void TaskMain_StopCharge(uint8 gunNo,uint8 reason,uint8 errCode)
{
	/* 关模块 */
	uint8 msg[2];
	msg[0] = gunNo;
	msg[1] = MOD_CTRL_OFF;

	DebugInfoByCon("TaskMain_StopCharge...");
	
	SendMsgWithNByte(MSG_MOD_ONOFF,2,&msg[0],APP_TASK_DLMOD_PRIO);
	
	STOP_CHARGE(gunNo);
	gPtrRunData[gunNo]->logic->stopReason = reason;
	gPtrRunData[gunNo]->logic->errCode = errCode;
	
}


#ifdef A_B_GUN_TOGETER
extern ScreenLocalPara_t  sGlocalPara;
#endif

uint8 TaskMain_CheckCardInfo(uint8 gunNo)
{
	
	u32toc_u u2c;
	uint8 data[8] = {0};
	CARD_INFO *ptrCardinfo = TskCard_GetCardInfPtr();
   PARAM_DEV_TYPE *ptrDevPara = ChgData_GetDevParamPtr();
	 PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();    //patli 20190923 add
	
		if( STEP_IDEL == gPtrRunData[gunNo]->logic->workstep) { 
			 if( 0 == memcmp(conDefusrpin,ptrCardinfo->userPin\
					,sizeof(ptrCardinfo->userPin)) || \
					0 == memcmp(gPtrRunData[gunNo]->logic->usrpin,ptrCardinfo->userPin\
					,sizeof(ptrCardinfo->userPin))) {
						if( ptrCardinfo->cardtype == E_MANG_CARD ) { /*管理卡*/
							if(STEP_IDEL == gPtrRunData[(gunNo+1)%DEF_MAX_GUN_NO]->logic->workstep ) {
								SendMsgWithNByte(MSG_MANAGE_CARD ,0,NULL,APP_TASK_SCREEN_PRIO);		
							}
							return TRUE;
						}		
            			data[0] = gunNo;							
						data[1] = 1;	
						
						/***澄鹏管理卡判断，不受余额限制 patli 20190926*****/
						if( (ptrCardinfo->money > 100) || ( ptrDevPara->onlinecard == E_ONLINE_CARD )
							||(ptrCardinfo->cardtype == E_SUPER_CARD)) {		

#ifdef A_B_GUN_TOGETER
							if(sGlocalPara.gun_no==gunNo&&gPtrRunData[gunNo]->gun->statu.bits.cc1stu==CC1_4V)
#endif								
								SendMsgWithNByte(MSG_LOCKED_CARD,2,&data[0],APP_TASK_CARD_PRIO);	
					
						}else {
							data[1] = CARD_NOMONEY_ERR;
							SendMsgWithNByte(MSG_VERIFY_RESULT,2,&data[0],APP_TASK_SCREEN_PRIO);
							Delay10Ms(200);
							TaskMian_SendProcessMsg(gunNo,IDEL,0,0,0);
						}	              								
				}else { /*密码错误*/
					SendMsgWithNByte(MSG_RESQ_PASSWD,0,NULL,APP_TASK_SCREEN_PRIO);
				}								
		}else if (STEP_IDEL != gPtrRunData[gunNo]->logic->workstep ) {
			
			if(STEP_CHARGEING == gPtrRunData[gunNo]->logic->workstep || STEP_CHGEND == gPtrRunData[gunNo]->logic->workstep \
			 || STEP_WAITOVER == gPtrRunData[gunNo]->logic->workstep || STEP_WAIT_SUB == gPtrRunData[gunNo]->logic->workstep) {
				 /*结算扣费*/
					if( 0 ==  memcmp(gPtrRunData[gunNo]->bill->cardNo,ptrCardinfo->cardNo,sizeof(ptrCardinfo->cardNo))) {
						 data[0] = gunNo;
						 TskCReader_CpyLockCardNo(ptrCardinfo->cardNo);
						
						 u2c.i = gPtrRunData[gunNo]->bill->billmoney;
						 if(  ptrDevPara->onlinecard == E_ONLINE_CARD ) {
							 u2c.i = 0;                 						 
						 }
						
						 memcpy(&data[1],(int *)&u2c.i,4);
						
						 TskCReader_CpyLockCardNo(ptrCardinfo->cardNo);
						 if( 0 == Bill_GetBillData(gunNo)->IsPay ) {
								SendMsgWithNByte(MSG_UNLOCK_CARD,5,&data[0],APP_TASK_CARD_PRIO);
								Delay10Ms(1);
						 }else {
								data[0] = gunNo;
								data[1] = 1;
								SendMsgWithNByte(MSG_UNLOCK_RESULT ,2,&data[0],APP_TASK_SCREEN_PRIO);	
								TaskMian_SendProcessMsg(gunNo,ACCOUNT,0,0,Bill_GetBillData(gunNo)->IsPay);
						 }
					}else {
						/*非启动卡*/
						data[0] = gunNo;
						data[1] = 2;
						SendMsgWithNByte(MSG_UNLOCK_RESULT ,2,&data[0],APP_TASK_SCREEN_PRIO);	
					}
				}/*没启动成功*/
			  else {
					data[0] = gunNo;
					data[1] = CARD_NOTIDLE_ERR;
					SendMsgWithNByte(MSG_VERIFY_RESULT,2,&data[0],APP_TASK_SCREEN_PRIO);
			  }
		}
		return TRUE;
}

extern uint8 Rd_SaveCpPeriodFeeParam(uint8 devtype);
extern CTRL_STEP  gunACtrl; /*记得初始化*/
extern CTRL_STEP  gunBCtrl; /*记得初始化*/

/*消息处理*/
#ifdef A_B_GUN_TOGETER
void TaskMain_DealMsg()
{
		uint8 gun_no;
		MSG_STRUCT msg;
		uint8 data[8] = {0};
		memset(&msg,0,sizeof(MSG_STRUCT));
		CARD_INFO *ptrCardinfo = TskCard_GetCardInfPtr();
	
	
		if(RcvMsgFromQueue(&msg) == TRUE) {
				switch(msg.MsgType)
				{
					case MSG_TO_ENTER_IDLE:
	//					DevEnterIdleStatus(&gunACtrl);
						
	//					DevEnterIdleStatus(&gunBCtrl);
						break;
					case MSG_PARAM_INPUT:
						if( PARA_SCREEN == msg.MsgData[0] ) {
							/* 刷卡器开始寻卡 */
							SendMsgWithNByte(MSG_CHECK_CARD,0,NULL,APP_TASK_CARD_PRIO);
						}
						break;
					case MSG_START_STOP:  /*充电开始控制,用于APP、触摸屏、按钮等操作*/ 
						gun_no = msg.MsgData[1];
						DebugInfoWithPbuf(CON_MOD, "TaskMain_DealMsg START_STOP",msg.MsgData, msg.MsgLenth,0);
						/*发送启动消息前，需要完成账户信息的填写*/
					  if( 1 == msg.MsgData[0] ) {  //启动
							if( STEP_IDEL == gPtrRunData[gun_no]->logic->workstep) {
								TaskMian_StartCharge(ChgData_GetStartParaPtr(gun_no));
								memcpy(gPtrRunData[gun_no]->logic->usrpin,conDefusrpin,sizeof(gPtrRunData[gun_no]->logic->usrpin));
								data[0] = 1; /*成功，失败 */
								data[1] = 1;//TaskMian_StartCharge(PtrStartPara); /* 1启动成功	2启动失败  */
								data[2] = gun_no;
								SendMsgWithNByte(MSG_START_STOP_RESULT,3,&data[0],APP_TASK_BACK_PRIO);		
							}else if((STEP_BRM_SUB == gPtrRunData[gun_no]->logic->workstep)\
									 &&(START_BY_VIN == gPtrRunData[gun_no]->logic->startby||START_BY_ONLINECARD == gPtrRunData[gun_no]->logic->startby))
							{
								TaskMian_StartCharge(ChgData_GetStartParaPtr(gun_no));
								memcpy(gPtrRunData[gun_no]->logic->usrpin,conDefusrpin,sizeof(gPtrRunData[gun_no]->logic->usrpin));
							}
							else {
								data[0] = 1; /*成功，失败*/
								data[1] = 2; /*失败原因*/
								data[2] = gun_no;
								SendMsgWithNByte(MSG_START_STOP_RESULT,3,&data[0],APP_TASK_BACK_PRIO);		
							}
						}else if( 0 == msg.MsgData[0] ) {
							if( STEP_CHARGEING == gPtrRunData[gun_no]->logic->workstep) {  //2019-04-26
								if(0 == msg.MsgData[2] ){  //0=按键  1=后台  2=后台通讯故障5分钟	
									TaskMain_StopCharge(gun_no,KEY_REASON,ECODE_NORMAL);
								}
								else if(1 == msg.MsgData[2] ){
								TaskMain_StopCharge(gun_no,EAPP_REASON,ECODE_NORMAL);
								}
								else if(2 == msg.MsgData[2] ){
								TaskMain_StopCharge(gun_no,EERR_REASON,ECODE107_BACKOFF);
									Check_SetErrCode(gun_no,ECODE107_BACKOFF);
								}
							}
						}
						break;	
					case MSG_ISO_FINISH:			 /*绝缘完成*/
						//A枪处理
						if( GET_WORKSTEP(AGUN_NO) == STEP_ISOWAIT_SUB) {
							SET_WORKSTEP(AGUN_NO,STEP_ISOFINISH_SUB);
						}
						if( GET_WORKSTEP(BGUN_NO)== STEP_ISOWAIT_SUB) {
							SET_WORKSTEP(BGUN_NO,STEP_ISOFINISH_SUB);
						}
						break;
					case MSG_CARD_INFO: 		   /*检测到有效卡*/
						if( 1 == msg.MsgData[0] ) {
							//A枪处理
							if ( CC1_4V != gPtrRunData[AGUN_NO]->gun->statu.bits.cc1stu \
								&&CC1_4V != gPtrRunData[BGUN_NO]->gun->statu.bits.cc1stu \
								&& (ptrCardinfo->cardtype == E_USER_CARD ) ) {
								data[0] = AGUN_NO;
								data[1] = RES_FAIL; /*锁卡失败*/
								data[2] = 1;
								SendMsgWithNByte(MSG_LOCK_RESULT,3,&data[0],APP_TASK_SCREEN_PRIO);	
								Delay10Ms(5); //100
								break;
							}

							//刷管理卡处理
							if(ptrCardinfo->cardtype == E_MANG_CARD)
							{
								TaskMain_CheckCardInfo(AGUN_NO);
								break;
							}
							
							if(gPtrRunData[AGUN_NO]->gun->statu.bits.cc1stu==CC1_4V)
							{	
								TaskMain_CheckCardInfo(AGUN_NO);
							}
							
							if(gPtrRunData[BGUN_NO]->gun->statu.bits.cc1stu==CC1_4V)
							{
								TaskMain_CheckCardInfo(BGUN_NO);
							}

							
						}else { //msg.MsgData[0] == 5
	//						 /*检测到无效卡，需要触摸屏那边给出提示*/
						}
						break;
					case MSG_GET_PASSWD:		 /*从触摸屏获取到密码*/
						//A枪处理
						if( STEP_IDEL == gPtrRunData[AGUN_NO]->logic->workstep) {
								memcpy(gPtrRunData[AGUN_NO]->logic->usrpin,msg.MsgData,3);
								SendMsgWithNByte(MSG_CHECK_CARD,0,NULL,APP_TASK_CARD_PRIO);
								TaskMain_CheckCardInfo(AGUN_NO);							
						 }

						if( STEP_IDEL == gPtrRunData[BGUN_NO]->logic->workstep) {
								memcpy(gPtrRunData[BGUN_NO]->logic->usrpin,msg.MsgData,3);
								SendMsgWithNByte(MSG_CHECK_CARD,0,NULL,APP_TASK_CARD_PRIO);
								TaskMain_CheckCardInfo(BGUN_NO);							
						 }
						
						break;
					case MSG_LOCK_RESULT:
						if( 1 == msg.MsgData[0] ) {/*锁卡*/
							 if( RES_SUCC ==  msg.MsgData[1] ) { /*成功*/
								 /*刷卡器开始寻卡(带参数延时时间,40s后寻卡)*/
								stoc_u tmps2c;
								tmps2c.s = 10;
								data[0] = 1;
								data[1] = tmps2c.c[0];
								data[2] = tmps2c.c[1];
								SendMsgWithNByte(MSG_CHECK_CARD,3,&data[0],APP_TASK_CARD_PRIO);
								 
								PARAM_DEV_TYPE *ptrDevPara = ChgData_GetDevParamPtr();

								//A枪处理
								if( STEP_IDEL == gPtrRunData[AGUN_NO]->logic->workstep &&gPtrRunData[AGUN_NO]->gun->statu.bits.cc1stu==CC1_4V) {
									if( ptrDevPara->onlinecard == E_ONLINE_CARD ) {
										 data[0] = AGUN_NO;
										 data[1] = ptrCardinfo->cardtype;	//patli 20190929 add card type
											
										 SendMsgWithNByte(MSG_ONLINE_CARD,2,&data[0],APP_TASK_BACK_PRIO);
											
	//------------------add0705zyf	
										 ChgData_GetStartParaPtr(AGUN_NO)->startby = START_BY_ONLINECARD;
										 Delay10Ms(1);
										 SendMsgWithNByte(MSG_LOCK_RESULT,3,&data[0],APP_TASK_SCREEN_PRIO); 
	//------------------	
											
									}else {
											data[0] = AGUN_NO;
											data[1] = RES_SUCC;
											data[2] = 0;
											SendMsgWithNByte(MSG_LOCK_RESULT,3,&data[0],APP_TASK_SCREEN_PRIO);	
											Delay10Ms(10);
									}
								 }
								
								
								//B枪处理
								 if( STEP_IDEL == gPtrRunData[BGUN_NO]->logic->workstep&&gPtrRunData[BGUN_NO]->gun->statu.bits.cc1stu==CC1_4V) {
									if( ptrDevPara->onlinecard == E_ONLINE_CARD ) {
										data[0] = BGUN_NO;
										data[1] = ptrCardinfo->cardtype;	 //patli 20190929 add card type
																	 
										SendMsgWithNByte(MSG_ONLINE_CARD,2,&data[0],APP_TASK_BACK_PRIO);
																	 
							 //------------------add0705zyf  
										ChgData_GetStartParaPtr(BGUN_NO)->startby = START_BY_ONLINECARD;
										Delay10Ms(1);
										SendMsgWithNByte(MSG_LOCK_RESULT,3,&data[0],APP_TASK_SCREEN_PRIO); 
							 //------------------	 
									}else {
										 data[0] = BGUN_NO;
										 data[1] = RES_SUCC;
										 data[2] = 0;
										 SendMsgWithNByte(MSG_LOCK_RESULT,3,&data[0],APP_TASK_SCREEN_PRIO);  
										 Delay10Ms(10);
									}
								}
								
							 }
							 else {
									;
							 }
						}
						break;	
					case MSG_UNLOCK_RESULT:
						if( 0 == msg.MsgData[0] ) { /*解卡*/
							if( RES_SUCC ==  msg.MsgData[1] ) { /*解卡成功*/
								gun_no = msg.MsgData[2];
								msg.MsgData[0] = gun_no;
								if(STEP_CHARGEING == gPtrRunData[gun_no]->logic->workstep) {
								 TaskMain_StopCharge(gun_no,ECARD_REASON,ECODE_NORMAL);
								 /*结算成功*/							 
								 Bill_GetBillData(gun_no)->IsPay = 1;
								 SendMsgWithNByte(MSG_UNLOCK_RESULT ,2,&msg.MsgData[0],APP_TASK_SCREEN_PRIO);	
								}else if( STEP_WAITOVER == gPtrRunData[gun_no]->logic->workstep) {
								 Bill_GetBillData(gun_no)->IsPay = 1;
								 SendMsgWithNByte(MSG_UNLOCK_RESULT ,2,&msg.MsgData[0],APP_TASK_SCREEN_PRIO);	
								 TaskMian_SendProcessMsg(gun_no,ACCOUNT,0,0,Bill_GetBillData(gun_no)->IsPay);
								}

#if 0
								//B枪处理
								msg.MsgData[0] = BGUN_NO;
								if(STEP_CHARGEING == gPtrRunData[BGUN_NO]->logic->workstep) {
								 TaskMain_StopCharge(BGUN_NO,ECARD_REASON,ECODE_NORMAL);
								 /*结算成功*/							 
								 Bill_GetBillData(BGUN_NO)->IsPay = 1;
								 SendMsgWithNByte(MSG_UNLOCK_RESULT ,2,&msg.MsgData[0],APP_TASK_SCREEN_PRIO);	
								}else if( STEP_WAITOVER == gPtrRunData[BGUN_NO]->logic->workstep) {
								 Bill_GetBillData(BGUN_NO)->IsPay = 1;
								 SendMsgWithNByte(MSG_UNLOCK_RESULT ,2,&msg.MsgData[0],APP_TASK_SCREEN_PRIO);	
								 TaskMian_SendProcessMsg(BGUN_NO,ACCOUNT,0,0,Bill_GetBillData(BGUN_NO)->IsPay);
								}
#endif

							}
							else {
								;
							}
						}
						break;
					case MSG_PARAM_STORE:
						if(msg.MsgData[0] == SAVE_ALL_TYPE ) {
							Rd_SaveOperParam(FM_IC);
							Delay10Ms(2);
							Rd_SaveDevParam(FM_IC);
							Delay10Ms(2);
							Rd_SaveFeePara(FM_IC);
							Delay10Ms(2);
						}else if(msg.MsgData[0] == SAVE_OPER_TYPE) {
							Rd_SaveOperParam(FM_IC);
							Delay10Ms(2);
						}else if(msg.MsgData[0] == SAVE_DEV_TYPE){
							Rd_SaveDevParam(FM_IC);
							Delay10Ms(2);
						}else if(msg.MsgData[0] == SAVE_FEE_TYPE ){
							Rd_SaveFeePara(FM_IC);
							Delay10Ms(2);
						}
					#ifdef CP_PERIOD_FEE
						else if(msg.MsgData[0] == SAVE_CP_PERIOD_FEE_TYPE ){
							Rd_SaveCpPeriodFeeParam(FM_IC);
							Delay10Ms(2);
						}
					#endif
						else {
							;
						}
						break;
					case MSG_PARAM_CLEAR:
			 		break; 
					case MSG_RECORD_CLEAR:
						Rd_ClearE2romRecordData();
			 			break; 				
					default:
						break;
				}
	  }

}

#else
void TaskMain_DealMsg(uint8 gunNo)
{
	uint8 prio;
	MSG_STRUCT msg;
	uint8 data[8] = {0};
	memset(&msg,0,sizeof(MSG_STRUCT));
  	START_PARAM *PtrStartPara = ChgData_GetStartParaPtr(gunNo);
  	CARD_INFO *ptrCardinfo = TskCard_GetCardInfPtr();

#if 0	//patli 20200207
	if( AGUN_NO == gunNo ) {
		prio = APP_TASK_AGUNMAINCTRL_PRIO;
	}else if( BGUN_NO == gunNo ) {
		prio = APP_TASK_BGUNMAINCTRL_PRIO;
	}else {
		prio = OSTCBCur->OSTCBPrio;
	}
	
	if(RcvPrioMsgFromQueue(&msg,prio) == TRUE) {
#else	
	if(RcvMsgFromQueue(&msg) == TRUE) {
#endif
			switch(msg.MsgType)
			{
				case MSG_PARAM_INPUT:
					if( PARA_SCREEN == msg.MsgData[0] ) {
						/* 刷卡器开始寻卡 */
						SendMsgWithNByte(MSG_CHECK_CARD,0,NULL,APP_TASK_CARD_PRIO);
					}
					break;
				case MSG_START_STOP:  /*充电开始控制,用于APP、触摸屏、按钮等操作*/ 
					
					DebugInfoWithPbuf(CON_MOD, "TaskMain_DealMsg START_STOP",msg.MsgData, msg.MsgLenth,0);
					/*发送启动消息前，需要完成账户信息的填写*/
				  if( 1 == msg.MsgData[0] ) {  //启动
						if( STEP_IDEL == gPtrRunData[gunNo]->logic->workstep) {
							TaskMian_StartCharge(PtrStartPara);
							memcpy(gPtrRunData[gunNo]->logic->usrpin,conDefusrpin,sizeof(gPtrRunData[gunNo]->logic->usrpin));
							data[0] = 1; /*成功，失败 */
							data[1] = 1;//TaskMian_StartCharge(PtrStartPara); /* 1启动成功  2启动失败  */
							data[2] = gunNo;
							SendMsgWithNByte(MSG_START_STOP_RESULT,3,&data[0],APP_TASK_BACK_PRIO);		
						}else if((STEP_BRM_SUB == gPtrRunData[gunNo]->logic->workstep)\
						         &&(START_BY_VIN == gPtrRunData[gunNo]->logic->startby||START_BY_ONLINECARD == gPtrRunData[gunNo]->logic->startby))
						{
							TaskMian_StartCharge(PtrStartPara);
							memcpy(gPtrRunData[gunNo]->logic->usrpin,conDefusrpin,sizeof(gPtrRunData[gunNo]->logic->usrpin));
						}
						else {
							data[0] = 1; /*成功，失败*/
							data[1] = 2; /*失败原因*/
							data[2] = gunNo;
							SendMsgWithNByte(MSG_START_STOP_RESULT,3,&data[0],APP_TASK_BACK_PRIO);		
						}
					}else if( 0 == msg.MsgData[0] ) {
						if( STEP_CHARGEING == gPtrRunData[gunNo]->logic->workstep) {  //2019-04-26
							if(0 == msg.MsgData[2] ){  //0=按键  1=后台  2=后台通讯故障5分钟	
								TaskMain_StopCharge(gunNo,KEY_REASON,ECODE_NORMAL);
							}
							else if(1 == msg.MsgData[2] ){
						    TaskMain_StopCharge(gunNo,EAPP_REASON,ECODE_NORMAL);
							}
							else if(2 == msg.MsgData[2] ){
						    TaskMain_StopCharge(gunNo,EERR_REASON,ECODE107_BACKOFF);
								Check_SetErrCode(gunNo,ECODE107_BACKOFF);
							}
						}
					}
					break;	
				case MSG_ISO_FINISH:             /*绝缘完成*/
					if( GET_WORKSTEP(gunNo) < STEP_ISOFINISH_SUB) {
						SET_WORKSTEP(gunNo,STEP_ISOFINISH_SUB);
					}
					break;
				case MSG_CARD_INFO:            /*检测到有效卡*/
					if( 1 == msg.MsgData[0] ) {
							if ( CC1_4V != gPtrRunData[gunNo]->gun->statu.bits.cc1stu \
								&& (ptrCardinfo->cardtype == E_USER_CARD ) ) {
								data[0] = gunNo;
								data[1] = RES_FAIL; /*锁卡失败*/
								data[2] = 1;
								SendMsgWithNByte(MSG_LOCK_RESULT,3,&data[0],APP_TASK_SCREEN_PRIO);	
								Delay10Ms(5); //100
								break;
							}
						  TaskMain_CheckCardInfo(gunNo);
           	}else { //msg.MsgData[0] == 5
//						 /*检测到无效卡，需要触摸屏那边给出提示*/
					}
					break;
				case MSG_GET_PASSWD:         /*从触摸屏获取到密码*/
					if( STEP_IDEL == gPtrRunData[gunNo]->logic->workstep) {
							memcpy(gPtrRunData[gunNo]->logic->usrpin,msg.MsgData,3);
							SendMsgWithNByte(MSG_CHECK_CARD,0,NULL,APP_TASK_CARD_PRIO);
							TaskMain_CheckCardInfo(gunNo);							
					 }
					break;
				case MSG_LOCK_RESULT:
					if( 1 == msg.MsgData[0] ) {/*锁卡*/
						 if( RES_SUCC ==  msg.MsgData[1] ) { /*成功*/
							 /*刷卡器开始寻卡(带参数延时时间,40s后寻卡)*/
							  stoc_u tmps2c;
								tmps2c.s = 10;
								data[0] = 1;
								data[1] = tmps2c.c[0];
								data[2] = tmps2c.c[1];
								SendMsgWithNByte(MSG_CHECK_CARD,3,&data[0],APP_TASK_CARD_PRIO);
							 
							  PARAM_DEV_TYPE *ptrDevPara = ChgData_GetDevParamPtr();
							 
							 if( STEP_IDEL == gPtrRunData[gunNo]->logic->workstep) {
								  if( ptrDevPara->onlinecard == E_ONLINE_CARD ) {
									 data[0] = gunNo;
									 data[1] = ptrCardinfo->cardtype;   //patli 20190929 add card type
										
									 SendMsgWithNByte(MSG_ONLINE_CARD,2,&data[0],APP_TASK_BACK_PRIO);
										
//------------------add0705zyf	
									 PtrStartPara->startby = START_BY_ONLINECARD;
									 Delay10Ms(1);
									 SendMsgWithNByte(MSG_LOCK_RESULT,3,&data[0],APP_TASK_SCREEN_PRIO); 
//------------------	
										
									}else {
								    data[0] = gunNo;
										data[1] = RES_SUCC;
								    data[2] = 0;
										SendMsgWithNByte(MSG_LOCK_RESULT,3,&data[0],APP_TASK_SCREEN_PRIO);	
								    Delay10Ms(10);
									}
							 }
						 }else {
								;
						 }
					}
					break;	
				case MSG_UNLOCK_RESULT:
					if( 0 == msg.MsgData[0] ) { /*解卡*/
						msg.MsgData[0] = gunNo;
						if( RES_SUCC ==  msg.MsgData[1] ) { /*解卡成功*/
							if(STEP_CHARGEING == gPtrRunData[gunNo]->logic->workstep) {
							 TaskMain_StopCharge(gunNo,ECARD_REASON,ECODE_NORMAL);
							 /*结算成功*/							 
							 Bill_GetBillData(gunNo)->IsPay = 1;
							 SendMsgWithNByte(MSG_UNLOCK_RESULT ,2,&msg.MsgData[0],APP_TASK_SCREEN_PRIO);	
							}else if( STEP_WAITOVER == gPtrRunData[gunNo]->logic->workstep) {
							 Bill_GetBillData(gunNo)->IsPay = 1;
							 SendMsgWithNByte(MSG_UNLOCK_RESULT ,2,&msg.MsgData[0],APP_TASK_SCREEN_PRIO);	
							 TaskMian_SendProcessMsg(gunNo,ACCOUNT,0,0,Bill_GetBillData(gunNo)->IsPay);
							}
						}else {
							;
						}
					}
					break;
        case MSG_PARAM_STORE:
					if(msg.MsgData[0] == SAVE_ALL_TYPE ) {
							Rd_SaveOperParam(FM_IC);
							Delay10Ms(2);
							Rd_SaveDevParam(FM_IC);
							Delay10Ms(2);
							Rd_SaveFeePara(FM_IC);
							Delay10Ms(2);
					}else if(msg.MsgData[0] == SAVE_OPER_TYPE) {
						  Rd_SaveOperParam(FM_IC);
						  Delay10Ms(2);
					}else if(msg.MsgData[0] == SAVE_DEV_TYPE){
						  Rd_SaveDevParam(FM_IC);
						  Delay10Ms(2);
					}else if(msg.MsgData[0] == SAVE_FEE_TYPE ){
						  Rd_SaveFeePara(FM_IC);
						  Delay10Ms(2);
					}
					#ifdef CP_PERIOD_FEE
					else if(msg.MsgData[0] == SAVE_CP_PERIOD_FEE_TYPE ){
						  Rd_SaveCpPeriodFeeParam(FM_IC);
						  Delay10Ms(2);
					}
					#endif
					else {
						;
					}
					break;
				case MSG_PARAM_CLEAR:
         break;	
        case MSG_RECORD_CLEAR:
					Rd_ClearE2romRecordData();
         break;					
				default:
					break;
			}
  }
}
#endif	






