/**
  ******************************************************************************
  * @file    TaskAcMeter.h
  * @author  
  * @version v1.0
  * @date    2018-08-03
  * @brief   
  ******************************************************************************
*/
#ifndef _TASKACMETER_H_0001
#define _TASKACMETER_H_0001
#include "typedefs.h"

typedef struct {
	uint16 Uab;           	//Uab
	uint16 Ubc;           	//Ubc
	uint16 Uca;           	//Uca
	uint16 Iab;           	//
	uint16 Ibc;           	//
	uint16 Ica;           	//
	union {
		uint16 word;
			uint8  byte[2];
			struct {
				uint16 commerr     : 1;
				uint16 UabOver    : 1;
				uint16 UbcOver    : 1;
				uint16 UcaOver    : 1;
				uint16 currabOver  : 1;
				uint16 currbcOver  : 1;
				uint16 currcaOver  : 1;
				uint16 UabUnder   : 1;
				uint16 UbcUnder   : 1;
				uint16 UcaUnder   : 1;
				uint16 UDeficy    : 1; /*三相不平衡*/
				uint16 rsv         : 5;
	    }bits;
	}statu;
	uint8 commuOvertimecnt;   //通讯超时次数
}DEV_ACMETER_TYPE;


typedef struct {
	uint8 adr;
	uint8 comNo;
	uint16 baudrate;
	
	uint16 voltOver;
	uint16 voltUnder;
	uint16 currOver;
	
}DEV_ACMETER_PARAM;

typedef struct
{
	uint8   seq;
	uint16  startReg;
	uint16  regNum;
}ACMETER_REG_DEF;


extern DEV_ACMETER_TYPE *TskAc_GetMeterDataPtr(uint8 gunNo);


extern void TaskAcMeter(void *p_arg);

extern void CheckACMeterDataErr(void);


extern uint8 ReadAcMeterData(void);

























#endif
