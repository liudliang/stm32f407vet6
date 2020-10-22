
#include "main.h"


#include "can.h"
#include "Common.h"
#include "message.h"
#include "List.h"

#include "TaskModule.h"



//#ifndef  ChargeAC_Mode_EN   //
//const uint16 valuetype[] = {OUT_VOLTE,OUT_CURR,SETTING_LIMIT_POINT,INPUT_TEMPER,ALARM_WORDS\
//                         ,INPUT_AC_AVOLT,INPUT_AC_BVOLT,GROUP_NUMBER};
//#else
//const uint16 valuetype[] = {OUT_VOLTE,OUT_CURR,SETTING_LIMIT_POINT,INPUT_TEMPER,ALARM_WORDS\
//                         ,INPUT_AC_AVOLT,INPUT_AC_BVOLT,INPUT_AC_CVOLT,GROUP_NUMBER};
//#endif
//#define VALUETYPE_NUM (sizeof(valuetype)/sizeof(uint16))


extern int32 CanHander[];/* CAN句柄 */
/*初始化CAN*/
void TskMod_InitCAN(void)
{
	uint32 Type=0;
	
	/*获取CAN的句柄*/
	CanHander[CDMOD_CAN] = Can_Open(0);
	/*CAN全部接收*/
//	Type = CAN_TYPE_GROUP_RCV;//CAN_TYPE_GROUP_RCV
//	Can_Ctrl(CanHander[CDMOD_CAN],CAN_RCV_TYPE_SET,&Type,0,0);

//	/*波特率设置*/
//	Type = 125000;
//	Can_Ctrl(CanHander[CDMOD_CAN],CAN_BAUD_SET,&Type,0,0);

}

  

CMD_LIST gCmdList[30];
#define LIST_CMD_NUM (sizeof(gCmdList)/sizeof(CMD_LIST))

/***********命令列表初始化*****************************************/
void TskMod_InitData(void)
{
	
	//memset(&gCmdList,0,sizeof(gCmdList));	
}


/********************************************************************
接收充电模块的数据

*********************************************************************/
uint8 TskMod_ReadData(uint8 localAdr,uint8 dstAdr,uint16 *nCmd)
{
	uint16 cmd;
	uint8 rtn;
	uint16 cnt,tmp;
	uint8 addr = 0;
	CAN_MSG Canmsg;
	
	CDFRAME_HEAD *frmhead = (CDFRAME_HEAD *)&(Canmsg.Id.ExtId);
	memset(&Canmsg,0,sizeof(CAN_MSG));
	 
	tmp = 0;
  rtn = 0;
	cnt = 0;
	cmd = ERR_CMD;
	do{ 
		if(Can_Read(CanHander[CDMOD_CAN],&Canmsg,0) > 0) {
			if(CdModel_CheckCanId(Canmsg.Id.ExtId) > 0) {
				if( (localAdr == frmhead->sbit.dst) && (dstAdr == frmhead->sbit.src)) {
					  cmd = CdModel_ParseFrame(addr,(uint8 *)Canmsg.Data);
					  if( cmd > ERR_CMD ) {
						  tmp++;
					}
				}
			}
		}else {
			if( tmp > 0 ) {
				*nCmd = cmd;
				break;
			}
			Delay10Ms(2); 
		}
    cnt++;			
	}while(cnt < 5);
	
	rtn = tmp > 0 ? 1 : 0;
	return rtn;
}


/********************************************************************
gCmdList队列的命令发送    
*********************************************************************/
int TskMod_ExecuCmd(void)
{
	
   uint16 rCmd;

	 CMD_LIST * tempCmd;
	 struct list *node = CdMod_GetHead();
	 while(node)
	 {
		 tempCmd = (CMD_LIST *)node->cmdst;
		 CdModel_SetData(tempCmd->dstAddr,tempCmd->groupAddress,tempCmd->cmd,tempCmd->vtype,(void *)&(tempCmd->value));
		 Delay10Ms(2);
		 if ( (BRAODCAST_ADR != tempCmd->dstAddr) && (TskMod_ReadData(CD_HOSTID,tempCmd->dstAddr,&rCmd)) ) {
			 if ( rCmd == tempCmd->cmd ) {
				  tempCmd->succ = 1;;
			 }
		 }else {
			  if ( rCmd == tempCmd->cmd ) {
					tempCmd->exec++;
			  }
		 }
		 Delay10Ms(2);
		 if((tempCmd->exec > CMD_SEND_TIMES) || (1 ==  tempCmd->succ)) {
			  CdMod_deleteCmd(node->id);
		 }
		 node = node->next;
	 }
	 return 0;
}

