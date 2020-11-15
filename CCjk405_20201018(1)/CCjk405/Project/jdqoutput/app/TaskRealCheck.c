/**
  ******************************************************************************
  * @file    RealCheck.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#include "main.h"
#include "gpio.h"
#include "bms.h"
#include "ChgData.h"
#include "TaskRealCheck.h"
#include "TaskMainCtrl.h"
#include <stdlib.h>
#include "MeaureGunTemperature.h"
#include "common.h"

#include "Adc_Calc.h"
/**********************************************************************************
*STM32F407双枪的状态量
*IN0急停状态
*IN1交流输入断路器状态
*IN2交流接触器状态
*IN3防雷器
*IN4 A枪直流接触器状态
*IN5 B枪直流接触器状态
*IN6 功率分配接触器状态
*IN7 A枪熔丝状态
*IN8 B枪熔丝状态
*IN9  A枪电子锁反馈
*IN10 B枪电子锁反馈
*IN11 门禁反馈
*IN12 烟雾报警
*IN13 空
*IN14 平衡报警
*IN15  枪归位检测
*
*TK1 A枪温度1
*TK2 A枪温度2
*TA1 B枪温度1
*TA2 B枪温度2
*TB1 桩体温度1
*TB2 桩体温度2
*
*DC2-SWI/O B枪直流接触器控制
*DC1-SWI/O A枪直流接触器控制 
*AC2-SWI/O A枪电子锁控制
*AC1-SWI/O 交流接触器控制
*FAN2I/O   B枪泄放电阻
*FAN1I/O   A枪泄放电阻
*GROUP2I/O 功率分配控制
*
*
*
***********************************************************************************/
typedef enum
{
	CC1_SEQ = 0x01,
	ELOCK_SEQ = 0x02,
	K3K4_SEQ = 0x03,
	KM_SEQ   = 0x04,
	
}INPUT_SEQ_DEFINE;

u16tobit_u gInputStu; /*状态量采集 N2状态放在Byte0 N3状态放在byte1*/

DEV_INPUT_TYPE gInput[DEF_MAX_GUN_NO];
DEV_GUN_TYPE   gGun[DEF_MAX_GUN_NO];
	
extern CHARGE_TYPE * gPtrRunData;
extern CHARGE_TYPE *ChgData_GetRunDataPtr(uint8 gunNo);
extern DEV_RELAY_TYPE * Relay_GetRelayDataPtr(uint8 no);

static uint8 Gpio_ReadInputIo_CC1_SEQ(uint8 Gunno);

DEV_INPUT_TYPE *Check_GetInputDataptr(uint8 gunNo)
{
	return &gInput[gunNo%DEF_MAX_GUN_NO];
}

DEV_GUN_TYPE *Check_GetGunDataPtr(uint8 gunNo)
{
	return &gGun[gunNo];
}

uint8 Check_CC1Statu(uint8 gunNo)
{
	 uint8 stu = gGun[gunNo].statu.bits.cc1stu;
	 return stu;
}


const uint8 conCheckIOseq[] = {CC1_SEQ,ELOCK_SEQ,KM_SEQ,K3K4_SEQ};
#define REAL_CHECK_IO_NUM  sizeof(conCheckIOseq)

#ifdef STM32F10X_HD
uint8 Check_ChoseChl(uint8 chl)
{
		uint8 tmp = chl%8;
		WriteLoutputDataBit(LOUTPUT3_SA2, (BitAction)((tmp&(1<<2))>>2)); 
		WriteLoutputDataBit(LOUTPUT2_SA1, (BitAction)((tmp&(1<<1))>>1)); 
		WriteLoutputDataBit(LOUTPUT1_SA0, (BitAction)((tmp&(1<<0))>>0)); 

		WriteLoutputDataBit(LOUTPUT6_SB2, (BitAction)((tmp&(1<<2))>>2)); 
		WriteLoutputDataBit(LOUTPUT5_SB1, (BitAction)((tmp&(1<<1))>>1)); 
		WriteLoutputDataBit(LOUTPUT4_SB0, (BitAction)((tmp&(1<<0))>>0)); 

		WriteLoutputDataBit(LOUTPUT9_SC2, (BitAction)((tmp&(1<<2))>>2)); 
		WriteLoutputDataBit(LOUTPUT8_SC1, (BitAction)((tmp&(1<<1))>>1)); 
		WriteLoutputDataBit(LOUTPUT7_SC0, (BitAction)((tmp&(1<<0))>>0)); 

		return tmp;
}

uint8 Check_N1_2_3_InputIo(void)
{
   uint16 i,j;   //INPUT0
	 uint8 tmpstu1[8] = { 0 };
	 uint8 tmpstu2[8] = { 0 };
   uint8 tmpstu2_1[8] = { 0 };
   uint8 tmpstu2_2[8] = { 0 };	

	 for( i = 0 ; i < 8 ; i++ )
	 {
		  Check_ChoseChl(i);
		  for( j = 0 ; j < 2000; j++ ) {__nop(); }
		  tmpstu1[i] = (~Gpio_ReadInputIo(INPUT0)) & 0x01 ; //N2 上的INPUTIO
			tmpstu2[i] = (~Gpio_ReadInputIo(INPUT1)) & 0x01 ; //N3 上的INPUTIO
	 }
   Delay10Ms(1);
	 
	 for( i = 0 ; i < 8 ; i++ )
	 {
			Check_ChoseChl(i);
			for( j = 0 ; j < 2000; j++ ) {__nop(); }
			tmpstu2_1[i] = (~Gpio_ReadInputIo(INPUT0)) & 0x01 ; //N2 上的INPUTIO
			tmpstu2_2[i] = (~Gpio_ReadInputIo(INPUT1)) & 0x01; //N3 上的INPUTIO
	 }
		 
	for( i = 0 ; i < 8 ; i++ )
	{
		 if (tmpstu1[i] == tmpstu2_1[i] ) {
				if( 1 == tmpstu1[i] ) {
					gInputStu.byte[0] |= (1 << i);
				}else {
					gInputStu.byte[0] &= ~(1 << i );
				}
		 } 
		 if (tmpstu2[i] == tmpstu2_2[i] ) {
				if( 1 == tmpstu2[i] ) {
					gInputStu.byte[1] |= (1 << i);
				}else {
					gInputStu.byte[1] &= ~(1 << i );
				}
		 }
	}
		
	return 0;
}


