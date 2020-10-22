/**
  ******************************************************************************
  * @file    MCP2515.c 
  * @author  Aunice Application Team
  * @version V1.4.0
  * @date    04-August-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
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
#include "MCP2515.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static void MCP2515_Reset(void); 
static void MCP2515_BitModify(uint8_t Addr, uint8_t Mask, uint8_t Data); 
static void MCP2515_SendByte(uint8_t Addr, uint8_t Data); 
static uint8_t MCP2515_ReadByte(uint8_t Addr); 
static uint8_t MCP2515_SPI_CMD(uint8_t CMD); 
static void SetCanBusFilter(uint32_t Filter, uint32_t Mask);
static uint8_t IsSendMail0Busy(void);
static uint8_t IsSendMail1Busy(void);
static uint8_t IsSendMail2Busy(void);
static void CAN_SendData0(uint8_t *CAN_TX_Buf, Frame_TypeDef *Frame);
static void CAN_SendData1(uint8_t *CAN_TX_Buf, Frame_TypeDef *Frame);
static void CAN_SendData2(uint8_t *CAN_TX_Buf, Frame_TypeDef *Frame);

extern void CAN_SendDataBusy0(void);
extern void CAN_SendDataBusy1(void);
extern void CAN_SendDataBusy2(void);

/* Public functions ---------------------------------------------------------*/
extern void Main_Delay(int32_t nCount);

/*RXB0's data registers:*/ 
static uint8_t RXB0D[8] = {RXB0D0, RXB0D1, RXB0D2, RXB0D3, RXB0D4, RXB0D5, RXB0D6, RXB0D7}; 
/*RXB1's data registers:*/ 
static uint8_t RXB1D[8] = {RXB1D0, RXB1D1, RXB1D2, RXB1D3, RXB1D4, RXB1D5, RXB1D6, RXB1D7};

/*TXB0's data registers:*/ 
static uint8_t TXB0D[8] = {TXB0D0, TXB0D1, TXB0D2, TXB0D3, TXB0D4, TXB0D5, TXB0D6, TXB0D7};

/*TXB1's data registers:*/ 
static uint8_t TXB1D[8] = {TXB1D0, TXB1D1, TXB1D2, TXB1D3, TXB1D4, TXB1D5, TXB1D6, TXB1D7};

/*TXB2's data registers:*/ 
static uint8_t TXB2D[8] = {TXB2D0, TXB2D1, TXB2D2, TXB2D3, TXB2D4, TXB2D5, TXB2D6, TXB2D7};

/* Private variables ---------------------------------------------------------*/
static uint32_t MCP2515_FilterID = 0;
static uint32_t MCP2515_MaskID = 0;

/**
  * @brief  MCP2515_Init.
  * @param  None.
  *         
  * @retval None.
  */
