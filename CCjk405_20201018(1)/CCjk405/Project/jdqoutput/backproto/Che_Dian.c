/**
  ******************************************************************************
  * @file    Task_Screen.c
  * @author  zqj
  * @version v1.0
  * @date    2016-03-08
  * @brief   
  ******************************************************************************
	*/
//#include "M_Global.h"
#include "Hmi_Api.h"
#include "Uart.h"
#include "Che_Dian.h"
#include "TaskBackComm.h"

#include "Common.h"
#include "message.h"
#include "app_cfg.h"
#include "ChgData.h"
#include "main.h"
#include "RecordFat.h"
#include "gpio.h"

#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include "rtc.h"

#define Frame_Head1  0x75
#define Frame_Head2  0x72
#define CMD_NULL     0xff
#define CARDNO_NUM   16

#ifdef CHE_DIAN_BACKCOMM

extern uint8 Rd_FindUnAccountRecord(CALCBILL_DATA_ST bill[]);
static const uint8 conCtrlPrio[] = {APP_TASK_AGUNMAINCTRL_PRIO,APP_TASK_BGUNMAINCTRL_PRIO};
static const uint8 conSoure[] = {CLIENTA,CLIENTB};

//BackRun_t  BackRun;
extern BackRun_t  BackRun;

void CheDian_LifeAuto(uint8 gunNo,uint8 dir);
void CheDian_LoginAuto(uint8 gunNo,uint8 dir);
void CheDian_QuitAuto(uint8 gunNo,uint8 dir);
void CheDian_GetDevPara(uint8 gunNo,uint8 dir);
void CheDian_GetChgData(uint8 gunNo,uint8 dir);
void CheDian_SetPara(uint8 gunNo,uint8 dir);
void CheDian_RecordAuto(uint8 gunNo,uint8 dir);
void CheDian_AckOnOffSend(uint8 gunNo,uint8 onoff );
void CheDian_UpDataAuto(uint8 gunNo,uint8 dir);
void CheDian_SetTime(uint8 gunNo,uint8 dir);
void CheDian_SetOnOff(uint8 gunNo,uint8 dir);
void CheDian_CardnoAuto(uint8 gunNo,uint8 dir);
void CheDin_FrameSend(uint8 type,uint8 cmd,uint8 soure,uint16 index);


const CheDian_t conCheDian[] = {                 //车电网通讯信息处理
	
	{AUTO_LIFE     , CheDian_LifeAuto},     /*心跳*/		
	{AUTO_LOGIN    , CheDian_LoginAuto},    /*登入*/		
	{AUTO_QUIT     , CheDian_QuitAuto},     /*退出登入*/	
	{GET_DEVPARA   , CheDian_GetDevPara},   /*获取终端数据*/
	{GET_CHGDATA   , CheDian_GetChgData },  /*获取实时数据*/	 //GetUpData_CheDian
	{SET_PARA      , CheDian_SetPara},      /*设置参数*/	
	{AUTO_RECORD   , CheDian_RecordAuto},   /*发送历史记录*/ 	   
	{AUTO_UPDATA   , CheDian_UpDataAuto},   /*发送枪状态数据*/
	{SET_TIME      , CheDian_SetTime},      /*对时*/
	{SET_ONOFF     , CheDian_SetOnOff},     /*启停命令*/ 
	{AUTO_CARDNO   , CheDian_CardnoAuto},   /*发送卡信息*/
	{CMD_NULL      , NULL},   /*无效*/
};

BackRun_t *GetBackRun(void)
{
	return &BackRun;
}

/*充电桩地址*/
uint32 Get_chargeId(void)
{
	PARAM_DEV_TYPE *BgDevParam =ChgData_GetDevParamPtr();
	
	return Common_Ascii2Uint32(BgDevParam->chargeId,sizeof(BgDevParam->chargeId));	
}


/***********************************************************************
** 函数名称: CheDian_PackgCheck()
** 功能描述：校验收后台帧的合法性
** 参数:     
**返回值：   
*************************************************************************/
uint8  CheDian_PackgCheck(uint8 *buf,uint16 len)
{
	uint32 temp, temp1;
	 
 	if( len > (CheDian_MAX-8) ) {
		return 1; //帧长
	}
	 
	if( buf[len - 5] != 0x68) {
		return 2;//结束标识符
	}
	 
	if( (buf[9]&0xf0) != 0x00 ) {
		return 3;//来源  
	}
	 
	temp = Common_LittleToInt(&buf[4]);
	if( temp != Get_chargeId()) {
		return 4;	//桩地址
	}
  
  temp = Common_CalclongSum(buf , (len - 4));
	temp1 = Common_LittleToInt(&buf[len - 4]);
	if( temp != temp1) {
		return 5;	//校验
	}
	 
	return 0;
}
 
/****************************
 获取后台下发数据
 ****************************/
uint8  CheDian_RcvUploadDataDeal(uint8 *CtrData)
{
	static uint8 sCmd9cnt[DEF_MAX_GUN_NO] = {0};
	uint8 cnt,stat,rtn,gunNo;
	uint16 readlen,num,dlen;	
	uint16 u16TmpLen = 0;
	uint8 *ptr = NULL;
	int32 getlen;
  cnt = 0;
	stat = 0;
	num = 0;
	dlen = 0;
	getlen = 0;
	readlen = 1;   //每次读一个数据
	rtn = 0;
	ptr = BackRun.Data;
	BackRun.Len = 0;
	memset(BackRun.Data,0,CheDian_MAX);
	
	do {
		if(1 == cnt)
		{
			rtn = 0;
		}
#ifdef AN_ETH_TASK
		getlen = Back_Read(ptr,readlen);

#else
		getlen = NetHandler.ReadData(ptr,readlen);
#endif
		if( getlen > 0 ) {
			 cnt = 0;

			 switch(stat)
			{
				 case 0:
				 {
					  if( Frame_Head1 == *ptr ) {
							 ptr++ ;
							 stat = 1;
						}
						break;
				 }
				 case 1:
				 {
					   if( Frame_Head2 == *ptr ) {
							 ptr++;
							 stat = 2;
						}else {
							stat = 0;
							ptr = BackRun.Data;//BackRun.Data;
						}
						break;
				 }
				 case 2:
				 {
					   stat = 3;
					   ptr++;
             break;
				 }
				 case 3:
				 {
					   u16TmpLen = (BackRun.Data[3] << 8)+ BackRun.Data[2] + 2;//(BackRun.Data[3]<<8) + BackRun.Data[2] + 2;//2个头字节
					   if((u16TmpLen > 255) ||(u16TmpLen < 10)){
							 stat = 0;
							 ptr =  BackRun.Data;//BackRun.Data;
						 }
						 else{
					     readlen =  u16TmpLen -4;//255-4  下次读取长度
					     dlen = 4;
					     stat = 4;
					     ptr++;
						 }
             break;
				 }
				 case 4:
				 {
					  if( getlen == readlen ) {
							 ptr += getlen;
							 dlen += getlen;
							 if( dlen == u16TmpLen ) {
									rtn = 1;
									break;
							 }
							
						}else if (getlen < readlen  ){
							  readlen = readlen - getlen;
							  dlen += getlen;
							  ptr += getlen;
						}
					 break; 
				 }
				 default:
					 break;
			}
		}
		else {
			if(0 == stat){
				 return FALSE;
			}
			Delay5Ms(2);
			cnt++; 
		}
		
		if( 1 == rtn ) {
			 break;
		}
  }while((cnt < 2 ) && (ptr <  &BackRun.Data[CheDian_MAX]));//&BackRun.Data[120])); /*接收大于120字节退出*/
	
	
  	num = (uint8)(ptr - BackRun.Data);  //&BackRun.Data[0]);
	
	if( (0 == rtn ) || num < CheDian_FrameOther ) {
		   return FALSE;
	  }

	/*校验报文*/
	if(( CheDian_PackgCheck(BackRun.Data,u16TmpLen)) > 0 ) {
		 memset(BackRun.Data,0,CheDian_MAX);
		 return FALSE;
	}
	ptr = BackRun.Data;
//	memcpy(BackRun.Data,buf,u16TmpLen);
	CtrData[0] = BackRun.Data[8];        //cmd
	CtrData[1] = BackRun.Data[9]&0x0f;   //枪号

	DebugRecvCmd(CtrData[0],BackRun.Data, u16TmpLen);

	if(CtrData[1]>0){
		CtrData[1] = (CtrData[1] - 1)% DEF_MAX_GUN_NO;
	}
	
	PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	
	if (GUNNUM_2 == devparaPtr->gunnum)
	{
		gunNo = CtrData[1];
		
		if(CtrData[0] == GET_CHGDATA)
		{
			if(sCmd9cnt[gunNo] > 0)        //在一枪连续发送2次实时数据，改发其他枪数据
			{
				sCmd9cnt[gunNo] = 0;
				gunNo = (gunNo + 1)% DEF_MAX_GUN_NO;
				sCmd9cnt[gunNo] = 1;
				CtrData[1] = gunNo;
			}
			else
			{
				sCmd9cnt[gunNo]++;
				gunNo = (gunNo + 1)% DEF_MAX_GUN_NO;
				sCmd9cnt[gunNo] = 0;
			}
		}	
	}
	
	BackRun.Len = u16TmpLen;
	return TRUE;
}


