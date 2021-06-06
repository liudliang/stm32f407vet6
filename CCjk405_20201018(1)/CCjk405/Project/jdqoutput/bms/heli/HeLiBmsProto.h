/**
  ******************************************************************************
  * @file    helibmsproto.h
  * @author  
  * @version v1.0
  * @date    
  * @brief   
  ******************************************************************************
	*/
#ifndef _HELIBMSPROTO_H
#define _HELIBMSPROTO_H
#include "GbtBmsProto.h"
#include "can.h"

//������ʾ���롣ע�ⲻҪ����꣨E_ERR_CODE���������غ�
typedef enum
{
	HELI_ECODE46_CONNECTOR_TEMPERATURE_LOSS = 46,  /*������¶ȶ�ʧ*/
	HELI_ECODE47_OTHER = 47,                       /*����*/
	HELI_ECODE48_OUTSIDE_KM_40LESS = 48,			 /*��ص�ѹ����40V*/
	HELI_ECODE49_BMS_STOP_NORMOL = 49,			   /*��ط��ͳ�����*/
	
	HELI_ECODE109_CHARG_VOL_OVER = 109,              /*��ѹ���䱣��*/
	HELI_ECODE110_OVER_TEMPERATURE = 110,            /*���±���*/
	HELI_ECODE111_INTERLOCK = 111,                   /*��������*/
	HELI_ECODE112_LOWER_TEMPERATURE = 112,           /*���³�籣��*/
	HELI_ECODE113_CELL_VOL_LOWER = 113,              /*�����ѹ����*/
	HELI_ECODE114_CURR_OVER = 114,                   /*����������*/
	HELI_ECODE115_BMS_FAULT = 115,                   /*BMS ���ϱ���*/
	HELI_ECODE116_CONNECTOR_OVER_TEMPERATURE = 116,  /*��������±���*/
	HELI_ECODE117_CC2_FAULT = 117,                   /*��� CC2 �쳣*/
	HELI_ECODE118_CELLS_VOL_DIFF = 118,              /*����ѹ�����*/
	HELI_ECODE119_CELLS_TEMPERATURE_DIFF = 119,      /*�����²����*/

}E_ERR_CODE_HELI;



typedef struct
{
	uint8 gunNo;
	uint8 pf;
	uint32 ID;
}_t_heli_send_data_info;

void heli_deal_bms_msg(CAN_MSG canmsg,uint8 gunnum);
void heli_bms_send_data(_t_heli_send_data_info *sendinf, uint8 *pbuf, uint16 length);
SEND_PROTO_ST *heli_get_proto_list(void);
void heli_polling(void);
uint8 heli_bms_send_ccs(uint8 *buf,void* st,uint8 gunnum);
void Heli_delay_off_fan_flag_set(void);
#endif



