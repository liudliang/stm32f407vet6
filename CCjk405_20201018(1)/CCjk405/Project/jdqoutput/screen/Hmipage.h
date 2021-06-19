#ifndef _HMIPAGE_H
#define _HMIPAGE_H


/*BMS��������*/
#define DGUS_BMSDETAIL_ADR         0x1100     /*BMS�������*/

#define DGUS_BMSDETAIL_TEXT_ADR    0x11A0     /*BMS��������ı�*/
#define DGUS_BMSDETAIL_OFFSET      0x20      

/*�������ñ���*/
#define DGUS_REMOTEIP_ADR    0x1200   /*Զ��IP��ַ*/
#define DGUS_LOCALIP_ADR     0x1220   /*����IP��ַ */
#define DGUS_NETMASK_ADR     0x1230   /*���������ַ*/
#define DGUS_NETGATE_ADR     0x1240   /*���ص�ַ*/
#define DGUS_MAC_ADR     		 0x1250   /*mac��ַ*/
#define DGUS_SSID_ADR     	 0x1280   /*SSID��ַ*/
#define DGUS_PASSWORD_ADR    0x1290   /*�����ַ*/
#define DGUS_ID_ADR          0x12B0   /*��׮ID*/

#define DGUS_SYSTIME_ADR          0x12C0   /*ϵͳʱ��*/

/*���ݱ���*/
#define DGUS_REMOTEPORT_ADR  0x1210   /*�˿ںŵ�ַ*/
#define DGUS_DHCPEN_ADR      0x1260   /*DHCPʹ��*/
#define DGUS_ETHEN_ADR       0x1270   /*����ʹ��*/
#define DGUS_NETWAY_ADR      0x12A0   /*������ʽ*/

#define DGUS_PARA_START_ADR       0x1300   /*�������ÿ�ʼ��ַ*/

/*�߼��������ñ���*/
#define DGUS_SENRA_START_ADR1     0x1310  /*�߼��������ÿ�ʼ��ַ1*/
#define DGUS_SENRA_START_ADR2     0x1340  /*�߼��������ÿ�ʼ��ַ2*/

#define DGUS_PAPA_PASSWORD_ADR    0x1400  /*�����������õ�ַ*/
#define DGUS_PAPA_PASSWORD_ADR1   0x1410  /*ȷ������*/
#define DGUS_PAPA_PHONE_ADR       0x1420  /*��������绰*/
#define DGUS_KEYCODE_ADR          0x1430  /*��Կ*/

/*�������ò���*/
#define DGUS_RATE_START_ADR_A1    0x1560  /*Aǹ�������ÿ�ʼ��ַ*/
#define DGUS_RATE_START_ADR_A2    0x1570  /*Aǹ�������ÿ�ʼ��ַ*/

#define DGUS_RATE_START_ADR_B1    0x15A0  /*Bǹ�������ÿ�ʼ��ַ*/
#define DGUS_RATE_START_ADR_B2    0x15B0  /*Bǹ�������ÿ�ʼ��ַ*/

/*���Բ�������*/
#define DGUS_DEBUG_ADR1             0x1600     /*���Խ���1��ַ*/
#define DGUS_DEBUG_ADR2             0x1610     /*���Խ���2��ַ*/
#define DGUS_DEBUGTEXT_ADR          0x1700     /*���Խ���汾��Ϣ�ı���ַ*/
#define DGUS_DEBUGTEXT_OFFSET       0x18

/*ģ���������*/
#define DGUS_MODEDETAIL_ADDR1       0x1680   /*ģ����������ַ1*/
#define DGUS_MODEDETAIL_OFFSET      0x10
#define DGUS_MODEDETAIL_ADDR2       0x1790   /*ģ����������ַ2*/
#define DGUS_MODEDETAIL_ADDR3       0x17A0   /*ģ����������ַ3*/

