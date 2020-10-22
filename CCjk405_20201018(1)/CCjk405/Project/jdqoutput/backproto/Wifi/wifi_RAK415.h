#ifndef __WIFI_RAK415_H
#define __WIFI_RAK415_H

#ifdef  __cplusplus
  extern "C" {
#endif
 #include "typedefs.h"
#include "chgdata.h"   
//#include "os_cpu.h"

/************** Socket端口配置参数 ********************/
//typedef struct
//{
//    uint8  RemoteIP[4];             /* 服务器IP地址 */
//    uint16 RemoteSocket;            /* 服务器端口 */
//    uint16 LocalSocket;             /* 本机端口 */
//    
//}SOCKET_PARA_STR;

///************** STA模式下配置参数 ********************/
//typedef struct
//{
//    int8   Ssid[32];                /* 指定路由AP网络名称, ASCII码存储，不包括字符串结束标志 */
//    int8   Password[32];            /* 指定路由AP网络密码, ASCII码存储，不包括字符串结束标志 */
//    uint8  SecuEn;                  /* 指定路由AP是否加密，0=不加密，1=加密 */
//    uint8  DhcpEn;                  /* 获取IP地址方式，0=静态设置，1=动态获取  */
//    uint8  LocalIP[4];              /* 本机IP地址 */
//    uint8  NetMask[4];              /* 子网掩码 */
//    uint8  NetGateAddr[4];          /* 网关地址 */
//    uint8  DnsSever1[4];            /* DNS服务器地址 */
//    uint8  DnsSever2[4];            /* 备用DNS服务器地址 */  
//}STA_PARA_STR;

///************** WIFI网络配置参数 ********************/
//typedef struct
//{
//    SOCKET_PARA_STR   SocketPara;   /* Socket配置参数 */
//    STA_PARA_STR      StaModePara;  /* STA模式配置参数 */  
//    
//}WIFI_NET_CONFIG_PARA;

//extern WIFI_NET_CONFIG_PARA  WifiNetConfigPara;        /* WIFI网络配置参数 */

//ErrorStatus RAK415_OpenAssistCmdMode(uint8 ComNo);       /* 开启WiFi模块辅助命令模式 */
ErrorStatus RAK415_SetPara(uint8 ComNo,ETH_PARAM_T *Para);         /* 设置WiFi模块参数 */
ErrorStatus RAK415_Reset_Cmd(uint8 ComNo);                    /* 命令复位WiFi模块 */
ErrorStatus Check_RAK415_TCPStatus(uint8 ComNo);              /* 查询WiFi模块TCP连接状态 */
ErrorStatus  RAK415_ParaConfig(uint8 ComNo,ETH_PARAM_T *Para);     /* 配置WiFi模块 */
ErrorStatus  GSM2_ParaConfig(uint8 ComNo,ETH_PARAM_T *Para);

ErrorStatus InEasyTxRx(uint8 ComNo);                           /* 进入透传模式 */

#ifdef __cplusplus
  }
#endif


#endif
