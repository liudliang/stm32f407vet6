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

/* Á¬½Ó×´Ì¬¶¨Òå */
typedef enum
{
	CONNECT_OK=0,			/* Á¬½ÓOK */
	CONNECT_OFF,			/* Á¬½Ó¶Ï¿ª */
}CONNECT_DEFINE;
/*ºóÌ¨Í¨Ñ¶Ğ­Òé**/
typedef enum
{	
	CONN_CheDian = 0, /* ³µµçÍøºóÌ¨ */
	CONN_Aunice,      /* °ÂÄÍºóÌ¨ */
	CONN_CHGBIRD,     /* ³äÖ®ÄñºóÌ¨ */    
	CONN_CHGZPLD,     /* ÕıÆ½Â¡µØºóÌ¨ */
	CONN_CHCP,        /* ³ÎÅôºóÌ¨ */
}Back_AgreeTYPE;

#define  SUPPORT_Agree  CONN_CheDian 
/* ÍøÂçĞ­Òé */
typedef struct
{
	void 	(*InitAgree)(void);/* ³õÊ¼»¯ÍøÂç */
	void  (*DealBackMsg)(MSG_STRUCT *msg);/* ´¦Àí½ÓÊÕÊı¾İ*/
	void  (*RunAgree)(void);/* ÈÎÎñÔËĞĞ */
}Back_Agreement;	

/* ºóÌ¨¿ØÖÆ½á¹¹ */
typedef struct
{
//	uint8 BackConStatus;				/* ³¢ÊÔÁ¬½Ó×´Ì¬ 0:¶Ï¿ª£¬·¢ËÍÁ¬½ÓĞÅºÅ 1:Á¬½Ó £¬ÒÑ¾­·¢ËÍĞÅºÅÁË  */
	uint8 AutoSndCmdFlag;				/* ×Ô¶¯·¢ËÍÃüÁî */
	uint8 CmdCode;							/* ÃüÁîÂë */
	uint8 CmdCnt;								/* ·¢ËÍÃüÁî´ÎÊı */
	uint8 CmdRrdCnt;						/* ¼ÇÂ¼·¢ËÍ´ÎÊı */
	uint8 CardType;							/* ¿¨ºÅÀàĞÍ 4£º°×Ãûµ¥ 5£ºÎ´Öª¿¨ */
	
	uint8  Runflag[DEV_GUNNO];					/* ºóÌ¨³äµç±êÖ¾ */
	uint8  AgreeType;					/* Í¨ĞÅÀàĞÍ */
	uint8  InterType;					/* Í¨ĞÅ½Ó¿Ú*/
	uint8  CommStatus;					/* Í¨ĞÅ×´Ì¬ */
	uint8  u8ShowCommStatus;
	uint16 errCount;
	uint32 CommTimers;					/* Í¨ĞÅ¶¨Ê±Æ÷ */
	
//	uint8  Stage;								/* Í¨Ñ¶Á¬½Ó½×¶Î */
	uint32 DealyTimers;					/* Í¨Ñ¶¸´Î»ÑÓÊ±¶¨Ê±Æ÷ */
	uint32 SerialNo;					/* ÉÏ´«¼ÇÂ¼ºÅ */
	uint32 RecordAddr;					/* ÉÏ´«¼ÇÂ¼µØÖ· */
}BACK_CTRL_STRUCT;

typedef struct BackRunPara
{  
	 uint32  UpCardNo;
	 uint8  Data[BACK_DATA_MAX+20];   //ÂÖÁ÷´¦Àí²»ÓÃ·Ö¿ª£¬Ö¡È«²¿ĞÅÏ¢	 
	 uint8  Commforbidden;  //Í¨Ñ¶½ûÓÃ 0£ºÕı³£    1£º½ûÓÃ£¬µÇÈëÍË³ö
//	 uint8  RecordAutoEN;  //ºóÌ¨Ö÷¶¯ÉÏ´«¼ÇÂ¼Ê¹ÄÜ£º0 ²»Ö÷¶¯ÉÏ´«£¬1Ö÷¶¯²éÑ¯ÉÏ´«
	 uint8  RecordCnt;     //Î´ÉÏ´«ºóÌ¨³äµç¼ÇÂ¼×ÜÊı
	 uint8  RecordMsg;
	 uint8  RcvbeatCnt;   /*ĞÄÌø°ü¼ÆÊı*/
//	 uint32  Recordno;     //Î´ÉÏ´«ºóÌ¨³äµç¼ÇÂ¼ºÅ
	 uint16 Len;           /*Ö¡×Ü³¤Êı¾İ³¤¶È*/
	
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

extern uint32 DevRunSec;     //Éè±¸ÔËĞĞ¶àÉÙÃ

/*ºóÌ¨²ÎÊı*/
extern BACK_CTRL_STRUCT g_BackCtrl;
extern  MSG_STRUCT Backmsg;
extern  NET_DATA_INTER NetHandler;/* µ×²ãÍøÂç¾ä±ú */
extern  void InitTCPData(void);
extern  uint8 TskBack_CommStatus(void);
extern  uint8 TskBack_CommStatusSet(uint8 staus);
/*ºóÌ¨ÈÎÎñ*/
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
