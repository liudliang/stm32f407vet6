
#include "rtc.h"

#if defined (STM32F10X_CL)
#include "stm32f10x_bkp.h"
#include "stm32f10x_rtc.h"
#endif

#if defined (STM32F4XX)
#include "stm32f4xx_rtc.h"
#include "common.h"
#endif

/*******************************************************************************
* 本文件实现基于RTC的日期功能，提供年月日的读写。（基于ANSI-C的time.h）
* ANSI-C的标准库中，提供了两种表示时间的数据  型：
* time_t:   	UNIX时间戳（从1970-1-1起到某时间经过的秒数）
* 	typedef unsigned int time_t;
* 
* struct tm:	Calendar格式（年月日形式）
*   tm结构如下：
*   struct tm {
*   	int tm_sec;   // 秒 seconds after the minute, 0 to 60
*   					 (0 - 60 allows for the occasional leap second)
*   	int tm_min;   // 分 minutes after the hour, 0 to 59
*		int tm_hour;  // 时 hours since midnight, 0 to 23
*		int tm_mday;  // 日 day of the month, 1 to 31
*		int tm_mon;   // 月 months since January, 0 to 11
*		int tm_year;  // 年 years since 1900
*		int tm_wday;  // 星期 days since Sunday, 0 to 6
*		int tm_yday;  // 从元旦起的天数 days since January 1, 0 to 365
* 		int tm_isdst; // 夏令时？？Daylight Savings Time flag
* 		...
* 	}
* 	其中wday，yday可以自动产生，软件直接读取
* 	mon的取值为0-11
*	***注意***：
*	tm_year:在time.h库中定义为1900年起的年份，即2008年应表示为2008-1900=108
* 	这种表示方法对用户来说不是十分友好，与现实有较大差异。
* 	所以在本文件中，屏蔽了这种差异。
* 	即外部调用本文件的函数时，tm结构体类型的日期，tm_year即为2008
* 	注意：若要调用系统库time.c中的函数，需要自行将tm_year-=1900
*******************************************************************************/

#if defined (STM32F4XX)
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LSE_MAX_TRIALS_NB         6

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int8_t RTC_Error = 0;

int8_t RTC_Configuration(RTC_InitTypeDef* RTC_InitStruct);
#endif


/*******************************************************************************
* Function Name  : Time_ConvUnixToCalendar(time_t t)
* Description    : RTC时间 -> 日历时间
* Input 		 : u32 t  当前RTC时间
* Output		 : None
* Return		 : struct tm
*******************************************************************************/
struct tm Time_ConvUnixToCalendar(time_t t)
{
	struct tm *t_tm;
	t_tm = localtime(&t);
	t_tm->tm_year += 1900;	//localtime转换结果的tm_year是相对值，需要转成绝对值
	return *t_tm;
}

/*******************************************************************************
* Function Name  : Time_ConvCalendarToUnix(struct tm t)
* Description    : 转化calendar时钟到RTC时钟 
* Input 		 : struct tm t
* Output		 : None
* Return		 : time_t
*******************************************************************************/
time_t Time_ConvCalendarToUnix(struct tm t)
{
	//t.tm_year -= 2000; 
	t.tm_year -= 1900;
    t.tm_mon -= 1;
	return mktime(&t);
}

/*******************************************************************************
* Function Name  : Time_GetUnixTime()
* Description    : 获取RTC时间
* Input 		 : None
* Output		 : None
* Return		 : time_t t
*******************************************************************************/
time_t Time_GetUnixTime(void)
{
	struct tm t_tm;
#if defined (STM32F10X_CL)
	return (time_t)RTC_GetCounter();
#endif

#if defined (STM32F4XX)
	t_tm = Time_GetCalendarTime();
	return (time_t)Time_ConvCalendarToUnix(t_tm);	
#endif
	
}

static struct tm gSystemTime;
/*******************************************************************************
* Function Name  : Time_GetCalendarTime()
* Description    : 获取日历时间（struct tm）
* Input 		 : None
* Output		 : None
* Return		 : time_t t
*******************************************************************************/
struct tm Time_GetCalendarTime(void)
{
	
