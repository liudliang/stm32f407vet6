/**
  ******************************************************************************
  * @file    An485Proto.c
  * @author  
  * @version v1.0
  * @date    2018-08-03
  * @brief   
  ******************************************************************************
*/
#include "typedefs.h"

#include "string.h"
#include "common.h"

#include "CdModel.h"

#include "An485Proto.h"




#define CRC8_POLY	0x07
static uint8 Crc8(uint8 *Pmsg, uint8 MsgSize)
{
	uint16 i, j, carry;
	uint16 msg,Poly,crc;

	Poly = CRC8_POLY ;
	crc = *Pmsg++;	// first byte loaded in "crc"
	for(i = 0 ; i < MsgSize-1 ; i ++)
	{
		msg = *Pmsg++;	// next byte loaded in "msg"
		for(j = 0 ; j < 8 ; j++)
		{
			carry = crc & 0x80;	// check if MSB=1
			crc = (crc << 1) | (msg >> 7);	// Shift 1 bit of next byte into crc
			if(carry) crc ^= Poly;	// If MSB = 1, perform XOR
			msg <<= 1;	// Shift left msg byte by 1
			msg &= 0x00FF;
		}
  }
	for(j = 0 ; j < 8 ; j++)
	{
		carry = crc & 0x80; crc <<= 1;
		if(carry) crc ^= Poly;
	}
	crc &= 0x00FF;	//We only want one byte (lower)
	return(crc);
	
}




uint8 AnProto_SendDataPkgHead(uint8 adr,uint8 devtype,FRAME_DATA *ptrData,uint8 *buf)
{
	uint8 index = 0;
	buf[index++] = START_FG;
	buf[index++] = devtype;
	buf[index++] = adr;
	buf[index++] = ptrData->info.byte;
	buf[index++] = ptrData->cmdtype;  //Common_Change4Byte
	
	u32toc_u tmp32;
	tmp32.i = 0;
	memcpy(tmp32.c,ptrData->cmddata,4);

	buf[index++] = tmp32.c[3];
	buf[index++] = tmp32.c[2];
	buf[index++] = tmp32.c[1];
	buf[index++] = tmp32.c[0];
	return index;
}

uint8 AnProto_TransAscii(uint8 *buf,uint8 len)
{
	uint8 i;
	stoc_u tmp16;
	uint8 tmpbuf[200];
	uint8 index = 0;
	if(buf[index++] != START_FG ) {
		 return 0;
	}
	memset(tmpbuf,0,sizeof(tmpbuf));
	for(i = 0 ; i < len ; i++ ) {
		tmp16.s = HexToAsc(buf[index+i]);
		tmpbuf[0+i*2] = tmp16.c[1] ;
		tmpbuf[1+i*2] = tmp16.c[0] ;
	}

	return index;
}

uint8 AnProto_TransAscAndPkgTail(uint8 *buf,uint8 len)
{
	uint8 i;
	stoc_u tmp16;
	uint8 tmpbuf[200];
	uint8 index = 0;
	if(buf[index++] != START_FG ) {
		 return 0;
	}
	memset(tmpbuf,0,sizeof(tmpbuf));
	for(i = 0 ; i < (len-1) ; i++ ) {
		tmp16.s = HexToAsc(buf[index+i]);
		tmpbuf[0+i*2] = tmp16.c[1] ;
		tmpbuf[1+i*2] = tmp16.c[0] ;
	}
	tmpbuf[i*2] = Crc8(&tmpbuf[0],i*2);
	
	tmp16.s = HexToAsc(tmpbuf[i*2]);
	tmpbuf[i*2] = tmp16.c[1];
	tmpbuf[i*2+1] = tmp16.c[0];
	
	tmpbuf[i*2+2] = TAIL_FG;
	
	index += i*2+3;
	memcpy(&buf[1],&tmpbuf[0],i*2+3);
  
	return index;
	
}


uint8 AnProto_TransHexAckData(uint8 *buf,uint8 len)
{
	uint8 tmpbuf[100] ;
	
	uint8 i,j,start;
	
	start  = 0xff;
	
	for( i = 0 ; i < (len-8);i++ )
	{
		if(buf[i] == START_FG ){
			 start = i+1;
		   break;
		}
	}
	if( start == 0xff ) {
		return 0;
	}

	j = 0;
	memset(tmpbuf,0,sizeof(tmpbuf));
	for( i = start ; i < (len-start-1) ; ) {
		tmpbuf[j++] = AscToHex(&buf[i]);
		i += 2;
	}
	if( j < 1 ) {
		return 0;
	}
	tmpbuf[j++] = TAIL_FG;
	
	buf[0] = START_FG;
	memcpy(&buf[1],&tmpbuf[0],j);
	
	return j+1;
}

