/**
  ******************************************************************************
  * @file    bms.h
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#ifndef _BMS_H_0001
#define _BMS_H_0001

#include "typedefs.h"

#define VOLT_PROTO(a) (a)
#define CURR_PROTO(a) (4000 > (a) ? (4000-(a)):0) 

#pragma pack(1)
typedef  struct CHM
{
	uint8 ver[3];
}CHM_ST;

typedef  struct BHM
{
	uint16 maxvolte;
}BHM_ST;
	
typedef  struct CRM
{
	uint8 idenf;         /*0x00:不能辨识 0xAA :能辨识*/
	uint8 serlnumber[4];
	uint8 areacode[3];
}CRM_ST;


typedef  struct OPT1
{
	uint8 seqno;
	uint8 date;
	uint8 month;
	uint8 hearh;
	uint8 yearl;
	uint8 rsv1;
	uint8 rsv2;
	uint8 rsv3;
}IN_BRM_ST;


typedef  struct BRM
{
	uint8 ver[3];
	uint8 battype;
	uint16 battecapty;
	uint16 battvolte;
	uint8 manufname[4];
	uint32 battgroupno;
	uint8 year;
	uint8 month;
	uint8 date;
	uint8 chgtimes[3];
	uint8 rightflag;
	uint8 rsv;
	uint8 vin[17];
	IN_BRM_ST buildinfo; /*可选项*/
}BRM_ST;

typedef  struct BCP
{
	uint16 sbatmaxvolt;
	uint16 maxcurr;
	uint16 sumenergy;
	uint16 chgmaxvolt;
	uint8 tempmax;
	uint16 batsoc;
	uint16 batcurvolt;
}BCP_ST;

typedef  struct CTS
{
	  uint16 year;
	  uint8 month;
	  uint8 date;
	  uint8 hour;
	  uint8 min;
	  uint8 sec;
}CTS_ST;

typedef  struct CML
{
	uint16 maxoutvolt;
	uint16 minoutvolt;
	uint16 maxoutcur;
	uint16 minoutcur;
}CML_ST;


typedef  struct BRO
{
	uint8 brostu;
}BRO_ST;

typedef  struct CRO
{
	uint8 crostu;
}CRO_ST;

typedef  struct BCL
{
	uint16 needvolt;
	uint16 needcurr;
	uint8  chgmode;
	uint16 bakneedvolt;  /*bak 用于备份,判断需求是否变化*/
	uint16 bakneedcurr;
	uint8  bakchgmode;
}BCL_ST;


typedef  struct BCS
{
	uint16 detecvolt;
	uint16 deteccurr;
	 union {
		uint16 word;
		 struct {
			uint16 volt:12;
			uint16 groupno:4;
		}bits;
	}singbat;
	uint8 cursoc;
	uint16 lasttimes; /*剩余时间分*/
}BCS_ST;

typedef  struct CCS
{
	uint16 outvolt;
	uint16 outcurr;
	uint16 totalchgtime;
	 union {
		uint16 word;
		 struct {
			uint16 fg : 2;  /*协议中只有两位有效*/
			uint16 rsv:14;
		}bits;	
	}allowed;
}CCS_ST;

typedef  struct BSM
{
	uint8 singbatmaxvno;
	uint8 batmaxtemp;
	uint8 batmatempno;
	uint8 batmintemp;
	uint8 batmintempno;
	 union
	{
		uint8 byte[2];
		uint16 word;
		 struct
		{
			uint16 singbatValm:2;
			uint16 socalm:2;
			uint16 curralm:2;
			uint16 tempalm:2;
			uint16 insulalm:2;
			uint16 linkalm:2;
			uint16 allowed:2;
			uint16 rsv:2;	
		}bits;
	}battalm;
}BSM_ST;


typedef  union
{
	uint16 word;
	 struct
	{
		uint16 volt:12;
		uint16 gpno:4;
	}bits;
}VOL_ST;

typedef  struct BMV
{
	VOL_ST singbat[256];
	
}BMV_ST;  /*可选项*/

typedef  struct BMT
{
	 uint8 singbattemp[128];
}BMT_ST;  /*可选项*/


typedef  struct BSP
{
	uint8 batrsv[16];
}BSP_ST; /*可选项*/

typedef struct {
	uint32 u32WaitTicks;
	uint16 u16NeedCurr;
	uint16 u16NeedVolt;
}PARA_BMSCTRL;


