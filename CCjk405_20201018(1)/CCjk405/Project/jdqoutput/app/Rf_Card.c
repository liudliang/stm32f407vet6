//Rf_Card.c

#include "main.h"
#include "Card.h"
#include "common.h"
#include "uart.h"

extern void CARD_GetIcSecrtPin(uint8 Type,uint8 *Data);

/* 扇区号 */
typedef enum
{
	SECT_NO_0 = 0,
	SECT_NO_1,
	SECT_NO_2,
	SECT_NO_3,
	SECT_NO_4,
	SECT_NO_5,
	SECT_NO_6,
	SECT_NO_7,
	SECT_NO_8,
	SECT_NO_9,
	SECT_NO_10,
	SECT_NO_11,
	SECT_NO_12,
	SECT_NO_13,
	SECT_NO_14,
	SECT_NO_15,
}RF_SECT_NO;

/* 块号  */
typedef enum
{
	BLOCK_NO_0 = 0,
	BLOCK_NO_1,
	BLOCK_NO_2,
	BLOCK_NO_3,
}RF_BLOCK_NO;

//static CARD_SINGLE_SECT_STRUCT SectTab[10] = {
//{SECT_NO_0,BLOCK_NO_1,STORE_LOG},					/* 扇区0：块1：存储充电记录		密码默认：892551063333 */
//{SECT_NO_0,BLOCK_NO_2,STORE_PIN},					/* 扇区0：块2：存储用户密码 	密码默认：892551063333 */

//{SECT_NO_1,BLOCK_NO_0,STORE_IcNo},				/* 扇区1：块0：存储卡号 			密码默认：892551063333 */
//{SECT_NO_1,BLOCK_NO_1,STORE_LockFg},			/* 扇区1：块1：存储灰锁标志 	密码默认：892551063333 */
//{SECT_NO_1,BLOCK_NO_2,STORE_Money},				/* 扇区1：块2：存储余额   		密码默认：892551063333 */

//{SECT_NO_2,BLOCK_NO_0,STORE_KEY},					/* 扇区2：块0：存储KEY密钥    密码默认：601552985678 */
//{SECT_NO_2,BLOCK_NO_1,STORE_BakMoney},		/* 扇区2：块1：存储备份余额 	密码默认：601552985678 */
//};


//CARD_PWD_STRUCT GetPinWithStoreType(uint8 Type)
//{
//	CARD_PWD_STRUCT Data;
//	
//	CARD_GetIcSecrtPin(Type,Data.Pin);
//	Data.SectNo = SectTab[Type].SectNo;
//	Data.BlockNo = SectTab[Type].BlockNo;
//	
//	return Data;
//}
extern uint32 UartHandler[];
/* 校验报文并且发送数据 */
static void CalcSumAndSndData(uint8 ComNo,uint8 *Txbuf,uint8 Txlen)
{
	uint16 DataLen = 0;
	
	DataLen = Txlen - 4;
	Txbuf[1] = DataLen/256;
	Txbuf[2] = DataLen % 256;
	
	Txbuf[Txlen++] = Common_CalculateNOR(Txbuf);
	
	Uart_ClearReadBuf(UartHandler[ComNo]);
	Uart_Write(UartHandler[ComNo],Txbuf,Txlen);
}

/* 校验接收到的数据 */
static BOOL RF_VerfiyRcvDataPack(uint8 *pbuf,uint8 *HeadNo,uint8 length)
{
	uint8 i;
	uint16 DataLen=0;
	
	for(i = 0; i < 4; i++)
	{
		if(pbuf[i] == 0x02)
			break;
	}
	if((length- i) < 4)
	{
		return FALSE;
	}
	
	DataLen = ((uint16)pbuf[i+1])<<8 | pbuf[i+2];
	
	if(pbuf[i+DataLen+3] == 0x03)
	{
		if(Common_CalculateNOR(&pbuf[i]) == pbuf[i+DataLen+4])
		{
			*HeadNo = i+3;
			return TRUE;
		}
	}
	return FALSE;
}

