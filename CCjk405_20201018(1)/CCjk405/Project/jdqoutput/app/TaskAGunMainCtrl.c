/**
  ******************************************************************************
  * @file    TaskMainCtrl.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#include "TaskMainCtrl.h"
#include "BmsCommUnit.h"


#include "ChgData.h"

void TaskAGunMainCtrl_proc(CTRL_STEP* gun_ctrl,PROTO_ST *ptrProto)
{


//	TaskMain_DealMsg(AGUN_NO);

	ChargeCtrlStep(gun_ctrl,ptrProto);
	
	
}

void TaskAGunMainCtrl(void *p_arg) 
{
	CTRL_STEP  gunACtrl; /*记得初始化*/

	memset(&gunACtrl,0,sizeof(CTRL_STEP));
	gunACtrl.gunNo = AGUN_NO;
	
	Message_QueueCreat(APP_TASK_AGUNMAINCTRL_PRIO);
	
	PROTO_ST *ptrProto = NULL;
	
	
	do{ /*等待获取协议*/
     ptrProto = BmsCommUnit_GetProtoPtr(AGUN_NO);
		 Delay10Ms(1);
	}while(NULL == ptrProto );
		
	while(1) {

	    TaskRunTimePrint("TaskAGunMainCtrl begin", OSPrioCur);


#if 0 		  		
		
		TaskMain_DealMsg(AGUN_NO);
	
		ChargeCtrlStep(&gunACtrl,ptrProto);
#else		
		TaskAGunMainCtrl_proc(&gunACtrl, ptrProto);
#endif		
		
		Delay10Ms(10);   //patli 20200106 Delay10Ms(1);

		TaskRunTimePrint("TaskAGunMainCtrl end", OSPrioCur);
		  		
	} //end while(1)	
	
}



