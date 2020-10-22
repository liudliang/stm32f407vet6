/**
  ******************************************************************************
  * @file    RTC_BKPDomain.c 
  * @author  Aunice Application Team
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
#include "RTC_BKPDomain.h"

/* Private macro -------------------------------------------------------------*/
#define RTC_BKP_DR_NUMBER   0x14

/* Private variables ---------------------------------------------------------*/
static const uint32_t aBKPDataReg[RTC_BKP_DR_NUMBER] =
{
	RTC_BKP_DR0,  RTC_BKP_DR1,  RTC_BKP_DR2, 
	RTC_BKP_DR3,  RTC_BKP_DR4,  RTC_BKP_DR5,
	RTC_BKP_DR6,  RTC_BKP_DR7,  RTC_BKP_DR8, 
	RTC_BKP_DR9,  RTC_BKP_DR10, RTC_BKP_DR11, 
	RTC_BKP_DR12, RTC_BKP_DR13, RTC_BKP_DR14, 
	RTC_BKP_DR15, RTC_BKP_DR16, RTC_BKP_DR17, 
	RTC_BKP_DR18, RTC_BKP_DR19,
};
	
/**
  * @brief  RTC_BKPDomain.
  * @param  None
  * @retval None
  */
void RTC_BKPDomain(void)
{
	/* Enable the PWR APB1 Clock Interface */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);
	
	/* Wait for RTC APB registers synchronisation */
	RTC_WaitForSynchro();
	
	/*  Backup SRAM ***************************************************************/
	/* Enable BKPSRAM Clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
	
	/* Enable the Backup SRAM low power Regulator to retain it's content in VBAT mode */
  PWR_BackupRegulatorCmd(ENABLE);

  /* Wait until the Backup SRAM low power Regulator is ready */
  while(PWR_GetFlagStatus(PWR_FLAG_BRR) == RESET)
  {
  }
}

/**
  * @brief  system_softReset.
  * @param  None
  * @retval None
  */
void system_softReset(void)
{
	__set_FAULTMASK(1);
	NVIC_SystemReset();
}

/**
  * @brief  Write data to all Backup data registers.
  * @param  FirstBackupData: data to write to first backup data register.
  * @retval None
  */
void WriteToBackupReg(uint8_t index, uint32_t BackupData)
{
	if (index >= RTC_BKP_DR_NUMBER)
	{
		return;
	}
	
	RTC_WriteBackupRegister(aBKPDataReg[index], BackupData);
}

/**
  * @brief  Read data to all Backup data registers.
  * @param  FirstBackupData: data to write to first backup data register.
  * @retval None
  */
uint32_t ReadFromBackupReg(uint8_t index)
{
	uint32_t BackupData = 0;
	
	if (index >= RTC_BKP_DR_NUMBER)
	{
		return 0;
	}
	
	BackupData = RTC_ReadBackupRegister(aBKPDataReg[index]);
	
	return BackupData;
}

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
