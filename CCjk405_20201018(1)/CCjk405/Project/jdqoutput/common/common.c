/**
  ******************************************************************************
  * @file    common.c
  * @author  zqj
  * @version v1.0
  * @date    2016-02-25
  * @brief   用了存放公用函数
  ******************************************************************************
	*/
//#include "M_Global.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const char conNumber[] = {'0','1','2','3','4','5','6','7','8','9'\
                         ,'A','B','C','D','E','F'};

const char conNumberL[] = {'0','1','2','3','4','5','6','7','8','9'\
                         ,'a','b','c','d','e','f'};
	
uint8 Common_Hex2bcd8(uint8 hex)
{
	return ((hex%10) + ((hex/10)<<4));
}
uint8 Common_Bcd2hex8(uint8 bcd)
{
	return ((bcd&0xf) + ((bcd&0xf0)>>4)*10);
}
uint8 Common_Bcd_Decimal(uint8 bcd)
{
	 return bcd-(bcd>>4)*6;
}
uint8 Common_Decimal_Bcd(uint8 decimal)
{
	 return decimal+(decimal/10)*6;
}

uint32 Common_Bcd2hex32(uint32 bcd)
{
	uint32 hex32;
	
	hex32 =   bcd&0xf;
	hex32 +=  ((bcd&0x000000f0)>>4)*10;
	hex32 +=  ((bcd&0x00000f00)>>8)*100;
	hex32 +=  ((bcd&0x0000f000)>>12)*1000;
	hex32 +=  ((bcd&0x000f0000)>>16)*10000;
	hex32 +=  ((bcd&0x00f00000)>>20)*100000;
	hex32 +=  ((bcd&0x0f000000)>>24)*1000000;
	hex32 +=  ((bcd&0xf0000000)>>28)*10000000;
	return hex32;
}

uint32 Common_Hex2bcd32(uint32 hex)
{
	uint32 bcd;
//	if(hex > 100000000)
//	{
//		return 0xffffffff;
//	}
	bcd = (hex/10000000)<<28;
	bcd += ((hex%10000000)/1000000)<<24;
	bcd += ((hex%1000000)/100000)<<20;
	bcd += ((hex%100000)/10000)<<16;
	bcd += ((hex%10000)/1000)<<12;
	bcd += ((hex%1000)/100)<<8;
	bcd += ((hex%100)/10)<<4;
	bcd += (hex%10);
	return bcd;
}


uint32 Common_CalcBCD32(uint32 largeBCD, uint32 smallBCD)
{
 	uint32 LargeHex, smallHex;
	LargeHex = Common_Bcd2hex32(largeBCD);
	smallHex = Common_Bcd2hex32(smallBCD);
	if(LargeHex < smallHex)
	{
		return 0xffffffff;
	}
	return Common_Hex2bcd32(LargeHex - smallHex);
}

uint16 Common_getLittleValue(uint16 bigValue)
{
	uint16 littleValue = 0;
	littleValue = ((bigValue&0x00ff)<<8) | ((bigValue&0xff00)>>8);
	return littleValue;
}

uint16 Common_getBigValue(uint16 littleValue)
{
	uint16 bigValue = 0;
	bigValue = (littleValue&0x00ff)<<8|(littleValue&0xff00)>>8;
	return bigValue;
}


int16 Common_getPNBigValue(int16 littleValue)
{
	int16 bigValue = 0;
	bigValue = (littleValue&0x00ff)<<8|(littleValue&0xff00)>>8;
	return bigValue;
}

int16 Common_getPNLittleValue(int16 BigValue)
{
	int16 LittleValue = 0;
	LittleValue = (BigValue&0x00ff)<<8|(BigValue&0xff00)>>8;
	return LittleValue;
}

uint32 Common_get2BigValue(uint32 littleValue)
{
	uint32 bigValue = 0;
	bigValue = ((littleValue&0xff)<<24)|((littleValue&0xff00)<<8)|((littleValue&0xff0000)>>8)|((littleValue&0xff000000)>>24);
	return bigValue;
}
/*32位  大小端切换*/
uint32 Common_get2LittleValue(uint32 BigValue)
{
	uint32 LittleValue = 0;
	LittleValue = (((BigValue&0xff000000)>>24)|((BigValue&0xff0000)>>8)|((BigValue&0xff00)<<8)|((BigValue&0xff)<<24));
	return LittleValue;
}

