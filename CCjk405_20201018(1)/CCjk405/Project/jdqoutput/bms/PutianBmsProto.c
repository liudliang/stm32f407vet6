
/**
  ******************************************************************************
  * @file    PutianBmsProto.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/

#include "main.h"
#include "can.h"
#include "GbtBmsProto.h"
#include "PutianBms.h"
#include "BmsCan.h"
#include "bms.h"
#include "ChgData.h"

#define PT_RTN_SUCESS 1
#define PT_RTN_FAIL   0



const RCV_PROTO_ST gRcvPutianProtoList[] = \
{
	{0xEC,0xEC,8	,500,NULL   ,BmsAckPkgEC},     /*多包通知标志*/
  {0xEB,0xEB,8	,500,NULL   ,BmsAckPkgEB},     /*多包通知接收*/
  {0x30,0x02,49	,250,ProcBRM,NULL},     		   /*BRM*/
  {0x02,0xFF,49	,250,ProcBRM,NULL},      			 /*BRM*/
  {0x06,0xFF,13	,250,ProcPutianBCP,NULL},      /*BCP*/
	{0x09,0xFF,1	,250,ProcPutianBRO,NULL},      /*BRO*/
	{0x10,0xFF,5	,50, ProcPutianBCL,NULL},      /*BCL*/
	{0x11,0xFF,9	,50, ProcPutianBCS,NULL},      /*BCS*/
	{0x13,0xFF,7	,250,ProcPutianBSM,NULL},      /*BSM*/
	{0x15,0xFF,512,250,ProcBMV,NULL},      			 /*BMV*/
	{0x16,0xFF,128,250,ProcBMT,NULL},      			 /*BMT*/
	{0x17,0xFF,16	,250,ProcBSP,NULL},      			 /*BSP*/
	{0x19,0xFF,4	,10, ProcBST,NULL},      			 /*BST*/
  {0x1C,0xFF,7	,10, ProcBSD,NULL},      			 /*BSD*/
  {0x1E,0xFF,4	,250,ProcBEM,NULL},      			 /*BEM*/
  
	{0xff,0xFF,0xff,0xff,NULL,NULL},
};

SEND_PROTO_ST gChgPutianProtoList[] = \
{
  {SINGLE_FRM,CRICLE_DISALLOWED,0x01,6,8,WAITMS(250),250,0,ChgSendPutianPkgCRM},  /*CRM*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x07,6,7,WAITMS(500),500,0,ChgSendPkgCTS},  /*CTS*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x08,6,8,WAITMS(250),250,0,ChgSendPutianPkgCML},  /*CML*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x0A,5,1,WAITMS(250),250,0,ChgSendPutianPkgCRO},  /*CRO*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x14,6,8,WAITMS(50) ,50, 0,ChgSendPutianPkgCCS},  /*CCS*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x1A,5,5,WAITMS(10) ,10, 0,ChgSendPkgCST},  /*CST*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x1D,6,8,WAITMS(250),250,0,ChgSendPkgCSD},  /*CSD*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x1F,4,4,WAITMS(250),250,0,ChgSendPkgCEM},  /*CEM*/

  {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,NULL}, /*此项必须保留*/
};

void Putian_SendData(GSEND_INFO *sendinf,uint8 *pbuf,uint16 length);


//typedef enum {

//	BHM_CODE = 0x27,  VBI_PTCODE = 0x30,
//	BRM_CODE = 0x02,  BRM_PTCODE = 0x02,
//	BCP_CODE = 0x06,  BCP_PTCODE = 0x06,
//	BRO_CODE = 0x09,  BRO_PTCODE = 0x09,
//	BCL_CODE = 0x10,  BCL_PTCODE = 0x10,
//	BCS_CODE = 0x11,  BCS_PTCODE = 0x11,
//	BSM_CODE = 0x13,  BSM_PTCODE = 0x12,
//	BMV_CODE = 0x15,  BMV_PTCODE = 0x15,
//	BMT_CODE = 0x16,  BMT_PTCODE = 0x16,
//	BSP_CODE = 0x17,  BSOC_PTCODE= 0x17,
//	BST_CODE = 0x19,  BST_PTCODE = 0x19,
//	BSD_CODE = 0x1C,  BSD_PTCODE = 0x1C,
//	BEM_CODE = 0x1E,  BEM_PTCODE = 0x1E,
//}E_BMSCODE;


