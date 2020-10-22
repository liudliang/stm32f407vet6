/**
  ******************************************************************************
  * @file   Task485Module.c
  * @author  zqj
  * @version v1.0
  * @date    2016-02-24
  * @brief   
  ******************************************************************************
	*/
#include "uart.h"
#include "An485Proto.h"
#include "CdModel.h"
#include "Common.h"
#include "List.h"
#include "message.h"
#include "Task485Module.h"
#include "ChgData.h"


extern uint32 UartHandler[];

static uint32 gUartCdModHandle;

extern uint8 gSetGrpFg,m_gModnum;
extern uint8 m_gModAddr,m_gModCmdListIndex;

/* 初始化串口 */
static void Tsk485Module_InitCom(void)
{
	int32 tmp;
	/* 打开串口 */
	UartHandler[MOD_COM] = Uart_Open(MOD_COM);
	gUartCdModHandle = UartHandler[MOD_COM];
	
	/*设置波特率*/
 	//tmp  = param->dcmetbaud;  //METER_BAUD;
	tmp = 9600;
	
	Uart_IOCtrl(UartHandler[MOD_COM], SIO_BAUD_SET, &tmp);	

	/*设置校验位*/
	tmp  = USART_Parity_Odd;//USART_Parity_Even;
	Uart_IOCtrl(UartHandler[MOD_COM], SIO_PARITY_BIT_SET, &tmp);

	/*设置停止位*/
	tmp  = USART_StopBits_1;
	Uart_IOCtrl(UartHandler[MOD_COM], SIO_STOP_BIT_SET, &tmp);	
	
	/*设置数据为*/
	tmp = USART_WordLength_9b;
	Uart_IOCtrl(UartHandler[MOD_COM], SIO_DATA_BIT_SET, &tmp);	
	
}

//static  COMM_MODLIST gModCommList;
//void Tsk485Module_InitData(void)
//{
//	  uint8 i;
//	  PARAM_OPER_TYPE *ptrRunParam = ChgData_GetRunParamPtr();
//	  CDMOD_DATA *ptrModData = NULL;
//	  ptrModData = CdModData_GetDataPtr(0);
//	  memset(ptrModData,0,sizeof(CDMOD_DATA)*CD_MAX_NUM);
//	  for( i = 0 ; i < CD_MAX_NUM; i++ ) {
//			ptrModData[i].commErr = 1;  /*上电默认通讯故障*/
//			ptrModData[i].addr = 0;
//			ptrModData[i].alarmInfo.bits.ModStu = 1; /*模块关机*/
//			gModCommList.AddrList[i] = i+1;
//			
//		}
//		gModCommList.vaild = 0;
//		gModCommList.num = 0;
//		
//}

///*检测可通讯的模块是否与配置一致，以此判断是否有通讯故障*/
//uint8 Tsk485Module_CheckCommErr(void)
//{
//	 uint8 i,cnt;
//	 CDMOD_DATA * ptrModData = CdModData_GetDataPtr(0);
//	 PARAM_OPER_TYPE *ptrRunParam = ChgData_GetRunParamPtr();
//	 cnt = 0;
//	 for( i = 0 ; i < CD_MAX_NUM; i++ ) {
//		 if( 0 ==  ptrModData[i].commErr ) {
//			   ptrModData[i].vaild = 1;
//				 cnt++;
//		 }else {
//			 ptrModData[i].vaild = 0;
//		 }
//	 }
//	 
//	 if( cnt == ptrRunParam->modnum ) {
//		 gModCommList.num = cnt;
//		 gModCommList.vaild = 1;
//		 cnt = 0;
//	 }else {
//		 gModCommList.vaild = 0;
//		 cnt = 1;
//	 }
//	 return cnt;
//}

//uint8 Tsk485Module_CheckOnOffStu( uint8 stu)
//{
//	 uint8 i,cnt;
//	 CDMOD_DATA * ptrModData = CdModData_GetDataPtr(0);
//	 PARAM_OPER_TYPE *ptrRunParam = ChgData_GetRunParamPtr();
//	 cnt = 0;
//	 for( i = 0 ; i < CD_MAX_NUM; i++ ) {
//		 if(ptrModData[i].vaild == 1 ) {
//			 if( 0 ==  ptrModData[i].alarmInfo.bits.ModWork == stu ) {
//						cnt++;
//			 }
//	  }
//	 }
//	 
//	 if( cnt == ptrRunParam->modnum ) {
//			return TRUE;
//	 }
//	 return FALSE;
//}