typedef  struct BST
{
	 union {
		uint8 byte;
		 struct {
			uint8 socok:2;
			uint8 voltok:2;
			uint8 singvolt:2;
			uint8 initiative:2; /*充电机主动停止*/
		}bits;
	}stoprsn;
	 union {
		uint16 word;
		 struct{
			uint16 isoflt:2;
			uint16 lkovtp:2;
			uint16 parovtp:2;
			uint16 chglkflt:2;
			uint16 batovflt:2;
			uint16 rlyflt:2;
			uint16 ckpt2flt:2;
			uint16 otherflt:2;
		}bits;
	}faultrsn;	
	 union {
		uint8 byte;
		 struct{
			uint8 overcurr:2;
			uint8 unusualvolt:2;
			uint8 rsv:4;
		}bits;
	}errorrsn;
}BST_ST;

typedef  struct CST
{
	union
	{
		uint8 byte;
		 struct {
			uint8 condok:2;
			uint8 manual:2;
			uint8 fault:2;
			uint8 initiative:2;
		}bits;
	}stoprsn;
	 union{
		uint16 word;
		uint8 byte[2];
		 struct {
			uint16 chgovtp : 2;
			uint16 lkflt:2;
			uint16 insideovtp:2;
			uint16 transflt:2;
			uint16 scramflt:2;
			uint16 otherflt:2;
			uint16 rsv:4;
		}bits;
	}faultrsn;
	 union {
		uint8 byte;
		 struct {
			uint8 currerr:2;
			uint8 unusualvolt:2;
			uint8 rsv:4;
		}bits;
	}errorrsn;
}CST_ST;

typedef  struct BSD
{
	uint8 stopsoc;
	uint16 singbatminvolt;
	uint16 singbatmaxvolt;
	uint8  batmintemp;
	uint8  batmaxtemp;
}BSD_ST;

typedef  struct CSD
{
	 uint16 chgtotaltime;
	 uint16 outenergy;
	 uint8 chgcode[4];
}CSD_ST;


typedef  struct BEM
{
	 union {
		uint32 dword;
		 struct {
			uint32 crmtmout:2;
			uint32 crmoktmout:2;
			uint32 rsv1:4;
			uint32 cmltmout:2;
			uint32 crotmout:2;
			uint32 rsv2:4;
			uint32 ccstmout:2;
			uint32 csttmout:2;
			uint32 rsv3:4;
			uint32 csdtmout:2;
			uint32 other:6;
		}bits;
	}berrcode;
}BEM_ST;

/*
typedef struct CEM
{
	union 
	{
		uint32 dword;
		uint8 byte[4];
		struct 
		{
			uint32 bhmtmout : 2;
			uint32 brmtmout : 2;
			uint32 rsv1     : 4;
			uint32 brotmout : 2;
			uint32 bcstmout : 2;
			uint32 rsv2     : 4;
			uint32 bcltmout : 2;
			uint32 bcttmout : 2;
			uint32 rsv3     : 4;
			uint32 bsdtmout : 2;
			uint32 other    : 6;
		}bits;
	}cerrcode;
}CEM_ST;
*/

/* GB/T 27930-2015 */
typedef struct CEM
{
	union 
	{
		uint32 dword;
		uint8 byte[4];
		struct 
		{
			uint32 brmtmout : 2;
			uint32 rsv1     : 6;
			uint32 bcptmout : 2;
			uint32 brotmout : 2;
			uint32 rsv2     : 4;
			uint32 bcstmout : 2;
			uint32 bcltmout : 2;
			uint32 bsttmout : 2;
			uint32 rsv3     : 2;
			uint32 bsdtmout : 2;
			uint32 other    : 6;
		}bits;
	}cerrcode;
}CEM_ST;

typedef  struct BMS
{
	 BHM_ST bhm;
	 BRM_ST brm;
	 BCP_ST bcp;
	 BRO_ST bro;
	 BCL_ST bcl;
	 BCS_ST bcs;
	 BSM_ST bsm;
	 /*BMV_ST bmv;     可选项*/
	 /*BMT_ST bmt;     可选项*/
	 /*BSP_ST bsp;     可选项*/
	 BST_ST bst;
	 BSD_ST bsd;
	 BEM_ST bem;
}BMSDATA_ST;

typedef  struct CHG
{
	 CHM_ST chm;
	 CRM_ST crm;
	 CTS_ST cts;
	 CML_ST cml;
	 CRO_ST cro;
	 CCS_ST ccs;
	 CST_ST cst;
	 CSD_ST csd;
	 CEM_ST cem;
}CHGDATA_ST;

#pragma pack()

