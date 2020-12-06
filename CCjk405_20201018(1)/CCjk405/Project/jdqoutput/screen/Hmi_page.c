/**
  ******************************************************************************
  * @file    Hmi_page.c
  * @author  zqj
  * @version v1.0
  * @date    2016-03-08
  * @brief   
  ******************************************************************************
	*/
//#include "M_Global.h"
#include "Hmi_Api.h"
#include "Common.h"
#include "Hmipage.h"
#include "message.h"
#include "Screen.h"
#include "BillingUnit.h"
#include "rtc.h"
#include "secrt.h"
#include <stdio.h>
#include <stdlib.h>
#include "ChgData.h"
#include "TaskAcMeter.h"
#include "TaskIsoCheck.h"
#include <stdlib.h>
#include "RecordFat.h"
#include "gpio.h"
#include "RelayOut.h"
#include "TaskBackComm.h"

#define MAX_ENERGY_LIMIT  600
#define MIN_ENERGY_LIMIT  1
#define MAX_MONEY_LIMIT   600
#define MIN_MONEY_LIMIT   1
#define MAX_TIMES_LIMIT   8000
#define MIN_TIMES_LIMIT   1

#define MAX_ENERGY_LIMIT_RELA  (MAX_ENERGY_LIMIT)
#define MIN_ENERGY_LIMIT_RELA  (MIN_ENERGY_LIMIT)
#define MAX_MONEY_LIMIT_RELA   (MAX_MONEY_LIMIT)
#define MIN_MONEY_LIMIT_RELA   (MIN_MONEY_LIMIT)
#define MAX_TIMES_LIMIT_RELA    MAX_TIMES_LIMIT
#define MIN_TIMES_LIMIT_RELA    MIN_TIMES_LIMIT




#define  SC_VAR    sGlocalPara.rcv         /*触摸屏变量*/

extern CARD_OPER_DATA gCardOperData;
extern SCREN_CTRLPARA gScrenCtrlPara;

uint8 gSetPage = 0;  //页码

extern uint8 gUploadBuf[];

extern ScreenLocalPara_t  sGlocalPara;

/*显示文本信息*/
extern void Screen_ShowMessage(char *message,uint16 adr);

/*开启超时计数*/
extern void Screen_StartDownCounter(uint8 picNo,uint16 sec);
/*停止超时计数*/
extern void Screen_DownCounterStop(void);

/*触摸屏串口数据发送函数*/
int32 Screen_UartWrite(uint8 *buf,	int32 size);
/*充电记录页面刷新*/
uint8  Page_RfrashQueryPage(uint32 page);
/*费率参数读*/
uint8 Page_ParaRateRead(uint8 gunNo);
/*网络参数显示*/
void Pagez_NetParaShow(void);
/*网络参数设置*/
uint8 Page_NetParaSetToBoard(void);
/*高级参数设置2 读*/
void Page_ParaRead3(void);

extern uint8 UpCardNoASCII(uint8 *buff,uint8 *CardNo);	

uint8 Page_Note_StartChargMode(uint8 gunno,uint8 startchargmode);

/*返回页面*/
uint8 Page_GetSystemPageNo(uint8 currNo)
{
	uint8 rtnpageNo = 0;
	PARAM_DEV_TYPE *PtrDevPara = ChgData_GetDevParamPtr();
  if( PtrDevPara->safeclass < E_SAFE2 ) {
		sGlocalPara.Sysflag = E_SAFE1;
	}
		if(E_SAFE1 == sGlocalPara.Sysflag)
	  {
			 rtnpageNo = E_PICTURE23;
    }
		else
		{
			 rtnpageNo = E_PICTURE23;//E_PICTURE22;
    }  

	return rtnpageNo;
}

void Screen_ClearReadBuff(void)
{
	uint8 cnt,rtn;
	uint8 buf[64];
	
	cnt = 0;
	while(cnt < 10){
     rtn = Screen_UartRead(buf,32) ;   //读可能存在的残余数据
     if(0 == rtn){
         break;
     }
     cnt++;
   }
	 
}
/*获取寄存器数据*/
uint8 Page_GetSameValData(uint8 *buf,uint16 adr,uint8 regnum)
{
	 uint8 sendbuf[60] = {0};
	 
   int32 len,tlen;
   uint16 tmp16,tmplen;
   uint8 * pRcv = buf;

//   uint8 getlen = 0;
	 uint8 cnt;

	 //读可能存在的残余数据	 	 
	 Delay10Ms(10);
	 Screen_ClearReadBuff();
   tmp16 = Hmi_PackQueryReg(sendbuf,adr,CMD_RD_VAR,regnum);
   Screen_UartWrite(sendbuf,tmp16);
	 Delay10Ms(2);
   //Delay10Ms(regnum+16); 
	 cnt = 0;
   tlen = 0;
	 tmplen = 0;
   do {
		len = Screen_UartRead(pRcv,160-tlen);
		if( len > 0 ) {
		  pRcv += len;
		  tlen += len;
			if( tlen >= regnum*2 + 9 ) {
				 break;
			}	
      cnt = 0;			
		}else {
			if( ( tmplen == tlen )&& ( tlen > 3 )) {
					break;
			}
			cnt++;
			tmplen = tlen;
			Delay10Ms(2);
		}
  }while( cnt < 15 ); /*100ms 没收到字节退出*/
		
  	Delay10Ms(5);  //两次发送命令要延时，收到数据等待延时在发请求
	Screen_ClearReadBuff();
	
	if( tlen < 6 ) {
		return FALSE;
	}
	 
	tlen = tmplen > tlen ? tmplen: tlen;
	
//	printf("Page_GetSameValData len=%d\r\n", tlen);
	/*校验报文*/
	if( RT_ERROR == Hmi_PackgCheck(buf,tlen) ) {
		 return FALSE;
	}
	tmp16 = (buf[4] << 8) + buf[5];
	if(tmp16 == adr ) {		
		return TRUE;
	}
	return FALSE;
}

void Gun_StartPara(uint8 gun_no)
{
	START_PARAM  *PtrStartPara = ChgData_GetStartParaPtr(gun_no);
	CHARGE_TYPE  *BgRunData = ChgData_GetRunDataPtr(gun_no);
	
	uint8 sCtrData[2] ; 
	
	sCtrData[0] = 1;//启动
	
	memset(PtrStartPara->account,0,20);
	PtrStartPara->gunNo = gun_no;
	PtrStartPara->startby = START_BY_GUN;
	PtrStartPara->money = 1000000;
	PtrStartPara->chgmod = CHG_AUTO_TYPE;
	PtrStartPara->vailfg = 1;
	if(AGUN_NO == gun_no)
	{
	  SendMsgWithNByte(MSG_START_STOP,2,sCtrData,APP_TASK_AGUNMAINCTRL_PRIO);
	}
	else if(BGUN_NO == gun_no)
	{
		SendMsgWithNByte(MSG_START_STOP,2,sCtrData,APP_TASK_BGUNMAINCTRL_PRIO);
	}
	
}

extern uint8 TskBack_CommStatus(void);
uint8 Gun_VINStartPara(uint8 gun_no)
{
	START_PARAM  *PtrStartPara = ChgData_GetStartParaPtr(gun_no);
	CHARGE_TYPE  *BgRunData = ChgData_GetRunDataPtr(gun_no);
	
	uint8 sCtrData[2] ,errfg =0,msg[3] = {0}; 
	
	if(1 == TskBack_CommStatus()){
		errfg = 1;
	}
	else{
//		sCtrData[0] = 1;//启动
		
//		memset(PtrStartPara->account,0,20);
//		PtrStartPara->gunNo = gun_no;
//		PtrStartPara->startby = START_BY_VIN;
//		PtrStartPara->money = 1000000;
//		PtrStartPara->chgmod = CHG_AUTO_TYPE;
//		PtrStartPara->vailfg = 1;
//		if(AGUN_NO == gun_no)
//		{
//			SendMsgWithNByte(MSG_START_STOP,2,sCtrData,APP_TASK_AGUNMAINCTRL_PRIO);
//		}
//		else if(BGUN_NO == gun_no)
//		{
//			SendMsgWithNByte(MSG_START_STOP,2,sCtrData,APP_TASK_BGUNMAINCTRL_PRIO);
//		}
		if( 1 == Page_Note_StartChargMode(gun_no,START_BY_VIN) )
		{
			PtrStartPara->startby = START_BY_VIN;
			PtrStartPara->chgmod = CHG_AUTO_TYPE;
			Screen_SetChgFlag(0);
			sGlocalPara.startkeylasttm[gun_no] = GetSystemTick();
			sCtrData[0] = gun_no;
			sCtrData[1] = START_BY_VIN;
			if(AGUN_NO == gun_no)
			{
				SendMsgWithNByte(MSG_ONLINE_CARD,2,sCtrData,APP_TASK_BACK_PRIO);
			}
			else
			{
				SendMsgWithNByte(MSG_ONLINE_CARD,2,sCtrData,APP_TASK_BACK_PRIO);
			}
			
			msg[0] = gun_no;
			msg[1] = IDEL;
			SendMsgWithNByte(MSG_WORK_STATU,5,&msg[0],APP_TASK_SCREEN_PRIO);		
			
		}		
  }
	Delay10Ms(3);
	return errfg;
}
/*开始充电按钮*/
void Page_KeyVINStart_A()
{ 
	 char *text;
	
	//澄鹏VIN充电不支持 patli 20190927 
	
	PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	
	if(CONN_CHCP == BackCOMM->agreetype){
		 Screen_ShowMessage((char *)"  ",DGUS_IDLE_PROADR_A);
		 text = "VIN充电不支持";
		 Screen_ShowMessage(text,DGUS_IDLE_PROADR_A);
		
	}else{
		
	 if(Gun_VINStartPara(AGUN_NO)){
		 Screen_ShowMessage((char *)"  ",DGUS_IDLE_PROADR_A);
		 text = "网络故障，无法VIN启动";
		 Screen_ShowMessage(text,DGUS_IDLE_PROADR_A);	
	 }
 }
}



//extern GUN_FIRSTCONNC_ST *Get_GunFirstConncST(uint8 gunno);
//-----------add0529zyf  主动开启充电提示
uint8 Page_Note_StartChargMode(uint8 gunno,uint8 startchargmode)
{
	char *text ;
	uint8 msg[2],flag = 0;
	//CHARGE_TYPE  *BgRunData = ChgData_GetRunDataPtr(gunno);
	switch(startchargmode)
	{
		case START_BY_CARD:
		case START_BY_ONLINECARD:
			Screen_ShowMessage((char *)"  ",DGUS_IDLE_PROADR_B);
			Screen_ShowMessage((char *)"  ",DGUS_IDLE_PROADR_A);

			if(AGUN_NO == gunno)
			{
				text = "已选择A枪,请刷卡开始充电";
				Screen_ShowMessage(text,DGUS_IDLE_PROADR_A);
			}
			else
			{
				text = "已选择B枪,请刷卡开始充电";
				Screen_ShowMessage(text,DGUS_IDLE_PROADR_B);
			}
			msg[0] = gunno; 
			SendMsgWithNByte(MSG_SELECT_GUNCARD,1,&msg[0],APP_TASK_CARD_PRIO);
			sGlocalPara.gun_no = gunno;
			
			break;
		case START_BY_VIN:
//			Screen_ShowMessage((char *)"  ",DGUS_IDLE_PROADR_B);
//			Screen_ShowMessage((char *)"  ",DGUS_IDLE_PROADR_A);
//		
//			if(AGUN_NO == gunno)
//			{
//				if(CC1_4V != BgRunData->gun->statu.bits.cc1stu)
//				{
//					text = "已选择A枪,请插枪！！！";
//				}
//				else
//				{
//					if(1 < Get_GunFirstConncST(AGUN_NO)->u8GunFirstconnecFlag)
//					{
//						text = "A枪VIN提取中,请稍后!!";
//					}
//					else
//					{
//						if(0 == BgRunData->bms->car.brm.vin[0])
//						{
//							text = "A枪VIN获取失败,请重新插枪";
//						}
//						else
//						{
//							text = "VIN后台验证中,请稍等！！！";
//							flag = 1;
//						}
//					}
//				}
//				Screen_ShowMessage(text,DGUS_IDLE_PROADR_A);	
//			}
//			else
//			{
//				if(CC1_4V != BgRunData->gun->statu.bits.cc1stu)
//				{
//					text = "已选择B枪,请插枪！！！";
//				}
//				else
//				{
//					if(1 < Get_GunFirstConncST(BGUN_NO)->u8GunFirstconnecFlag)
//					{
//						text = "B枪VIN提取中,请稍后!!";
//					}
//					else
//					{
//						if(0 == BgRunData->bms->car.brm.vin[0])
//						{
//							text = "B枪VIN获取失败,请重新插枪";
//						}
//						else
//						{
//							text = "VIN后台验证中,请稍等！！！";
//							flag = 1;
//						}
//					}
//				}
//				Screen_ShowMessage(text,DGUS_IDLE_PROADR_B);	
//				
//			}

			break;
		case START_BY_PASSWD:

			break;

		default:
			break;
	}
	return flag;
}

void Page_StartCharge_A()
{ 
//		

	#if (SUPPORT_GUN_START == GUNSTART_SUPPORT )
	 Gun_StartPara(AGUN_NO);
	#else
	  char *text ;
	  uint8 msg[2];
//----------------------modify0529zyf
		PARAM_DEV_TYPE *ptrDevPara = ChgData_GetDevParamPtr();
		START_PARAM  *PtrStartPara = ChgData_GetStartParaPtr(AGUN_NO);
	  PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
		if( (ptrDevPara->onlinecard == E_ONLINE_CARD) && (CONN_CHGBIRD == BackCOMM->agreetype) )
		{
			sGlocalPara.gun_no = AGUN_NO;
			PtrStartPara->gunNo = AGUN_NO;
			Screen_StartDownCounter(E_PICTURE51,DEFAULT_COUNTDOWN_VALUE);
			Screen_SetChgFlag(1);
			Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSWSHOW,9);
			Hmi_ClearReg(DGUS_STARTMODE_CHARGACCOUNT,9);
			Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSW,9);
			Hmi_ChangePicture(E_PICTURE51);
			return;

		}
//-----------------------
		Page_Note_StartChargMode(AGUN_NO,START_BY_CARD);
//	  Screen_ShowMessage((char *)"  ",DGUS_IDLE_PROADR_B);
//		Screen_ShowMessage((char *)"  ",DGUS_IDLE_PROADR_A);
//	
//		text = "已选择A枪,请刷卡开始充电";
//		Screen_ShowMessage(text,DGUS_IDLE_PROADR_A);	
//	  msg[0] = AGUN_NO; 
//	  SendMsgWithNByte(MSG_SELECT_GUNCARD,1,&msg[0],APP_TASK_CARD_PRIO);
//    sGlocalPara.gun_no = AGUN_NO;
	#endif
}

/*开始充电按钮*/
void Page_KeyVINStart_B()
{ 
	 char *text;
	
	//澄鹏VIN充电不支持 patli 20190927 
	
	PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	
	if(CONN_CHCP == BackCOMM->agreetype){
		 Screen_ShowMessage((char *)"  ",DGUS_IDLE_PROADR_B);
		 text = "VIN充电不支持";
		 Screen_ShowMessage(text,DGUS_IDLE_PROADR_B);
		
	}else{	
		
	 if(Gun_VINStartPara(BGUN_NO)){
		 Screen_ShowMessage((char *)"  ",DGUS_IDLE_PROADR_B);
		 text = "网络故障，无法VIN启动";
		 Screen_ShowMessage(text,DGUS_IDLE_PROADR_B);	
	 }
 }
}