/***********************************************
车电通讯数据帧发送
soure     帧来源与枪号  ClientFrame, AutoFrame
type      传输类型
gunNo       枪号
len   数据域加数据域前面11个参数
***********************************************/
void CheDin_FrameSend(uint8 type,uint8 cmd,uint8 soure,uint16 len)
{
	uint32  CalclongSum;
	uint8 u8tmp, index = 0;
//STX		0X75 (K)  0X72(H)
	BackRun.Data[index++]  = Frame_Head1;
	BackRun.Data[index++]  = Frame_Head2;   
//帧长,减去2个起始数据
	BackRun.Len = CheDian_FrameOther + len - DataStartAddr;
	Common_LittleShortTo(&BackRun.Data[index],(BackRun.Len - 2));  //发送帧长度参数
	index += 2;
//终端号	
	Common_LittleIntTo(&BackRun.Data[index],Get_chargeId());
  index += 4;  
//命令字		
	BackRun.Data[index++]  = cmd;
//帧来源与枪号	
	PARAM_OPER_TYPE *BgRunParam = ChgData_GetRunParamPtr();
	if(GUNNUM_2 == BgRunParam->gunnum){//双枪
		BackRun.Data[index++]  = soure; 
	}
	else{
		BackRun.Data[index++]  = CLIENTO; 
	}
//传输类型	
	BackRun.Data[index++] = type;	
//结束符	
	BackRun.Data[BackRun.Len - 5] = 0x68;      
//校验和：校验码前面的所有数据
	CalclongSum = Common_CalclongSum(BackRun.Data , (BackRun.Len - 4));
	Common_LittleIntTo(&BackRun.Data[BackRun.Len - 4],CalclongSum);  
//发送数据函数

#ifdef AN_ETH_TASK	
	SendMsgToTcpClient(MSG_DATA_TO_TCPCLIENT,BackRun.Len,BackRun.Data);  
#else
  if(NetHandler.WriteData(BackRun.Data, BackRun.Len) != BackRun.Len){
		NetHandler.WriteData(BackRun.Data, BackRun.Len);
  }
#endif  
	u8tmp = BackRun.Len > 100 ? 100 : BackRun.Len;

	DebugSendCmd(cmd, BackRun.Data, BackRun.Len);
	
	Delay10Ms((u8tmp)/50 + 3);		
	
}


/********************************
10时段费率转48时段费率设置
puff存储格式
10个费率
10个开始时间：小时与时间分开
都只保持A端口费率
*********************************/
uint16 ServerSetFeiLv(uint8 gunNo,uint8 *puff)           //写费率
{
	uint8 i, j,unlen, stopNo, Next = 0, priceNo, lastpriceNo = 0, BMsgData[2];
	uint16 index = DataStartAddr, tmp1, tmp2, GetIndex ,GetIndexSum;
  
	PARAM_FEE_TYPE BgParam;
	PARAM_FEE_TYPE *BFeiLvPtr = &BgParam;
	PARAM_FEE_TYPE *SysDevParam = ChgData_GetFeeRate(gunNo);
	
	memcpy(BFeiLvPtr,SysDevParam,sizeof(PARAM_FEE_TYPE));
	
	SetFeiLv_t BackFeiLv[10];
	uint32 sBackSrvrate = 0,sBackBespkrate = 0,sBackParkrate = 0;
	uint8  sFeiLvNum = 0;
	
	memset(BackFeiLv,0,10*sizeof(SetFeiLv_t));
	 
	//数据单元个数
	GetIndexSum = Common_LittleToShort(&puff[index]);
	index +=2;
	
	//获取后台设置的时段与费率
	for(i = 0; i < GetIndexSum; i++)
	{  
		Delay5Ms(1);			
		GetIndex = Common_LittleToShort(&puff[index]);
		index +=3;//长度字节+1
		
		if(GetIndex == 0x0014){  //停车费单元
				sBackParkrate = Common_LittleToShort(&puff[index]);
				index +=2;
				continue;
		}
		
		if((GetIndex >= 0x0015)&&(GetIndex <= 0x001E)){  //费率单元
				BackFeiLv[GetIndex - 0x0015].FeiLv = Common_LittleToShort(&puff[index]);			  
				index +=2;
				sFeiLvNum++;
				continue;
		}
	
		if((GetIndex >= 0x001F)&&(GetIndex <= 0x0028)){   //开始时间单元
				BackFeiLv[GetIndex - 0x001F].startH = puff[index++];
				BackFeiLv[GetIndex - 0x001F].startM = puff[index++];
				
				if(0x001F != GetIndex){						
					BackFeiLv[GetIndex -1- 0x001F].stopH = BackFeiLv[GetIndex - 0x001F].startH;
					BackFeiLv[GetIndex -1- 0x001F].stopM = BackFeiLv[GetIndex - 0x001F].startM;
					tmp1 = BackFeiLv[GetIndex -1- 0x001F].stopH * 60 + BackFeiLv[GetIndex -1- 0x001F].stopM;
					tmp2 = BackFeiLv[GetIndex -1- 0x001F].startH * 60 + BackFeiLv[GetIndex -1- 0x001F].startM;
					if(tmp1 < tmp2) //开始时间不能小于结束时间
					{
						return GetIndex;
					}
				}
				continue;
		}
		
		if(GetIndex == 0x0029){  //服务费单元				  
				sBackSrvrate = Common_LittleToInt(&puff[index]);
				for(j=0;j<10;j++){									
					BFeiLvPtr->SectTimeFee.srvrate[j] = sBackSrvrate  * (BILL_PRICE_BASE/100);
				}
				index +=4;
				continue;
		}	
		if(GetIndex == 0x002d){  //服务费单元	5个小数	
				unlen = Common_LittleToInt(&puff[index]);
				index +=4;
				for(j=0;j<unlen;j++){									
					BackFeiLv[j].FeiLv = Common_LittleToInt(&puff[index])/(1000);
					index +=4;
				}
				for(j=0;j<unlen;j++){									
					sBackSrvrate = Common_LittleToInt(&puff[index]);
					BFeiLvPtr->SectTimeFee.srvrate[j] = sBackSrvrate  / (100000/BILL_PRICE_BASE);
					index +=4;
				}
				BFeiLvPtr->SectTimeFee.serivetype = SRV_ENERGY_TYPE;
				continue;
		}	
		index +=puff[index - 1];//没用到的单元参数				
	}
	
	Delay5Ms(1);
	BackFeiLv[sFeiLvNum - 1].stopH = 24;
	BackFeiLv[sFeiLvNum - 1].stopM = 0;
	
	//转化成48时段计费 tmp1=10时段开始时间：分   tmp2=10时段结束时间：分  Next：48时段号
	tmp1 = 0;  //
	for(i = 0; i < sFeiLvNum; i++)
	{		 
		 priceNo = 0xff;
		 //检查这个费率与前面时段是否相同
		 for(j=0;j < lastpriceNo;j++){    
			 if(BackFeiLv[i].FeiLv == BFeiLvPtr->SectTimeFee.price[j]){
				 priceNo = j;
			 }
		 }
		 Delay5Ms(1);
		 //添加一个新费率
		 if(0xff == priceNo){				 
			 priceNo = lastpriceNo;
//				 if(lastpriceNo>7){    modify0701zyf
			 if(lastpriceNo>=10){
				 return lastpriceNo;
			 }
			 BFeiLvPtr->SectTimeFee.price[priceNo] = BackFeiLv[i].FeiLv;
			 lastpriceNo++;				 
		 }
		 
		 if(BackFeiLv[i].stopM < 15){
			 stopNo = 0;
		 }
		 else if(BackFeiLv[i].stopM > 45){
			 stopNo = 2;
		 }
		 else{
			 stopNo = 1;
		 }
		 stopNo += BackFeiLv[i].stopH * 2;
		 BFeiLvPtr->SectTimeFee.cdsectNo[i] = Next;			 //计算后台费率在触摸屏对应开始时段  //计算后台费率在触摸屏对应开始时段
		 for(j=Next;j < stopNo;j++){
			 BFeiLvPtr->SectTimeFee.sectNo[Next]  = priceNo;
			 Next++;
		 }			 
		 if(Next>47){
			 break;
		 }
	}
	
	for(i=0;i<10;i++){
		BFeiLvPtr->SectTimeFee.price[i] = BFeiLvPtr->SectTimeFee.price[i] * (BILL_PRICE_BASE/100);
//			BFeiLvPtr->SectTimeFee.srvrate[i] = sBackSrvrate  * (BILL_PRICE_BASE/100);
		BFeiLvPtr->SectTimeFee.bespkrate[i] = sBackBespkrate  * (BILL_PRICE_BASE/100);
		BFeiLvPtr->SectTimeFee.parkrate[i] = sBackParkrate  * (BILL_PRICE_BASE/100);
	}

	if(memcmp(BFeiLvPtr,SysDevParam,sizeof(PARAM_FEE_TYPE)) != 0){
			memcpy(SysDevParam,BFeiLvPtr,sizeof(PARAM_FEE_TYPE));
			memcpy(ChgData_GetFeeRate(BGUN_NO),BFeiLvPtr,sizeof(PARAM_FEE_TYPE));

			BMsgData[0] = SAVE_FEE_TYPE;//
			BMsgData[1] = gunNo;
			SendMsgWithNByte(MSG_PARAM_STORE,2,&BMsgData[0],conCtrlPrio[gunNo]);
	}
	
	return 0;
}

/********************************
48时段费率转10时段费率
puff存储格式
10个费率
10个开始时间：小时与时间分开
*********************************/
uint8 ServerReadFeiLv(uint8 gunNo,uint8 *puff)           //读费率
{
	uint32 temp = 0;
	uint8 i, Next =0, addr, sectNo;
	
	PARAM_FEE_TYPE *BFeiLvPtr = ChgData_GetFeeRate(AGUN_NO);
	
	for(i = 0; i < 48; i++)
	{
		  if(47 != i){				
			  if(BFeiLvPtr->SectTimeFee.sectNo[i] == BFeiLvPtr->SectTimeFee.sectNo[i+1]){					
			    if(46 == i){
					  i = 47;
				  }
				  else {
					  continue;
				  }
		    }
			}
			Next++;			
			if(Next > 10){    //上报时段只有10个
				return Next;
			}
			
			addr = (Next-1)<<1;
			sectNo = BFeiLvPtr->SectTimeFee.sectNo[i];
			temp = BFeiLvPtr->SectTimeFee.price[sectNo] / (BILL_PRICE_BASE/100);
			Common_LittleShortTo(&puff[addr],temp);//price是32位，值待确定
			addr += 20;
			puff[addr]   = i / 2;
			puff[addr+1] = (i % 2) * 30;				
	}

	for(i=Next;i<10;i++){  //时间24：00填充剩余费率段	
			addr = (i<<1) + 20;
			puff[addr]   = 24;
			puff[addr+1] = 0;						
	}
	return 1;
}

