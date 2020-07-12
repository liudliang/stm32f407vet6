#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "common.h"

#include "gpio.h"
#include "dma.h"
#include "iwdg.h"
#include "timer.h"
#include "24cxx.h"
#include "ds18b20.h"
#include "w25qxx.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "tcp_client_demo.h"

#include "malloc.h"
#include "src/ff.h"
#include "exfuns/exfuns.h"

#include "includes.h"
#include "os_app_hooks.h"


#include "MainData.h"
#include "Debug.h"

//ALIENTEK 探索者STM32F407开发板 UCOSIII实验
//例4-1 UCOSIII UCOSIII移植

//UCOSIII中以下优先级用户程序不能使用，ALIENTEK
//将这些优先级分配给了UCOSIII的5个系统内部任务
//优先级0：中断服务服务管理任务 OS_IntQTask()
//优先级1：时钟节拍任务 OS_TickTask()
//优先级2：定时任务 OS_TmrTask()
//优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
//优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com  
//广州市星翼电子科技有限公司  
//作者：正点原子 @ALIENTEK


//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);


//任务控制块
OS_TCB TestTaskTCB;
//任务堆栈	
CPU_STK TEST_TASK_STK[TEST_STK_SIZE];
void test_task(void *p_arg);


//任务控制块
OS_TCB CardReaderTaskTCB;
//任务堆栈	
CPU_STK CARDREADER_TASK_STK[CARDREADER_STK_SIZE];
//任务函数
void CardReader_task(void *p_arg);


//任务控制块
OS_TCB	FloatTaskTCB;
//任务堆栈
CPU_STK	FLOAT_TASK_STK[FLOAT_STK_SIZE];
//任务函数
void float_task(void *p_arg);



//任务控制块
OS_TCB	RealTimeCheckTaskTCB;
//任务堆栈
CPU_STK	RealTimeCheck_TASK_STK[RealTimeCheck_STK_SIZE];
//任务函数
void RealTimeCheck_task(void *p_arg);



//任务控制块
OS_TCB TaskStackUsageTaskTCB;
//任务堆栈	
CPU_STK TaskStackUsage_TASK_STK[TaskStackUsage_STK_SIZE];
void TaskStackUsage_task(void *p_arg);



//任务控制块
OS_TCB	USBHostTaskTCB;
//任务堆栈
CPU_STK	USBHost_TASK_STK[USBHost_STK_SIZE];
//任务函数
void USBHost_task(void *p_arg);



/*
*********************************************************************************************************
*	函 数 名: CreateNewFile
*	功能说明: 在SD卡创建一个新文件，文件内容填写“www.armfly.com”
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void CreateNewFile(void)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS *fs = mymalloc(SRAMIN,sizeof(FATFS));		//为file申请内存
	DIR *DirInf = mymalloc(SRAMIN,sizeof(DIR));		//为file申请内存
	FIL *file = (FIL *)mymalloc(SRAMIN,sizeof(FIL));		//为file申请内存
	
	if((NULL == DirInf) || (NULL == file) )
	{
		myfree(SRAMIN,fs);//释放内存
		myfree(SRAMIN,DirInf);//释放内存
		myfree(SRAMIN,file);//释放内存
		DEBUG_Printf("malloc ViewRootDir failed!");
		return ;
	}
  uint32_t bw;
// 	/* 挂载文件系统 */
//	result = f_mount(FS_USB, &fs);			/* Mount a logical drive */
//	if (result != FR_OK)
//	{
//		printf("挂载文件系统失败 (%d)\r\n", result);
//	}
//	result = f_mount(fs,"1:",1);

//	/* 打开根文件夹 */
//	result = f_opendir(DirInf, "1:"); /* 如果不带参数，则从当前目录开始 */
//	if (result != FR_OK)
//	{
//		printf("打开根目录失败 (%d)\r\n", result);
//	}

	/* 打开文件 */
	result = f_open(file, "1:STM32F407.txt", FA_WRITE);
	if( (result != FR_OK) && (result != FR_EXIST) )
	{
		printf("打开STM32F407.txt 文件失败！！\r\n");
	}

	/* 写一串数据 */
	result = f_write(file, "FatFS Write Demo2\r\n", 34, &bw);
	if (result == FR_OK)
	{
		printf("STM32F407.txt 文件写入成功\r\n");
	}
	else
	{
		printf("STM32F407.txt 文件写入失败\r\n");
	}

	/* 关闭文件*/
	f_close(file);
	/* 卸载文件系统 */
	f_mount(fs, "1:", NULL);

