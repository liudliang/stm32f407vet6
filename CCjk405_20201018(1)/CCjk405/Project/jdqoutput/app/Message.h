#ifndef _MESSAGE_H
#define _MESSAGE_H

#ifdef  __cplusplus
extern  "C" {
#endif
#include "config.h"
	
#define RES_SUCC   1
#define RES_FAIL   0

#define MSG_DATA_SIZE 32

typedef enum E_MSG
{
	MSG_NONE = 0,
	MSG_PARAM_INPUT = 1,        /*参数输入       触摸屏-->主控*/
  	MSG_RESQ_PASSWD = 2,        /*请求密码输入   主控-->触摸屏*/   
	MSG_GET_PASSWD = 3,         /*获取到密码     触摸屏-->主控*/ 
  	MSG_VERIFY_RESULT = 4,      /*密码校验结果   主控-->触摸屏*/ 
  	MSG_WORK_STATU = 5,         /*工作状态0 1 2 3 4主控-->触摸屏*/ 
	
	MSG_CHECK_CARD = 6,
	MSG_MANAGE_CARD = 7,
	MSG_CARD_INFO  = 8,			/*byte0-信息:1-有卡0-无卡*/
	MSG_LOCKED_CARD = 9,        /*灰锁卡         主控-->触摸屏*/
	MSG_UNLOCK_CARD = 10,       /*解锁卡         触摸屏-->主控*/
	MSG_LOCK_RESULT = 11,       /*解锁结果byte0-枪号； byte1-结果:0-失败1-成功 */
	MSG_UNLOCK_RESULT = 12,     /*解卡结果       主控-->触摸屏  byte0-枪号；byte1- 结果:0-失败1-成功2-非启动卡*/
	MSG_SYSTEM_SETTING = 13,    /*系统设置       主控-->触摸屏*/
	MSG_PARAM_STORE = 14,       /*保存参数 0,1,2,3     -->主控*/
  	MSG_PARAM_CLEAR = 15,       /*清除参数 0,1,2,3,4   -->主控*/
	MSG_RECORD_CLEAR= 16,       /*清除充电记录*/
	
	MSG_START_STOP = 20,        /*启动，停止充电，启动前需填写相关参数 0 停止 1 启动 byte0-控制:0-停止，1-启动；byte1-枪号*/
	MSG_START_STOP_RESULT = 21, /*启动停止结果 主控反馈byte0-结果:1-成功，2-失败; byte1-原因; byte2-枪号*/
	MSG_UP_RECORD = 22,         /*上传历史记录 主控--> 后台 */
	MSG_CHGSTU_END = 23,        /*充电结束*/
	MSG_RECORD_RESULT = 24,         /*上传历史记录结果 后台-->主控*/
	
	MSG_ISO_START = 30,         /*启动绝缘检测   主控-->ISO*/
	MSG_ISO_STOP = 31,          /*停止绝缘检测   主控-->ISO*/
	MSG_ISO_FINISH = 32,        /*绝缘检测完成   ISO-->主控*/
	
	MSG_MOD_ONOFF = 40,         /*模块开关机 0关机 1开机    主控-->电力模块*/
	MSG_MOD_ADJUST = 41,        /*模块输出调整 1电压 2 电流 主控-->电力模块*/
	MSG_MOD_GRPSET = 42,        /*模块分组设置              主控-->电力模块*/ 
	MSG_MOD_DELALLCMD = 43,     /*删除命令列表中所有命令*/
	MSG_MODDATA_INIT = 44,      /*模块数据初始化*/
	MSG_MOD_PRESTOP = 45,      /*分组预启动前，模块关机*/
	MSG_MOD_DGRPSET = 46,      /*双枪分组设置，可设置单模块分组*/
	MSG_MODGRP_ADJUST = 47,    /*双枪功率分配调节电压、电流*/
	
	MSG_CARD_CHECK = 50,        /*检测到有效卡        读卡器-->主控   */
	MSG_CARDREADER_IDEL = 51,   /*读卡器返回IDEL状态  主控-->读卡器*/
	
	MSG_ONLINE_CARD = 60 ,      /*在线卡通知后台*/
	MSG_VIN_CARD = 61 ,      /*VIN通知后台*/
	
	MSG_UPPLUG_GUN = 62 ,      /*等待拔枪  充之鸟*/
	MSG_WAIT_RECORD = 63 ,      /*等待结算   充之鸟*/
	
	MSG_SELECT_GUNCARD = 80,    /*选择刷卡启动哪个枪*/

	MSG_TCPCLIENT_DATA_TO_BACK = 90,  	//TCP 任务数据发送给BACK任务；参数是数据指针
	MSG_DATA_TO_TCPCLIENT = 91,      	//BACK 任务数据发送给TCP任务	；参数是数据指针

	MSG_RESET_ETH = 92,					//网卡复位消息；参数无

	MSG_DEV_TO_DEBUG_HB=93,				//发给NET DEBUG的心跳；参数无
	
	MSG_DATA_TO_NETDEBUG = 94,  		//数据发送给NETDEBUG任务	

	MSG_DATA_PTR_TO_NETDEBUG = 95,		//发送调试数据给netdebug;参数是数据指针

	MSG_TO_NETDEBUG_FTPUPGRADE_OK = 96, 		//FTP 升级用户程序成功；无用
	
	MSG_TO_ENTER_IDLE = 97, 		//设备进入待机状态消息，参数无
	
	MSG_TO_PRINT_TASK_STATUS = 98, 		//调试打印当前任务名称，参数无

	MSG_MOD_GET_INFO = 99,

 /*****************以下不用，后续删除*****************************************/
//	MSG_STARTJY_CTRL   = 1, 
//	MSG_FINISH_JY      = 2, 
//	MSG_CHECK_CARD     = 3, 
//	MSG_VALID_CARD     = 4, 
//	MSG_UALOCK_CARD    = 5, 
//	MSG_ACCOUNT_CARD   = 6, /*????*/
//  MSG_UALOCK_RESULT  = 7,	/*??????*/
//	MSG_NOT_STARTCARD  = 8, /*????*/
//	MSG_MANAGE_CARD    = 9, /*??????*/
//	MSG_GET_PASSWORD   = 10,/*?????*/
//	MSG_IDEL_STATU     = 11,/*??????*/
//	MSG_READY_CHARGE   = 12,/*??????*/
//	MSG_CHARGEING_STATU =13,/*???*/
//	MSG_ERR_STATU      = 14,/*????*/
//	
//	MSG_START_CTRL     = 15,/*????*/
//	MSG_REQ_PW         = 16,/*????*/
//	MSG_ADJUST_PW      = 17,/*????*/
//	MSG_STOPOUT_PW     = 18,/*??????*/
//	MSG_STOP_CTRL      = 19,/*????*/
//	MSG_ADJUST_CURR    = 20,/*?????*/
//	MSG_ADJUST_VOLT    = 22,/*????*/
////	MSG_MOD_ONOFF      = 23,/*??????*/
//	MSG_MOD_GROUP      = 24,/*????*/
//	
//	MSG_MC_GET_IC_INFO = 80,/**/
//	MSG_MC_UP_CUR_RECORD=81,
//	MSG_MC_START_FAULT  =82,
//	MSG_MC_START_SUCCESS=83,
//	MSG_MC_END_CHARGE   =84,
}MSG_TYPE;


typedef enum {
	CARD_PASSWD_ERR = 1,  //读卡错误,请重试！
	CARD_ILLEGAL_ERR =2,  //不可识别卡
	CARD_NOMONEY_ERR = 3, //金额不足
	CARD_NOTIDLE_ERR = 4, //不是非空闲
	CARD_VIN_NO_MATCH = 5,  //VIN码不匹配  patli 20190926
}E_CARRDCHECK_ERR;
	



typedef struct 
{
	uint8		 Valid; /* 0?? 1?? */
	uint8    MsgType;
	uint8    MsgLenth;
	uint8    MsgData[MSG_DATA_SIZE];
	uint8	 *PtrMsg;
}MSG_STRUCT;

typedef struct 
{
	uint8	gunno; /* 0?? 1?? */
	uint8   status;
	uint8   reason;
	uint8   errcode;
	uint8	accFg;
	uint8   valid;
}MSG_WORK_STATUS_STRUCT;



extern BOOL  Message_QueueCreat(uint8 TaskPrio);
extern BOOL RcvMsgFromQueue(MSG_STRUCT* outmsg);

extern BOOL RcvPrioMsgFromQueue(MSG_STRUCT* outmsg,uint8 prio);

extern BOOL  SendMsg2Queue(MSG_STRUCT *msg, uint8 DestPrio);

extern void SendMsgPack(uint8 Type,uint8 Len,uint8 Data,uint8 Prio);
//extern void SendMsgDoubleByte(uint8 Type,uint8 Len,void *Data,uint8 Prio);

extern void SendMsgWithNByte(uint8 Type,uint8 Len,void *Data,uint8 Prio);

extern void SendMsgWithNByteByPtr(uint8 Type,uint8 Len,void *Data,uint8 Prio);



#ifdef  __cplusplus
}
#endif
#endif

