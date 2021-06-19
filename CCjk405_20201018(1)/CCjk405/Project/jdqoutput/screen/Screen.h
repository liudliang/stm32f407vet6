#ifndef _SCREEN_H
#define _SCREEN_H

#include "typedefs.h"
#include "ChgData.h"

#define INVALID_REG_ADR   0xffff
#define  DEFAULT_COUNTDOWN_VALUE    120       /*120s*/

#define TICK_TIMES_10S          (OS_TICKS_PER_SEC*10)           /*1000 *10ms  5s*/   

#define TICK_TIMES_2S           (OS_TICKS_PER_SEC*2)  

#define SHOW_DATESHOW_TIMES      (OS_TICKS_PER_SEC*1)         /*70*10���趨ˢ�¼Ĵ���������*/

#define SHOW_CYCLIC_TICK_TIMES   (OS_TICKS_PER_SEC*1)         /*100 * 10ms���趨ˢ�¼Ĵ���������*/
#define TICK_TIMES_1MIN          (OS_TICKS_PER_SEC*60)        /*1����*/



#define MAX_REG_NUM_ONCE_SEND    50          /*�趨һ��ˢ�µļĴ�������*/

#define KEY_ADRESS_DEF  0x2000

#define DGUS_LOGOSHOW_ADR            0x18E0   /*logo��ַ*/
#define DGUS_NET_ADDR                0x18F0   /*������ʶ��ַ��ʾ*/
#define DGUS_KEYSTOPA_ADDR                0x18FA   /*������ʶ��ַ��ʾ*/
#define DGUS_KEYSTOPB_ADDR                0x18FB   /*������ʶ��ַ��ʾ*/
#define  DGUS_TIMESHOW_ADR           0x22A0   /*����ʱ����ʾ*/

#define  DGUS_NUMBER_ADR_A           0x3000   /*Aǹ���*/
#define  DGUS_NUMBER_ADR_B           0x3010   /*Bǹ���*/
#define  DGUS_CURRATR_ADR            0x3050   /*��ǰ������ʾ*/

#define  DGUS_VOLTUAB_ADR_A           0x3100   /*��ʼ����Uab*/
#define  DGUS_VOLTUBC_ADR_A           0x3120   /*��ʼ����Ubc*/
#define  DGUS_VOLTUCA_ADR_A           0x3140   /*��ʼ����Uca*/
#define  DGUS_VOLTUAB_ADR_B           0x3160   /*��ʼ����Uab*/
#define  DGUS_VOLTUBC_ADR_B           0x3180   /*��ʼ����Ubc*/
#define  DGUS_VOLTUCA_ADR_B           0x31A0   /*��ʼ����Uca*/

#define DGUS_TWODIMBARCODE_ADDR_A    0x1900   /*Aǹ��ά���ַ*/
#define DGUS_TWODIMBARCODE_ADDR_B    0x1980   /*Bǹ��ά���ַ*/

#define DGUS_DEVID_ADDR              0x12B0   /*���׮id*/
#define DGUS_SYS_TIME_ADR             0x12C0   /*ϵͳʱ��*/

#define DGUS_VERSION_ADR             0x1748    /*����汾��ʾ*/

#define  DGUS_SETENERGY_ADR          0x10E0   /*�������*/
#define  DGUS_SETMONEY_ADR           0x10E2   /*�����Ǯ*/
#define  DGUS_AETTIME_ADR            0x10E4   /*����ʱ��*/

#define  DGUS_CHARGE_ADDR_A          0x1010   /*Aǹ���ڳ��������ʼ��ַ*/
#define  DGUS_CHARGID_ADR_A          0x1070   /*Aǹ���ڳ�翨��*/
#define  DGUS_CHARGQAY_ADR_A         0x1090   /*Aǹ���ڳ�緽ʽ*/

#define  DGUS_CHARGE_ADDR_B          0x1000   /*Bǹ���ڳ��������ʼ��ַ*/
#define  DGUS_CHARGID_ADR_B          0x1030   /*Bǹ���ڳ�翨��*/
#define  DGUS_CHARGQAY_ADR_B         0x1050   /*Bǹ���ڳ�緽ʽ*/