//	/* 卸载文件系统 */
//	f_mount(FS_USB, NULL);
			myfree(SRAMIN,fs);//释放内存
		myfree(SRAMIN,DirInf);//释放内存
		myfree(SRAMIN,file);//释放内存
}

//-----------------------------------------------------------


//test code任务函数
void test_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	
	CHARGE_TYPE *PtrRunData = ChgData_GetRunDataPtr();
	uint8_t k2status = PtrRunData->input->statu.bits.key2;
	uint8_t k3status = PtrRunData->input->statu.bits.key3;
	uint8_t *p = NULL,*pBuffer = NULL;
	uint8_t result = 0;
	u8 datatemp[50] = {0};
	u16 reboottimes;
	//-----------------test spi func
//	const u8 TEXT_Buffer[]={"Explorer STM32F4 SPI TEST"};
//	u32 FLASH_SIZE=16*1024*1024;	//FLASH 大小为16字节
//	u8 SIZE = sizeof(TEXT_Buffer);
//	
//	W25QXX_Write((u8*)TEXT_Buffer,FLASH_SIZE-100,SIZE);
//	OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1s
//	W25QXX_Read(datatemp,FLASH_SIZE-100,SIZE);
//	DEBUG_Printf((char *)datatemp);
//-----------------
	
//-----------------test lwip
	while(lwip_comm_init()) 		//lwip初始化
	{
		DEBUG_Printf("Lwip Init failed!\r\n"); 	//lwip初始化失败
		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);  
	}
	
	while(tcp_client_init()) 									//初始化tcp_client(创建tcp_client线程)
	{
		DEBUG_Printf("TCP Client failed!!\r\n"); 		//tcp创建失败
		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err); 
	}
	
#if LWIP_DHCP
	lwip_comm_dhcp_creat(); //创建DHCP任务
#endif
//-----------------	
	
//--------------------------test fatfs
  exfuns_ViewRootDir("1:");
	
//	CreateNewFile();
//	ReadFileData();
//	ViewRootDir();
	
	TEST_DATA_TYPE test_fatfs_data;
	SST_Rd_BufferRead((u8 *)datatemp,0, sizeof(datatemp), "1:STM32F407.txt");
	sprintf(DEBUG_Buff,"fatfs_test txtdata is %s!!\r\n",datatemp);
	DEBUG_Printf(DEBUG_Buff);	
	
	memset(datatemp,0,sizeof(datatemp));
	SST_Rd_BufferRead(datatemp,0, sizeof(test_fatfs_data), "1:test.data");
	sprintf(DEBUG_Buff,"fatfs_test data is %s!!\r\n",datatemp);
	DEBUG_Printf(DEBUG_Buff);	
	
	reboottimes = AT24CXX_ReadLenByte(AT24Cxx_RebootTimes_ADDR,4);
	reboottimes = Common_Bcd2hex32(reboottimes);
	test_fatfs_data.head = 0x7576;
	test_fatfs_data.reboottime = reboottimes;
	test_fatfs_data.temprature = DMA_GetTemprate();
	test_fatfs_data.crc = Crc16_Calc((uint8_t *)&test_fatfs_data,sizeof(TEST_DATA_TYPE)-2);
	test_fatfs_data.end = "\r";
//	test_fatfs_data.end++;
//	test_fatfs_data.end = "\n";
//  uint16_t test4kbuf[5000] = {0};
//	memset(&test4kbuf,'1',5000);
//	test4kbuf[4999] = 'b';
	pBuffer = (uint8_t *)mymalloc(SRAMIN,5000);;//&test_fatfs_data;
	u16 j;
	for(j = 0; j < 4999; j++) *(pBuffer + j) = '1';
	*(pBuffer + j) = 'b';
	SST_Rd_BufferWrite(pBuffer, 0, 5000, "1:test.data");
	OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); 

//	memset(&test_fatfs_data,0,sizeof(test_fatfs_data));
//	memset(&test4kbuf,0,5000);
for(u16 j = 0; j < 5000; j++) *(pBuffer + j) = 0;
	SST_Rd_BufferRead(pBuffer,0, 5000, "1:test.data");	
	sprintf(DEBUG_Buff,"fatfs_test data is %c!!\r\n",pBuffer[4999]);
	myfree(SRAMIN,pBuffer);
	
	DEBUG_Printf(DEBUG_Buff);	
	exfuns_ViewRootDir("1:");
	
	u32 total,free;
	exf_getfree("1:",&total,&free);
	sprintf(DEBUG_Buff,"fatfs data Total/FREE %d/%d!!\r\n",total,free);
	DEBUG_Printf(DEBUG_Buff);
