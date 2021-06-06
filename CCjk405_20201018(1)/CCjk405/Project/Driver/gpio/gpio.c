/****************************************Copyright (c)**************************************************
        ���ڰ��͵����������޹�˾
				
***FILE��gpio.c
****************************************************************************************************/
#include "gpio.h"

/* Private macro ------------------------------------------------------------*/
const GPIO_INFO_T conInput_gpio[] = 
{
	{INPUT0_GPIO_PORT,  INPUT0_GPIO_CLK,  INPUT0_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* ��ͣ״̬ */
#if 0		
	{INPUT1_GPIO_PORT,  INPUT1_GPIO_CLK,  INPUT1_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* ���������·��״̬ */
	{INPUT2_GPIO_PORT,  INPUT2_GPIO_CLK,  INPUT2_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* �����Ӵ��� */
	{INPUT3_GPIO_PORT,  INPUT3_GPIO_CLK,  INPUT3_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* ������ */
	{INPUT4_GPIO_PORT,  INPUT4_GPIO_CLK,  INPUT4_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* Aǹֱ���Ӵ��� */
	{INPUT5_GPIO_PORT,  INPUT5_GPIO_CLK,  INPUT5_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* Bǹֱ���Ӵ��� */
	{INPUT6_GPIO_PORT,  INPUT6_GPIO_CLK,  INPUT6_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* ���ʷ���Ӵ��� */
	{INPUT7_GPIO_PORT,  INPUT7_GPIO_CLK,  INPUT7_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* Aǹ��˿״̬ */
	{INPUT8_GPIO_PORT,  INPUT8_GPIO_CLK,  INPUT8_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* Bǹ��˿״̬ */
	{INPUT9_GPIO_PORT,  INPUT9_GPIO_CLK,  INPUT9_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* Aǹ���������� */
	{INPUT10_GPIO_PORT, INPUT10_GPIO_CLK, INPUT10_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* Bǹ���������� */
	{INPUT11_GPIO_PORT, INPUT11_GPIO_CLK, INPUT11_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* �Ž����� */
	{INPUT12_GPIO_PORT, INPUT12_GPIO_CLK, INPUT12_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* ���뿪��1 */
	{INPUT13_GPIO_PORT, INPUT13_GPIO_CLK, INPUT13_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* ���뿪��2 */
	{INPUT14_GPIO_PORT, INPUT14_GPIO_CLK, INPUT14_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* ���뿪��3 */
	{INPUT15_GPIO_PORT, INPUT15_GPIO_CLK, INPUT15_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* ���뿪��4 */
	{INPUT16_GPIO_PORT, INPUT16_GPIO_CLK, INPUT16_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* Aǹ���Ƶ���CC1 */
	{INPUT17_GPIO_PORT, INPUT17_GPIO_CLK, INPUT17_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},  /* Bǹ���Ƶ���CC1 */
#endif

};

const GPIO_INFO_T conLoutput_gpio[] = 
{
	{LOUTPUT0_GPIO_PORT, LOUTPUT0_GPIO_CLK, LOUTPUT0_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /*������*/
    {LOUTPUT1_GPIO_PORT, LOUTPUT1_GPIO_CLK, LOUTPUT1_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},	/*SA0 ��©���*/
    {LOUTPUT2_GPIO_PORT, LOUTPUT2_GPIO_CLK, LOUTPUT2_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},	/*SA1 ��©���*/
	{LOUTPUT3_GPIO_PORT, LOUTPUT3_GPIO_CLK, LOUTPUT3_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_OD},	/*SA2 ��©���*/
};

const GPIO_INFO_T conJoutput_gpio[] = 
{
	{JOUTPUT0_GPIO_PORT,  JOUTPUT0_GPIO_CLK,  JOUTPUT0_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* Bǹֱ���Ӵ������� */
	{JOUTPUT1_GPIO_PORT,  JOUTPUT1_GPIO_CLK,  JOUTPUT1_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* Aǹֱ���Ӵ������� */
	{JOUTPUT2_GPIO_PORT,  JOUTPUT2_GPIO_CLK,  JOUTPUT2_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* Aǹ���������� */ 
	{JOUTPUT3_GPIO_PORT,  JOUTPUT3_GPIO_CLK,  JOUTPUT3_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* �����Ӵ������� */ 
 	{JOUTPUT4_GPIO_PORT,  JOUTPUT4_GPIO_CLK,  JOUTPUT4_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* Bǹй�ŵ��� */
	{JOUTPUT5_GPIO_PORT,  JOUTPUT5_GPIO_CLK,  JOUTPUT5_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* Aǹй�ŵ��� */ 
	{JOUTPUT6_GPIO_PORT,  JOUTPUT6_GPIO_CLK,  JOUTPUT6_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* Bǹ���������� */
	{JOUTPUT7_GPIO_PORT,  JOUTPUT7_GPIO_CLK,  JOUTPUT7_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* ���ʷ������ */
	{JOUTPUT8_GPIO_PORT,  JOUTPUT8_GPIO_CLK,  JOUTPUT8_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* Bǹ����ָʾ�� */ 
	{JOUTPUT9_GPIO_PORT,  JOUTPUT9_GPIO_CLK,  JOUTPUT9_GPIO_PIN,  GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* Aǹ����ָʾ�� */
	{JOUTPUT10_GPIO_PORT, JOUTPUT10_GPIO_CLK, JOUTPUT10_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* Bǹ���ָʾ�� */
	{JOUTPUT11_GPIO_PORT, JOUTPUT11_GPIO_CLK, JOUTPUT11_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* Aǹ���ָʾ�� */
 	{JOUTPUT12_GPIO_PORT, JOUTPUT12_GPIO_CLK, JOUTPUT12_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* BMS1������Դ��ѹѡ��(����12V������24V) */
	{JOUTPUT13_GPIO_PORT, JOUTPUT13_GPIO_CLK, JOUTPUT13_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* BMS1������Դ��� */
	{JOUTPUT14_GPIO_PORT, JOUTPUT14_GPIO_CLK, JOUTPUT14_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* BMS2������Դ��ѹѡ��(����12V������24V) */
	{JOUTPUT15_GPIO_PORT, JOUTPUT15_GPIO_CLK, JOUTPUT15_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP},  /* BMS2������Դ���*/
};

const GPIO_INFO_T conLED_gpio[] = 
{
	{LED1_GPIO_PORT, LED1_GPIO_CLK, LED1_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP}, /* CPU_RUN */
  {LED2_GPIO_PORT, LED2_GPIO_CLK, LED2_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP}, /* BMS1_RUN */
  {LED3_GPIO_PORT, LED3_GPIO_CLK, LED3_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP}, /* BMS2_RUN */
	{LED4_GPIO_PORT, LED4_GPIO_CLK, LED4_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP}, /* power on */
  {LED5_GPIO_PORT, LED5_GPIO_CLK, LED5_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP}, /* charging */
  {LED6_GPIO_PORT, LED6_GPIO_CLK, LED6_GPIO_PIN, GPIO_Speed_50MHz, GPIO_PuPd_NOPULL, GPIO_OType_PP}, /* fault */	
};

#define INPUT_NUM			    (sizeof(conInput_gpio) / sizeof(GPIO_INFO_T))                          
#define LOUTPUT_NUM		    (sizeof(conLoutput_gpio) / sizeof(GPIO_INFO_T))	
#define JOUTPUT_NUM				(sizeof(conJoutput_gpio) / sizeof(GPIO_INFO_T))
#define LED_IO_NUM        (sizeof(conLED_gpio)/sizeof(GPIO_INFO_T))

/****************************************************************************************
** ��������: uint8 ReadInputDataBit(void)
** ��������: ��ȡ1· Input IO
** �䡡��: ���������
** �䡡��: ��Ӧͨ��IOֵ
** ȫ�ֱ���:
** ����:   ������Ŷ�ȡ��Ӧ������״̬
******************************************************************************************/
void LEDToggle(Led_TypeDef Led)
{
	static uint8 fg[LED_MAX] = {0};
	fg[Led] = ~fg[Led];
	GPIO_WriteBit(conLED_gpio[Led].ioport, conLED_gpio[Led].iopin, (BitAction)(fg[Led] & 0x01));
}

/****************************************************************************************
** ��������: uint8 ReadInputDataBit(void)
** ��������: ��ȡ1· Input IO
** �䡡��: ���������
** �䡡��: ��Ӧͨ��IOֵ
** ȫ�ֱ���:
** ����:   ������Ŷ�ȡ��Ӧ������״̬
******************************************************************************************/
void LEDOn(Led_TypeDef Led)
{
	GPIO_WriteBit(conLED_gpio[Led].ioport, conLED_gpio[Led].iopin, Bit_SET);
}

/****************************************************************************************
** ��������: uint8 ReadInputDataBit(void)
** ��������: ��ȡ1· Input IO
** �䡡��: ���������
** �䡡��: ��Ӧͨ��IOֵ
** ȫ�ֱ���:
** ����:   ������Ŷ�ȡ��Ӧ������״̬
******************************************************************************************/
void LEDOFF(Led_TypeDef Led)
{
	GPIO_WriteBit(conLED_gpio[Led].ioport, conLED_gpio[Led].iopin, Bit_RESET);
}

/****************************************************************************************
** ��������: uint8 ReadInputDataBit(void)
** ��������: ��ȡ1· Input IO
** �䡡��: ���������
** �䡡��: ��Ӧͨ��IOֵ
** ȫ�ֱ���:
** ����:   ������Ŷ�ȡ��Ӧ������״̬
******************************************************************************************/
uint8 ReadInputDataBit(Input_TypeDef inputn)
{
	return GPIO_ReadInputDataBit(conInput_gpio[inputn].ioport, conInput_gpio[inputn].iopin);
}

/****************************************************************************************
** ��������: WriteLoutputDataBit(Loutput_TypeDef no,BitAction val)
** ��������: 1·�����
** �䡡��: no:���������
			val:Bit_RESET = 0,
  				Bit_SET	 = 1
** �䡡��: ��
** ȫ�ֱ���:
** ����:   д��Ӧ���ֵ
******************************************************************************************/
void WriteLoutputDataBit(Loutput_TypeDef no, BitAction val)
{
	GPIO_WriteBit(conLoutput_gpio[no].ioport, conLoutput_gpio[no].iopin, val);
}

/****************************************************************************************
** ��������: WriteJoutputDataBit(Loutput_TypeDef no,BitAction val)
** ��������: ���1· �̵�������
** �䡡��: no:���������
			val:Bit_RESET = 0,
  				Bit_SET	 = 1
** �䡡��: ��
** ȫ�ֱ���:
** ����:   д��Ӧ���ֵ
//ע�⣺PE12:PE13 =  S:RESET  [1:0] ��� 0 [0:1] ��� 1
******************************************************************************************/
void WriteJoutputDataBit(Joutput_TypeDef no, BitAction val)
{
	GPIO_WriteBit(conJoutput_gpio[no].ioport, conJoutput_gpio[no].iopin, val);	
}

/****************************************************************************************
** ��������: uint8 ReadInputDataBit(void)
** ��������: ��ȡ1· Input IO
** �䡡��: ���������
** �䡡��: ��Ӧͨ��IOֵ
** ȫ�ֱ���:
** ����:   ������Ŷ�ȡ��Ӧ������״̬
******************************************************************************************/
static void LEDIOInit(Led_TypeDef Led)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  
	RCC_APB1PeriphClockCmd(conLED_gpio[Led].iobusclk, ENABLE);

	GPIO_InitStructure.GPIO_Pin = conLED_gpio[Led].iopin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = conLED_gpio[Led].iospeed;
	GPIO_InitStructure.GPIO_OType = conLED_gpio[Led].ioOType;
	GPIO_InitStructure.GPIO_PuPd = conLED_gpio[Led].ioPuPd;

	GPIO_Init(conLED_gpio[Led].ioport, &GPIO_InitStructure);
}

/****************************************************************************************
** ��������: uint8 ReadInputDataBit(void)
** ��������: ��ȡ1· Input IO
** �䡡��: ���������
** �䡡��: ��Ӧͨ��IOֵ
** ȫ�ֱ���:
** ����:   ������Ŷ�ȡ��Ӧ������״̬
******************************************************************************************/
static void Led_all_Init(void)
{
	uint8 i;
	
	for( i = 0 ; i < LED_IO_NUM; i++ ) 
	{
		LEDIOInit((Led_TypeDef)i);
		GPIO_SetBits(conLED_gpio[i].ioport,conLED_gpio[i].iopin);
	}
}

/****************************************************************************************
** ��������: 
** ��������: 
** �䡡��:  				
** �䡡��: ��
** ȫ�ֱ���:
** ����:   д��Ӧ���ֵ
******************************************************************************************/
static void InputIoInit(Input_TypeDef inputn)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  
	RCC_APB1PeriphClockCmd(conInput_gpio[inputn].iobusclk, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = conInput_gpio[inputn].iopin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = conInput_gpio[inputn].iospeed;
	GPIO_InitStructure.GPIO_OType = conInput_gpio[inputn].ioOType;
	GPIO_InitStructure.GPIO_PuPd = conInput_gpio[inputn].ioPuPd;
	
	GPIO_Init(conInput_gpio[inputn].ioport, &GPIO_InitStructure);
}

/****************************************************************************************
** ��������:  Input_IO_Init(void)
** ��������: ���뿪������ʼ��
** �䡡��: ��
** �䡡��: ��
** ȫ�ֱ���:
** ����:  ��ʼ������
******************************************************************************************/
static void Input_IO_Init(void)
{
	uint8 i = 0;
	
	for (i = 0; i < INPUT_NUM; i++)
	{
		InputIoInit((Input_TypeDef)i);
		GPIO_ResetBits(conInput_gpio[i].ioport, conInput_gpio[i].iopin);
	}	
}

/****************************************************************************************
** ��������: WriteLoutputDataBit(Loutput_TypeDef no,BitAction val)
** ��������: 1·�����
** �䡡��: no:���������
			val:Bit_RESET = 0,
  				Bit_SET	 = 1
** �䡡��: ��
** ȫ�ֱ���:
** ����:   д��Ӧ���ֵ
******************************************************************************************/
static void LoutputIoInit(Loutput_TypeDef outputn)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(conLoutput_gpio[outputn].iobusclk, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = conLoutput_gpio[outputn].iopin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = conLoutput_gpio[outputn].iospeed;
	GPIO_InitStructure.GPIO_OType = conLoutput_gpio[outputn].ioOType;
	GPIO_InitStructure.GPIO_PuPd = conLoutput_gpio[outputn].ioPuPd;

	GPIO_Init(conLoutput_gpio[outputn].ioport, &GPIO_InitStructure);
}

/****************************************************************************************
** ��������:  Loutput_IO_Init(void)
** ��������: ����������ʼ��
** �䡡��: ��
** �䡡��: ��
** ȫ�ֱ���:
** ����:  ��ʼ����������
******************************************************************************************/
static void Loutput_IO_Init(void)
{
	uint8 i = 0;
	
	for(i = 0; i < LOUTPUT_NUM; i++)
	{
		LoutputIoInit((Loutput_TypeDef)i);
		GPIO_ResetBits(conLoutput_gpio[i].ioport, conLoutput_gpio[i].iopin);
	}
}

/****************************************************************************************
** ��������: void JoutputIoInit(Joutput_TypeDef outputn)
** ��������: �̵������
** �䡡��: 				
** �䡡��: 
** ȫ�ֱ���:
** ����:   д��Ӧ���ֵ
** ע�⣺PE12:PE13 =  S:RESET  [1:0] ��� 0 [0:1] ��� 1
******************************************************************************************/
static void JoutputIoInit(Joutput_TypeDef outputn)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(conJoutput_gpio[outputn].iobusclk, ENABLE);

	GPIO_InitStructure.GPIO_Pin = conJoutput_gpio[outputn].iopin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = conJoutput_gpio[outputn].iospeed;
	GPIO_InitStructure.GPIO_OType = conJoutput_gpio[outputn].ioOType;
	GPIO_InitStructure.GPIO_PuPd = conJoutput_gpio[outputn].ioPuPd;

	GPIO_Init(conJoutput_gpio[outputn].ioport, &GPIO_InitStructure);
}

/****************************************************************************************
** ��������:  Joutput_IO_Init(void)
** ��������: �̵������������ʼ��
** �䡡��: ��
** �䡡��: ��
** ȫ�ֱ���:
** ����:  ��ʼ���̵����������
******************************************************************************************/
static void Joutput_IO_Init(void)
{
	uint8 i = 0;
	
	for(i = 0; i < JOUTPUT_NUM; i++)
	{
		JoutputIoInit((Joutput_TypeDef)i);
		GPIO_ResetBits(conJoutput_gpio[i].ioport, conJoutput_gpio[i].iopin);
	}	
}

/****************************************************************************************
** ��������: WriteLoutputDataBit(Loutput_TypeDef no,BitAction val)
** ��������: 1·�����
** �䡡��: no:���������
			val:Bit_RESET = 0,
  				Bit_SET	 = 1
** �䡡��: ��
** ȫ�ֱ���:
** ����:   д��Ӧ���ֵ
******************************************************************************************/
void InPut_OutPut_Init(void)
{
	Led_all_Init();
	Input_IO_Init();
	Loutput_IO_Init();
	Joutput_IO_Init();
}
