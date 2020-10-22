/**
  ******************************************************************************
  * @file    stm32f4x7_eth_bsp.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013 
  * @brief   STM32F4x7 Ethernet hardware configuration.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lwip/opt.h"
#include "stm32f4x7_eth.h"
#include "stm32f4x7_eth_bsp.h"
#include "main.h"
#include "netif.h"
#include "netconf.h"
#include "lwip/dhcp.h"
#include "EthNet.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ETH_InitTypeDef ETH_InitStructure;
__IO uint32_t  EthStatus = 0;
extern struct netif gnetif;
#ifdef USE_DHCP
extern __IO uint8_t DHCP_state;
#endif /* LWIP_DHCP */

/* Private function prototypes -----------------------------------------------*/
static void ETH_GPIO_Config(void);
static void ETH_MACDMA_Config(void);
static uint32_t Eth_Link_PHYITConfig(uint16_t PHYAddress);
static void Eth_Link_EXTIConfig(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  ETH_BSP_Config
  * @param  None
  * @retval None
  */
void ETH_BSP_Config(void)
{
  /***************************************************************************
    NOTE: 
         When using Systick to manage the delay in Ethernet driver, the Systick
         must be configured before Ethernet initialization and, the interrupt 
         priority should be the highest one.
  *****************************************************************************/

  /* Configure the GPIO ports for ethernet pins */
  ETH_GPIO_Config();
	
	/* Configure the Ethernet MAC/DMA */
  ETH_MACDMA_Config();

  /* Get Ethernet link status*/
  if(ETH_ReadPHYRegister(DP83848_PHY_ADDRESS, PHY_SR) & 1)
  {
    EthStatus |= ETH_LINK_FLAG;
  }

  /* Configure the PHY to generate an interrupt on change of link status */
  Eth_Link_PHYITConfig(DP83848_PHY_ADDRESS);

  /* Configure the EXTI for Ethernet link status. */
  Eth_Link_EXTIConfig(); 
}

/**
  * @brief  Configures the Ethernet Interface
  * @param  None
  * @retval None
  */
static void ETH_MACDMA_Config(void)
{  
  /* Enable ETHERNET clock  */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx |
                        RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);

  /* Reset ETHERNET on AHB Bus */
  ETH_DeInit();

  /* Software reset */
  ETH_SoftwareReset();

  /* Wait for software reset */
  while (ETH_GetSoftwareResetStatus() == SET);

  /* ETHERNET Configuration --------------------------------------------------*/
  /* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
  ETH_StructInit(&ETH_InitStructure);

  /* Fill ETH_InitStructure parametrs */
  /*------------------------   MAC   -----------------------------------*/
  ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
//  ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Disable;
//  ETH_InitStructure.ETH_Speed = ETH_Speed_10M;
//  ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;

  ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
  ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
  ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
  ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
  ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
  ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
  ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
  ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
#ifdef CHECKSUM_BY_HARDWARE
  ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif

  /*------------------------   DMA   -----------------------------------*/  
  
  /* When we use the Checksum offload feature, we need to enable the Store and Forward mode: 
  the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum, 
  if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
  ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
  ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
  ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;

  ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
  ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;
  ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
  ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
  ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;
  ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
  ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
  ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

  /* Configure Ethernet */
  EthStatus = ETH_Init(&ETH_InitStructure, DP83848_PHY_ADDRESS);
}

/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval None
  */
static void ETH_GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOs clocks */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOG, ENABLE);

  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	  /* MII/RMII Media interface selection --------------------------------------*/
#ifdef MII_MODE /* Mode MII with STM324xx-EVAL  */
 #ifdef PHY_CLOCK_MCO
	
  /* Output HSE clock (25MHz) on MCO pin (PA8) to clock the PHY */
  RCC_MCO1Config(RCC_MCO1Source_HSE, RCC_MCO1Div_1);
 #endif /* PHY_CLOCK_MCO */

  SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_MII);
#elif defined RMII_MODE  /* Mode RMII with STM324xx-EVAL */

  SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);
#endif

