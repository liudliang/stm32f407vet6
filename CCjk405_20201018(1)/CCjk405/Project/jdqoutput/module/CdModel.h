/*HcModel.h*/
#ifndef __HCMODEL_H_001
#define __HCMODEL_H_001
#include "typedefs.h"
#include "message.h"
#include "can.h"


#ifdef STM32F10X_HD
#define RS485_CDMOD
#else
#define CAN_CDMOD
#endif

#define CDMOD_CAN 	CAN_2   //patli 20200318 CAN_COM3

#define CD_HOSTID  0xF0
#define CDPROTO_NO 0x60

#define MOD_CMD_ON   0
#define MOD_CMD_OFF  1
#define MOD_CMD_GRP_OFF 2

#define CD_MAX_NUM  8     /*60KW  4 * 15KW */

#define CD_GROUP_No    1
#define CD_global_add  0

#define BRAODCAST_ADR  0
#define CMD_SEND_TIMES 2

#define MAX_MOD_GROUP  2
#define MAX_GPMOD_NUM  8



#define CD_ModeAC_EndVlot  3100   //0.1V   最低304V
#define CD_ModeAC_TopVlot  3900   //0.1V   最高394V

#define CD_ModeIn_EndVlotDC  2050   //0.1V    模块最低200V
#define CD_ModeIn_TopVlotDC  7750   //0.1V     模块最高780V


typedef enum
{  
	 SETTING_DATA = 0x0,      /*设置数据*/
	 ACK_SETTINGDATA = 0x1,      /*设置应答请求*/
	 REQ_BYTEDATA = 0x2,      /*查询数据*/
	 ACK_REQ_DATA  = 0x3,   /*查询应答数据*/
	 MULTI_DATA_CMD = 0x4,    /*综合数据请求命令*/
	
}CD_MSGTYPE;

/*CAN 协议*/
typedef enum 
{
	ERR_CMD   = 0x0000,
	OUT_VOLTE = 0x0001,          /*模块端口输出电压*/
	OUT_CURR  = 0x0002,          //取模块端口输出电流
	MOD_LIMIT_POIN = 0x0003,     //模块限流点    读不到这个寄存器
	INPUT_TEMPER = 0x000B,       //进风口环境温度
	INPUT_AC_AVOLT = 0x000C,       //取模块AC输入电压
	INPUT_AC_BVOLT = 0x000D,       //取模块AC输入电压
	INPUT_AC_CVOLT = 0x000E,       //取模块AC输入电压
	ALARM_WORDS = 0x0040,            /*读取当前告警状态*/

	
	SETTING_MOD_VOLTE = 0x0021,    /*设置模块电压*/
	SETTING_LIMIT_POINT = 0x22,    /*设模块限流点*/
	START_STOP_OPER = 0x0030,       /*设置开关机操作*/ 
  GROUP_NUMBER = 0x00BD,         //设置模块组号	
  
}CD_VALUETYPE;



typedef enum
{
	START_HCMOD = 0x0000,
	STOP_HCMOD = 0x0001,
	WAIT_OPER  = 0x0000,
	RECF_CURR  = 0x0001,   /*整流*/
	CONTRA_CURR = 0x0002,  /*逆变*/
	
}CD_CMD;

typedef enum
{ 
	 CAP_DISENABLE = 0x0000,
   CAP_ENABLE = 0x0001,
	
}CD_CAP;

typedef struct {
	uint8 vaild;
	uint8 num;
	uint8 AddrList[CD_MAX_NUM+1];
}COMM_MODLIST;


#ifdef  CAN_CDMOD
 #define MOD_ONOFF    START_STOP_OPER
 #define MOD_SETVOLT  SETTING_MOD_VOLTE
 #define MOD_SETCURR  SETTING_LIMIT_POINT
 #define COFF_DATA    1024/10 
 #define MOD_SETGROUP  GROUP_NUMBER
#else
 #define MOD_ONOFF     EONOFF
 #define MOD_SETVOLT   EVREF
 #define MOD_SETCURR   EILMT
 #define COFF_DATA     100
 #define MOD_SETGROUP  EGRPADRESS
