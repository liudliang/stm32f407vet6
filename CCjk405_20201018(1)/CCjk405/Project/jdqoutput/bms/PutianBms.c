/**
  ******************************************************************************
  * @file    PutianBms.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#include "common.h"
#include "Bms.h"
#include "GbtBmsProto.h"
#include "PutianBms.h"

#include "ChgData.h"

#include "TaskMainCtrl.h"
//#include "M_Global.h"
#include "rtc.h"	
#include "message.h"
#include <string.h>

extern PARA_BMSCTRL gBmsCtrl[DEF_MAX_GUN_NO];

#define CURR_PROTO_PUTIAN(a) (8000 > (a) ? 8000-(a) : 0)

PT_BMS gPtBmsData;

extern uint16 gStepCurrent;

PT_BMS * PtBms_GetBmsDataPtr(uint8 gunNo)
{
	return &gPtBmsData;
}


void ProcPtBRM(void *item,uint8 *data,uint8 GunNo)
{	 
	
	
	
//	 BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(GunNo) ;
//	 const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
//	 if ( (NULL == ptrItem) || (ptrItem->pgn != BRM_CODE)) 
//   { 
//       return; 
//   }
//	  memcpy(&ptr->brm,data,sizeof(BRM_ST));
//	 
//	  if( GET_WORKSTEP() == STEP_BRM_WAIT_SUB ) {
//			SET_WORKSTEP(STEP_BRM_SUB);
//		}
//	 /*发送CRM AA*/
//	 if( 0 < Gbt_ReportSingPkg(GunNo,CRM_CODE,SPN_READYOK)) {
//		  ((CHGDATA_ST *)Bms_GetChgDataPtr(GunNo))->crm.idenf = SPN_READYOK;
//		  Gbt_CtrlCricle(GunNo,CRM_CODE,CRICLE_ALLOWED,1);
//	 }
	
	 return;
}

void ProcPutianBCP(void *item,uint8 *data,uint8 gunNo)
{
	 stoc_u tmp16;
	 BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(gunNo) ;
	 const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	 if ( (NULL == ptrItem) || (ptrItem->pgn != BCP_PTCODE)) 
   { 
       return; 
   }
	 //memcpy(&ptr->bcp,data,sizeof(BCP_ST));
	 tmp16.c[0] = data[0];
	 tmp16.c[1] = data[1];
	 ptr->bcp.sbatmaxvolt = tmp16.s;
	 
	 tmp16.c[0] = data[2];
	 tmp16.c[1] = data[3];
	 ptr->bcp.maxcurr = CURR_PROTO_PUTIAN(tmp16.s); 

	 tmp16.c[0] = data[5];
	 tmp16.c[1] = data[6];
	 ptr->bcp.chgmaxvolt = tmp16.s;
	 
	 ptr->bcp.tempmax = data[7];
	 
	 tmp16.c[0] = data[8];
	 tmp16.c[1] = data[9];
	 ptr->bcp.batsoc = tmp16.s;
	 
	 tmp16.c[0] = data[10];
	 tmp16.c[1] = data[11];
	 ptr->bcp.batcurvolt = tmp16.s;
	
	 if( GET_WORKSTEP(gunNo) == STEP_BCPWAIT_SUB ) {
			SET_WORKSTEP(gunNo,STEP_BCP_SUB);
		  Delay5Ms(1);
	 }
	 
	 /* 步进电流清零*/
	 gStepCurrent = 0;
	 return;
}

void ProcPutianBRO(void *item,uint8 *data,uint8 GunNo)
{
	 BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(GunNo) ;
	 const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	 if ( (NULL == ptrItem) || (ptrItem->pgn != BRO_PTCODE)) 
   { 
       return; 
   }
	 //memcpy(&ptr->bro,data,sizeof(BRO_ST));
	 if(data[0] != 0x04) {
		ptr->bro.brostu = BRO_READYNO;  /*未准备好*/
	 }else {
		 ptr->bro.brostu = BRO_READYOK;
	 }
	 
	 if (BRO_READYOK == ptr->bro.brostu) { 
		 if( GET_WORKSTEP(GunNo) == STEP_BROWAIT_SUB ) {
		  SET_WORKSTEP(GunNo,STEP_BRO_SUB);
		}
	 }
	 return;
}


