/**
  ******************************************************************************
  * @file    An485Proto.h
  * @author  
  * @version v1.0
  * @date    2018-08-03
  * @brief   
  ******************************************************************************
*/
#ifndef __AN485PROTO_H_0001
#define __AN485PROTO_H_0001
#include "typedefs.h"

#define MIN_PKG_LEN  19
#define START_FG   0x7E
#define TAIL_FG    0x0D
#define DEVTYPE_CODE 0x00





#pragma pack(1)
typedef  union
{
  uint8 byte;
	 struct {
		uint8 msgtype:4;
		uint8 grpadr:4;
	}bits;
}GRPINFO;

typedef  struct
{
	uint8 startfg;
	uint8 devtype;
	uint8 addr;
  GRPINFO info;
	uint8 cmdtype;
	uint8 data[4];
	uint8 crc;
	uint8 tail;
}FRAME485_T;

typedef  struct
{
	GRPINFO info;
  uint8   cmdtype;
  uint8   cmddata[4];	
}FRAME_DATA;
#pragma pack()


extern uint8 AnProto_SendDataPkg(uint8 adr,uint8 devtype,FRAME_DATA *ptrData,uint8 *buf);

extern uint8 AnProto_ParseAckData(uint8 *buf,uint8 rcvLen,uint8 adr);




#endif
