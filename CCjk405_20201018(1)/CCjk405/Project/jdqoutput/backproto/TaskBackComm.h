/**
  ******************************************************************************
  * @file    TaskBackComm.h
  * @author  zqj
  * @version v1.0
  * @date    2017-03-09
  * @brief   
  ******************************************************************************
	*/
	
#ifndef _TASKBACKCOMM_H
#include "typedefs.h"
#include "Message.h"
#include "ChgData.h"
#include "Back_Inter.h"

//#include "NetLay.h"

#define WIFI_RESET_L   Gpio_WritiLoutputIo(OUT9,Bit_RESET)
#define WIFI_RESET_H	 Gpio_WritiLoutputIo(OUT9,Bit_SET)	
#define TEST_COM   MOD_COM
#define DEV_GUNNO   2

#define  BACK_DATA_MAX    512

/* 连接状态定义 */
typedef enum
{
	CONNECT_OK=0,			/* 连接OK */
	CONNECT_OFF,			/* 连接断开 */
}CONNECT_DEFINE;
/*后台通讯协议**/
typedef enum
{	
	CONN_CheDian = 0, /* 车电网后台 */
	CONN_Aunice,      /* 奥耐后台 */
	CONN_CHGBIRD,     /* 充之鸟后台 */    
	CONN_CHGZPLD,     /* 正平隆地后台 */
	CONN_CHCP,        /* 澄鹏后台 */
	BMS_HELI,         /* 合力协议，此协议不为后台协议，暂时放在此处，待改 */
}Back_AgreeTYPE;

#define  SUPPORT_Agree  CONN_CheDian 
/* 网络协议 */
typedef struct
{
	void 	(*InitAgree)(void);/* 初始化网络 */
	void  (*DealBackMsg)(MSG_STRUCT *msg);/* 处理接收数据*/
	void  (*RunAgree)(void);/* 任务运行 */
}Back_Agreement;	

/* 后台控制结构 */
typedef struct
{
//	uint8 BackConStatus;				/* 尝试连接状态 0:断开，发送连接信号 1:连接 ，已经发送信号了  */
	uint8 AutoSndCmdFlag;				/* 自动发送命令 */
	uint8 CmdCode;							/* 命令码 */
	uint8 CmdCnt;								/* 发送命令次数 */
	uint8 CmdRrdCnt;						/* 记录发送次数 */
	uint8 CardType;							/* 卡号类型 4：白名单 5：未知卡 */
	
	uint8  Runflag[DEV_GUNNO];					/* 后台充电标志 */
	uint8  AgreeType;					/* 通信类型 */
	uint8  InterType;					/* 通信接口*/
	uint8  CommStatus;					/* 通信状态 */
	uint8  u8ShowCommStatus;
	uint16 errCount;
	uint32 CommTimers;					/* 通信定时器 */
	
//	uint8  Stage;								/* 通讯连接阶段 */
	uint32 DealyTimers;					/* 通讯复位延时定时器 */
	uint32 SerialNo;					/* 上传记录号 */
	uint32 RecordAddr;					/* 上传记录地址 */
}BACK_CTRL_STRUCT;

typedef struct BackRunPara
{  
	 uint32  UpCardNo;
	 uint8  Data[BACK_DATA_MAX+20];   //轮流处理不用分开，帧全部信息	 
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
	 uint8* ptr_msg;
	 uint16 msg_readed_num;
	 uint16 msg_len;
}BackRun_t;

extern uint32 DevRunSec;     //设备运行多少?

/*后台参数*/
extern BACK_CTRL_STRUCT g_BackCtrl;
extern  MSG_STRUCT Backmsg;
extern  NET_DATA_INTER NetHandler;/* 底层网络句柄 */
extern  void InitTCPData(void);
extern  uint8 TskBack_CommStatus(void);
extern  uint8 TskBack_CommStatusSet(uint8 staus);
/*后台任务*/
extern void Task_BackComm(void *p_arg);

//extern int32 TskBack_ReadData(NET_DATA_INTER *pNet,uint8 *pbuf, uint16 MaxSize);

extern void Get_BcdCardNo(uint8 *CardNo,uint8 *buff);
	
extern uint8 UpCardNoASCII(uint8 *buff,uint8 *CardNo);	

extern uint8 updataTime(uint8 *puff ,uint8 tpye);

extern uint8 GetConnectType(void);

extern uint8 GetWorkStep(uint8 gunNo);

extern uint8 GetWorkStatus(uint8 gunNo);
extern int32 Back_Read(uint8 *buf,uint16 len);

	
	
	
	
	























	
	
#endif
