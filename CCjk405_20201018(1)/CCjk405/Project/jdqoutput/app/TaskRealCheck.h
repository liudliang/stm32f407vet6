/**
  ******************************************************************************
  * @file    RealCheck.h
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/

#ifndef __CHECKSTATUS_H_0001
#define __CHECKSTATUS_H_0001

#include "typedefs.h"

#define CHECK_TRUE  1
#define CHECK_FALSE 0

#define MAX_ERR_BYTE 15

/*最大支持120个故障字*/
typedef enum
{
	ECODE_NORMAL = 0,    			/*充电正常*/
	ECODE1_ELOCK  = 1,   			/*电子锁故障  */
	ECODE2_BHMTM  = 2,   			/*BHM超时*/
	ECODE3_XFTM   = 3,   			/*泄放超时*/
	ECODE4_BRMTM  = 4,   			/*BRM超时 */
	ECODE5_BCPTM  = 5,   			/*BCP超时*/
	ECODE6_BROTM  = 6,   			/*BRO超时*/
	ECODE7_BCLTM  = 7,   			/*BCL超时*/
	ECODE8_BCSTM  = 8,   			/*BCS超时*/
	ECODE9_BSTTM  = 9,  		  /*BST超时*/
	ECODE10_CSTTM  = 10,  		/*CST超时*/
	ECODE11_BSDTM  = 11,   		/*BSD超时*/
	ECODE12_KM10V  = 12,     	/*接触器外侧电压大于10V*/
	ECODE13_KM60V  = 13,     	/*泄放电压不低于60V*/
	EOCDE14_PARAM  = 14,     	/*车辆参数错误*/
	ECODE15_PKGVOLT= 15,    	/*报文电压不匹配*/
	ECODE16_BATSTU = 16,      /*电池状态错误*/
	ECODE17_FORBID = 17,      /*充电禁止*/
	ECODE18_CC1LNK = 18,      /*充电枪 cc1连接错误*/
	ECODE19_MODVOLT= 19,      /*模块电压调整错误*/
	ECODE20_INOUT10V = 20,    /*接触器内外侧电压不小于10V*/
	ECODE31_BMSCOMM= 31,      /*BMS通讯错误*/
	
	ECODE32_GUNTMPOVER = 32,  /*充电枪过温*/
	/*绝缘单元*/
	ECODE35_ISOCOMM =35,      /*绝缘单元通讯错误*/
	ECODE36_1POSERR= 36,      /*一路正对地故障*/
	ECODE37_1NEGERR= 37,      /*一路负对地故障*/
	ECODE38_2POSERR= 38,      /*二路正对地故障*/
	ECODE39_2NEGERR= 39,      /*二路负对地故障*/
	
	ECODE41_MODCOMMERR = 41,  /*模块通讯故障*/
	ECODE42_DC1OVER =42,      /*直流输出过压*/
	ECODE43_DC2OVER =43,      /*2枪直流输出过压*/ 
	ECODE44_DC3OVER =44,      /*3枪直流输出过压*/ 
	ECODE45_DC4OVER =45,      /*4枪直流输出过压*/ 
	
	
	
	/*BMS*/
	ECODE50_BMSFINISH = 50,   /*收到BMS停止报文,达到SOC设定值*/ 
	ECODE51_BMSTVOLT=51,      /*收到BMS停止报文,达到总电压设定值*/   
	ECODE52_BMSSVOLT=52,      /*收到BMS停止报文,达到单体电压设定值*/ 
	ECODE53_BMSCST  =53,      /*收到BMS停止报文,充电机主动终止*/ 

	ECODE55_BMSISO  = 55,      /*收到BMS绝缘故障报文*/ 
	ECODE56_BMSLKTMP= 56,      /*收到BMS输出连接器过温度故障*/ 
	ECODE57_BMSPARTTMP=57,     /*收到BMS元件故障*/ 
	ECODE58_BMSCLKERR =58,     /*收到BMS充电连接器故障*/
	ECODE59_BMSBATTMP = 59,    /*收到BMS电池组温度过高*/ 
	ECODE60_BMSKMERR  = 60,    /*收到BMS高压继电器故障*/ 
	ECODE61_BMSCHK2ERR= 61,    /*收到BMS检测点2电压检测故障*/ 
	ECODE62_BMSOTHERERR=62,    /*收到BMS其它故障*/ 
	ECODE64_BMSCURROVER=64,    /*收到BMS电流过大故障*/ 
	ECODE65_BMSVOLTERR = 65,   /*收到BMS电压异常故障*/ 
	
	ECODE67_BMSCRMTM  = 67,    /*BMS接收充电机CRM 00报文超时*/
	ECODE68_BMSCRMATM = 68,    /*BMS接收充电机CRM AA报文超时*/
	ECODE69_BMSCTSTM  = 69,    /*BMS接收充电机CTS和CML报文超时*/
	ECODE70_BMSCROTM  = 70,    /*BMS接收充电机CRO报文超时*/
	ECODE71_BMSCCSTM  = 71,    /*BMS接收充电机CCS报文超时*/
	ECODE72_BMSCSTTM  = 72,    /*BMS接收充电机CST报文超时*/
	ECODE73_BMSCSDTM  = 73,    /*BMS接收充电机CSD报文超时*/
	ECODE74_BMSST10MIN = 74 ,  /*充电暂停超过10分钟*/
	
	//SPN3521 充电机终止充电原因
	ECODE80_CHGFINISH = 80,    /*达到充电机设定条件中止*/
	ECODE81_CHGHUMST  = 81,    /*人工主动停止*/
	ECODE82_CHGERRST  = 82,    /*故障停止*/
	ECODE83_CHGBST    = 83,    /*收到BMS中止报文停止*/
	ECODE84_NOMONEY   = 84,    /*账户余额不足*/
	
	//SPN3522 充电机中止充电故障原因
	ECODE85_CHGTMOVER = 85,    /*充电机过温度故障*/
	ECODE86_CHGLKERR  = 86,    /*充电连接器故障*/
	ECODE87_CHGINTMPERR = 87,  /*充电机内部过温故障*/
	ECODE88_CHGENERGY = 88,    /*电量不能传送*/
  ECODE89_CHGJTST   = 89,    /*充电机急停故障*/
	ECODE90_CHGOTHER  = 90,    /*其它故障*/
	
	//SPN3523 充电机中止充电错误原因
	ECODE91_CHGCURRERR= 91,     /*充电电流不匹配*/
	ECODE92_CHGVOLTERR= 92,     /*充电机电压异常*/
	
	/*DC Meter*/
	ECODE93_METERCOMMERR=93,    /*直流表通讯故障*/
	ECODE94_CURROVER = 94,      /*直流输出过流*/  
	
	/*AC METER*******/
	ECODE95_ACMETCOMMERR = 95,   /*交流表通讯故障*/
	ECODE96_ACVOVERR  = 96,      /*交流输入过压*/
	ECODE97_ACUnerERR = 97,
	ECODE98_ACDEFICY = 98,       /*交流缺相*/
	ECODE99_DooracsERR = 99,
	ECODE100_DoorafERR = 100,    /*门禁 后门*/
	ECODE101_DCFAUSEERR = 101,   /*直流输出熔丝故障*/   //20181206 add
	ECODE102_SPDERR = 102,       /*防雷器故障*/         //20181206 add
	ECODE103_GUNOVERTEMPER = 103,     /*枪过温度故障*/
	ECODE104_PAVOUTSIDEREVERSE = 104,  /*枪外侧电压反接(BMS端)*/
	ECODE105_PAVINSIDEREVERSE = 105,   /*枪内侧电压反接(模块端)*/
	ECODE106_VINBACKTM = 106,   /*后台获取VIN信息超时*/
	ECODE107_BACKOFF = 107,   /*后台连接断开*/
	ECODE108_BCPOVERVOLT = 108, /*充电电压超过BCP报文最高允许值*/
}E_ERR_CODE;

