/**
  ******************************************************************************
  * @file    chgdata.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
*/
#include "TaskMainCtrl.h"
#include "ChgData.h"
#include "Screen.h"
#include <string.h>

//#define DEF_MAX_GUN_NO   2
//PARAM_COMM_TYPE gCommPara;

PARAM_OPER_TYPE gRunParam;

PARAM_DEV_TYPE gDevParam;

PARAM_FEE_TYPE gFeeRateParam[DEF_MAX_GUN_NO];

CHARGE_TYPE  gRunData[DEF_MAX_GUN_NO];

DEV_LOGIC_TYPE gLogic[DEF_MAX_GUN_NO];

START_PARAM gStartParam[DEF_MAX_GUN_NO];

#ifdef CP_PERIOD_FEE
PARAM_CP_PERIOD_FEE_TYPE gCpPeriodFee;
#endif

/*包含在设备参数中*/

DEV_LOGIC_TYPE * ChgData_GetLogicDataPtr(uint8 gunNo)
{
	 return &gLogic[gunNo];
}
//充电全部参数
CHARGE_TYPE *ChgData_GetRunDataPtr(uint8 gunNo)
{
	 return &gRunData[gunNo];
}


PARAM_DEV_TYPE *ChgData_GetDevParamPtr(void)
{
	 return &gDevParam;
}

PARAM_OPER_TYPE *ChgData_GetRunParamPtr(void)
{
	 return &gRunParam;
}

PARAM_FEE_TYPE *ChgData_GetFeeRate(uint8 gunNo)
{
	 return &gFeeRateParam[gunNo];
}

PARAM_COMM_TYPE *ChgData_GetCommParaPtr(void)
{
	return &gDevParam.bkcomm;
}

START_PARAM *ChgData_GetStartParaPtr(uint8 gunNo)
{
	return &gStartParam[gunNo%DEF_MAX_GUN_NO];
}

#ifdef CP_PERIOD_FEE
PARAM_CP_PERIOD_FEE_TYPE *ChgData_GetCpPeriodFeeRate(void)
{
	 return &gCpPeriodFee;
}

#endif

const uint8 conDefLocalIp[] = {192,168,10,185};

#if 0
const uint8 conDefRemoteIp[] = {192,168,10,104};
#define REMOTE_PORT 9001
#else

const uint8 conDefRemoteIp[] = {119,23,161,46};
#define REMOTE_PORT 8000

#endif
const uint8 conDefNetMask[] = {255,255,255,0};
const uint8 conDefNetGate[] = {192,168,10,1};

const char *conDefSsid = "af";
const char *conDefPasswd ="af123456789";

const uint8 conKey[] = {0xAA,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0xBB,0x11,0x22,0x33,0x44,0x55,0x66,0x77};

uint8 ChgData_InitDefaultDevPara(void)
{
		memset(&gDevParam,0,sizeof(PARAM_DEV_TYPE));
	
	  memset(&gStartParam,0,sizeof(START_PARAM));

		gDevParam.dcmetbaud = 9600;
		gDevParam.dcmetAdr = 0;
		gDevParam.chargeAddr = 1;
	
	strcpy((char *)gDevParam.chargeId,"1812070001");
//	  strcpy((char *)gDevParam.chargeId,"75512345689");
		gDevParam.onlinecard = E_ONLINE_CARD;
	
	 gDevParam.maxmodnum = CD_MAX_NUM;
	 memcpy(gDevParam.musrpasswd,"111111",sizeof(gDevParam.musrpasswd));
	
	/*卡相关*/
	memcpy(gDevParam.card.safe.keyCode,conKey,sizeof(gDevParam.card.safe.keyCode));  /*界面待修改*/
	
	/*网络相关*/

	 gDevParam.bkcomm.conntype = 2;  //2:RJ45  0：wifi  1 4G

	 memcpy(gDevParam.bkcomm.netpara.LocalIpAddr,conDefLocalIp,4);
	 memcpy(gDevParam.bkcomm.netpara.RemoteIpAddr,conDefRemoteIp,4);
	 gDevParam.bkcomm.netpara.RemotePort = REMOTE_PORT;
	 
	 memcpy(gDevParam.bkcomm.netpara.NetMask,conDefNetMask,4);
	 memcpy(gDevParam.bkcomm.netpara.NetGate,conDefNetGate,4);

	 /*wifi相关*/
	 gDevParam.bkcomm.netpara.StaPara.DhcpEn = 1;//默认动态IP地址

	 gDevParam.bkcomm.netpara.StaPara.LocalSocket = 5005;
	 memcpy(gDevParam.bkcomm.netpara.StaPara.Ssid,(void *)conDefSsid,strlen(conDefSsid));
	 memcpy(gDevParam.bkcomm.netpara.StaPara.Password,(void *)conDefPasswd,strlen(conDefPasswd));
	 gDevParam.bkcomm.netpara.StaPara.SecuEn = 1;
	 
	 gDevParam.safeclass = E_SAFE3;
	 strcpy((char *)&gDevParam.telephone,"7551234567");
	 

 return 0;
}