typedef struct
{
	 uint32 lockstart;
	 uint32 bhmstart;
	 uint32 xfstart; /*泄放*/
	 uint32 brmstart;
	 uint32 bcpstart;
	 uint32 brostart;
	 uint32 bclstart;
	 uint32 bcsstart;
	 uint32 bststart;
	 uint32 cststart;
	 uint32 bsdstart;
	 union {
		uint32 dword;
		uint8 byte[4];
		 struct {
			 uint32 normal     : 1;   /*0 正常*/
			 uint32 locktmout  : 1; 	/*1 电子锁超时*/
			 uint32 bhmtmout   : 1; 	/*2 bhm超时*/
			 uint32 xftmout    : 1; 	/*3泄放On后，继电器外侧电压5s内是否降到60V以下*/
			 uint32 brmtmout   : 1; 	/*4 BRM 超时*/
			 uint32 bcptmout   : 1; 	/*5*/
			 uint32 brotmout   : 1; 	/*6*/
			 uint32 bcstmout   : 1; 	/*7*/
			 uint32 bcltmout   : 1; 	/*8*/
			 uint32 bsttmout   : 1; 	/*9*/
			 uint32 csttmout   : 1;  	/*10CST 后100ms 电流降到5A以下*/
			 uint32 bsdtmout   : 1;  	/*11*/
			 uint32 kmvolt10err : 1;  /*12 k1k2 外侧电压<10V 2*/
			 uint32 kmvolt60err : 1;  /*13k1k2 外侧电压<60V 2*/
			 uint32 paramerr    : 1;  /*14 车辆参数错误 3*/
			 uint32 carvolterr  : 1;  /*15 与报文电池电压偏差 <±5%*/
			 
			 uint32 batstuerr   : 1;  /*16 电池状态是否正常*/
			 uint32 disallow    : 1;  /*17 充电禁止标志*/
			 uint32 cc1         : 1;  /*18 cc1状态*/
			 uint32 modvolt     : 1;  /*19 模块电压调整故障*/
			 uint32 inoutvolt   : 1;  /*20 接触器内外侧电压差大于10V*/
			 uint32 rsvbit3     : 1;  /*21*/
			 uint32 rsvbit4     : 1;  /*22*/
			 uint32 rsvbit5     : 1;  /*23*/
			 uint32 rsvbit6     : 1;  /*24*/
			 uint32 rsvbit7     : 1;  /*25*/
			 uint32 rsvbit8     : 1;  /*26*/
			 uint32 rsvbit9     : 1;  /*27*/
			 uint32 rsvbit10    : 1;  /*28*/
			 uint32 rsvbit11    : 1;  /*29*/
			 uint32 rsvbit12    : 1;  /*30*/
			 uint32 commerr     : 1;  /*31*/
		 }bits;
	 }stu;
}ERR_STEP_ST;


//typedef enum {
//	LOCK_TM = 0x01,    /*电子锁*/
//	BHM_TM  = 0x02,    /*bhm*/
//	XF_TM   = 0x03,    /*泄放*/
//	BRM_TM  = 0x04,    /*brm*/
//	BCP_TM  = 0x05,    /*bcp*/
//	BRO_TM  = 0x06,    /*bro*/
//	BCL_TM  = 0x07,    /*bcl*/
//	BCS_TM  = 0x08,    /*bcs*/
//	BST_TM  = 0x09,    /*bct*/
//	CST_TM  = 0x0A,    /*cst 100ms*/
//	BSD_TM  = 0x0B,    /*bsd*/
//	
//	ISO_ERR = 0x0c,    /*绝缘故障*/
//	KMVOLT10_ERR = 0x20, /*接触器外侧电压<10V*/
//	KMVOLT60_ERR = 0x21, /*接触器外侧电压<60V*/
//	PARAM_ERR    = 0x22, /*参数是否合适*/
//	CARVOL_ERR   = 0x23, /*与报文电池电压偏差 <±5%*/
//	BATSTU_ERR   = 0x24, /*电池状态是否正常*/
//	DISALLOW_FG  = 0x25, /*充电禁止标志*/
//	CC1LINKED_ERR = 0x26, /*CC1连接状态*/
//	MODVOLT_ERR   = 0x27, /*模块电压输出未响应*/
//	KMINOUT_ERR   = 0x28, /*接触器内外侧电压差大于10V*/
//	
//	
//	EMET_COMM_ERR = 0x40,   /*直流表通讯故障*/
//	ISO_COMM_ERR   = 0x42,  /*绝缘模块通讯故障*/
//	DYMOD_COMM_ERR  = 0x43, /*电源模块通讯故障*/
//	EACMET_COMM_ERR = 0x44, /*交流表通讯通讯故障*/
//	
//	
//}E_STEP_ERR;


