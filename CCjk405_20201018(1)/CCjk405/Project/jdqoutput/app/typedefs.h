/**
  ******************************************************************************
  * @file    typedefs.h
  * @author  zqj
  * @version v1.0
  * @date    2016-02-24
  * @brief   
  ******************************************************************************
	*/
#ifndef _TYPEDEFS_H_0001
#define _TYPEDEFS_H_0001

typedef long long      uint64;
typedef unsigned char  uint8;                   /* defined for unsigned 8-bits integer variable 	无符号8位整型变量  */
typedef signed   char  int8;                    /* defined for signed 8-bits integer variable		  有符号8位整型变量  */
typedef unsigned short uint16;                  /* defined for unsigned 16-bits integer variable 	无符号16位整型变量 */
typedef signed   short int16;                   /* defined for signed 16-bits integer variable 		有符号16位整型变量 */
typedef unsigned int   uint32;                  /* defined for unsigned 32-bits integer variable 	无符号32位整型变量 */
typedef signed   int   int32;                   /* defined for signed 32-bits integer variable 		有符号32位整型变量 */
typedef float          fp32;                    /* single precision floating point variable (32bits)    单精度浮点数（32位长度） */
typedef double         fp64;                    /* double precision floating point variable (64bits)    双精度浮点数（64位长度） */
//typedef unsigned long long  uint64;


//typedef	unsigned char BOOL;
typedef void VOID;

//#define  NULL   ( (void *)0 )

typedef int32 (*FptrWriteData)(uint8 *buf,uint16 len); 
typedef int32 (*FptrReadData)(uint8 *buf,uint16 len);

#undef  NULL
#define NULL (void *)0

//#ifndef __cplusplus
//typedef enum {FALSE = 0, TRUE = !FALSE} bool;
//#endif


#endif


