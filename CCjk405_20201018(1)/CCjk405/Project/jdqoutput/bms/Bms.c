/**
  ******************************************************************************
  * @file    bms.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#include "ChgData.h"
#include "GbtBmsProto.h"
#include "common.h"
#include "Bms.h"
#include "TaskMainCtrl.h"
//#include "M_Global.h"
#include "rtc.h"	
#include "message.h"
#include "debug.h"
#include <string.h>

#include "Adebug.h"


extern void TaskMain_StopCharge(uint8 reason,uint8 errCode);

uint16 gStepCurrent = 0; 
PARA_BMSCTRL gBmsCtrl[DEF_MAX_GUN_NO];
static DEV_BMS_TYPE gBmsData[DEF_MAX_GUN_NO];

DEV_BMS_TYPE * Bms_GetBmsDataPtr(uint8 gunNo)
{
	ASSERT(gunNo < DEF_MAX_GUN_NO);
	return &gBmsData[gunNo < DEF_MAX_GUN_NO ? gunNo:0];
}
 
BMSDATA_ST *Bms_GetBmsCarDataPtr(uint8 gunNo) 
{
	ASSERT(gunNo < DEF_MAX_GUN_NO);
	return &gBmsData[gunNo < DEF_MAX_GUN_NO ? gunNo:0].car;
}
	
CHGDATA_ST *Bms_GetChgDataPtr(uint8 gunNo)
{
	ASSERT(gunNo < DEF_MAX_GUN_NO);
	return &gBmsData[gunNo < DEF_MAX_GUN_NO ? gunNo:0].chg;
}

ERR_STEP_ST *Bms_GetStepStPtr(uint8 gunNo)
{
	ASSERT(gunNo < DEF_MAX_GUN_NO);
	return &gBmsData[gunNo < DEF_MAX_GUN_NO ? gunNo:0].err;
}

/* 收到BMS的包标记位 */
static RCV_PACK_ST gBmsPack[DEF_MAX_GUN_NO];

RCV_PACK_ST *Bms_GetBmsPack(uint8 gunNo)
{
	return &gBmsPack[gunNo < DEF_MAX_GUN_NO ? gunNo:0];
}

void Bms_InitData(uint8 gunNo)
{
	memset(&gBmsData[gunNo],0,sizeof(DEV_BMS_TYPE));
  	memset(&gBmsCtrl[gunNo],0,sizeof(PARA_BMSCTRL));
	memset(&gBmsPack[gunNo],0,sizeof(RCV_PACK_ST));
	Bms_ClearAllTmoutFg(gunNo);
}

void Bms_InitErrData(uint8 gunNo)
{
	ASSERT(gunNo < DEF_MAX_GUN_NO);
	memset(&gBmsData[gunNo].car.bst,0,sizeof(BST_ST));
	memset(&gBmsData[gunNo].car.bsd,0,sizeof(BSD_ST));
	memset(&gBmsData[gunNo].car.bem,0,sizeof(BEM_ST));
}



void Bms_ClearAllTmoutFg(uint8 gunNo)
{
	 gBmsData[gunNo].err.stu.dword =  0;
}

/*获取故障号 1 ~ 31*/
uint8 Bms_GetErrCode(uint8 gunNo)
{
	uint8 i;
	for( i = 0 ;i < 32; i++ ) {
		if( gBmsData[gunNo].err.stu.dword & ((uint32)(1<< i) )) {
			break;
		}
	}
	i = ( i < 32 ) ? i: 0 ;
	return i;
}

uint32 Bms_GetStartTimeCount(uint8 gunNo,uint8 tg)
{
   uint32 rtn = 0;
	 ERR_STEP_ST * ptrErr = Bms_GetStepStPtr(gunNo);
	 
	 switch(tg) {
		 case LOCK_TM:
			 rtn = ptrErr->lockstart ;
			 break;
		 case BHM_TM:
			 rtn = ptrErr->bhmstart;
			 break;
		 case XF_TM:
			 rtn = ptrErr->xfstart;
			 break;
		 case BRM_TM:
			 rtn = ptrErr->brmstart;
			 break;
		 case BCP_TM:
			 rtn = ptrErr->bcpstart;
			 break;
		 case BRO_TM:
			 rtn = ptrErr->brostart;
			 break;
		 case BCL_TM:
			 rtn = ptrErr->bclstart;
			 break;
		 case BCS_TM:
			 rtn = ptrErr->bcsstart;
			 break;
		 case BST_TM:
			 rtn = ptrErr->bststart;
			 break;
		 case CST_TM:
			 rtn = ptrErr->cststart;
			 break;
		 case BSD_TM:
			 rtn = ptrErr->bsdstart;
			 break;
		 default:
			 break;
	 }
	return rtn;
}

void Bms_StartTimeCount(uint8 gunNo,uint8 tg,uint32 ticks)
{
	 ERR_STEP_ST * ptrErr = Bms_GetStepStPtr(gunNo);
	 switch(tg) {
		 case LOCK_TM:
			 ptrErr->lockstart = ticks;
			 break;
		 case BHM_TM:
			 ptrErr->bhmstart = ticks;
			 break;
		 case XF_TM:
			 ptrErr->xfstart = ticks;
			 break;
		 case BRM_TM:
			 ptrErr->brmstart = ticks;
			 break;
		 case BCP_TM:
			 ptrErr->bcpstart = ticks;
			 break;
		 case BRO_TM:
			 ptrErr->brostart = ticks;
			 break;
		 case BCL_TM:
			 ptrErr->bclstart = ticks;
			 break;
		  case BCS_TM:
			 ptrErr->bcsstart = ticks;
			 break;
		 case BST_TM:
			 ptrErr->bststart = ticks;
			 break;
		 case CST_TM:
			 ptrErr->bststart = ticks;
			 break;
		 case BSD_TM:
			 ptrErr->bsdstart = ticks;
			 break;
		 default:
			 break;
	 }
}

