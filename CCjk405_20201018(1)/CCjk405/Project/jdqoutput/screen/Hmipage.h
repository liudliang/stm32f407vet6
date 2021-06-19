#ifndef _HMIPAGE_H
#define _HMIPAGE_H


/*BMS参数变量*/
#define DGUS_BMSDETAIL_ADR         0x1100     /*BMS充电详情*/

#define DGUS_BMSDETAIL_TEXT_ADR    0x11A0     /*BMS充电详情文本*/
#define DGUS_BMSDETAIL_OFFSET      0x20      

/*参数设置变量*/
#define DGUS_REMOTEIP_ADR    0x1200   /*远程IP地址*/
#define DGUS_LOCALIP_ADR     0x1220   /*本地IP地址 */
#define DGUS_NETMASK_ADR     0x1230   /*子网掩码地址*/
#define DGUS_NETGATE_ADR     0x1240   /*网关地址*/
#define DGUS_MAC_ADR     		 0x1250   /*mac地址*/
#define DGUS_SSID_ADR     	 0x1280   /*SSID地址*/
#define DGUS_PASSWORD_ADR    0x1290   /*密码地址*/
#define DGUS_ID_ADR          0x12B0   /*本桩ID*/

#define DGUS_SYSTIME_ADR          0x12C0   /*系统时间*/

/*数据变量*/
#define DGUS_REMOTEPORT_ADR  0x1210   /*端口号地址*/
#define DGUS_DHCPEN_ADR      0x1260   /*DHCP使能*/
#define DGUS_ETHEN_ADR       0x1270   /*网络使能*/
#define DGUS_NETWAY_ADR      0x12A0   /*联网方式*/

#define DGUS_PARA_START_ADR       0x1300   /*参数设置开始地址*/

/*高级参数设置变量*/
#define DGUS_SENRA_START_ADR1     0x1310  /*高级参数设置开始地址1*/
#define DGUS_SENRA_START_ADR2     0x1340  /*高级参数设置开始地址2*/

#define DGUS_PAPA_PASSWORD_ADR    0x1400  /*参数密码设置地址*/
#define DGUS_PAPA_PASSWORD_ADR1   0x1410  /*确认密码*/
#define DGUS_PAPA_PHONE_ADR       0x1420  /*参数服务电话*/
#define DGUS_KEYCODE_ADR          0x1430  /*秘钥*/

/*费率设置参数*/
#define DGUS_RATE_START_ADR_A1    0x1560  /*A枪费率设置开始地址*/
#define DGUS_RATE_START_ADR_A2    0x1570  /*A枪费率设置开始地址*/

#define DGUS_RATE_START_ADR_B1    0x15A0  /*B枪费率设置开始地址*/
#define DGUS_RATE_START_ADR_B2    0x15B0  /*B枪费率设置开始地址*/

/*调试参数变量*/
#define DGUS_DEBUG_ADR1             0x1600     /*调试界面1地址*/
#define DGUS_DEBUG_ADR2             0x1610     /*调试界面2地址*/
#define DGUS_DEBUGTEXT_ADR          0x1700     /*调试界面版本信息文本地址*/
#define DGUS_DEBUGTEXT_OFFSET       0x18

/*模块参数变量*/
#define DGUS_MODEDETAIL_ADDR1       0x1680   /*模块详情界面地址1*/
#define DGUS_MODEDETAIL_OFFSET      0x10
#define DGUS_MODEDETAIL_ADDR2       0x1790   /*模块详情界面地址2*/
#define DGUS_MODEDETAIL_ADDR3       0x17A0   /*模块详情界面地址3*/

/*充电记录变量*/
#define DGUS_REC_TEXTSTART_ADR      0x2800    /*充电记录文本起始地址*/ 
#define DGUS_REC_TEXTSTART_OFFSET   0x20      /*充电记录文本偏移地址*/
#define DGUS_REC_DATASTART_ADR      0x1500    /*充电记录数据起始地址*/
#define DGUS_REC_DATASTART_OFFSET   0x02      /*充电记录数据偏移地址*/
#define DGUS_REC_GUNMUN_ADR         0x2E00    /*充电记录枪号文本地址*/
#define DGUS_REC_GUNMUN_OFFSET      0x08      /*充电记录枪号文本偏移地址*/
#define DGUS_REC_ENERGY_ADR         0x1540
#define DGUS_REC_ENERGY_OFFSET      0x02  

#define DGUS_REC_PAGENO_ADR         0x2F00    /*充电记录页码*/

#define DGUS_AUPOWCHOOSE_ADR        0x3080   /*辅源界面文本提示*/

/*开启充电方式*/
#define DGUS_STARTMODE_ACCOUNT_ADR      0x3210      /*密码充电充电账号文本偏移地址*/
#define DGUS_STARTMODE_PASSWD_ADR       0x3240      /*密码充电账号密码文本偏移地址*/
 



/*开始充电按钮A枪*/
extern void Page_StartCharge_A(void);
/*开始充电按钮B枪*/
extern void Page_StartCharge_B(void);
/*系统设置*/
extern void Page_SysSet(void);

