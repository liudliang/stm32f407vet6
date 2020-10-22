/**
  ******************************************************************************
  * @file    RecordFat.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
	
#include "string.h"
#include "I2C_FLASH.h"
#include "BillingUnit.h"
#include "ChgData.h"
#include "RecordFat.h"
#include "main.h"

OS_EVENT *gRdE2romLock = NULL;

RD_PARA_ST gFullE2ROMTabl;
RD_PARA_ST gTempE2ROMTabl;

extern uint8 SST_Rd_BufferWrite(uint8 *pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);
extern uint8 SST_Rd_BufferRead(uint8 *recv, uint32 adr, uint16 size);

/*保存存储器数据*/
uint8 SaveStoreData(uint8 devtype,void *src,uint32 adr,uint16 size)
{
	uint8 rtn = 0;
	uint8 err;
	
	OSSemPend(gRdE2romLock, 0, &err);
	
	if (devtype == FM_IC)
	{
		rtn = I2C_WriteNbyte((uint8*)src,adr,size);
	}
	else if (devtype == SST_IC) 
	{
		rtn = SST_Rd_BufferWrite((uint8 *)src,adr,size);
	}
	
	OSSemPost(gRdE2romLock);
	
	return rtn;
}

/*读取存储器数据*/
uint8 ReadStoreData(uint8 devtype,void *obj,uint16 size,uint32 adr)
{
	uint8 rtn = 0;
	uint8 err;
	
	OSSemPend(gRdE2romLock, 0, &err);
	
	if (devtype == FM_IC )
	{
		rtn = I2C_ReadNbyte((uint8*)obj,adr,size);
	}
	else if (devtype == SST_IC) 
	{
		rtn = SST_Rd_BufferRead((uint8 *)obj,adr,size);
	}
	
	OSSemPost(gRdE2romLock);
	
	return rtn;
}

extern unsigned short Crc16_Calc(unsigned char *updata, unsigned short len);
extern PARAM_OPER_TYPE *ChgData_GetRunParamPtr(void);

#define BACK_OFFSET  8
uint8 Rd_SaveDevParam(uint8 devtype)
{
//	 Wdg_feeddog();

	 PARAM_DEV_TYPE *ptrDev = ChgData_GetDevParamPtr();
	 ptrDev->head = 0x55AA;
	 ptrDev->crc = Crc16_Calc((uint8 *)ptrDev,sizeof(PARAM_DEV_TYPE)-2);

	 SaveStoreData(devtype,(uint8 *)ptrDev,EEROM_DEVPARA_ADR,sizeof(PARAM_DEV_TYPE));
//	 Wdg_feeddog();
	 SaveStoreData(devtype,(uint8 *)ptrDev,(EEROM_DEVPARA_ADR+sizeof(PARAM_DEV_TYPE)+BACK_OFFSET),sizeof(PARAM_DEV_TYPE));
	 
	return 0;
}

uint8 Rd_SaveOperParam(uint8 devtype)
{
//	 Wdg_feeddog();
	
	 PARAM_OPER_TYPE *ptrParm = ChgData_GetRunParamPtr();
	 ptrParm->head = 0x55AA;
	 ptrParm->crc = Crc16_Calc((uint8 *)ptrParm,sizeof(PARAM_OPER_TYPE)-2);
	
	 SaveStoreData(devtype,(uint8 *)ptrParm,EEROM_OPERPARA_ADR,sizeof(PARAM_OPER_TYPE));
//	 Wdg_feeddog();
	 SaveStoreData(devtype,(uint8 *)ptrParm,EEROM_OPERPARA_ADR+sizeof(PARAM_OPER_TYPE)+BACK_OFFSET,sizeof(PARAM_OPER_TYPE));
	
	 return 0;
}

uint16 Rd_SaveFeePara(uint8 devtype)
{
//	 Wdg_feeddog();

	 PARAM_FEE_TYPE *ptrParm = ChgData_GetFeeRate(AGUN_NO);
	 ptrParm->head = 0x55AA;
	 ptrParm->crc = Crc16_Calc((uint8 *)ptrParm,sizeof(PARAM_FEE_TYPE)-2);
	
	 SaveStoreData(devtype,(uint8 *)ptrParm,EEROM_FEEPARA_ADR,sizeof(PARAM_FEE_TYPE));
//	 Wdg_feeddog();
	 SaveStoreData(devtype,(uint8 *)ptrParm,EEROM_FEEPARA_ADR+sizeof(PARAM_FEE_TYPE)+BACK_OFFSET,sizeof(PARAM_FEE_TYPE));
	 
	 return 0;
}

