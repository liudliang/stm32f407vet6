#ifndef __Back_Inter_H
#define __Back_Inter_H


#include "ChgData.h"

/* �������ݽӿ� */
typedef struct
{
	void 	(*Init)(ETH_PARAM_T *Para);/* ��ʼ������ */
	void  (*Reset)(void);/* ��λ���� */
	int32 (*WriteData)(uint8 *buf,uint16 len);/* д���� */
	int32 (*ReadData)(uint8 *buf,uint16 len);/* ������ */
}NET_DATA_INTER;


/* WIFI�ӿں��� */
extern void Init_WifiModule(ETH_PARAM_T *Para);/* wifi���ڳ�ʼ�� */
extern void Reset_RAK415(void);/* wifi��λ */
extern int32 Uart_ReadData(uint8 *buf, uint16 maxSize);/* �������� */
extern int32 Uart_WriteData(uint8 *buf, uint16 len);/* �������� */
/* 4G�ӿں��� */
extern void Reset_4G(void);
extern void Init_4GModule(ETH_PARAM_T *Para);
extern void Init_2GModule(ETH_PARAM_T *Para);
/* LWIP�ӿں��� */
//extern void Init_Lwip(ETH_PARAM_T *Para);/* LWIP��ʼ�� */
//extern void Reset_Lwip(void);/* LWIP��λ */
//extern int32 Lwip_ReadData(uint8 *buf,uint16 maxSize);/* �������� */
//extern int32 Lwip_WriteData(uint8 *buf,uint16 len);/* �������� */
/* GPRS�ӿں��� */

extern void Init_NetInterFunc(NET_DATA_INTER *pNet,uint8 Type);






















#endif