/* Ethernet pins configuration ************************************************/
   /*
        ETH_MDIO -------------------------> PA2
        ETH_MDC --------------------------> PC1
        //ETH_PPS_OUT ----------------------> PB5
        //ETH_MII_CRS ----------------------> PH2
        //ETH_MII_COL ----------------------> PH3
        //ETH_MII_RX_ER --------------------> PI10
        //ETH_MII_RXD2 ---------------------> PH6
        //ETH_MII_RXD3 ---------------------> PH7
        //ETH_MII_TX_CLK -------------------> PC3
        //ETH_MII_TXD2 ---------------------> PC2
        //ETH_MII_TXD3 ---------------------> PB8
        ETH_MII_RX_CLK/ETH_RMII_REF_CLK---> PA1
        ETH_MII_RX_DV/ETH_RMII_CRS_DV ----> PA7
        ETH_MII_RXD0/ETH_RMII_RXD0 -------> PC4
        ETH_MII_RXD1/ETH_RMII_RXD1 -------> PC5
        ETH_MII_TX_EN/ETH_RMII_TX_EN -----> PG11
        ETH_MII_TXD0/ETH_RMII_TXD0 -------> PG13
        ETH_MII_TXD1/ETH_RMII_TXD1 -------> PG14
        ETH_MII_RX_ER: PB10                   */

  /* Configure PA1, PA2 and PA7 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

  /* Configure PC1, PC4 and PC5 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);

  /* Configure PG11, PG14 and PG13 */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; 
	GPIO_Init(GPIOG, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource13, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_ETH);
}

/**
  * @brief  Configure the PHY to generate an interrupt on change of link status.
  * @param PHYAddress: external PHY address  
  * @retval None
  */
uint32_t Eth_Link_PHYITConfig(uint16_t PHYAddress)
{
  uint16_t tmpreg = 0;

  /* Read MICR register */
  tmpreg = ETH_ReadPHYRegister(PHYAddress, PHY_MICR);

  /* Enable output interrupt events to signal via the INT pin */
  tmpreg |= (uint16_t)(PHY_MICR_INT_EN | PHY_MICR_INT_OE);
  if(!(ETH_WritePHYRegister(PHYAddress, PHY_MICR, tmpreg)))
  {
    /* Return ERROR in case of write timeout */
    return ETH_ERROR;
  }

  /* Read MISR register */
  tmpreg = ETH_ReadPHYRegister(PHYAddress, PHY_MISR);

  /* Enable Interrupt on change of link status */
  tmpreg |= (uint16_t)PHY_MISR_LINK_INT_EN;
  if(!(ETH_WritePHYRegister(PHYAddress, PHY_MISR, tmpreg)))
  {
    /* Return ERROR in case of write timeout */
    return ETH_ERROR;
  }
  /* Return SUCCESS */
  return ETH_SUCCESS;   
}

/**
  * @brief  EXTI configuration for Ethernet link status.
  * @param PHYAddress: external PHY address  
  * @retval None
  */
