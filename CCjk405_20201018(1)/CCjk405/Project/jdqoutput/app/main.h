//main.h
#ifndef _MAIN_H_001
#define _MAIN_H_001

#ifdef  __cplusplus
extern  "C" {
#endif

#include "config.h"	 
#include "common.h"
	
#define COM1        0
#define COM2        1
#define COM3        2
#define COM4        3
#define COM5        4


extern uint32 g_TimerMs;

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#if LWIP_DHCP
	#define USE_DHCP       /* enable DHCP, if disabled static address is used */
#endif

/* Uncomment SERIAL_DEBUG to enables retarget of printf to  serial port (COM1 on STM32 evalboard) 
   for debug purpose */   
//#define SERIAL_DEBUG 

/* MAC ADDRESS: MAC_ADDR0:MAC_ADDR1:MAC_ADDR2:MAC_ADDR3:MAC_ADDR4:MAC_ADDR5 */
#define MAC_ADDR0   2
#define MAC_ADDR1   0
#define MAC_ADDR2   0
#define MAC_ADDR3   0
#define MAC_ADDR4   0
#define MAC_ADDR5   0

/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   1
#define IP_ADDR3   8

/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   1
#define GW_ADDR3   1

/* MII and RMII mode selection, for STM324xG-EVAL Board(MB786) RevB ***********/
#define RMII_MODE  // User have to provide the 50 MHz clock by soldering a 50 MHz
                     // oscillator (ref SM7745HEV-50.0M or equivalent) on the U3
                     // footprint located under CN3 and also removing jumper on JP5. 
                     // This oscillator is not provided with the board. 
                     // For more details, please refer to STM3240G-EVAL evaluation
                     // board User manual (UM1461).


//#define MII_MODE

/* Uncomment the define below to clock the PHY from external 25MHz crystal (only for MII mode) */
//#ifdef 	MII_MODE
// #define PHY_CLOCK_MCO
//#endif

/* STM324xG-EVAL jumpers setting
    +==========================================================================================+
    +  Jumper |       MII mode configuration            |      RMII mode configuration         +
    +==========================================================================================+
    +  JP5    | 2-3 provide 25MHz clock by MCO(PA8)     |  Not fitted                          +
    +         | 1-2 provide 25MHz clock by ext. Crystal |                                      +
    + -----------------------------------------------------------------------------------------+
    +  JP6    |          2-3                            |  1-2                                 +
    + -----------------------------------------------------------------------------------------+
    +  JP8    |          Open                           |  Close                               +
    +==========================================================================================+
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Main_Delay(int32_t nCount);
#if (PRODUCTS_LEVEL == FACTORY_UPGRADE_VERSION)

/* Private variables ---------------------------------------------------------*/
#define REGDEFAULTVALUE         0             
#define REGUSBHNONCONNECTION    0xA5B5C0C0    //U盘未连接
#define REGUSBHCONNECTION       0xA5B5C1C1    //U盘连接 
#define REGUPGRADEFINISH        0xC2C2A5B5    //升级完成

enum JUDGE_TO_APPLICATION
{
	JUDGE_TO_USB_PROGRAM,
	JUDGE_TO_FACTORY_PROGRAM,
	JUDGE_TO_USER_PROGRAM,
};


extern void IAP_To_Application(void);
extern uint8 Judge_To_Application(void);
extern uint8 Where_To_Application(void);
#endif

#ifdef  __cplusplus
}
#endif

#endif