/* 读取卡返回链路数据包 */
static uint8 RF_CardLinkReadFrame(uint8 ComNo,uint8 *pbuf, uint8 MaxSize, uint16 timeout)
{
	uint8 tmpLen = 0;
	uint8 slength = 0;
	uint8 length = 0;
	uint16 count;
  count = 0;
	do
	{
		tmpLen = Uart_Read(UartHandler[ComNo], pbuf, MaxSize - length);
		if (tmpLen > 0 ) 
		{
			pbuf += tmpLen;
			length += tmpLen;
		}
		Delay10Ms(2);	 // 等20ms
		if((slength == length) && (length > 3) ) 
		{
			 break;
		}
		slength = length;	
	}while( count++ < 100);	  // 1000ms超时范围内
	
	return length;
}

/* 处理接收的数据  */
static uint8 RF_deal_Rxdata(uint8 *pbuf,IC_API_DATA *data)
{
	uint8 ret=100;
	
	if(pbuf[0] == 0x34)
	{
		switch(pbuf[1])
		{
			/* 寻卡 */
			case 0x30:
			{
				if(pbuf[2] == 0x59)
					ret = RP_SUCC;	
				else
					ret = RP_NO_IC;
				break;
			}
			/* 校验密码 */
			case 0x32:
			{
				if(pbuf[3] == 0x59)
					ret = RP_SUCC;
				else if(pbuf[3] == 0x33)
					ret = RP_FAIL;
				else
					ret = RP_NO_IC;
				break;
			}
			/* 读取缓冲区 */
			case 0x33:
			{
				if(pbuf[4] == 0x59)
				{
					memcpy(data->RfData,&pbuf[5],16);
					ret = RP_SUCC;
				}
				else 
					ret = RP_FAIL;
				break;
			}
			/* 写缓冲区数据 */
			case 0x34:
			{
				ret = RP_SUCC;
				break;
			}
			/* 增值 */
			case 0x37:
			{
				if(pbuf[4] == 0x59)
					ret = RP_SUCC;
				else
					ret = RP_FAIL;
				break;
			}
			/* 减值 */
			case 0x38:
			{
				if(pbuf[4] == 0x59)
					ret = RP_SUCC;
				else
					ret = RP_FAIL;
				break;
			}
			default:
				break;
		}
	}
	else if(pbuf[0] == 0x35)
	{
		if(pbuf[1] == 0x33)
			ret = RP_FAIL;
	}
	return ret;
}

/* 处理RF卡通信命令 */
static uint8 RF_ProceedCardLinkCmd(uint8 ComNo,IC_API_DATA *data)
{
	uint8 Rxbuf[80] = { 0 },HeadNo=0,RcvLen=0;

	/* 接收数据 */
	RcvLen = RF_CardLinkReadFrame(ComNo,Rxbuf, 80, CARD_COMM_TIMEOUT);
	
	if((RcvLen > 4) && (RF_VerfiyRcvDataPack(Rxbuf,&HeadNo,RcvLen) == TRUE) )
	{
		return RF_deal_Rxdata(&Rxbuf[HeadNo],data);
	}
	else
	{
		return RP_ERR;
	}
}

/* 发送寻卡命令 */
static void RF_SendCheckIcCmd(uint8 ComNo)
{
	uint8 Txbuf[50],index=0;
	
	Txbuf[index++] = 0x02;/* STX */
	Txbuf[index++] = 0;
	Txbuf[index++] = 0;/* 长度*/
	Txbuf[index++] = 0x34;    /* cmd */
	Txbuf[index++] = 0x30;		 /* cmd Para */
	Txbuf[index++] = 0x03;		 /* ETX */
	CalcSumAndSndData(ComNo,Txbuf,index);
}

/* 校验密码 */
static void RF_SendVerfiyPwdCmd(uint8 ComNo,CARD_PWD_STRUCT *Pwd)
{
	uint8 Txbuf[50],index=0;
	
	Txbuf[index++] = 0x02;/* STX */
	Txbuf[index++] = 0;
	Txbuf[index++] = 0;/* 长度*/
	Txbuf[index++] = 0x34;    /* cmd */
	Txbuf[index++] = 0x32;		 /* cmd Para */
	Txbuf[index++] = Pwd->SectNo;/* 扇区号 */
	Txbuf[index++] = Pwd->Pin[0];
	Txbuf[index++] = Pwd->Pin[1];
	Txbuf[index++] = Pwd->Pin[2];
	Txbuf[index++] = Pwd->Pin[3];
	Txbuf[index++] = Pwd->Pin[4];
	Txbuf[index++] = Pwd->Pin[5];
	Txbuf[index++] = 0x03;		 /* ETX */
	CalcSumAndSndData(ComNo,Txbuf,index);
}