/*************************
心跳
**************************/
void CheDian_LifeAuto(uint8 gunNo,uint8 dir)
{	
	uint16 index = DataStartAddr;
	
	if(SendEN == dir){           //发送帧处理
		BackRun.RcvbeatCnt++;
		BackRun.Data[index++] = CheDian_LifeTime;	//心跳间隔
		//设备运行了多少时间
		Common_LittleIntTo(&BackRun.Data[index],DevRunSec);
		index +=4;
		
		CheDin_FrameSend(AutoFrame, AUTO_LIFE, CLIENTO, index);
		BackRun.LifeTime = GetSystemTick();
	}
	else if(RcvFlag == dir){    //接收帧处理
    BackRun.RcvbeatCnt = 0;
  }
}

/**************************
登入请求
***************************/
void CheDian_LoginAuto(uint8 gunNo,uint8 dir)
{
	uint16 index = DataStartAddr;
	uint8 i;
	
	if(SendEN == dir){           //发送帧处理
		struct tm time_now;
		//登入时间
		time_now = Time_GetSystemCalendarTime();		
		BackRun.Data[index++] = Common_Hex2bcd8(time_now.tm_year/100);	//年
		BackRun.Data[index++] = Common_Hex2bcd8(time_now.tm_year%100);
		BackRun.Data[index++] = Common_Hex2bcd8(time_now.tm_mon);
		BackRun.Data[index++] = Common_Hex2bcd8(time_now.tm_mday);
		BackRun.Data[index++] = Common_Hex2bcd8(time_now.tm_hour);
		BackRun.Data[index++] = Common_Hex2bcd8(time_now.tm_min);
		BackRun.Data[index++] = Common_Hex2bcd8(time_now.tm_sec);
		//设备类型
		PARAM_OPER_TYPE *BgRunParam = ChgData_GetRunParamPtr();
		if(GUNNUM_2 == BgRunParam->gunnum){//双枪
			BackRun.Data[index++] = DEV_TYPE_D;
		}
		else{
			BackRun.Data[index++] = DEV_TYPE_S;
		}
		//预留信息
		BackRun.Data[index++] = 0x00;
		BackRun.Data[index++] = 0x00;
		BackRun.Data[index++] = 0x00;
		BackRun.Data[index++] = 0x00;
		//桩版本号		
		BackRun.Data[index++] = 0x00;
		BackRun.Data[index++] = 0x00;
		BackRun.Data[index++] = 0x00;
		BackRun.Data[index++] = 0x00;
    //保留信息	
		memset(&BackRun.Data[index], 0, 16);
		index += 16;
	
		CheDin_FrameSend(AutoFrame, AUTO_LOGIN, CLIENTO, index);
	}
	else if(RcvFlag == dir){    //接收帧处理
    BackRun.LifeTime   = 0;
		BackRun.CardnoTime = GetSystemTick();		
		BackRun.ErrTime    = GetSystemTick();
		BackRun.RecordTime = 0;
		
		if(0 == BackRun.Data[DataStartAddr]){
			BackRun.Commforbidden = 0x00;        //登入
		}
		for(i=0;i<DEF_MAX_GUN_NO;i++){
			CheDian_UpDataAuto(i,SendEN);
	//		BackRun.UpDataTime[i] = GetSystemTick();
		}
		
  }
}

/******************************************
登入退出请求
****************************************/
void CheDian_QuitAuto(uint8 gunNo,uint8 dir)
{
	uint16 index = DataStartAddr;
	
	if(SendEN == dir){           //发送帧处理
		struct tm time_now;
		//登入时间
		time_now = Time_GetSystemCalendarTime();		
		BackRun.Data[index++] = Common_Hex2bcd8(time_now.tm_year/100);	//年
		BackRun.Data[index++] = Common_Hex2bcd8(time_now.tm_year%100);
		BackRun.Data[index++] = Common_Hex2bcd8(time_now.tm_mon);
		BackRun.Data[index++] = Common_Hex2bcd8(time_now.tm_mday);
		BackRun.Data[index++] = Common_Hex2bcd8(time_now.tm_hour);
		BackRun.Data[index++] = Common_Hex2bcd8(time_now.tm_min);
		BackRun.Data[index++] = Common_Hex2bcd8(time_now.tm_sec);
		
		CheDin_FrameSend(RequestFrame, AUTO_QUIT, CLIENTO, index);
	}
	else if(RcvFlag == dir){    //接收帧处理
		
    if(0 == BackRun.Data[DataStartAddr]){
			BackRun.Commforbidden = 0xAA;        //登入退出
		}
		CheDian_UpDataAuto(gunNo,SendEN);
  }
}

/***********************
主机获取终端数据
***********************/
void CheDian_GetDevPara(uint8 gunNo,uint8 dir)
{	
	DEV_ACMETER_TYPE *BgAcMete = TskAc_GetMeterDataPtr(0);
	PARAM_DEV_TYPE *BgDevParam = ChgData_GetDevParamPtr();
	PARAM_OPER_TYPE *BgRunParam = ChgData_GetRunParamPtr();
	PARAM_FEE_TYPE *BFeiLvPtr = ChgData_GetFeeRate(0);         //TEST
	
	int16 sendunit = 0;
	uint16 index = DataStartAddr , GetIndex = 0;
	uint8 Getpuff[100],GetFeiLv[40], addr, GetIndexSum = 0, i;
	
	if(gunNo >= DEF_MAX_GUN_NO)return;
	if(RcvFlag == dir){    //接收帧处理
		    
		GetIndexSum = Common_LittleToShort(&BackRun.Data[index]);//数据单元个数
		if((GetIndexSum > 50)||(0 == GetIndexSum)){
			return;
		}
		index +=2;
		
		ServerReadFeiLv(gunNo,GetFeiLv); //48时段转成10时段
		
		memcpy(Getpuff,&BackRun.Data[index],(GetIndexSum * 2));
		memset(&BackRun.Data[0],0,CheDian_MAX);
		for(i = 0;i < GetIndexSum;i ++){
			GetIndex = Common_LittleToShort(&Getpuff[i<<1]);
			Common_LittleShortTo(&BackRun.Data[index],GetIndex);
			index +=2;
			switch(GetIndex){
				case 0x0001:   //桩类型
					BackRun.Data[index++] = 1;
				  
					if(GUNNUM_2 == BgRunParam->gunnum){//双枪
						BackRun.Data[index++] = DEV_TYPE_D;
					}
					else{
						BackRun.Data[index++] = DEV_TYPE_S;
					}
					break;
				case 0x0002:   //桩标号
					BackRun.Data[index++] = 4;
				  Common_LittleIntTo(&BackRun.Data[index],Get_chargeId());  
          index += 4;	
					break;
				case 0x0003:    //IP
					BackRun.Data[index++] = 4;
				  memcpy(&BackRun.Data[index],&BgDevParam->bkcomm.netpara.LocalIpAddr,4);
          index += 4;	
					break;
				case 0x0004:    //gw 网关
					BackRun.Data[index++] = 4;
				  memcpy(&BackRun.Data[index],&BgDevParam->bkcomm.netpara.NetGate,4);  
          index += 4;	
					break;
				case 0x0005:    //Mask 子网掩码
					BackRun.Data[index++] = 4;
				  memcpy(&BackRun.Data[index],&BgDevParam->bkcomm.netpara.NetMask,4);
          index += 4;	
					break;
				case 0x0006:    //网络端口号
					BackRun.Data[index++] = 2;
				  Common_LittleShortTo(&BackRun.Data[index],BgDevParam->bkcomm.netpara.RemotePort);
          index += 2;	
					break;
				case 0x0007:    //桩体命名LW
					BackRun.Data[index++] = 8;
				  Common_LittleShortTo(&BackRun.Data[index],Get_chargeId());  
          index += 8;	
					break;
				case 0x0008:    //控制方式：1开启远程，0关闭
					BackRun.Data[index++] = 1;
				  BackRun.Data[index++] = 1; 
					break;
				case 0x0009:    //结算方式：1远程，0本地
					BackRun.Data[index++] = 1;
				  BackRun.Data[index++] = 1; 
					break;
				case 0x000A:    //计量方式：0分段，1统一电价
					BackRun.Data[index++] = 1;
				  BackRun.Data[index++] = 0; 
					break;
				case 0x000B:    //桩体最高电压
					BackRun.Data[index++] = 4;
				  Common_LittleShortTo(&BackRun.Data[index],BgRunParam->maxvolt);
          index += 4;
					break;
				case 0x000C:    //桩体最高电流
					BackRun.Data[index++] = 4;
				  Common_LittleShortTo(&BackRun.Data[index], BgRunParam->maxcurr);
          index += 4;
					break;
				case 0x000D:    //桩体最低电压
					BackRun.Data[index++] = 4;
				  Common_LittleShortTo(&BackRun.Data[index],BgRunParam->minvolt);
          index += 4;
					break;
//				case 0x000E:    //桩体额定电压LW
//					BackRun.Data[index++] = 4;
//				  Common_LittleIntTo(&BackRun.Data[index],add);
//          index += 4;
//					break;
//				case 0x000F:    //桩体额定电流LW
//					BackRun.Data[index++] = 4;
//				  Common_LittleIntTo(&BackRun.Data[index],add);
//          index += 4;
//					break;
				case 0x0010:    //进相电压 A  
					BackRun.Data[index++] = 4;
				  Common_LittleShortTo(&BackRun.Data[index],BgAcMete->Uab);
          index += 4;
					break;
				case 0x0011:    //进相电压 B 
					BackRun.Data[index++] = 4;
				  Common_LittleShortTo(&BackRun.Data[index],BgAcMete->Ubc);
          index += 4;
					break;
				case 0x0012:    //进相电压 C 
					BackRun.Data[index++] = 4;
				  Common_LittleShortTo(&BackRun.Data[index],BgAcMete->Uca);
          index += 4;
					break;
				case 0x0013:    //是否加停车服务费：1加收，0不
					BackRun.Data[index++] = 1;
				  BackRun.Data[index++] = 0;
					break;
				case 0x0014:    //停车单价0.01
					BackRun.Data[index++] = 2;
				  BackRun.Data[index++] = 0;
		      BackRun.Data[index++] = 0;
				  Delay5Ms(1);
					break;
				case 0x0015:    //阶段电价0.01
			  case 0x0016:
        case 0x0017:
        case 0x0018:
        case 0x0019:
        case 0x001A:
        case 0x001B:
        case 0x001C:
        case 0x001D:
        case 0x001E:	
          addr = (GetIndex - 0x0015)<<1;					
					BackRun.Data[index++] = 2;
				  memcpy(&BackRun.Data[index],&GetFeiLv[addr],2);
          index += 2;
					break;
				case 0x001F:    //阶段时间
			  case 0x0020:
        case 0x0021:
        case 0x0022:
        case 0x0023:
        case 0x0024:
        case 0x0025:
        case 0x0026:
        case 0x0027:
        case 0x0028:		
          addr = (GetIndex - 0x0015)<<1;			
					BackRun.Data[index++] = 2;
				  memcpy(&BackRun.Data[index],&GetFeiLv[addr],2);
          index += 2;
					break;
				case 0x0029:			//充电服务费		
					BackRun.Data[index++] = 4;
				  Common_LittleShortTo(&BackRun.Data[index],BFeiLvPtr->SectTimeFee.srvrate[0]);
          index += 4;
					break;
				case 0x002A:			//硬件版本
					BackRun.Data[index++] = 20;
				  memset(&BackRun.Data[index] ,0,20);
		      index += 20;
					break;
				case 0x002B:			//软件版本
					BackRun.Data[index++] = 20;
				  memset(&BackRun.Data[index] ,0,20);
		      index += 20;
					break;
//				case 0x002C:			//机器串码	
//					BackRun.Data[index++] = 25;
//				  memset(&BackRun.Data[index] ,0,25);				  
//  			  memcpy(&BackRun.Data[index] ,BgDevParam->chargeId,8);
//		      index += 25;
//					break;
				default:
					sendunit --;
				  index = index -2;
					break;
			}
			sendunit++;
			if(index > CheDian_MAX)
			{
				return;
			}
		}	
    Common_LittleShortTo(&BackRun.Data[DataStartAddr],sendunit);				
		CheDin_FrameSend(ResponseFrame, GET_DEVPARA, CLIENTO, index);
  }
}

