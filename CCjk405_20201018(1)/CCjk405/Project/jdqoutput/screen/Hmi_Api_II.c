
/**
  ******************************************************************************
  * @file    Hmi_Api_II.c
  * @author  
  * @version v1.0
  * @date    2021-06-06
  * @brief   此文件为迪文dgus_ii屏幕开发文件（区别于dgus的部分）
  ******************************************************************************
	*/
	
//#include "M_Global.h"
#include "Common.h"
#include "main.h"
#include "Uart.h"
#include "gpio.h"
#include "Hmi_Api.h"
#include "screen.h"
#include <time.h>

#define DGUS_BEEP_REG   0x02   /*蜂鸣器*/
#define DGUS_RTC_REG    0x20   /*RTC*/
#define DGUS_LIGHT_REG  0x01  /*亮度控制 0x00 - 0x40*/

#define DGUS_PICNO_REG  0x03  /*当前图片号*/
#define DGUS_TOUCH_REG  0x06   /*是否有触摸*/

#define BEEP_ON   __BEEP_ON() 
#define BEEP_OFF  __BEEP_OFF() 
#define HARDWARE_BEEP  1

/*触摸屏串口数据发送函数*/
extern int32 Screen_UartWrite(uint8 *buf,	int32 size);
extern int32 Screen_UartRead(uint8 *buf,uint16 size);

/***********************************************************************
** 函数名称: Hmi_PackFrameBegin()
** 功能描述：打包帧头
** 参数:     
**返回值：   
*************************************************************************/
uint8  Hmi_PackFrameBegin(uint8 *buf,uint8 cmd,uint16 addr)
{
	uint8 txlen	=	0;
	
	buf[txlen++] 	=	FRAME_HEAD1;
	buf[txlen++] 	=	FRAME_HEAD2;
	buf[txlen++] 	=	0x00;		     //长度

//--------------------modify	
	if(CMD_WR_REG == cmd)
	{
		buf[txlen++] 	=	cmd+2;             //命令码
		if(PIC_ID_REG == addr)
		{
			buf[txlen++] 	=	0;
			buf[txlen++] 	=	0x84;
			buf[txlen++] 	=	0x5a;
			buf[txlen++] 	=	0x01;			
		}
		else if(DGUS_LIGHT_REG == addr)
		{
			buf[txlen++] 	=	0;
			buf[txlen++] 	=	0x82;		
		}
		else
		{
	
		}		
		
//		buf[txlen++] 	=	addr & 0x00ff;
	}
	else if(CMD_RD_REG == cmd)
	{
		buf[txlen++] 	=	cmd+2;             //命令码
		if(DGUS_PICNO_REG == addr)
		{
			buf[txlen++] 	=	0;
			buf[txlen++] 	=	0x14;			
		}	

		
	}
	else
	{
		buf[txlen++] 	=	cmd;             //命令码
		buf[txlen++] 	=	addr>>8;	//地址=0x0010  ~0x007F
		buf[txlen++] 	=	addr & 0x00ff;
	}	
//--------------------	
       return txlen;
}

/***********************************************************************
** 函数名称: Hmi_PackFrameEnd()
** 功能描述：计算crc,并将结果加到末尾,修改帧长度
** 参数:     
**返回值：   
*************************************************************************/
//extern unsigned short Crc16_Calc_for_screen(unsigned char *updata, unsigned short len);
extern unsigned short Crc16_Calc(unsigned char *updata, unsigned short len);
uint8 Hmi_PackFrameEnd(uint8 *buf,uint8 tlen)
{
	uint16 crc;
	crc = Crc16_Calc(&buf[3],tlen - 3);
	buf[tlen++] = crc >> 8;
	buf[tlen++] = crc & 0xFF;
  buf[2] = tlen - 3;

	return tlen;
}



