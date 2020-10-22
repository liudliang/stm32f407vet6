/**
  ******************************************************************************
  * @file    Slavefunc.c
  * @author  zqj
  * @version v1.0
  * @date    2018-04-09
  * @brief   
  ******************************************************************************
	*/
#include "main.h"	 
#ifdef SUB_SLAVE	


#include "common.h"
#include "ChgData.h"
#include "Slavefunc.h"
#include "string.h"
#include "Msproto.h"
#include "TaskSlaveComm.h"


uint16 sfunc_GetStatuWord(void);

uint8 proc_powerqry11(uint8 * buf);
uint8 pkg_powerqry11(uint8 * buf);

uint8 proc_realupdate12(uint8 * buf);
uint8 pkg_realupdate12(uint8 * buf);

uint8 proc_startparam13(uint8 * buf);
uint8 pkg_startparam13(uint8 * buf);

uint8 proc_stopparam14(uint8 * buf);
uint8 pkg_stopparam14(uint8 * buf);

uint8 pkg_everygroupFF(uint8 * buf);

uint8 proc_syndevpara21(uint8 *buf);
uint8 proc_synratedevpara22(uint8 *buf);
uint8 proc_syntime23(uint8 *buf);
uint8 proc_synrecord31(uint8 *buf);
uint8 proc_synupgrade(uint8 *buf);


const MSCMD_PGN_PROC_ST gMs01ProcList[] = \
{
	{POWER_QUERY_11,0,proc_powerqry11,  pkg_powerqry11},
  {REAL_UPDATE_12,0,proc_realupdate12,pkg_realupdate12},
  {START_PARAM_13,0,proc_startparam13,pkg_startparam13},
  {STOP_PARAM_14 ,0,proc_stopparam14, pkg_stopparam14},
  {EVERY_GROUP_FF,0,NULL,pkg_everygroupFF},
  {NOT_EXSIT_PGN ,0,NULL,NULL},
};

const MSCMD_PGN_PROC_ST gMs02ProcList[] = \
{
	{DEV_PARA_21,0,proc_syndevpara21,NULL},
  {RATE_PARA_22,0,NULL,NULL},
  {DTIME_PARA_23,0,NULL,NULL},
  {NOT_EXSIT_PGN,0,NULL,NULL},
};

const MSCMD_PGN_PROC_ST gMs03ProcList[] = \
{
	{READ_RECORD_31,0,NULL,NULL},
  {NOT_EXSIT_PGN,0,NULL,NULL},

};

const MSCMD_PGN_PROC_ST gMs05ProcList[] = \
{
	{READY_UPGRADE_51,0,NULL,NULL},
  {START_UPGRADE_52,0,NULL,NULL},
  {NOT_EXSIT_PGN,0,NULL,NULL},
};


const MSCMD_PGN_PROC_ST *Slave_GetPgnProc(uint8 pgn,const MSCMD_PGN_PROC_ST *pgninfo)
{
	uint8 i;
	if( NULL != pgninfo ) {
		for( i = 0; pgninfo[i].pgn != NOT_EXSIT_PGN;i++)
		{
			if( pgn == pgninfo[i].pgn ) {
				 return  (pgninfo+i);
			}
		}
	}
	return NULL;
}


/*****************************CMD_QUERYDATA_01*****************************************/
//	{POWER_QUERY_11,0,NULL},
//  {REAL_UPDATE_12,0,NULL},
//  {START_PARAM_13,0,NULL},
//  {STOP_PARAM_14,0,NULL},
//  {EVERY_GROUP_FF,0,NULL},
//  {NOT_EXSIT_PGN,0,NULL},
	
/*功率请求，分体桩无此项*/
uint8 proc_powerqry11(uint8 * buf)
{
	return 0;
}

uint8 pkg_powerqry11(uint8 * buf)
{
	return 0;
}


/*实时上报，分体桩无此项*/
uint8 proc_realupdate12(uint8 * buf)
{
	return 0;
}

uint8 pkg_realupdate12(uint8 * buf)
{
	return 0;
}


/*启动消息字节数可能比较大*/
typedef struct
{
	uint8 fg; /**/
	uint8 gunno;
	uint8 ntype;
	uint8 bytenum; /*必须小于32*/
	uint8 data[32]; 
	uint8 rsv[4]; 
}START_MESSAGE;

START_MESSAGE gStatMessage;

