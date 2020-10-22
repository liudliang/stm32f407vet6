/****************************************************************************************
Filename: Adebug.c
Author: patli
Datetime: 20190930
Version: v0.01
****************************************************************************************/
#include "typedefs.h"
//#include "malloc.h"

#include "delay.h"

#include "main.h"
#include "uart.h"
#include "rtc.h"

#include "lwip/opt.h"
#include "lwip_comm.h"
#include "lwip_sys.h"
#include "lwip/api.h"
#include "lwip/tcp.h"

#include "Adebug.h"

#include "ChgData.h"
#include "flash_if_for_upgrade.h"
#include "stm32f4x7_eth_bsp.h"


#ifdef AUNICE_DEBUG

uint8 m_debug_str[][5] =
{
	{"MOD"},
	{"BMS"},	
	{"CON"},
	{"SCR"},
	{"BAC"},
	{"TCP"},
	{"FTP"},
	{"USB"},
	{"CHK"},
	{"UNK"},		

};

OS_EVENT *msg_smp;

uint32 debug_index = 0;
uint8 debug_buf[DEBUG_MAX_BUF_LEN];
uint8 mac_addr[6] = {0};
uint8 charge_id[16] ={0};
uint8 client_id = 0;
uint8 head_buf[200];

uint16 task_run_status = 0;  //各任务运行标志
uint16 uart_status = 0;  //all uart send and recv status,  high byte is recv status, low byte is send status
uint8  can_status = 0;   //all can send and recv status, high 4 bits is recv, low 4bits is send status
uint8  gun_status = 0;   //all gun status, high 4bits is B gun 4bit--cc1 5bit---locked, low 4bits is A gun 0bit---cc1 1bit-----locked;
uint8  msg_full = 0;	//message queue is full, 0---no full; 1---full
uint8 SendLog = LOG_OFF;			//net send log flag, 0--not send; 1---send
MOD_LOG_BIT_t mod_log_flag; 		//每一位对应MOD 日志开关，LOG_ON --开，LOG_OFF--关，一共32种日志
	
extern DEV_ISO_TYPE gIsoData[DEF_MAX_GUN_NO];

#ifdef NET_DEBUG
//#define SCOMM_DEBUG_NET

#if (PRODUCTS_LEVEL == DEBUG_VERSION)  
//uint8 debug_ip[4] = {192, 168, 3, 29};
//#define REMOTE_DEBUG_PORT				    8099

uint8 debug_ip[4] = {119, 23, 161, 46};
#define REMOTE_DEBUG_PORT				    30000

#else
uint8 debug_ip[4] = {119, 23, 161, 46};
#define REMOTE_DEBUG_PORT				    30000 	// 60080

#endif
uint8 disable_dev = DISABLE_OFF; 	//DEVICE DISABLE FALG

uint8 restart_dev = RESTART_OFF; 	//DEVICE DISABLE FALG

uint8 MainVersion = 0;   			//主版本数
uint8 SubVersion = 0;				//次版本数
uint16 d_re_connect_num = 0;  		//重连次数

#define DEBUG_MAX_RX_BUF_LEN				    256
#define CMD_NULL          						0xff

#if 0  //patli 20200216
OS_STK NET_DEBUG_TASK_STK[NET_DEBUG_STK_SIZE];
#endif

uint32 NET_DEBUG_TXBUF[DEBUG_MAX_BUF_LEN/sizeof(uint32)+1];
uint32 NET_DEBUG_RXBUF[DEBUG_MAX_RX_BUF_LEN/sizeof(uint32)+1];

TCP_CLIENT_CST gNetDebugCtrl;

struct netconn *net_debug_conn;

OS_EVENT *d_net_smp;
OS_EVENT *d_net_tx_smp;

extern uint8_t RJ45LineLinkStatus(void);


#define VarDataStartAddr (2+2+1+16)   //变化数据起始位------2字节帧头，2字节帧长度，1字节命令字，16字节DEV ID

NetDebugProtoFrame_t m_debug_frame;  //协议数据
uint8 tran_data[256];				//传输数据


void ClearDebugStatus()
{
	//clear check dev status
	task_run_status = 0;
	uart_status = 0;  
	can_status = 0;   
	gun_status = 0; 
	msg_full = 0;
}

uint8 IsNetDebugConnect()
{
	return gNetDebugCtrl.client_flag == TCP_CONNECT_ON;
}


void SendMsgToNetDebug(uint8 Type,uint8 Len,void *Data)
{
	if(gNetDebugCtrl.client_flag == TCP_CONNECT_ON)
		SendMsgWithNByte(Type,Len,Data,NET_DEBUG_TASK_PRIO); 
		//SendMsgWithNByte(Type,Len,Data,TCPCLIENT_PRIO); 
	

}

void SendMsgToNetDebugByPtr(uint8 Type,uint8 Len,void *Data)
{
	if(gNetDebugCtrl.client_flag == TCP_CONNECT_ON)
		SendMsgWithNByteByPtr(Type,Len,Data,NET_DEBUG_TASK_PRIO); 
		//SendMsgWithNByteByPtr(Type,Len,Data,TCPCLIENT_PRIO); 

}



int32 NetDebug_Read(uint8 *buf,uint16 len)
{
	if(len <=gNetDebugCtrl.bufsize - gNetDebugCtrl.rxhead)
	{
		memcpy(buf, &gNetDebugCtrl.rxbuf[gNetDebugCtrl.rxhead],len);
		gNetDebugCtrl.rxhead += len;
	}else{

		memcpy(buf, &gNetDebugCtrl.rxbuf[gNetDebugCtrl.rxhead],gNetDebugCtrl.bufsize - gNetDebugCtrl.rxhead);
		gNetDebugCtrl.rxhead = gNetDebugCtrl.bufsize;
		return gNetDebugCtrl.bufsize - gNetDebugCtrl.rxhead;
 	
		}
 
	return len;
}

/***********************************************************************
** 函数名称: NetDebug_PackgCheck()
** 功能描述：校验收后台帧的合法性
** 参数:     
**返回值：   
*************************************************************************/
uint8  NetDebug_PackgCheck(uint8 *buf,uint16 len)
{
	uint32 temp, temp1;
	 
	 
	if( buf[len - 6] != 0x68) {
		return 2;//结束标识符
	}	 
	 
	 if (0 != strcmp((const char *)&buf[5], (const char *)&charge_id[0])) //正平隆地是字符串的桩号
	 {
		 return 4;	 //桩地址
	 }
  
  	temp = Common_CalclongSum(buf , (len - 5));
	temp1 = Common_LittleToInt(&buf[len - 5]);
	if( temp != temp1) {
		return 5;	//校验
	}
	 
	return 0;
}

/****************************
 获取后台下发数据
 ****************************/
