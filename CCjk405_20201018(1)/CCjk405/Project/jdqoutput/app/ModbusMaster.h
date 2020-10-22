/**
  ******************************************************************************
  * @file    ModbusMaster.h
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/

#ifndef _MODBUSMASTER_H_0001
#define _MODBUSMASTER_H_0001
#include "typedefs.h"

#define MODBUS_CMD_03  0x03
#define MODBUS_CMD_06  0x06


/*Modbus 06命令写单个寄存器 数据打包*/
uint8 Modbus_SingleRegPkg(uint8 *buf,uint8 devadr,uint8 cmd,uint16 wreg,uint16 val);
/*Modbus 06命令写单个寄存器 命令打包*/
extern uint8 Modbus_WritSingleRegPkg(uint8 *buf,uint8 devadr,uint16 wreg,uint16 val);

/*Modbus 03命令读寄存器 命令打包*/
extern uint8 Modbus_ReadPkg(uint8 *buf,uint8 devadr,uint16 startadr,uint8 regNum);

/*Modbus 校验数据 03 06 命令码校验 03正确返回 命令长度 06 返回1 ，错误返回0*/
extern uint8 Modbus_CheckReadPkg(uint8 *buf,uint8 devadr,uint8 cmd,uint8 regNum,uint8 len ) ;

























#endif



