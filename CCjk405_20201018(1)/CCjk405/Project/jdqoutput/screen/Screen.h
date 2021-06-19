#ifndef _SCREEN_H
#define _SCREEN_H

#include "typedefs.h"
#include "ChgData.h"

#define INVALID_REG_ADR   0xffff
#define  DEFAULT_COUNTDOWN_VALUE    120       /*120s*/

#define TICK_TIMES_10S          (OS_TICKS_PER_SEC*10)           /*1000 *10ms  5s*/   

#define TICK_TIMES_2S           (OS_TICKS_PER_SEC*2)  

#define SHOW_DATESHOW_TIMES      (OS_TICKS_PER_SEC*1)         /*70*10，设定刷新寄存器的周期*/

#define SHOW_CYCLIC_TICK_TIMES   (OS_TICKS_PER_SEC*1)         /*100 * 10ms，设定刷新寄存器的周期*/
#define TICK_TIMES_1MIN          (OS_TICKS_PER_SEC*60)        /*1分钟*/



#define MAX_REG_NUM_ONCE_SEND    50          /*设定一次刷新的寄存器数量*/

#define KEY_ADRESS_DEF  0x2000

#define DGUS_LOGOSHOW_ADR            0x18E0   /*logo地址*/
#define DGUS_NET_ADDR                0x18F0   /*联网标识地址显示*/
#define DGUS_KEYSTOPA_ADDR                0x18FA   /*联网标识地址显示*/
#define DGUS_KEYSTOPB_ADDR                0x18FB   /*联网标识地址显示*/
#define  DGUS_TIMESHOW_ADR           0x22A0   /*日历时间显示*/

#define  DGUS_NUMBER_ADR_A           0x3000   /*A枪编号*/
#define  DGUS_NUMBER_ADR_B           0x3010   /*B枪编号*/
#define  DGUS_CURRATR_ADR            0x3050   /*当前费率显示*/

#define  DGUS_VOLTUAB_ADR_A           0x3100   /*初始界面Uab*/
#define  DGUS_VOLTUBC_ADR_A           0x3120   /*初始界面Ubc*/
#define  DGUS_VOLTUCA_ADR_A           0x3140   /*初始界面Uca*/
#define  DGUS_VOLTUAB_ADR_B           0x3160   /*初始界面Uab*/
#define  DGUS_VOLTUBC_ADR_B           0x3180   /*初始界面Ubc*/
#define  DGUS_VOLTUCA_ADR_B           0x31A0   /*初始界面Uca*/

#define DGUS_TWODIMBARCODE_ADDR_A    0x1900   /*A枪二维码地址*/
#define DGUS_TWODIMBARCODE_ADDR_B    0x1980   /*B枪二维码地址*/

#define DGUS_DEVID_ADDR              0x12B0   /*充电桩id*/
#define DGUS_SYS_TIME_ADR             0x12C0   /*系统时间*/

#define DGUS_VERSION_ADR             0x1748    /*软件版本显示*/

#define  DGUS_SETENERGY_ADR          0x10E0   /*输入电量*/
#define  DGUS_SETMONEY_ADR           0x10E2   /*输入金钱*/
#define  DGUS_AETTIME_ADR            0x10E4   /*输入时间*/

#define  DGUS_CHARGE_ADDR_A          0x1010   /*A枪正在充电数据起始地址*/
#define  DGUS_CHARGID_ADR_A          0x1070   /*A枪正在充电卡号*/
#define  DGUS_CHARGQAY_ADR_A         0x1090   /*A枪正在充电方式*/

#define  DGUS_CHARGE_ADDR_B          0x1000   /*B枪正在充电数据起始地址*/
#define  DGUS_CHARGID_ADR_B          0x1030   /*B枪正在充电卡号*/
#define  DGUS_CHARGQAY_ADR_B         0x1050   /*B枪正在充电方式*/

#define  DGUS_FAULT_TEXT_ADR_A1      0x2300   /*A枪故障提示信息1*/
#define  DGUS_FAULT_TEXT_ADR_A2      0x2340   /*A枪故障提示信息2*/

#define  DGUS_FAULT_TEXT_ADR_B1      0x2380   /*B枪故障提示信息1*/
#define  DGUS_FAULT_TEXT_ADR_B2      0x23C0   /*B枪故障提示信息2*/