#define  DGUS_FAULT_TEXT_ADR_A1      0x2300   /*Aǹ������ʾ��Ϣ1*/
#define  DGUS_FAULT_TEXT_ADR_A2      0x2340   /*Aǹ������ʾ��Ϣ2*/

#define  DGUS_FAULT_TEXT_ADR_B1      0x2380   /*Bǹ������ʾ��Ϣ1*/
#define  DGUS_FAULT_TEXT_ADR_B2      0x23C0   /*Bǹ������ʾ��Ϣ2*/

#define  DGUS_CARDID_ADR_A           0x2400   /*Aǹ���㿨��        �ı�*/
#define  DGUS_START_ADR_A            0x2430   /*Aǹ���㿨��ʼ      �ı�*/
#define  DGUS_STOP_ADR_A             0x2460   /*Aǹ���㿨����      �ı�*/
#define  DGUS_REASON_ADR_A           0x2490   /*Aǹ����ԭ��*/
#define  DGUS_ENERGY_ADR_A           0x24C0   /*Aǹ���㿨ʹ�õ���  �ı�*/
#define  DGUS_MONEY_ADR_A            0x24F0   /*Aǹ���㿨ʹ�ý��  �ı�*/
#define  DGUS_TIME_ADR_A             0x2520   /*Aǹ���㿨ʹ��ʱ��  �ı�*/
#define  DGUS_WAY_ADR_A              0x2550   /*Aǹ���㿨��緽ʽ  �ı�*/
#define  DGUS_SOC_ADR_A              0x2580   /*Aǹ���㿨����SOC   �ı�*/
#define  DGUS_REMONEY_ADR_A          0x25B0   /*Aǹ���㿨���      �ı�*/

#define  DGUS_CARDID_ADR_B           0x2600   /*Bǹ���㿨��        �ı�*/
#define  DGUS_START_ADR_B            0x2630   /*Bǹ���㿨��ʼ      �ı�*/
#define  DGUS_STOP_ADR_B             0x2660   /*Bǹ���㿨����      �ı�*/
#define  DGUS_REASON_ADR_B           0x2690   /*Bǹ����ԭ��*/
#define  DGUS_ENERGY_ADR_B           0x26C0   /*Bǹ���㿨ʹ�õ���  �ı�*/
#define  DGUS_MONEY_ADR_B            0x26F0   /*Bǹ���㿨ʹ�ý��  �ı�*/
#define  DGUS_TIME_ADR_B             0x2720   /*Bǹ���㿨ʹ��ʱ��  �ı�*/
#define  DGUS_WAY_ADR_B              0x2750   /*Bǹ���㿨��緽ʽ  �ı�*/
#define  DGUS_SOC_ADR_B              0x2780   /*Bǹ���㿨����SOC   �ı�*/
#define  DGUS_REMONEY_ADR_B          0x27B0   /*Bǹ���㿨���      �ı�*/

#define DGUS_ACCOUNTS_OFFSET         0x30     /*�����ı�ƫ����*/
#define DGUS_REC_START_A             DGUS_CARDID_ADR_A       /*��һ����¼��ʼ��ַ����ַ�����й�������0x30*/
#define DGUS_REC_START_B             DGUS_CARDID_ADR_B

#define  DGUS_PW_SHOW_ADR            0x1800   /*���������ı���ַ*/
#define  DGUA_PWSH_ADR               0x1810   /*������ʾ�ı���ַ*/
#define  DGUS_COUNTDOWN_REGADR       0x1820   /*����ʱ��������ַ*/
#define  DGUS_PW_CARD_ADR            0x1830   /*�����������ı���ַ*/
#define  DGUS_PW_CARDSH_ADR          0x1840   /*��������ʾ�ı���ַ*/

