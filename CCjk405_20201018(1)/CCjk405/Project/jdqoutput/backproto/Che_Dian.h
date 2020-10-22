/**
  ******************************************************************************
  * @file    Che_Dian.h
  * @author  lw
  * @version v1.0
  * @date    2018-07-31
  * @brief   车电网通讯协议
  ******************************************************************************
	*/
#ifndef _Che_Dian_H_0001
#define _Che_Dian_H_0001

#include "typedefs.h"
#include "Message.h"
#include "ChgData.h"


#define CheDian_1S(n)		(OS_TICKS_PER_SEC*n) /* ??10ms??,??1S */
#define CheDian_1MIN(n)	(CheDian_1S(60)*n)/* 1min */

/***********************
0:交流单枪
4:直流单枪
5:一体双枪

************************/
#define  DEV_TYPE_S  4
#define  DEV_TYPE_D  5
#define  CheDian_MAX    255
#define  CheDian_FrameOther    16    //信息域外的其他参数个数
#define  DataStartAddr    11    //信息域初始地址
#define  CheDian_LifeTime    0x10    //S心跳间隔

#if 0

typedef struct CheDianPara
{  
	 uint32  UpCardNo;
	 uint8  Data[CheDian_MAX+20];   //轮流处理不用分开，帧全部信息	 
	 uint8  Commforbidden;  //通讯禁用 0：正常    1：禁用，登入退出
//	 uint8  RecordAutoEN;  //后台主动上传记录使能：0 不主动上传，1主动查询上传
	 uint8  RecordCnt;     //未上传后台充电记录总数
	 uint8  RecordMsg;
	 uint8  RcvbeatCnt;   /*心跳包计数*/
//	 uint32  Recordno;     //未上传后台充电记录号
	 uint16 Len;           /*帧总长数据长度*/
	
	 uint32 LifeTime;
	 uint32 RecordTime;
	 uint32 ErrTime;
	 uint32 UpDataTime[DEF_MAX_GUN_NO];
	 uint32 CardnoTime;
	 uint32 RunTimeSec;
}CheDianRun_t;

#endif

typedef union  
{
	uint32 LongData;
	uint8  ByteData[4];
}U32Data;

typedef union  
{
	uint16 ShortData;
	uint8  ByteData[2];
}U16Data;

typedef struct CheDian
{
	uint8   cmd;                 /*命令*/
//	uint16  Writeaddr;                 /*参数写入开始地址*/
//	uint16  Readaddr;                 /*参数读出开始地址*/
	void  (*func)(uint8 GunNo,uint8 dir);   /*功能 GunNo枪号0：全局，1：A枪 2：B枪 dir收发方向*/
}CheDian_t;

typedef enum
{
	Nothing   = 0,  
	RcvFlag   = 1, 
	SendEN    = 2,
	
}CheDian_busy;

typedef enum
{
	CheDian_GUN0   = 0,  /*整个设备状态*/
	CheDian_GUNA   = 1, 
	CheDian_GUNB    = 2,
	
}CheDian_DEV;
typedef enum
{
	ON_SUCC   = 0,  
	ON_FAIL   = 1, 
	OFF_SUCC   = 2,  
	OFF_FAIL   = 3,
	
	OTHER_ERR = 0xff,
	
}CheDian_OnOff;

typedef enum
{
	RequestFrame    = 0,  
	ResponseFrame   = 1, 
	AutoFrame       = 2,
	
}CheDian_FrameType;

typedef enum
{
	ServerFrame   = 0x00,  
	CLIENTO   = 0x10,    /*设备全局*/
	CLIENTA   = 0x11,    /*设备A枪*/
	CLIENTB  = 0x12,     /*设备B枪*/
	
}CheDian_FrameSoure;

typedef enum
{
	AUTO_LIFE     = 0x01,  /*心跳*/
	AUTO_LOGIN    = 0x02,  /*登入*/
	AUTO_QUIT     = 0x03,/*退出登入*/
	GET_DEVPARA   = 0x04,  /*获取终端数据*/
	GET_CHGDATA   = 0x09,  /*获取实时数据*/
	SET_PARA      = 0x05,  /*设置参数*/
	AUTO_RECORD   = 0x06,    /*发送历史记录*/ 
	AUTO_ERR      = 0x07,  /*获取告警数据*/
	AUTO_UPDATA   = 0x14,  /*自动上报实时数据*/
	SET_TIME      = 0x08,  /*对时*/
	SET_ONOFF     = 0x10,  /*启停命令*/
	AUTO_CARDNO   = 0x30,/*发送卡信息*/
	
}CheDian_ClientCmd;

//extern CheDianRun_t  CheDianRun;

//uint8  CheDian_RcvUploadDataDeal(uint8 *buff);
//uint8  CheDian_fun_Deal(uint8 cmd,uint8 GunNo,uint8 dir);
//void GetDevicRunTime(void);
uint8 CheDian_GetAuto(uint8 *CtrData);
void CheDian_Init(void);
void CheDian_PreBackMsg(MSG_STRUCT *msg);
void CheDian_Run(void);
//BackRun_t *GetCheDianRun(void);
uint8 Monitor_NetConnect(uint8 linkfg);

#endif