/*充电详情A枪*/
extern void Page_BMSDetails_A(void);
/*充电详情B枪*/
extern void Page_BMSDetails_B(void);
/*VIN充电停止按钮*/
extern void Page_VINStop_A(void);
extern void Page_VINStop_B(void);

/*设置金额充电响应函数*/
extern void Page_KeySetMoneyChargeMode(void);
/*设置电能充电模式*/
extern void Page_KeySetEnergyChargeMode(void);
/*设置时间充电模式*/
extern void Page_KeySetTimesChargveMode(void);
/*设置自动充电模式*/
extern void Page_KeySetAutoChargeMode(void);
/*退出充电方式选择页面响应函数*/
extern void  Page_KeyExitChoose(void);
/*设置金额 | 电量|时间|自动 模式后 处理函数*/
extern void Page_VarStartCharging(void);

/*设置按电量充电确定按钮*/
extern void Page_KeyEnergyEnter(void);
/*设定金额确定*/
extern void Page_KeyMoneyEnter(void);
 /*设定时间确定*/
extern void Page_KeyTimeEnter(void);
/*金额、电量、时间、自动充电返回按钮*/
extern void Page_KeyChargeModeSetReturn(void);

/*BMS详情返回*/
extern void Page_KeyBMSReturn(void);

/*参数设置返回*/
extern void Page_Sysreturn(void);
/*系统设置按钮*/
extern void Page_SystemSet(void);
/*充电记录按钮*/
extern void Page_RechargeRecord(void);
/*调试界面*/
extern void Page_Debug(void);

/*参数设置按钮1*/
extern void Page_SystemSet1(void);
/*费率设置*/
extern void Page_RatesSet(void);

/*时间设置*/
extern void Page_KeySetTime(void);
/*第一页参数读取按钮*/
extern void Page_KeySystem1Read(void);
/*第一页参数读设置按钮*/
extern void Page_KeySystem1Set(void);
/*第一页参数返回按钮*/
extern void Page_KeySystem1Return(void);
/*高级参数设置按键*/
extern void Page_KeySeniorSystem(void);

/*高级参数读取按钮*/
extern void Page_KeySystem2Read(void);
/*高级参数设置按钮*/
extern void Page_KeySystem2Set(void);
/*高级参数返回按钮*/
extern void Page_KeySystem2Return(void);
/*高级参数设置下一页*/
extern void Page_KeySystemNext(void);
/*擦除记录*/
extern void Page_KeyClear(void);
/*系统重启*/
extern void Page_KeySyRestart(void);

/*高级参数设置2读按钮*/
extern void Page_KeySystem3Read(void);
/*高级参数设置2设置按钮*/
extern void Page_KeySystem3Set(void);
/*高级参数设置2返回*/
extern void Page_KeySystem3Return(void);

/*充电记录上一页按钮*/
extern void Page_KeyRecordUpPage(void);
/*充电记录下一页按钮*/
extern void Page_KeyRecordDownPage(void);
/*充电记录返回*/
extern void Page_KeyRecordReturn(void);

/*费率读取按钮*/
extern void Page_KeyRatesRead_A(void);
/*费率返设置按钮*/
extern void Page_KeyRatesSet_A(void);
/*费率返回按钮*/
extern void Page_KeyRatesReturn(void);

/*调试界面下一页按钮*/
extern void Page_KeyDebugNext(void);
/*模块详情*/
extern void Page_KeyModeDetail(void);
/*调试界面返回*/
extern void Page_KeyDebugReturn(void);

/*调试界面上一页*/
extern void Page_KeyDebugPrev(void);

/*模块详情上一页*/
extern void Page_KeyModePrev(void);
/*模块详情下一页*/
extern void Page_KeyModeNext(void);
/*模块详情返回*/
void Page_KeyModeReturn(void);

/*密码输入确认按键响应函数*/
extern void Page_KeyPassWdEnter(void);
/*密码页面返回按钮*/
extern void Page_KeyPwReturn(void);

/*卡密码确认*/
extern void Page_KeyPwCardEnter(void);
/*卡密码返回*/
extern void Page_KeyPwCardReturn(void);

/*A枪费率*/
extern void Page_KeyRateChoose_A(void);
/*B枪费率*/
extern void Page_KeyRateChoose_B(void);
/*费率选择界面返回*/
extern void Page_KeyRateChoose_Return(void);

/*费率读取按钮*/
extern void Page_KeyRatesRead_B(void);
/*费率返设置按钮*/
extern void Page_KeyRatesSet_B(void);

/*用户密码输入界面 显示 **** */
extern void Page_KeyPwparam(void);
/*卡密码输入界面*显示 ******/
extern void Page_UploadPassWd(void);
/*获取是否有触摸*/
extern void Page_RegIStouch(void);
/*获取设置系统的时间*/
extern uint8_t Page_KeySetSystime(void);

/*参数信息写*/
extern uint8 Page_ParaWrite1(uint8 *ndata);
/*高级参数信息写*/
extern uint8 Page_ParaWrite2(uint8 *ndata);
/*参数信息读*/
extern uint8 Page_ParaRead1(void);
/*调试界面1参数显示*/
extern void Page_DebugPage(void);
/*调试界面2参数显示*/
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