uint32 Common_CalclongSum(uint8* pbuf, uint16 lenth)
{
	uint32 sum = 0;
	while(lenth--)
	{
		sum += *pbuf;
		pbuf++;
	}
	return sum;	
}


uint16 Common_CalculateSum(uint8* pbuf, uint16 lenth)
{
	uint16 sum = 0;
	while(lenth--)
	{
		sum += *pbuf;
		pbuf++;
	}
	return sum;	
}

uint8 Common_CalcByteSum(uint8* pbuf, uint8 lenth)
{
	uint8 sum = 0;
	while(lenth--)
	{
		sum += *pbuf;
		pbuf++;
	}
	return sum;
}


uint8 Common_CalculateNOR(uint8 *p)
{
	uint8 redata;
	uint16 i;
	uint16 len;

	len = p[1]*256+p[2];
	redata = p[0]^p[1];
	for(i=2;i<len+4;i++)
	{
		 redata = redata^p[i];
	}
	return 	redata;
}

/***********************************************************************
功能： Common_Change2Byte 
参数:  交换两个字节的顺序
返回值：交换后的结果
*************************************************************************/
uint16 Common_Change2Byte(uint16 *data)
{
	 stoc_u tmp ;
	 tmp.c[0] = ((stoc_u*)data)->c[1];
	 tmp.c[1] = ((stoc_u*)data)->c[0];
	 *data = tmp.s;
	 return tmp.s;
}




/***********************************************************************
功能： Common_Change4Byte 
参数:  交换4个字节的顺序
返回值：交换后的结果
*************************************************************************/
uint32 Common_Change4Byte(uint32 *data)
{
	 u32toc_u *ptr = (u32toc_u *)data;
	 u32toc_u tmp;
	
	 tmp.c[0] = ptr->c[3];
	 tmp.c[1] = ptr->c[2];
	 tmp.c[2] = ptr->c[1];
	 tmp.c[3] = ptr->c[0];
	
	 ptr->i = tmp.i;
	
	 return tmp.i;
}


/***********************************************************************
*函数名称：Common_ModbusCRC
*功能描述：计算crc值
*    参数：
*  返回值：
*************************************************************************/
uint16 Common_ModbusCRC(uint8 *data, uint16 len)
{
	int16 i, j;
	uint16 crc=0xFFFF, flag=0;

	for (i = 0; i < len; i++)
	{
		crc ^= data[i];
		for (j = 0; j < 8; j++)
		{
			flag = crc & 0x0001;
			crc >>= 1;
			if (flag)
			{
				crc ^= 0xA001;
			}
		}
	}
	return crc;
}


/***********************************************************************
*函数名称：Common_BcdToAsc
*功能描述：将BCD码串转换成ASC码,字母大写
*    参数：*str,*bcd,bynum
*  返回值：字符串长度,
*************************************************************************/
uint8 Common_BcdToAsc(uint8 *str,uint8 *bcd,uint8 bynum)    //
{
	 uint8 i;
	 uint8 *ptr = str;
	
	 for(i = 0; i < bynum; i++)
	 {
		 *ptr++ = conNumber[((bcd[i]>>4) & 0x0f)] ; 
		 *ptr++ = conNumber[(bcd[i] & 0x0f)] ;  
	 }
	 *ptr++ = 0;
	 return bynum*2;
}/***********************************************************************
*函数名称：Common_BcdToAsc
*功能描述：将BCD码串转换成ASC码，字母小写
*    参数：*str,*bcd,bynum
*  返回值：字符串长度,
*************************************************************************/
uint8 Common_BcdToAscLit(uint8 *str,uint8 *bcd,uint8 bynum)    //
{
	 uint8 i;
	 uint8 *ptr = str;
	
	 for(i = 0; i < bynum; i++)
	 {
		 *ptr++ = conNumberL[((bcd[i]>>4) & 0x0f)] ; 
		 *ptr++ = conNumberL[(bcd[i] & 0x0f)] ;  
	 }
	 *ptr++ = 0;
	 return bynum*2;
}