uint8  RcvUploadDataDeal(uint8 *CtrData)
{
	uint8 cnt,stat,rtn;
	uint16 readlen,num,dlen;	
	uint16 u16TmpLen = 0;
	uint8 *ptr = NULL;
	int32 getlen;
  cnt = 0;
	stat = 0;
	num = 0;
	dlen = 0;
	getlen = 0;
	readlen = 1;   //每次读一个数据
	rtn = 0;
	
	ptr = m_debug_frame.data;
	m_debug_frame.frame_len = 0;
	memset(m_debug_frame.data,0,sizeof(m_debug_frame.data));
	
	do {
		if(1 == cnt)
		{
			rtn = 0;
		}
		getlen = NetDebug_Read(ptr,readlen);

		if( getlen > 0 ) {
			 cnt = 0;

			 switch(stat)
			{
				 case 0:
				 {
					  if( SOH== *ptr ) {
							 ptr++ ;
							 stat = 1;
						}
						break;
				 }
				 case 1:
				 {
					   if( STX== *ptr ) {
							 ptr++;
							 stat = 2;
						}else {
							stat = 0;
							ptr = m_debug_frame.data;//BackRun.Data;
						}
						break;
				 }
				 case 2:
				 {
					   stat = 3;
					   ptr++;
             		   break;
				 }
				 case 3:
				 {
					   u16TmpLen = (m_debug_frame.data[3] << 8)+ m_debug_frame.data[2];		//(BackRun.Data[3]<<8) + BackRun.Data[2] + 2;//2个头字节
					   if((u16TmpLen > 255) ||(u16TmpLen < 10)){
							 stat = 0;
							 ptr =  m_debug_frame.data;			//BackRun.Data;
						 }
						 else{
					     readlen =  u16TmpLen - 4;				//下次读取长度
					     stat = 4;
						 dlen = 4;
					     ptr++;
						 }
             break;
				 }
				 case 4:
				 {
					  if( getlen == readlen ) {
							 ptr += getlen;
							 dlen += getlen;
							 if( dlen == u16TmpLen ) {
									rtn = 1;
									break;
							 }
							
						}else if (getlen < readlen  ){
							  readlen = readlen - getlen;
							  dlen += getlen;
							  ptr += getlen;
						}
					 break; 
				 }
				 default:
					 break;
			}
		}
		else {
			if(0 == stat){
				 return FALSE;
			}
			Delay5Ms(2);
			cnt++; 
		}
		
		if( 1 == rtn ) {
			 break;
		}
  }while((cnt < 5 ) && (ptr <  &m_debug_frame.data[128]));	//&BackRun.Data[120])); /*接收大于120字节退出*/
	
	
  	num = (uint8)(ptr - m_debug_frame.data);  //&BackRun.Data[0]);
	
	if( (0 == rtn ) || num < 10 ) {
		   return FALSE;
	  }

	/*校验报文*/
	if(( NetDebug_PackgCheck(m_debug_frame.data, u16TmpLen)) > 0 ) {
		 memset(m_debug_frame.data,0,sizeof(m_debug_frame.data));
		 return FALSE;
	}
	
	ptr = m_debug_frame.data;

	
	CtrData[0] = m_debug_frame.data[4];        //cmd
	
	m_debug_frame.frame_len = u16TmpLen+10;

	
//	DebugInfoWithPbuf(BAC_MOD, "NetDebug RcvUploadData", m_debug_frame.data, m_debug_frame.frame_len);
	
	return TRUE;
}


static void NetDebug_FrameSend(uint8 cmd,uint16 len)
{
	uint32  CalclongSum;
	uint16 index = 0;
	
	memset(tran_data, 0, sizeof(tran_data));

	tran_data[index++] = SOH;
	tran_data[index++] = STX;

	//frame len = head(2) + len(2)+cmd(1) +end(2) + csum(4) + data
	Common_LittleShortTo(&tran_data[index], len + 11);  //发送帧长度参数
	index += 2;

	tran_data[index++] = cmd;
	
	memcpy(&tran_data[index], m_debug_frame.data, len);
	index += len;

	//正文结束符	
	tran_data[index++] = ETX;

	//校验和：校验码前面的所有数据
	CalclongSum = Common_CalclongSum(tran_data , index);
	Common_LittleIntTo(&tran_data[index],CalclongSum);  
	index += sizeof(CalclongSum);	

	//传输结束符	
	tran_data[index++] = EOT;	
	
	//发送数据函数
	Net_Debug_Write(tran_data, index, 0);
	
//	DebugRecvCmd(CtrData[0],BackRun.Data, u16TmpLen);
//	DebugInfoWithPbuf(BAC_MOD, "NetDebug_FrameSend", tran_data, index);


}
/*************************
心跳
**************************/

static void NetDebug_LifeAuto(uint8 dir)
{	

	uint16 index = 0;
	uint32 upgrade_flag = 0;
	uint8  Reason = 0;
	uint8  errBuf[MAX_ERRBUF_SIZE] = {0};

	if(dir == SEND_DIR)
	{
		//clear m_debug_frame to 0
		memset(m_debug_frame.data, 0, sizeof(m_debug_frame.data));

		//copy charge id to m_debug_frame
		memcpy(&m_debug_frame.data[index], charge_id, sizeof(charge_id));
		index += sizeof(charge_id);
//		printf("charge_id1 = %s\r\n", charge_id);

		//copy mac address to m_debug_frame
		memcpy(&m_debug_frame.data[index], mac_addr, sizeof(mac_addr));
		index += sizeof(mac_addr);	

		//add  version letter
		m_debug_frame.data[index++] = VERSION_LETTER;

#if (PRODUCTS_LEVEL == DEBUG_VERSION ||PRODUCTS_LEVEL == RELEASE_VERSION) 				  //调试版本

		//add main version to m_debug_frame
		m_debug_frame.data[index++] = MAIN_VERSION;

		//add sub version to m_debug_frame
		m_debug_frame.data[index++] = SUB_VERSION;

#else 

		//add main version to m_debug_frame
		m_debug_frame.data[index++] = U_MAIN_VERSION;

		//add sub version to m_debug_frame
		m_debug_frame.data[index++] = U_SUB_VERSION;

#endif	

		//设备运行了多少时间
		Common_LittleIntTo(&m_debug_frame.data[index],DevRunSec);
		index +=4;

		//网络重连次数
		Common_LittleShortTo(&m_debug_frame.data[index], d_re_connect_num);
		index +=2;

		//A枪工作状态
		m_debug_frame.data[index++] = GetWorkStep(AGUN_NO);

		//B枪工作状态
		m_debug_frame.data[index++] = GetWorkStep(BGUN_NO);

		//A枪错误码
		Check_GetErrBit(AGUN_NO,&errBuf[0],MAX_ERRBUF_SIZE,&Reason);
		m_debug_frame.data[index++] = errBuf[0];
		
		//B枪错误码
		Check_GetErrBit(BGUN_NO,&errBuf[0],MAX_ERRBUF_SIZE,&Reason);
		m_debug_frame.data[index++] = errBuf[0];
		

		NetDebug_FrameSend(DEV_TO_NETDEBUG_HB_REQ, index);

	}else if(dir == RECV_DIR){

		index = VarDataStartAddr;

		//IS DISABLE DEVICE 
		disable_dev = m_debug_frame.data[index++];

		//IS RESTART DEVICE 
		restart_dev = m_debug_frame.data[index++];
		
		//is send log
		SendLog = m_debug_frame.data[index++];		// SEND LOG FALG, 

		//MOD LOG FLAG
		mod_log_flag.mod_log.dword = Common_LittleToInt(&m_debug_frame.data[index]);
		index += 4;	
		
		MainVersion = m_debug_frame.data[index++];  //MAIN VERSION
		SubVersion = m_debug_frame.data[index++];	//SUB VERSION	

		if(restart_dev)
		{
#if (PRODUCTS_LEVEL != FACTORY_UPGRADE_VERSION)		 /*非出厂升级版本模识*/
			SoftReset();		//设备重新启动
#endif	
		
		}

		if(MainVersion > MAIN_VERSION || (MainVersion == MAIN_VERSION && SubVersion > SUB_VERSION))
		{
//			SendMsgToNetDebug(MSG_DATA_TO_NETDEBUG, strlen("USER SOFTWARE NEED UPGRADE!"), "USER SOFTWARE NEED UPGRADE!");
		
			//set software upgrade bit, and reset.
#if (PRODUCTS_LEVEL != FACTORY_UPGRADE_VERSION)		 /*非出厂升级版本模识*/
			if(!GetWorkStatus(AGUN_NO)&&!GetWorkStatus(BGUN_NO))
			{	
				//FACTORY_SETTING_BASE_ADDRESS SET USER_PROGRAM_ON
				upgrade_flag = USER_PROGRAM_ON;
				Flash_Write32BitDatasWithErase(FACTORY_SETTING_BASE_ADDRESS, 1, &upgrade_flag);
			
#if (PRODUCTS_LEVEL == RELEASE_VERSION)
				//software reset
				SoftReset();	//设备重新启动
#endif	

//				printf("USER SOFTWARE NEED UPGRADE!\n");

			}
#endif				
		}
		
	}
	
}


