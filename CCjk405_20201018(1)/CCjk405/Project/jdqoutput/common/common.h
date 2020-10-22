/**
  ******************************************************************************
  * @file    common.h
  * @author  zqj
  * @version v1.0
  * @date    2016-02-25
  * @brief   用了存放公用函数
  ******************************************************************************
	*/
#ifndef _COMMON_H_0001
#define _COMMON_H_0001
#include "typedefs.h"
#include "debug.h"
#include "Adebug.h"


#define TIM_1S		(OS_TICKS_PER_SEC) /* 每隔10ms计时，总共1S */
#define TIM_2S		(OS_TICKS_PER_SEC*2) /* 每隔10ms计时，总共1S */
#define TIM_3S		(OS_TICKS_PER_SEC*3) /* 每隔10ms计时，总共1S */
#define TIM_4S		(OS_TICKS_PER_SEC*4) /* 每隔10ms计时，总共1S */
#define TIM_5S		(OS_TICKS_PER_SEC*5) /* 每隔10ms计时，总共1S */
#define TIM_10S	(OS_TICKS_PER_SEC*10)/* 10s */

#define TIM_NS(n)		(OS_TICKS_PER_SEC*(n)) /* 每隔10ms计时，总共1S */

#define TIM_1MIN	(OS_TICKS_PER_SEC*60)/* 1min */
#define TIM_100MS (OS_TICKS_PER_SEC/10)
#define TIM_NMIN(n)	(OS_TICKS_PER_SEC*60*(n))/* 1min */

#define TIM_1MIN	(OS_TICKS_PER_SEC*60)/* 1min */
#define TIM_10MS ((OS_TICKS_PER_SEC)/100)


#define COMMON_RTN_ERR 0
#define COMMON_RTN_SUCC 1

typedef struct
{
	uint8 u8Seq;
	uint8 u8Adr;
	uint8 prio;
}SEQ_ADR_ST;



typedef union
{
	 uint16 s;
	 uint8  c[2];
}stoc_u;

typedef union
{
	 uint32 i;
	 uint8  c[4];
}u32toc_u;

typedef union
{
	uint8  nbyte;
	struct
	{
		uint16 bit0 :1;
		uint16 bit1 :1;
		uint16 bit2 :1;
		uint16 bit3 :1;
		uint16 bit4 :1;
		uint16 bit5 :1;
		uint16 bit6 :1;
		uint16 bit7 :1;
	}bits;
}u8tobit_u;


typedef union
{
	uint16 word;
	uint8  byte[2];
	struct
	{
		uint16 bit0 :1;
		uint16 bit1 :1;
		uint16 bit2 :1;
		uint16 bit3 :1;
		uint16 bit4 :1;
		uint16 bit5 :1;
		uint16 bit6 :1;
		uint16 bit7 :1;
		uint16 bit8 :1;
		uint16 bit9 :1;
		uint16 bit10 :1;
		uint16 bit11 :1;
		uint16 bit12 :1;
		uint16 bit13 :1;
		uint16 bit14 :1;
		uint16 bit15 :1;
	}bits;
}u16tobit_u;

typedef union
{
	uint32 dword;
	uint16 word[2];
	uint8  byte[4];
	struct {
		uint32 bit0 : 1;
		uint32 bit1 : 1;
		uint32 bit2 : 1;
		uint32 bit3 : 1;
		uint32 bit4 : 1;
		uint32 bit5 : 1;
		uint32 bit6 : 1;
		uint32 bit7 : 1;
		uint32 bit8 : 1;
		uint32 bit9 : 1;
		uint32 bit10 : 1;
		uint32 bit11 : 1;
		uint32 bit12 : 1;
		uint32 bit13 : 1;
		uint32 bit14 : 1;
		uint32 bit15 : 1;
		uint32 bit16 : 1;
		uint32 bit17 : 1;
		uint32 bit18 : 1;
		uint32 bit19 : 1;
		uint32 bit20 : 1;
		uint32 bit21 : 1;
		uint32 bit22 : 1;
		uint32 bit23 : 1;
		uint32 bit24 : 1;
		uint32 bit25 : 1;
		uint32 bit26 : 1;
		uint32 bit27 : 1;
		uint32 bit28 : 1;
		uint32 bit29 : 1;
		uint32 bit30 : 1;
		uint32 bit31 : 1;
	}bits;
}DWORT_BIT_ST;