uint8 AnProto_ParseFillAckData(uint8 *buf,uint8 len,uint8 adr)
{
	 
	 uint8 rtn =0;
	 CDMOD_DATA *ptrModdata = NULL;
	
	 if( buf[1] != 0x00 ) {
		  return 0;
	 }
	 
	 FRAME485_T *frame = (FRAME485_T *)buf;
	 if( adr != frame->addr ) {
		 return 0;
	 }
   
	 ptrModdata = CdModData_GetDataPtr(adr-1);
	 if( NULL == ptrModdata ) {
		 return 0;
	 }
	 
	 ptrModdata->lastticks = GetSystemTick();
	 ptrModdata->addr = adr;
	 
	 u32toc_u tmp32;
	 if( frame->info.bits.msgtype == EREAD_RESP )
	 {
		 switch(frame->cmdtype){
			 case EVOUT:
				 tmp32.c[0] = frame->data[3];
			   tmp32.c[1] = frame->data[2];
			   tmp32.c[2] = frame->data[1];
			   tmp32.c[3] = frame->data[0];
			   ptrModdata->outVolte = tmp32.i / 100; /*扩大10倍*/
				 break;
			 case ECURR:
				 tmp32.c[0] = frame->data[3];
			   tmp32.c[1] = frame->data[2];
			   tmp32.c[2] = frame->data[1];
			   tmp32.c[3] = frame->data[0];
			   ptrModdata->outCurr = tmp32.i / 100; /*扩大10倍*/
				 break;
			 case EVREF:
				 break;
			 case EILMT:
				 break;
			 case EONOFF:
				 break;
			 case EMODST:
				 tmp32.c[0] = frame->data[3];
			   tmp32.c[1] = frame->data[2];
			   tmp32.c[2] = frame->data[1];
			   tmp32.c[3] = frame->data[0];
			   ptrModdata->alarmInfo.u32d = tmp32.i; /*4字节有没有交换，待确认*/
				 break;
			 case ETEMP:
				 tmp32.c[0] = frame->data[3];
			   tmp32.c[1] = frame->data[2];
			   tmp32.c[2] = frame->data[1];
			   tmp32.c[3] = frame->data[0];
			   ptrModdata->temper = tmp32.i;
				 break;
			 case EGRPADRESS:
				 ptrModdata->grpNo = frame->data[3];
				 break;
			 default:
				 break;
		 }
		 rtn = 1;
	 }else if( frame->info.bits.msgtype == ESET_RESP ){
		   switch(frame->cmdtype) {
				  case EVREF:
						break;
					case EILMT:
						tmp32.c[0] = frame->data[3];
						tmp32.c[1] = frame->data[2];
						tmp32.c[2] = frame->data[1];
						tmp32.c[3] = frame->data[0];
						ptrModdata->limtPoint = tmp32.i;
						break;
				 
			 }
		 rtn = 1;
	 }
	 return rtn;
}

//uint8 AnProto_ParseAckData(uint8 *buf,uint8 len,uint8 adr);
//uint8 AnProto_SendDataPkgHead(uint8 adr,uint8 devtype,FRAME_DATA *ptrData,uint8 *buf);
//uint8 AnProto_TransHexAckData(uint8 *buf,uint8 len);
//uint8 AnProto_ParseFillAckData(uint8 *buf,uint8 len,uint8 adr);
//uint8 AnProto_TransAscAndPkgTail(uint8 *buf,uint8 len);


uint8 AnProto_SendDataPkg(uint8 adr,uint8 devtype,FRAME_DATA *ptrData,uint8 *buf)
{
	uint8 index = 0;
	index = AnProto_SendDataPkgHead(adr,devtype,ptrData,buf);
	index = AnProto_TransAscAndPkgTail(buf,index);
	return index;
}


uint8 AnProto_CheckCrc(uint8 *buf,uint8 rcvLen ) 
{
	uint8 i,crc1,crc2,start;

	
	for( i = 0 ; i < (rcvLen-8);i++ )
	{
		if(buf[i] == START_FG ){
			 start = i+1;
		   break;
		}
	}
	
	if( (buf[i] != START_FG) || ( buf[i+19] != TAIL_FG)) {
		return 0;
	}
	
	crc1 = AscToHex(&buf[i+17]);
	
	crc2 = Crc8(&buf[start],16);
	
	return (crc1 == crc2);
	
}

uint8 AnProto_ParseAckData(uint8 *buf,uint8 rcvLen,uint8 adr)
{
	uint8 len;
	if( rcvLen < 19 ) {
		return 0;
	}
	
	len = AnProto_CheckCrc(buf,rcvLen);
	if( 0 == len ) {
		 return 0;
	}
	
	len = AnProto_TransHexAckData(buf,rcvLen);
	if( len < 11 ) {
		return 0;
	}
	
	len = AnProto_ParseFillAckData(buf,len,adr);
	
	return len;
	
}