/*
读设备参数
*/
uint8 Rd_ReadDevParam(uint8 devtype) 
{
	 uint8 i,rtn;
	 uint16 crc;
	 PARAM_DEV_TYPE param;
	 PARAM_DEV_TYPE *ptrDev = ChgData_GetDevParamPtr();
	 memset(&param,0,sizeof(PARAM_DEV_TYPE));
	 //rtn = sizeof(PARAM_DEV_TYPE);
	 rtn = 0;
	 for( i = 0 ; i < 5 ; i++ ) {
		ReadStoreData(devtype,(uint8 *)&param,sizeof(PARAM_DEV_TYPE),EEROM_DEVPARA_ADR);
		if(param.head == 0x55AA ) {
			crc = Crc16_Calc((uint8 *)&param,sizeof(PARAM_DEV_TYPE)-2);
			if( crc == param.crc ) {
				memcpy(ptrDev,&param,sizeof(PARAM_DEV_TYPE));
				rtn = 1;
				break;
			}
		}	
    Delay10Ms(1);		
	 }
	 if( 0 == rtn ) {
		  memset(&param,0,sizeof(PARAM_OPER_TYPE));
		 for( i = 0 ; i < 5 ; i++ ) {
  			ReadStoreData(devtype,(uint8 *)&param,sizeof(PARAM_DEV_TYPE),(EEROM_DEVPARA_ADR+sizeof(PARAM_DEV_TYPE)+BACK_OFFSET));
				if(param.head == 0x55AA ) {
					crc = Crc16_Calc((uint8 *)&param,sizeof(PARAM_DEV_TYPE)-2);
					if( crc == param.crc ) {
						memcpy(ptrDev,&param,sizeof(PARAM_DEV_TYPE));
						rtn = 1;
						break;
					}
				}	
			Delay10Ms(1);		
		}
	 }
	 return rtn;
}

/*读运行操作参数*/
uint8 Rd_ReadOperParam(uint8 devtype) 
{
	 uint8 i,rtn;
	 uint16 crc;
	 PARAM_OPER_TYPE param;
	 PARAM_OPER_TYPE *ptrParm = ChgData_GetRunParamPtr();
	 memset(&param,0,sizeof(PARAM_OPER_TYPE));
	//rtn = sizeof(PARAM_OPER_TYPE);
	 rtn = 0;
	 for( i = 0 ; i < 5 ; i++ ) {
		ReadStoreData(devtype,(uint8 *)&param,sizeof(PARAM_OPER_TYPE),EEROM_OPERPARA_ADR);
		if(param.head == 0x55AA ) {
			crc = Crc16_Calc((uint8 *)&param,sizeof(PARAM_OPER_TYPE)-2);
			if( crc == param.crc ) {
				memcpy(ptrParm,&param,sizeof(PARAM_OPER_TYPE));
				rtn = 1;
				break;
			}
		}	
    Delay10Ms(1);		
	 }
	 if( 0 == rtn ) {
		  memset(&param,0,sizeof(PARAM_OPER_TYPE));
		 for( i = 0 ; i < 5 ; i++ ) {
				ReadStoreData(devtype,(uint8 *)&param,sizeof(PARAM_OPER_TYPE),(EEROM_OPERPARA_ADR+sizeof(PARAM_OPER_TYPE)+BACK_OFFSET));
				if(param.head == 0x55AA ) {
					crc = Crc16_Calc((uint8 *)&param,sizeof(PARAM_OPER_TYPE)-2);
					if( crc == param.crc ) {
						memcpy(ptrParm,&param,sizeof(PARAM_OPER_TYPE));
						rtn = 1;
						break;
					}
				}	
			Delay10Ms(1);		
		}
	 }
	 return rtn;
}

#ifdef CP_PERIOD_FEE
uint8 Rd_SaveCpPeriodFeeParam(uint8 devtype)
{
//	 Wdg_feeddog();

	 PARAM_CP_PERIOD_FEE_TYPE *ptrFee = ChgData_GetCpPeriodFeeRate();

	 ptrFee->head = 0x55AA;
	 ptrFee->crc = Crc16_Calc((uint8 *)ptrFee,sizeof(PARAM_CP_PERIOD_FEE_TYPE)-2);

	 SaveStoreData(devtype,(uint8 *)ptrFee,EEROM_CP_PEROID_FEE_ADR,sizeof(PARAM_CP_PERIOD_FEE_TYPE));
//	 Wdg_feeddog();
	 SaveStoreData(devtype,(uint8 *)ptrFee,(EEROM_CP_PEROID_FEE_ADR+sizeof(PARAM_CP_PERIOD_FEE_TYPE)+BACK_OFFSET),sizeof(PARAM_CP_PERIOD_FEE_TYPE));
	 
	return 0;
}