static const NetDebugProtoHandle_t protoHandle[] = {                 //通讯信息处理	
	  {DEV_TO_NETDEBUG_HB_REQ, NetDebug_LifeAuto},     /*心跳*/	
	  {NETDEBUG_TO_DEV_HB_RES, NetDebug_LifeAuto},	   /*心跳*/ 
	  {CMD_NULL 	 , NULL},				   /*无效*/

	  
};

/***********************************************************************
***函数名称: NetDebugfun
** 功能描述: 通过命令查找函数执行
***    参数: 
**   返回值:   
*************************************************************************/
static uint8  NetDebug_fun_Deal(uint8 cmd,uint8 dir)
{
	 uint16 i;
	 const NetDebugProtoHandle_t *FunHandle = &protoHandle[0]; 
	 
	 for( i = 0 ; FunHandle->cmd != CMD_NULL ; i++ ) {
		 if( cmd == FunHandle->cmd ) {
			 if( NULL != FunHandle->func ) {
			   FunHandle->func(dir);
				 break;
			 }
			 return  FALSE;
		 }
		 FunHandle++;
	 }
	 return TRUE;
}



void Net_Debug_Write(uint8 *buf, uint16 len, uint8 isbug)
{
	int8 t_err = 0 ,s_err = 0;
	uint16 tmp_len = 0;	  
//return 0 ;
	//请求信号量
	OSSemPend(d_net_tx_smp,0,&s_err);	//与网络任务互斥
	
	//判断是否连接成功
	if(gNetDebugCtrl.client_flag && len < DEBUG_MAX_BUF_LEN)
	{	
	
#ifdef SCOMM_DEBUG_NET	
		ScommDebugPrintStrWithPi("DEBUG TCP BUF FILLING len=", len);
		ScommDebugPrintStr((char*)buf);
#endif

#ifdef WRITE_CACHE
		if(isbug)
		{
	//		tmp_len = len >= (DEBUG_MAX_BUF_LEN - gNetDebugCtrl.txlen) ? (DEBUG_MAX_BUF_LEN - gNetDebugCtrl.txlen):len;
	//		memcpy(&gNetDebugCtrl.txbuf[gNetDebugCtrl.txlen],buf,tmp_len);
	//		gNetDebugCtrl.txlen += tmp_len;
	//		tmp_len = gNetDebugCtrl.txlen + len;
			if((gNetDebugCtrl.txlen + len) >  WRITE_CACHE_BUF_LEN)
			{
				t_err = netconn_write(net_debug_conn, gNetDebugCtrl.txbuf, gNetDebugCtrl.txlen ,NETCONN_COPY); 
				memcpy(gNetDebugCtrl.txbuf,buf,len);
				gNetDebugCtrl.txlen = len;
				
			}
			else
			{
				memcpy(&gNetDebugCtrl.txbuf[gNetDebugCtrl.txlen],buf,len);
				gNetDebugCtrl.txlen += len;
				//释放信号量
				OSSemPost(d_net_tx_smp);
				return;    //must retrun
				
			}
			
		}
		else
#endif		
		{
			t_err = netconn_write(net_debug_conn ,buf, len ,NETCONN_COPY); 
		}
#ifdef SCOMM_DEBUG_NET
			ScommDebugPrintStrWithPi("DEBUG TCP WRITE ERR=",t_err);
#endif

		if(t_err == ERR_OK)
		{
			gNetDebugCtrl.u32SndCnts=0; //clear send flag times
	//		ScommDebugPrintStr("DEBUG TCP WRITE OK");
		}else{

//			printf("Net_Debug_Write t_err =%d\r\n", t_err); 
			
			if(t_err == ERR_INPROGRESS)
			{
				SendLog = 0; //temp set log 0, stop				
				gNetDebugCtrl.txlen = 0;  //clear buf
			}
			else{				
			gNetDebugCtrl.u32SndCnts++;
			ScommDebugPrintStrWithPi("net_debug_thread WRITE FAIL", t_err);
			}
			Delay10Ms(20);
			
		}


#ifdef SCOMM_DEBUG_NET
		ScommDebugPrintStrWithPi("DEBUG TCP BUF FILLED len=", gNetDebugCtrl.txlen);
#endif
	}
	//释放信号量
	OSSemPost(d_net_tx_smp);

}


void net_debug_init()
{
  uint8 err = 0;
  
	memset(&gNetDebugCtrl,0,sizeof(TCP_CLIENT_CST));
	gNetDebugCtrl.txbuf = (uint8 *)&NET_DEBUG_TXBUF[0]; 
	gNetDebugCtrl.rxbuf = (uint8 *)&NET_DEBUG_RXBUF[0]; 

	gNetDebugCtrl.txlen = 0; 
	
	gNetDebugCtrl.bufsize = 0;
}

void net_debug_close()
{
	err_t s_err = 0;   //请求信号量

	gNetDebugCtrl.client_flag = TCP_CONNECT_OFF;

//	OSSemPend(d_net_tx_smp,0,&s_err);	//与网络写操作互斥
	
	ScommDebugPrintStr("net_debug_thread net_debug_close...");
//	printf("net_debug_thread net_debug_close...");
	
						
	netconn_close(net_debug_conn);
	Delay10Ms(1);  /**/
	netconn_delete(net_debug_conn);
	
	//释放信号量
//	OSSemPost(d_net_tx_smp);
	
	DelaySec(3);  /**/
	
	
}

