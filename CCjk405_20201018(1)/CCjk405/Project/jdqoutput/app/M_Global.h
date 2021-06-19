/***********************************************************************
�ļ����ƣ�
��    �ܣ�
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
 
#ifndef _M_GLOBAL_H_
#define _M_GLOBAL_H_


//��ӱ�Ҫ��ͷ�ļ�
#include "M_Type_P.h"
#include "M_CMSIS_P.h" 

//#include "typedefs.h"

//#define VER2_1_0
/***********************ϵͳ����ѡ��********************************************************
*****ע�� ֻ��ʹ�ã���ֹ�޸�
******************************************************************************/  
#define ONLINE_CARD            		1                 /*֧�����߿�����Ҫ��̨��֤*/
#define OFFLINE_CARD           		0                 /*���߿�*/

#define _43SCREEN              		1                 /*4.3����*/
#define _07SCREEN              		2                 /*7����*/

#define GUNSTART_SUPPORT       		0                 /*��ǹ�����֧��*/
#define GUNSTART_DONTSUPPORT   		1                 /*��ǹ����� ��֧��*/
//#define GUNSTART_VIN_SUPPORT   		2                 /*��ǹ����� ��VIN�˺�*/

#define _2DMBARCODE_SUPPORT    		1                 /*֧�ֶ�ά��*/
#define _2DMBARCODE_DONTSUPPORT 	0                 /*��֧�ֶ�ά��*/

#define WDG_ENABLE                8
#define WDG_DISABLE               9




/*****************************ע��****************************************
***������������ѡ�
***����������ɺ�����ؽ� PRODUCTS_LEVEL ���Լ�������ΪLEAVE_FACTORY_TEST
**************************************************************************/

#define DEBUG_VERSION           	5                /*���Լ��������԰汾*/
#define RELEASE_VERSION     		6                /*���������汾*/
#define FACTORY_UPGRADE_VERSION     7                /*���������汾*/

#define PRODUCTS_LEVEL  		DEBUG_VERSION                  // DEBUG_VERSION , RELEASE_VERSION, FACTORY_UPGRADE_VERSION


//#define USE_STDPERIPH_DRIVER
//#define STM32F4XX
//#define USE_USB_OTG_FS
//#define USE_STM324xG_EVAL


/*************************************************************************************************************************
* �汾����
* ÿ����һ���汾����Ҫ�����ּ�1
**************************************************************************************************************************
*�汾˵�������ڱ������ж��壩
*VER2_0_1_T1      //���ݲ泵���׮��Ŀ����ǹ����汾
*VER2_0_1         //��׼�汾
*V4071 A04�汾    20190425  1���޸��˵�ģ�������������Ƶ�50A 2��BMS�����������׮����������ǿ�Ʊ������Ϊ׮��������
*V4071 A05�汾    20190520  1���޸��˴������ȼ�������̨�������ȼ��ŵ����
*V4071 A06�汾    20190529  1��˫ǹ���ʱ�����ѹ��󣬺�ʱ�ε�����ʱ����ǰ��ʱ�εĵ��������ֵ�ǰʱ�κű���gBackSectNo�ĳ�gBackSectNo[DEF_MAX_GUN_NO]
*V4071 A07�汾    20190625  1�������˹���BMSЭ��BCP��������ֵ��BCL���������ֵ��ͻ�����⡣ȡ���߼����Сֵ��Ϊ���������
*V4071 A08�汾    20190716  1����������ע�͵�USB���֣������BOOT��������ͻ��
                            2�����ڲ��Ҳ��ֵĳ�ʱʱ��Ķ�(stm32f4x7_eth.h��)������Ϳ��Ź���ͻ��
**************************************************************************************************************************/
#include "stm32f4xx.h"

	
  /**��Ե��������********************************************************************************/	
	#define  ISO_SINGLE     0  /*��·��Ե���ģ��  ��ɫ*/
	#define  WBISO_SINGLE   1  /*WB9162��·��Ե���ģ�� ��ɫ*/
	#define  ISO_AUNICE     2  /*��˾�Բ�ģ��*/
	
	#define  ISO_TYPE      ISO_AUNICE     //ISO_AUNICE//WBISO_SINGLE                   /*��Ե����ѡ��*/
  /****************************************************************************************/	
  /*��boot�ϲ�0x08020000��U������0x08080000*/
	#define SUPPORT_GUN_START   	 GUNSTART_SUPPORT//GUNSTART_DONTSUPPORT /*ǹ����*/
	
	#define BOOT_BASE_ADDRESS	 				0x08000000		//boot ����������ʼ��ַ
	#define FACTORY_SETTING_BASE_ADDRESS		0x08060000		//���� ����������ʼ��ַ
	#define FACTORY_UPGRADE_BASE_ADDRESS	 	0x08020000		//���� ����������ʼ��ַ
	#define FACTORY_RELEASE_BASE_ADDRESS		0x08080000		//���� ����������ʼ��ַ	
	#define FACTORY_RELEASE_END_ADDRESS			0x080E0000		//���� ���������������ʼ��ַ 

  	#if (PRODUCTS_LEVEL == DEBUG_VERSION)					//���԰汾	
//	#define BMSTEST											//ʹ��BMSģ����
	#define M_VERSION1 "D4052"         	     //1ΪBMSTEST����1����BMSTEST           	/*�汾��׺��û��Ϊ�� .t1*/  
	#define FLASH_BASE_LEAVE_FACTORY     BOOT_BASE_ADDRESS
	#define VERSION_LETTER		0x54  //  'T'
	#define TASK_RUN_TIME_PRINTF
	#define TASK_RUN_TIME_PRINTF_NO_COMM	
