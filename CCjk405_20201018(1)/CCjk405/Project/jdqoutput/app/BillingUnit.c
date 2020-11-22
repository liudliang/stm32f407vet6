/**
  ******************************************************************************
  * @file    BillingUnit.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
*/
#include "main.h"
#include "common.h"
#include "rtc.h"
#include "ChgData.h"
#include "Message.h"
#include "TaskMainCtrl.h"
#include "TaskDcMeter.h"
#include "Bms.h"
#include "BillingUnit.h"
#include "TaskBackComm.h"

#define SYSTEM_TICK_LOC 10

static uint8 gBackSectNo[DEF_MAX_GUN_NO] = {0} ;

REAL_BILL_DATA_ST gFeeData[DEF_MAX_GUN_NO];

CALCBILL_DATA_ST * Bill_GetBillData(uint8 gunNo)
{
	return &(gFeeData[gunNo].bill);
}

REAL_BILL_DATA_ST *GetFeeData(uint8 gunNo)
{
  return &(gFeeData[gunNo]);
}

void Bill_InitAgunData(void)
{
	uint32 feerate = 0,srvrate = 0;
	PARAM_FEE_TYPE *param = ChgData_GetFeeRate(AGUN_NO);
	
	feerate = gFeeData[AGUN_NO].real.currfeerate;
	srvrate = gFeeData[AGUN_NO].real.currSrvrate;
	memset(&gFeeData[AGUN_NO],0,sizeof(REAL_BILL_DATA_ST));
	gFeeData[AGUN_NO].real.currfeerate = feerate;
	gFeeData[AGUN_NO].real.currSrvrate = srvrate;
	gFeeData[AGUN_NO].real.serivetype = param->SectTimeFee.serivetype;
	gFeeData[AGUN_NO].real.bespktype = param->SectTimeFee.bespktype;
	gFeeData[AGUN_NO].real.parktype = param->SectTimeFee.parktype;	
}


void Bill_InitBgunData(void)
{
	uint32 feerate = 0,srvrate = 0;
	PARAM_FEE_TYPE *param = ChgData_GetFeeRate(BGUN_NO);
	
	feerate = gFeeData[BGUN_NO].real.currfeerate;
	srvrate = gFeeData[BGUN_NO].real.currSrvrate;
	memset(&gFeeData[BGUN_NO],0,sizeof(REAL_BILL_DATA_ST));
	gFeeData[BGUN_NO].real.currfeerate = feerate;
	gFeeData[BGUN_NO].real.currSrvrate = srvrate;
	gFeeData[BGUN_NO].real.serivetype = param->SectTimeFee.serivetype;
	gFeeData[BGUN_NO].real.bespktype = param->SectTimeFee.bespktype;
	gFeeData[BGUN_NO].real.parktype = param->SectTimeFee.parktype;	
}

void Bill_InitData(uint8 gunNo)
{
	 if( AGUN_NO == gunNo ) {
		 Bill_InitAgunData();
	 }else if( BGUN_NO == gunNo ) {
		 Bill_InitBgunData();
	 }
	 return;
}


/* 根据系统时间查找时段，从0时段开始 */
uint8 Bill_FindSectNo(void)
{
	uint8 ret,tmp;
	struct tm systime = Time_GetSystemCalendarTime();
	tmp = systime.tm_min > 30 ? 1: 0;
	ret = systime.tm_hour * 2 + tmp;
	return ret % MAX_HALFHOUR_SECT;
}

/*根据指定时间查找时段号*/
uint8 Bill_FindSectNoByTime(uint8 hour,uint8 min)
{
	uint8 ret,tmp;
	tmp = min > 30 ? 1 : 0;
	ret = hour * 2 + tmp;
	return ret % MAX_HALFHOUR_SECT;
}


///*设置卡相关信息*/
//void Bill_SetCardInfo(CARD_INFO *pCardinf)
//{
//	gFeeData[AGUN_NO].bill.cardtype = pCardinf->cardtype;
//	gFeeData[AGUN_NO].bill.beforemoney = pCardinf->money;
//	memcpy(gFeeData[AGUN_NO].bill.cardNo,pCardinf->cardNo,sizeof(gFeeData[AGUN_NO].bill.cardNo));
//}

