#ifndef _CARDREADER_H
#define _CARDREADER_H

#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 
//功能：   
//作者：
//创建日期:2020/04/25
//版本：V1.0									  
//////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	uint8_t   cardtype; 			/*充电卡类型*/
	uint8_t   cardNo[20];     /* 充电卡号*/
	uint8_t   userPin[3];			/* 用户PIN码 */
	uint8_t   lockFlag;				/* 锁卡标志 */
	int32_t   money;					/* 卡余额 */
	uint8_t   key[20];				/* key码  */
	int32_t  backMoney;			/* 备份卡余额 */	
}CARD_INFO;

#pragma pack(1)
typedef  struct
{
	uint8_t sect0pwd[6];
	uint8_t sect1pwd[6];
	uint8_t sect2pwd[6];
	uint8_t keyCode[16];
	uint8_t defusrpin[8];
	uint8_t defautusrpwd[6];
}CARD_PARA;

typedef  struct
{
	CARD_PARA safe;
	//CARD_INFO info;
}CARD_PARAM_ST;

#pragma pack()

typedef  union
{
	uint8_t RfData[16];
	struct
	{
		uint8_t   ctype; /*类型*/
		uint32_t  addr;
		struct{
			uint8_t year;
			uint8_t mon;
			uint8_t date;
			uint8_t hour;
			uint8_t min;
			uint8_t sec;
		}tim;
	  uint32_t money;
	  uint8_t  sum;
	}rec;
}LAST_RECORD;


typedef struct
{
	uint8_t        u8Active;    /*0：A  1：B*/
	CARD_PARA    para;
	
	CARD_INFO    info;
	
	uint8_t        checkFg;
	
	uint8_t       lockFg;
	uint8_t       unLockFg;
	
	int32_t       unlockmoney;
	uint32_t       delaytime;
	uint8_t        lkcardNo[20];     /*解锁卡卡号*/
	
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
	E_USER_CARD = 0,    /*在线普通用户卡*/
	E_MANG_CARD = 1,	  /*设置管理卡*/ 
	E_SUPER_CARD = 2,   /*超级权限用户卡*/
	E_OFFLINE_USER_CARD = 3, /*离线普通用户卡，首字符是7*/
	
	
}E_RDCARD_TYPES;

#endif