//uint8 Tsk485Module_GetModCommErr(void)
//{
//	return (gModCommList.vaild == 1 ? 0 : 1);
//}


//uint8 Tsk485Module_DelAllCmd(void)
//{
//	 struct list *node = CdMod_GetHead();
//	 while(node)
//	 {
//		  CdMod_deleteCmd(node->id);
//		  node = node->next;   
//	 }
//	return 0;
//}




#define MAX_MODBUF_SIZE 64
const uint8 conRCmdTypeList[] = {EMODST,EVOUT,ECURR,EILMT,EGRPADRESS};

#define CMDTYPE_LIST_NUM (sizeof(conRCmdTypeList))


static uint8 ReadCdModData(uint8 adr,uint8 cmdtypeindex)
{

	 uint8 rlen,tmplen;
	 uint8 cnt = 0;
	 uint8 len = 0;
	 uint8 *ptr = NULL;
	 FRAME_DATA frmdata;
	 CDMOD_DATA *ptrModData = NULL;
	
	 memset(&frmdata,0,sizeof(FRAME_DATA));
	 frmdata.info.bits.grpadr = 1;
	 frmdata.info.bits.msgtype = EREAD_DATA;
	 frmdata.cmdtype = conRCmdTypeList[cmdtypeindex%CMDTYPE_LIST_NUM];
	 
   uint8 buf[MAX_MODBUF_SIZE] = {0};
   memset(buf,0,sizeof(buf));

   len = AnProto_SendDataPkg(adr,DEVTYPE_CODE,&frmdata,buf);
   if(MIN_PKG_LEN > len ) {
		  return 0;
	 }
   Uart_ClearReadBuf(gUartCdModHandle);
   if( Uart_Write(gUartCdModHandle, buf, len) != len ) {
		  return 0;
	 }
	 Delay10Ms(4);
	 
	 cnt = 0;
	 len = 0;
	 ptr = &buf[0];
	 tmplen = 0;
   do {
		 rlen = Uart_Read(gUartCdModHandle,ptr,MAX_MODBUF_SIZE-len);
		 if( rlen > 0 ) {
				ptr += rlen;
				len += rlen;
				if( MAX_MODBUF_SIZE <= len ) {
					 break;
				}
				cnt = 0;
		  }else {
				if( ( tmplen == len )&& ( len > 3 ) ) {
					break;
				}
    		tmplen = len;
				cnt++;
				Delay10Ms(2);
		  }
	 }while( cnt < 10 );
	
	tmplen = 0;
	if( len > MIN_PKG_LEN) {
		  tmplen = AnProto_ParseAckData(buf,len,adr);
			ptrModData = CdModData_GetDataPtr(adr-1);
		  if( NULL == ptrModData ) {
						return 0;
			}
			if( tmplen > 0 ) {
				ptrModData->lastticks = GetSystemTick();
				ptrModData->commErr = 0;
				ptrModData->vaild = 1;
			}
  }else {
			 ptrModData = CdModData_GetDataPtr(adr-1);
			 if( NULL == ptrModData ) {
						return 0;
			}
		  if( GetSystemTick() - ptrModData->lastticks > 6*TIM_1S) {
				ptrModData->commErr = 1;
				ptrModData->vaild = 0;
			}
	}
	
	return tmplen;
}



///*模块设置命令放入列表*/
//int Tsk485Mod_InsertModCmdInList(uint8 adr,uint8 grpNo,ECMD_TYPE cmd,uint32 value)
//{
//	 CMD_LIST cmdl;
//	 int err,cnt;
//	 
//	 err = -1;
//	 memset(&cmdl,0,sizeof(CMD_LIST));
//   cmdl.dstAddr = adr;
//   cmdl.cmd  = ESET_DATA;
//	 cmdl.vtype = cmd;   
//	 cmdl.value = value;
//	 cmdl.groupAddress = grpNo;
//	 cnt = 0;
//   err = CdMod_InsertOneCmd(&cmdl);
//	 while (err == -1){
//		  Delay10Ms(2);
//		  err = CdMod_InsertOneCmd(&cmdl);
//		  if( err != -1 ) {
//				break;
//			}
//			if( cnt++ > 10 ) {
//				err = -1;
//				break;
//			}
//	 }
//	 return err;
//}	

