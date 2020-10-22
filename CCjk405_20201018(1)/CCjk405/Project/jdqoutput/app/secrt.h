/**
  ******************************************************************************
  * @file    secrt.h
  * @author  zqj
  * @version v1.0
  * @date    2016-03-08
  * @brief   
  ******************************************************************************
	*/
#ifndef _SECRT_H_0001
#define _SECRT_H_0001
	
extern uint8* encode(uint8 *code,uint8 prim[],uint8 len);
extern uint8* decode(uint8 *decode,uint8 code[],uint8 len);


#endif

