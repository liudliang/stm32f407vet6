/**
  ******************************************************************************
  * @file    TaskIsoCheck.h
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#ifndef __TASKISOCHECK_H_0001
#define __TASKISOCHECK_H_0001
#include "typedefs.h"
#include "M_Global.h"
#include "message.h"



#if (ISO_TYPE ==  ISO_SINGLE) 
  #define MODBUS_ISO_ADR           0x03
	#define READ_REG_NUM             3             //0x0c
	#define MAX_RCV_SIZE             64
	#define MODBUS_ISO_START_REG     0x03          //0x00
	#define MODBUS_ISOCTRL_REG       0x17          //0x20
#endif
#if (ISO_TYPE ==  WBISO_SINGLE) 
  #define MODBUS_ISO_ADR           0x03
	#define READ_REG_NUM             5             //0x0c
	#define MAX_RCV_SIZE             64
	#define MODBUS_ISO_START_REG     0x00          //0x00
	#define MODBUS_ISOCTRL_REG       0x00          //0x00
#endif
#if (ISO_TYPE ==  ISO_AUNICE) /*奥耐自产绝缘模块*/
  #define MODBUS_ISO_ADR           0x01
	#define READ_REG_NUM             12 
	#define MAX_RCV_SIZE             64
	#define MODBUS_ISO_START_REG     0x00
	#define MODBUS_ISOCTRL_REG       0x20
#endif

//typedef struct
//{
//	uint32 starttick;
//	uint8  fg;
//	uint8  msgfg;
//	uint8  stopfg;
//	uint16 waittm;
//}CTRL_PARA_ST;




typedef struct {
		int16 vdc1;        /*接触器内侧电压*/
		int16 vdc2;        /*空*/
		uint16 res1pos;    /*一路正对地电阻*/
		uint16 res1neg;    /*一路负对地电阻*/
		uint16 res2pos;    /*二路正对地电阻*/
		uint16 res2neg;    /*二路正对地电阻*/
		int16 vdc3;        /*接触器外侧电压*/
		int16 vdc4;        /*空*/
		uint16 uab;
		uint16 ubc;
		uint16 uac;
		uint16 ver;
		union {
			uint16 word;
			uint8  byte[2];
			struct {
				uint16 commerr : 1; 	//0
				uint16 v1Over  : 1; 	//1
				uint16 v1Under : 1; 	//2
				uint16 v2Over  : 1; 	//3
				uint16 v2Under : 1; 	//4
				uint16 v3Over  : 1; 	//5
				uint16 v3Under : 1; 	//6
				uint16 v4Over  : 1; 	//7
				
				uint16 v4Under : 1; 	//8  4
				uint16 R1posErr : 1; 	//9  1路正对地故障
				uint16 R1negErr : 1; 	//10 1路负对地故障
				uint16 R2posErr : 1; 	//11 2路正对地故障
				uint16 R2negErr : 1; 	//12 2路负对地故障
				uint16 AcVoer  : 1;  	//13 交流过压
				uint16 AcUnder : 1;  	//14 交流欠压
				uint16 AcDeficy: 1;  	//15 交流缺相
			}bits;
		}statu;
}DEV_ISO_TYPE;


#pragma pack(1)
typedef  struct
{
	int16 vdc1;            
	int16 vdc2;
	uint16 res1pos;
	uint16 res1neg;
	uint16 res2pos;
	uint16 res2neg;
	uint16 vdc3;           
	uint16 vdc4;
	uint16 uab;
	uint16 ubc;
	uint16 uac;
	uint16 ver;

}ISO_PROTODA_ST;
#pragma pack()





extern void init_data(void);

extern void TskIso_InitCom(void);

extern void TskIso_DealMsg(MSG_STRUCT * msg);

extern uint8 TskIso_ReadAndDealData(void) ;

















#endif


