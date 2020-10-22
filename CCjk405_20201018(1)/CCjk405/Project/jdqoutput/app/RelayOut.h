/**
  ******************************************************************************
  * @file    relayOut.h
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
	
#ifndef __RELAYOUT_H_0001
#define __RELAYOUT_H_0001
#include "typedefs.h"

#define BMS_24V   1
#define BMS_12V   0

#define CTRL_ON   1
#define CTRL_OFF  0

#define MOD_CTRL_ON  0
#define MOD_CTRL_OFF 1

#define KM_ON   1
#define KM_OFF  0

typedef struct
{
	union {
		uint16 word;
		struct {
			uint16 km1 : 1;  /*km1¿ØÖÆ×´Ì¬*/
			uint16 km2 : 1;  /*km2¿ØÖÆ×´Ì¬*/
			uint16 km3 : 1;  /*km3¿ØÖÆ×´Ì¬*/
			uint16 km4 : 1;  /*km4¿ØÖÆ×´Ì¬*/
			uint16 rsv    : 12;
		}bits;	
	}ctrl;
	union {
		  uint16 word;
			uint8  byte[2];
			struct { 
				uint16 km1    : 1;   /*K1·´À¡×´Ì¬*/
				uint16 km2    : 1;   /*K2·´À¡×´Ì¬*/
				uint16 km3    : 1;   /*K3·´À¡×´Ì¬*/
				uint16 km4    : 1;   /*K4·´À¡×´Ì¬*/
				uint16 apow   : 1;   /*¸¨ÖúµçÔ´A*/
				uint16 bpow   : 1;   /*¸¨ÖúµçÔ´B*/
				uint16 rsv    : 10;
	    }bits;
	}statu;
}DEV_RELAY_TYPE;

extern void RelayOut_Init(void);

extern uint8 RelayOut_Locked(uint8 gunNo,uint8 fg);

extern uint8 RelayOut_AssistPower(uint8 gunNo,uint8 onoff);

extern uint8 RelayOut_Power12_24V(uint8 gunNo,uint8 fg);
extern uint8 RelayOut_GetBmsPower(uint8 gunNo);

extern uint8 RelayOut_DcKmOut(uint8 gunNo,uint8 onoff);

extern uint8 RelayOut_XfResOnOff(uint8 gunNo,uint8 onoff);

extern uint8 RelayOut_AcKmOut(uint8 onoff);

extern uint8 RelayOut_RunChargeLed(uint8 gunNo,uint8 onoff);

extern uint8 RelayOut_BreakdownLed(uint8 gunNo,uint8 onoff);

/* 2×éÄ£¿éÁªÂç½Ó´¥Æ÷*/
extern uint8 RelayOut_DcConTactKmOut(uint8 onoff);















#endif

