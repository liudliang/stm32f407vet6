#ifndef __DS18B20_H
#define __DS18B20_H 
#include "sys.h"   
//////////////////////////////////////////////////////////////////////////////////	 
//功能：DS18B20驱动代码
//作者：
//创建日期:2020/05/10
//版本：V1.0							  
//////////////////////////////////////////////////////////////////////////////////

//IO方向设置
#define DS18B20_IO_IN()  {GPIOE->MODER&=~(3<<(2*2));GPIOG->MODER|=0<<2*2;}	//PE2输入模式
#define DS18B20_IO_OUT() {GPIOG->MODER&=~(3<<(2*2));GPIOG->MODER|=1<<2*2;} 	//PE2输出模式
 
////IO操作函数											   
#define	DS18B20_DQ_OUT PEout(2) //数据端口	PE2
#define	DS18B20_DQ_IN  PEin(2)  //数据端口	PE2 
   	
u8 DS18B20_Init(void);			//初始化DS18B20
short DS18B20_Get_Temp(void);	//获取温度
void DS18B20_Start(void);		//开始温度转换
void DS18B20_Write_Byte(u8 dat);//写入一个字节
u8 DS18B20_Read_Byte(void);		//读出一个字节
u8 DS18B20_Read_Bit(void);		//读出一个位
u8 DS18B20_Check(void);			//检测是否存在DS18B20
void DS18B20_Rst(void);			//复位DS18B20    
#endif