void MCP2515_Init(uint32_t Filter, uint32_t Mask) 
{ 
	uint8_t dummy = 0;
	
	/*First initialize the SPI periphere*/ 
	/*Then initialize the MCP2515.Step as follow: 
	1>Get into configuration mode by reset MCP2515 or write the bit of CANCTRL.REQOP. 
	2>Check the CANSTAT by MCP2515_ReadByte(CANSTAT) if MCP2515 is already into configuration mode. 
	3>Configurate the registers to set the baudrate:CN1,CN2,CN3. 
	4>Configurate TXRTSCTRL to set the function of TXnRTS pin. 
	5>Configurate TRXBnCTRL to set the pirority of transmit mailbox:TXB0,TXB1,TXB2. 
	6>Configurate TXBn'SID ,EID and DLC.This step can configurate when you are in normal mode. 
	7>Configurate RXFnSIDH and RXFnSIDL to resceive the specific ID. 
	8>Configurate RXMnSIDH and RXMnSIDL to mask the RXFnSID's ID. 
	9>Configurate CANINTE to enable or disable the interrupts. 
	10>Return to the normal mode and double check if it is return to the normal mode by CANSTAT. 
	*/   

	/*Flip into the Configuration Mode*/ 
	MCP2515_Reset(); 
	//_delay_(0x4000);/* 1ms _delay_ using Fcpu = 8Mhz*/
	Main_Delay(1);
	
	//Set the baudrate 
	//set CNF1,SJW=00,lengthe is 1TQ,BRP=49,TQ=[2*(BRP+1)]/Fsoc=2*50/8M=12.5us 
	MCP2515_SendByte(CNF1, CAN_125Kbps); 
	//set CNF2,SAM=0,caputre one time,PHSEG1=(2+1)TQ=3TQ,PRSEG=(0+1)TQ=1TQ 
	MCP2515_SendByte(CNF2, (0x80 | PHSEG1_3TQ | PRSEG_1TQ)); 
	//set CNF3,PHSEG2=(2+1)TQ=3TQ,when CANCTRL.CLKEN=1,set CLKOUT pin to output 
	MCP2515_SendByte(CNF3, PHSEG2_3TQ); 
	
	/* 
	3 Transimit Buffers: TXB0, TXB1, TXB2 
	2 Receive   Buffers: RXB0, RXB1 
	6 ID Filter: RXF0~RXF5 
	2 ID Mask Regieter: RXM0, RXM1 
	*/
	
	//configurate RXB0' registers  
	MCP2515_SendByte(RXB0CTRL, 0x00);//Receive all valid messages using either standard or extended identifiers that meet filter criteria
	MCP2515_SendByte(RXB1CTRL, 0x00);//Receive all valid messages using either standard or extended identifiers that meet filter criteria
	
	//MCP2515_SendByte(RXF0SIDH, 0xFF);//Fileter register 
	//MCP2515_SendByte(RXF0SIDL, 0xE0);//Just receive ID=0x7FF frame 
	//MCP2515_SendByte(RXF1SIDH, 0xFD);//Fileter register 
	//MCP2515_SendByte(RXF1SIDL, 0xC0);//Just receive ID=0x7EE frame 
	//MCP2515_SendByte(RXM0SIDH, 0xFF);//Mask register 
	//MCP2515_SendByte(RXM0SIDL, 0xE0); 
	
	SetCanBusFilter(Filter, Mask);
	
	MCP2515_FilterID = Filter;
	MCP2515_MaskID = Mask;
	
	//Configurate TXB0's ID and DLC registers 
	//MCP2515_SendByte(TXB0SIDH, 0xFF);//Standard ID 
	//MCP2515_SendByte(TXB0SIDL, 0xE0);//Standard ID 
	//MCP2515_SendByte(TXB0DLC, DLC_1);//DLC 
	
	/*Set the RXB0 or RXB1 interrupt enableling*/ 
	//MCP2515_SendByte(CANINTE, RX0IE | RX1IE | TX0IE | TX1IE | TX2IE | ERRIE | MERRE);  //Enable RXB0 RXB1 interrupt 
	MCP2515_SendByte(CANINTE, RX0IE | RX1IE | ERRIE | MERRE);  //Enable RXB0 RXB1 interrupt
	
	MCP2515_SendByte(CANCTRL, REQOP_NORMAL | CLKOUT_ENABLED);//Go into the normal mode 
	
	dummy = MCP2515_ReadByte(CANSTAT);	
	if (OPMODE_NORMAL != (dummy & 0xE0))
	{		
		MCP2515_SendByte(CANCTRL, REQOP_NORMAL | CLKOUT_ENABLED);//If normal ok
	}			
}

/** 
  * @brief  Reset the MCP2515 then U go into the configruation mode 
  * @param  none 
  * @retval none 
  */ 
static void MCP2515_Reset(void) 
{ 
	MCP2515_CS_LOW();                
	MCP2515_SPI_Send(SPI_RESET);    
	MCP2515_CS_HIGH();                
}

/** 
  * @brief  Modify the bit of the register 
  * @param  Add:Address of register 
  * @param  Mask:1-->U can modify the corresponding bit;0-->U can not modify the corresponding bit 
  * @param  Data:the data U need to be sent from MCU to MCP2515 
  * @retval none 
  */ 
static void MCP2515_BitModify(uint8_t Addr, uint8_t Mask, uint8_t Data) 
{ 
	MCP2515_CS_LOW();                
	MCP2515_SPI_Send(SPI_BIT_MODIFY); 
	MCP2515_SPI_Send(Addr); 
	MCP2515_SPI_Send(Mask); 
	MCP2515_SPI_Send(Data); 
	MCP2515_CS_HIGH();    
}