extern void ProBmsStepAdjustCurr(void);
/*BMS需求报文*/
void ProcPutianBCL(void *item,uint8 *data,uint8 gunNo)
{
	 BCL_ST tmpbcl;
	 PARAM_OPER_TYPE *ptrRunPara = ChgData_GetRunParamPtr();
	 BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(gunNo) ;
	 const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	 if ( (NULL == ptrItem) || (ptrItem->pgn != BCL_PTCODE)) 
   { 
			return; 
   }
	 Bms_SetStepErrFg(gunNo,BCL_TM,0);
	 Bms_StartTimeCount(gunNo,BCL_TM,GetSystemTick());
	 Bms_StartTimeCount(gunNo,BCS_TM,GetSystemTick());
	 
	 stoc_u tmp16;
	 tmp16.c[0] = data[0];
	 tmp16.c[1] = data[1];
	 tmpbcl.needvolt = tmp16.s;/*电压扩大10倍*/
	 
	 if( tmpbcl.needvolt > ptr->bcp.chgmaxvolt || tmpbcl.needvolt > ptrRunPara->maxvolt+50 \
		 || tmpbcl.needvolt < ptrRunPara->minvolt ){ /*最高需求大于 > 755V  <100V 认为数据出错*/
			 return;
	 }
	 
	 tmp16.c[0] = data[2];
	 tmp16.c[1] = data[3];
	 tmpbcl.needcurr =  CURR_PROTO_PUTIAN(tmp16.s);    //(4000 > tmp16.s ) ? (4000 - tmp16.s) : 0; /*电流扩大10倍*/
	 tmpbcl.chgmode = data[4];
	 if( tmpbcl.needcurr > 8000 || tmpbcl.needcurr > ptr->bcp.maxcurr ) {
			 return;
	 }
	 
	 
	 memcpy(&ptr->bcl,&tmpbcl,sizeof(BCL_ST));
	 
	 uint16 u16TempCurr,u16TempVolt;
	 if( GET_WORKSTEP(gunNo) == STEP_CHARGEING) {
		 u16TempCurr = abs(ptr->bcl.needcurr - gBmsCtrl[gunNo].u16NeedCurr);
		 u16TempVolt = abs(ptr->bcl.needvolt - gBmsCtrl[gunNo].u16NeedVolt);
		 
		 if( (GetSystemTick() - gBmsCtrl[gunNo].u32WaitTicks > TIM_NS(2)) \
			 || ( u16TempCurr > 50 ) || ( u16TempVolt > 50 ) ) {
				gBmsCtrl[gunNo].u16NeedCurr = ptr->bcl.needcurr;
     		gBmsCtrl[gunNo].u16NeedVolt = ptr->bcl.needvolt;	
        gBmsCtrl[gunNo].u32WaitTicks = GetSystemTick();				 
		    SendMsgWithNByte(MSG_MODGRP_ADJUST,1,&gunNo,APP_TASK_DLMOD_PRIO);
		 }
	 }
	 
	return;
}

void ProcPutianBCS(void *item,uint8 *data,uint8 gunNo)
{
	BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(gunNo) ;
	const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	if ( (NULL == ptrItem) || (ptrItem->pgn != BCS_PTCODE)) 
	{ 
		 return; 
	}
	
	Bms_SetStepErrFg(gunNo,BCS_TM,0);
	
   Bms_StartTimeCount(gunNo,BCL_TM,GetSystemTick());
	 Bms_StartTimeCount(gunNo,BCS_TM,GetSystemTick());
	 stoc_u tmp16;
	 tmp16.c[0] = data[0];
	 tmp16.c[1] = data[1];
	 ptr->bcs.detecvolt = tmp16.s;/*电压扩大10倍*/

	 tmp16.c[0] = data[2];
	 tmp16.c[1] = data[3];
	 ptr->bcs.deteccurr = CURR_PROTO_PUTIAN(tmp16.s);
	
	 ptr->bcs.cursoc = data[6];
	 ptr->bcs.lasttimes = data[7];
	
	 	/*停止发送CRO*/
	if( GET_WORKSTEP(gunNo) != STEP_CHARGEING ) {
	 CHGDATA_ST *ptrBms = Bms_GetChgDataPtr(gunNo);
	 if( ptrBms->cro.crostu == CRO_READYOK ) {
		  Putian_CtrlCricle(gunNo,CRO_CODE,CRICLE_DISALLOWED,1);
		  ptrBms->cro.crostu  = CRO_READYNO;
	 }
	 Putian_CtrlCricle(gunNo,CCS_CODE,CRICLE_ALLOWED,1);
	}
  Delay5Ms(1);
	return;
}

