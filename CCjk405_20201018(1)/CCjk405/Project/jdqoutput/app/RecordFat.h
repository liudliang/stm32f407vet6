/**
  ******************************************************************************
  * @file    RecordFat.h
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#ifndef __RECORDFAT_H_0001
#define __RECORDFAT_H_0001
#include "typedefs.h"
#include "BillingUnit.h"

#pragma pack(1)

typedef enum
{
	SST_IC = 0,
	FM_IC    = 1,
}E_STOREDEV;

typedef  struct
{
	uint16 head;
	CALCBILL_DATA_ST bill;
	uint16 crc;
}RECORD_ST;

typedef  struct
{
	uint16 head;
	uint8  nType;         /*存储类型*/
	uint16 tablrdno;      /*存储表记录号*/
	uint32 currrdno;      /*当前记录号*/
	uint32 totlrdno;      /*总记录号  */
	uint32 baktolrdno;    /*备份记录数*/
	uint16 crc;           /*校验和*/
}RD_PARA_ST;  /*该结构必须小与32byte ,否则会出问题*/

#pragma pack()

extern void Wdg_feeddog(void);
	
#define SINGLE_SECT_LEN							  (4096UL)	 /* 单个扇区长度 */

//#define MAX_SECT_NO_FULL_NUM     		40
//#define MAX_SECT_FULL_NUM        		200

#define SINGLE_RECORD_LEN			   			(sizeof(RECORD_ST))		/* 单条记录长度 */
//#define SINGLE_SECT_REORD_NUM		 			(SINGLE_SECT_LEN/SINGLE_RECORD_LEN)		/* 单个扇区存储记录的条数 *条 */

//#define MAX_NO_FULL_NUM	  			  		(MAX_SECT_NO_FULL_NUM * SINGLE_SECT_REORD_NUM)
//#define MAX_FULL_NUM	     			  		(MAX_SECT_FULL_NUM * SINGLE_SECT_REORD_NUM)

#define SINGLE_PARA_LEN						  	(sizeof(RD_PARA_ST))		/* 单条记录参数长度 */
#define MAX_RECORD_PARA_NUM				  	(SINGLE_SECT_LEN/SINGLE_PARA_LEN)		/* 最大的记录参数条数  1个扇区*/

#define SINGLE_SYS_PARA_LEN				   	sizeof(SET_PARA)	/* 单条系统参数长度 */ 
#define MAX_SYS_PARA_NUM					   	(SINGLE_SECT_LEN/SINGLE_SYS_PARA_LEN)		/* 最大的系统参数条数 1个扇区*/

#define ONLINE_SINGLE_PARA_LEN				(sizeof(ONLINE_RECORD_PARA))		/* 单条记录参数长度 */
#define ONLINE_MAX_RECORD_PARA_NUM		(SINGLE_SECT_LEN/ONLINE_SINGLE_PARA_LEN)		/* 最大的记录参数条数 */

#define ONLINE_MAX_SECT_NUM	          10
#define ONLINE_SINGLE_RECORD_LEN			(sizeof(ONLINE_BILL_STRUCT))													/* 单条记录长度 */
#define ONLINE_SINGLE_SECT_REORD_NUM	(SINGLE_SECT_LEN/ONLINE_SINGLE_RECORD_LEN) 					  /* 单个扇区存储记录条数 4096/180= 22.75 存储22条 */
#define ONLINE_MAX_RECORD_NUM					(ONLINE_MAX_SECT_NUM * ONLINE_SINGLE_SECT_REORD_NUM)	/* 存储记录总条数 */

/********************************************************************
参数包括：PARAM_DEV_TYPE   	设备参数  						   
          PARAM_OPER_TYPE  	运行参数  						 
          PARAM_FEE_TYPE   	费率参数(共1108字节）  
					存储区共4K byte空间
*********************************************************************/
/* 存储空间定义 */
#define STORE_HEAD                0xF5A5
#define STORE_USE_HEAD_ADDR				0x7FF000		  /* 首次初始化标志空间 */
#define STORE_USE_HEAD_FLAG				0xA5A5B5B4	  /* 首次初始化标志值 */