void Page_StartCharge_B()
{ 
	#if (SUPPORT_GUN_START == GUNSTART_SUPPORT )
	 Gun_StartPara(BGUN_NO);
	#else
//----------------------modify0529zyf
		PARAM_DEV_TYPE *ptrDevPara = ChgData_GetDevParamPtr();
	  PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
		if( (ptrDevPara->onlinecard == E_ONLINE_CARD) && (CONN_CHGBIRD == BackCOMM->agreetype) )
		{
			sGlocalPara.gun_no = BGUN_NO;
			Screen_StartDownCounter(E_PICTURE51,DEFAULT_COUNTDOWN_VALUE);
			Screen_SetChgFlag(1);
			Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSWSHOW,9);
			Hmi_ClearReg(DGUS_STARTMODE_CHARGACCOUNT,9);
			Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSW,9);
			Hmi_ChangePicture(E_PICTURE51);
			return;
		}
//-----------------------
		Page_Note_StartChargMode(BGUN_NO,START_BY_CARD);
//	 uint8 msg[2]; 
//	 char *text;
//	 Screen_ShowMessage((char *)"  ",DGUS_IDLE_PROADR_A);
//	 Screen_ShowMessage((char *)"  ",DGUS_IDLE_PROADR_B);
//	 text = "已选择B枪,请刷卡开始充电";
//	 Screen_ShowMessage(text,DGUS_IDLE_PROADR_B);
//	 msg[0] = BGUN_NO;
//	 SendMsgWithNByte(MSG_SELECT_GUNCARD,1,&msg[0],APP_TASK_CARD_PRIO);
//   sGlocalPara.gun_no = BGUN_NO;
	#endif
}
/*系统设置*/
void Page_SysSet()
{
	  uint8 tmppage;
    sGlocalPara.setparafg = 1;
	  tmppage = Page_GetSystemPageNo(E_PICTURE1);
	  Screen_SetChgFlag(1);	
	  Hmi_ChangePicture(tmppage);

}

/*设置金额充电响应函数*/
void Page_KeySetMoneyChargeMode()
{
	 uint8 buf[64] = {0};
	
	 Screen_ShowMessage((char *)"  ",DGUS_INPUT_PROADR);
	 sprintf((char *)buf,"请输入%d ~ %d 之间的数！",MIN_MONEY_LIMIT,MAX_MONEY_LIMIT);
	 Screen_ShowMessage((char *)buf,DGUS_INPUT_PROADR);
	 Screen_StartDownCounter(E_PICTURE19,DEFAULT_COUNTDOWN_VALUE);
	 Hmi_ChangePicture(E_PICTURE19);
}

/*设置电能充电模式*/
void Page_KeySetEnergyChargeMode()
{
	 uint8 buf[64] = {0};
	 
 	 Screen_ShowMessage((char *)"  ",DGUS_INPUT_PROADR);
	 sprintf((char *)buf,"请输入%d ~ %d 之间的数！",MIN_ENERGY_LIMIT_RELA,MAX_ENERGY_LIMIT_RELA);		
	 Screen_ShowMessage((char *)buf,DGUS_INPUT_PROADR);
	 Screen_StartDownCounter(E_PICTURE18,DEFAULT_COUNTDOWN_VALUE);
	 Hmi_ChangePicture(E_PICTURE18);

}

/*设置时间充电模式*/
void Page_KeySetTimesChargveMode() 
{
	 uint8 buf[64] = {0};
	 
	 Screen_ShowMessage((char *)"  ",DGUS_INPUT_PROADR);
	 sprintf((char *)buf,"请输入%d ~ %d 之间的数！",MIN_TIMES_LIMIT_RELA,MAX_TIMES_LIMIT_RELA);
	 Screen_ShowMessage((char *)buf,DGUS_INPUT_PROADR);
	 Screen_StartDownCounter(E_PICTURE20,DEFAULT_COUNTDOWN_VALUE);
	 Hmi_ChangePicture(E_PICTURE20);

}


/*退出充电方式选择页面响应函数*/
void  Page_KeyExitChoose()
{
	 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
	 Screen_DownCounterStop();  //停止倒计时
	 Screen_SetChgFlag(0);
}	

/*开始充电*/
void Page_VarStartCharging()
{	
	  uint8 sCtrData[2] ; 
	
	 	START_PARAM  *PtrStartPara = ChgData_GetStartParaPtr(sGlocalPara.gun_no);
	  CARD_INFO *ptrCardinfo = TskCard_GetCardInfPtr();
	
	  Screen_DownCounterStop();
	  sCtrData[0] = 1;//启动

	  memset(PtrStartPara->account,0,20);
	  PtrStartPara->gunNo = sGlocalPara.gun_no;
	  PtrStartPara->startby = START_BY_CARD;
	  PtrStartPara->money = ptrCardinfo->money;
    memcpy(PtrStartPara->account,ptrCardinfo->cardNo,sizeof(ptrCardinfo->cardNo));
	  Screen_SetChgFlag(0);
		if(AGUN_NO == sGlocalPara.gun_no)
	  {
		sCtrData[1] = AGUN_NO;//枪号
	    SendMsgWithNByte(MSG_START_STOP,2,sCtrData,APP_TASK_AGUNMAINCTRL_PRIO);
	  }
	  else if(BGUN_NO == sGlocalPara.gun_no)
	  {
		  sCtrData[1] = BGUN_NO;//枪号
		  SendMsgWithNByte(MSG_START_STOP,2,sCtrData,APP_TASK_BGUNMAINCTRL_PRIO);
	  }
	  Delay10Ms(10);
	
}

/*设置自动充电模式*/
void Page_KeySetAutoChargeMode()
{	
	 START_PARAM *StartPara = ChgData_GetStartParaPtr(sGlocalPara.gun_no);
	 if( StartPara->vailfg == 0 ) 
	 {
		  StartPara->vailfg = 1;
		  StartPara->chgmod = CHG_AUTO_TYPE;
	 }
	 Page_VarStartCharging();
}


/*设置按电量充电确定按钮*/
void Page_KeyEnergyEnter()
{
	 char *test;
   uint8 buf[64] = {0};
	 uint32 tempenergy;
	 
	 tempenergy = SC_VAR.charge.setenergy ;
	 if( tempenergy > MAX_ENERGY_LIMIT_RELA 
		 || tempenergy < MIN_ENERGY_LIMIT_RELA )
	 {
		  Screen_ShowMessage((char *)"  ",DGUS_INPUT_PROADR);
      test = "输入超过限制,请重新输入";
      Screen_ShowMessage(test,DGUS_INPUT_PROADR);
		  Delay10Ms(200);
		  sprintf((char *)buf,"请输入%d ~ %d 之间的数！",MIN_ENERGY_LIMIT_RELA,MAX_ENERGY_LIMIT_RELA);		
	    Screen_ShowMessage((char *)buf,DGUS_INPUT_PROADR);
		  return;
	 }
	 
	 START_PARAM *StartPara = ChgData_GetStartParaPtr(sGlocalPara.gun_no);
	 if( StartPara->vailfg == 0 ) 
	 {
		  StartPara->vailfg = 1;
		  StartPara->chgmod = CHG_ENERGY_TYPE;
	    StartPara->setenergy  = SC_VAR.charge.setenergy * 100; /*0.01Kwh*/
	 }
	 Page_VarStartCharging();
	 
}	


/*设定金额确定按钮*/
void Page_KeyMoneyEnter()
{
	 char *test;
   uint8 buf[64] = {0};
	 int32 tempmoney;

	 tempmoney = SC_VAR.charge.setmoney ;
	 if( tempmoney > MAX_MONEY_LIMIT_RELA 
		 || tempmoney < MIN_MONEY_LIMIT_RELA )
	 {
		  Screen_ShowMessage((char *)"  ",DGUS_INPUT_PROADR);
      test = "输入超过限制,请重新输入";
      Screen_ShowMessage(test,DGUS_INPUT_PROADR);
		  Delay10Ms(200);
		  sprintf((char *)buf,"请输入%d ~ %d 之间的数！",MIN_MONEY_LIMIT_RELA,MAX_MONEY_LIMIT_RELA);		
	    Screen_ShowMessage((char *)buf,DGUS_INPUT_PROADR);
		 return;
	 }
	 
	 START_PARAM *StartPara = ChgData_GetStartParaPtr(sGlocalPara.gun_no);
	 if( StartPara->vailfg == 0 ) 
	 {
		  StartPara->vailfg = 1;
		  StartPara->chgmod = CHG_MONEY_TYPE;
	    StartPara->setmoney  = SC_VAR.charge.setmoney * 100 ; /*0.01元*/
	 }
	 Page_VarStartCharging();
	
}

 /*设定时间确定按钮*/
void Page_KeyTimeEnter()
{
	 char *test;
   uint8 buf[64] = {0};
	 uint32 temptime;

	 temptime = SC_VAR.charge.settime;
	 if( temptime > MAX_TIMES_LIMIT_RELA 
		 || temptime< MIN_TIMES_LIMIT_RELA )
	 {
      Screen_ShowMessage((char *)"  ",DGUS_INPUT_PROADR);
      test = "输入超过限制,请重新输入";
      Screen_ShowMessage(test,DGUS_INPUT_PROADR);
		  Delay10Ms(200);
		  sprintf((char *)buf,"请输入%d ~ %d 之间的数！",MIN_TIMES_LIMIT_RELA,MAX_TIMES_LIMIT_RELA);		
	    Screen_ShowMessage((char *)buf,DGUS_INPUT_PROADR);
		 return;
	 }
	 
	 START_PARAM *StartPara = ChgData_GetStartParaPtr(sGlocalPara.gun_no);
	 if( StartPara->vailfg == 0 ) 
	 {
		  StartPara->vailfg = 1;
		  StartPara->chgmod = CHG_TIME_TYPE;
			StartPara->settime  = SC_VAR.charge.settime*60;  /*以秒为单位*/
	 }
	
	 Page_VarStartCharging();
}
 


/*金额、电量、时间、自动充电返回按钮*/ 
void Page_KeyChargeModeSetReturn()
{
   Hmi_ChangePicture(E_PICTURE17);
	 Screen_ShowMessage((char *)"  ",DGUS_INPUT_PROADR);
	 Screen_DownCounterStop();
}


void  BmsDetails(uint8 gunNo)
{
	 uint8 cnt = 0;
	 uint16 tmp16 = 0;
	 uint16 data[68] = {0};
   uint8 buf[100] = {0};
//   char *text;

    BMSDATA_ST *BmsDataCarPtr = Bms_GetBmsCarDataPtr(gunNo); 
	 
	 	tmp16 = BmsDataCarPtr->bcl.needcurr ;   //充电需求电流
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
	 
	  tmp16 = BmsDataCarPtr->bcl.needvolt ;   //充电需求电压
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;

    tmp16 = BmsDataCarPtr->bcp.tempmax ;   //最高允许温度
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;

   	tmp16 = BmsDataCarPtr->bcs.detecvolt;   //充电电压测量值   
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
	 
   	tmp16 = BmsDataCarPtr->bcs.deteccurr;   //充电电流测量值   
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;	

   	tmp16 = BmsDataCarPtr->bhm.maxvolte;   //最高允许充电电压
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
   	tmp16 = BmsDataCarPtr->bcp.maxcurr;    //最高允许充电电流   
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;		

   	tmp16 = BmsDataCarPtr->bcs.singbat.bits.volt/10;   //最高单体电池电压
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;	

   	tmp16 = BmsDataCarPtr->bcs.singbat.bits.groupno;   //最高单体电池组号  
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
   	tmp16 = BmsDataCarPtr->bsm.batmaxtemp;       //最高动力电池温度
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;		

   	tmp16 = BmsDataCarPtr->bsm.batmatempno;     //最高温度检测点编号
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
   	tmp16 = BmsDataCarPtr->bsm.batmintempno;     //最低电池温度检测点编号
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;

    tmp16 = BmsDataCarPtr->brm.battype;     //电池类型
		if(0xFF == tmp16)
		{
        tmp16 = 0x09;
    }
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
   	tmp16 = BmsDataCarPtr->bcp.sumenergy/10;     //电池容量
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;

   	tmp16 = BmsDataCarPtr->bcp.chgmaxvolt;     //电池总电压
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
   	tmp16 = BmsDataCarPtr->bsm.batmintemp;       //最低电池温度
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;			
		
   	tmp16 = BmsDataCarPtr->bsm.battalm.bits.socalm;       //单体SOC状态
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;	
				
   	tmp16 = BmsDataCarPtr->bsm.battalm.bits.tempalm;       //单体电池温度
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;	
		
   	tmp16 = BmsDataCarPtr->bsm.battalm.bits.allowed;       //单体电池允许
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;	
		
   	tmp16 = BmsDataCarPtr->bsm.battalm.bits.singbatValm;       //单体电池电压
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;		
		
   	tmp16 = BmsDataCarPtr->bsm.battalm.bits.curralm;       //单体电池电流状态	
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
   	tmp16 = BmsDataCarPtr->bsm.battalm.bits.insulalm;       //单体电池绝缘状态
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
			
	  tmp16 = Hmi_PackWriteReg(buf,(uint8*)&data,DGUS_BMSDETAIL_ADR,CMD_WR_VAR,2*cnt);
	  Screen_UartWrite(buf,tmp16);
		
//		text = (char *)(BmsDataCarPtr->brm.manufname);    //电池厂家
		memcpy(buf,BmsDataCarPtr->brm.manufname,sizeof(BmsDataCarPtr->brm.manufname));
		buf[sizeof(BmsDataCarPtr->brm.manufname)] = 0;
		Screen_ShowMessage((char *)buf,DGUS_BMSDETAIL_TEXT_ADR);  
		memset(buf,0,sizeof(buf));

		buf[0] = BmsDataCarPtr->brm.ver[0] + '0';
		buf[1] = BmsDataCarPtr->brm.ver[1] + '0';
		buf[2] = BmsDataCarPtr->brm.ver[2] + '0';		
		buf[3] = 0;                                       //BMS协议版本
		Screen_ShowMessage((char *)buf,DGUS_BMSDETAIL_TEXT_ADR+DGUS_BMSDETAIL_OFFSET);
			
}

/*A枪 BMS详情按钮*/
void Page_BMSDetails_A()
{ 
    BmsDetails(AGUN_NO); 
	  sGlocalPara.bmskey = AGUN_NO;
  	Hmi_ChangePicture(E_PICTURE21);
}

/*B枪 BMS详情*/
void Page_BMSDetails_B()
{ 
    BmsDetails(BGUN_NO); 
	  sGlocalPara.bmskey = BGUN_NO;
  	Hmi_ChangePicture(E_PICTURE21);
}

void Page_VINStop_A()
{
	 uint8 CtrData[3];
	 CHARGE_TYPE  *RunDataPrt = ChgData_GetRunDataPtr(AGUN_NO);
	
	 PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	 if((START_BY_PASSWD == RunDataPrt->logic->startby) || ((CONN_CHGBIRD == BackCOMM->agreetype) && (START_BY_BKGROUND == RunDataPrt->logic->startby)))   //add0612zyf
	 {
			sGlocalPara.gun_no = AGUN_NO;			
			Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSWSHOW,9);
			Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSW,9);
			Hmi_ChangePicture(E_PICTURE52);
			Screen_StartDownCounter(E_PICTURE52,DEFAULT_COUNTDOWN_VALUE);
		  Screen_SetChgFlag(1);
			return;
	 }
	
