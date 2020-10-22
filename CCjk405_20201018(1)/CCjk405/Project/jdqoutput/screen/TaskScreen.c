
//#include "M_Global.h"
//#include "Cp_Pwm.h"
#include "Hmi_Api.h"
#include "Common.h"
#include "Uart.h"
#include "message.h"
#include "ChgData.h"
#include "Hmipage.h"
#include "TaskCardReader.h"
#include "BillingUnit.h"
#include "Screen.h"
#include "Bms.h"
#include "TaskBackComm.h"
#include "wdg.h"
#include "gpio.h"



#include <stdio.h>
#include <string.h>
#include "rtc.h"


#define TOUCH_IDEL  0xaa
#define TOUCH_OPER  0x55
#define TOUCH_STOP  0xa0

#define DAYTIME_LIGHT         64
#define NIGHT_LIGHT           20
#define OPERRATE_ADD_LIGHT    40 


#define PASSWD_ERR_IN_STATUS_CHARING  0
#define PASSWD_ERR_IN_STATUS_END      1

static uint32 gPtrUartHandle = NULL;
static struct tm gScreenSystime;
static uint32 gOpertime = 0;

ScreenLocalPara_t  sGlocalPara;
 
uint16  Screen_UploadDataDeal(void);


/*刷卡打开背光*/
void Screen_OpenBackLightByCard(void);
void Screen_AlarmDisplay(uint8 num);
 
void currfeeshow(void);
void Netshow(void);
void ChargeShow(uint8 gunNo);
extern uint8 UpCardNoASCII(uint8 *buff,uint8 *CardNo);	

SCREN_CTRLPARA gScrenCtrlPara;

typedef struct
{
	uint8 u8PicNo;
	uint8 u8GunAstu1;
	uint8 u8GunBstu2;
}SCREN_PAGE_SEL;


#define  VAR0    (sGlocalPara.rcv.PinCode[0])
#define  VAR1    (sGlocalPara.rcv.charge.setenergy)
#define  VAR2    (sGlocalPara.rcv.charge.setmoney)
#define  VAR3    (sGlocalPara.rcv.charge.settime)
#define  VAR4    (sGlocalPara.rcv.CardPinCode[0])


uint8 gUploadBuf[64] = {0};
const UploadOper_t conUploadVar[] = { 
		{DGUS_PW_SHOW_ADR,STRING_VAR,6,NONE_DOT,(uint16 *)&VAR0,Page_UploadPassWd},  	/*输入密码*/
		{DGUS_SETENERGY_ADR,DATA_VAR,2,NONE_DOT,(uint16 *)&VAR1,Page_KeyEnergyEnter},   /*设置电量*/
		{DGUS_SETMONEY_ADR,DATA_VAR,2,NONE_DOT,(uint16 *)&VAR2,Page_KeyMoneyEnter},    /*设置金额*/
		{DGUS_AETTIME_ADR,DATA_VAR,2,NONE_DOT,(uint16 *)&VAR3,Page_KeyTimeEnter},    	/*设置时间*/
//		{0x0003,INNER_REG,10,0,(uint16 *)gUploadBuf,Page_RegPictureNo},    					/*获取图片号*/
		{0x0006,INNER_REG,10,0,(uint16 *)gUploadBuf,Page_RegIStouch},      						/*获取是否有触摸*/
		{DGUS_PW_CARD_ADR,PASSWD_VAR,10,NONE_DOT,(uint16 *)&VAR4,Page_KeyPwparam},  	/*输入密码*/ 

	{INVALID_REG_ADR,0x0000,E_PICTUREANY,NULL,NULL},   /*无效*/
};
const Keyfunc1_t conKeyRegDef[] = {
	
	  {KEY_ADRESS_DEF,0x00A0,E_PICTURE1,Page_StartCharge_A,NULL},             /*1页 A枪开始充电*/
		{KEY_ADRESS_DEF,0x00A1,E_PICTURE1,Page_StartCharge_B,NULL},             /*1页 B枪开始充电*/
    {KEY_ADRESS_DEF,0x00A2,E_PICTURE1,Page_SysSet,NULL},                  /*1页 系统设置*/
		
		{KEY_ADRESS_DEF,0x00A3,E_PICTURE5,Page_BMSDetails_B,NULL},           	  /*5页 BMS详情  */		
    {KEY_ADRESS_DEF,0x00A4,E_PICTURE5,Page_BMSDetails_A,NULL},             	/*5页 BMS详情  */
	
		{KEY_ADRESS_DEF,0x00D7,E_PICTURE5,Page_VINStop_A,NULL},        		/*VIN停止按钮*/
		{KEY_ADRESS_DEF,0x00D8,E_PICTURE5,Page_VINStop_B,NULL},        		/*VIN充电停止按钮*/
		
    {KEY_ADRESS_DEF,0x0010,E_PICTURE17,Page_KeySetMoneyChargeMode,NULL},   /*17页 充电方式    按金额充*/
    {KEY_ADRESS_DEF,0x0011,E_PICTURE17,Page_KeySetEnergyChargeMode,NULL},  /*17页 充电方式    按电量充*/
    {KEY_ADRESS_DEF,0x0012,E_PICTURE17,Page_KeySetTimesChargveMode,NULL},  /*17页 充电方式    按时间充*/
    {KEY_ADRESS_DEF,0x0013,E_PICTURE17,Page_KeySetAutoChargeMode,NULL},    /*17页 充电方式    自动充电*/
    {KEY_ADRESS_DEF,0x00A5,E_PICTURE17,Page_KeyExitChoose,NULL},           /*17页 充电方式    返回*/

    {KEY_ADRESS_DEF,0x00A6,E_PICTURE18,Page_KeyChargeModeSetReturn,NULL},  /*18页 按电量充    返回*/

    {KEY_ADRESS_DEF,0x00A7,E_PICTURE19,Page_KeyChargeModeSetReturn,NULL},  /*19页 按金额充    返回*/

    {KEY_ADRESS_DEF,0x00A8,E_PICTURE20,Page_KeyChargeModeSetReturn,NULL},  /*20页 按时间充    返回*/
		
	  {KEY_ADRESS_DEF,0x00A9,E_PICTURE21,Page_KeyBMSReturn,NULL},        		/*21页 BMS详情 返回*/
			
		{KEY_ADRESS_DEF,0x00AA,E_PICTURE22,Page_Sysreturn,NULL},            	/*22页 系统设置    返回*/
		{KEY_ADRESS_DEF,0x0020,E_PICTURE22,Page_SystemSet1,NULL},            	/*22页 系统设置*/
    {KEY_ADRESS_DEF,0x0021,E_PICTURE22,Page_RechargeRecord,NULL},       	/*22页 充电记录*/	
		{KEY_ADRESS_DEF,0x0022,E_PICTURE22,Page_Debug,NULL},                	/*22页 调试界面*/
				
		{KEY_ADRESS_DEF,0x0023,E_PICTURE23,Page_SystemSet,NULL},   		        /*23页 系统设置*/
		{KEY_ADRESS_DEF,0x0024,E_PICTURE23,Page_RatesSet,NULL},             	/*23页 费率设置*/

		{KEY_ADRESS_DEF,0x00AB,E_PICTURE24,Page_KeySetTime,NULL},           	/*24页 时间校准*/		
		{KEY_ADRESS_DEF,0x00AC,E_PICTURE24,Page_KeySystem1Read,NULL},       	/*24页 参数读取按钮*/		
		{KEY_ADRESS_DEF,0x00AD,E_PICTURE24,Page_KeySystem1Set,NULL},        	/*24页 参数设置按钮*/
		{KEY_ADRESS_DEF,0x00AE,E_PICTURE24,Page_KeySystem1Return,NULL},     	/*24页 参数返回按钮*/	

		{KEY_ADRESS_DEF,0x00AF,E_PICTURE25,Page_KeySeniorSystem,NULL},       	/*25页 参数高级设置按钮*/		
		
		{KEY_ADRESS_DEF,0x00B0,E_PICTURE26,Page_KeySystem2Read,NULL},      		/*26页 参数读取按钮*/		
		{KEY_ADRESS_DEF,0x00B1,E_PICTURE26,Page_KeySystem2Set,NULL},       		/*26页 参数设置按钮*/
		{KEY_ADRESS_DEF,0x00B2,E_PICTURE26,Page_KeySystem2Return,NULL},    		/*26页 参数返回按钮*/
		{KEY_ADRESS_DEF,0x00B3,E_PICTURE26,Page_KeySystemNext,NULL},		   		/*26页 下一页*/				
		{KEY_ADRESS_DEF,0x00B4,E_PICTURE26,Page_KeySyRestart,NULL},        		/*26页 系统复位*/
		{KEY_ADRESS_DEF,0x00B5,E_PICTURE26,Page_KeyClear,NULL},		         		/*26页 擦除记录*/

		{KEY_ADRESS_DEF,0x00B6,E_PICTURE27,Page_KeySystem3Read,NULL},      		/*27页 系统参数设置读*/
		{KEY_ADRESS_DEF,0x00B7,E_PICTURE27,Page_KeySystem3Set,NULL},       		/*27页 系统参数设置写*/
		{KEY_ADRESS_DEF,0x00B8,E_PICTURE27,Page_KeySystem3Return,NULL},    		/*27页 系统参数设置返回*/
		
		{KEY_ADRESS_DEF,0x00B9,E_PICTURE28,Page_KeyRecordUpPage,NULL},      	/*28页，记录查询上一页按钮*/		
		{KEY_ADRESS_DEF,0x00BA,E_PICTURE28,Page_KeyRecordDownPage,NULL},    	/*28页，记录查询下一页按钮*/
    {KEY_ADRESS_DEF,0x00BB,E_PICTURE28,Page_KeyRecordReturn,NULL},      	/*28页  返回*/
					
  	{KEY_ADRESS_DEF,0x00BC,E_PICTURE29,Page_KeyRatesRead_A,NULL},        	/*29页 费率读取按钮*/		
		{KEY_ADRESS_DEF,0x00BD,E_PICTURE29,Page_KeyRatesSet_A,NULL},         	/*29页 费率设置按钮*/
		{KEY_ADRESS_DEF,0x00BE,E_PICTURE29,Page_KeyRatesReturn,NULL},      	 /*29页 费率返回按钮*/	
		
    {KEY_ADRESS_DEF,0x00BF,E_PICTURE30,Page_KeyDebugNext,NULL},        		/*30页 调试界面 下一页*/		
		{KEY_ADRESS_DEF,0x00C0,E_PICTURE30,Page_KeyModeDetail,NULL},       		/*30页 模块详情 */		
	  {KEY_ADRESS_DEF,0x00C1,E_PICTURE30,Page_KeyDebugReturn,NULL},		   		/*30页 调试界面 返回*/
		
		{KEY_ADRESS_DEF,0x00C2,E_PICTURE31,Page_KeyDebugPrev,NULL},		     		/*31页 调试界面2 上一页*/	
		{KEY_ADRESS_DEF,0x00C3,E_PICTURE31,Page_KeyDebugReturn,NULL},      		/*31页 调试界面 返回*/
		
		{KEY_ADRESS_DEF,0x00C4,E_PICTURE32,Page_KeyModePrev,NULL},         		/*32页 模块详情  上一页*/		
		{KEY_ADRESS_DEF,0x00C5,E_PICTURE32,Page_KeyModeNext,NULL},         		/*32页 模块详情  下一页*/
		{KEY_ADRESS_DEF,0x00C6,E_PICTURE32,Page_KeyModeReturn,NULL},       		/*32页 模块详情   返回*/		
		
	  {KEY_ADRESS_DEF,0x00C7,E_PICTURE33,Page_KeyPassWdEnter,NULL},      		/*33页 密码输入    确认*/
    {KEY_ADRESS_DEF,0x00C8,E_PICTURE33,Page_KeyPwReturn,NULL},         		/*33页 密码输入    返回*/
		
		{KEY_ADRESS_DEF,0x00C9,E_PICTURE34,Page_KeyPwCardEnter,NULL},     		/*34页 卡密码确认*/
		{KEY_ADRESS_DEF,0x00CA,E_PICTURE34,Page_KeyPwCardReturn,NULL},    		/*34页 卡密码返回*/
				
		{KEY_ADRESS_DEF,0x00CB,E_PICTURE39,Page_KeyRateChoose_Return ,NULL},   /*39页 费率选择 返回*/
		{KEY_ADRESS_DEF,0x0025,E_PICTURE39,Page_KeyRateChoose_A,NULL},    		 /*39页 费率选择 A枪*/
		{KEY_ADRESS_DEF,0x0026,E_PICTURE39,Page_KeyRateChoose_B,NULL},    		 /*39页 费率选择 B枪*/
		
  	{KEY_ADRESS_DEF,0x00CC,E_PICTURE40,Page_KeyRatesRead_B,NULL},        		/*29页 费率读取按钮*/		
		{KEY_ADRESS_DEF,0x00CD,E_PICTURE40,Page_KeyRatesSet_B,NULL},         		/*29页 费率设置按钮*/
		{KEY_ADRESS_DEF,0x00CE,E_PICTURE40,Page_KeyRatesReturn,NULL},      		  /*29页 费率返回按钮*/			
					
		{KEY_ADRESS_DEF,0x00D0,E_PICTURE1,Page_KeyAuciliaryPow_A,NULL},       /*A枪切换辅源*/
		{KEY_ADRESS_DEF,0x00D1,E_PICTURE1,Page_KeyAuciliaryPow_B,NULL},       /*B枪切换辅源*/
		
		{KEY_ADRESS_DEF,0x0030,E_PICTURE45,Page_KeyAuPowChoose12,NULL},       /*45页 辅源界面返回*/
		{KEY_ADRESS_DEF,0x0031,E_PICTURE45,Page_KeyAuPowChoose24,NULL},		    /*45页 辅源界面12V*/
		{KEY_ADRESS_DEF,0x00D2,E_PICTURE45,Page_KeyAuPowReturn,NULL},		      /*45页 辅源界面24V*/
		
		{KEY_ADRESS_DEF,0x00D3,E_PICTURE46,Page_KeyAuPowConfirm,NULL},        /*46页 确认*/
		{KEY_ADRESS_DEF,0x00D4,E_PICTURE46,Page_KeyAuPowCancel,NULL},         /*46页 取消*/
		
		{KEY_ADRESS_DEF,0x00D5,E_PICTURE1,Page_KeyVINStart_A,NULL},
		{KEY_ADRESS_DEF,0x00D6,E_PICTURE1,Page_KeyVINStart_B,NULL},


		{KEY_ADRESS_DEF,0x00DA,E_PICTURE51,Page_KeyStartModeReturn,NULL},           /*51页 返回*/
		{KEY_ADRESS_DEF,0x00DB,E_PICTURE51,Page_KeyPasswdModeEnter,NULL},           /*51页 密码充电确认*/
//		{KEY_ADRESS_DEF,0x00DC,E_PICTURE46,Page_KeyCardModeEnter,NULL},           /*51页 密码充电确认*/

		{KEY_ADRESS_DEF,0x00E0,E_PICTURE52,Page_KeyChargPwdReturn,NULL},           /*52页 返回*/
		{KEY_ADRESS_DEF,0x00E1,E_PICTURE52,Page_KeyChargPwdEnter,NULL},           /*52页 充电密码确认*/
 
		
		{INVALID_REG_ADR,0x0000,E_PICTUREANY,NULL,NULL},   										/*无效*/
};