uint8 Rd_ReadCpPeriodFeeParam(uint8 devtype)
{
	 uint8 i,rtn;
	 uint16 crc;
	 PARAM_CP_PERIOD_FEE_TYPE param;
	 PARAM_CP_PERIOD_FEE_TYPE *ptrParm = ChgData_GetCpPeriodFeeRate();

	 memset(&param,0,sizeof(PARAM_CP_PERIOD_FEE_TYPE));
	//rtn = sizeof(PARAM_FEE_TYPE);
	 rtn = 0;
	 for( i = 0 ; i < 5 ; i++ ) {
		ReadStoreData(devtype,(uint8 *)&param,sizeof(PARAM_CP_PERIOD_FEE_TYPE),EEROM_CP_PEROID_FEE_ADR);
		if(param.head == 0x55AA ) {
			crc = Crc16_Calc((uint8 *)&param,sizeof(PARAM_CP_PERIOD_FEE_TYPE)-2);
			if( crc == param.crc ) {
				memcpy(ptrParm,&param,sizeof(PARAM_CP_PERIOD_FEE_TYPE));
				rtn = 1;
				break;
			}
		}	
    Delay10Ms(1);		
	 }
	 if( 0 == rtn ) {
		  memset(&param,0,sizeof(PARAM_CP_PERIOD_FEE_TYPE));
		 for( i = 0 ; i < 3 ; i++ ) {
				ReadStoreData(devtype,(uint8 *)&param,sizeof(PARAM_CP_PERIOD_FEE_TYPE),(EEROM_CP_PEROID_FEE_ADR+sizeof(PARAM_CP_PERIOD_FEE_TYPE)+BACK_OFFSET));
				if(param.head == 0x55AA ) {
					crc = Crc16_Calc((uint8 *)&param,sizeof(PARAM_CP_PERIOD_FEE_TYPE)-2);
					if( crc == param.crc ) {
						memcpy(ptrParm,&param,sizeof(PARAM_CP_PERIOD_FEE_TYPE));
						rtn = 1;
						break;
					}
				}	
			Delay10Ms(1);		
		}
	 }
	 return rtn;
	
}

#endif

uint8 Rd_ReadFeeParam(uint8 devtype)
{
	 uint8 i,rtn;
	 uint16 crc;
	 PARAM_FEE_TYPE param;
	 PARAM_FEE_TYPE *ptrParmA = ChgData_GetFeeRate(AGUN_NO);
	 PARAM_FEE_TYPE *ptrParmB = ChgData_GetFeeRate(BGUN_NO);
	 memset(&param,0,sizeof(PARAM_FEE_TYPE));
	//rtn = sizeof(PARAM_FEE_TYPE);
	 rtn = 0;
	 for( i = 0 ; i < 5 ; i++ ) {
		ReadStoreData(devtype,(uint8 *)&param,sizeof(PARAM_FEE_TYPE),EEROM_FEEPARA_ADR);
		if(param.head == 0x55AA ) {
			crc = Crc16_Calc((uint8 *)&param,sizeof(PARAM_FEE_TYPE)-2);
			if( crc == param.crc ) {
				memcpy(ptrParmA,&param,sizeof(PARAM_FEE_TYPE));
				memcpy(ptrParmB,&param,sizeof(PARAM_FEE_TYPE));
				rtn = 1;
				break;
			}
		}	
    Delay10Ms(1);		
	 }
	 if( 0 == rtn ) {
		  memset(&param,0,sizeof(PARAM_FEE_TYPE));
		 for( i = 0 ; i < 3 ; i++ ) {
				ReadStoreData(devtype,(uint8 *)&param,sizeof(PARAM_FEE_TYPE),(EEROM_FEEPARA_ADR+sizeof(PARAM_FEE_TYPE)+BACK_OFFSET));
				if(param.head == 0x55AA ) {
					crc = Crc16_Calc((uint8 *)&param,sizeof(PARAM_FEE_TYPE)-2);
					if( crc == param.crc ) {
						memcpy(ptrParmA,&param,sizeof(PARAM_FEE_TYPE));
						memcpy(ptrParmB,&param,sizeof(PARAM_FEE_TYPE));
						rtn = 1;
						break;
					}
				}	
			Delay10Ms(1);		
		}
	 }
	 return rtn;
	
}

