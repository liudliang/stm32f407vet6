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
//#include "M_Global.h"
#include "Uart.h"
#include "common.h"
#include "ModbusMaster.h"
#include "TaskDcMeter.h"
#include "ChgData.h"
#include "wdg.h"
#include "TaskAcMeter.h"
#include "TaskBackComm.h"
#include "HeLiBmsProto.h"
#include "Gpio.h"

#ifdef DC_MET_ON

OS_EVENT *gUartCommLock = NULL;



DEV_METER_TYPE gMeterData[DEF_MAX_GUN_NO];
//DEV_METER_PARAM gMeterParam;

#define METER_REG_START_ADR  0x14
#define METER_REG_NUM        0x06

typedef enum
{
	READ_ENERGY = 0,
	READ_VOLT = 1,
}E_READTYPE;


const SEQ_ADR_ST conDcMetSeqAdr[] = {\
	{0,2},
	{1,3},
};


/**********��˾ʹ��ֱ����*************/
const METER_REG_DEF conEMeterReg[] = \
{
	{0,0x0000,0x0002},
  {1,0x0014,0x0006},
};

static uint8 AnlyzeMeterEnergy(uint8 gunNo,uint8 *data,uint8 len)
{
	 u32toc_u energy;
	 energy.c[3] = data[3];
	 energy.c[2] = data[4];
	 energy.c[1] = data[5];
	 energy.c[0] = data[6];
	 
	 DEV_METER_TYPE *pMet = TskDc_GetMeterDataPtr(gunNo);
	 pMet->energy = energy.i;
   return energy.i;
}

static uint8 AnlyzeMeterVoltCurr(uint8 gunNo,uint8 *data,uint8 len)
{
	 static uint8 cnt[DEF_MAX_GUN_NO][2]={0};
	 u32toc_u u32da;
	 u32da.c[3] = data[7];
	 u32da.c[2] = data[8];
	 u32da.c[1] = data[9];
	 u32da.c[0] = data[10];

	 DEV_METER_TYPE *pMet = TskDc_GetMeterDataPtr(gunNo);
	 if(data[7]&0x80)  //������Ϊ0
	 {
		  if(++cnt[gunNo][0]>10){
			  pMet->current = 0;
			}
	 }  
   else{
	   cnt[gunNo][0] = 0;
	   pMet->current = (u32da.i + 400 )/1000;  /*Ϊʵ�ʵ�����10�� ���˴�Э���ĵ�������*/
	 }
	
	 u32da.c[3] = data[11];
	 u32da.c[2] = data[12];
	 u32da.c[1] = data[13];
	 u32da.c[0] = data[14];

	 if(data[11]&0x80)  //������Ϊ0
	 {
		  if(++cnt[gunNo][1]>10){
			  pMet->volt = 0;
			}
	 }  
   else{
		 cnt[gunNo][1] = 0;
	   pMet->volt = (u32da.i + 400)/1000;     /*Ϊʵ�ʵ�ѹ��10�� ���˴�Э���ĵ�������*/
	 }
	 return 1;
}

DEV_METER_TYPE *TskDc_GetMeterDataPtr(uint8 gunNo)
{
	return &gMeterData[gunNo];
}


extern uint32 UartHandler[];

static uint32 gUartMetHandle;


/* ��ʼ������ */
static void TskDc_InitCom(void)
{
	int32 tmp;
	PARAM_DEV_TYPE *param = ChgData_GetDevParamPtr();
	/* �򿪴��� */
	UartHandler[METER_COM] = Uart_Open(METER_COM);
	gUartMetHandle = UartHandler[METER_COM];
	
	/*���ò�����*/
 	//tmp  = param->dcmetbaud;  //METER_BAUD;
	tmp = 19200;
	
	Uart_IOCtrl(UartHandler[METER_COM], SIO_BAUD_SET, &tmp);	

	/*����У��λ*/
	tmp  = USART_Parity_Odd;//USART_Parity_Even;
	Uart_IOCtrl(UartHandler[METER_COM], SIO_PARITY_BIT_SET, &tmp);

	/*����ֹͣλ*/
	tmp  = USART_StopBits_1;
	Uart_IOCtrl(UartHandler[METER_COM], SIO_STOP_BIT_SET, &tmp);	
	
	/*��������Ϊ*/
	tmp = USART_WordLength_9b;
	Uart_IOCtrl(UartHandler[METER_COM], SIO_DATA_BIT_SET, &tmp);	
	
	if( NULL == gUartCommLock ) {
		gUartCommLock = OSSemCreate(1);
	}
}


