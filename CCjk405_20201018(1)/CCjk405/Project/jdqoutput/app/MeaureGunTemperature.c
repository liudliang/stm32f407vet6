/**
  ******************************************************************************
  * @file    MeaureGunTemperature.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
*/
#include "MeaureGunTemperature.h"
#include "gpio.h"
#include "Adc_Calc.h"

#define MTVREF   3.3f   
/* kemit 科敏 4200阻值表 */
/* Private functions ---------------------------------------------------------*/
static const uint32 TableT[] =
{
	4081681, 3807481, 3553165, 3317188, 3098153, 2894768, 2705846, 2530291, 2367098, 2215339,   /*-40 ~ -31 */
	2074159, 1942772, 1820451, 1706528, 1600386, 1501455, 1409212, 1323170, 1242883, 1167936,    /*-30 ~ -21 */
	1097949, 1032568, 971466,  914342,  860917,  810933,  764149,  720345,  679315,  640870,    /*-20 ~ -11 */
	604833,  571041,  539342,  509595,  481670,  455446,  430810,  407657,  385891,  365421,    /*-10 ~ -1 */
	346162,  328038,  310974,  294903,  279762,  265492,  252038,  239350,  227379,  216081,    /*0 ~ 9 */
	205414,  195341,  185824,  176830,  168327,  160645,  153014,  145788,  138942,  132456,    /*10 ~ 19 */
	126308,  120479,  114950,  109704,  104726,  100000,  95512,   91249,   87199,   83349,     /*20 ~ 29 */	                                 
	79689,   76209,   72898,   69748,   66750,   63896,   61179,   58589,   56123,   53772,     /*30 ~ 39 */	   
	51483,   49350,   47316,   45377,   43527,   41762,   40077,   38469,   36934,   35468,     /*40 ~ 49 */	
	34067,   32729,   31450,   30228,   29059,   27942,   26873,   25850,   24871,   23935,     /*50 ~ 59 */	
	23038,   22179,   21357,   20569,   19814,   19090,   18397,   17732,   17094,   16483,     /*60 ~ 69 */	
	15896,   15333,   14793,   14275,   13777,   13260,   12804,   12366,   11946,   11543,     /*70 ~ 79 */	
	11155,   10783,   10425,   10081,   9751,    9433,    9127,    8833 ,   8550,    8278,      /*80 ~ 89 */	
	8024,    7772,    7529 ,   7296,    7070,    6853,    6643,    6441,    6246,    6057,      /*90 ~ 99 */	
	5876,    5700,    5531,    5367,    5209,    5056,    4909,    4766,    4629,    4496,      /*100 ~ 109 */	
	4367,    4242,    4699,    4006,    3893,    3778,    3673,    3571,    3473,    3378,      /*110 ~ 119 */	
	3286,    3197,    3111,    3028,    2947,    2869,    2793,    2720,    2649,    2580,      /*120~  129 */	
	2513,																																				                /*130*/	
};

static uint8_t TABLE_NUM = sizeof(TableT) / sizeof(uint32);

/**
* @brief  SetCD4051BMTChannel
* @param  None
* @retval None
*/
static void CD4051BM_Delay(void)
{
	uint32_t n = 10000;
	
	while (n--)
	{
	}
}
	
/**
* @brief  SetCD4051BMTChannel
* @param  None
* @retval None
*/
void SetCD4051BMTChannel(uint8_t ch)
{
	switch(ch)
	{
		case 0:
			WriteLoutputDataBit(LOUTPUT1_SA0, Bit_RESET);
			WriteLoutputDataBit(LOUTPUT2_SA1, Bit_RESET);
			WriteLoutputDataBit(LOUTPUT3_SA2, Bit_RESET);
			break;
		case 1:
			WriteLoutputDataBit(LOUTPUT1_SA0, Bit_SET);
			WriteLoutputDataBit(LOUTPUT2_SA1, Bit_RESET);
			WriteLoutputDataBit(LOUTPUT3_SA2, Bit_RESET);
			break;
		case 2:
			WriteLoutputDataBit(LOUTPUT1_SA0, Bit_RESET);
			WriteLoutputDataBit(LOUTPUT2_SA1, Bit_SET);
			WriteLoutputDataBit(LOUTPUT3_SA2, Bit_RESET);
			break;
		case 3:
			WriteLoutputDataBit(LOUTPUT1_SA0, Bit_SET);
			WriteLoutputDataBit(LOUTPUT2_SA1, Bit_SET);
			WriteLoutputDataBit(LOUTPUT3_SA2, Bit_RESET);
			break;
		case 4:
			WriteLoutputDataBit(LOUTPUT1_SA0, Bit_RESET);
			WriteLoutputDataBit(LOUTPUT2_SA1, Bit_RESET);
			WriteLoutputDataBit(LOUTPUT3_SA2, Bit_SET);
			break;
		case 5:
			WriteLoutputDataBit(LOUTPUT1_SA0, Bit_SET);
			WriteLoutputDataBit(LOUTPUT2_SA1, Bit_RESET);
			WriteLoutputDataBit(LOUTPUT3_SA2, Bit_SET);
			break;
		case 6:
			WriteLoutputDataBit(LOUTPUT1_SA0, Bit_RESET);
			WriteLoutputDataBit(LOUTPUT2_SA1, Bit_SET);
			WriteLoutputDataBit(LOUTPUT3_SA2, Bit_SET);
			break;
		case 7:
			WriteLoutputDataBit(LOUTPUT1_SA0, Bit_SET);
			WriteLoutputDataBit(LOUTPUT2_SA1, Bit_SET);
			WriteLoutputDataBit(LOUTPUT3_SA2, Bit_SET);
			break;
	}
	
	CD4051BM_Delay();   //模拟开关的通道选择延时
}