#define  DGUS_IDLE_PROADR_A          0x2100   /*Aǹ����״̬��ʾ�ı�*/
#define  DGUS_IDLE_PROADR_B          0x2120   /*Bǹ����״̬��ʾ�ı�*/
#define  DGUS_CHARG_PROADR_A         0x2140   /*Aǹ���״̬��ʾ�ı�*/
#define  DGUS_CHARG_PROADR_B         0x2160   /*Bǹ���״̬��ʾ�ı�*/
#define  DGUS_ACCOUNT_PROADR_A       0x2180   /*Aǹ����״̬��ʾ�ı�*/
#define  DGUS_ACCOUNT_PROADR_B       0x21A0   /*Bǹ����״̬��ʾ�ı�*/
#define  DGUS_CHARWAY_PROADR         0x21C0   /*��緽ʽѡ�������ʾ�ı�*/
#define  DGUS_INPUT_PROADR           0x21E0   /*���������������ʾ�ı�*/
#define  DGUS_SETPA_PROADR           0x2200   /*���ò���������ʾ�ı�*/
#define  DGUS_SETSENPA_PROADR        0x2220   /*���ø߼�����������ʾ�ı�*/
#define  DGUS_RECORD_PROADR          0x2240   /*����¼������ʾ�ı�*/
#define  DGUS_SETMONEY_PROADR        0x2260   /*���÷�����ʾ�ı�*/
#define  DGUS_PASSWD_PROADR          0x2280   /*������֤������ʾ�ı�*/
#define  DGUS_CHAREADY_PROADR        0x22C0   /*׼�������ʾ�ı�*/

//--------add0529zyf
#define  DGUS_STARTMODE_GUNNO         0x3500   /*51ҳ  ǹ��*/
#define  DGUS_STARTMODE_CHARGACCOUNT  0x3510   /*51ҳ  ����˺�*/
#define  DGUS_STARTMODE_CHARGAPSW     0x3530   /*51ҳ  �������*/
#define  DGUS_STARTMODE_CHARGAPSWSHOW 0x3550   /*51ҳ  ���������ʾ*/


//������������
#define  DGUS_TEMPER_A                0x3300   /*Aǹ�¶� */
#define  DGUS_TEMPER_B                0x3302   /*Bǹ�¶� */
#define  DGUS_TEMPER_PLAT             0x3304   /*׮�¶� */
#define  DGUS_RATE_POWER_A            0x3310   /*Aǹ��������*/
#define  DGUS_RATE_SERVER_A           0x3330   /*Aǹ�������*/
#define  DGUS_RATE_OCCUPY_A           0x3350   /*Aǹռ׮����*/
#define  DGUS_RATE_POWER_B            0x3410   /*Bǹ��������*/
#define  DGUS_RATE_SERVER_B           0x3430   /*Bǹ�������*/
#define  DGUS_RATE_OCCUPY_B           0x3450   /*Bǹռ׮����*/
#define  DGUS_STOPCHARG_WAITOVER_A    0x3600   /*��֮��-������磬�ȴ���ǹ*/
#define  DGUS_STOPCHARG_WAITOVER_B    0x3620   /*��֮��-������磬�ȴ���ǹ*/
#define  DGUS_STARTCHARG_WAITING      0x3640   /*��֮��-������磬�ȴ���̨��֤*/




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
	 uint16 nsReg;     /*�����short*/
	 uint8  num;
	 uint16 nsData[1]; /*�����short*/
}ScrFrame_t;
#pragma pack()

#pragma pack(1)
typedef struct SCREEN_FRAME1
{
	 uint8 head1;
	 uint8 head2;
	 uint8 length;
	 uint8 cmd;
	 uint8 nReg;       /*�ڲ��Ĵ�����ַ ����һ���ֽ�*/
	 uint8 num;
	 uint16 nsData[1]; /*�����short*/
}ScrInRegFrame_t;
#pragma pack()


typedef struct KEY_FUNC1
{
	uint16  keyaddr;               /*������ַ*/
	uint16  code;                  /*����*/
	uint8   picNo;                 /*ͼƬ��ַ*/
	void (*func)(void);   /*��������*/
	uint16 (*keyclick)(void);          /*��������*/
}Keyfunc1_t;

