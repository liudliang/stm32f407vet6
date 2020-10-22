#include "lwip_comm.h" 
//#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h" 
//#include "lwip/timers.h"
//#include "lwip/tcp_impl.h"
#include "lwip/ip4_frag.h"
#include "lwip/tcpip.h" 
//#include "malloc.h"
#include "delay.h" 
#include <stdio.h>
#include "ucos_ii.h" 
//#include "lwipopts.h" 
#include "chgdata.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//lwip通用驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/8/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   
   
  
__lwip_dev lwipdev;						//lwip控制结构体 
struct netif lwip_netif;				//定义一个全局的网络接口

extern u32 memp_get_memorysize(void);	//在memp.c里面定义
extern u8_t *memp_memory;				//在memp.c里面定义.
extern u8_t *ram_heap;					//在mem.c里面定义.

//netif up
void an_netif_set_link_up()
{
	netif_set_link_up(&lwip_netif);
}


//netif down
void an_netif_set_link_down()
{
	netif_set_link_down(&lwip_netif);

}


/////////////////////////////////////////////////////////////////////////////////
//lwip两个任务定义(内核任务和DHCP任务)

//lwip内核任务堆栈(优先级和堆栈大小在lwipopts.h定义了) 

//lwip DHCP任务
//设置任务优先级
//#define LWIP_DHCP_TASK_PRIO       		7
//设置任务堆栈大小
//efine LWIP_DHCP_STK_SIZE  		    156  // patli 20200115 128
//任务堆栈，采用内存管理的方式控制申请	
static OS_STK  LWIP_DHCP_TASK_STK[LWIP_DHCP_STK_SIZE];	
//任务函数
void lwip_dhcp_task(void *pdata); 


//用于以太网中断调用
err_t lwip_pkt_handle(void)
{
	return ethernetif_input(&lwip_netif);
}



//lwip 默认IP设置
//lwipx:lwip控制结构体指针
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	u32 sn0;
	sn0=*(vu32*)(0x1FFF7A10);//获取STM32的唯一ID的前24位作为MAC地址后三字节
	//默认远端IP为:192.168.1.115
	lwipx->remoteip[0]=192;	
	lwipx->remoteip[1]=168;
	lwipx->remoteip[2]=1;
	lwipx->remoteip[3]=102;
	//MAC地址设置(高三字节固定为:2.0.0,低三字节用STM32唯一ID)
	lwipx->mac[0]=2;//高三字节(IEEE称之为组织唯一ID,OUI)地址固定为:2.0.0
	lwipx->mac[1]=0;
	lwipx->mac[2]=0;
	lwipx->mac[3]=(sn0>>16)&0XFF;//低三字节用STM32的唯一ID
	lwipx->mac[4]=(sn0>>8)&0XFFF;;
	lwipx->mac[5]=sn0&0XFF; 
	//默认本地IP为:192.168.1.30
	lwipx->local_ip[0]=192;	
	lwipx->local_ip[1]=168;
	lwipx->local_ip[2]=1;
	lwipx->local_ip[3]=30;
	//默认子网掩码:255.255.255.0
	lwipx->netmask[0]=255;	
	lwipx->netmask[1]=255;
	lwipx->netmask[2]=255;
	lwipx->netmask[3]=0;
	//默认网关:192.168.1.1
	lwipx->gateway[0]=192;	
	lwipx->gateway[1]=168;
	lwipx->gateway[2]=1;
	lwipx->gateway[3]=1;	
	lwipx->dhcpstatus=0;//没有DHCP	
} 