/************************************ 
获取其他任务发送过来的信息或命令
************************************/
uint8 TcpDebugHandleMsg(MSG_STRUCT *msg)
{
	uint8 u8GunNo;
	uint8 buf[120];
	uint8  Reason = 0;
	uint8  AerrBuf[MAX_ERRBUF_SIZE] = {0};
	uint8  BerrBuf[MAX_ERRBUF_SIZE] = {0};
	DEV_METER_TYPE *pMet; 
	
	switch(msg->MsgType)
	{
		case MSG_DATA_TO_NETDEBUG:  /*设备发数据给TCP CLIENT, 发给后台 */
		{	
			Net_Debug_Write(msg->MsgData, msg->MsgLenth, 1);
			break;
		}
		case MSG_DATA_PTR_TO_NETDEBUG:  /*设备发数据给TCP CLIENT, 发给后台 */
		{	
			Net_Debug_Write(msg->PtrMsg, msg->MsgLenth, 1);
			break;
		}
		case MSG_DEV_TO_DEBUG_HB:  /*每5S发送心跳给NET DEBUG平台*/
		{	
			NetDebug_LifeAuto(SEND_DIR);
			break;
		}
		case MSG_TO_NETDEBUG_FTPUPGRADE_OK:  /**/
		{	
			//NetDebug_LifeAuto(SEND_DIR);
			
//			Flash_Write32BitDatasWithErase(FACTORY_SETTING_BASE_ADDRESS, 1, USER_PROGRAM_OFF);
			break;
		}
		case MSG_TO_PRINT_TASK_STATUS:  /**/
		{	
#if (PRODUCTS_LEVEL != FACTORY_UPGRADE_VERSION)
			
			Check_GetErrBit(AGUN_NO,&AerrBuf[0],MAX_ERRBUF_SIZE,&Reason);
			Check_GetErrBit(BGUN_NO,&BerrBuf[0],MAX_ERRBUF_SIZE,&Reason);

			memset(buf, 0, sizeof(buf));
			sprintf(buf, "TASKS:0x%x;GUNS:0x%x;CANS:0x%x;UARTS:0x%x;MSGF:%d;AST:%d;BST:%d;AERR:%d;BERR:%d",
				task_run_status, gun_status, can_status, uart_status, msg_full,GetWorkStep(AGUN_NO), GetWorkStep(BGUN_NO),AerrBuf[0],BerrBuf[0]);
			
			DebugInfo(CHK_MOD,buf);			

			if(GetWorkStep(AGUN_NO) != STEP_IDEL )
			{
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "A绝缘内:%dV;A绝缘外:%dV;A1正阻:%d;A1负阻:%d",gIsoData[AGUN_NO].vdc1, gIsoData[AGUN_NO].vdc3, gIsoData[AGUN_NO].res1pos, gIsoData[AGUN_NO].res1neg);
				DebugInfo(CHK_MOD,buf);
				
				memset(buf, 0, sizeof(buf));
				pMet = TskDc_GetMeterDataPtr(AGUN_NO);
				sprintf(buf, "A_DCMeter volt=%dV; A_DCMeter curr=%dA",pMet->volt, pMet->current);
				DebugInfo(CHK_MOD,buf);

			}
			
			if(GetWorkStep(BGUN_NO) != STEP_IDEL)
			{
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "B绝缘内:%dV;B绝缘外:%dV;B1正阻:%d;B1负阻:%d",gIsoData[BGUN_NO].vdc1, gIsoData[BGUN_NO].vdc3, gIsoData[BGUN_NO].res1pos, gIsoData[BGUN_NO].res1neg);
				DebugInfo(CHK_MOD,buf);
				
				memset(buf, 0, sizeof(buf));
				pMet = TskDc_GetMeterDataPtr(BGUN_NO);
				sprintf(buf, "B_DCMeter volt=%dV; B_DCMeter curr=%dA",pMet->volt, pMet->current);
				DebugInfo(CHK_MOD,buf);
			}
#endif			
			break;
		}
		
		default:
			break;
	}

	return 0;
}

uint8 net_debug_lwiptcp_Init(void)
{
	PARAM_COMM_TYPE *ptrNetPara = ChgData_GetCommParaPtr();
	uint8         lwip_init_num = 0;	
	
	net_debug_init();
	
	if(ptrNetPara->conntype != CONN_ETH)
	{
		while(lwip_comm_init(1))
		{	
			DelaySec(5);
			
		}			
			
			
#if LWIP_DHCP
		if(1 == ptrNetPara->netpara.StaPara.DhcpEn){
			lwip_comm_dhcp_creat(); //??DHCP??
		}
		else{
			lwipdev.dhcpstatus=2;	//DHCP成功
		}
#endif
	
	}

	Debug_Get_Mac();
	
	return 1;	
	
}

uint8 net_debug_try_connect()   //retrurn 1---success; 0-----fail
{
	static ip_addr_t server_ipaddr,loca_ipaddr;
	static u16_t 		 server_port,loca_port;	
	err_t err;
	
	if(gNetDebugCtrl.client_flag == TCP_CONNECT_OFF)
	{
		while( lwipdev.dhcpstatus != 2 ) {
			DelaySec(3);
		}

		d_re_connect_num++;
		
		net_debug_conn = netconn_new(NETCONN_TCP);
		if( NULL == net_debug_conn ) {
			
//			printf("net_debug_try_connect netconn_new fail\r\n");				
			DelaySec(2);
			return 0;
		}
				
		IP4_ADDR(&server_ipaddr, debug_ip[0],debug_ip[1],debug_ip[2],debug_ip[3]);
		
		ScommDebugPrintStrWithPx("net_debug_thread net_debug_conn", (int)net_debug_conn);
		ScommDebugPrintStrWithPi("net_debug_thread REMOTE_DEBUG_PORT", REMOTE_DEBUG_PORT+client_id);
				
		err = netconn_connect(net_debug_conn,&server_ipaddr,REMOTE_DEBUG_PORT+client_id);
				
		ScommDebugPrintStrWithPi("net_debug_thread netconn_connect err", err);

		if(err != ERR_OK) {
			do{
//				printf("net_debug_try_connect netconn_connect fail = %d\r\n", err); 			
				err = netconn_delete(net_debug_conn);
				ScommDebugPrintStrWithPi("net_debug_thread netconn_delete err", err);					
				DelaySec(2);
			}while(err != ERR_OK);
			
			return 0;
		}
				
		gNetDebugCtrl.client_flag = TCP_CONNECT_ON;
			
		net_debug_conn->recv_timeout = 10;
		netconn_getaddr(net_debug_conn,&loca_ipaddr,&loca_port,1); 				
				
		ScommDebugPrintStr("Wellcome to Aunice Net Debug!");
				
	}	

	return 1;
}

void net_debug_thread_mian_proc()   
{
#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
		OS_CPU_SR  cpu_sr = 0;
#endif
	u32 u32data_len = 0;
	err_t err,recv_err;
	MSG_STRUCT msg;
	uint8 CtrData[2];
	struct pbuf *q;
	struct netbuf *recvbuf;

#if 0	
	if (RcvMsgFromQueue(&msg) == TRUE)
	{
		TcpDebugHandleMsg(&msg);
	}
#endif	

	//请求信号量
	OSSemPend(d_net_tx_smp,0,&err);
	
	/*接收处理*/
	if((recv_err = netconn_recv(net_debug_conn,&recvbuf)) == ERR_OK)
	{					
		gNetDebugCtrl.u32SndCnts=0; //clear send flag times
	
		u32data_len=0; 
	
#if 1											
//		OS_ENTER_CRITICAL(); 
		for(q=recvbuf->p;q!=NULL;q=q->next) 
		{
			if(u32data_len > DEBUG_MAX_RX_BUF_LEN){
				break; 
			}
						
			memcpy(&gNetDebugCtrl.rxbuf[u32data_len],q->payload,q->len);							
			u32data_len += q->len;		
		}
//		OS_EXIT_CRITICAL(); 
	
		gNetDebugCtrl.bufsize = u32data_len;						
		gNetDebugCtrl.rxhead= 0;
	
							
		if (RcvUploadDataDeal(CtrData) == TRUE) 
		{
			NetDebug_fun_Deal(CtrData[0],RECV_DIR); 							//处理协议信息					
							
		}
#endif
						
		netbuf_delete(recvbuf);
							
	}
	else if(recv_err == (err_t)ERR_CLSD||recv_err == (err_t)ERR_BUF){
		ScommDebugPrintStr("net_debug_thread connect closed, and reconnect...");
		gNetDebugCtrl.u32SndCnts = 0;
//		printf("netconn_recv recv_err =%d\r\n", recv_err);				
							
		net_debug_close();
					
		DelaySec(1);
	}	
						
	//释放信号量
	OSSemPost(d_net_tx_smp);
						
	if( gNetDebugCtrl.u32SndCnts > 10 ) { /* 10大约1分钟，30为3分钟 */
		ScommDebugPrintStrWithPi("net_debug_thread send max num fail,and reconnect...",gNetDebugCtrl.u32SndCnts);
		gNetDebugCtrl.u32SndCnts = 0;
		net_debug_close();
	}	

}