#endif



#pragma pack(1)
typedef union
{
  uint32 ExtId;
	struct
	{
		uint32 res2:1;      /*保留 1*/
		uint32 res1:1;      /*保留 1*/
		uint32 cnt :1;      /*连续帧*/
		uint32 src:8;       /*源地址*/
		uint32 dst:8;       /*目的地址*/
		uint32 ptp:1;       /*点对点*/
		uint32 protno:9;    /*协议号*/
		uint32 rsv:3;       /*保留*/
		
	}sbit;
		
}CDFRAME_HEAD;


typedef enum
{
	ESET_DATA = 0,
	ESET_RESP = 1,
	EREAD_DATA= 2,
	EREAD_RESP = 3,
}EMSG_TYPE;

/*RS485 协议 */
typedef enum
{
	EVOUT = 0,
	ECURR = 1,
	EVREF = 2,
	EILMT = 3,
	EONOFF= 4,
	EMODST = 8,
	ETEMP  = 30,
	EGRPADRESS = 89,
}ECMD_TYPE;




typedef  union{
  uint8 uc[4];
  uint16 us[2];
  uint32 u32d;
  float  f32d;	
}DATA4_TYPE;





typedef  union
{
	uint16 us;
	uint8 uc[2];
}DATA2_TYPE;


typedef  union
{
   uint8 u8;
	  struct 
		{ 
			uint8 msgType:4;	
		 	uint8 groupAddress:4;
			 
	  }bits;
}GRPTYPE;

typedef union
{
   uint8 dat[8];
     struct DA
		{
		  GRPTYPE ntype;
		  uint8 errType;
		  DATA2_TYPE vtype;
      DATA4_TYPE val;	
		}db;
}CDFRAME_DATA;



/*奥耐V1.2版 2017-11-03*/
typedef  union
{
	uint32 u32d;
	 struct {
		uint32 ovAcVolt:1;      /*0交流输入过压*/
		uint32 underAcVolt:1;   /*1交流输入欠压*/
		uint32 ovAcVoltstop:1;  /*2交流过压关机，模块保护*/
		uint32 rsv0:3;          /*3,4,5*/
	 	uint32 ovVolt:1 ;       /*6直流输出过压*/
		uint32 ovVoltstop:1;    /*7直流过压关机*/
		
		uint32 underVolt:1 ;    /*8直流输出欠压*/
		uint32 ErrFan:1;        /*9风扇不运行*/
		uint32 rsv1:2;          /*10,11保留*/
		uint32 ovTermp:1;       /*12过温保护*/
		uint32 rsv2:1;          /*13保留*/
		uint32 PFCovTermp1:1;   /*14PFC过温保护1*/
		uint32 PFCovTermp2:1;   /*15PFC过温保护2*/
		uint32 DCovTermp1:1;    /*16DC过温度保护1*/
		uint32 DCovTermp2:1;    /*17DC过温保护2*/
		uint32 ErrModel:1;      /*18模块故障，PFC与DCDC通信故障*/
		uint32 rsv3:1;          /*19保留*/
		uint32 PFCErr:1;        /*20PFC故障*/
		uint32 DCDCErr:1;       /*21DCDC故障*/
		uint32 rsv4:1;          /*22保留*/
		uint32 rsv5:1;          /*23保留*/
		uint32 rsv6:1;          /*24保留*/
		uint32 ModStu:1;        /*25模块状态 1：关机 0：开机*/ 
		uint32 ModWork:1;       /*26模块工资方式 1：限流 0：恒压*/ 
		uint32 rsv7:1;          /*27保留*/
		uint32 rsv8:1;          /*28保留*/
		uint32 rsv9:1;          /*29保留*/
		uint32 rsv10:1;         /*30保留*/
		uint32 rsv11:1;         /*31保留*/
	}bits;
}ALARM_STATU;

#define DLMOD_ON   0
#define DLMOD_OFF  1


typedef  union
{
	 uint32 u32d;
	  struct
	 {
		 uint8 pfchw;
		 uint8 pfcsw;
		 uint8 dcdchw;
		 uint8 dcdcsw;
	 }us;
}VERSION_ST;



