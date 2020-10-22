#ifndef __CARD_H
#define __CARD_H

#define CARD_COMM_TIMEOUT		100 	/* 通讯超时500ms */


/* 存储数据类型 */
typedef enum
{
	STORE_LOG = 0,	/* 最后一次充电记录 */
	STORE_PIN	= 1,	/* 用户密码 */
	
	STORE_IcNo = 2,	/* IC卡号 */
	STORE_LockFg = 3,		/* 灰锁标志 */
	STORE_Money = 4,		/* IC卡余额 */
	
	STORE_KEY = 5,	/* 卡KEY密钥 */
	STORE_BakMoney = 6,	/* IC卡备份余额 */
	
	STORE_ESAM = 7,	/* ESAM卡 */
}STROE_TYPE;

/* 操作IC响应 */
typedef enum
{
	RP_ERR=0,/* 通讯错误 */
	RP_NO_IC,/* 无卡 */
	RP_SUCC,/* 成功 */
	RP_FAIL,/* 失败 */
	RP_REP,/* 补充交易 */
}OPERATER_IC_RESULT;

/* 卡灰锁标志 */
typedef enum  
{
	IC_UNLOCK = 0x30,		// 正常
	IC_LOCK 	= 0x31,	 	// 灰锁 
	
	CARD_LOCKED_FG = 0x01,
	CARD_UNLOCKED_FG = 0x00,
}LOCK_CARD_STATUS;


/* 密码结构 */
typedef struct
{
	uint8 Pin[6];/* ESAM卡RF卡都有 */
	uint8 SectNo;/* 用于RF卡 */
	uint8 BlockNo;/* 用于RF卡*/
}CARD_PWD_STRUCT;

/* 扇区结构 */
typedef struct
{
	uint8 SectNo;
	uint8 BlockNo;
	uint8 StoreType;
}CARD_SINGLE_SECT_STRUCT;

typedef struct
{
	uint8 UserPin[3];/* 用户PIN码 */
	
	union
	{
		uint8 Data[16];
		struct
		{/* ESAM卡 */
			uint8 IcNo[8];/* IC卡号 */
			uint8 LockFlag;/* 锁卡标志 */
			int32 Money;	/* 卡余额 */
		}Esam;
		struct
		{/* RF卡 */
			uint8 IcNo[8];/* IC卡号 */
			uint8 LockFlag;/* 锁卡标志 */
			int32 Money;	/* 卡余额 */
		}Rf;
	}Comm;
	
	uint8 Key[16];/* key码  */
	uint32 BackMoney;	/* 备份卡余额 */
}IC_DATA;


typedef union
{
	uint8 RfData[16];
	struct
	{/* ESAM卡 */
		uint8 IcNo[8];/* IC卡号 */
		uint8 LockFlag;/* 锁卡标志 */
		uint32 Money;	/* 卡余额 */
	}Esam;
}IC_API_DATA;

/********************ESAM卡调用函数******************************/
/* 寻卡 */
extern uint8 ESAM_CheckCard(void);
//======================================
/* 校验密码 */
extern uint8 ESAM_VerfiyPwd(uint8 Type,IC_API_DATA *data);
/* 锁卡 */
extern uint8 ESAM_LockIc(void);
/* 解卡 */
extern uint8 ESAM_UnLockIc(uint32 Money,IC_API_DATA *data);
/* 补充交易 */
extern uint8 ESAM_ReplenDeal(IC_API_DATA *data);
/***********************END**************************************/



/********************RF卡调用函数******************************/
/* 寻卡 */
extern uint8 RF_CheckCard(void);
/* 校验密码 */
extern uint8 RF_VerfiySectPwd(uint8 Type,IC_API_DATA *data);
/* 读扇区命令 */
extern uint8 RF_ReadSectData(uint8 Type,IC_API_DATA *Data);
/* 写扇区命令 */
extern uint8 RF_WriteSectData(uint8 Type,IC_API_DATA *Data);
/* 减值 */
extern uint8 RF_MinusVaule(uint8 Type,uint32 Money);
/* 增值 */
extern uint8 RF_AddVaule(uint8 Type,uint32 Money);
/***********************END**************************************/


/*************************卡调用函数： 备注：根据数据结构不同，修改函数******************/
extern void CARD_GetIcSecrtPin(uint8 Type,uint8 *Data);




//extern CARD_PWD_STRUCT GetPinWithStoreType(uint8 Type);



/*检卡*/
extern uint8 RF_CheckChgCard(uint8 ComNo);


/*读取一个扇区的一块内容*/
extern uint8 RF_ReadSectorData(uint8 ComNo,CARD_PWD_STRUCT *Pwd,uint8 *data);//16byte


/*写一个扇区的一块内容*/
extern uint8 RF_WriteSectorData(uint8 ComNo,CARD_PWD_STRUCT *Pwd,uint8 *data);//16byte


/*RF减值*/
extern uint8 RF_DecMoney(uint8 ComNo,CARD_PWD_STRUCT *Pwd,int32 Money);








#endif
