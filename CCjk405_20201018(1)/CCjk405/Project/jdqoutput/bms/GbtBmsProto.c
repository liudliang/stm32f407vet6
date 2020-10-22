/**
  ******************************************************************************
  * @file    GbtBmsProto.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/

#include "main.h"
#include "can.h"
#include "GbtBmsProto.h"
#include "BmsCan.h"
#include "bms.h"
#include "ChgData.h"
#include "Adebug.h"
void Gbt_SendData(GSEND_INFO *sendinf,uint8 *pbuf,uint16 length);

const RCV_PROTO_ST gRcvProtoList[] = \
{
	{0xEC, 0xEC, 8	,  1000, NULL   , BmsAckPkgEC},     /*多包通知标志*/
  {0xEB, 0xEB, 8	,  1000, NULL   , BmsAckPkgEB},     /*多包通知接收*/
  {0x27, 0xFF, 2	,  1000, ProcBHM, NULL},      /*BHM*/
  {0x02, 0xFF, 49	,  1000, ProcBRM, NULL},      /*BRM*/
  {0x06, 0xFF, 13	,  1000, ProcBCP, NULL},      /*BCP*/
	{0x09, 0xFF, 1	,  1000, ProcBRO, NULL},      /*BRO*/
	{0x10, 0xFF, 5	,  1000, ProcBCL, NULL},      /*BCL*/
	{0x11, 0xFF, 9	,  1000, ProcBCS, NULL},      /*BCS*/
	{0x13, 0xFF, 7	,  1000, ProcBSM, NULL},      /*BSM*/
	{0x15, 0xFF, 512,  1000, ProcBMV, NULL},      /*BMV*/
	{0x16, 0xFF, 128,  1000, ProcBMT, NULL},      /*BMT*/
	{0x17, 0xFF, 16	,  1000, ProcBSP, NULL},      /*BSP*/
	{0x19, 0xFF, 4	,  1000, ProcBST, NULL},      /*BST*/
  {0x1C, 0xFF, 7	,  1000, ProcBSD, NULL},      /*BSD*/
  {0x1E, 0xFF, 4	,  1000, ProcBEM, NULL},      /*BEM*/
  
	{0xff, 0xFF, 0xff, 0xff,NULL,NULL},
};

#ifdef BMS_USE_TIMER
SEND_PROTO_ST gChgProtoListA_500ms[] = 
{
  	{SINGLE_FRM,CRICLE_DISALLOWED,CTS_CODE,6,7,WAITMS(500),1000,0,ChgSendPkgCTS},  /*CTS*/

  	{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,NULL}, /*此项必须保留*/
};

SEND_PROTO_ST gChgProtoListA_250ms[] = 
{
	{SINGLE_FRM,CRICLE_DISALLOWED,CHM_CODE,6,3,WAITMS(250),1000,0,ChgSendPkgCHM},  /*CHM*/ 
  	{SINGLE_FRM,CRICLE_DISALLOWED,CRM_CODE,6,8,WAITMS(250),1000,0,ChgSendPkgCRM},  /*CRM*/
  	{SINGLE_FRM,CRICLE_DISALLOWED,CML_CODE,6,8,WAITMS(250),1000,0,ChgSendPkgCML},  /*CML*/
  	{SINGLE_FRM,CRICLE_DISALLOWED,CRO_CODE,4,1,WAITMS(250),1000,0,ChgSendPkgCRO},  /*CRO*/
  	{SINGLE_FRM,CRICLE_DISALLOWED,CEM_CODE,4,4,WAITMS(250),1000,0,ChgSendPkgCEM},  /*CEM*/

  	{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,NULL}, /*此项必须保留*/
};

SEND_PROTO_ST gChgProtoListA_50ms[] = 
{
  	{SINGLE_FRM,CRICLE_DISALLOWED,CCS_CODE,6,7,WAITMS(50), 1000,0,ChgSendPkgCCS},  /*CCS*/

  	{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,NULL}, /*此项必须保留*/
};

SEND_PROTO_ST gChgProtoListA_10ms[] = 
{
  	{SINGLE_FRM,CRICLE_DISALLOWED,CST_CODE,4,5,WAITMS(10), 1000,0,ChgSendPkgCST},  /*CST*/
  	{SINGLE_FRM,CRICLE_DISALLOWED,CSD_CODE,6,8,WAITMS(10), 1000,0,ChgSendPkgCSD},  /*CSD*/

  	{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,NULL}, /*此项必须保留*/
};

