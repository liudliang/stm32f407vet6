/****************************************Copyright (c)**************************************************
        深圳奥耐电气技术有限公司
				
***FILE：can.c
****************************************************************************************************/
#include "can.h"
#include "gpio.h"
#include "MCP2515.h"
#include "Adebug.h"
#include "ChgData.h"
#include "TaskBackComm.h"

#define CAN3                ((CAN_TypeDef *)0x00000003)

/* CAN defualt parameter */
static STM32F40x_SIO_CAN_PARAM CANParameter[CAN_DEVICE_NUMS] = 
{
	{(void *)CAN1, CAN_TYPE_ALL_RCV, 250000},
	{(void *)CAN2, CAN_TYPE_ALL_RCV, 125000},
};

static STM32F40x_CAN_CHAN CANChan[CAN_DEVICE_NUMS] = { 0 };

static BOOL CANDevDefInit(STM32F40x_CAN_CHAN *pChan);
static void CanTxMsgUpConverDown(CanTxMsg* txmsg,CAN_MSG* msg);
static void CanRxMsgDownConverUp(CAN_MSG *msg, CanRxMsg *rxmsg);

int32 CanHander[CAN_DEVICE_NUMS] = { 0 };

#define BMS1_CANID  0x000056F4
#define BMS1_MASKID 0x0000FFFF

#define HELI_BMS1_CANID  0x180000F4      //合力
#define HELI_BMS1_MASKID 0xFF0000FF

#define BMS2_CANID  0x06000003
#define BMS2_MASKID 0x1FF00007

#define BMS3_CANID  0x06000003
#define BMS3_MASKID 0x1FF00007
 
/****************************************************************************************
** 函数名称: void CANLED_Toggle(CANLED_TypeDef no)
** 功能描述: CAN通信指示
** 输　入: 
** 输　出: 无
** 全局变量:
** 描述:   写对应输出值
******************************************************************************************/
static void CANLED_Toggle(CANLED_TypeDef no)
{
	if (CANRX1 == no)
	{
		LEDToggle(LED2);
	}
	else if (CANRX2 == no)
	{
		LEDToggle(LED3);
	}
}

/*************************************************
CanPortInit - initialize  a  CAN port and clock.

writer:rm

RETURN: N
*************************************************/
static void CanPortAndClockInit(STM32F40x_CAN_CHAN* pChan)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	CAN_TypeDef* pbase = (CAN_TypeDef *)pChan->base;
	
	if(CAN1 == pbase)
	{
		/* Enable GPIO clock */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		
		/* Enable CAN clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
		
		/* Connect CAN pins to AF */
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_CAN1);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_CAN1);
		
		/* Configure CAN RX and TX pins */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	}
	else if(CAN2 == pbase)
	{
		/* Enable GPIO clock */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		
		/* CAN configuration ********************************************************/
		/* Enable CAN clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
		
		/* Connect CAN pins to AF */
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_CAN2);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_CAN2);

		/* Configure CAN RX and TX pins */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_12;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	}
}

/*************************************************
CanIntInit - initialize  a  CAN INT.

writer:rm

RETURN: N
*************************************************/
static void CanIntInit(STM32F40x_CAN_CHAN* pChan)
{
	NVIC_InitTypeDef  NVIC_InitStructure;
	CAN_TypeDef* pbase = (CAN_TypeDef *)pChan->base;
	
	if (CAN1 == pbase)
	{
		NVIC_InitStructure.NVIC_IRQChannel = CAN1_TX_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x2;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x3;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		CAN_ClearITPendingBit(pbase, CAN_IT_TME);
		CAN_ITConfig(pbase, CAN_IT_TME, ENABLE);

		CAN_ITConfig(pbase, CAN_IT_FMP0 | CAN_IT_FMP1, ENABLE);
	}
	else if (CAN2 == pbase)
	{
		NVIC_InitStructure.NVIC_IRQChannel = CAN2_TX_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x3;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x4;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		CAN_ClearITPendingBit(pbase, CAN_IT_TME);
		CAN_ITConfig(pbase, CAN_IT_TME, ENABLE);

		CAN_ITConfig(pbase, CAN_IT_FMP0 | CAN_IT_FMP1, ENABLE);
	}
}