/** 
  * @brief  Send a byte to MCP2515 through SPI to return corresponding status you need 
  * @param  Add:Address of register 
  * @param  Data:the data U need to be sent from MCU to MCP2515 
  * @retval none 
  */ 
static void MCP2515_SendByte(uint8_t Addr, uint8_t Data) 
{ 
	MCP2515_CS_LOW();        
	MCP2515_SPI_Send(SPI_WRITE); 
	MCP2515_SPI_Send(Addr); 
	MCP2515_SPI_Send(Data); 
	MCP2515_CS_HIGH();        
}

/** 
  * @brief  Send a command to MCP2515 through SPI to return corresponding status you need 
  * @param  CMD:command 
  * @retval Data:Data from MCP2515'SO pin 
  */ 
static uint8_t MCP2515_ReadByte(uint8_t Addr) 
{ 
	uint8_t Data;                    
	MCP2515_CS_LOW();                
	MCP2515_SPI_Send(SPI_READ); 
	MCP2515_SPI_Send(Addr); 
	Data = MCP2515_SPI_Receive();    
	MCP2515_CS_HIGH();                
	return Data; 
}

/** 
  * @brief  Send a command to MCP2515 through SPI to return corresponding status you need 
  * @param  CMD:SPI_RX_STATUS; 
                SPI_READ_STATUS; 
                SPI_READ_RX; 
  * @retval Data:the status you need 
  */ 
static uint8_t MCP2515_SPI_CMD(uint8_t CMD) 
{ 
	uint8_t Data;                    
	MCP2515_CS_LOW();                
	MCP2515_SPI_Send(CMD); 
	Data = MCP2515_SPI_Receive();    
	MCP2515_CS_HIGH();                
	return Data; 
}

/** 
  * @brief   Send n bytes with a given standard ID  corresponding to frame type 
  * @param   CAN_TX_Buf: data to be sent 
  * @param   DLC: DLC <= 8 
  * @param   SID: <= 0x7FF 
  * @param   CAN_FrameType:CAN_STD,CAN_RTR        
  * @retval  None 
  */ 
static void CAN_SendData0(uint8_t *CAN_TX_Buf, Frame_TypeDef *Frame) 
{ 
	uint8_t tempdata; 
	uint8_t HSID, LSID, EID8, EID0; 
	
	if (Frame->Type == CAN_STD) 
	{ 
		/*Set the ID of the frame*/ 
		HSID = (uint8_t)(Frame->ID >> 3); 
		LSID = (uint8_t)((Frame->ID << 5) & 0xE0); 
		MCP2515_SendByte(TXB0SIDH, HSID); 
		MCP2515_SendByte(TXB0SIDL, LSID); 
		/*Set the DLC and the type of the frame*/ 
		MCP2515_SendByte(TXB0DLC, Frame->DLC); 
		
		/*Write the data into the TXB0 data registers */ 
		for (tempdata = 0; tempdata < Frame->DLC; tempdata++)
		{		
			MCP2515_SendByte(TXB0D[tempdata], CAN_TX_Buf[tempdata]);  
		}	
	} 
	else if (Frame->Type == CAN_EXTD)
	{
		/*Set the ID of the frame*/ 
		HSID = (uint8_t)(Frame->ID >> 21); 
		LSID = (uint8_t)((Frame->ID >> 13) & 0xE0) | CAN_EXTD | (uint8_t)((Frame->ID >> 16) & 0x03); 
		EID8 = (uint8_t)(Frame->ID >> 8);
		EID0 = (uint8_t)Frame->ID;
		MCP2515_SendByte(TXB0SIDH, HSID); 
		MCP2515_SendByte(TXB0SIDL, LSID); 
		MCP2515_SendByte(TXB0EID8, EID8); 
		MCP2515_SendByte(TXB0EID0, EID0); 
		/*Set the DLC and the type of the frame*/ 
		MCP2515_SendByte(TXB0DLC, Frame->DLC); 
		
		/*Write the data into the TXB0 data registers */ 
		for (tempdata=0; tempdata < Frame->DLC; tempdata++)
		{		
			MCP2515_SendByte(TXB0D[tempdata], CAN_TX_Buf[tempdata]);
		}	
	}
	else if (Frame->Type == CAN_RTR)
	{ 
		/*Set the ID of the frame*/ 
		HSID = (uint8_t)(Frame->ID >> 3); 
		LSID = (uint8_t)((Frame->ID << 5) & 0xE0); 
		MCP2515_SendByte(TXB0SIDH, HSID); 
		MCP2515_SendByte(TXB0SIDL, LSID); 
		
		/*Set the type of the frame*/ 
		MCP2515_SendByte(TXB0DLC, CAN_RTR); 
	} 

	MCP2515_BitModify(CANINTF, 0x04, 0x00);
	
	/*Send the SPI_RTS_TXB0 request command to MCP2515 to send the data loaded in the data register*/ 
	MCP2515_CS_LOW();
	MCP2515_SPI_Send(SPI_RTS_TXB0); 
	MCP2515_CS_HIGH(); 
}

