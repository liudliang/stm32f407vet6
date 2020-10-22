/**
  ******************************************************************************
  * @file    lcd_log.h
  * @author  MCD Application Team
  * @version V5.0.2
  * @date    05-March-2012
  * @brief   header for the lcd_log.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  __LCD_LOG_H__
#define  __LCD_LOG_H__

/* Includes ------------------------------------------------------------------*/

//#include "lcd_log_conf.h"

/** @addtogroup Utilities
  * @{
  */
  
/** @addtogroup STM32_EVAL
  * @{
  */ 

/** @addtogroup Common
  * @{
  */

/** @addtogroup LCD_LOG
  * @{
  */
  
/** @defgroup LCD_LOG
  * @brief 
  * @{
  */ 


/** @defgroup LCD_LOG_Exported_Defines
  * @{
  */ 

/** These value can be changed by user */

/**
  * @}
  */ 

/** @defgroup LCD_LOG_Exported_Types
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup LCD_LOG_Exported_Macros
  * @{
  */
#ifdef USELCDLOG
#define  LCD_ErrLog(...)    printf("ERROR: ") ;\
                            printf(__VA_ARGS__)

#define  LCD_UsrLog(...)    printf(__VA_ARGS__)


#define  LCD_DbgLog(...)   	printf(__VA_ARGS__)

#endif

#define  LCD_ErrLog(...)    
                            

#define  LCD_UsrLog(...)    


#define  LCD_DbgLog(...)   	
/**
  * @}
  */ 

/** @defgroup LCD_LOG_Exported_Variables
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup LCD_LOG_Exported_FunctionsPrototype
  * @{
  */ 

/**
  * @}
  */ 


#endif /* __LCD_LOG_H__ */

/**
  * @}
  */

/**
  * @}
  */ 

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */  

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