/*************************************************
CANDevDefInit - initialize  a  CAN .

writer:rm

RETURN: N
*************************************************/
static BOOL CANDevDefInit(STM32F40x_CAN_CHAN* pChan)
{
	CAN_TypeDef* pbase = (CAN_TypeDef *)pChan->base;
	uint8 fitercnt = 0;
	PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	
	/*port and clock init*/
	CanPortAndClockInit(pChan);

	/* CAN register init */
	CAN_DeInit(pbase);
	/* CAN  init */
	CAN_StructInit(&pChan->CAN_InitStructure);
		
	/* CAN cell init */
  pChan->CAN_InitStructure.CAN_TTCM = DISABLE;
  pChan->CAN_InitStructure.CAN_ABOM = ENABLE;
  pChan->CAN_InitStructure.CAN_AWUM = DISABLE;
  pChan->CAN_InitStructure.CAN_NART = DISABLE;
  pChan->CAN_InitStructure.CAN_RFLM = DISABLE;
  pChan->CAN_InitStructure.CAN_TXFP = DISABLE;
  pChan->CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	
  /* CAN Baudrate = 250 KBps (CAN clocked at 42 MHz) */
  pChan->CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
  pChan->CAN_InitStructure.CAN_BS1 = CAN_BS1_12tq;
  pChan->CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
  
  if(CAN1 == pbase)
  {
		if(BMS_HELI == BackCOMM->agreetype)
		{	
			pChan->CAN_InitStructure.CAN_Prescaler = 16;
		}
		else
		{
			pChan->CAN_InitStructure.CAN_Prescaler = 8;
		}
  }
  else
  {
  	pChan->CAN_InitStructure.CAN_Prescaler = 16;
  }
	
	CAN_Init(pbase, &pChan->CAN_InitStructure);

	pChan->canID <<= 3;
	pChan->canMaskId <<= 3;
	
	/* CAN filter init */
	if(CAN1 == pbase)
	{	
		pChan->CAN_FilterInitStructure.CAN_FilterNumber = fitercnt;
		pChan->CAN_FilterInitStructure.CAN_FilterMode   = CAN_FilterMode_IdMask;
		pChan->CAN_FilterInitStructure.CAN_FilterScale  = CAN_FilterScale_32bit;
		pChan->CAN_FilterInitStructure.CAN_FilterIdHigh = (pChan->canID >> 16) & 0xffff;  
		pChan->CAN_FilterInitStructure.CAN_FilterIdLow  = pChan->canID & 0xffff;	
		pChan->CAN_FilterInitStructure.CAN_FilterMaskIdHigh = (pChan->canMaskId >> 16) & 0xffff;		
		pChan->CAN_FilterInitStructure.CAN_FilterMaskIdLow  = pChan->canMaskId & 0xffff;	
		pChan->CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;
		pChan->CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	}
	else if(CAN2 == pbase)
	{
		pChan->CAN_FilterInitStructure.CAN_FilterNumber = CAN_FITER_NO + fitercnt;
		pChan->CAN_FilterInitStructure.CAN_FilterMode   = CAN_FilterMode_IdMask;
		pChan->CAN_FilterInitStructure.CAN_FilterScale  = CAN_FilterScale_32bit;
		pChan->CAN_FilterInitStructure.CAN_FilterIdHigh = (pChan->canID >> 16) & 0xffff;  
		pChan->CAN_FilterInitStructure.CAN_FilterIdLow  = pChan->canID & 0xffff;	
		pChan->CAN_FilterInitStructure.CAN_FilterMaskIdHigh = (pChan->canMaskId >> 16) & 0xffff;		
		pChan->CAN_FilterInitStructure.CAN_FilterMaskIdLow  = pChan->canMaskId & 0xffff;	
		pChan->CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;
		pChan->CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	}
	
	CAN_FilterInit(&pChan->CAN_FilterInitStructure); 
	
	/*INT set */
	CanIntInit(pChan);
	
	return TRUE;
}

