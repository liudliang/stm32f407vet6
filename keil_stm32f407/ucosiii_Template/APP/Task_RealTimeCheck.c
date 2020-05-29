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

static uint8_t iostatus[3][20] = {0};
u16tobit_u gInputStu; /*状态量采集*/

DEV_INPUT_TYPE gRealTimeCheck_Input;
DEV_STATUS_TYPE gRealTimeCheck_Status;

DEV_INPUT_TYPE *Check_GetInputDataptr(void)
{
	return &gRealTimeCheck_Input;
}

DEV_STATUS_TYPE *Check_GetStatusDataptr(void)
{
	return &gRealTimeCheck_Status;
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


void Check_ReadAllInputIo(void)
{
	static uint8_t i;
	i = i%3;
			
	iostatus[i][0] = ~ReadInputDataBit(INPUT_KEY1);			/* 0 */
	iostatus[i][1] = ~ReadInputDataBit(INPUT_KEY2);      /* 1 */
	iostatus[i][2] = ~ReadInputDataBit(INPUT_KEY3);      /* 2 */
//		iostatus[i][3] = ReadInputDataBit(INPUT3);      /* 3 */
//		iostatus[i][4] = ReadInputDataBit(INPUT4);      /* 4 */
//		iostatus[i][5] = ReadInputDataBit(INPUT5);      /* 5 */
//		iostatus[i][6] = ReadInputDataBit(INPUT6);      /* 6 */
//		iostatus[i][7] = ReadInputDataBit(INPUT7);      /* 7 */
//		iostatus[i][8] = ReadInputDataBit(INPUT8);      /* 8 */
//		iostatus[i][9] = ReadInputDataBit(INPUT9);      /* 9 */
//		iostatus[i][10] = ReadInputDataBit(INPUT10);    /* 10 */
//		iostatus[i][11] = ReadInputDataBit(INPUT11);    /* 11 */
//		iostatus[i][12] = ReadInputDataBit(INPUT_ACC1);
//		iostatus[i][13] = ReadInputDataBit(INPUT_BCC1);
	
	i++;
}

uint8_t Check_AllInputIo(void)
{
  uint8_t i;
	
	for( i = 0 ; i < 14 ; i++ ) {
		if( (iostatus[0][i] == iostatus[1][i]) && (iostatus[2][i] == iostatus[1][i]) ) {
			if((iostatus[0][i] & 0x01)== 1 ) {
			  gInputStu.word |= (1 << i );
			}else {
				 gInputStu.word &= ~(1 << i );
			}
		}
	}
	return 0;
}

extern short DS18B20_Get_Temp(void);
void Check_DevStatus(void)
{
	static uint8_t i = 0;
	static int16_t devtemp[3] = {0};
	
	if(3 <= i)
	{
		i = i%3;
		gRealTimeCheck_Status.temperature = (devtemp[0] + devtemp[1] + devtemp[2])/3;
	}
	devtemp[i] = DS18B20_Get_Temp();
	i++;
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
		Check_DevStatus();
		
		OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_HMSM_STRICT,&err); //延时50ms
	}
}