/*设置启动相关信息*/
void Bill_SetStartInfo(START_PARAM *startpara)
{
	uint8 gunNo = startpara->gunNo % DEF_MAX_GUN_NO;
	gFeeData[gunNo].bill.IsPay = 0;
	gFeeData[gunNo].bill.IsReport = 0;
	
	gFeeData[gunNo].bill.chgtype = 0;
	gFeeData[gunNo].bill.chgmod = startpara->chgmod;
	gFeeData[gunNo].bill.startmod = startpara->startby;
	
	gFeeData[gunNo].bill.cardtype = 0;
	gFeeData[gunNo].bill.beforemoney = startpara->money;
	gFeeData[gunNo].bill.aftermoney = gFeeData[gunNo].bill.beforemoney;
	memcpy(gFeeData[gunNo].bill.cardNo,startpara->account,sizeof(gFeeData[gunNo].bill.cardNo));
	memcpy(gFeeData[gunNo].bill.vincode,startpara->vincode,sizeof(gFeeData[gunNo].bill.vincode));
	
	memcpy(gFeeData[gunNo].bill.serialNo,startpara->serilNo,sizeof(gFeeData[gunNo].bill.serialNo));
	
	/* 澄鹏后台使用32位用户号，在这里增加 */
	memcpy(gFeeData[gunNo].bill.cardNo2, startpara->account2, sizeof(gFeeData[gunNo].bill.cardNo2));
}

/*获取实时电价和服务电价*/
void Bill_GetCurrentFeeReate(uint8 gunNo)
{
	uint8 sectNo = 0;
	static uint32 slastticks[DEF_MAX_GUN_NO] = { 0 };
	if( GetSystemTick() - slastticks[gunNo] > 100 ) {
		slastticks[gunNo] = GetSystemTick();
		PARAM_FEE_TYPE *ptrfee = ChgData_GetFeeRate(gunNo);
		struct tm systime = Time_GetSystemCalendarTime();
		if( systime.tm_year > 2007  && systime.tm_year < 2080){
				sectNo = Bill_FindSectNoByTime(systime.tm_hour,systime.tm_min);
			  gFeeData[gunNo].real.cursectno = sectNo;
			  gFeeData[gunNo].real.currfeerate = ptrfee->SectTimeFee.price[ptrfee->SectTimeFee.sectNo[sectNo]%10];
		    gFeeData[gunNo].real.currSrvrate = ptrfee->SectTimeFee.srvrate[ptrfee->SectTimeFee.sectNo[sectNo]%10];
			  gFeeData[gunNo].real.parkfeerate = ptrfee->SectTimeFee.parkrate[ptrfee->SectTimeFee.sectNo[sectNo]%10];
		}	
	}
}


/*初始化计费数据*/
void Bill_SetStartData(uint8 gunNo)
{
	PARAM_DEV_TYPE *param = ChgData_GetDevParamPtr(); 
	struct tm systime = Time_GetSystemCalendarTime();

	
	gFeeData[gunNo].bill.gunno = gunNo;
	
	gFeeData[gunNo].bill.starttm[0] = Common_Hex2bcd8(systime.tm_year%100);
	gFeeData[gunNo].bill.starttm[1] = Common_Hex2bcd8(systime.tm_mon);
	gFeeData[gunNo].bill.starttm[2] = Common_Hex2bcd8(systime.tm_mday);
	gFeeData[gunNo].bill.starttm[3] = Common_Hex2bcd8(systime.tm_hour);
	gFeeData[gunNo].bill.starttm[4] = Common_Hex2bcd8(systime.tm_min);
	gFeeData[gunNo].bill.starttm[5] = Common_Hex2bcd8(systime.tm_sec);
	memcpy(gFeeData[gunNo].bill.stoptm,gFeeData[gunNo].bill.starttm,6);
	gFeeData[gunNo].bill.chgsecs = 0;
	
	gFeeData[gunNo].bill.startsect = Bill_FindSectNoByTime(systime.tm_hour,systime.tm_min);
	gFeeData[gunNo].bill.stopsect = gFeeData[gunNo].bill.startsect;
	
	gFeeData[gunNo].real.cursectno = gFeeData[gunNo].bill.startsect;
	gBackSectNo[gunNo] = gFeeData[gunNo].real.cursectno;
	//gFeeData.real.currfeerate = ptrfee->SectTimeFee.price[ptrfee->SectTimeFee.sectNo[gFeeData.real.cursectno]];
	/* 用于已充时间计算 */
	gFeeData[gunNo].real.startticks = GetSystemTick(); 
	
	/*开始电能数据*/
	DEV_METER_TYPE *pDcMet = TskDc_GetMeterDataPtr(gunNo);
	gFeeData[gunNo].bill.startenergy = pDcMet->energy;
	gFeeData[gunNo].bill.stopenergy = pDcMet->energy+1;
	gFeeData[gunNo].real.tmpstartenergy = gFeeData[gunNo].bill.startenergy;
	gFeeData[gunNo].bill.energy = 0;
	
	gFeeData[gunNo].bill.billmoney = 0;
	
	memset((void *)gFeeData[gunNo].bill.sectenergy,0,2*MAX_CHG_SECT);
	gFeeData[gunNo].bill.energymoney = 0;
	gFeeData[gunNo].bill.serivemoney = 0;	
}