/** 
  * @brief   Send n bytes with a given standard ID  corresponding to frame type 
  * @param   CAN_TX_Buf: data to be sent 
  * @param   DLC: DLC <= 8 
  * @param   SID: <= 0x7FF 
  * @param   CAN_FrameType:CAN_STD,CAN_RTR        
  * @retval  None 
  */ 
static void CAN_SendData1(uint8_t *CAN_TX_Buf, Frame_TypeDef *Frame) 
{ 
	uint8_t tempdata; 
	uint8_t HSID, LSID, EID8, EID0; 
	
	if (Frame->Type == CAN_STD) 
	{ 
		/*Set the ID of the frame*/ 
		HSID = (uint8_t)(Frame->ID >> 3); 
		LSID = (uint8_t)((Frame->ID << 5) & 0xE0); 
		MCP2515_SendByte(TXB1SIDH, HSID); 
		MCP2515_SendByte(TXB1SIDL, LSID); 
		/*Set the DLC and the type of the frame*/ 
		MCP2515_SendByte(TXB1DLC, Frame->DLC); 
		
		/*Write the data into the TXB1 data registers */ 
		for (tempdata = 0; tempdata < Frame->DLC; tempdata++)
		{		
			MCP2515_SendByte(TXB1D[tempdata], CAN_TX_Buf[tempdata]);  
		}	
	} 
	else if (Frame->Type == CAN_EXTD)
	{
		/*Set the ID of the frame*/ 
		HSID = (uint8_t)(Frame->ID >> 21); 
		LSID = (uint8_t)((Frame->ID >> 13) & 0xE0) | CAN_EXTD | (uint8_t)((Frame->ID >> 16) & 0x03); 
		EID8 = (uint8_t)(Frame->ID >> 8);
		EID0 = (uint8_t)Frame->ID;
		MCP2515_SendByte(TXB1SIDH, HSID); 
		MCP2515_SendByte(TXB1SIDL, LSID); 
		MCP2515_SendByte(TXB1EID8, EID8); 
		MCP2515_SendByte(TXB1EID0, EID0); 
		/*Set the DLC and the type of the frame*/ 
		MCP2515_SendByte(TXB1DLC, Frame->DLC); 
		
		/*Write the data into the TXB1 data registers */ 
		for (tempdata=0; tempdata < Frame->DLC; tempdata++)
		{		
			MCP2515_SendByte(TXB1D[tempdata], CAN_TX_Buf[tempdata]);
		}	
	}
	else if (Frame->Type == CAN_RTR)
	{ 
		/*Set the ID of the frame*/ 
		HSID = (uint8_t)(Frame->ID >> 3); 
		LSID = (uint8_t)((Frame->ID << 5) & 0xE0); 
		MCP2515_SendByte(TXB1SIDH, HSID); 
		MCP2515_SendByte(TXB1SIDL, LSID); 
		
		/*Set the type of the frame*/ 
		MCP2515_SendByte(TXB1DLC, CAN_RTR); 
	} 

	MCP2515_BitModify(CANINTF, 0x08, 0x00);
	
	/*Send the SPI_RTS_TXB1 request command to MCP2515 to send the data loaded in the data register*/ 
	MCP2515_CS_LOW();
	MCP2515_SPI_Send(SPI_RTS_TXB1); 
	MCP2515_CS_HIGH();
}