typedef struct PAGE_DEF
{
	uint16  addr;         /*�����Ĵ�����ַ*/
	uint8   ntype;        /*��������*/
	uint8   bynum;        /*�����ֽ���*/
	uint8   dotnum;       /*С����λ��*/
	void *  var;          /*������ַ*/
}PageReg_t;

typedef struct UPLOAD_VAR
{
	uint16  addr;                  /*�����Ĵ�����ַ*/
	uint8   ntype;        				 /*��������*/
	uint8   bynum;        				 /*�����ֽ���*/
	uint8   dotnum;       				 /*С����λ��*/
	void *  var;          				 /*������ַ*/
	void (*func)(void);   /*��������*/
}UploadOper_t;


typedef enum
{
	DATA_VAR   = 0x01,  /*���ݱ���*/
	STRING_VAR = 0x02,  /*�ַ�������*/
	PASSWD_VAR = 0x08,  /*�������*/
	TIME_VAR   = 0x09,  /*ʱ�ӱ���*/
	
	INNER_REG  = 0xf0,  /*�ڲ��Ĵ�������*/
	
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
	NUMBER_KEY_BOARD_CODE = 0x01,      /*��DGUS���ö�Ӧ*/
	TEXT_KEY_BOARD_CODE   = 0x03,      /*��DGUS���ö�Ӧ*/
	SECT_KEY_TXTBOARD_CODE = 0x05,     /*���������ı����̶�Ӧ*/
	SECT_KEY_BOARD_CODE = 0x07,        /*�������ü��̶�Ӧ*/
	
	KEY_ENERGY_INPUT_CODE = 0x02,
	KEY_MONEY_INPUT_CODE = 0x04,
	KEY_TIME_INPUT_CODE = 0x06,
	
}E_KEYCTRL_CODE; 



typedef struct
{
	 uint8 start;
	 uint8 stop;
	 uint8  picNo;  /*ͼƬ���*/
	 int16 currVal;
	 uint32 inVal;
	 
}DownCount_t;


typedef struct
{
	 DEV_LOGIC_TYPE  charge;            /*������*/
	 uint8           PinCode[8];        /*����*/  
	 uint8           CardPinCode[8];
	 uint8           systime[14];        /*ϵͳʱ�� 20 21 06 19 13 16 00*/ 
}ScreenRcv_t;


typedef struct
{
	 uint8 statu;
	 uint8 value;
}ScreenTouch_t;


/*�������ȫ�ֱ�������ṹ*/
typedef struct 
{
	 uint8       Sysflag;           /*������־λ*/
	 uint8       RFreshFg;          /*ҳ������ˢ�±�־*/
	 DownCount_t DownCounter;       /*ҳ�浹��ʱʱ��*/
	 ScreenRcv_t rcv;
	 ScreenTouch_t light;            /*���ȿ���*/
	 uint8       errCode;            /*�豸������*/
	 uint8       keepPic;
	 uint8       setparafg;          /*���ý����־*/
	 uint8       gun_no;            /*ǹ��*/
	 uint8       ModUpDownKey;
	 uint8       bmskey;
	 uint8       powerkey;
	 uint32      lasttm;  /*�����ʱ��*/
	 uint32      bms24vtm[DEF_MAX_GUN_NO];  /*������Դ24V����ʱ��*/
	
	 uint32      startkeylasttm[2];  /*add0604����ȷ��������簴ť�����ʱ��*/  
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
	 E_PICTURE47 = 47,  //��ǹ����
	 E_PICTURE48 = 48,  //��ǹ���
	 E_PICTURE49 = 49,  //��ǹ����
	 E_PICTURE50 = 50,  //��ǹ����

	 E_PICTURE51 = 51,  //��翪����ʽ
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


/*��������������*/
void Task_Screen_Main(void *p_arg);

/*���������ں���*/
extern int32 Screen_UartWrite(uint8 *buf,	int32 size);

/*�����������ں���*/
extern int32 Screen_UartRead(uint8 *buf,uint16 size);

extern void Screen_SetChgFlag(uint8 fg);


#endif

