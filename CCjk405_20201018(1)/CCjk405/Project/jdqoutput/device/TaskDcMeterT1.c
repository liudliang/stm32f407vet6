/**
  ******************************************************************************
  * @file    TaskDcMeter.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
*/

#include <string.h>
#include "M_Global.h"
#include "Uart.h"
#include "common.h"
#include "ModbusMaster.h"
#include "TaskDcMeter.h"
#include "ChgData.h"
#include "wdg.h"
OS_EVENT *gUartCommLock = NULL;



DEV_METER_TYPE gMeterData;
//DEV_METER_PARAM gMeterParam;

#define METER_REG_START_ADR  0x14
#define METER_REG_NUM        0x06

typedef enum
{
	READ_ENERGY = 0,
	READ_VOLT = 1,
}E_READTYPE;


const METER_REG_DEF conEMeterReg[] = \
{
	{0,0x000C,0x0002},
  {1,0x0048,0x0002},
};

static uint8 AnlyzeMeterEnergy(uint8 *data,uint8 len)
{
	 u32toc_u energy;
	 energy.c[3] = data[3];
	 energy.c[2] = data[4];
	 energy.c[1] = data[5];
	 energy.c[0] = data[6];
	 
	 DEV_METER_TYPE *pMet = TskDc_GetMeterDataPtr();
	 pMet->energy = energy.i;
   return energy.i;
}

static uint8 AnlyzeMeterVoltCurr(uint8 *data,uint8 len)
{
	 stoc_u u32da;
	
	 u32da.c[1] = data[5];
	 u32da.c[0] = data[6];
	  
	 DEV_METER_TYPE *pMet = TskDc_GetMeterDataPtr();
	 pMet->current = (u32da.s + 0 )/100;  /**/
	
	 u32da.c[1] = data[3];
	 u32da.c[0] = data[4];

	 pMet->volt = (u32da.s + 0)/10;     /**/
	 return 1;
}


DEV_METER_TYPE *TskDc_GetMeterDataPtr(uint8 gunNo)
{
	return &gMeterData[gunNo];
}

//DEV_METER_PARAM *TskDc_GetParamPtr(void)
//{
//	return &gMeterParam;
//}


extern uint32 UartHandler[];

static uint32 gUartMetHandle;


/* 初始化串口 */
static void TskDc_InitCom(void)
{
	int32 tmp;
	PARAM_DEV_TYPE *param = ChgData_GetDevParamPtr();
	/* 打开串口 */
	UartHandler[METER_COM] = Uart_Open(METER_COM);
	gUartMetHandle = UartHandler[METER_COM];
	
	/*设置波特率*/
 	//tmp  = param->dcmetbaud;  //METER_BAUD;
	tmp = 9600;
	
	Uart_IOCtrl(UartHandler[METER_COM], SIO_BAUD_SET, &tmp);	

	/*设置校验位*/
	tmp  = USART_Parity_Even;//USART_Parity_Even;
	Uart_IOCtrl(UartHandler[METER_COM], SIO_PARITY_BIT_SET, &tmp);

	/*设置停止位*/
	tmp  = USART_StopBits_1;
	Uart_IOCtrl(UartHandler[METER_COM], SIO_STOP_BIT_SET, &tmp);	
	
	/*设置数据为*/
	tmp = USART_WordLength_9b;
	Uart_IOCtrl(UartHandler[METER_COM], SIO_DATA_BIT_SET, &tmp);	
	
	if( NULL == gUartCommLock ) {
		gUartCommLock = OSSemCreate(1);
	}
}


void TskDc_InitData(void)
{
	 memset(&gMeterData,0,sizeof(DEV_METER_TYPE));
	
//	 CHARGE_TYPE *pRunData = ChgData_GetRunDataPtr();
//	 pRunData->meter = &gMeterData;
	
	 TskDc_InitCom();
	
}


static uint8 ReadDcMeterData(uint8 adr,E_READTYPE context)
{
	 uint8 reg,regnum;
	 uint8 rlen,tmplen;
	 uint8 cnt = 0;
	 uint8 len = 0;
	 uint8 *ptr = NULL;
	 #define MAX_DCMET_SIZE  32
   uint8 buf[MAX_DCMET_SIZE] = {0};
  
   reg = conEMeterReg[context%2].startReg;
   regnum = conEMeterReg[context%2].regNum;

   uint8 err;
   OSSemPend(gUartCommLock, 0, &err);

   Uart_ClearReadBuf(gUartMetHandle);
   len = Modbus_ReadPkg(buf,adr,reg,regnum);
   Uart_Write(gUartMetHandle, buf, len);

	 Delay10Ms(1);
	 memset(buf,0,MAX_DCMET_SIZE);
	 
	 cnt = 0;
	 len = 0;
	 ptr = &buf[0];
	 tmplen = MAX_DCMET_SIZE;
   do {
		 rlen = Uart_Read(gUartMetHandle,ptr,MAX_DCMET_SIZE-len);
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
	 
	OSSemPost(gUartCommLock);
	 
	tmplen = 0;
	if( len >= (regnum*2)+5 ) {
			tmplen = Modbus_CheckReadPkg(buf,adr,MODBUS_CMD_03,regnum,len);
			if( tmplen > 1 ) {
				if(context == READ_ENERGY ) {
					AnlyzeMeterEnergy(buf,len);
				}else if( context == READ_VOLT ) {
					AnlyzeMeterVoltCurr(buf,len);
				}
			}
  }
	

	
	return tmplen;
}

static void CheckMeterDataErr(void)
{
	static uint8 errCnt[2] = {0};
  
  PARAM_OPER_TYPE *param = ChgData_GetRunParamPtr();

	DEV_METER_TYPE *pMet = TskDc_GetMeterDataPtr();
	if( pMet->volt > param->overdcvolt ) {
		 if( errCnt[0]++ > 5 ) {
			 pMet->statu.bits.vlOver = 1;
		 }
	}else {
		errCnt[0] = 0;
		pMet->statu.bits.vlOver = 0;
	}
	
	if( pMet->current > param->overdccurr ) {
		if( errCnt[1]++ > 5 ) {
			 pMet->statu.bits.currOver = 1;
		 }
	}else {
		errCnt[0] = 1;
		pMet->statu.bits.currOver = 0;
	}
	
}


#define MET_ERR_TIMES 200
void TaskDcMeter(void *p_arg)
{
	uint16 cnt = 0;
	//uint8 rtn = 0;
	DEV_METER_TYPE *pMet = TskDc_GetMeterDataPtr();
  PARAM_DEV_TYPE *param = ChgData_GetDevParamPtr();
	
	TskDc_InitData();
	param->dcmetAdr = 2;
	while(1)
	{
		 if( ReadDcMeterData(param->dcmetAdr,READ_ENERGY) > 0) {
			 cnt = 0;
			 pMet->statu.bits.commerr = 0;
		 }else {
			 cnt++;
		 }
		 Delay5Ms(1);
		 if(ReadDcMeterData(param->dcmetAdr,READ_VOLT) > 0) {
			  cnt = 0;
			  pMet->statu.bits.commerr = 0;
		 }else {
			 cnt++;
		 }
		 
		 CheckMeterDataErr();
		 
		 if( cnt > MET_ERR_TIMES ) {
			  pMet->statu.bits.commerr = 1;
			  if( cnt > 3 * MET_ERR_TIMES ) {
					TskDc_InitCom();
					cnt = 0;
				}
			 
		 }
//		 Wdg_feeddog();
		 Delay10Ms(8);
	 }
}