typedef enum
{
	ENULL_REASON = 0, /*未知原因停止*/
	ECARD_REASON = 1, /*刷卡停止*/
	EAPP_REASON  = 2, /*移动客户端停止*/
	EERR_REASON  = 3, /*故障停止*/
	EAUTO_REASON = 4, /*达到设定条件*/
	EBMS_REASON  = 5, /*BMS主动停止*/
	ESYS_PWDOWN  = 6, /*系统掉电*/
	KEY_REASON  = 7,  /*触摸屏按键停止*/
	
}E_STOPREASON_CODE;

typedef struct {
	uint16 temper1;
	union {
		  uint32 word;
			uint8  byte[4];
			struct { 
			uint32	 stop    : 1;   /*0急停*/
			uint32	 acqf    : 1;   /*1交流断路器*/
			uint32	 ackm    : 1;   /*2交流接触器*/
			uint32	 spd     : 1;   /*3防雷器*/
			uint32	 posfuse : 1;   /*4正熔丝*/
			uint32	 negfuse : 1;   /*5负熔丝*/
			uint32	 dooracs : 1;   /*6门禁1*/	
			uint32	 dooraf  : 1;   /*7门禁2*/
			uint32	 fan     : 1;   /*8风机*/
			uint32	 smoke   : 1;   /*9烟雾告警*/
			uint32	 water   : 1;   /*10水浸告警*/
			uint32	 key1    : 1;   /*11拨码开关1*/
			uint32	 key2    : 1;   /*12拨码开关2*/
			uint32	 key3    : 1;   /*13拨码开关3*/
			uint32	 key4    : 1;   /*14拨码开关4*/
			uint32	 dckmp   : 1;   /*15 1#正直流接触器状态*/
			uint32   dckmn   : 1;   /*16 1#负直流接触器状态*/
			uint32	 dc2kmp   : 1;   /*17 2#正直流接触器状态*/
			uint32   dc2kmn   : 1;   /*18 2#负直流接触器状态*/	
			uint32   rsv     : 13;
	    }bits;
	}statu;
}DEV_INPUT_TYPE;

