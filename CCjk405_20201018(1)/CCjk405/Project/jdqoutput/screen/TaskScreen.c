
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
#include "HeLiBmsProto.h"



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


/*ˢ���򿪱���*/
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
#define  VAR5    (sGlocalPara.rcv.systime[0])


uint8 gUploadBuf[64] = {0};
const UploadOper_t conUploadVar[] = { 
		{DGUS_PW_SHOW_ADR,STRING_VAR,6,NONE_DOT,(uint16 *)&VAR0,Page_UploadPassWd},  	/*��������*/
		{DGUS_SETENERGY_ADR,DATA_VAR,2,NONE_DOT,(uint16 *)&VAR1,Page_KeyEnergyEnter},   /*���õ���*/
		{DGUS_SETMONEY_ADR,DATA_VAR,2,NONE_DOT,(uint16 *)&VAR2,Page_KeyMoneyEnter},    /*���ý��*/
		{DGUS_AETTIME_ADR,DATA_VAR,2,NONE_DOT,(uint16 *)&VAR3,Page_KeyTimeEnter},    	/*����ʱ��*/
//		{0x0003,INNER_REG,10,0,(uint16 *)gUploadBuf,Page_RegPictureNo},    					/*��ȡͼƬ��*/
		{0x0006,INNER_REG,10,0,(uint16 *)gUploadBuf,Page_RegIStouch},      						/*��ȡ�Ƿ��д���*/
		{DGUS_PW_CARD_ADR,PASSWD_VAR,10,NONE_DOT,(uint16 *)&VAR4,Page_KeyPwparam},  	/*��������*/ 
		{DGUS_SYS_TIME_ADR,STRING_VAR,16,NONE_DOT,(uint16 *)&VAR5,NULL},  	          /*����ϵͳʱ��*/ 		

	{INVALID_REG_ADR,0x0000,E_PICTUREANY,NULL,NULL},   /*��Ч*/
};
const Keyfunc1_t conKeyRegDef[] = {
	
	  {KEY_ADRESS_DEF,0x00A0,E_PICTURE1,Page_StartCharge_A,NULL},             /*1ҳ Aǹ��ʼ���*/
		{KEY_ADRESS_DEF,0x00A1,E_PICTURE1,Page_StartCharge_B,NULL},             /*1ҳ Bǹ��ʼ���*/
    {KEY_ADRESS_DEF,0x00A2,E_PICTURE1,Page_SysSet,NULL},                  /*1ҳ ϵͳ����*/
		
		{KEY_ADRESS_DEF,0x00A3,E_PICTURE5,Page_BMSDetails_B,NULL},           	  /*5ҳ BMS����  */		
    {KEY_ADRESS_DEF,0x00A4,E_PICTURE5,Page_BMSDetails_A,NULL},             	/*5ҳ BMS����  */
	
		{KEY_ADRESS_DEF,0x00D7,E_PICTURE5,Page_VINStop_A,NULL},        		/*VINֹͣ��ť*/
		{KEY_ADRESS_DEF,0x00D8,E_PICTURE5,Page_VINStop_B,NULL},        		/*VIN���ֹͣ��ť*/
		
    {KEY_ADRESS_DEF,0x0010,E_PICTURE17,Page_KeySetMoneyChargeMode,NULL},   /*17ҳ ��緽ʽ    ������*/
    {KEY_ADRESS_DEF,0x0011,E_PICTURE17,Page_KeySetEnergyChargeMode,NULL},  /*17ҳ ��緽ʽ    ��������*/
    {KEY_ADRESS_DEF,0x0012,E_PICTURE17,Page_KeySetTimesChargveMode,NULL},  /*17ҳ ��緽ʽ    ��ʱ���*/
    {KEY_ADRESS_DEF,0x0013,E_PICTURE17,Page_KeySetAutoChargeMode,NULL},    /*17ҳ ��緽ʽ    �Զ����*/
    {KEY_ADRESS_DEF,0x00A5,E_PICTURE17,Page_KeyExitChoose,NULL},           /*17ҳ ��緽ʽ    ����*/

    {KEY_ADRESS_DEF,0x00A6,E_PICTURE18,Page_KeyChargeModeSetReturn,NULL},  /*18ҳ ��������    ����*/

    {KEY_ADRESS_DEF,0x00A7,E_PICTURE19,Page_KeyChargeModeSetReturn,NULL},  /*19ҳ ������    ����*/

    {KEY_ADRESS_DEF,0x00A8,E_PICTURE20,Page_KeyChargeModeSetReturn,NULL},  /*20ҳ ��ʱ���    ����*/
		
	  {KEY_ADRESS_DEF,0x00A9,E_PICTURE21,Page_KeyBMSReturn,NULL},        		/*21ҳ BMS���� ����*/
			
		{KEY_ADRESS_DEF,0x00AA,E_PICTURE22,Page_Sysreturn,NULL},            	/*22ҳ ϵͳ����    ����*/
		{KEY_ADRESS_DEF,0x0020,E_PICTURE22,Page_SystemSet1,NULL},            	/*22ҳ ϵͳ����*/
    {KEY_ADRESS_DEF,0x0021,E_PICTURE22,Page_RechargeRecord,NULL},       	/*22ҳ ����¼*/	
		{KEY_ADRESS_DEF,0x0022,E_PICTURE22,Page_Debug,NULL},                	/*22ҳ ���Խ���*/
				
		{KEY_ADRESS_DEF,0x0023,E_PICTURE23,Page_SystemSet,NULL},   		        /*23ҳ ϵͳ����*/
		{KEY_ADRESS_DEF,0x0024,E_PICTURE23,Page_RatesSet,NULL},             	/*23ҳ ��������*/

		{KEY_ADRESS_DEF,0x00AB,E_PICTURE24,Page_KeySetTime,NULL},           	/*24ҳ ʱ��У׼*/		
		{KEY_ADRESS_DEF,0x00AC,E_PICTURE24,Page_KeySystem1Read,NULL},       	/*24ҳ ������ȡ��ť*/		
		{KEY_ADRESS_DEF,0x00AD,E_PICTURE24,Page_KeySystem1Set,NULL},        	/*24ҳ �������ð�ť*/
		{KEY_ADRESS_DEF,0x00AE,E_PICTURE24,Page_KeySystem1Return,NULL},     	/*24ҳ �������ذ�ť*/	

		{KEY_ADRESS_DEF,0x00AF,E_PICTURE25,Page_KeySeniorSystem,NULL},       	/*25ҳ �����߼����ð�ť*/		
		
		{KEY_ADRESS_DEF,0x00B0,E_PICTURE26,Page_KeySystem2Read,NULL},      		/*26ҳ ������ȡ��ť*/		
		{KEY_ADRESS_DEF,0x00B1,E_PICTURE26,Page_KeySystem2Set,NULL},       		/*26ҳ �������ð�ť*/
		{KEY_ADRESS_DEF,0x00B2,E_PICTURE26,Page_KeySystem2Return,NULL},    		/*26ҳ �������ذ�ť*/
		{KEY_ADRESS_DEF,0x00B3,E_PICTURE26,Page_KeySystemNext,NULL},		   		/*26ҳ ��һҳ*/				
		{KEY_ADRESS_DEF,0x00B4,E_PICTURE26,Page_KeySyRestart,NULL},        		/*26ҳ ϵͳ��λ*/
		{KEY_ADRESS_DEF,0x00B5,E_PICTURE26,Page_KeyClear,NULL},		         		/*26ҳ ������¼*/

		{KEY_ADRESS_DEF,0x00B6,E_PICTURE27,Page_KeySystem3Read,NULL},      		/*27ҳ ϵͳ�������ö�*/
		{KEY_ADRESS_DEF,0x00B7,E_PICTURE27,Page_KeySystem3Set,NULL},       		/*27ҳ ϵͳ��������д*/
		{KEY_ADRESS_DEF,0x00B8,E_PICTURE27,Page_KeySystem3Return,NULL},    		/*27ҳ ϵͳ�������÷���*/
		
		{KEY_ADRESS_DEF,0x00B9,E_PICTURE28,Page_KeyRecordUpPage,NULL},      	/*28ҳ����¼��ѯ��һҳ��ť*/		
		{KEY_ADRESS_DEF,0x00BA,E_PICTURE28,Page_KeyRecordDownPage,NULL},    	/*28ҳ����¼��ѯ��һҳ��ť*/
    {KEY_ADRESS_DEF,0x00BB,E_PICTURE28,Page_KeyRecordReturn,NULL},      	/*28ҳ  ����*/
					
  	{KEY_ADRESS_DEF,0x00BC,E_PICTURE29,Page_KeyRatesRead_A,NULL},        	/*29ҳ ���ʶ�ȡ��ť*/		
		{KEY_ADRESS_DEF,0x00BD,E_PICTURE29,Page_KeyRatesSet_A,NULL},         	/*29ҳ �������ð�ť*/
		{KEY_ADRESS_DEF,0x00BE,E_PICTURE29,Page_KeyRatesReturn,NULL},      	 /*29ҳ ���ʷ��ذ�ť*/	
		
    {KEY_ADRESS_DEF,0x00BF,E_PICTURE30,Page_KeyDebugNext,NULL},        		/*30ҳ ���Խ��� ��һҳ*/		
		{KEY_ADRESS_DEF,0x00C0,E_PICTURE30,Page_KeyModeDetail,NULL},       		/*30ҳ ģ������ */		
	  {KEY_ADRESS_DEF,0x00C1,E_PICTURE30,Page_KeyDebugReturn,NULL},		   		/*30ҳ ���Խ��� ����*/
		
		{KEY_ADRESS_DEF,0x00C2,E_PICTURE31,Page_KeyDebugPrev,NULL},		     		/*31ҳ ���Խ���2 ��һҳ*/	
		{KEY_ADRESS_DEF,0x00C3,E_PICTURE31,Page_KeyDebugReturn,NULL},      		/*31ҳ ���Խ��� ����*/
		
		{KEY_ADRESS_DEF,0x00C4,E_PICTURE32,Page_KeyModePrev,NULL},         		/*32ҳ ģ������  ��һҳ*/		
		{KEY_ADRESS_DEF,0x00C5,E_PICTURE32,Page_KeyModeNext,NULL},         		/*32ҳ ģ������  ��һҳ*/
		{KEY_ADRESS_DEF,0x00C6,E_PICTURE32,Page_KeyModeReturn,NULL},       		/*32ҳ ģ������   ����*/		
		
	  {KEY_ADRESS_DEF,0x00C7,E_PICTURE33,Page_KeyPassWdEnter,NULL},      		/*33ҳ ��������    ȷ��*/
    {KEY_ADRESS_DEF,0x00C8,E_PICTURE33,Page_KeyPwReturn,NULL},         		/*33ҳ ��������    ����*/
		
		{KEY_ADRESS_DEF,0x00C9,E_PICTURE34,Page_KeyPwCardEnter,NULL},     		/*34ҳ ������ȷ��*/
		{KEY_ADRESS_DEF,0x00CA,E_PICTURE34,Page_KeyPwCardReturn,NULL},    		/*34ҳ �����뷵��*/
				
		{KEY_ADRESS_DEF,0x00CB,E_PICTURE39,Page_KeyRateChoose_Return ,NULL},   /*39ҳ ����ѡ�� ����*/
		{KEY_ADRESS_DEF,0x0025,E_PICTURE39,Page_KeyRateChoose_A,NULL},    		 /*39ҳ ����ѡ�� Aǹ*/
		{KEY_ADRESS_DEF,0x0026,E_PICTURE39,Page_KeyRateChoose_B,NULL},    		 /*39ҳ ����ѡ�� Bǹ*/
		
  	{KEY_ADRESS_DEF,0x00CC,E_PICTURE40,Page_KeyRatesRead_B,NULL},        		/*29ҳ ���ʶ�ȡ��ť*/		
		{KEY_ADRESS_DEF,0x00CD,E_PICTURE40,Page_KeyRatesSet_B,NULL},         		/*29ҳ �������ð�ť*/
		{KEY_ADRESS_DEF,0x00CE,E_PICTURE40,Page_KeyRatesReturn,NULL},      		  /*29ҳ ���ʷ��ذ�ť*/			
					
		{KEY_ADRESS_DEF,0x00D0,E_PICTURE1,Page_KeyAuciliaryPow_A,NULL},       /*Aǹ�л���Դ*/
		{KEY_ADRESS_DEF,0x00D1,E_PICTURE1,Page_KeyAuciliaryPow_B,NULL},       /*Bǹ�л���Դ*/
		
		{KEY_ADRESS_DEF,0x0030,E_PICTURE45,Page_KeyAuPowChoose12,NULL},       /*45ҳ ��Դ���淵��*/
		{KEY_ADRESS_DEF,0x0031,E_PICTURE45,Page_KeyAuPowChoose24,NULL},		    /*45ҳ ��Դ����12V*/
		{KEY_ADRESS_DEF,0x00D2,E_PICTURE45,Page_KeyAuPowReturn,NULL},		      /*45ҳ ��Դ����24V*/
		
		{KEY_ADRESS_DEF,0x00D3,E_PICTURE46,Page_KeyAuPowConfirm,NULL},        /*46ҳ ȷ��*/
		{KEY_ADRESS_DEF,0x00D4,E_PICTURE46,Page_KeyAuPowCancel,NULL},         /*46ҳ ȡ��*/
		
		{KEY_ADRESS_DEF,0x00D5,E_PICTURE1,Page_KeyVINStart_A,NULL},
		{KEY_ADRESS_DEF,0x00D6,E_PICTURE1,Page_KeyVINStart_B,NULL},


		{KEY_ADRESS_DEF,0x00DA,E_PICTURE51,Page_KeyStartModeReturn,NULL},           /*51ҳ ����*/
		{KEY_ADRESS_DEF,0x00DB,E_PICTURE51,Page_KeyPasswdModeEnter,NULL},           /*51ҳ ������ȷ��*/
//		{KEY_ADRESS_DEF,0x00DC,E_PICTURE46,Page_KeyCardModeEnter,NULL},           /*51ҳ ������ȷ��*/

		{KEY_ADRESS_DEF,0x00E0,E_PICTURE52,Page_KeyChargPwdReturn,NULL},           /*52ҳ ����*/
		{KEY_ADRESS_DEF,0x00E1,E_PICTURE52,Page_KeyChargPwdEnter,NULL},           /*52ҳ �������ȷ��*/
 
		
		{INVALID_REG_ADR,0x0000,E_PICTUREANY,NULL,NULL},   										/*��Ч*/
};


