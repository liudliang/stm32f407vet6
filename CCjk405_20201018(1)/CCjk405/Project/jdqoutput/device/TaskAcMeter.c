/**
  ******************************************************************************
  * @file    TaskAcMeter.c
  * @author  
  * @version v1.0
  * @date    2018-08-03
  * @brief   
  ******************************************************************************
*/
#include "TaskIsoCheck.h"


#include <string.h>
//#include "M_Global.h"
#include "Uart.h"
#include "common.h"
#include "ModbusMaster.h"
#include "TaskAcMeter.h"
#include "ChgData.h"
#include "wdg.h"
#include "hwdevopt.h"

DEV_ACMETER_TYPE gAcMeterData;
DEV_ACMETER_TYPE *TskAc_GetMeterDataPtr(uint8 gunNo)
{
	return &gAcMeterData;
}

/****************************************************************************
**使用奥耐绝缘模块后，以下代码不参与编译
****************************************************************************/
#ifdef AC_METER  /*奥耐自产绝缘模块*/

const ACMETER_REG_DEF conACEMeterReg[] = \
{
	{1,0x0004,0x000a},

};


extern uint32 UartHandler[];

static uint32 gUartAcMetHandle;

extern OS_EVENT *gUartCommLock;
/* 初始化串口 */
static void TskAc_InitCom(void)
{
	int32 tmp;
	PARAM_DEV_TYPE *param = ChgData_GetDevParamPtr();
	/* 打开串口 */
	UartHandler[ACMETER_COM] = Uart_Open(ACMETER_COM);
	gUartAcMetHandle = UartHandler[ACMETER_COM];
	
	/*设置波特率*/
 	//tmp  = param->dcmetbaud;  //METER_BAUD;
	tmp = 9600;
	
	Uart_IOCtrl(UartHandler[ACMETER_COM], SIO_BAUD_SET, &tmp);	

	/*设置校验位*/
	tmp  = USART_Parity_Even;//USART_Parity_Even;
	Uart_IOCtrl(UartHandler[ACMETER_COM], SIO_PARITY_BIT_SET, &tmp);

	/*设置停止位*/
	tmp  = USART_StopBits_1;
	Uart_IOCtrl(UartHandler[ACMETER_COM], SIO_STOP_BIT_SET, &tmp);	
	
	/*设置数据为*/
	tmp = USART_WordLength_9b;
	Uart_IOCtrl(UartHandler[ACMETER_COM], SIO_DATA_BIT_SET, &tmp);	
	
#ifndef STM32F4XX	
	if( NULL == gUartCommLock ) {
		gUartCommLock = OSSemCreate(1);
	}
#endif
}


void TskAc_InitData(void)
{
	 memset(&gAcMeterData,0,sizeof(DEV_ACMETER_TYPE));
	
//	 CHARGE_TYPE *pRunData = ChgData_GetRunDataPtr();
//	 pRunData->meter = &gMeterData;
	
	 TskAc_InitCom();
	
}


static uint8 AnlyzeACMeter(uint8 *data,uint8 len)
{
	 stoc_u Volab,Volbc,Volca;
//	 stoc_u Currab,Currbc,Currca;
	 Volab.c[1] = data[3];
	 Volab.c[0] = data[4];
	 Volbc.c[1] = data[5];
	 Volbc.c[0] = data[6];
	 Volca.c[1] = data[7];
	 Volca.c[0] = data[8];

//	 Currab.c[1] = data[11];
//	 Currab.c[0] = data[12];
//	 Currbc.c[1] = data[13];
//	 Currbc.c[0] = data[14];
//	 Currca.c[1] = data[15];
//	 Currca.c[0] = data[16];
	 
	 DEV_ACMETER_TYPE *pMet = TskAc_GetMeterDataPtr(0);
	 pMet->Uab = Volab.s/10;
	 pMet->Ubc = Volbc.s/10;
	 pMet->Uca = Volca.s/10;
	 pMet->Iab = 0;
	 pMet->Ibc = 0;
	 pMet->Ica = 0;
	 return 1;

}

