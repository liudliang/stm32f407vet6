/***********************************************************************
文件名称：
功    能：
编写时间：
编 写 人：
注    意：
***********************************************************************/
 
#ifndef _M_TYPE_P_H_
#define _M_TYPE_P_H_


/*
 * 使用该模块需要定义的基本数据类型
 *
 *   BOOLEAN: 布尔型
 *   INT8U  : 无符号的8位数
 *   INT8S  : 有符号的8位数
 *   INT16U : 无符号的16位数
 *   INT16S : 有符号的16位数
 *   INT32U : 无符号的32位数
 *   INT32S : 有符号的32位数
 * 	 FP32   : 单精度浮点数
 *   FP64   : 双精度浮点型
 * 
 *   Type_DataWidth  :  数据宽度类型
 *
 */


typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U; 
typedef signed   char  INT8S; 
typedef unsigned short INT16U;
typedef signed   short INT16S;
typedef unsigned int   INT32U;
typedef signed   int   INT32S;
typedef float          FP32;   
typedef double         FP64;   


//数据宽度类型
typedef enum{
    DATA_W8=0,	// 8位宽度
    DATA_W16,   //16位宽度
    DATA_W32,	//32位宽度
    DATA_W64	//64位宽度
}Type_DataWidth;


// 两个布尔类型
#ifndef M_TRUE
	#define M_TRUE	(1)
#endif

#ifndef M_FALSE
	#define M_FALSE (0)
#endif

#endif
