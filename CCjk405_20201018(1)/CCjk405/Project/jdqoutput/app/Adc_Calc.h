/****************************************************************************************
Filename: 
Author: 
Datetime: 
Version: 
****************************************************************************************/

#ifndef __ADCCALC_H
#define __ADCCALC_H
#include "typedefs.h"
//#include "M_Global.h"

#define  JY_START 0x0A
#define  JY_STOP  0x00

#define RTN_JYCONTINUE  1
#define RTN_JYEND       2
#define RTN_ERR         3


#define RELAY1  GPIO_Pin_10
#define RELAY2  GPIO_Pin_11




typedef union
{
  uint16 JyCtrl;
  struct
  {
     uint16 b1Allow:4;   /*一路0x0A启动：0x00停止 */
     uint16 b2Allow:4;   /*二路0x0A启动：0x00停止 */
     uint16 bchkTm:8;    /*bit8-15：检测时间S*/
  }sbit;
}ST_JYCTRL;


typedef struct
{
//	uint16 uab;    /*Uab*/
//	uint16 ubc;    /*Ubc*/
//	uint16 uca;    /*Uca*/
//	int32 viso[2];  /*绝缘一路采样负对地电压*/
//	int32 vdc[4];   /*母线一路电压 1,2路采样绝缘，3,4路只采集直流电压*/
    uint16 visopos;  /*A母线正对地电压*/
    uint16 visoneg;  /*A母线负对地电压*/
    uint16 visodiff; /*A母线压差*/
    uint16 Respos;   /*A母线正对地电阻*/  
    uint16 Resneg;   /*A母线负对地电阻*/
//	uint16 vdciso[2];   /*vdciso[0]为板采集到的接触器内侧电压，vdciso[1]为外侧电压*/
		int32 vdciso[2];   /*vdciso[0]为板采集到的接触器内侧电压，vdciso[1]为外侧电压*/
//    uint16 xfStatu;      /*电压泄放控制状态 bit0：一路  bit1：二路*/
    uint32 ResSeting;    /*绝缘电阻设定值*/
	
	  uint16 temprature;  /*温度*/
    uint16 cc1;  /*cc1*/
	  
    uint16 guntemprature[2];  /*枪温度2个检测点值*/
}ST_SAMPDATA;

ST_SAMPDATA *AdcCalc_GetValue(void);
void sys_ADC1_Init(void);
#endif                                                  