static void net_debug_thread(void *arg)
{
	u32 u32data_len = 0;
	struct pbuf *q;
	err_t err,recv_err;
	MSG_STRUCT msg;
	uint8 CtrData[2];
	struct netbuf *recvbuf;

	net_debug_lwiptcp_Init();
	
	Message_QueueCreat(NET_DEBUG_TASK_PRIO); 		

	while(1)
	{			
		TaskRunTimePrint("net_debug_thread begin", OSPrioCur);
		
		if (ETH_LINK_FLAG == RJ45LineLinkStatus())
		{

			if(net_debug_try_connect()) 
			{
				net_debug_thread_mian_proc();   
			}

			Delay10Ms(50);   //500ms
			
		}else{
		
			DelaySec(5);  //延时一秒后再去读网线状态
		}
		
		TaskRunTimePrint("net_debug_thread end", OSPrioCur);
	
	}
}
#endif


uint8 debug_comm = BACK_COM; //  BACK_COM;    // BACK_COM CARD_COM;
uint32 debug_comm_baud = 115200;				//256000;    //115200; //460800  //921600;
uint8 debug_comm_buf[256];


/* 初始化调试串口 */
void Init_DebugCom(void)
{
	uint32 tmp;
	
	if(debug_comm > 10) return;
	
	/* 获取串口 */
	UartHandler[debug_comm] = Uart_Open(debug_comm);
	
	/* 波特率 */
 	tmp  =debug_comm_baud;     //256000;    //115200; //460800  //921600;
	Uart_IOCtrl(UartHandler[debug_comm], SIO_BAUD_SET, &tmp);	

	/* 无校验 */
	tmp  = USART_Parity_No;
	Uart_IOCtrl(UartHandler[debug_comm], SIO_PARITY_BIT_SET, &tmp);

	/* 停止位 */
	tmp  = USART_StopBits_1;
	Uart_IOCtrl(UartHandler[debug_comm], SIO_STOP_BIT_SET, &tmp);	
	
	/* 9个数据位 */
	tmp = USART_WordLength_8b;
	Uart_IOCtrl(UartHandler[debug_comm], SIO_DATA_BIT_SET, &tmp);
	

	if(debug_comm == COM1)
		USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	
	if(debug_comm == COM5)
		USART_ITConfig(UART5, USART_IT_TXE, DISABLE);			

	
}

void An_Print(u8 *s)
{	
	uint8 err;	

	if(debug_comm > 10 || GetConnectType() != CONN_ETH) return;
	
	STM32F10x_UART_CHAN* pChan = (STM32F10x_UART_CHAN*)UartHandler[debug_comm];
	
//	OSSemPend(pChan->uartLock, 500, &err);
		
	while(*s!='\0')
	{ 		
		
		if(pChan->devicType == RS485TYPE ) {
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_SET);	//set ctr pin
		}
		
		USART_SendData(pChan->base,*s);
		s++;
		
		while(USART_GetFlagStatus(pChan->base,USART_FLAG_TC )==RESET){			
			if(pChan->devicType == RS485TYPE )
				GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_RESET);  //clr ctr pin
		};
	}
	
//	OSSemPost(pChan->uartLock);

	
}


#ifdef SCOMM_DEBUG   //串口通信调试接口
OS_EVENT *d_comm_smp;

#pragma import(__use_no_semihosting) 
struct __FILE
{
	int handle;
};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x) 
{
	x = x;
}

int fputc(int ch,FILE *f)
{
	uint8 err;	

	if(debug_comm > 10) return 0;
	
	STM32F10x_UART_CHAN* pChan = (STM32F10x_UART_CHAN*)UartHandler[debug_comm];
	
	OSSemPend(pChan->uartLock, 500, &err);
	
	if(pChan->devicType == RS485TYPE ) {
		GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_SET);	//set ctr pin
	}
	
	USART_SendData(pChan->base,(unsigned char)ch);
	
	while(USART_GetFlagStatus(pChan->base,USART_FLAG_TC )==RESET){	
		if(pChan->devicType == RS485TYPE )
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_RESET);  //clr ctr pin
		};
	
	OSSemPost(pChan->uartLock);
	
	return(ch);
}

uint16 debug_comm_write_len = 0;
void Send_data(int32 devDesc, u8 *s)
{	
#ifndef TASK_RUN_TIME_PRINTF				
	uint8 err;	
	uint8 buf_len = strlen(s);
	
	if(devDesc > 10) return;
	
	STM32F10x_UART_CHAN* pChan = (STM32F10x_UART_CHAN*)UartHandler[devDesc];
	
	//请求互斥量
//	OSMutexPend(d_comm_smp,0,&err); 
	
//	OSSemPend(pChan->uartLock, 500, &err);
#ifdef WRITE_CACHE
	if((debug_comm_write_len + buf_len) >=  WRITE_CACHE_BUF_LEN)
	{
		Print_data(devDesc, pChan->SendBuf, debug_comm_write_len);
		memcpy(pChan->SendBuf,s,buf_len);
		debug_comm_write_len = buf_len;
		
	}
	else
	{
		memcpy(&pChan->SendBuf[debug_comm_write_len],s,buf_len);
		debug_comm_write_len += buf_len;
		
	}

#else		
	while(*s!='\0')	
	{ 		
		
		if(pChan->devicType == RS485TYPE ) {
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_SET);	//set ctr pin
		}
		
		USART_SendData(pChan->base,*s);
		s++;
		
		while(USART_GetFlagStatus(pChan->base,USART_FLAG_TC )==RESET){			
			if(pChan->devicType == RS485TYPE )
				GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_RESET);  //clr ctr pin
		};
	}
#endif	

//	OSSemPost(pChan->uartLock);
#endif	
	//释放互斥量
//	OSMutexPost(d_comm_smp);
	
}

void Print_data(int32 devDesc, u8 *s, uint16 len)
{	
	uint8 err;	

	if(devDesc > 10) return;
	
	STM32F10x_UART_CHAN* pChan = (STM32F10x_UART_CHAN*)UartHandler[devDesc];
	
	//请求互斥量
	
//	OSSemPend(pChan->uartLock, 500, &err);
		
	while(len>0)
	{ 		
		
		if(pChan->devicType == RS485TYPE ) {
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_SET);	//set ctr pin
		}
		
		USART_SendData(pChan->base,*s);
		s++;
		len--;
		
		while(USART_GetFlagStatus(pChan->base,USART_FLAG_TC )==RESET){			
			if(pChan->devicType == RS485TYPE )
				GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_RESET);  //clr ctr pin
		};
	}

	//释放互斥量
//	OSSemPost(pChan->uartLock);
	
}



void ScommDebugPrintStr(char* buf)
{
	TM_RTC_Time_t systime= Time_GetRtcTime();
    uint8 err= 0;
	
	//请求信号量
	OSSemPend(d_comm_smp,0,&err); 

	memset(debug_comm_buf, 0, sizeof(debug_comm_buf));
		
	sprintf((char*)debug_comm_buf, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d %s\r\n", systime.year,systime.month,systime.date,
	systime.hours,systime.minutes,systime.seconds,systime.milliseconds,buf);
	
	Send_data(debug_comm, debug_comm_buf);

	//释放信号量
	OSSemPost(d_comm_smp); 
	
}