extern uint32 gSystemTick; 
#define GetSystemTick()		gSystemTick





extern void Common_Invert(uint8*buf,uint8 len); /*颠倒字符串顺序*/

extern uint8 Common_Hex2bcd8(uint8 hex); /* HEX转换为BCD 8位*/
extern uint8 Common_Bcd2hex8(uint8 bcd);/* BCD转换为HEX 8位 */

extern uint32 Common_Bcd2hex32(uint32 bcd);/* BCD转换为HEX 32位*/
extern uint32 Common_Hex2bcd32(uint32 hex);/* HEX转换为BCD 32位*/


extern uint16 Common_getLittleValue(uint16 bigValue);/* 获取小端 16位*/
extern uint16 Common_getBigValue(uint16 littleValue);/* 获取大端 16位*/

extern int16 Common_getPNBigValue(int16 littleValue);/* 获取大端 16位带符号*/
extern int16 Common_getPNLittleValue(int16 BigValue);/* 获取小端 16位带符号*/

extern uint32 Common_get2BigValue(uint32 littleValue);/* 获取大端 32位*/
extern uint32 Common_get2LittleValue(uint32 BigValue);/* 获取小端 32位*/

extern uint32 Common_CalclongSum(uint8* pbuf, uint16 lenth);/* 计算校验和 */
extern uint16 Common_CalculateSum(uint8* pbuf, uint16 lenth);/* 计算校验和 */
extern uint8 Common_CalcByteSum(uint8* pbuf, uint8 lenth);/* 计算校验和 */
extern uint8 Common_CalculateNOR(uint8 *p);/* 计算异或校验 */
extern uint16 Common_ModbusCRC(uint8 *data, uint16 len);/* MODBUS校验 */

/*BCD 码转10进制*/
extern uint8 Common_Bcd_Decimal(uint8 bcd); 

extern uint16 Common_Change2Byte(uint16 *data);


extern uint32 Common_Change4Byte(uint32 *data);

/*将BCD码转换成ASC码字符串,大写字符*/
extern uint8 Common_BcdToAsc(uint8 *str,uint8 *bcd,uint8 bynum);
/*将BCD码转换成ASC码字符串,小写字符*/
extern uint8 Common_BcdToAscLit(uint8 *str,uint8 *bcd,uint8 bynum) ;

/*将整数转换成float字符串*/
extern uint8 Common_IntToFloatStr(uint8 *str,int32 data,uint8 dotpos);



/*IP地址格式转换*/
extern uint8 Common_FormatIpAddr(char *str,uint8 ipv4[]);



/*将字符串变成整数*/
extern uint32 Common_atol(char *str);

/*格式化mac地址*/
extern uint8 Common_FormatMacAddr(char *str,uint8 mac[]);

/*去除字符串末尾的空格 0xff*/
extern char *Common_trim(char *str);

extern uint8 Common_AscToBcd(uint8 *bcd,uint8 *str);


extern uint8 Common_NormlBcdToAsc(uint8 *str,uint8 *bcd,uint8 bynum);

extern uint16 Common_Change2ByteBigType(uint16 data);

uint8 Common_BigIntTo(uint8 *pbuf,uint32 utemp);//uint32位按大端模式转4个uint8
uint8  Common_LittleIntTo(uint8 *pbuf,uint32 utemp);//uint32位转4个uint8

uint8 Common_LittleShortTo(uint8 *pbuf,uint16 utemp);//uint16位转2个uint8
uint8 Common_BigShortTo(uint8 *pbuf,uint16 utemp);//uint16位转2个uint8
uint32 Common_LittleToInt(uint8 *pbuf);//4个uint8转uint32小端
uint32 Common_BigToInt(uint8 *pbuf);//4个uint8转uint32大端
uint16 Common_LittleToShort(uint8 *pbuf);//2个uint8转uint16

extern uint16 HexToAsc(uint8 Hex);

/*大端模式转换*/
extern uint8 AscToHex(uint8 *buf);

extern uint32 Common_Ascii2Uint32(uint8 *str,uint8 len);

extern uint8 Common_TranCarNo(uint8 *CardNo,uint8 len);

extern uint8 Common_bufTransAscii(uint8 *buf,uint16 len, uint8* dest);

#endif