static void Eth_Link_EXTIConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the INT (PG12) Clock */
  RCC_AHB1PeriphClockCmd(ETH_LINK_GPIO_CLK, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  /* Configure INT pin as input */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = ETH_LINK_PIN;
  GPIO_Init(ETH_LINK_GPIO_PORT, &GPIO_InitStructure);

  /* Connect EXTI Line to INT Pin */
  SYSCFG_EXTILineConfig(ETH_LINK_EXTI_PORT_SOURCE, ETH_LINK_EXTI_PIN_SOURCE);

  /* Configure EXTI line */
  EXTI_InitStructure.EXTI_Line = ETH_LINK_EXTI_LINE;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set the EXTI interrupt to priority 1*/
  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 11;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

#if 0
/**
  * @brief  This function handles Ethernet link status.
  * @param  None
  * @retval None
  */
void Eth_Link_ITHandler(uint16_t PHYAddress)
{
  /* Check whether the link interrupt has occurred or not */
  if(((ETH_ReadPHYRegister(PHYAddress, PHY_MISR)) & PHY_LINK_STATUS) != 0)
  {
    if((ETH_ReadPHYRegister(PHYAddress, PHY_SR) & 1))
    {
      netif_set_link_up(&gnetif);
    }
    else
    {
      netif_set_link_down(&gnetif);
    }
  }
}

/**
  * @brief  Link callback function, this function is called on change of link status.
  * @param  The network interface
  * @retval None
  */
void ETH_link_callback(struct netif *netif)
{
  __IO uint32_t timeout = 0;
 uint32_t tmpreg,RegValue;
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
#ifndef USE_DHCP
  //uint8_t iptab[4] = {0};
  //uint8_t iptxt[20];
#endif /* USE_DHCP */

  if(netif_is_link_up(netif))
  {
    /* Restart the autonegotiation */
    if(ETH_InitStructure.ETH_AutoNegotiation != ETH_AutoNegotiation_Disable)
    {
      /* Reset Timeout counter */
      timeout = 0;

      /* Enable Auto-Negotiation */
      ETH_WritePHYRegister(DP83848_PHY_ADDRESS, PHY_BCR, PHY_AutoNegotiation);

      /* Wait until the auto-negotiation will be completed */
      do
      {
        timeout++;
      } while (!(ETH_ReadPHYRegister(DP83848_PHY_ADDRESS, PHY_BSR) & PHY_AutoNego_Complete) && (timeout < (uint32_t)PHY_READ_TO));

      /* Reset Timeout counter */
      timeout = 0;

      /* Read the result of the auto-negotiation */
      RegValue = ETH_ReadPHYRegister(DP83848_PHY_ADDRESS, PHY_SR);
    
      /* Configure the MAC with the Duplex Mode fixed by the auto-negotiation process */
      if((RegValue & PHY_DUPLEX_STATUS) != (uint32_t)RESET)
      {
        /* Set Ethernet duplex mode to Full-duplex following the auto-negotiation */
        ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;  
      }
      else
      {
        /* Set Ethernet duplex mode to Half-duplex following the auto-negotiation */
        ETH_InitStructure.ETH_Mode = ETH_Mode_HalfDuplex;
      }
      /* Configure the MAC with the speed fixed by the auto-negotiation process */
      if(RegValue & PHY_SPEED_STATUS)
      {
        /* Set Ethernet speed to 10M following the auto-negotiation */
        ETH_InitStructure.ETH_Speed = ETH_Speed_10M; 
      }
      else
      {
        /* Set Ethernet speed to 100M following the auto-negotiation */ 
        ETH_InitStructure.ETH_Speed = ETH_Speed_100M;      
      }

      /*------------------------ ETHERNET MACCR Re-Configuration --------------------*/
      /* Get the ETHERNET MACCR value */  
      tmpreg = ETH->MACCR;

      /* Set the FES bit according to ETH_Speed value */ 
      /* Set the DM bit according to ETH_Mode value */ 
      tmpreg |= (uint32_t)(ETH_InitStructure.ETH_Speed | ETH_InitStructure.ETH_Mode);

      /* Write to ETHERNET MACCR */
      ETH->MACCR = (uint32_t)tmpreg;

      _eth_delay_(ETH_REG_WRITE_DELAY);
      tmpreg = ETH->MACCR;
      ETH->MACCR = tmpreg;
    }

    /* Restart MAC interface */
    ETH_Start();

#ifdef USE_DHCP
    ipaddr.addr = 0;
    netmask.addr = 0;
    gw.addr = 0;
    DHCP_state = DHCP_START;
#else
    IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
    IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
    IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
#endif /* USE_DHCP */

    netif_set_addr(&gnetif, &ipaddr , &netmask, &gw);
    
    /* When the netif is fully configured this function must be called.*/
    netif_set_up(&gnetif);    

#ifdef USE_LCD
    /* Set the LCD Text Color */


    /* Display message on the LCD */


    /* Set the LCD Text Color */


  #ifndef USE_DHCP
    /* Display static IP address */
    iptab[0] = IP_ADDR3;
    iptab[1] = IP_ADDR2;
    iptab[2] = IP_ADDR1;
    iptab[3] = IP_ADDR0;
    sprintf((char*)iptxt, "  %d.%d.%d.%d", iptab[3], iptab[2], iptab[1], iptab[0]);

    /* Clear LCD */


  #endif /* USE_DHCP */
#endif /* USE_LCD */
  }
  else
  {
    ETH_Stop();
#ifdef USE_DHCP
    DHCP_state = DHCP_LINK_DOWN;
    dhcp_stop(netif);
#endif /* USE_DHCP */

    /*  When the netif link is down this function must be called.*/
    netif_set_down(&gnetif);
#ifdef USE_LCD
    /* Set the LCD Text Color */

    /* Display message on the LCD */

    /* Set the LCD Text Color */

#endif /* USE_LCD */
  }
}
#endif

#define PHY_ADDRESS              DP83848_PHY_ADDRESS
//#define  ETH_LINKOK              1
//#define  ETH_LINKERR             0

static uint8 PowerOnForEthnet = FALSE;
extern	ETH_DMADESCTypeDef  *DMATxDescToSet;
extern	ETH_DMADESCTypeDef  *DMARxDescToGet;
	ETH_DMADESCTypeDef  *DMAPTPTxDescToSet;
	ETH_DMADESCTypeDef  *DMAPTPRxDescToGet;

///*MAC层DMA占用的内存*/ 
//uint8 Rx_Buff[ETH_RXBUFNB][ETH_MAX_PACKET_SIZE];
//uint8 Tx_Buff[ETH_TXBUFNB][ETH_MAX_PACKET_SIZE]; 	
extern  uint8 Rx_Buff[];
extern  uint8 Tx_Buff[];

//struct netif    EMACNetif;						       // 网卡对象
//CONFIG_PARA	  g_Para;
OS_EVENT* SemEthnetSend ;//= NULL;
OS_EVENT* SemEthnetRcv ;//= NULL;

//ETH_DMADESCTypeDef  ETH_DMADESCTypeDef[ETH_RXBUFNB], DMATxDscrTab[ETH_TXBUFNB];
extern  ETH_DMADESCTypeDef DMARxDscrTab[];
extern  ETH_DMADESCTypeDef DMATxDscrTab[];

// ===============================================================================
// MAC 地址配置
// 获取MCU ID的低48位 作为MAC地址
// ==============================================================================
void MACAddressCfg(uint8 macaddr[])
{
	 /*非组播地址*/
  macaddr[0] &= 0xfe; 
	
	ETH_MACAddressConfig(ETH_MAC_Address0, macaddr);  // MAC地址设置 
}

void ETH_HwReset(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
	//以太网复位脚 PC2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	

	GPIO_WriteBit(GPIOC, GPIO_Pin_2, Bit_RESET);
	
	Delay10Ms(5);
	
	GPIO_WriteBit(GPIOC, GPIO_Pin_2, Bit_SET);
}

// ******************************************************************************
// sysEthnetHwInit - initialize  ethnet Devece struct and Hardware at first.
// RETURN: NONE
// ******************************************************************************
uint8 sysEthnetHwInit(void)
{
	uint8 rtn = ETH_SUCCESS;
	ETH_InitTypeDef ETH_InitStructure;
	
	
	if (FALSE == PowerOnForEthnet)
	{
		PowerOnForEthnet = TRUE;
		
		/* Enable ETHERNET clock  */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx | RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);  
		
		ETH_HwReset();

		ETH_GPIO_Config();		

  	ETH_DeInit();	                  // Reset ETHERNET on AHB Bus

		ETH_SoftwareReset();	          // Software reset  and Wait for software reset  
		while(ETH_GetSoftwareResetStatus()==SET){; }

	  ETH_StructInit(&ETH_InitStructure);	 // 以太网控制参数配置 
  
		//以太网MAC层控制参数再配置  
	  ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Disable;  // RxFIFO收到完整帧后才能被读取 	//rm add for test

	  ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;  	                 // 自动协商
	 // ETH_InitStructure.ETH_Speed = ETH_Speed_100M;                                      
	  ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;                         // 不使用内部回环            
	 // ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;                                                                                  
	  ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;               // 发生冲突时不进行重传                                                                                   
	  ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;         // 不进行自动CRC处理                                                                                                                                                                         
	  ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Enable;                              // 接收所有数据不进行过滤                                                                                                 
	  ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Disable; // 不接收广播数据      
	  
		ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;                   // 禁止杂乱模态                                                            
	  ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;       // 多播时判断地址      
	  ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;           // 单播时判断地址                       
		
	  ETH_Init(&ETH_InitStructure, PHY_ADDRESS);

		// DMATxDscrTab 发送控制数据结构 Tx_Buff 发送数据缓冲区起始地址  ETH_TXBUFNB 缓冲区个数=2
	  ETH_DMATxDescChainInit(DMATxDscrTab, Tx_Buff, ETH_TXBUFNB); // DMA发送缓冲区初始化 创建链表 写入DMA寄存器   
	  ETH_DMARxDescChainInit(DMARxDscrTab, Rx_Buff, ETH_RXBUFNB); // DMA接收缓冲区初始化 创建链表 写入DMA寄存器	
		
		MACAddressCfg(NET0->macaddress);
		//MACAddressCfg(); 		// 获取配置以太网MAC地址
	

		SemEthnetSend = OSSemCreate(1);	 // 以太网发送信号量
		SemEthnetRcv = OSSemCreate(1);	 // 以太网接收信号量
	
		
		ETH_Start();		// Enable MAC and DMA transmission and reception	
	}
	
	return rtn;
}
uint8 sysEthnetHwReset(void)
{
	uint8 rtn = ETH_SUCCESS;
	ETH_InitTypeDef ETH_InitStructure;
	
	

		ETH_HwReset();

	

 // 	ETH_DeInit();	                  // Reset ETHERNET on AHB Bus

//		ETH_SoftwareReset();	          // Software reset  and Wait for software reset  
//		while(ETH_GetSoftwareResetStatus()==SET){; }

	  ETH_StructInit(&ETH_InitStructure);	 // 以太网控制参数配置 
  
		//以太网MAC层控制参数再配置  
	  ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Disable;  // RxFIFO收到完整帧后才能被读取 	//rm add for test

	  ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;  	                 // 自动协商
	 // ETH_InitStructure.ETH_Speed = ETH_Speed_100M;                                      
	  ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;                         // 不使用内部回环            
	 // ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;                                                                                  
	  ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;               // 发生冲突时不进行重传                                                                                   
	  ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;         // 不进行自动CRC处理                                                                                                                                                                         
	  ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Enable;                              // 接收所有数据不进行过滤                                                                                                 
	  ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Disable; // 不接收广播数据      
	  
		ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;                   // 禁止杂乱模态                                                            
	  ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;       // 多播时判断地址      
	  ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;           // 单播时判断地址                       

    
		
	  ETH_Init(&ETH_InitStructure, PHY_ADDRESS);
	
	return rtn;
}
uint8 sysEthnetLink(void)
{
	if(ETH_ReadPHYRegister(0x01, PHY_BSR) & PHY_AutoNego_Complete)
	{
		return ETH_LINKOK;
	}
	
	return ETH_LINKERR;
}