/**************************************************************************************************************************/
extern uint8 Rd_InitFlashData(void);
extern uint8 ChgData_InitDefaultCpPeriodFeePara(void);

/*初始化存储记录数据*/
void Rd_InitRecord(void)
{
	uint8 res=0, errcnt = 0;
	
	ChgData_InitDefaultOperPara();
	do{		
		 res= Rd_ReadOperParam(FM_IC);
		 errcnt++;//while(1);/*参数 出错*/
		 if(!res&&errcnt >= 3)
		 {
			Rd_SaveOperParam(FM_IC);
			errcnt = 0;
			break;
		 }
	}while(res&&errcnt<3);
	
	ChgData_InitDefaultDevPara();
//for test 	Rd_SaveDevParam(FM_IC);
	do{
		res = Rd_ReadDevParam(FM_IC);
		errcnt++;
		if(!res&&errcnt>=3)
		{
			Rd_SaveDevParam(FM_IC);
			break;
		}
	}while(res&&errcnt<3);	
	errcnt = 0;
	
	ChgData_InitDefaultFeePara();
	do{
		res = Rd_ReadFeeParam(FM_IC);
		errcnt++;
		if(!res&&errcnt>=3)
		{
			Rd_SaveFeePara(FM_IC);
			break;
		}
	}while(res&&errcnt<3);
	errcnt = 0;

#ifdef CP_PERIOD_FEE
	ChgData_InitDefaultCpPeriodFeePara();
	do{
		res = Rd_ReadCpPeriodFeeParam(FM_IC);
		errcnt++;
		if(!res&&errcnt>=3)
		{
			Rd_SaveCpPeriodFeeParam(FM_IC);
			break;
		}
	}while(res&&errcnt<3);
	errcnt = 0;

#endif
	
	/*记录头初始化*/
	Rd_InitE2romData();
	do{
		res = Rd_InitE2romData();
		errcnt++;
		if(!res&&errcnt>=3)
		{
			SoftReset();	  
		}
    }while(res&&errcnt<3);		


	PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	if(DEV_DC_S >= devparaPtr->Sysparalarm.bits.systemtype){
		devparaPtr->gunnum = GUNNUM_1;
		devparaPtr->Sysparalarm.bits.systemtype = DEV_DC_S;
	}
	else{
		devparaPtr->gunnum = GUNNUM_2;
	}
}

/**************************************************************************************************************************/
/*
24L64  8K byte  用与存储临时记录
*/

int Rd_E2romSaveCtrlParam(uint8 devtype, RD_PARA_ST *E2romTabl,uint16 addr)
{
	 uint8 i;
	 int rtn = I2C_FALSE;
	 uint16 crc;
	 E2romTabl->head = STORE_HEAD;
	 crc = Crc16_Calc((uint8 *)E2romTabl,sizeof(RD_PARA_ST)-2);
	
	 E2romTabl->crc = crc;

	 for ( i = 0 ; i < 3 ; i++ ) {
		 
			rtn = SaveStoreData(devtype,(uint8 *)E2romTabl,addr,sizeof(RD_PARA_ST)); 
			if(I2C_TRUE == rtn ) {
				break;
			}
		 Delay10Ms(1);
	 }
	 return  rtn;
}


int Rd_E2romReadCtrlParam(uint8 devtype, RD_PARA_ST *E2romTabl,uint16 addr)
{
	uint8 i;
	uint8 rtn;
	uint16 crc;
	RD_PARA_ST ctrltabl;
	for( i = 0 ; i < 3; i++ ) {
		rtn = ReadStoreData(devtype,(uint8*)&ctrltabl,sizeof(RD_PARA_ST),addr);
		if( I2C_TRUE == rtn ) {
			if(ctrltabl.head == STORE_HEAD) {
				crc = Crc16_Calc((uint8 *)&ctrltabl,sizeof(RD_PARA_ST)-2);
				if( crc == ctrltabl.crc ) {
					memcpy(E2romTabl,&ctrltabl,sizeof(RD_PARA_ST));
					return I2C_TRUE;
				}
			}	
		}
		 Delay10Ms(1);
	}
 return I2C_FALSE;
}

void Rd_E2romClearTempRecord(RD_PARA_ST *E2romTabl)
{
	if(E2romTabl->nType == ETEMP_RECORD) {
		E2romTabl->currrdno = 0;
		E2romTabl->baktolrdno = 0;
		E2romTabl->totlrdno = 0;
		Rd_E2romSaveCtrlParam(FM_IC,E2romTabl,EEROM_TEMPPARATABL_ADR); 
		Rd_E2romSaveCtrlParam(FM_IC,E2romTabl,EEROM_TEMPBACK1TABL_ADR); /*备份存储表*/
	}
}