void ProcPutianBSM(void *item,uint8 *data,uint8 gunNo)
{
	 BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(gunNo) ;
	 const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	 if ( (NULL == ptrItem) || (ptrItem->pgn != BSM_PTCODE)) 
   { 
       return; 
   }
	 
	 memset(&ptr->bsm,0,sizeof(BSM_ST));
	 stoc_u tmp16;
	 tmp16.c[0] = data[0];
	 tmp16.c[1] = data[1];
	 
   tmp16.s = data[3];
	 ptr->bsm.batmaxtemp = tmp16.s - 40; /*温度偏移40*/
	 tmp16.s = data[4];
	 ptr->bsm.batmintemp = tmp16.s - 40; /*温度偏移40*/
	 
	 
//	 memcpy(&ptr->bsm,data,sizeof(BSM_ST));
//	 ptr->bsm.batmaxtemp -= 50; /*温度偏移50*/
//	 ptr->bsm.batmintemp -= 50;
	 ptr->bsm.battalm.bits.singbatValm = data[5] & 0x01;
	 ptr->bsm.battalm.bits.socalm = data[5] & 0x04;
	 
	 if(ptr->bsm.battalm.bits.singbatValm != 0x00 || ptr->bsm.battalm.bits.socalm == 0x01 \
		 || ptr->bsm.battalm.bits.tempalm == 0x02 || ptr->bsm.battalm.bits.linkalm == 0x01 \
	   || ptr->bsm.battalm.bits.curralm == 0x01 || ptr->bsm.battalm.bits.insulalm == 0x01 ) {
			 if (ptr->bsm.battalm.bits.allowed == 0x00 ) { /*不允许充电*/
				 /*设置故障位*/
				 Bms_SetStepErrFg(gunNo,DISALLOW_FG,1);
				 /***************************************************************************
				    发送消息，告诉停机
				 *****************************************************************************/
			 } 
	 }else {
		 Bms_SetStepErrFg(gunNo,DISALLOW_FG,0);
	 }
	 return;
}


/*00 01 00 00 00 00 00 00  */
/*AA 01 00 00 00 00 00 00  */
/*
*老代码的逻辑
*如何收到了BRM 或者收到了VBI ,Data[3] = 1,
*不再发送CRM
*/
uint8 ChgSendPutianPkgCRM(uint8 *buf,void* st,uint8 gunNo)
{
	uint8 index = 0,res;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st; 
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(gunNo);
	res = 0;
	if((NULL != st) || (pCtrl->pf == CRM_PTCODE) ) {
		pCtrl->spn = pdata->crm.idenf;
		if( pCtrl->spn == SPN_READYOK ) {
			res = 1;
		}
		buf[index++] = 0x00;
		buf[index++] = 0x00;
		buf[index++] = 0x00;
		buf[index++] = res;
		buf[index++] = 0x00;
		buf[index++] = 0x00;
		buf[index++] = 0x00;
		buf[index++] = 0x00;
		pCtrl->gunNo = gunNo;
	}
	return index;	
}


uint8 ChgSendPutianPkgCRO(uint8 *buf,void* st,uint8 gunNo)
{
	uint8 index = 0;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st; 
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(gunNo);
	
	if( (NULL != st) || (pCtrl->pf == CRO_PTCODE) ) {
		/*未准备好 0x02 ,准备好 0x04*/
		pCtrl->spn = pdata->cro.crostu;
		if( CRO_READYOK == pCtrl->spn ) {
				buf[index++] = 0x04;
		}else {
				buf[index++] = 0x02;
		}			
			
		pCtrl->gunNo = gunNo;
	}
	return index;	
}



uint8 ChgSendPutianPkgCCS(uint8 *buf,void* st,uint8 gunNo)
{
	stoc_u tmp16;
	uint8 index = 0;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st; 
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(gunNo);
	
	if( (NULL != st) || (pCtrl->pf == CCS_PTCODE) ) {
		tmp16.s = VOLT_PROTO(pdata->ccs.outvolt);
		buf[index++] = tmp16.c[0];
		buf[index++] = tmp16.c[1];
		
		tmp16.s = CURR_PROTO_PUTIAN(pdata->ccs.outcurr);
		buf[index++] = tmp16.c[0];
		buf[index++] = tmp16.c[1];
		
		tmp16.s = pdata->ccs.totalchgtime;
		buf[index++] = tmp16.c[0];
		buf[index++] = tmp16.c[1];

		pCtrl->gunNo = gunNo;
	}
	return index;	
}

uint8 ChgSendPutianPkgCML(uint8 *buf,void* st,uint8 gunNo)
{
	uint8 index = 0;
	uint16 tmp16;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st; 
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(gunNo);
	
	if( (NULL != st) || (pCtrl->pf == CML_PTCODE)) {
		tmp16 = VOLT_PROTO(pdata->cml.maxoutvolt);
		buf[index++] = tmp16 & 0xff;
		buf[index++] = tmp16 >> 8;
		
		tmp16 = VOLT_PROTO(pdata->cml.minoutvolt);
		buf[index++] = tmp16 & 0xff;
		buf[index++] = tmp16 >> 8;
		
		tmp16 = CURR_PROTO_PUTIAN(pdata->cml.maxoutcur);
		buf[index++] = tmp16 & 0xff;
		buf[index++] = tmp16 >> 8;
		
		tmp16 = 0;
		buf[index++] = tmp16 & 0xff;
		buf[index++] = tmp16 >> 8;
		
		pCtrl->gunNo = gunNo;
	}
	return index;	
}



