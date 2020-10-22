/**
  ******************************************************************************
  * @file    TaskDcMeter.h
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
*/
#ifndef _TASKDCMETER_H_0001
#define _TASKDCMETER_H_0001
#include "typedefs.h"

typedef struct {
	uint16 volt;          /*   0.1V    */
	uint16 current;       /*   0.1A    */
	uint32 energy;        /*   0.01Kwh */ 
	union {
		uint16 word;
			uint8  byte[2];
			struct {
				uint16 commerr : 1;
		    uint16 vlOver  : 1;
				uint16 vlUnder : 1;
				uint16 currOver: 1;
				uint16 rsv     : 12;
	    }bits;
	}statu;
	
	uint16 rem;        /*   0.01Kwh */ 

	uint8 commOvertimecnt;   //通讯超时次数
	
}DEV_METER_TYPE;


typedef struct {
	uint8 adr;
	uint8 comNo;
	uint16 baudrate;
	
	uint16 voltOver;
	uint16 voltUnder;
	uint16 currOver;
	
}DEV_METER_PARAM;

typedef struct
{
	uint8   seq;
	uint16  startReg;
	uint16  regNum;
}METER_REG_DEF;


extern DEV_METER_TYPE *TskDc_GetMeterDataPtr(uint8 gunNo);


extern void TaskDcMeter(void *p_arg);



























#endif