uint16 Rd_E2romSaveTempRecord(RD_PARA_ST *E2romTabl,CALCBILL_DATA_ST *bill)
{
	uint16 static sErrCount = 0;
	uint8 rtn,i;
	RECORD_ST record;
	record.head = STORE_HEAD;
	memcpy(&record.bill,bill,sizeof(CALCBILL_DATA_ST));
	record.crc = Crc16_Calc((uint8 *)&record,sizeof(RECORD_ST)-2);
	if(E2romTabl->nType != ETEMP_RECORD) {
			 return 0;
	}
	if(E2romTabl->totlrdno - E2romTabl->baktolrdno != 1 && E2romTabl->totlrdno > 0) {
		if(E2romTabl->nType == ETEMP_RECORD) {
			if( I2C_TRUE != Rd_E2romReadCtrlParam(FM_IC,E2romTabl,EEROM_TEMPPARATABL_ADR) ) {
				Rd_E2romReadCtrlParam(FM_IC,E2romTabl,EEROM_TEMPBACK1TABL_ADR);
			}
		}
  }
  if( (E2romTabl->totlrdno - E2romTabl->baktolrdno == 1 ) || (0 == E2romTabl->totlrdno) ) {
		sErrCount = 0;
		for ( i = 0 ; i < 3 ; i++ ) { 
			rtn =  SaveStoreData(FM_IC,(uint8 *)&record,EEROM_TEMPRD_ADR+E2romTabl->currrdno*sizeof(RECORD_ST),sizeof(RECORD_ST));  //使用FM_IC 
			if(I2C_TRUE == rtn ) {
				E2romTabl->currrdno = (E2romTabl->currrdno+1) % MAX_TEMPRD_NUM;
        if( E2romTabl->totlrdno < MAX_TEMPRD_NUM ) {
					E2romTabl->baktolrdno = E2romTabl->totlrdno;
					E2romTabl->totlrdno++;
				}else {
					E2romTabl->totlrdno = MAX_TEMPRD_NUM;
				}
				
				Rd_E2romSaveCtrlParam(FM_IC,E2romTabl,EEROM_TEMPPARATABL_ADR); 
				Rd_E2romSaveCtrlParam(FM_IC,E2romTabl,EEROM_TEMPBACK1TABL_ADR); /*备份存储表*/
				
				break;
			}
			 Delay10Ms(1);
		}
	 }else {
		 if( sErrCount++ > 2000 ) {
			 sErrCount = 0;
			 E2romTabl->currrdno = 0;
			 E2romTabl->baktolrdno = 0;
			 E2romTabl->totlrdno = 0;
		 }
	 }
   return E2romTabl->currrdno;
}

uint16 Rd_E2romSaveFullRecord(RD_PARA_ST *E2romTabl,CALCBILL_DATA_ST *bill)
{
	uint16 static sErrCount = 0;
	uint8 rtn,i;
	RECORD_ST record;
	record.head = STORE_HEAD;
	memcpy(&record.bill,bill,sizeof(CALCBILL_DATA_ST));
	record.crc = Crc16_Calc((uint8 *)&record,sizeof(RECORD_ST)-2);
	if (E2romTabl->nType != EFULL_RECORD ) {
		 return 0;
	}
	
	if(E2romTabl->totlrdno - E2romTabl->baktolrdno != 1 && E2romTabl->totlrdno > 0) {
		if( I2C_TRUE != Rd_E2romReadCtrlParam(FM_IC,E2romTabl,EEROM_FULLPARATABL_ADR) ) {
				Rd_E2romReadCtrlParam(FM_IC,E2romTabl,EEROM_FULLBACK1TABL_ADR); 
		}
  }
	
  if( (E2romTabl->totlrdno - E2romTabl->baktolrdno == 1 ) || (0 == E2romTabl->totlrdno) ) {
		sErrCount = 0;
		for ( i = 0 ; i < 3 ; i++ ) { 
			rtn =  SaveStoreData(SST_IC,(uint8 *)&record,EEROM_RECORD_ADR+E2romTabl->currrdno*sizeof(RECORD_ST),sizeof(RECORD_ST));   
			if(I2C_TRUE == rtn ) {
				E2romTabl->currrdno = (E2romTabl->currrdno+1) % MAX_RECORD_NUM;
				
				if( E2romTabl->totlrdno < MAX_RECORD_NUM ) {
						E2romTabl->baktolrdno = E2romTabl->totlrdno;
						E2romTabl->totlrdno++;
				}else {
					E2romTabl->totlrdno = MAX_RECORD_NUM;
				}
			
				Rd_E2romSaveCtrlParam(FM_IC,E2romTabl,EEROM_FULLPARATABL_ADR); 
				Rd_E2romSaveCtrlParam(FM_IC,E2romTabl,EEROM_FULLBACK1TABL_ADR); /*备份存储表*/
			
				break;
			}
			 Delay10Ms(1);
		}
	 }else {
		 if( sErrCount++ > 2000 ) {
			 sErrCount = 0;
			 E2romTabl->currrdno = 0;
			 E2romTabl->baktolrdno = 0;
			 E2romTabl->totlrdno = 0;
		 }
	 }
	 
   return E2romTabl->currrdno;
}