/****************************************
获取告警数据：车电桩协议中6.3告警定
*******************************************/
void GetRunAlarm(uint8 gunNo,uint8 *buff)
{
//	uint8 i;
	DEV_ACMETER_TYPE *BgAcMete = TskAc_GetMeterDataPtr(gunNo);
	CHARGE_TYPE  *ptrBgRunData = ChgData_GetRunDataPtr(gunNo);
	
	memset(buff,0,8);
	if(1 == ptrBgRunData->meter->statu.bits.commerr){  //?????
		buff[0] += 0x10;
	}

	if(1 == Check_GetErrCode(gunNo,ECODE18_CC1LNK)){
		buff[0] += 0x20;
	}

	if(BgAcMete->statu.bits.UabOver){  //过压 A
		buff[1] += 0x01;
	}

	else if(BgAcMete->statu.bits.UabUnder){  //欠压 A
		buff[1] += 0x02;
	}

	if(BgAcMete->statu.bits.UbcOver){  //过压 B
		buff[1] += 0x04;
	}
	else if(BgAcMete->statu.bits.UbcUnder){  //欠压 B
		buff[1] += 0x08;
	}

	if(BgAcMete->statu.bits.UcaOver){  //过压 C
		buff[1] += 0x10;
	}
	else if(BgAcMete->statu.bits.UcaUnder){  //欠压 C
		buff[1] +=0x20;
	}
	
	if(1 == ptrBgRunData->input->statu.bits.stop){  //急停
		buff[2] += 0x01;
	}
	if(1 == ptrBgRunData->input->statu.bits.spd){   //防雷器
		buff[2] += 0x02;
	}
	
//	if(BgRunData->iso->statu.word){
//		buff[2] += 0x20;
//	}
		
//	if(BgRunData->bms->car.bsm.battalm.bits.linkalm){
//		buff[2] += 0x08;
//	}
	if(1 == Check_GetErrCode(gunNo,ECODE31_BMSCOMM)){
		buff[2] += 0x08;
	}
}
/****************************
判断充电卡类型  LW
程序中与后台定义的类型需要转换
****************************/
uint8 GetCardType2(uint8 *cardno,uint8 chgtype)
{
	uint8 rtn = 0;
	if(START_BY_CARD == chgtype){ //???
		rtn = 0x61;
	}
	else if(START_BY_BKGROUND == chgtype){ //???
		rtn = 0x71;
	}
	else if(START_BY_ONLINECARD == chgtype){ //???
		rtn = 0x70;
	}
	else if(START_BY_VIN == chgtype){ //app????		
			rtn = 0x78;      //app????		
	}
	return rtn;
}

/********************
0 空闲
1 连接中，非空闲，后台可充电
2 充电中
3 充电完成
********************/
uint8 GetWorkType(uint8 gunNo)
{
	CHARGE_TYPE  *BgRunData = ChgData_GetRunDataPtr(gunNo);
  uint8 rtn = 0;

  switch(BgRunData->logic->workstep){
		case  STEP_IDEL:      /*空闲阶段*/
			rtn = (CC1_4V == BgRunData->gun->statu.bits.cc1stu) ? 1 : 0;
			break;
	  case  STEP_START:
		case  STEP_LINK:
		case  STEP_BPOWER:
		case  STEP_SKHANDS:	
    case  STEP_ISOCHECK_SUB:			
	  case  STEP_ISOWAIT_SUB:
		case  STEP_ISOFINISH_SUB:		
		case  STEP_PARAMSET:
		case  STEP_OLD_GBT_SUB:			
		case  STEP_BRM_WAIT_SUB:			
	  case  STEP_BRM_SUB:
		case  STEP_BCPWAIT_SUB:
		case  STEP_BCP_SUB:			
	  case  STEP_BROWAIT_SUB:
		case  STEP_BRO_SUB:			
	  case  STEP_ADJUST_SUB:			
		case  STEP_CHARGEING:		
      rtn = 2;
     break;		
		case  STEP_CHGEND:
		case  STEP_WAITOVER:	
			//已经上传记录
			rtn = (0 == g_BackCtrl.Runflag[gunNo]) ? 3 : 2;	
			break;
    case  STEP_WAIT_SUB:	
      rtn = 3;
      break;		
		default:
			break;;
	}
  return rtn;	
}

/************************************************
获取实时数据
待处理：将充电运行状态改变，主动上传一次实时数据
待测试时候看后台是否不断下发实时请求命令
*************************************************/
uint8 GetUpData_ItemData(uint8 gunNo, uint16 item,uint8 *buf)
{
	   uint8 index ;
	   uint8 *ptrData = buf;

		 CHARGE_TYPE  *PtrBgRunData = ChgData_GetRunDataPtr(gunNo);
		 REAL_BILL_DATA_ST *PtrBgFeeData = GetFeeData(gunNo);
	
	   index = 2;
	   Common_LittleShortTo(&ptrData[0],item);
		 switch(item){
//				case 0x0603:   //整车蓄电池额度容量  
//					ptrData[index++] = 2;
//  			  Common_LittleShortTo(&ptrData[index],PtrBgRunData->bms->car.brm.battecapty);  
//          index += 2;	
//					break;
//				case 0x0604:   //额度总电压  
//					ptrData[index++] = 2;
//				  Common_LittleShortTo(&ptrData[index],PtrBgRunData->bms->car.brm.battvolte);  
//          index += 2;	
//					break;
				case 0x0B01:    //设备状态  
					ptrData[index++] = 1;
				  ptrData[index++] = GetWorkType(gunNo);
					break;
				case 0x0B02:    //充电卡号
					ptrData[index++] = 20;				  
				  UpCardNoASCII(&ptrData[index],PtrBgRunData->bill->cardNo);
		      index += 20;	
					break;
				case 0x0B03:    //车辆VIN    
					ptrData[index++] = 17;
				  memset(&ptrData[index], 0, 17); 
				  memcpy(&ptrData[index],PtrBgRunData->bms->car.brm.vin,17);
          index += 17;	
					break;
				case 0x0B04:    //充电电压
					ptrData[index++] = 4;
          Common_LittleIntTo(&ptrData[index],PtrBgRunData->meter->volt); 				
          index += 4;	
					break;
				case 0x0B05:    //充电电流
					ptrData[index++] = 4;
				  Common_LittleIntTo(&ptrData[index],PtrBgRunData->meter->current*10);  
          index += 4;	
					break;
				case 0x0B06:    //充电时间
					ptrData[index++] = 4;
				  Common_LittleIntTo(&ptrData[index],PtrBgRunData->logic->time);  
          index += 4;	
					break;
				case 0x0B07:    //充电金额
					ptrData[index++] = 4;
				  Common_LittleIntTo(&ptrData[index],PtrBgRunData->logic->money);  
          index += 4;	
					break;
				case 0x0B08:    // 充电电能
					ptrData[index++] = 4;
				  Common_LittleIntTo(&ptrData[index],PtrBgRunData->logic->energy);  
          index += 4;	
					break;
				case 0x0B09:    //剩余时间  
					ptrData[index++] = 4;
				  Common_LittleShortTo(&ptrData[index],PtrBgFeeData->real.lasttimes);
          index += 4;
					break;
				case 0x0B0A:    //当前SOC  
					ptrData[index++] = 1;
				  ptrData[index++] = PtrBgFeeData->bill.soc % 101;
					break;
				case 0x0B0B:    //告警信息  
					ptrData[index++] = 8;
				  memset(&ptrData[index],0,8);
				  GetRunAlarm(gunNo,&ptrData[index]);
          index += 8;
					break;
				case 0x0B0C:    //充电卡余额
					ptrData[index++] = 4;
				  Common_LittleIntTo(&ptrData[index],PtrBgRunData->bill->beforemoney);
          index += 4;
					break;
				case 0x0B0D:    //充电卡类型
					ptrData[index++] = 1;
				  ptrData[index++] = GetCardType2(PtrBgRunData->bill->cardNo,PtrBgRunData->bill->startmod);  //GetCardType(PtrBgRunData->bill->cardNo,PtrBgRunData->bill->cardtype);
					break;
				case 0x0B0E:    //充电方式  1:立即充电 2：预约充电
					ptrData[index++] = 1;
				  ptrData[index++] = 1;
					break;
				case 0x0B0F:    //充电模式  
					ptrData[index++] = 1;
				  ptrData[index++] = 0;;
					break;
//				case 0x0B10:    //电压需求 
//					ptrData[index++] = 4;
//				  Common_LittleShortTo(&ptrData[index],PtrBgRunData->bms->car.bcl.needvolt);
//          index += 4;
//					break;
//				case 0x0B11:    //电流需求 
//					ptrData[index++] = 4;
//				  Common_LittleShortTo(&ptrData[index],PtrBgRunData->bms->car.bcl.needcurr);
//          index += 4;
//					break;
//				case 0x0B12:    //车位锁状态 
//					ptrData[index++] = 1;
//				  ptrData[index++] = 0x18; //未知
//					break;
				case 0x0B13:    //当前电能表读数					
					ptrData[index++] = 4;
				  Common_LittleIntTo(&ptrData[index],PtrBgRunData->meter->energy);
          index += 4;
					break;
//				case 0x0B14:    //充电电压
//					ptrData[index++] = 4;
//				  Common_LittleIntTo(&ptrData[index],PtrBgRunData->meter->volt); 	
//          index += 4;
//					break;
//				case 0x0B15:			//充电电流	
//					ptrData[index++] = 4;
//				  Common_LittleIntTo(&ptrData[index],PtrBgRunData->meter->current*10); 	
//          index += 4;
//					break;
				default:
					index -= 2;
					break;
			}
			return index;
}