typedef struct
{
   uint8 code;
   char * str;
}STR_ERR_ST;

const STR_ERR_ST conErrStr[]=
{ 
	{ECODE_NORMAL,            "ERROR!!!"},
  {ECODE1_ELOCK,            "����������"},
	{ECODE2_BHMTM,            "BHM��ʱ"},
	{ECODE3_XFTM,             "й�ų�ʱ"},
	{ECODE4_BRMTM,            "BRM��ʱ"},
	{ECODE5_BCPTM,            "BCP��ʱ"},
	{ECODE6_BROTM,            "BRO��ʱ"},
	{ECODE7_BCLTM,            "BCL��ʱ"},
	{ECODE8_BCSTM,            "BCS��ʱ"},
	{ECODE9_BSTTM,            "BST��ʱ"},
	{ECODE10_CSTTM,           "CST��ʱ"},
	{ECODE11_BSDTM,           "BSD��ʱ"},
	{ECODE12_KM10V,           "KM����ѹ����10V"},
	{ECODE13_KM60V,           "ֱ���Ӵ���ճ��"},//"KM����ѹ����60V"},
	{EOCDE14_PARAM,           "��������������"},
	{ECODE15_PKGVOLT,         "�뱨�ĵ�ѹƫ����ڡ�5%"},
	{ECODE16_BATSTU,          "���״̬������"},
	{ECODE17_FORBID,          "����BMS��ֹ���"},
	{ECODE18_CC1LNK,          "���ǹ���Ӵ���"},
	{ECODE19_MODVOLT,         "ģ���ѹ��������"},
	{ECODE20_INOUT10V,        "KM������ѹ�����10V"},
	{ECODE31_BMSCOMM ,        "BMSͨѶ����"},
	{ECODE32_GUNTMPOVER,      "���ǹ����"},
  {ECODE35_ISOCOMM,         "��Եģ��ͨѶ����"},
  {ECODE36_1POSERR,         "��Ե���Եع���"},
	{ECODE37_1NEGERR,         "��Ե���Եع���"},
	{ECODE38_2POSERR,         "��Ե����"},//"��Ե��·���Եع���"},
	{ECODE39_2NEGERR,         "��Ե��·���Եع���"},
	{ECODE41_MODCOMMERR,      "��Դģ��ͨѶ����"},
	{ECODE42_DC1OVER,         "ֱ�������ѹ"},
	{ECODE43_DC2OVER,         "2ֱ�������ѹ"},
	{ECODE44_DC3OVER,         "3ֱ�������ѹ"},
	{ECODE45_DC4OVER,         "4ֱ�������ѹ"},
{HELI_ECODE46_CONNECTOR_TEMPERATURE_LOSS,  "heli������¶ȶ�ʧ"},
{HELI_ECODE47_OTHER,                       "heli����"},
{HELI_ECODE48_OUTSIDE_KM_40LESS,			     "heli��ص�ѹ����40V"},
{HELI_ECODE49_BMS_STOP_NORMOL,             "heli������"},    
	{ECODE50_BMSFINISH,       "�յ�BMSֹͣ����,�ﵽSOC�趨ֵ"},
	{ECODE51_BMSTVOLT,        "�յ�BMSֹͣ����,�ﵽ�ܵ�ѹ�趨ֵ"},
	{ECODE52_BMSSVOLT,        "�յ�BMSֹͣ����,�ﵽ�����ѹ�趨ֵ"},
	{ECODE53_BMSCST,          "�յ�BMSֹͣ����,����������ֹ"},
	{ECODE55_BMSISO,          "�յ�BMS��Ե���ϱ���"},
	{ECODE56_BMSLKTMP,        "�յ�BMS������������¶ȹ���"},
	{ECODE57_BMSPARTTMP,      "�յ�BMSԪ������"},
	{ECODE58_BMSCLKERR,       "�յ�BMS�������������"},
	{ECODE59_BMSBATTMP,        "�յ�BMS������¶ȹ���"},
	{ECODE60_BMSKMERR,        "�յ�BMS��ѹ�̵�������"},
	{ECODE61_BMSCHK2ERR,      "�յ�BMS����2��ѹ������"},
	{ECODE62_BMSOTHERERR,     "�յ�BMS��������"},
	{ECODE64_BMSCURROVER,     "�յ�BMS�����������"},
	{ECODE65_BMSVOLTERR,      "�յ�BMS��ѹ�쳣����"},
	{ECODE67_BMSCRMTM,        "BMS���ճ���CRM 00���ĳ�ʱ"},
	{ECODE68_BMSCRMATM,       "BMS���ճ���CRM AA���ĳ�ʱ"},
	{ECODE69_BMSCTSTM,        "BMS���ճ���CTS��CML���ĳ�ʱ"},
	{ECODE70_BMSCROTM,        "BMS���ճ���CRO���ĳ�ʱ"},
	{ECODE71_BMSCCSTM,        "BMS���ճ���CCS���ĳ�ʱ"},
	{ECODE72_BMSCSTTM,        "BMS���ճ���CST���ĳ�ʱ"},
	{ECODE73_BMSCSDTM,        "BMS���ճ���CSD���ĳ�ʱ"},
	{ECODE74_BMSST10MIN,      "�����ͣ����30����"},
	{ECODE80_CHGFINISH,       "�ﵽ�����趨������ֹ"},
	{ECODE81_CHGHUMST,        "�˹�����ֹͣ"},
	{ECODE82_CHGERRST,        "����ֹͣ"},
	{ECODE83_CHGBST,          "�յ�BMS��ֹ����ֹͣ"},
	{ECODE84_NOMONEY,			    "�˻�������������ֹ"},
	{ECODE85_CHGTMOVER,       "�������¶ȹ���"},
	{ECODE86_CHGLKERR,        "�������������"},
	{ECODE87_CHGINTMPERR,     "�����ڲ����¹���"},
	{ECODE88_CHGENERGY,       "�������ܴ���"},
	{ECODE89_CHGJTST,         "�ֶ�ֹͣ,���ٰ�һ�θ�λ"},//"��ͣ����,��������Ť��ͣ��ť�ָ�"},
  {ECODE90_CHGOTHER,        "��������"},
	{ECODE91_CHGCURRERR,      "��������ƥ��"},    
	{ECODE92_CHGVOLTERR,      "������ѹ�쳣"},
	{ECODE93_METERCOMMERR,    "ֱ����ͨѶ����"},
	{ECODE94_CURROVER,        "ֱ���������"},
	{ECODE95_ACMETCOMMERR,    "������ͨѶ����"},
	{ECODE96_ACVOVERR,        "���������ѹ"},
	{ECODE97_ACUnerERR,       "��������Ƿѹ"},
	{ECODE98_ACDEFICY,        "����ȱ��"},
	{ECODE99_DooracsERR,      "�Ž�1����"},
	{ECODE100_DoorafERR,      "�Ž�2����"},
	{ECODE101_DCFAUSEERR,     "ֱ����˿����"},   //20181206 add
	{ECODE102_SPDERR,         "����������"},
	{ECODE103_GUNOVERTEMPER,     "ǹ���¶ȹ���"},
	{ECODE104_PAVOUTSIDEREVERSE,  "ǹ����ѹ����(BMS��)"},
	{ECODE105_PAVINSIDEREVERSE,   "ǹ�ڲ��ѹ����(ģ���)"},
	{ECODE106_VINBACKTM,          "��̨��ȡVIN��Ϣ��ʱ"},
	{ECODE107_BACKOFF,"��̨ͨѶ����"},
	{ECODE108_BCPOVERVOLT,"����ѹ����BCP�����������ֵ"},
	
{HELI_ECODE109_CHARG_VOL_OVER,              "heli��ѹ���䱣��"},
{HELI_ECODE110_OVER_TEMPERATURE,            "heli���±���"},
{HELI_ECODE111_INTERLOCK,                   "heli��������"},
{HELI_ECODE112_LOWER_TEMPERATURE,           "heli���³�籣��"},
{HELI_ECODE113_CELL_VOL_LOWER,              "heli�����ѹ����"},
{HELI_ECODE114_CURR_OVER,                   "heli����������"},
{HELI_ECODE115_BMS_FAULT,                   "heliBMS ���ϱ���"},
{HELI_ECODE116_CONNECTOR_OVER_TEMPERATURE,  "heli��������±���"},
{HELI_ECODE117_CC2_FAULT,                   "heli��� CC2 �쳣"},
{HELI_ECODE118_CELLS_VOL_DIFF,              "heli����ѹ�����"},
{HELI_ECODE119_CELLS_TEMPERATURE_DIFF,      "heli�����²����"},	
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
***��������: Screen_GetKeyItemByKeyValue
** ��������: ͨ������ֵ���Ұ�������Ŀ
***    ����: 
**   ����ֵ:   
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
***��������: Screen_GetUpLoadVarItem
** ��������: ͨ����ַ���ҼĴ����ϴ�����Ŀ
***    ����: 
**   ����ֵ:   
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
***��������: Screen_KeyDeal
** ��������: ����ִ�еĶ���
***    ����: 
**   ����ֵ:   
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
***��������: Screen_GetUpLoadVarItemNum
** ��������: ��ȡ�ϴ����ñ����ĸ���
***    ����: 
**   ����ֵ:   
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
***��������: Screen_DataDotChange
** ��������: ������ݽ���С����ת��
***    ����: 
**   ����ֵ:   
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
***��������: Screen_Fill_DataVar
** ��������: ������ݱ�����ֵ,���ڽ������ݸ�ֵ
***    ����: 
**   ����ֵ:   
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
***��������: Screen_ChangePassWord
** ��������: ��Э��֡�л�ȡ���룬����bcd��ʽת��
***    ����: 
**   ����ֵ:   
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
***��������: Screen_UploadVarDeal
** ��������: �ϴ����ݴ���
***    ����: 
**   ����ֵ:   
*************************************************************************/
void Screen_UploadVarDeal(uint16 adr,uint16 regNum,uint16 *data)
{
	uint16 i,srcadd;
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
       if((DWIN_DGUS_II == SCREEN_TYPE) && (DGUS_PW_SHOW_ADR == adr))
			 {
				 for( i = 0; i < regNum*2;i++ ) {
					 if(ptr[i] == 0xff ) {
						 ptr[i] = 0;
						 break;
					 }
				 }		
				 memcpy(varitem->var,&ptr[2],(regNum-2)*2);				 
			 }	
			 else
			 {
				 for( i = 0; i < regNum*2;i++ ) {
					 if(ptr[i] == 0xff ) {
						 ptr[i] = 0;
						 break;
					 }
				 }	
				 memcpy(varitem->var,data,regNum*2);				 
			 }

			 
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
		varitem->func();               /*�������޸�*/
	}
		
}

