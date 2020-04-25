#ifndef _TEALTIMECHECK_H
#define _TEALTIMECHECK_H
#include "sys.h"
#include "cpu.h"
#include "includes.h"

//////////////////////////////////////////////////////////////////////////////////	 
//功能：实时检测数据	   
//作者：
//创建日期:2020/04/25
//版本：V1.0										  
////////////////////////////////////////////////////////////////////////////////// 

typedef struct {
	uint16_t temper1;
	union {
		  uint32_t word;
			uint8_t  byte[4];
			struct { 
			uint32_t	 stop    : 1;   /*0*/
			uint32_t	 acqf    : 1;   /*1*/
			uint32_t	 ackm    : 1;   /*2*/
			uint32_t	 spd     : 1;   /*3*/
			uint32_t	 posfuse : 1;   /*4*/
			uint32_t	 negfuse : 1;   /*5*/
			uint32_t	 dooracs : 1;   /*6*/	
			uint32_t	 dooraf  : 1;   /*7*/
			uint32_t	 fan     : 1;   /*8*/
			uint32_t	 smoke   : 1;   /*9*/
			uint32_t	 water   : 1;   /*10*/
			uint32_t	 key1    : 1;   /*11*/
			uint32_t	 key2    : 1;   /*12*/
			uint32_t	 key3    : 1;   /*13*/
			uint32_t	 key4    : 1;   /*14*/
			uint32_t	 dckmp   : 1;   /*15*/
			uint32_t   dckmn   : 1;   /*16*/
			uint32_t	 dc2kmp   : 1;   /*17*/
			uint32_t   dc2kmn   : 1;   /*18*/	
			uint32_t   rsv     : 13;
	    }bits;
	}statu;
}DEV_INPUT_TYPE;





DEV_INPUT_TYPE *Check_GetInputDataptr(void);



#endif