uint8 Bms_GetStepErrFg(uint8 gunNo,uint8 tg)
{
	uint8 rtn = 0;
	ERR_STEP_ST *ptrErr = Bms_GetStepStPtr(gunNo);
	switch(tg) {
		 case LOCK_TM:
			 rtn = ptrErr->stu.bits.locktmout;
			 break;
		 case BHM_TM:
			 rtn = ptrErr->stu.bits.bhmtmout;
			 break;
		 case XF_TM:
			 rtn = ptrErr->stu.bits.xftmout;
			 break;
		 case BRM_TM:
			 rtn = ptrErr->stu.bits.brmtmout;
			 break;
		 case BCP_TM:
			 rtn = ptrErr->stu.bits.bcptmout;
			 break;
		 case BRO_TM:
			 rtn = ptrErr->stu.bits.brotmout;
			 break;
		 case BCL_TM:
			 rtn = ptrErr->stu.bits.bcltmout;
			 break;
		 case BCS_TM:
			 rtn = ptrErr->stu.bits.bcstmout;
			 break;
		 case BST_TM:
			 rtn = ptrErr->stu.bits.bsttmout;
			 break;
		 case CST_TM:
			 rtn = ptrErr->stu.bits.csttmout;
			 break;
		 case BSD_TM:
			 rtn = ptrErr->stu.bits.bsdtmout;
			 break;
		 case KMVOLT10_ERR:
			 rtn = ptrErr->stu.bits.kmvolt10err;
		   break;
		 case KMVOLT60_ERR:
			 rtn = ptrErr->stu.bits.kmvolt60err;
		   break;
		 case PARAM_ERR:
			 rtn = ptrErr->stu.bits.paramerr;
			 break;
		 case CARVOL_ERR:
			 rtn = ptrErr->stu.bits.carvolterr;
			 break;
		 case BATSTU_ERR:
			 rtn =  ptrErr->stu.bits.batstuerr;
			 break;
		 case DISALLOW_FG:
			 rtn =  ptrErr->stu.bits.disallow;
			 break;
		 case CC1LINKED_ERR:
			 rtn = ptrErr->stu.bits.cc1;
		   break;
		 case MODVOLT_ERR:
			 rtn = ptrErr->stu.bits.modvolt;
		  break;
		 case KMINOUT_ERR:
			  rtn = ptrErr->stu.bits.inoutvolt;
		  break;
		 default:
			 break;
	 }
	 return rtn;
}

void Bms_SetStepErrFg(uint8 gunNo,uint8 tg,uint8 yesorno)
{
	ERR_STEP_ST *ptrErr = Bms_GetStepStPtr(gunNo);
	#define ERR_BITS ptrErr->stu.bits 
	switch(tg) {
		 case LOCK_TM:
			 ERR_BITS.locktmout = yesorno;
			 break;
		 case BHM_TM:
			 ERR_BITS.bhmtmout = yesorno;
			 break;
		 case XF_TM:
			 ERR_BITS.xftmout = yesorno;
			 break;
		 case BRM_TM:
			 ERR_BITS.brmtmout = yesorno;
			 break;
		 case BCP_TM:
			 ERR_BITS.bcptmout = yesorno;
			 break;
		 case BRO_TM:
			 ERR_BITS.brotmout = yesorno;
			 break;
		 case BCL_TM:
			 ERR_BITS.bcltmout = yesorno;
			 break;
		 case BCS_TM:
			 ERR_BITS.bcstmout = yesorno;
			 break;
		 case BST_TM:
			 ERR_BITS.bsttmout = yesorno;
			 break;
		 case CST_TM:
			 ERR_BITS.csttmout = yesorno;
			 break;
		 case BSD_TM:
			 ERR_BITS.bsdtmout = yesorno;
			 break;
		 case KMVOLT10_ERR:
			 ERR_BITS.kmvolt10err = yesorno;
		   break;
		 case KMVOLT60_ERR:
			 ERR_BITS.kmvolt60err = yesorno;
		   break;
		 case PARAM_ERR:
			 ERR_BITS.paramerr = yesorno;
			 break;
		 case CARVOL_ERR:
			 ERR_BITS.carvolterr = yesorno;
			 break;
		 case BATSTU_ERR:
			 ERR_BITS.batstuerr = yesorno;
			 break;
		 case DISALLOW_FG:
			 ERR_BITS.disallow = yesorno;
			 break;
		 case CC1LINKED_ERR:
			 ERR_BITS.cc1 = yesorno;
			 break;
		 case MODVOLT_ERR:
			 ERR_BITS.modvolt = yesorno;
		  break;
		 case KMINOUT_ERR:
			 ERR_BITS.inoutvolt = yesorno;
			 break;
		 default:
			 break;
	 }
}


uint8 BmsAckPkgEC(uint8 *buf,void* st,uint8 GunNo)
{
	uint8 index = 0;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st; 
	if( (NULL != st) || (pCtrl->tg == EC10_TG) ) {
		buf[index++] = EC11_TG;
		buf[index++] = pCtrl->frmNum;
		buf[index++] = 0x01;
		buf[index++] = 0xff;
		buf[index++] = 0xff;
		buf[index++] = 0x00;
		buf[index++] = pCtrl->pf;
		buf[index++] = 0x00;
	
		
		pCtrl->frmNum = 1;
	}
	
	DebugInfoWithPbuf(BMS_MOD,"应答多包请求，发送EC", buf, 8, 2);
	
	return index;
}

