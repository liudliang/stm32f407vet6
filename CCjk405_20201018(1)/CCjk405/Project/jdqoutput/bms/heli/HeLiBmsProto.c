/**
  ******************************************************************************
  * @file    helibmsproto.c
  * @author  
  * @version v1.0
  * @date    2020-11-04
  * @brief   
  ******************************************************************************
  */
#include "main.h"	
#include "gpio.h"
#include "TaskRealCheck.h"
#include "TaskMainCtrl.h"
#include "Bms.h"
#include "TaskMainCtrl.h"
#include "Screen.h"
#include "HeLibmsProto.h"
#include "BmsCan.h"


#if 0
const RCV_PROTO_ST g_heli_RcvProtoList[] = \
{
	{0x10, 0xFF, 5	,  1000, ProcBCL, NULL},      /*BCL*/
	{0x11, 0xFF, 9	,  1000, ProcBCS, NULL},      /*BCS*/
	{0x13, 0xFF, 7	,  1000, ProcBSM, NULL},      /*BSM*/
  
	{0xff, 0xFF, 0xff, 0xff,NULL,NULL},
};
#endif

//��ʱ�ط��ȱ�־��5���ӣ�
static uint8 s_delay_off_fan_flag = 0;
static uint32 s_delay_off_fan_ticks = 0;



void Heli_delay_off_fan_flag_set(void)
{
	s_delay_off_fan_flag = 1;
	s_delay_off_fan_ticks = GetSystemTick();
}


SEND_PROTO_ST g_heli_ChgProtoList[] = \
{
  {SINGLE_FRM,CRICLE_DISALLOWED,0x12,6,8,WAITMS(100), 1000,0,heli_bms_send_ccs},  /*CCS*/
  {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,NULL}, /*������뱣��*/
};

SEND_PROTO_ST *heli_get_proto_list(void)
{
	return &g_heli_ChgProtoList[0];
}

extern void Gun_StartPara(void);
extern void TaskMian_SendProcessMsg(uint8 gunNo,uint8 statu,uint8 reason,uint8 errcode);
extern void TskMain_SetStoping(uint8 step,uint8 fg);
extern DEV_INPUT_TYPE *Check_GetInputDataptr(uint8 gunnum);

//�����ǹ�½���
extern void ClearGunFallingEdge(void);

static uint8 DrawGunFallingEdge = 0;
static uint8 GunCC1LevelState[2] = {1, 1};

static void DetermineGunFallingEdge(void)
{
	static uint8 index = 0;

	if (1 == DrawGunFallingEdge)
	{
		return;    //�Ѿ����½��ؾͲ����ж���
	}
	
	if (CC1_4V != Check_CC1Statu(AGUN_NO))
	{
		GunCC1LevelState[index] = 1;
	}
	else
	{
		GunCC1LevelState[index] = 0;
	}
	
	if (0 == GunCC1LevelState[0])
	{
		index = 0; //�����
		return;
	}
  else
	{
		index = 1; //��һ��״̬���ж�
	}
	
	if ((1 == GunCC1LevelState[0]) && (0 == GunCC1LevelState[1]))
	{
		DrawGunFallingEdge = 1;   //�жϵ��½���
		
		//��λ״̬
		index = 0; //�����
		GunCC1LevelState[0] = 1;
		GunCC1LevelState[1] = 1;
	}
}
	
static void heli_check_bms_timeout(void)
{	
	if(TskMain_GetWorkStep(AGUN_NO) == STEP_ADJUST_SUB)
	{
	  if(GetSystemTick() - Bms_GetStartTimeCount(0,BCL_TM) > SECS2TICKS(ChgData_GetRunParamPtr()->bcltm) ) {
			Check_SetErrCode(0,ECODE7_BCLTM);
		}
		
		if(GetSystemTick() - Bms_GetStartTimeCount(0,BCS_TM) > SECS2TICKS(ChgData_GetRunParamPtr()->bcstm) ) {
			Check_SetErrCode(0,ECODE8_BCSTM);
		}
	}
}

static 	uint8 errBuf[MAX_ERRBUF_SIZE] = { 0 },Reason = 0;