/*启动参数*/
uint8 proc_startparam13(uint8 * buf)
{
	 if( gStatMessage.fg == 0x01 ) {
		 return 0;	 
	 }
	 
	 gStatMessage.fg = 1;    /*由处理函数清零*/
	 gStatMessage.gunno = buf[0];
	 gStatMessage.ntype = buf[1];
	 gStatMessage.bytenum = buf[2] % 33;
	 memset( gStatMessage.data,0,36);
	 memcpy(gStatMessage.data,&buf[3],gStatMessage.bytenum);
	 
	 /*在此添加消息，将全局启动结构地址（因消息字节过多，定义全局结构处理）
	 传递过去，使用完成后 fg标志清零*/
	return 0;
}

uint8 pkg_startparam13(uint8 * buf)
{
	return 0;
}

/*停止参数*/
uint8 proc_stopparam14(uint8 * buf)
{
	return 0;
}

uint8 pkg_stopparam14(uint8 * buf)
{
	return 0;
}

/*轮寻命令*/
uint8 proc_everygroupFF(uint8 * buf)
{
	return 0;
}

uint8 sfunc_pkgoneparam(uint8 no,uint8 *buf )
{
	uint8 len = 0;
	switch(no)
	{
		case 0:
		case 2:
			len = pkg_startparam13(buf);
			break;
		case 1:
		case 3:
			len = pkg_stopparam14(buf);
			break;
		case 4:
			len = pkg_powerqry11(buf);
			break;
		case 5:
			len = proc_realupdate12(buf);
			break;
		default:
			len = 0;
			break;
	}
	return len;
}


static START_STU_ST sStatuWord = {0x0000};
uint8 pkg_everygroupFF(uint8 * buf)
{
	 uint8 pkglen;
	 uint16 i;
	 uint16 tmp16 = 0;
	 START_STU_ST tmpstu;
	 tmpstu.word = sfunc_GetStatuWord()&0x003f;
	 if(sStatuWord.word != tmpstu.word) {
		 tmp16 = sStatuWord.word ^ tmpstu.word ;
		 for( i = 0 ; i < 6 ; i++ ) {
        if(tmp16 & ((uint16)(1 << i)) ) {
					if( (0 == i) || (2 == i) ) {
						 pkglen = sfunc_pkgoneparam(i,buf);
						 if(tmpstu.word & 0x0001 ) {
								sStatuWord.word |= 0x0001 ;
						 }else {
							  sStatuWord.word &= ~(0x0001<<i) ;
						 }
					}else if ( (1 == i) || (3 == i) ) {
						 pkglen = sfunc_pkgoneparam(i,buf);
						 if(tmpstu.word & 0x0002 ) {
								sStatuWord.word |= 0x0002 ;
						 }else {
							  sStatuWord.word &= ~(0x0002<<i) ;
						 }
					}else if ( 4 == i ) {
						 pkglen = sfunc_pkgoneparam(i,buf);
						 if(tmpstu.word & 0x0004 ) {
								sStatuWord.word |= 0x0004 ;
						 }else {
							  sStatuWord.word &= ~(0x0004<<i) ;
						 }
					}else if( 5 == i ) {
						 pkglen = sfunc_pkgoneparam(i,buf);
						 if(tmpstu.word & 0x0020 ) {
								sStatuWord.word |= 0x0020 ;
						 }else {
							  sStatuWord.word &= ~(0x0020<<i) ;
						 }
					}else {
						sStatuWord.word = tmpstu.word;
						pkglen = pkg_realupdate12(buf);
					}
				} //end if
     }	//end for		 
	 }else {
		 /*实时上报数据*/
		 pkglen = pkg_realupdate12(buf);
	 }
	return pkglen;
}


/***********************************************************************/


/********************************CMD_PARASYN_02*************************/
//	{DEV_PARA_21,0,NULL},
//  {RATE_PARA_22,0,NULL},
//  {DTIME_PARA_23,0,NULL},
//  {NOT_EXSIT_PGN,0,NULL},

uint8 proc_syndevpara21(uint8 *buf)
{
	
	return 0;
}

uint8 proc_synratedevpara22(uint8 *buf)
{
	
	return 0;
}

uint8 proc_syntime23(uint8 *buf)
{
	
	return 0;
}



/***********************************************************************/


/********************************CMD_READRECORD_03*************************/
//	{READ_RECORD_31,0,NULL},
//  {NOT_EXSIT_PGN,0,NULL},
uint8 proc_synrecord31(uint8 *buf)
{
	 return 0;
}









