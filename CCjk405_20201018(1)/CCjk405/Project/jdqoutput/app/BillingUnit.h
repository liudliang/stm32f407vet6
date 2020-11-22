/**
  ******************************************************************************
  * @file    BillingUnit.h
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#ifndef _BILLINGUNIT_H_0001
#define _BILLINGUNIT_H_0001

#include "typedefs.h"
#include "TaskCardReader.h"
#include "TaskMainCtrl.h"

#define  MAXSFEILVNUMS            24

#pragma pack(1)
typedef  struct
{	
	uint8 serivetype;    /*服务费方式 0：无 1：按电量 2：按次*/
	uint8 bespktype;     /*预约费方式 0：无 1：按时间*/
	uint8 parktype;      /*停车费方式 0：无 1：按时间*/
	uint8 lossrate;    /*充之鸟 电损值 百分比*/
	uint8 sectNo[48];    /*存放着时段费率号 */
	uint8 cdsectNo[MAXSFEILVNUMS];  /*车电网10段费率保存*/  
	
	int32 price[MAXSFEILVNUMS];     /*电费 */
  int32 srvrate[MAXSFEILVNUMS];   /*服务费*/
	int32 bespkrate[MAXSFEILVNUMS]; /*预约务费*/
	int32 parkrate[MAXSFEILVNUMS];  /*停车费*/
}SECT_PRICE_ST;
	
#define MAX_HALFHOUR_SECT  48


#define MAX_CHG_SECT 48
#define BILL_PRICE_HECI 10000
#define BILL_PRICE_YIDIAN 100
#define BILL_PRICE_BASE BILL_PRICE_HECI   //保留2位小数
/* 账单记录结构 */
typedef  struct
{
	uint32 storeNo;        /*记录存储序号*/
	uint8  chgtype;  			 /*充电方式 1:立即充电 2预约充电            */
	uint8  chgmod;   			 /*充电模式 0 自动 1.金额 2.时间 3.电量      */
	uint8  cardtype; 			 /*充电卡类型*/
	uint8  startmod;       /*启动模式： 1：离线卡  2、在线卡 3、后台app*/
	uint8  cardNo[20];     /*充电卡号*/
	uint8  vincode[17];    /*VIN码*/
	uint8  serialNo[32];   /*记录流水号 77*/
	
	int32 beforemoney;     /*充电前余额*/
	int32 aftermoney;      /*充电余额*/
 	int32 energymoney;     /*电费*/
	int32 serivemoney;     /*服务费*/
	int32 parkmoney;       /*停车费*/
	int32 bespkmoney;      /*预约费*/
	int32 billmoney;       /*账单金额 */
	
	uint8  startsoc;       /*开始soc*/
	uint8  soc ;           /*当前soc*/
	
	uint32 startenergy;    /*开始电能*/
	uint32 stopenergy;     /*结束电能*/
	uint32 energy;         /*充电电能 真实*/
	
	uint8  startsect;      /*开始时段号*/
	uint8  stopsect;       /*结束时段号 44*/
	uint32 sectenergy[MAX_CHG_SECT]; /*时段电能 0.01Kwh 96*/	
	
	uint8  termid[4];      /*电损电量*/
	uint32 parksecs;       /*停车时长 秒*/
	uint8  backtype;       /*充电是使用的后台类型*/
	uint8  gunno;          /*充电枪号*/
	
	uint8  starttm[6];     /*充电开始时间*/
    uint8  stoptm[6];      /*充电结束时间*/
	uint32 chgsecs;        /*充电时长 秒*/
	
	
	uint8  endreason;      /*充电结束原因*/
	uint8  errcode;        /*故障码*/
	uint8  IsReport;       /*是否上报*/
	uint8  IsPay;          /*是否结算 30*/
	
	uint8  cardNo2[12];    /*澄鹏的后台,用户号扩展到32位,在这里增加*/
	uint8  stopreason[16]; /*充电结束原因,16字节的每个bit位代表一个结束原因*/
}CALCBILL_DATA_ST; 

typedef  struct
{	
	uint8   serivetype;    /*服务费方式 0：无 1：按电量 2：按次*/
	uint8   bespktype;     /*预约费方式 0：无 1：按时间*/
	uint8   parktype;      /*停车费方式 0：无 1：按时间*/
	
	int32   bespkmoney;
	int32   srvmoney;
	int32   parkmoney;
	
	uint8   cursectno;     /*当前时段号*/
	int32   currSrvrate;   /*当前服务费*/
	int32   currfeerate;   /*当前费率 (实时电价) */
	int32   parkfeerate;   /*当前费率 (实时电价) */
	uint32  startticks;
	uint32  lasttimes;       /*剩余充电时间*/
	uint32  tmpstartenergy;  /*临时开始计量*/

}REAL_DATA_ST;

typedef  struct
{
	CALCBILL_DATA_ST bill;
	REAL_DATA_ST     real;
	
}REAL_BILL_DATA_ST;

#pragma pack()

typedef enum {
	SRV_NULL_TYPE  =  0,
	SRV_ENERGY_TYPE = 1,
	SRV_TIMES_TYPE =  2,
	
	BESPK_NULL_TYPE = 0,
	BESPK_TIME_TYPE = 1,
	BESPK_TMES_TYPE = 2,
	
	PARKT_NULL_TYPE = 0,
	PARKT_TIME_TYPE = 1,
	PARKT_TMES_TYPE = 2,
}E_FEE;




extern CALCBILL_DATA_ST * Bill_GetBillData(uint8 gunNo);
extern REAL_BILL_DATA_ST *GetFeeData(uint8 gunNo);
extern void Bill_InitData(uint8 gunNo);

/* 根据系统时间查找时段，从0时段开始 */
extern uint8 Bill_FindSectNo(void);

/*根据指定时间查找时段号*/
extern uint8 Bill_FindSectNoByTime(uint8 hour,uint8 min);

/*根据指定时间查找时段号*/
extern uint8 Bill_FindSectNoByTime(uint8 hour,uint8 min);

/*设置卡相关信息*/
//extern void Bill_SetCardInfo(CARD_INFO *pCardinf);

/*初始化计费数据*/
extern void Bill_SetStartData(uint8 gunNo);


/*设置结束计费数据*/
extern void Bill_SetStopData(uint8 gunNo,uint8 reason,uint8 errcode);

/*实时计费数据计算*/
extern uint32 Bill_RealCalc(uint8 gunNo,uint8 minEry);

extern void Bill_SetStartInfo(START_PARAM *startpara);


extern void Bill_GetCurrentFeeReate(uint8 gunNo);


extern REAL_BILL_DATA_ST *GetFeeData(uint8 gunNo);



extern void Bill_SetCarBmsData(uint8 gunNo);
void heli_Bill_SetCarBmsData(uint8 gunNo);
























#endif

