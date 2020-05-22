/**
  ******************************************************************************
  * @file    common.c
  * @author  
  * @version v1.0
  * @date    
  * @brief   用了存放公用函数
  ******************************************************************************
	*/
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const char conNumber[] = {'0','1','2','3','4','5','6','7','8','9'\
                         ,'A','B','C','D','E','F'};

const char conNumberL[] = {'0','1','2','3','4','5','6','7','8','9'\
                         ,'a','b','c','d','e','f'};
	
uint8_t Common_Hex2bcd8(uint8_t hex)
{
	return ((hex%10) + ((hex/10)<<4));
}
uint8_t Common_Bcd2hex8(uint8_t bcd)
{
	return ((bcd&0xf) + ((bcd&0xf0)>>4)*10);
}
uint8_t Common_Bcd_Decimal(uint8_t bcd)
{
	 return bcd-(bcd>>4)*6;
}
uint8_t Common_Decimal_Bcd(uint8_t decimal)
{
	 return decimal+(decimal/10)*6;
}

uint32_t Common_Bcd2hex32(uint32_t bcd)
{
	uint32_t hex32;
	
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

uint32_t Common_Hex2bcd32(uint32_t hex)
{
	uint32_t bcd;
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


uint32_t Common_CalcBCD32(uint32_t largeBCD, uint32_t smallBCD)
{
 	uint32_t LargeHex, smallHex;
	LargeHex = Common_Bcd2hex32(largeBCD);
	smallHex = Common_Bcd2hex32(smallBCD);
	if(LargeHex < smallHex)
	{
		return 0xffffffff;
	}
	return Common_Hex2bcd32(LargeHex - smallHex);
}

uint16_t Common_getLittleValue(uint16_t bigValue)
{
	uint16_t littleValue = 0;
	littleValue = ((bigValue&0x00ff)<<8) | ((bigValue&0xff00)>>8);
	return littleValue;
}

uint16_t Common_getBigValue(uint16_t littleValue)
{
	uint16_t bigValue = 0;
	bigValue = (littleValue&0x00ff)<<8|(littleValue&0xff00)>>8;
	return bigValue;
}


int16_t Common_getPNBigValue(int16_t littleValue)
{
	int16_t bigValue = 0;
	bigValue = (littleValue&0x00ff)<<8|(littleValue&0xff00)>>8;
	return bigValue;
}

int16_t Common_getPNLittleValue(int16_t BigValue)
{
	int16_t LittleValue = 0;
	LittleValue = (BigValue&0x00ff)<<8|(BigValue&0xff00)>>8;
	return LittleValue;
}

uint32_t Common_get2BigValue(uint32_t littleValue)
{
	uint32_t bigValue = 0;
	bigValue = ((littleValue&0xff)<<24)|((littleValue&0xff00)<<8)|((littleValue&0xff0000)>>8)|((littleValue&0xff000000)>>24);
	return bigValue;
}
/*32位  大小端切换*/
uint32_t Common_get2LittleValue(uint32_t BigValue)
{
	uint32_t LittleValue = 0;
	LittleValue = (((BigValue&0xff000000)>>24)|((BigValue&0xff0000)>>8)|((BigValue&0xff00)<<8)|((BigValue&0xff)<<24));
	return LittleValue;
}

uint32_t Common_CalclongSum(uint8_t* pbuf, uint16_t lenth)
{
	uint32_t sum = 0;
	while(lenth--)
	{
		sum += *pbuf;
		pbuf++;
	}
	return sum;	
}


uint16_t Common_CalculateSum(uint8_t* pbuf, uint16_t lenth)
{
	uint16_t sum = 0;
	while(lenth--)
	{
		sum += *pbuf;
		pbuf++;
	}
	return sum;	
}

uint8_t Common_CalcByteSum(uint8_t* pbuf, uint8_t lenth)
{
	uint8_t sum = 0;
	while(lenth--)
	{
		sum += *pbuf;
		pbuf++;
	}
	return sum;
}


uint8_t Common_CalculateNOR(uint8_t *p)
{
	uint8_t redata;
	uint16_t i;
	uint16_t len;

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
uint16_t Common_Change2Byte(uint16_t *data)
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
uint32_t Common_Change4Byte(uint32_t *data)
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
uint16_t Common_ModbusCRC(uint8_t *data, uint16_t len)
{
	int16_t i, j;
	uint16_t crc=0xFFFF, flag=0;

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
uint8_t Common_BcdToAsc(uint8_t *str,uint8_t *bcd,uint8_t bynum)    //
{
	 uint8_t i;
	 uint8_t *ptr = str;
	
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
uint8_t Common_BcdToAscLit(uint8_t *str,uint8_t *bcd,uint8_t bynum)    //
{
	 uint8_t i;
	 uint8_t *ptr = str;
	
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
uint8_t Common_NormlBcdToAsc(uint8_t *str,uint8_t *bcd,uint8_t bynum)
{
	 uint8_t i;
	 uint8_t *ptr = str;
	
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
uint8_t Common_IntToFloatStr(uint8_t *str,int32_t data,uint8_t dotpos)
{
	 uint8_t i;
	 uint8_t fg;
	 uint32_t tmp,inttmp,ftmp;
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

uint8_t Common_FormatIpAddr(char *str,uint8_t ipv4[])
{
	 uint8_t len,i,cnt;
	 uint8_t  tmp8; //data[4][3] = {{0}};
   

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

uint8_t Common_FormatMacAddr(char *str,uint8_t mac[])
{
	 uint8_t len,i,j;
   uint16_t temp,temp2;
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
uint16_t Common_Change2ByteBigType(uint16_t data)
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
uint32_t Common_atol(char *str)
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
	  uint16_t len = strlen(str);
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
uint8_t Common_AscToBcd(uint8_t *bcd,uint8_t *str)
{
	 //uint8_t tmp;
	 uint8_t i,bynum;
	 uint8_t *ptr = bcd;
	 uint8_t buf[64] = {0};
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
uint8_t Common_LittleShortTo(uint8_t *pbuf,uint16_t utemp)
{
	stoc_u temp;
	temp.s = utemp;
	pbuf[0] = temp.c[0];
	pbuf[1] = temp.c[1];
	return 2;
}
/* 16位按小端模式转换为2个8位数*/
uint8_t Common_BigShortTo(uint8_t *pbuf,uint16_t utemp)
{
	stoc_u temp;
	temp.s = utemp;
	pbuf[1] = temp.c[0];
	pbuf[0] = temp.c[1];
	return 2;
}
/* 32位按小端模式转换为4个8位数*/
uint8_t Common_LittleIntTo(uint8_t *pbuf,uint32_t utemp)
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
uint8_t Common_BigIntTo(uint8_t *pbuf,uint32_t utemp)
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
uint32_t Common_LittleToInt(uint8_t *pbuf)
{	
	u32toc_u temp;
	
	temp.c[0] = pbuf[0];
	temp.c[1] = pbuf[1];
	temp.c[2] = pbuf[2];
	temp.c[3] = pbuf[3];
	return temp.i;
}
/*将4个8位数据转成小端32位*/
uint32_t Common_BigToInt(uint8_t *pbuf)
{	
	u32toc_u temp;
	
	temp.c[3] = pbuf[0];
	temp.c[2] = pbuf[1];
	temp.c[1] = pbuf[2];
	temp.c[0] = pbuf[3];
	return temp.i;
}
/*将2个8位数据转成小端16位*/
uint16_t Common_LittleToShort(uint8_t *pbuf)
{	
	stoc_u temp;
	
	temp.c[0] = pbuf[0];
	temp.c[1] = pbuf[1];
	return temp.s;
}


uint16_t HexToAsc(uint8_t Hex)
{
	stoc_u tmp16;
 	uint8_t tmp;
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
uint8_t AscToHex(uint8_t *buf)
{
	uint8_t tmp = 0;
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
uint32_t Common_Ascii2uint32_t(uint8_t *str,uint8_t len)
{
	uint8_t i ;
	uint32_t u32data = 0;
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
uint8_t Common_TranCarNo(uint8_t *CardNo,uint8_t len)
{
	//uint8_t len = strlen((char *)CardNo);
	uint8_t *ptr = CardNo;
	if( len < 7 ) { 
		return len;
	}
	ptr[3] = '*';
	ptr[4] = '*';
	ptr[5] = '*';
	ptr[6] = '*';
	return len;
}
void Common_Invert(uint8_t *buf,uint8_t len)
{
	uint8_t i,j,temp;
	for(i= 0;i<(len/2);i++)
	{
		temp = buf[i];
		j = len - i - 1;
		buf[i] = buf[j];
    buf[j] = 	temp;			
	}
}



//buf 数组转16进制字符串
uint8_t Common_bufTransAscii(uint8_t *buf,uint16_t len, uint8_t* dest)
{
	uint8_t i;
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