#define STORE_BASE_ADDR						(0x000000UL)	/* 存储基地址 */

/*参数存储区，各占一个扇区*/
#define SYS_DEV_PARA_ADDR					STORE_BASE_ADDR/* 系统参数存储区4KB */
#define SYS_DEV_PARA_SIZE					0x1000  /*1个扇区*/

#define SYS_OPER_PARA_ADDR				(SYS_DEV_PARA_ADDR+SYS_DEV_PARA_SIZE)/* 系统参数存储区4KB */
#define SYS_OPER_PARA_SIZE				0x1000  /*1个扇区*/

#define SYS_FEE_PARA_ADDR					(SYS_OPER_PARA_ADDR+SYS_OPER_PARA_SIZE)/* 系统参数存储区4KB */
#define SYS_FEE_PARA_SIZE					0x1000  /*1个扇区*/

/**/
#define RECORD_PARA_ADDR					(SYS_FEE_PARA_ADDR+SYS_FEE_PARA_SIZE)/* 记录控制参数存储区4KB*/
#define RECORD_PARA_SIZE					0x1000

#define RECORD_TEMP_ADDR					(RECORD_PARA_ADDR + RECORD_PARA_SIZE)/* 临时记录存储区 4KB*/
#define TEMP_SIZE									0x1000

#define RECORD_NO_FULL_ADDR				(RECORD_TEMP_ADDR + TEMP_SIZE) /* 未完整记录存储区  1.5MB*/
#define NO_FULL_SIZE							MAX_SECT_NO_FULL_NUM*0x1000

#define RECORD_FULL_ADDR					(RECORD_NO_FULL_ADDR + NO_FULL_SIZE) /* 完整记录存储区 2MB */
#define FULL_SIZE									MAX_SECT_FULL_NUM*0x1000

/*****************************联机记录宏定义*****************************************/
#define ONLINE_PARA_ADDR					(RECORD_FULL_ADDR + FULL_SIZE)/* 联机记录参数存储区 */
#define ONLINE_PARA_SIZE					0x1000

#define ONLINE_TEMP_ADDR					(ONLINE_PARA_ADDR+ONLINE_PARA_SIZE)/* 联机临时记录地址 */
#define ONLINE_TEMP_SIZE					 0x1000

#define ONLINE_RECORD_ADDR				(ONLINE_TEMP_ADDR+ONLINE_TEMP_SIZE)/* 联机记录存储区 */
#define ONLINE_RECORD_SIZE				(ONLINE_MAX_SECT_NUM*0x1000)
/*********************************END***********************************************/

/*********************************EEROM******************************************/
/*1Kbyte 一块，共8块 */
#define EFULL_RECORD                0x3A
#define ETEMP_RECORD                0x3B

/*记录管理区*/
#define  EEROM_TEMPPARATABL_ADR     0
#define  EEROM_TEMPBACK1TABL_ADR    32
#define  EEROM_FULLPARATABL_ADR     64
#define  EEROM_FULLBACK1TABL_ADR    96      

/*参数区*/
#define  EEROM_OPERPARA_ADR         (128 + 64)  /* 实际52字节  */
#define  EEROM_DEVPARA_ADR          (256 + 64)  /* 实际168字节 */
#define  EEROM_FEEPARA_ADR          (700 + 64)  /* 实际464字节 */ 

/*临时记录区*/
#define  EEROM_TEMPRD_ADR           1280 
#define  EEROM_TEMPRD_SIZE          2048
#define  MAX_TEMPRD_NUM             (EEROM_TEMPRD_SIZE / SINGLE_RECORD_LEN)     /*8条记录*/

#ifdef CP_PERIOD_FEE
#define  EEROM_CP_PEROID_FEE_ADR           (EEROM_TEMPRD_ADR+EEROM_TEMPRD_SIZE)  //3328
#define  EEROM_CP_PEROID_FEE_SIZE          149
#endif