//typedef enum{
//	CHM_CODE = 0x26,
//	CRM_CODE = 0x01, CRM_PTCODE = 0x01,
//	CTS_CODE = 0x07, CTS_PTCODE = 0x07,
//	CML_CODE = 0x08, CML_PTCODE = 0x08,
//	CRO_CODE = 0x0A, CRO_PTCODE = 0x0A,
//	CCS_CODE = 0x12, CCS_PTCODE = 0x14,
//	CST_CODE = 0x1A, CST_PTCODE = 0x1A,
//	CSD_CODE = 0x1D, CSD_PTCODE = 0x1D,
//	CEM_CODE = 0x1F,CEM_PTCODE = 0x1F,
//}E_CHGCODE;


uint8 Putian_GbtCode_Trans(uint8 gbtcode)
{
	uint8 rtnCode = 0;
	switch(gbtcode){
		case CCS_CODE:
			rtnCode = CCS_PTCODE;
			break;
		case BSM_CODE:
			rtnCode = BSM_PTCODE;
		break;
		case BSP_CODE:
			rtnCode = BSOC_PTCODE;
		break;
		default:
     rtnCode = gbtcode;
     break;		
	}
	return rtnCode;
}

const RCV_PROTO_ST *Putian_GetRcvProcItem(uint8 pf)
{
	 uint8 i,ptPf;
	 const RCV_PROTO_ST *ptr = NULL;
	 for( i = 0 ; gRcvPutianProtoList[i].pgn != 0xff ; i++ ) {
		  ptPf = Putian_GbtCode_Trans(pf);
		   if( gRcvPutianProtoList[i].pgn == ptPf ) {
				  ptr = &gRcvPutianProtoList[i];
				  break;
			 }
	 }
	 return ptr;
}

SEND_PROTO_ST *Putian_SendProcItem(uint8 pf,uint8 gunNo) 
{
	uint8 i,ptPf;
  SEND_PROTO_ST *ptr = NULL;
	for( i = 0 ; gChgPutianProtoList[i].pgn != 0xff ; i++ ) {
		 ptPf = Putian_GbtCode_Trans(pf);
		 if ( gChgPutianProtoList[i].pgn == ptPf ) {
			  ptr = &gChgPutianProtoList[i];
			  break;
		 }
	}
	return ptr;
}


void Putian_Init(uint8 gunNo)
{
	  uint8 i;
	 	for( i = 0 ; gChgPutianProtoList[i].pgn != 0xff ; i++ ) {
		   gChgPutianProtoList[i].allowed = CRICLE_DISALLOWED;
			 gChgPutianProtoList[i].currticks = GetSystemTick();
		}
		CHGDATA_ST *pData = Bms_GetChgDataPtr(gunNo);
		pData->crm.idenf = 0;
		pData->cro.crostu = 0;
		
}

/*单条报文发送 设置SPN的值*/
uint8 Putian_ReportSingPkg(uint8 gunNo,uint8 pf,uint8 spn)
{
	uint8 rtn,ptPf;
	uint8 buf[10];
	uint16 len;
	uint16 i = 0;
	rtn = 0;
	GSEND_INFO ctrlinf ;
	SEND_PROTO_ST *pItem = NULL;
	for( i = 0 ;gChgPutianProtoList[i].frmtype != 0xff; i++)  {
		 ptPf = Putian_GbtCode_Trans(pf);
		 if( gChgPutianProtoList[i].pgn == ptPf ) {
			 pItem = &gChgPutianProtoList[i];
			 if( NULL != pItem->pkg ) {
				   memset(&ctrlinf,0,sizeof(GSEND_INFO));
				   ctrlinf.gunNo = gunNo;
					 ctrlinf.pf = pItem->pgn;
					 ctrlinf.pri = pItem->prio;
					 ctrlinf.srcAdr = PUTIAN_CHARGER_ADDR;
					 ctrlinf.objAdr = BMS_ADDR;
					 ctrlinf.frmNum = 1;
					 len = pItem->pkg(buf,(void *)&ctrlinf,gunNo);
				   if( len > 0 ) {  
							ctrlinf.srcAdr = PUTIAN_CHARGER_ADDR;
							Putian_SendData(&ctrlinf,buf,len);
							pItem->currticks  =  GetSystemTick();
						  rtn = len;
					 }
			  }
				break;
	   }
  }
	return rtn;
}