SEND_PROTO_ST gChgProtoListB_500ms[] = 
{
  	{SINGLE_FRM,CRICLE_DISALLOWED,CTS_CODE,6,7,WAITMS(500),1000,0,ChgSendPkgCTS},  /*CTS*/

  	{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,NULL}, /*此项必须保留*/
};

SEND_PROTO_ST gChgProtoListB_250ms[] = 
{
	{SINGLE_FRM,CRICLE_DISALLOWED,CHM_CODE,6,3,WAITMS(250),1000,0,ChgSendPkgCHM},  /*CHM*/ 
  	{SINGLE_FRM,CRICLE_DISALLOWED,CRM_CODE,6,8,WAITMS(250),1000,0,ChgSendPkgCRM},  /*CRM*/
  	{SINGLE_FRM,CRICLE_DISALLOWED,CML_CODE,6,8,WAITMS(250),1000,0,ChgSendPkgCML},  /*CML*/
  	{SINGLE_FRM,CRICLE_DISALLOWED,CRO_CODE,4,1,WAITMS(250),1000,0,ChgSendPkgCRO},  /*CRO*/
  	{SINGLE_FRM,CRICLE_DISALLOWED,CEM_CODE,4,4,WAITMS(250),1000,0,ChgSendPkgCEM},  /*CEM*/

  	{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,NULL}, /*此项必须保留*/
};

SEND_PROTO_ST gChgProtoListB_50ms[] = 
{
  	{SINGLE_FRM,CRICLE_DISALLOWED,CCS_CODE,6,7,WAITMS(50), 1000,0,ChgSendPkgCCS},  /*CCS*/

  	{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,NULL}, /*此项必须保留*/
};

SEND_PROTO_ST gChgProtoListB_10ms[] = 
{
  	{SINGLE_FRM,CRICLE_DISALLOWED,CST_CODE,4,5,WAITMS(10), 1000,0,ChgSendPkgCST},  /*CST*/
  	{SINGLE_FRM,CRICLE_DISALLOWED,CSD_CODE,6,8,WAITMS(10), 1000,0,ChgSendPkgCSD},  /*CSD*/

  	{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,NULL}, /*此项必须保留*/
};


#endif


//周期主动上报给车
SEND_PROTO_ST gChgProtoListA[] = \
{
	{SINGLE_FRM,CRICLE_DISALLOWED,0x26,6,3,WAITMS(250),1000,0,ChgSendPkgCHM},  /*CHM*/ 
  {SINGLE_FRM,CRICLE_DISALLOWED,0x01,6,8,WAITMS(250),1000,0,ChgSendPkgCRM},  /*CRM*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x07,6,7,WAITMS(500),1000,0,ChgSendPkgCTS},  /*CTS*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x08,6,8,WAITMS(250),1000,0,ChgSendPkgCML},  /*CML*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x0A,4,1,WAITMS(250),1000,0,ChgSendPkgCRO},  /*CRO*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x12,6,7,WAITMS(50-7), 1000,0,ChgSendPkgCCS},  /*CCS(减去定时误差)*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x1A,4,5,WAITMS(10-5), 1000,0,ChgSendPkgCST},  /*CST(减去定时误差)*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x1D,6,8,WAITMS(10), 1000,0,ChgSendPkgCSD},  /*CSD*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x1F,2,4,WAITMS(250),1000,0,ChgSendPkgCEM},  /*CEM*/

  {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,NULL}, /*此项必须保留*/
};

SEND_PROTO_ST gChgProtoListB[] = \
{
	{SINGLE_FRM,CRICLE_DISALLOWED,0x26,6,3,WAITMS(250),1000,0,ChgSendPkgCHM},  /*CHM*/ 
  {SINGLE_FRM,CRICLE_DISALLOWED,0x01,6,8,WAITMS(250),1000,0,ChgSendPkgCRM},  /*CRM*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x07,6,7,WAITMS(500),1000,0,ChgSendPkgCTS},  /*CTS*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x08,6,8,WAITMS(250),1000,0,ChgSendPkgCML},  /*CML*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x0A,4,1,WAITMS(250),1000,0,ChgSendPkgCRO},  /*CRO*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x12,6,7,WAITMS(50-7), 1000,0,ChgSendPkgCCS},  /*CCS(减去定时误差)*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x1A,4,5,WAITMS(10-5), 1000,0,ChgSendPkgCST},  /*CST(减去定时误差)*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x1D,6,8,WAITMS(10), 1000,0,ChgSendPkgCSD},  /*CSD*/
  {SINGLE_FRM,CRICLE_DISALLOWED,0x1F,2,4,WAITMS(250),1000,0,ChgSendPkgCEM},  /*CEM*/

  {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,NULL}, /*此项必须保留*/
};