//--------------------------	
	
	while(1)
	{
		if(k2status != PtrRunData->input->statu.bits.key2)
		{
			k2status = PtrRunData->input->statu.bits.key2;
			if(1 == k2status)
			{
//				p=mymalloc(SRAMIN,2048);//申请2K字节  
				LED_Toggle(LED2);
				tcp_client_flag|=LWIP_SEND_DATA; //标记LWIP有数据要发送;
			}
		}
		
		if(k3status != PtrRunData->input->statu.bits.key3)
		{
			k3status = PtrRunData->input->statu.bits.key3;
			if(1 == k3status)
			{
//				myfree(SRAMIN,p);//释放内存
				LED_Toggle(LED3);
				debug_tcp_client_flag|=LWIP_SEND_DATA; //标记LWIP有数据要发送;
			}
		}
		
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时20ms
	}
}

////led1任务函数
//void led2_task(void *p_arg)
//{
//	OS_ERR err;
//	p_arg = p_arg;
//	while(1)
//	{
//		LED_Toggle(LED3);
//		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
//	}
//}


//浮点测试任务
void float_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	uint8_t i;
	uint16_t adcx = 0;
	static float float_num[3]={0};
	float pro=0;//进度
	uint8_t SEND_BUF_SIZE = 20;
  uint16_t *SendBuff = DMA_GetAdcAver();
	
	
	
	while(1)
	{
//		float_num+=0.01f;
//		OS_CRITICAL_ENTER();	//进入临界区


//	  if(DMA_GetFlagStatus(DMA2_Stream0,DMA_FLAG_TCIF0)!=RESET)//等待DMA2_Steam0传输完成
//		{ 
//		  for(i = 0; i < SEND_BUF_SIZE;i++)
//			{
//				adcx += SendBuff[i];
//			}
//		    adcx = adcx/SEND_BUF_SIZE;
//		    DMA_ClearFlag(DMA2_Stream0,DMA_FLAG_TCIF0);//清除DMA2_Steam7传输完成标志
//		}
//		pro=DMA_GetCurrDataCounter(DMA2_Stream0);//得到当前还剩余多少个数据
//		pro=1-pro/30;//得到百分比	  
//		pro*=100;      			    //扩大100倍
		
//		adcx=Get_Adc_Average(ADC_Channel_10,20);
		for(i = 0; i < 3; i++)
		{
		  float_num[i]=(float)SendBuff[i]*(3.3/4096);
		}			
		sprintf(DEBUG_Buff,"float_num[0]=%.4f,float_num[1]=%.4f,18b20=%d,temperature=%f\r\n",float_num[0],float_num[1],ChgData_GetRunDataPtr()->dev_status->temperature,DMA_GetTemprate());
    DEBUG_Printf(DEBUG_Buff);
//		OS_CRITICAL_EXIT();		//退出临界区
			 

//		LED_Toggle(LED3);  //运行灯
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1s

	}
}


//TaskStackUsage任务函数
void TaskStackUsage_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	CPU_STK_SIZE free,used;

	DEBUG_Printf("app start running!\r\n");
	while(1)
	{
		sprintf(DEBUG_Buff,"CPU useage:%d\r\n", OSStatTaskCPUUsage);
		DEBUG_Printf(DEBUG_Buff);
		
		OSTaskStkChk(&TaskStackUsageTaskTCB,&free,&used,&err);
		sprintf(DEBUG_Buff,"TaskStackUsageTask used/free:%d/%d  usage:%%%d\r\n",used,free,(used*100)/(used+free));
		DEBUG_Printf(DEBUG_Buff);
		
		OSTaskStkChk(&TestTaskTCB,&free,&used,&err);
		sprintf(DEBUG_Buff,"Led1TaskTCB used/free:%d/%d  usage:%%%d\r\n",used,free,(used*100)/(used+free));
		DEBUG_Printf(DEBUG_Buff);
		
		OSTaskStkChk(&CardReaderTaskTCB,&free,&used,&err);
		sprintf(DEBUG_Buff,"Led2TaskTCB used/free:%d/%d  usage:%%%d\r\n",used,free,(used*100)/(used+free));
		DEBUG_Printf(DEBUG_Buff);
		
		OSTaskStkChk(&FloatTaskTCB,&free,&used,&err);
		sprintf(DEBUG_Buff,"FloatTaskTCB used/free:%d/%d  usage:%%%d\r\n",used,free,(used*100)/(used+free));
		DEBUG_Printf(DEBUG_Buff);
		
		OSTaskStkChk(&RealTimeCheckTaskTCB,&free,&used,&err);
		sprintf(DEBUG_Buff,"RealTimeCheckTaskTCB used/free:%d/%d  usage:%%%d\r\n",used,free,(used*100)/(used+free));
		DEBUG_Printf(DEBUG_Buff);
		
		sprintf(DEBUG_Buff,"mymalloc size/useage:%d/%d\r\n", MEM1_MAX_SIZE,my_mem_perused(SRAMIN));
		DEBUG_Printf(DEBUG_Buff);
		
		sprintf(DEBUG_Buff,"ETH LINK:%d\r\n",DP83848_GetPHYlinkStatus());
		DEBUG_Printf(DEBUG_Buff);
		
		DEBUG_Printf("\r\n\r\n\r\n");
		
		OSTimeDlyHMSM(0,0,3,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时3s
	}
}