typedef struct
{
   uint8 code;
   char * str;
}STR_ERR_ST;

const STR_ERR_ST conErrStr[]=
{ 
	{ECODE_NORMAL,            "ERROR!!!"},
  {ECODE1_ELOCK,            "电子锁故障"},
	{ECODE2_BHMTM,            "BHM超时"},
	{ECODE3_XFTM,             "泄放超时"},
	{ECODE4_BRMTM,            "BRM超时"},
	{ECODE5_BCPTM,            "BCP超时"},
	{ECODE6_BROTM,            "BRO超时"},
	{ECODE7_BCLTM,            "BCL超时"},
	{ECODE8_BCSTM,            "BCS超时"},
	{ECODE9_BSTTM,            "BST超时"},
	{ECODE10_CSTTM,           "CST超时"},
	{ECODE11_BSDTM,           "BSD超时"},
	{ECODE12_KM10V,           "KM外侧电压大于10V"},
	{ECODE13_KM60V,           "直流接触器粘连"},//"KM外侧电压大于60V"},
	{EOCDE14_PARAM,           "车辆参数不合适"},
	{ECODE15_PKGVOLT,         "与报文电压偏差大于±5%"},
	{ECODE16_BATSTU,          "电池状态不正常"},
	{ECODE17_FORBID,          "车辆BMS禁止充电"},
	{ECODE18_CC1LNK,          "充电枪连接错误"},
	{ECODE19_MODVOLT,         "模块电压调整错误"},
	{ECODE20_INOUT10V,        "KM内外侧电压差大于10V"},
	{ECODE31_BMSCOMM ,        "BMS通讯错误"},
	{ECODE32_GUNTMPOVER,      "充电枪过温"},
  {ECODE35_ISOCOMM,         "绝缘模块通讯故障"},
  {ECODE36_1POSERR,         "绝缘正对地故障"},
	{ECODE37_1NEGERR,         "绝缘负对地故障"},
	{ECODE38_2POSERR,         "绝缘故障"},//"绝缘二路正对地故障"},
	{ECODE39_2NEGERR,         "绝缘二路负对地故障"},
	{ECODE41_MODCOMMERR,      "电源模块通讯故障"},
	{ECODE42_DC1OVER,         "直流输出过压"},
	{ECODE43_DC2OVER,         "2直流输出过压"},
	{ECODE44_DC3OVER,         "3直流输出过压"},
	{ECODE45_DC4OVER,         "4直流输出过压"},
	{ECODE50_BMSFINISH,       "收到BMS停止报文,达到SOC设定值"},
	{ECODE51_BMSTVOLT,        "收到BMS停止报文,达到总电压设定值"},
	{ECODE52_BMSSVOLT,        "收到BMS停止报文,达到单体电压设定值"},
	{ECODE53_BMSCST,          "收到BMS停止报文,充电机主动终止"},
	{ECODE55_BMSISO,          "收到BMS绝缘故障报文"},
	{ECODE56_BMSLKTMP,        "收到BMS输出连接器过温度故障"},
	{ECODE57_BMSPARTTMP,      "收到BMS元件故障"},
	{ECODE58_BMSCLKERR,       "收到BMS充电连接器故障"},
	{ECODE59_BMSBATTMP,        "收到BMS电池组温度过高"},
	{ECODE60_BMSKMERR,        "收到BMS高压继电器故障"},
	{ECODE61_BMSCHK2ERR,      "收到BMS检测点2电压检测故障"},
	{ECODE62_BMSOTHERERR,     "收到BMS其它故障"},
	{ECODE64_BMSCURROVER,     "收到BMS电流过大故障"},
	{ECODE65_BMSVOLTERR,      "收到BMS电压异常故障"},
	{ECODE67_BMSCRMTM,        "BMS接收充电机CRM 00报文超时"},
	{ECODE68_BMSCRMATM,       "BMS接收充电机CRM AA报文超时"},
	{ECODE69_BMSCTSTM,        "BMS接收充电机CTS和CML报文超时"},
	{ECODE70_BMSCROTM,        "BMS接收充电机CRO报文超时"},
	{ECODE71_BMSCCSTM,        "BMS接收充电机CCS报文超时"},
	{ECODE72_BMSCSTTM,        "BMS接收充电机CST报文超时"},
	{ECODE73_BMSCSDTM,        "BMS接收充电机CSD报文超时"},
	{ECODE74_BMSST10MIN,      "充电暂停超过30分钟"},
	{ECODE80_CHGFINISH,       "达到充电机设定条件中止"},
	{ECODE81_CHGHUMST,        "人工主动停止"},
	{ECODE82_CHGERRST,        "故障停止"},
	{ECODE83_CHGBST,          "收到BMS中止报文停止"},
	{ECODE84_NOMONEY,			    "账户余额低于下限中止"},
	{ECODE85_CHGTMOVER,       "充电机过温度故障"},
	{ECODE86_CHGLKERR,        "充电连接器故障"},
	{ECODE87_CHGINTMPERR,     "充电机内部过温故障"},
	{ECODE88_CHGENERGY,       "电量不能传送"},
	{ECODE89_CHGJTST,         "急停故障,请向右旋扭急停按钮恢复"},
  {ECODE90_CHGOTHER,        "其它故障"},
	{ECODE91_CHGCURRERR,      "充电电流不匹配"},    
	{ECODE92_CHGVOLTERR,      "充电机电压异常"},
	{ECODE93_METERCOMMERR,    "直流表通讯故障"},
	{ECODE94_CURROVER,        "直流输出过流"},
	{ECODE95_ACMETCOMMERR,    "交流表通讯故障"},
	{ECODE96_ACVOVERR,        "交流输入过压"},
	{ECODE97_ACUnerERR,       "交流输入欠压"},
	{ECODE98_ACDEFICY,        "交流缺相"},
	{ECODE99_DooracsERR,      "门禁1故障"},
	{ECODE100_DoorafERR,      "门禁2故障"},
	{ECODE101_DCFAUSEERR,     "直流熔丝故障"},   //20181206 add
	{ECODE102_SPDERR,         "防雷器故障"},
	{ECODE103_GUNOVERTEMPER,     "枪过温度故障"},
	{ECODE104_PAVOUTSIDEREVERSE,  "枪外侧电压反接(BMS端)"},
	{ECODE105_PAVINSIDEREVERSE,   "枪内侧电压反接(模块端)"},
	{ECODE106_VINBACKTM,          "后台获取VIN信息超时"},
	{ECODE107_BACKOFF,"后台通讯故障"},
	{ECODE108_BCPOVERVOLT,"充电电压超过BCP报文最高允许值"},
};

#define ERROR_NUM (sizeof(conErrStr)/sizeof(STR_ERR_ST))

const STR_ERR_ST *Screen_GetErrSting(uint8 code)
{
   uint8 i;	 
	 const STR_ERR_ST *prt = NULL;
	 
	 for(i = 0; i < ERROR_NUM; i++)
	 {
	    if(conErrStr[i].code == code)
			{
			   prt = &conErrStr[i];
				 return prt;
			}
	 }
	 
	 return &conErrStr[0];

}

/***********************************************************************
***函数名称: Screen_GetKeyItemByKeyValue
** 功能描述: 通过按键值查找按键的条目
***    参数: 
**   返回值:   
*************************************************************************/
const Keyfunc1_t * Screen_GetKeyItemByKeyValue(uint16 keyvalue)
{
	 uint16 i;
	 const Keyfunc1_t *keyitem = &conKeyRegDef[0];
	 for( i = 0 ; keyitem->keyaddr != INVALID_REG_ADR ; i++ ) {
		 if( keyvalue == keyitem->code ) {
			   return  keyitem;
		 }
		 keyitem++;
	 }
	 return NULL;
 }

  
/***********************************************************************
***函数名称: Screen_GetUpLoadVarItem
** 功能描述: 通过地址查找寄存器上传的条目
***    参数: 
**   返回值:   
*************************************************************************/
const UploadOper_t *Screen_GetUpLoadVarItem(uint16 adr)
{
	uint16 i;
	const UploadOper_t *varitem = &conUploadVar[0];
	
	for( i = 0 ; varitem->addr != INVALID_REG_ADR ; i++ ) {
		if( varitem->addr == adr ) {
			return varitem;
		}
		varitem++;	
	}
	 return NULL;
}

/***********************************************************************
***函数名称: Screen_KeyDeal
** 功能描述: 按键执行的动作
***    参数: 
**   返回值:   
*************************************************************************/
void Screen_KeyDeal(uint16 adr,uint16 val)
{
 	 const Keyfunc1_t *key =  Screen_GetKeyItemByKeyValue(val);
	 if( NULL == key ) {
		 return;
	 }
	 
	 if( key->func != NULL ) {
		 key->func();
	 }

}


/***********************************************************************
***函数名称: Screen_GetUpLoadVarItemNum
** 功能描述: 获取上传设置变量的个数
***    参数: 
**   返回值:   
*************************************************************************/
uint16 Screen_GetUpLoadVarItemNum(void)
{
	 uint16 i;
	 for( i = 0; conUploadVar[i].addr != INVALID_REG_ADR; i++ ) {
		 ;
	 }
	 return i;
	
}

/***********************************************************************
***函数名称: Screen_DataDotChange
** 功能描述: 填充数据进行小数点转换
***    参数: 
**   返回值:   
*************************************************************************/
uint8 Screen_DataDotChange(void *var,uint8 bynum,int8 dotpos)
{
	 uint8 i,tmp;
	 uint32 da = 0;
	 uint32 dot,md;
	 
   bynum = bynum > sizeof(uint32)	? sizeof(uint32):bynum; 
	 memcpy(&da,var,bynum);
	
	 tmp = dotpos < 0 ?dotpos*(-1):dotpos;
	
	 dot = 1;
	 for( i = 0; i< tmp ; i++) {
		 dot *= 10;
	 }
	
	 if( dotpos >= 0) {
		 da *= dot;
	 }else {
		 da /= dot;
		 md = da % dot;
		 if( md >= dot /2 ) {
			 da += 1;
		 }
	 }
   memcpy(var,&da,bynum);
	 
	 return 0;
}

/***********************************************************************
***函数名称: Screen_Fill_DataVar
** 功能描述: 填充数据变量的值,用于接收数据赋值
***    参数: 
**   返回值:   
*************************************************************************/
uint16 Screen_Fill_DataVar(uint16 *buf,void *var,uint8 bynum,int8 dot)
{
	  uint16 cnt = 0;
	  uint16 tmp16; 
	  uint32 tmp32 = 0;
	
	  cnt = 0;
		switch(bynum){
			case 1:
				{
					memcpy(&tmp16,var,1);
					tmp16 &= 0x00ff;
					Common_Change2Byte(&tmp16);
					Screen_DataDotChange(&tmp16,2,dot);
					buf[cnt++] = tmp16;
				}
	      case 2:
				{
					memcpy(&tmp16,var,2);
					Common_Change2Byte(&tmp16);
					Screen_DataDotChange(&tmp16,2,dot);
					buf[cnt++] = tmp16;
					break;
				}
				case 4 :
				{
					memcpy(&tmp32,var,4);
					Common_Change4Byte(&tmp32);
					Screen_DataDotChange(&tmp16,4,dot);
					buf[cnt++] = tmp32 & 0x0000ffff;
					buf[cnt++] = tmp32 >> 16;
					break;
				}default:
				  break;
		}
		return cnt;	
}

/***********************************************************************
***函数名称: Screen_ChangePassWord
** 功能描述: 从协议帧中获取密码，并按bcd格式转换
***    参数: 
**   返回值:   
*************************************************************************/
uint8 Screen_ChangePassWord(uint8 *obj,uint16 *src,uint16 len)
{
	 uint8 i;
	 uint8 pw[8] = {0};
	 uint8 *ptr = (uint8 *)src;
   len = len*2 > 8 ? 4 : len;

	 for( i = 0 ; i < len*2 ; i++) {
		 if(ptr[i] != 0xff ) {
			  pw[i/2] |= ptr[i]-0x30;
			  if(i%2 == 0) {
					pw[i/2] <<= 4;
				}
			  if(ptr[i+1] == 0xff ) {
					break;
				}
		 }else {
			 break;
		 }	 
	 }
	 memcpy(obj,pw,(i+2)/2);
	 return i+1;
}


/***********************************************************************
***函数名称: Screen_UploadVarDeal
** 功能描述: 上传数据处理
***    参数: 
**   返回值:   
*************************************************************************/
void Screen_UploadVarDeal(uint16 adr,uint16 regNum,uint16 *data)
{
	uint16 i;
	uint8 *ptr = (uint8 *)data;
	const UploadOper_t *varitem = Screen_GetUpLoadVarItem(adr);
	if( NULL == varitem ) {
		return;
	}
	
	switch (varitem->ntype ) {
		case DATA_VAR:
		{
			
			Screen_Fill_DataVar(varitem->var,(void*)data,varitem->bynum,varitem->dotnum);
			break;
		}
		case PASSWD_VAR:
		{
			Screen_ChangePassWord(varitem->var,data,regNum);
			break;
		}
		case STRING_VAR:
		{
			 for( i = 0; i < regNum*2;i++ ) {
				 if(ptr[i] == 0xff ) {
					 ptr[i] = 0;
					 break;
				 }
			 }
			 memcpy(varitem->var,data,regNum*2);
		}
		break;
		case TIME_VAR:
		{
			memcpy(varitem->var,data,regNum*2);
		}
		break;
		case INNER_REG:
		{
			memcpy(varitem->var,data,regNum*2);
		}
		break;
		default:
			break;	
	}
	
	if( varitem->func != NULL ) {
		varitem->func();               /*参数待修改*/
	}
		
}