/*************************************************
CAN_Open - open can device.

writer:rm

description:

RETURN: device dirscription if successful,or 0
*************************************************/
int32 Can_Open(uint32 deviceId)
{
	PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	
	switch(deviceId)
	{
		case CAN_1:
			if (FALSE == CANChan[deviceId].deviceOpenFlag)
			{
				CANChan[deviceId].deviceOpenFlag = TRUE;	
				CANChan[deviceId].devicType      = CANParameter[deviceId].devicType;
				CANChan[deviceId].frameType      = CAN_FRAME_EXTERNED;
				CANChan[deviceId].baudRate       = CANParameter[deviceId].defBuardRate;
				CANChan[deviceId].base           = CANParameter[deviceId].base; 
				if(BMS_HELI == BackCOMM->agreetype)
				{		
					CANChan[deviceId].baudRate       = 125000;
					CANChan[deviceId].canID          = HELI_BMS1_CANID;  
					CANChan[deviceId].canMaskId      = HELI_BMS1_MASKID; 
				}
				else
				{
					CANChan[deviceId].canID          = BMS1_CANID;  
					CANChan[deviceId].canMaskId      = BMS1_MASKID; 
				}					
				
				QueueCreate(CANChan[deviceId].rcvBuf,  MAX_CAN_RCV_BUF_LENTH,  sizeof(CAN_MSG), 0, 0);
				QueueCreate(CANChan[deviceId].sendBuf, MAX_CAN_SEND_BUF_LENTH, sizeof(CAN_MSG), 0, 0);

				CANDevDefInit(&CANChan[deviceId]);

				return (int32)&CANChan[deviceId];
			}
			
			break;
		
		case CAN_2:
			if (FALSE == CANChan[deviceId].deviceOpenFlag)
			{
				CANChan[deviceId].deviceOpenFlag = TRUE;	
				CANChan[deviceId].devicType      = CANParameter[deviceId].devicType;
				CANChan[deviceId].frameType      = CAN_FRAME_EXTERNED;
				CANChan[deviceId].baudRate       = CANParameter[deviceId].defBuardRate;
				CANChan[deviceId].base           = CANParameter[deviceId].base;
				CANChan[deviceId].canID          = BMS2_CANID;
				CANChan[deviceId].canMaskId      = BMS2_MASKID;
				
				QueueCreate(CANChan[deviceId].rcvBuf,  MAX_CAN_RCV_BUF_LENTH,  sizeof(CAN_MSG), 0, 0);
				QueueCreate(CANChan[deviceId].sendBuf, MAX_CAN_SEND_BUF_LENTH, sizeof(CAN_MSG), 0, 0);

				CANDevDefInit(&CANChan[deviceId]);

				return (int32)&CANChan[deviceId];
			}
			
			break;		
			
		default:
			break;
	}
	
	return 0;
}

/*************************************************
CAN_Close - close can device.

writer:rm

description:

RETURN: TURE if successful,or FALSE
*************************************************/
int32 Can_Close(int32 devDesc)
{
	uint8 ret = FALSE;
	
	STM32F40x_CAN_CHAN *pChan = (STM32F40x_CAN_CHAN *)devDesc;
	
	if ((NULL != pChan) && (TRUE == pChan->deviceOpenFlag))
	{
		pChan->deviceOpenFlag = FALSE;
		
		CAN_ITConfig(pChan->base, CAN_IT_FMP0 | CAN_IT_FMP1, DISABLE);
		CAN_ITConfig(pChan->base, CAN_IT_BOF|CAN_IT_EPV, DISABLE);
		
		ret = TRUE;
	}
	
	return ret;	
}

/*************************************************
Can_Write - write data to Can

writer:rm
	
RETURN: data number writed,-1 error
*************************************************/
int32 Can_Write(int32 devDesc, CAN_MSG *msg)
{	
	CanTxMsg txmessage = { 0 };
	CAN_MSG Qmsg = { 0 };
	
	uint8_t mp2515TxMsg[8] = { 0x0 };
	Frame_TypeDef mp2515IDFrame = { 0 };
	
	int32 ret = 0;

	STM32F40x_CAN_CHAN *pChan = (STM32F40x_CAN_CHAN*)devDesc;

	if ((NULL == pChan) || (FALSE == pChan->deviceOpenFlag))
	{
		return 0;
	}

	ret = QueueWrite(pChan->sendBuf, (uint8*)msg);
	
	if (ret == QUEUE_OK)
	{
		QueueRead((uint8 *)&Qmsg, pChan->sendBuf);
		
		if ((CAN1 == pChan->base) || (CAN2 == pChan->base))
		{
			CAN_ITConfig(pChan->base, CAN_IT_TME, ENABLE);

			CanTxMsgUpConverDown(&txmessage, &Qmsg);
			CAN_Transmit(pChan->base, &txmessage);
		}
		
		ret =  sizeof(CAN_MSG);
	}
	
	return ret;
}