/***********************************************************************
***��������: Screen_UploadDataDeal
** ��������: �������������ݵĴ���
***    ����: 
**   ����ֵ:   
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
		
  }while((cnt < 3 ) && (ptr < &rcvbuf[100])); /*���մ���100�ֽ��˳�*/
	
	
END:
  	num = (uint8)(ptr - &rcvbuf[0]);
	
	if( num < 3 || tlen < 5 ) {
		   return FALSE;
	  }
	
	
//	printf("Screen_UploadDataDeal len=%d\r\n", tlen);
	/*У�鱨��*/
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
			Screen_KeyDeal(reg16,data16);	   /*�����ϴ�����*/
		}
		Screen_OpenBackLightByCard();
		sGlocalPara.lasttm = GetSystemTick();
	}
	else {
		if(sframe->cmd != CMD_RD_REG ) {
			Screen_OpenBackLightByCard();
			sGlocalPara.lasttm = GetSystemTick();	
			Screen_UploadVarDeal(reg16,sframe->num,sframe->nsData);  /*����¼���ϴ�����*/
		}else { /*CMD = 0x81 ���ڲ��Ĵ�������*/
			sRegFrame = (ScrInRegFrame_t *)sframe;
			Screen_UploadVarDeal(sRegFrame->nReg,sRegFrame->num,sRegFrame->nsData);
		}
		
	}
	
	return TRUE;
}