typedef struct {
	 uint16 u16temp;
	 union {
		uint16 word;
			uint8  byte[2];
			struct { 
				uint16 cc1stu     : 2; /* 1: cc1 4V; 2: cc1 6v; 3: cc1 12v */
				uint16 elockstu   : 1; /* 2 电子锁状态*/
		    uint16 elockctrl  : 1; /* 3 电子锁控制*/
				uint16 overtemper : 1; /* 4 过温度值*/
				uint16  kmpstu    : 1; /* 5 接触器正状态*/
				uint16  kmnstu    : 1; /* 6 接触器负状态*/
				uint16  bcpovervolt : 1; /* 7 测量电压超过BCP最大充电电压 */
				uint16 rsv        : 8;
	    }bits;
	}statu;
  int32 i32temper[2];

}DEV_GUN_TYPE;


/*检测CC1状态*/
extern uint8 Check_CC1Statu(uint8 gunNo);
/*检测电子锁状态*/
extern uint8 Check_ElockStatu(void);
/*检测K3K4状态*/
extern uint8 Check_K3K4Statu(void);
/*判断接触器外侧电压是否小于某个值*/
extern uint8 Check_KmOutVoltLess(uint8 gunNo,uint16 volt);
/*检测车辆电压是否与报文相差 <±5%*/
extern uint8 Check_CarVolt(uint8 gunNo);
/*检测接触器内侧电压是否在接触器外侧电压 10V以内*/
extern uint8 Check_KmInAndOut(uint8 gunNo);
/*检测车辆参数是否合适 */
extern uint8 Check_CarParam(uint8 gunNo);
/*检测直流输出接触器*/
extern uint8 Check_KmStatu(void);
/*或者绝缘检测输出电压*/
extern uint16 Check_SuitableVolt(uint8 gunNo);
/*充电枪对象数据*/
extern DEV_GUN_TYPE *Check_GetGunDataPtr(uint8 gunNo);



/*判断接触器内侧电压是否小于某个值*/
extern uint8 Check_KmInVoltLess(uint8 gunNo,uint16 volt);

extern uint8 Check_BcpToModSetVolt(uint8 gunNo);


/*判断模块输出电压是否与实际采样相符*/
extern uint8 Check_OutVoltIsOk(uint8 gunNo,uint16 volt);


extern uint8 Check_SetErrCode(uint8 gunNo,E_ERR_CODE errCode);

/*检查错误状态字是否设置*/
extern uint8 *GetgErrBitData(uint8 gunNo);

extern uint8 Check_GetErrCode(uint8 gunNo,E_ERR_CODE errCode);

extern uint8 Check_ClearErrBit(uint8 gunNo,E_ERR_CODE errCode );

extern uint8 Check_ClearAllErrBit(uint8 gunNo);

extern uint8 Check_GetErrBit(uint8 gunNo,uint8 *buf,uint8 bufsize,uint8 *num);

extern void SoftReset(void);

extern void resetInput(void);



extern void InitCC1ADCValue(uint8 Gunno);
extern uint8 Check_AllInputIo(void);
extern void Check_TranIOStatus(void);
extern void Check_stopBtn(void);
extern void Check_DialcodeBoard(void);
extern void Check_GunTemper(void);
extern void Check_BcpDCOverVolt(void);
extern void CheckGunFirstConnc(uint8 gunno);

#ifdef STM32F10X_HD

extern uint8 Check_N1_2_3_InputIo(void);

extern void Check_SoftRSTKeyBoard(void);
#endif

#endif