	struct tm t_tm;
#if defined (STM32F10X_CL)
	time_t t_t;
	t_t = (time_t)RTC_GetCounter();
	t_tm = Time_ConvUnixToCalendar(t_t);
	t_tm.tm_mon += 1; 		
#endif

#if defined (STM32F4XX)
	RTC_TimeTypeDef   RTC_TimeStructure;
	RTC_DateTypeDef   RTC_DateStructure;
	 /* Get info from RTC here */
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
	t_tm.tm_sec    =  RTC_TimeStructure.RTC_Seconds;
	t_tm.tm_min    =  RTC_TimeStructure.RTC_Minutes;
	t_tm.tm_hour   =  RTC_TimeStructure.RTC_Hours;	

	RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
	t_tm.tm_year =  RTC_DateStructure.RTC_Year + 2000;	 //localtime转换结果的tm_year是相对值，需要转成绝对值
	t_tm.tm_mon =  RTC_DateStructure.RTC_Month;
	t_tm.tm_mday =  RTC_DateStructure.RTC_Date;
	t_tm.tm_wday =  RTC_DateStructure.RTC_WeekDay;
#endif
  
	if(t_tm.tm_year > 2003 && t_tm.tm_mon < 13  && t_tm.tm_hour < 25 ) {
		gSystemTime = t_tm;	
	}
	return t_tm;
}

struct tm Time_GetSystemCalendarTime(void)
{
	return gSystemTime;
}



 void Time_SetCalendarTime(struct tm t);
/*******************************************************************************
* Function Name  : Time_SetUnixTime()
* Description    : 写入RTC时间
* Input 		 : time_t t
* Output		 : None
* Return		 : None
*******************************************************************************/
void Time_SetUnixTime(time_t t)
{
#if defined (STM32F10X_CL)
	RTC_WaitForLastTask();
	RTC_SetCounter((u32)t);
	RTC_WaitForLastTask();
#endif

#if defined (STM32F4XX)
	struct tm t_tm;
	t_tm = Time_ConvUnixToCalendar(t);
	Time_SetCalendarTime(t_tm);
#endif
	return;
}

/*******************************************************************************
* Function Name  : Time_SetCalendarTime()
* Description    : 将给定的Calendar格式时间写入RTC
* Input 		 : struct tm t
* Output		 : None
* Return		 : None
*******************************************************************************/
void Time_SetCalendarTime(struct tm t)
{
#if defined (STM32F10X_CL)
	t.tm_mon -= 1; 
	Time_SetUnixTime(Time_ConvCalendarToUnix(t));
#endif

#if defined (STM32F4XX)
	RTC_TimeTypeDef   RTC_TimeStructure;
	RTC_DateTypeDef   RTC_DateStructure;

//	RTC_InitTypeDef RTC_InitStruct;
//	RTC_Configuration(&RTC_InitStruct);	                                      // 配置RTC
//	RTC_InitStruct.RTC_HourFormat = RTC_HourFormat_24;
//	RTC_Init(&RTC_InitStruct);
	
	RTC_TimeStructure.RTC_Seconds = t.tm_sec;
    RTC_TimeStructure.RTC_Minutes = t.tm_min;
    RTC_TimeStructure.RTC_Hours   = t.tm_hour;
    RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);

    RTC_DateStructure.RTC_Year  = t.tm_year-2000;
    RTC_DateStructure.RTC_Month = t.tm_mon;
    RTC_DateStructure.RTC_Date  = t.tm_mday;
	RTC_DateStructure.RTC_WeekDay  = t.tm_wday;
    RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
#endif

	return;
}

#if defined (STM32F10X_CL)
// ================================================================================================================
// void RTC_Configuration(void) RTC配置 
// ================================================================================================================