/***********************************************************************
***函数名称: Screen_UploadDataDeal
** 功能描述: 触摸屏接收数据的处理
***    参数: 
**   返回值:   
*************************************************************************/

uint16  Screen_UploadDataDeal(void)
{
	stoc_u sc;
	uint8 cnt,stat,dlen;
	uint16 len,tlen,num;
	uint8 rcvbuf[256];
	uint16 reg16,data16;
	ScrInRegFrame_t *sRegFrame = NULL;
	ScrFrame_t *sframe = (ScrFrame_t *)rcvbuf;
	
	uint8 *ptr = NULL;
	ptr = rcvbuf;
	tlen = 0;
	cnt = 0;
	stat = 0;
	num = 0;
	len = 1;
	do {
		tlen = Uart_Read(gPtrUartHandle,ptr,len);
		if( tlen > 0 ) {
			
			switch(stat)
			{
				 case 0:
				 {
					  if( FRAME_HEAD1 == *ptr ) {
							 ptr++ ;
							 stat = 1;
						}
						break;
				 }
				 case 1:
				 {
					   if( FRAME_HEAD2 == *ptr ) {
							 ptr++;
							 stat = 2;
						}else {
							stat = 0;
							ptr = rcvbuf;
						}
						break;
				 }
				 case 2:
				 {
					   len =  *ptr % 252;//255-3
					   dlen = len;
					   stat = 3;
					   ptr++;
             break;
				 }
				 case 3:
				 {
					  if( tlen == len ) {
							 ptr += tlen;
							 tlen = dlen + 3;
							 goto END;
						}else if (tlen < len ){
							 len = tlen - len;
							 ptr += tlen;
						}
					 break; 
				 }
			}
		}
		else {
			Delay10Ms(1);
			cnt++; 
		}
		
  }while((cnt < 3 ) && (ptr < &rcvbuf[100])); /*接收大于100字节退出*/
	
	
END:
  	num = (uint8)(ptr - &rcvbuf[0]);
	
	if( num < 3 || tlen < 5 ) {
		   return FALSE;
	  }
	
	
//	printf("Screen_UploadDataDeal len=%d\r\n", tlen);
	/*校验报文*/
	if( RT_ERROR == Hmi_PackgCheck(rcvbuf,tlen) ) {
		 return FALSE;
	}

	sc.c[0] = sframe->nsReg >> 8;
	sc.c[1] = sframe->nsReg;
	
	reg16 = sframe->nsReg;
	data16 = sframe->nsData[0];
	Common_Change2Byte(&reg16);
	Common_Change2Byte(&data16);
	
	if ( KEY_ADRESS_DEF == sc.s ) {
		if(0 != sGlocalPara.light.value)
		{
			Screen_KeyDeal(reg16,data16);	   /*按键上传数据*/
		}
		Screen_OpenBackLightByCard();
		sGlocalPara.lasttm = GetSystemTick();
	}
	else {
		if(sframe->cmd != CMD_RD_REG ) {
			Screen_OpenBackLightByCard();
			sGlocalPara.lasttm = GetSystemTick();	
			Screen_UploadVarDeal(reg16,sframe->num,sframe->nsData);  /*数据录入上传数据*/
		}else { /*CMD = 0x81 读内部寄存器返回*/
			sRegFrame = (ScrInRegFrame_t *)sframe;
			Screen_UploadVarDeal(sRegFrame->nReg,sRegFrame->num,sRegFrame->nsData);
		}
		
	}
	
	return TRUE;
}


/***********************************************************************
***函数名称: Screen_ClearShowVar
** 功能描述: 清除显示变量
***    参数: 
**   返回值:   
*************************************************************************/
void Screen_ClearShowVar(void)
{
	  char *str = "      ";
	  uint8 bynum = 0;
	  uint8 buf[MAX_REG_NUM_ONCE_SEND*2] = {0};
    uint8 sendbuf[128] = {0};
    uint16 regAdr = 0 ;
   
		bynum = Hmi_PackWriteReg(sendbuf,buf,regAdr,CMD_WR_VAR,bynum);
		Screen_UartWrite(sendbuf,bynum);

    regAdr = conUploadVar[1].addr;
    bynum = (Screen_GetUpLoadVarItemNum()-1)*2*2 %64; /*不大于64个*/
		memset(sendbuf,0,bynum);
    
		bynum = Hmi_PackWriteReg(sendbuf,buf,regAdr,CMD_WR_VAR,bynum);
		Screen_UartWrite(sendbuf,bynum);
		
    Delay10Ms(1);
    Hmi_ShowText(DGUS_IDLE_PROADR_A,strlen(str),(uint8 *)str);
    Delay10Ms(1);
    Hmi_ShowText(DGUS_IDLE_PROADR_B,strlen(str),(uint8 *)str);
		Delay10Ms(1);
		Hmi_ShowText(DGUS_CHARG_PROADR_A,strlen(str),(uint8 *)str);
		Delay10Ms(1);
		Hmi_ShowText(DGUS_CHARG_PROADR_B,strlen(str),(uint8 *)str);  
    Delay10Ms(1);
		Hmi_ShowText(DGUS_ACCOUNT_PROADR_A,strlen(str),(uint8 *)str);
		Delay10Ms(1);
		Hmi_ShowText(DGUS_ACCOUNT_PROADR_B,strlen(str),(uint8 *)str);  
    Delay10Ms(1);		
		Hmi_ShowText(DGUS_CHARWAY_PROADR,strlen(str),(uint8 *)str);		
    Delay10Ms(1);
		Hmi_ShowText(DGUS_INPUT_PROADR,strlen(str),(uint8 *)str);			
	  Delay10Ms(1);				
		Hmi_ShowText(DGUA_PWSH_ADR,strlen(str),(uint8 *)str); 
	  Delay10Ms(1);
		Hmi_ShowText(DGUS_PW_CARDSH_ADR,strlen(str),(uint8 *)str); 
    Delay10Ms(1);



}

/***********************************************************************
***函数名称: Screen_SetTimeToBoardOnKey
** 功能描述: 将触摸屏时间设置到主控板
***    参数: 
**   返回值:   
*************************************************************************/
uint8 Screen_SetTimeToBoardOnKey(void)
{
	 #define MAX_R_SIZE 100
	 uint8 cnt,errcnt;
	 uint8 tlen ,slen;
	 uint8 len = 0;
	 uint8 buf[MAX_R_SIZE] = {0};
   uint8 *ptr = buf;

   tlen = 0;
   cnt = 0;
   errcnt = 0;
   slen = 0;
	 Hmi_RequestScreenTime();  //请求触摸屏时间发送函数
   Delay10Ms(2);
	 do {
		len = Uart_Read(gPtrUartHandle,ptr,MAX_R_SIZE-tlen);
		if( len > 0 ) {
		  ptr += len;
		  tlen += len;
			if( tlen >= MAX_R_SIZE ) {
				break;
			}
		}
		if( tlen != slen ) {
			slen = tlen;
		}else if(slen > 2 ){
			cnt++;
		}
		errcnt++;
	  Delay10Ms(1);
  }while( (errcnt < 5) && (cnt < 1) ); /*接收大于100字节退出*/
	
	if( tlen < 6 ) {
		return FALSE;
	}
	
//	printf("Screen_SetTimeToBoardOnKey len=%d\r\n", tlen);
	/*校验报文*/
	if( RT_ERROR == Hmi_PackgCheck(buf,tlen) ) {
		 return FALSE;
	}
	
	struct tm systime;
	systime.tm_year = Common_Bcd2hex8(buf[6])+2000;
	systime.tm_mon  = Common_Bcd2hex8(buf[7]);
	systime.tm_mday = Common_Bcd2hex8(buf[8]);
	systime.tm_wday = Common_Bcd2hex8(buf[9]);
	systime.tm_hour = Common_Bcd2hex8(buf[10]);
	systime.tm_min  = Common_Bcd2hex8(buf[11]);
	systime.tm_sec  = Common_Bcd2hex8(buf[12]);
	
	RTC_SetDateTime(systime);
		
	return TRUE;
}




/***********************************************************************
***函数名称: Screen_UartWrite
** 功能描述: 触摸屏串口发送数据函数
***    参数: 
**   返回值:   
*************************************************************************/
int32 Screen_UartWrite(uint8 *buf,	int32 size)
{
	#if 1
	return Uart_Write(gPtrUartHandle,buf,size);
	#else
	return 0;
	#endif
}

/***********************************************************************
***函数名称: Screen_UartRead
** 功能描述: 读取触摸屏串口数据
***    参数: 
**   返回值:   
*************************************************************************/
int32 Screen_UartRead(uint8 *buf,uint16 size)
{
	return Uart_Read(gPtrUartHandle,buf,size);
}



/***********************************************************************
***函数名称: Screen_ComInit
** 功能描述: 初始化触摸屏串口
***    参数: 
**   返回值:   
*************************************************************************/
/* 初始化串口 */
void Screen_ComInit(void)
{
	int32 tmp;

	/* 获取串口 */
	UartHandler[SCREEN_COM] = Uart_Open(SCREEN_COM);
	assert_param(UartHandler[SCREEN_COM]);
  gPtrUartHandle = UartHandler[SCREEN_COM]; 
	
	/* 波特率 */
 	tmp  = 115200;
	Uart_IOCtrl(UartHandler[SCREEN_COM], SIO_BAUD_SET, &tmp);	

	/* 无校验 */
	tmp  = USART_Parity_No;
	Uart_IOCtrl(UartHandler[SCREEN_COM], SIO_PARITY_BIT_SET, &tmp);

	/* 停止位 */
	tmp  = USART_StopBits_1;
	Uart_IOCtrl(UartHandler[SCREEN_COM], SIO_STOP_BIT_SET, &tmp);	
	
	/* 8个数据位 */
	tmp = USART_WordLength_8b;
	Uart_IOCtrl(UartHandler[SCREEN_COM], SIO_DATA_BIT_SET, &tmp);	
}


/***********************************************************************
***函数名称: Screen_Init
** 功能描述: 触摸屏任务相关初始化内容
***    参数: 
**   返回值:   
*************************************************************************/
void Screen_Init(void)
{
	
	memset(&sGlocalPara,0,sizeof(ScreenLocalPara_t));
	gScreenSystime = Time_GetSystemCalendarTime();	
	sGlocalPara.light.value = DAYTIME_LIGHT;	
	sGlocalPara.light.statu = TOUCH_OPER;
  memset(&gScrenCtrlPara,0,sizeof(gScrenCtrlPara));
	sGlocalPara.lasttm = GetSystemTick();	
}


/***********************************************************************
***函数名称: Screen_LightControl
** 功能描述: 触摸屏亮度 屏保 控制
***    参数: 无
**   返回值:   
**  亮度范围 0 --- 64 
*************************************************************************/


void Screen_LightControl(void)
{
	
	static uint8  lightpic = E_PICTURE1;  //屏幕由屏保变亮时显示的界面 add1204
	static uint8  sWorkStatu[DEF_MAX_GUN_NO] = {0};
	static uint16 sStatu[DEF_MAX_GUN_NO] = {0};
	static uint32 ltime = 0;
	static uint32 ideltime = 0;
	DEV_GUN_TYPE *ptrGunA = Check_GetGunDataPtr(AGUN_NO);
	DEV_GUN_TYPE *ptrGunB = Check_GetGunDataPtr(BGUN_NO);
	
	if((sStatu[AGUN_NO] != ptrGunA->statu.bits.cc1stu)||(sStatu[BGUN_NO] != ptrGunB->statu.bits.cc1stu)  )  //枪连接
	{
		 sStatu[AGUN_NO] = ptrGunA->statu.bits.cc1stu;
		 sStatu[BGUN_NO] = ptrGunB->statu.bits.cc1stu;
		 Screen_OpenBackLightByCard();
	}

	if( (sGlocalPara.light.statu == TOUCH_OPER)	&& (0 == sGlocalPara.light.value) )  //add20200107
	{
		gScrenCtrlPara.u8keepPic = lightpic;			
		Hmi_ChangePicture(lightpic);  
	}


	if( sGlocalPara.light.statu == TOUCH_OPER ) {
		if( gScreenSystime.tm_hour < 6 || gScreenSystime.tm_hour > 19 ){
			 sGlocalPara.light.value = NIGHT_LIGHT;
		}else {
			sGlocalPara.light.value = DAYTIME_LIGHT;
		}
		ideltime = GetSystemTick();
		Hmi_SetLcdLight(sGlocalPara.light.value);	
    }
	
	if( GetSystemTick() - ltime >  WAITMS(100) ){
		 ltime = GetSystemTick();
		 if(sGlocalPara.light.statu == TOUCH_IDEL ) {
//			  sGlocalPara.light.value = sGlocalPara.light.value > 0?--sGlocalPara.light.value:0;
			  Hmi_SetLcdLight(sGlocalPara.light.value);
			  if(sGlocalPara.light.value == 0) {
					sGlocalPara.light.statu = TOUCH_STOP;
				}
		 } 
	 }
	 
	 if( sGlocalPara.light.statu == TOUCH_STOP ) {
		 if( GetSystemTick() - ideltime >  5 * 6 * TICK_TIMES_10S )
		 {
			   ideltime = GetSystemTick();
				 sGlocalPara.light.statu = TOUCH_IDEL;
		 }
		 ltime = GetSystemTick();
	 }
	 	 
	  if( (0 == sGlocalPara.light.value ) && (RT_EOK == Hmi_CheckIstouch()) ) {
	 //if (RT_EOK == Hmi_CheckIstouch()) {
			 sGlocalPara.light.statu = TOUCH_OPER;
			 gOpertime = GetSystemTick();
			 Hmi_BuzzerBeep(20);
		}
		
		if( (TskMain_GetWorkStep(AGUN_NO) != sWorkStatu[AGUN_NO]) || (TskMain_GetWorkStep(BGUN_NO) != sWorkStatu[BGUN_NO]) ) {
			sWorkStatu[AGUN_NO] = TskMain_GetWorkStep(AGUN_NO);
			sWorkStatu[BGUN_NO] = TskMain_GetWorkStep(BGUN_NO);
			Screen_OpenBackLightByCard();
		}
		
		if( (TskMain_GetWorkStep(AGUN_NO) != STEP_CHARGEING) && (TskMain_GetWorkStep(BGUN_NO) != STEP_CHARGEING) ) {
			 if( gOpertime + 1 * 3 * TICK_TIMES_10S  < GetSystemTick() ) {
						if( (sGlocalPara.light.statu !=  TOUCH_STOP) \
							 &&	(sGlocalPara.light.statu !=  TOUCH_IDEL)) {
								 
							 sGlocalPara.light.statu = TOUCH_STOP;
							 ideltime = GetSystemTick();
						}
			 }
		}
    else {
			 sGlocalPara.light.statu = TOUCH_OPER;
			 gOpertime = GetSystemTick();
		}
}



