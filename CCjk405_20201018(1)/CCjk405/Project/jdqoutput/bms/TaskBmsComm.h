/**
  ******************************************************************************
  * @file    TaskSlaveComm.h
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#ifndef __TASKSLAVECOMM_H_0001
#define __TASKSLAVECOMM_H_0001
#include "typedefs.h"



typedef struct {
	uint8 protoNo;
	void (*init)(uint8 GunNo);
	uint8 (*rcvanddeal)(uint8 GunNo,void *rtninfo,uint8 *pbuf);
	uint8 (*criclereport)(uint8 gunNo);
	uint8 (*singreport)(uint8 gunNo,uint8 pf,uint8 spn);
	uint8 (*ctrlreport)(uint8 gunNo,uint8 pf,uint8 allow,uint8 fg);
	
}PROTO_ST;



extern PROTO_ST *TskBms_GetProtoPtr(uint8 gunNo);

















#endif