void Check_TranIOStatus(void)
{
	PARAM_OPER_TYPE *param = ChgData_GetRunParamPtr();
	
	DEV_INPUT_TYPE *pInputDataptr = Check_GetInputDataptr(0);
	DEV_GUN_TYPE *pGunDataPtr = Check_GetGunDataPtr(0);
	DEV_RELAY_TYPE *pRelayDataPtr = Relay_GetRelayDataPtr(0);
	
	pInputDataptr->statu.bits.dckmp   = gInputStu.bits.bit0;   /*正直流接触器状态*/
	pInputDataptr->statu.bits.dckmn   = gInputStu.bits.bit1;   /*负直流接触器状态*/
	pInputDataptr->statu.bits.stop    = gInputStu.bits.bit2 ;  /*急停*/
	pInputDataptr->statu.bits.ackm    = gInputStu.bits.bit3;   /*交流接触器*/
	pInputDataptr->statu.bits.spd     = gInputStu.bits.bit5;   /*防雷器*/
	pGunDataPtr->statu.bits.elockstu  = ~gInputStu.bits.bit12&0x01;  /*电子锁反馈 */
	
	pInputDataptr->statu.bits.acqf    = 0;   /*交流断路器*/
	pInputDataptr->statu.bits.posfuse = 0;   /*正熔丝*/
	pInputDataptr->statu.bits.negfuse = 0;   /*负熔丝*/
	
	if(0 == param->Sysparalarm.bits.curfewsignal){               //门禁信号
	  pInputDataptr->statu.bits.dooracs = gInputStu.bits.bit7;   /*门禁1 bit7 */	
	  pInputDataptr->statu.bits.dooraf  = gInputStu.bits.bit7;   /*门禁2 bit7 后门*/
	}
	else{
		pInputDataptr->statu.bits.dooracs = ~gInputStu.bits.bit7&0x01;   /*门禁1*/	
	  pInputDataptr->statu.bits.dooraf  = ~gInputStu.bits.bit7&0x01;   /*门禁2*/
	}
	
	
	pInputDataptr->statu.bits.fan     = 0;   /*风机*/
	pInputDataptr->statu.bits.smoke   = 0;   /*烟雾告警*/
	pInputDataptr->statu.bits.water   = 0;   /*水浸告警*/
	
	pRelayDataPtr->statu.bits.km1 = pInputDataptr->statu.bits.dckmp;
	pRelayDataPtr->statu.bits.km2 = pInputDataptr->statu.bits.dckmn;
		
}

#endif




/*
** 所有的输入IO在此处处理，其它任务取相应的状态即可
*/
#ifdef STM32F10X_HD
uint8 Check_AllInputIo(void)
{
	DEV_GUN_TYPE *pGunDataPtr = Check_GetGunDataPtr(0);
	DEV_INPUT_TYPE *pInputDataptr = Check_GetInputDataptr();
	DEV_LOGIC_TYPE *ptrStu =  ChgData_GetLogicDataPtr();
	ptrStu->runstu.bits.posFU = gInputStu.bits.bit0;
	ptrStu->runstu.bits.negFU = gInputStu.bits.bit1;
	ptrStu->runstu.bits.emcystop = gInputStu.bits.bit2;
	ptrStu->runstu.bits.acKm = pInputDataptr->statu.bits.ackm;
	ptrStu->runstu.bits.spd  = pInputDataptr->statu.bits.spd;
	ptrStu->runstu.bits.elockstu = pGunDataPtr->statu.bits.elockstu;
  ptrStu->runstu.bits.gunCC1 = Gpio_ReadInputIo_CC1_SEQ(0);
	gPtrRunData->gun->statu.bits.cc1stu = ptrStu->runstu.bits.gunCC1;
	return 0;
}
#endif


void Check_stopBtn(void)
{
	DEV_INPUT_TYPE *pInputDataptr = Check_GetInputDataptr(AGUN_NO);
  CHARGE_TYPE *PtrRunData = ChgData_GetRunDataPtr(AGUN_NO);
	
	if (1 == pInputDataptr->statu.bits.stop) {
		PtrRunData->logic->errCode = ECODE89_CHGJTST;
		PtrRunData->logic->stopReason = EERR_REASON;
		Check_SetErrCode(AGUN_NO,ECODE89_CHGJTST);
		Check_SetErrCode(BGUN_NO,ECODE89_CHGJTST);
    	RelayOut_BreakdownLed(AGUN_NO,CTRL_ON); /* 故障指示灯 */
		RelayOut_BreakdownLed(BGUN_NO,CTRL_ON); /* 故障指示灯 */
		
		//RelayOut_AcKmOut(JOUT_OFF); /* 断开交流接触器 */
	}else {
		if(Check_GetErrCode(AGUN_NO,ECODE89_CHGJTST)) { /*故障状态在主控中清除*/
				RelayOut_BreakdownLed(AGUN_NO,CTRL_OFF); /* 故障指示灯 */
			  RelayOut_BreakdownLed(BGUN_NO,CTRL_OFF); /* 故障指示灯 */
		}
  }
}

uint16 Check_SuitableVolt(uint8 gunNo)
{
	 uint16 volt = 0;
	 BMSDATA_ST *pBms = Bms_GetBmsCarDataPtr(gunNo);
	 CHGDATA_ST *pChg = Bms_GetChgDataPtr(gunNo);
	 if(pBms->bhm.maxvolte > pChg->cml.maxoutvolt ){
			 volt =  pChg->cml.maxoutvolt;
		}else {
			 volt = pBms->bhm.maxvolte;
			 if(volt < pChg->cml.minoutvolt ) {
					volt =  pChg->cml.minoutvolt;
			}	
		}
	  return volt;
}


/*判断接触器内侧是否达到开启模块的最低电压*/
uint8 Check_KmInVoltOver(uint8 gunNo,uint16 volt_startISO)
{
	int16 Vdc = ((CHARGE_TYPE *)ChgData_GetRunDataPtr(gunNo))->meter->volt;
	if( abs(Vdc) > volt_startISO ) {
		 return CHECK_TRUE;
	}
	return CHECK_FALSE;

}