/*******************************
充电实时参数上传
********************************/
#define MAX_UNIT_BUFF  256
void CheDian_GetChgData(uint8 gunNo,uint8 dir)
{
	uint16 index;
  uint16	Item,rtn;
	uint8 GetIndexSum = 0;
	uint8 Getpuff[MAX_UNIT_BUFF];
 
	uint8 i;
	int16 sendunit = 0;

  index = DataStartAddr;

	if(gunNo >= DEF_MAX_GUN_NO)return;
	if ( RcvFlag == dir ){    //接收帧处理
	
    //数据单元个数
		GetIndexSum = Common_LittleToShort(&BackRun.Data[index]);
		if((GetIndexSum > 128)||(0 == GetIndexSum)){
			return;
		}
		
		index +=2;
		
		//保存需要单元参数
		memcpy(Getpuff,&BackRun.Data[index],(GetIndexSum * 2));
		memset(&BackRun.Data[0],0,CheDian_MAX);		
		
		//填充单元参数
		sendunit = 0;
		for(i = 0 ; i < GetIndexSum ; i++){
			Item = Common_LittleToShort(&Getpuff[i<<1]);
			rtn =  GetUpData_ItemData(gunNo,Item,&BackRun.Data[index]);
			if( rtn > 0 ) {
				index += rtn;
				sendunit++;
			}
			if(index > CheDian_MAX)
			{
				return;
			}
		}//end for
		BackRun.UpDataTime[gunNo] = GetSystemTick();
    Common_LittleShortTo(&BackRun.Data[DataStartAddr],sendunit);		
		CheDin_FrameSend(ResponseFrame, GET_CHGDATA, conSoure[gunNo], index);
   }
}

/*************************************
主机写参数
************************************/
void CheDian_SetPara(uint8 gunNo,uint8 dir)
{
	uint16 index = DataStartAddr, temp;
	
//	if(gunNo >= DEF_MAX_GUN_NO)return;
	if(RcvFlag == dir){    //接收帧处理
	  if(0 != ServerSetFeiLv(AGUN_NO,BackRun.Data))
		{
			return;
		}
		//数据单元个数
		BackRun.Data[index++] = 0x14;
		BackRun.Data[index++] = 0;
		
	//数据单元标识
 	  for(uint8 i = 0; i < 0x14; i++)
		{
			temp = i + 0x15;
			memcpy(&BackRun.Data[index],&temp,2);
			index += 2;
		}		
		CheDin_FrameSend(ResponseFrame, SET_PARA, conSoure[gunNo], index);	
  }
}


/***************************************** 
获取充电记录 
SerialNo = 0
找任意记录上传
否则找流水号等于SerialNo上传，

找到的记录赋值给BackGet
没有返回1
有返回0
******************************************/
uint8  CheDian_ChgRecord(CALCBILL_DATA_ST *BackGet)
{
	return Rd_FindUnAccountRecord(BackGet);
}	

/************************ 
响应主站对充电记录的确认 
对比流水号
**************************/
uint32 AckHostSureChgRd(uint8 *buf)
{
	uint32 TmpSerialNum,  reson;
	uint8 GetSerialNum[32];
	
	TmpSerialNum = Common_LittleToInt(&buf[0]);  //流水号
	reson = Common_LittleToInt(&buf[8]);         //结果
	if( reson > 1 ){   //0正常，1流水号重复 2其他
		return 0xffff;
	}
	memset(GetSerialNum,0,32);
	memcpy(GetSerialNum,&buf[0],4);
	if(1 == Clear_RecordIsReport(GetSerialNum)){
			return TmpSerialNum;
	}
	return 0xffff;
}


/*填充时段电量*/
void  Fill_SectEnergy(uint8 *buff,CALCBILL_DATA_ST *ptrBackRecord)
{
	uint8 TimeNum = 10;
	uint8 i, StaNum , endNum;
	uint32 u32Tmp;
	PARAM_FEE_TYPE *SysDevParam = ChgData_GetFeeRate(AGUN_NO);
	
	StaNum = 0;
	for(i = 0; i < TimeNum; i++){
		
		if(i < (TimeNum - 1)){		
			endNum = (0 == SysDevParam->SectTimeFee.cdsectNo[i+1]) ? MAX_CHG_SECT : (SysDevParam->SectTimeFee.cdsectNo[i+1]%MAX_CHG_SECT);
		}
		else{
			endNum = MAX_CHG_SECT;
		}
		
		u32Tmp = 0;
		for(;StaNum < endNum; StaNum++){
			u32Tmp += ptrBackRecord->sectenergy[StaNum];
		}
		Common_LittleIntTo(&buff[i*8],u32Tmp);
		if(endNum >= MAX_CHG_SECT){
			break;
		}
	}
}

/**填充充电记录**/
uint16 Fill_OneRecordItem(uint16 Item,uint8 *data,CALCBILL_DATA_ST *ptrBackRecord)
{
	uint8 index ;
	uint8 *ptrItemData = data;
	
	index = 0;
	Common_LittleShortTo(&ptrItemData[index],Item);
	index +=2;
	switch(Item){
				case 0x0101:   //充电方式  1：立即充电   2：预约充电
					ptrItemData[index++] = 1;
				  ptrItemData[index++] = 1;
					break;
				case 0x0102:   //充电模式
					ptrItemData[index++] = 1;
				  ptrItemData[index++] = 0;
					break;
				case 0x0103:    //充电卡类型   
					ptrItemData[index++] = 1;
				  ptrItemData[index++] = GetCardType2(ptrBackRecord->cardNo,ptrBackRecord->startmod);
					break;
				case 0x0104:    //充电卡号
					ptrItemData[index++] = 20;
				  UpCardNoASCII(&ptrItemData[index],ptrBackRecord->cardNo);
          index += 20;	
					break;
				case 0x0105:    //车辆VIN
					ptrItemData[index++] = 17;
				  memcpy(&ptrItemData[index],&ptrBackRecord->vincode,17); 
          index += 17;	
					break;
				case 0x0106:    //充电前余额
					ptrItemData[index++] = 4;			
				  Common_LittleIntTo(&ptrItemData[index],ptrBackRecord->beforemoney);
          index += 4;	
					break;
//				case 0x0107:    //充电电压
//					ptrItemData[index++] = 4;
//				  Common_LittleIntTo(&ptrItemData[index],ptrBackRecord->beforemoney); //lw
//          index += 4;	
//					break;
//				case 0x0108:    //充电电流
//					ptrItemData[index++] = 4;
//				  Common_LittleIntTo(&ptrItemData[index],ptrBackRecord->beforemoney); //lw
//          index += 4;	 
//					break;
				case 0x0109:    //充电时间
					ptrItemData[index++] = 4;
				  Common_LittleIntTo(&ptrItemData[index],ptrBackRecord->chgsecs);
          index += 4;	
					break;
				case 0x010A:    //充电金额
					ptrItemData[index++] = 4;
				  Common_LittleIntTo(&ptrItemData[index],ptrBackRecord->billmoney);
          index += 4; 
					break;
				case 0x010B:    //充电电量
					ptrItemData[index++] = 4;
				  Common_LittleIntTo(&ptrItemData[index],ptrBackRecord->energy);
          index += 4;
					break;
				case 0x010C:    //充电前电量
					ptrItemData[index++] = 4;
				  Common_LittleIntTo(&ptrItemData[index],ptrBackRecord->startenergy);
          index += 4;
					break;
				case 0x010D:    //充电后电量
					ptrItemData[index++] = 4;
				  Common_LittleIntTo(&ptrItemData[index],ptrBackRecord->stopenergy);
          index += 4;
					break;
//				case 0x010E:    //剩余时间  lw
//					ptrItemData[index++] = 4;
//				  Common_LittleIntTo(&ptrItemData[index],ptrBackRecord->storeNo);
//          index += 4;
//					break;
				case 0x010F:    //当前SOC
					ptrItemData[index++] = 1;
				  ptrItemData[index++] = ptrBackRecord->soc;
					break;
				case 0x0110:    //是否上传主站
					ptrItemData[index++] = 1;
				  ptrItemData[index++] = ptrBackRecord->IsReport;
					break;
				case 0x0111:    //是否付费  
					ptrItemData[index++] = 1;
				  ptrItemData[index++] = 0;//BackRecord.IsPay;
					break;
				case 0x0112:    //终止原因
					ptrItemData[index++] = 1;
				  ptrItemData[index++] = ptrBackRecord->endreason;
					break;
				case 0x0113:    //充电开始时间 
					ptrItemData[index++] = 7;
				  ptrItemData[index++] = ptrBackRecord->starttm[0];
				  ptrItemData[index++] = 0x20;
				  memcpy(&ptrItemData[index],&(ptrBackRecord->starttm[1]),5); 
          index += 5;
					break;
				case 0x0114:    //充电结束时间 
					ptrItemData[index++] = 7;
				  ptrItemData[index++] = ptrBackRecord->stoptm[0];
				  ptrItemData[index++] = 0x20;
				  memcpy(&ptrItemData[index],&ptrBackRecord->stoptm[1],5); 
          index += 5;
					break;
				case 0x0115:    //记录流水号	
					ptrItemData[index++] = 4;	
				  memcpy(&ptrItemData[index],ptrBackRecord->serialNo,4);
          index += 4;
					break;
				case 0x0116:    //记录存储号			
					ptrItemData[index++] = 4;
				  memcpy(&ptrItemData[index],ptrBackRecord->serialNo,4);
	//			  Common_LittleIntTo(&ptrData[index],BackRecord.storeNo);			  
          index += 4;
					break;
				case 0x0117:			//10个时段充电电量与金额   lw
					ptrItemData[index++] = 80;
				  memset(&ptrItemData[index],0,80);
				  Fill_SectEnergy(&ptrItemData[index],ptrBackRecord);
	//			  Common_LittleIntTo(&ptrItemData[index],ptrBackRecord->energy);
          index += 80;
					break;
				default:
					index -= 2;
					break;
			}
			return index;
}