/** 
  * @brief   Send n bytes with a given standard ID  corresponding to frame type 
  * @param   CAN_TX_Buf: data to be sent 
  * @param   DLC: DLC <= 8 
  * @param   SID: <= 0x7FF 
  * @param   CAN_FrameType:CAN_STD,CAN_RTR        
  * @retval  None 
  */ 
static void CAN_SendData2(uint8_t *CAN_TX_Buf, Frame_TypeDef *Frame) 
{ 
	uint8_t tempdata; 
	uint8_t HSID, LSID, EID8, EID0; 
	
	if (Frame->Type == CAN_STD) 
	{ 
		/*Set the ID of the frame*/ 
		HSID = (uint8_t)(Frame->ID >> 3); 
		LSID = (uint8_t)((Frame->ID << 5) & 0xE0); 
		MCP2515_SendByte(TXB2SIDH, HSID); 
		MCP2515_SendByte(TXB2SIDL, LSID); 
		/*Set the DLC and the type of the frame*/ 
		MCP2515_SendByte(TXB2DLC, Frame->DLC); 
		
		/*Write the data into the TXB2 data registers */ 
		for (tempdata = 0; tempdata < Frame->DLC; tempdata++)
		{		
			MCP2515_SendByte(TXB2D[tempdata], CAN_TX_Buf[tempdata]);  
		}	
	} 
	else if (Frame->Type == CAN_EXTD)
	{
		/*Set the ID of the frame*/ 
		HSID = (uint8_t)(Frame->ID >> 21); 
		LSID = (uint8_t)((Frame->ID >> 13) & 0xE0) | CAN_EXTD | (uint8_t)((Frame->ID >> 16) & 0x03); 
		EID8 = (uint8_t)(Frame->ID >> 8);
		EID0 = (uint8_t)Frame->ID;
		MCP2515_SendByte(TXB2SIDH, HSID); 
		MCP2515_SendByte(TXB2SIDL, LSID); 
		MCP2515_SendByte(TXB2EID8, EID8); 
		MCP2515_SendByte(TXB2EID0, EID0); 
		/*Set the DLC and the type of the frame*/ 
		MCP2515_SendByte(TXB2DLC, Frame->DLC); 
		
		/*Write the data into the TXB2 data registers */ 
		for (tempdata=0; tempdata < Frame->DLC; tempdata++)
		{		
			MCP2515_SendByte(TXB2D[tempdata], CAN_TX_Buf[tempdata]);
		}	
	}
	else if (Frame->Type == CAN_RTR)
	{ 
		/*Set the ID of the frame*/ 
		HSID = (uint8_t)(Frame->ID >> 3); 
		LSID = (uint8_t)((Frame->ID << 5) & 0xE0); 
		MCP2515_SendByte(TXB2SIDH, HSID); 
		MCP2515_SendByte(TXB2SIDL, LSID); 
		
		/*Set the type of the frame*/ 
		MCP2515_SendByte(TXB2DLC, CAN_RTR); 
	} 

	MCP2515_BitModify(CANINTF, 0x10, 0x00);
	
	/*Send the SPI_RTS_TXB2 request command to MCP2515 to send the data loaded in the data register*/ 
	MCP2515_CS_LOW();
	MCP2515_SPI_Send(SPI_RTS_TXB2); 
	MCP2515_CS_HIGH();
}

/** 
  * @brief  CAN_SendDataBusy0. 
  * @param  void 
  * @retval void 
  */ 
void CAN_SendDataBusy0(void)
{
	uint8_t  tempdata = 0;
	uint32_t TIMEOUT  = 0xA380; 
	
	do
	{
		tempdata = MCP2515_ReadByte(TXB0CTRL); 
		TIMEOUT--;
	} while (((tempdata & 0x08) == 0x08) && (TIMEOUT != 0));
	
	if (0 == TIMEOUT)
	{
		MCP2515_Init(MCP2515_FilterID, MCP2515_MaskID);
	}
}