/*读取指定记录号的记录，记录号1--n，返回读取的记录号，0则未读取到*/
uint16 Rd_E2romReadRecord(uint16 absNo,CALCBILL_DATA_ST *bill)
{
	uint8 rtn,i;
	uint16 crc;
	uint16 tempRecNo;
	RECORD_ST record;
	
	if(absNo > gFullE2ROMTabl.totlrdno) {
		return 0xffff;
	}

	if( gFullE2ROMTabl.currrdno >= absNo ) {
		tempRecNo = gFullE2ROMTabl.currrdno - absNo;
	}else {
		tempRecNo = gFullE2ROMTabl.currrdno+MAX_RECORD_NUM - absNo;
	}
	
	
	for ( i = 0 ; i < 3 ; i++ ) {
			rtn = ReadStoreData(SST_IC,(uint8 *)&record,sizeof(RECORD_ST),EEROM_RECORD_ADR + (tempRecNo)*sizeof(RECORD_ST));     
			if(I2C_TRUE == rtn ) {
				crc = Crc16_Calc((uint8 *)&record,sizeof(RECORD_ST)-2);
				if( crc == record.crc ) {
					memcpy(bill,&record.bill,sizeof(CALCBILL_DATA_ST));
					return absNo;
				}
			}
			Delay10Ms(1);
	}
	return 0xffff;
}

uint16 Rd_E2romReadCurrTempRecord(RD_PARA_ST *E2romTabl,CALCBILL_DATA_ST *bill)
{
	uint8 rtn,i;
	uint16 crc;
	RECORD_ST record;
	
	rtn = 0;
	if(E2romTabl->currrdno < 1) {
		return 0;
	}
	for ( i = 0 ; i < 3 ; i++ ) {
			rtn = ReadStoreData(FM_IC,(uint8 *)&record,sizeof(RECORD_ST),EEROM_TEMPRD_ADR + (E2romTabl->currrdno-1)*sizeof(RECORD_ST));   //使用FM_IC
			if(I2C_TRUE == rtn ) {
				crc = Crc16_Calc((uint8 *)&record,sizeof(RECORD_ST)-2);
				if( crc == record.crc ) {
					memcpy(bill,&record.bill,sizeof(CALCBILL_DATA_ST));
					return 1;
				}
			}
			Delay10Ms(1);
	}
	
	return rtn;
}

uint8 Rd_IsHaveTempRecord(void)
{
	if(gTempE2ROMTabl.totlrdno > 0 ) {
		return 1;
	}
	return 0;
}