#define  DGUS_CARDID_ADR_A           0x2400   /*A枪结算卡号        文本*/
#define  DGUS_START_ADR_A            0x2430   /*A枪结算卡开始      文本*/
#define  DGUS_STOP_ADR_A             0x2460   /*A枪结算卡结束      文本*/
#define  DGUS_REASON_ADR_A           0x2490   /*A枪结束原因*/
#define  DGUS_ENERGY_ADR_A           0x24C0   /*A枪结算卡使用电量  文本*/
#define  DGUS_MONEY_ADR_A            0x24F0   /*A枪结算卡使用金额  文本*/
#define  DGUS_TIME_ADR_A             0x2520   /*A枪结算卡使用时间  文本*/
#define  DGUS_WAY_ADR_A              0x2550   /*A枪结算卡充电方式  文本*/
#define  DGUS_SOC_ADR_A              0x2580   /*A枪结算卡结算SOC   文本*/
#define  DGUS_REMONEY_ADR_A          0x25B0   /*A枪结算卡余额      文本*/

#define  DGUS_CARDID_ADR_B           0x2600   /*B枪结算卡号        文本*/
#define  DGUS_START_ADR_B            0x2630   /*B枪结算卡开始      文本*/
#define  DGUS_STOP_ADR_B             0x2660   /*B枪结算卡结束      文本*/
#define  DGUS_REASON_ADR_B           0x2690   /*B枪结束原因*/
#define  DGUS_ENERGY_ADR_B           0x26C0   /*B枪结算卡使用电量  文本*/
#define  DGUS_MONEY_ADR_B            0x26F0   /*B枪结算卡使用金额  文本*/
#define  DGUS_TIME_ADR_B             0x2720   /*B枪结算卡使用时间  文本*/
#define  DGUS_WAY_ADR_B              0x2750   /*B枪结算卡充电方式  文本*/
#define  DGUS_SOC_ADR_B              0x2780   /*B枪结算卡结算SOC   文本*/
#define  DGUS_REMONEY_ADR_B          0x27B0   /*B枪结算卡余额      文本*/

#define DGUS_ACCOUNTS_OFFSET         0x30     /*结算文本偏移量*/
#define DGUS_REC_START_A             DGUS_CARDID_ADR_A       /*第一条记录起始地址，地址必须有规律增加0x30*/
#define DGUS_REC_START_B             DGUS_CARDID_ADR_B

#define  DGUS_PW_SHOW_ADR            0x1800   /*密码输入文本地址*/
#define  DGUA_PWSH_ADR               0x1810   /*密码显示文本地址*/
#define  DGUS_COUNTDOWN_REGADR       0x1820   /*倒计时计数器地址*/
#define  DGUS_PW_CARD_ADR            0x1830   /*卡密码输入文本地址*/
#define  DGUS_PW_CARDSH_ADR          0x1840   /*卡密码显示文本地址*/

#define  DGUS_IDLE_PROADR_A          0x2100   /*A枪空闲状态提示文本*/
#define  DGUS_IDLE_PROADR_B          0x2120   /*B枪空闲状态提示文本*/
#define  DGUS_CHARG_PROADR_A         0x2140   /*A枪充电状态提示文本*/
#define  DGUS_CHARG_PROADR_B         0x2160   /*B枪充电状态提示文本*/
#define  DGUS_ACCOUNT_PROADR_A       0x2180   /*A枪结算状态提示文本*/
#define  DGUS_ACCOUNT_PROADR_B       0x21A0   /*B枪结算状态提示文本*/
#define  DGUS_CHARWAY_PROADR         0x21C0   /*充电方式选择界面提示文本*/
#define  DGUS_INPUT_PROADR           0x21E0   /*输入充电参数界面提示文本*/
#define  DGUS_SETPA_PROADR           0x2200   /*设置参数界面提示文本*/
#define  DGUS_SETSENPA_PROADR        0x2220   /*设置高级参数界面提示文本*/
#define  DGUS_RECORD_PROADR          0x2240   /*充电记录界面提示文本*/
#define  DGUS_SETMONEY_PROADR        0x2260   /*设置费率提示文本*/
#define  DGUS_PASSWD_PROADR          0x2280   /*密码验证错误提示文本*/
#define  DGUS_CHAREADY_PROADR        0x22C0   /*准备充电提示文本*/

//--------add0529zyf
#define  DGUS_STARTMODE_GUNNO         0x3500   /*51页  枪号*/
#define  DGUS_STARTMODE_CHARGACCOUNT  0x3510   /*51页  充电账号*/
#define  DGUS_STARTMODE_CHARGAPSW     0x3530   /*51页  充电密码*/
#define  DGUS_STARTMODE_CHARGAPSWSHOW 0x3550   /*51页  充电密码显示*/


