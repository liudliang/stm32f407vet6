#include "dma.h"																	   	  
#include "delay.h"		 
//////////////////////////////////////////////////////////////////////////////////	 
//功能：ADC+DMA多通道采集驱动代码	   
//作者：zyf
//创建日期:2020/04/16
//版本：V1.0								  
////////////////////////////////////////////////////////////////////////////////// 	 
 
//将采集到的adc值放入数组中保存
 
#define   DMA_PER      3        //外设数量
#define   DMA_NUM      10        //采集次数

static u16 DMA_Adc1Aver[DMA_PER];    //DMA_AdcAver[0]:pc0   DMA_AdcAver[1]:pc1  DMA_AdcAver[2]:pc2 
static u16 DMA_Adc1[DMA_NUM][DMA_PER]; 
 
u16 *DMA_GetAdcAver(void)
{
	return DMA_Adc1Aver; 
}


void DMA_GPIO_config()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);                          //使能GPIOC时钟
	
	//先初始化ADC1通道IO口
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 ;
	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;                                   //模拟输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);    
}

//adc初始化配置
void DMA_ADC_config()
{
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef       ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);                           //使能ADC1时钟
 
 
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,ENABLE);	                           //ADC1复位
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,DISABLE);                           //复位结束
 
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                       //独立模式
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;  //两个采样阶段之间的延迟x个时钟
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;               //DMA使能（DMA传输下要设置使能）
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;                    //预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz 
  ADC_CommonInit(&ADC_CommonInitStructure);                                      //初始化 
	
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                         //12位模式
  ADC_InitStructure.ADC_ScanConvMode =ENABLE;                                    //扫描（开启DMA传输要设置扫描）
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                             //开启连续转换（开启DMA传输要设置连续转换）
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;    //禁止触发检测，使用软件触发
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                         //右对齐	
  ADC_InitStructure.ADC_NbrOfConversion = 3;                                    //有几个通道传输就写几 （DMA传输下要设置为通道数）
  ADC_Init(ADC1, &ADC_InitStructure);                                            //ADC初始化
	
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1,  ADC_SampleTime_480Cycles);  //res[0]-PC0
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2,  ADC_SampleTime_480Cycles);  //res[1]-PC1
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3,  ADC_SampleTime_480Cycles); //res[2]-PC2
 
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);                                                         //开启AD转换器

  ADC_SoftwareStartConv(ADC1);//使ADC1开启软件启动功能	
}

//DMAx的各通道配置
//这里的传输形式是固定的,这点要根据不同的情况来修改
//从存储器->外设模式/8位数据宽度/存储器增量模式
//DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
//chx:DMA通道选择,@ref DMA_channel DMA_Channel_0~DMA_Channel_7
//par:外设地址
//mar:存储器地址
//ndtr:数据传输量  
void DMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx,u32 par,u32 mar,u16 ndtr)
{ 
  NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef  DMA_InitStructure;
	
	if((u32)DMA_Streamx>(u32)DMA2)//得到当前stream是属于DMA2还是DMA1
	{
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2时钟使能 
		
	}else 
	{
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//DMA1时钟使能 
	}
  DMA_DeInit(DMA_Streamx);
	
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}//等待DMA可配置 
	
  /* 配置 DMA Stream */
  DMA_InitStructure.DMA_Channel = chx;  //通道选择
  DMA_InitStructure.DMA_PeripheralBaseAddr = par;//DMA外设地址
  DMA_InitStructure.DMA_Memory0BaseAddr = mar;//DMA 存储器0地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;//外设模式到存储器
  DMA_InitStructure.DMA_BufferSize = ndtr;//数据传输量 (如要采2组数据，一组10个，则此处填2*10)
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//外设数据长度:16位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//存储器数据长度:16位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;// 使用循环模式 
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//中等优先级
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
  DMA_Init(DMA_Streamx, &DMA_InitStructure);//初始化DMA Stream
	
  DMA_ClearFlag(DMA2_Stream0,DMA_IT_TC);
	DMA_ITConfig(DMA2_Stream0,DMA_IT_TC,ENABLE);
	
	  //中断优先级NVIC设置
  NVIC_InitStructure.NVIC_IRQChannel=DMA2_Stream0_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01;                     //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01;                            //响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	while (DMA_GetCmdStatus(DMA2_Stream0) != DISABLE){}	
  DMA_Cmd(DMA_Streamx, ENABLE);	
} 


//整体的配置函数。配置的先后顺序，否则会导致接收的数据错位
void MYDMA_Config(void)
{ 
	DMA_GPIO_config();
	DMA_Config(DMA2_Stream0, DMA_Channel_0, (u32)&ADC1->DR,(u32)DMA_Adc1,DMA_PER*DMA_NUM);
	DMA_ADC_config();
}

void sys_ADC1_Config(void)
{
	MYDMA_Config();
}



static void DMA_filter(void)
{
	register u16 sum=0;
	u8 count=0,i=0,j=0;
	for(;i<DMA_PER;i++)
	{
		while(j<DMA_NUM)
		{
			if(DMA_Adc1[j][i]<0)
      {
			}
			else
			{
			  sum+=DMA_Adc1[j][i];
				count++;
			}
		  j++;
		}
		DMA_Adc1Aver[i]=sum/count;
		sum=0;
		count=0;
		j=0;
	}
}


//DMA传输开启函数
//当dma数据填充满时触发此中断函数
//DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7 
//ndtr:数据传输量  
void DMA2_Stream0_IRQHandler(void) 
{
	if (DMA_GetFlagStatus(DMA2_Stream0, DMA_IT_TCIF0) == SET)  
	{
		DMA_filter();
		DMA_ClearFlag(DMA2_Stream0, DMA_IT_TCIF0); 
	}

}



















