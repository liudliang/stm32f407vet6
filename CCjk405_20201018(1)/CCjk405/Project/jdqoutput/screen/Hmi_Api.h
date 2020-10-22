/**
  ******************************************************************************
  * @file    Hmi_Api.h
  * @author  zqj
  * @version v1.0
  * @date    2016-03-09
  * @brief   
  ******************************************************************************
	*/
#include "typedefs.h"

#ifndef _HMI_API_H_0001
#define _HMI_API_H_0001


#define RT_TRUE  1
#define RT_FALSE 0

#define RT_EOK   1
#define RT_ERROR 0

#define FRAME_HEAD1      0x5A    //帧头1
#define FRAME_HEAD2      0xA5     //帧头2
#define MIN_FRAME_LEN    6

#define CMD_WR_REG 0x80
#define CMD_RD_REG 0x81
#define CMD_WR_VAR 0x82
#define CMD_RD_VAR 0x83
#define CMD_WR_QUX 0x84

#define REG_PIC_ID 0x03

#define CLOCK_REG_ADDR   0x0001   //
#define CLOCK_REG        0x1f     //时钟寄存器


#define   LED_NOW_REG     0x01    //LED 亮度 0x00~0x40
#define   BZ_TIME_REG     0x02    //蜂鸣器控制 val*10
#define   PIC_ID_REG      0x03    //页面寄存器

#define   LED_NOW_REG_LEN   1
#define   BZ_TIME_REG_LEN   1
#define   PIC_ID_REG_LEN    2


/*打包请求触摸屏寄存器报文*/
extern uint8 Hmi_PackQueryReg(uint8 *buf,uint16 regAdr,\
                                         uint8 nCmd,uint8 regNum);

/*打包清除触摸屏寄存器报文*/
extern uint8 Hmi_PackClearReg(uint8 *obj,uint16 regAdr,uint8 nCmd,uint8 regNum);

/*触摸屏返回报文检测*/
extern uint8  Hmi_PackgCheck(uint8 *buf,uint8 rxLen);

/*打包写触摸屏寄存器报文*/
extern uint8 Hmi_PackWriteReg(uint8 *obj,uint8 *src,\
                       uint16 regAdr,uint8  nCmd,uint8 byNum);
											 
/*发送切换页面的命令*/		
extern void Hmi_ChangePicture(uint8 picNo);

/*文本显示函数*/
extern void Hmi_ShowText(uint16 adr,uint16 len,uint8 *text);

/*关闭弹出键盘*/
extern void Hmi_ShutDownKeyBoard(uint8 value);

/*清除指定位置开始的寄存器*/
extern uint8 Hmi_ClearReg(uint16 regAdr,uint8 regNum);

/*蜂鸣器鸣叫*/
extern void Hmi_BuzzerBeep(uint8 value);

/*请求触摸屏时间*/
extern void Hmi_RequestScreenTime(void);

/*写一个变量寄存器*/
extern uint8 Hmi_WriteOneVar(uint16 regAdr,uint16 data);

/*设置触摸屏亮度*/ 
extern void Hmi_SetLcdLight(uint8 value);


/*请求图片号*/
extern void Hmi_RequestPicNo(void);

/*请求是否有触摸*/
extern void Hmi_RequestIstouch(void);

/*写多个寄存器变量*/
extern uint8 Hmi_WriteMulitVar(uint16 regAdr,uint16 *data,uint16 regNum);

/*触摸检测*/
extern uint8 Hmi_CheckIstouch(void);

/*关闭背光*/
extern void Hmi_CloseBackLight(void);

extern void Hmi_RegStrCheck(uint8 *str,uint8 len);

/*获取当前屏幕号*/
extern uint8 Hmi_GetCurrPicNo(void);

/*获取触摸屏返回的字符串*/
extern uint8 Hmi_GetRcvString(char *objSrc, uint8 *buf,uint8 len);

											 

											 
























#endif