#if 0
#define N 50 //每通道采50次
#define M 5 //为12个通道

extern u16 AD_Value[N][M]; //用来存放ADC转换结果，也是DMA的目标地址
extern u16 After_filter[M]; //用来存放求平均值之后的结果
extern u16 AdcValue[5];


u16 GetVolt(u16 advalue)
{

	return (u16)(advalue * 330 / 4096); //求的结果扩大了100倍，方便下面求出小数

}




void filter(void)
{
	int sum = 0;
	u8 count;
	for(int i=0;i<M;i++)

	{

	for ( count=0;count<N;count++)

	{

	sum += AD_Value[count][i];

	}

	After_filter[i]=sum/N;

	sum=0;
	}

}



/**
* @brief  GetADC1Channel6Value
* @param  None
* @retval None
*/
uint16_t GetADC1ChannelValue(void)
{
	uint16_t ADC1_CH6_Value = 0;
	u16 value[M];


	

	while (ADC_GetSoftwareStartConvStatus(ADC1) != RESET);
//	ADC_SoftwareStartConv(ADC1);
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

	ADC1_CH6_Value = ADC_GetConversionValue(ADC1);

	return ADC1_CH6_Value;
	
	
//	if(DMA_GetFlagStatus(DMA2_Stream0,DMA_FLAG_TCIF0)!=RESET)	/*传输完成*/
//	{

//		printf("adc0=%d ",AdcValue[0]);
//		printf("adc1=%d ",AdcValue[1]);
//		printf("adc2=%d ",AdcValue[2]);
//		printf("adc3=%d ",AdcValue[3]);
//		printf("adc4=%d ",AdcValue[4]);
//	
//		DMA_ClearFlag(DMA2_Stream0,DMA_FLAG_TCIF0);/*清零*/
//	}
//	return AdcValue[0];
}
#endif





/**
* @brief  CalculationRes
* @param  None
* @retval None
*/
static float CalculationRes(uint8 ch)
{
	//通过AD值计算外部热敏电阻的值
	float res = 0;
	float voltage = 0;
	float advalue = 0;
	float current = 0;

#if 0	
	advalue = (float)GetADC1Channel6Value();
#else
//	advalue = AdcValue[0];
    advalue = AdcCalc_GetValue()->guntemprature[ch];

#endif
	
	voltage = (advalue / 4096.0f) * MTVREF;
	
	current = (MTVREF - voltage) / (10 * 1000);

  res = voltage / current;
	
	return res;
}

/**
* @brief  GetCD4051BMTChannelRes
* @param  None
* @retval None
*/
static float GetCD4051BMTChannelRes(uint8 ch)
{
	float channelRes = 0;
	ch %= 8;
//	SetCD4051BMTChannel(ch);
	
	channelRes = CalculationRes(ch);
	
  return channelRes;	
}

/**
* @brief  InquireTableTemper
* @param  None
* @retval None
*/
static uint8 InquireTableTemper(uint32 Resistance)
{
	uint8 Low = 0, High = TABLE_NUM - 1, Mid = 0;
	uint32 MidVal = 0;
	
	if (Resistance >= TableT[0])
	{
		Mid = 0;
		return Mid;
	}
	
	if (Resistance <= TableT[TABLE_NUM - 1])
	{
		Mid = TABLE_NUM - 1;
		return Mid;
	}
		
	while (Low <= High)
	{
		Mid = (Low + High) / 2;
		MidVal = TableT[Mid];
		
		if (Resistance > MidVal)
		{
				High = Mid - 1;
		}
		else if (Resistance < MidVal)
		{
				Low = Mid + 1;
		}
		else
		{
				return Mid;
		}
	}
	
	return Mid;
}

/**
* @brief  GetCD4051BMTChannelTemper
* @param  None
* @retval None
*/
int32_t GetCD4051BMTChannelTemper(uint8_t ch)
{
	float NtcRes = 0;
	int32_t temper = 0;
	
	ch %= 8;
	
	NtcRes = GetCD4051BMTChannelRes(ch);
	
	temper = InquireTableTemper((uint32_t)(NtcRes * 10));
	
	temper -= 40;
	
	return temper;
}