//END 3328 (EEROM_CP_PEROID_FEE_ADR+EEROM_CP_PEROID_FEE_SIZE)

/*FMIC历史记录区*/
/*定位到了外部的SST存储器 一个记录长251 + 16字节*/
#define  EEROM_RECORD_ADR       		0
#define  EEROM_RECORD_SIZE      		(3072 * 1024)
#define  MAX_RECORD_NUM         		(EEROM_RECORD_SIZE / SINGLE_RECORD_LEN)    /*10000条记录*/

/*****************************************************************************/

/*
** 保存参数，保存的时gRunParam结构中的所有数据
*/
extern uint8 Rd_SaveSysParam(void);

/*
** 读参数，读gRunParam结构中的所有数据
*/
extern uint8 Rd_ReadSysParam(void);


/*初始化存储记录数据*/
extern void Rd_InitRecord(void);
extern void Rd_SaveRdCrlParam(RD_PARA_ST *SaveTabl) ;
extern uint16 Rd_ReadRdCtrlPara(RD_PARA_ST *SaveTabl);
extern uint16 Rd_SaveRecord(RD_PARA_ST *SaveTabl,CALCBILL_DATA_ST *bill);
/*读取指定记录号的记录，记录号1--n,当前记录为第一条为第一条记录，返回读取的记录号，0则未读取到*/
extern uint16 Rd_ReadCurrentRecord(uint8 rdno,CALCBILL_DATA_ST *bill);
extern uint8 Rd_InitFlashData(void);

/*******************E2ROM*************************************************/
extern int Rd_E2romSaveCtrlParam(uint8 devtype, RD_PARA_ST *E2romTabl,uint16 addr);
extern int Rd_E2romReadCtrlParam(uint8 devtype, RD_PARA_ST *E2romTabl,uint16 addr);
extern uint16 Rd_E2romSaveTempRecord(RD_PARA_ST *E2romTabl,CALCBILL_DATA_ST *bill);

extern uint16 Rd_E2romSaveFullRecord(RD_PARA_ST *E2romTabl,CALCBILL_DATA_ST *bill);

/*读取指定记录号的记录，记录号1--n，返回读取的记录号，0则未读取到*/
extern uint16 Rd_E2romReadRecord(uint16 absNo,CALCBILL_DATA_ST *bill);
extern uint16 Rd_SaveOneRecord(uint8 rdno,CALCBILL_DATA_ST *bill);

extern uint8 Rd_InitE2romData(void) ;
extern uint8 Rd_ClearE2romRecordData(void);

/*查询一条未上传记录*/
extern uint8 Rd_FindUnAccountRecord(CALCBILL_DATA_ST bill[]);

/*清除上传标志*/
extern uint8 Clear_RecordIsReport(uint8  *BillserialNo);
/*清除上传标志 修改记录参数*/
extern uint8 Clear_RecordIsReportBird(uint8  *BillserialNo,uint8 *buf);

/*读取未结算记录*/
extern uint16 Rd_E2romReadCurrTempRecord(RD_PARA_ST *E2romTabl,CALCBILL_DATA_ST *bill);


extern uint16 Rd_E2romTempToFullSave(uint8 gunNo);
extern uint16 Rd_E2romTempToFullBillBird(uint8 gunNo,CALCBILL_DATA_ST bill[]);

extern void Rd_E2romClearTempRecord(RD_PARA_ST *E2romTabl);

extern uint8 Rd_IsHaveTempRecord(void);

/*******************************************************/
extern uint8 Rd_SaveDevParam(uint8 devtype);

extern uint8 Rd_SaveOperParam(uint8 devtype);

extern uint16 Rd_SaveFeePara(uint8 devtype);

extern uint32 Rd_GetMaxRecord(void);

/*充之鸟程序升级*/
extern uint16 SST_Rd_E2romSaveUpData(uint16 num,uint8 *buf);
extern uint16 SST_Rd_E2romSaveUpDataHead(uint8 *buf,uint16 len);


#endif