typedef struct {
	CHGDATA_ST chg;
	BMSDATA_ST car;
	ERR_STEP_ST err;
}DEV_BMS_TYPE;

typedef struct      /* 收到BMS对应的包，对应位置1 */
{
	union
	{
		uint16 word;
		struct
		{
			uint16 rcvBHM : 1;  /*BHM*/
			uint16 rcvBRM : 1;  /*BRM*/
			uint16 rcvBCP : 1;  /*BCP*/
			uint16 rcvBRO : 1;  /*BRO*/
			uint16 rcvBCL : 1;  /*BCL*/
			uint16 rcvBCS : 1;  /*BCS*/
			uint16 rcvBSM : 1;  /*BSM*/
			uint16 rcvBMV : 1;  /*BMV*/
			uint16 rcvBMT : 1;  /*BMT*/
			uint16 rcvBSP : 1;  /*BSP*/
			uint16 rcvBST : 1;  /*BST*/
			uint16 rcvBSD : 1;  /*BSD*/
			uint16 rcvBEM : 1;  /*BEM*/
			uint16 RSV    : 3;
		}bits;
	}statu;
}RCV_PACK_ST;

/**************************函数***********************************************/
extern BMSDATA_ST *Bms_GetBmsCarDataPtr(uint8 gunNo);

extern CHGDATA_ST *Bms_GetChgDataPtr(uint8 gunNo);

extern RCV_PACK_ST *Bms_GetBmsPack(uint8 gunNo);

extern uint8 BmsAckPkgEC(uint8 *buf,void* st,uint8 GunNo);

extern uint8 BmsAckPkgEB(uint8 *buf,void* st,uint8 GunNo);

extern uint8 ChgSendPkgCHM(uint8 *buf,void* st,uint8 GunNo);

extern uint8 ChgSendPkgCRM(uint8 *buf,void* st,uint8 GunNo);

extern uint8 ChgSendPkgCTS(uint8 *buf,void* st,uint8 GunNo);

extern uint8 ChgSendPkgCML(uint8 *buf,void* st,uint8 GunNo);

extern uint8 ChgSendPkgCRO(uint8 *buf,void* st,uint8 GunNo);

extern uint8 ChgSendPkgCCS(uint8 *buf,void* st,uint8 GunNo);

extern uint8 ChgSendPkgCST(uint8 *buf,void* st,uint8 GunNo);

extern uint8 ChgSendPkgCSD(uint8 *buf,void* st,uint8 GunNo);

extern uint8 ChgSendPkgCEM(uint8 *buf,void* st,uint8 GunNo);


/*拒绝接收多包*/
extern uint8 ChgSendPkgRefuseMuliti(uint8 *buf,void* st,uint8 GunNo);

extern void ProcBHM(void *item,uint8 *data,uint8 GunNo);

extern void ProcBRM(void *item,uint8 *data,uint8 GunNo);

extern void ProcBCP(void *item,uint8 *data,uint8 GunNo);

extern void ProcBRO(void *item,uint8 *data,uint8 GunNo);

extern void ProcBCL(void *item,uint8 *data,uint8 GunNo);

extern void ProcBCS(void *item,uint8 *data,uint8 GunNo);

extern void ProcBSM(void *item,uint8 *data,uint8 GunNo);

extern void ProcBMV(void *item,uint8 *data,uint8 GunNo);

extern void ProcBMT(void *item,uint8 *data,uint8 GunNo);

extern void ProcBSP(void *item,uint8 *data,uint8 GunNo);

extern void ProcBST(void *item,uint8 *data,uint8 GunNo);

extern void ProcBSD(void *item,uint8 *data,uint8 GunNo);

extern void ProcBEM(void *item,uint8 *data,uint8 GunNo);

extern uint32 Bms_GetStartTimeCount(uint8 gunNo,uint8 tg);

extern void Bms_StartTimeCount(uint8 gunNo,uint8 tg,uint32 ticks);

extern void Bms_SetStepErrFg(uint8 gunNo,uint8 tg,uint8 yesorno);

extern ERR_STEP_ST *Bms_GetStepStPtr(uint8 gunNo);

extern void Bms_ClearAllTmoutFg(uint8 gunNo);

extern uint8 Bms_GetStepErrFg(uint8 gunNo,uint8 tg);

extern uint8 Bms_GetErrCode(uint8 gunNo);  /*获取启动失败故障码 0 ~ 32 */

extern void TskBms_ProtoInit(uint8 gunNo,uint8 protoNo);

extern void Bms_InitErrData(uint8 gunNo);



#endif