void Tsk485Mod_SetGroup(uint16 addr,uint8 grpNo)
{
	uint8 buf[30];
	uint8 len;
	FRAME_DATA frmData ;
	memset(&frmData,0,sizeof(FRAME_DATA));
	frmData.info.bits.grpadr = 0;
	frmData.info.bits.msgtype = ESET_DATA;
	frmData.cmdtype = EGRPADRESS ;
	frmData.cmddata[0] = grpNo;
	len = AnProto_SendDataPkg(addr,DEVTYPE_CODE,&frmData,buf);
	Uart_Write(gUartCdModHandle, buf, len);
	Delay10Ms(5);
}

uint8 Tsk485Mod_ExecuCmd()
{
	 uint8 len;
	
	 uint8 buf[MAX_MODBUF_SIZE];
   FRAME_DATA frmData ;
	 CMD_LIST * tempCmd;
	 struct list *node = CdMod_GetHead();
	 while(node)
	 {
		 tempCmd = (CMD_LIST *)node->cmdst;
		 memset(&frmData,0,sizeof(FRAME_DATA));
		 frmData.info.bits.grpadr = tempCmd->groupAddress;
		 frmData.info.bits.msgtype = tempCmd->cmd;
		 frmData.cmdtype = tempCmd->vtype;         /*4 模块关机*/
		 memcpy(frmData.cmddata,&tempCmd->value,4);
		 memset(buf,0,sizeof(buf));

		 len = AnProto_SendDataPkg(tempCmd->dstAddr,DEVTYPE_CODE,&frmData,buf);
		 Uart_ClearReadBuf(gUartCdModHandle);
		 Uart_Write(gUartCdModHandle, buf, len);
		 Delay10Ms(10);
		 tempCmd->exec++;
		 if((tempCmd->exec > CMD_SEND_TIMES) ) { //|| (1 ==  tempCmd->succ)) {
			  tempCmd->succ = 1;
			  CdMod_deleteCmd(node->id);
			  node = node->next;          /*连续发送3次后，执行下一条*/
		 } 
	 }
	 return 0;
}



extern COMM_MODLIST gModCommList;
static const uint8 conModAdr[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	
#ifdef STM32F10X_HD
void Task_CdModule(void *p_arg)
{
	
	  MSG_STRUCT msg;
    
	  Tsk485Module_InitCom();
	  CdModData_Init();
	  m_gModAddr = 0;
	  m_gModCmdListIndex = 0;
	  Message_QueueCreat(APP_TASK_DLMOD_PRIO);
	  DEV_LOGIC_TYPE *ptrLogData = ChgData_GetLogicDataPtr();
	  PARAM_OPER_TYPE *ptrRunParam = ChgData_GetRunParamPtr();
	
	  gSetGrpFg = 0;
	  m_gModnum = ptrRunParam->modnum;
    while(1)
		{			 
			 if(RcvMsgFromQueue(&msg) == TRUE)
			 {
					CdModData_MsgDeal(&msg);
			 }
			 
			  /*先执行控制命令*/
				Tsk485Mod_ExecuCmd();
	
			  if((1 == gSetGrpFg) && (ptrLogData->chgtimes < 5) ){
					m_gModnum =  ptrRunParam->modnum;
				  Tsk485Mod_SetGroup(conModAdr[m_gModAddr],1);	
				  Delay10Ms(5);
			  }	
				
				/*轮询采集数据*/
			  ReadCdModData(gModCommList.AddrList[m_gModAddr],m_gModCmdListIndex);
	      
				m_gModAddr++;
			  if( m_gModAddr == m_gModnum ) {
					
					m_gModAddr = 0;
					m_gModCmdListIndex++;
					m_gModCmdListIndex %= CMDTYPE_LIST_NUM;
					
					if ( 1 == gModCommList.vaild ) {
						m_gModnum = gModCommList.num;
					}
				}
				
				/*轮询采集数据*/
			  ReadCdModData(gModCommList.AddrList[m_gModnum+1],m_gModCmdListIndex);
				
				CdModData_CheckCommErr();
				
				Delay10Ms(5);			
    }
}

#endif
