#ifndef __DP83848_H
#define __DP83848_H
#include "sys.h"
#include "stm32f4x7_eth.h"
//////////////////////////////////////////////////////////////////////////////////	 
//DP83848 驱动代码	   							  
////////////////////////////////////////////////////////////////////////////////// 

/* Ethernet Flags for EthStatus variable */ 
#define ETH_LINK_FLAG           0x10 /* Ethernet Link Flag */

#define DP83848_PHY_ADDRESS  	1//0x00				//DP83848 PHY芯片地址.
//#define DP83848_RST 		   	PDout(3) 			//LAN8720复位引脚	 

extern ETH_DMADESCTypeDef *DMARxDscrTab;			//以太网DMA接收描述符数据结构体指针
extern ETH_DMADESCTypeDef *DMATxDscrTab;			//以太网DMA发送描述符数据结构体指针 
extern uint8_t *Rx_Buff; 							//以太网底层驱动接收buffers指针 
extern uint8_t *Tx_Buff; 							//以太网底层驱动发送buffers指针
extern ETH_DMADESCTypeDef  *DMATxDescToSet;			//DMA发送描述符追踪指针
extern ETH_DMADESCTypeDef  *DMARxDescToGet; 		//DMA接收描述符追踪指针 
extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;	//DMA最后接收到的帧信息指针
 

u8 DP83848_Init(void);
u8 DP83848_Get_Speed(void);
u8 DP83848_GetPHYlinkStatus(void);
u8 ETH_MACDMA_Config(void);
FrameTypeDef ETH_Rx_Packet(void);
u8 ETH_Tx_Packet(u16 FrameLength);
u32 ETH_GetCurrentTxBuffer(void);
u8 ETH_Mem_Malloc(void);
void ETH_Mem_Free(void);


#endif 