/*VIN小于len长度*/
void Dealadd_VIN(uint8 *buff,uint8 len)
{
	uint8 i;
	for( i = 0 ; i < len; i++ ) {
		if(( buff[i] == 0x00 )||(buff[i] == '-')) {
			buff[i] = 'A';
		}
				
	}
}

void Bill_SetCarBmsData(uint8 gunNo)
{
	/*bms数据*/
	BMSDATA_ST *pBms = Bms_GetBmsCarDataPtr(gunNo);
  gFeeData[gunNo].bill.startsoc = pBms->bcp.batsoc/10;
	gFeeData[gunNo].bill.soc = pBms->bcp.batsoc;
	memcpy(gFeeData[gunNo].bill.vincode,&(pBms->brm.vin),17);
	Dealadd_VIN(gFeeData[gunNo].bill.vincode,17);
}

void heli_Bill_SetCarBmsData(uint8 gunNo)
{
		/*bms数据*/
	BMSDATA_ST *pBms = Bms_GetBmsCarDataPtr(gunNo);
  gFeeData[gunNo].bill.startsoc = pBms->bcs.cursoc;
	gFeeData[gunNo].bill.soc = pBms->bcs.cursoc;
}


/*设置结束计费数据*/
void Bill_SetStopData(uint8 gunNo,uint8 reason,uint8 errcode)
{
	  uint32 chgenergy;
	  /*停止时间*/
		struct tm systime = Time_GetSystemCalendarTime();
		gFeeData[gunNo].bill.stoptm[0] = Common_Hex2bcd8(systime.tm_year%100);
		gFeeData[gunNo].bill.stoptm[1] = Common_Hex2bcd8(systime.tm_mon);
		gFeeData[gunNo].bill.stoptm[2] = Common_Hex2bcd8(systime.tm_mday);
		gFeeData[gunNo].bill.stoptm[3] = Common_Hex2bcd8(systime.tm_hour);
		gFeeData[gunNo].bill.stoptm[4] = Common_Hex2bcd8(systime.tm_min);
		gFeeData[gunNo].bill.stoptm[5] = Common_Hex2bcd8(systime.tm_sec);
	  gFeeData[gunNo].bill.chgsecs = (GetSystemTick() - gFeeData[gunNo].real.startticks)/ TIM_1S;
	  gFeeData[gunNo].bill.stopsect = Bill_FindSectNoByTime(systime.tm_hour,systime.tm_min);
	
	  /*停止电能*/ 
	  DEV_METER_TYPE *pDcMet = TskDc_GetMeterDataPtr(gunNo);
	  gFeeData[gunNo].bill.stopenergy = pDcMet->energy;
	
	  PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	  if(CONN_CHGBIRD == BackCOMM->agreetype){ 
			PARAM_FEE_TYPE *SysDevParam = ChgData_GetFeeRate(gunNo);
      gFeeData[gunNo].bill.energy = (gFeeData[gunNo].bill.stopenergy -  gFeeData[gunNo].bill.startenergy)*(100 + SysDevParam->SectTimeFee.lossrate)/100;//电量电损
		}
		else{
			gFeeData[gunNo].bill.energy = gFeeData[gunNo].bill.stopenergy - gFeeData[gunNo].bill.startenergy;		
		}
		
	  gFeeData[gunNo].bill.aftermoney = gFeeData[gunNo].bill.beforemoney - gFeeData[gunNo].bill.billmoney;
	
		/*bms数据*/
		BMSDATA_ST *pBms = Bms_GetBmsCarDataPtr(gunNo);
		gFeeData[gunNo].bill.soc = pBms->bcs.cursoc;
	
	  gFeeData[gunNo].bill.endreason = reason;
		gFeeData[gunNo].bill.errcode = errcode;
		
		//充电结束原因,每个bit位代表一个结束原因
		memset(&gFeeData[gunNo].bill.stopreason, 0, sizeof(gFeeData[gunNo].bill.stopreason));
		memcpy(&gFeeData[gunNo].bill.stopreason, GetgErrBitData(gunNo), MAX_ERR_BYTE);  //15字节的故障存储区
}

