#ifndef __NET_LAY_COMM_H
#define __NET_LAY_COMM_H

#include "typedefs.h"
#include "chgdata.h"

/* 网络数据接口 */
typedef struct
{
	void 	(*Init)(ETH_PARAM_T *Para);/* 初始化网络 */
	void  (*Reset)(void);/* 复位网络 */
	int32 (*WriteData)(uint8 *buf,uint16 len);/* 写数据 */
	int32 (*ReadData)(uint8 *buf,uint16 len);/* 读数据 */
}NET_DATA_INTER;


/* WIFI接口函数 */
extern void Init_WifiModule(ETH_PARAM_T *Para);/* wifi串口初始化 */
extern void Reset_RAK415(void);/* wifi复位 */
extern int32 Uart_ReadData(uint8 *buf, uint16 maxSize);/* 接收数据 */
extern int32 Uart_WriteData(uint8 *buf, uint16 len);/* 发送数据 */

/* LWIP接口函数 */
extern void Init_Lwip(ETH_PARAM_T *Para);/* LWIP初始化 */
extern void Reset_Lwip(void);/* LWIP复位 */
extern int32 Lwip_ReadData(uint8 *buf,uint16 maxSize);/* 接收数据 */
extern int32 Lwip_WriteData(uint8 *buf,uint16 len);/* 发送数据 */
/* GPRS接口函数 */

extern void Init_NetInterFunc(NET_DATA_INTER *pNet,uint8 Type);






















#endif