uint16 FillGetRecord(CALCBILL_DATA_ST *ptrBackRecord)
{
	uint16 i,rtn;
	uint16 sendunit = 0;
	uint16 startitem = 0x0101; 
	uint16 enditem = 0x0118; 
	uint16 index = DataStartAddr;
	//数据单元个数		
	index +=2;
	memset(&BackRun.Data[0],0,CheDian_MAX);
	for( i = startitem; i < enditem; i++ ) {
		rtn = Fill_OneRecordItem(i,&BackRun.Data[index],ptrBackRecord);
		if( rtn > 0 ) {
			sendunit++;
			index += rtn;
			if(index > CheDian_MAX){
				return 0;
			}
		}
	}
	Common_LittleShortTo(&BackRun.Data[DataStartAddr],sendunit);
	
	return index;
}


/*************************
上传在线充电记录
**************************/
void CheDian_RecordAuto(uint8 gunNo,uint8 dir)
{	
	uint16 index = DataStartAddr;
	CALCBILL_DATA_ST  BackGetRecor;
	if(gunNo >= DEF_MAX_GUN_NO)return;
	if(SendEN == dir){           //发送帧处理
		if( 1 == BackRun.RecordMsg ) {  /*从账单上传*/
				BackRun.RecordMsg = 0;
				memcpy(&BackGetRecor, Bill_GetBillData(gunNo%DEF_MAX_GUN_NO),sizeof(CALCBILL_DATA_ST));
			
				index = FillGetRecord(&BackGetRecor);
        gunNo = BackGetRecor.gunno;				
				CheDin_FrameSend( AutoFrame, AUTO_RECORD, conSoure[gunNo], index);	
			
	  }else if(1 == CheDian_ChgRecord(&BackGetRecor)) {			
			index = FillGetRecord(&BackGetRecor);	
      gunNo = BackGetRecor.gunno;			
			CheDin_FrameSend(AutoFrame, AUTO_RECORD, conSoure[gunNo], index);	
    }	
	}
	else if(RcvFlag == dir){    //接收帧处理  		
		AckHostSureChgRd(&BackRun.Data[index+2]);			
  }	
}




/***********************
主机对时
***********************/
void CheDian_SetTime(uint8 gunNo,uint8 dir)
{
	uint16 index = DataStartAddr;
	
	if(RcvFlag == dir){    //接收帧处理
		//更新时间
		updataTime(&BackRun.Data[DataStartAddr], 0);
    //确认标识符		
		BackRun.Data[index++] = 0x00;
		CheDin_FrameSend(ResponseFrame, SET_TIME, CLIENTO, index);		
  }
}

/***********************************
处理主机发送的关闭启动信息
0.启动成功   1.启动失败
2.停止成功   3.停止失败
***********************************/
extern uint8 IsJudgeDevErrDtatus(void);
uint8 Deal_SetOnOff(uint8 gunNo)
{
	START_PARAM  *PtrStartPara = ChgData_GetStartParaPtr(gunNo);
	CHARGE_TYPE  *BgRunData = ChgData_GetRunDataPtr(gunNo);
	
	uint8 CtrlType,CtrData[3] ; 
	uint16 index = DataStartAddr;
	uint32 Value32;
	
  CtrlType = BackRun.Data[index++];
	if(CtrlType == 0x01)
	{
		/* 启动充电 */
		if ((PtrStartPara->vailfg != 0) || (CC1_4V != BgRunData->gun->statu.bits.cc1stu))
		{
			/* 被占用，不能使用 */
			/* 枪没插好 */
			return ON_FAIL;
		}
		else
		{				
			    PtrStartPara->startby = START_BY_BKGROUND; /*扫码后台*/	
			    //卡 号
			    Get_BcdCardNo(PtrStartPara->account,&BackRun.Data[index]);
			    //memcpy(PtrStartPara->account,&BackRun.Data[index],20);
			    index += 20;
					//卡余额
					PtrStartPara->money = Common_LittleToInt(&BackRun.Data[index]);
					index += 4;
					//后台交易流水号
			    memset(PtrStartPara->serilNo,0,32);
			    memcpy(PtrStartPara->serilNo,&BackRun.Data[index],4);
					index += 4;
					//充电模式				
          CtrlType	= (Common_LittleToInt(&BackRun.Data[index]) % 4);//CTR_AUTO
					index += 4;
					//充电模式值：金额-分  时间-秒  电量-0.01KW*H	
			    Value32 = Common_LittleToInt(&BackRun.Data[index]);
			    index += 4;
			    switch(CtrlType){
						case 0:
							PtrStartPara->chgmod = CHG_AUTO_TYPE;
							break;
						case 1:
							PtrStartPara->chgmod = CHG_MONEY_TYPE;
							PtrStartPara->setmoney = Value32;
							break;
						case 2:
							PtrStartPara->chgmod = CHG_TIME_TYPE;
							PtrStartPara->settime = Value32;
							break;
						case 3:
							PtrStartPara->chgmod = CHG_ENERGY_TYPE;
							PtrStartPara->setenergy = Value32;
							break;
						default:
							PtrStartPara->chgmod = CHG_AUTO_TYPE;
							break;
					} 

					if(1 == GetWorkType(gunNo))
					{
						CtrData[0] = 1;//启动
			      CtrData[1] = gunNo;//枪号
						PtrStartPara->vailfg = 1;
						PtrStartPara->gunNo = gunNo;
						PtrStartPara->startby = START_BY_BKGROUND;
						SendMsgWithNByte(MSG_START_STOP,2,&CtrData[0],conCtrlPrio[gunNo]);
						g_BackCtrl.Runflag[gunNo] = 1;
						Delay5Ms(100);
						return ON_SUCC;
					}
					else{
						CtrData[0] = gunNo;//枪号
			      CtrData[1] = CARD_NOTIDLE_ERR;
						SendMsgWithNByte(MSG_VERIFY_RESULT,2,&CtrData[0],APP_TASK_SCREEN_PRIO);
					}
//					PtrStartPara->vailfg = 0;
					return ON_FAIL;			  
		}
	}
	else if(0x02 == CtrlType)
	{/* 结束充电 */
  	CtrData[0] = 0;//停止
		CtrData[1] = gunNo;//枪号
		CtrData[2] = 1;//0=按键  1=后台
		SendMsgWithNByte(MSG_START_STOP,3,&CtrData[0],conCtrlPrio[gunNo]);	 
		Delay5Ms(2);
		return OFF_SUCC;
	}
	else{
		CtrData[0] = gunNo;//枪号
	  CtrData[1] = CARD_ILLEGAL_ERR;
		SendMsgWithNByte(MSG_VERIFY_RESULT,2,&CtrData[0],APP_TASK_SCREEN_PRIO);
	}
	return  OTHER_ERR;
}

/*主机启动停止设置*/
void CheDian_SetOnOff(uint8 gunNo,uint8 dir)
{
	uint8 temp;
	
	if(gunNo >= DEF_MAX_GUN_NO)return ;
	
	if(RcvFlag == dir){    //接收帧处理				
	  /*启停命令处理函数*/
		temp = Deal_SetOnOff(gunNo);
		if( OTHER_ERR != temp ) {
			CheDian_AckOnOffSend(gunNo,temp);
//			CheDian_UpDataAuto(gunNo,SendEN);
		}		
  }
}