/** 
  * @brief  CAN_SendDataBusy1. 
  * @param  void 
  * @retval void 
  */ 
void CAN_SendDataBusy1(void)
{
	uint8_t  tempdata = 0;
	uint32_t TIMEOUT  = 0xA380; 
	
	do
	{
		tempdata = MCP2515_ReadByte(TXB1CTRL); 
		TIMEOUT--;
	} while (((tempdata & 0x08) == 0x08) && (TIMEOUT != 0));
	
	if (0 == TIMEOUT)
	{
		MCP2515_Init(MCP2515_FilterID, MCP2515_MaskID);
	}	
}

/** 
  * @brief  CAN_SendDataBusy2. 
  * @param  void 
  * @retval void 
  */ 
void CAN_SendDataBusy2(void)
{
	uint8_t  tempdata = 0;
	uint32_t TIMEOUT  = 0xA380; 
	
	do
	{
		tempdata = MCP2515_ReadByte(TXB2CTRL); 
		TIMEOUT--;
	} while (((tempdata & 0x08) == 0x08) && (TIMEOUT != 0));
	
	if (0 == TIMEOUT)
	{
		MCP2515_Init(MCP2515_FilterID, MCP2515_MaskID);
	}	
}

/** 
  * @brief  IsSendMail0Busy. 
  * @param  void 
  * @retval void 
  */
static uint8_t IsSendMail0Busy(void)
{
	uint8_t  tempdata = 0;
	
	tempdata = MCP2515_ReadByte(TXB0CTRL);
	
	return (tempdata & 0x08) == 0x08 ? 1 : 0;	
}

/** 
  * @brief  IsSendMail1Busy. 
  * @param  void 
  * @retval void 
  */
static uint8_t IsSendMail1Busy(void)
{
	uint8_t  tempdata = 0;
	
	tempdata = MCP2515_ReadByte(TXB1CTRL);

	return (tempdata & 0x08) == 0x08 ? 1 : 0;
}


/** 
  * @brief  IsSendMail2Busy. 
  * @param  void 
  * @retval void 
  */
static uint8_t IsSendMail2Busy(void)
{
	uint8_t  tempdata = 0;
	
	tempdata = MCP2515_ReadByte(TXB2CTRL);

	return (tempdata & 0x08) == 0x08 ? 1 : 0;
}

/** 
  * @brief  MCP2515_CAN_Transmit. 
  * @param  void 
  * @retval void 
  */
int8_t MCP2515_CAN_Transmit(uint8_t *CAN_TX_Buf, Frame_TypeDef *Frame)
{
	int8_t transmit_mailbox = -1;
	
	/* Select one empty transmit mailbox */
  if (IsSendMail0Busy() == 0)
  {
    transmit_mailbox = 0;
		CAN_SendData0(CAN_TX_Buf, Frame);
  }
  else if (IsSendMail1Busy() == 0)
  {
    transmit_mailbox = 1;
		CAN_SendData1(CAN_TX_Buf, Frame);
  }
  else if (IsSendMail2Busy() == 0)
  {
    transmit_mailbox = 2;
		CAN_SendData2(CAN_TX_Buf, Frame);
  }
  else
  {
    transmit_mailbox = -1;
  }
	
	return transmit_mailbox;
}


/**
  * @brief  SetCanBusFilter.
  * @param  None
  * @retval None
  */