void RTC_Configuration(void)
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_AHB1Periph_BKPSRAM, ENABLE);	 // 打开电源和后备接口时钟
  PWR_BackupAccessCmd(ENABLE);	                       // 使能对后备寄存器和RTC的访问 
  BKP_DeInit();		                                     // 复位后备寄存器
  RCC_LSEConfig(RCC_LSE_ON);	                         // 32.768K Hz 低速外部谐振器 为实时时钟提供一个低功耗且精准的时钟源 RCC_BDCR-LSEON 
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	 // 等待 LSE稳定 
  {}

  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);	             // 设置LSE作为RTC时钟源 
  RCC_RTCCLKCmd(ENABLE);	                             // 使能 RTC时钟 
  RTC_WaitForSynchro();			                           // 时钟寄存器同步完成  
  RTC_WaitForLastTask();			                         // 等待对RTC寄存器写操作完成  
  RTC_WaitForLastTask();		                           // 等待对RTC寄存器写操作完成
  RTC_SetPrescaler(32767);                             // RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) 预分频器 设置为1S的周期
  RTC_WaitForLastTask();		                           // 等待对RTC寄存器写操作完成
}
#endif



//int8_t Iwdg_LSI_Init()
//{
//	 int32 RTC_Error = 0,RTC_Timeout=0x10000;
//	 RCC_LSICmd(ENABLE);
//  /* Wait till LSI is ready */
//  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
//  {
//    if (RTC_Timeout-- == 0)
//    {
//      RTC_Error = -1;
//			break;
//    }
//  }
//	return RTC_Error;
//}


#if defined (STM32F4XX)
/**
* @brief  Configures the RTC peripheral.
* @param  None
* @retval None
*/
int8_t RTC_Configuration(RTC_InitTypeDef* RTC_InitStruct)
{
  uint8_t trialno;
  int32 RTC_Error = 0,RTC_Timeout=0x10000;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);

#ifdef RTC_CLOCK_SOURCE_LSI
  /* Enable the LSI OSC */
  RCC_LSICmd(ENABLE);
  /* Wait till LSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {
    if (RTC_Timeout-- == 0)
    {
      RTC_Error = -1;
      break;
    }
  }
	
  if (RTC_Error == -1)
  {
    return -1;
  }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
#endif

#ifdef RTC_CLOCK_SOURCE_LSE

  for ( trialno = 0 ; trialno <= LSE_MAX_TRIALS_NB ; trialno++)
  {
    /* Enable the LSE OSC */
    RCC_LSEConfig(RCC_LSE_ON);
    RTC_Timeout = 0x500;         //old is 0x100  20190320 modify by 
    
    if ((trialno == 1) && (trialno == 4))
    {
      //GL_State_Message((uint8_t *)"RTC and backup Starting.. ");
    }

    else if ((trialno == 2)&& (trialno == 5))
    {
      //GL_State_Message((uint8_t *)"RTC and backup Starting...");
    }
    
    else if ((trialno == 3)&& (trialno == 6))
    {
      //GL_State_Message((uint8_t *)"RTC and backup Starting.  ");
    }
    OSTimeDly(2); //20181213
    /* Wait till LSE is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
    {
      if (RTC_Timeout-- == 0)
      {
        RTC_Error = -1;
        RCC_LSEConfig(RCC_LSE_OFF);
        break;
      }
	    OSTimeDly(2); //20190319 没有32K晶振有时不启振
    }
    
    if (RTC_Error == 0)
    {
      break;
    }
  }

  if (RTC_Error == -1)
  {
    return -1;
  }
  
  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
#endif
  
#ifdef RTC_CLOCK_SOURCE_HSE
  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div31);
#endif
  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

#ifdef RTC_CLOCK_SOURCE_HSE
  /*Calender Configuration*/
  RTC_InitStruct->RTC_AsynchPrediv =  0x189D;
  RTC_InitStruct->RTC_SynchPrediv = 0x7F;
#endif

#ifdef RTC_CLOCK_SOURCE_LSI
  /*Calender Configuration*/
  RTC_InitStruct->RTC_AsynchPrediv = 0x7F;
  RTC_InitStruct->RTC_SynchPrediv =  0xCB;
#endif

#ifdef RTC_CLOCK_SOURCE_LSE
  /*Calender Configuration*/
  RTC_InitStruct->RTC_AsynchPrediv = 0x7F;
  RTC_InitStruct->RTC_SynchPrediv =  0xFF;
#endif

  /* Wait for RTC APB registers synchronisation */
  if ( RTC_WaitForSynchro() == ERROR)
  {
    return -1;
  }
  return 0;
}

#endif