/***********************************************************************
***函数名称: Screen_ShowMessage
** 功能描述: 显示字符串
***    参数: 
**   返回值:   
*************************************************************************/
void Screen_ShowMessage(char *message,uint16 adr)
{
	 Hmi_ShowText(adr,strlen(message),(uint8 *)message);
}


/***********************************************************************
***函数名称: Screen_ShowDateTime
** 功能描述: 日历时间显示，一天触摸屏和主板对时一次
***    参数: 
**   返回值:   
*************************************************************************/
void Screen_ShowDateTime(void)
{
	  static uint32 rTick = 0;
	  static uint8 sCnt = 0;
	  uint8 buf[64] = {0};
	 
    if( GetSystemTick() - rTick > SHOW_DATESHOW_TIMES/2 ) {
			 rTick = GetSystemTick();
			
			if(GetSystemTick() > 1*TICK_TIMES_1MIN && (sCnt++ < 10) ) {
				 //Screen_SetTimeToBoard();				
			}

			gScreenSystime = Time_GetSystemCalendarTime();
			
			sprintf((char*)buf, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d\r\n", gScreenSystime.tm_year,gScreenSystime.tm_mon,gScreenSystime.tm_mday,
			gScreenSystime.tm_hour,gScreenSystime.tm_min,gScreenSystime.tm_sec);
		 
			Screen_ShowMessage((char *)buf,DGUS_TIMESHOW_ADR);
			
			/*每6个小时对时一次*/
			if( ((gScreenSystime.tm_hour == 0) || (gScreenSystime.tm_hour == 6) \
				|| (gScreenSystime.tm_hour == 12) || (gScreenSystime.tm_hour == 18)) \
			&& (gScreenSystime.tm_min == 5) && (gScreenSystime.tm_sec == 5) ){
				if(TskBack_CommStatus() ) { /*没有与主站时才执行校时*/
					//Screen_SetTimeToBoard();
				}
			}
		}
}


/***********************************************************************
***函数名称: Screen_OpenBackLightByCard
** 功能描述: 打开显示屏背光
***    参数: 
**   返回值:   
*************************************************************************/

void Screen_OpenBackLightByCard(void)
{
	  sGlocalPara.light.statu = TOUCH_OPER;
	  gOpertime = GetSystemTick();
}


/***********************************************************************
***函数名称: Screen_ShowCountDown
** 功能描述: 显示倒计时
***    参数: 
**   返回值:   
*************************************************************************/
void Screen_ShowCountDown(void)
{
	 uint8 bynum;
	 uint8 sbuf[20] = {0};
	 uint16 data = sGlocalPara.DownCounter.currVal;
	 Common_Change2Byte(&data);
		 
	 bynum = Hmi_PackWriteReg(sbuf,(uint8 *)&data,DGUS_COUNTDOWN_REGADR,CMD_WR_VAR,2);
	 Screen_UartWrite(sbuf,bynum);	
}

/***********************************************************************
***函数名称: Screen_DecCountDown
** 功能描述: 倒计时
***    参数: 
**   返回值:   
*************************************************************************/
void Screen_DecCountDown(void)
{
	 if( 1 == sGlocalPara.DownCounter.start){
		  if(GetSystemTick() > TIM_1S + sGlocalPara.DownCounter.inVal) {
				 sGlocalPara.DownCounter.inVal = GetSystemTick();
				 if( sGlocalPara.DownCounter.currVal > 0 ) {
					 sGlocalPara.DownCounter.currVal--;
				 }
         Screen_ShowCountDown();
			}
	 }
}

/***********************************************************************
***函数名称: Screen_StartDownCounter
** 功能描述: 开启倒计时
***    参数: 
**   返回值:   
*************************************************************************/
void Screen_StartDownCounter(uint8 picNo,uint16 sec)
{
	 sGlocalPara.DownCounter.picNo = picNo;
	 sGlocalPara.DownCounter.inVal = GetSystemTick();
	 sGlocalPara.DownCounter.currVal = sec;
	 sGlocalPara.DownCounter.start = 1;
	 Screen_ShowCountDown();
}

/***********************************************************************
***函数名称: Screen_StartDownCounter
** 功能描述: 倒计时是否时间到
***    参数: 
**   返回值:   
*************************************************************************/
uint8 Screen_IsDownCounterToZero(void)
{
	 uint8 ret = FALSE;
	 if( 1 == sGlocalPara.DownCounter.start ) {
		  if( sGlocalPara.DownCounter.currVal < 1) {
				sGlocalPara.DownCounter.start = 0;
				ret = TRUE;
			}
	 }else {
		 ret = FALSE;
	 }
	 return ret;
}

/***********************************************************************
***函数名称: Screen_DownCounterStop
** 功能描述: 停止倒计时
***    参数: 
**   返回值:   
*************************************************************************/
void Screen_DownCounterStop(void)
{
	 sGlocalPara.DownCounter.start = 0;
	 sGlocalPara.DownCounter.currVal = 0;
	 sGlocalPara.DownCounter.inVal = GetSystemTick();
}



/***********************************************************************
***函数名称: Screen_Fill_VarToBuf
** 功能描述: 用于将变量的值发送到触摸屏
***    参数: 
**   返回值:   
*************************************************************************/

uint16 Screen_Fill_VarToBuf(uint16 *buf,void *var,uint8 bynum,int8 dot)
{
	  uint16 cnt = 0;
	  uint16 tmp16; 
	  uint32 tmp32 = 0;
	
	  cnt = 0;
		switch(bynum){
			case 1:
				{
					memcpy(&tmp16,var,1);
					tmp16 &= 0x00ff;
					Screen_DataDotChange(&tmp16,2,dot);
					Common_Change2Byte(&tmp16);
					buf[cnt++] = tmp16;
				}
	      case 2:
				{
					memcpy(&tmp16,var,2);
					Screen_DataDotChange(&tmp16,2,dot);
					Common_Change2Byte(&tmp16);
					buf[cnt++] = tmp16;
					break;
				}
				case 4 :
				{
					memcpy(&tmp32,var,4);
					Screen_DataDotChange(&tmp16,4,dot);
					Common_Change4Byte(&tmp32);
					buf[cnt++] = tmp32 & 0x0000ffff;
					buf[cnt++] = tmp32 >> 16;
					break;
				}default:
				  break;
		}
		return cnt;	
}



/***********************************************************************
***函数名称: Screen_ShowCyclic 
** 功能描述: 触摸屏低速循环显示任务
***    参数: 
**   返回值:   
*************************************************************************/
const char *concharway[] = \
{
  "刷卡启动",
	"移动客户端启动",
  "在线卡启动",
	"VIN启动",
	"插枪即充",
	"手动启动",
	"账户密码启动",
};

#define CHARWAY_MUN  ((sizeof(concharway)/sizeof(char *)) + 1)

const char *concharstopway[] = \
{
  "请刷卡停止",
	"请使用移动客户端停止",	
  "请使用在线卡停止",
	"请点击按钮停止",
	"请点击按钮停止",
	"请手动停止",
	"请点击按键停止",
};

const char * conReasonString[] = \
{
   "未知原因停止",
	 "刷卡停止",
	 "移动客户端停止",
	 "故障停止",
	 "达到设定条件停止",
	 "BMS主动停止",
	 "补传订单",
	 "触摸屏按键停止",
};

#define REASOM_MUN  ((sizeof(conReasonString)/sizeof(char *)) + 1)


/*清除结算界面文本显示*/
void Clear_AccountShow(uint8 guno)
{
	uint8 i;
	uint16 TempAddr;
	if(AGUN_NO == guno)
	{
	  TempAddr = DGUS_REC_START_A;
	}
	else if(BGUN_NO == guno)
	{
		TempAddr = DGUS_REC_START_B;
	}
  for( i = 0 ; i < 10 ; i++ )
  {
	   Screen_ShowMessage(" ",TempAddr);  //清除文本变量
		 TempAddr += DGUS_ACCOUNTS_OFFSET;
  }
}


/***********************************************************************
***函数名称: Screen_OpenAccountCardDeal
** 功能描述: 未结算卡处理
***    参数: 
**   返回值:   
*************************************************************************/

const uint16 conActAddr[DEF_MAX_GUN_NO][10]=\
{
  {DGUS_CARDID_ADR_A,DGUS_START_ADR_A,DGUS_STOP_ADR_A,DGUS_REASON_ADR_A,DGUS_ENERGY_ADR_A\
	 ,DGUS_MONEY_ADR_A,DGUS_TIME_ADR_A,DGUS_WAY_ADR_A,DGUS_SOC_ADR_A,DGUS_REMONEY_ADR_A},
	{DGUS_CARDID_ADR_B,DGUS_START_ADR_B,DGUS_STOP_ADR_B,DGUS_REASON_ADR_B,DGUS_ENERGY_ADR_B\
	 ,DGUS_MONEY_ADR_B,DGUS_TIME_ADR_B,DGUS_WAY_ADR_B,DGUS_SOC_ADR_B,DGUS_REMONEY_ADR_B},

};