/***********************************************************************
***��������: Screen_ClearShowVar
** ��������: �����ʾ����
***    ����: 
**   ����ֵ:   
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
    bynum = (Screen_GetUpLoadVarItemNum()-1)*2*2 %64; /*������64��*/
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
***��������: Screen_SetTimeToBoardOnKey
** ��������: ��������ʱ�����õ����ذ�
***    ����: 
**   ����ֵ:   
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
	 Hmi_RequestScreenTime();  //��������ʱ�䷢�ͺ���
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
  }while( (errcnt < 5) && (cnt < 1) ); /*���մ���100�ֽ��˳�*/
	
	if( tlen < 6 ) {
		return FALSE;
	}
	
//	printf("Screen_SetTimeToBoardOnKey len=%d\r\n", tlen);
	/*У�鱨��*/
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
***��������: Screen_UartWrite
** ��������: ���������ڷ������ݺ���
***    ����: 
**   ����ֵ:   
*************************************************************************/
int32 Screen_UartWrite(uint8 *buf,	int32 size)
{
	#if 1
	if(SCREEN_TYPE == DWIN_DGUS_II)
	{
		Delay10Ms(5);
	}		
	return Uart_Write(gPtrUartHandle,buf,size);	
	#else
	return 0;
	#endif
}

/***********************************************************************
***��������: Screen_UartRead
** ��������: ��ȡ��������������
***    ����: 
**   ����ֵ:   
*************************************************************************/
int32 Screen_UartRead(uint8 *buf,uint16 size)
{
	return Uart_Read(gPtrUartHandle,buf,size);
}



/***********************************************************************
***��������: Screen_ComInit
** ��������: ��ʼ������������
***    ����: 
**   ����ֵ:   
*************************************************************************/
/* ��ʼ������ */
void Screen_ComInit(void)
{
	int32 tmp;

	/* ��ȡ���� */
	UartHandler[SCREEN_COM] = Uart_Open(SCREEN_COM);
	assert_param(UartHandler[SCREEN_COM]);
  gPtrUartHandle = UartHandler[SCREEN_COM]; 
	
	/* ������ */
 	tmp  = 38400;//115200;
	Uart_IOCtrl(UartHandler[SCREEN_COM], SIO_BAUD_SET, &tmp);	

	/* ��У�� */
	tmp  = USART_Parity_No;
	Uart_IOCtrl(UartHandler[SCREEN_COM], SIO_PARITY_BIT_SET, &tmp);

	/* ֹͣλ */
	tmp  = USART_StopBits_1;
	Uart_IOCtrl(UartHandler[SCREEN_COM], SIO_STOP_BIT_SET, &tmp);	
	
	/* 8������λ */
	tmp = USART_WordLength_8b;
	Uart_IOCtrl(UartHandler[SCREEN_COM], SIO_DATA_BIT_SET, &tmp);	
}


/***********************************************************************
***��������: Screen_Init
** ��������: ������������س�ʼ������
***    ����: 
**   ����ֵ:   
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
***��������: Screen_LightControl
** ��������: ���������� ���� ����
***    ����: ��
**   ����ֵ:   
**  ���ȷ�Χ 0 --- 64 
*************************************************************************/


void Screen_LightControl(void)
{
	
	static uint8  lightpic = E_PICTURE1;  //��Ļ����������ʱ��ʾ�Ľ��� add1204
	static uint8  sWorkStatu[DEF_MAX_GUN_NO] = {0};
	static uint16 sStatu[DEF_MAX_GUN_NO] = {0};
	static uint32 ltime = 0;
	static uint32 ideltime = 0;
	DEV_GUN_TYPE *ptrGunA = Check_GetGunDataPtr(AGUN_NO);
	DEV_GUN_TYPE *ptrGunB = Check_GetGunDataPtr(BGUN_NO);
	
	if((sStatu[AGUN_NO] != ptrGunA->statu.bits.cc1stu)||(sStatu[BGUN_NO] != ptrGunB->statu.bits.cc1stu)  )  //ǹ����
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
***��������: Screen_ShowMessage
** ��������: ��ʾ�ַ���
***    ����: 
**   ����ֵ:   
*************************************************************************/
void Screen_ShowMessage(char *message,uint16 adr)
{
	 Hmi_ShowText(adr,strlen(message),(uint8 *)message);
}


/***********************************************************************
***��������: Screen_ShowDateTime
** ��������: ����ʱ����ʾ��һ�촥�����������ʱһ��
***    ����: 
**   ����ֵ:   
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
			
			/*ÿ6��Сʱ��ʱһ��*/
			if( ((gScreenSystime.tm_hour == 0) || (gScreenSystime.tm_hour == 6) \
				|| (gScreenSystime.tm_hour == 12) || (gScreenSystime.tm_hour == 18)) \
			&& (gScreenSystime.tm_min == 5) && (gScreenSystime.tm_sec == 5) ){
				if(TskBack_CommStatus() ) { /*û������վʱ��ִ��Уʱ*/
					//Screen_SetTimeToBoard();
				}
			}
		}
}


/***********************************************************************
***��������: Screen_OpenBackLightByCard
** ��������: ����ʾ������
***    ����: 
**   ����ֵ:   
*************************************************************************/

void Screen_OpenBackLightByCard(void)
{
	  sGlocalPara.light.statu = TOUCH_OPER;
	  gOpertime = GetSystemTick();
}


/***********************************************************************
***��������: Screen_ShowCountDown
** ��������: ��ʾ����ʱ
***    ����: 
**   ����ֵ:   
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
***��������: Screen_DecCountDown
** ��������: ����ʱ
***    ����: 
**   ����ֵ:   
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
***��������: Screen_StartDownCounter
** ��������: ��������ʱ
***    ����: 
**   ����ֵ:   
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
***��������: Screen_StartDownCounter
** ��������: ����ʱ�Ƿ�ʱ�䵽
***    ����: 
**   ����ֵ:   
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
***��������: Screen_DownCounterStop
** ��������: ֹͣ����ʱ
***    ����: 
**   ����ֵ:   
*************************************************************************/
void Screen_DownCounterStop(void)
{
	 sGlocalPara.DownCounter.start = 0;
	 sGlocalPara.DownCounter.currVal = 0;
	 sGlocalPara.DownCounter.inVal = GetSystemTick();
}



/***********************************************************************
***��������: Screen_Fill_VarToBuf
** ��������: ���ڽ�������ֵ���͵�������
***    ����: 
**   ����ֵ:   
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
***��������: Screen_ShowCyclic 
** ��������: ����������ѭ����ʾ����
***    ����: 
**   ����ֵ:   
*************************************************************************/
const char *concharway[] = \
{
  "ˢ������",
	"�ƶ��ͻ�������",
  "���߿�����",
	"VIN����",
	"��ǹ����",
	"�ֶ�����",
	"�˻���������",
};

#define CHARWAY_MUN  ((sizeof(concharway)/sizeof(char *)) + 1)

const char *concharstopway[] = \
{
  "��ˢ��ֹͣ",
	"��ʹ���ƶ��ͻ���ֹͣ",	
  "��ʹ�����߿�ֹͣ",
	"������ťֹͣ",
	"������ťֹͣ",
	"���ֶ�ֹͣ",
	"��������ֹͣ",
};

const char * conReasonString[] = \
{
   "δ֪ԭ��ֹͣ",
	 "ˢ��ֹͣ",
	 "�ƶ��ͻ���ֹͣ",
	 "����ֹͣ",
	 "�ﵽ�趨����ֹͣ",
	 "BMS����ֹͣ",
	 "��������",
	 "����������ֹͣ",
};

#define REASOM_MUN  ((sizeof(conReasonString)/sizeof(char *)) + 1)


/*�����������ı���ʾ*/
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
	   Screen_ShowMessage(" ",TempAddr);  //����ı�����
		 TempAddr += DGUS_ACCOUNTS_OFFSET;
  }
}


/***********************************************************************
***��������: Screen_OpenAccountCardDeal
** ��������: δ���㿨����
***    ����: 
**   ����ֵ:   
*************************************************************************/

const uint16 conActAddr[DEF_MAX_GUN_NO][10]=\
{
  {DGUS_CARDID_ADR_A,DGUS_START_ADR_A,DGUS_STOP_ADR_A,DGUS_REASON_ADR_A,DGUS_ENERGY_ADR_A\
	 ,DGUS_MONEY_ADR_A,DGUS_TIME_ADR_A,DGUS_WAY_ADR_A,DGUS_SOC_ADR_A,DGUS_REMONEY_ADR_A},
	{DGUS_CARDID_ADR_B,DGUS_START_ADR_B,DGUS_STOP_ADR_B,DGUS_REASON_ADR_B,DGUS_ENERGY_ADR_B\
	 ,DGUS_MONEY_ADR_B,DGUS_TIME_ADR_B,DGUS_WAY_ADR_B,DGUS_SOC_ADR_B,DGUS_REMONEY_ADR_B},

};