// ================================================================================
//  void Time_Adjust(void)
//  时钟校准 
//  ==============================================================================
void Time_Adjust(struct tm CalTime)
{
#if defined (STM32F10X_CL)
	RTC_Configuration();	                // 配置RTC 
	PWR_BackupAccessCmd(ENABLE);	        // 使能对后备寄存器和RTC的访问 
	RTC_WaitForLastTask();				        // 等待上次写寄存器操作完成
	Time_SetCalendarTime(CalTime);
	RTC_WaitForLastTask();			          // 等待写寄存器操作完成
	PWR_BackupAccessCmd(DISABLE);	        // 禁能对后备寄存器和RTC的访问 
	PWR_BackupAccessCmd(ENABLE);	        // 使能对后备寄存器和RTC的访问 
	BKP_WriteBackupRegister(BKP_DR1, BKP_RTC_CFG_FLAG);	        // 写入操作标志	
	PWR_BackupAccessCmd(DISABLE);	        // 使能对后备寄存器和RTC的访问 
#endif

#if defined (STM32F4XX)
	//RTC_InitTypeDef RTC_InitStruct;
	//RTC_Configuration(&RTC_InitStruct);	                                      // 配置RTC
	//RTC_InitStruct.RTC_HourFormat = RTC_HourFormat_24;
	//RTC_Init(&RTC_InitStruct);
	Time_SetCalendarTime(CalTime);
#endif	
}

//#include"appstart.h"
void sysRtcHwInit(void)
{	 
	struct tm CalTime;

#if defined (STM32F10X_CL)
	if (BKP_ReadBackupRegister(BKP_DR1) != BKP_RTC_CFG_FLAG)			// 读取备份寄存器偏移量为4的位置 - 备份数据寄存器不正确或者没有被用过
  {
    uart_test("BKP init\r\n", strlen("BKP init\r\n"));
		RTC_Configuration();	                                      // 配置RTC 

		CalTime.tm_year = 2010;
		CalTime.tm_mon  = 8;
		CalTime.tm_mday = 10;
		CalTime.tm_hour = 11;
		CalTime.tm_min  = 30;
		CalTime.tm_sec  = 0;
    Time_Adjust(CalTime);	
		                                      // 配置时分秒 -  初始化一个默认时间 
		PWR_BackupAccessCmd(ENABLE);	        // 使能对后备寄存器和RTC的访问 
    BKP_WriteBackupRegister(BKP_DR1, BKP_RTC_CFG_FLAG);	        // 写入操作标志
		PWR_BackupAccessCmd(DISABLE);	        // 使能对后备寄存器和RTC的访问 
  }
  else		                                                      // 备份寄存器已经被操作过 
  {
    RTC_WaitForSynchro();			                                  // 等待RTC寄存器同步 
    RTC_WaitForLastTask();		                                  // 等待寄存器写操作完成 
  }
#endif

#if defined (STM32F4XX)
	RTC_InitTypeDef RTC_InitStruct;
	RTC_Configuration(&RTC_InitStruct);	                                      // 配置RTC
	RTC_InitStruct.RTC_HourFormat = RTC_HourFormat_24;
	RTC_Init(&RTC_InitStruct);
	if(RTC_ReadBackupRegister(RTC_BKP_DR1) != BKP_RTC_CFG_FLAG)
	{
		RTC_InitTypeDef RTC_InitStruct;
		RTC_Configuration(&RTC_InitStruct);	                                      // 配置RTC
		RTC_InitStruct.RTC_HourFormat = RTC_HourFormat_24;
		RTC_Init(&RTC_InitStruct);

		CalTime.tm_year = 2013;
		CalTime.tm_mon  = 9;
		CalTime.tm_mday = 12;
		CalTime.tm_hour = 20;
		CalTime.tm_min  = 20;
		CalTime.tm_sec  = 0;
		CalTime.tm_wday  = 0;
		Time_Adjust(CalTime);	
		RTC_WriteBackupRegister(RTC_BKP_DR1,BKP_RTC_CFG_FLAG);

	}
#endif
}

/*该函数为了兼容207的触摸屏*/
ErrorStatus RTC_SetDateTime(struct tm * CalTime)
{
   Time_Adjust(*CalTime);
   return SUCCESS;
}