// ========================================================================================================
// void sysSTM32F40xAssertClocks(void)
// 描述: 	系统时钟检查
// 返回值: 无
// ========================================================================================================
static void sysSTM32F40xAssertClocks(void)
{
	RCC_ClocksTypeDef RCC_Clocks = { 0 };
	
	RCC_GetClocksFreq(&RCC_Clocks);
	
	if (RCC_Clocks.SYSCLK_Frequency != SystemCoreClock)
	{
		while (1);
	}
}

// ========================================================================================================
// void RCC_Configuration(void)
// 描述: 	使能高速时钟
// 返回值: 无
// ========================================================================================================
static void RCC_Configuration(void)
{																	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	sysSTM32F40xAssertClocks();
}

void Hanrdware_Init(void)
{

	delay_init(168);  	//时钟初始化
	RCC_Configuration();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组配置
//	uart_init(115200);  //串口初始化
	sys_SerialHwInit();
	
	InPut_OutPut_Init();
	sys_ADC1_Config();
//	IWDG_Init(4,1500); //与分频数为 64,重载值为 1500,溢出时间为 3s
	TimerInit();        //定时器初始化
	AT24CXX_Init();     //24C02 初始化
  DS18B20_Init();	    //DS18B20初始化
	W25QXX_Init();			//W25QXX初始化
	
	my_mem_init(SRAMIN);		//初始化内部内存池
	

	

}




void HardWare_Check(void)
{
	OS_ERR err;
	uint8_t res = 0,tmpdata[4] = {0};
	uint32_t reboottimes = 0;
	if(AT24CXX_Check())//检测不到24c02
	{
		DEBUG_Printf("24C02 Check Failed!\r\n");	
		while(1);
	}
	else
	{
		sprintf(DEBUG_Buff,"24C02 Check Successfull!\r\n");
		DEBUG_Printf(DEBUG_Buff);
//		AT24CXX_Read(AT24Cxx_RebootTimes_ADDR,(uint8_t *)&reboottimes,4);
		reboottimes = AT24CXX_ReadLenByte(AT24Cxx_RebootTimes_ADDR,4);
		reboottimes = Common_Bcd2hex32(reboottimes) + 1;
		sprintf(DEBUG_Buff,"reboot times：%d!\r\n",reboottimes);
		DEBUG_Printf(DEBUG_Buff);
		reboottimes = Common_Hex2bcd32(reboottimes);
//		AT24CXX_Write(0,(uint8_t*)reboottimes,4);    //以BCD码格式存储
		AT24CXX_WriteLenByte(AT24Cxx_RebootTimes_ADDR,reboottimes,4);
	}
	
	if(1 == DS18B20_Check())
	{
		DEBUG_Printf("DS18B20 Check Failed!\r\n");	
	}
	
	if(W25Q64 != W25QXX_ReadID())								//检测不到W25Q128
	{
		DEBUG_Printf("W25Q64 Check Failed!\r\n");
	}
	else
	{
		if(1 == exfuns_init(EX_FLASH))							//为fatfs相关变量申请内存		
		{
			DEBUG_Printf("fatfs Disk malloc false!");
			while(1);
		}			
//  	f_mount(fs[0],"0:",1); 					//挂载SD卡 
 	  res = f_mount(exfuns_GetfsArea(EX_FLASH),"1:",1); 				//挂载FLASH.		
		
		FIL* file=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//为file申请内存
		if(NULL == file)
		{
			myfree(SRAMIN,file);//释放内存
			DEBUG_Printf("malloc file ptr failed!");
			while(1);
		}
		res=f_open(file,"1:test.data",FA_READ );      // 打开文件是否存在
	  f_close(file);
		if(FR_OK != res)//FLASH磁盘,FAT文件系统错误,重新格式化FLASH
	  {
		  DEBUG_Printf("Flash Disk Formatting...");	//格式化FLASH
		  res=f_mkfs("1:",1,4096);//格式化FLASH,1,盘符;1,不需要引导区,8个扇区为1个簇	
			res=f_open(file,"1:test.data",FA_CREATE_ALWAYS | FA_WRITE );      // 打开文件是否存在
	    f_close(file);	
		}			
		
		myfree(SRAMIN,file);//释放内存
		
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1s
	}		
	
	
}