/***********************************************************************
*函数名称：Common_NormlBcdToAsc
*功能描述：将BCD码串转换成ASC码,顺序不变
*    参数：*str,*bcd,bynum
*  返回值：字符串长度,
*************************************************************************/
uint8 Common_NormlBcdToAsc(uint8 *str,uint8 *bcd,uint8 bynum)
{
	 uint8 i;
	 uint8 *ptr = str;
	
	 for(i = 0; i < bynum ;i++ )
	 {
		 *ptr++ = conNumber[((bcd[i]>>4) & 0x0f)] ; 
		 *ptr++ = conNumber[(bcd[i] & 0x0f)] ;  
	 }
	 *ptr++ = 0;
	 return bynum*2;
}




/***********************************************************************
*函数名称：Common_IntToFloatStr
*功能描述：将整数变成浮点数字符串
*    参数：*str,data,dotpos小数位数
*  返回值：小数位数，限制为最大2位
*************************************************************************/
uint8 Common_IntToFloatStr(uint8 *str,int32 data,uint8 dotpos)
{
	 uint8 i;
	 uint8 fg;
	 uint32 tmp,inttmp,ftmp;
	 char dotstr[10] = {0};
   char tmpstr[20];
   fg = 0;
   if( data & 0x80000000 ) {
		 fg = 1;
		 data *= -1;
	 }

	 tmp = 1;
	 for( i = 0 ; i < dotpos ; i++ ) {
		  tmp = tmp * 10 ;
	 }
	 
	 inttmp = data / tmp;
	 
	 ftmp = data % tmp;
	 
	 sprintf((char *)tmpstr,"%d",inttmp);
	 
	 dotstr[0] = '.';

	 for( i = 1; i < dotpos+1; i++)
	 {
			tmp = tmp/10;
		  if(i == dotpos)
			{
				dotstr[i] = conNumber[ftmp%10];	
			}
			else
			{
				dotstr[i] = conNumber[ftmp/tmp];
				ftmp = ftmp%tmp;
			}		
	 }
	 dotstr[i] = 0;
		
	 if(fg == 1 ) {
		str[0] = '-';
		str[1] = 0;
		strcat((char *)str,tmpstr);
	 }else {
		 strcpy((char *)str,tmpstr);
	 }
	 strcat((char *)str,dotstr);
	  
	return strlen((char *)str);
}


/***********************************************************************
*函数名称：Common_IntToFloatStr
*功能描述：将IP地址字符串格式化成整数
*    参数：
*  返回值：
*************************************************************************/

uint8 Common_FormatIpAddr(char *str,uint8 ipv4[])
{
	 uint8 len,i,cnt;
	 uint8  tmp8; //data[4][3] = {{0}};
   

   //j = 0;
   cnt = 0;
   len = strlen(str);
   
   memset(ipv4,0,4);

   for( i = 0; i < len; i++ )
   {
		  if( (str[i] <'0' || str[i] > '9') && str[i] != '.' ) {
				break;
			}
		 
		  if((str[i] == '.')&& (cnt < 4)) {
				 cnt++;
				 continue;
			}
			
			if( (str[i+1] == '.') || (str[i+1] == 0xff) || (str[i+1] == 0x00) ) {
				tmp8 = str[i]-'0';
		    ipv4[cnt] += tmp8;
				continue;
			}
			
			tmp8 = str[i]-'0';
		  ipv4[cnt] += tmp8;
			ipv4[cnt] *= 10;
			
	 }
	 
	 return COMMON_RTN_SUCC;
   
}




/***********************************************************************
*Common_FormatMacAddr
*功能描述：将mac地址字符串格式化成整数
*    参数：
*  返回值：
*************************************************************************/

