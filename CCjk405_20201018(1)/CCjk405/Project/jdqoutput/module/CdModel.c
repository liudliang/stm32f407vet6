/*CdModel.c*/

#include "main.h"
#include "can.h"
#include "Common.h"
#include "List.h"
#include "ChgData.h"
#include "CdModel.h"
#include "PowerSplitt.h"




uint8 gSetGrpFg,m_gModnum;
uint8 m_gModAddr,m_gModCmdListIndex;
COMM_MODLIST gModCommList[MAX_MOD_GROUP];
MOD_CTRL_PARA gModGpPara[MAX_MOD_GROUP];

CDMOD_DATA gCdmodData[CD_MAX_NUM+1];
extern int32 CanHander[];/* CAN句柄 */

MOD_CTRL_PARA *CdModData_GetModGpPara(uint8 grpNo)
{
	return &gModGpPara[grpNo];
}


CDMOD_DATA * CdModData_GetDataPtr(uint8 no)
{
	if( no < CD_MAX_NUM ) {
		 return &gCdmodData[no];
	}
	return NULL;
}


/**grpNo 组号从0组开始*/
void CdModData_Init(uint8 grpNo)
{
	  uint8 i,pregrp;
	  CDMOD_DATA * ptrModData = CdModData_GetDataPtr(0);
	  PARAM_OPER_TYPE *ptrRunParam = ChgData_GetRunParamPtr();
		int8 len = 0;
	  pregrp = grpNo > 0 ? grpNo-1 : 0;
	  uint8 start = grpNo *(ptrRunParam->grpmodnum[pregrp]);
	  uint8 end = start + (ptrRunParam->grpmodnum[grpNo]);
	
	  memset(&ptrModData[start],0,sizeof(CDMOD_DATA)*ptrRunParam->grpmodnum[grpNo]);

	
	  for( i = start ; i < end; i++ ) {
			ptrModData[i].commErr = 1;  /*上电默认通讯故障*/
			ptrModData[i].addr = 0;
			ptrModData[i].grpNo = grpNo+1; /*1,2*/
			ptrModData[i].alarmInfo.bits.ModStu = 1; /*模块关机*/
			gModCommList[grpNo].AddrList[i] = i+1;
			
		}
		gModCommList[grpNo].vaild = 0;
		gModCommList[grpNo].num = 0;
		
		DEV_LOGIC_TYPE *ptrLogic = ChgData_GetLogicDataPtr((grpNo)% ptrRunParam->grpmodnum[grpNo]);
		ptrLogic->gunInUsed = 1;
		/*初始化功率分配参数*/
		PowerSplitt_ParamInit(grpNo);
		
}