/*����¼����*/
#define DGUS_REC_TEXTSTART_ADR      0x2800    /*����¼�ı���ʼ��ַ*/ 
#define DGUS_REC_TEXTSTART_OFFSET   0x20      /*����¼�ı�ƫ�Ƶ�ַ*/
#define DGUS_REC_DATASTART_ADR      0x1500    /*����¼������ʼ��ַ*/
#define DGUS_REC_DATASTART_OFFSET   0x02      /*����¼����ƫ�Ƶ�ַ*/
#define DGUS_REC_GUNMUN_ADR         0x2E00    /*����¼ǹ���ı���ַ*/
#define DGUS_REC_GUNMUN_OFFSET      0x08      /*����¼ǹ���ı�ƫ�Ƶ�ַ*/
#define DGUS_REC_ENERGY_ADR         0x1540
#define DGUS_REC_ENERGY_OFFSET      0x02  

#define DGUS_REC_PAGENO_ADR         0x2F00    /*����¼ҳ��*/

#define DGUS_AUPOWCHOOSE_ADR        0x3080   /*��Դ�����ı���ʾ*/

/*������緽ʽ*/
#define DGUS_STARTMODE_ACCOUNT_ADR      0x3210      /*���������˺��ı�ƫ�Ƶ�ַ*/
#define DGUS_STARTMODE_PASSWD_ADR       0x3240      /*�������˺������ı�ƫ�Ƶ�ַ*/
 



/*��ʼ��簴ťAǹ*/
extern void Page_StartCharge_A(void);
/*��ʼ��簴ťBǹ*/
extern void Page_StartCharge_B(void);
/*ϵͳ����*/
extern void Page_SysSet(void);

/*�������Aǹ*/
extern void Page_BMSDetails_A(void);
/*�������Bǹ*/
extern void Page_BMSDetails_B(void);
/*VIN���ֹͣ��ť*/
extern void Page_VINStop_A(void);
extern void Page_VINStop_B(void);

/*���ý������Ӧ����*/
extern void Page_KeySetMoneyChargeMode(void);
/*���õ��ܳ��ģʽ*/
extern void Page_KeySetEnergyChargeMode(void);
/*����ʱ����ģʽ*/
extern void Page_KeySetTimesChargveMode(void);
/*�����Զ����ģʽ*/
extern void Page_KeySetAutoChargeMode(void);
/*�˳���緽ʽѡ��ҳ����Ӧ����*/
extern void  Page_KeyExitChoose(void);
/*���ý�� | ����|ʱ��|�Զ� ģʽ�� ������*/
extern void Page_VarStartCharging(void);

/*���ð��������ȷ����ť*/
extern void Page_KeyEnergyEnter(void);
/*�趨���ȷ��*/
extern void Page_KeyMoneyEnter(void);
 /*�趨ʱ��ȷ��*/
extern void Page_KeyTimeEnter(void);
/*��������ʱ�䡢�Զ���緵�ذ�ť*/
extern void Page_KeyChargeModeSetReturn(void);

/*BMS���鷵��*/
extern void Page_KeyBMSReturn(void);

/*�������÷���*/
extern void Page_Sysreturn(void);
/*ϵͳ���ð�ť*/
extern void Page_SystemSet(void);
/*����¼��ť*/
extern void Page_RechargeRecord(void);
/*���Խ���*/
extern void Page_Debug(void);

/*�������ð�ť1*/
extern void Page_SystemSet1(void);
/*��������*/
extern void Page_RatesSet(void);

/*ʱ������*/
extern void Page_KeySetTime(void);
/*��һҳ������ȡ��ť*/
extern void Page_KeySystem1Read(void);
/*��һҳ���������ð�ť*/
extern void Page_KeySystem1Set(void);
/*��һҳ�������ذ�ť*/
extern void Page_KeySystem1Return(void);
/*�߼��������ð���*/
extern void Page_KeySeniorSystem(void);

/*�߼�������ȡ��ť*/
extern void Page_KeySystem2Read(void);
/*�߼��������ð�ť*/
extern void Page_KeySystem2Set(void);
/*�߼��������ذ�ť*/
extern void Page_KeySystem2Return(void);
/*�߼�����������һҳ*/
extern void Page_KeySystemNext(void);
/*������¼*/
extern void Page_KeyClear(void);
/*ϵͳ����*/
extern void Page_KeySyRestart(void);