/***********************************************************************
** 函数名称: Hmi_PackWriteReg()
** 功能描述：打包写屏寄存器的报文
** 参数:     
**返回值：   
*************************************************************************/
uint8 Hmi_PackWriteReg(uint8 *obj,uint8 *src,\
                       uint16 regAdr,uint8  nCmd,uint8 byNum)
{
	uint8 txlen = 0;
	uint8 *ptr = obj ;

	txlen = Hmi_PackFrameBegin(ptr,nCmd,regAdr);
	
	memcpy(&obj[txlen],src,byNum);
	txlen += byNum;

  if(DGUS_TWODIMBARCODE_ADDR_A == regAdr)	 //二维码显示
	{
		obj[txlen++] = 0xff;
		obj[txlen++] = 0xff;
	}
	else
	{
		obj[txlen++] = 0;
	}
	
	txlen = Hmi_PackFrameEnd(ptr,txlen);
	return txlen;
}

/***********************************************************************
** 函数名称: Hmi_WriteOneVar()
** 功能描述：写一个变量寄存器
** 参数:     
**返回值：   
*************************************************************************/
uint8 Hmi_WriteOneVar(uint16 regAdr,uint16 data)
{
	 uint8 txlen;
	 uint8 buf[10];
	 uint8 localdata[2];
	 localdata[0] = data >> 8;
	 localdata[1] = data & 0xff;
	 txlen = Hmi_PackWriteReg(buf,localdata,regAdr,CMD_WR_VAR,2);
	 Screen_UartWrite(buf,txlen);
	 return txlen;
}


/***********************************************************************
** 函数名称: Hmi_WriteMulitVar()
** 功能描述：写多个变量寄存器 < 32
** 参数:     
**返回值：   
*************************************************************************/
uint8 Hmi_WriteMulitVar(uint16 regAdr,uint16 *data,uint16 regNum)
{
	 uint8 txlen;
	 uint8 buf[100];
	 uint8 localdata[64];
	 uint8 i;
	
	 regNum = regNum >32? 32 : regNum;
	
	 for( i = 0 ; i < regNum; i++ )
	 {
	   localdata[i*2+0] = data[i] >> 8;
	   localdata[i*2+1] = data[i] & 0xff;
	 }
	 
	 txlen = Hmi_PackWriteReg(buf,localdata,regAdr,CMD_WR_VAR,2*regNum);
	 Screen_UartWrite(buf,txlen);
	 return txlen;
}


/***********************************************************************
** 函数名称: Hmi_PackgCheck()
** 功能描述：校验收到的屏的包的合法性
** 参数:     
**返回值：   
*************************************************************************/
uint8  Hmi_PackgCheck(uint8 *buf,uint8 rxLen)
 {
 	uint8 i,end,tmplen;
 	uint8 *ptr = buf;
	stoc_u nCrc ;
	uint8 ret = RT_ERROR;
	
	if( rxLen > MIN_FRAME_LEN ) {
		end = rxLen - MIN_FRAME_LEN;
	}else {
		end = 1;
	}

	for( i = 0 ; i  < end; i++)
  	{
		if ( (FRAME_HEAD1 == ptr[i]) && (FRAME_HEAD2 == ptr[i+1]) )
		{
			tmplen = ptr[i+2];
			if(tmplen+i > (rxLen - 3)) 
			{
				return RT_ERROR;
			}

			nCrc.s = Crc16_Calc(&ptr[i+3],tmplen-2);
		
			if( (nCrc.c[1] == ptr[tmplen+1]) && (nCrc.c[0] == ptr[tmplen+2]))
			{
				if( i != 0 )
				{
					memcpy(buf,ptr+i,tmplen+3);
				}
				ret = RT_EOK;
				break;
			}
		}
	}   
	return ret;
 }
 

/***********************************************************************
** 函数名称: Hmi_ClearReg()
** 功能描述：清除指定位置的寄存器
** 参数:     
**返回值：   
*************************************************************************/
uint8 Hmi_ClearReg(uint16 regAdr,uint8 regNum)
{
	uint8 cnt;
	uint8 txlen = 0;
	uint8 buf[64] = {0};
	uint8 *ptr = buf ;

  regNum = regNum > 32? 32:regNum;

	txlen = Hmi_PackFrameBegin(ptr,CMD_WR_VAR,regAdr);

	for(cnt = 0; cnt < regNum ;cnt++ ){
			ptr[txlen++] = 0;
			ptr[txlen++] = 0;
	}

	txlen = Hmi_PackFrameEnd(ptr,txlen);
	
	Screen_UartWrite(buf,txlen);
	return txlen;
}

