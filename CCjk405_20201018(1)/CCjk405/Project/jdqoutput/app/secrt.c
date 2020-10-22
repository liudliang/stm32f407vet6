/**
  ******************************************************************************
  * @file    secrt.c
  * @author  zqj
  * @version v1.0
  * @date    2016-03-08
  * @brief   
  ******************************************************************************
	*/

#include "typedefs.h"
#include "secrt.h"

uint8* encode(uint8 *code,uint8 prim[],uint8 len) {
    for(int i = 0 ; i < len ; i++ ) {
		code[i] = ( prim[i]+i )^0xa5;
	}
	return code;
}


uint8* decode(uint8 *decode,uint8 code[],uint8 len) {
  for( int i = 0 ; i < len ; i++ ) {
	  decode[i] = (code[i] ^ 0xa5 )-i;
  }
  return decode;
}