void ScommDebugPrintStrWithPx(char* buf, int32 param)
{
	TM_RTC_Time_t systime= Time_GetRtcTime();	
    uint8 err= 0;
	
	//请求信号量
	OSSemPend(d_comm_smp,0,&err); 

	memset(debug_comm_buf, 0, sizeof(debug_comm_buf));
		
	sprintf((char*)debug_comm_buf, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d %s=0x%x\r\n", systime.year,systime.month,systime.date,
	systime.hours,systime.minutes,systime.seconds,systime.milliseconds,buf, param);
	
	Send_data(debug_comm, debug_comm_buf);
	
	//释放信号量
	OSSemPost(d_comm_smp); 
	
}

void ScommDebugPrintStrWithPi(char* buf, int32 param)
{
	TM_RTC_Time_t systime= Time_GetRtcTime();
    uint8 err= 0;
	
	//请求信号量
	OSSemPend(d_comm_smp,0,&err); 

	memset(debug_comm_buf, 0, sizeof(debug_comm_buf));
		
	sprintf((char*)debug_comm_buf, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d %s=%d\r\n", systime.year,systime.month,systime.date,
	systime.hours,systime.minutes,systime.seconds,systime.milliseconds,buf, param);	

	Send_data(debug_comm, debug_comm_buf);
	
	//释放信号量
	OSSemPost(d_comm_smp); 


}

void ScommDebugPrintStrWithPf(char* buf, float param)
{
	TM_RTC_Time_t systime= Time_GetRtcTime();
    uint8 err= 0;
	
	//请求信号量
	OSSemPend(d_comm_smp,0,&err); 

	memset(debug_comm_buf, 0, sizeof(debug_comm_buf));
		
	sprintf((char*)debug_comm_buf, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d %s=%f\r\n", systime.year,systime.month,systime.date,
	systime.hours,systime.minutes,systime.seconds,systime.milliseconds,buf, param);

	Send_data(debug_comm, debug_comm_buf);
	
	//释放信号量
	OSSemPost(d_comm_smp); 
	
}


void ScommDebugPrintStrWithLen(char* buf, uint16 len)
{		
#if 0	
	res_len = Uart_Write_log(UartHandler[debug_comm],(uint8 *)buf,strlen((char*)buf));
#else
		Send_data(debug_comm, (u8*)buf);
#endif
	
}


#else

#if 1   //(PRODUCTS_LEVEL == DEBUG_VERSION)

#pragma import(__use_no_semihosting) 
struct __FILE
{
	int handle;
};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x) 
{
	x = x;
}

int fputc(int ch,FILE *f)
{
	uint8 err;	
#if OS_CRITICAL_METHOD == 3
	OS_CPU_SR  cpu_sr;
#endif  

	if(debug_comm > 10|| GetConnectType() != CONN_ETH) return 0;
	
	STM32F10x_UART_CHAN* pChan = (STM32F10x_UART_CHAN*)UartHandler[debug_comm];
	
//	OSSemPend(pChan->uartLock, 500, &err);
	OS_ENTER_CRITICAL();	
	
	if(pChan->devicType == RS485TYPE ) {
		GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_SET);	//set ctr pin
	}
	
	USART_SendData(pChan->base,(unsigned char)ch);
	
	while(USART_GetFlagStatus(pChan->base,USART_FLAG_TC )==RESET){	
		if(pChan->devicType == RS485TYPE )
			GPIO_WriteBit(pChan->directionGPIO,pChan->rs485ContrlPin,Bit_RESET);  //clr ctr pin
		};

	OS_EXIT_CRITICAL();
	
	
//	OSSemPost(pChan->uartLock);
	
	return(ch);
}


#endif

int32 ScommDebugPrintStr(char* buf)
{
	
	return 0; //res_len;
}

int32 ScommDebugPrintStrWithPx(char* buf, int32 param)
{
	
	return 0; // res_len;
}

int32 ScommDebugPrintStrWithPi(char* buf, int32 param)
{
	return 0; 

}

int32 ScommDebugPrintStrWithPf(char* buf, float param)
{	
	return 0; //res_len;
}


int32 ScommDebugPrintStrWithLen(char* buf, uint16 len)
{			
	
	return 0; //res_len;
}

#endif

#ifdef TASK_RUN_TIME_PRINTF				
void TaskRunTimePrint(char* buf, int32 param)
{
	TM_RTC_Time_t systime= Time_GetRtcTime();
    uint8 err= 0;
	
#ifdef SCOMM_DEBUG   //串口通信调试接口
	//请求信号量
	OSSemPend(d_comm_smp,0,&err); 

	memset(debug_comm_buf, 0, sizeof(debug_comm_buf));
	sprintf(debug_comm_buf,"%.2d:%.2d:%.2d.%.3d %s=%d; flag=%x\r\n", systime.hours,systime.minutes,systime.seconds,systime.milliseconds,buf, param, task_run_status);
		
	Print_data(debug_comm, debug_comm_buf, strlen(debug_comm_buf));	
	
//释放信号量
OSSemPost(d_comm_smp); 
#endif
	
#ifdef NET_DEBUG  //20191008
	setbit(task_run_status, param);
#endif

	

}
#else
void TaskRunTimePrint(char* buf, int32 param)
{
	
#ifdef NET_DEBUG  //20191008
	setbit(task_run_status, param);
#endif

}

#endif

uint8 IsModlLogOn(uint8 mod)
{
	uint32 log = mod_log_flag.mod_log.dword >> mod;

	return log&0x00000001;
}


/*充电桩mac 地址*/
void Debug_Get_Mac()
{
#if (PRODUCTS_LEVEL != FACTORY_UPGRADE_VERSION)
  	PARAM_COMM_TYPE *ptrNetPara = ChgData_GetCommParaPtr();

	//获取MAC地址
	memset(mac_addr, 0, sizeof(mac_addr));
  	memcpy(mac_addr, ptrNetPara->netpara.MacAdress, sizeof(ptrNetPara->netpara.MacAdress));

	
//	printf("mac_addr = %x\r\n", mac_addr);
#endif
}


/*充电桩地址*/
void Debug_Get_chargeId()
{
#if (PRODUCTS_LEVEL != FACTORY_UPGRADE_VERSION)
	PARAM_DEV_TYPE *BgDevParam =ChgData_GetDevParamPtr();	

	memset(charge_id, 0,sizeof(charge_id));
	memcpy(charge_id, BgDevParam->chargeId, sizeof(BgDevParam->chargeId));

//	printf("charge_id0 = %s\r\n", charge_id);
#endif
}


//电源模块LOG调试接口
void DebugInfoByMod(uint8 *buf)
{
	DebugInfo(MOD_MOD, buf);
		
}


//与车辆BMS通信LOG调试接口
void DebugInfoByBms(uint8 *buf)
{
	DebugInfo(BMS_MOD, buf);	
		
}

//控制引导LOG调试接口
void DebugInfoByCon(uint8 *buf)
{
	DebugInfo(CON_MOD, buf);
		
}
//屏幕log调试接口
void DebugInfoByScr(uint8 *buf)
{
	DebugInfo(SCR_MOD, buf);
		
}
//后台log调试接口
void DebugInfoByBack(uint8 *buf)
{
	DebugInfo(BAC_MOD, buf);
		
}

//ftp log调试接口
void DebugInfoByFtp(uint8 *buf)
{
	DebugInfo(FTP_MOD, buf);
		
}

//CHK log调试接口
void DebugInfoByChk(uint8 *buf)
{
	DebugInfo(CHK_MOD, buf);
		
}



uint32 Get_debug_index()
{
	return debug_index++;
}


void DebugInfoOutput(uint8 *buf, uint16 len)
{
	if(SendLog)
	{
#ifdef NET_DEBUG  //20191008	
//		SendMsgToNetDebugByPtr(MSG_DATA_TO_NETDEBUG,len,buf);
		Net_Debug_Write(buf, len, 1);
#endif

#ifdef SCOMM_DEBUG		
		ScommDebugPrintStrWithLen((char*)buf, len);
//	printf("%s", buf);
#endif
	}

}