/* 所有报文发送周期关闭 */
void Putian_CtrlCricleAllclose(void)
{
	uint16 i = 0;
	
	for( i = 0 ;gChgPutianProtoList[i].frmtype != 0xff; i++){
		gChgPutianProtoList[i].allowed = CRICLE_DISALLOWED;
		gChgPutianProtoList[i].currticks = 0;
	}
}


/*报文周期发送*/
uint8 Putian_CricleReport(uint8 gunNo)
{
	 uint16 len = 0;
	 uint16 i = 0;
   GSEND_INFO ctrlinf ;
	 uint8 buf[10] = {0};
	 SEND_PROTO_ST *pItem = NULL;
	 for( i = 0 ;gChgPutianProtoList[i].frmtype != 0xff; i++)  {
		 if( gChgPutianProtoList[i].allowed == CRICLE_ALLOWED ) {
			 pItem = &gChgPutianProtoList[i];
			 if(GetSystemTick()- pItem->currticks >= pItem->cricle ) {
				 if( NULL != pItem->pkg ) {
					 memset(&ctrlinf,0,sizeof(GSEND_INFO));
					 ctrlinf.gunNo = gunNo;
					 ctrlinf.pf = pItem->pgn;
					 ctrlinf.pri = pItem->prio;
					 ctrlinf.srcAdr = PUTIAN_CHARGER_ADDR;
					 ctrlinf.objAdr = BMS_ADDR;
					 ctrlinf.frmNum = 1;
					 len = pItem->pkg(buf,(void *)&ctrlinf,gunNo);
					 if(len > 0 ) {
						  ctrlinf.frmNum = (len / 8 ) + ((len % 8) > 0);
							Putian_SendData(&ctrlinf,buf,len);
						  pItem->currticks  =  GetSystemTick();
              Delay5Ms(1);
					 }
				 }
			 }	 
		 }
	 }
	 return 0;
}

/*报文发送周期关闭*/
uint8 Putian_CtrlCricle(uint8 gunNo,uint8 pf,uint8 allow,uint8 fg)
{
	  uint8 ptPf = 0;
	  uint16 i;
	  for( i = 0 ;gChgPutianProtoList[i].frmtype != 0xff; i++)  {
			ptPf = Putian_GbtCode_Trans(pf);
			if( ptPf == gChgPutianProtoList[i].pgn ) {
				 gChgPutianProtoList[i].allowed = allow;
				 if( 0 == fg ) { 
					gChgPutianProtoList[i].currticks = GetSystemTick();
				 }else { /*立即发送*/
					 gChgPutianProtoList[i].currticks = 0;
				 }
				 break;
			}
	 }
	 return 0;
}


uint8 Putian_AckPkgFrame(void *ctrlinf,uint8 *buf,uint8 GunNo)
{
	 uint8 pf;
	 GSEND_INFO *pCtrl =(GSEND_INFO *)ctrlinf;
	 GSEND_INFO locInfo;
	 memcpy(&locInfo,pCtrl,sizeof(GSEND_INFO));
	 pf = pCtrl->pf;
	 if(pCtrl->tg == EC10_TG ) {
		 pf = pCtrl->ackpf;
	 }
	 const RCV_PROTO_ST *ptr = Putian_GetRcvProcItem(pf);
	 if( (NULL == ptr) || (NULL == ptr->ackpkg)) {
		  return 0;
	 } 
	 if( locInfo.pf == VBI_PTCODE ) {
		 locInfo.pf = BRM_PTCODE;
	 }
	 return (ptr->ackpkg(buf,&locInfo,GunNo));
}