/*判断接触器外侧电压是否小于某个值*/
uint8 Check_KmOutVoltLess(uint8 gunNo,uint16 volt)
{
	/*选择3路电压作为接触器外侧电压*/
//	int16 Vdc3 = ((CHARGE_TYPE *)ChgData_GetRunDataPtr(gunNo))->iso->vdc3;
    int16 Vdc3 = AdcCalc_GetValue()->vdciso[1];
#ifdef BMSTEST	 
	Vdc3 = volt-10; /*for 测试*/
#endif
	DebugInfoWithPi(CHK_MOD, "Check_KmOutVoltLess abs(Vdc3)", abs(Vdc3));
	DebugInfoWithPi(CHK_MOD, "Check_KmOutVoltLess volt", volt);


	if( abs(Vdc3) < volt ) {
		 return CHECK_TRUE;
	}
	return CHECK_FALSE;
}

/*判断接触器外侧电压是否小于某个值*/
uint8 Check_KmInVoltLess(uint8 gunNo,uint16 volt)
{
	/*选择3路电压作为接触器内侧电压*/
	//uint16 Vdc = ((CHARGE_TYPE *)ChgData_GetRunDataPtr())->iso->vdc1;
	/*103板，接触器内侧电压采用电能表电压*/
	uint16 Vdc = ((CHARGE_TYPE *)ChgData_GetRunDataPtr(gunNo))->meter->volt;
	if( Vdc < volt ) {
		 return CHECK_TRUE;
	}
	return CHECK_FALSE;
}

/*判断模块输出电压是否与实际采样相符*/
uint8 Check_OutVoltIsOk(uint8 gunNo,uint16 volt)
{
//	int16 Vdc1 = ((CHARGE_TYPE *)ChgData_GetRunDataPtr(gunNo))->iso->vdc1;   /*接触器内侧电压*/
	int16 Vdc1 = ((CHARGE_TYPE *)ChgData_GetRunDataPtr(gunNo))->meter->volt;
	/*电压在10V以内*/
//	DebugInfoWithPi(CHK_MOD, "Check_OutVoltIsOk abs(Vdc1)", abs(Vdc1));
//	DebugInfoWithPi(CHK_MOD, "Check_OutVoltIsOk volt", volt);
	
	if( abs(Vdc1 - volt ) < VOLT_TRAN(10) ) {
		 return CHECK_TRUE;
	}
	
	if (Vdc1 < -100)
	{
		 Check_SetErrCode(gunNo, ECODE105_PAVINSIDEREVERSE);   //枪内侧电压反接(模块端)	 
		 return CHECK_FALSE;
	}
	
	return CHECK_FALSE;
}

/*检测车辆电压是否与报文相差 <±5%*/
/*
**实际检测Vdc1 一路接触器上端 
**        Vdc3 一路接触器下端
**        Vdc2 二路接触器上端
**        Vdc4 二路接触器下端
*/   
uint8 Check_CarVolt(uint8 gunNo)
{
	uint8 rtn = CHECK_FALSE;
//	int16 Vdc3 = ((CHARGE_TYPE *)ChgData_GetRunDataPtr(gunNo))->iso->vdc3; /*接触器外侧电压*/
    int16 Vdc3 = AdcCalc_GetValue()->vdciso[1];
	BMSDATA_ST *pBms = Bms_GetBmsCarDataPtr(gunNo);
	CHGDATA_ST *pChg = Bms_GetChgDataPtr(gunNo);
	
#ifdef BMSTEST	 
	Vdc3 = pBms->bcp.batcurvolt; /*for 测试*/
#endif
	 
	int16 tmpVdc1 = Vdc3 * 1.05;
	int16 tmpVdc2 = Vdc3 * 0.95;
	
	/* 车辆端自己测的电压不应该是负值,最高符号位不应该是1 */
	if ((pBms->bcp.batcurvolt & 0x8000) == 0x8000)
	{
		Check_SetErrCode(gunNo, ECODE104_PAVOUTSIDEREVERSE);   //枪外侧电压反接(BMS端)			 
		return CHECK_FALSE;
	}
	
	if ((pChg->cml.maxoutvolt >=  pBms->bcp.batcurvolt) && (pChg->cml.minoutvolt <= pBms->bcp.batcurvolt)) 
	{
		if ((pBms->bcp.batcurvolt <= tmpVdc1) && (pBms->bcp.batcurvolt >= tmpVdc2)) 
		{
			rtn = CHECK_TRUE;
			return rtn;
		}
	}
		
	return rtn;
}

/*检测接触器内侧电压是否在接触器外侧电压 10V以内*/
uint8 Check_KmInAndOut(uint8 gunNo)
{
	uint16 Vdc1,Vdc3;
//	Vdc1 =  ((CHARGE_TYPE *)ChgData_GetRunDataPtr(gunNo))->iso->vdc3;
    Vdc1 = AdcCalc_GetValue()->vdciso[1];
	//uint16 Vdc3 = ((CHARGE_TYPE *)ChgData_GetRunDataPtr())->iso->vdc3;
	Vdc3 = ((CHARGE_TYPE *)ChgData_GetRunDataPtr(gunNo))->meter->volt;
#ifdef BMSTEST	
	Vdc1 = Vdc3; /*for test*/
#endif	

	DebugInfoByChk("检测接触器内侧电压是否在接触器外侧电压 10V以内");

	if( abs( Vdc1 - Vdc3 ) < VOLT_TRAN(10) ) {
		  return CHECK_TRUE;
	}
	return CHECK_FALSE;
}

uint8 Check_BcpToModSetVolt(uint8 gunNo)
{
//	 uint16 Vdc3 = ((CHARGE_TYPE *)ChgData_GetRunDataPtr(gunNo))->iso->vdc3;
   uint16 Vdc3 = AdcCalc_GetValue()->vdciso[1];
   BMSDATA_ST *pBms = Bms_GetBmsCarDataPtr(gunNo);
	
	  if( ( pBms->bcl.needvolt <= pBms->bcp.chgmaxvolt ) && ( Vdc3 < pBms->bcp.chgmaxvolt ) ) {
				return CHECK_TRUE;
		}
		return CHECK_FALSE;
	
}

