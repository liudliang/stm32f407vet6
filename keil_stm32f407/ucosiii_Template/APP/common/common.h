/**
  ******************************************************************************
  * @file    common.h
  * @author  
  * @version v1.0
  * @date    
  * @brief   用了存放公用函数
  ******************************************************************************
	*/
#ifndef _COMMON_H_0001
#define _COMMON_H_0001
#include "sys.h"
//#include "typedefs.h"
//#include "debug.h"
//#include "Adebug.h"


#define TIM_1S		(OSCfg_TickRate_Hz) /* 每隔10ms计时，总共1S */
#define TIM_2S		(OSCfg_TickRate_Hz*2) /* 每隔10ms计时，总共1S */
#define TIM_3S		(OSCfg_TickRate_Hz*3) /* 每隔10ms计时，总共1S */
#define TIM_4S		(OSCfg_TickRate_Hz*4) /* 每隔10ms计时，总共1S */
#define TIM_5S		(OSCfg_TickRate_Hz*5) /* 每隔10ms计时，总共1S */
#define TIM_10S	(OSCfg_TickRate_Hz*10)/* 10s */

#define TIM_NS(n)		(OSCfg_TickRate_Hz*(n)) /* 每隔10ms计时，总共1S */

#define TIM_1MIN	(OSCfg_TickRate_Hz*60)/* 1min */
#define TIM_100MS (OSCfg_TickRate_Hz/10)
#define TIM_NMIN(n)	(OSCfg_TickRate_Hz*60*(n))/* 1min */

#define TIM_1MIN	(OSCfg_TickRate_Hz*60)/* 1min */
#define TIM_10MS ((OSCfg_TickRate_Hz)/100)


#define COMMON_RTN_ERR 0
#define COMMON_RTN_SUCC 1

typedef struct
{
	uint8_t u8Seq;
	uint8_t u8Adr;
	uint8_t prio;
}SEQ_ADR_ST;



typedef union
{
	 uint16_t s;
	 uint8_t  c[2];
}stoc_u;

typedef union
{
	 uint32_t i;
	 uint8_t  c[4];
}u32toc_u;

typedef union
{
	uint8_t  nbyte;
	struct
	{
		uint16_t bit0 :1;
		uint16_t bit1 :1;
		uint16_t bit2 :1;
		uint16_t bit3 :1;
		uint16_t bit4 :1;
		uint16_t bit5 :1;
		uint16_t bit6 :1;
		uint16_t bit7 :1;
	}bits;
}u8tobit_u;


typedef union
{
	uint16_t word;
	uint8_t  byte[2];
	struct
	{
		uint16_t bit0 :1;
		uint16_t bit1 :1;
		uint16_t bit2 :1;
		uint16_t bit3 :1;
		uint16_t bit4 :1;
		uint16_t bit5 :1;
		uint16_t bit6 :1;
		uint16_t bit7 :1;
		uint16_t bit8 :1;
		uint16_t bit9 :1;
		uint16_t bit10 :1;
		uint16_t bit11 :1;
		uint16_t bit12 :1;
		uint16_t bit13 :1;
		uint16_t bit14 :1;
		uint16_t bit15 :1;
	}bits;
}u16tobit_u;

typedef union
{
	uint32_t dword;
	uint16_t word[2];
	uint8_t  byte[4];
	struct {
		uint32_t bit0 : 1;
		uint32_t bit1 : 1;
		uint32_t bit2 : 1;
		uint32_t bit3 : 1;
		uint32_t bit4 : 1;
		uint32_t bit5 : 1;
		uint32_t bit6 : 1;
		uint32_t bit7 : 1;
		uint32_t bit8 : 1;
		uint32_t bit9 : 1;
		uint32_t bit10 : 1;
		uint32_t bit11 : 1;
		uint32_t bit12 : 1;
		uint32_t bit13 : 1;
		uint32_t bit14 : 1;
		uint32_t bit15 : 1;
		uint32_t bit16 : 1;
		uint32_t bit17 : 1;
		uint32_t bit18 : 1;
		uint32_t bit19 : 1;
		uint32_t bit20 : 1;
		uint32_t bit21 : 1;
		uint32_t bit22 : 1;
		uint32_t bit23 : 1;
		uint32_t bit24 : 1;
		uint32_t bit25 : 1;
		uint32_t bit26 : 1;
		uint32_t bit27 : 1;
		uint32_t bit28 : 1;
		uint32_t bit29 : 1;
		uint32_t bit30 : 1;
		uint32_t bit31 : 1;
	}bits;
}DWORT_BIT_ST;