/*�߼���������2����ť*/
extern void Page_KeySystem3Read(void);
/*�߼���������2���ð�ť*/
extern void Page_KeySystem3Set(void);
/*�߼���������2����*/
extern void Page_KeySystem3Return(void);

/*����¼��һҳ��ť*/
extern void Page_KeyRecordUpPage(void);
/*����¼��һҳ��ť*/
extern void Page_KeyRecordDownPage(void);
/*����¼����*/
extern void Page_KeyRecordReturn(void);

/*���ʶ�ȡ��ť*/
extern void Page_KeyRatesRead_A(void);
/*���ʷ����ð�ť*/
extern void Page_KeyRatesSet_A(void);
/*���ʷ��ذ�ť*/
extern void Page_KeyRatesReturn(void);

/*���Խ�����һҳ��ť*/
extern void Page_KeyDebugNext(void);
/*ģ������*/
extern void Page_KeyModeDetail(void);
/*���Խ��淵��*/
extern void Page_KeyDebugReturn(void);

/*���Խ�����һҳ*/
extern void Page_KeyDebugPrev(void);

/*ģ��������һҳ*/
extern void Page_KeyModePrev(void);
/*ģ��������һҳ*/
extern void Page_KeyModeNext(void);
/*ģ�����鷵��*/
void Page_KeyModeReturn(void);

/*��������ȷ�ϰ�����Ӧ����*/
extern void Page_KeyPassWdEnter(void);
/*����ҳ�淵�ذ�ť*/
extern void Page_KeyPwReturn(void);

/*������ȷ��*/
extern void Page_KeyPwCardEnter(void);
/*�����뷵��*/
extern void Page_KeyPwCardReturn(void);

/*Aǹ����*/
extern void Page_KeyRateChoose_A(void);
/*Bǹ����*/
extern void Page_KeyRateChoose_B(void);
/*����ѡ����淵��*/
extern void Page_KeyRateChoose_Return(void);

/*���ʶ�ȡ��ť*/
extern void Page_KeyRatesRead_B(void);
/*���ʷ����ð�ť*/
extern void Page_KeyRatesSet_B(void);

/*�û������������ ��ʾ **** */
extern void Page_KeyPwparam(void);
/*�������������*��ʾ ******/
extern void Page_UploadPassWd(void);
/*��ȡ�Ƿ��д���*/
extern void Page_RegIStouch(void);
/*��ȡ����ϵͳ��ʱ��*/
extern uint8_t Page_KeySetSystime(void);

/*������Ϣд*/
extern uint8 Page_ParaWrite1(uint8 *ndata);
/*�߼�������Ϣд*/
extern uint8 Page_ParaWrite2(uint8 *ndata);
/*������Ϣ��*/
extern uint8 Page_ParaRead1(void);
/*���Խ���1������ʾ*/
extern void Page_DebugPage(void);
/*���Խ���2������ʾ*/
extern void Page_DebugPage2(void);

extern void Page_ModeDetail(uint8 code);

extern void  BmsDetails(uint8 gunNo);

extern void Page_KeyAuciliaryPow_A(void);

extern void Page_KeyAuciliaryPow_B(void);

extern void Page_KeyAuPowChoose12(void);

extern void Page_KeyAuPowChoose24(void);

extern void Page_RtnPowChoose12(uint8 gunNo);


extern void Page_KeyAuPowReturn(void);

extern void Page_KeyAuPowConfirm(void);

extern void Page_KeyAuPowCancel(void);

extern void Page_KeyVINStart_A(void);

extern void Page_KeyVINStart_B(void);

extern void Page_KeyPasswdModeEnter(void);

extern void Page_KeyStartModeReturn(void);

extern void Page_KeyChargPwdReturn(void);

extern void Page_KeyChargPwdEnter(void);
#endif