/*  检测车辆参数是否合适 */
uint8 Check_CarParam(uint8 gunNo)
{
	BMSDATA_ST *pBms = Bms_GetBmsCarDataPtr(gunNo);
	CHGDATA_ST *pChg = Bms_GetChgDataPtr(gunNo);
	/* if(pBms->bcp.chgmaxvolt > pChg->cml.maxoutvolt \*/
	if( pBms->bcp.chgmaxvolt < pChg->cml.minoutvolt )
	{
		return CHECK_FALSE; /*参数不合适*/
	}
	return CHECK_TRUE;
}

/*故障位存储位置*/
static uint8 gErrBit[DEF_MAX_GUN_NO][MAX_ERR_BYTE] = {0};
/*故障位屏蔽字*/
static uint8 gMaskBit[DEF_MAX_GUN_NO][MAX_ERR_BYTE] ={ \
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
};

uint8 *GetgErrBitData(uint8 gunNo)
{
	return (uint8 *)&gErrBit[gunNo];
}

uint8 Check_SetErrCode(uint8 gunNo,E_ERR_CODE errCode)
{
	 uint16 nbyte,mod;

	 DebugInfoWithPi(CHK_MOD,"Check_SetErrCode errCode", errCode);
	 
	 nbyte = (errCode / 8) % MAX_ERR_BYTE;
	 mod = errCode % 8;
	 gErrBit[gunNo][nbyte] |= ( 1 << mod );
	
	 return 0;
}


uint8 Check_GetErrCode(uint8 gunNo,E_ERR_CODE errCode)
{
	 uint16 nbyte,mod;
	 nbyte = (errCode / 8) % MAX_ERR_BYTE;
	 mod = errCode % 8;
	
	 if( gErrBit[gunNo][nbyte] & (1 << mod ) ) {
		 return 1;
	 }
	 
	 return 0;
}



uint8 Check_ClearErrBit(uint8 gunNo,E_ERR_CODE errCode )
{
	 uint16 nbyte,mod;
	 nbyte = (errCode / 8 ) % MAX_ERR_BYTE;
	 mod = errCode % 8;
	 gErrBit[gunNo][nbyte] &= (~( 1 << mod ));
	 
	 return 0;
}

uint8 Check_ClearAllErrBit(uint8 gunNo)
{
	uint16 i;
	for( i = 0 ; i < MAX_ERR_BYTE  ; i++ ) {
		 gErrBit[gunNo][i] = 0;
	 }
	 return 0;
}


uint8 Check_GetErrBit(uint8 gunNo,uint8 *buf,uint8 bufsize,uint8 *num )
{
	  uint16 i,j,nbyte,mod;
	  uint8 tmp[MAX_ERR_BYTE];
	  i = 0;
	  for( i = 0 ; i < MAX_ERR_BYTE ; i++ ) {
			tmp[i] = gErrBit[gunNo][i] & gMaskBit[gunNo][i];
		}
		j = 0;
		buf[0] = 0;
		for( i = 0 ; i < MAX_ERR_BYTE * 8 ; i++ ) {
			nbyte = i / 8;
			mod = i % 8;
			if( tmp[nbyte] & ( 1 << mod ) ) {
				buf[j++] = i;
				break;
			}
		}
		
		*num = j;
	  return j;
}

void SoftReset(void)
{ 
	__set_FAULTMASK(1);      // 关闭所有中端
	NVIC_SystemReset();// 复位
}

#ifdef STM32F10X_HD
#define SOFTRSTSAMPNUMS  50
/* 软件复位按钮 */
void Check_SoftRSTKeyBoard(void)
{
	static uint8 sampcount = 0;
	static uint8 sum = 0;
	uint8 status = 0;
	
	status = ~ReadInputDataBit(INPUT_RSTKEYBOARD) & 0x01;
	
	sum += status;     //状态每次累加
	
	sampcount++;
	
	if (sampcount >= SOFTRSTSAMPNUMS)
	{
		sampcount = 0;
		
		if (sum >= SOFTRSTSAMPNUMS)  //状态每次都是1，和为采样次数，按键稳定
		{
			SoftReset();
		}
		
		sum = 0;
	}
}
#endif



void Check_DialcodeBoard(void)
{
	DEV_INPUT_TYPE *pInputDataptr = Check_GetInputDataptr(AGUN_NO);
	
	pInputDataptr->statu.bits.key1 = ReadInputDataBit(INPUT_KEY1);
	pInputDataptr->statu.bits.key2 = ReadInputDataBit(INPUT_KEY2);
	pInputDataptr->statu.bits.key3 = ReadInputDataBit(INPUT_KEY3);
	pInputDataptr->statu.bits.key4 = ReadInputDataBit(INPUT_KEY4);
}
	
/* Private macro ------------------------------------------------------------*/

#ifdef STM32F10X_HD
#define CC1SAMPLENUMBERS    50    //采样50次 
/*
**CC1采样值存放数组
*/
static uint16 CC1NADCValue[CC1SAMPLENUMBERS] = { 0 };  //采样存放数组

/*
**初始化为没有插枪连接的时候AD口电压2.118V，对应的ADC值，数组中数据默认为2.118V状态
**AD基准2.5V
**
*/
void InitCC1ADCValue(uint8 Gunno)
{
	uint16 count = 0;
	
	while (count < CC1SAMPLENUMBERS)
	{
		CC1NADCValue[count] = 3470;  /* (2.118V / 2.5V) * 4096 = 3470.1312; */
		
		count++;
	}	
}

/*
**求CC1的ADC数组中数据的平均值
**
**
*/
static uint32 CC1_AVGVoltage(void)
{
	uint16 count = 0;
	uint32 sum = 0;      //采样数据累计和
	uint32 avg = 0;      //平均值
	
	while (count < CC1SAMPLENUMBERS)
	{
		sum += CC1NADCValue[count];
		
		count++;
	}
	
	avg = sum / CC1SAMPLENUMBERS;
	
	return avg;
}

/*
**获取CC1的电压值
**
**
*/
static uint32 Get_CC1_Voltage(void)
{
	static uint16 scount = 0;   //采样计数器
	
	uint16 ADCValue = 0;        //ADC的输出值
	uint32 avg = 0;
	
	ADCValue = ADC_GetConversionValue(ADC2);
	CC1NADCValue[scount] = ADCValue;
	
	scount++;
	
	if (scount >= CC1SAMPLENUMBERS)
	{
		scount = 0;
	}

	avg = CC1_AVGVoltage();
	
	return avg;
}