/*结算*/
uint8 Screen_OpenAccountCardDeal(uint8 gunNo)
{
	 uint8 tmp;
	 uint16 rtn;
	 char str[10];
   uint8 text[120] = {0};
   
	 REAL_BILL_DATA_ST 	*FeeDataPtr = GetFeeData(gunNo);
   CHARGE_TYPE  *RunDataPrt = ChgData_GetRunDataPtr(gunNo);
	 
	 Clear_AccountShow(gunNo);   //清除界面显示
	 
	 /*显示卡号*/
    if( START_BY_VIN != FeeDataPtr->bill.startmod){
		 rtn = UpCardNoASCII(text,FeeDataPtr->bill.cardNo);// g_RunData.Ctrl.IcCardNo	
		}
		else{
			memcpy(text,FeeDataPtr->bill.vincode,17);
			rtn = 17;
		}	 
//	 rtn = UpCardNoASCII(text,FeeDataPtr->bill.cardNo);// g_RunData.Ctrl.IcCardNo
	 text[rtn] = 0;
	 Common_TranCarNo(text,rtn);
	 Hmi_ShowText(conActAddr[gunNo][0],rtn,text);
	 memset(text,0,sizeof(text));
	 
	 sprintf((char *)text,"%d年%d月%d日 ",Common_Bcd_Decimal(FeeDataPtr->bill.starttm[0])+2000\
	                                 ,Common_Bcd_Decimal(FeeDataPtr->bill.starttm[1])%13\
	                                 ,Common_Bcd_Decimal(FeeDataPtr->bill.starttm[2])%32 );
                                 
	  //时
	  sprintf(str,"%2d:",Common_Bcd_Decimal(FeeDataPtr->bill.starttm[3])%24 );
	  str[0] = (str[0] == 0x20)? 0x30 : str[0];
	  strcat((char *)text,str);
	  //分
	  sprintf(str,"%2d:",Common_Bcd_Decimal(FeeDataPtr->bill.starttm[4])%60 );
	  str[0] = (str[0] == 0x20)? 0x30 : str[0];
	  strcat((char *)text,str);
	  // 秒
	  sprintf(str,"%2d",Common_Bcd_Decimal(FeeDataPtr->bill.starttm[5])%60 );
	  str[0] = (str[0] == 0x20)? 0x30 : str[0];
	  strcat((char *)text,str);	 
	  /*显示开始时间*/
	  Hmi_ShowText(conActAddr[gunNo][1],strlen((char *)text),text);

//------------------
		PARAM_DEV_TYPE *ptrDevPara = ChgData_GetDevParamPtr();
	  PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
		if( (ptrDevPara->onlinecard == E_ONLINE_CARD) && (CONN_CHGBIRD == BackCOMM->agreetype) )
		{
			memset(text,0,sizeof(text));
		
			/*显示充电时长*/
			sprintf((char *)text,"%d",FeeDataPtr->bill.chgsecs/60);
			strcat((char *)text,"分钟");
			Hmi_ShowText(conActAddr[gunNo][2],strlen((char *)text),text);
			memset(text,0,sizeof(text));

			/*显示占桩时长*/   //待改
			sprintf((char *)text,"%d",FeeDataPtr->bill.parksecs/60);    
			strcat((char *)text,"分钟");
			Hmi_ShowText(conActAddr[gunNo][3],strlen((char *)text),text);
			memset(text,0,sizeof(text));

			/*显示电量*/
			Common_IntToFloatStr(text,FeeDataPtr->bill.energy,2);
			strcat((char *)text," 度");
			Hmi_ShowText(conActAddr[gunNo][4],strlen((char *)text),text);
			memset(text,0,sizeof(text));

			/*显示电费金额*/
			Common_IntToFloatStr(text,FeeDataPtr->bill.billmoney,2);
			strcat((char *)text," 元");
			Hmi_ShowText(conActAddr[gunNo][5],strlen((char *)text),text);
			memset(text,0,sizeof(text));

			/*显示服务费*/      //待改
			Common_IntToFloatStr(text,FeeDataPtr->bill.serivemoney,2);
			strcat((char *)text," 元");
			Hmi_ShowText(conActAddr[gunNo][6],strlen((char *)text),text);
			memset(text,0,sizeof(text));

			/*显示占桩费*/      //待改
			Common_IntToFloatStr(text,FeeDataPtr->bill.parkmoney,2);
			strcat((char *)text," 元");
			Hmi_ShowText(conActAddr[gunNo][7],strlen((char *)text),text);
			memset(text,0,sizeof(text));

			/*显示消费总额*/    //待改
			Common_IntToFloatStr(text,FeeDataPtr->bill.billmoney,2);
			strcat((char *)text," 元");
			Hmi_ShowText(conActAddr[gunNo][8],strlen((char *)text),text);
			memset(text,0,sizeof(text));

		}
		else
		{
//------------------

			memset(text,0,sizeof(text));
		 
			sprintf((char *)text,"%d年%d月%d日 ",Common_Bcd_Decimal(FeeDataPtr->bill.stoptm[0])+2000\
																		 ,Common_Bcd_Decimal(FeeDataPtr->bill.stoptm[1])%13\
																		 ,Common_Bcd_Decimal(FeeDataPtr->bill.stoptm[2])%32 );
																		 
			//时
			sprintf(str,"%2d:",Common_Bcd_Decimal(FeeDataPtr->bill.stoptm[3])%24 );
			str[0] = (str[0] == 0x20)? 0x30 : str[0];
			strcat((char *)text,str);
			//分
			sprintf(str,"%2d:",Common_Bcd_Decimal(FeeDataPtr->bill.stoptm[4])%60 );
			str[0] = (str[0] == 0x20)? 0x30 : str[0];
			strcat((char *)text,str);
			// 秒
			sprintf(str,"%2d",Common_Bcd_Decimal(FeeDataPtr->bill.stoptm[5])%60 );
			str[0] = (str[0] == 0x20)? 0x30 : str[0];
			strcat((char *)text,str);
			
			/*显示结束时间*/				
			Hmi_ShowText(conActAddr[gunNo][2],strlen((char *)text),text);
			memset(text,0,sizeof(text));
			/*显示原因*/
			tmp = FeeDataPtr->bill.endreason % REASOM_MUN;
			if(EERR_REASON == tmp)
			{
				const STR_ERR_ST *errstr;
				errstr = Screen_GetErrSting( FeeDataPtr->bill.errcode);
				Screen_ShowMessage(errstr->str,conActAddr[gunNo][3]);  //故障结束
			}
			else
			{
				/*正常结束*/
				Hmi_ShowText(conActAddr[gunNo][3],strlen(conReasonString[tmp]),(uint8 *)conReasonString[tmp]);
			}
			
			/*显示电量*/
			Common_IntToFloatStr(text,FeeDataPtr->bill.energy,2);
			strcat((char *)text," 度");
			Hmi_ShowText(conActAddr[gunNo][4],strlen((char *)text),text);
			memset(text,0,sizeof(text));
			
			/*显示金额*/
			Common_IntToFloatStr(text,FeeDataPtr->bill.billmoney,2);
			strcat((char *)text," 元");
			Hmi_ShowText(conActAddr[gunNo][5],strlen((char *)text),text);
			memset(text,0,sizeof(text));
		
			/*显示充电时长*/
			sprintf((char *)text,"%d",FeeDataPtr->bill.chgsecs/60);
			strcat((char *)text,"分钟");
			Hmi_ShowText(conActAddr[gunNo][6],strlen((char *)text),text);
			memset(text,0,sizeof(text));
			
			/*显示充电方式*/
			tmp = RunDataPrt->logic->startby % CHARWAY_MUN ;
			Hmi_ShowText(conActAddr[gunNo][7],strlen(concharway[tmp]),(uint8 *)concharway[tmp]);
			memset(text,0,sizeof(text));
			
			/*显式结束SOC*/
			sprintf((char *)text,"%d",FeeDataPtr->bill.soc);
			strcat((char *)text,"%");
			Hmi_ShowText(conActAddr[gunNo][8],strlen((char *)text),text);
			memset(text,0,sizeof(text));
		}
				
		/*卡余额*/
		Common_IntToFloatStr((uint8 *)str,FeeDataPtr->bill.aftermoney,2);
		strcat(str," 元");
		Screen_ShowMessage(str,conActAddr[gunNo][9]) ;	
				
		return TRUE;
}



/***********************************************************************
***函数名称: Screen_OperTimeOut
** 功能描述: 操作超时处理
***    参数: 
**   返回值:   
*************************************************************************/
void Screen_OperTimeOut(void)
{
	 uint8 PageNo;
	 if(Screen_IsDownCounterToZero() == TRUE) {
		 Screen_DownCounterStop(); 
		 PageNo = Hmi_GetCurrPicNo();
		 if(PageNo == sGlocalPara.DownCounter.picNo)
		 {
			 switch(sGlocalPara.DownCounter.picNo) {
				 case E_PICTURE17:   /*充电方式选择界面*/
					 Delay10Ms(10);	
				   Screen_SetChgFlag(0);
					 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);  
			  	 break;
				 case E_PICTURE18:   /*电量输入界面*/
					 Delay10Ms(1); 
				   Screen_SetChgFlag(0);	
				   Hmi_ShutDownKeyBoard(NUMBER_KEY_BOARD_CODE);
					 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);				   
					 break;
				 case E_PICTURE19:    /*金额输入界面*/
					 Delay10Ms(1);
				   Screen_SetChgFlag(0);	
				   Hmi_ShutDownKeyBoard(NUMBER_KEY_BOARD_CODE);
					 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);				   
					 break;
				 case E_PICTURE20:     /*时间输入界面*/
					 Delay10Ms(1);
				   Screen_SetChgFlag(0);	
 				   Hmi_ShutDownKeyBoard(NUMBER_KEY_BOARD_CODE);
					 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);			  
					 break;
				 case E_PICTURE33:    /*用户密码输入界面*/
					 Hmi_ShutDownKeyBoard(TEXT_KEY_BOARD_CODE);
					 Page_KeyPwReturn();
					 break;
				 case E_PICTURE34:  /*刷卡密码输入界面*/
					 Hmi_ShutDownKeyBoard(TEXT_KEY_BOARD_CODE);
					 Page_KeyPwCardReturn();
					 break;      
				 case E_PICTURE45:
					 Screen_SetChgFlag(0);
					 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
					 break; 
				 case E_PICTURE51:     //add0529zyf
				 case E_PICTURE52:
					 Screen_SetChgFlag(0);	
				   Hmi_ShutDownKeyBoard(TEXT_KEY_BOARD_CODE);
					 Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSWSHOW,9);
			     Hmi_ClearReg(DGUS_STARTMODE_CHARGACCOUNT,9);
			     Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSW,9);
					 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
				   break;
				 default:
					 break;			 
		  }
	   }
		 Delay10Ms(1); 
   }
}


/***********************************************************************
***函数名称: Screen_MessageDeal
** 功能描述: 触摸屏任务消息处理
***    参数: 
**   返回值:   
*************************************************************************/

const char * CheckString[] = \
{
  "准备开始...",
	"正在开启辅助电源...",
	"检测电子锁状态...",
	"等待BHM报文...",
	"进入绝缘检测...",
	"进行老国标探测...",
	"电源模块开始输出...",
	"等待绝缘检测完成...",
	"等待泄放结束...",
	"等待接收BRM报文...",
	"等待接收BCP报文...",
	"等待接收BRO报文...",
	"检测车辆参数是否合适...",
	"电源模块输出调整...",
  "即将开始充电...",
	"VIN后台检测...",
};

const SCREN_PAGE_SEL conPageSel[] = {\
	{E_PICTURE1,IDEL,IDEL},
  {E_PICTURE2,IDEL,CHARGING},
  {E_PICTURE3,IDEL,E_ERROR},
  {E_PICTURE4,IDEL,ACCOUNT},
	{E_PICTURE4,IDEL,FINISH},
  {E_PICTURE5,CHARGING,IDEL},
  {E_PICTURE6,CHARGING,CHARGING},
  {E_PICTURE7,CHARGING,E_ERROR},
  {E_PICTURE8,CHARGING,ACCOUNT},
	{E_PICTURE8,CHARGING,FINISH},
  {E_PICTURE9,E_ERROR,IDEL},
  {E_PICTURE10,E_ERROR,CHARGING},
  {E_PICTURE11,E_ERROR,E_ERROR},
  {E_PICTURE12,E_ERROR,ACCOUNT},
	{E_PICTURE12,E_ERROR,FINISH},
  {E_PICTURE13,ACCOUNT,IDEL},
  {E_PICTURE13,FINISH,IDEL},
  {E_PICTURE14,ACCOUNT,CHARGING},
	{E_PICTURE14,FINISH,CHARGING},
  {E_PICTURE15,ACCOUNT,E_ERROR},
	{E_PICTURE15,FINISH,E_ERROR},
  {E_PICTURE16,ACCOUNT,ACCOUNT},	
  {E_PICTURE16,FINISH,FINISH},
	{E_PICTUREANY,0,0},
	
};


void Screen_SetChgFlag(uint8 fg)
{
   gScrenCtrlPara.u8ChgFlag = fg;
}


/*获取当前的页面号，0xff为错误*/
uint8 Screen_GetTurnPicNo(SCREN_CTRLPARA *ptrCtrlPara)
{
	PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	uint8 i;
  uint8 rtn = E_PICTUREANY;
	for( i = 0 ; conPageSel[i].u8PicNo != E_PICTUREANY ;i++ ) {
		if(ptrCtrlPara->u8GunAStu == conPageSel[i].u8GunAstu1 \
			&& ptrCtrlPara->u8GunBStu == conPageSel[i].u8GunBstu2 ) {
				ptrCtrlPara->CurrPic = conPageSel[i].u8PicNo;
		    rtn = ptrCtrlPara->CurrPic;
	  }
	}
	if(E_PICTUREANY == rtn ) {
    ptrCtrlPara->u8keepPic = Hmi_GetCurrPicNo();
    return ptrCtrlPara->u8keepPic;
	}
	if( 1 == ptrCtrlPara->u8ChgFlag) {
		 rtn = E_PICTUREANY;
	}else {
		ptrCtrlPara->u8keepPic = rtn;
	}
	
	if(GUNNUM_1 == devparaPtr->gunnum){
		switch(rtn){
			case E_PICTURE1:
			case E_PICTURE2:
			case E_PICTURE3:
			case E_PICTURE4:
				rtn = E_PICTURE47;
			  ptrCtrlPara->u8keepPic = rtn;
				break;
			case E_PICTURE5:
			case E_PICTURE6:
			case E_PICTURE7:
			case E_PICTURE8:
				rtn = E_PICTURE48;
			  ptrCtrlPara->u8keepPic = rtn;
				break;
			case E_PICTURE9:
			case E_PICTURE10:
			case E_PICTURE11:
			case E_PICTURE12:
				rtn = E_PICTURE49;
			  ptrCtrlPara->u8keepPic = rtn;
				break;
			case E_PICTURE13:
			case E_PICTURE14:
			case E_PICTURE15:
			case E_PICTURE16:
				rtn = E_PICTURE50;
			  ptrCtrlPara->u8keepPic = rtn;
				break;
			default:
				break;
		}
	}
	return rtn;
}

const uint16  MsgWkstatuAdr[DEF_MAX_GUN_NO][6] = {\
	{DGUS_IDLE_PROADR_A,DGUS_CHAREADY_PROADR,DGUS_ACCOUNT_PROADR_A,DGUS_FAULT_TEXT_ADR_A1\
	,DGUS_FAULT_TEXT_ADR_A2,DGUS_CHARG_PROADR_A},
	{DGUS_IDLE_PROADR_B,DGUS_CHAREADY_PROADR,DGUS_ACCOUNT_PROADR_B,DGUS_FAULT_TEXT_ADR_B1\
	,DGUS_FAULT_TEXT_ADR_B2,DGUS_CHARG_PROADR_B},
	
};

extern DEV_RELAY_TYPE * Relay_GetRelayDataPtr(uint8 no);
void IDEL_ShowMessage(uint8 gunNo)
{	
	char temp[64];
	DEV_RELAY_TYPE *pRelayDataPtr = Relay_GetRelayDataPtr(gunNo);
	
	Screen_ShowMessage((char *)"  ",MsgWkstatuAdr[gunNo][0]);
	strcpy(temp,"欢迎使用 ");
	if(gunNo == AGUN_NO){
		if(BMS_24V == pRelayDataPtr->statu.bits.apow){
			strcat(temp,"当前辅源为24V");
		}
		else{
			strcat(temp,"当前辅源为12V");
		}
	}
	else{
		if(BMS_24V == pRelayDataPtr->statu.bits.bpow){
			strcat(temp,"当前辅源为24V");
		}
		else{
			strcat(temp,"当前辅源为12V");
		}
	}
  Screen_ShowMessage(temp,MsgWkstatuAdr[gunNo][0]);	
}
void IDEL_ShowGunState(uint8 gunNo)
{	
	char temp[64],rtn;
	static uint8 scnt[DEF_MAX_GUN_NO]={0};
	CHARGE_TYPE  *BgRunData = ChgData_GetRunDataPtr(gunNo%DEF_MAX_GUN_NO);
	DEV_RELAY_TYPE *pRelayDataPtr = Relay_GetRelayDataPtr(gunNo);
	
	
	strcpy(temp,"欢迎使用 ");
	rtn = 0;
	if(gunNo == AGUN_NO){
		if((scnt[AGUN_NO]== 0) && (CC1_4V == BgRunData->gun->statu.bits.cc1stu) ){
			scnt[AGUN_NO] = 1;
			if(BMS_24V == pRelayDataPtr->statu.bits.apow){
				strcat(temp,"当前辅源为24V");
			}
			else{
				strcat(temp,"当前辅源为12V");
			}
		}
		else if(CC1_4V != BgRunData->gun->statu.bits.cc1stu){
			scnt[AGUN_NO] = 0;
			strcat(temp,"请插枪！！！");
		}
		else{
			rtn = 1;
		}
	}
	else{
		if((scnt[BGUN_NO]== 0) && (CC1_4V == BgRunData->gun->statu.bits.cc1stu) ){
			scnt[BGUN_NO] = 1;
			if(BMS_24V == pRelayDataPtr->statu.bits.bpow){
				strcat(temp,"当前辅源为24V");
			}
			else{
				strcat(temp,"当前辅源为12V");
			}
		}
		else if(CC1_4V != BgRunData->gun->statu.bits.cc1stu){
			scnt[BGUN_NO] = 0;
			strcat(temp,"请插枪！！！");
		}
		else{
			rtn = 1;
		}
	}
  if(0 == rtn){
		Screen_ShowMessage((char *)"  ",MsgWkstatuAdr[gunNo][0]);
		Screen_ShowMessage(temp,MsgWkstatuAdr[gunNo][0]);
	}		
}