//	 if(START_BY_VIN == RunDataPrt->logic->startby)
	 if (SUPPORT_GUN_START == GUNSTART_SUPPORT )
	 {
		 CtrData[0] = 0;//停止
		 CtrData[1] = AGUN_NO;//枪号
		 CtrData[2] = 0;//0=按键  1=后台
		 SendMsgWithNByte(MSG_START_STOP,3,&CtrData[0],APP_TASK_AGUNMAINCTRL_PRIO);	
	 }
}
void Page_VINStop_B()
{
	 uint8 CtrData[3];
	 CHARGE_TYPE  *RunDataPrt = ChgData_GetRunDataPtr(BGUN_NO);

	 PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	 if((START_BY_PASSWD == RunDataPrt->logic->startby) || ((CONN_CHGBIRD == BackCOMM->agreetype) && (START_BY_BKGROUND == RunDataPrt->logic->startby)))   //add0612zyf
	 {
			sGlocalPara.gun_no = BGUN_NO;			
			Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSWSHOW,9);
			Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSW,9);
			Hmi_ChangePicture(E_PICTURE52);
			Screen_StartDownCounter(E_PICTURE52,DEFAULT_COUNTDOWN_VALUE);
		  Screen_SetChgFlag(1);
			return;
	 }
	
	 if(START_BY_VIN == RunDataPrt->logic->startby)//|| (START_BY_PASSWD == RunDataPrt->logic->startby))
	 {
		 CtrData[0] = 0;//停止
		 CtrData[1] = BGUN_NO;//枪号
		 CtrData[2] = 0;//0=按键  1=后台
		 SendMsgWithNByte(MSG_START_STOP,3,&CtrData[0],APP_TASK_BGUNMAINCTRL_PRIO);	
	 }
}
//const char *ModeErrString[] = \
//{
//	"正常",
//  "交流输入过压",
//	"交流输入欠压",
//	"交流过压关机",
//	"直流输出过压",
//	"直流过压关机",
//	"直流输出欠压",
//	"风扇不运行",
//	"过温保护",
//	"PFC过温保护1",
//	"PFC过温保护2",
//	"DC过温保护1",
//	"DC过温保护2",
//	"PFC与DC通信故障",
//	"PFC故障",
//	"DCDC故障",
//};



/*系统参数返回按钮*/
void Page_Sysreturn()
{
	PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	
	if(GUNNUM_2 == devparaPtr->gunnum){
    Hmi_ChangePicture(E_PICTURE1);
	}
	else{
		Hmi_ChangePicture(E_PICTURE47);
	}
	Screen_SetChgFlag(0);	
	if(E_SAFE1 == sGlocalPara.Sysflag )
	{
	    sGlocalPara.Sysflag = E_SAFE3;
		  sGlocalPara.setparafg = 0;
	}
}




/*系统设置按钮*/
void Page_SystemSet()
{	  
	 Page_ParaRead1();
	 Hmi_ChangePicture(E_PICTURE25);	  
}

/*系统设置按钮1*/
void Page_SystemSet1()
{
	 Hmi_ChangePicture(E_PICTURE33);
	 Screen_StartDownCounter(E_PICTURE33,DEFAULT_COUNTDOWN_VALUE);
	 gSetPage = E_PICTURE22;
}

/*充电记录按钮*/
uint32 gPage;
void Page_RechargeRecord()
{
  uint8 num;
	gPage = 0;
	
	num = Page_RfrashQueryPage(gPage);
	Hmi_ChangePicture(E_PICTURE28); 
  if(0 == num)
  {
		char *text = "充电记录已读完";
		Screen_ShowMessage(text,DGUS_RECORD_PROADR);
		Delay10Ms(200);
		Screen_ShowMessage(" ",DGUS_RECORD_PROADR);
  }
	else
	{
	   gPage = 1;
	}
	
}

/*A枪费率*/
void Page_KeyRateChoose_A()
{
   Hmi_ChangePicture(E_PICTURE29);
   Page_ParaRateRead(AGUN_NO);  	
}

/*B枪费率*/
void Page_KeyRateChoose_B()
{
  Hmi_ChangePicture(E_PICTURE40);
  Page_ParaRateRead(BGUN_NO);  	
}

/*费率选择界面返回*/
void Page_KeyRateChoose_Return()
{
   Hmi_ChangePicture(E_PICTURE23);
}


/*费率设置按钮*/

void Page_RatesSet()
{	
	//Page_ParaRateRead();  
  Hmi_ChangePicture(E_PICTURE33);	
	Screen_StartDownCounter(E_PICTURE33,DEFAULT_COUNTDOWN_VALUE);
	gSetPage = E_PICTURE23;
}

/*调试界面按钮*/
void Page_Debug()
{ 
	Page_DebugPage();
  Hmi_ChangePicture(E_PICTURE30);
	
}



/*清除一行充电记录显示*/
void Page_ClearOneLineRecord(uint8 line)
{
	uint8 i;
	uint16 DwinAddr,GunNoAddr;
	uint16 data[60] = {0};

	DwinAddr = DGUS_REC_TEXTSTART_ADR+line*4*DGUS_REC_TEXTSTART_OFFSET;
  GunNoAddr = DGUS_REC_GUNMUN_ADR+DGUS_REC_GUNMUN_OFFSET*line;
  for( i = 0 ; i < 4 ; i++ )
  {
	   Screen_ShowMessage(" ",DwinAddr);  //清除文本变量
		 DwinAddr += DGUS_REC_TEXTSTART_OFFSET;
  }
	Screen_ShowMessage(" ",GunNoAddr);
  Hmi_WriteMulitVar(DGUS_REC_DATASTART_ADR + line*3*0x02,&data[0],6);  //清除数据变量

}

/*清楚整页充电记录显示*/
void Page_ClearRecordPage(void)
{
	uint8 i;
	for(i = 0; i< 9 ; i++)
	{
	  Page_ClearOneLineRecord(i);
	  Delay10Ms(1);
	}
}
const char * conReasonString1[] = \
{
   "未知原因停止",
	 "刷卡停止",
	 "移动客户端停止 ",
	 "故障停止 ",
	 "达到设定条件",
	 "BMS主动停止",
	 "补传订单",
	 "触摸屏按键停止",
};

extern uint16 Rd_E2romReadRecord(uint16 absNo,CALCBILL_DATA_ST *bill);
/*刷新一个查询页面*/
uint8  Page_RfrashQueryPage(uint32 page)
{
	uint8 i,temp,rdNo;
  char str[4];
  uint8 text[164] = {0};
  uint16 TextAddr,DataAddr,GunNoAddr,EnerAddr;
	uint32 rtn,rnum;
  uint16 data[7];
  uint32 tmp32;


  CALCBILL_DATA_ST record;

	rdNo = page  * 9 +1;
	rnum = Rd_E2romReadRecord(rdNo,&record);
	if(0xffff == rnum )  
  { 		
		  Page_ClearRecordPage();
		  return 0; 		
  }
	
	sprintf((char *)text,"第 %d 页",page+1);
  Hmi_ShowText(DGUS_REC_PAGENO_ADR,strlen((char *)text),text);		
	TextAddr = DGUS_REC_TEXTSTART_ADR;
	DataAddr = DGUS_REC_DATASTART_ADR;
	GunNoAddr = DGUS_REC_GUNMUN_ADR;
	EnerAddr = DGUS_REC_ENERGY_ADR;
  for( i = 0 ; i < 9 ; i++ ) 
	{
		rnum = Rd_E2romReadRecord(rdNo+i,&record);
	  if(0xffff == rnum )  
    { 
			Page_ClearOneLineRecord(i);  /*清一行显示*/
		  continue;
    }
		
		if(AGUN_NO == record.gunno)
		{
			 strcpy((char *)text," A");
		}
		else if(BGUN_NO == record.gunno)
		{
		   strcpy((char *)text," B");
		}
		Hmi_ShowText(GunNoAddr,strlen((char *)text),text);	 /*显示枪号*/
		memset(text,0,sizeof(text));
		
		 /*显示卡号*/	 
		if( START_BY_VIN != record.startmod){
		 rtn = UpCardNoASCII(text,record.cardNo);// g_RunData.Ctrl.IcCardNo	
		}
		else{
			memcpy(text,record.vincode,17);
			rtn = 17;
		}
     text[rtn] = 0;
     Common_TranCarNo(text,rtn);		
		 Hmi_ShowText(TextAddr,rtn,text);
		 memset(text,0,sizeof(text));
			
		 TextAddr += DGUS_REC_TEXTSTART_OFFSET;
		 sprintf((char *)text,"%2d.%2d ",Common_Bcd_Decimal(record.starttm[1])%13\
																		,Common_Bcd_Decimal(record.starttm[2])%32 );		
      text[0] = (text[0] == 0x20)? 0x30 : text[0];
      text[3] = (text[3] == 0x20)? 0x30 : text[3];					
			//时
			sprintf(str,"%2d:",Common_Bcd_Decimal(record.starttm[3])%24 );
			str[0] = (str[0] == 0x20)? 0x30 : str[0];
			strcat((char *)text,str);
		 //分
		  sprintf(str,"%2d:",Common_Bcd_Decimal(record.starttm[4])%60 );
			str[0] = (str[0] == 0x20)? 0x30 : str[0];
			strcat((char *)text,str);
		 // 秒
			sprintf(str,"%2d",Common_Bcd_Decimal(record.starttm[5])%60 );
			str[0] = (str[0] == 0x20)? 0x30 : str[0];
			strcat((char *)text,str);	 
			/*显示开始时间*/
			Hmi_ShowText(TextAddr,strlen((char *)text),text);
			memset(text,0,sizeof(text));
		 		 
		  TextAddr += DGUS_REC_TEXTSTART_OFFSET;
			sprintf((char *)text,"%2d.%2d ",Common_Bcd_Decimal(record.stoptm[1])%13\
																		 ,Common_Bcd_Decimal(record.stoptm[2])%32 );
			text[0] = (text[0] == 0x20)? 0x30 : text[0];
      text[3] = (text[3] == 0x20)? 0x30 : text[3];																		 
			//时
			sprintf(str,"%2d:",Common_Bcd_Decimal(record.stoptm[3])%24 );
			str[0] = (str[0] == 0x20)? 0x30 : str[0];
			strcat((char *)text,str);
		 //分
		  sprintf(str,"%2d:",Common_Bcd_Decimal(record.stoptm[4])%60 );
			str[0] = (str[0] == 0x20)? 0x30 : str[0];
			strcat((char *)text,str);
		 // 秒
		  sprintf(str,"%2d",Common_Bcd_Decimal(record.stoptm[5])%60 );
			str[0] = (str[0] == 0x20)? 0x30 : str[0];
			strcat((char *)text,str);			
			/*显示结束时间*/				
			Hmi_ShowText(TextAddr,strlen((char *)text),text);
			memset(text,0,sizeof(text));
			
			/*显示原因*/
		  TextAddr += DGUS_REC_TEXTSTART_OFFSET;
		  temp = (sizeof(conReasonString1) / sizeof(char *)) + 1;
		  temp = record.endreason%temp;
			strcpy((char *)text,conReasonString1[temp]);	
			if(0x03 == temp)
			{
			  sprintf(str,"%d",record.errcode);  //故障码
			  strcat((char *)text,str);
			}
		  Hmi_ShowText(TextAddr,strlen((char *)text),text);
			memset(text,0,sizeof(text));
					
			tmp32 = i+1;
			Common_Change4Byte(&tmp32);
			data[0] = tmp32 & 0xffff;		 
			data[1] = tmp32 >> 16;	
			
			tmp32 = record.billmoney;   //消费金额
			Common_Change4Byte(&tmp32);
			data[2] = tmp32 & 0xffff;		 
			data[3] = tmp32 >> 16;
			
			tmp32 = record.aftermoney;   //余额
			Common_Change4Byte(&tmp32);
			data[4] = tmp32 & 0xffff;		 
			data[5] = tmp32 >> 16;
			
			tmp32 = Hmi_PackWriteReg(text,(uint8*)&data,DataAddr,CMD_WR_VAR,2*6);
			Screen_UartWrite(text,tmp32);	
			memset(text,0,sizeof(text));
			
			tmp32 = record.energy;        //电量
			Common_Change4Byte(&tmp32);
			data[0] = tmp32 & 0xffff;		 
			data[1] = tmp32 >> 16;
			tmp32 = Hmi_PackWriteReg(text,(uint8*)&data,EnerAddr,CMD_WR_VAR,2*2);
			Screen_UartWrite(text,tmp32);
						
      TextAddr  += DGUS_REC_TEXTSTART_OFFSET;
		  DataAddr  += 0x06;
		  GunNoAddr += DGUS_REC_GUNMUN_OFFSET;
		  EnerAddr += DGUS_REC_ENERGY_OFFSET;
	}
	
  return 1;
}


/*充电记录上一页按钮*/
//uint8 gPage;
void Page_KeyRecordUpPage()
{	
	 gPage = gPage > 1 ? gPage - 2 : gPage - 1;
   Page_RfrashQueryPage(gPage);
	 gPage++;
}	

/*充电记录下一页按钮*/
void Page_KeyRecordDownPage()
{   
	  uint32 num,pagemax;
    
	  /*最大页数*/
	  num = Rd_GetMaxRecord();
	  pagemax = num / 9;
	  pagemax = (num % 9) ? ++pagemax : pagemax;
	  if(gPage == pagemax)
		{
			char *text = "充电记录已读完";
			Screen_ShowMessage(text,DGUS_RECORD_PROADR);
			Delay10Ms(200);
			Screen_ShowMessage(" ",DGUS_RECORD_PROADR);	
    }
	
	  gPage = gPage >= pagemax ? gPage - 1  : gPage ;	
    Page_RfrashQueryPage(gPage);

    gPage++;
}

/*充电记录返回*/
void Page_KeyRecordReturn()
{
	  uint8 tmppage;
	  tmppage = Page_GetSystemPageNo(E_PICTURE28);
	  Hmi_ChangePicture(tmppage);
}