//充电界面新增项
#define  DGUS_TEMPER_A                0x3300   /*A枪温度 */
#define  DGUS_TEMPER_B                0x3302   /*B枪温度 */
#define  DGUS_TEMPER_PLAT             0x3304   /*桩温度 */
#define  DGUS_RATE_POWER_A            0x3310   /*A枪电量费率*/
#define  DGUS_RATE_SERVER_A           0x3330   /*A枪服务费率*/
#define  DGUS_RATE_OCCUPY_A           0x3350   /*A枪占桩费率*/
#define  DGUS_RATE_POWER_B            0x3410   /*B枪电量费率*/
#define  DGUS_RATE_SERVER_B           0x3430   /*B枪服务费率*/
#define  DGUS_RATE_OCCUPY_B           0x3450   /*B枪占桩费率*/
#define  DGUS_STOPCHARG_WAITOVER_A    0x3600   /*充之鸟-结束充电，等待拔枪*/
#define  DGUS_STOPCHARG_WAITOVER_B    0x3620   /*充之鸟-结束充电，等待拔枪*/
#define  DGUS_STARTCHARG_WAITING      0x3640   /*充之鸟-开启充电，等待后台验证*/




typedef enum
{
   IDEL    = 0,
	 CHECK   = 1,
	 FINISH  = 2,
	 ACCOUNT = 3,
	 E_ERROR = 4,
	 CHARGING = 5,

}WORK_STATU;


/********************************************/

//#define DGUS_CHAR_TIME_ADR          0x2022
//#define DGUS_CHAR_ENERGY_ADR        0x2024
//#define DGUS_CHAR_MONEY_ADR         0x2026
/********************************************/


#pragma pack(1)
typedef struct SCREEN_FRAME
{
	 uint8 head1;
	 uint8 head2;
	 uint8 length;
	 uint8 cmd;
	 uint16 nsReg;     /*倒序的short*/
	 uint8  num;
	 uint16 nsData[1]; /*倒序的short*/
}ScrFrame_t;
#pragma pack()

#pragma pack(1)
typedef struct SCREEN_FRAME1
{
	 uint8 head1;
	 uint8 head2;
	 uint8 length;
	 uint8 cmd;
	 uint8 nReg;       /*内部寄存器地址 仅有一个字节*/
	 uint8 num;
	 uint16 nsData[1]; /*倒序的short*/
}ScrInRegFrame_t;
#pragma pack()


typedef struct KEY_FUNC1
{
	uint16  keyaddr;               /*按键地址*/
	uint16  code;                  /*键码*/
	uint8   picNo;                 /*图片地址*/
	void (*func)(void);   /*按键功能*/
	uint16 (*keyclick)(void);          /*按键计数*/
}Keyfunc1_t;

typedef struct PAGE_DEF
{
	uint16  addr;         /*变量寄存器地址*/
	uint8   ntype;        /*变量类型*/
	uint8   bynum;        /*变量字节数*/
	uint8   dotnum;       /*小数点位数*/
	void *  var;          /*变量地址*/
}PageReg_t;

typedef struct UPLOAD_VAR
{
	uint16  addr;                  /*变量寄存器地址*/
	uint8   ntype;        				 /*变量类型*/
	uint8   bynum;        				 /*变量字节数*/
	uint8   dotnum;       				 /*小数点位数*/
	void *  var;          				 /*变量地址*/
	void (*func)(void);   /*按键功能*/
}UploadOper_t;


typedef enum
{
	DATA_VAR   = 0x01,  /*数据变量*/
	STRING_VAR = 0x02,  /*字符串变量*/
	PASSWD_VAR = 0x08,  /*密码变量*/
	TIME_VAR   = 0x09,  /*时钟变量*/
	
	INNER_REG  = 0xf0,  /*内部寄存器变量*/
	
}E_REG_TYPE;

typedef enum
{
	 NONE_DOT = 0,
	 ONE_DOT  = 1,
	 TWO_DOT  = 2,
	 THREE_DOT  = 3,
	 FORE_DOT  = 4,	
}E_DOT_NUM;

typedef enum
{
	NUMBER_KEY_BOARD_CODE = 0x01,      /*与DGUS设置对应*/
	TEXT_KEY_BOARD_CODE   = 0x03,      /*与DGUS设置对应*/
	SECT_KEY_TXTBOARD_CODE = 0x05,     /*参数设置文本键盘对应*/
	SECT_KEY_BOARD_CODE = 0x07,        /*参数设置键盘对应*/
	
	KEY_ENERGY_INPUT_CODE = 0x02,
	KEY_MONEY_INPUT_CODE = 0x04,
	KEY_TIME_INPUT_CODE = 0x06,
	
}E_KEYCTRL_CODE; 