/*接收最后一个时调用*/
uint8 BmsAckPkgEB(uint8 *buf,void* st,uint8 GunNo)
{
	uint8 index = 0;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st;  //EC10_TG
	if( (NULL != st) || (pCtrl->pf == GBT_MULIT_PACKAGE) ) {
		buf[index++] = EC13_TG;
		buf[index++] = pCtrl->length & 0xff;
		buf[index++] = pCtrl->length >> 8;
		buf[index++] = pCtrl->frmNum;
		buf[index++] = 0xff;
		buf[index++] = 0x00;
		buf[index++] = pCtrl->pf;
		buf[index++] = 0x00;
		pCtrl->frmNum = 1;
	}

	DebugInfoWithPbuf(BMS_MOD,"响应最后一包，发送EC", buf,8, 2);
	
	return index;
}

/*01 01 00 */
uint8 ChgSendPkgCHM(uint8 *buf,void* st,uint8 GunNo)
{
	uint8 index = 0;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st;  //EC10_TG
	
//	DebugInfoByBms("发送CHM");
	if( (NULL != st) || (pCtrl->pf == CHM_CODE) ) {
		if( OLD_GBT_FG == pCtrl->otherFg ) {
			buf[index++] = 0x00;
		}else {
			buf[index++] = 0x01;
		}
		buf[index++] = 0x01;
		buf[index++] = 0x00;
		pCtrl->gunNo = GunNo;
	}

	DebugInfoWithPbuf(BMS_MOD,"CHM", buf, sizeof(CHM_ST),2);
	
	return index;	
}

/*拒绝接收多包*/
uint8 ChgSendPkgRefuseMuliti(uint8 *buf,void* st,uint8 GunNo)
{
	uint8 index = 0;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st;  //EC10_TG
	if( (NULL != st) ) {
		buf[index++] = 0xFF;
		buf[index++] = 0xFF;
		buf[index++] = 0xFF;
		buf[index++] = 0xFF;
		buf[index++] = 0xFF;
	
		buf[index++] = pCtrl->ctrl.info.pf[0];
		buf[index++] = pCtrl->ctrl.info.pf[1];
		buf[index++] = pCtrl->ctrl.info.pf[2];
	}

	DebugInfoWithPbuf(BMS_MOD,"拒绝接收多包", buf, 8, 2);
	
	return index;
}

/*00 01 00 00 00 00 00 00  */
/*AA 01 00 00 00 00 00 00  */
uint8 ChgSendPkgCRM(uint8 *buf,void* st,uint8 GunNo)
{
	uint8 index = 0;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st; 
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(GunNo);
	
	if((NULL != st) || (pCtrl->pf == CRM_CODE) ) {
		pCtrl->spn = pdata->crm.idenf;
		if( pCtrl->spn == SPN_READYNO ) {
			Gbt_CtrlCricle(GunNo,CHM_CODE,CRICLE_DISALLOWED,0);
		}
		buf[index++] = pCtrl->spn;
		buf[index++] = 0x01;
		buf[index++] = 0x00;
		buf[index++] = 0x00;
		buf[index++] = 0x00;
		buf[index++] = 0x00;
		buf[index++] = 0x00;
		buf[index++] = 0x00;
		
		pCtrl->gunNo = GunNo;
	}
	
	DebugInfoWithPbuf(BMS_MOD,"CRM", buf, sizeof(CRM_ST),0);
	return index;	
}


uint8 ChgSendPkgCTS(uint8 *buf,void* st,uint8 GunNo)
{
	uint8 index = 0;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st; 
	
	struct tm systime;
	systime = Time_GetSystemCalendarTime();
	
	if( (NULL != st) || (pCtrl->pf == CTS_CODE) ) {
		buf[index++] = Common_Hex2bcd8(systime.tm_sec);
		buf[index++] = Common_Hex2bcd8(systime.tm_min);
		buf[index++] = Common_Hex2bcd8(systime.tm_hour);
		buf[index++] = Common_Hex2bcd8(systime.tm_mday);
		buf[index++] = Common_Hex2bcd8(systime.tm_mon);
		buf[index++] = Common_Hex2bcd8(systime.tm_year%100);
		buf[index++] = 0x20;
	
		pCtrl->gunNo = GunNo;
	}
	DebugInfoWithPbuf(BMS_MOD,"CTS", buf, sizeof(CTS_ST),0);
	
	return index;	
}


uint8 ChgSendPkgCML(uint8 *buf,void* st,uint8 GunNo)
{
	uint8 index = 0;
	uint16 tmp16;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st; 
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(GunNo);
	
	if( (NULL != st) || (pCtrl->pf == CML_CODE)) {
		tmp16 = VOLT_PROTO(pdata->cml.maxoutvolt);
		buf[index++] = tmp16 & 0xff;
		buf[index++] = tmp16 >> 8;
		
		tmp16 = VOLT_PROTO(pdata->cml.minoutvolt);
		buf[index++] = tmp16 & 0xff;
		buf[index++] = tmp16 >> 8;
		
		tmp16 = CURR_PROTO(pdata->cml.maxoutcur);
		buf[index++] = tmp16 & 0xff;
		buf[index++] = tmp16 >> 8;
		
		tmp16 = CURR_PROTO(pdata->cml.minoutcur);
		buf[index++] = tmp16 & 0xff;
		buf[index++] = tmp16 >> 8;
		
		pCtrl->gunNo = GunNo;
	}
	
	DebugInfoWithPbuf(BMS_MOD,"CML", buf, sizeof(CML_ST),0);
	
	return index;	
}

uint8 ChgSendPkgCRO(uint8 *buf,void* st,uint8 GunNo)
{
	uint8 index = 0;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st; 
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(GunNo);
	
	if( (NULL != st) || (pCtrl->pf == CRO_CODE) ) {
		pCtrl->spn = pdata->cro.crostu;
		buf[index++] = pCtrl->spn;
		
		pCtrl->gunNo = GunNo;
	}

	DebugInfoWithPbuf(BMS_MOD,"CRO", buf, sizeof(CRO_ST),0);
	
	return index;	
}