uint8 Page_ParaWrite1(uint8 *ndata)
{
	 uint16 tmp16 = 0;
   uint8 buf[164] = {0};
   uint16 *ptr ;
   uint8 msg[2];
	 
   PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	 
		if( FALSE == Page_GetSameValData(buf,DGUS_PARA_START_ADR,7)) {
			if( FALSE == Page_GetSameValData(buf,DGUS_PARA_START_ADR,7)) {
					char *text = "数据参数保存失败";
					Screen_ShowMessage(text,DGUS_SETPA_PROADR);
					Delay10Ms(150);
					Screen_ShowMessage(" ",DGUS_SETPA_PROADR);
					return FALSE;
			}
		}		
		ptr = (uint16 *)&buf[7];
		
		tmp16 = Common_Change2Byte(ptr++);
	  tmp16 = tmp16 > 65000 ? 65000 : tmp16 ;
		devparaPtr->overdcvolt = tmp16;   //直流过压值
		
		tmp16 = Common_Change2Byte(ptr++);
	  tmp16 = tmp16 > 65000 ? 65000 : tmp16 ;
		devparaPtr->overdccurr = tmp16;   //直流过流值
		
		tmp16 = Common_Change2Byte(ptr++);
	  tmp16 = tmp16 > 65000 ? 65000 : tmp16 ;
		devparaPtr->overacvolt = tmp16;   //交流过压值
		
		tmp16 = Common_Change2Byte(ptr++);
	  tmp16 = tmp16 > 65000 ? 65000 : tmp16 ;
		devparaPtr->underacvolt = tmp16;   //交流欠压值
		
		tmp16 = Common_Change2Byte(ptr++);
		tmp16 = tmp16 > 1000 ? 1000 : tmp16 ;
		devparaPtr->gunmaxtemper = tmp16;   //充电枪过温值
				
		tmp16 = Common_Change2Byte(ptr++);		
		tmp16 = tmp16 > 65000 ? 65000 : tmp16 ;
		devparaPtr->isoresist = tmp16;   //绝缘电阻告警值
		
		devparaPtr->minvolt = MODULE_MINVOL;
			
    if( FALSE == Page_NetParaSetToBoard())    //网络参数设置
		{
			 char *text = "网络参数保存失败";
	     Screen_ShowMessage(text,DGUS_SETPA_PROADR);
	     Delay10Ms(150);
	     Screen_ShowMessage(" ",DGUS_SETPA_PROADR);
		   return FALSE;
		}
//				SAVE_ALL_TYPE
//				  SAVE_OPER_TYPE
//				  SAVE_DEV_TYPE
//				  SAVE_FEE_TYPE
	msg[0] = SAVE_OPER_TYPE;
  SendMsgWithNByte(MSG_PARAM_STORE,1,&msg[0],APP_TASK_AGUNMAINCTRL_PRIO);	 //发送参数保存消息
			
		return TRUE;
}


uint8 Page_ParaWrite2(uint8 *ndata)
{
	 uint16 tmp16 = 0;
   uint8 buf[164] = {0};
   uint16 *ptr ;
	 uint8 msg[2],modenum[2];
	 uint8 i,fg;
	 PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	 PARAM_DEV_TYPE *chgparaPtr = ChgData_GetDevParamPtr();
	 
	 
	 	 fg = 0;
	   for( i = 0 ; i < 3 ; i++ ) {
			 if( TRUE == Page_GetSameValData(buf,DGUS_SENRA_START_ADR1,12)) {
				  fg = 1;
				  break;
			  }
	   }
		 if(0 == fg)
		 {
				char *text = "参数保存失败";
				Screen_ShowMessage(text,DGUS_SETSENPA_PROADR);
				Delay10Ms(150);
				Screen_ShowMessage(" ",DGUS_SETSENPA_PROADR);
				return FALSE;
			}
			
		ptr = (uint16 *)&buf[7];
		
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 2 ? 0 : tmp16;
	  devparaPtr->Sysparalarm.bits.BMSpower = tmp16 ;   //BMS电源
	
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 2 ? 0 : tmp16;
		devparaPtr->Sysparalarm.bits.curfewalarm = tmp16 ;   //门禁告警

		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 2 ? 0 : tmp16;
		devparaPtr->Sysparalarm.bits.curfewsignal = tmp16 ;       //门禁信号
		
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 2 ? 0 : tmp16;
		devparaPtr->Sysparalarm.bits.opencharge = tmp16;       //开门停充
		
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 2 ? 0 : tmp16;
		devparaPtr->Sysparalarm.bits.insulalarm = tmp16 ;   //绝缘告警 
		
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 2 ? 0 : tmp16;
		devparaPtr->Sysparalarm.bits.prevraysignl = tmp16 ;   //防雷信号
	
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 2 ? 0 : tmp16;
		devparaPtr->Sysparalarm.bits.fanalarm = tmp16;   //风机告警
		
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 2 ? 0 : tmp16;
		devparaPtr->Sysparalarm.bits.eleclock = tmp16;   //电子锁检测
		
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 2 ? 0 : tmp16;
		devparaPtr->Sysparalarm.bits.eleclocktype = tmp16;   //电子锁类型	
		
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 2 ? 0 : tmp16;
		devparaPtr->Sysparalarm.bits.eleclockback = tmp16;   //电子锁反馈   
	  devparaPtr->elockallow = tmp16;
		
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 2 ? 0 : tmp16;
		devparaPtr->Sysparalarm.bits.eleunlock = tmp16;  		 //电子锁解锁		
		
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 2 ? 0 : tmp16;
		devparaPtr->Sysparalarm.bits.batteryalarm = tmp16;   //电池反接告警

     fg = 0;
	   for( i = 0 ; i < 3 ; i++ ) {
			 if( TRUE == Page_GetSameValData(buf,DGUS_SENRA_START_ADR1+12,12)) {
				  fg = 1;
				  break;
			  }
	   }
		 if(0 == fg)
		 {
				char *text = "参数保存失败";
				Screen_ShowMessage(text,DGUS_SETSENPA_PROADR);
				Delay10Ms(150);
				Screen_ShowMessage(" ",DGUS_SETSENPA_PROADR);
				return FALSE;
			}
		 
		ptr = (uint16 *)&buf[7];	
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 10 ? 10 : tmp16;
		devparaPtr->Sysparalarm.bits.systemtype = tmp16;   		//系统类型	
    devparaPtr->gunnum = (devparaPtr->Sysparalarm.bits.systemtype<2)?GUNNUM_1:GUNNUM_2;	

		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 10 ? 10 : tmp16;
		devparaPtr->Sysparalarm.bits.chargemode = tmp16;   		// 充电模式
		
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > CD_MAX_NUM ? CD_MAX_NUM : tmp16;
		devparaPtr->modnum = tmp16;  				 // 模块总数
		
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > CD_MAX_NUM ? CD_MAX_NUM : tmp16;
		modenum[0] = tmp16;		  //一组模块数量

		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > CD_MAX_NUM ? CD_MAX_NUM : tmp16;
		modenum[1] = tmp16;    //二组模块数量
		
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 300 ? 300 : tmp16;
		 devparaPtr->Sysparalarm.bits.powersplitt = tmp16;  		 // 功率分配			

		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 500 ? 500 : tmp16;
		devparaPtr->Fanstartemper = tmp16;   // 风机启转温度				}		
		
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 500 ? 500 : tmp16;
		devparaPtr->Fanstoptemper = tmp16;   // 风机停止温度		
	
		tmp16 = Common_Change2Byte(ptr++);
	  tmp16 = tmp16 > 500 ? 500 : tmp16;
		devparaPtr->Fanturnstemper = tmp16;   // 风机转满温度			
			
		tmp16 = Common_Change2Byte(ptr++);
	  tmp16 = tmp16 > 2500 ? 2500 : tmp16;
		devparaPtr->maxcurr = tmp16;   				// 最大输出电流		
	
		tmp16 = Common_Change2Byte(ptr++);
    tmp16 = tmp16 > 8000 ? 8000 : tmp16;	
		devparaPtr->maxvolt = tmp16;   				// 最大输出电压	
	
		tmp16 = Common_Change2Byte(ptr++);    
		if( devparaPtr->maxvolt > 6000 ) {
			tmp16 = tmp16 > 500 ? 500 : tmp16;  //old is 300 ,A04版本修改  20190425
    }else {
      tmp16 = tmp16 > 1200 ? 1200 : tmp16;
    }			
		devparaPtr->singmodcurr = tmp16;  	 	//单模块额定电流 
		
		fg = 0;
		for( i = 0 ; i < 3 ; i++ ) {
			if( TRUE == Page_GetSameValData(buf,DGUS_SENRA_START_ADR1+24,10)) {
			   fg = 1;
			   break;
			}
		}
		if(0 == fg)
		{
			 char *text = "参数保存失败";
			 Screen_ShowMessage(text,DGUS_SETSENPA_PROADR);
			 Delay10Ms(150);
			 Screen_ShowMessage(" ",DGUS_SETSENPA_PROADR);
			 return FALSE;
		}
		ptr = (uint16 *)&buf[7];	
		tmp16 = Common_Change2Byte(ptr++);
		tmp16 = tmp16 > 300 ? 300 : tmp16;	
		devparaPtr->elocktm = tmp16;   				//电子锁超时	
	
		tmp16 = Common_Change2Byte(ptr++); 
		tmp16 = tmp16 > 300 ? 300 : tmp16;	
		devparaPtr->bhmtm = tmp16;   					//BHM超时
		
		tmp16 = Common_Change2Byte(ptr++);
		tmp16 = tmp16 > 300 ? 300 : tmp16;	
	  devparaPtr->xftm = tmp16;   					//泄放超时	
		
		tmp16 = Common_Change2Byte(ptr++);
		tmp16 = tmp16 > 300 ? 300 : tmp16;	
		devparaPtr->brmtm = tmp16;  				 	//BRM超时	
				
		tmp16 = Common_Change2Byte(ptr++);
		tmp16 = tmp16 > 300 ? 300 : tmp16;	
		devparaPtr->bcptm = tmp16;   					//BCP超时	
				
		tmp16 = Common_Change2Byte(ptr++);
		tmp16 = tmp16 > 300 ? 300 : tmp16;	
		devparaPtr->brotm = tmp16;   					//BRO超时	
				
		tmp16 = Common_Change2Byte(ptr++);
		tmp16 = tmp16 > 300 ? 300 : tmp16;	
		devparaPtr->bcltm = tmp16;   					//BCL超时	
				
		tmp16 = Common_Change2Byte(ptr++);
		tmp16 = tmp16 > 300 ? 300 : tmp16;	
		devparaPtr->bcstm = tmp16;   					//BCS超时	

		tmp16 = Common_Change2Byte(ptr++);
		tmp16 = tmp16 > 300 ? 300 : tmp16;	
	  devparaPtr->bsttm = tmp16;   					//BST超时	

		tmp16 = Common_Change2Byte(ptr++);
		tmp16 = tmp16 > 300 ? 300 : tmp16;	
		devparaPtr->bsdtm = tmp16;   					//BSD超时	
		
	 if(GUNNUM_1 == devparaPtr->gunnum)
   {  
		  if( devparaPtr->modnum == modenum[0])
			{
				devparaPtr->grpmodnum[0] = modenum[0];
				devparaPtr->grpmodnum[1] = 0;
				
				msg[0] = SAVE_ALL_TYPE;
				SendMsgWithNByte(MSG_PARAM_STORE,1,&msg[0],APP_TASK_AGUNMAINCTRL_PRIO);  //发送参数保存消息
				char *text = "参数保存成功！";
				Screen_ShowMessage(text,DGUS_SETSENPA_PROADR);
			}
			else
			{
				char *text = "一组模块个数要等于模块总数";
				Screen_ShowMessage(text,DGUS_SETSENPA_PROADR);
			}	  
		 
	 }
	 else if(GUNNUM_2 == devparaPtr->gunnum)
   {
			if( devparaPtr->modnum == ( modenum[0]+ modenum[1]))
			{
				devparaPtr->grpmodnum[0] = modenum[0];
				devparaPtr->grpmodnum[1] = modenum[1];
				
				msg[0] = SAVE_ALL_TYPE;
				SendMsgWithNByte(MSG_PARAM_STORE,1,&msg[0],APP_TASK_AGUNMAINCTRL_PRIO);  //发送参数保存消息
				char *text = "参数保存成功！";
				Screen_ShowMessage(text,DGUS_SETSENPA_PROADR);
			}
			else
			{
				char *text = "分组模块之和必须等于模块总数";
				Screen_ShowMessage(text,DGUS_SETSENPA_PROADR);
			}			
	  }	
		Delay10Ms(100);
		Screen_ShowMessage(" ",DGUS_SETSENPA_PROADR);
		
		return TRUE;
}


/*读参数按钮操作*/
uint8 Page_ParaRead1()
{
		uint8 cnt = 0;
		uint16 tmp16 = 0;
		uint16 data[64] = {0};
		uint8 buf[100] = {0};

	  PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	 
	  tmp16 = devparaPtr->overdcvolt;   		//直流过压值
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
		tmp16 = devparaPtr->overdccurr;   		//直流过流值
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
		tmp16 = devparaPtr->overacvolt;   		//交流过压值
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
		tmp16 = devparaPtr->underacvolt;   		//交流欠压值
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
		tmp16 = devparaPtr->gunmaxtemper;   	//充电枪过温值
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
    tmp16 = devparaPtr->isoresist;      	//绝缘电阻告警值
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;	
	 
		tmp16 = Hmi_PackWriteReg(buf,(uint8*)&data,DGUS_PARA_START_ADR,CMD_WR_VAR,2*cnt);
		Screen_UartWrite(buf,tmp16);	 
	
		Pagez_NetParaShow(); //网络参数显示
	
		char *text = "参数读取成功！";
		Screen_ShowMessage(text,DGUS_SETPA_PROADR);
		Delay10Ms(100);
		Screen_ShowMessage(" ",DGUS_SETPA_PROADR);
	
		return 0;
}

/*读参数按钮操作*/

uint8 Page_ParaRead2()
{
	 uint8 cnt = 0;
	 uint16 tmp16 = 0;
	 uint16 data[68] = {0};
   uint8 buf[100] = {0};
	 
	  PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	 
	  tmp16 = devparaPtr->Sysparalarm.bits.BMSpower ;   //BMS电源
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
	  tmp16 = devparaPtr->Sysparalarm.bits.curfewalarm;   //门禁告警
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;

	  tmp16	= devparaPtr->Sysparalarm.bits.curfewsignal;       //门禁信号
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;	
		
	  tmp16 = devparaPtr->Sysparalarm.bits.opencharge;       //开门停充
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
		tmp16 = devparaPtr->Sysparalarm.bits.insulalarm;   //绝缘告警
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
		tmp16 = devparaPtr->Sysparalarm.bits.prevraysignl;   //防雷信号
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;	

		tmp16 = devparaPtr->Sysparalarm.bits.fanalarm;   //风机告警
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;		
		
		tmp16 = devparaPtr->Sysparalarm.bits.eleclock;   //电子锁检测
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
		tmp16 = devparaPtr->Sysparalarm.bits.eleclocktype;   //电子锁类型
		Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
		devparaPtr->Sysparalarm.bits.eleclockback = devparaPtr->elockallow;
		tmp16 = devparaPtr->Sysparalarm.bits.eleclockback;   //电子锁反馈
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
	
		tmp16 = devparaPtr->Sysparalarm.bits.eleunlock;   //电子锁解锁
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
	
		tmp16 = devparaPtr->Sysparalarm.bits.batteryalarm;   //电池反接告警
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;	
		
		tmp16 = Hmi_PackWriteReg(buf,(uint8*)&data,DGUS_SENRA_START_ADR1,CMD_WR_VAR,2*cnt);
		Screen_UartWrite(buf,tmp16);	
		
		cnt = 0;
		tmp16 = devparaPtr->Sysparalarm.bits.systemtype;   //系统类型
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;			
		
		tmp16 = devparaPtr->Sysparalarm.bits.chargemode;   //充电模式
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;		

		tmp16 = devparaPtr->modnum;   					//模块总数
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
		tmp16 = devparaPtr->grpmodnum[0];   		//一组模块数量
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;

		tmp16 = devparaPtr->grpmodnum[1];   		//二组模块数量
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
		tmp16 = devparaPtr->Sysparalarm.bits.powersplitt ;   //功率分配
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;	
	 
	  tmp16 = devparaPtr->Fanstartemper ;        //风机启转温度
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;	
	 
	  tmp16 = devparaPtr->Fanstoptemper ;        //风机停止温度
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
		tmp16 = devparaPtr->Fanturnstemper ;        //风机转满温度
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
		tmp16 = devparaPtr->maxcurr ;        //最大输出电流
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
	  tmp16 = devparaPtr->maxvolt ;        //最大输出电压
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
	 
	  tmp16 = devparaPtr->singmodcurr ;        //单模块额定电流
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
	  tmp16 = devparaPtr->elocktm ;           //电子锁超时
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;

	  tmp16 = devparaPtr->bhmtm;           //BHM超时
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;

	  tmp16 = devparaPtr->xftm ;           //泄放超时
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;

    tmp16 = devparaPtr->brmtm;           //BRM超时
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;	 
		
    tmp16 = devparaPtr->bcptm;           //BCP超时
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;

    tmp16 = devparaPtr->brotm;           //BRO超时
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
    tmp16 = devparaPtr->bcltm;           //BCL超时
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;	

    tmp16 = devparaPtr->bcstm;           //BCS超时
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;

    tmp16 = devparaPtr->bsttm;           //BST超时
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;

    tmp16 = devparaPtr->bsdtm;           //BSD超时
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;	
		
		tmp16 = Hmi_PackWriteReg(buf,(uint8*)&data,DGUS_SENRA_START_ADR1+12,CMD_WR_VAR,2*cnt);
		Screen_UartWrite(buf,tmp16);	
		
	  char *text = "参数读取成功！";
	  Screen_ShowMessage(text,DGUS_SETSENPA_PROADR);		
	  Delay10Ms(100);
	  Screen_ShowMessage(" ",DGUS_SETSENPA_PROADR);
		
    return 0;		
 }