static void SetCanBusFilter(uint32_t Filter, uint32_t Mask)
{	
	uint8_t SIDH = 0;
	uint8_t SIDL = 0;
	uint8_t EID8 = 0;
	uint8_t EID0 = 0;
	uint8_t temp = 0;
	
	Filter &= 0x1FFFFFFF;
	Mask   &= 0x1FFFFFFF;
	
	SIDH   =  Filter >> 21;

	temp   = Filter >> 18;
	temp  &= 0x07;
	temp <<= 5;
	SIDL   = temp;
	
	temp = Filter >> 16;
	temp &= 0x03;
	
	SIDL |= temp;
	
	EID8 = Filter >> 8;
	EID0 = Filter;
	
	MCP2515_SendByte(RXF0SIDH, SIDH);
	MCP2515_SendByte(RXF0SIDL, SIDL | 0x08);
	MCP2515_SendByte(RXF0EID8, EID8);
	MCP2515_SendByte(RXF0EID0, EID0);

	MCP2515_SendByte(RXF1SIDH, SIDH);
	MCP2515_SendByte(RXF1SIDL, SIDL | 0x08);
	MCP2515_SendByte(RXF1EID8, EID8);
	MCP2515_SendByte(RXF1EID0, EID0);
	
	MCP2515_SendByte(RXF2SIDH, SIDH);
	MCP2515_SendByte(RXF2SIDL, SIDL | 0x08);
	MCP2515_SendByte(RXF2EID8, EID8);
	MCP2515_SendByte(RXF2EID0, EID0);

	MCP2515_SendByte(RXF3SIDH, SIDH);
	MCP2515_SendByte(RXF3SIDL, SIDL | 0x08);
	MCP2515_SendByte(RXF3EID8, EID8);
	MCP2515_SendByte(RXF3EID0, EID0);
	
	MCP2515_SendByte(RXF4SIDH, SIDH);
	MCP2515_SendByte(RXF4SIDL, SIDL | 0x08);
	MCP2515_SendByte(RXF4EID8, EID8);
	MCP2515_SendByte(RXF4EID0, EID0);

	MCP2515_SendByte(RXF5SIDH, SIDH);
	MCP2515_SendByte(RXF5SIDL, SIDL | 0x08);
	MCP2515_SendByte(RXF5EID8, EID8);
	MCP2515_SendByte(RXF5EID0, EID0);
	
	SIDH   = Mask >> 21;

	temp   = Mask >> 18;
	temp  &= 0x07;
	temp <<= 5;
	SIDL   = temp;
	
	temp = Mask >> 16;
	temp &= 0x03;
	
	SIDL |= temp;
	
	EID8 = Mask >> 8;
	EID0 = Mask;
	
	MCP2515_SendByte(RXM0SIDH, SIDH);
	MCP2515_SendByte(RXM0SIDL, SIDL); 
	MCP2515_SendByte(RXM0EID8, EID8);
	MCP2515_SendByte(RXM0EID0, EID0);	

	MCP2515_SendByte(RXM1SIDH, SIDH);
	MCP2515_SendByte(RXM1SIDL, SIDL); 
	MCP2515_SendByte(RXM1EID8, EID8);
	MCP2515_SendByte(RXM1EID0, EID0);	
}

/** 
  * @brief  Receive n bytes from MCP2515 RXB0 
  * @param  none 
  * @retval Data:return the effectively data from RXB0 
  */ 
int8_t CAN_RxData0(uint8_t *CAN_RX_Buf, Frame_TypeDef *Frame)
{
	uint8_t  tempdata  = 0; 
	uint32_t CAN_ID    = 0;   
	uint32_t CAN_EID   = 0;  
	uint32_t CAN_SID   = 0; 
	uint32_t CAN_EIDH2 = 0;

	uint8_t  CAN_SIDL = 0; 
	uint8_t  CAN_SIDH = 0; 
	uint8_t  CAN_DLC  = 0;
	uint8_t  IDE = 0;
	uint8_t  i = 0;
	
	tempdata = MCP2515_SPI_CMD(SPI_RX_STATUS); 

	if ((tempdata & 0x40) == 0x40)
	{
		MCP2515_BitModify(CANINTF, 0x01, 0x00);  
		CAN_SIDH = MCP2515_ReadByte(RXB0SIDH);
		CAN_SIDL = MCP2515_ReadByte(RXB0SIDL); 
		CAN_EIDH2 = (uint32_t)(CAN_SIDL & 03);
		CAN_SID = (uint32_t)(CAN_SIDH << 8) | (uint32_t)CAN_SIDL;
		CAN_SID >>= 5; 		 
		CAN_EID  = (uint32_t)(MCP2515_ReadByte(RXB0EID8) << 8);
		CAN_EID |= (uint32_t)MCP2515_ReadByte(RXB0EID0);
		CAN_ID = (CAN_SID << 18) | (CAN_EIDH2 << 16) | CAN_EID;
		CAN_DLC = MCP2515_ReadByte(RXB0DLC) & 0x0F;
		
		IDE = CAN_SIDL & 0x08;
		
		Frame->Type = 0x08 == IDE ? CAN_EXTD : CAN_STD;
		Frame->ID = CAN_ID;
		Frame->DLC = CAN_DLC;
		
		for (i = 0; i < Frame->DLC; i++)
		{		 
			CAN_RX_Buf[i] = MCP2515_ReadByte(RXB0D[i]); 
		}
		
		return 1;
	}
	
	return 0;
}

