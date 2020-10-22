/**
  ******************************************************************************
  * @file    ff_Lock.h 
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    04-August-2014
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
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
#ifndef __FF_LOCK_H
#define __FF_LOCK_H

/* Includes ------------------------------------------------------------------*/
#include <includes.h>
//#include "config.h"
#include "src/ff.h"
#include "stm32f4xx.h"

/* Exported functions --------------------------------------------------------*/
extern FRESULT f_mount_Lock(FATFS* fs,const TCHAR* path,BYTE opt);
extern FRESULT f_open_Lock(FIL* fp,const TCHAR* path,BYTE mode);
FRESULT f_lseek_Lock(FIL* fp,DWORD ofs);
FRESULT f_read_Lock(FIL* fp,void* buff,UINT btr,UINT* br);
FRESULT f_write_Lock(FIL* fp,const void *buff,UINT btw,UINT* bw);
FRESULT f_sync_Lock(FIL* fp);
FRESULT f_close_Lock(FIL* fp);



#endif /* __FF_LOCK_H */

/**
  * @}
  */
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