/***********************************************************************
** 函数名称: Hmi_PackQueryReg()
** 功能描述：打包请求寄存器命令报文
** 参数:     
**返回值：   
*************************************************************************/
uint8 Hmi_PackQueryReg(uint8 *buf,uint16 regAdr,\
                                         uint8 nCmd,uint8 regNum)
{
	uint8_t txlen = 0;
	uint8_t *ptr = buf ;

	txlen = Hmi_PackFrameBegin(ptr,nCmd,regAdr);
	
	ptr[txlen++] = regNum;

	txlen = Hmi_PackFrameEnd(ptr,txlen);
	return txlen;
}



/***********************************************************************
** 函数名称: Hmi_ChangePicture
** 功能描述：切换页面的函数,使用前串口必须打开
** 参数:  buf:缓存  picNO：页面编号   
**返回值：   
*************************************************************************/
void Hmi_ChangePicture(uint8 picNo)
{
	 uint8 len;
	 uint8 buf[32];
	 uint16 page = picNo;
	 Common_Change2Byte(&page);
	 len =  Hmi_PackWriteReg(buf,(uint8*)&page,PIC_ID_REG,CMD_WR_REG,PIC_ID_REG_LEN);
	 Screen_UartWrite(buf,len);
}



/***********************************************************************
** 函数名称: Hmi_ShowText
** 功能描述： 显示文本函数
** 参数:    len,文本长度，最大128个字节
**返回值：   
*************************************************************************/
void Hmi_ShowText(uint16 adr,uint16 len,uint8 *text)
{

	 uint8 buf[200] ;
	 memset(buf,0xff,sizeof(buf));
	 len = len > 180 ? 180 : len;
	 
	 len =  Hmi_PackWriteReg(buf,(uint8*)text,adr,CMD_WR_VAR,len+1);
	 Screen_UartWrite(buf,len);
}


/***********************************************************************
** 函数名称: Hmi_ShutDownKeyBoard
** 功能描述： 显示文本函数
** 参数:    len,文本长度，最大128个字节
**返回值：   
*************************************************************************/
void Hmi_ShutDownKeyBoard(uint8 value)
{
	 uint8 len;
	 uint8 buf[10];
	 len =  Hmi_PackWriteReg(buf,&value,0x4f,CMD_WR_REG,1);
	 Screen_UartWrite(buf,len);
}



/***********************************************************************
** 函数名称: Hmi_ReadMulitVar()
** 功能描述：读多个变量寄存器 < 32
** 参数:     
**返回值：   
*************************************************************************/
uint8 Hmi_ReadMulitVar(uint16 regAdr,uint16 *data,uint16 regNum)
{
	#define MAX_RM_SIZE 100
	 uint8 tlen,cnt,slen,i;
	 uint8 buf[MAX_RM_SIZE];
	 uint8 len,*ptr;
	 uint16 tmp16;
	
	 regNum = regNum >32 ? 32 : regNum;
	 tlen = Hmi_PackQueryReg(buf,regAdr,CMD_RD_VAR,regNum);
	 Screen_UartWrite(buf,tlen);
	 Delay10Ms(2);
	
	 ptr = (uint8 *)buf;
	 tlen = 0;
	 cnt = 0;
	 do {
		len = Screen_UartRead( ptr,MAX_RM_SIZE-tlen);
		if( len > 0 ) {
		  ptr += len;
		  tlen += len;
			if( tlen >= MAX_RM_SIZE ) {
				break;
			}
			cnt = 3;
		}
		if( ( slen == tlen )&& (tlen > 3 ) ) {
		   break;
		}
		slen = tlen;
		cnt++;
	  Delay10Ms(2);
  }while((cnt < 5) && (tlen < 100)); /*接收大于100字节退出*/
	
	if( tlen < 6 ) {
		return FALSE;
	}
	
	tmp16 = buf[4] *256 + buf[5];
	if(buf[3] != CMD_RD_VAR || regAdr !=  tmp16 ){
		  return FALSE;
	}

//	printf("Hmi_ReadMulitVar len=%d\r\n", tlen);
	/*校验报文*/
	if( RT_ERROR == Hmi_PackgCheck(buf,tlen) ) {
		 return FALSE;
	}

	
	ptr = &buf[7];
	for( i = 0 ; i < regNum ; i++ ) {
		data[i] = Common_Change2Byte((uint16 *)ptr);
		ptr += 2;
	}
	 return i;
}