//开始任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
	
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif

#if OS_CFG_APP_HOOKS_EN				//使用钩子函数
	App_OS_SetAllHooks();			
#endif	
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		

#ifdef DEBUG_ON   //调试打开	
	DEBUG_Init();
#endif

  HardWare_Check();
  MainCtrlUnit_Init();


	OS_CRITICAL_ENTER();	//进入临界区
	
	//创建检测各任务堆栈使用情况任务
	OSTaskCreate((OS_TCB 	* )&TaskStackUsageTaskTCB,		
				 (CPU_CHAR	* )"TaskStackUsage task", 		
                 (OS_TASK_PTR )TaskStackUsage_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TaskStackUsage_TASK_PRIO,     
                 (CPU_STK   * )&TaskStackUsage_TASK_STK[0],	
                 (CPU_STK_SIZE)TaskStackUsage_STK_SIZE/10,	
                 (CPU_STK_SIZE)TaskStackUsage_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);
	
	//创建LED1任务
	OSTaskCreate((OS_TCB 	* )&TestTaskTCB,		
				 (CPU_CHAR	* )"test task", 		
                 (OS_TASK_PTR )test_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TEST_TASK_PRIO,     
                 (CPU_STK   * )&TEST_TASK_STK[0],	
                 (CPU_STK_SIZE)TEST_STK_SIZE/10,	
                 (CPU_STK_SIZE)TEST_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
				 
	//创建CARDREADER任务
	OSTaskCreate((OS_TCB 	* )&CardReaderTaskTCB,		
				 (CPU_CHAR	* )"CardReader task", 		
                 (OS_TASK_PTR )CardReader_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )CARDREADER_TASK_PRIO,     	
                 (CPU_STK   * )&CARDREADER_TASK_STK[0],	
                 (CPU_STK_SIZE)CARDREADER_STK_SIZE/10,	
                 (CPU_STK_SIZE)CARDREADER_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);
				 
	//创建浮点测试任务
	OSTaskCreate((OS_TCB 	* )&FloatTaskTCB,		
				 (CPU_CHAR	* )"float test task", 		
                 (OS_TASK_PTR )float_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )FLOAT_TASK_PRIO,     	
                 (CPU_STK   * )&FLOAT_TASK_STK[0],	
                 (CPU_STK_SIZE)FLOAT_STK_SIZE/10,	
                 (CPU_STK_SIZE)FLOAT_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);				
	//创建实时检测任务
	OSTaskCreate((OS_TCB 	* )&RealTimeCheckTaskTCB,		
				 (CPU_CHAR	* )"RealTimeCheck test task", 		
                 (OS_TASK_PTR )RealTimeCheck_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )RealTimeCheck_TASK_PRIO,     	
                 (CPU_STK   * )&RealTimeCheck_TASK_STK[0],	
                 (CPU_STK_SIZE)RealTimeCheck_STK_SIZE/10,	
                 (CPU_STK_SIZE)RealTimeCheck_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);			
								 
#ifdef USB_HOST_SUPPORT
	//U盘host任务
	OSTaskCreate((OS_TCB 	* )&USBHostTaskTCB,		
				 (CPU_CHAR	* )"USBHost test task", 		
                 (OS_TASK_PTR )USBHost_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )USBHost_TASK_PRIO,     	
                 (CPU_STK   * )&USBHost_TASK_STK[0],	
                 (CPU_STK_SIZE)USBHost_STK_SIZE/10,	
                 (CPU_STK_SIZE)USBHost_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);			
#endif
								 
	OS_CRITICAL_EXIT();	//退出临界区
	
								 
	while(1)
	{
		LED_Toggle(LED1);  //运行灯
		IWDG_Feed();
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1s
	}
//	OSTaskDel((OS_TCB*)0,&err);	//删除start_task任务自身							 
			

}


int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	Hanrdware_Init();
	
	OSInit(&err);		//初始化UCOSIII
	OS_CRITICAL_ENTER();//进入临界区
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);  //开启UCOSIII
	while(1);
}