//LWIP初始化(LWIP启动的时候使用)
//返回值:0,成功
//      1,内存错误
//      2,LAN8720初始化失败
//      3,网卡添加失败.
u8 lwip_comm_init(uint8 flag)
{
	OS_CPU_SR cpu_sr;
	struct netif *Netif_Init_Flag;		//调用netif_add()函数时的返回值,用于判断网络初始化是否成功
	ip_addr_t ipaddr;  			//ip地址
	ip_addr_t netmask; 			//子网掩码
	ip_addr_t gw;      			//默认网关 
	
	if(LAN8720_Init())
	{
		
//		printf("lwip_comm_init,LAN8720_Init failr\n");	
		return 2;			//初始化LAN8720失败
	}
	

	tcpip_init(NULL,NULL);				//初始化tcp ip内核,该函数里面会创建tcpip_thread内核任务
	lwip_comm_default_ip_set(&lwipdev);	//设置默认IP等信息
	
	/*获取Mac地址，用于界面显示*/
	PARAM_COMM_TYPE *ptrNetPara = ChgData_GetCommParaPtr();  
	ptrNetPara->netpara.MacAdress[0] = lwipdev.mac[0];
	ptrNetPara->netpara.MacAdress[1] = lwipdev.mac[1];
	ptrNetPara->netpara.MacAdress[2] = lwipdev.mac[2];
	ptrNetPara->netpara.MacAdress[3] = lwipdev.mac[3];
	ptrNetPara->netpara.MacAdress[4] = lwipdev.mac[4];
	ptrNetPara->netpara.MacAdress[5] = lwipdev.mac[5];
	
#if LWIP_DHCP		//使用动态IP
  if(1 == ptrNetPara->netpara.StaPara.DhcpEn){
		ipaddr.addr = 0;
		netmask.addr = 0;
		gw.addr = 0;
	}
	else{
		memcpy(lwipdev.local_ip,ptrNetPara->netpara.LocalIpAddr,sizeof(lwipdev.local_ip));
		memcpy(lwipdev.netmask,ptrNetPara->netpara.NetMask,sizeof(lwipdev.netmask));
		memcpy(lwipdev.gateway,ptrNetPara->netpara.NetGate,sizeof(lwipdev.gateway));
		IP4_ADDR(&ipaddr,lwipdev.local_ip[0],lwipdev.local_ip[1],lwipdev.local_ip[2],lwipdev.local_ip[3]);
	  IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	  IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	}
#else				//使用静态IP
	
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
//	printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
//	printf("静态IP地址........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
//	printf("子网掩码..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
//	printf("默认网关..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
#endif
	OS_ENTER_CRITICAL();  //进入临界区
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&tcpip_input);//向网卡列表中添加一个网口
	OS_EXIT_CRITICAL();  //退出临界区
	if(Netif_Init_Flag==NULL)return 3;//网卡添加失败 
	else//网口添加成功后,设置netif为默认值,并且打开netif网口
	{
		netif_set_default(&lwip_netif); //设置netif为默认网口
		netif_set_up(&lwip_netif);		//打开netif网口
	}
	return 0;//操作OK.
}   
//如果使能了DHCP
#if LWIP_DHCP
//创建DHCP任务
void lwip_comm_dhcp_creat(void)
{
	OS_CPU_SR cpu_sr;
	OS_ENTER_CRITICAL();  //进入临界区
#if(OS_TASK_STAT_STK_CHK_EN)
	OSTaskCreateExt(lwip_dhcp_task,(void*)0,(OS_STK*)&LWIP_DHCP_TASK_STK[LWIP_DHCP_STK_SIZE-1],LWIP_DHCP_TASK_PRIO, LWIP_DHCP_TASK_PRIO, LWIP_DHCP_TASK_STK, LWIP_DHCP_STK_SIZE, NULL, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);//创建DHCP任务 

#else	
	OSTaskCreate(lwip_dhcp_task,(void*)0,(OS_STK*)&LWIP_DHCP_TASK_STK[LWIP_DHCP_STK_SIZE-1],LWIP_DHCP_TASK_PRIO);//创建DHCP任务 
#endif
	OS_EXIT_CRITICAL();  //退出临界区
}
//删除DHCP任务
void lwip_comm_dhcp_delete(void)
{
//patli 20191227	dhcp_stop(&lwip_netif); 		//关闭DHCP
	OSTaskDel(LWIP_DHCP_TASK_PRIO);	//删除DHCP任务
}

void lwipHandleMsg(MSG_STRUCT *msg)
{
	
	switch(msg->MsgType)
	{
		case MSG_RESET_ETH:  /*设备发数据给TCP CLIENT, 发给后台 */
		{	
//			printf("MSG_RESET_ETH,正在重启网络。。。\r\n");   
#if 1
			lwipdev.dhcpstatus = 0;
				
	//		dhcp_stop(&lwip_netif); 		//关闭DHCP		
			
	//		dhcp_cleanup(&lwip_netif);
	//		netif_remove(&lwip_netif);
			
	//		Delay10Ms(100);

#if 0			
			OSTaskDel(TCPIP_THREAD_PRIO); //删除DHCP任务

			ETH_Mem_Free();

			lwip_comm_mem_free1();
			
	//		mymem_init(SRAMIN); 
			Tcp_DataInit();
			
			while(lwip_comm_init(1)) 	
			{
				Delay10Ms(200);
			}	
#endif			
			
		//	printf("MSG_RESET_ETH,dhcp_start。。。\r\n");   
			
	//		dhcp_start(&lwip_netif);//开启DHCP 
#else
	       	SoftReset();
#endif
			
			break;
		}
		default:
			break;
	}
}