uint8* SetHeadDebug(uint8 mod, uint8 *buf)
{
	TM_RTC_Time_t systime= Time_GetRtcTime();

	if(strlen(buf) < 156)
	{

		//head buf len max is 100
		memset(head_buf, 0, sizeof(head_buf));
	//	sprintf((char*)head_buf, "%s %.10d %.4d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d %s %s ",charge_id,Get_debug_index(),systime.year,systime.month,systime.date,
		sprintf((char*)head_buf, "\r\n%s %.10d %.2d:%.2d:%.2d.%.3d %s %s ",charge_id,Get_debug_index(),	systime.hours,systime.minutes,systime.seconds,systime.milliseconds, m_debug_str[mod], buf);

		return head_buf;
	}
	else
	{
			memset(head_buf, 0, sizeof(head_buf));
			sprintf((char*)head_buf, "\r\n%s %.10d %.2d:%.2d:%.2d.%.3d %s %s ",charge_id,Get_debug_index(), systime.hours,systime.minutes,systime.seconds,systime.milliseconds, m_debug_str[mod], "ERR");
		
			return head_buf;
	}
}

//带模块ID的调试接口

//uint8 d_info_buf[DEBUG_MAX_INFO_LEN];
OS_EVENT *debug_smp;  //保护全局变量和流程

void DebugInfo(uint8 mod, uint8 *buf)
{
    uint8 err= 0;
	
	//请求信号量
	OSSemPend(debug_smp,0,&err); 

	memset(debug_buf, 0, sizeof(debug_buf));

#ifdef NET_CONTROL_LOG
	if(IsModlLogOn(mod))
	{
		sprintf((char*)debug_buf,"%s", SetHeadDebug(mod, buf));
		DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
	}

#else

	switch(mod){
		case MOD_MOD:
		#ifdef MODULE_DEBUG	
			sprintf((char*)debug_buf,"%s",SetHeadDebug(mod,buf));
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));
		#endif
			break;
		case BMS_MOD:
		#ifdef BMS_DEBUG 
			sprintf((char*)debug_buf,"%s",SetHeadDebug(mod,buf));
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));
		#endif
			break;
		case CON_MOD:
		#ifdef CONTROL_DEBUG 
			sprintf((char*)debug_buf,"%s",SetHeadDebug(mod,buf));
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));
		#endif
			break;
		case SCR_MOD:
		#ifdef SCREEN_DEBUG 			
			sprintf((char*)debug_buf,"%s",SetHeadDebug(mod,buf));
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));
		#endif
			break;
		case BAC_MOD:
		#ifdef BACK_COMM_DEBUG 
			sprintf((char*)debug_buf,"%s",SetHeadDebug(mod,buf));
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));
		#endif
			break;
		case TCP_MOD:
		#ifdef BACK_COMM_DEBUG 
			sprintf((char*)debug_buf,"%s",SetHeadDebug(mod,buf));
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));
		#endif
			break;
		case FTP_MOD:			
		#ifdef FTP_DEBUG 
			sprintf((char*)debug_buf,"%s",SetHeadDebug(mod,buf));
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));
		#endif
			break;
		
		default:
			sprintf((char*)debug_buf,"%s",SetHeadDebug(UNK_MOD,buf));
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));
			break;
		
		}	

#endif	
	
	//释放信号量
	OSSemPost(debug_smp); 

		
}