#ifdef SCREEN_CARD
extern void CardReader_MsgProc(MSG_STRUCT *msg);
#endif

extern DEV_ISO_TYPE * TskIso_GetDataPtr(uint8 gunNo);
void Screen_MessageDeal(void)
{
	 const STR_ERR_ST *errstr;
	 MSG_STRUCT msg;
	 uint8 tmp = 0,gunNo = 0,TempPicNo = 0,power = 0;
 	 char *test;
   char temp[64];
	 uint16 TempAddr = 0x0000;
	 char checkMessage[100] = { 0 };
	 char paramMessage[64] = { 0 };
		
	 int16 Vdc1 = 0;
	 int16 Vdc3 = 0;
	 int16 VDCBHM = 0;
	 int16 VDCBCP = 0;
	 uint16 respos;    /*正对地电阻*/
	 uint16 resneg;    /*负对地电阻*/
	 
	 PARAM_DEV_TYPE *chgparaPtr = ChgData_GetDevParamPtr();
   CARD_INFO *ptrCardinfo = TskCard_GetCardInfPtr();
	 DEV_RELAY_TYPE *pRelayDataPtr = Relay_GetRelayDataPtr(0);	
   if( RcvMsgFromQueue(&msg) == TRUE ) {

		 switch(msg.MsgType)
		 {
			 case MSG_RESQ_PASSWD:  /*请求输入密码*/
			   Screen_ShowMessage((char *)"  ",DGUS_PW_CARD_ADR);
				 Hmi_BuzzerBeep(10);
				 Screen_ShowMessage((char *)"  ",DGUS_PW_CARDSH_ADR);/*清除密码显示*/
			 	 Hmi_ChangePicture(E_PICTURE34);
				 Screen_StartDownCounter(E_PICTURE34,DEFAULT_COUNTDOWN_VALUE);			
				 break;
			 case MSG_VERIFY_RESULT:   /*鉴权失败*/		
         gunNo = msg.MsgData[0];
         Screen_ShowMessage((char *)"  ",MsgWkstatuAdr[gunNo][0]);			 
				 switch(msg.MsgData[1])
				 {
					 case CARD_PASSWD_ERR:
						 test = "读卡错误,请重试！";						
						 break;
					 case CARD_ILLEGAL_ERR:
						 test = "不可识别卡";												 
						 break;
					 case CARD_NOMONEY_ERR:
						 test = "卡余额不足，请充值";						 					 
						 break;
					 case CARD_NOTIDLE_ERR:
						 test = "非空闲状态，请稍后再试";						
						 break;
					 case CARD_VIN_NO_MATCH:
						 test = "VIN码不匹配";						
						 break;					 
					 default:
						 break;
				 }
				 sGlocalPara.startkeylasttm[gunNo] = GetSystemTick() - 31; 				 
				 
         Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][0]);				 
         Delay10Ms(200);         				 
				 Screen_ShowMessage((char *)"   ",TempAddr);	
				 break;
			 case MSG_WORK_STATU:  /*充电桩状态*/
				 gunNo = msg.MsgData[0]%DEF_MAX_GUN_NO;	
				 if(AGUN_NO == gunNo)
				 {	
					 power = pRelayDataPtr->statu.bits.apow;					 
					  gScrenCtrlPara.u8GunAStu = msg.MsgData[1];							 
					  TempPicNo = Screen_GetTurnPicNo(&gScrenCtrlPara);	
            if((E_PICTUREANY == TempPicNo)||(E_PICTURE1 == gScrenCtrlPara.u8ChgFlag)||(E_PICTURE47 == gScrenCtrlPara.u8ChgFlag))						
						{							
							break;						
						}		
					}
				 else if(BGUN_NO == gunNo)
				 {
					  power = pRelayDataPtr->statu.bits.bpow;		
						gScrenCtrlPara.u8GunBStu = msg.MsgData[1];
						TempPicNo = Screen_GetTurnPicNo(&gScrenCtrlPara);
						if((E_PICTUREANY == TempPicNo)||(E_PICTURE1 == gScrenCtrlPara.u8ChgFlag))
						{							
							break;
						}	
				 }	
			 
				 switch(msg.MsgData[1])
				 {							 
						 case IDEL:    /*空闲*/ 
							 if( 0 == sGlocalPara.setparafg ) {										 
								 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
								 Screen_ShowMessage((char *)"  ",MsgWkstatuAdr[gunNo][0]);
								 if((GetSystemTick() - sGlocalPara.startkeylasttm[gunNo]) < 30*TIM_1S )      //add0604zyf
								 {
										START_PARAM  *PtrStartPara = ChgData_GetStartParaPtr(gunNo);
										if(PtrStartPara->startby == START_BY_PASSWD)
										{
											strcpy(temp,"账号密码验证中...");
										}
										else{
											strcpy(temp,"欢迎使用 ");
										}
  							 }
								 else
								 {		
								   strcpy(temp,"欢迎使用 ");
								 }

									//test = "欢迎使用 ";
								 if(0 == power)
								 {
									  strcat(temp,"当前辅源为12V");
								 }
								 else if(1 == power)
								 {
									  strcat(temp,"当前辅源为24V");
								 }
									Screen_ShowMessage(temp,MsgWkstatuAdr[gunNo][0]);	
							 }							 
							// IDEL_ShowMessage(gunNo);	
							 break;
//						 case CHECK:   /*自检*/
//							 tmp = msg.MsgData[2];
//							 sGlocalPara.setparafg = 0;
//							 Screen_ShowMessage((char *)"   ",MsgWkstatuAdr[gunNo][1]);	
//							 Screen_ShowMessage((char *)CheckString[tmp],MsgWkstatuAdr[gunNo][1]);					  
//							 Hmi_ChangePicture(E_PICTURE41);						
//							 break;
						case CHECK:   /*自检*/
							 tmp = msg.MsgData[2];
							 sGlocalPara.setparafg = 0;
							 						  
							 Vdc1   = ((CHARGE_TYPE *)ChgData_GetRunDataPtr(gunNo))->meter->volt;
							 Vdc3   = ((CHARGE_TYPE *)ChgData_GetRunDataPtr(gunNo))->iso->vdc3;
							 VDCBHM = ((BMSDATA_ST *)Bms_GetBmsCarDataPtr(gunNo))->bhm.maxvolte;
							 VDCBCP = ((BMSDATA_ST *)Bms_GetBmsCarDataPtr(gunNo))->bcp.batcurvolt;
							 
							 respos = ((DEV_ISO_TYPE *)TskIso_GetDataPtr(gunNo))->res1pos;
							 resneg = ((DEV_ISO_TYPE *)TskIso_GetDataPtr(gunNo))->res1neg;
						
						   strcat(checkMessage, (char *)CheckString[tmp]);
						   
						   switch (tmp)
							 {
								 case E6_DCOUT:  //电源模块开始输出...
									 sprintf(paramMessage, "参数:ISOV=%d.%dV,METV=%d.%dV", Vdc3/10, Vdc3%10, Vdc1/10, Vdc1%10); 
									 break;
						     case E7_WTISOF: //等待绝缘检测完成...
									 sprintf(paramMessage, "参数:METV=%d.%dV,BHMV=%d.%dV", Vdc1/10, Vdc1%10, VDCBHM/10, VDCBHM%10);
									 break;
								 case E8_WTXFF:  //等待泄放结束...
									 sprintf(paramMessage, "参数:RESP=%dKΩ,RESN=%dKΩ", respos, resneg);
									 break;
								 case E12_CHKPARA: //等待接收BRO报文...
									 sprintf(paramMessage, "参数:ISOV=%d.%dV,BCPV=%d.%dV", Vdc3/10, Vdc3%10, VDCBCP/10, VDCBCP%10);
									 break;
								 case E13_DCOUTADJ: //电源模块输出调整...
									 sprintf(paramMessage, "参数:ISOV=%d.%dV,METV=%d.%dV,BCPV=%d.%d", Vdc3/10, Vdc3%10, Vdc1/10, Vdc1%10, VDCBCP/10, VDCBCP%10);
									 break;
								 default:
									 sprintf(paramMessage, "参数:ISOV=%d.%dV,METV=%d.%dV", Vdc3/10, Vdc3%10, Vdc1/10, Vdc1%10);
									 break;
							 }
							 
							 strcat(checkMessage, paramMessage);
							 Screen_ShowMessage((char *)"   ",MsgWkstatuAdr[gunNo][1]);
						   Screen_ShowMessage(checkMessage,MsgWkstatuAdr[gunNo][1]);
							 Hmi_ChangePicture(E_PICTURE41);						
							 break;

						 case FINISH:    /*充电结束*/															 
							 Screen_ShowMessage((char *)"  ",MsgWkstatuAdr[gunNo][2]);									 
													 
							 if(0 == msg.MsgData[4])
							 { 
								  strcpy(temp,"未结算,结算后才能拔出充电枪！");
							 }
							 else if(1 == msg.MsgData[4])
							 {  
								  tmp = msg.MsgData[2] % REASOM_MUN;						   
							    strcpy((char *)temp,conReasonString[tmp]);	
									strcat(temp,",已结算,请拔枪归位");
							 }	
							 else if(2 == msg.MsgData[4])
							 {  
								  strcpy(temp,"未结算,枪连接中.....！");
							 }
							 Screen_ShowMessage(temp,MsgWkstatuAdr[gunNo][2]) ;			
							 Screen_OpenAccountCardDeal(gunNo);									 
							 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);						 				   
							 break;
						 case ACCOUNT:   /*结算*/		
							 Screen_OpenAccountCardDeal(gunNo);
							 if(1 == msg.MsgData[4])
							 {
									strcpy((char *)temp,"充电结束,感谢您的使用！");								
							 }
							 else if(0 == msg.MsgData[4])
							 {												
									strcpy((char *)temp,"未结算,请刷卡结算");         								 
							 }
//               else if(2 == msg.MsgData[4])
//							 {												
//									strcpy((char *)temp,"未结算,等待后台账单");         								 
//							 }							 
							Screen_ShowMessage(temp,MsgWkstatuAdr[gunNo][2]) ;	
							Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
							break;
						 case E_ERROR:   /*故障*/																 
							 sGlocalPara.setparafg = 0;
							 errstr = Screen_GetErrSting(msg.MsgData[3]);
							 Screen_ShowMessage(errstr->str,MsgWkstatuAdr[gunNo][3]);
							 strcpy(temp,"请拨打服务电话：");
							 strcat(temp,(char *)chgparaPtr->telephone);		
							 Screen_ShowMessage(temp,MsgWkstatuAdr[gunNo][4]);						 
							 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
							 break;
						 case CHARGING:  /*正在充电*/
							 ChargeShow(gunNo);
							 sGlocalPara.setparafg = 0;
							 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);		
							 break;


						 default:
							 break;
					 
				 }		 
				 break;
			 case MSG_UPPLUG_GUN:    /*等待拔枪  充之鸟*/
				  tmp = 1;	
			    gunNo = msg.MsgData[0]%DEF_MAX_GUN_NO;	
					if(AGUN_NO == gunNo)
					{		  
						Hmi_WriteOneVar(DGUS_STOPCHARG_WAITOVER_A,tmp);
					}
					else
					{
						Hmi_WriteOneVar(DGUS_STOPCHARG_WAITOVER_B,tmp);
					}
					Screen_ShowMessage((char *)"  ",MsgWkstatuAdr[gunNo][2]);	
					strcpy(temp,"未结算,枪连接中.....！");
					Screen_ShowMessage(temp,MsgWkstatuAdr[gunNo][2]) ;	
				 break;
			 case MSG_WAIT_RECORD:    /*等待结算   充之鸟*/
				 tmp = 3;	
			   gunNo = msg.MsgData[0]%DEF_MAX_GUN_NO;	
				 if(AGUN_NO == gunNo)
					{
						Hmi_WriteOneVar(DGUS_STOPCHARG_WAITOVER_A,tmp);
					}
					else
					{
						Hmi_WriteOneVar(DGUS_STOPCHARG_WAITOVER_B,tmp);
					}
					Screen_ShowMessage((char *)"  ",MsgWkstatuAdr[gunNo][2]);	
					if(1 == msg.MsgData[0]){
						strcpy(temp,"未结算,结算等待超时！");
					}
					else{
					  strcpy(temp,"未结算,等待后台账单.....！");
					}
					Screen_ShowMessage(temp,MsgWkstatuAdr[gunNo][2]) ;	
				 break;
			 case MSG_LOCKED_CARD:    /*灰锁卡*/
//				 Screen_ShowMessage((char *)"  ",MsgWkstatuAdr[gunNo][0]);
//			 		test = "已检测到卡，验证中...";
//				Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][0]);
//					 Delay10Ms(100); 
//				  Screen_ShowMessage(" ",MsgWkstatuAdr[gunNo][0]);			 
				 break;
			 case MSG_LOCK_RESULT:  /*锁卡结果*/
//				 gunNo = msg.MsgData[0];
				 Screen_ShowMessage((char *)"  ",MsgWkstatuAdr[gunNo][0]);
			 