/*高级参数按钮*/
void Page_KeySeniorSystem()
{
   Hmi_ChangePicture(E_PICTURE33);	
	 Screen_StartDownCounter(E_PICTURE33,DEFAULT_COUNTDOWN_VALUE);
	 gSetPage = E_PICTURE25;
}

/*第一页参数读取按钮*/
void Page_KeySystem1Read()
{  
		Page_ParaRead1();
}

/*第一页参数读设置按钮*/
void Page_KeySystem1Set()
{
		Page_ParaWrite1(NULL);
}	

/*第一页参数返回按钮*/
void Page_KeySystem1Return()
{
	  uint8 tmppage;
	  tmppage = Page_GetSystemPageNo(E_PICTURE24);
	  Hmi_ChangePicture(tmppage);
	
}	

/*系统参数 时间校准按钮*/
extern uint8 Screen_SetTimeToBoardOnKey(void);
void Page_KeySetTime()
{
	  Screen_SetTimeToBoardOnKey();
	  char *str = "日期时间校验成功！";
	  Screen_ShowMessage(str,DGUS_SETPA_PROADR);
	  Delay10Ms(200);
	  Screen_ShowMessage(" ",DGUS_SETPA_PROADR);
}

/*系统重启*/
void Page_KeySyRestart()
{
   SoftReset();		
}


/*第二页参数读取按钮*/
void Page_KeySystem2Read()
{
   Page_ParaRead2();
}

/*第二页参数设置按钮*/
void Page_KeySystem2Set()
{
   Page_ParaWrite2(NULL);
}

/*第二页参数返回按钮*/
void Page_KeySystem2Return()
{ 
	Page_ParaRead1();
  Hmi_ChangePicture(E_PICTURE25);	
	memset(SC_VAR.PinCode,0,sizeof(SC_VAR.PinCode));
	Screen_ShowMessage((char *)" ",DGUA_PWSH_ADR);
}	

void Page_KeySystemNext()
{	 
   Hmi_ChangePicture(E_PICTURE27);
	 Page_ParaRead3();	
}

void Page_KeyClear()
{
	SendMsgWithNByte(MSG_RECORD_CLEAR,0,NULL,APP_TASK_AGUNMAINCTRL_PRIO);  //擦除记录消息
	Page_ClearRecordPage();
	Delay10Ms(200);				
	char *text = "记录擦除成功！";
	Screen_ShowMessage(text,DGUS_SETSENPA_PROADR);
	Delay10Ms(100);
	Screen_ShowMessage(" ",DGUS_SETSENPA_PROADR);

}
//	uint8 startH;
//	uint8 startM;
//	uint8 stopH;
//	uint8 stopM;
//	
//	uint8   serivetype;    /*服务费方式 0：无 1：按电量 2：按次*/
//	uint8   bespktype;     /*预约费方式 0：无 1：按时间*/
//	uint8   parktype;      /*停车费方式 0：无 1：按时间*/
//	uint8   sectNo[48];
//	
//	uint32 price[10];
//  uint32 srvrate[10];
//	uint32 bespkrate[10];
//	uint32 parkrate[10];

/*费率设置*/	 

uint8 Page_ParaRateSet(uint8 gunNo)
{
	 uint8 rtn = 0;
	 uint16 i;
	 uint16 tmp16 = 0;
   uint8 buf[200] = {0};
   uint16 *ptr ;	
   uint8 msg[2];
   uint16 TempAddr1 = 0x00,TempAddr2 = 0x00;
	 uint32 *ptr32;
	 uint32 tmp32 = 0;
	 
	 if(AGUN_NO == gunNo)
	 {
		 TempAddr1 = DGUS_RATE_START_ADR_A1;
		 TempAddr2 = DGUS_RATE_START_ADR_A2; 	 
	 
	 }else if(BGUN_NO == gunNo)
	 {
	   TempAddr1 = DGUS_RATE_START_ADR_B1;
		 TempAddr2 = DGUS_RATE_START_ADR_B2;
	 }	 
	 
   PARAM_FEE_TYPE *feePtr = ChgData_GetFeeRate(gunNo);
	 PARAM_FEE_TYPE *OtherfeePtr = ChgData_GetFeeRate((gunNo+1)%DEF_MAX_GUN_NO);
	 
	 rtn = FALSE;
	 for( i = 0 ; i < 3 ; i++ ) {
		 if( TRUE == Page_GetSameValData(buf,TempAddr1,14)) {
			 rtn = TRUE;
			 break;
			}
		  //Delay10Ms(1);
   }
	 
	 if( FALSE == rtn ) {
		  return FALSE;
	 }
		
	  ptr32 = (uint32 *)&buf[7];
	  for(i = 0; i < 4; i++)
	  {
		   tmp32 = Common_Change4Byte(ptr32++);
			 tmp32 = tmp32 > 1000000 ? 1000000 : tmp32 ;
			 feePtr->SectTimeFee.price[i] = tmp32;   
	  }
		
	 	tmp32 = Common_Change4Byte(ptr32++);
		tmp32 = tmp32 > 1000000 ? 1000000 : tmp32 ;
		for(i = 0; i < 10; i++)
		{
		   feePtr->SectTimeFee.srvrate[i] = tmp32;   //服务费
		}
		
	 	tmp32 = Common_Change4Byte(ptr32++);
	  tmp32 = tmp32 > 1000000 ? 1000000 : tmp32 ;
		for(i = 0; i < 10; i++)
		{
		  feePtr->SectTimeFee.parkrate[i] = tmp32;   //停车费
    }
		
	 	tmp32 = Common_Change4Byte(ptr32++);
	  tmp32 = tmp32 > 10000 ? 10000 : tmp32 ;
		for(i = 0; i < 10; i++)
		{
		   feePtr->SectTimeFee.bespkrate[i] = tmp32;   //预约费
		}
   
	 rtn = FALSE;
	 for( i = 0 ; i < 3 ; i++ ) {
	    if( TRUE == Page_GetSameValData(buf,TempAddr2,24)) {
           rtn = TRUE;
			     break;
	    }
			//Delay10Ms(1);
		}
	 
	  if( FALSE == rtn ) {
		  return FALSE;
	  }
    ptr = (uint16 *)&buf[7];
    for( i = 0 ; i < 24 ; i++ )
	  {
		  tmp16 = Common_Change2Byte(ptr++);
		  if((tmp16 > 0)&& ( tmp16 < 5))
		  {
			  feePtr->SectTimeFee.sectNo[i] = tmp16 - 1;
      }	 
	   }
		
	 rtn = FALSE;
	 for( i = 0 ; i < 3 ; i++ ) {
		 if( TRUE == Page_GetSameValData(buf,TempAddr2+24,24)) {
			   rtn = TRUE;
			   break;
			}
		  //Delay10Ms(1);
	 }
	 if( FALSE == rtn ) {
		  return FALSE;
	  }
	 
    ptr = (uint16 *)&buf[7];
    for( i = 24 ; i < 48 ; i++ )
	  {
		  tmp16 = Common_Change2Byte(ptr++);
		  if((tmp16 > 0)&& ( tmp16 < 5))
		  {
			  feePtr->SectTimeFee.sectNo[i] = tmp16 - 1;
      }	 
	   }
		
	 
	 
	 
//	 		SAVE_ALL_TYPE
//			SAVE_OPER_TYPE
//		  SAVE_DEV_TYPE
//		  SAVE_FEE_TYPE
	memcpy(OtherfeePtr,feePtr,sizeof(PARAM_FEE_TYPE)); 
	msg[0] = SAVE_FEE_TYPE;
	SendMsgWithNByte(MSG_PARAM_STORE,1,&msg[0],APP_TASK_AGUNMAINCTRL_PRIO);  //发送参数保存消息	 
	 
	char *text = "费率保存成功！";
	Screen_ShowMessage(text,DGUS_SETMONEY_PROADR);
	Delay10Ms(200);
	Screen_ShowMessage(" ",DGUS_SETMONEY_PROADR);
	return TRUE;
}

//	uint8   serivetype;    /*服务费方式 0：无 1：按电量 2：按次*/
//	uint8   bespktype;     /*预约费方式 0：无 1：按时间*/
//	uint8   parktype;      /*停车费方式 0：无 1：按时间*/
//	
//	uint32 price;          /*电费*/
//  uint32 srvrate;        /*服务费*/
//	uint32 bespkrate;      /*预约务费*/
//	uint32 parkrate;       /*停车费*/

/*费率读取*/
uint8 Page_ParaRateRead(uint8 gunNo)
{
	
	 uint8 i,cnt,cnt2;
	 uint16 tmp16 = 0;
	 uint16 data[128] = {0};
   uint8 buf[200] = {0};
	 uint16 TempAddr1 = 0x00,TempAddr2 = 0x00;
	 uint32 tmp32 = 0;
	 cnt = 0;	
   cnt2 = 0;
	 	     
	 if(AGUN_NO == gunNo)
	 {
		 TempAddr1 = DGUS_RATE_START_ADR_A1;
		 TempAddr2 = DGUS_RATE_START_ADR_A2; 	 
	 
	 }else if(BGUN_NO == gunNo)
	 {
	   TempAddr1 = DGUS_RATE_START_ADR_B1;
		 TempAddr2 = DGUS_RATE_START_ADR_B2;
	 }
	 
    PARAM_FEE_TYPE *feePtr = ChgData_GetFeeRate(gunNo);
	  for(i = 0; i < 4; i++)
	  {	 
			 tmp32 = feePtr->SectTimeFee.price[i] ;
			 Common_Change4Byte(&tmp32);
			 data[cnt++] = tmp32 & 0xffff;  
	     data[cnt++] = tmp32 >> 16;
	  }
			 	
		tmp32 = feePtr->SectTimeFee.srvrate[0];
		Common_Change4Byte(&tmp32);
		data[cnt++] = tmp32 & 0xffff;  
	  data[cnt++] = tmp32 >> 16;   //服务费

		tmp32 = feePtr->SectTimeFee.parkrate[0];
		Common_Change4Byte(&tmp32);
		data[cnt++] = tmp32 & 0xffff;  
	  data[cnt++] = tmp32 >> 16;	   //停车费

		tmp32 = feePtr->SectTimeFee.bespkrate[0];
		Common_Change4Byte(&tmp32);
		data[cnt++] = tmp32 & 0xffff;  
	  data[cnt++] = tmp32 >> 16;   //预约费
		
		tmp16 = Hmi_PackWriteReg(buf,(uint8*)&data,TempAddr1,CMD_WR_VAR,2*cnt);
		Screen_UartWrite(buf,tmp16);	

		for( i = 0 ; i < 48; i++ )
		{
			data[cnt2++] = feePtr->SectTimeFee.sectNo[i] + 1;		 	 
		}	 
		Hmi_WriteMulitVar(TempAddr2,&data[0],cnt2/2);
		Hmi_WriteMulitVar(TempAddr2+24,&data[24],cnt2/2);

		char *text1 = "费率读取成功！";
		Screen_ShowMessage(text1,DGUS_SETMONEY_PROADR);
		Delay10Ms(200);
		Screen_ShowMessage(" ",DGUS_SETMONEY_PROADR);

	  return 0;	 
}


/*A枪费率读取按钮*/
void Page_KeyRatesRead_A()
{	
  Page_ParaRateRead(AGUN_NO);  
}

/*A枪费率设置按钮*/
void Page_KeyRatesSet_A()
{
 if (	FALSE == Page_ParaRateSet(AGUN_NO))
 {
 	  char *text = "A枪费率保存失败";
	  Screen_ShowMessage(text,DGUS_SETMONEY_PROADR);
	  Delay10Ms(200);
	  Screen_ShowMessage(" ",DGUS_SETMONEY_PROADR);
 }
}



/*B枪费率读取按钮*/
void Page_KeyRatesRead_B()
{	
  Page_ParaRateRead(BGUN_NO)	;	
}

/*B枪费率设置按钮*/
void Page_KeyRatesSet_B()
{
	if(FALSE == Page_ParaRateSet(BGUN_NO))
	{
	 	 char *text = "B枪费率保存失败";
	   Screen_ShowMessage(text,DGUS_SETMONEY_PROADR);
	   Delay10Ms(200);
	   Screen_ShowMessage(" ",DGUS_SETMONEY_PROADR);
	
	}
}


/*费率返回按钮*/
void Page_KeyRatesReturn()
{

	Hmi_ChangePicture(E_PICTURE39);
  memset(SC_VAR.PinCode,0,sizeof(SC_VAR.PinCode));  /*清除密码信息*/
	Screen_ShowMessage((char *)" ",DGUA_PWSH_ADR);
}

/*密码输入确认按键响应函数*/