// ========================================================================================
//  brief  Receives a packet and copies it to memory pointed by ppkt.
//  param  ppkt: pointer to application packet receive buffer.
//  retval : ETH_ERROR: if there is error in reception
//  framelength: received packet size if packet reception is correct
//  =======================================================================================
uint32_t ETH_HandleRxPkt(uint8_t *ppkt)
{ 
  uint32_t offset = 0, framelength = 0;
	/*描述符正被DMA或CPU使用 */
  if((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) != (uint32_t)RESET)	
  {
    return ETH_ERROR; 
  }
  /*没有错误状态 且 是帧的第一个描述符 且 是帧的最后一个描述符 */
  if(((DMARxDescToGet->Status & ETH_DMARxDesc_ES) == (uint32_t)RESET) && 
     ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) != (uint32_t)RESET) && 
     ((DMARxDescToGet->Status & ETH_DMARxDesc_FS) != (uint32_t)RESET))   
  {      
		/*接收包的 Frame Length 去掉4字节CRC */
    framelength = ((DMARxDescToGet->Status & ETH_DMARxDesc_FL) >> ETH_DMARxDesc_FrameLengthShift) - 4;
		/*从DMA 接收描述符的buf指针取数据到ppkt指向的位置 */
    for(offset=0; offset<framelength; offset++)       
    {
      (*(ppkt + offset)) = (*(__IO uint8_t *)((DMARxDescToGet->Buffer1Addr) + offset));
    }
  }
  else
  {
    framelength = ETH_ERROR;	                          
  }
	
  DMARxDescToGet->Status = ETH_DMARxDesc_OWN; 	          
 
  if ((ETH->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)  	 
  {
    ETH->DMASR = ETH_DMASR_RBUS;		                      
    ETH->DMARPDR = 0;		                                  
  }
  
  if((DMARxDescToGet->ControlBufferSize & ETH_DMARxDesc_RCH) != (uint32_t)RESET) 
  {     
    DMARxDescToGet = (ETH_DMADESCTypeDef*) (DMARxDescToGet->Buffer2NextDescAddr);
  }
  else                                                                          
  {   
    if((DMARxDescToGet->ControlBufferSize & ETH_DMARxDesc_RER) != (uint32_t)RESET)
    {
      DMARxDescToGet = (ETH_DMADESCTypeDef*) (ETH->DMARDLAR);     
    }
    else
    { 
			
      DMARxDescToGet = (ETH_DMADESCTypeDef*) ((uint32_t)DMARxDescToGet + 0x10 + ((ETH->DMABMR & ETH_DMABMR_DSL) >> 2));      
    }
  }
  
  return (framelength);  
}