void DebugInfoWithPx(uint8 mod, uint8 *buf, int32 param)
{
    uint8 err= 0;
	
	//请求信号量
	OSSemPend(debug_smp,0,&err); 

	memset(debug_buf, 0, sizeof(debug_buf));

#ifdef NET_CONTROL_LOG
	if(IsModlLogOn(mod))
	{
		sprintf((char*)debug_buf,"%s0x%x", SetHeadDebug(mod, buf), param);
		DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
	}
	
#else
	switch(mod){
		case MOD_MOD:
		#ifdef MODULE_DEBUG 
			sprintf((char*)debug_buf,"%s0x%x", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case BMS_MOD:
		#ifdef BMS_DEBUG 
			sprintf((char*)debug_buf,"%s0x%x", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case CON_MOD:
		#ifdef CONTROL_DEBUG 
			sprintf((char*)debug_buf,"%s0x%x", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case SCR_MOD:
		#ifdef SCREEN_DEBUG 
			sprintf((char*)debug_buf,"%s0x%x", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case BAC_MOD:
		#ifdef BACK_COMM_DEBUG 
			sprintf((char*)debug_buf,"%s0x%x", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case TCP_MOD:
		#ifdef BACK_COMM_DEBUG 
			sprintf((char*)debug_buf,"%s0x%x", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case FTP_MOD:			
		#ifdef FTP_DEBUG 
			sprintf((char*)debug_buf,"%s0x%x", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		default:
			sprintf((char*)debug_buf,"%s0x%x", SetHeadDebug(UNK_MOD, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
			break;
		
		}	
#endif
	
	//释放信号量
	OSSemPost(debug_smp); 

		
}


void DebugInfoWithPi(uint8 mod, uint8 *buf, int32 param)
{
    uint8 err= 0;
	
	//请求信号量
	OSSemPend(debug_smp,0,&err); 

	memset(debug_buf, 0, sizeof(debug_buf));

#ifdef NET_CONTROL_LOG
	if(IsModlLogOn(mod))
	{
		sprintf((char*)debug_buf,"%s%d", SetHeadDebug(mod, buf), param);
		DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
	}
		
#else
	switch(mod){
		case MOD_MOD:
		#ifdef MODULE_DEBUG 
			sprintf((char*)debug_buf,"%s%d", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case BMS_MOD:
		#ifdef BMS_DEBUG 
			sprintf((char*)debug_buf,"%s%d", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case CON_MOD:
		#ifdef CONTROL_DEBUG 
			sprintf((char*)debug_buf,"%s%d", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case SCR_MOD:
		#ifdef SCREEN_DEBUG 
			sprintf((char*)debug_buf,"%s%d", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case BAC_MOD:
		#ifdef BACK_COMM_DEBUG 
			sprintf((char*)debug_buf,"%s%d", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case TCP_MOD:
		#ifdef BACK_COMM_DEBUG 
			sprintf((char*)debug_buf,"%s%d", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case FTP_MOD:			
		#ifdef FTP_DEBUG 
			sprintf((char*)debug_buf,"%s%d", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		default:
			sprintf((char*)debug_buf,"%s%d", SetHeadDebug(UNK_MOD, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
			break;
		
		}
#endif
	
	//释放信号量
	OSSemPost(debug_smp); 

		
}

void DebugInfoWithPf(uint8 mod, uint8 *buf, float param)
{
    uint8 err= 0;

	//请求信号量
	OSSemPend(debug_smp,0,&err); 

	memset(debug_buf, 0, sizeof(debug_buf));

#ifdef NET_CONTROL_LOG
	if(IsModlLogOn(mod))
	{
		sprintf((char*)debug_buf,"%s %s %s %f", SetHeadDebug(mod, buf), param);
		DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
	}
			
#else
	switch(mod){
		case MOD_MOD:
		#ifdef MODULE_DEBUG 
			sprintf((char*)debug_buf,"%s %s %s %f", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case BMS_MOD:
		#ifdef BMS_DEBUG 
			sprintf((char*)debug_buf,"%s %s %s %f", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case CON_MOD:
		#ifdef CONTROL_DEBUG 
			sprintf((char*)debug_buf,"%s %s %s %f", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case SCR_MOD:
		#ifdef SCREEN_DEBUG 
			sprintf((char*)debug_buf,"%s %s %s %f", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case BAC_MOD:
		#ifdef BACK_COMM_DEBUG 
			sprintf((char*)debug_buf,"%s %s %s %f", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case TCP_MOD:
		#ifdef BACK_COMM_DEBUG 
			sprintf((char*)debug_buf,"%s %s %s %f", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case FTP_MOD:			
		#ifdef FTP_DEBUG 
			sprintf((char*)debug_buf,"%s %s %s %f", SetHeadDebug(mod, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		default:
			sprintf((char*)debug_buf,"%s %s %s %f", SetHeadDebug(UNK_MOD, buf), param);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
			break;
		
		}

#endif
		
		//释放信号量
		OSSemPost(debug_smp); 

		
}


#define DEBUG_DATA_MAX_BUF_LEN (DEBUG_MAX_BUF_LEN-100)
//uint8 d_date_buf[DEBUG_DATA_MAX_BUF_LEN+4];

void SetDebugData(uint8* dest, uint8* data, uint16 data_len)
{
    uint16 data_len_t = 0;
//	memset(d_date_buf, 0, sizeof(d_date_buf));
	//不能超出BUF
    data_len_t =  data_len >= DEBUG_DATA_MAX_BUF_LEN/2 ? DEBUG_DATA_MAX_BUF_LEN/2:data_len;
	Common_bufTransAscii(data, data_len_t, dest);

//	return dest;

}

void DebugInfoWithPbuf(uint8 mod, uint8 *buf, uint8 *data, uint16 data_len, uint8 mod_sec)
{
  //return;  //patli tmp

    uint8 err= 0;

  //过滤信息判断
  	if(mod_sec > 1 && DevRunSec%mod_sec) return;
	
	//请求信号量
	OSSemPend(debug_smp,0,&err); 
	memset(debug_buf, 0, sizeof(debug_buf));	

#ifdef NET_CONTROL_LOG
	if(IsModlLogOn(mod))
	{
		#if 0
		sprintf((char*)debug_buf,"%s %s %s %s\r\n", SetHeadDebug(mod, buf),  SetDebugData(data, data_len));
		#else
		sprintf((char*)debug_buf,"%s", SetHeadDebug(mod, buf));
		SetDebugData(&debug_buf[strlen((char*)debug_buf)],data, data_len);
		#endif	
		DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
	}
			
#else

	switch(mod){
		case MOD_MOD:
		#ifdef MODULE_DEBUG 
			sprintf((char*)debug_buf,"%s", SetHeadDebug(mod, buf));
			SetDebugData(&debug_buf[strlen((char*)debug_buf)],data, data_len);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case BMS_MOD:
		#ifdef BMS_DEBUG 
			sprintf((char*)debug_buf,"%s", SetHeadDebug(mod, buf));
			SetDebugData(&debug_buf[strlen((char*)debug_buf)],data, data_len);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case CON_MOD:
		#ifdef CONTROL_DEBUG 
			sprintf((char*)debug_buf,"%s", SetHeadDebug(mod, buf));
			SetDebugData(&debug_buf[strlen((char*)debug_buf)],data, data_len);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case SCR_MOD:
		#ifdef SCREEN_DEBUG 
			sprintf((char*)debug_buf,"%s", SetHeadDebug(mod, buf));
			SetDebugData(&debug_buf[strlen((char*)debug_buf)],data, data_len);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case BAC_MOD:
		#ifdef BACK_COMM_DEBUG 
			sprintf((char*)debug_buf,"%s", SetHeadDebug(mod, buf));
			SetDebugData(&debug_buf[strlen((char*)debug_buf)],data, data_len);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case TCP_MOD:
		#ifdef BACK_COMM_DEBUG 
			sprintf((char*)debug_buf,"%s", SetHeadDebug(mod, buf));
			SetDebugData(&debug_buf[strlen((char*)debug_buf)],data, data_len);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		case FTP_MOD:			
		#ifdef FTP_DEBUG 
			sprintf((char*)debug_buf,"%s", SetHeadDebug(mod, buf));
			SetDebugData(&debug_buf[strlen((char*)debug_buf)],data, data_len);
			DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
		#endif
			break;
		default:
				sprintf((char*)debug_buf,"%s", SetHeadDebug(UNK_MOD, buf));
				SetDebugData(&debug_buf[strlen((char*)debug_buf)],data, data_len);
				DebugInfoOutput(debug_buf, strlen((char*)debug_buf));		
			break;
		}	
#endif
	
	//释放信号量
	OSSemPost(debug_smp); 
		
}

/**********************************************************************************
function: adebug_init
1.ucos start debug thread
***********************************************************************************/

int8 adebug_init(void)
{
	int8 res;
	OS_CPU_SR cpu_sr;
	uint8 err = 0;
  	PARAM_COMM_TYPE *ptrNetPara = ChgData_GetCommParaPtr();

	memset(&mod_log_flag, 0, sizeof(mod_log_flag));	

#ifdef SCOMM_DEBUG   //串口通信调试接口
	d_comm_smp = OSSemCreate(1);
#endif

  	debug_smp = OSSemCreate(1);

  	msg_smp = OSSemCreate(1);
  
#ifdef NET_DEBUG	
	d_net_smp = OSSemCreate(1);	//patli 20191016
	
	d_net_tx_smp = OSSemCreate(1);	//patli 20191016
#endif	


//初始化调试串口
//	d_comm_smp = OSMutexCreate(0,&err);	 
	Init_DebugCom();

	Debug_Get_chargeId();


	ScommDebugPrintStr("充电机系统启动中。。。");
	An_Print("充电机系统启动中。。。\r\n");

	
	DebugInfoWithPi(BAC_MOD,"ptrNetPara->conntype",ptrNetPara->conntype);
	
	DebugInfoWithPi(BAC_MOD,"ptrNetPara->agreetype",ptrNetPara->agreetype);

	client_id = ptrNetPara->agreetype;

//初始化网络调试任务模块	
#ifdef NET_DEBUG  //patli 20200216 NET_DEBUG
	
	OS_ENTER_CRITICAL();
	OSTaskCreate(net_debug_thread,(void*)0,(OS_STK*)&NET_DEBUG_TASK_STK[NET_DEBUG_STK_SIZE-1],NET_DEBUG_TASK_PRIO); 
	OS_EXIT_CRITICAL();

#endif
	
	return res;
}

#else


//电源模块LOG调试接口
void DebugInfoByMod(uint8 *buf)
{
		
}

//与车辆BMS通信LOG调试接口
void DebugInfoByBms(uint8 *buf)
{
		
}

//控制引导LOG调试接口
void DebugInfoByCon(uint8 *buf)
{
		
}
//屏幕log调试接口
void DebugInfoByScr(uint8 *buf)
{

}
//后台log调试接口
void DebugInfoByBack(uint8 *buf)
{
		
}

//ftp log调试接口
void DebugInfoByFtp(uint8 *buf)
{

}

//CHK log调试接口
void DebugInfoByChk(uint8 *buf)
{
		
}


//带模块ID的调试接口
void DebugInfo(uint8 mod, uint8 *buf)
{
		
}

void DebugInfoWithPx(uint8 mod, uint8 *buf, int32 param)
{
		
}


void DebugInfoWithPi(uint8 mod, uint8 *buf, int32 param)
{
		
}

void DebugInfoWithPf(uint8 mod, uint8 *buf, float param)
{
		
}

void DebugInfoWithPbuf(uint8 mod, uint8 *buf, uint8 *data, uint16 data_len, uint8 mod_sec)
{
}
#endif