/*����*/
uint8 Screen_OpenAccountCardDeal(uint8 gunNo)
{
	 uint8 tmp;
	 uint16 rtn;
	 char str[10];
   uint8 text[120] = {0};
   
	 REAL_BILL_DATA_ST 	*FeeDataPtr = GetFeeData(gunNo);
   CHARGE_TYPE  *RunDataPrt = ChgData_GetRunDataPtr(gunNo);
	 
	 Clear_AccountShow(gunNo);   //���������ʾ
	 
	 /*��ʾ����*/
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
	 
	 sprintf((char *)text,"%d��%d��%d�� ",Common_Bcd_Decimal(FeeDataPtr->bill.starttm[0])+2000\
	                                 ,Common_Bcd_Decimal(FeeDataPtr->bill.starttm[1])%13\
	                                 ,Common_Bcd_Decimal(FeeDataPtr->bill.starttm[2])%32 );
                                 
	  //ʱ
	  sprintf(str,"%2d:",Common_Bcd_Decimal(FeeDataPtr->bill.starttm[3])%24 );
	  str[0] = (str[0] == 0x20)? 0x30 : str[0];
	  strcat((char *)text,str);
	  //��
	  sprintf(str,"%2d:",Common_Bcd_Decimal(FeeDataPtr->bill.starttm[4])%60 );
	  str[0] = (str[0] == 0x20)? 0x30 : str[0];
	  strcat((char *)text,str);
	  // ��
	  sprintf(str,"%2d",Common_Bcd_Decimal(FeeDataPtr->bill.starttm[5])%60 );
	  str[0] = (str[0] == 0x20)? 0x30 : str[0];
	  strcat((char *)text,str);	 
	  /*��ʾ��ʼʱ��*/
	  Hmi_ShowText(conActAddr[gunNo][1],strlen((char *)text),text);

//------------------
		PARAM_DEV_TYPE *ptrDevPara = ChgData_GetDevParamPtr();
	  PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
		if( (ptrDevPara->onlinecard == E_ONLINE_CARD) && (CONN_CHGBIRD == BackCOMM->agreetype) )
		{
			memset(text,0,sizeof(text));
		
			/*��ʾ���ʱ��*/
			sprintf((char *)text,"%d",FeeDataPtr->bill.chgsecs/60);
			strcat((char *)text,"����");
			Hmi_ShowText(conActAddr[gunNo][2],strlen((char *)text),text);
			memset(text,0,sizeof(text));

			/*��ʾռ׮ʱ��*/   //����
			sprintf((char *)text,"%d",FeeDataPtr->bill.parksecs/60);    
			strcat((char *)text,"����");
			Hmi_ShowText(conActAddr[gunNo][3],strlen((char *)text),text);
			memset(text,0,sizeof(text));

			/*��ʾ����*/
			Common_IntToFloatStr(text,FeeDataPtr->bill.energy,2);
			strcat((char *)text," ��");
			Hmi_ShowText(conActAddr[gunNo][4],strlen((char *)text),text);
			memset(text,0,sizeof(text));

			/*��ʾ��ѽ��*/
			Common_IntToFloatStr(text,FeeDataPtr->bill.billmoney,2);
			strcat((char *)text," Ԫ");
			Hmi_ShowText(conActAddr[gunNo][5],strlen((char *)text),text);
			memset(text,0,sizeof(text));

			/*��ʾ�����*/      //����
			Common_IntToFloatStr(text,FeeDataPtr->bill.serivemoney,2);
			strcat((char *)text," Ԫ");
			Hmi_ShowText(conActAddr[gunNo][6],strlen((char *)text),text);
			memset(text,0,sizeof(text));

			/*��ʾռ׮��*/      //����
			Common_IntToFloatStr(text,FeeDataPtr->bill.parkmoney,2);
			strcat((char *)text," Ԫ");
			Hmi_ShowText(conActAddr[gunNo][7],strlen((char *)text),text);
			memset(text,0,sizeof(text));

			/*��ʾ�����ܶ�*/    //����
			Common_IntToFloatStr(text,FeeDataPtr->bill.billmoney,2);
			strcat((char *)text," Ԫ");
			Hmi_ShowText(conActAddr[gunNo][8],strlen((char *)text),text);
			memset(text,0,sizeof(text));

		}
		else
		{
//------------------

			memset(text,0,sizeof(text));
		 
			sprintf((char *)text,"%d��%d��%d�� ",Common_Bcd_Decimal(FeeDataPtr->bill.stoptm[0])+2000\
																		 ,Common_Bcd_Decimal(FeeDataPtr->bill.stoptm[1])%13\
																		 ,Common_Bcd_Decimal(FeeDataPtr->bill.stoptm[2])%32 );
																		 
			//ʱ
			sprintf(str,"%2d:",Common_Bcd_Decimal(FeeDataPtr->bill.stoptm[3])%24 );
			str[0] = (str[0] == 0x20)? 0x30 : str[0];
			strcat((char *)text,str);
			//��
			sprintf(str,"%2d:",Common_Bcd_Decimal(FeeDataPtr->bill.stoptm[4])%60 );
			str[0] = (str[0] == 0x20)? 0x30 : str[0];
			strcat((char *)text,str);
			// ��
			sprintf(str,"%2d",Common_Bcd_Decimal(FeeDataPtr->bill.stoptm[5])%60 );
			str[0] = (str[0] == 0x20)? 0x30 : str[0];
			strcat((char *)text,str);
			
			/*��ʾ����ʱ��*/				
			Hmi_ShowText(conActAddr[gunNo][2],strlen((char *)text),text);
			memset(text,0,sizeof(text));
			/*��ʾԭ��*/
			tmp = FeeDataPtr->bill.endreason % REASOM_MUN;
			Screen_ShowMessage("                         ",conActAddr[gunNo][3]);  //����ı�����
			if(EERR_REASON == tmp)
			{
				const STR_ERR_ST *errstr;
				errstr = Screen_GetErrSting( FeeDataPtr->bill.errcode);
				Screen_ShowMessage(errstr->str,conActAddr[gunNo][3]);  //���Ͻ���
			}
			else
			{
				/*��������*/
				Hmi_ShowText(conActAddr[gunNo][3],strlen(conReasonString[tmp]),(uint8 *)conReasonString[tmp]);
			}
			
			/*��ʾ����*/
			Common_IntToFloatStr(text,FeeDataPtr->bill.energy,2);
			strcat((char *)text," ��");
			Hmi_ShowText(conActAddr[gunNo][4],strlen((char *)text),text);
			memset(text,0,sizeof(text));
			
			/*��ʾ���*/
			Common_IntToFloatStr(text,FeeDataPtr->bill.billmoney,2);
			strcat((char *)text," Ԫ");
			Hmi_ShowText(conActAddr[gunNo][5],strlen((char *)text),text);
			memset(text,0,sizeof(text));
		
			/*��ʾ���ʱ��*/
			sprintf((char *)text,"%d",FeeDataPtr->bill.chgsecs/60);
			strcat((char *)text,"����    ");
			Hmi_ShowText(conActAddr[gunNo][6],strlen((char *)text),text);
			memset(text,0,sizeof(text));
			
			/*��ʾ��緽ʽ*/
			tmp = RunDataPrt->logic->startby % CHARWAY_MUN ;
			Hmi_ShowText(conActAddr[gunNo][7],strlen(concharway[tmp]),(uint8 *)concharway[tmp]);
			memset(text,0,sizeof(text));
			
			/*��ʽ����SOC*/
			sprintf((char *)text,"%d",FeeDataPtr->bill.soc);
			strcat((char *)text,"%    ");
			Hmi_ShowText(conActAddr[gunNo][8],strlen((char *)text),text);
			memset(text,0,sizeof(text));
		}
				
		/*�����*/
		Common_IntToFloatStr((uint8 *)str,FeeDataPtr->bill.aftermoney,2);
		strcat(str," Ԫ");
		Screen_ShowMessage(str,conActAddr[gunNo][9]) ;	
				
		return TRUE;
}



/***********************************************************************
***��������: Screen_OperTimeOut
** ��������: ������ʱ����
***    ����: 
**   ����ֵ:   
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
				 case E_PICTURE17:   /*��緽ʽѡ�����*/
					 Delay10Ms(10);	
				   Screen_SetChgFlag(0);
					 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);  
			  	 break;
				 case E_PICTURE18:   /*�����������*/
					 Delay10Ms(1); 
				   Screen_SetChgFlag(0);	
				   Hmi_ShutDownKeyBoard(NUMBER_KEY_BOARD_CODE);
					 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);				   
					 break;
				 case E_PICTURE19:    /*����������*/
					 Delay10Ms(1);
				   Screen_SetChgFlag(0);	
				   Hmi_ShutDownKeyBoard(NUMBER_KEY_BOARD_CODE);
					 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);				   
					 break;
				 case E_PICTURE20:     /*ʱ���������*/
					 Delay10Ms(1);
				   Screen_SetChgFlag(0);	
 				   Hmi_ShutDownKeyBoard(NUMBER_KEY_BOARD_CODE);
					 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);			  
					 break;
				 case E_PICTURE33:    /*�û������������*/
					 Hmi_ShutDownKeyBoard(TEXT_KEY_BOARD_CODE);
					 Page_KeyPwReturn();
					 break;
				 case E_PICTURE34:  /*ˢ�������������*/
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
***��������: Screen_MessageDeal
** ��������: ������������Ϣ����
***    ����: 
**   ����ֵ:   
*************************************************************************/

