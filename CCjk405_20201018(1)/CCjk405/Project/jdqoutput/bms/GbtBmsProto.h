/**
  ******************************************************************************
  * @file    GbtBmsProto.h
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#ifndef _GBTBMSPROTO_H_0001
#define _GBTBMSPROTO_H_0001

#include "typedefs.h"


#define COMM_TIMEOUT	25		/* 通信超时250ms */ //test 8000 

#define BMS_ADDR		      	0xF4	/* BMS地址 */
#define CHARGER_ADDR				0x56	/* 充电控制器地址 */
#define PUTIAN_CHARGER_ADDR 0xE5  /*比亚迪E6（普天协议）充电桩地址*/


#define GBT_RTN_SUCESS 1
#define GBT_RTN_FAIL   0
#define GBT_RTN_ERRDATA  0x50
#define GBT_RTN_OTHER    0x55

#define GBT_MULIT_FRAME   0xEC
#define GBT_MULIT_PACKAGE 0xEB

#define CRICLE_ALLOWED     1
#define CRICLE_DISALLOWED  0


#define OLD_GBT_FG   0xEE   /*老国标SPN定义*/

typedef enum
{
	SINGLE_FRAME = 0,
	DOUBLE_FRAME = 1,
}FRAME_TYPE_DEF;


typedef enum
{
	EC10_TG = 0x10, //多包请求 车发
	EC11_TG = 0x11, //多包请求应答 桩发
	EC13_TG = 0x13, //多包接受完   桩发
	
}E_ECTG;

typedef enum {
	BHM_CODE = 0x27,
	BRM_CODE = 0x02,
	BCP_CODE = 0x06,
	BRO_CODE = 0x09,
	BCL_CODE = 0x10,
	BCS_CODE = 0x11,
	BSM_CODE = 0x13,
	BMV_CODE = 0x15,
	BMT_CODE = 0x16,
	BSP_CODE = 0x17,
	BST_CODE = 0x19,
	BSD_CODE = 0x1C,
	BEM_CODE = 0x1E,
}E_BMSCODE;


typedef enum{
	CHM_CODE = 0x26,
	CRM_CODE = 0x01,
	CTS_CODE = 0x07,
	CML_CODE = 0x08,
	CRO_CODE = 0x0A,
	CCS_CODE = 0x12,
	CST_CODE = 0x1A,
	CSD_CODE = 0x1D,
	CEM_CODE = 0x1F,
}E_CHGCODE;



typedef enum
{
	SPN_READYOK = 0xAA,
	SPN_READYNO= 0x00,
	
}E_SPNCODE;

typedef enum
{
	BRO_READYOK = 0xAA,
	BRO_READYNO= 0x00,
}E_BROCODE;

typedef enum
{
	CRO_READYOK = 0xAA,
	CRO_READYNO= 0x00,
}E_CROCODE;

/* PDU数据头定义 */
typedef union
{
	uint32 ExtId;/* 扩展帧标识符 */
	struct
	{
		uint32 SA:8;/* 源地址 */
		uint32 PS:8;/* 目的地址 */
		uint32 PF:8;/* 报文类型 */
		uint32 DP:1;/* 数据页 */
		uint32 R:1;/* 保留位 */
		uint32 Pri:3;/* 优先级 */
		uint32 Reserve:3;/* 保留：默认为：0*/
	}sbit;
}PDU_HEAD_STRUCT;



typedef struct FRM_FORMAT
{
	PDU_HEAD_STRUCT head;
	uint8  data[1];
}FRAME_FORMAT;



typedef enum  EFRMDIR
{
	CTRL_ACK_FRM = 0,
	CTRL_M_CMD_FRM = 1,
	CTRL_M_STATU_FRM = 2,
	CTRL_M_DATA_FRM = 3,
	CTRL_M_ERR_FRM = 4,
	CTRL_M_HEARTBEAT_FRM = 5,
	
	TCU_ACK = 6,
	TCU_MSEND = 7,
	TCU_M_HEARTBEAT_FRM = 8,
}E_FRMDIR;


