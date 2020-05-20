#ifndef _DEBUG_H
#define _DEBUG_H

#include "sys.h"
#include "string.h"
#include "M_Global.h"

//////////////////////////////////////////////////////////////////////////////////	 
//功能：   
//作者：
//创建日期:2020/04/25
//版本：V1.0									  
//////////////////////////////////////////////////////////////////////////////////

extern u8 debug_tcp_client_flag;
extern char DEBUG_Buff[50];
void DEBUG_Printf(char *s);
void DEBUG_Init(void);
#endif