void Page_KeyPassWdEnter()
{  
	  char password[8];
	  struct tm Systime = Time_GetSystemCalendarTime();
			
	  sprintf((char*)password, "%4d%d", Systime.tm_year,Systime.tm_mon+Systime.tm_mday);	
    PARAM_DEV_TYPE *param = ChgData_GetDevParamPtr(); 
	  #if (PRODUCTS_LEVEL ==  DEBUG_VERSION)
	   strcpy(password,"201809");
    #endif
		if(((strlen(password)==strlen((char *)SC_VAR.PinCode))&&(strcmp(password,(char *)SC_VAR.PinCode)==0))\
			||((strlen((char *)param->musrpasswd)==strlen((char *)SC_VAR.PinCode))&&(strcmp((char *)param->musrpasswd,(char *)SC_VAR.PinCode)==0))\
			)
		 {		 
			 Screen_DownCounterStop();
			 switch(gSetPage)
			 {
					case E_PICTURE22:
						if(strcmp(password,(char *)SC_VAR.PinCode)==0)
						{
               break;
            }
						Page_ParaRead1();
						Hmi_ChangePicture(E_PICTURE24);
						gSetPage = 0;
						break;
					case E_PICTURE23:					      
						Hmi_ChangePicture(E_PICTURE39);
					  gSetPage = 0;
						break;	
					case E_PICTURE25:							
						 Page_ParaRead2();
						 Hmi_ChangePicture(E_PICTURE26);
					   gSetPage = 0;
						break;					
					default:
						break;
			 }	
       Screen_ShowMessage((char *)" ",DGUA_PWSH_ADR);			 
		   memset(SC_VAR.PinCode,0,sizeof(SC_VAR.PinCode));
		 }
		 else
		 {
				Screen_ShowMessage((char *)"密码输入错误",DGUS_PASSWD_PROADR);
				Delay10Ms(200);
				Screen_ShowMessage((char *)"      ",DGUS_PASSWD_PROADR);
			  Screen_ShowMessage((char *)" ",DGUS_STARTMODE_CHARGAPSWSHOW);
		 }
   		 
}

/*密码页面返回按钮*/
void Page_KeyPwReturn()
{
    Hmi_ChangePicture(gSetPage);  	
}	


/*密码输入后，界面显示****** */
void Page_UploadPassWd()
{
	 Screen_ShowMessage((char *)"  ******",DGUA_PWSH_ADR);

}



/*获取是否有触摸*/
void Page_RegIStouch(void)
{
  static uint8 count = 0;
	
	if( gUploadBuf[0] == 0x03 )
	{
		 count++;
	}else {
		if( count > 2 ) {
		  count = 0;
	 }
	}
}

void Page_KeyPwparam()
{
	
  Screen_ShowMessage((char *)"  ******",DGUS_PW_CARDSH_ADR);
	
}


/*调试界面参数显示*/
void Page_DebugPage()
{
	 u16tobit_u  tmpbit[3];
	 uint16 tmp = 0;
	 uint16 tmp16 = 0;
   uint8 buf[10] = {0};

	  PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	  CHARGE_TYPE  *RunDataPrt_A = ChgData_GetRunDataPtr(AGUN_NO);
	  CHARGE_TYPE  *RunDataPrt_B = ChgData_GetRunDataPtr(BGUN_NO);
	  DEV_ACMETER_TYPE *AcmeterPtr = TskAc_GetMeterDataPtr(AGUN_NO);

    tmpbit[0].word = 0;
    tmpbit[1].word = 0;
    tmpbit[2].word = 0;

	  tmpbit[0].bits.bit0 = AcmeterPtr->statu.bits.commerr;    //AC表通信故障

	  tmpbit[0].bits.bit1 = RunDataPrt_A->iso->statu.bits.commerr;    //绝缘1通信故障

	  tmpbit[0].bits.bit2 = RunDataPrt_B->iso->statu.bits.commerr;     //绝缘2通信故障
 
    tmpbit[0].bits.bit3 = devparaPtr->Sysparalarm.bits.devicealarm;  //刷卡机通信故障

	  tmpbit[0].bits.bit4 = RunDataPrt_A->dlmod->commErr;             //模块总通信故障
	
	 	tmpbit[0].bits.bit5 = RunDataPrt_A->meter->statu.bits.commerr;      //DC电表1通信故障
		
	 	tmpbit[0].bits.bit6 = RunDataPrt_B->meter->statu.bits.commerr;      //DC电表2通信中断	
		
	 	tmpbit[0].bits.bit7 = RunDataPrt_A->relay->statu.bits.km1;         //DC1正KM状态
  		
	 	tmpbit[0].bits.bit8 = RunDataPrt_A->relay->statu.bits.km2;         //DC1负KM状态

    tmpbit[0].bits.bit9 = RunDataPrt_B->relay->statu.bits.km1;         //DC2正KM状态

    tmpbit[0].bits.bit10 = RunDataPrt_B->relay->statu.bits.km1;        //DC2负KM状态
		
		tmpbit[0].bits.bit11 = RunDataPrt_A->input->statu.bits.ackm;         //交流KM状态
		
		tmpbit[1].bits.bit0 = RunDataPrt_A->relay->statu.bits.km3;          //泄放1路KM状态
		
		tmpbit[1].bits.bit1 = RunDataPrt_A->relay->statu.bits.km4;          //泄放2路KM状态		
		
		tmpbit[1].bits.bit2 = 0;      //熔丝1路正状态
		
		tmpbit[1].bits.bit3 = 0;      //熔丝1路负状态		
		
		tmpbit[1].bits.bit4 = 0;      //熔丝2路正状态
		
		tmpbit[1].bits.bit5 = 0;      //熔丝2路负状态				
		
	  tmp = RunDataPrt_A->gun->statu.bits.cc1stu ;     // 1路CC1状态		
		if((CC1_NONEV == RunDataPrt_A->gun->statu.bits.cc1stu)||(CC1_12V == RunDataPrt_A->gun->statu.bits.cc1stu))
		{
		  tmp = 3;
		}
		Hmi_WriteOneVar(0x1603,tmp);
		tmp = RunDataPrt_B->gun->statu.bits.cc1stu ;;     //2路CC1状态
		if((CC1_NONEV == RunDataPrt_B->gun->statu.bits.cc1stu)||(CC1_12V == RunDataPrt_B->gun->statu.bits.cc1stu))
		{
		  tmp = 3;
		}
		Hmi_WriteOneVar(0x1604,tmp);	

		tmpbit[1].bits.bit6 = RunDataPrt_A->gun->statu.bits.elockstu;   //电子锁1状态
		
		tmpbit[1].bits.bit7 = RunDataPrt_B->gun->statu.bits.elockstu;  //电子锁2状态
		
		tmpbit[1].bits.bit8 = RunDataPrt_A->iso->statu.bits.R1negErr;
		
		tmpbit[1].bits.bit8 |= RunDataPrt_A->iso->statu.bits.R1posErr;   //绝缘1路故障
		
		tmpbit[1].bits.bit9 = RunDataPrt_A->iso->statu.bits.R2negErr;
		
		tmpbit[1].bits.bit9 |= RunDataPrt_A->iso->statu.bits.R2posErr;		 //绝缘2路故障
		
		tmpbit[2].bits.bit0 = RunDataPrt_A->input->statu.bits.stop;   //急停状态
		
		tmpbit[2].bits.bit1 = RunDataPrt_A->input->statu.bits.fan;  //风机状态
		
		tmpbit[2].bits.bit2= RunDataPrt_A->input->statu.bits.smoke;  //烟雾告警
		
		tmpbit[2].bits.bit3 = RunDataPrt_A->input->statu.bits.water;  //水浸状态
		
		tmpbit[2].bits.bit4 = RunDataPrt_A->input->statu.bits.dooracs;  //门禁1告警
		
		tmpbit[2].bits.bit5 = RunDataPrt_A->input->statu.bits.dooraf;  //门禁2告警
		
		tmpbit[2].bits.bit6 = RunDataPrt_A->logic->runstu.bits.modErr;   //模块总故障
		
		tmpbit[2].bits.bit7 = RunDataPrt_A->input->statu.bits.spd;     //防雷器故障
		
		Common_Change2Byte(&tmpbit[0].word); 
		Common_Change2Byte(&tmpbit[1].word);
		Common_Change2Byte(&tmpbit[2].word);
				
	  tmp16 = Hmi_PackWriteReg(buf,(uint8*)&tmpbit,DGUS_DEBUG_ADR1,CMD_WR_VAR,2*3);
	  Screen_UartWrite(buf,tmp16);
		 
}

/*调试界面2 文本显示*/
void Page_DebugPage2_Text()
{
	char str[18] = {0};
	char *test;

	PARAM_DEV_TYPE *chgparaPtr = ChgData_GetDevParamPtr();

    test = "v1.1";        /*模块软件版本*/
		Screen_ShowMessage(test,DGUS_DEBUGTEXT_ADR);

    test = "v1.1";       /*模块硬件版本*/
		Screen_ShowMessage(test,DGUS_DEBUGTEXT_ADR+DGUS_DEBUGTEXT_OFFSET);

    test = "v1.1";       /*系统硬件版本*/
		Screen_ShowMessage(test,DGUS_DEBUGTEXT_ADR+DGUS_DEBUGTEXT_OFFSET*2);

		/*主控板软件版本*/
	  //sprintf(str,"%s.%s.%s",M_VERSION1,M_VERSION2,M_VERSION3);
#ifdef AUNICE_DEBUG
	  sprintf(str,"%s.%c%d.%.2d",M_VERSION1,VERSION_LETTER,MAIN_VERSION,SUB_VERSION);    //按邓总的规则改的版本号
#else	  
	  sprintf(str,"%s.%s",M_VERSION1,M_VERSION3);    //按邓总的规则改的版本号
#endif	  
		Screen_ShowMessage(str,DGUS_DEBUGTEXT_ADR+DGUS_DEBUGTEXT_OFFSET*3);

    test = "7 SCREEN";      /*触摸屏软件版本*/
		Screen_ShowMessage(test,DGUS_DEBUGTEXT_ADR+DGUS_DEBUGTEXT_OFFSET*4);

    test = "ver1.1";     /*主控板硬件版本*/
		Screen_ShowMessage(test,DGUS_DEBUGTEXT_ADR+DGUS_DEBUGTEXT_OFFSET*5);
	
		/*充电桩ID显示*/
		Hmi_ClearReg(DGUS_ID_ADR,1);
		memcpy(str,chgparaPtr->chargeId,sizeof(chgparaPtr->chargeId));
		str[sizeof(chgparaPtr->chargeId)] = 0; //结束符
		Screen_ShowMessage(str,DGUS_ID_ADR);
	 
	 
		test = "22.651646°E";     /*经度*/
		Screen_ShowMessage(test,DGUS_DEBUGTEXT_ADR+DGUS_DEBUGTEXT_OFFSET*6); 
	 
	  test = "114.098014°N";     /*纬度*/
		Screen_ShowMessage(test,DGUS_DEBUGTEXT_ADR+DGUS_DEBUGTEXT_OFFSET*7);	
}

void Page_DebugPage2()
{
		uint8 cnt = 0;
	  uint16 tmp16 = 0;
	  uint16 data[80] = {0};
    uint8 buf[100] = {0};

	  PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	  CHARGE_TYPE  *RunDataPrt_A = ChgData_GetRunDataPtr(AGUN_NO);
		CHARGE_TYPE  *RunDataPrt_B = ChgData_GetRunDataPtr(BGUN_NO);
	  PARAM_DEV_TYPE *chgparaPtr = ChgData_GetDevParamPtr();
	  DEV_ACMETER_TYPE *AcmeterPtr_A = TskAc_GetMeterDataPtr(AGUN_NO);

	 	tmp16 = RunDataPrt_A->input->temper1;        //充电桩温度
		if(0 == tmp16){tmp16 = 26;}
    Common_Change2Byte(&tmp16); 		
    data[cnt++] = tmp16;

	 	tmp16 = RunDataPrt_A->gun->i32temper[0];     //充电枪温度
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;
		
	 	tmp16 = RunDataPrt_A->meter->volt;         //DC表1电压
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;		

	 	tmp16 = RunDataPrt_B->meter->volt;         
    Common_Change2Byte(&tmp16);                //DC表2电压 
    data[cnt++] = tmp16;		
		
	 	tmp16 = RunDataPrt_A->meter->current;       //DC表1电流
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;		
		
	 	tmp16 = RunDataPrt_B->meter->current;       
    Common_Change2Byte(&tmp16);                 //DC表2电流 
    data[cnt++] = tmp16;

	 	tmp16 = RunDataPrt_A->iso->vdc3;           //绝缘单元1路电压  3路外侧电压
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;
		
	 	tmp16 = RunDataPrt_B->iso->vdc3;           //绝缘单元2路电压
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;		
		
	 	tmp16 = RunDataPrt_A->iso->res1pos;           //绝缘1路正对地电阻
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;		
		
	 	tmp16 = RunDataPrt_B->iso->res1pos;           //绝缘2路正对地电阻
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;		
		
	 	tmp16 = RunDataPrt_A->iso->res1neg;           //绝缘1路负对地电阻
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;		
		
	 	tmp16 = RunDataPrt_B->iso->res1neg;           //绝缘2路负对地电阻
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;			
		
	 	tmp16 = RunDataPrt_A->dlmod[1].outVolte;     //输出电压
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;			

	 	tmp16 = RunDataPrt_A->dlmod[1].outCurr;     //输出电流
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;

	 	tmp16 = AcmeterPtr_A->Uab;          //Vab电压
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;

	 	tmp16 = AcmeterPtr_A->Ubc;          //Vbc电压
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;

	 	tmp16 = AcmeterPtr_A->Uca;          //Vca电压
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;
		
	 	tmp16 = devparaPtr->modnum;          //系统模块数量
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;		
		
		tmp16 = devparaPtr->maxvolt;          //最大输出电压
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;

		tmp16 = devparaPtr->mincurr;          //最低输出电流
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;

		tmp16 = devparaPtr->maxcurr;          //最大输出电流
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;

		tmp16 = M_VERSION4;                   //子版本号
    Common_Change2Byte(&tmp16); 
    data[cnt++] = tmp16;
	
    data[cnt++] = 0;
    data[cnt++] = 0;
    data[cnt++] = 0;
		data[cnt++] = 0;
    data[cnt++] = 0;

	  tmp16 = Hmi_PackWriteReg(buf,(uint8*)&data,DGUS_DEBUG_ADR2,CMD_WR_VAR,2*cnt);
	  Screen_UartWrite(buf,tmp16);
		
		Page_DebugPage2_Text();

}


/*调试界面 返回*/
void Page_KeyDebugReturn()
{
	  uint8 tmppage;
	  tmppage = Page_GetSystemPageNo(E_PICTURE30);
	  Hmi_ChangePicture(tmppage);
 
}

/*22页 调试界面 下一页*/
void Page_KeyDebugNext()
{
	Page_DebugPage2();
  Hmi_ChangePicture(E_PICTURE31);
}

 /*模块详情 */		
void Page_KeyModeDetail()
{
	sGlocalPara.ModUpDownKey = 0;
  Page_ModeDetail(0);
  Hmi_ChangePicture(E_PICTURE32);
}