uint8 ChgSendPkgCCS(uint8 *buf,void* st,uint8 GunNo)
{
	stoc_u tmp16;
	uint8 index = 0;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st; 
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(GunNo);
	
	/* 充电机在充电阶段所发送的充电机充电状态报文CCS的字节长度为8，
	   不符合GBT 34658-2017中DP.3001项中关于CCS报文字节长度应为7的要求，因此应将CCS的第8个字节内容删除 */
	
	if( (NULL != st) || (pCtrl->pf == CCS_CODE) ) {
		tmp16.s = VOLT_PROTO(pdata->ccs.outvolt);
		buf[index++] = tmp16.c[0];
		buf[index++] = tmp16.c[1];
		
		tmp16.s = CURR_PROTO(pdata->ccs.outcurr);
		buf[index++] = tmp16.c[0];
		buf[index++] = tmp16.c[1];
		
		/* 标准规定充电机充电状态报文CCS的第5，6字节为累计充电时间，数据分辨率为1分钟每位 */
		tmp16.s = pdata->ccs.totalchgtime;
		buf[index++] = tmp16.c[0];
		buf[index++] = tmp16.c[1];
		
		/* 当充电机在充电阶段收到BSM报文中电池状态一切正常，但充电请求为禁止充电 */
		/* 修改如下 */
		
		buf[index++] = 0xfc|(uint8)(pdata->ccs.allowed.bits.fg);  
		buf[index++] = 0xff;
		pCtrl->gunNo = GunNo;
	}
	DebugInfoWithPbuf(BMS_MOD,"CCS", buf, sizeof(CCS_ST),2);
	return index;	
}

extern CHARGE_TYPE * gPtrRunData[DEF_MAX_GUN_NO];

uint8 ChgSendPkgCST(uint8 *buf,void* st,uint8 GunNo)
{
	uint8 index = 0;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st; 
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(GunNo);
	
	if ((NULL != st) || (pCtrl->pf == CST_CODE)) 
	{
		if (1 == gPtrRunData[GunNo]->input->statu.bits.stop)  /* 急停 */
		{
			pdata->cst.stoprsn.bits.fault = 1;     /* 故障中止 */
			pdata->cst.faultrsn.bits.scramflt = 1; /* 充电机急停 */
		}
		
		if (ECARD_REASON == gPtrRunData[GunNo]->logic->stopReason) /*刷卡停止 */
		{
			pdata->cst.stoprsn.bits.manual = 1;  /* 人工中止 */
		}
		
		buf[index++] = pdata->cst.stoprsn.byte;
		buf[index++] = pdata->cst.faultrsn.byte[0];
		buf[index++] = 0xf0 | (uint8)(pdata->cst.faultrsn.byte[1]);
		buf[index++] = 0xf0 | pdata->cst.errorrsn.byte;
		
		pCtrl->gunNo = GunNo;
	}

	DebugInfoWithPbuf(BMS_MOD,"CST", buf, sizeof(CST_ST),0);
	
	return index;	
}

uint8 ChgSendPkgCSD(uint8 *buf,void* st,uint8 GunNo)
{
	uint8 index = 0;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st; 
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(GunNo);
	
	if( (NULL != st) || (pCtrl->pf == CSD_CODE) ) {
		buf[index++] = pdata->csd.chgtotaltime & 0xff;
		buf[index++] = pdata->csd.chgtotaltime >>8;
		buf[index++] = pdata->csd.outenergy & 0xff;
		buf[index++] = pdata->csd.outenergy >> 8;
		buf[index++] = pdata->csd.chgcode[0];
		buf[index++] = pdata->csd.chgcode[1];
		buf[index++] = pdata->csd.chgcode[2];
		buf[index++] = pdata->csd.chgcode[3];
		pCtrl->gunNo = GunNo;
	}
	
	DebugInfoWithPbuf(BMS_MOD,"CSD", buf, sizeof(CSD_ST),0);
	
	return index;	
}


uint8 ChgSendPkgCEM(uint8 *buf,void* st,uint8 GunNo)
{
	//stoc_u tmp16;
	uint8 index = 0;
	GSEND_INFO *pCtrl =(GSEND_INFO *)st; 
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(GunNo);
	if( (NULL != st) || (pCtrl->pf == CEM_CODE) ) {
		buf[index++] = pdata->cem.cerrcode.byte[0];
		buf[index++] = pdata->cem.cerrcode.byte[1];
		buf[index++] = pdata->cem.cerrcode.byte[2];
		buf[index++] = pdata->cem.cerrcode.byte[3];
		pCtrl->gunNo = GunNo;
	}

	DebugInfoWithPbuf(BMS_MOD,"CEM", buf, sizeof(CEM_ST),2);
	
	return index;	
}

/*********************************BMS******************************************************/
/*********************************BMS******************************************************/

void ProcBHM(void *item,uint8 *data,uint8 GunNo)
{ 
	 BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(GunNo) ;
	 const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	 if ( (NULL == ptrItem) || (ptrItem->pgn != BHM_CODE)) 
   { 
       return; 
   }
	 stoc_u *ptr16 = (stoc_u *)data;
	 ptr->bhm.maxvolte = ptr16->s;

 
 	DebugInfoWithPbuf(BMS_MOD,"BHM", data, sizeof(stoc_u),2);
	 /*进入 启动绝缘监测*/
	 if( GET_WORKSTEP(GunNo) == STEP_SKHANDS ) {
			SET_WORKSTEP(GunNo,STEP_ISOCHECK_SUB);
//patli 20200226		  Delay5Ms(1);
	 }
	 return;
}

