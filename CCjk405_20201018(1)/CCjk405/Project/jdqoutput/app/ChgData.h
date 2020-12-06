/**
  ******************************************************************************
  * @file    chgdata.h
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
*/
#ifndef __CHGDATA_H_0001
#define __CHGDATA_H_0001

#include "TaskIsoCheck.h"
#include "RelayOut.h"
#include "TaskRealCheck.h"
#include "Bms.h"
#include "TaskDcMeter.h"
#include "BillingUnit.h"
#include "TaskCardReader.h"
#include "TaskAcMeter.h"
#include "CdModel.h"

#define MAIN_ADDR 0xf8

#define DEF_MAX_GUN_NO   2    /*定义枪数量*/

#define MODULE_MINVOL 400    //模块的最小开启电压 
#define MODULE_MAXVOL 4000   //patli 20201018 1100   //模块的最大开启电压

typedef union
{
	uint32 dword;
	uint16 word[2];
	uint8  byte[4];
	struct {
		uint32 modErr   : 1; //0
		uint32 bit1     : 1; //1
		uint32 bit2     : 1; //2
		uint32 bit3     : 1; //3
		uint32 bit4     : 1; //4
		uint32 bit5     : 1; //5
		uint32 bit6     : 1; //6
		uint32 bit7     : 1; //7  cc1
		uint32 bit8     : 1; //8
		uint32 bit9     : 1; //9
		uint32 bit10    : 1; //10
		uint32 bit11    : 1; //11
		uint32 bit12    : 1; //12
		uint32 bit13    : 1; //13
		uint32 bit14    : 1; //14
		uint32 flasherr : 1; //15 存储出错
		
		uint32 posFU    : 1;//16正熔芯
		uint32 negFU    : 1;//17负熔芯
		uint32 emcystop : 1; //18急停
		uint32 acKm     : 1; //19交流接触器状态
		uint32 gunCC1   : 2; //20  0~3
		uint32 spd      : 1;  //22 防雷器状态
		uint32 elockstu : 1; //23  

		uint32 rsv22    : 1; //24
		uint32 rsv23    : 1; //25
		uint32 rsv24    : 1; //26
		uint32 rsv25    : 1; //27
		uint32 rsv26    : 1; //28
		uint32 rsv2     : 1; //29
		uint32 rsv3     : 1; //30
		uint32 rsv5     : 1; //31
		
	}bits;
}START_STU_ST;

typedef enum
{
	CHG_AUTO_TYPE   = 1,
	CHG_TIME_TYPE   = 2,
	CHG_MONEY_TYPE  = 3,
	CHG_ENERGY_TYPE = 4,
	
}E_CHG_NTYEP;


//设备运行状态与参数
typedef struct {
	
	uint8  workstep;     /*充电阶段*/
	uint8  startby;      /*启动方式：0 刷卡 1 后台 2 VIN 3 插枪即充 4 按键*/
	uint8  chgmod;       /*充电方式 1：自动 2：按时间 3：按金额 4：按电量*/
	uint8  stopReason;   /*结束原因*/
	uint8  errCode;      /*故障码*/
	uint8  gunInUsed;    /*充电枪使用中*/
	uint32 settime;
	int32  setmoney;
	uint32 setenergy;

	uint32 time;
	int32  money;
	uint32 energy;
	uint8  account[20];
	uint8  usrpin[3];
	uint8  startfg;      /*启动成功，进入充电标志*/
	uint8  recsavefg;    /*记录存储标志*/
	START_STU_ST runstu; /*充电桩运行状态*/
	uint32 chgtimes;     /*充电桩本次上电充电次数*/
	uint32 u32TranPageTicks;  /*充电结束等待跳转时间*/
}DEV_LOGIC_TYPE;

typedef enum
{
	START_BY_CARD = 0,
	START_BY_BKGROUND = 1,
	START_BY_ONLINECARD = 2,
	START_BY_VIN  = 3,
	START_BY_GUN  = 4, //
	START_BY_KEY  = 5,

	START_BY_PASSWD = 6,  //账户密码登入
	
}ESTART_BY_WAY;

/* 后台连接类型定义  */
typedef enum
{
	CONN_WIFI=0,		
	CONN_4G,
  CONN_ETH,	
	CONN_2G,	
	CONN_GPRS,
}CONNECT_TYPE;
typedef enum
{
	PARA_SCREEN = 1,
	PARA_BKGROUND = 2,
}ESTARTPARAM_BY;

//typedef enum
//{
//	CHARGE_WAY_AUTO = 1,
//	CHARGE_WAY_TIME = 2,
//	CHARGE_WAY_MONEY = 3,
//	CHARGE_WAY_ENERGY = 4,
//}ECHARGE_WAY;