const char * CheckString[] = \
{
  "׼����ʼ...",
	"���ڿ���������Դ...",
	"��������״̬...",
	"�ȴ�BHM����...",
	"�����Ե���...",
	"�����Ϲ���̽��...",
	"��Դģ�鿪ʼ���...",
	"�ȴ���Ե������...",
	"�ȴ�й�Ž���...",
	"�ȴ�����BRM����...",
	"�ȴ�����BCP����...",
	"�ȴ�����BRO����...",
	"��⳵�������Ƿ����...",
	"��Դģ���������...",
  "������ʼ���...",
	"VIN��̨���...",
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


/*��ȡ��ǰ��ҳ��ţ�0xffΪ����*/
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
	strcpy(temp,"��ӭʹ�� ");
	if(gunNo == AGUN_NO){
		if(BMS_24V == pRelayDataPtr->statu.bits.apow){
			strcat(temp,"��ǰ��ԴΪ24V");
		}
		else{
			strcat(temp,"��ǰ��ԴΪ12V");
		}
	}
	else{
		if(BMS_24V == pRelayDataPtr->statu.bits.bpow){
			strcat(temp,"��ǰ��ԴΪ24V");
		}
		else{
			strcat(temp,"��ǰ��ԴΪ12V");
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
	
	
	strcpy(temp,"��ӭʹ�� ");
	rtn = 0;
	if(gunNo == AGUN_NO){
		if((scnt[AGUN_NO]== 0) && (CC1_4V == BgRunData->gun->statu.bits.cc1stu) ){
			scnt[AGUN_NO] = 1;
			if(BMS_24V == pRelayDataPtr->statu.bits.apow){
				strcat(temp,"��ǰ��ԴΪ24V");
			}
			else{
				strcat(temp,"��ǰ��ԴΪ12V");
			}
		}
		else if(CC1_4V != BgRunData->gun->statu.bits.cc1stu){
			scnt[AGUN_NO] = 0;
			strcat(temp,"���ǹ������");
		}
		else{
			rtn = 1;
		}
	}
	else{
		if((scnt[BGUN_NO]== 0) && (CC1_4V == BgRunData->gun->statu.bits.cc1stu) ){
			scnt[BGUN_NO] = 1;
			if(BMS_24V == pRelayDataPtr->statu.bits.bpow){
				strcat(temp,"��ǰ��ԴΪ24V");
			}
			else{
				strcat(temp,"��ǰ��ԴΪ12V");
			}
		}
		else if(CC1_4V != BgRunData->gun->statu.bits.cc1stu){
			scnt[BGUN_NO] = 0;
			strcat(temp,"���ǹ������");
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

extern DEV_INPUT_TYPE *Check_GetInputDataptr(uint8 gunNo);
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
	 uint16 respos;    /*���Եص���*/
	 uint16 resneg;    /*���Եص���*/
	 
	 PARAM_DEV_TYPE *chgparaPtr = ChgData_GetDevParamPtr();
   CARD_INFO *ptrCardinfo = TskCard_GetCardInfPtr();
	 DEV_RELAY_TYPE *pRelayDataPtr = Relay_GetRelayDataPtr(0);	
	 DEV_INPUT_TYPE *pInputDataptr = Check_GetInputDataptr(AGUN_NO);
   if( RcvMsgFromQueue(&msg) == TRUE ) {

		 switch(msg.MsgType)
		 {
			 case MSG_RESQ_PASSWD:  /*������������*/
			   Screen_ShowMessage((char *)"  ",DGUS_PW_CARD_ADR);
				 Hmi_BuzzerBeep(10);
				 Screen_ShowMessage((char *)"  ",DGUS_PW_CARDSH_ADR);/*���������ʾ*/
			 	 Hmi_ChangePicture(E_PICTURE34);
				 Screen_StartDownCounter(E_PICTURE34,DEFAULT_COUNTDOWN_VALUE);			
				 break;
			 case MSG_VERIFY_RESULT:   /*��Ȩʧ��*/		
         gunNo = msg.MsgData[0];
         Screen_ShowMessage((char *)"  ",MsgWkstatuAdr[gunNo][0]);			 
				 switch(msg.MsgData[1])
				 {
					 case CARD_PASSWD_ERR:
						 test = "��������,�����ԣ�";						
						 break;
					 case CARD_ILLEGAL_ERR:
						 test = "����ʶ��";												 
						 break;
					 case CARD_NOMONEY_ERR:
						 test = "�����㣬���ֵ";						 					 
						 break;
					 case CARD_NOTIDLE_ERR:
						 test = "�ǿ���״̬�����Ժ�����";						
						 break;
					 case CARD_VIN_NO_MATCH:
						 test = "VIN�벻ƥ��";						
						 break;					 
					 default:
						 break;
				 }
				 sGlocalPara.startkeylasttm[gunNo] = GetSystemTick() - 31; 				 
				 
         Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][0]);				 
         Delay10Ms(200);         				 
				 Screen_ShowMessage((char *)"   ",TempAddr);	
				 break;
			 case MSG_WORK_STATU:  /*���׮״̬*/
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
						 case IDEL:    /*����*/ 
							 if( 0 == sGlocalPara.setparafg ) {										 
								 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
								 Screen_ShowMessage((char *)"  ",MsgWkstatuAdr[gunNo][0]);
								 if((GetSystemTick() - sGlocalPara.startkeylasttm[gunNo]) < 30*TIM_1S )      //add0604zyf
								 {
										START_PARAM  *PtrStartPara = ChgData_GetStartParaPtr(gunNo);
										if(PtrStartPara->startby == START_BY_PASSWD)
										{
											strcpy(temp,"�˺�������֤��...");
										}
										else{
											strcpy(temp,"��ӭʹ�� ");
										}
  							 }
								 else
								 {		
								   strcpy(temp,"��ӭʹ�� ");
								 }

									//test = "��ӭʹ�� ";
								 if(0 == power)
								 {
									  strcat(temp,"��ǰ��ԴΪ12V");
								 }
								 else if(1 == power)
								 {
									  strcat(temp,"��ǰ��ԴΪ24V");
								 }
									Screen_ShowMessage(temp,MsgWkstatuAdr[gunNo][0]);	
							 }							 
							// IDEL_ShowMessage(gunNo);	
							 break;
//						 case CHECK:   /*�Լ�*/
//							 tmp = msg.MsgData[2];
//							 sGlocalPara.setparafg = 0;
//							 Screen_ShowMessage((char *)"   ",MsgWkstatuAdr[gunNo][1]);	
//							 Screen_ShowMessage((char *)CheckString[tmp],MsgWkstatuAdr[gunNo][1]);					  
//							 Hmi_ChangePicture(E_PICTURE41);						
//							 break;
						case CHECK:   /*�Լ�*/
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
								 case E6_DCOUT:  //��Դģ�鿪ʼ���...
									 sprintf(paramMessage, "����:ISOV=%d.%dV,METV=%d.%dV", Vdc3/10, Vdc3%10, Vdc1/10, Vdc1%10); 
									 break;
						     case E7_WTISOF: //�ȴ���Ե������...
									 sprintf(paramMessage, "����:METV=%d.%dV,BHMV=%d.%dV", Vdc1/10, Vdc1%10, VDCBHM/10, VDCBHM%10);
									 break;
								 case E8_WTXFF:  //�ȴ�й�Ž���...
									 sprintf(paramMessage, "����:RESP=%dK��,RESN=%dK��", respos, resneg);
									 break;
								 case E12_CHKPARA: //�ȴ�����BRO����...
									 sprintf(paramMessage, "����:ISOV=%d.%dV,BCPV=%d.%dV", Vdc3/10, Vdc3%10, VDCBCP/10, VDCBCP%10);
									 break;
								 case E13_DCOUTADJ: //��Դģ���������...
									 sprintf(paramMessage, "����:ISOV=%d.%dV,METV=%d.%dV,BCPV=%d.%d", Vdc3/10, Vdc3%10, Vdc1/10, Vdc1%10, VDCBCP/10, VDCBCP%10);
									 break;
								 default:
									 sprintf(paramMessage, "����:ISOV=%d.%dV,METV=%d.%dV", Vdc3/10, Vdc3%10, Vdc1/10, Vdc1%10);
									 break;
							 }
							 
							 strcat(checkMessage, paramMessage);
							 Screen_ShowMessage((char *)"   ",MsgWkstatuAdr[gunNo][1]);
						   Screen_ShowMessage(checkMessage,MsgWkstatuAdr[gunNo][1]);
							 Hmi_ChangePicture(E_PICTURE41);						
							 break;

						 case FINISH:    /*������*/															 
							 Screen_ShowMessage((char *)"  ",MsgWkstatuAdr[gunNo][2]);									 
													 
							 if(0 == msg.MsgData[4])
							 { 
								  strcpy(temp,"δ����,�������ܰγ����ǹ��");
							 }
							 else if(1 == msg.MsgData[4])
							 {  
								  tmp = msg.MsgData[2] % REASOM_MUN;						   
							    strcpy((char *)temp,conReasonString[tmp]);	
									strcat(temp,",�ѽ���,���ǹ��λ");
							 }	
							 else if(2 == msg.MsgData[4])
							 {  
								  strcpy(temp,"δ����,ǹ������.....��");
							 }
							 Screen_ShowMessage(temp,MsgWkstatuAdr[gunNo][2]) ;			
							 Screen_OpenAccountCardDeal(gunNo);									 
							 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);						 				   
							 break;
						 case ACCOUNT:   /*����*/		
							 Screen_OpenAccountCardDeal(gunNo);
							 if(1 == msg.MsgData[4])
							 {
									strcpy((char *)temp,"������,��л����ʹ�ã�");								
							 }
							 else if(0 == msg.MsgData[4])
							 {												
									strcpy((char *)temp,"δ����,��ˢ������");         								 
							 }
//               else if(2 == msg.MsgData[4])
//							 {												
//									strcpy((char *)temp,"δ����,�ȴ���̨�˵�");         								 
//							 }							 
							Screen_ShowMessage(temp,MsgWkstatuAdr[gunNo][2]) ;	
							Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
							break;
						 case E_ERROR:   /*����*/																 
							 sGlocalPara.setparafg = 0;
//							 if(1 == pInputDataptr->statu.bits.stop)
//							 {
//								 errstr = Screen_GetErrSting(ECODE89_CHGJTST);
//							 }
//							 else
//							 {
									errstr = Screen_GetErrSting(msg.MsgData[3]);
//							 }
							 Screen_ShowMessage((char *)"               ",MsgWkstatuAdr[gunNo][2]);
							 Screen_ShowMessage(errstr->str,MsgWkstatuAdr[gunNo][3]);
							 strcpy(temp,"�벦�����绰��");
							 strcat(temp,(char *)chgparaPtr->telephone);		
							 Screen_ShowMessage(temp,MsgWkstatuAdr[gunNo][4]);						 
							 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);
							 break;
						 case CHARGING:  /*���ڳ��*/
							 ChargeShow(gunNo);
							 sGlocalPara.setparafg = 0;
							 Hmi_ChangePicture(gScrenCtrlPara.u8keepPic);		
							 break;


						 default:
							 break;
					 
				 }		 
				 break;
			 case MSG_UPPLUG_GUN:    /*�ȴ���ǹ  ��֮��*/
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
					strcpy(temp,"δ����,ǹ������.....��");
					Screen_ShowMessage(temp,MsgWkstatuAdr[gunNo][2]) ;	
				 break;
			 case MSG_WAIT_RECORD:    /*�ȴ�����   ��֮��*/
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
						strcpy(temp,"δ����,����ȴ���ʱ��");
					}
					else{
					  strcpy(temp,"δ����,�ȴ���̨�˵�.....��");
					}
					Screen_ShowMessage(temp,MsgWkstatuAdr[gunNo][2]) ;	
				 break;
			 case MSG_LOCKED_CARD:    /*������*/