void Pagez_NetParaShow()
{

  char str[20] = {0};
  
	 PARAM_DEV_TYPE *chgparaPtr = ChgData_GetDevParamPtr();
	 PARAM_COMM_TYPE *Eth = ChgData_GetCommParaPtr();
	
	 /*充电桩ID显示*/
	 Hmi_ClearReg(DGUS_ID_ADR,1); 
   memcpy(str,chgparaPtr->chargeId,sizeof(chgparaPtr->chargeId));
	 str[sizeof(chgparaPtr->chargeId)] = 0;  //结束符
   Screen_ShowMessage((char *)str,DGUS_ID_ADR);

	
	  /*远程IP显示*/
   Hmi_ClearReg(DGUS_REMOTEIP_ADR,1);
   sprintf(str,"%d.%d.%d.%d",Eth->netpara.RemoteIpAddr[0],Eth->netpara.RemoteIpAddr[1],\
	 Eth->netpara.RemoteIpAddr[2],Eth->netpara.RemoteIpAddr[3]);
	 Screen_ShowMessage(str,DGUS_REMOTEIP_ADR);

		/*远程端口号*/
		Hmi_ClearReg(DGUS_REMOTEPORT_ADR,1);
		Hmi_WriteOneVar(DGUS_REMOTEPORT_ADR,Eth->netpara.RemotePort);

    /*本地ip*/
   Hmi_ClearReg(DGUS_LOCALIP_ADR,1);
   sprintf(str,"%d.%d.%d.%d",Eth->netpara.LocalIpAddr[0],Eth->netpara.LocalIpAddr[1],\
	 Eth->netpara.LocalIpAddr[2],Eth->netpara.LocalIpAddr[3]);
   Screen_ShowMessage(str,DGUS_LOCALIP_ADR);

    /*子网掩码*/
   Hmi_ClearReg(DGUS_NETMASK_ADR,1);
   sprintf(str,"%d.%d.%d.%d",Eth->netpara.NetMask[0],Eth->netpara.NetMask[1],\
	 Eth->netpara.NetMask[2],Eth->netpara.NetMask[3]);
   Screen_ShowMessage(str,DGUS_NETMASK_ADR);
	 
	  /*网关*/
	 Hmi_ClearReg(DGUS_NETGATE_ADR,1);
   sprintf(str,"%d.%d.%d.%d",Eth->netpara.NetGate[0],Eth->netpara.NetGate[1],Eth->netpara.NetGate[2],Eth->netpara.NetGate[3]);
   Screen_ShowMessage(str,DGUS_NETGATE_ADR);
	 
	  /*mac*/
	 Hmi_ClearReg(DGUS_MAC_ADR,1);
   sprintf(str,"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",Eth->netpara.MacAdress[0],Eth->netpara.MacAdress[1],Eth->netpara.MacAdress[2]\
	 ,Eth->netpara.MacAdress[3],Eth->netpara.MacAdress[4],Eth->netpara.MacAdress[5]);
   Screen_ShowMessage(str,DGUS_MAC_ADR);
	 	 	 	 
	 /*DHCP ip*/
   Hmi_ClearReg(DGUS_DHCPEN_ADR,1);
	 Hmi_WriteOneVar(DGUS_DHCPEN_ADR,Eth->netpara.StaPara.DhcpEn);
	 
	 /*网络使能*/
	 Hmi_ClearReg(DGUS_ETHEN_ADR,1);
	 Hmi_WriteOneVar(DGUS_ETHEN_ADR,Eth->netpara.StaPara.SecuEn);
	 
	 /*SSID*/
	 Hmi_ClearReg(DGUS_SSID_ADR,1);
   sprintf(str,"%s",Eth->netpara.StaPara.Ssid);
   Screen_ShowMessage(str,DGUS_SSID_ADR);
	 	 
	 /*PASSWORD*/
	 Hmi_ClearReg(DGUS_PASSWORD_ADR,1);
   sprintf(str,"%s",Eth->netpara.StaPara.Password);
   Screen_ShowMessage(str,DGUS_PASSWORD_ADR);
	 
	 /*联网方式*/
	 Hmi_ClearReg(DGUS_NETWAY_ADR,1);
   Hmi_WriteOneVar(DGUS_NETWAY_ADR,Eth->conntype);
 
}


/* 网络参数设置*/
uint8 Page_NetParaSetToBoard()
{
	  uint8 data[100];
	  uint8 *ptr;
	  uint8 msg[2];  
    uint8 i,fg;
	
	 PARAM_DEV_TYPE *chgparaPtr = ChgData_GetDevParamPtr();
   PARAM_COMM_TYPE *Eth = ChgData_GetCommParaPtr();
	
	    /*一次读48个容易失败，分开读取在组合*/
	  fg = 0;
	  for( i = 0 ; i < 3 ; i++ ) {
			if( TRUE == Page_GetSameValData(data,DGUS_REMOTEIP_ADR,8)) {   /*远程IP*/
				   fg = 1;
					 break;;
			}
	  }
		if( 0 == fg ) {
			return FALSE;
		}
		ptr = &data[7];
		Hmi_RegStrCheck(ptr,32);
		Common_FormatIpAddr((char *)ptr,Eth->netpara.RemoteIpAddr); 
		
		 /*端口号*/
		if( FALSE == Page_GetSameValData(data,DGUS_REMOTEPORT_ADR,1)) {
			 if( FALSE == Page_GetSameValData(data,DGUS_REMOTEPORT_ADR,1) ) {
				  return FALSE;
			 }
		}
		Eth->netpara.RemotePort = ((uint16)data[7] << 8) | data[8];
		
		/*DHCP_EN*/	
		if( FALSE == Page_GetSameValData(data,DGUS_DHCPEN_ADR,1)) {
				 return FALSE;
		}
		Eth->netpara.StaPara.DhcpEn = data[8]&0x01;	
		
		if(0 == Eth->netpara.StaPara.DhcpEn){//DHCP禁用，读取静态IP
	 
			   /*本地IP地址*/
				if( FALSE == Page_GetSameValData(data,DGUS_LOCALIP_ADR,8)) {
						if( FALSE == Page_GetSameValData(data,DGUS_LOCALIP_ADR,8) ) {
							return FALSE;
						}
				}
				ptr = &data[7];
				Hmi_RegStrCheck(ptr,32);
				Common_FormatIpAddr((char *)ptr,Eth->netpara.LocalIpAddr);
				 /*网关地址*/
				if( FALSE == Page_GetSameValData(data,DGUS_NETGATE_ADR,8)) {
						if( FALSE == Page_GetSameValData(data,DGUS_NETGATE_ADR,8) ) {
							return FALSE;
						}
				}
				ptr = &data[7];
				Hmi_RegStrCheck(ptr,32);
				Common_FormatIpAddr((char *)ptr,Eth->netpara.NetGate);
				
				/*mac*/
				if( FALSE == Page_GetSameValData(data,DGUS_MAC_ADR,9)) {
					if( FALSE == Page_GetSameValData(data,DGUS_MAC_ADR,9)) {
						return FALSE;
					}
				}
				ptr = &data[7];
				Common_FormatMacAddr((char *)ptr,Eth->netpara.MacAdress);
				
				/*子网掩码地址*/
				if( FALSE == Page_GetSameValData(data,DGUS_NETMASK_ADR,8)) {
						 if(  FALSE == Page_GetSameValData(data,DGUS_NETMASK_ADR,8)) {
							 return FALSE;
						 }
				}
				ptr = &data[7];
				Common_FormatIpAddr((char *)ptr,Eth->netpara.NetMask);	
    }

		
		
		/*网络使能*/
		if( FALSE == Page_GetSameValData(data,DGUS_ETHEN_ADR,1)) {
			return FALSE;
		}
		Eth->netpara.StaPara.SecuEn = data[8]&0x01;	
		
		/*SSID地址*/
		if( FALSE == Page_GetSameValData(data,DGUS_SSID_ADR,10)) {
			if( FALSE == Page_GetSameValData(data,DGUS_SSID_ADR,10)) {
					return FALSE;
			}
		}
		ptr = &data[7];
		Hmi_RegStrCheck(ptr,32);
		Common_trim((char *)ptr);
		strcpy((char *)(Eth->netpara.StaPara.Ssid),(char *)ptr);
		
		 /*密码地址*/
		if( FALSE == Page_GetSameValData(data,DGUS_PASSWORD_ADR,10)) {
			if( FALSE == Page_GetSameValData(data,DGUS_PASSWORD_ADR,10) ) {
					return FALSE;
			}
		}
		ptr = &data[7];
		Hmi_RegStrCheck(ptr,32);
		Common_trim((char *)ptr);
		strcpy((char *)(Eth->netpara.StaPara.Password),(char *)ptr);	
		
		 /*联网方式*/	
		if( FALSE == Page_GetSameValData(data,DGUS_NETWAY_ADR,1)) {
				return FALSE;
		}
		Eth->conntype = data[8]%0x06;	
		
		/*本桩ID*/	
		if( FALSE == Page_GetSameValData(data,DGUS_ID_ADR,10)) {
			  if( FALSE == Page_GetSameValData(data,DGUS_ID_ADR,10) ) {
					return FALSE;
				}
		}
		ptr = &data[7];
		Hmi_GetRcvString((char *)chgparaPtr->chargeId,ptr,sizeof(chgparaPtr->chargeId));
			
		msg[0] = SAVE_DEV_TYPE;
		SendMsgWithNByte(MSG_PARAM_STORE,1,&msg[0],APP_TASK_AGUNMAINCTRL_PRIO);

	 char *text = "参数设置成功！";
	 Screen_ShowMessage(text,DGUS_SETPA_PROADR);
	 Delay10Ms(200);
	 Screen_ShowMessage(" ",DGUS_SETPA_PROADR);
		
		return TRUE;
}


/*模块详情*/
void Page_ModeDetail(uint8 code)
{
   uint8 cnt = 0;
	 uint8 i = 0;
	 uint16 tmp16 = 0;
	 uint16 data[32] = {0};
   uint8 buf[40] = {0};
   uint32 tmp32;
   u16tobit_u tmpbit;
	 CHARGE_TYPE  *RunDataPtr = ChgData_GetRunDataPtr(AGUN_NO);

     for(i = 0 ; i < 3;i++ )
     {
			  cnt = 0;
				tmp16 = 1 + code*3 + i ;                    //模块号
				Common_Change2Byte(&tmp16);
				data[cnt++] = tmp16;
				
				tmp16 = RunDataPtr->dlmod[code*3+i].inVolteACA ;   //ACA电压
				Common_Change2Byte(&tmp16);
				data[cnt++] = tmp16;
			 
				tmp16 = RunDataPtr->dlmod[code*3+i].inVolteACB ;   //ACB电压
				Common_Change2Byte(&tmp16);
				data[cnt++] = tmp16;	 
			 
				tmp16 = RunDataPtr->dlmod[code*3+i].inVolteACC ;   //ACC电压
				Common_Change2Byte(&tmp16);
				data[cnt++] = tmp16;
				
				tmp16 = RunDataPtr->dlmod[code*3+i].limtPoint ;   //限流点
				Common_Change2Byte(&tmp16);
				data[cnt++] = tmp16;
				
				tmp16 = RunDataPtr->dlmod[code*3+i].outVolte;     //输出电压
				Common_Change2Byte(&tmp16);
				data[cnt++] = tmp16;

				tmp16 = RunDataPtr->dlmod[code*3+i].outCurr ;     //输出电流
				Common_Change2Byte(&tmp16);
				data[cnt++] = tmp16;

				tmp16 = RunDataPtr->dlmod[code*3+i].temper;       //环境温度
				Common_Change2Byte(&tmp16);
				data[cnt++] = tmp16;			
				
				tmp32 = RunDataPtr->dlmod[code*3+i].alarmInfo.u32d;       //告警
				Common_Change4Byte(&tmp32);
		    data[cnt++] = tmp32 & 0xffff;
				data[cnt++] = tmp32 >> 16;	
				
				tmpbit.word = 0;
				tmpbit.bits.bit0 = RunDataPtr->dlmod[code*3+i].alarmInfo.bits.ModStu;
				tmpbit.bits.bit1 = RunDataPtr->dlmod[code*3+i].alarmInfo.bits.ModWork;
				tmpbit.bits.bit2 = RunDataPtr->dlmod[code*3+i].commErr;
				Common_Change2Byte(&tmpbit.word);
				data[cnt++] = tmpbit.word;
				
				tmp16 = Hmi_PackWriteReg(buf,(uint8*)&data,DGUS_MODEDETAIL_ADDR1+i*DGUS_MODEDETAIL_OFFSET,CMD_WR_VAR,2*cnt);
	      Screen_UartWrite(buf,tmp16);			
		  }
}



 /*27页 模块详情  下一页*/
void Page_KeyModeNext()
{
	uint8 num;
	PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	num = 0;
	if((devparaPtr->modnum > 0) && (devparaPtr->modnum <= CD_MAX_NUM))
	{
     num = (devparaPtr->modnum - 1 ) / 3;
  }
	
	if( sGlocalPara.ModUpDownKey < num) /*取实际模块个数*/
	{
		sGlocalPara.ModUpDownKey++;
	}
  Page_ModeDetail(sGlocalPara.ModUpDownKey);
}

/*27页 模块详情  上一页 */
void Page_KeyModePrev()
{
	if( sGlocalPara.ModUpDownKey > 0 ) {
		sGlocalPara.ModUpDownKey--;
	}
  Page_ModeDetail(sGlocalPara.ModUpDownKey);
}


/*27页 模块详情   返回*/
void Page_KeyModeReturn()
{
	Page_DebugPage();
  Hmi_ChangePicture(E_PICTURE30);
}



/*28页 调试界面2 上一页*/
void Page_KeyDebugPrev()
{
	Page_DebugPage();
  Hmi_ChangePicture(E_PICTURE30);
}

 /*29页 BMS详情 返回*/
void Page_KeyBMSReturn()
{
   Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
}

/*高级设置2 读*/
void Page_ParaRead3()
{
	 uint8 cnt = 0;
	 uint16 tmp16 = 0;
	 uint16 data[64] = {0};
   uint8 buf[100] = {0};
   char str[20] = {0};

   PARAM_DEV_TYPE *chgparaPtr = ChgData_GetDevParamPtr();

    tmp16 = chgparaPtr->maxmodnum;   //最大模块数量
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
    tmp16 = chgparaPtr->safeclass;   //系统安全级别
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
    tmp16 = chgparaPtr->onlinecard;   //刷卡方式
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;

	  tmp16 = chgparaPtr->bkcomm.agreetype;    //后台方式
	  Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
	 
    tmp16 = Hmi_PackWriteReg(buf,(uint8*)&data,DGUS_SENRA_START_ADR2,CMD_WR_VAR,2*cnt);
	  Screen_UartWrite(buf,tmp16);	 	 

    Hmi_ClearReg(DGUS_PAPA_PASSWORD_ADR,1);
	  sprintf(str,"%s",chgparaPtr->musrpasswd);
    Screen_ShowMessage(str,DGUS_PAPA_PASSWORD_ADR);  /*输入密码*/
		
		Hmi_ClearReg(DGUS_PAPA_PASSWORD_ADR1,1);
	  sprintf(str,"%s",chgparaPtr->musrpasswd);
    Screen_ShowMessage(str,DGUS_PAPA_PASSWORD_ADR1);  /*确认密码*/
		
		Hmi_ClearReg(DGUS_PAPA_PHONE_ADR,1);
	  sprintf(str,"%s",chgparaPtr->telephone);
    Screen_ShowMessage(str,DGUS_PAPA_PHONE_ADR);   /*服务电话*/

  	/*读卡器KEY*/
		cnt = 0;
    encode((uint8 *)data,gCardOperData.para.keyCode,16);
    Common_NormlBcdToAsc(buf,(uint8 *)data,16);
    memcpy(data,buf,32);
    cnt = 16;
    tmp16 = Hmi_PackWriteReg(buf,(uint8*)&data,DGUS_KEYCODE_ADR,CMD_WR_VAR,2*cnt);
		Screen_UartWrite(buf,tmp16);	

		char *text = "参数读取成功！";
		Screen_ShowMessage(text,DGUS_SETSENPA_PROADR);
		Delay10Ms(100);
		Screen_ShowMessage(" ",DGUS_SETSENPA_PROADR);	
}