typedef struct
{
	DEV_ISO_TYPE *iso;            /*接触器内侧电压 VDC1 接触器外侧电压VDC3*/
	DEV_METER_TYPE *meter;        /**/
	DEV_GUN_TYPE *gun;
	DEV_BMS_TYPE *bms;            /**/
	DEV_RELAY_TYPE *relay;
	DEV_INPUT_TYPE *input;
	DEV_LOGIC_TYPE *logic;
	START_PARAM *startparam;
	CALCBILL_DATA_ST *bill;
	CDMOD_DATA *dlmod;
	DEV_ACMETER_TYPE *AcMet;
}CHARGE_TYPE;

#pragma pack(1)
typedef  struct
{
    uint16 LocalSocket;            /*本机端口*/
    int8   Ssid[32];               /*路由器AP*/
    int8   Password[32];           /*路由器AP密码*/
    uint8  SecuEn;                 /*AP是否加密*/
    uint8  DhcpEn;                 /*DHCP是否使能*/
    uint8  DnsSever1[4];           /*DNS服务器地址1*/
    uint8  DnsSever2[4];           /*DNS服务器地址2*/   
}ETH_STA_PARA;

typedef  struct
{
	uint8 LocalIpAddr[4];            /*本地IP地址*/
	uint8 RemoteIpAddr[4];           /*远程IP地址*/
	uint16 RemotePort;               /*远程端口号*/
	uint8 NetMask[4];                /*子网掩码*/
	uint8 NetGate[4];                /*网关*/
	uint8 MacAdress[6];              /*mac地址*/
	ETH_STA_PARA StaPara;            /*wifi模块参数*/
}ETH_PARAM_T; 

typedef  struct
{
	uint8 conntype;                /*后台方式*/
	uint8 agreetype;                /*后台通讯协议*/
	ETH_PARAM_T  netpara;        /*网络参数*/
}PARAM_COMM_TYPE; /*通讯参数*/


typedef  union
{
	uint32 system;
	uint8  byte[4];
	 struct {
		uint32 eleclock      :  1;    //0 电子锁检测
		uint32 eleclocktype  :  1;    //1 电子锁类型
		uint32 eleclockback  :  1;    //2 电子锁反馈
		uint32 curfewalarm   :  1;    //3 门禁告警
		uint32 curfewsignal  :  1;    //4 门禁信号
		uint32 opencharge    :  1;    //5 开门停充
		uint32 metertype     :  1;    //6 电表类型
		uint32 meteralarm    :  1;    //7 电表告警
		uint32 chargemode    :  2;    //8 充电模式
		uint32 BMSpower      :  1;    //10 BMS电源
		uint32 prevraysignl  :  1;    //11 防雷信号
		uint32 devicetype    :  1;    //12 刷卡机类型
		uint32 devicealarm   :  1;    //13 刷卡机告警
		uint32 batteryalarm  :  1;    //14 电池反接告警
		uint32 fanalarm      :  1;    //15 风机告警
		uint32 outputcheck   :  1;    //16 输出校验
		uint32 EPOsignal     :  1;    //17 EPO信号
		uint32 insulalarm    :  1;    //18 绝缘告警
		uint32 fandrive      :  1;    //19 风机驱动
		uint32 systemtype    :  2;    //20系统类型  1 单枪模式  2双枪模式
		uint32 clearecord    :  1;    //22 清除记录
		uint32 eleunlock     :  1;    //23电子锁解锁，为1时充完电不管是否结算，都会自动解锁
		uint32 powersplitt   :  1;    //24 功率分配 0 不开启分配  1：开启分配
		uint32 rsv           :  7;    // 保留
		
	}bits;
}SYSTEM_ALARM;  //add tss 20180814


/*共 52字节 */
typedef  struct  
{
	uint16 head;       /*存储时使用*/
	uint16 maxvolt;    /*最大输出电压 750*/
	uint16 minvolt;    /*最小输出电压*/
	uint16 maxcurr;    /*最大输出电流*/
	uint16 mincurr;    /*最小输出电流*/
	uint16 overcurr;   /*单枪最大电流过流值*/
	uint16 overdcvolt;
	uint16 overdccurr;
	uint16 underdcvolt;
	
	uint8  envmaxtemper; /*环境过温值*/
	uint8  gunmaxtemper; /*充电枪过温值*/
	uint8  modnum;       /*模块总数量*/
	uint8  grpmodnum[2];   /*一组模块数量  20181130*/
	uint16 singmodcurr;  /*单模块最大电流*/
	
	uint16 overacvolt;  /*交流过压值*/
	uint16 underacvolt; /*交流欠压值*/
	
	uint16 isoresist;   /*绝缘电阻告警值*/
	
	uint8 elockallow;   /*电子锁故障屏蔽 0:检测电子锁故障 1：不检测电子锁故障*/
	uint8 elocktm;      /*s*/
	uint8 bhmtm;
	uint8 xftm;
	uint8 brmtm;
	uint8 bcptm;
	uint8 brotm;
	uint8 bcltm;
	uint8 bcstm;
	uint8 bsttm;
	uint8 bsdtm;
	
	uint8 gunnum;
	

	uint8 Fanstartemper;    //风机启转温度   add tss 20180814
	uint8 Fanstoptemper;    //风机停止温度
	uint8 Fanturnstemper;   //风机转满温度
	uint8 Weavercoeffic;    //并机系数
	uint8 WalkIntime;       //WalkIn时间

	
	SYSTEM_ALARM  Sysparalarm;
	
//	uint8 SysPincod[6];    //参数设置密码
	
	/************必须在最后位置**********************************/
 	uint16 crc;      /*存储时使用 必须放在结构的最后*/
	
}PARAM_OPER_TYPE;  /*业务参数 60byte*/