/***********************************************************************
** 函数名称: Hmi_HardwareBeep
** 功能描述： 蜂鸣器鸣叫
** 参数:    len,文本长度，最大128个字节
**返回值：   
*************************************************************************/
void Hmi_HardwareBeep(uint8 ms)
{
	 uint8 tmp;
	 BEEP_ON;
	 tmp = ms;
	 ms = tmp > 1 ? tmp:1;	
	 Delay10Ms(ms);
	 BEEP_OFF;
}



/***********************************************************************
** 函数名称: Hmi_BuzzerBeep
** 功能描述： 蜂鸣器鸣叫
** 参数:    len,文本长度，最大128个字节
**返回值：   
*************************************************************************/
void Hmi_BuzzerBeep(uint8 value)
{
	#if HARDWARE_BEEP
	  Hmi_HardwareBeep(value);
	#else
	 uint8 len;
	 uint8 buf[10];
	 len =  Hmi_PackWriteReg(buf,&value,DGUS_BEEP_REG,CMD_WR_REG,1);
	 Screen_UartWrite(buf,len);
	#endif
}





/***********************************************************************
** 函数名称: Hmi_RequestScreenTime
** 功能描述：请求触摸屏时间发送函数
** 参数:   
**返回值：   
*************************************************************************/
void Hmi_RequestScreenTime(void)
{
	 uint8 len;
	 uint8 buf[32] = {0};
	 len = Hmi_PackQueryReg(buf,DGUS_RTC_REG,CMD_RD_REG,8);
   Screen_UartWrite(buf,len);
}


/***********************************************************************
** 函数名称: Hmi_SetLcdLight
** 功能描述：设置触摸屏亮度  0x00 --- 0x40
** 参数:   
**返回值：   
*************************************************************************/
void Hmi_SetLcdLight(uint8 value)
{
	 uint8 len;
	 uint8 buf[10];
	 len =  Hmi_PackWriteReg(buf,&value,DGUS_LIGHT_REG,CMD_WR_REG,1);
	 Screen_UartWrite(buf,len);
	
}


//#define DGUS_PICNO_REG  0x03  /*当前图片号*/
//#define DGUS_TOUCH_REG  0x06   /*是否有触摸*/

/***********************************************************************
** 函数名称: Hmi_RequestPicNo
** 功能描述：获取当前屏号
** 参数:   
**返回值：   
*************************************************************************/
void Hmi_RequestPicNo(void)
{
	 uint8 len;
	 uint8 buf[32] = {0};
	 len = Hmi_PackQueryReg(buf,DGUS_PICNO_REG,CMD_RD_REG,1);
   Screen_UartWrite(buf,len);
}


/***********************************************************************
** 函数名称: Hmi_RequestIstouch
** 功能描述：获取当前屏号
** 参数:   
**返回值：   
*************************************************************************/
void Hmi_RequestIstouch(void)
{
	 uint8 len;
	 uint8 buf[32] = {0};
	 len = Hmi_PackQueryReg(buf,DGUS_TOUCH_REG,CMD_RD_REG,1);
   Screen_UartWrite(buf,len);
}



/***********************************************************************
** 函数名称: Hmi_CheckIstouch
** 功能描述：检测触摸屏被按下
** 参数:   
**返回值：   
*************************************************************************/
//extern int32 Screen_UartRead(uint8 *buf);
uint8 Hmi_CheckIstouch(void)
{
	 uint8 len;
	 uint16 num;
	 uint8 buf[32] = {0};
	 len = Hmi_PackQueryReg(buf,DGUS_TOUCH_REG,CMD_RD_REG,1);
   Screen_UartWrite(buf,len);

   len = 0;
   do
	 {
			num = Uart_GetBytesNumInBuff(UartHandler[SCREEN_COM]);
		  Delay10Ms(1); 
		  if( ++len > 5 ) {
				 len = 0;
				 break;
			}
	 }while( num < 9 );

    memset(buf,0,sizeof(buf));
    len = Screen_UartRead(buf,32);

    if( (len < 9) && (buf[3] == 0x81) && (buf[4] == 0x06 ) ) {
			return RT_ERROR;
		}

	
	//printf("Hmi_CheckIstouch len=%d\r\n", len);

    if( RT_EOK == Hmi_PackgCheck(buf,len)) {
				 if( buf[6] == 0x01 ) {
					  return RT_EOK;
				 }
		}
		
    return RT_ERROR;
}