SEND_PROTO_ST * GetProtoListPtr(uint8 gunNo) 
{
	SEND_PROTO_ST *ptr = NULL;
	if(gunNo == AGUN_NO ) {
		ptr = &gChgProtoListA[0];
	}else {
		ptr = &gChgProtoListB[0];
	}
	return ptr;
}



const RCV_PROTO_ST *Gbt_GetRcvProcItem(uint8 pf)
{
	 uint8 i;
	 const RCV_PROTO_ST *ptr = NULL;
	 for( i = 0 ; gRcvProtoList[i].pgn != 0xff ; i++ ) {
		   if( gRcvProtoList[i].pgn == pf ) {
				  ptr = &gRcvProtoList[i];
				  break;
			 }
	 }
	 return ptr;
}

SEND_PROTO_ST *Gbt_SendProcItem(uint8 pf,uint8 gunNo) 
{
	uint8 i;
  SEND_PROTO_ST *ptr = NULL;
	SEND_PROTO_ST *ptrList = GetProtoListPtr(gunNo);
	
	for( i = 0 ; ptrList[i].pgn != 0xff ; i++ ) {
		 if ( ptrList[i].pgn == pf ) {
			  ptr = &ptrList[i];
			  break;
		 }
	}
	return ptr;
}

void Gbt_Init(uint8 gunNo)
{
	  uint8 i;
	  SEND_PROTO_ST *ptrList = GetProtoListPtr(gunNo);
	
	 	for( i = 0 ; ptrList[i].pgn != 0xff ; i++ ) {
		   ptrList[i].allowed = CRICLE_DISALLOWED;
			 ptrList[i].currticks = GetSystemTick();
		}
		CHGDATA_ST *pData = Bms_GetChgDataPtr(gunNo);
		pData->crm.idenf = 0;
		pData->cro.crostu = 0;
		
}

#ifdef BMS_USE_TIMER
void BmsSingReport(int8 gunNo, SEND_PROTO_ST *ptrList, uint8 pf, uint8 spn)
{	
	uint16 len = 0;
	uint16 i = 0;
	GSEND_INFO ctrlinf ;
	uint8 buf[50] = {0}, err;
	SEND_PROTO_ST *pItem = NULL;
	
	//请求互斥量
//	OSMutexPend(tm_bms_mutex,0,&err);   //patli 20191203

	for(i = 0;ptrList[i].frmtype != 0xff; i++) {
		if( ptrList[i].pgn == pf ) {
			pItem = &ptrList[i];
			if( NULL != pItem->pkg ) {
				memset(&ctrlinf,0,sizeof(GSEND_INFO));
				ctrlinf.gunNo = gunNo;
				ctrlinf.pf = pItem->pgn;
				ctrlinf.pri = pItem->prio;
				ctrlinf.srcAdr = CHARGER_ADDR;
				ctrlinf.objAdr = BMS_ADDR;
				ctrlinf.frmNum = 1;
				if(spn == OLD_GBT_FG ) {
					  ctrlinf.otherFg = OLD_GBT_FG;
				  }
				
				len = pItem->pkg(buf,(void *)&ctrlinf,gunNo);
				if(len > 0 ) {
					ctrlinf.frmNum = (len / 8 ) + ((len % 8) > 0);
					Gbt_SendData(&ctrlinf,buf,len);
				}
			}

			break;
		}
	}

	//释放互斥量
//	OSMutexPost(tm_bms_mutex);   //patli 20191203	
}
#endif