//�˺�����������1500ms��ѭ���С�����Э��Ҫ���ǹ���ܳ��
void heli_polling(void)
{
	static uint8 cnt = 0,s_chargflag = 0;
	DEV_INPUT_TYPE *pInputDataptr = Check_GetInputDataptr(AGUN_NO);
	
	if (TskMain_GetWorkStep(AGUN_NO) == STEP_IDEL)
	{
		if(((1 == DrawGunFallingEdge) && (0 == pInputDataptr->statu.bits.stop)) && (4 == cnt++))  //���½��ز��Ҽ�ͣ��ť������������
		{
			cnt = 0;
			Gun_StartPara();
		}
	}
	
	if(1 == pInputDataptr->statu.bits.stop)
	{
		Check_SetErrCode(AGUN_NO,ECODE89_CHGJTST);  //��ͣ����
		Check_GetErrBit(AGUN_NO,&errBuf[0],MAX_ERRBUF_SIZE,&Reason);
		if((STEP_CHGEND != TskMain_GetWorkStep(AGUN_NO)) && (STEP_WAITOVER != TskMain_GetWorkStep(AGUN_NO)))
		{
			TaskMian_SendProcessMsg(0,E_ERROR,errBuf[0],errBuf[0]);
		}
	}
	else
	{
		Check_ClearErrBit(AGUN_NO,ECODE89_CHGJTST);
	}
	
	
	if (CC1_4V != Check_CC1Statu(AGUN_NO))
	{
		Check_SetErrCode(AGUN_NO,ECODE18_CC1LNK);    //ǹδ����
		Check_GetErrBit(AGUN_NO,&errBuf[0],MAX_ERRBUF_SIZE,&Reason);
//		TaskMian_SendProcessMsg(0,E_ERROR,errBuf[0],0);
		
		ClearGunFallingEdge();           /*�����ǹ�½��� */
	}
	else
	{
		Check_ClearErrBit(AGUN_NO,ECODE18_CC1LNK);
	}
	
	DetermineGunFallingEdge();  //Ѱ���½���
//	Delay10Ms(150);
	
	heli_check_bms_timeout();
	
//-----------------���ȴ���	
	if(1 == s_delay_off_fan_flag)
	{
		if(GetSystemTick() > (s_delay_off_fan_ticks+5*60*TIM_1S))
		{
			s_delay_off_fan_flag = 0;
		}
	}
	if((STEP_WAITOVER == TskMain_GetWorkStep(0)) || (STEP_WAIT_SUB == TskMain_GetWorkStep(0)) || (STEP_IDEL == TskMain_GetWorkStep(0)))
	{
		if(0 == s_delay_off_fan_flag)
		{
			RelayOut_AcKmOut(JOUT_OFF);
		}
	}
//-----------------

	
}

//�����ǹ�½���
void ClearGunFallingEdge(void)
{
	DEV_INPUT_TYPE *pInputDataptr = Check_GetInputDataptr(AGUN_NO);
	
//	if (1 == pInputDataptr->statu.bits.stop)
//	{
//		return;
//	}
	
	DrawGunFallingEdge = 0;
}