/*检测可通讯的模块是否与配置一致，以此判断是否有通讯故障*/
uint8 CdModData_CheckCommErr(uint8 grpNo)
{
	 static uint8 sErrCnt[2] = {0};
	 uint8 i,cnt,pregrp;
	 uint32 curr_gun[2] = {0},vol_gun[2] = {0},tmpdata = 0;
	 CDMOD_DATA * ptrModData = CdModData_GetDataPtr(0);
	 PARAM_OPER_TYPE *ptrRunParam = ChgData_GetRunParamPtr();
	 MOD_CTRL_PARA *ptrModGpPara = CdModData_GetModGpPara((grpNo)%MAX_MOD_GROUP);
	 DEV_METER_TYPE *pDCMet = TskDc_GetMeterDataPtr(grpNo);
	 

#if 1	
	  grpNo %= MAX_MOD_GROUP;
	  pregrp = grpNo > 0 ? grpNo-1 : 0;
	
	  uint8 start = grpNo *(ptrRunParam->grpmodnum[pregrp]);
	  uint8 end = start + (ptrRunParam->grpmodnum[grpNo]);
	
	 cnt = 0;
	 for( i = start ; i < end; i++ ) {
		 if( GetSystemTick() -  ptrModData[i].lastticks > 6*TIM_1S) {
				ptrModData[i].lastticks = GetSystemTick();
				ptrModData[i].commErr = 1;
				ptrModData[i].vaild = 0;
		  }
			
		 if( 0 ==  ptrModData[i].commErr ) {
			   ptrModData[i].vaild = 1;
				 cnt++;
			
				 curr_gun[grpNo] += ptrModData[i].outCurr;    //add20201124zyf
			   tmpdata = ptrModData[i].outVolte;
				 if(vol_gun[grpNo] < tmpdata)
				 {
					 vol_gun[grpNo] = tmpdata;
				 }
		 }else {
			 ptrModData[i].vaild = 0;
		 }
	 }
	 
	if(DCMETER_RECT == DCMETER_TYPE)      //add20201124zyf
	{
		pDCMet->current = curr_gun[grpNo]; 
		pDCMet->volt = vol_gun[grpNo]; 
	}
	 
	 if((cnt <= (ptrRunParam->grpmodnum[grpNo]) ) && (cnt > 0 ) ) {
		 gModCommList[grpNo].num = cnt;
		 gModCommList[grpNo].vaild = 1;
		 ptrModGpPara->u8CommStu = 1;
		 sErrCnt[grpNo] = 0;
		 cnt = 0;
	 }else {
		 if( sErrCnt[grpNo]++ > 15 ) {
			 ptrModGpPara->u8CommStu = 0;
		 }
		 gModCommList[grpNo].vaild = 0;
		 cnt = 1;
	 }
#endif

	 return cnt;
}


uint8 CdModData_CheckOnOffStu(uint8 grpNo,uint8 stu)
{
	 uint8 i,cnt,pregrp;
	 CDMOD_DATA * ptrModData = CdModData_GetDataPtr(0);
	 PARAM_OPER_TYPE *ptrRunParam = ChgData_GetRunParamPtr();
	
	 grpNo %= MAX_MOD_GROUP;
	 pregrp = grpNo > 0 ? grpNo-1 : 0;
	 uint8 start = grpNo *(ptrRunParam->grpmodnum[pregrp]);
	 uint8 end = start + (ptrRunParam->grpmodnum[grpNo]);

	 cnt = 0;
	 for( i = start ; i < end; i++ ) {
		 if(ptrModData[i].vaild == 1 ) {
			 if( ptrModData[i].alarmInfo.bits.ModStu == stu ) {
					cnt++;
			 }
	  }
	 }
	 if( (cnt <= ptrRunParam->grpmodnum[grpNo]) &&(cnt > 0 )) {
			return TRUE;
	 }
	 return FALSE;
}


uint8 CdModData_GetModCommErr(uint8 grpNo)
{
	 uint8 i,cnt,pregrp;
	 CDMOD_DATA * ptrModData = CdModData_GetDataPtr(0);
	 PARAM_OPER_TYPE *ptrRunParam = ChgData_GetRunParamPtr();
	
	  grpNo %= MAX_MOD_GROUP;
	  pregrp = grpNo > 0 ? grpNo-1 : 0;
	  uint8 start = grpNo *(ptrRunParam->grpmodnum[pregrp]);
	  uint8 end = start + (ptrRunParam->grpmodnum[grpNo]);
	
	 cnt = 0;
	 for( i = start ; i < end; i++ ) {
		 if(ptrModData[i].vaild == 1 ) {
				cnt++;
	  }
	 }
	 if( (cnt <= ptrRunParam->modnum/MAX_MOD_GROUP) && (cnt > 0 )) {
			return TRUE;
	 }
	 return FALSE;

}


uint8 CdModData_DelAllCmd(void)
{
	 struct list *node = CdMod_GetHead();
	 while(node)
	 {
		  CdMod_deleteCmd(node->id);
		  node = node->next;   
	 }
	return 0;
}




