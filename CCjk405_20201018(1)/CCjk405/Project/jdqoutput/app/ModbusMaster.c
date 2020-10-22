/**
  ******************************************************************************
  * @file    ModbusMaster.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#include "ModbusMaster.h"
#include "common.h"
#include "crc.h"
#include <string.h>


/*Modbus 06命令写单个寄存器 数据打包*/
uint8 Modbus_SingleRegPkg(uint8 *buf,uint8 devadr,uint8 cmd,uint16 wreg,uint16 val)
{
	stoc_u crc;
	uint8 index = 0;
	buf[index++] = devadr;
	buf[index++] = cmd;
	buf[index++] = wreg >> 8;
	buf[index++] = wreg & 0xff;
	buf[index++] = val >> 8;
	buf[index++] = val & 0xff;
	
	crc.s = Crc16_Calc(buf,index);
	buf[index++] = crc.c[1];
	buf[index++] = crc.c[0];
	return index;

}
/*Modbus 06命令写单个寄存器 数据打包*/
uint8 Modbus_WritSingleRegPkg(uint8 *buf,uint8 devadr,uint16 wreg,uint16 val)
{
	stoc_u crc;
	uint8 index = 0;
	buf[index++] = devadr;
	buf[index++] = 0x06;
	buf[index++] = wreg >> 8;
	buf[index++] = wreg & 0xff;
	buf[index++] = val >> 8;
	buf[index++] = val & 0xff;
	
	crc.s = Crc16_Calc(buf,index);
	buf[index++] = crc.c[1];
	buf[index++] = crc.c[0];
	return index;

}

/*Modbus 03命令读寄存器*/
uint8 Modbus_ReadPkg(uint8 *buf,uint8 devadr,uint16 startadr,uint8 regNum)
{
	stoc_u crc;
	uint8 index = 0;
	buf[index++] = devadr;
	buf[index++] = 0x03;
	buf[index++] = startadr >> 8;
	buf[index++] = startadr & 0xff;
	buf[index++] = 0x00;
	buf[index++] = regNum;//0x0c;
	
	crc.s = Crc16_Calc(buf,index);
	buf[index++] = crc.c[1];
	buf[index++] = crc.c[0];
	return index;
}



/*Modbus 校验数据 03 06 命令码校验*/
uint8 Modbus_CheckReadPkg(uint8 *buf,uint8 devadr,uint8 cmd,uint8 regNum,uint8 len ) 
{
	 uint8 datalen;
	 stoc_u crc;
	 uint8 head = 0xff;
	 uint8 i = 0;
	 if( len < 4 ) {
		 return 0;
	 }
	 for( i = 0 ;i < len-4 ; i++ ) {
		 if(buf[i] == devadr ) {
			  if(buf[i+1] == cmd ) {
					if( MODBUS_CMD_03 == cmd ) {
						if(buf[i+2] == regNum * 2 ) {
							head = i;
							break;
						}
					}
          break;					
				}	
			}
	 }
	 if ((i < len-4) )  {
		 if( (MODBUS_CMD_03 == cmd) && (head < len-4) ) {
				datalen = 3+buf[head+2];
				crc.s = Crc16_Calc(&buf[head],datalen);
				if(crc.c[1] == buf[head+datalen] && crc.c[0] == buf[head+datalen+1] ) {
					if( 0 != head ) {
							memcpy(&buf[0],&buf[head],datalen+2);
					}
					return datalen+2;
			} 
		 } else if( MODBUS_CMD_06 == buf[i+1]){
			  crc.s = Crc16_Calc(&buf[i],6);
			  if(crc.c[1] == buf[i+6] && crc.c[0] == buf[i+7]) {
					 return 1;
				}
		 }
	 }
	 return 0;
}