uint16 Rd_E2romTempToFullSave(uint8 gunNo)
{
	uint16 rtn;
	RECORD_ST record;
	if( gTempE2ROMTabl.totlrdno == 0 ) {
		return 0;
	}
	rtn = Rd_E2romReadCurrTempRecord(&gTempE2ROMTabl,&record.bill);
	if( (1 == rtn ) && (gunNo == record.bill.gunno)) {
		record.bill.IsPay = 0;
		record.bill.endreason = ESYS_PWDOWN;
		Rd_E2romSaveFullRecord(&gFullE2ROMTabl,&record.bill); 
		/*清除TempRecord中的记录*/
		Rd_E2romClearTempRecord(&gTempE2ROMTabl);
		rtn =  1;
	}
	else{
		rtn = 0;
	}
	return rtn;
}
/*充之鸟协议，断电把保存在临时记录数据取出 CALCBILL_DATA_ST*/
uint16 Rd_E2romTempToFullBillBird(uint8 gunNo,CALCBILL_DATA_ST bill[])
{
	uint16 rtn;
	RECORD_ST record;
	if( gTempE2ROMTabl.totlrdno == 0 ) {
		return 0;
	}
	rtn = Rd_E2romReadCurrTempRecord(&gTempE2ROMTabl,&record.bill);
	if( (1 == rtn) && (gunNo == record.bill.gunno)) {
		record.bill.IsPay = 0;
		record.bill.endreason = ESYS_PWDOWN;
		memcpy(bill,&record.bill,sizeof(CALCBILL_DATA_ST));
		rtn = 1;
	}
	else{
		rtn = 0;
	}
	return rtn;
}
uint8 Rd_FindUnAccountRecord(CALCBILL_DATA_ST bill[])
{
	uint8 rtn;
	uint16 i;
	uint16 crc;
	RECORD_ST record;
	for( i = 0; i < gFullE2ROMTabl.totlrdno;i++ ) {
		rtn = ReadStoreData(SST_IC,(uint8 *)&record,sizeof(RECORD_ST),EEROM_RECORD_ADR + i*sizeof(RECORD_ST));   		
			if(I2C_TRUE == rtn ) {
				crc = Crc16_Calc((uint8 *)&record,sizeof(RECORD_ST)-2);
				if( crc == record.crc ) {
					if((record.bill.IsReport == 0 )&&((record.bill.startmod == START_BY_BKGROUND) \
						|| (START_BY_ONLINECARD == record.bill.startmod)|| (START_BY_VIN == record.bill.startmod)|| (START_BY_PASSWD == record.bill.startmod))) {
						memcpy(bill,&record.bill,sizeof(CALCBILL_DATA_ST));
						return 1;
					}
				}
			}
		Delay5Ms(1);
	}
	return 0;
}

uint8 Clear_RecordIsReport(uint8  *BillserialNo)
{
	uint8 rtn;
	uint16 i;
	uint16 crc;
	RECORD_ST record;
	for( i = 0; i < gFullE2ROMTabl.totlrdno;i++ ) {
		rtn = ReadStoreData(SST_IC,(uint8 *)&record,sizeof(RECORD_ST),EEROM_RECORD_ADR + i*sizeof(RECORD_ST));   
			if(I2C_TRUE == rtn ) {
				crc = Crc16_Calc((uint8 *)&record,sizeof(RECORD_ST)-2);
				if( crc == record.crc ) {
					if((memcmp(record.bill.serialNo,BillserialNo,32) == 0 )&& ((record.bill.startmod == START_BY_BKGROUND)||(record.bill.startmod == START_BY_ONLINECARD) \
						||(record.bill.startmod == START_BY_VIN)||(record.bill.startmod == START_BY_PASSWD))\
					 &&(record.bill.IsReport == 0 )) {
						record.bill.IsReport = 1 ;
						record.crc = Crc16_Calc((uint8 *)&record,sizeof(RECORD_ST)-2);
						 SaveStoreData(SST_IC,(uint8 *)&record,EEROM_RECORD_ADR + i*sizeof(RECORD_ST),sizeof(RECORD_ST)); 
						return 1;
					}
				}
			}
			Delay5Ms(1);
	}
	return 0;
}

uint8 Clear_RecordIsReportBird(uint8  *BillserialNo,uint8 *buf)
{
	uint8 rtn;
	uint16 i;
	uint16 crc;
	uint32 temp;
	RECORD_ST record;
	for( i = 0; i < gFullE2ROMTabl.totlrdno;i++ ) {
		rtn = ReadStoreData(SST_IC,(uint8 *)&record,sizeof(RECORD_ST),EEROM_RECORD_ADR + i*sizeof(RECORD_ST));   
			if(I2C_TRUE == rtn ) {
				crc = Crc16_Calc((uint8 *)&record,sizeof(RECORD_ST)-2);
				if( crc == record.crc ) {
					if((memcmp(record.bill.serialNo,BillserialNo,32) == 0 )&& \
						((record.bill.startmod == START_BY_BKGROUND)||(record.bill.startmod == START_BY_ONLINECARD)||(record.bill.startmod == START_BY_VIN)||(record.bill.startmod == START_BY_PASSWD))\
					 &&(record.bill.IsReport == 0 )) {
						 if(0 == buf[0] || 2 == buf[0] ){
							 record.bill.aftermoney = Common_LittleToInt(&buf[1]);  //余额
							 record.bill.energy = Common_LittleToInt(&buf[5]);   //电量
							 record.bill.serivemoney = Common_BigToInt(&buf[9]);  //服务费
							 record.bill.energymoney = Common_BigToInt(&buf[13]); //电费
							 record.bill.parkmoney = Common_BigToInt(&buf[17]);  //站桩费
							 record.bill.billmoney = record.bill.energymoney \
	                + record.bill.serivemoney + record.bill.bespkmoney + record.bill.parkmoney;
						 }
						record.bill.IsReport = 1 ;
						record.crc = Crc16_Calc((uint8 *)&record,sizeof(RECORD_ST)-2);
						 SaveStoreData(SST_IC,(uint8 *)&record,EEROM_RECORD_ADR + i*sizeof(RECORD_ST),sizeof(RECORD_ST)); 
						return 1;
					}
				}
			}
			Delay5Ms(1);
	}
	return 0;
}