typedef enum EFRMTYPE
{
	 SINGLE_FRM = 0,
	 MULTI_FRM = 1,
	 RCVERR_FRM = 0x55,
	
}E_FRMTYPE;


#define PROTO_GBT    0x11
#define PROTO_PT     0x12

typedef struct RCV_PROTOCOL
{
	uint8  pgn;       //PGN
	uint8  ackpgn;    //应答PGN
	uint16 bytes;     //字节数
	uint16 timeout;   //超时 s
	void   (*proc)(void *item,uint8 *data,uint8 GunNo);  //接收函数
  uint8  (*ackpkg)(uint8 *buf,void* st,uint8 GunNo);	 //接收包立即应答函数
}RCV_PROTO_ST;

typedef struct SEND_PROTOCOL
{
	uint8  frmtype;   //帧类型  多帧 、单帧
	uint8  allowed;   /*过程中可被修改*/
	uint8  pgn;        //PGN
	uint8  prio;       /*优先权*/
	uint16 bytes;     //字节数
	uint16 cricle;
	uint16 timeout;   //超时 s
	uint32 currticks;
	uint8 (*pkg)(uint8 *data,void *ctrlinfo,uint8 GunNo);      
}SEND_PROTO_ST;

/**************************需要字节对齐的结构******************************/
#pragma pack(1)

#define MAX_DATABY_NUM  100
typedef  struct
{
	uint8   ps;
	uint8   sa;
	uint8   frmTag;
	uint8   pri;
	uint32  pgn;
	uint16  dlen;
	uint8   data[MAX_DATABY_NUM];
}SFRM_INFO;


typedef  struct
{
	 uint8  tg;
	 uint16 length;
	 uint8  pkgnum;
	 uint8  rsvFF;
	 uint8  pf[3];
}ACKBMS_ST;

#pragma pack()

/*******************************************************************/



#define MAX_MULITI_BUF_SIZE 100
typedef struct
{
	uint8  mulitifrmNo;
	uint16 index;
	ACKBMS_ST info;
	uint8  multibuf[MAX_MULITI_BUF_SIZE];
}PROC_CTRL_ST;


/*注意本结构体添加成员必须在后边添加*/
typedef struct
{
	uint8 gunNo; 
	uint8 pri;   		/*优先级*/
	uint8 pf;
	uint8 objAdr;
	uint8 srcAdr;
	uint8 spn;
	uint8  tg;      /*多帧的标志*/
	uint8 ackpf;
	uint8  frmNum;
	uint8  otherFg;  /*20181207 特殊标志，暂时在CHM使用*/
	uint16 length;
	PROC_CTRL_ST ctrl;
          
}GSEND_INFO;

#ifdef BMS_USE_TIMER

extern SEND_PROTO_ST gChgProtoListA_500ms[];

extern SEND_PROTO_ST gChgProtoListA_250ms[];

extern SEND_PROTO_ST gChgProtoListA_50ms[]; 

extern SEND_PROTO_ST gChgProtoListA_10ms[]; 

extern SEND_PROTO_ST gChgProtoListB_500ms[];

extern SEND_PROTO_ST gChgProtoListB_250ms[];

extern SEND_PROTO_ST gChgProtoListB_50ms[]; 

extern SEND_PROTO_ST gChgProtoListB_10ms[]; 


#endif

/*****************************function****************************************************/
extern void Gbt_Init(uint8 gunNo);
/*报文接收处理函数*/
extern uint8 Gbt_RcvDealFrame(uint8 GunNo,void *rtninfo,uint8 *pbuf);
/*周期报文发送函数*/
extern uint8 Gbt_CricleReport(uint8 gunNo);
/*单条报文发送函数*/
extern uint8 Gbt_ReportSingPkg(uint8 gunNo,uint8 pf,uint8 spn);
/*周期报文发送控制*/
extern uint8 Gbt_CtrlCricle(uint8 gunNo,uint8 pf,uint8 allow,uint8 fg);
/* 所有报文发送周期关闭 */
extern void Gbt_CtrlCricleAllclose(uint8 gunNo);

#endif