/*******************************************************************
处理主机对上传卡信息处理应答信息
CtrData[0]保留
CtrData[1]卡状态 
0:正常；1：充电中；2：余额不足； 3：挂失；4：无效卡;5:其他
********************************************************************/
void Deal_AckCardno(uint8 gunNo)
{
		START_PARAM  *PtrStartPara = ChgData_GetStartParaPtr(gunNo);
	  CHARGE_TYPE  *BgRunData = ChgData_GetRunDataPtr(gunNo);

		uint8 Cardstate,CtrData[2],GetCardNo[20]; 
		uint16 index = DataStartAddr;
    	
	    PtrStartPara->startby = START_BY_ONLINECARD; /*刷卡*/	
			//卡 号
	    memset(GetCardNo,0,sizeof(GetCardNo));
			Get_BcdCardNo(GetCardNo,&BackRun.Data[index]);
	    
			index += 20;
			
			//卡余额
			PtrStartPara->money = Common_LittleToInt(&BackRun.Data[index]);
			index += 4;
			//卡状态
			Cardstate = BackRun.Data[index];//BackRun.Data[index+24]
			index += 1;
			//后台交易流水号
			memset(PtrStartPara->serilNo,0,32);
			memcpy(PtrStartPara->serilNo,&BackRun.Data[index],4);
			index += 4;
		 if((0 == Cardstate)&&(memcmp(&GetCardNo,PtrStartPara->account,20)==0)\
			 &&(1 == GetWorkType(gunNo))){  //卡状态  可以充电
					CtrData[0] = 1;		
				  CtrData[1] = gunNo;//枪号
          g_BackCtrl.Runflag[gunNo] = 1;			
          PtrStartPara->vailfg = 1;		
          PtrStartPara->gunNo = gunNo;			 
          SendMsgWithNByte(MSG_START_STOP,2,&CtrData[0],conCtrlPrio[gunNo]);	
		}
		else if(3 < Cardstate){
			CtrData[0] = gunNo;//枪号
	    CtrData[1] = CARD_ILLEGAL_ERR;
		  SendMsgWithNByte(MSG_VERIFY_RESULT,2,&CtrData[0],APP_TASK_SCREEN_PRIO);
		}
		else if(1 != GetWorkType(gunNo)){
			CtrData[0] = gunNo;//枪号
			CtrData[1] = CARD_NOTIDLE_ERR;
		  SendMsgWithNByte(MSG_VERIFY_RESULT,2,&CtrData[0],APP_TASK_SCREEN_PRIO);
		}
}
/*******************************************************************
处理主机对上传卡信息处理应答信息
CtrData[0]保留
CtrData[1]卡状态 
0:正常；1：充电中；2：余额不足； 3：挂失；4：无效卡;5:其他
********************************************************************/
void Deal_AckVIN(uint8 gunNo)
{
		START_PARAM  *PtrStartPara = ChgData_GetStartParaPtr(gunNo);
	  CHARGE_TYPE  *BgRunData = ChgData_GetRunDataPtr(gunNo);

		uint8 Cardstate,CtrData[2],GetCardNo[20]; 
		uint16 index = DataStartAddr;
    	
	    PtrStartPara->startby = START_BY_VIN; /*刷卡*/	
			//卡 号
	    memset(GetCardNo,0,sizeof(GetCardNo));
			memcpy(GetCardNo,&BackRun.Data[index],17);
	    
			index += 20;
			
			//卡余额
			PtrStartPara->money = Common_LittleToInt(&BackRun.Data[index]);
			index += 4;
			//卡状态   	CARD_PASSWD_ERR = 1,  //读卡错误,请重试！	CARD_ILLEGAL_ERR =2,  //不可识别卡 	CARD_NOMONEY_ERR = 3, //金额不足 	CARD_NOTIDLE_ERR = 4, //不是非空闲 	CARD_VIN_NO_MATCH = 5,  //VIN码不匹配  patli 20190926
			Cardstate = BackRun.Data[index];//BackRun.Data[index+24] 
			index += 1;
			//后台交易流水号
			memset(PtrStartPara->serilNo,0,32);
			memcpy(PtrStartPara->serilNo,&BackRun.Data[index],4);
			index += 4;

	 if((0 == Cardstate)&&(memcmp(&GetCardNo,PtrStartPara->vincode,17)==0)\
			 &&(STEP_IDEL == BgRunData->logic->workstep)){  //卡状态  可以充电
	//	 if((0 == Cardstate)&&(memcmp(&GetCardNo,PtrStartPara->vincode,17)==0)){  //卡状态  可以充电
			
				  CtrData[0] = 1;		
				  CtrData[1] = gunNo;//枪号
          g_BackCtrl.Runflag[gunNo] = 1;			
          PtrStartPara->vailfg = 1;		
          PtrStartPara->gunNo = gunNo;	
          PtrStartPara->vinback =2;				 
          SendMsgWithNByte(MSG_START_STOP,2,&CtrData[0],conCtrlPrio[gunNo]);	
		}
		else if(3 < Cardstate){
			CtrData[0] = gunNo;//枪号
	    CtrData[1] = CARD_ILLEGAL_ERR;
		  SendMsgWithNByte(MSG_VERIFY_RESULT,2,&CtrData[0],APP_TASK_SCREEN_PRIO);
		}
		else if(1 != GetWorkType(gunNo)){
			CtrData[0] = gunNo;//枪号
			CtrData[1] = CARD_NOTIDLE_ERR;
		  SendMsgWithNByte(MSG_VERIFY_RESULT,2,&CtrData[0],APP_TASK_SCREEN_PRIO);
		}
}
/*****************************************
上传刷卡信息
待处理：可能不用或从主控发送上传命令
*****************************************/
void CheDian_CardnoAuto(uint8 gunNo,uint8 dir)
{
	uint16 index = DataStartAddr;
	
	if(gunNo >= DEF_MAX_GUN_NO)return ;
	
	START_PARAM  *PtrStartPara = ChgData_GetStartParaPtr(gunNo);
	CARD_INFO *PtrCard = TskCard_GetCardInfPtr();	
	CHARGE_TYPE  *BgRunData = ChgData_GetRunDataPtr(gunNo);
		
	if(SendEN == dir){           //发送帧处理
	
    //卡号			
		memset(&BackRun.Data[index], 0, 20); 
		if(START_BY_VIN != PtrStartPara->startby){
			UpCardNoASCII(&BackRun.Data[index],PtrCard->cardNo);
			memcpy(PtrStartPara->account, PtrCard->cardNo, 20); 
		}
		else{
			memcpy(PtrStartPara->vincode, BgRunData->bms->car.brm.vin, sizeof(PtrStartPara->vincode));
			Dealadd_VIN(PtrStartPara->vincode,17);
			memcpy(&BackRun.Data[index], PtrStartPara->vincode, sizeof(PtrStartPara->vincode)); 
		}
		
		index += 20;		
		CheDin_FrameSend(AutoFrame, AUTO_CARDNO, conSoure[gunNo], index);			
	}
	else if(RcvFlag == dir){    //接收帧处理
		if(START_BY_VIN != PtrStartPara->startby){
      Deal_AckCardno(gunNo);	
		}
		else{
			Deal_AckVIN(gunNo);	
		}
  }
}
/*************************
发送启动 停止响应报文
后台以充电状态判断，直接上传此状态
**************************/
void CheDian_AckOnOffSend(uint8 gunNo,uint8 onoff )
{	 
  uint16 index = DataStartAddr;
	
	BackRun.Data[index++] = onoff;
	CheDin_FrameSend(AutoFrame, SET_ONOFF, conSoure[gunNo], index);	
}
/*******************************
主动枪状态
A枪：0
B枪：1
待处理：和0x04  与0x09命令重复
*******************************/
void CheDian_UpDataAuto(uint8 gunNo,uint8 dir)
{
	uint16 rtn,Item;
	uint16 index = DataStartAddr;
	uint16 sendunit = 0;
	
	if(gunNo >= DEF_MAX_GUN_NO)return ;
	
	if(SendEN == dir){           //发送帧处理
	    
		memset(&BackRun.Data[0],0,CheDian_MAX);
		index +=2;
		Item = 0x0B0B;
		rtn =  GetUpData_ItemData(gunNo,Item,&BackRun.Data[index]);
		if( rtn > 0 ) {
			index += rtn;
			sendunit++;
		}
		
		Item = 0x0B01;
		rtn =  GetUpData_ItemData(gunNo,Item,&BackRun.Data[index]);
		if( rtn > 0 ) {
			index += rtn;
			sendunit++;
		}
		BackRun.UpDataTime[gunNo] = GetSystemTick();		
		Common_LittleShortTo(&BackRun.Data[DataStartAddr],sendunit);		
		CheDin_FrameSend(AutoFrame, GET_CHGDATA, conSoure[gunNo], index);	
	}
}
/***********************************************************************
***函数名称: CheDianfun
** 功能描述: 通过命令查找函数执行
***    参数: 
**   返回值:   
*************************************************************************/
uint8  CheDian_fun_Deal(uint8 cmd,uint8 gunNo,uint8 dir)
{
	 uint16 i;
	 const CheDian_t *CheDian = &conCheDian[0];
	 
	 for( i = 0 ; CheDian->cmd != CMD_NULL ; i++ ) {
		 if( cmd == CheDian->cmd ) {
			 if( NULL != CheDian->func ) {
			   CheDian->func(gunNo,dir);
				 break;
			 }
			 return  FALSE;
		 }
		 CheDian++;
	 }
	 return TRUE;
}

/************************************ 
获取需要主动上传命令
CtrData[0] 命令类型
CtrData[1] 枪号
************************************/
uint8 CheDian_GetAuto(uint8 *CtrData)
{		
	PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	
	 static uint8 sGunlink[DEF_MAX_GUN_NO];  // 0: not  1:link
	 static uint32 sWaitTicks[DEF_MAX_GUN_NO] = { 0 };
	 static uint32 sGuntype[DEF_MAX_GUN_NO] = { 0 };
   uint8 i;
	
   CtrData[1]  = 0;
	 if( 0xAA == BackRun.Commforbidden ){  //登入退出，不执行	或以有其他任务	 
		 if( GetSystemTick() < sWaitTicks[0] + TIM_NS(10) ) {
			  sWaitTicks[0] = GetSystemTick();
			  CtrData[0] = AUTO_LOGIN;
				return TRUE;	
		 }
		 return FALSE;
	 }
	 
//	 for(i=0;i<DEF_MAX_GUN_NO;i++){
//		if(sGuntype[i] != GetWorkType(i)){		
//			CtrData[0] = AUTO_UPDATA;	
//      CtrData[1] = i;
//			sGuntype[i] = GetWorkType(i);
//	
//			return TRUE;
//		}
//  }
  for(i=0;i<devparaPtr->gunnum;i++){
		CHARGE_TYPE  *ptrBgRunData = ChgData_GetRunDataPtr(i);
	  if(( sGunlink[i] != ptrBgRunData->gun->statu.bits.cc1stu )||(sGuntype[i] != GetWorkType(i))){
		    sGunlink[i] = ptrBgRunData->gun->statu.bits.cc1stu;
			  sGuntype[i] = GetWorkType(i);
			  CtrData[0] = AUTO_UPDATA;	
        CtrData[1] = i;				
		    return TRUE;
	  }
  }	
	for(i=0;i<devparaPtr->gunnum;i++){  //DEF_MAX_GUN_NO
		if(GetSystemTick() > BackRun.UpDataTime[i] + TIM_NS(15)){		
			CtrData[0] = AUTO_UPDATA;	   //超过30秒没发APP会出现结算提示，后台会根据09命令判断离线
      CtrData[1] = i;	
			
			return TRUE;
		}
  }
	
//	if( GetSystemTick() > sWaitTicks[1] + TIM_NS(50)) {
//		 sWaitTicks[1] = GetSystemTick();
//		 CtrData[0] = AUTO_UPDATA;
//		 CtrData[1] = GetSystemTick()%2;
//     return TRUE;		
//	}
	 


	/* 主动定时上传记录 */
	if((TskMain_GetWorkStep(AGUN_NO) != STEP_CHGEND ) &&(TskMain_GetWorkStep(BGUN_NO) != STEP_CHGEND )\
		&&(GetSystemTick()  > BackRun.RecordTime + TIM_NS(60) ) ) \
	{		
			 BackRun.RecordTime = GetSystemTick();
			 CtrData[0] = AUTO_RECORD;	
       return TRUE;		
	}
		
	/* 定时上传心跳包 */
	if(GetSystemTick() >  BackRun.LifeTime + TIM_NS(10) )
	{
		  BackRun.LifeTime = GetSystemTick();
			CtrData[0] = AUTO_LIFE;	
		  return TRUE;
	}
	return FALSE;
}