typedef struct
{
	 uint8 start;
	 uint8 stop;
	 uint8  picNo;  /*图片编号*/
	 int16 currVal;
	 uint32 inVal;
	 
}DownCount_t;


typedef struct
{
	 DEV_LOGIC_TYPE  charge;            /*充电参数*/
	 uint8           PinCode[8];        /*密码*/  
	 uint8           CardPinCode[8];
	 uint8           systime[14];        /*系统时间 20 21 06 19 13 16 00*/ 
}ScreenRcv_t;


typedef struct
{
	 uint8 statu;
	 uint8 value;
}ScreenTouch_t;


/*与屏相关全局变量管理结构*/
typedef struct 
{
	 uint8       Sysflag;           /*管理卡标志位*/
	 uint8       RFreshFg;          /*页面数据刷新标志*/
	 DownCount_t DownCounter;       /*页面倒计时时钟*/
	 ScreenRcv_t rcv;
	 ScreenTouch_t light;            /*亮度控制*/
	 uint8       errCode;            /*设备错误码*/
	 uint8       keepPic;
	 uint8       setparafg;          /*设置界面标志*/
	 uint8       gun_no;            /*枪号*/
	 uint8       ModUpDownKey;
	 uint8       bmskey;
	 uint8       powerkey;
	 uint32      lasttm;  /*最后动作时间*/
	 uint32      bms24vtm[DEF_MAX_GUN_NO];  /*辅助电源24V设置时间*/
	
	 uint32      startkeylasttm[2];  /*add0604操作确定开启充电按钮最后动作时间*/  
}ScreenLocalPara_t;

typedef enum
{
	E_SAFE1 =  1,
	E_SAFE2 =  2,
	E_SAFE3 =  3,
	
}E_SYSSAFE_CLASS;


typedef enum
{
	 E_PICTURE0 = 0,
   E_PICTURE1 = 1, 
	 E_PICTURE2 = 2, 
	 E_PICTURE3 = 3,
	 E_PICTURE4 = 4,
	 E_PICTURE5 = 5,
	 E_PICTURE6 = 6,
	 E_PICTURE7 = 7,
	 E_PICTURE8 = 8,
	 E_PICTURE9 = 9,
	 E_PICTURE10 = 10,
	 E_PICTURE11 = 11,
	 E_PICTURE12 = 12,	
	 E_PICTURE13 = 13,
	 E_PICTURE14 = 14,
	 E_PICTURE15 = 15,	
	 E_PICTURE16 = 16,
	 E_PICTURE17 = 17,
	 E_PICTURE18 = 18,	 
	 E_PICTURE19 = 19,
	 E_PICTURE20 = 20,
	 E_PICTURE21 = 21,
	 E_PICTURE22 = 22,
	 E_PICTURE23 = 23,
	 E_PICTURE24 = 24,
	 E_PICTURE25 = 25,
	 E_PICTURE26 = 26,
	 E_PICTURE27 = 27,
	 E_PICTURE28 = 28,
	 E_PICTURE29 = 29,
	 E_PICTURE30 = 30,
	 E_PICTURE31 = 31,
	 E_PICTURE32 = 32,	 
   E_PICTURE33 = 33,
   E_PICTURE34 = 34,
	 E_PICTURE35 = 35,
	 E_PICTURE36 = 36,
	 E_PICTURE37 = 37,
	 E_PICTURE38 = 38,
	 E_PICTURE39 = 39,
	 E_PICTURE40 = 40,
	 E_PICTURE41 = 41,	
	 E_PICTURE42 = 42,
	 E_PICTURE43 = 43,
	 E_PICTURE44 = 44,
	 E_PICTURE45 = 45,
	 E_PICTURE46 = 46,
	 E_PICTURE47 = 47,  //单枪空闲
	 E_PICTURE48 = 48,  //单枪充电
	 E_PICTURE49 = 49,  //单枪故障
	 E_PICTURE50 = 50,  //单枪结算

	 E_PICTURE51 = 51,  //充电开启方式
	 E_PICTURE52 = 52,
	 
   E_PICTUREANY = 0xff,
	 
}E_PIC_DEF;


typedef struct
{
	uint8 u8GunAStu;
	uint8 u8GunBStu;
	uint8 CurrPic;
	uint8 u8ChgFlag;
	uint8 u8keepPic;
}SCREN_CTRLPARA;


/*触摸屏屏任务函数*/
void Task_Screen_Main(void *p_arg);

/*触摸屏串口函数*/
extern int32 Screen_UartWrite(uint8 *buf,	int32 size);

/*触摸屏读串口函数*/
extern int32 Screen_UartRead(uint8 *buf,uint16 size);

extern void Screen_SetChgFlag(uint8 fg);


#endif