/*清除记录索引*/
uint8 Rd_ClearE2romRecordData(void)
{
	gTempE2ROMTabl.currrdno = 0;
	gTempE2ROMTabl.tablrdno = 0;
	gTempE2ROMTabl.totlrdno = 0;
	gTempE2ROMTabl.baktolrdno = 0;
	gTempE2ROMTabl.nType = ETEMP_RECORD;

	Rd_E2romSaveCtrlParam(FM_IC,&gTempE2ROMTabl,EEROM_TEMPPARATABL_ADR);
	Rd_E2romSaveCtrlParam(FM_IC,&gTempE2ROMTabl,EEROM_TEMPBACK1TABL_ADR);

	gFullE2ROMTabl.currrdno = 0;
	gFullE2ROMTabl.tablrdno = 0;
	gFullE2ROMTabl.totlrdno = 0;
	gFullE2ROMTabl.baktolrdno = 0;
	gFullE2ROMTabl.nType = EFULL_RECORD;
	Rd_E2romSaveCtrlParam(FM_IC,&gFullE2ROMTabl,EEROM_FULLPARATABL_ADR);
	Rd_E2romSaveCtrlParam(FM_IC,&gFullE2ROMTabl,EEROM_FULLBACK1TABL_ADR);

	return 0;	
}

uint8 Rd_InitE2romData(void) 
{
	 int rtn;
	 gRdE2romLock = OSSemCreate(1);
	
	 rtn = Rd_E2romReadCtrlParam(FM_IC,&gTempE2ROMTabl,EEROM_TEMPPARATABL_ADR);
	 if( I2C_FALSE == rtn ) {
		 rtn = Rd_E2romReadCtrlParam(FM_IC,&gTempE2ROMTabl,EEROM_TEMPBACK1TABL_ADR);
		 if(I2C_FALSE == rtn) {
			 /*存储器错误*/
			 //while(1); /*调试时使用*/
			 gTempE2ROMTabl.currrdno = 0;
			 gTempE2ROMTabl.tablrdno = 0;
			 gTempE2ROMTabl.totlrdno = 0;
			 gTempE2ROMTabl.baktolrdno = 0;
			 gTempE2ROMTabl.nType = ETEMP_RECORD;
			 
			 Rd_E2romSaveCtrlParam(FM_IC,&gTempE2ROMTabl,EEROM_TEMPPARATABL_ADR);
			 Rd_E2romSaveCtrlParam(FM_IC,&gTempE2ROMTabl,EEROM_TEMPBACK1TABL_ADR);
			 //return 0;
		 }
	 }
	 gTempE2ROMTabl.nType = ETEMP_RECORD;
	 
	 rtn = Rd_E2romReadCtrlParam(FM_IC,&gFullE2ROMTabl,EEROM_FULLPARATABL_ADR);
	 if( I2C_FALSE == rtn ) {
		 rtn = Rd_E2romReadCtrlParam(FM_IC,&gFullE2ROMTabl,EEROM_FULLBACK1TABL_ADR);
		 if(I2C_FALSE == rtn) {
			 /*存储器错误*/
			 //while(1); /*调试时使用*/
			 gFullE2ROMTabl.currrdno = 0;
			 gFullE2ROMTabl.tablrdno = 0;
			 gFullE2ROMTabl.totlrdno = 0;
			 gFullE2ROMTabl.baktolrdno = 0;
			 gFullE2ROMTabl.nType = EFULL_RECORD;
			 Rd_E2romSaveCtrlParam(FM_IC,&gFullE2ROMTabl,EEROM_FULLPARATABL_ADR);
			 Rd_E2romSaveCtrlParam(FM_IC,&gFullE2ROMTabl,EEROM_FULLBACK1TABL_ADR);
			 return 0;
		 }
	 }
	 gFullE2ROMTabl.nType = EFULL_RECORD;
	  
	 return 1;
}

uint32 Rd_GetMaxRecord(void)
{
	return gFullE2ROMTabl.totlrdno;
}

/**********************************************************************************************************/





