uint8 Common_FormatMacAddr(char *str,uint8 mac[])
{
	 uint8 len,i,j;
   uint16 temp,temp2;
   char c;
	
   j = 0;
   len = strlen(str);
	 if( len < 17 ) {
		  return COMMON_RTN_ERR;
	 }
	 
   
  memset(mac,0,6);
	
	for( i = 0; i < 6; i++ )
	{
		temp = 0;
		temp2 = 0;
		c = str[j];
		
		if( j >= 18 ) {
			break;
		}
		
		while( c == ':' ) {
			j++;
			c = str[j];
		}
		
		if( c >= 'a' && c <= 'f' ) // 两个字符中的第一个 比如 "0f" ,则表示是字符 '0'
		temp = ( c - 'a' ) + 10; 
		else
		temp = ( c - '0' ) ;
		j++;


		c = str[j];
		if( c >= 'a' && c <= 'f' ) // 两个字符中的第二个,如 "f8" ,那么表示字符 '8'
		temp2 = ( c - 'a' ) + 10;
		else
		temp2 = ( c - '0' ) ;


		temp = temp * 16;
		temp += temp2;
		j++;
		*(mac+i) = temp;
	}
 
	 return COMMON_RTN_SUCC;
   
}

/***********************************************************************
功能： Common_Change2Byte 
参数:  交换两个字节的顺序
返回值：交换后的结果
*************************************************************************/
uint16 Common_Change2ByteBigType(uint16 data)
{
	 stoc_u tmp,*ptr;
	 ptr = (stoc_u *)&data;
	 tmp.c[0] = ptr->c[1];
	 tmp.c[1] = ptr->c[0];
	
	 return tmp.s;
}



/***********************************************************************
*Common_atol
*功能描述：将字符串变成整数
*    参数：
*  返回值：
*************************************************************************/
uint32 Common_atol(char *str)
{
	return atol(str);
}


/***********************************************************************
*trim
*功能描述：去掉字符串末尾的回车、换行、空格
*    参数：
*  返回值：
*************************************************************************/
char *Common_trim(char *str)
{
	  uint16 len = strlen(str);
    char *p = str; 
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
        p ++;
			if( p > (str+len)) {
				return str;
			}
		}
    str = p; 
    p = str + strlen(str) - 1; 
    while (*p == 0x20 || *p == '\t' || *p == '\r' || *p == '\n' || *p == 0xff){
        -- p;
			if( p == str ) {
				 break;
			}
		}
    *(p + 1) = 0; 
    return str; 
}


/***********************************************************************
*Common_AscToBcd
*功能描述：将ASC码转换成BCD码
*    参数：
*  返回值：
*************************************************************************/
uint8 Common_AscToBcd(uint8 *bcd,uint8 *str)
{
	 //uint8 tmp;
	 uint8 i,bynum;
	 uint8 *ptr = bcd;
	 uint8 buf[64] = {0};
	 bynum = strlen((char *)str)%60 ;
   memcpy(buf,str,bynum);
	 bynum += 1;

	 for(i = 0; i < bynum/2;i++)
	 {
		 if(buf[i*2] < 0x40 ) {
				*ptr = ((buf[i*2] - 0x30 ) & 0x0f )<< 4;
		 }else {
			 *ptr  = ((buf[i*2] - 0x41 + 10) & 0x0f ) << 4;
		 }
		 if(buf[i*2+1] < 0x40 ) {
				*ptr |= ((buf[i*2+1] -0x30) & 0x0f) ;
		 }else {
			 *ptr |= ((buf[i*2+1] -0x41 + 10 ) & 0x0f) ;
		 }
		  ptr++;
	 }
	 *ptr++ = 0;
	 return bynum/2;
}
/* 16位按小端模式转换为2个8位数*/
uint8 Common_LittleShortTo(uint8 *pbuf,uint16 utemp)
{
	stoc_u temp;
	temp.s = utemp;
	pbuf[0] = temp.c[0];
	pbuf[1] = temp.c[1];
	return 2;
}
/* 16位按小端模式转换为2个8位数*/
uint8 Common_BigShortTo(uint8 *pbuf,uint16 utemp)
{
	stoc_u temp;
	temp.s = utemp;
	pbuf[1] = temp.c[0];
	pbuf[0] = temp.c[1];
	return 2;
}
/* 32位按小端模式转换为4个8位数*/
uint8 Common_LittleIntTo(uint8 *pbuf,uint32 utemp)
{
	u32toc_u temp;
	temp.i = utemp;
	pbuf[0] = temp.c[0];
	pbuf[1] = temp.c[1];
	pbuf[2] = temp.c[2];
	pbuf[3] = temp.c[3];
	return 4;
	
}
/* 32位按大端模式转换为4个8位数*/
uint8 Common_BigIntTo(uint8 *pbuf,uint32 utemp)
{
	u32toc_u temp;
	temp.i = utemp;
	pbuf[3] = temp.c[0];
	pbuf[2] = temp.c[1];
	pbuf[1] = temp.c[2];
	pbuf[0] = temp.c[3];
	return 4;
	
}
/*将4个8位数据转成小端32位*/
uint32 Common_LittleToInt(uint8 *pbuf)
{	
	u32toc_u temp;
	
	temp.c[0] = pbuf[0];
	temp.c[1] = pbuf[1];
	temp.c[2] = pbuf[2];
	temp.c[3] = pbuf[3];
	return temp.i;
}
/*将4个8位数据转成小端32位*/
uint32 Common_BigToInt(uint8 *pbuf)
{	
	u32toc_u temp;
	
	temp.c[3] = pbuf[0];
	temp.c[2] = pbuf[1];
	temp.c[1] = pbuf[2];
	temp.c[0] = pbuf[3];
	return temp.i;
}
/*将2个8位数据转成小端16位*/
uint16 Common_LittleToShort(uint8 *pbuf)
{	
	stoc_u temp;
	
	temp.c[0] = pbuf[0];
	temp.c[1] = pbuf[1];
	return temp.s;
}


