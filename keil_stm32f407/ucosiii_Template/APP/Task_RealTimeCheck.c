#include "sys.h"
#include "gpio.h"
#include "RealTimeCheck.h"
#include "delay.h"
#include "common.h"

//////////////////////////////////////////////////////////////////////////////////	 
//功能：实时检测数据	   
//作者：zyf
//创建日期:2020/04/25
//版本：V1.0										  
////////////////////////////////////////////////////////////////////////////////// 


u16tobit_u gInputStu; /*状态量采集*/

DEV_INPUT_TYPE gRealTimeCheck_Input;

DEV_INPUT_TYPE *Check_GetInputDataptr(void)
{
	return &gRealTimeCheck_Input;
}

/*将检测的io状态赋值给全局变量数据 */
void Check_TranIOStatus(void)
{
//	PARAM_OPER_TYPE *param = ChgData_GetRunParamPtr();
	
	DEV_INPUT_TYPE *pInputDataptr = Check_GetInputDataptr();
//	DEV_GUN_TYPE *pGunDataPtr = Check_GetGunDataPtr();
	
	pInputDataptr->statu.bits.key1   = gInputStu.bits.bit0;   			/*key1状态*/
	pInputDataptr->statu.bits.key2   = gInputStu.bits.bit1;   			/*key2状态*/
	pInputDataptr->statu.bits.key3   = gInputStu.bits.bit2 ;  			/*key3状态*/


}

uint8_t Check_AllInputIo(void)
{
	uint8_t i;
	uint8_t data[3][20] = {0};
	for( i = 0 ; i < 3 ; i++ ) {
		data[i][0] = ~ReadInputDataBit(INPUT_KEY1);			/* 0 */
		data[i][1] = ~ReadInputDataBit(INPUT_KEY2);      /* 1 */
		data[i][2] = ~ReadInputDataBit(INPUT_KEY3);      /* 2 */
//		data[i][3] = ReadInputDataBit(INPUT3);      /* 3 */
//		data[i][4] = ReadInputDataBit(INPUT4);      /* 4 */
//		data[i][5] = ReadInputDataBit(INPUT5);      /* 5 */
//		data[i][6] = ReadInputDataBit(INPUT6);      /* 6 */
//		data[i][7] = ReadInputDataBit(INPUT7);      /* 7 */
//		data[i][8] = ReadInputDataBit(INPUT8);      /* 8 */
//		data[i][9] = ReadInputDataBit(INPUT9);      /* 9 */
//		data[i][10] = ReadInputDataBit(INPUT10);    /* 10 */
//		data[i][11] = ReadInputDataBit(INPUT11);    /* 11 */
//		data[i][12] = ReadInputDataBit(INPUT_ACC1);
//		data[i][13] = ReadInputDataBit(INPUT_BCC1);
		delay_ms(20);
	}
	
	for( i = 0 ; i < 14 ; i++ ) {
		if( (data[0][i] == data[1][i]) && (data[2][i] == data[1][i]) ) {
			if((data[0][i] & 0x01)== 1 ) {
			  gInputStu.word |= (1 << i );
			}else {
				 gInputStu.word &= ~(1 << i );
			}
		}
	}
	return 0;
}






//TaskStackUsage任务函数
void RealTimeCheck_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	
//	printf("app start running!\n");	
	while(1)
	{
    Check_AllInputIo();
		Check_TranIOStatus();
		
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时20ms
	}
}