/*************************************************
Can_Read - read data from Can

writer:rm

note:
	the function will be pend when no data 
	
RETURN: data byte numbers readed
*************************************************/
int32 Can_Read(int devDesc,	CAN_MSG* buf,	uint32 timeOut)
{
	static uint8 count = 0;
	int32 ret = 0;
	
	STM32F40x_CAN_CHAN *pChan = (STM32F40x_CAN_CHAN*)devDesc;
	
	if((NULL == pChan) || (FALSE  == pChan->deviceOpenFlag))
	{
		return ERROR;
	}

	if (QueueRead((uint8*)buf, (void *)pChan->rcvBuf)==QUEUE_OK)
	{
		ret = sizeof(CAN_MSG);
	}

	if (sizeof(CAN_MSG) == ret)
	{
		count++;
	}

	if (count >= 10)
	{
		count = 0;
		
		if (CAN1 == pChan->base)
		{
			CANLED_Toggle(CANRX1);
		}
		else if (CAN2 == pChan->base)
		{
			CANLED_Toggle(CANRX2);
		}
	}
	
	return ret;		
}

/*************************************************
Can_ClearReadBuff - i.

writer:

RETURN: N
*************************************************/
void Can_ClearReadBuff(int devDesc)
{
	uint8 cnt,rtn;
	CAN_MSG buf;
	cnt = 0;
	
	while(cnt < 10){
     rtn = Can_Read(devDesc,&buf,0) ;   //读可能存在的残余数据
     if(0 == rtn){
         break;
     }
     cnt++;
   }
	 
}

/*************************************************
CanIntInit - initialize  a  CAN INT.

writer:rm

RETURN: N
*************************************************/
static void CanRxMsgDownConverUp(CAN_MSG *msg, CanRxMsg *rxmsg)
{
	uint32 i = 0;
	
	if (rxmsg->IDE == CAN_ID_STD)
	{
		msg->Id.StdId = rxmsg->StdId;
		msg->FrmDef.FF = CAN_FRAME_STANDARD;
	}
	else
	{
		msg->Id.ExtId = rxmsg->ExtId;
		msg->FrmDef.FF = CAN_FRAME_EXTERNED;
	}
	
	if (rxmsg->RTR==CAN_RTR_DATA)
	{
		msg->FrmDef.RTR = CAN_DATA_TYPE_FRAME;
	}
	else
	{
		msg->FrmDef.RTR = CAN_REMOTE_TYPE_FRAME;
	}
	
	msg->DLC = rxmsg->DLC;
	
	for (i = 0; i < 8; i++)
	{
		msg->Data[i] = rxmsg->Data[i];
	}
}

/*************************************************
CanIntInit - initialize  a  CAN INT.

writer:rm

RETURN: N
*************************************************/
static void CanTxMsgUpConverDown(CanTxMsg *txmsg, CAN_MSG *msg)
{
	uint32 i = 0;
	
	if (msg->FrmDef.FF == CAN_FRAME_STANDARD)
	{
		txmsg->StdId = msg->Id.StdId;
		txmsg->IDE = CAN_ID_STD;
	}
	else
	{
		txmsg->ExtId = msg->Id.ExtId;
		txmsg->IDE = CAN_ID_EXT;
	}

	if (msg->FrmDef.RTR==CAN_DATA_TYPE_FRAME)
	{
		txmsg->RTR = CAN_RTR_DATA;
	}
	else
	{
		txmsg->RTR = CAN_RTR_REMOTE;
	}
	
	txmsg->DLC = msg->DLC;
	
	for (i = 0; i < 8; i++)
	{
		txmsg->Data[i] = msg->Data[i];
	}
}

/*************************************************
CanIntInit - initialize  a  CAN INT.

writer:rm

RETURN: N
*************************************************/
static void Can_Rx_Deal(CAN_TypeDef* CANx, uint8_t FIFONumber)
{
	CanRxMsg rxmessage = { 0 };
	CAN_MSG msg = { 0 };
	STM32F40x_CAN_CHAN *pCan = NULL;
	
	int32 i = 0;
	
	rxmessage.IDE = CAN_Id_Extended;
	CAN_Receive(CANx, FIFONumber, &rxmessage);
	
	for (i = 0; i < CAN_DEVICE_NUMS; i++)
	{	
		if (((CAN_TypeDef *)CANChan[i].base == CANx) && (TRUE == CANChan[i].deviceOpenFlag))
		{
			pCan = &CANChan[i];
			break;
		}
	}
	
	if (NULL == pCan) 
	{
		return;
	}
	
	CanRxMsgDownConverUp(&msg, &rxmessage);
	QueueWriteInInt(pCan->rcvBuf, (uint8*)&msg);
}