/*高级设置2 写*/
uint8 Page_ParaWrite3(uint8 *ndata)
{ 
	 uint8 rtn;
	 uint16 tmp16 = 0;
   uint8 data[32];
   uint8 buf[164] = {0};
   uint16 *ptr ;
	 uint8 msg[2];
	 uint8 *ptr8;
	 rtn = 0;
	 uint8 password1[8],password2[8];
	 char *text;

	 PARAM_DEV_TYPE *chgparaPtr = ChgData_GetDevParamPtr();

		if( FALSE == Page_GetSameValData(buf,DGUS_SENRA_START_ADR2,4)) {
			if( FALSE == Page_GetSameValData(buf,DGUS_SENRA_START_ADR2,4)) {
			   return FALSE;
			}
		}
		ptr = (uint16 *)&buf[7];
		tmp16 = Common_Change2Byte(ptr++);
		tmp16 = tmp16 > CD_MAX_NUM ? CD_MAX_NUM : tmp16;
		chgparaPtr->maxmodnum = tmp16 ;   //最大模块数
		tmp16 = Common_Change2Byte(ptr++);
		tmp16 = tmp16 > 10 ? 10 : tmp16;
		chgparaPtr->safeclass = tmp16 ;   //系统安全级别
		tmp16 = Common_Change2Byte(ptr++);
		tmp16 = tmp16 > 3 ? 0 : tmp16;
		chgparaPtr->onlinecard = tmp16 ;       //刷卡方式  0 E_OFFLINE_CARD  1 E_ONLINE_CARD
		tmp16 = Common_Change2Byte(ptr++);
		tmp16 = tmp16 > 10 ? 0 : tmp16;
		chgparaPtr->bkcomm.agreetype = tmp16;   //后台方式 0 亿电 1 奥耐 2充之鸟


		if( FALSE == Page_GetSameValData(buf,DGUS_PAPA_PASSWORD_ADR,4)) {
			if( FALSE == Page_GetSameValData(buf,DGUS_PAPA_PASSWORD_ADR,4)) {
			   return FALSE;
			} 
		}
		ptr8 = (uint8 *)&buf[7];			
		Hmi_GetRcvString((char *)password1,ptr8,8);
		password1[7] = 0;   //管理员密码


		if( FALSE == Page_GetSameValData(buf,DGUS_PAPA_PASSWORD_ADR1,4)) {
			if( FALSE == Page_GetSameValData(buf,DGUS_PAPA_PASSWORD_ADR1,4)) {
			   return FALSE;
			}	 
		}
		ptr8 = (uint8 *)&buf[7];
		Hmi_GetRcvString((char *)password2,ptr8,8);
		password2[7] = 0;  //确认管理员密码   


		if( FALSE == Page_GetSameValData(buf,DGUS_PAPA_PHONE_ADR,8)) {
			if( FALSE == Page_GetSameValData(buf,DGUS_PAPA_PHONE_ADR,8)) {
			   return FALSE;
			}		
		}
		ptr8 = (uint8 *)&buf[7];
		Hmi_GetRcvString((char *)chgparaPtr->telephone,ptr8,16);
		chgparaPtr->telephone[strlen((char *)ptr8)%15] = 0;		        //服务电话

		if( FALSE == Page_GetSameValData(buf,DGUS_KEYCODE_ADR,16)||(buf[6] != 16)) {
			if( FALSE == Page_GetSameValData(buf,DGUS_KEYCODE_ADR,16)||(buf[6] != 16)) {
			   return FALSE;
			}	  
		}
		buf[7+32] = 0; /*字符串末尾置0*/
		Common_AscToBcd(data,&buf[7]);
		decode(buf,data,16);
		memcpy(chgparaPtr->card.safe.keyCode,buf,16);
		memcpy(gCardOperData.para.keyCode,buf,16);	  //读卡器KEY

		if((strcmp((char *)password1,(char *)password2)==0)&&(strlen((char *)password1)==strlen((char *)password2)))
		{
			memcpy(chgparaPtr->musrpasswd,password1,7);
			chgparaPtr->musrpasswd[7] = 0; 	 
			msg[0] = SAVE_ALL_TYPE;
			SendMsgWithNByte(MSG_PARAM_STORE,1,&msg[0],APP_TASK_AGUNMAINCTRL_PRIO);  //发送参数保存消息
			text = "参数保存成功！";				
		}
		else
		{
		  text = "请确认密码前后是否一致";
		}


		Screen_ShowMessage(text,DGUS_SETSENPA_PROADR);
		Delay10Ms(200);
		Screen_ShowMessage(" ",DGUS_SETSENPA_PROADR);

		return TRUE;
	
}

void Page_KeySystem3Read()
{
   Page_ParaRead3();	 
}

void Page_KeySystem3Set()
{

  Page_ParaWrite3(NULL);
}
	
/*高级设置2 返回*/
void Page_KeySystem3Return()
{
   Hmi_ChangePicture(E_PICTURE26);
	 Page_ParaRead2();

}


/*输入卡密码确认 按键*/
void Page_KeyPwCardEnter()
{
   SendMsgWithNByte(MSG_GET_PASSWD,sizeof(SC_VAR.CardPinCode),&SC_VAR.CardPinCode,APP_TASK_AGUNMAINCTRL_PRIO);
}
	
/*输入卡密码 返回*/
void Page_KeyPwCardReturn()
{
   Hmi_ChangePicture(gScrenCtrlPara.u8keepPic); 
	 Screen_ShowMessage((char *)" ",DGUS_PASSWD_PROADR);
	 memset(SC_VAR.PinCode,0,sizeof(SC_VAR.CardPinCode));
	 Screen_ShowMessage((char *)" ",DGUS_PW_CARDSH_ADR);
}

extern DEV_RELAY_TYPE * Relay_GetRelayDataPtr(uint8 no);
/*辅助电源切换按键*/
void Page_KeyAuciliaryPow_A()
{
	 char *text; 
	 sGlocalPara.powerkey = AGUN_NO;
	
	 Screen_StartDownCounter(E_PICTURE45,DEFAULT_COUNTDOWN_VALUE);
	 Screen_SetChgFlag(1);
	 Hmi_ChangePicture(E_PICTURE45);
	
	 DEV_RELAY_TYPE *pRelayDataPtr = Relay_GetRelayDataPtr(sGlocalPara.powerkey);
	 if(BMS_12V == pRelayDataPtr->statu.bits.apow)
	 {
	    text = "当前A枪辅源为12V";
		 
	 }
	 else if(BMS_24V == pRelayDataPtr->statu.bits.apow)
	 {
	    text = "当前A枪辅源为24V";
	 }
   Screen_ShowMessage(text,DGUS_AUPOWCHOOSE_ADR);
	 
}

/*辅助电源切换按键*/
void Page_KeyAuciliaryPow_B()
{
   char *text; 
	 sGlocalPara.powerkey = BGUN_NO;
	
   Screen_StartDownCounter(E_PICTURE45,DEFAULT_COUNTDOWN_VALUE);
	 Screen_SetChgFlag(1);
	 Hmi_ChangePicture(E_PICTURE45);
	
	 DEV_RELAY_TYPE *pRelayDataPtr = Relay_GetRelayDataPtr(sGlocalPara.powerkey);
	 if(BMS_12V == pRelayDataPtr->statu.bits.bpow)
	 {
	    text = "当前B枪辅源为12V";
		 sGlocalPara.bms24vtm[AGUN_NO] = GetSystemTick();
	 }
	 else if(BMS_24V == pRelayDataPtr->statu.bits.bpow)
	 {
	    text = "当前B枪辅源为24V";
	 }
   Screen_ShowMessage(text,DGUS_AUPOWCHOOSE_ADR); 
}

extern void IDEL_ShowMessage(uint8 gunNo);
/*辅助电源选择12V按键*/
void Page_KeyAuPowChoose12()
{
	DEV_RELAY_TYPE *pRelayDataPtr = Relay_GetRelayDataPtr(sGlocalPara.powerkey);
  RelayOut_Power12_24V(sGlocalPara.powerkey%DEF_MAX_GUN_NO,BMS_12V); 
	IDEL_ShowMessage(sGlocalPara.powerkey%DEF_MAX_GUN_NO);
  Screen_DownCounterStop();  //停止倒计时
	Screen_SetChgFlag(0);
	Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
}

/*辅助电源选择24V按键*/
void Page_KeyAuPowChoose24()
{

  Hmi_ChangePicture(E_PICTURE46);

	DEV_RELAY_TYPE *pRelayDataPtr = Relay_GetRelayDataPtr(sGlocalPara.powerkey);
	
  RelayOut_Power12_24V(sGlocalPara.powerkey%DEF_MAX_GUN_NO,BMS_24V);  
  IDEL_ShowMessage(sGlocalPara.powerkey%DEF_MAX_GUN_NO);	
	Screen_DownCounterStop();  //停止倒计时
	Screen_SetChgFlag(0);
	Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
	sGlocalPara.bms24vtm[sGlocalPara.powerkey%DEF_MAX_GUN_NO] = GetSystemTick();
}



void Page_KeyAuPowReturn()
{	
	Screen_SetChgFlag(0);
  Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
}



void Page_KeyAuPowConfirm()
{
	DEV_RELAY_TYPE *pRelayDataPtr = Relay_GetRelayDataPtr(sGlocalPara.powerkey);
  RelayOut_Power12_24V(sGlocalPara.powerkey,1);  
	Screen_DownCounterStop();  //停止倒计时
	Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);

}

void Page_KeyAuPowCancel()
{
  Hmi_ChangePicture(E_PICTURE45);
}




void Page_KeyStartModeReturn()
{
	Screen_DownCounterStop();  //停止倒计时
	Screen_SetChgFlag(0);
  Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
}

uint8 ChangeReadVarForm(uint8 *des,stoc_u *source,uint8 len)
{
	uint8 i;
	for(i = 0; i < len/2; i++)
	{
		des[2*i] = source[i].c[1];
		des[2*i+1] = source[i].c[0];
	}
	
	for(i = 0; i < len; i++)
	{
		if((0xff == des[i]) || (0 == des[i]))
		{
			break;
		}
	}
	return i;
	
}

extern void Get_BcdCardNo(uint8 *CardNo,uint8 *buff);
void Page_KeyPasswdModeEnter()
{
	uint8 data[5] = {0};
	stoc_u tmp[10] = {0};
  uint8 buf[20];
	Screen_DownCounterStop();  //停止倒计时
	data[0] = sGlocalPara.gun_no;
	data[1] = START_BY_PASSWD;
	START_PARAM  *PtrStartPara = ChgData_GetStartParaPtr(data[0]);
	Hmi_ReadMulitVar(DGUS_STARTMODE_CHARGACCOUNT,&tmp[0],8);
	ChangeReadVarForm(&buf[0],&tmp[0],16);
	Get_BcdCardNo(PtrStartPara->account,buf);
	
	Hmi_ReadMulitVar(DGUS_STARTMODE_CHARGAPSW,&tmp[0],8);
	ChangeReadVarForm(&buf[0],&tmp[0],16);
  Get_BcdCardNo(PtrStartPara->password,buf);
	Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSWSHOW,9);
	Hmi_ClearReg(DGUS_STARTMODE_CHARGACCOUNT,9);
	Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSW,9);
	PtrStartPara->startby = START_BY_PASSWD;
  PtrStartPara->chgmod = CHG_AUTO_TYPE;
	SendMsgWithNByte(MSG_ONLINE_CARD,2,&data[0],APP_TASK_BACK_PRIO);
	Screen_SetChgFlag(0);
	sGlocalPara.startkeylasttm[data[0]] = GetSystemTick();
  Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);

}


void Page_KeyCardModeEnter()
{
	Screen_DownCounterStop();  //停止倒计时
	Page_Note_StartChargMode(sGlocalPara.gun_no,START_BY_ONLINECARD);
	Screen_SetChgFlag(0);
  Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);

}


/*充电密码界面返回*/
void Page_KeyChargPwdReturn()
{
	Screen_SetChgFlag(0);
	Screen_DownCounterStop();
	Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
}
#ifdef CHARGE_BIRD_BACK_COMM
extern uint8 ChargeBird_PassWord(uint8 gunNo,uint8 *buf,uint8 *startchargpwd);
#endif
extern void GET_MD5_STARTPASSWD(uint8 gunno,uint8 *buf);
/*充电密码界面确认*/
void Page_KeyChargPwdEnter()
{

  char password[16] = {0};
	stoc_u tmp[10] = {0};
	uint8 CtrData[3],buf[20],md5_pwd[32],md5_startpwd[32];
  uint8 stopflag = 0,pwdlen = 0;
  START_PARAM  *PtrStartPara = ChgData_GetStartParaPtr(sGlocalPara.gun_no);

	Hmi_ReadMulitVar(DGUS_STARTMODE_CHARGAPSW,&tmp[0],8);
	pwdlen = ChangeReadVarForm(&buf[0],&tmp[0],16);
  Get_BcdCardNo((uint8 *)&password,buf);

	PARAM_DEV_TYPE *ptrDevPara = ChgData_GetDevParamPtr();
	PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	
	#ifdef CHARGE_BIRD_BACK_COMM
	if( (CONN_CHGBIRD == BackCOMM->agreetype) && (ptrDevPara->onlinecard == E_ONLINE_CARD) )
	{
		ChargeBird_PassWord(sGlocalPara.gun_no,&md5_pwd[0],(uint8 *)&password[0]);  //MD5(MD5(密码+帐号)+盐)
    GET_MD5_STARTPASSWD(sGlocalPara.gun_no,md5_startpwd);
		if(0 == memcmp(&md5_pwd[0],&md5_startpwd[0],32))
		{
			stopflag = 1;
		}
	}
	#endif


	if(((strlen(PtrStartPara->password) == strlen(password)) && (strcmp(PtrStartPara->password,password) == 0)) || (1 == stopflag) )
	{		 
	   Screen_DownCounterStop();
		 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
		 Screen_SetChgFlag(0);
//     Screen_ShowMessage((char *)"      ",DGUS_STARTMODE_CHARGAPSW);			 
		 CtrData[0] = 0;//停止
		 CtrData[1] = sGlocalPara.gun_no;//枪号
		 CtrData[2] = 0;//0=按键  1=后台
		 if(AGUN_NO == CtrData[1]){
		   SendMsgWithNByte(MSG_START_STOP,3,&CtrData[0],APP_TASK_AGUNMAINCTRL_PRIO);	
		 }
		 else if(BGUN_NO == CtrData[1]){
		   SendMsgWithNByte(MSG_START_STOP,3,&CtrData[0],APP_TASK_BGUNMAINCTRL_PRIO);	
		 }
		 Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSW,9);
		 Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSWSHOW,9);
	}
	else
	{
		 Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSW,9);
		 Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSWSHOW,9);
		 if(0 == pwdlen)
		 {
			 Screen_ShowMessage((char *)"密码不能为空",DGUS_PASSWD_PROADR);
		 }
		 else
		 {
			 Screen_ShowMessage((char *)"密码错误,请重新输入",DGUS_PASSWD_PROADR);
		 }
		 
		 Delay10Ms(200);
		 Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSW,9);
		 Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSWSHOW,9);
		 Screen_ShowMessage((char *)"       ",DGUS_PASSWD_PROADR);
//		 Screen_ShowMessage((char *)"     ",DGUS_STARTMODE_CHARGAPSW);
	}
}