void TskDc_InitData(void)
{
	 memset(&gMeterData,0,sizeof(DEV_METER_TYPE));
	 TskDc_InitCom();
	
}



static uint8 ReadDcMeterData(uint8 gunNo,E_READTYPE context)
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

#ifndef STM32F4XX
   uint8 err;
   OSSemPend(gUartCommLock, 0, &err);
#endif 

   Uart_ClearReadBuf(gUartMetHandle);
   len = Modbus_ReadPkg(buf,conDcMetSeqAdr[gunNo].u8Adr,reg,regnum);
   Uart_Write(gUartMetHandle, buf, len);

	 Delay10Ms(2);
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

#ifndef STM32F4XX	
	OSSemPost(gUartCommLock);
#endif
	 
	tmplen = 0;
	if( len >= (regnum*2)+5 ) {
			tmplen = Modbus_CheckReadPkg(buf,conDcMetSeqAdr[gunNo].u8Adr,MODBUS_CMD_03,regnum,len);
			if( (tmplen > 1) && (buf[0] == conDcMetSeqAdr[gunNo].u8Adr)) {
				if(context == READ_ENERGY ) {
					AnlyzeMeterEnergy(gunNo,buf,len);
				}else if( context == READ_VOLT ) {
					AnlyzeMeterVoltCurr(gunNo,buf,len);
				}
			}
  }
	return tmplen;
}



#else

DEV_METER_TYPE gMeterData[DEF_MAX_GUN_NO];

DEV_METER_TYPE *TskDc_GetMeterDataPtr(uint8 gunNo)
{
	return &gMeterData[gunNo];
}

#endif

static void CheckMeterDataErr(uint8 gunNo)
{
	static uint8 errCnt[2][2] = {0};
  
  PARAM_OPER_TYPE *param = ChgData_GetRunParamPtr();
	DEV_METER_TYPE *pMet = TskDc_GetMeterDataPtr(gunNo);
	
	if( pMet->volt > param->overdcvolt ) {
		 if( errCnt[gunNo][0]++ > 5 ) {
			 pMet->statu.bits.vlOver = 1;
		 }
	}else {
		errCnt[gunNo][0] = 0;
		pMet->statu.bits.vlOver = 0;
	}
	if( pMet->current > param->overdccurr ||
		pMet->current > (param->singmodcurr*param->modnum+100)) { /* ����10A */
		if( errCnt[gunNo][1]++ > 20 ) {
			 pMet->statu.bits.currOver = 1;
		 }
	}else {
		errCnt[gunNo][1] = 0;
		pMet->statu.bits.currOver = 0;
	}
}


//�Ƴ�ʼ��״̬
void led_init(void)
{
	LEDOFF(LED4);    //red
	LEDOn(LED5);     //green   
	LEDOFF(LED6);	   //yellow
}

////�ϵ� - ��ɫ; ����� �C ��ɫ; ���� �C ��ɫ
////�ߵ�ƽ������,�͵�ƽϨ���.
//void led_show(void)
//{
//	static uint8 s_agun_work_step = STEP_IDEL;
//	
//	if(s_agun_work_step != GetWorkStep(AGUN_NO))
//	{
//		s_agun_work_step = GetWorkStep(AGUN_NO);
//		if(STEP_CHARGEING == GetWorkStep(AGUN_NO))
//		{
//			LEDOn(LED5);
//		}
//		else
//		{
//			LEDOFF(LED5);
//		}
//	}

//	
//}