#ifdef CP_PERIOD_FEE
uint8 ChgData_InitDefaultCpPeriodFeePara(void)
{
	uint8 i;
	for( i = 0; i < MAXSFEILVNUMS ; i++ ) {
		
		gCpPeriodFee.BackFeiLv[i].FeiLv= BILL_PRICE_BASE;  /*电费    1元*/
		gCpPeriodFee.BackFeiLv[i].startH=0;  /*起始时0*/
		gCpPeriodFee.BackFeiLv[i].startM=0;  /*起始分0*/
		gCpPeriodFee.BackFeiLv[i].stopH=0;  /*结束时0*/
		gCpPeriodFee.BackFeiLv[i].stopM=0;  /*结束分0*/
	}	

	return 0;
}

#endif

uint8 ChgData_InitDefaultFeePara(void)
{
	uint8 i;
	for( i = 0; i < MAXSFEILVNUMS ; i++ ) {
		
		gFeeRateParam[AGUN_NO].SectTimeFee.price[i] = BILL_PRICE_BASE;  /*电费    1元*/
		gFeeRateParam[AGUN_NO].SectTimeFee.srvrate[i] =0;  /*服务费  0*/
		gFeeRateParam[AGUN_NO].SectTimeFee.bespkrate[i]=0; /*预约费 0*/
		gFeeRateParam[AGUN_NO].SectTimeFee.parkrate[i] = 0;/*停车费  0*/
	}
	gFeeRateParam[AGUN_NO].SectTimeFee.serivetype = SRV_ENERGY_TYPE;  /*服务费收费方式  按时间 按电量 按次*/
	gFeeRateParam[AGUN_NO].SectTimeFee.bespktype = BESPK_NULL_TYPE; /*预约费收费方式  按时间 按电量 按次*/
  gFeeRateParam[AGUN_NO].SectTimeFee.parktype = PARKT_NULL_TYPE;  /*停车费收费方式  按时间 按电量 按次*/
	
	for( i = 0 ; i < 48 ; i++ ) {
		gFeeRateParam[AGUN_NO].SectTimeFee.sectNo[i] = 0;
	}
	
	memcpy(&gFeeRateParam[BGUN_NO],&gFeeRateParam[AGUN_NO],sizeof(PARAM_FEE_TYPE));
	return 0;
}

//定义叉车充电机的一些默认参数  
static uint16 MaxCurr_system_test = 180;//200;
static uint16 MinCurr_system_test = 1;
static uint16 MaxModnum_system_test = 2;
static uint16 SingleModnumVol_system_test = 100;

uint8 ChgData_InitDefaultOperPara(void)
{
		gRunParam.maxvolt = MODULE_MAXVOL;//VOLT_TRAN(750);     /*0.1*/
	  gRunParam.minvolt = MODULE_MINVOL;//VOLT_TRAN(200);
	  gRunParam.maxcurr = CURR_10TRAN(MaxCurr_system_test);
	  gRunParam.mincurr = CURR_10TRAN(MinCurr_system_test);
	  
	  gRunParam.overcurr = CURR_10TRAN(MaxCurr_system_test+2);
	  gRunParam.overdcvolt = MODULE_MAXVOL+20;//VOLT_TRAN(750);
	  gRunParam.underdcvolt = MODULE_MINVOL;//VOLT_TRAN(200);
	
	  gRunParam.envmaxtemper = 85;
	  gRunParam.gunmaxtemper = 85;
	  
	  gRunParam.modnum = MaxModnum_system_test;
	  gRunParam.singmodcurr = CURR_10TRAN(SingleModnumVol_system_test);
	  gRunParam.overacvolt =  VOLT_TRAN(456);
	  gRunParam.underacvolt = VOLT_TRAN(304);
	  
	  gRunParam.isoresist = 50; /*K欧姆*/
	  gRunParam.elockallow = 1; /*默认不检测故障*/
		gRunParam.elocktm = 20;
		gRunParam.bhmtm = 40;
		gRunParam.xftm =  20;
		gRunParam.brmtm = 20;
		gRunParam.bcptm = 10;
		gRunParam.brotm = 13;
		gRunParam.bcltm = 10;
		gRunParam.bcstm = 10;
		gRunParam.Sysparalarm.bits.eleunlock = 0; /*默认不解锁*/
		gRunParam.Sysparalarm.bits.systemtype = DEV_DC_D;
		
		return 0;
}


void  ChgData_SetStaticOperPara(void)
{
	
	//gRunParam.singmodcurr = CURR_10TRAN(25);  /*单模块输出电流值*/
	
}