void ProcBRM(void *item,uint8 *data,uint8 GunNo)
{	 
	BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(GunNo);
	RCV_PACK_ST *ptrBmsPack = Bms_GetBmsPack(GunNo);
	const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;

	if ((NULL == ptrItem) || (ptrItem->pgn != BRM_CODE)) 
	{ 
		return; 
	}
	
	ptrBmsPack->statu.bits.rcvBRM = 1;
	
	memcpy(&ptr->brm,data,sizeof(BRM_ST));
	  DebugInfoWithPbuf(BMS_MOD,"BRM", data, sizeof(BRM_ST),0);

	if ((GET_WORKSTEP(GunNo) <= STEP_BRM_WAIT_SUB )&&(GET_WORKSTEP(GunNo) >= STEP_ISOCHECK_SUB)) 
	{
		SET_WORKSTEP(GunNo,STEP_BRM_SUB);
//patli 20200226			Delay5Ms(1); 
	}

	return;
}

void ProcBCP(void *item,uint8 *data,uint8 GunNo)
{
	 BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(GunNo) ;
	 const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	 
	 if ( (NULL == ptrItem) || (ptrItem->pgn != BCP_CODE)) 
   {
		 return; 
   }
	 
	 DebugInfoWithPbuf(BMS_MOD,"BCP", data, sizeof(BCP_ST),0);
	 
	 memcpy(&ptr->bcp,data,sizeof(BCP_ST));
	 stoc_u tmp16;
	 tmp16.c[0] = data[2];
	 tmp16.c[1] = data[3];
	 ptr->bcp.maxcurr = CURR_PROTO(tmp16.s);    //(4000 > tmp16.s ) ? (4000 - tmp16.s) : 0; /*最高允许充电电流 * 10*/
	
	 if( GET_WORKSTEP(GunNo) == STEP_BCPWAIT_SUB ) {
			SET_WORKSTEP(GunNo,STEP_BCP_SUB);
//patli 20200226			  Delay5Ms(1);
	 }
	 
	 /* 步进电流清零*/
	 gStepCurrent = 0;
	 return;
}

void ProcBRO(void *item,uint8 *data,uint8 GunNo)
{
	 BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(GunNo) ;
	 const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	 if ( (NULL == ptrItem) || (ptrItem->pgn != BRO_CODE)) 
   { 
       return; 
   }
	 memcpy(&ptr->bro,data,sizeof(BRO_ST));
	 
	 DebugInfoWithPbuf(BMS_MOD,"BRO", data, sizeof(BRO_ST),0);
	 
	 if (BRO_READYOK == ptr->bro.brostu) { 
		 if( GET_WORKSTEP(GunNo) == STEP_BROWAIT_SUB ) {
		  SET_WORKSTEP(GunNo,STEP_BRO_SUB);
//patli 20200226				Delay5Ms(1);
		}
	 }
	 return;
}


#define MDSTEPCURRENT   300   //30A一次

/* 本ProBmsStepAdjustCurr ProcBCL 函数为适应航叉BMS问题而修改，其需求的电压电流值有时会报异常值导致不能充电故障，修改后使协议流程通过 */
/* 但是其值可能是错误的 */
/* 2018年11月08日为适应杭叉的BMS问题修改，其BMS需求电压有时会超过BCP最大电压，故将判断条件去掉 */ 
void ProBmsStepAdjustCurr(void)
{
 	uint16 tmp16;
 	stoc_u tmp16su;

    static uint32 sStepTicks = 0;
	 
	static BCL_ST stmpbcl = {0,0,0,0,0,0};
	uint8  msg[6] = {0};
	PARAM_OPER_TYPE *ptrRunParam = ChgData_GetRunParamPtr();
	uint8 tmpNum = ptrRunParam->modnum ;
	BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(0) ;
	CHARGE_TYPE *ptrRunData = ChgData_GetRunDataPtr(AGUN_NO);
    tmp16 = abs(ptr->bcl.needcurr - ptrRunData->meter->current);

    if( (tmpNum == 0 ) || ( tmpNum > 20 )) { /*防止配置出错*/
		 tmpNum = 10;
	}
	 
	 if( (stmpbcl.chgmode != ptr->bcl.chgmode) \
	 || (stmpbcl.needcurr != ptr->bcl.needcurr) \
	 || ((tmp16 > 50) && ( ptrRunData->meter->current < (tmpNum * ptrRunParam->singmodcurr)-20 ) ) \
	 || (stmpbcl.needvolt != ptr->bcl.needvolt) ) 
	 {
		 tmp16 = ptr->bcl.needcurr;
		 if( (GetSystemTick() - sStepTicks > 2 * TIM_1S) )
		 {
			sStepTicks = GetSystemTick();
				
			if( tmp16 > gStepCurrent ) 
			{
	        	gStepCurrent = (gStepCurrent+MDSTEPCURRENT) < tmp16 ? (gStepCurrent+MDSTEPCURRENT) : tmp16;
			}
			else if (MDSTEPCURRENT < gStepCurrent) 
			{
				gStepCurrent = (gStepCurrent-MDSTEPCURRENT) > tmp16 ? gStepCurrent - MDSTEPCURRENT : tmp16;
			}
			else 
			{
				gStepCurrent = tmp16;
			}
//			if( (ptr->bcl.needvolt > ptr->bcp.chgmaxvolt) || (ptr->bcl.needvolt > (ptrRunParam->maxvolt+50 ))\
//					 || ptr->bcl.needvolt < 100 )
//			{ /*最高需求大于 > 755V 或者 < 10V 认为数据出错*/
//					  return;
//			}

			if (ptr->bcl.needvolt > ptrRunParam->maxvolt || ptr->bcl.needvolt < ptrRunParam->minvolt) { 
				return;
			}
			 
			tmp16su.s = ptr->bcl.needvolt-10;
			msg[0] = 0; /*枪号，待改写*/
			msg[1] = tmp16su.c[0];  
			msg[2] = tmp16su.c[1];
			
			tmp16su.s = gStepCurrent / tmpNum;
			tmp16su.s = tmp16su.s > ptrRunParam->singmodcurr ? ptrRunParam->singmodcurr : tmp16su.s;
			msg[3] = tmp16su.c[0];  
			msg[4] = tmp16su.c[1];
			msg[5] = ptr->bcl.chgmode;
			SendMsgWithNByte(MSG_MOD_ADJUST,5,&msg[0],APP_TASK_DLMOD_PRIO);
			
			stmpbcl.chgmode = ptr->bcl.chgmode;
			stmpbcl.needcurr = gStepCurrent;
			stmpbcl.needvolt = ptr->bcl.needvolt-5;  /*-0.5V*/
		}
	}
	else 
	{ /*需求电压、电流无需调整*/
		sStepTicks = GetSystemTick();
	}	
}