uint32 Bill_RealFee(uint8 gunno)
{

	 uint8 tmp,sectNO,i,tmp8;
   uint32 sum ,sumSrv;	
	 //uint32 tmpSum = 0;

	 PARAM_FEE_TYPE *ptrfee = ChgData_GetFeeRate(gunno);
	 
	 if(gFeeData[gunno].bill.stopsect >= gFeeData[gunno].bill.startsect) {
		  tmp = gFeeData[gunno].bill.stopsect - gFeeData[gunno].bill.startsect  + 1;
	 }else {
		  tmp = gFeeData[gunno].bill.stopsect + MAX_HALFHOUR_SECT - gFeeData[gunno].bill.startsect + 1;
	 }
	 
	 sum = 0;
	 sumSrv = 0;
	 tmp8 = gFeeData[gunno].bill.startsect;
	 
   tmp %= (MAX_HALFHOUR_SECT+1);
	 for( i = 0 ; i < tmp ; i++ ) {
		 sectNO = (tmp8 + i ) % MAX_HALFHOUR_SECT;
	
		 sum += gFeeData[gunno].bill.sectenergy[sectNO] * ptrfee->SectTimeFee.price[ptrfee->SectTimeFee.sectNo[sectNO]];
		 
		 if( SRV_ENERGY_TYPE == gFeeData[gunno].real.serivetype ) {
			 sumSrv += gFeeData[gunno].bill.sectenergy[sectNO] * ptrfee->SectTimeFee.srvrate[ptrfee->SectTimeFee.sectNo[sectNO]]; 
		 }
	 }//end for
	 
//	 if(sum < 10) {
//		 	gFeeData.bill.serivemoney = sumSrv/BILL_PRICE_BASE;//100;
//	 }
	 PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	  if(CONN_CHGBIRD == BackCOMM->agreetype){ 
			PARAM_FEE_TYPE *SysDevParam = ChgData_GetFeeRate(gunno);
      sum = sum * (100 + SysDevParam->SectTimeFee.lossrate)/100;//电量电损
			sumSrv = sumSrv * (100 + SysDevParam->SectTimeFee.lossrate)/100;//电量电损
		}
		
	 gFeeData[gunno].bill.energymoney = sum/BILL_PRICE_BASE;//100
	 
	 /*服务费计算*/
	 if( SRV_ENERGY_TYPE == gFeeData[gunno].real.serivetype ) {
			gFeeData[gunno].bill.serivemoney = sumSrv/BILL_PRICE_BASE;//100
	 }else if(SRV_TIMES_TYPE == gFeeData[gunno].real.serivetype) {
		 gFeeData[gunno].bill.serivemoney = ptrfee->SectTimeFee.srvrate[0];
	 }else { /*不收服务费*/
		 gFeeData[gunno].bill.serivemoney = 0;
	 }
	 
	 
	 
	 /*预约费计算*/
	 if(BESPK_TIME_TYPE == gFeeData[gunno].real.bespktype ) {
		 gFeeData[gunno].bill.bespkmoney = gFeeData[gunno].bill.chgsecs/60 * ptrfee->SectTimeFee.bespkrate[0];
	 }else if (BESPK_TMES_TYPE == gFeeData[gunno].real.bespktype) {
		 gFeeData[gunno].bill.bespkmoney = ptrfee->SectTimeFee.bespkrate[0];
	 }else {
		 gFeeData[gunno].bill.bespkmoney = 0;
	 }
	 
	 /*停车费*/
	 if( PARKT_TIME_TYPE == gFeeData[gunno].real.parktype ) {
		 gFeeData[gunno].bill.parkmoney = gFeeData[gunno].bill.chgsecs/60 * ptrfee->SectTimeFee.parkrate[0];
	 }else if (PARKT_TMES_TYPE == gFeeData[gunno].real.parktype) {
		 gFeeData[gunno].bill.bespkmoney = ptrfee->SectTimeFee.parkrate[0];
	 }else {
		 gFeeData[gunno].bill.parkmoney = 0;
	 }
	
	 gFeeData[gunno].bill.billmoney = gFeeData[gunno].bill.energymoney \
	 + gFeeData[gunno].bill.serivemoney + gFeeData[gunno].bill.bespkmoney + gFeeData[AGUN_NO].bill.parkmoney;


	 return gFeeData[gunno].bill.billmoney;
	 
}