/* 读取扇区、块数据 */
static void RF_SendReadIcSectCmd(uint8 ComNo,CARD_PWD_STRUCT *Pwd)
{
	uint8 Txbuf[50],index=0;
	
	Txbuf[index++] = 0x02;/* STX */
	Txbuf[index++] = 0;
	Txbuf[index++] = 0;/* 长度*/
	Txbuf[index++] = 0x34;    /* cmd */
	Txbuf[index++] = 0x33;		 /* cmd Para */
	Txbuf[index++] = Pwd->SectNo;  /* 扇区号 */
	Txbuf[index++] = Pwd->BlockNo; /* 块号 */
	Txbuf[index++] = 0x03;		 /* ETX */
	CalcSumAndSndData(ComNo,Txbuf,index);
}

/* 写卡内扇区和块缓冲区 */
static void RF_SendWriteIcSectCmd(uint8 ComNo,CARD_PWD_STRUCT *Pwd,IC_API_DATA *data)
{
	uint8 Txbuf[50],index=0;
	
	Txbuf[index++] = 0x02;/* STX */
	Txbuf[index++] = 0;
	Txbuf[index++] = 0;/* 长度*/
	Txbuf[index++] = 0x34;    /* cmd */
	Txbuf[index++] = 0x34;		 /* cmd Para */
	Txbuf[index++] = Pwd->SectNo;  /* 扇区号 */
	Txbuf[index++] = Pwd->BlockNo; /* 块号 */
	memcpy(&Txbuf[index],data,sizeof(IC_API_DATA));
	index += sizeof(IC_API_DATA);
	Txbuf[index++] = 0x03;		 /* ETX */
	CalcSumAndSndData(ComNo,Txbuf,index);
}

/* 增值命令 */
static void RF_SendAddVaule(uint8 ComNo,CARD_PWD_STRUCT *Pwd,int32 Money)
{
	uint8 Txbuf[50],index=0;
	
	Txbuf[index++] = 0x02;/* STX */
	Txbuf[index++] = 0;
	Txbuf[index++] = 0;/* 长度*/
	Txbuf[index++] = 0x34;    /* cmd */
	Txbuf[index++] = 0x37;		 /* cmd Para */
	Txbuf[index++] = Pwd->SectNo;  /* 扇区号 */
	Txbuf[index++] = Pwd->BlockNo; /* 块号 */
	memcpy(&Txbuf[index],&Money,sizeof(Money));
	index += 4;
	Txbuf[index++] = 0x03;		 /* ETX */
	CalcSumAndSndData(ComNo,Txbuf,index);
}

/* 减值命令 */
static void RF_SendMinusVaule(uint8 ComNo,CARD_PWD_STRUCT *Pwd,int32 Money)
{
	uint8 Txbuf[50],index=0;
	
	Txbuf[index++] = 0x02;/* STX */
	Txbuf[index++] = 0;
	Txbuf[index++] = 0;/* 长度*/
	Txbuf[index++] = 0x34;    /* cmd */
	Txbuf[index++] = 0x38;		 /* cmd Para */
	Txbuf[index++] = Pwd->SectNo;  /* 扇区号 */
	Txbuf[index++] = Pwd->BlockNo; /* 块号 */
	memcpy(&Txbuf[index],&Money,sizeof(Money));
	index += 4;
	Txbuf[index++] = 0x03;		 /* ETX */
	CalcSumAndSndData(ComNo,Txbuf,index);
}

/* 寻卡命令 */
uint8 RF_CheckCard(void)
{
	RF_SendCheckIcCmd(CARD_COM);
	return RF_ProceedCardLinkCmd(CARD_COM,0);
}

///* 校验扇区密码 */
//uint8 RF_VerfiySectPwd(uint8 Type,IC_API_DATA *data)
//{
//	CARD_PWD_STRUCT Pwd;
//	Pwd = GetPinWithStoreType(Type);
//	RF_SendVerfiyPwdCmd(CARD_COM,&Pwd);
//	return RF_ProceedCardLinkCmd(CARD_COM,data);
//}




///* 读取扇区数据 */
//uint8 RF_ReadSectData(uint8 Type,IC_API_DATA *Data)
//{
//	CARD_PWD_STRUCT Pwd;
//	Pwd = GetPinWithStoreType(Type);
//	RF_SendReadIcSectCmd(CARD_COM,&Pwd);
//	return RF_ProceedCardLinkCmd(CARD_COM,Data);
//}