#define MAX_MULITI_BUF_SIZE 100
static uint8 gMulitPkgBuf[DEF_MAX_GUN_NO][MAX_MULITI_BUF_SIZE];
/* 接收CAN链路帧 */
uint8 Putian_RcvDealFrame(uint8 GunNo,void *rtninfo,uint8 *pbuf)
{
	CAN_MSG Canmsg;
   
	static uint16 index[DEF_MAX_GUN_NO];
	static uint8 sMultifrmNo[DEF_MAX_GUN_NO];
  uint8 sMultiRcvOk = 0;
	static ACKBMS_ST  info[DEF_MAX_GUN_NO];
	
	uint16 len;
	GSEND_INFO *ctrlinf = (GSEND_INFO *)rtninfo;
	uint8 sbuf[20] = {0};
  PDU_HEAD_STRUCT *ptrPF = NULL;
  const RCV_PROTO_ST *ptrItem1 = NULL;
	
  memset(&Canmsg,0,sizeof(CAN_MSG));

	if(Bms_CanRead(&Canmsg,0,GunNo) > 0)
	{
		ptrPF = (PDU_HEAD_STRUCT *)(&Canmsg.Id.ExtId);
		if ( ptrPF->sbit.PS != PUTIAN_CHARGER_ADDR || ptrPF->sbit.SA != BMS_ADDR) { /*普天的桩地址为0xE5*/
				 return GBT_RTN_FAIL;
		}
	
		
		switch(ptrPF->sbit.PF)
		{
			case GBT_MULIT_FRAME:
			{
				memcpy(&info[GunNo],Canmsg.Data,8);
			 /*多包不符合标准，发拒绝接收多包*/
			 if(info[GunNo].tg != 0x10 || info[GunNo].length > info[GunNo].pkgnum*8 \
				 || info[GunNo].pkgnum > 20 ) {
					ctrlinf->tg = info[GunNo].tg;
					ctrlinf->frmNum = info[GunNo].pkgnum;
					ctrlinf->length = info[GunNo].length;

					ctrlinf->pf = info[GunNo].pf[1];
					ctrlinf->objAdr = BMS_ADDR;
					ctrlinf->pri = ptrPF->sbit.Pri;
					ctrlinf->srcAdr = PUTIAN_CHARGER_ADDR;

					ctrlinf->gunNo = GunNo;
					len = ChgSendPkgRefuseMuliti(sbuf,ctrlinf,0);
					ctrlinf->ackpf = GBT_MULIT_FRAME;
					 
					Putian_SendData(ctrlinf,sbuf,len);
					Delay5Ms(1);
				  return 0;
			 }
			 index[GunNo] = 0;
			 sMultiRcvOk = 0;
			 sMultifrmNo[GunNo] = 1;
			 ctrlinf->tg = info[GunNo].tg;
			 ctrlinf->frmNum = info[GunNo].pkgnum;
			 ctrlinf->length = info[GunNo].length;
			
			 ctrlinf->pf = info[GunNo].pf[1];
			 ctrlinf->objAdr = BMS_ADDR;
			 ctrlinf->pri = ptrPF->sbit.Pri;
			 ctrlinf->srcAdr = PUTIAN_CHARGER_ADDR;
			
			 ctrlinf->gunNo = GunNo;
			 ctrlinf->ackpf = GBT_MULIT_FRAME;
			 len = Putian_AckPkgFrame(ctrlinf,sbuf,GunNo);
			 ctrlinf->ackpf = GBT_MULIT_FRAME;
			 
			 Putian_SendData(ctrlinf,sbuf,len);
			 Delay5Ms(1);
			 return GBT_RTN_SUCESS;;
			}
			case GBT_MULIT_PACKAGE:
			{
				if( 1 == sMultifrmNo[GunNo]) {
					index[GunNo] = 0;
					len = 0;
					memset(gMulitPkgBuf[GunNo],0,100);
					if(info[GunNo].pf[1] == 0x30) info[GunNo].pf[1] = 0x02;
				}
				sMultifrmNo[GunNo]++;
				index[GunNo]  = Canmsg.Data[0] > 0 ? Canmsg.Data[0] : 1;
				Canmsg.DLC = Canmsg.DLC > 8 ? 8 : Canmsg.DLC;
				if( (index[GunNo] < (info[GunNo].pkgnum+1)) && (index[GunNo] < 20)) {
						memcpy(&gMulitPkgBuf[GunNo][((index[GunNo]-1)*7)%MAX_MULITI_BUF_SIZE],&Canmsg.Data[1],Canmsg.DLC-1);
				}
				if( sMultifrmNo[GunNo] > info[GunNo].pkgnum ) {
							sMultiRcvOk = 0x55;
							break;
				}
				break;
			}
			default:
			{
				ptrItem1 = Putian_GetRcvProcItem(ptrPF->sbit.PF);
				if( (NULL != ptrItem1) && (NULL != ptrItem1->proc)) {
						ptrItem1->proc((void *)ptrItem1,Canmsg.Data,GunNo);
					  return GBT_RTN_SUCESS;
				}
			  return GBT_RTN_ERRDATA;	
			}
		}// end case
		
		if(0x55 == sMultiRcvOk) {
			   sMultiRcvOk = 0;
				 const RCV_PROTO_ST *ptrItem = Putian_GetRcvProcItem(info[GunNo].pf[1]);
				 if( (NULL != ptrItem) && (NULL != ptrItem->proc)) {
					 ptrItem->proc((void *)ptrItem,gMulitPkgBuf[GunNo],GunNo);
					 
					 ctrlinf->tg = info[GunNo].tg;
					 ctrlinf->frmNum = info[GunNo].pkgnum;
					 ctrlinf->length = info[GunNo].length;
			
					 ctrlinf->pf = info[GunNo].pf[1];
					 ctrlinf->objAdr = ptrPF->sbit.SA;
					 ctrlinf->pri = ptrPF->sbit.Pri;
					 ctrlinf->srcAdr = ptrPF->sbit.PS;
			
			     ctrlinf->gunNo = GunNo;
			     ctrlinf->ackpf = GBT_MULIT_PACKAGE;
					 
					 len = Putian_AckPkgFrame(ctrlinf,sbuf,GunNo);
					 ctrlinf->ackpf = GBT_MULIT_FRAME;
					 
					 Putian_SendData(ctrlinf,sbuf,len);
					  Delay5Ms(1);
					 return GBT_RTN_SUCESS;
				}
		}
	} 
	return GBT_RTN_FAIL;
}