/*
**单屏单枪的103板CC1状态说明，采用AD采样方式
没有插枪连接的时候AD口电压2.118V，对应AD值  3446
接1K电阻的时候AD口电压1.073V，对应AD值     1758  
接500Ω电阻的时候AD口电压0.723V，对应AD值  1190
*/

/*
**正负200mv对应的AD值做判断的条件
**150mv对应的AD值 245
**
*/
static uint8 Gpio_ReadInputIo_CC1_SEQ(uint8 Gunno)
{
	uint32 u32data = 0;
	u32data = Get_CC1_Voltage();
	if ((u32data > (3446 - 245))) //&& (u32data < (3446 + 245)))  //1761
	{
		return CC1_12V; /*按下机械按钮*/
	}

	else if ((u32data > (1758 - 245)) && (u32data < (1758 + 245)))
	{
		return CC1_6V; /*未插枪*/
	}
	
	else if ((u32data > (1190 - 245)) && (u32data < (1190 + 245)))
	{
		return CC1_4V; /*已连接*/
	}
	
	return CC1_NONEV;
}
#endif

/*******************************************************STM32F4XX*********************************************************/
#ifdef STM32F4XX
/* Private macro ------------------------------------------------------------*/
#define AGUN_CC1_GPIO_PORT        GPIOE
#define AGUN_CC1_GPIO_PIN         GPIO_Pin_10

#define BGUN_CC1_GPIO_PORT        GPIOE
#define BGUN_CC1_GPIO_PIN         GPIO_Pin_11

#define CC1SAMPLENUMBERS          8 //patli 20200326 20    //2é?ù20′? 


/* 0急停状态 */
/* 1交流输入断路器状态 */
/* 2交流接触器 */
/* 3防雷器 */
/* 4 A枪直流接触器 */
/* 5 B枪直流接触器 */
/* 6 功率分配接触器 */
/* 7 A枪熔丝状态 */
/* 8 B枪熔丝状态 */
/* 9 A枪电子锁反馈 */
/* 10 B枪电子锁反馈 */
/* 11 门禁反馈 */
/* 12 拨码开关1 */
/* 13 拨码开关2 */
/* 14 拨码开关3 */
/* 15 拨码开关4 */
/* 16 A枪控制导引CC1 */
/* 17 B枪控制导引CC1 */
//	INPUT0 = 0, 
//	INPUT1,
//	INPUT2,
//	INPUT3,
//	INPUT4,
//	INPUT5,
//	INPUT6,
//	INPUT7,
//	INPUT8,
//	INPUT9,
//	INPUT10,
//	INPUT11,
//	INPUT_KEY1,
//	INPUT_KEY2,
//	INPUT_KEY3,
//	INPUT_KEY4,
//	INPUT_ACC1,
//	INPUT_BCC1,
#define IO_SAMPLE_NUM   3
uint8 Io_data[IO_SAMPLE_NUM][10];
uint8 Io_data_num = 0;

/**
* @brief  GetInputChannelRes
* @param  None
* @retval None
*/
extern void SetCD4051BMTChannel(uint8 ch);

uint8 GetInputChannelRes(uint8 ch)
{
	float channelRes = 0;
	
	SetCD4051BMTChannel(ch);
	
	channelRes = ReadInputDataBit(INPUT0);
	
  return channelRes;	
}

void resetInput()
{
	memset(Io_data, 0, sizeof(Io_data));
}


void Read_AllInputIo(void)
{	
	Io_data_num %= IO_SAMPLE_NUM;
	
	Io_data[Io_data_num][0] = GetInputChannelRes(INPUT0);			/* 0急停状态 */

Io_data[Io_data_num][1] = GetInputChannelRes(INPUT1);			/* 1直流接触器状态 */
Io_data[Io_data_num][2] = GetInputChannelRes(INPUT2);			/* 2枪电子锁状态 */
	Io_data_num++;



#if 0	
	Io_data[Io_data_num][1] = GetInputChannelRes(INPUT1);		/* 1交流输入断路器状态 */
	Io_data[Io_data_num][2] = GetInputChannelRes(INPUT2);		/* 2交流接触器 */
	Io_data[Io_data_num][3] = GetInputChannelRes(INPUT3);		/* 3防雷器 */
	Io_data[Io_data_num][4] = GetInputChannelRes(INPUT4);		/* 4 A枪直流接触器 */
	Io_data[Io_data_num][5] = GetInputChannelRes(INPUT5);		/* 5 B枪直流接触器 */
	Io_data[Io_data_num][6] = GetInputChannelRes(INPUT6);		/* 6 功率分配接触器 */
	Io_data[Io_data_num][7] = GetInputChannelRes(INPUT7);		/* 7 A枪熔丝状态 */
	Io_data[Io_data_num++][8] = GetInputChannelRes(INPUT8);		/* 8 B枪熔丝状态 */
#endif


}

uint8 Check_AllInputIo(void)
{
	uint8 i;

	for( i = 0 ; i < 16 ; i++ ) {
		if( Io_data[0][i] == Io_data[1][i] &&Io_data[0][i] == Io_data[2][i]) {
			if(Io_data[0][i] == 1 ) {
			  gInputStu.word |= (1 << i );
			}else {
				 gInputStu.word &= ~(1 << i );
			}
		}
	}
	return 0;
}