///* 写扇区数据 */
//uint8 RF_WriteSectData(uint8 Type,IC_API_DATA *Data)
//{
//	CARD_PWD_STRUCT Pwd;
//	Pwd = GetPinWithStoreType(Type);
//	RF_SendWriteIcSectCmd(CARD_COM,&Pwd,Data);
//	return RF_ProceedCardLinkCmd(CARD_COM,0);
//}

/* 增值命令 */
//uint8 RF_AddVaule(uint8 Type,uint32 Money)
//{
//	CARD_PWD_STRUCT Pwd;
//	Pwd = GetPinWithStoreType(Type);
//	
//	RF_SendAddVaule(CARD_COM,&Pwd,Money);
//	return RF_ProceedCardLinkCmd(CARD_COM,0);
//}

/* 减值命令 */
//uint8 RF_MinusVaule(uint8 Type,uint32 Money)
//{
//	CARD_PWD_STRUCT Pwd;
//	Pwd = GetPinWithStoreType(Type);
//	
//	RF_SendMinusVaule(CARD_COM,&Pwd,Money);
//	return RF_ProceedCardLinkCmd(CARD_COM,0);
//}


/*读取一个扇区的一块内容*/
uint8 RF_GetSectorData(uint8 ComNo,CARD_PWD_STRUCT *Pwd,uint8 *data) //16byte
{
	  RF_SendVerfiyPwdCmd(CARD_COM,Pwd);
	  if( RP_SUCC == RF_ProceedCardLinkCmd(CARD_COM,(void *)data)) {
			RF_SendReadIcSectCmd(CARD_COM,Pwd);
			/*读取扇区数据*/
			if( RP_SUCC == RF_ProceedCardLinkCmd(CARD_COM,(void *)data) ){
				 return RP_SUCC;
			}
		}
		return RP_ERR;
		
}

/*RF减值*/
 /*卡内余额为负数时 扣钱不能是负数或0，否则无法返回正确*/
uint8 RF_DecMoney(uint8 ComNo,CARD_PWD_STRUCT *Pwd,int32 Money)
{
	  RF_SendVerfiyPwdCmd(ComNo,Pwd);
	  Delay10Ms(2);	 // 等20ms
	  if( RP_SUCC == RF_ProceedCardLinkCmd(ComNo,(void *)NULL)) {
			
			  RF_SendMinusVaule(ComNo,Pwd,Money);
			  Delay10Ms(3);	 // 等30ms
				return RF_ProceedCardLinkCmd(ComNo,0);
		}
		return RP_ERR;
}

/*检卡*/
uint8 RF_CheckChgCard(uint8 ComNo)
{
	RF_SendCheckIcCmd(ComNo);
	Delay10Ms(2);	 // 等20ms
	return RF_ProceedCardLinkCmd(ComNo,0);
	
}

/*读取一个扇区的一块内容*/
uint8 RF_ReadSectorData(uint8 ComNo,CARD_PWD_STRUCT *Pwd,uint8 *data) //16byte
{
	  RF_SendVerfiyPwdCmd(ComNo,Pwd);
	  Delay10Ms(2);	 // 等20ms
	  if( RP_SUCC == RF_ProceedCardLinkCmd(ComNo,(void *)data)) {
			RF_SendReadIcSectCmd(ComNo,Pwd);
			Delay10Ms(2);	 // 等20ms
			/*读取扇区数据*/
			if( RP_SUCC == RF_ProceedCardLinkCmd(ComNo,(void *)data) ){
				 return RP_SUCC;
			}
		}
		return RP_ERR;
}

/*写一个扇区的一块内容*/
uint8 RF_WriteSectorData(uint8 ComNo,CARD_PWD_STRUCT *Pwd,uint8 *data) //16byte
{
	  RF_SendVerfiyPwdCmd(ComNo,Pwd);
	  Delay10Ms(2);	 // 等20ms
	  if( RP_SUCC == RF_ProceedCardLinkCmd(ComNo,(void *)data)) {
			RF_SendWriteIcSectCmd(ComNo,Pwd,(void *)data);
			Delay10Ms(2);	 // 等20ms
			/*读取扇区数据*/
			if( RP_SUCC == RF_ProceedCardLinkCmd(ComNo,(void *)data) ){
				 return RP_SUCC;
			}
		}
		return RP_ERR;
}