//----------------����Э�鲿��
extern void ProBmsStepAdjustCurr(void);
void heli_proc_bcl(RCV_PROTO_ST *item,uint8 *data,uint8 gunnum)
{
	BCL_ST tmpbcl = { 0 };
	BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(gunnum);
	PARAM_OPER_TYPE *ptrRunParam = ChgData_GetRunParamPtr();
	const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	stoc_u tmp16;
	
	if ((NULL == ptrItem) || (ptrItem->pgn != BCL_CODE)) 
	{ 
		return; 
	}
	ptr->bcl.needvolt = 0;
	ptr->bcl.needcurr = 0;
	ptr->bcl.chgmode  = 0;
	Bms_SetStepErrFg(AGUN_NO,BCL_TM,0);
	Bms_StartTimeCount(AGUN_NO,BCL_TM,GetSystemTick());
	Bms_StartTimeCount(AGUN_NO,BCS_TM,GetSystemTick());
	tmp16.c[0] = data[1];
	tmp16.c[1] = data[0];
	tmpbcl.needvolt = tmp16.s;  /*��ѹ����10��*/
	tmp16.c[0] = data[3];
	tmp16.c[1] = data[2];
	tmpbcl.needcurr =  tmp16.s;//CURR_PROTO(tmp16.s); /*��������10��*/
	tmpbcl.chgmode = 0x02;
	ptr->bcl.needvolt = tmpbcl.needvolt;
	ptr->bcl.needcurr = tmpbcl.needcurr;
	ptr->bcl.chgmode  = tmpbcl.chgmode;
	
	ptr->bcp.chgmaxvolt = tmpbcl.needvolt;
	/* �����泵Э�� */
	if((0x01 == data[4]) || (0x01 == data[6]))
	{
//		if(0x01 == data[6])
//		{
//			ptr->bst.stoprsn.bits.socok = 1;
//		}
//		else
		{
			ptr->bst.heli_faultrsn.byte = data[6];
		}
		//�����ط��͹����Ĺ���data[6]������**
		ClearGunFallingEdge();     //�����ǹ�½���
		return;
	}
	/* 2018��11��08��Ϊ��Ӧ�����BMS�����޸ģ���BMS�����ѹ��ʱ�ᳬ��BCP����ѹ���ʽ��ж�����ȥ�� */ 
	/* tmpbcl.needvolt > ptr->bcp.chgmaxvolt */
	/*���������� > 755V  <100V ��Ϊ���ݳ���*/
	if (tmpbcl.needvolt > ptrRunParam->maxvolt || tmpbcl.needvolt < ptrRunParam->minvolt) { 
		 return;
	} 
	/* 2018��11��09��Ϊ��Ӧ�����BMS�����޸ģ���BMS�������ֵ��ʱ�����߼��������ʽ��ж�����ȥ�� */ 
	if (tmpbcl.needcurr > 4000) {
		return;
	}
	if (GET_WORKSTEP(AGUN_NO) == STEP_CHARGEING) 
	{
		ProBmsStepAdjustCurr();
	}
	Delay5Ms(1); 
	return;	
}

void heli_proc_bsm(RCV_PROTO_ST *item,uint8 *data,uint8 gunnum)
{
	BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(gunnum) ;
	const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	
	if ( (NULL == ptrItem) || (ptrItem->pgn != BSM_CODE)) 
    { 
		return; 
    } 
//	 memcpy(&ptr->bsm,data,sizeof(BSM_ST));
//	 ptr->bsm.batmaxtemp -= 50; /*�¶�ƫ��50*/
//	 ptr->bsm.batmintemp -= 50; 
	ptr->bsm.batmintemp  = data[3];
	ptr->bsm.batmintemp -= 40;
	if(ptr->bsm.battalm.bits.singbatValm != 0x00 || ptr->bsm.battalm.bits.socalm == 0x01 \
		|| ptr->bsm.battalm.bits.tempalm == 0x02 || ptr->bsm.battalm.bits.linkalm == 0x01 \
	    || ptr->bsm.battalm.bits.curralm == 0x01 || ptr->bsm.battalm.bits.insulalm == 0x01 ) 
	{
		if (ptr->bsm.battalm.bits.allowed == 0x00 ) 
		{ /*��������*/
			/*���ù���λ*/
			Bms_SetStepErrFg(gunnum,DISALLOW_FG,1);
		 /***************************************************************************
			������Ϣ������ͣ��
		 *****************************************************************************/
		} 
	}
	else 
	{
		Bms_SetStepErrFg(gunnum,DISALLOW_FG,0);
	}
	 return;	
}

void heli_proc_bcs(RCV_PROTO_ST *item,uint8 *data,uint8 gunnum)
{
	stoc_u tmp16;
	BMSDATA_ST *ptr = (BMSDATA_ST *)Bms_GetBmsCarDataPtr(gunnum) ;
	const RCV_PROTO_ST *ptrItem = (const RCV_PROTO_ST *)item;
	
	if ( (NULL == ptrItem) || (ptrItem->pgn != BCS_CODE)) 
	{ 
		 return; 
	}
	Bms_SetStepErrFg(gunnum,BCS_TM,0);
	Bms_StartTimeCount(gunnum,BCS_TM,GetSystemTick());
//	memcpy(&ptr->bcs,data,sizeof(BCS_ST));
//	ptr->bcs.deteccurr = CURR_PROTO(ptr->bcs.deteccurr); 
    ptr->bcs.cursoc = data[4] * 0.4;
    ptr->bsm.batmaxtemp = data[5] - 40;
	tmp16.c[0] = data[7];
	tmp16.c[1] = data[6];
	ptr->bcp.batcurvolt = tmp16.s;
	Delay5Ms(1);
	return;	
}