// =======================================================================================
// brief  Transmits a packet, from application buffer, pointed by ppkt.
// param ppkt: pointer to application packet buffer to transmit.
// param FrameLength: Tx Packet size.
// retval : ETH_ERROR: in case of Tx desc owned by DMA
// ETH_SUCCESS: for correct transmission
// =======================================================================================
uint32_t ETH_HandleTxPkt(uint8_t *ppkt, uint16_t FrameLength)
{ 
  uint32_t offset = 0;
    
   if((DMATxDescToSet->Status & ETH_DMATxDesc_OWN) != (uint32_t)RESET)	    // 查看描述符是不是被DMA或CPU占用
  {
    return ETH_ERROR;		                                                  // 占用则返回错误 
  }
       
  for(offset=0; offset<FrameLength; offset++)                             // 把数据装到DMA发送描述符
  {
    (*(__IO uint8_t *)((DMATxDescToSet->Buffer1Addr) + offset)) = (*(ppkt + offset));
  }
        
  DMATxDescToSet->ControlBufferSize = (FrameLength & ETH_DMATxDesc_TBS1);	// 设置数据帧长度   
  DMATxDescToSet->Status |= ETH_DMATxDesc_LS | ETH_DMATxDesc_FS;				  // 设置段配置位
  DMATxDescToSet->Status |= ETH_DMATxDesc_OWN;		                        // 出让描述符控制权给DMA
  if ((ETH->DMASR & ETH_DMASR_TBUS) != (uint32_t)RESET)		                // 如果寄存器状态字发送位被禁止，则恢复发送
  {
    ETH->DMASR = ETH_DMASR_TBUS;		                                      // 恢复发送
    ETH->DMATPDR = 0;			                            
  }
  
  // 更新DMA 发送描述符  
  if((DMATxDescToSet->Status & ETH_DMATxDesc_TCH) != (uint32_t)RESET)		          // 链表方式
  {     
    DMATxDescToSet = (ETH_DMADESCTypeDef*) (DMATxDescToSet->Buffer2NextDescAddr); // 指向下一个描述符   
  }
  else                                                                            // Ring Mode
  {  
    if((DMATxDescToSet->Status & ETH_DMATxDesc_TER) != (uint32_t)RESET)			      // 最后一个描述符
    {
      DMATxDescToSet = (ETH_DMADESCTypeDef*) (ETH->DMATDLAR);                     // 第一个描述符 
    }
    else											                                                    // 下一个描述符
    {  
      DMATxDescToSet = (ETH_DMADESCTypeDef*) ((uint32_t)DMATxDescToSet + 0x10 + ((ETH->DMABMR & ETH_DMABMR_DSL) >> 2));      
    }
  }
  return ETH_SUCCESS; 
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
