
#ifndef __LAN8720_H
#define __LAN8720_H
#include "typedefs.h"
#include "stm32f4x7_eth.h"

/* Ethernet Flags for EthStatus variable */   
#define ETH_LINK_FLAG           0x10 /* Ethernet Link Flag */

#define LAN8720_PHY_ADDRESS  	0x00				//LAN8720 PHY????.
#define LAN8720_RST 		   	PDout(3) 			//LAN8720????	 

#if 0
extern ETH_DMADESCTypeDef *DMARxDscrTab;			//???DMA????????????
extern ETH_DMADESCTypeDef *DMATxDscrTab;			//???DMA???????????? 
extern uint8_t *Rx_Buff; 							//?????????buffers?? 
extern uint8_t *Tx_Buff; 							//?????????buffers??
#else
//extern __align(4) ETH_DMADESCTypeDef DMARxDscrTab[ETH_RXBUFNB];	
//extern __align(4) ETH_DMADESCTypeDef DMATxDscrTab[ETH_TXBUFNB];
//extern __align(4) uint8_t Rx_Buff[ETH_RX_BUF_SIZE*ETH_RXBUFNB]; 					
//extern __align(4) uint8_t Tx_Buff[ETH_TX_BUF_SIZE*ETH_TXBUFNB]; 	
#endif

extern ETH_DMADESCTypeDef  *DMATxDescToSet;			//DMA?????????
extern ETH_DMADESCTypeDef  *DMARxDescToGet; 		//DMA????????? 
extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;	//DMA???????????
 

extern u8 LAN8720_Init(void);
extern u8 LAN8720_Get_Speed(void);
extern u8 ETH_MACDMA_Config(void);
extern FrameTypeDef ETH_Rx_Packet(void);
extern u8 ETH_Tx_Packet(u16 FrameLength);
extern u32 ETH_GetCurrentTxBuffer(void);
extern u8 ETH_Mem_Malloc(void);
extern void ETH_Mem_Free(void);
extern uint8_t RJ45LineLinkStatus(void);

#endif 


