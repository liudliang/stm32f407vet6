/****************************************Copyright (c)**************************************************                                                           
**
**文件名: Config.h     
**
**作者:  RM       版本:v1.0        日期: 2010/4/26
**
**描述:    			
**
**		用户配置文件
**
**历史记录:        


****************************************************************************************************/
#ifndef __CONFIG_H 
#define __CONFIG_H

#include "ff.h"

//这一段无需改动
//This segment should not be modified
//#ifndef TRUE
//#define TRUE  1
//#endif
//
//#ifndef FALSE
//#define FALSE 0
//#endif
typedef long long      uint64;
typedef unsigned char  uint8;                   /* defined for unsigned 8-bits integer variable 	无符号8位整型变量  */
typedef signed   char  int8;                    /* defined for signed 8-bits integer variable		有符号8位整型变量  */
typedef unsigned short uint16;                  /* defined for unsigned 16-bits integer variable 	无符号16位整型变量 */
typedef signed   short int16;                   /* defined for signed 16-bits integer variable 		有符号16位整型变量 */
typedef unsigned int   uint32;                  /* defined for unsigned 32-bits integer variable 	无符号32位整型变量 */
typedef signed   int   int32;                   /* defined for signed 32-bits integer variable 		有符号32位整型变量 */
typedef float          fp32;                    /* single precision floating point variable (32bits) 单精度浮点数（32位长度） */
typedef double         fp64;                    /* double precision floating point variable (64bits) 双精度浮点数（64位长度） */

#ifndef FALSE
typedef enum {FALSE = 0, TRUE = !FALSE} bool;
#endif

//rm add
typedef void 		(*FUNCPTR) ();	/* ptr to function returning int */
//typedef	unsigned char BOOL;
typedef void VOID;
/********************************/
/*      uC/OS-II specital code  */
/*      uC/OS-II的特殊代码      */
/********************************/

#define     USER_USING_MODE    0x10                    /*  User mode ,ARM 32BITS CODE 用户模式,ARM代码                  */
// 
                                                     /*  Chosen one from 0x10,0x30,0x1f,0x3f.只能是0x10,0x30,0x1f,0x3f之一       */
#include "Includes.h"
			



extern void print(uint8 data);
/********************************/
/*      ARM的特殊代码           */
/*      ARM specital code       */
/********************************/
//这一段无需改动
//This segment should not be modify
#if defined  STM32F10X_HD
	#include    "stm32f10x.h"
#else
	#include    "stm32f4xx.h"
#endif

/********************************/
/*     应用程序配置             */
/*Application Program Configurations*/
/********************************/
//以下根据需要改动
#include    <stdio.h>
#include    <stdlib.h>
#include    <setjmp.h>
#include    <rt_misc.h>


/********************************/
/*     本例子的配置             */
/*Configuration of the example */
/********************************/
/* System configuration .Fosc、Fcclk、Fcco、Fpclk must be defined */
/* 系统设置, Fosc、Fcclk、Fcco、Fpclk必须定义*/
#define Fosc            12000000                    //Crystal frequence,10MHz~25MHz，should be the same as actual status. 
						    //应当与实际一至晶振频率,10MHz~25MHz，应当与实际一至
#define FCLK           ((Fosc/(5*2))*169)                  //System frequence,208MHZ
						   
#define FHCLK           (FCLK / 2)              //AHB BUS frequence

#define FPCLK           (FHCLK / 2)             //VPB clock frequence.
						            //VPB时钟频率，只能为(FCLK / 4)的1、2、4倍

#define DISP_BGPIC 0 

#define Delay10Ms(x)  OSTimeDly((x)*2)		// 延时x*10毫秒
#define Delay5Ms(x)   OSTimeDly((x))		  // 延时x*5毫秒
#define DelaySec(x)   OSTimeDly(OS_TICKS_PER_SEC * x) 	// 延时x秒

#define WAITMS(x)    ((x)/(1000/OS_TICKS_PER_SEC))   /*将毫秒转成系统滴答数*/



#endif


/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/