/*注意：添加不能超过300字节 */
typedef struct
{
	uint16 head;        /*存储时使用*/
	uint16 dcmetbaud;   /*电能表波特率*/
	uint16 dcmetAdr;    /*直流表地址*/
	uint8  chargeAddr;  /*充电桩通讯地址*/
	uint8  chargeId[16]; /*桩ID*/
	uint8  onlinecard;  /*在线卡1 离线卡 0 刷卡方式*/
	uint8  musrpasswd[8];/*管理员密码*/
	uint8  telephone[16]; /*联系电话*/
	uint8  safeclass;    /*系统安全级别*/
	uint8  maxmodnum;     /*最大模块数量*/
  CARD_PARAM_ST card; /*卡参数*/
	PARAM_COMM_TYPE  bkcomm; /*后台通讯参数*/
	uint16 crc;         /*存储时使用*/
}PARAM_DEV_TYPE;      /*充电桩设备类参数 168*/

/*共464字节*/
typedef struct
{
	uint16 head;                                   /*存储时使用*/
	SECT_PRICE_ST SectTimeFee;  
	uint16 crc;                                    /*存储时使用*/
}PARAM_FEE_TYPE;  /*费率参数 共464字节 */

typedef struct SetFeiLv
{
	 uint16 FeiLv;   //费率
	 uint8 startH;
	 uint8 startM;
	 uint8 stopH;
	 uint8 stopM;
}SetFeiLv_t;

#ifdef CP_PERIOD_FEE
typedef struct
{
	uint16 head; 
	uint8  num;   //阶段数
	SetFeiLv_t BackFeiLv[MAXSFEILVNUMS];
	uint16 crc; 
	
}PARAM_CP_PERIOD_FEE_TYPE;  // 占用2+1+6*24+2 = 148字节
#endif

#pragma pack()


//PARAM_DEV_TYPE    设备参数类型
//PARAM_FEE_TYPE    费率参数类型
//PARAM_OPER_TYPE   运行参数类型

typedef enum
{
	SAVE_ALL_TYPE = 0,
	SAVE_OPER_TYPE = 1,
	SAVE_DEV_TYPE  = 2,
  SAVE_FEE_TYPE = 3,
  #ifdef CP_PERIOD_FEE
  SAVE_CP_PERIOD_FEE_TYPE = 4,
  #endif
}E_SAVETYPE;

typedef enum
{
	GUNNUM_1 = 1,
	GUNNUM_2 = 2,
	GUNNUM_3 = 3,
  GUNNUM_4 = 4,
}E_GUNNUM;

typedef enum
{
	DEV_DC_S = 1,
	DEV_DC_D = 2,
}E_DEVTYPE;

/*在线卡1 离线卡 0 刷卡方式*/
typedef enum
{
	E_OFFLINE_CARD = 0,
	E_ONLINE_CARD = 1,
	
}E_STARTCARD_TYPE;

#define AGUN_NO  0
#define BGUN_NO  1


extern CHARGE_TYPE *ChgData_GetRunDataPtr(uint8 gunNo);

extern PARAM_OPER_TYPE *ChgData_GetRunParamPtr(void);

extern PARAM_FEE_TYPE *ChgData_GetFeeRate(uint8 gunNo);

extern PARAM_DEV_TYPE *ChgData_GetDevParamPtr(void);

extern DEV_LOGIC_TYPE * ChgData_GetLogicDataPtr(uint8 gunNo);


extern PARAM_COMM_TYPE *ChgData_GetCommParaPtr(void);

extern uint8 ChgData_InitDefaultOperPara(void);

extern uint8 ChgData_InitDefaultFeePara(void);

extern uint8 ChgData_InitDefaultDevPara(void);

extern START_PARAM *ChgData_GetStartParaPtr(uint8 gunNo);


#ifdef CP_PERIOD_FEE
extern PARAM_CP_PERIOD_FEE_TYPE *ChgData_GetCpPeriodFeeRate(void);

#endif




















#endif