//				 Screen_ShowMessage((char *)"  ",MsgWkstatuAdr[gunNo][0]);
//			 		test = "�Ѽ�⵽������֤��...";
//				Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][0]);
//					 Delay10Ms(100); 
//				  Screen_ShowMessage(" ",MsgWkstatuAdr[gunNo][0]);			 
				 break;
			 case MSG_LOCK_RESULT:  /*�������*/
//				 gunNo = msg.MsgData[0];
				 Screen_ShowMessage((char *)"  ",MsgWkstatuAdr[gunNo][0]);
			 
//------------------add0705zyf  patli 20190929
			   if( chgparaPtr->onlinecard == E_ONLINE_CARD ) 
				 {	 
					  if(ptrCardinfo->cardtype == E_SUPER_CARD)
						{
							test = "��⵽������Ա�������������...";
							Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][0]);
						}else if(ptrCardinfo->cardtype == E_OFFLINE_USER_CARD)
						{
							 test = "��⵽������ͨ�������������...";
							 Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][0]);
						
						}else{
							test = "��⵽�����û�������֤��...";
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
								test = "����ʧ��";
								Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][0]);
								Delay10Ms(200);
							}else if(1 == msg.MsgData[2]) {
								test = "δ��⵽���ǹ�������²�ǹ��";
								Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][0]);
								Delay10Ms(200);
							}
					 }
					 else if(RES_SUCC == msg.MsgData[1])
					 {							 
						 char temp1[10];
						 test = "�����ɹ�����ѡ����ģʽ";
						 Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][0]);
						 Hmi_BuzzerBeep(20);
						 Delay10Ms(200);
						 strcpy((char *)temp,"��ǰ����");		
						 Common_IntToFloatStr((uint8 *)temp1,ptrCardinfo->money,2);					 
						 strcat(temp,temp1);
						 strcat(temp," Ԫ");
						 Screen_ShowMessage(temp,DGUS_CHARWAY_PROADR);	
						 Hmi_ChangePicture(E_PICTURE17);
						 Screen_SetChgFlag(1);						 
						 Screen_StartDownCounter(E_PICTURE17,DEFAULT_COUNTDOWN_VALUE);    			 
					 }	
				 }				 
				 Delay10Ms(100); 
	//			 Screen_ShowMessage(" ",MsgWkstatuAdr[gunNo][0]);				
				 break;
			 case MSG_UNLOCK_RESULT:  /*�������*/
				 gunNo = msg.MsgData[0];
				 switch(msg.MsgData[1])
				 {
					 case 0:
						 test = "����ʧ�ܣ������ˢ��";						
						 break;
					 case 1:
						 test = "ˢ���ɹ�������ֹͣ...";            
						 break;
					 case 2:
						 test = "�����������뻻�����ԣ�";					   
						 break;
					 default:
							break;
				  }	
				  Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][2]);
					Delay10Ms(10); 
				  Screen_ShowMessage(test,MsgWkstatuAdr[gunNo][5]);
				  Delay10Ms(300); 					
				  break;
			 case MSG_SYSTEM_SETTING:  /*����ϵͳ����*/
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
***��������: TwoDimBarCode
** ��������: ��ά������
***    ����: 
**   ����ֵ:   
*************************************************************************/
#define GUN_NO_STR "0"
#define LT_TWODIMBARCODR_HEAD  "http://wx.aunice.com.cn/wx/#/charging/"
#define LT_TWODIMBARCODR_CHGZPLD_HEAD  "http://www.cityfastfilling.com/dskcserviceapp/charge/"
#define BIRD_TWODIM  "{\"gunNumber\":2,\"snNumber\":\"1801000360292770\",\"type\":\"1\"}"
#define BIRD_TWODIM1  "{\"gunNumber\":"
#define BIRD_TWODIM2  ",\"snNumber\":\""
#define BIRD_TWODIM3 "\",\"type\":\"1\"}"
#define HELI_LBX  "http://www.lucksion.com"