/*单条报文发送 设置SPN的值*/
uint8 Gbt_ReportSingPkg(uint8 gunNo,uint8 pf,uint8 spn)
{
  uint8 rtn = 0;

	uint8 buf[10];
	uint16 len;
	uint16 i = 0;
	rtn = 0;
	GSEND_INFO ctrlinf ;
	SEND_PROTO_ST *pItem = NULL;
  SEND_PROTO_ST *ptrList = GetProtoListPtr(gunNo);
	
	for( i = 0 ;ptrList[i].frmtype != 0xff; i++)  {
		 if( ptrList[i].pgn == pf ) {
			  pItem = &ptrList[i];
			 if( NULL != pItem->pkg ) {
				   memset(&ctrlinf,0,sizeof(GSEND_INFO));
				   ctrlinf.gunNo = gunNo;
					 ctrlinf.pf = pItem->pgn;
					 ctrlinf.pri = pItem->prio;
					 ctrlinf.srcAdr = CHARGER_ADDR;
					 ctrlinf.objAdr = BMS_ADDR;
					 ctrlinf.frmNum = 1;
				   if(spn == OLD_GBT_FG ) {
						 ctrlinf.otherFg = OLD_GBT_FG;
					 }
					 len = pItem->pkg(buf,(void *)&ctrlinf,gunNo);
				   if( len > 0 ) {  
							ctrlinf.srcAdr = CHARGER_ADDR;
							Gbt_SendData(&ctrlinf,buf,len);
							pItem->currticks  =  GetSystemTick();
						  rtn = len;
					 }
			  }
				break;
	   }
  }

#ifdef BMS_USE_TIMER
	if(gunNo == AGUN_NO)
	{
	 	BmsSingReport(gunNo,gChgProtoListA_500ms, pf, spn);
	 	BmsSingReport(gunNo,gChgProtoListA_250ms, pf, spn);
	 	BmsSingReport(gunNo,gChgProtoListA_50ms, pf, spn);
	 	BmsSingReport(gunNo,gChgProtoListA_10ms, pf, spn);
	}
	else
	{		
	 	BmsSingReport(gunNo,gChgProtoListB_500ms, pf, spn);
	 	BmsSingReport(gunNo,gChgProtoListB_250ms, pf, spn);
	 	BmsSingReport(gunNo,gChgProtoListB_50ms, pf, spn);
	 	BmsSingReport(gunNo,gChgProtoListB_10ms, pf, spn);
	}
#else	
  uint8 rtn;
  uint8 buf[10];
  uint16 len;
  uint16 i = 0;
  rtn = 0;
  GSEND_INFO ctrlinf ;
  SEND_PROTO_ST *pItem = NULL;
SEND_PROTO_ST *ptrList = GetProtoListPtr(gunNo);
  
  for( i = 0 ;ptrList[i].frmtype != 0xff; i++)	{
	   if( ptrList[i].pgn == pf ) {
			pItem = &ptrList[i];
		   if( NULL != pItem->pkg ) {
				 memset(&ctrlinf,0,sizeof(GSEND_INFO));
				 ctrlinf.gunNo = gunNo;
				   ctrlinf.pf = pItem->pgn;
				   ctrlinf.pri = pItem->prio;
				   ctrlinf.srcAdr = CHARGER_ADDR;
				   ctrlinf.objAdr = BMS_ADDR;
				   ctrlinf.frmNum = 1;
				 if(spn == OLD_GBT_FG ) {
					   ctrlinf.otherFg = OLD_GBT_FG;
				   }
				   len = pItem->pkg(buf,(void *)&ctrlinf,gunNo);
				 if( len > 0 ) {  
						  ctrlinf.srcAdr = CHARGER_ADDR;
						  Gbt_SendData(&ctrlinf,buf,len);
						  pItem->currticks	=  GetSystemTick();
						rtn = len;
				   }
			}
			  break;
	 }
}

#endif

	
	return rtn;
}

/*报文周期发送*/
uint8 Gbt_CricleReport(uint8 gunNo)
{
	 uint16 len = 0;
	 uint16 i = 0;
   GSEND_INFO ctrlinf ;
	 uint8 buf[50] = {0};
	 SEND_PROTO_ST *pItem = NULL;
   SEND_PROTO_ST *ptrList = GetProtoListPtr(gunNo);

	 for(i = 0;ptrList[i].frmtype != 0xff; i++)  {
		 if( ptrList[i].allowed == CRICLE_ALLOWED ) {
			 pItem = &ptrList[i];
			 if(GetSystemTick()- pItem->currticks >= pItem->cricle) {
				 if( NULL != pItem->pkg ) {
					 memset(&ctrlinf,0,sizeof(GSEND_INFO));
					 ctrlinf.gunNo = gunNo;
					 ctrlinf.pf = pItem->pgn;
					 ctrlinf.pri = pItem->prio;
					 ctrlinf.srcAdr = CHARGER_ADDR;
					 ctrlinf.objAdr = BMS_ADDR;
					 ctrlinf.frmNum = 1;
					 len = pItem->pkg(buf,(void *)&ctrlinf,gunNo);
					 if(len > 0 ) {
						  ctrlinf.frmNum = (len / 8 ) + ((len % 8) > 0);
							Gbt_SendData(&ctrlinf,buf,len);
						  pItem->currticks  =  GetSystemTick();
              Delay5Ms(1);
					 }
				 }
			 }	 
		 }
	 }
	 return 0;
}