//------------------add0705zyf  patli 20190929
			   if( chgparaPtr->onlinecard == E_ONLINE_CARD ) 
				 {	 
					  if(ptrCardinfo->cardtype == E_SUPER_CARD)
						{
							test = "检测到超级会员卡，正启动充电...";
							Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][0]);
						}else if(ptrCardinfo->cardtype == E_OFFLINE_USER_CARD)
						{
							 test = "检测到离线普通卡，正启动充电...";
							 Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][0]);
						
						}else{
							test = "检测到在线用户卡，验证中...";
							Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][0]);
						}
						Hmi_BuzzerBeep(20);
				 }
				 else
				 {	 
//------------------			 
					 if(RES_FAIL == msg.MsgData[1])
					 {
							if( 0 == msg.MsgData[2] ) {
								test = "锁卡失败";
								Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][0]);
								Delay10Ms(200);
							}else if(1 == msg.MsgData[2]) {
								test = "未检测到充电枪，请重新插枪！";
								Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][0]);
								Delay10Ms(200);
							}
					 }
					 else if(RES_SUCC == msg.MsgData[1])
					 {							 
						 char temp1[10];
						 test = "锁卡成功，请选择充电模式";
						 Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][0]);
						 Hmi_BuzzerBeep(20);
						 Delay10Ms(200);
						 strcpy((char *)temp,"当前卡余额：");		
						 Common_IntToFloatStr((uint8 *)temp1,ptrCardinfo->money,2);					 
						 strcat(temp,temp1);
						 strcat(temp," 元");
						 Screen_ShowMessage(temp,DGUS_CHARWAY_PROADR);	
						 Hmi_ChangePicture(E_PICTURE17);
						 Screen_SetChgFlag(1);						 
						 Screen_StartDownCounter(E_PICTURE17,DEFAULT_COUNTDOWN_VALUE);    			 
					 }	
				 }				 
				 Delay10Ms(100); 
	//			 Screen_ShowMessage(" ",MsgWkstatuAdr[gunNo][0]);				
				 break;
			 case MSG_UNLOCK_RESULT:  /*解锁结果*/
				 gunNo = msg.MsgData[0];
				 switch(msg.MsgData[1])
				 {
					 case 0:
						 test = "读卡失败，请继续刷卡";						
						 break;
					 case 1:
						 test = "刷卡成功，正在停止...";            
						 break;
					 case 2:
						 test = "非启动卡，请换卡再试！";					   
						 break;
					 default:
							break;
				  }	
				  Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][2]);
					Delay10Ms(10); 
				  Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][5]);
				  Delay10Ms(300); 					
				  break;
			 case MSG_SYSTEM_SETTING:  /*进入系统设置*/
			 case MSG_MANAGE_CARD: 
	       sGlocalPara.Sysflag = E_SAFE1; 
			   sGlocalPara.setparafg = 1;	 
			   Screen_SetChgFlag(1);		
  			 Hmi_ChangePicture(E_PICTURE23);
         Hmi_BuzzerBeep(10);			 
				 break;
			 default:
				 break;
		  		 
		 }

#ifdef SCREEN_CARD
		 CardReader_MsgProc(&msg);
#endif
	
	 }
}

extern char *ChargeCP_GetCPTwoDim(void);
/***********************************************************************
***函数名称: TwoDimBarCode
** 功能描述: 二维码生成
***    参数: 
**   返回值:   
*************************************************************************/
#define GUN_NO_STR "0"
#define LT_TWODIMBARCODR_HEAD  "http://wx.aunice.com.cn/wx/#/charging/"
#define LT_TWODIMBARCODR_CHGZPLD_HEAD  "http://www.cityfastfilling.com/dskcserviceapp/charge/"
#define BIRD_TWODIM  "{\"gunNumber\":2,\"snNumber\":\"1801000360292770\",\"type\":\"1\"}"
#define BIRD_TWODIM1  "{\"gunNumber\":"
#define BIRD_TWODIM2  ",\"snNumber\":\""
#define BIRD_TWODIM3 "\",\"type\":\"1\"}"
void TwoDimBarCode(void)
{
	char text[128 + 4] = { 0 };
	char ID_A[18] = { 0 };
  char ID_B[18] = { 0 };
	
	PARAM_DEV_TYPE *chgparaPtr = ChgData_GetDevParamPtr();
	PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	
	 /*充电桩A枪编号显示*/
	Hmi_ClearReg(DGUS_NUMBER_ADR_A,1);
  memcpy(ID_A,chgparaPtr->chargeId,sizeof(chgparaPtr->chargeId));
  strcat(ID_A,"0");
	ID_A[sizeof(chgparaPtr->chargeId)+1] = 0;
  Screen_ShowMessage(ID_A,DGUS_NUMBER_ADR_A);
	
	/*充电桩B枪编号显示*/
	if(GUNNUM_2 == devparaPtr->gunnum){
		Hmi_ClearReg(DGUS_NUMBER_ADR_B,1);
		memcpy(ID_B,chgparaPtr->chargeId,sizeof(chgparaPtr->chargeId));
		strcat(ID_B,"1");
		ID_B[sizeof(chgparaPtr->chargeId)+1] = 0;
		Screen_ShowMessage(ID_B,DGUS_NUMBER_ADR_B);
	}

	if (CONN_Aunice == BackCOMM->agreetype){
		memset(text,0,sizeof(text));
		strcpy(text,LT_TWODIMBARCODR_HEAD);
		memset(ID_A,0,sizeof(ID_A));
		memcpy(ID_A,chgparaPtr->chargeId,sizeof(chgparaPtr->chargeId));
		strcat(text,ID_A);
		strcat(text,"/");
		strcat(text,"0");
		Hmi_ShowText(DGUS_TWODIMBARCODE_ADDR_A,strlen(text),(uint8 *)text);
		
		memset(text,0,sizeof(text));
		strcpy(text,LT_TWODIMBARCODR_HEAD);
		memset(ID_B,0,sizeof(ID_B));
		memcpy(ID_B,chgparaPtr->chargeId,sizeof(chgparaPtr->chargeId));
		strcat(text,ID_B);
		strcat(text,"/");
		strcat(text,"1");
		Hmi_ShowText(DGUS_TWODIMBARCODE_ADDR_B,strlen(text),(uint8 *)text);
	}
	
	else if (CONN_CheDian == BackCOMM->agreetype){  //奥耐云充，车电网协议
		memset(ID_A,0,sizeof(ID_A));
		memset(text,0,sizeof(text));
		memcpy(ID_A,chgparaPtr->chargeId,sizeof(chgparaPtr->chargeId));
		strcpy(text,ID_A);
		Hmi_ShowText(DGUS_TWODIMBARCODE_ADDR_A,strlen(text),(uint8 *)text);	
		if(GUNNUM_2 == devparaPtr->gunnum){
			memset(ID_B,0,sizeof(ID_B));
			memset(text,0,sizeof(text));
			memcpy(ID_B,chgparaPtr->chargeId,sizeof(chgparaPtr->chargeId));
			strcpy(text,ID_B);
			Hmi_ShowText(DGUS_TWODIMBARCODE_ADDR_B,strlen(text),(uint8 *)text);
		}
  }
  
	else if(CONN_CHGBIRD == BackCOMM->agreetype){
	  memset(text,0,sizeof(text));
		strcpy( text,BIRD_TWODIM1);
		strcat(text,"1");
		strcat( text,BIRD_TWODIM2);
		strcat(text,ID_A);
		strcat( text,BIRD_TWODIM3);
	  Hmi_ShowText(DGUS_TWODIMBARCODE_ADDR_A,strlen(text),(uint8 *)text);
		if(GUNNUM_2 == devparaPtr->gunnum){
			memset(text,0,sizeof(text));
			strcpy( text,BIRD_TWODIM1);
		  strcat(text,"2");
		  strcat( text,BIRD_TWODIM2);
		  strcat(text,ID_B);
		  strcat( text,BIRD_TWODIM3);
	    Hmi_ShowText(DGUS_TWODIMBARCODE_ADDR_B,strlen(text),(uint8 *)text);
	  }
	}
	
	else if (CONN_CHGZPLD == BackCOMM->agreetype){
		memset(text, 0, sizeof(text));
		strcpy(text, LT_TWODIMBARCODR_CHGZPLD_HEAD);
		memset(ID_A, 0, sizeof(ID_A));
		memcpy(ID_A,chgparaPtr->chargeId,sizeof(chgparaPtr->chargeId));
		strcat(text,ID_A);
		strcat(text,"|");
		strcat(text,"1");
		Hmi_ShowText(DGUS_TWODIMBARCODE_ADDR_A,strlen(text),(uint8 *)text);
		
		if(GUNNUM_2 == devparaPtr->gunnum){
			memset(text, 0, sizeof(text));
			strcpy(text, LT_TWODIMBARCODR_CHGZPLD_HEAD);
			memset(ID_B, 0, sizeof(ID_B));
			memcpy(ID_B,chgparaPtr->chargeId,sizeof(chgparaPtr->chargeId));
			strcat(text,ID_B);
			strcat(text,"|");
			strcat(text,"2");
			Hmi_ShowText(DGUS_TWODIMBARCODE_ADDR_B,strlen(text),(uint8 *)text);
		}
	}

	else if (CONN_CHCP == BackCOMM->agreetype){

	}

	uint16 posA[2] = {0};

	if(GUNNUM_1 == devparaPtr->gunnum)
	{
		 posA[0] = 510;
		 posA[1] = 144;
		 Hmi_WriteMulitVar(0x4D01,posA,2);
	}
	else if(GUNNUM_2 == devparaPtr->gunnum)
	{
		 posA[0] = 110;
		 posA[1] = 120;
		 Hmi_WriteMulitVar(0x4D01,posA,2);
	}	 

	uint16 posB[2] = {513,120};
	Hmi_WriteMulitVar(0x4F01,posB,2);
}

/*三相电源界面显示*/
void  VoltShow(uint8 gunNo)
{
  	uint8 text[64] = {0};
		uint8 text1[8] = {0};
	  DEV_ACMETER_TYPE *AcmeterPtr = TskAc_GetMeterDataPtr(gunNo);
		PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();

//		DEV_ACMETER_TYPE *AcmeterPtr_B = TskAc_GetMeterDataPtr(BGUN_NO);
		if(AGUN_NO == gunNo){
			strcpy((char *)text,"Uab:");
			Common_IntToFloatStr(text1,AcmeterPtr->Uab ,1);
			strcat((char *)text,(char *)text1);
			Hmi_ShowText(DGUS_VOLTUAB_ADR_A,strlen((char *)text),text);
			memset(text,0,sizeof(text));
			
			strcpy((char *)text,"Ubc:");
			Common_IntToFloatStr(text1,AcmeterPtr->Ubc ,1);
			strcat((char *)text,(char *)text1);
			Hmi_ShowText(DGUS_VOLTUBC_ADR_A,strlen((char *)text),text);
			memset(text,0,sizeof(text));
			
			strcpy((char *)text,"Uca:");
			Common_IntToFloatStr(text1,AcmeterPtr->Uca ,1);
			strcat((char *)text,(char *)text1);
			Hmi_ShowText(DGUS_VOLTUCA_ADR_A,strlen((char *)text),text);
			memset(text,0,sizeof(text));
		}
		else if(BGUN_NO == gunNo && GUNNUM_2 == devparaPtr->gunnum){
			strcpy((char *)text,"Uab:");
			Common_IntToFloatStr(text1,AcmeterPtr->Uab ,1);
			strcat((char *)text,(char *)text1);
			Hmi_ShowText(DGUS_VOLTUAB_ADR_B,strlen((char *)text),text);
			memset(text,0,sizeof(text));
			
			strcpy((char *)text,"Ubc:");
			Common_IntToFloatStr(text1,AcmeterPtr->Ubc ,1);
			strcat((char *)text,(char *)text1);
			Hmi_ShowText(DGUS_VOLTUBC_ADR_B,strlen((char *)text),text);
			memset(text,0,sizeof(text));
			
			strcpy((char *)text,"Uca:");
			Common_IntToFloatStr(text1,AcmeterPtr->Uca ,1);
			strcat((char *)text,(char *)text1);
			Hmi_ShowText(DGUS_VOLTUCA_ADR_B,strlen((char *)text),text);
			memset(text,0,sizeof(text));
		}
		
}


/*当前费率显示函数*/
void currfeeshow()
{
	  uint8 text[64] = {0};
		uint8 text1[16] = {0};

		REAL_BILL_DATA_ST *FeedDataPrt = GetFeeData(AGUN_NO);
    PARAM_DEV_TYPE *ptrDevPara = ChgData_GetDevParamPtr();
		PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();    //patli 20190923 add
		
		strcpy((char *)text,"当前电价:");
		Common_IntToFloatStr(text1,FeedDataPrt->real.currfeerate ,4);
		strcat((char *)text,(char *)text1);
	  strcat((char *)text,"元/Kwh ");  //patli 20190923 add 1 byte space
		
		if (CONN_CHCP != BackCOMM->agreetype){	  //patli 20190923 add
			strcat((char *)text,"服务费:");
			Common_IntToFloatStr(text1,FeedDataPrt->real.currSrvrate,4);
			strcat((char *)text,(char *)text1);
			strcat((char *)text,"元/Kwh "); 
		}
		
    if( ptrDevPara->onlinecard == E_ONLINE_CARD ) {		
       strcat((char *)text,"在线卡模式"); 
		}
		else 
		{
		  strcat((char *)text,"离线卡模式"); 
		}
		Hmi_ShowText(DGUS_CURRATR_ADR,strlen((char *)text),text);
    memset(text,0,sizeof(text));
    
  /*软件版本号显示*/
	 //sprintf((char *)text,"%s.%s.%s",M_VERSION1,M_VERSION2,M_VERSION3);
#ifdef AUNICE_DEBUG
		sprintf((char *)text,"%s.%c%d.%.2d", M_VERSION1,VERSION_LETTER, MAIN_VERSION,SUB_VERSION);    //按邓总的规则改的版本号
#else	  
   sprintf((char *)text,"%s.%s",M_VERSION1,M_VERSION3);    //按邓总的规则改的版本号
#endif	 

	 Hmi_ShowText(DGUS_DEBUGTEXT_ADR+DGUS_DEBUGTEXT_OFFSET*3,strlen((char *)text),text);

		VoltShow(AGUN_NO);  //电源显示
		VoltShow(BGUN_NO);  //电源显示
		
}

const uint16 ChargeAdr[2][4] = {\
	{DGUS_CHARGE_ADDR_A,DGUS_CHARGID_ADR_A,DGUS_CHARGQAY_ADR_A,DGUS_CHARG_PROADR_A},
	{DGUS_CHARGE_ADDR_B,DGUS_CHARGID_ADR_B,DGUS_CHARGQAY_ADR_B,DGUS_CHARG_PROADR_B},
	
};