typedef  struct 
{
	uint16 inVolteACA;       /*输入电压*/
	uint16 inVolteACB;       /*输入电压*/
	uint16 inVolteACC;       /*输入电压*/
	uint16 outVolte;         /*输出电压*/
	uint16 outCurr;          /*输出电流*/
	uint16 limtPoint;        /*限流点*/
	uint16 temper;           /*温度*/
	ALARM_STATU alarmInfo;
	uint8 onoff;             //1 关闭  ；0 启动
	uint8 commErr;           /*模块通信故障*/
	uint8  grpNo;           /*模块组号*/
	uint8  addr;            /*模块地址*/
	uint8  vaild;           /*是否有效*/
	uint32 lastticks;       /*最后通讯时的ticks*/
	
}CDMOD_DATA;



typedef  struct 
{
	uint16 outVolte;    /*输出电压*/
	uint16 limtPoint;
	uint8 onoff;
	uint8 modegroupnum;
	
}CDMOD_Set;

#pragma pack()


typedef struct
{
	 uint8  succ;
	 uint8  exec;
	 uint8  dstAddr;
	 uint8  cmd;
	 uint8  groupAddress;
	 uint16 vtype;
   uint32 value;
}CMD_LIST;

typedef struct
{
	uint8  u8Num;            /*组内模块数量*/
	uint8  u8GpNo;           /*组号*/
	uint8  u8CommStu;        /*组模块通讯状态 1，通讯正常，0无通讯*/
	uint8  u8SplitNum;       /*本次分配模块数*/
	uint8  u16Allowed;       /*是否允许分配*/
	uint8  u8SlipCnt;        /*分配计数*/
	uint16 u16Curr;          /*分配电流*/
	uint16 u16Volt;          /*分配电压*/
	uint16 u16StepCurr;      /*分配步进电流*/
	uint8  u8Step;           /*控制步骤*/
	uint8 u8Addr[MAX_GPMOD_NUM];        /*组内模块地址*/
	uint8 u8WorkStu[MAX_GPMOD_NUM];     /*组内模块状态*/
	uint8 u8HwGpModNum;     /*硬件组内模块数量*/
	uint8 u8HwGpAddr[MAX_GPMOD_NUM];    /*硬件组内模块地址，由硬件固化*/
	uint16 u16HwTolCurr;       /*硬件组内模块总电流*/
	uint32 u32WaitTicks;
	uint32 u32ExitTicks;      /*退出计时*/
	uint32 u32StepTicks;
}MOD_CTRL_PARA;



//extern CDMOD_DATA gCdmodData[];


extern void CdModData_Init(uint8 grpNo);



extern CDMOD_DATA * CdModData_GetDataPtr(uint8 no);


extern struct list *CdMod_GetHead(void);

extern int CdMod_InsertOneCmd(CMD_LIST * scmd);
extern int CdMod_deleteCmd(int id);

extern int CdMod_ExecuCmd(void);

extern int CdMod_IsListExec(int id,uint16 wmsecs) ;

extern int CdMod_DeleteAllCmd(void);


/*检测可通讯的模块是否与配置一致，以此判断是否有通讯故障*/
extern uint8 CdModData_CheckCommErr(uint8 grpNo);

extern uint8 CdModData_CheckOnOffStu(uint8 grpNo,uint8 stu);

extern uint8 CdModData_GetModCommErr(uint8 grpNo);

extern uint8 CdModData_DelAllCmd(void);


extern int CdModData_InsertModCmdInList(uint8 adr,uint8 grpNo,uint16 cmd,uint32 value);


extern void CdModData_MsgDeal(MSG_STRUCT* msg);

extern uint8 CdModData_SetGroupNo(uint8 adr,uint8 grpNo);


extern MOD_CTRL_PARA *CdModData_GetModGpPara(uint8 grpNo);

/*发布广播命令*/
extern uint8 CdModData_SetGroupCmd(uint8 grpNo,uint8 cmd,uint32 value);

extern uint8 CdModData_SetSingleCmd(uint8 adr,uint8 cmd,uint32 value);



#endif