/************************************ 
将车电网协议数据清零初始化
************************************/
void CheDian_Init(void)
{
	memset(&BackRun,0,sizeof(BackRun));
//	BackRun.RecordAutoEN = 1;
}

/************************************ 
获取其他任务发送过来的信息或命令
************************************/
/* 处理后台消息 */
void CheDian_PreBackMsg(MSG_STRUCT *msg)
{
	uint8 u8GunNo;
	uint8 CtrData[4];
	
	switch(msg->MsgType)
	{
		case MSG_ONLINE_CARD:/* 请求获取IC卡帐号信息 */
		{	
			u8GunNo = msg->MsgData[0] % DEF_MAX_GUN_NO;
//      if(1 != GetWorkType(msg->MsgData[0])){
//		    CtrData[0] = msg->MsgData[0];//枪号
//			  CtrData[1] = CARD_NOTIDLE_ERR;
//		    SendMsgWithNByte(MSG_VERIFY_RESULT,2,&CtrData[0],APP_TASK_SCREEN_PRIO);
//	    }
//      else{		
			  CheDian_CardnoAuto(u8GunNo,SendEN);
//			}
			break;
		}
		case MSG_VIN_CARD:/* 请求获取IC卡帐号信息 */
		{	
			u8GunNo = msg->MsgData[0] % DEF_MAX_GUN_NO;	
			CheDian_CardnoAuto(u8GunNo,SendEN);
			break;
		}
		case MSG_UP_RECORD:/* 请求上传当前充电记录 */
		{	
      u8GunNo = msg->MsgData[0] % DEF_MAX_GUN_NO;			
      BackRun.RecordMsg = 1;
			BackRun.RecordTime = 0;			
			BackRun.RecordTime =  GetSystemTick();
			CheDian_RecordAuto(u8GunNo,SendEN);
			Delay10Ms(5);
			g_BackCtrl.Runflag[u8GunNo] = 0;
//			CheDian_UpDataAuto(u8GunNo,SendEN);			
			break;
		}
		case MSG_START_STOP_RESULT:
		{
			u8GunNo = msg->MsgData[2] % DEF_MAX_GUN_NO;	
		  CheDian_UpDataAuto(u8GunNo,SendEN);
			Delay10Ms(5);
			break;
		}
#ifdef AN_ETH_TASK
		case MSG_TCPCLIENT_DATA_TO_BACK:
		{
			BackRun.ptr_msg = msg->PtrMsg;
			BackRun.msg_len = msg->MsgLenth;
			BackRun.msg_readed_num = 0;
			//读取主机下发数据并进行处理		  
			if(CheDian_RcvUploadDataDeal(CtrData) == TRUE) {
					  CheDian_fun_Deal(CtrData[0],CtrData[1],RcvFlag);								  //处理信息
					  g_BackCtrl.errCount = 0;
					  g_BackCtrl.CommStatus = CONNECT_OK;			  /*0 通讯正常	1：故障*/
					  TskBack_CommStatusSet(CONNECT_OK);
					  g_BackCtrl.CommTimers = GetSystemTick();	  
			Monitor_NetConnect(0);	  
			Delay10Ms(1);			  
			}
			
			break;
		}
#endif		
		default:
			break;
	}
}

uint8 CheDian_check_START_STOP(void)
{
	uint8 i;
  static uint8 sGuntype[DEF_MAX_GUN_NO];
	
	for(i=0;i<DEF_MAX_GUN_NO;i++){
		if(sGuntype[i] != GetWorkType(i)){		
			CheDian_UpDataAuto(i,SendEN);
			sGuntype[i] = GetWorkType(i);
		}
  }
	return 0;
}

extern void SoftReset(void);
uint8 Monitor_NetConnect(uint8 linkfg)
{
	  PARAM_DEV_TYPE *ptrDevPara = ChgData_GetDevParamPtr();
	  struct tm  tmlocal = Time_GetSystemCalendarTime();
	  static uint8 fg = 0;
	  static uint32 sResetMcuTicks = 0;
	  PARAM_COMM_TYPE *ptrBackCOMM = ChgData_GetCommParaPtr();
	
	  if( 0 == linkfg ) {
			fg = 0;
			sResetMcuTicks = GetSystemTick();
			return 0;
		}
	
	  if( ptrDevPara->onlinecard != E_ONLINE_CARD ) {
			return 0;
		}
	  
//		if( ptrBackCOMM->conntype != CONN_ETH ) {
//			return 0;
//		}
		
		if( TskMain_GetWorkStep(AGUN_NO) != STEP_IDEL  \
			&& TskMain_GetWorkStep(BGUN_NO) != STEP_IDEL ) {
			sResetMcuTicks = GetSystemTick();
			return 0;
		}
		
		if( 0 == fg ) {
			fg = 1;
			sResetMcuTicks = GetSystemTick();
		}
		
		if( ( 1 == fg ) && ( GetSystemTick() > sResetMcuTicks + TIM_NMIN(15) ) ) {
			if( tmlocal.tm_hour == 18 ) {
				 SoftReset();
			}else {
				 fg = 0;
			}
			
		}
		
		return 0;
}


/************************************ 
车电网运行
************************************/
void CheDian_Run(void)
{
	uint8 CtrData[4];
	PARAM_COMM_TYPE *ptrBackCOMM = ChgData_GetCommParaPtr();
	static uint32 CheDian_delay = 0,CheDian_ticks = 0;
	static uint8 sDelayT = 3;
//	uint32 getruntime;
	/* 获取运行时间*/
	  if(GetSystemTick() > CheDian_ticks + TIM_NS(5)){
			CheDian_delay++;
		}
	  CheDian_ticks = GetSystemTick();
	  
		if(g_BackCtrl.CommStatus  == CONNECT_OFF)
		{				
				if(GetSystemTick() > CheDian_delay + TIM_NS(sDelayT))//5
				{
						CheDian_fun_Deal(AUTO_LOGIN,0,SendEN);							//处理信息,发送登入
					  CheDian_delay = GetSystemTick();
						sDelayT = ( g_BackCtrl.errCount++ < 20 ) ? 3 : 8;
				}
				
		}
		else if( CheDian_GetAuto(CtrData) > 0 ) {      	// 获取主动上传命令			  
				CheDian_fun_Deal(CtrData[0],CtrData[1],SendEN);  				    	//主动上发数据
			  Delay10Ms(5);   //延长时间，否则两个数据包间隔太短，通过4G模块上发变成一个数据包

		}						

#ifndef AN_ETH_TASK
      //读取主机下发数据并进行处理			
			if(CheDian_RcvUploadDataDeal(CtrData) == TRUE) {
 					CheDian_fun_Deal(CtrData[0],CtrData[1],RcvFlag);  								//处理信息
					g_BackCtrl.errCount = 0;
					g_BackCtrl.CommStatus = CONNECT_OK;             /*0 通讯正常  1：故障*/
				  TskBack_CommStatusSet(CONNECT_OK);
					g_BackCtrl.CommTimers = GetSystemTick();	
          Monitor_NetConnect(0);	
          Delay10Ms(1);				
			}
			else {
#endif				
				/* 后台通信重新连接*/
				if(GetSystemTick() > g_BackCtrl.CommTimers + TIM_NMIN(1))
				{
					 
					 g_BackCtrl.errCount = 0;
					 g_BackCtrl.CommStatus = CONNECT_OFF;
					 g_BackCtrl.CommTimers = GetSystemTick();
					 
					 /*系统复位,必须放置在通讯未连接10分钟后执行*/
//					 Monitor_NetConnect(1);
				}
//				if(GetSystemTick() - g_BackCtrl.CommTimers > TIM_NS(15)){
				if( (BackRun.RcvbeatCnt > 1 ) && (BackRun.RcvbeatCnt % 3 == 0) \
					  &&(GetSystemTick() > g_BackCtrl.CommTimers + TIM_NS(8))) {
							
					  g_BackCtrl.CommTimers = GetSystemTick();
					  CheDian_fun_Deal(AUTO_LOGIN,0,SendEN);
//patli 20200128					  TskBack_CommStatusSet(CONNECT_OFF);
					  Delay10Ms(10);		
				}
				
				if( (g_BackCtrl.CommStatus == CONNECT_OK) && (BackRun.RcvbeatCnt > 10) ) {                       //10次心跳没回应
					 BackRun.RcvbeatCnt = 0;
					 g_BackCtrl.CommStatus = CONNECT_OFF;
					 TskBack_CommStatusSet(CONNECT_OFF);
					 CheDian_delay = 0;								//立即发送登入
					 g_BackCtrl.CommTimers = GetSystemTick() - TIM_NMIN(1);
					 Delay10Ms(10);
				}
#ifndef AN_ETH_TASK				
				
		 }	
#endif

}
#else
void CheDian_Init(void)
{

}

void CheDian_PreBackMsg(MSG_STRUCT *msg)
{

}

void CheDian_Run(void)
{

}

#endif