/***********************************************************************
***函数名称: Screen_CloseBackLight
** 功能描述: 关闭背光
***    参数: 
**   返回值:   
*************************************************************************/
void Hmi_CloseBackLight(void)
{
		 uint8 cnt = 3;
		 do {
				Hmi_SetLcdLight(0);
				Delay10Ms(2);
		 }while(cnt--);
}


/***********************************************************************
***函数名称: Hmi_RegStrCheck
** 功能描述: 触摸屏字符串检查
***    参数: 
**   返回值:   
*************************************************************************/
void Hmi_RegStrCheck(uint8 *str,uint8 len)
{
	 uint8 i;
	 for( i = 0; i< len ; i++ )
	 {
		  if( str[i] == 0xff ) {
				 str[i] = 0;
				 break;
			} 
	 }
}

//2017-03-11 add
/***********************************************************************
** 函数名称: Hmi_GetCurrPicNo
** 功能描述：获取当前屏号
** 参数:   
**返回值：返回无效值 0xff ，其它为触摸屏号   
*************************************************************************/
uint8 Hmi_GetCurrPicNo(void)
{
	 uint8 len,num;
	 uint8 buf[64] = {0};
	 len = Hmi_PackQueryReg(buf,DGUS_PICNO_REG,CMD_RD_REG,1);
   Screen_UartWrite(buf,len);
   
   len = 0;
   do
	 {
			num = Uart_GetBytesNumInBuff(UartHandler[SCREEN_COM]);
		  Delay10Ms(1); 
		  if( ++len > 5 ) {
				 len = 0;
				 break;
			}
	 }while( num < 8 );

    memset(buf,0,sizeof(buf));
    len = Screen_UartRead(buf,64);

    if( (len < 9) && (buf[3] == 0x83) && (buf[5] == 0x14 ) ) {
			return 0xff;
	}

//	printf("Hmi_GetCurrPicNo len=%d\r\n", len);
    if( RT_EOK == Hmi_PackgCheck(buf,len)) {
				num = buf[8];/*屏号<255*/
		}else {
			num = 0xff;
		}
		
    return num;
}

/*设置显示屏时间函数*/
/* 0x5A A5 0B 82 00 10 13 0a 01 00 0b 0c 0d 00  2019-10-01 11:12:13*/  
uint8 Hmi_SetTime(struct tm *systime)
{
	uint8 index,txlen;
	uint8 buf[20];
	index = 0;
	
	buf[index++] 	=	FRAME_HEAD1;
	buf[index++] 	=	FRAME_HEAD2;
	buf[index++] 	=	0x00;		     //长度
	buf[index++] 	=	0x82;            //命令码
	buf[index++] 	=	0x00;
	buf[index++] 	=	0x10;
	buf[index++] 	=	Common_Hex2bcd8(systime->tm_year - 2000);
	buf[index++] 	=	Common_Hex2bcd8(systime->tm_mon);
	buf[index++] 	= Common_Hex2bcd8(systime->tm_mday);
	buf[index++] 	= 0;
	buf[index++] 	= Common_Hex2bcd8(systime->tm_hour);
	buf[index++] 	= Common_Hex2bcd8(systime->tm_min);
	buf[index++] 	= Common_Hex2bcd8(systime->tm_sec);
	buf[index++] 	= 0;	
	
	txlen = Hmi_PackFrameEnd(buf,index);
	Screen_UartWrite(buf,txlen);

	return 0;
}

/*获取触摸屏返回的字符串*/
uint8 Hmi_GetRcvString(char *objSrc, uint8 *buf,uint8 len)
{
	uint8 i,cnt;
	cnt = 0;
	for( i = 0 ; i < len ; i++ ) {
		if( buf[i] != 0xFF ) {
			objSrc[i] = buf[i];
			cnt++;
		}else {
			objSrc[i] = 0;
		}
	}
	return cnt;
}