void TwoDimBarCode(void)
{
	char text[128 + 4] = { 0 };
	char ID_A[18] = { 0 };
  char ID_B[18] = { 0 };
	
	PARAM_DEV_TYPE *chgparaPtr = ChgData_GetDevParamPtr();
	PARAM_OPER_TYPE *devparaPtr = ChgData_GetRunParamPtr();
	PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();
	
	 /*���׮Aǹ�����ʾ*/
	Hmi_ClearReg(DGUS_NUMBER_ADR_A,1);
  memcpy(ID_A,chgparaPtr->chargeId,sizeof(chgparaPtr->chargeId));
  strcat(ID_A,"0");
	ID_A[sizeof(chgparaPtr->chargeId)+1] = 0;
  Screen_ShowMessage(ID_A,DGUS_NUMBER_ADR_A);
	
	/*���׮Bǹ�����ʾ*/
	if(GUNNUM_2 == devparaPtr->gunnum){
		Hmi_ClearReg(DGUS_NUMBER_ADR_B,1);
		memcpy(ID_B,chgparaPtr->chargeId,sizeof(chgparaPtr->chargeId));
		strcat(ID_B,"1");
		ID_B[sizeof(chgparaPtr->chargeId)+1] = 0;
		Screen_ShowMessage(ID_B,DGUS_NUMBER_ADR_B);
	}

	if (BMS_HELI == BackCOMM->agreetype){
		memset(text,0,sizeof(text));
		strcpy(text,HELI_LBX);
		Hmi_ShowText(DGUS_TWODIMBARCODE_ADDR_A,strlen(text),(uint8 *)text);
	}	
	else if (CONN_Aunice == BackCOMM->agreetype){
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
	
	else if (CONN_CheDian == BackCOMM->agreetype){  //�����Ƴ䣬������Э��
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

/*�����Դ������ʾ*/
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


/*��ǰ������ʾ����*/
void currfeeshow()
{
	  uint8 text[64] = {0};
		uint8 text1[16] = {0};

		REAL_BILL_DATA_ST *FeedDataPrt = GetFeeData(AGUN_NO);
    PARAM_DEV_TYPE *ptrDevPara = ChgData_GetDevParamPtr();
		PARAM_COMM_TYPE *BackCOMM = ChgData_GetCommParaPtr();    //patli 20190923 add
		
		strcpy((char *)text,"��ǰ���:");
		Common_IntToFloatStr(text1,FeedDataPrt->real.currfeerate ,4);
		strcat((char *)text,(char *)text1);
	  strcat((char *)text,"Ԫ/Kwh ");  //patli 20190923 add 1 byte space
		
		if (CONN_CHCP != BackCOMM->agreetype){	  //patli 20190923 add
			strcat((char *)text,"�����:");
			Common_IntToFloatStr(text1,FeedDataPrt->real.currSrvrate,4);
			strcat((char *)text,(char *)text1);
			strcat((char *)text,"Ԫ/Kwh "); 
		}
		
    if( ptrDevPara->onlinecard == E_ONLINE_CARD ) {		
       strcat((char *)text,"���߿�ģʽ"); 
		}
		else 
		{
		  strcat((char *)text,"���߿�ģʽ"); 
		}
		Hmi_ShowText(DGUS_CURRATR_ADR,strlen((char *)text),text);
    memset(text,0,sizeof(text));
    
  /*����汾����ʾ*/
	 //sprintf((char *)text,"%s.%s.%s",M_VERSION1,M_VERSION2,M_VERSION3);
#ifdef AUNICE_DEBUG
		sprintf((char *)text,"%s.%c%d.%.2d", M_VERSION1,VERSION_LETTER, MAIN_VERSION,SUB_VERSION);    //�����ܵĹ���ĵİ汾��
#else	  
   sprintf((char *)text,"%s.%s",M_VERSION1,M_VERSION3);    //�����ܵĹ���ĵİ汾��
#endif	 

	 Hmi_ShowText(DGUS_DEBUGTEXT_ADR+DGUS_DEBUGTEXT_OFFSET*3,strlen((char *)text),text);

		VoltShow(AGUN_NO);  //��Դ��ʾ
		VoltShow(BGUN_NO);  //��Դ��ʾ
		
}

const uint16 ChargeAdr[2][4] = {\
	{DGUS_CHARGE_ADDR_A,DGUS_CHARGID_ADR_A,DGUS_CHARGQAY_ADR_A,DGUS_CHARG_PROADR_A},
	{DGUS_CHARGE_ADDR_B,DGUS_CHARGID_ADR_B,DGUS_CHARGQAY_ADR_B,DGUS_CHARG_PROADR_B},
	
};

/*��������ʾ*/
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

		tmp32 = FeeDataPtr->bill.billmoney;   //���ѽ��
    Common_Change4Byte(&tmp32);
    data[cnt++] = tmp32 & 0xffff;		 
    data[cnt++] = tmp32 >> 16;
		
		tmp16 = FeeDataPtr->bill.chgsecs/60;   //���ʱ��
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;

	 	tmp16 = FeeDataPtr->bill.energy;        //�ѳ����
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
	 
	 	tmp16 = RunDataPrt->meter->volt;        //����ѹ
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
		tmp16 = RunDataPrt->meter->current;        //������
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
	 
	 	tmp16 = FeeDataPtr->real.lasttimes;          //ʣ��ʱ��
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
	 	 
    tmp16 = FeeDataPtr->bill.startsoc;        //��ʼSOC
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
	 
		tmp16 = FeeDataPtr->bill.soc;             //��ǰSOC
    Common_Change2Byte(&tmp16);
    data[cnt++] = tmp16;
		
	  tmp16 = Hmi_PackWriteReg(buf,(uint8*)&data,ChargeAdr[gunNo][0],CMD_WR_VAR,2*cnt);
	  Screen_UartWrite(buf,tmp16);

/************************��֮����������*****************************************/
    if(CONN_CHGBIRD == BackCOMM->agreetype){  //CONN_CHGBIRD
				cnt = 0;
				CHARGE_TYPE  *RunDataPrt_A = ChgData_GetRunDataPtr(AGUN_NO);
				CHARGE_TYPE  *RunDataPrt_B = ChgData_GetRunDataPtr(BGUN_NO);
				tmp32 = RunDataPrt_A->gun->i32temper[0]*100;   // ǹA�¶�
				Common_Change4Byte(&tmp32);
				data[cnt++] = tmp32 & 0xffff;		 
				data[cnt++] = tmp32 >> 16;

				tmp32 = RunDataPrt_B->gun->i32temper[0]*100;   //ǹB�¶�
				Common_Change4Byte(&tmp32);
				data[cnt++] = tmp32 & 0xffff;		 
				data[cnt++] = tmp32 >> 16;

				CDMOD_DATA *ptrModData = CdModData_GetDataPtr(0);
				tmp32 = ptrModData->temper*10 ;//׮�¶�   ģ�������¶�
				Common_Change4Byte(&tmp32);
				data[cnt++] = tmp32 & 0xffff;		 
				data[cnt++] = tmp32 >> 16;

				memset(&buf[0],0,sizeof(buf));
				tmp16 = Hmi_PackWriteReg(buf,(uint8*)&data,DGUS_TEMPER_A,CMD_WR_VAR,2*cnt);
				Screen_UartWrite(buf,tmp16);

				
				memset(&buf[0],0,sizeof(buf));                   //�����
				memset(text1,0,sizeof(text1));
				REAL_BILL_DATA_ST *FeedDataPrt = GetFeeData(gunNo);
				Common_IntToFloatStr(text1,FeedDataPrt->real.currfeerate/100 ,2);
				strcat((char *)buf,(char *)text1);
				strcat((char *)buf,"Ԫ/Kwh");
				Hmi_ShowText(DGUS_RATE_POWER_A+gunNo*0x100,strlen(buf),buf);


				memset(&buf[0],0,sizeof(buf));                   //�������
				memset(text1,0,sizeof(text1));
				Common_IntToFloatStr(text1,FeedDataPrt->real.currSrvrate/100 ,2);
				strcat((char *)buf,(char *)text1);
				strcat((char *)buf,"Ԫ/Kwh");
				Hmi_ShowText(DGUS_RATE_SERVER_A+gunNo*0x100,strlen(buf),buf);
						
				memset(&buf[0],0,sizeof(buf));                   //ռ׮����   ���ģ������
				memset(text1,0,sizeof(text1));
				Common_IntToFloatStr(text1,FeedDataPrt->real.parkfeerate/100 ,2);
				strcat((char *)buf,(char *)text1);
				strcat((char *)buf,"Ԫ/����");
				Hmi_ShowText(DGUS_RATE_OCCUPY_A+gunNo*0x100,strlen(buf),buf);
	  }
/*******************************************************************************/

		 
		/*��ʾ����*/		
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
		     
		/*��ʾ��緽ʽ*/
		tmp = RunDataPrt->logic->startby % CHARWAY_MUN;
		Hmi_ShowText(ChargeAdr[gunNo][2],strlen(concharway[tmp]),(uint8 *)concharway[tmp]);
		 
		/*��������ʽ��ʾ*/
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
		  Netshow();       /*�����־��ʾ*/
		 	currfeeshow();   /*���ʰ汾����ʾ*/  
	    switch(PageNo)
		  {
				case E_PICTURE0: 
					Screen_SetChgFlag(0);
					break;
				case E_PICTURE1:  //�����Ӵ����Ͽ�ʱ��������ż������
					IDEL_ShowGunState(BGUN_NO);
				case E_PICTURE47:	
					if(sTwoDim){
					  TwoDimBarCode(); /*��ά����ʾ*/
						sTwoDim = 1;
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


/*����״̬��ʾ*/
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


/*logo��ʾ*/
void LogoShow()
{
		Hmi_WriteOneVar(DGUS_LOGOSHOW_ADR,0);
}


/*������������*/
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
	LogoShow();      	/*logo��ʾ*/
	Delay10Ms(20);
	TwoDimBarCode(); /*��ά����ʾ*/
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
		
//		TaskRunTimePrint("Task_Screen_Main begin", OSPrioCur);

#if 1
		/*���մ��������ݴ���*/
		Screen_UploadDataDeal();
		
		/*ʱ��ˢ��*/
		Screen_ShowDateTime();
		
      	/*���������ȿ���*/
		Screen_LightControl();
		
		/*����������ˢ��*/ 	
     	Screen_ShowCyclic();
		
		/*��������Ϣ����*/
		Screen_MessageDeal();
		
		/*����������ʱ*/
		Screen_DecCountDown();
		
		/*��ʱ����*/
		Screen_OperTimeOut();		

#endif

//		TaskRunTimePrint("Task_Screen_Main end", OSPrioCur);

		Delay10Ms(APP_TASK_SCREEN_DELAY);  //patli 20200106 Delay5Ms(2); ����̫���̫����̫��������Ϣ���ղ���ȫ
			
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

