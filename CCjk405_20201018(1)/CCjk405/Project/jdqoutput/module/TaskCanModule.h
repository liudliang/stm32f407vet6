/**
  ******************************************************************************
  * @file    TaskModule.h
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#ifndef _TASKMODULE_H_0001
#define _TASKMODULE_H_0001
#include "typedefs.h"
#include "CdModel.h"







extern int TskMod_InsertModCmdInList(uint8 adr,uint8 grpNo,CD_VALUETYPE cmd,uint32 value);

extern uint32  CdModel_CheckCanId(uint32 extId);

extern uint32 CdModel_SendData(uint8 objAdr,uint8 cmd,uint16 valtype,uint32 *u32data);

extern uint32 CdModel_ReadData(uint8 frmNum);

extern uint16 CdModel_ParseFrame(uint8 srcAdr,uint8 *data);

extern uint32 CdModel_SetData(uint8 objAdr,uint8 groupAddress,uint8 ptp,uint16 cmd,uint16 valtype,DATA4_TYPE *data);

extern uint32 CdModel_AddrSet(uint16 cmd,uint16 valtype,DATA4_TYPE *data);












#endif


