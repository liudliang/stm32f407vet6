/**
  ******************************************************************************
  * @file    BmsCommUnit.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
	
#include "BmsCommUnit.h"

#include "ChgData.h"
#include "GbtBmsProto.h"	
#include "PutianBms.h"

	
PROTO_ST gProto[DEF_MAX_GUN_NO] = { 0 };

PROTO_ST *BmsCommUnit_GetProtoPtr(uint8 gunNo)
{
	if( (gProto[gunNo].init != NULL) &&  ((PROTO_GBT == gProto[gunNo].protoNo) \
		|| (PROTO_PT ==  gProto[gunNo].protoNo) ) ) {
		return &gProto[gunNo];
	}
	return NULL;
}


void BmsCommUnit_ProtoInit(uint8 gunNo,uint8 protoNo)
{
	switch(protoNo){
		case PROTO_GBT:
			gProto[gunNo].protoNo = PROTO_GBT;
			gProto[gunNo].init = Gbt_Init;
			gProto[gunNo].rcvanddeal = Gbt_RcvDealFrame;
			gProto[gunNo].criclereport = Gbt_CricleReport;
			gProto[gunNo].ctrlreport = Gbt_CtrlCricle;
			gProto[gunNo].singreport = Gbt_ReportSingPkg;
			break;
	  case PROTO_PT:
			gProto[gunNo].protoNo = PROTO_PT;
			gProto[gunNo].init = Putian_Init;
			gProto[gunNo].rcvanddeal = Putian_RcvDealFrame;
			gProto[gunNo].criclereport = Putian_CricleReport;
			gProto[gunNo].ctrlreport = Putian_CtrlCricle;
			gProto[gunNo].singreport = Putian_ReportSingPkg;
			break;
		default:
			gProto[gunNo].protoNo = PROTO_GBT;
			gProto[gunNo].init = Gbt_Init;
			gProto[gunNo].rcvanddeal = Gbt_RcvDealFrame;
			gProto[gunNo].criclereport = Gbt_CricleReport;
			gProto[gunNo].ctrlreport = Gbt_CtrlCricle;
			gProto[gunNo].singreport = Gbt_ReportSingPkg;
			break;
	}
	
	gProto[gunNo].init(gunNo);
	
}
	
