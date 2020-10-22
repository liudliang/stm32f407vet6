/***********************************************************************
文件名称：
功    能：
编写时间：
编 写 人：
注    意：
***********************************************************************/
#include "M_Type_P.h"

/*
 * 功能：
 *       延时n x us
 * 参数：
 *       INT32U n  延时us数
 * 返回：
 *       无
 * 说明：
 *       无
 */
void M_Delay_us(INT32U n)
{
	volatile INT32U i; 
	
	//STM32F103ZE 72MHz	延时
	while(n--){
		for(i=0;i<32;i++);
	}


}

/*
 * 功能：
 *       延时n x ms
 * 参数：
 *       INT32U n 延时ms数
 * 返回：
 *       无
 * 说明：
 *       无
 */
void M_Delay_ms(INT32U n)
{
	volatile INT32U i,j;
	/*
	//STM32F103ZE 72MHz	延时
	while(n--){
		for(i=0;i<10;i++){
			for(j=0;j<900;j++);
		}
	}
	*/
	//STM32F407ZG 168MHz 延时
	while(n--){
		for(i=0;i<47;i++){
			for(j=0;j<900;j++);
		}
	}
}


