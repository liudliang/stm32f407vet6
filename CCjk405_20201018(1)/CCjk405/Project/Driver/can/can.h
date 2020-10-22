/****************************************Copyright (c)**************************************************
        深圳奥耐电气技术有限公司
****************************************************************************************************/
#ifndef _CAN_H
#define _CAN_H
    

#include "stm32f4xx_can.h"
#include "config.h"
#include "queue.h"

#define CAN_ERROR                      -1

#define CAN_1	                      0	     //can1
#define CAN_2	                      1 	   //can2
//patli 20200318 #define CAN_3	                      2 	   //can3

#define CAN_DEVICE_NUMS             2 //patli 20200318   3	     /*numbers of can*/

#define RCC_APB2Periph_GPIO_CAN1    RCC_APB2Periph_GPIOA
#define GPIO_CAN1                   GPIOA  
#define GPIO_Pin_CAN1_RX            GPIO_Pin_11
#define GPIO_Pin_CAN1_TX            GPIO_Pin_12
 
#define RCC_APB2Periph_GPIO_CAN2    RCC_APB2Periph_GPIOB
#define GPIO_CAN2                   GPIOB  
#define GPIO_Pin_CAN2_RX            GPIO_Pin_5
#define GPIO_Pin_CAN2_TX            GPIO_Pin_6

/*CAN LED*/
typedef enum 
{
  CANRX1 = 0,
  CANTX1,
  CANRX2,
  CANTX2,
} CANLED_TypeDef;

#define CAN_TYPE_ALL_RCV	       1          /*CAN 全接收*/

#define	CAN_FRAME_STANDARD	     0          /*FF:standard frame type*/
#define	CAN_FRAME_EXTERNED	     1          /*FF:externed frame type*/
#define CAN_DATA_TYPE_FRAME      0          /*RTR:data frame*/ 
#define CAN_REMOTE_TYPE_FRAME    1          /*RTR:REMOTE frame*/

#define CAN_FITER_NO             14         //CAN fiter no

/** 
* @brief  CAN message structure definition  
*/
typedef struct
{
	union
	{
		uint32 StdId;   /*!< Specifies the standard identifier.
					            This parameter can be a value between 0 to 0x7FF. */

		uint32 ExtId;   /*!< Specifies the extended identifier.
					            This parameter can be a value between 0 to 0x1FFFFFFF. */
	}Id;
	struct
	{
		uint8 FF:1;     /*!< Specifies the type of identifier for the message that will be transmitted.
						          This parameter can be a value of @ref CAN_identifier_type */

		uint8 RTR:1;    /*!< Specifies the type of frame for the message that will be transmitted.
										  This parameter can be a value of @ref CAN_remote_transmission_request */
	}FrmDef;		      /*frame define*/

	uint8 DLC;        /*!< Specifies the length of the frame that will be transmitted.
				              This parameter can be a value between 0 to 8 */

	uint8 Data[8];    /*!< Contains the data to be transmitted. It ranges from 0 to 0xFF. */
}CAN_MSG,*P_CAN_MSG;

#define MAX_CAN_RCV_PK_BUF_NUM	   30 	/*can rcv package numbers in buffer*/
#define MAX_CAN_SEND_PK_BUF_NUM	   10	  /*can send package numbers in buffer*/

#define MAX_CAN_SEND_BUF_LENTH     (sizeof(DataQueue) + MAX_CAN_SEND_PK_BUF_NUM * sizeof(CAN_MSG))   /*can send buf*/
#define MAX_CAN_RCV_BUF_LENTH      (sizeof(DataQueue) + MAX_CAN_RCV_PK_BUF_NUM  * sizeof(CAN_MSG))   /*can rcv buf*/

/* Device Initialization Structure */
typedef struct 
{
	void*	 base;	                                 //register base address
	uint8	 devicType;		                           //CAN_TYPE_ALL_RCV master ,=CAN_TPYE_FILTE slave
	uint32 defBuardRate;                           //默认波特率 
} STM32F40x_SIO_CAN_PARAM;

typedef struct 
{
	uint8 	sendBuf[MAX_CAN_SEND_BUF_LENTH];         //send    buf
	uint8	  rcvBuf[MAX_CAN_RCV_BUF_LENTH];           //receive buf
	uint8	  deviceOpenFlag;	                         //device flag opened
	uint8	  devicType;		                           //CAN_TYPE_ALL_RCV master ,=CAN_TPYE_FILTE slave*/
 	uint8   frameType;		                           //CAN_FRAME_STANDARD= standard identifiy , CAN_FRAME_EXTERNED= extended identifiy*/
	void    *base;	                                 //register base address
	uint32  baudRate;                                //baud rate 
	uint32  canID;						                       //can ID:   EID[28-0]  IDE   RTR  保留 D[31-3]	  D[2]	D[1] D[0]
	uint32	canMaskId;					                     //can mask ID: EID[28-0]  IDE   RTR  保留 D[31-3]	  D[2]	D[1] D[0]
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;	 //can filter init structure
	CAN_InitTypeDef        CAN_InitStructure;	       //can init structure
}STM32F40x_CAN_CHAN, *STM32F40x_CAN_PCHAN;

extern int32 Can_Open(uint32 canId);    /*open the CAN,id is canId*/
extern int32 Can_Close(int32 devDesc);  /*close can device. the devDesc is value of CAN_Open(uint32 canId) return */
extern int32 Can_Write(int32 devDesc, CAN_MSG* msg);
extern int32 Can_Read(int32 devDesc, CAN_MSG* buf, uint32 timeOut);

extern void Can_ClearReadBuff(int devDesc);

#endif /*_CAN_H*/