/*报文发送周期关闭 fg=1 立即启动第一包发送*/
uint8 Gbt_CtrlCricle(uint8 gunNo,uint8 pf,uint8 allow,uint8 fg)
{
	  uint16 i = 0;  
		
#ifdef BMS_USE_TIMER
	if(gunNo == AGUN_NO)
	{
	 	for( i = 0 ;gChgProtoListA_500ms[i].frmtype != 0xff; i++)  {
		   if( pf == gChgProtoListA_500ms[i].pgn ) {
				gChgProtoListA_500ms[i].allowed = allow;
				break;
		   }
		}

 		for( i = 0 ;gChgProtoListA_250ms[i].frmtype != 0xff; i++)	{
	   		if( pf == gChgProtoListA_250ms[i].pgn ) {
				gChgProtoListA_250ms[i].allowed = allow;
				break;
	   		}
		}

 
 		for( i = 0 ;gChgProtoListA_50ms[i].frmtype != 0xff; i++)	{
	   		if( pf == gChgProtoListA_50ms[i].pgn ) {
				gChgProtoListA_50ms[i].allowed = allow;
				break;
	   		}
		}

		for( i = 0 ;gChgProtoListA_10ms[i].frmtype != 0xff; i++)   {
	  		if( pf == gChgProtoListA_10ms[i].pgn ) {
		   		gChgProtoListA_10ms[i].allowed = allow;
		   		break;
	  		}
   		}
	}
	else
	{
	 	for( i = 0 ;gChgProtoListB_500ms[i].frmtype != 0xff; i++)  {
		   if( pf == gChgProtoListB_500ms[i].pgn ) {
				gChgProtoListB_500ms[i].allowed = allow;
				break;
		   }
		}

 		for( i = 0 ;gChgProtoListB_250ms[i].frmtype != 0xff; i++)	{
	   		if( pf == gChgProtoListB_250ms[i].pgn ) {
				gChgProtoListB_250ms[i].allowed = allow;
				break;
	   		}
		}

 
 		for( i = 0 ;gChgProtoListB_50ms[i].frmtype != 0xff; i++)	{
	   		if( pf == gChgProtoListB_50ms[i].pgn ) {
				gChgProtoListB_50ms[i].allowed = allow;
				break;
	   		}
		}

		for( i = 0 ;gChgProtoListB_10ms[i].frmtype != 0xff; i++)   {
	  		if( pf == gChgProtoListB_10ms[i].pgn ) {
		   		gChgProtoListB_10ms[i].allowed = allow;
		   		break;
	  		}
   		}
	}
#else
   	SEND_PROTO_ST *ptrList = GetProtoListPtr(gunNo);
   
 	for( i = 0 ;ptrList[i].frmtype != 0xff; i++)  {
	   if( pf == ptrList[i].pgn ) {
			ptrList[i].allowed = allow;
			if( 0 == fg ) { 
				ptrList[i].currticks = GetSystemTick();
			}else { /*立即发送*/
				ptrList[i].currticks = 0;
			}
			break;
	   }
	}

#endif
	  
	 return 0;
}