/** 
  * @brief  Receive n bytes from MCP2515 RXB1 
  * @param  none 
  * @retval Data:return the effectively data from RXB1 
  */ 
int8_t CAN_RxData1(uint8_t *CAN_RX_Buf, Frame_TypeDef *Frame)
{
	uint8_t  tempdata  = 0; 
	uint32_t CAN_ID    = 0;   
	uint32_t CAN_EID   = 0;  
	uint32_t CAN_SID   = 0; 
	uint32_t CAN_EIDH2 = 0;

	uint8_t  CAN_SIDL = 0; 
	uint8_t  CAN_SIDH = 0; 
	uint8_t  CAN_DLC  = 0;
	uint8_t  IDE = 0;
	uint8_t  i = 0;
	
	tempdata = MCP2515_SPI_CMD(SPI_RX_STATUS); 

	if ((tempdata & 0x80) == 0x80)
	{
		MCP2515_BitModify(CANINTF, 0x02, 0x00); 
		CAN_SIDH = MCP2515_ReadByte(RXB1SIDH);
		CAN_SIDL = MCP2515_ReadByte(RXB1SIDL); 
		CAN_EIDH2 = (uint32_t)(CAN_SIDL & 03);
		CAN_SID = (uint32_t)(CAN_SIDH << 8) | (uint32_t)CAN_SIDL;
		CAN_SID >>= 5; 		 
		CAN_EID  = (uint32_t)(MCP2515_ReadByte(RXB1EID8) << 8);
		CAN_EID |= (uint32_t)MCP2515_ReadByte(RXB1EID0);
		CAN_ID = (CAN_SID << 18) | (CAN_EIDH2 << 16) | CAN_EID;
		CAN_DLC = MCP2515_ReadByte(RXB1DLC) & 0x0F;
		
		IDE = CAN_SIDL & 0x08;
		
		Frame->Type = 0x08 == IDE ? CAN_EXTD : CAN_STD;
		Frame->ID = CAN_ID;
		Frame->DLC = CAN_DLC;
		
		for (i = 0; i < Frame->DLC; i++)
		{		 
			CAN_RX_Buf[i] = MCP2515_ReadByte(RXB1D[i]); 
		}
		
		return 1;
	}
	
	return 0;
}

/** 
  * @brief  RXB0_Status
  * @param  none 
  * @retval Data:return the Status from RXB0 
  */ 
int8_t RXB0_Status(void)
{
	uint8_t  tempdata  = 0; 
	
	tempdata = MCP2515_SPI_CMD(SPI_RX_STATUS); 
	
	if ((tempdata & 0x40) == 0x40)
	{
		return 1;
	}
	
	return 0;
}

/** 
  * @brief  RXB1_Status
  * @param  none 
  * @retval Data:return the Status from RXB1
  */ 
int8_t RXB1_Status(void)
{
	uint8_t  tempdata  = 0;

	tempdata = MCP2515_SPI_CMD(SPI_RX_STATUS); 
	
	if ((tempdata & 0x80) == 0x80)
	{
		return 1;
	}
	
	return 0;
}

/** 
  * @brief  MCP2515_Read_CANINTF
  * @param  none 
  * @retval Data:return the Status from CANINTF
  */ 
int8_t MCP2515_Read_CANINTF(void)
{
	uint8_t dummy = 0;
	
	dummy = MCP2515_ReadByte(CANINTF);
	
	return dummy;
}

/** 
  * @brief  MCP2515_BitModify_CANINTF
  * @param  none 
  * @retval Data:return the Status from CANINTF
  */ 
void MCP2515_BitModify_CANINTF(uint8_t Addr, uint8_t Mask, uint8_t Data)
{
	MCP2515_BitModify(Addr, Mask, Data);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