/*BMS需求报文*/
void ProcBCL(void *item,uint8 *data,uint8 gunNo)
{
	BCL_ST tmpbcl;
	CHGDATA_ST *ptrBms = Bms_GetChgDataPtr(gunNo);
	PARAM_OPER_TYPE *ptrRunPara = ChgData_GetRunParamPtr();
	BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(gunNo);
	RCV_PACK_ST *ptrBmsPack = Bms_GetBmsPack(gunNo);
	const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;

	uint16 u16TempCurr = 0, u16TempVolt = 0;

	if ( (NULL == ptrItem) || (ptrItem->pgn != BCL_CODE)) 
	{ 
		return; 
	}

	ptrBmsPack->statu.bits.rcvBCL = 1;
	
	Bms_SetStepErrFg(gunNo,BCL_TM,0);
	Bms_StartTimeCount(gunNo,BCL_TM,GetSystemTick());
	Bms_StartTimeCount(gunNo,BCS_TM,GetSystemTick());   //有些车型不发送BCS

	stoc_u tmp16;
	tmp16.c[0] = data[0];
	tmp16.c[1] = data[1];
	tmpbcl.needvolt = tmp16.s;/*电压扩大10倍*/

//		if( tmpbcl.needvolt > ptr->bcp.chgmaxvolt || tmpbcl.needvolt > ptrRunPara->maxvolt+50 \
//		|| tmpbcl.needvolt < ptrRunPara->minvolt )
//		{ 
//			/*最高需求大于 > 755V  <100V 认为数据出错*/
//			return;
//		}

	if (tmpbcl.needvolt > ptr->bcp.chgmaxvolt)
	{
		tmpbcl.needvolt = ptr->bcp.chgmaxvolt;
	}
	
	if (tmpbcl.needvolt > ptrRunPara->maxvolt)
	{
		tmpbcl.needvolt = ptrRunPara->maxvolt;
	}
	
	if (tmpbcl.needvolt < ptrRunPara->minvolt)
	{
		tmpbcl.needvolt = ptrRunPara->minvolt;
	}
	
	tmp16.c[0] = data[2];
	tmp16.c[1] = data[3];
	tmpbcl.needcurr =  CURR_PROTO(tmp16.s);    //(4000 > tmp16.s ) ? (4000 - tmp16.s) : 0; /*电流扩大10倍*/
	tmpbcl.chgmode = data[4];

	//很多车厂BCP报文的最高允许充电电流是不做逻辑判断的，故下面的if语句注释了，改成取需求电流和最高允许充电电流的最小值-20190620 
	//	 if( tmpbcl.needcurr > 4000 || tmpbcl.needcurr > ( ptr->bcp.maxcurr + 10 ) ) {
	//		   if(tmpbcl.needcurr > 200) { //20190402
	//					return;
	//			 }
	//	 }

	if (tmpbcl.needcurr > ptr->bcp.maxcurr)
	{
		tmpbcl.needcurr = ptr->bcp.maxcurr;
	}

	if( tmpbcl.needcurr > ptrRunPara->maxcurr ) 
	{ 
		tmpbcl.needcurr = ptrRunPara->maxcurr;
	}

	memcpy(&ptr->bcl,&tmpbcl,sizeof(BCL_ST));

	
	DebugInfoWithPbuf(BMS_MOD,"BCL", (uint8*)&ptr->bcl, sizeof(BCL_ST),2);

	if (GET_WORKSTEP(gunNo) == STEP_CHARGEING) 
	{
		u16TempCurr = abs(ptr->bcl.needcurr - gBmsCtrl[gunNo].u16NeedCurr);
		u16TempVolt = abs(ptr->bcl.needvolt - gBmsCtrl[gunNo].u16NeedVolt);

		if( (GetSystemTick() - gBmsCtrl[gunNo].u32WaitTicks > TIM_NS(2)) \
		|| ((( u16TempCurr > 10 ) || ( u16TempVolt > 50 )) \
		&& (GetSystemTick() - gBmsCtrl[gunNo].u32WaitTicks > TIM_NS(1))) ) 
		{
			gBmsCtrl[gunNo].u16NeedCurr = ptr->bcl.needcurr;
			gBmsCtrl[gunNo].u16NeedVolt = ptr->bcl.needvolt;	
			gBmsCtrl[gunNo].u32WaitTicks = GetSystemTick();				 
			SendMsgWithNByte(MSG_MODGRP_ADJUST,1,&gunNo,APP_TASK_DLMOD_PRIO);
		}
	}

	/* (GBT 34658-2017中DN.3001项)标准规定充电机充电状态报文CCS的起始发送条件为收到BCL和BCS报文。
		 此项中当充电机在发送CRO(AA)报文期间只收到了BCL报文，未收到BCS报文时，充电机回复了CCS报文，因此不正确
	*/
	/* (GBT 34658-2017中DN.3002项)标准规定充电机充电状态报文CCS的起始发送条件为收到BCL和BCS报文。
		 此项中当充电机在发送CRO(AA)报文期间只收到了BCS报文，未收到BCL报文时，充电机回复了CCS报文，因此不正确
	*/
	
	/* 标准关于CRO(AA)报文中止发送条件以及CCS报文起始发送条件均为收到BCL和BCS报文 */
	
	/* 需要按国标的要求更改 */
	
	/*停止发送CRO*/
	

	#ifdef KAIPUBIAO
	if ((1 == ptrBmsPack->statu.bits.rcvBCL) && (1 == ptrBmsPack->statu.bits.rcvBCS))
	#endif
	{
		if (ptrBms->cro.crostu == CRO_READYOK) 
		{
			Gbt_CtrlCricle(gunNo,CRO_CODE,CRICLE_DISALLOWED,0);
			ptrBms->cro.crostu  = CRO_READYNO;
			
			Gbt_ReportSingPkg(gunNo,CCS_CODE,0);
			Gbt_CtrlCricle(gunNo,CCS_CODE,CRICLE_ALLOWED,0);
		}
	}

//patli 20200226		Delay5Ms(1); 
	
	return;
}