/*实时计费数据计算,返回本次计算电能量,最小计算电量*/
uint32 Bill_RealCalc(uint8 gunNo,uint8 minEry)
{
   uint32 chgenergy;	 
	 static uint32 sCalcEnergy[DEF_MAX_GUN_NO] = {0};
	 uint8 tmp,testtmp=0;//,i,sectNO;
	
	 DEV_METER_TYPE *pDcMet = TskDc_GetMeterDataPtr(gunNo);

	 gFeeData[gunNo].bill.stopenergy = pDcMet->energy;
	 tmp = Bill_FindSectNo();
	 gFeeData[gunNo].real.cursectno = tmp ;
	
//	 PARAM_FEE_TYPE *ptrfee = ChgData_GetFeeRate(gunNo);
	
	 gFeeData[gunNo].bill.stopsect = gFeeData[gunNo].real.cursectno;	
	 gFeeData[gunNo].bill.chgsecs = (GetSystemTick() - gFeeData[gunNo].real.startticks)/(TIM_1S); //秒	 
  
  /*充之鸟计算电损电量*/
    PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	  if(CONN_CHGBIRD == BackCOMM->agreetype){ 
			PARAM_FEE_TYPE *SysDevParam = ChgData_GetFeeRate(gunNo);
      gFeeData[gunNo].bill.energy = (gFeeData[gunNo].bill.stopenergy -  gFeeData[gunNo].bill.startenergy)*(100 + SysDevParam->SectTimeFee.lossrate)/100;
		}
		else{
			gFeeData[gunNo].bill.energy = gFeeData[gunNo].bill.stopenergy -  gFeeData[gunNo].bill.startenergy;
		}
		
	 /*bms数据*/
		BMSDATA_ST *pBms = Bms_GetBmsCarDataPtr(gunNo);
		gFeeData[gunNo].bill.soc = pBms->bcs.cursoc;
	  gFeeData[gunNo].real.lasttimes = pBms->bcs.lasttimes;
	 
    tmp = gFeeData[gunNo].real.cursectno;
		
		if(gFeeData[gunNo].bill.stopenergy < gFeeData[gunNo].real.tmpstartenergy)   //add0708zyf
		{
			return 0;
		}
		
    if( gBackSectNo[gunNo] == gFeeData[gunNo].real.cursectno ) {
			gFeeData[gunNo].bill.sectenergy[tmp] = gFeeData[gunNo].bill.stopenergy - gFeeData[gunNo].real.tmpstartenergy;	 //0.01Kwh
			
			
		}else if(gFeeData[gunNo].real.cursectno == (gBackSectNo[gunNo]+1) % MAX_HALFHOUR_SECT ){
			
			gFeeData[gunNo].bill.sectenergy[gBackSectNo[gunNo]] = gFeeData[gunNo].bill.stopenergy - gFeeData[gunNo].real.tmpstartenergy;	
			gFeeData[gunNo].real.tmpstartenergy = gFeeData[gunNo].bill.stopenergy;
			gBackSectNo[gunNo] = gFeeData[gunNo].real.cursectno; 			
		}
		else{
			tmp++;
		}
		
		/*防止突然断电*/
		Bill_SetStopData(gunNo,0,0);
		
		/*计算费用*/
		Bill_RealFee(gunNo);
		
		if( gFeeData[gunNo].bill.stopenergy > sCalcEnergy[gunNo]+100 ) {
			 sCalcEnergy[gunNo] = gFeeData[gunNo].bill.stopenergy;
			 return 100;
		}
	  return 0;
}




	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
