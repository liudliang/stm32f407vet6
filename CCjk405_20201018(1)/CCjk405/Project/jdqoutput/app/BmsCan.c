/**
  ******************************************************************************
  * @file    BmsCan.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/

#include "main.h"
#include "can.h"
#include "BmsCan.h"

extern int32 CanHander[];/* CAN句柄 */

/* 初始化CAN */
void Init_CAN(uint8 CanNo)
{
	/* 获取CAN句柄 */
	CanNo %= CAN_COM_MAX;
	CanHander[CanNo] = Can_Open(CanNo);
}

int32 Bms_CanRead(CAN_MSG* buf, uint32 timeOut, uint8 portNo)
{
	 int32 rtn = 0;
	
	 rtn = Can_Read(CanHander[portNo], buf, timeOut);
	
	 return rtn;
}

int32 Bms_CanWrite(CAN_MSG* msg, uint8 portNo)
{
	 int32 rtn = 0;
 
	 rtn = Can_Write(CanHander[portNo], msg);
	
	 return rtn;
}