/*模块设置命令放入列表*/
int TskMod_InsertModCmdInList(uint8 adr,uint8 grpNo,CD_VALUETYPE cmd,uint32 value)
{
	 CMD_LIST cmdl;
	 int err,cnt;
	 
	 err = -1;
	 memset(&cmdl,0,sizeof(CMD_LIST));
   cmdl.dstAddr = adr;
   cmdl.cmd  = SETTING_DATA;
	 cmdl.vtype = cmd;   
	 cmdl.value = value;
	 cmdl.groupAddress = grpNo;
	 cnt = 0;
   err = CdMod_InsertOneCmd(&cmdl);
	 while (err == -1){
		  Delay10Ms(2);
		  err = CdMod_InsertOneCmd(&cmdl);
		  if( err != -1 ) {
				break;
			}
			if( cnt++ > 10 ) {
				err = -1;
				break;
			}
	 }
	 return err;
}	



const uint16 conGatherCdMod[] = {0x0001,0x0002,0x0003,0x0040,0x000c,0x000d,0x000e};
uint8 TskMod_GatherCdModData(uint8 objAdr)
{
	uint8 rtn;
	uint16 cnt,tmp;
	uint8 addr = 0;
	CAN_MSG Canmsg;
	static uint8 sSeq = 0;
	uint32 temp = 0;
	CdModel_SendData(objAdr,REQ_BYTEDATA,conGatherCdMod[sSeq],&temp);
	Delay10Ms(2); 
	
  CDFRAME_HEAD *frmhead = (CDFRAME_HEAD *)&(Canmsg.Id.ExtId);
	memset(&Canmsg,0,sizeof(CAN_MSG));
	 
	tmp = 0;
  rtn = 0;
	cnt = 0;
	do{ 
		if(Can_Read(CanHander[CDMOD_CAN],&Canmsg,0) > 0) {
			if(CdModel_CheckCanId(Canmsg.Id.ExtId) > 0) {
				if( objAdr == frmhead->sbit.src ) {
			      addr = frmhead->sbit.src;
					  if( CdModel_ParseFrame(addr,(uint8 *)Canmsg.Data)) {
						  tmp++;
					   }
				 }
			 }
		}else {
			if( tmp > 0 ) {
				break;
			}
			Delay10Ms(2); 
		}
		cnt++;			
	}while(cnt < 5);
	
	sSeq++;
	sSeq %= (sizeof(conGatherCdMod)/sizeof(uint16));
	
	rtn = tmp > 0 ? 1 : 0;
	
	return rtn;
}





void TskMod_DealMessage(void)
{
	 MSG_STRUCT msg;
	 if( RcvMsgFromQueue(&msg) == TRUE ) {
		 	switch(msg.MsgType)
			{
				case MSG_MOD_ADJUST:

			  break;
//				case MSG_ADJUST_VOLT:
//					break;
//				case MSG_MOD_ONOFF:
//					break;
//				case MSG_MOD_GROUP:
//					break;
		default:
			break;
	}
	 }
	
	


}



#ifdef CAN_CDMOD
void Task_CdModule(void *p_arg)
{
    
    uint8 modAddr;
	  TskMod_InitCAN();
	  TskMod_InitData();
	  CdModData_Init();
	
	  modAddr = 0;
    while(1)
		{			 
			  /*先执行控制命令*/
				TskMod_ExecuCmd();
			  /*轮询采集数据*/
        TskMod_GatherCdModData(modAddr);
			
			  modAddr = (modAddr+1)% CD_MAX_NUM;
			
				Delay10Ms(1);			
    }
}

#endif