//DHCP处理任务
void lwip_dhcp_task(void *pdata)
{
	PARAM_COMM_TYPE *ptrNetPara = ChgData_GetCommParaPtr();  
	MSG_STRUCT msg;
	u32 ip=0,netmask=0,gw=0, disp_num = 0;
	dhcp_start(&lwip_netif);//开启DHCP 
	lwipdev.dhcpstatus=0;	//正在DHCP
	
//	printf("正在查找DHCP服务器,请稍等...........\r\n");  	
		
	Message_QueueCreat(LWIP_DHCP_TASK_PRIO); 

	while(1)
	{ 
		TaskRunTimePrint("lwip_dhcp_task Begin", OSPrioCur);
	
		if (RcvMsgFromQueue(&msg) == TRUE)
		{
			lwipHandleMsg(&msg);		 //处理消息
		}

		
//		printf("正在获取地址...\r\n");
		ip=lwip_netif.ip_addr.addr;		//读取新IP地址
		netmask=lwip_netif.netmask.addr;//读取子网掩码
		gw=lwip_netif.gw.addr;			//读取默认网关
		
		disp_num++;
		
		if(ip!=0)   					//当正确读取到IP地址的时候
		{
			if(disp_num > 10)
			{
//	 			printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
//				printf("通过DHCP获取到IP地址..............%d.%d.%d.%d\r\n",ip&0xff,(ip>>8)&0xff,(ip>>16)&0xff,(ip>>24)&0xff);
//				printf("通过DHCP获取到子网掩码............%d.%d.%d.%d\r\n",netmask&0xff,(netmask>>8)&0xff,(netmask>>16)&0xff,(netmask>>24)&0xff);
//				printf("通过DHCP获取到的默认网关..........%d.%d.%d.%d\r\n",gw&0xff,(gw>>8)&0xff,(gw>>16)&0xff,(gw>>24)&0xff);
			}

			if(lwipdev.dhcpstatus != 2)
			{
				struct dhcp * dhcp = netif_dhcp_data(&lwip_netif);
				
				//解析出通过DHCP获取到的IP地址
				lwipdev.local_ip[3]=(uint8_t)(ip>>24); 
				lwipdev.local_ip[2]=(uint8_t)(ip>>16);
				lwipdev.local_ip[1]=(uint8_t)(ip>>8);
				lwipdev.local_ip[0]=(uint8_t)(ip);

				//解析通过DHCP获取到的子网掩码地址
				lwipdev.netmask[3]=(uint8_t)(netmask>>24);
				lwipdev.netmask[2]=(uint8_t)(netmask>>16);
				lwipdev.netmask[1]=(uint8_t)(netmask>>8);
				lwipdev.netmask[0]=(uint8_t)(netmask);

				//解析出通过DHCP获取到的默认网关
				lwipdev.gateway[3]=(uint8_t)(gw>>24);
				lwipdev.gateway[2]=(uint8_t)(gw>>16);
				lwipdev.gateway[1]=(uint8_t)(gw>>8);
				lwipdev.gateway[0]=(uint8_t)(gw);

				memcpy(ptrNetPara->netpara.LocalIpAddr,lwipdev.local_ip,4);
				memcpy(ptrNetPara->netpara.NetGate,lwipdev.gateway,4);
				memcpy(ptrNetPara->netpara.NetMask,lwipdev.netmask,4);					
				
			
				netif_set_addr(&lwip_netif, &(dhcp->offered_ip_addr),&(dhcp->offered_sn_mask), &(dhcp->offered_gw_addr));
				netif_set_default(&lwip_netif);
				netif_set_up(&lwip_netif);
				
				lwipdev.dhcpstatus = 2;
			}
			
			
			Delay10Ms(50); //延时250ms
//patli 20191228			break;
		}
	else{
			
			struct dhcp * dhcp = netif_dhcp_data(&lwip_netif);
			lwipdev.dhcpstatus=1;//1,进入DHCP获取状态.
//			printf("DHCP服务进入DHCP获取状态!\r\n");

			if(dhcp->tries > LWIP_MAX_DHCP_TRIES)
			{
				lwipdev.dhcpstatus=0XFF;//DHCP失败.
//				printf("DHCP服务超时,使用静态IP地址!\r\n");
//				printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
//				printf("静态IP地址........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
//				printf("子网掩码..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
//				printf("默认网关..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			
			}
		}

		 Delay10Ms(LWIP_DHCP_TASK_DELAY); //延时2.5s
			
			//打印统计信息
			
		if(disp_num > 10)
		{
			stats_display();
			disp_num = 0;
		}

		
		TaskRunTimePrint("lwip_dhcp_task end", OSPrioCur);
	}
	
//patli 20200102	lwip_comm_dhcp_delete();	//删除DHCP任务 
}
#endif 





























