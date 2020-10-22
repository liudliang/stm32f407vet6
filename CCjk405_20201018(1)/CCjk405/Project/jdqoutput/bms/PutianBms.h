/**
  ******************************************************************************
  * @file    PutianBms.h
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#ifndef __PUTIANBMS_H_0001	
#define __PUTIANBMS_H_0001
#include "typedefs.h"


typedef  struct  
{
	uint8  batfacname[8];
	uint8  date;        /*BCD码*/
	uint8  month;
	uint16 year;
	uint32 chgnum;
	uint16 batgrpinfo;
}PT_BRM;

typedef struct
{
	uint8 ver[8];
}PT_BVM;

typedef union
{
	uint8 bystuts;
	struct {
		uint8 crotm : 1;
		uint8 comtm : 1;
		uint8 brmtm : 1;
		uint8 rsv   : 5;
	}bits;
}PT_CE1;

typedef union
{
	uint8 bystuts;
	struct {
		uint8 bp1tm : 1;
		uint8 cbrotm : 1;
		uint8 bcptm : 1;
		uint8 bbrotm : 1;
		uint8 rsv   : 4;
	}bits;
}PT_CE2;

typedef struct
{
	uint16 maxchgvolt;    /*蓄电池模块最高允许充电总电压0~655.35*/
	uint16 maxchgcurr;    /*偏移-3200*/
	uint8  maxCapper;      /*最大允许充电容量，0~100%*/
	uint16 maxtotalvoalt; /*最高允许充电总电压 0~6553.5*/
	uint8  temp;
}PT_BCP;


typedef struct
{
	uint16 carVin;
	uint16 batinf; /*LSB 12bit 电池串联数 MSB 4bit 电池并联数*/
	uint16 soc;
	uint16 totalCap;   /*总容量*/
}PT_BP1;

typedef struct
{
	uint16  chgcurr;      /*充电电流*/
	uint16  totalvolt;    /*电池总电压*/
	uint8   pcode;        /*供应商代码*/
	uint8   grpnum;       /*电池组数*/
	uint16  sbatnum;      /*每组电池只数*/
}PT_BP2;


typedef struct
{
	uint8 bro;
}PT_BRO;


typedef struct
{
	uint16 needvolt;
	uint16 needcurr;
	uint16 neepw;
	uint8  chgmode;
}PT_BCL;


typedef struct
{
	uint16 batvolt;
	uint16 batcurr;
	uint8 maxtemp;
	uint8 soc;
	uint8 lasttm;
}PT_BCS;


typedef union
{
	uint8 bystu;
	struct
	{
		uint8 hvol : 1;
		uint8 lvol : 1;
		uint8 hsoc : 1;
		uint8 lsoc : 1;
		uint8 hcurr: 1;
		uint8 tmper: 1;
		uint8 jherr: 1; /*组内均衡故障*/
		uint8 pperr: 1; /*组内匹配故障*/
	}bits;
}PT_BATSTU_BIT;

typedef struct
{
	uint16 maxvgrpinf;
	uint8  batNo;
	uint8  maxtemper;
	uint8  maxtemperNo;
	PT_BATSTU_BIT errstu;
}PT_BS1;


typedef union
{
	uint8 bystu[2];
	uint8 wordstr;
	struct
	{
		uint16 Iso   : 1;
		uint16 link  : 1;  /*高压连接状态*/
		uint16 wdg   : 1;  /*看门狗状态*/
		uint16 allow : 1;  /*充电允许*/
		uint16 btype : 3;  /*电池类型*/
		uint16 opmod : 3;  /*操作模式*/
		uint16 sysfg : 3;  /*电池电压单元标识*/
		uint16 lifeinfo : 2; /*BMS生命信息*/
	}bits;
}PT_BATINF_BIT;

typedef struct
{
	PT_BATINF_BIT stu;
}PT_BS2;


typedef struct
{
	PT_BRM brm;
	PT_CE1 ce1;
	PT_CE2 ce2;
	PT_BCP bcp;
	PT_BP1 bp1;
	PT_BP2 bp2;
	PT_BRO bro;
	PT_BCL bcl;
	PT_BCS bcs;
	PT_BS1 bs1;
	PT_BS2 bs2;
	
}PT_BMS;


typedef enum {
	VBI_PTCODE = 0x30,
	BRM_PTCODE = 0x02,
	BCP_PTCODE = 0x06,
	BRO_PTCODE = 0x09,
	BCL_PTCODE = 0x10,
	BCS_PTCODE = 0x11,
	BSM_PTCODE = 0x12,
	BMV_PTCODE = 0x15,
	BMT_PTCODE = 0x16,
	BSOC_PTCODE= 0x17,
	BST_PTCODE = 0x19,
	BSD_PTCODE = 0x1C,
	BEM_PTCODE = 0x1E,
}E_PUTIANBMSCODE;


typedef enum{

	CRM_PTCODE = 0x01,
	CTS_PTCODE = 0x07,
	CML_PTCODE = 0x08,
	CRO_PTCODE = 0x0A,
	CCS_PTCODE = 0x14,
	CST_PTCODE = 0x1A,
	CSD_PTCODE = 0x1D,
	CEM_PTCODE = 0x1F,
}E_PUTIANCHGCODE;




extern PT_BMS * PtBms_GetBmsDataPtr(uint8 gunNo);

extern void ProcPtBRM(void *item,uint8 *data,uint8 GunNo);



extern uint8 ChgSendPutianPkgCCS(uint8 *buf,void* st,uint8 GunNo);

extern uint8 ChgSendPutianPkgCML(uint8 *buf,void* st,uint8 GunNo);


extern uint8 ChgSendPutianPkgCRM(uint8 *buf,void* st,uint8 GunNo);




extern void Putian_Init(uint8 GunNo);

/*单条报文发送 设置SPN的值*/
extern uint8 Putian_ReportSingPkg(uint8 gunNo,uint8 pf,uint8 spn);


/*报文周期发送*/
extern uint8 Putian_CricleReport(uint8 gunNo);


/*报文发送周期关闭*/
extern uint8 Putian_CtrlCricle(uint8 gunNo,uint8 pf,uint8 allow,uint8 fg);

 /* 接收CAN链路帧 */
extern uint8 Putian_RcvDealFrame(uint8 GunNo,void *rtninfo,uint8 *pbuf);

extern void ProcPutianBRO(void *item,uint8 *data,uint8 GunNo);

extern void ProcPutianBCP(void *item,uint8 *data,uint8 GunNo);

extern void ProcPutianBCL(void *item,uint8 *data,uint8 GunNo);

extern void ProcPutianBCS(void *item,uint8 *data,uint8 GunNo);


extern void ProcPutianBSM(void *item,uint8 *data,uint8 GunNo);


extern uint8 ChgSendPutianPkgCRO(uint8 *buf,void* st,uint8 GunNo);


/* 所有报文发送周期关闭 */
extern void Putian_CtrlCricleAllclose(void);




#endif









