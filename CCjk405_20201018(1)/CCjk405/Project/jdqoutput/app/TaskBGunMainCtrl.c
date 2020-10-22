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

void TaskBGunMainCtrl_proc(CTRL_STEP* gun_ctrl, PROTO_ST *ptrProto)
{

#ifndef A_B_GUN_TOGETER
	TaskMain_DealMsg(BGUN_NO);
#endif

	ChargeCtrlStep(gun_ctrl,ptrProto);
	
	
}


void TaskBGunMainCtrl(void *p_arg) 
{
	CTRL_STEP  gunBCtrl; /*记得初始化*/

	memset(&gunBCtrl,0,sizeof(CTRL_STEP));
	gunBCtrl.gunNo = BGUN_NO;
	
	Message_QueueCreat(APP_TASK_BGUNMAINCTRL_PRIO);
	
	PROTO_ST *ptrProto = NULL;
	
	do{ /*等待获取协议*/
     ptrProto = BmsCommUnit_GetProtoPtr(BGUN_NO);
		 Delay10Ms(1);
	}while(NULL == ptrProto );
		
	while(1) {
		
	TaskRunTimePrint("TaskBGunMainCtrl begin", OSPrioCur);
		

#if 0 		  		
				TaskMain_DealMsg(BGUN_NO);
				
				ChargeCtrlStep(&gunBCtrl,ptrProto);
#else		
		TaskBGunMainCtrl_proc(&gunBCtrl, ptrProto);
#endif		

		
		
		Delay10Ms(10);  // Delay10Ms(1);

	TaskRunTimePrint("TaskBGunMainCtrl end", OSPrioCur);
		
	} //end while(1)	
	
}