void TskDcAc_InitData(void)
{
	 memset(&gMeterData,0,sizeof(DEV_METER_TYPE));
	 memset(TskAc_GetMeterDataPtr(AGUN_NO),0,sizeof(DEV_ACMETER_TYPE));
//	 TskAc_InitData();
	led_init();
}

#define MET_ERR_TIMES 200
void TaskDcMeter(void *p_arg)
{



#ifndef DC_AC_ISO_REALCHECK
	uint8 u8AdrSeq = 0;
	uint16 cnt[DEF_MAX_GUN_NO] = {0};
	DEV_METER_TYPE *pMet = NULL;
	
	TskDc_InitData();
#else
	uint8 u8AdrSeq = 0,cyclecnt = 0;
	uint16 cnt[DEF_MAX_GUN_NO] = {0};
	DEV_METER_TYPE *pMet = NULL;
	uint8 gunno = AGUN_NO;
	MSG_STRUCT msg;
	PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();

	TskDcAc_InitData();   //��ģ��ɼ���ѹ����
#ifdef DC_MET_ON
	TskDc_InitData();
#endif

//	gPtrRunData = ChgData_GetRunDataPtr(AGUN_NO);
  	InitCC1ADCValue(0);
  
  #ifdef STM32F4XX
	InitCC1ADCValue(1);
  #endif

#ifdef ISO_ON

#if (ISO_TYPE ==  ISO_AUNICE) /*�����Բ���Եģ��*/
	memset(&msg,0,sizeof(MSG_STRUCT));
  	init_data();
	TskIso_InitCom();
	Message_QueueCreat(APP_TASK_DCMETER_PRIO);
#endif	

#if (ISO_TYPE ==  WBISO_SINGLE) 
	uint8 u8IsoSeq = 0;
//	MSG_STRUCT msg;
	memset(&msg,0,sizeof(MSG_STRUCT));
	init_data();
	TskIso_InitCom();
	Message_QueueCreat(APP_TASK_DCMETER_PRIO);
#endif

#endif

#ifdef AC_METER_ON 
  uint16 ac_cnt = 0;
  //uint8 rtn = 0;
  DEV_ACMETER_TYPE *pMetAc = TskAc_GetMeterDataPtr(0);
  PARAM_DEV_TYPE *param = ChgData_GetDevParamPtr();  
  TskAc_InitData();
#endif  

#endif	

	resetInput();
pMet = TskDc_GetMeterDataPtr(u8AdrSeq);	
	while(1)
	{
		TaskRunTimePrint("TaskDcMeter begin", OSPrioCur);

#ifdef DC_MET_ON
		 
		 if( ReadDcMeterData(u8AdrSeq,READ_ENERGY) > 0) {
			 cnt[u8AdrSeq] = 0;
			 pMet->statu.bits.commerr = 0;
		 }else {
			 cnt[u8AdrSeq]++;
		 }
//palti 20200220		 Delay5Ms(1);
		 
		
//		TaskRunTimePrint("TaskDcMeter0", OSPrioCur);
		
		 
		 if(ReadDcMeterData(u8AdrSeq,READ_VOLT) > 0) {
			  cnt[u8AdrSeq] = 0;
			  pMet->statu.bits.commerr = 0;
		 }else {
			 cnt[u8AdrSeq]++;
		 }

//		 TaskRunTimePrint("TaskDcMeter1", OSPrioCur);
		 
		 CheckMeterDataErr(u8AdrSeq);
		 
		 if( cnt[u8AdrSeq] > MET_ERR_TIMES ) {
			  pMet->statu.bits.commerr = 1;
			  if( cnt[u8AdrSeq] > 3 * MET_ERR_TIMES ) {
					TskDc_InitCom();
					cnt[u8AdrSeq] = 0;
				}
		 }
		 
//		 TaskRunTimePrint("TaskDcMeter2", OSPrioCur);
		 
		PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
		u8AdrSeq++;		 
		u8AdrSeq %= devparaPtr->gunnum;//DEF_MAX_GUN_NO;

#endif

#ifdef DC_AC_ISO_REALCHECK
		/**********************************Task_RealCheck****************************************/
        
	//    DebugInfoByChk("ʵʱ��⿪ʼ");
	 #ifdef STM32F10X_HD
		Check_N1_2_3_InputIo();
	  	Check_SoftRSTKeyBoard();	//��λ��ť����
	 #endif

//--------------------------����Ƿ��������ѹ����ȡ��ѹ������ʱ
	 CheckMeterDataErr(u8AdrSeq);
	 CheckACMeterDataErr();
	 
	 if(pMet->commOvertimecnt > (500/APP_TASK_DCMETER_DELAY)) //�����5s��ʱ
	 {
	 	pMet->statu.bits.commerr = 1;
	 }
	 else
     {
        if( STEP_CHARGEING == TskMain_GetWorkStep(u8AdrSeq) )
        {
        	pMet->commOvertimecnt++;
        }
		pMet->statu.bits.commerr = 0;
     }
	 
//--------------------------

		Check_AllInputIo();
	 
		Check_TranIOStatus();

		Check_stopBtn();		 //��ͣ����

		Check_DialcodeBoard();	 //���뿪�ؼ��

		Check_GunTemper();		 //���ǹ���¶�

		Check_BcpDCOverVolt();	 //���������г���ѹ���ܴ���BCP��ߵ�ѹ
//		CheckGunFirstConnc(gunno);

//		gunno = (gunno + 1)%2;

//		TaskRunTimePrint("TaskDcMeter3", OSPrioCur);
		
//	    DebugInfoByChk("ʵʱ������");
		/******************************Task_IsoCheck**********************************************************/
//	    DebugInfoByChk("��Ե��⿪ʼ");
#ifdef ISO_ON

#if (ISO_TYPE ==  ISO_AUNICE) /*�����Բ���Եģ��*/

		if(RcvMsgFromQueue(&msg) == TRUE)
		{
			TskIso_DealMsg(&msg);
		}
		
		TskIso_ReadAndDealData();
		
		CheckACMeterDataErr();
#endif

#if (ISO_TYPE ==  WBISO_SINGLE) 

//		TaskRunTimePrint("TaskDcMeter4", OSPrioCur);

		if(RcvMsgFromQueue(&msg) == TRUE)
		{
			TskIso_DealMsg(&msg);
		}

		 TskWbIso_ReadAndDealData(u8IsoSeq);
		 u8IsoSeq++;
		 
//		 TaskRunTimePrint("TaskDcMeter5", OSPrioCur);
		 
//		 PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
		 u8IsoSeq %= devparaPtr->gunnum;//WBISO_NUM;
#endif
//		DebugInfoByChk("��Ե������");
#endif

/********************************************TaskAcMeter**************************************************/
#ifdef AC_METER 
 //	DebugInfoByCon("������⿪ʼ");

if( ReadAcMeterData() > 0) {
		ac_cnt = 0;
		pMetAc->statu.bits.commerr = 0;
	}

//	TaskRunTimePrint("TaskDcMeter6", OSPrioCur);
	
	CheckACMeterDataErr();

	if( ac_cnt++ > MET_ERR_TIMES ) {
		pMetAc->statu.bits.commerr = 1;
		pMetAc->Uab = 0;
		pMetAc->Ubc = 0;
		pMetAc->Uca = 0;
		pMetAc->Iab = 0;
		pMetAc->Ibc = 0;
		pMetAc->Ica = 0;
	}
	
	DebugInfoWithPx(CHK_MOD,"����״̬",pMetAc->statu.word);

//DebugInfoByChk("����������");
	
#endif

#endif
		
		TaskRunTimePrint("TaskDcMeter end", OSPrioCur);

//		 Wdg_feeddog();
	if((BMS_HELI == BackCOMM->agreetype) && ((cyclecnt++) > 7))
	{
		cyclecnt = 0;
		heli_polling();
	}
	
		Delay10Ms(APP_TASK_DCMETER_DELAY);



	 }
}