uint16 HexToAsc(uint8 Hex)
{
	stoc_u tmp16;
 	uint8 tmp;
	tmp16.s = 0;
	
	tmp = (Hex >> 4)&0x0f;
	if( tmp > 9 ) {
		tmp16.c[1] = 'A' + (tmp-10);
	}else {
		tmp16.c[1] = '0' + tmp;
	}
	
	tmp = Hex & 0x0f;
	if( tmp > 9 ) {
		tmp16.c[0] = 'A' + (tmp-10);
	}else {
		tmp16.c[0] = '0' + tmp;
	}
	return tmp16.s;
}

/*大端模式转换*/
uint8 AscToHex(uint8 *buf)
{
	uint8 tmp = 0;
	if(buf[0] >= 'A' && buf[0] <= 'F') {
		 tmp = buf[0] - 'A'+10;
	}else if( buf[0] >= '0' && buf[0] <= '9' ){
		tmp = buf[0] -'0';
	}
	tmp <<= 4;
	
	if(buf[1] >= 'A' && buf[1] <= 'F') {
		tmp += buf[1] - 'A'+10;
	}else if( buf[1] >= '0' && buf[1] <= '9' ){
		tmp += buf[1] -'0';
	}
	return tmp;
}

/*数字字符串转换成整形数，不支持负数 
*返回值：32位无符号整型数
*/
uint32 Common_Ascii2Uint32(uint8 *str,uint8 len)
{
	uint8 i ;
	uint32 u32data = 0;
	for( i = 0 ; i < len ; i++ ) {
		if( str[i] >= '0' && str[i] <= '9') {
			u32data = str[i] - '0' + u32data * 10;
		}else {
			break;
		}
	}
	return u32data;
}


/*卡号中间4位变成***  */
uint8 Common_TranCarNo(uint8 *CardNo,uint8 len)
{
	//uint8 len = strlen((char *)CardNo);
	uint8 *ptr = CardNo;
	if( len < 7 ) { 
		return len;
	}
	ptr[3] = '*';
	ptr[4] = '*';
	ptr[5] = '*';
	ptr[6] = '*';
	return len;
}
void Common_Invert(uint8 *buf,uint8 len)
{
	uint8 i,j,temp;
	for(i= 0;i<(len/2);i++)
	{
		temp = buf[i];
		j = len - i - 1;
		buf[i] = buf[j];
    buf[j] = 	temp;			
	}
}



//buf 数组转16进制字符串
uint8 Common_bufTransAscii(uint8 *buf,uint16 len, uint8* dest)
{
	uint8 i;
	stoc_u tmp16;

	for(i = 0 ; i < len ; i++ ) {
		tmp16.s = HexToAsc(buf[i]);
		dest[0+i*2] = tmp16.c[1] ;
		dest[1+i*2] = tmp16.c[0] ;
	}
	
	dest[2+i*2] = 0;
	dest[3+i*2] = 0;

	return i;
}