/***********************************************************************/


/********************************CMD_UPGRADE_05*************************/
//	{READY_UPGRADE_51,0,NULL},
//  {START_UPGRADE_52,0,NULL},
//  {NOT_EXSIT_PGN,0,NULL},
uint8 proc_synupgrade(uint8 *buf)
{
	
	return 0;
}

/***********************************************************************/




//		CMD_QUERYDATA_01   = 0x01,  /*查询数据*/
//		CMD_PARASYN_02     = 0x02,  /*参数同步*/
//		CMD_READRECORD_03  = 0x03,  /*读取记录*/
//		CMD_READEVENT_04   = 0x04,  /*读取事件*/
//		CMD_UPGRADE_05     = 0x05,  /*升级*/
	

/*处理接受到的命令 ,返回false 不进行应答，返回true 应答*/
uint8 sfunc_dealpkg(uint8 * data,uint8 Cmd,uint8 *dafg,uint8 *num)
{
	  uint8 i,j,*ptr,dlen,rtn;
	  const MSCMD_PGN_PROC_ST *pgnList = NULL;
	  const MSCMD_PGN_PROC_ST *pgnPtr = NULL;
		uint8 len = data[0] > 2 ? data[0]-2 : 0;
	  len -= 3;  
	  
	  switch(Cmd )
	 {
		  case CMD_QUERYDATA_01:
				pgnList = gMs01ProcList;
				break;
			case CMD_PARASYN_02:
				pgnList = gMs02ProcList;
				break;
			case CMD_READRECORD_03:
				pgnList = gMs03ProcList;
				break;
			case CMD_READEVENT_04:
				pgnList = NULL;
				break;
			case CMD_UPGRADE_05:
				pgnList = gMs05ProcList;
				break;
			default:
				break;
	 }
	j = 0;
	ptr = &data[3]; /*data[0] 数据域长度，data[1]、data[2] 状态字，data[3] PGN*/
	rtn = SLAVE_FALSE; //  SLAVE_FALSE
	for( i = 0 ; i < len ; ) {
		pgnPtr = Slave_GetPgnProc(*ptr++,pgnList);
		if( NULL != pgnPtr ) {
			  dafg[(j++)%10] = pgnPtr->pgn;
			  *num = j;
				dlen = *ptr++;
				if( (dlen == pgnPtr->len) || (0 == dlen )) {
					if( NULL != pgnPtr->proc ) {
					   pgnPtr->proc(ptr);
					}
					ptr += dlen;
					i += ( 2+dlen );
					rtn = SLAVE_TRUE;
				}else {
					rtn = SLAVE_FALSE;
					break;
				}
		}else {
				rtn = SLAVE_FALSE;
				break;
		 }
	}//end for
	return rtn;	
}


uint16 sfunc_GetStatuWord(void)
{
	DEV_LOGIC_TYPE *ptrlogic = ChgData_GetLogicDataPtr();
	return ptrlogic->runstu.word;
}

/***************************************************************************************/
//uint8 (*pkgfun) (uint8 *buf,uint8 ackCmd,uint8 dafg);

uint8 sfunc_pkgfun(uint8 *buf,uint8 ackCmd,uint8 dafg,uint8 seq)
{
	uint8 dlen,index;
	  const MSCMD_PGN_PROC_ST *pgnList = NULL;
	  const MSCMD_PGN_PROC_ST *pgnPtr = NULL;
	  START_STU_ST stu;
	
	  index = 0;
	  if( 0 == seq ){
		   stu.word = sfunc_GetStatuWord();
			 buf[index++] = stu.byte[0];
			 buf[index++] = stu.byte[1];
		}
  
	  switch(ackCmd & 0x7f)
	 {
		  case CMD_QUERYDATA_01:
				pgnList = gMs01ProcList;
				break;
			case CMD_PARASYN_02:
				pgnList = gMs02ProcList;
				break;
			case CMD_READRECORD_03:
				pgnList = gMs03ProcList;
				break;
			case CMD_READEVENT_04:
				pgnList = NULL;
				break;
			case CMD_UPGRADE_05:
				pgnList = gMs05ProcList;
				break;
			default:
				break;
	 }
	  dlen = 0;
	 	pgnPtr = Slave_GetPgnProc(dafg,pgnList);
		if( NULL != pgnPtr ) {
			dlen = pgnPtr->pkgfun(&buf[index]);
		}
	  return dlen;
}

#endif