/*************************************************
CanIntInit - initialize  a  CAN INT.

writer:rm

RETURN: N
*************************************************/
static void Can_Tx_Deal(CAN_TypeDef* CANx)
{
	CanTxMsg txmessage = { 0 };
	CAN_MSG msg = { 0 };
	STM32F40x_CAN_CHAN *pCan = NULL;
	
	int32 i = 0;
	
	for (i = 0; i < CAN_DEVICE_NUMS; i++)
	{	
		if (((CAN_TypeDef *)CANChan[i].base == CANx) && (TRUE == CANChan[i].deviceOpenFlag))
		{
			pCan = &CANChan[i];
			break;
		}
	}
	
	if (NULL == pCan) 
	{
		return;
	}
	
	if (QueueReadInInt((uint8*)&msg, (void*)CANChan[i].sendBuf) == QUEUE_OK)
	{
		CanTxMsgUpConverDown(&txmessage, &msg);
		CAN_Transmit(CANx, &txmessage);
	}
	else
	{
		CAN_ITConfig(CANChan[i].base, CAN_IT_TME, DISABLE);
	}
}

/*************************************************
CanIntInit - initialize  a  CAN INT.

writer:rm

RETURN: N
*************************************************/
void CAN1_TX_IRQHandler(void)
{
	OS_CPU_SR  cpu_sr;
	OS_ENTER_CRITICAL();                                       // Tell uC/OS-II that we are starting an ISR
	OSIntNesting++;
	OS_EXIT_CRITICAL();
	
	if (CAN_GetITStatus(CAN1, CAN_IT_TME) != RESET)
	{
		CAN_ClearITPendingBit(CAN1, CAN_IT_TME);
		
		Can_Tx_Deal(CAN1);
		
		setbit(can_status, 0);  //tx status set
	}
	
	OSIntExit(); 
}

/*************************************************
CanIntInit - initialize  a  CAN INT.

writer:rm

RETURN: N
*************************************************/
void CAN1_RX0_IRQHandler(void)
{
	OS_CPU_SR  cpu_sr;
	OS_ENTER_CRITICAL();                                       // Tell uC/OS-II that we are starting an ISR
	OSIntNesting++;
	
	OS_EXIT_CRITICAL();
	
	Can_Rx_Deal(CAN1, CAN_FIFO0);
	
	setbit(can_status, 4);	//rx status set
	
	OSIntExit(); 
}

/*************************************************
CanIntInit - initialize  a  CAN INT.

writer:rm

RETURN: N
*************************************************/
void CAN1_RX1_IRQHandler(void)
{
	OS_CPU_SR  cpu_sr;
	OS_ENTER_CRITICAL();                                       // Tell uC/OS-II that we are starting an ISR
	OSIntNesting++;
	
	OS_EXIT_CRITICAL();
	
	Can_Rx_Deal(CAN1, CAN_FIFO1);
	
	setbit(can_status, 4);	//rx status set
	
	OSIntExit(); 
}

/*************************************************
CanIntInit - initialize  a  CAN INT.

writer:rm

RETURN: N
*************************************************/
void CAN2_TX_IRQHandler(void)
{
	OS_CPU_SR  cpu_sr;
	OS_ENTER_CRITICAL();                                       // Tell uC/OS-II that we are starting an ISR
	OSIntNesting++;
	OS_EXIT_CRITICAL();
	
	if (CAN_GetITStatus(CAN2, CAN_IT_TME) != RESET)
	{
		CAN_ClearITPendingBit(CAN2, CAN_IT_TME);
		
		Can_Tx_Deal(CAN2);
		
		setbit(can_status, 1);  //tx status set
	}
	
	OSIntExit(); 	
}

/*************************************************
CanIntInit - initialize  a  CAN INT.

writer:rm

RETURN: N
*************************************************/
void CAN2_RX0_IRQHandler(void)
{
	OS_CPU_SR  cpu_sr;
	OS_ENTER_CRITICAL();                                       // Tell uC/OS-II that we are starting an ISR
	OSIntNesting++;
	
	OS_EXIT_CRITICAL();
	
	Can_Rx_Deal(CAN2, CAN_FIFO0);
	
	setbit(can_status, 5);	//rx status set
	
	OSIntExit();
}