void ProcBCS(void *item,uint8 *data,uint8 GunNo)
{
	CHGDATA_ST *ptrBms = Bms_GetChgDataPtr(GunNo);
	BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(GunNo) ;
	RCV_PACK_ST *ptrBmsPack = Bms_GetBmsPack(GunNo);
	const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	if ( (NULL == ptrItem) || (ptrItem->pgn != BCS_CODE)) 
	{ 
		 return; 
	}
	
	ptrBmsPack->statu.bits.rcvBCS = 1;
	
	Bms_SetStepErrFg(GunNo,BCS_TM,0);
	Bms_StartTimeCount(GunNo,BCS_TM,GetSystemTick());
	memcpy(&ptr->bcs,data,sizeof(BCS_ST));
	
	DebugInfoWithPbuf(BMS_MOD,"BCS", data, sizeof(BCS_ST),2);
	
	ptr->bcs.deteccurr = CURR_PROTO(ptr->bcs.deteccurr); //(4000 > ptr->bcs.deteccurr) ? 4000 -  ptr->bcs.deteccurr : 0;
	
	/* 标准关于CRO(AA)报文中止发送条件以及CCS报文起始发送条件均为收到BCL和BCS报文 */
	/*停止发送CRO*/
	
	#ifdef KAIPUBIAO
	if ((1 == ptrBmsPack->statu.bits.rcvBCL) && (1 == ptrBmsPack->statu.bits.rcvBCS))
	#endif
	{
		if( ptrBms->cro.crostu == CRO_READYOK ) 
		{
			Gbt_CtrlCricle(GunNo,CRO_CODE,CRICLE_DISALLOWED,0);
			ptrBms->cro.crostu  = CRO_READYNO;

			Gbt_ReportSingPkg(GunNo,CCS_CODE,0);
			Gbt_CtrlCricle(GunNo,CCS_CODE,CRICLE_ALLOWED,0);
		}
	}
  
//patli 20200226		Delay5Ms(1);
	
	return;
}

void ProcBSM(void *item,uint8 *data,uint8 GunNo)
{
	BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(GunNo) ;
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(GunNo);
	const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	
	if ( (NULL == ptrItem) || (ptrItem->pgn != BSM_CODE)) 
	{ 
		 return; 
	}
	
	DebugInfoWithPbuf(BMS_MOD,"BSM", data, sizeof(BSM_ST),2);

	memcpy(&ptr->bsm,data,sizeof(BSM_ST));
	ptr->bsm.batmaxtemp -= 50; /*温度偏移50*/
	ptr->bsm.batmintemp -= 50;

	if(ptr->bsm.battalm.bits.singbatValm != 0x00 || ptr->bsm.battalm.bits.socalm != 0x00 \
	 || ptr->bsm.battalm.bits.tempalm == 0x02 || ptr->bsm.battalm.bits.linkalm == 0x01 \
	 || ptr->bsm.battalm.bits.curralm == 0x01 || ptr->bsm.battalm.bits.insulalm == 0x01 ) {
//patli 20191212		 if (ptr->bsm.battalm.bits.allowed == 0x00 ) { /*不允许充电*/
			 /*设置故障位*/
			 Bms_SetStepErrFg(GunNo,DISALLOW_FG,1);
			 /***************************************************************************
					发送消息，告诉停机
			 *****************************************************************************/
//		 } 
	}
	else 
	{
		if (ptr->bsm.battalm.bits.allowed == 0x01)
		{
			/* 收到BSM允许充电*/
			pdata->ccs.allowed.bits.fg = 0x01; /*允许充电*/
			Bms_SetStepErrFg(GunNo,DISALLOW_FG,0);
		}
		
		else if (ptr->bsm.battalm.bits.allowed == 0x00)
		{
			pdata->ccs.allowed.bits.fg = 0x00; /*暂停充电 */
			/* 10分钟以后再停止，用电流小的条件判断 */
		}
		
	}
	
	return;
}

void ProcBMV(void *item,uint8 *data,uint8 GunNo)
{ 
	BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(GunNo) ;
	const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;

	
	DebugInfoByBms("接收到BMV,暂不处理");
	if ( (NULL == ptrItem) || (ptrItem->pgn != BMV_CODE)) 
	{ 
		return; 
	}
	/*暂时不处理该报文*/
	//memcpy(&ptr->bmv,data,sizeof(BMV_ST));
	return;
}

void ProcBMT(void *item,uint8 *data,uint8 GunNo)
{
	BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(GunNo) ;
	const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;

	
	DebugInfoByBms("接收到BMT,暂不处理");
	if ( (NULL == ptrItem) || (ptrItem->pgn != BMT_CODE)) 
	{ 
		 return; 
	}
	/*暂时不处理该报文*/
	//memcpy(&ptr->bmt,data,sizeof(BMT_ST));
	return;
}