/* 0急停状态 */
/* 1交流输入断路器状态 */
/* 2交流接触器 */
/* 3防雷器 */
/* 4 A枪直流接触器 */
/* 5 B枪直流接触器 */
/* 6 功率分配接触器 */
/* 7 A枪熔丝状态 */
/* 8 B枪熔丝状态 */
/* 9 A枪电子锁反馈 */
/* 10 B枪电子锁反馈 */
/* 11 门禁反馈 */
void Check_TranIOStatus(void)
{
	PARAM_OPER_TYPE *param = ChgData_GetRunParamPtr();
	
	DEV_LOGIC_TYPE *ptrLogic =  ChgData_GetLogicDataPtr(AGUN_NO);
	DEV_INPUT_TYPE *pInputDataptr = Check_GetInputDataptr(AGUN_NO);
	DEV_GUN_TYPE *pGunDataPtr = Check_GetGunDataPtr(AGUN_NO);
	DEV_RELAY_TYPE *pRelayDataPtr = Relay_GetRelayDataPtr(AGUN_NO);
	
	pInputDataptr->statu.bits.dckmp   = gInputStu.bits.bit1;//gInputStu.bits.bit4;   			/*正直流接触器状态*/
	pInputDataptr->statu.bits.dckmn   = gInputStu.bits.bit1;//gInputStu.bits.bit4;   			/*负直流接触器状态*/
	pInputDataptr->statu.bits.stop    = ~gInputStu.bits.bit0&0x01 ;  			/*急停*/
	pInputDataptr->statu.bits.ackm    = gInputStu.bits.bit2;  		 	/*交流接触器*/
	pInputDataptr->statu.bits.spd     = gInputStu.bits.bit3;   			/*防雷器*/
	pGunDataPtr->statu.bits.elockstu  = gInputStu.bits.bit2&0x01; //~gInputStu.bits.bit9&0x01;  /*电子锁反馈 */
	
	pInputDataptr->statu.bits.acqf    = gInputStu.bits.bit1;   			/*交流断路器*/
	pInputDataptr->statu.bits.posfuse = gInputStu.bits.bit7;   			/*正熔丝*/
	pInputDataptr->statu.bits.negfuse = gInputStu.bits.bit7;   			/*负熔丝*/
	
	if(0 == param->Sysparalarm.bits.curfewsignal){               		//门禁信号
	  pInputDataptr->statu.bits.dooracs = gInputStu.bits.bit11;   	/*门禁1 bit7 */	
	  pInputDataptr->statu.bits.dooraf  = gInputStu.bits.bit11;   	/*门禁2 bit7 后门*/
	}
	else{
		pInputDataptr->statu.bits.dooracs = ~gInputStu.bits.bit11&0x01; /*门禁1*/	
	  pInputDataptr->statu.bits.dooraf  = ~gInputStu.bits.bit11&0x01; /*门禁2*/
	}
	
	pInputDataptr->statu.bits.fan     = 0;   													/*风机*/
	pInputDataptr->statu.bits.smoke   = 0;   													/*烟雾告警*/
	pInputDataptr->statu.bits.water   = 0;   													/*水浸告警*/
	pRelayDataPtr->statu.bits.km1 = pInputDataptr->statu.bits.dckmp;
	pRelayDataPtr->statu.bits.km2 = pInputDataptr->statu.bits.dckmn;
	
	pGunDataPtr->statu.bits.cc1stu = Gpio_ReadInputIo_CC1_SEQ(AGUN_NO);
	
	ptrLogic->runstu.bits.posFU = pInputDataptr->statu.bits.posfuse;
	ptrLogic->runstu.bits.negFU = pInputDataptr->statu.bits.negfuse;
	ptrLogic->runstu.bits.emcystop = pInputDataptr->statu.bits.stop;
	ptrLogic->runstu.bits.acKm = pInputDataptr->statu.bits.ackm;
	ptrLogic->runstu.bits.spd  = pInputDataptr->statu.bits.spd;
	ptrLogic->runstu.bits.elockstu = pGunDataPtr->statu.bits.elockstu;
	ptrLogic->runstu.bits.gunCC1 = pGunDataPtr->statu.bits.cc1stu;

//	DebugInfoWithPx(CHK_MOD,"A枪cc1stu",pGunDataPtr->statu.bits.cc1stu);
	if(pGunDataPtr->statu.bits.cc1stu == 1)		
		setbit(gun_status, 0);	//a gun cc1 status set
		
	if(pGunDataPtr->statu.bits.elockstu == 1) 	
		setbit(gun_status, 1);	//a gun lock status set

//	DebugInfoWithPx(CHK_MOD,"A枪状态量",pGunDataPtr->statu.word);
	
	
	/*******************B枪的状态量**************************************************************/
	pGunDataPtr = Check_GetGunDataPtr(BGUN_NO);
	pRelayDataPtr = Relay_GetRelayDataPtr(BGUN_NO);
	ptrLogic =  ChgData_GetLogicDataPtr(BGUN_NO);
  pInputDataptr = Check_GetInputDataptr(BGUN_NO);
	
	pInputDataptr->statu.bits.dckmp   = gInputStu.bits.bit5;   			/*正直流接触器状态*/
	pInputDataptr->statu.bits.dckmn   = gInputStu.bits.bit5;   			/*负直流接触器状态*/
	pInputDataptr->statu.bits.stop    = ~gInputStu.bits.bit0&0x01 ; 		 	/*急停*/
	pInputDataptr->statu.bits.ackm    = gInputStu.bits.bit2;   			/*交流接触器*/
	pInputDataptr->statu.bits.spd     = gInputStu.bits.bit3;   			/*防雷器*/
	pGunDataPtr->statu.bits.elockstu  = ~gInputStu.bits.bit10&0x01; /*电子锁反馈 */
	
	pInputDataptr->statu.bits.acqf    = gInputStu.bits.bit1;   			/*交流断路器*/
	pInputDataptr->statu.bits.posfuse = gInputStu.bits.bit8;   			/*正熔丝*/
	pInputDataptr->statu.bits.negfuse = gInputStu.bits.bit8;   			/*负熔丝*/
	
	if(0 == param->Sysparalarm.bits.curfewsignal){               		//门禁信号
	  pInputDataptr->statu.bits.dooracs = gInputStu.bits.bit11;   	/*门禁1 bit7 */	
	  pInputDataptr->statu.bits.dooraf  = gInputStu.bits.bit11;   	/*门禁2 bit7 后门*/
	}
	else{
		pInputDataptr->statu.bits.dooracs = ~gInputStu.bits.bit11&0x01;/*门禁1*/	
	  pInputDataptr->statu.bits.dooraf  = ~gInputStu.bits.bit11&0x01;/*门禁2*/
	}
	
	pInputDataptr->statu.bits.fan     = 0;   												/*风机*/
	pInputDataptr->statu.bits.smoke   = 0;   												/*烟雾告警*/
	pInputDataptr->statu.bits.water   = 0;   												/*水浸告警*/
	pRelayDataPtr->statu.bits.km1 = pInputDataptr->statu.bits.dckmp;
	pRelayDataPtr->statu.bits.km2 = pInputDataptr->statu.bits.dckmn;
	
	pGunDataPtr->statu.bits.cc1stu = Gpio_ReadInputIo_CC1_SEQ(BGUN_NO);
	
	
	ptrLogic->runstu.bits.posFU = pInputDataptr->statu.bits.posfuse;
	ptrLogic->runstu.bits.negFU = pInputDataptr->statu.bits.negfuse;
	ptrLogic->runstu.bits.emcystop = pInputDataptr->statu.bits.stop;
	ptrLogic->runstu.bits.acKm = pInputDataptr->statu.bits.ackm;
	ptrLogic->runstu.bits.spd  = pInputDataptr->statu.bits.spd;
	ptrLogic->runstu.bits.elockstu = pGunDataPtr->statu.bits.elockstu;
	ptrLogic->runstu.bits.gunCC1 = pGunDataPtr->statu.bits.cc1stu;
	
//	DebugInfoWithPx(CHK_MOD,"B枪cc1stu",pGunDataPtr->statu.bits.cc1stu);
//	DebugInfoWithPx(CHK_MOD,"B枪状态量",pGunDataPtr->statu.word);
	if(pGunDataPtr->statu.bits.cc1stu == 1)		
		setbit(gun_status, 4);	//b gun cc1 status set
		
	if(pGunDataPtr->statu.bits.elockstu == 1) 	
		setbit(gun_status, 5);	//b gun lock status set

//	DebugInfoWithPx(CHK_MOD,"接触器状态",pRelayDataPtr->statu.word);
	
//	DebugInfoWithPx(CHK_MOD,"输入状态量",pInputDataptr->statu.word);

	
}


