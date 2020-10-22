/**
  ******************************************************************************
  * @file    TaskCardReader.h
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
*/
#ifndef _TASKCARDREADER_H_0001
#define _TASKCARDREADER_H_0001
#include "typedefs.h"



typedef struct
{
	uint8   cardtype; 			/*充电卡类型*/
	uint8   cardNo[20];     /* 充电卡号*/
	uint8   userPin[3];			/* 用户PIN码 */
	uint8   lockFlag;				/* 锁卡标志 */
	int32   money;					/* 卡余额 */
	uint8   key[20];				/* key码  */
	int32  backMoney;			/* 备份卡余额 */	
}CARD_INFO;

#pragma pack(1)
typedef  struct
{
	uint8 sect0pwd[6];
	uint8 sect1pwd[6];
	uint8 sect2pwd[6];
	uint8 keyCode[16];
	uint8 defusrpin[8];
	uint8 defautusrpwd[6];
}CARD_PARA;


typedef  struct
{
	CARD_PARA safe;
	//CARD_INFO info;
}CARD_PARAM_ST;

#pragma pack()



typedef  union
{
	uint8 RfData[16];
	struct
	{
		uint8   ctype; /*类型*/
		uint32  addr;
		struct{
			uint8 year;
			uint8 mon;
			uint8 date;
			uint8 hour;
			uint8 min;
			uint8 sec;
		}tim;
	  uint32 money;
	  uint8  sum;
	}rec;
}LAST_RECORD;




//typedef  struct
//{
//	LAST_RECORD lastRec;
//	uint8       usrpwd[3];
//	uint8       CardNo[8];
//	uint8       lockfg;
//	int32       money;
//	//uint8     keyCode[16];
//	int32       bakmoney;
//}CARD_INFO;



typedef struct
{
	uint8        u8Active;    /*0：A枪 1：B枪*/
	CARD_PARA    para;
	
	CARD_INFO    info;
	
	uint8        checkFg;
	
	uint8       lockFg;
	uint8       unLockFg;
	
	int32       unlockmoney;
	uint32       delaytime;
	uint8        lkcardNo[20];     /*解锁卡卡号*/
	
}CARD_OPER_DATA;

typedef enum
{
	ERR_NULL = 0,
	ERR_KEY = 1,
	ERR_USRPWD = 2,
	ERR_READ = 3,
	ERR_NOCARD = 4,
	ERR_ERRCARD = 5,
	ERR_MANGERCARD = 6,/*管理卡*/
}E_RTNCARDINFO;

typedef enum
{
	E_USER_CARD = 0,   //在线普通用户卡
	E_MANG_CARD = 1,	//设置管理卡 {0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x88};
	E_SUPER_CARD = 2,   //增加澄鹏的管理卡类型，不受余额限制 首字符是8，patli 20190926 add
	E_OFFLINE_USER_CARD = 3, //离线普通用户卡，首字符是7
	
	
}E_RDCARD_TYPES;

extern CARD_INFO *TskCard_GetCardInfPtr(void);




/*读卡器任务*/
extern void TaskCardReader(void *p_arg);


extern uint8 TskCReader_CpyLockCardNo(uint8 * CardNo);

extern void TskReader_Init(void);

//extern void CardReader_MsgProc(MSG_STRUCT *msg);

extern void TaskCardReader_main_proc(void);



#endif