/*模块设置命令放入列表*/
//int Tsk485Mod_InsertModCmdInList(uint8 adr,uint8 grpNo,ECMD_TYPE cmd,uint32 value)
int CdModData_InsertModCmdInList(uint8 adr,uint8 grpNo,uint16 cmd,uint32 value)
{
	 CMD_LIST cmdl;
	 int err,cnt;
	 
	 err = -1;
	 memset(&cmdl,0,sizeof(CMD_LIST));
   cmdl.dstAddr = adr;
   cmdl.cmd  = ESET_DATA;
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




uint8 CdModData_StopGpMod(uint8 grpNo)
{
	uint8 i;
	grpNo %= MAX_MOD_GROUP;
	
	/*单模块关机*/
	for( i = 0 ; i < gModGpPara[grpNo].u8HwGpModNum;i++)
	{
		CdModData_InsertModCmdInList(gModGpPara[grpNo].u8HwGpAddr[i],grpNo,MOD_ONOFF,MOD_CMD_OFF);
	}
	
	/*设置模块分组*/
	for( i = 0 ; i < gModGpPara[grpNo].u8HwGpModNum;i++)
	{
		CdModData_InsertModCmdInList(gModGpPara[grpNo].u8HwGpAddr[i],0,MOD_SETGROUP,gModGpPara[grpNo].u8GpNo);
	}
	
	return 0;
}

uint8 CdModData_SetGroupNo(uint8 adr,uint8 grpNo)
{
	return CdModData_InsertModCmdInList(adr,0,MOD_SETGROUP,grpNo); 
}

uint8 CdModData_SetGroupCmd(uint8 grpNo,uint8 cmd,uint32 value)
{
   return CdModData_InsertModCmdInList(0,grpNo,cmd,value);
}

uint8 CdModData_SetSingleCmd(uint8 adr,uint8 cmd,uint32 value)
{
	return CdModData_InsertModCmdInList(adr,0,cmd,value); 
}

//void Tsk485Mod_MsgDeal(MSG_STRUCT* msg)
/**************************************************
ptrModGpPara->u8GpNo 组号从1 开始，1组，2组
***************************************************/
static const uint8 conModAdr[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
static const uint16 conGatherCdMod[] = {ALARM_WORDS,OUT_VOLTE,OUT_CURR,MOD_LIMIT_POIN,INPUT_TEMPER,INPUT_AC_AVOLT,INPUT_AC_BVOLT,INPUT_AC_CVOLT};
#define CAN_CMDTYPE_LIST_NUM  (sizeof(conGatherCdMod)/sizeof(uint16)) 
extern uint8 TskMod_GatherCdModData(uint8 objAdr,uint8 cmdtypeindex);

void CdModData_MsgDeal(MSG_STRUCT* msg)
{
	 uint8 grpNo = 0;
	 stoc_u tmps2c;
	 uint32 tmp32 =  0;
	 DEV_LOGIC_TYPE *ptrLogic; //= ChgData_GetLogicDataPtr((grpNo+1)% MAX_MOD_GROUP);
	
	 grpNo = msg->MsgData[0];
	 MOD_CTRL_PARA *ptrModGpPara = CdModData_GetModGpPara((grpNo)%MAX_MOD_GROUP);
	 switch(msg->MsgType)
	 {
		case MSG_MOD_GET_INFO:
				/*轮询采集数据*/
			TskMod_GatherCdModData(conModAdr[m_gModAddr],m_gModCmdListIndex);
	      
			m_gModAddr++;
			if( m_gModAddr == m_gModnum ) {
					
				gSetGrpFg = 0;
				m_gModAddr = 0;
				m_gModCmdListIndex++;
				m_gModCmdListIndex %= CAN_CMDTYPE_LIST_NUM;
					
			}
			  
			break;
		case MSG_MOD_ONOFF:
		  m_gModCmdListIndex = 0;
			if( msg->MsgData[1] == MOD_CMD_ON ) {
				
				DebugInfoByMod("电源模块:开");
			   CdModData_SetGroupCmd(ptrModGpPara->u8GpNo,MOD_ONOFF,MOD_CMD_ON);
			}else {
				 ptrLogic = ChgData_GetLogicDataPtr((grpNo)% MAX_MOD_GROUP);
				 ptrLogic->gunInUsed = 0;
				 CdModData_SetGroupCmd(ptrModGpPara->u8GpNo,MOD_ONOFF,MOD_CMD_OFF);
				 
				 DebugInfoByMod("电源模块:关");
			}
			break;
		case MSG_MOD_ADJUST:
			ptrLogic = ChgData_GetLogicDataPtr((grpNo)% MAX_MOD_GROUP);
			tmps2c.c[0] = msg->MsgData[3]; /*电流*/
			tmps2c.c[1] = msg->MsgData[4];
		  tmp32 = tmps2c.s * COFF_DATA;

		  
		  DebugInfoWithPi(MOD_MOD, "电源模块调整电流", tmp32);
		  CdModData_SetGroupCmd(ptrModGpPara->u8GpNo,MOD_SETCURR,tmp32);
		  tmps2c.c[0] = msg->MsgData[1]; /*电压*/
			tmps2c.c[1] = msg->MsgData[2];
		  tmp32 = tmps2c.s * COFF_DATA;

		  
		  DebugInfoWithPi(MOD_MOD, "电源模块调整电压", tmp32);
		  CdModData_SetGroupCmd(ptrModGpPara->u8GpNo,MOD_SETVOLT,tmp32); 
		  ptrLogic->gunInUsed = 0;
			break;
		case MSG_MOD_GRPSET:
		  gSetGrpFg = 1;
			break;
		case MSG_MOD_DELALLCMD:
			DebugInfoByMod("删除所有电源模块");
			
			CdModData_DelAllCmd();
			break;
		case MSG_MODDATA_INIT:
			/*组号从0开始*/
			DebugInfoByMod("初始化电源模块");
			
			CdModData_Init(grpNo);
		 break;
		case MSG_MOD_PRESTOP: /*启动前停止本组，设置组号*/
			
			DebugInfoByMod("启动前停止本组，设置组号");
			PowerSplitt_PreemExitGroup(grpNo,&gModGpPara[grpNo]);
			break;
		case MSG_MOD_DGRPSET:
			tmp32 = msg->MsgData[1];   /*模块地址*/
		  if( grpNo <= MAX_MOD_GROUP ) {
				CdModData_SetGroupNo(tmp32,ptrModGpPara->u8GpNo); 
			}
		 break;
		case MSG_MODGRP_ADJUST:  /*BCL中发送消息*/
			
			DebugInfoByMod("Code Bcl Need");
		    PowerSplitt_CalcPower(ptrModGpPara->u8GpNo,&gModGpPara[grpNo]);
			PowerSplitt_ModCtrlFunc(ptrModGpPara->u8GpNo,&gModGpPara[grpNo]);
			break;
		default:
			break;
	 }
}





static struct list *gListCmdHead = NULL;
struct list *CdMod_GetHead(void)
{
	return gListCmdHead;
}


/**************添加一个命令到列表中***********************************/
int CdMod_InsertOneCmd(CMD_LIST * scmd)
{
  return list_add(&gListCmdHead,sizeof(CMD_LIST),scmd);
}

/**********************************************************************
判断加入队列的命令是否被执行
已经执行返回1，超时返回0
**********************************************************************/
int CdMod_IsListExec(int id,uint16 wmsecs) 
{
	int rtn = 0;
  rtn = list_query(&gListCmdHead,id,NULL);
	rtn = rtn == 0 ? 1 : 0;
	return rtn;
}

/**********************************************************************
删除队列中的命令
参数 addr = 0xff 删除全部符合CMD = vtype的命令
**********************************************************************/
int CdMod_deleteCmd(int id)
{
  return list_del(&gListCmdHead,id);
}

/**********************************************************************
删除链表中所有的节点
返回删除的节点个数
**********************************************************************/
int CdMod_DeleteAllCmd(void)
{
	int count = 0;
	struct list *node = gListCmdHead;
	while(node)
	{
		count++;
		list_del(&gListCmdHead,node->id);
		node = node->next;
	}
	return count;
}