/* 所有报文发送周期关闭 */
void Gbt_CtrlCricleAllclose(uint8 gunNo)
{
	uint16 i = 0;
	
#ifdef BMS_USE_TIMER
	 if(gunNo == AGUN_NO)
	 {
	 	for( i = 0 ;gChgProtoListA_500ms[i].frmtype != 0xff; i++)  {
				gChgProtoListA_500ms[i].allowed = CRICLE_DISALLOWED;
		}

 		for( i = 0 ;gChgProtoListA_250ms[i].frmtype != 0xff; i++)	{
			gChgProtoListA_250ms[i].allowed = CRICLE_DISALLOWED;
		}

 
 		for( i = 0 ;gChgProtoListA_50ms[i].frmtype != 0xff; i++)	{
			gChgProtoListA_50ms[i].allowed = CRICLE_DISALLOWED;
		}

		for( i = 0 ;gChgProtoListA_10ms[i].frmtype != 0xff; i++)   {
		   gChgProtoListA_10ms[i].allowed = CRICLE_DISALLOWED;
   		}
	 }
	 else
	 {
		 for( i = 0 ;gChgProtoListB_500ms[i].frmtype != 0xff; i++)	{
				 gChgProtoListB_500ms[i].allowed = CRICLE_DISALLOWED;
		 }
		 
		 for( i = 0 ;gChgProtoListB_250ms[i].frmtype != 0xff; i++)	 {
			 gChgProtoListB_250ms[i].allowed = CRICLE_DISALLOWED;
		 }
		 
		 
		 for( i = 0 ;gChgProtoListB_50ms[i].frmtype != 0xff; i++)	 {
			 gChgProtoListB_50ms[i].allowed = CRICLE_DISALLOWED;
		 }
		 
		 for( i = 0 ;gChgProtoListB_10ms[i].frmtype != 0xff; i++)	{
			gChgProtoListB_10ms[i].allowed = CRICLE_DISALLOWED;
		 }
	 }
#else
	SEND_PROTO_ST *ptrList = GetProtoListPtr(gunNo);

	for( i = 0 ;ptrList[i].frmtype != 0xff; i++){
		ptrList[i].allowed = CRICLE_DISALLOWED;
		ptrList[i].currticks = 0;
	}
#endif
	
}


uint8 Gbt_AckPkgFrame(void *ctrlinf,uint8 *buf,uint8 GunNo)
{
	 uint8 pf;
	 GSEND_INFO *pCtrl =(GSEND_INFO *)ctrlinf;
	 pf = pCtrl->pf;
	 if(pCtrl->tg == EC10_TG ) {
		 pf = pCtrl->ackpf;
	 }
	 const RCV_PROTO_ST *ptr = Gbt_GetRcvProcItem(pf);
	 if( (NULL == ptr) || (NULL == ptr->ackpkg)) {
		  return 0;
	 } 
	 return (ptr->ackpkg(buf,pCtrl,GunNo));
}


uint8 Gbt_AckMultiPkgFrame(uint8 mpf,void *ctrlinf,uint8 *buf,uint8 GunNo)
{

	 GSEND_INFO *pCtrl =(GSEND_INFO *)ctrlinf;

	 const RCV_PROTO_ST *ptr = Gbt_GetRcvProcItem(mpf);
	 if( (NULL == ptr) || (NULL == ptr->ackpkg)) {
		  return 0;
	 } 
	 return (ptr->ackpkg(buf,pCtrl,GunNo));
}