/*充电界面显示*/
void ChargeShow(uint8 gunNo)
{
	int32 i32tmp = 0;
   uint8 cnt = 0;
	 uint16 tmp16 = 0;
	 uint32 tmp32;
	 uint16 data[64] = {0};
   uint8 buf[100] = {0};
	 uint8 text1[100] = {0};
	 
	 uint8 rtn,tmp;
	 
	  CHARGE_TYPE  *RunDataPrt = ChgData_GetRunDataPtr(gunNo);
    REAL_BILL_DATA_ST *FeeDataPtr = GetFeeData(gunNo);
    BMSDATA_ST *BmsDataCarPtr = Bms_GetBmsCarDataPtr(gunNo); 
	  PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();

		tmp32 = FeeDataPtr->bill.billmoney;   //消费金额
    Common_Change4Byte(&tmp32);
    data[cnt++] = tmp32 & 0xffff;		 
    data[cnt++] = tmp32 >> 16;
		
		tmp16 = FeeDataPtr->bill.chgsecs/60;   //充电时间
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;

	 	tmp16 = FeeDataPtr->bill.energy;        //已充电量
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
	 
	 	tmp16 = RunDataPrt->meter->volt;        //充电电压
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
		tmp16 = RunDataPrt->meter->current;        //充电电流
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
	 
	 	tmp16 = FeeDataPtr->real.lasttimes;          //剩余时间
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
	 	 
    tmp16 = FeeDataPtr->bill.startsoc;        //开始SOC
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
	 
		tmp16 = FeeDataPtr->bill.soc;             //当前SOC
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
	  tmp16 = Hmi_PackWriteReg(buf,(uint8*)&data,ChargeAdr[gunNo][0],CMD_WR_VAR,2*cnt);
	  Screen_UartWrite(buf,tmp16);

/************************充之鸟充电界面参数*****************************************/
    if(CONN_CHGBIRD == BackCOMM->agreetype){  //CONN_CHGBIRD
				cnt = 0;
				CHARGE_TYPE  *RunDataPrt_A = ChgData_GetRunDataPtr(AGUN_NO);
				CHARGE_TYPE  *RunDataPrt_B = ChgData_GetRunDataPtr(BGUN_NO);
				tmp32 = RunDataPrt_A->gun->i32temper[0]*100;   // 枪A温度
				Common_Change4Byte(&tmp32);
				data[cnt++] = tmp32 & 0xffff;		 
				data[cnt++] = tmp32 >> 16;

				tmp32 = RunDataPrt_B->gun->i32temper[0]*100;   //枪B温度
				Common_Change4Byte(&tmp32);
				data[cnt++] = tmp32 & 0xffff;		 
				data[cnt++] = tmp32 >> 16;

				CDMOD_DATA *ptrModData = CdModData_GetDataPtr(0);
				tmp32 = ptrModData->temper*10 ;//桩温度   模块进风口温度
				Common_Change4Byte(&tmp32);
				data[cnt++] = tmp32 & 0xffff;		 
				data[cnt++] = tmp32 >> 16;

				memset(&buf[0],0,sizeof(buf));
				tmp16 = Hmi_PackWriteReg(buf,(uint8*)&data,DGUS_TEMPER_A,CMD_WR_VAR,2*cnt);
				Screen_UartWrite(buf,tmp16);

				
				memset(&buf[0],0,sizeof(buf));                   //电费率
				memset(text1,0,sizeof(text1));
				REAL_BILL_DATA_ST *FeedDataPrt = GetFeeData(gunNo);
				Common_IntToFloatStr(text1,FeedDataPrt->real.currfeerate/100 ,2);
				strcat((char *)buf,(char *)text1);
				strcat((char *)buf,"元/Kwh");
				Hmi_ShowText(DGUS_RATE_POWER_A+gunNo*0x100,strlen(buf),buf);


				memset(&buf[0],0,sizeof(buf));                   //服务费率
				memset(text1,0,sizeof(text1));
				Common_IntToFloatStr(text1,FeedDataPrt->real.currSrvrate/100 ,2);
				strcat((char *)buf,(char *)text1);
				strcat((char *)buf,"元/Kwh");
				Hmi_ShowText(DGUS_RATE_SERVER_A+gunNo*0x100,strlen(buf),buf);
						
				memset(&buf[0],0,sizeof(buf));                   //占桩费率   待改，待添加
				memset(text1,0,sizeof(text1));
				Common_IntToFloatStr(text1,FeedDataPrt->real.parkfeerate/100 ,2);
				strcat((char *)buf,(char *)text1);
				strcat((char *)buf,"元/分钟");
				Hmi_ShowText(DGUS_RATE_OCCUPY_A+gunNo*0x100,strlen(buf),buf);
	  }
/*******************************************************************************/

		 
		/*显示卡号*/		
		if( START_BY_VIN != FeeDataPtr->bill.startmod){
		 rtn = UpCardNoASCII(buf,FeeDataPtr->bill.cardNo);// g_RunData.Ctrl.IcCardNo	
		}
		else{
			memcpy(buf,FeeDataPtr->bill.vincode,17);
			rtn = 17;
		}
		buf[rtn] = 0;
	  Common_TranCarNo(buf,rtn);
	  Hmi_ShowText(ChargeAdr[gunNo][1],rtn,(uint8 *)buf);
	  memset(buf,0,sizeof(buf));
		     
		/*显示充电方式*/
		tmp = RunDataPrt->logic->startby % CHARWAY_MUN;
		Hmi_ShowText(ChargeAdr[gunNo][2],strlen(concharway[tmp]),(uint8 *)concharway[tmp]);
		 
		/*充电结束方式显示*/
	// tmp = RunDataPrt->logic->startby % CHARWAY_MUN;
	 Hmi_ShowText(ChargeAdr[gunNo][3],strlen(concharstopway[tmp]),(uint8 *)concharstopway[tmp]);								 

}

extern uint8 ChangeReadVarForm(uint8 *des,stoc_u *source,uint8 len);
extern uint8 Hmi_ReadMulitVar(uint16 regAdr,uint16 *data,uint16 regNum);
void Screen_ShowCyclic()
{
   uint8 PageNo,tmplen = 0,tmpbuf[20];
	 stoc_u tmp[10] = {0};
	 char gunno = 'A';
	 static uint32 sTicks = 0;
	 static uint8 sTwoDim = 0;
	 PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	 
   if( GetSystemTick() - sTicks > TICK_TIMES_2S ) {
      sTicks = GetSystemTick();
		  PageNo = Hmi_GetCurrPicNo();
		  Netshow();       /*网络标志显示*/
		 	currfeeshow();   /*费率版本号显示*/  
	    switch(PageNo)
		  {
				case E_PICTURE1:  //交流接触器断开时，触摸屏偶尔重启
					IDEL_ShowGunState(BGUN_NO);
				case E_PICTURE47:	
					if(sTwoDim){
					  TwoDimBarCode(); /*二维码显示*/
						sTwoDim = 0;
					}
					IDEL_ShowGunState(AGUN_NO);
					break;
				case E_PICTURE2:
				case E_PICTURE10:
				case E_PICTURE14:	
					 sTwoDim = 1;
					 ChargeShow(BGUN_NO);
					break;
				case E_PICTURE5:
				case E_PICTURE7:
				case E_PICTURE8:		
				case E_PICTURE48:
					sTwoDim = 1;					
					ChargeShow(AGUN_NO);
				 break;
				case E_PICTURE6:	
					sTwoDim = 1;
					ChargeShow(BGUN_NO);
				  Delay10Ms(50);
				  ChargeShow(AGUN_NO);
					break;
				case E_PICTURE21:						
					BmsDetails(sGlocalPara.bmskey);       
					break;
				case E_PICTURE30:						
					Page_DebugPage();        
					break;
				case E_PICTURE31:						
					Page_DebugPage2();        
					break;
				case E_PICTURE32:						
					Page_ModeDetail(sGlocalPara.ModUpDownKey);           
					break;

				case E_PICTURE51:	
				case E_PICTURE52:
					if(1 == sGlocalPara.gun_no)
					{
						gunno = 'B';
					}
					Screen_ShowMessage((char*)&gunno,DGUS_STARTMODE_GUNNO); 

					Hmi_ReadMulitVar(DGUS_STARTMODE_CHARGAPSW,(uint16 *)&tmp[0],8);
          tmplen = ChangeReadVarForm(&tmpbuf[0],&tmp[0],16);
					memset(&tmpbuf[0],'*',tmplen);
					tmpbuf[tmplen] = 0;
					Hmi_ClearReg(DGUS_STARTMODE_CHARGAPSWSHOW,9);
					Screen_ShowMessage((char*)&tmpbuf[0],DGUS_STARTMODE_CHARGAPSWSHOW); 
					break;
					

				default:
					break;				
      }	 
   }
}


/*网络状态显示*/
void Netshow()
{
   u16tobit_u  tmpbit;
	 uint16  keystip = 0;
	 CHARGE_TYPE  *RunDataPrtA = ChgData_GetRunDataPtr(AGUN_NO);
	 CHARGE_TYPE  *RunDataPrtB = ChgData_GetRunDataPtr(BGUN_NO);
	
	 tmpbit.word = 0;	
	 tmpbit.bits.bit0 = TskBack_CommStatus() & 0x01;	
	 Hmi_WriteOneVar(DGUS_NET_ADDR,tmpbit.word);
	
//	 keystip = (START_BY_VIN == RunDataPrtA->logic->startby)? 1 :2;
	 if((START_BY_VIN == RunDataPrtA->logic->startby) || (START_BY_PASSWD == RunDataPrtA->logic->startby)){
		 keystip = 1;
	 }
	 else{
		 keystip = 2;
	 }
	 
	 	if (SUPPORT_GUN_START == GUNSTART_SUPPORT )
		{
			 keystip = 1;
		}
	 

	PARAM_DEV_TYPE *ptrDevPara = ChgData_GetDevParamPtr();
	PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	if( (CONN_CHGBIRD == BackCOMM->agreetype) && (ptrDevPara->onlinecard == E_ONLINE_CARD) && (START_BY_BKGROUND == RunDataPrtA->logic->startby) )
	{
		keystip = 1;
	}

	 Hmi_WriteOneVar(DGUS_KEYSTOPA_ADDR,keystip);
//	 keystip = (START_BY_VIN == RunDataPrtB->logic->startby)? 1 :2;
	 if((START_BY_VIN == RunDataPrtB->logic->startby) || (START_BY_PASSWD == RunDataPrtB->logic->startby)){
		 keystip = 1;
	 }
	 else{
		 keystip = 2;
	 }

	 if( (CONN_CHGBIRD == BackCOMM->agreetype) && (ptrDevPara->onlinecard == E_ONLINE_CARD) && (START_BY_BKGROUND == RunDataPrtB->logic->startby) )
	 {
		 keystip = 1;
	 }

	 Hmi_WriteOneVar(DGUS_KEYSTOPB_ADDR,keystip);
	
}


/*logo显示*/
void LogoShow()
{
		Hmi_WriteOneVar(DGUS_LOGOSHOW_ADR,0);
}


/*触摸屏主函数*/
void Task_Screen_Main(void *p_arg)
{
  uint8 cnt;
	PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	
	Screen_Init();
	Screen_ComInit();
	Message_QueueCreat(APP_TASK_SCREEN_PRIO);
	Delay10Ms(100);
	Hmi_SetLcdLight(DAYTIME_LIGHT);
	Hmi_ChangePicture(E_PICTURE0);
	Delay10Ms(100);
	
	cnt = 10;
	while (cnt--)
	{
		Delay10Ms(10);
	}
	Screen_SetTimeToBoardOnKey();
	Screen_ClearShowVar();
	Delay10Ms(50);
	LogoShow();      	/*logo显示*/
	Delay10Ms(20);
	TwoDimBarCode(); /*二维码显示*/
	Delay10Ms(20);
	if(GUNNUM_2 == devparaPtr->gunnum){
	  Hmi_ChangePicture(E_PICTURE1);	
	}
	else{
		Hmi_ChangePicture(E_PICTURE47);	
	}
	Delay10Ms(10);

#ifdef SCREEN_CARD
	TskReader_Init();
#endif	

	
	
	while(1) {
		
		TaskRunTimePrint("Task_Screen_Main begin", OSPrioCur);

#if 1
		/*接收触摸屏数据处理*/
		Screen_UploadDataDeal();
		
		/*时间刷新*/
		Screen_ShowDateTime();
		
      	/*触摸屏亮度控制*/
		Screen_LightControl();
		
		/*触摸屏任务刷新*/ 	
     	Screen_ShowCyclic();
		
		/*触摸屏消息处理*/
		Screen_MessageDeal();
		
		/*触摸屏倒计时*/
		Screen_DecCountDown();
		
		/*超时处理*/
		Screen_OperTimeOut();		

#endif

		TaskRunTimePrint("Task_Screen_Main end", OSPrioCur);

		Delay10Ms(APP_TASK_SCREEN_DELAY);  //patli 20200106 Delay5Ms(2); 不能太快和太慢，太慢导致消息接收不完全
			
	 }
}


static OS_STK  Stk_TaskScreenMain[APP_SCREEN_MAIIN_SIZE];

void Task_Screen_init()
{
	
	OS_CPU_SR cpu_sr;
	
	OS_ENTER_CRITICAL();
#if (OS_TASK_STAT_STK_CHK_EN)	
		OSTaskCreateExt(Task_Screen_Main,(void *)0, &Stk_TaskScreenMain[APP_SCREEN_MAIIN_SIZE - 1], APP_TASK_SCREEN_PRIO, APP_TASK_SCREEN_PRIO, Stk_TaskScreenMain, APP_SCREEN_MAIIN_SIZE, NULL, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#else
	OSTaskCreateExt(Task_Screen_Main,(void*)0,(OS_STK*)&Stk_TaskScreenMain[APP_SCREEN_MAIIN_SIZE-1],APP_TASK_SCREEN_PRIO, APP_TASK_SCREEN_PRIO, Stk_TaskScreenMain, ); 
#endif
	OS_EXIT_CRITICAL();
}

