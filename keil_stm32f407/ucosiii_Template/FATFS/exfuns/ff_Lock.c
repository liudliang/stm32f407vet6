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
#include "exfuns/ff_Lock.h"       /* FATFS */

/**
  * @brief  f_mount_Lock.
  * @param  None
  * @retval None
  */
FRESULT f_mount_Lock(
	FATFS* fs,			/* Pointer to the file system object (NULL:unmount)*/
	const TCHAR* path,	/* Logical drive number to be mounted/unmounted */
	BYTE opt			/* 0:Do not mount (delayed mount), 1:Mount immediately */
)
{
	FRESULT res = FR_OK;
	OS_ERR err;
	OSSchedLock(&err);

	res = f_mount (fs, path, opt);						/* Mount/Unmount a logical drive */

	OSSchedUnlock(&err);
	
	return res;
}

/**
  * @brief  f_open_Lock.
  * @param  None
  * @retval None
  */
FRESULT f_open_Lock(
	FIL* fp,			/* Pointer to the blank file object */
	const TCHAR* path,	/* Pointer to the file name */
	BYTE mode			/* Access mode and file open mode flags */
)
{
	FRESULT res = FR_OK;
  OS_ERR err;
	OSSchedLock(&err);

	res = f_open (fp, path, mode);			/* Open or create a file */

	OSSchedUnlock(&err);

	return res;
}

/**
  * @brief  f_lseek_Lock.
  * @param  None
  * @retval None
  */
FRESULT f_lseek_Lock(
	FIL* fp,		/* Pointer to the file object */
	DWORD ofs		/* File pointer from top of file */
)
{
	FRESULT res = FR_OK;
  OS_ERR err;
	OSSchedLock(&err);

	res = f_lseek (fp, ofs);						/* Move file pointer of a file object */

	OSSchedUnlock(&err);

	return res;
}

/**
  * @brief  f_read_Lock.
  * @param  None
  * @retval None
  */
FRESULT f_read_Lock(
	FIL* fp, 		/* Pointer to the file object */
	void* buff,		/* Pointer to data buffer */
	UINT btr,		/* Number of bytes to read */
	UINT* br		/* Pointer to number of bytes read */
)
{
	FRESULT res = FR_OK;
  OS_ERR err;	
	OSSchedLock(&err);
	
	res = f_read (fp, buff, btr, br);			/* Read data from a file */

	OSSchedUnlock(&err);
	
	return res;
}

/**
  * @brief  f_write_Lock.
  * @param  None
  * @retval None
  */
FRESULT f_write_Lock(
	FIL* fp,			/* Pointer to the file object */
	const void *buff,	/* Pointer to the data to be written */
	UINT btw,			/* Number of bytes to write */
	UINT* bw			/* Pointer to number of bytes written */
)
{
	FRESULT res = FR_OK;
  OS_ERR err;		
	OSSchedLock(&err);
	
	res = f_write (fp, buff, btw, bw);	/* Write data to a file */
	
	OSSchedUnlock(&err);
	
	return res;
}

/**
  * @brief  f_sync_Lock.
  * @param  None
  * @retval None
  */
FRESULT f_sync_Lock(
	FIL* fp		/* Pointer to the file object */
)
{
	FRESULT res = FR_OK;
  OS_ERR err;		
	OSSchedLock(&err);

	res = f_sync (fp);								/* Flush cached data of a writing file */

	OSSchedUnlock(&err);

	return res;
}

/**
  * @brief  f_close_Lock.
  * @param  None
  * @retval None
  */
FRESULT f_close_Lock(
	FIL *fp		/* Pointer to the file object to be closed */
)
{
	FRESULT res = FR_OK;
  OS_ERR err;	
	OSSchedLock(&err);

	res = f_close (fp);								/* Close an open file object */

	OSSchedUnlock(&err);

	return res;
}

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