/*
**CC1采样值存放数组
**
**
*/
static uint16 CC1NIOValue[DEF_MAX_GUN_NO][CC1SAMPLENUMBERS] = { 0 };  //采样存放数组

/*
**初始化为没有插枪连接的时候IO口为高电平,取反
**AD基准2.5V
**
*/
void InitCC1ADCValue(uint8 Gunno)
{
	uint16 count = 0;
	
	while (count < CC1SAMPLENUMBERS)
	{
		CC1NIOValue[Gunno][count] = 0;
		
		count++;
	}	
}

/*
**求CC1的IO数组中数据的平均值
**
**
*/
static uint32 CC1_AVGVoltage(uint8 Gunno)
{
	uint16 count = 0;
	uint32 sum = 0;      //采样数据累计和
	uint32 avg = 0;      //平均值
	
	while (count < CC1SAMPLENUMBERS)
	{
		sum += CC1NIOValue[Gunno][count];
		
		count++;
	}
	
	avg = sum / CC1SAMPLENUMBERS;
	
	return avg;
}

/*
**获取CC1的IO口电平值
**
**
*/
//extern u16 AdcValue[5];

static uint32 Get_CC1_Voltage(uint8 Gunno)
{
	static uint8 scount[DEF_MAX_GUN_NO] = {0};   //采样计数器
	
//	uint8  IOValue = 0;        //IO口电平值
	uint16  avg = 0;

#if 0	
	if (0 == Gunno)
	{
		IOValue = ~GPIO_ReadInputDataBit(AGUN_CC1_GPIO_PORT, AGUN_CC1_GPIO_PIN) & 0x01;
	}
	else if (1 == Gunno)
	{
		IOValue = ~GPIO_ReadInputDataBit(BGUN_CC1_GPIO_PORT, BGUN_CC1_GPIO_PIN) & 0x01;
	}
#endif

//	printf("Get_CC1_Voltage=%d ",AdcValue[1]);
	CC1NIOValue[Gunno][scount[Gunno]] = AdcCalc_GetValue()->cc1;//AdcValue[1];
	
	scount[Gunno]++;
	
	if (scount[Gunno] >= CC1SAMPLENUMBERS)
	{
		scount[Gunno] = 0;
	}

	avg = CC1_AVGVoltage(Gunno);
	
	return avg;
}


/*
**正负170mv对应的AD值做判断的条件
**170mv对应的AD值 211
**
*/

static uint8 Gpio_ReadInputIo_CC1_SEQ(uint8 Gunno)
{
#if 0 
	uint8 CC1_data = 0;
	CC1_data = Get_CC1_Voltage(Gunno);

	return 1 == CC1_data ? CC1_4V : CC1_NONEV;
#else
	uint32 u32data = 0;
	u32data = Get_CC1_Voltage(Gunno);

//	printf("CC1=%d ",u32data);
	
	if (u32data > (2613 - 211)) 
	{
		return CC1_12V; /*按下机械按钮*/
	}

	else if ( (u32data > (1307 - 211) ) && (u32data < (1307 + 211) ) )
	{
		return CC1_6V; /*未插枪*/
	}

	else if ( (u32data > (871 - 211) ) && (u32data < (871 + 211) ) )
	{
		return CC1_4V; /*已连接*/
	}

	return CC1_NONEV;

#endif
}

