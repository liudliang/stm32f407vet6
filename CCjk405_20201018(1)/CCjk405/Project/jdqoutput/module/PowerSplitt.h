/**
  ******************************************************************************
  * @file    PowerSplitt.h
  * @author  
  * @version v1.0
  * @date    2018-08-03
  * @brief   
  ******************************************************************************
*/
#ifndef _POWERSPLITT_H_0001
#define _POWERSPLITT_H_0001
#include "typedefs.h"
#include "CdModel.h"


#define PW_CTRL_ON    1
#define PW_CTRL_OFF   0


typedef enum
{
	ESET_CON_GROUP   = 1,
	ESET_CON_VOLCURR = 2,
	ESET_CON_ONOFF   = 3,
	ESET_CON_END     = 4,
	ESET_INDI_VOLCURR = 50,
	
	ESET_INDI_EXIT    = 80,  /*主动退出*/
	ESET_EXIT_RELAY   = 81,
	
	ESET_PREEM_EXIT   = 91,   /*抢占式退出*/
	ESET_PREEM_RELAY  = 92,  
	
	ESET_OTHER_DEFAULT= 0xFF,
}E_SPLITT_STEP;


extern uint8 PowerSplitt_ParamInit(uint8 grpNo);
extern uint8 PowerSplitt_CalcSplitModNum(uint16 needCurr,MOD_CTRL_PARA *ptrModPara);
extern uint8 PwoerSplitt_CalcStepCurrent(MOD_CTRL_PARA *ptrModPara);
extern uint8 PowerSplitt_CalcPower(uint8 grpNo,MOD_CTRL_PARA *ptrModPara);
extern uint8 PowerSplitt_ModCtrlFunc(uint8 grpNo,MOD_CTRL_PARA *ptrModPara);

extern uint8 PowerSplitt_PreemExitGroup(uint8 grpNo,MOD_CTRL_PARA *ptrModPara);























#endif