void ProcBSP(void *item,uint8 *data,uint8 GunNo)
{
	BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(GunNo) ;
	const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;

	
	DebugInfoByBms("接收到BSP,暂不处理");
	if ( (NULL == ptrItem) || (ptrItem->pgn != BSP_CODE)) 
	{ 
		 return; 
	}
	/*暂时不处理该报文*/
	//memcpy(&ptr->bmt,data,sizeof(BMT_ST));
	return;
}

void ProcBST(void *item,uint8 *data,uint8 GunNo)
{
	BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(GunNo) ;
	RCV_PACK_ST *ptrBmsPack = Bms_GetBmsPack(GunNo);
	CHGDATA_ST *pChg = Bms_GetChgDataPtr(GunNo);
	const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	
	if ( (NULL == ptrItem) || (ptrItem->pgn != BST_CODE)) 
	{ 
		return; 
	}
	
	DebugInfoWithPbuf(BMS_MOD,"BST", data, sizeof(BST_ST),0);
	memcpy(&ptr->bst,data,sizeof(BST_ST));

#if 1 //tmp
	if ((0 == ptr->bst.faultrsn.word) && (0 == ptr->bst.errorrsn.byte)&& (0 == ptr->bst.stoprsn.byte)) {
		ptr->bst.faultrsn.bits.otherflt = 1;
	}
#endif

	if( GET_WORKSTEP(GunNo) != STEP_WAITOVER &&  GET_WORKSTEP(GunNo) != STEP_IDEL \
		&& GET_WORKSTEP(GunNo) != STEP_CHGEND ) {
//patli 20200226			Delay10Ms(2);
	}

	/*******************************************************************
				消息通知结束充电
	********************************************************************/

	/* 在STEP_CHGEND流程中周期发送开启CST和CSD */
	/* 标准规定充电机充电状态报文CCS的中止发送条件为收到BST报文 */
	Gbt_CtrlCricle(GunNo,CCS_CODE,CRICLE_DISALLOWED,1); /* 停止发送CCS */		
	
	/* 当BMS在充电阶段主动中止充电发送BST报文时，充电机在收到BST报文后所回复的充电机中止充电报文CST的数据域内容为00 00 F0 F0，
	此内容未能表明中止充电原因，因此应修改为40 00 F0 F0，表示因收到BST报文中止充电 */
	/* 修改如下 */
	pChg->cst.stoprsn.bits.initiative = 0x01;
	
	return;
}

void ProcBSD(void *item,uint8 *data,uint8 GunNo)
{	
	BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(GunNo) ;
	RCV_PACK_ST *ptrBmsPack = Bms_GetBmsPack(GunNo);
	const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	
	if ( (NULL == ptrItem) || (ptrItem->pgn != BSD_CODE)) 
	{
		return; 
	}

	DebugInfoWithPbuf(BMS_MOD,"BSD", data, sizeof(BSD_ST),0);
	memcpy(&ptr->bsd,data,sizeof(BSD_ST));
	
	/*停止发送CST*/
	Gbt_CtrlCricle(GunNo,CST_CODE,CRICLE_DISALLOWED,0);
	/*停止发送CSD*/
	Gbt_CtrlCricle(GunNo,CSD_CODE,CRICLE_DISALLOWED,1);

	/*回复一帧CSD(* 此句需要回复)*/
	Gbt_ReportSingPkg(GunNo,CSD_CODE, 0);
	
	/*等待回复完成 */
//patli 20200226		Delay10Ms(20); 
	
	/*关闭BMS电源 */
	RelayOut_AssistPower(GunNo,ASSIST_POWER_OFF);
	
	return; 
}

void ProcBEM(void *item,uint8 *data,uint8 GunNo)
{
	BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(GunNo) ;
	RCV_PACK_ST *ptrBmsPack = Bms_GetBmsPack(GunNo);
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(GunNo);
	const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	
	if ( (NULL == ptrItem) && (ptrItem->pgn != BEM_CODE)) 
	{ 
		 return; 
	}
	
	ptrBmsPack->statu.bits.rcvBEM = 1;
	
		DebugInfoWithPbuf(BMS_MOD,"BEM", data, sizeof(BEM_ST), 2);
	memcpy(&ptr->bem,data,sizeof(BEM_ST));
		if(ptr->bem.berrcode.dword > 0)
		{
			if(ptr->bem.berrcode.bits.crmtmout == 0x01)
				DebugInfoByBms("接收CRM 0X00充电机辨识报文超时");
			else if(ptr->bem.berrcode.bits.crmoktmout == 0x01)				
				DebugInfoByBms("接收CRM 0XAA充电机辨识报文超时");
			else if(ptr->bem.berrcode.bits.cmltmout == 0x01)				
				DebugInfoByBms("接收充电机CML时间同步和最大能力报文超时");
			else if(ptr->bem.berrcode.bits.crotmout == 0x01)				
				DebugInfoByBms("接收充电机CRO完成充电准备报文超时");
			else if(ptr->bem.berrcode.bits.ccstmout == 0x01)				
				DebugInfoByBms("接收充电机CCS充电状态报文超时");
			else if(ptr->bem.berrcode.bits.csttmout == 0x01)				
				DebugInfoByBms("接收充电机CST中止报文超时");
			else if(ptr->bem.berrcode.bits.csdtmout == 0x01)				
				DebugInfoByBms("接收充电机CSD统计报文超时");
				
		}			
//patli 20200226		Delay10Ms(1);
	
	if( GET_WORKSTEP(GunNo) != STEP_WAITOVER &&  GET_WORKSTEP(GunNo) != STEP_IDEL \
		&& GET_WORKSTEP(GunNo) != STEP_CHGEND && GET_WORKSTEP(GunNo) != STEP_WAIT_SUB) 
	{
//patli 20200226				Delay10Ms(10);
	}
	
	return; 
}