#endif
/*******************************************************STM32F4XX** end *******************************************************/
static uint32 sContimes[DEF_MAX_GUN_NO][3] = {0}; 
uint8 Check_BcsAndMetdata(uint8 gunNo)
{
	 uint8 rtn ;
	 uint8 buf[100];
	 
	 BMSDATA_ST *pBms = Bms_GetBmsCarDataPtr(gunNo);
	 DEV_METER_TYPE *pDcMet = TskDc_GetMeterDataPtr(gunNo);
	 CALCBILL_DATA_ST *ptrBill = Bill_GetBillData(gunNo);
   rtn = 0;

   	memset(buf, 0, sizeof(buf));
   	sprintf(buf, "Check_BcsAndMetdata DcMet->curr:%d;Bms->needcurr:%d",pDcMet->current,pBms->bcl.needcurr);
   
   	DebugInfo(CHK_MOD, buf);
	
	 if( (pDcMet->current > pBms->bcl.needcurr+50) && ( pBms->bcl.needcurr > 50 )&& (ptrBill->chgsecs > 90 ) ) {
		 if(GetSystemTick() - sContimes[gunNo][0] > TIM_NS(50) ) {
			  Check_SetErrCode(gunNo,ECODE91_CHGCURRERR);
			  rtn = 1;
		 }
	 }else {
		 sContimes[gunNo][0] = GetSystemTick(); 
	 }
	 
	 if( (pDcMet->volt > pBms->bcl.needvolt+100)&& (pBms->bcl.needvolt > 600 ) && (ptrBill->chgsecs > 90) ) {
		 if(GetSystemTick() - sContimes[gunNo][1] > TIM_1MIN ) {
			  Check_SetErrCode(gunNo,ECODE92_CHGVOLTERR);
			   rtn = 1;
		 }
	 }else {
		 sContimes[gunNo][1] = GetSystemTick();
	 }
	 
	 if ( ptrBill->chgsecs >  5 * 60 ) {
		 if( pDcMet->current < 10 ) {
				if( GetSystemTick() - sContimes[gunNo][2] > TIM_NMIN(30) ) {
					Check_SetErrCode(gunNo,ECODE74_BMSST10MIN);
					rtn = 1;
				}
		 }else {
				sContimes[gunNo][2] = GetSystemTick();
		 }
	}else {
		sContimes[gunNo][2] = GetSystemTick();
	}
	 
	
	 return rtn;
}

/**
* @brief  Check_GunTemper
* @param  None
* @retval None
*/
void Check_GunTemper(void)
{
	DEV_GUN_TYPE *pGunADataPtr = Check_GetGunDataPtr(0);
//	DEV_GUN_TYPE *pGunBDataPtr = Check_GetGunDataPtr(1);
	PARAM_OPER_TYPE *ptrRunPara = ChgData_GetRunParamPtr();
	
	//温度值做2阶滤波
	pGunADataPtr->i32temper[0] = (pGunADataPtr->i32temper[0] + GetCD4051BMTChannelTemper(0)) / 2;  //A枪温度传感器1
	pGunADataPtr->i32temper[1] = (pGunADataPtr->i32temper[1] + GetCD4051BMTChannelTemper(1)) / 2;  //A枪温度传感器2

	
	
//	pGunBDataPtr->i32temper[0] = (pGunBDataPtr->i32temper[0] + GetCD4051BMTChannelTemper(2)) / 2;  //B枪温度传感器1
//	pGunBDataPtr->i32temper[1] = (pGunBDataPtr->i32temper[1] + GetCD4051BMTChannelTemper(3)) / 2;  //B枪温度传感器2

	//充电枪过温值
	if ((pGunADataPtr->i32temper[0] > ptrRunPara->gunmaxtemper) || (pGunADataPtr->i32temper[1] > ptrRunPara->gunmaxtemper) ) 
	{
		pGunADataPtr->statu.bits.overtemper = 1;                  //A枪过温告警
	}
	else
	{
		pGunADataPtr->statu.bits.overtemper = 0;
	}
	
//	if ((pGunBDataPtr->i32temper[0] > ptrRunPara->gunmaxtemper) || (pGunBDataPtr->i32temper[1] > ptrRunPara->gunmaxtemper))
//	{
//		pGunBDataPtr->statu.bits.overtemper = 1;                 //B枪过温告警
//	}
//	else
//	{
//		pGunBDataPtr->statu.bits.overtemper = 0;
//	}
}

/**
* @brief  Check_BcpDCOverVolt
* @param  None
* @retval None
*/
void Check_BcpDCOverVolt(void)
{

	BMSDATA_ST *ptr0 = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(0);
	DEV_METER_TYPE *pMet0 = TskDc_GetMeterDataPtr(0);
	
	BMSDATA_ST *ptr1 = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(1);
	DEV_METER_TYPE *pMet1 = TskDc_GetMeterDataPtr(1);
	
	DEV_GUN_TYPE *pGunADataPtr = Check_GetGunDataPtr(0);
	DEV_GUN_TYPE *pGunBDataPtr = Check_GetGunDataPtr(1);
	
	static uint16 errCnt[2] = { 0 };
	
	uint16 bcp0chgmaxvolt = ptr0->bcp.chgmaxvolt * 1.05;
	uint16 bcp1chgmaxvolt = ptr1->bcp.chgmaxvolt * 1.05;
	
	if (GET_WORKSTEP(0) == STEP_CHARGEING)
	{
		if (pMet0->volt > bcp0chgmaxvolt)
		{
			errCnt[0]++;
			
			if (errCnt[0] > 500)
			{				
				pGunADataPtr->statu.bits.bcpovervolt = 1;
			}
		}
		else
		{
			errCnt[0] = 0;
			pGunADataPtr->statu.bits.bcpovervolt = 0;
		}
	}
	
  if (GET_WORKSTEP(1) == STEP_CHARGEING)
	{
		if (pMet1->volt > bcp1chgmaxvolt)
		{
			errCnt[1]++;
			
			if (errCnt[1] > 500)
			{
				pGunBDataPtr->statu.bits.bcpovervolt = 1;
			}
		}
		else
		{
			errCnt[1] = 0;
			pGunBDataPtr->statu.bits.bcpovervolt = 0;
		}
	}

}

/*
**输入状态检测任务
**检测状态包括：cc1 、电子锁 、km、k3k4 
**
*/
#ifndef DC_AC_ISO_REALCHECK
void Task_RealCheck(void *p_arg)
{
  gPtrRunData = ChgData_GetRunDataPtr(AGUN_NO);
	InitCC1ADCValue(0);
	
	#ifdef STM32F4XX
		InitCC1ADCValue(1);
	#endif
	
	while(1) {
		 #ifdef STM32F10X_HD
			Check_N1_2_3_InputIo();
		  Check_SoftRSTKeyBoard();  //软复位按钮按下
		 #endif
			Check_AllInputIo();
		 
			Check_TranIOStatus();
		
			Check_stopBtn();         //急停按下
		
			Check_DialcodeBoard();   //拨码开关检测
		
			Check_GunTemper();       //检查枪的温度
		
			Check_BcpDCOverVolt();   //检查充电过程中充电电压不能大于BCP最高电压
		Delay10Ms(5);   //patli 20200106		  Delay10Ms(1);
	 }
}


#endif