void heli_deal_bms_msg(CAN_MSG canmsg,uint8 gunnum)
{
	RCV_PROTO_ST item = {0};
	
	switch(canmsg.Id.ExtId)
	{
		case 0x1806E5F4:  //BCL
			item.pgn = BCL_CODE;
			heli_proc_bcl(&item, canmsg.Data, gunnum);
			break;
		case 0x1806E6F4:  //BSM
			item.pgn = BSM_CODE;
			heli_proc_bsm(&item, canmsg.Data, gunnum);
			break;
		case 0x1800FFF4:  //BCS
			item.pgn = BCS_CODE;
			heli_proc_bcs(&item, canmsg.Data, gunnum);
			break;
		default:
			break;
	}
}

void heli_bms_send_data(_t_heli_send_data_info *sendinf, uint8 *pbuf, uint16 length)
{
	CAN_MSG Txmsg = { 0 };
	
	Txmsg.FrmDef.FF = CAN_FRAME_EXTERNED;
	Txmsg.FrmDef.RTR = CAN_DATA_TYPE_FRAME;
	memcpy(Txmsg.Data, &pbuf[0], length);
	Txmsg.DLC = length;
	Txmsg.Id.ExtId = sendinf->ID;
	Bms_CanWrite(&Txmsg, sendinf->gunNo);
	Delay5Ms(1);	
}

typedef struct
{
	uint8 gunNo;
	uint8 pf;
	uint32 ID;
}_t_heli_send_info;


//bit0:0���� 1:Ӳ������  bit1:0���� 1:�����¶ȹ���  bit2:0���� 1:�����ѹ����
//bit3:0�������⵽��ص�ѹ��������״̬ 1:�ر�״̬   bit4:0���� 1:ͨѶ���ܳ�ʱ
uint8 heli_check_fault_status(gunnum)
{
	uint8 rtn = 0;
	
	if( (1 == ChgData_GetRunDataPtr(gunnum)->input->statu.bits.stop) || (1 == ChgData_GetRunDataPtr(gunnum)->meter->statu.bits.commerr)) 
	{
		rtn += (1<<0);
	}	
	
//	if(1 == ChgData_GetRunDataPtr(gunnum)->gun->statu.bits.overtemper) 
//	{
//		rtn += (1<<1);
//	}
	
	if( (1== ChgData_GetRunDataPtr(gunnum)->AcMet->statu.bits.UDeficy) || (1== ChgData_GetRunDataPtr(gunnum)->AcMet->statu.bits.UabOver) \
		||(1== ChgData_GetRunDataPtr(gunnum)->AcMet->statu.bits.UabUnder) )
	{
		rtn += (1<<2);
	}
	
	switch(TskMain_GetWorkStep(gunnum))
	{
		case STEP_LINK:
		case STEP_BPOWER:	
		case STEP_BRO_SUB:
		case STEP_ADJUST_SUB:	
		case STEP_CHARGEING:	
			rtn += (1<<3);
			break;
		default:
			break;
	}
	
	return rtn;
	
}

uint8 heli_bms_send_ccs(uint8 *buf,void* st,uint8 gunnum)
{
	stoc_u tmp16 = { 0 };
	uint8 index = 0;
	_t_heli_send_info *pCtrl =(_t_heli_send_info *)st; 
	CHGDATA_ST *pdata = Bms_GetChgDataPtr(gunnum);
	
	if ((NULL != st) || (pCtrl->pf == CCS_CODE))
	{
		tmp16.s = VOLT_PROTO(pdata->ccs.outvolt);
		buf[index++] = tmp16.c[1];
		buf[index++] = tmp16.c[0];
		
		tmp16.s = CURR_PROTO(pdata->ccs.outcurr);
		buf[index++] = tmp16.c[1];
		buf[index++] = tmp16.c[0];
		

		buf[index++] = heli_check_fault_status(gunnum);
		
		buf[index++] = 0;
		buf[index++] = 0;
		buf[index++] = 0;
		
		pCtrl->gunNo = gunnum;
		pCtrl->ID = 0x18FF50E5;
	}
	
	return index;	
}