/* 发送CAN数据包 */
void Putian_SendData(GSEND_INFO *sendinf,uint8 *pbuf,uint16 length)
{
	CAN_MSG Txmsg = { 0 };
	PDU_HEAD_STRUCT *Pdu = NULL;
	uint8 index=4;
	uint8 TotalFrameNum = 1;//sendinf->frmNum;

	uint16 TxLen = length;// 需要发送的数据长度 
	
	memset(&Txmsg,0,sizeof(Txmsg));
	Pdu = (PDU_HEAD_STRUCT *)&Txmsg;
	Pdu->sbit.Pri = sendinf->pri;
	Pdu->sbit.DP = 0;
	Pdu->sbit.R = 0;
	Pdu->sbit.PS = sendinf->objAdr;
	Pdu->sbit.SA = sendinf->srcAdr;
	
	if( sendinf->ackpf != GBT_MULIT_FRAME ) {
		Pdu->sbit.PF = sendinf->pf;
	}else {
		Pdu->sbit.PF = sendinf->ackpf;
	}
	
	Txmsg.FrmDef.FF = CAN_FRAME_EXTERNED;
	Txmsg.FrmDef.RTR = CAN_DATA_TYPE_FRAME;

	index = 0;
	if (TotalFrameNum < 2)
	{/* 单帧 */	
		memcpy(Txmsg.Data,&pbuf[index],TxLen);
		Txmsg.DLC = TxLen;
		Bms_CanWrite(&Txmsg,sendinf->gunNo);
		Delay5Ms(1);
	}
	else
	{/* 多帧 */
		uint8 SendLen = 7;	/* 每次发送的数据长度 */
		for(uint8 FrameNo=1; FrameNo <= TotalFrameNum ; FrameNo++)
		{
			if(FrameNo == TotalFrameNum)
			{
				SendLen = TxLen%8;
			}
			Txmsg.Data[0] = FrameNo;
			memcpy(&Txmsg.Data[1],&pbuf[index],SendLen);
			Txmsg.DLC = SendLen+1;
			index += SendLen;
			TxLen -= SendLen;
			Bms_CanWrite(&Txmsg,sendinf->gunNo);
			Delay5Ms(1);
		}
	}
}