extern uint32_t gSystemTick; 
#define GetSystemTick()		gSystemTick





extern void Common_Invert(uint8_t*buf,uint8_t len); /*颠倒字符串顺序*/

extern uint8_t Common_Hex2bcd8(uint8_t hex); /* HEX转换为BCD 8位*/
extern uint8_t Common_Bcd2hex8(uint8_t bcd);/* BCD转换为HEX 8位 */

extern uint32_t Common_Bcd2hex32(uint32_t bcd);/* BCD转换为HEX 32位*/
extern uint32_t Common_Hex2bcd32(uint32_t hex);/* HEX转换为BCD 32位*/


extern uint16_t Common_getLittleValue(uint16_t bigValue);/* 获取小端 16位*/
extern uint16_t Common_getBigValue(uint16_t littleValue);/* 获取大端 16位*/

extern int16_t Common_getPNBigValue(int16_t littleValue);/* 获取大端 16位带符号*/
extern int16_t Common_getPNLittleValue(int16_t BigValue);/* 获取小端 16位带符号*/

extern uint32_t Common_get2BigValue(uint32_t littleValue);/* 获取大端 32位*/
extern uint32_t Common_get2LittleValue(uint32_t BigValue);/* 获取小端 32位*/

extern uint32_t Common_CalclongSum(uint8_t* pbuf, uint16_t lenth);/* 计算校验和 */
extern uint16_t Common_CalculateSum(uint8_t* pbuf, uint16_t lenth);/* 计算校验和 */
extern uint8_t Common_CalcByteSum(uint8_t* pbuf, uint8_t lenth);/* 计算校验和 */
extern uint8_t Common_CalculateNOR(uint8_t *p);/* 计算异或校验 */
extern uint16_t Common_ModbusCRC(uint8_t *data, uint16_t len);/* MODBUS校验 */

/*BCD 码转10进制*/
extern uint8_t Common_Bcd_Decimal(uint8_t bcd); 

extern uint16_t Common_Change2Byte(uint16_t *data);


extern uint32_t Common_Change4Byte(uint32_t *data);

/*将BCD码转换成ASC码字符串,大写字符*/
extern uint8_t Common_BcdToAsc(uint8_t *str,uint8_t *bcd,uint8_t bynum);
/*将BCD码转换成ASC码字符串,小写字符*/
extern uint8_t Common_BcdToAscLit(uint8_t *str,uint8_t *bcd,uint8_t bynum) ;

/*将整数转换成float字符串*/
extern uint8_t Common_IntToFloatStr(uint8_t *str,int32_t data,uint8_t dotpos);



/*IP地址格式转换*/
extern uint8_t Common_FormatIpAddr(char *str,uint8_t ipv4[]);



/*将字符串变成整数*/
extern uint32_t Common_atol(char *str);

/*格式化mac地址*/
extern uint8_t Common_FormatMacAddr(char *str,uint8_t mac[]);

/*去除字符串末尾的空格 0xff*/
extern char *Common_trim(char *str);

extern uint8_t Common_AscToBcd(uint8_t *bcd,uint8_t *str);


extern uint8_t Common_NormlBcdToAsc(uint8_t *str,uint8_t *bcd,uint8_t bynum);

extern uint16_t Common_Change2ByteBigType(uint16_t data);

uint8_t Common_BigIntTo(uint8_t *pbuf,uint32_t utemp);//uint32_t位按大端模式转4个uint8_t
uint8_t  Common_LittleIntTo(uint8_t *pbuf,uint32_t utemp);//uint32_t位转4个uint8_t

uint8_t Common_LittleShortTo(uint8_t *pbuf,uint16_t utemp);//uint16_t位转2个uint8_t
uint8_t Common_BigShortTo(uint8_t *pbuf,uint16_t utemp);//uint16_t位转2个uint8_t
uint32_t Common_LittleToInt(uint8_t *pbuf);//4个uint8_t转uint32_t小端
uint32_t Common_BigToInt(uint8_t *pbuf);//4个uint8_t转uint32_t大端
uint16_t Common_LittleToShort(uint8_t *pbuf);//2个uint8_t转uint16_t

extern uint16_t HexToAsc(uint8_t Hex);

/*大端模式转换*/
extern uint8_t AscToHex(uint8_t *buf);

extern uint32_t Common_Ascii2uint32_t(uint8_t *str,uint8_t len);

extern uint8_t Common_TranCarNo(uint8_t *CardNo,uint8_t len);

extern uint8_t Common_bufTransAscii(uint8_t *buf,uint16_t len, uint8_t* dest);

#endif