//	#define TASK_MESSAGE_PRINT

	#warning "DEBUG_VERSION"
		
  	#elif (PRODUCTS_LEVEL == FACTORY_UPGRADE_VERSION)		 /*���������汾ģʶ*/
	#define M_VERSION1 "D4071"								/*�汾��׺��û��Ϊ�� .t1*/
	#define FLASH_BASE_LEAVE_FACTORY	 BOOT_BASE_ADDRESS  //patli tmp    //BOOT_BASE_ADDRESS  //FACTORY_UPGRADE_BASE_ADDRESS
	#define VERSION_LETTER		0x55  //  'U'
	#warning "FACTORY_UPGRADE_VERSION"
	
  	#elif (PRODUCTS_LEVEL == RELEASE_VERSION)                /*��������ģʽ����U����������������*/
	#define FLASH_BASE_LEAVE_FACTORY     FACTORY_RELEASE_BASE_ADDRESS
	#warning "RELEASE_VERSION"
	#define TASK_RUN_TIME_PRINTF
	#define TASK_RUN_TIME_PRINTF_NO_COMM
		
	#if ISO_TYPE == WBISO_SINGLE
		#define M_VERSION1 "D407VAW_CP"                 
	#endif
		
	#if ISO_TYPE == ISO_AUNICE
			#define M_VERSION1 "D407VAA_CP" 
	#endif
		
	#define VERSION_LETTER		0x56  //  'V'
		
	#endif
	
#define WATCHDOG_ENABLE 	   WDG_ENABLE		   //WDG_DISABLE
	
//	#if (ISO_TYPE == ISO_AUNICE )		
//		#define M_VERSION2 "10"
//	#elif (ISO_TYPE == WBISO_SINGLE )
//	  #define M_VERSION2 "11"
//	#else
//	  #define M_VERSION2 "13"
//	#endif

//#define AN_ETH_IRQ           //��̫���жϽ������ݿ��ؿ���


//#define AN_ETH_TASK           //��̫�����񿪹ؿ���


#define CCM_USE           //CCM ʹ�ÿ��ؿ���

#if (PRODUCTS_LEVEL == FACTORY_UPGRADE_VERSION) 
#define NET_FTP_CLIENT   	1   //patli 20190909    
#define USB_HOST_SUPPORT		//patli 20191216
#endif


//#define UART_WRITE_NO_QUEUE



/***********************************VIN������**************************************/

//#define VIN_CHARGE_ON                         //patli 20191009

#define CP_PERIOD_FEE						//�����׶η���

/****************************************************************************************/
/***********************************���Һ�̨���ؿ���**************************************/

//#define 	CHE_DIAN_BACKCOMM
//#define ZPLD_BACK_COMM
//#define AUNICE_BACK_COMM
#define 	BMS_USE_TIMER
//#define CHARGE_BIRD_BACK_COMM
//#define CP_BACK_COMM

#define 	DC_AC_ISO_REALCHECK   	//DC AC ISO REALCHECK ����ϲ�
//#define SCREEN_CARD				//SCREEN CARD����ϲ�
//#define 	DC_MET_ON				//SCREEN  
//#define 	AC_MET_ON				//SCREEN  
#define 	SCREEN_ON				//SCREEN  
//#define CARD_ON
//#define  	ISO_ON
#define 	GUN_BMS_CTRL			//BMS CTRL����ϲ�
#define 	A_B_GUN_TOGETER			//A, B BMS CTRL����ϲ�
//#define TRY_START_CHARGIN		//֧��3������


//#define     STM32F407

/****************************************************************************************/
#if 1  // (PRODUCTS_LEVEL == DEBUG_VERSION)  //�ڲ�����ʱ�Ŵ���Щlog

#define AUNICE_DEBUG			//����������LOG����
//#define NET_DEBUG				//��̫�����LOG����
//#define SCOMM_DEBUG				//����ͨ�����LOG����
//#define ETH_TEST_DEBUG	

//#define ETH_TEMP_TEST
#endif


/*******************************************************************************************/	

#ifdef AUNICE_DEBUG
#define MAIN_VERSION	3
#define SUB_VERSION	    7

#define U_MAIN_VERSION	0
#define U_SUB_VERSION	1


#define M_VERSION4		6

#else
	#if (FLASH_BASE_LEAVE_FACTORY == BOOT_BASE_ADDRESS)	
		#define M_VERSION3 "T29"
	    #define M_VERSION4 6
		#warning "T26"
	#elif (FLASH_BASE_LEAVE_FACTORY == FACTORY_UPGRADE_BASE_ADDRESS)
	  #define M_VERSION3 "B06"
		#warning "B06"
	#else
	  #define M_VERSION3 "T2.16"
  		#define M_VERSION4 10
		#warning M_VERSION3
	#endif	
#endif		  

/****************************������**********************************************/	

#define DCMETER_DLT645_2007         0       //DLT645_2007Э��ĵ��
#define DCMETER_RECT                1       //�ӹ���ģ���е���

#define DCMETER_TYPE                DCMETER_RECT



/****************************��Ļ���**********************************************/	

#define DWIN_DGUS         0       //����dgusһ����
#define DWIN_DGUS_II      1       //����dgus������

#define SCREEN_TYPE                DWIN_DGUS_II

#endif