/*************************************************
CanIntInit - initialize  a  CAN INT.

writer:rm

RETURN: N
*************************************************/
void CAN2_RX1_IRQHandler(void)
{
 	OS_CPU_SR  cpu_sr;
	OS_ENTER_CRITICAL();                                       // Tell uC/OS-II that we are starting an ISR
	OSIntNesting++;
	
	OS_EXIT_CRITICAL();
	
	Can_Rx_Deal(CAN2, CAN_FIFO1);
	
	setbit(can_status, 5);	//rx status set
	
	OSIntExit(); 
}

/*************************************************
CanIntInit - initialize  a  CAN INT.

writer:rm

RETURN: N
*************************************************/


/*************************************************
CanIntInit - initialize  a  CAN INT.

writer:rm

RETURN: N
*************************************************/


/*************************************************
CanIntInit - initialize  a  CAN INT.

writer:rm

RETURN: N
*************************************************/
void EXTI1_IRQHandler(void)
{
#if 0
	uint8_t dummy = 0;
	uint8_t mp2515RxMsg[8] = { 0x0 };
	Frame_TypeDef mp2515IDFrame = { 0 };
	
	CAN_MSG Qmsg = { 0 };
	CanRxMsg rxmessage = { 0 };

  OS_CPU_SR  cpu_sr;
	OS_ENTER_CRITICAL();                                       // Tell uC/OS-II that we are starting an ISR
	OSIntNesting++;
	OS_EXIT_CRITICAL();

	if (EXTI_GetITStatus(EXTI_Line1) != RESET)
  {
    /* Clear interrupt pending bit */
    EXTI_ClearITPendingBit(EXTI_Line1);

		dummy = MCP2515_Read_CANINTF();
	
		if ((dummy & 0x80) == 0x80)  //报文错误中断标志位
		{
			MCP2515_BitModify_CANINTF(0x2C, 0xff, 0x00);
			MCP2515_Init(CANChan[CAN_3].canID, CANChan[CAN_3].canMaskId);
			OSIntExit();   
			return;
		}
		
		if ((dummy & 0x40) == 0x40)  //唤醒中断标志位
		{
			MCP2515_BitModify_CANINTF(0x2C, 0x40, 0x00);
		}
		
		if ((dummy & 0x20) == 0x20)  //错误中断标志位
		{
			MCP2515_BitModify_CANINTF(0x2C, 0xff, 0x00);
			MCP2515_Init(CANChan[CAN_3].canID, CANChan[CAN_3].canMaskId);
			OSIntExit();   
			return;
		}
		
		if ((dummy & 0x10) == 0x10) //发送缓冲器 2 空中断标志位
		{
			MCP2515_BitModify_CANINTF(0x2C, 0x10, 0x00);
		}
		
		if ((dummy & 0x08) == 0x08) //发送缓冲器 1 空中断标志位
		{
			MCP2515_BitModify_CANINTF(0x2C, 0x08, 0x00);
		}
		
		if ((dummy & 0x04) == 0x04) //发送缓冲器 0 空中断标志位
		{
			MCP2515_BitModify_CANINTF(0x2C, 0x04, 0x00);
		}
		
		if ((dummy & 0x02) == 0x02) //接收缓冲器 1 满中断标志位
		{
			CAN_RxData1(mp2515RxMsg, &mp2515IDFrame);

			MP2515RxMsgDataConver(&rxmessage, mp2515RxMsg, &mp2515IDFrame);
			CanRxMsgDownConverUp(&Qmsg, &rxmessage);
			QueueWriteInInt(CANChan[CAN_3].rcvBuf, (uint8*)&Qmsg);
		}
		
		if ((dummy & 0x01) == 0x01) //接收缓冲器 0 满中断标志位
		{
			CAN_RxData0(mp2515RxMsg, &mp2515IDFrame);
			
			MP2515RxMsgDataConver(&rxmessage, mp2515RxMsg, &mp2515IDFrame);
			CanRxMsgDownConverUp(&Qmsg, &rxmessage);
			QueueWriteInInt(CANChan[CAN_3].rcvBuf, (uint8*)&Qmsg);
		}			
  }
	
	OSIntExit();   
#endif	
}
