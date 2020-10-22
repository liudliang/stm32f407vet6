/**
  ******************************************************************************
  * @file    ff_Lock.c
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    04-August-2014
  * @brief   Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "ff_Lock.h"       /* FATFS */

/**
  * @brief  f_mount_Lock.
  * @param  None
  * @retval None
  */
FRESULT f_mount_Lock(BYTE ndrive, FATFS *ptrFATFS)
{
	FRESULT res = FR_OK;
	
	OSSchedLock();

	res = f_mount (ndrive, ptrFATFS);						/* Mount/Unmount a logical drive */

	OSSchedUnlock();
	
	return res;
}

/**
  * @brief  f_open_Lock.
  * @param  None
  * @retval None
  */
FRESULT f_open_Lock(FIL *ptrFIL, const XCHAR *pbuf, BYTE mode)
{
	FRESULT res = FR_OK;

	OSSchedLock();

	res = f_open (ptrFIL, pbuf, mode);			/* Open or create a file */

	OSSchedUnlock();

	return res;
}

/**
  * @brief  f_lseek_Lock.
  * @param  None
  * @retval None
  */
FRESULT f_lseek_Lock(FIL *ptrFIL, DWORD nds)
{
	FRESULT res = FR_OK;

	OSSchedLock();

	res = f_lseek (ptrFIL, nds);						/* Move file pointer of a file object */

	OSSchedUnlock();

	return res;
}

/**
  * @brief  f_read_Lock.
  * @param  None
  * @retval None
  */
FRESULT f_read_Lock(FIL *ptrFIL, void *pbuf, UINT numOfReadBytes, UINT *pnumOfReadedBytes)
{
	FRESULT res = FR_OK;
	
	OSSchedLock();
	
	res = f_read (ptrFIL, pbuf, numOfReadBytes, pnumOfReadedBytes);			/* Read data from a file */

	OSSchedUnlock();
	
	return res;
}

/**
  * @brief  f_write_Lock.
  * @param  None
  * @retval None
  */
FRESULT f_write_Lock(FIL *ptrFIL, const void *pbuf, UINT bytesToWrite, UINT *pbytesWritten)
{
	FRESULT res = FR_OK;
	
	OSSchedLock();
	
	res = f_write (ptrFIL, pbuf, bytesToWrite, pbytesWritten);	/* Write data to a file */
	
	OSSchedUnlock();
	
	return res;
}

/**
  * @brief  f_sync_Lock.
  * @param  None
  * @retval None
  */
FRESULT f_sync_Lock(FIL *ptrFIL)
{
	FRESULT res = FR_OK;

	OSSchedLock();

	res = f_sync (ptrFIL);								/* Flush cached data of a writing file */

	OSSchedUnlock();

	return res;
}

/**
  * @brief  f_close_Lock.
  * @param  None
  * @retval None
  */
FRESULT f_close_Lock(FIL *ptrFIL)
{
	FRESULT res = FR_OK;

	OSSchedLock();

	res = f_close (ptrFIL);								/* Close an open file object */

	OSSchedUnlock();

	return res;
}

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
