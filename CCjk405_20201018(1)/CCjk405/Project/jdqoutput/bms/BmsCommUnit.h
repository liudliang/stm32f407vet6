/**
  ******************************************************************************
  * @file    BmsCommUnit.h
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#ifndef _BMSCOMMUNIT_H_001
#define _BMSCOMMUNIT_H_001
#include "typedefs.h"
#include "TaskBmsComm.h"


#define COMMERR_TIMES  (10*200)  /*6S*/


extern PROTO_ST *BmsCommUnit_GetProtoPtr(uint8 gunNo);


extern void BmsCommUnit_ProtoInit(uint8 gunNo,uint8 protoNo);




















#endif