/* 接收CAN链路帧 
typedef  struct
{
	 uint8  tg;
	 uint16 length;
	 uint8  pkgnum;
	 uint8  rsvFF;
	 uint8  pf[3];
}ACKBMS_ST;
*/
uint8 Gbt_RcvDealFrame(uint8 GunNo,void *rtninfo,uint8 *pbuf)
{
	CAN_MSG Canmsg;
  uint8 sMultiRcvOk = 0;

	uint16 len;
	GSEND_INFO *ptrCtrlinf = (GSEND_INFO *)rtninfo;
	uint8 sbuf[20] = {0};
  PDU_HEAD_STRUCT *ptrPF = NULL;
  const RCV_PROTO_ST *ptrItem1 = NULL;
	
  memset(&Canmsg,0,sizeof(CAN_MSG));

	if(Bms_CanRead(&Canmsg,0,GunNo) > 0)
	{
		ptrPF = (PDU_HEAD_STRUCT *)(&Canmsg.Id.ExtId);
		if ( ptrPF->sbit.PS != CHARGER_ADDR || ptrPF->sbit.SA != BMS_ADDR) { /*普天的桩地址为0xE5*/
			if( ptrPF->sbit.SA == BMS_ADDR  || ptrPF->sbit.PS == PUTIAN_CHARGER_ADDR) {
				 memcpy(pbuf,&Canmsg,sizeof(CAN_MSG)); 
				 return GBT_RTN_OTHER;
			 }else {
				 return GBT_RTN_FAIL;
			}
		}
		
		/* 标准规定充电机应对所收到的所有报文的ID（包括数据页，保留位，PGN，源地址，目的地址等）是否正确进行判断。
		充电机在收到保留位错误（R=1）的BRO(AA)报文时，将其当成了正确的BRO(AA)报文，回复了CRO报文，因此不正确 */
		
		/* 标准规定充电机应对所收到的所有报文的ID（包括数据页，保留位，PGN，源地址，目的地址等）是否正确进行判断。
		此测试项中充电机在收到数据页错误（DP=1）的BRO(AA)报文时，将其当成了正确的BRO(AA)报文，回复了CRO报文，因此不正确 */
		
		/* 当BMS向充电机发送的辨识报文BRM中的连接请求中的PGN的高字节或低字节错误（如 00 02 01）时，充电机将其当成了正确的BRM报文，
		在使用传输协议功能正常接收后回复了CRM(AA)报文，因此不符合要求 */
		
		if (0 != ptrPF->sbit.R)
		{
			return GBT_RTN_FAIL;
		}
		
		if (0 != ptrPF->sbit.DP)
		{
			return GBT_RTN_FAIL;
		}
		
		switch (ptrPF->sbit.PF)
		{
			case GBT_MULIT_FRAME://多包请求命令 0x1CEC56F4  10 09 00 02 02 00 11 00
			{
			  /* 必须是多包处理的优先权 */
				if (7 != ptrPF->sbit.Pri)
				{
					return GBT_RTN_ERRDATA;
				}
				
				DebugInfoWithPbuf(BMS_MOD,"多包请求，发送EC", Canmsg.Data, 8,2);
				memcpy(&(ptrCtrlinf->ctrl.info),Canmsg.Data,8);
				/*多包不符合标准，发拒绝接收多包*/
				if(ptrCtrlinf->ctrl.info.tg != 0x10 || ptrCtrlinf->ctrl.info.length > ptrCtrlinf->ctrl.info.pkgnum*8 \
				 || ptrCtrlinf->ctrl.info.pkgnum > 20  || 0 != ptrCtrlinf->ctrl.info.pf[0] || 0 != ptrCtrlinf->ctrl.info.pf[2]) 
				{
					ptrCtrlinf->tg = ptrCtrlinf->ctrl.info.tg;
					ptrCtrlinf->frmNum = ptrCtrlinf->ctrl.info.pkgnum;
					ptrCtrlinf->length = ptrCtrlinf->ctrl.info.length;

					ptrCtrlinf->pf = ptrCtrlinf->ctrl.info.pf[1];
					ptrCtrlinf->objAdr = BMS_ADDR;
					ptrCtrlinf->pri = ptrPF->sbit.Pri;
					ptrCtrlinf->srcAdr = CHARGER_ADDR;

					ptrCtrlinf->gunNo = GunNo;
					len = ChgSendPkgRefuseMuliti(sbuf,ptrCtrlinf,0);
					Gbt_SendData(ptrCtrlinf,sbuf,len);
	//patli 20200226				Delay5Ms(1);
					return 0;
				}
				/*多包符合标准，发送允许应答包  11 02 01 FF FF 00 11 00 */
				ptrCtrlinf->ctrl.index = 0;
				sMultiRcvOk = 0;
				ptrCtrlinf->ctrl.mulitifrmNo = 1;
				ptrCtrlinf->tg = ptrCtrlinf->ctrl.info.tg;
				ptrCtrlinf->frmNum = ptrCtrlinf->ctrl.info.pkgnum;
				ptrCtrlinf->length = ptrCtrlinf->ctrl.info.length;

				ptrCtrlinf->pf = ptrCtrlinf->ctrl.info.pf[1];
				ptrCtrlinf->objAdr = BMS_ADDR;
				ptrCtrlinf->pri = ptrPF->sbit.Pri;
				ptrCtrlinf->srcAdr = CHARGER_ADDR;

				ptrCtrlinf->gunNo = GunNo;
				ptrCtrlinf->ackpf = GBT_MULIT_FRAME;
				len = Gbt_AckPkgFrame(ptrCtrlinf,sbuf,GunNo);
				Gbt_SendData(ptrCtrlinf,sbuf,len);
	//patli 20200226				Delay5Ms(1);
				return GBT_RTN_SUCESS;
			}
			case GBT_MULIT_PACKAGE://多包数据包 01 47 0D C4 0E 54 01 5F ；02 06 00 FF FF FF FF FF 
			{
				/* 必须是多包处理的优先权 */
				if (7 != ptrPF->sbit.Pri)
				{
					return GBT_RTN_ERRDATA;
				}
				
				DebugInfoWithPbuf(BMS_MOD,"多包信息", Canmsg.Data, 8,2);
				if( 1 == ptrCtrlinf->ctrl.mulitifrmNo) {
					ptrCtrlinf->ctrl.index = 0;
					len = 0;
					memset(ptrCtrlinf->ctrl.multibuf,0,100);
				}
				ptrCtrlinf->ctrl.mulitifrmNo++;
				ptrCtrlinf->ctrl.index  = Canmsg.Data[0] > 0 ? Canmsg.Data[0] : 1;
				Canmsg.DLC = Canmsg.DLC > 8 ? 8 : Canmsg.DLC;
				if( (ptrCtrlinf->ctrl.index < (ptrCtrlinf->ctrl.info.pkgnum+1)) && (ptrCtrlinf->ctrl.index < 20)) {
						memcpy(&ptrCtrlinf->ctrl.multibuf[((ptrCtrlinf->ctrl.index-1)*7)%MAX_MULITI_BUF_SIZE],&Canmsg.Data[1],Canmsg.DLC-1);
				}
				
				if( ptrCtrlinf->ctrl.mulitifrmNo > ptrCtrlinf->ctrl.info.pkgnum ) {
							sMultiRcvOk = 0x55;
							break;
				}
				break;
			}
			default:  //单包处理
			{
				/* 必须是单包处理的优先权 */
				if ((6 == ptrPF->sbit.Pri) || (4 == ptrPF->sbit.Pri) || (2 == ptrPF->sbit.Pri))
				{
					ptrItem1 = Gbt_GetRcvProcItem(ptrPF->sbit.PF);

					if ((NULL != ptrItem1) && (NULL != ptrItem1->proc)) 
					{
						ptrItem1->proc((void *)ptrItem1,Canmsg.Data,GunNo);
						return GBT_RTN_SUCESS;
					}
				}
				
				return GBT_RTN_ERRDATA;	
			}
		}// end case
		
		if(0x55 == sMultiRcvOk) {//多包收完处理  13 09 00 02 FF 00 11 00
			   sMultiRcvOk = 0;
				 const RCV_PROTO_ST *ptrItem = Gbt_GetRcvProcItem(ptrCtrlinf->ctrl.info.pf[1]);
				 if( (NULL != ptrItem) && (NULL != ptrItem->proc)) {
					 ptrItem->proc((void *)ptrItem,ptrCtrlinf->ctrl.multibuf,GunNo);
					 
					 ptrCtrlinf->tg = ptrCtrlinf->ctrl.info.tg;
					 ptrCtrlinf->frmNum = ptrCtrlinf->ctrl.info.pkgnum;
					 ptrCtrlinf->length = ptrCtrlinf->ctrl.info.length;
			
					 ptrCtrlinf->pf = ptrCtrlinf->ctrl.info.pf[1];
					 ptrCtrlinf->objAdr = ptrPF->sbit.SA;
					 ptrCtrlinf->pri = ptrPF->sbit.Pri;
					 ptrCtrlinf->srcAdr = ptrPF->sbit.PS;
			
			     ptrCtrlinf->gunNo = GunNo;
			     ptrCtrlinf->ackpf = GBT_MULIT_PACKAGE;
					 
					 len = Gbt_AckPkgFrame(ptrCtrlinf,sbuf,GunNo);
					 ptrCtrlinf->ackpf = GBT_MULIT_FRAME;
					 
					 Gbt_SendData(ptrCtrlinf,sbuf,len);
//patli 20200226						 Delay5Ms(1);
					 return GBT_RTN_SUCESS;
				}
		}
	} 
	return GBT_RTN_FAIL;
}
				
//				
////		0x1CECF456   FF FF FF FF FF 00 15 00  放弃接收多包

/* 发送CAN数据包 */
void Gbt_SendData(GSEND_INFO *sendinf,uint8 *pbuf,uint16 length)
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
//		Delay5Ms(1);
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
//			Delay5Ms(1);
		}
	}
}