uint8 ReadAcMeterData()
{
	 uint8 reg,regnum;
	 uint8 rlen,tmplen;
	 uint8 cnt = 0;
	 uint8 len = 0;
	 uint8 *ptr = NULL;
	 #define MAX_DCMET_SIZE  32
   uint8 buf[MAX_DCMET_SIZE] = {0};
  
   reg = conACEMeterReg[0].startReg;
   regnum = conACEMeterReg[0].regNum;

#ifndef STM32F4XX
   uint8 err;
   OSSemPend(gUartCommLock, 0, &err);
#endif

   Uart_ClearReadBuf(gUartAcMetHandle);

   len = Modbus_ReadPkg(buf,1,reg,regnum); 
   Uart_Write(gUartAcMetHandle, buf, len);
	
	 Delay10Ms(2);
	 memset(buf,0,MAX_DCMET_SIZE);
	 
	 cnt = 0;
	 len = 0;
	 ptr = &buf[0];
	 tmplen = 0;
   do {
		 rlen = Uart_Read(gUartAcMetHandle,ptr,MAX_DCMET_SIZE-len);
		 if( rlen > 0 ) {
			ptr += rlen;
		  len += rlen;
			 
			if( len >= MAX_DCMET_SIZE) {
				break;
			}
			cnt = 0;
		}else {
			if( ( tmplen == len )&& ( len > 3 ) ) {
					break;
				}
    		tmplen = len;
				cnt++;
				Delay10Ms(2);
		}
	}while(cnt < 10 );
	 
#ifndef STM32F4XX
	OSSemPost(gUartCommLock);
#endif
	
	tmplen = 0;
	if( len >= (regnum*2)+5 ) {
			tmplen = Modbus_CheckReadPkg(buf,1,MODBUS_CMD_03,regnum,len);
			if( tmplen > 1 ) {
					AnlyzeACMeter(buf,len);
			}
  }
	
	return tmplen;
}
    




/*
*该任务功能由综合绝缘单元替代
*
*/
#define MET_ERR_TIMES 200
#ifndef DC_AC_ISO_REALCHECK
void TaskAcMeter(void *p_arg)
{
	uint16 cnt = 0;
	//uint8 rtn = 0;
	DEV_ACMETER_TYPE *pMet = TskAc_GetMeterDataPtr(0);
  PARAM_DEV_TYPE *param = ChgData_GetDevParamPtr();
	
	TskAc_InitData();
	while(1)
	{
		 if( ReadAcMeterData() > 0) {
			 cnt = 0;
			 pMet->statu.bits.commerr = 0;
		 }
		 
		 CheckACMeterDataErr();
		 
		 if( cnt++ > MET_ERR_TIMES ) {
			 pMet->statu.bits.commerr = 1;
			 pMet->Uab = 0;
			 pMet->Ubc = 0;
			 pMet->Uca = 0;
			 pMet->Iab = 0;
			 pMet->Ibc = 0;
			 pMet->Ica = 0;
		 }
//		Wdg_feeddog();
		Delay10Ms(5);
	 }
}
#endif

#endif  //(ISO_TYPE !=  ISO_AUNICE)
void CheckACMeterDataErr(void)
{
	static uint8 errCnt[9] = {0};
  
  PARAM_OPER_TYPE *param = ChgData_GetRunParamPtr();

	DEV_ACMETER_TYPE *pMet = TskAc_GetMeterDataPtr(0);
	if( pMet->Uab > param->overacvolt ) {
		 if( errCnt[0]++ > 5 ) {
			 pMet->statu.bits.UabOver = 1;
		 }
	}else if( (pMet->Uab > 0) && (pMet->Uab < param->underacvolt) ) {
		 if( errCnt[1]++ > 5 ) {
			 pMet->statu.bits.UabUnder = 1;
		 }
	}else{
		errCnt[0] = 0;
		errCnt[1] = 0;
		pMet->statu.bits.UabOver = 0;
		pMet->statu.bits.UabUnder = 0;
	}
	
	if( pMet->Ubc > param->overacvolt ) {
		 if( errCnt[2]++ > 5 ) {
			 pMet->statu.bits.UbcOver = 1;
		 }
	}else if( (pMet->Ubc > 0) && (pMet->Ubc < param->underacvolt) ) {
		 if( errCnt[3]++ > 5 ) {
			 pMet->statu.bits.UbcUnder = 1;
		 }
	}else{
		errCnt[2] = 0;
		errCnt[3] = 0;
		pMet->statu.bits.UbcOver = 0;
		pMet->statu.bits.UbcUnder = 0;
	}

	if( pMet->Uca > param->overacvolt ) {
		 if( errCnt[4]++ > 5 ) {
			 pMet->statu.bits.UcaOver = 1;
		 }
	}else if( (pMet->Uca > 0) && (pMet->Uca < param->underacvolt) ) {
		 if( errCnt[5]++ > 5 ) {
			 pMet->statu.bits.UcaUnder = 1;
		 }
	}else{
		errCnt[4] = 0;
		errCnt[5] = 0;
		pMet->statu.bits.UcaOver = 0;
		pMet->statu.bits.UcaUnder = 0;
	}
	
	if( ( pMet->Uab < 600 || pMet->Ubc < 600 || pMet->Uca < 600) \
		&& ( pMet->Uab > 3000 ||  pMet->Ubc > 3000 ||  pMet->Uca > 3000 ))
	{
		pMet->statu.bits.UDeficy = 1; /*三相缺相*/
	}else {
		pMet->statu.bits.UDeficy = 0;
	}

//过流故障暂未判断
	
}




