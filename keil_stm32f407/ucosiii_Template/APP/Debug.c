#include "tcp_client_demo.h"
#include "dp83848.h"
#include "lwip_comm.h"

#include "sys.h"
#include "delay.h"
#include "usart.h"


#include "gpio.h"
#include "timer.h"
#include "Debug.h"


#include "lwip/api.h"

//////////////////////////////////////////////////////////////////////////////////	 
//功能：调试功能api  
//作者：
//创建日期:2020/05/09
//版本：V1.0								  
////////////////////////////////////////////////////////////////////////////////// 

char DEBUG_Buff[50] = {0}; 

static uint32_t gDebugHandler;


#ifdef SCOMM_DEBUG
#define DEBUG_COM    COM1   //调试打印口
#define DEBUG_COM_BAUD		        115200
void DebugScomm_init(void)
{
	int32_t tmp;
	UartHandler[DEBUG_COM] = Uart_Open(DEBUG_COM);
	gDebugHandler = UartHandler[DEBUG_COM];
	
 	tmp  = DEBUG_COM_BAUD;
	Uart_IOCtrl(UartHandler[DEBUG_COM], SIO_BAUD_SET, &tmp);	

	tmp  = USART_Parity_No;
	Uart_IOCtrl(UartHandler[DEBUG_COM], SIO_PARITY_BIT_SET, &tmp);

	tmp  = USART_StopBits_1;
	Uart_IOCtrl(UartHandler[DEBUG_COM], SIO_STOP_BIT_SET, &tmp);	
	
	tmp = USART_WordLength_8b;
	Uart_IOCtrl(UartHandler[DEBUG_COM], SIO_DATA_BIT_SET, &tmp);	
}
#endif



void DEBUG_Printf(char *s)
{
#ifdef SCOMM_DEBUG	
	Uart_Write(gDebugHandler, (uint8_t *)s, strlen(s));
#endif
	
}


#define DEBUG_TCP_CLIENT_RX_BUFSIZE	2000	//接收缓冲区长度
char *debug_tcp_client_sendbuf="DEBUG NETCONN TCP Client send data\r\n";	//TCP客户端发送数据缓冲区
u8 debug_tcp_client_recvbuf[DEBUG_TCP_CLIENT_RX_BUFSIZE];	//TCP客户端接收数据缓冲区
u8 debug_tcp_client_flag;		//TCP客户端数据发送标志位


#define DEBUG_REMOTE_PORT  8001
const uint8_t DEBUG_REMOTE_ADDR[4] = {192,168,0,102};
struct netconn *debugtcp_clientconn;					//TCP CLIENT网络连接结构体
static void debug_tcp_client_thread(void)
{
	OS_ERR err;
	//-----------------test lwip
//	while(lwip_comm_init()) 		//lwip初始化
//	{
//		DEBUG_Printf("Lwip Init failed!\r\n"); 	//lwip初始化失败
//		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);  
//	}
//	
//	while(tcp_client_init()) 									//初始化tcp_client(创建tcp_client线程)
//	{
//		DEBUG_Printf("TCP Client failed!!\r\n"); 		//tcp创建失败
//		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err); 
//	}
	
//#if LWIP_DHCP
//	lwip_comm_dhcp_creat(); //创建DHCP任务
//#endif
//-----------------	
	
	CPU_SR_ALLOC();
	
	u32 data_len = 0;
	struct pbuf *q;
	err_t recv_err;
	static ip_addr_t server_ipaddr,loca_ipaddr;
	static u16_t 		 server_port,loca_port;
	
	while(1)
	{
		if(1 == DP83848_GetPHYlinkStatus())
	  {
		  if(2 != lwipdev.dhcpstatus)	//DHCP成功
			{
				OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err); 
			}
			else
			{
				break;
			}
		}
		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err); 
	}

	

//	LWIP_UNUSED_ARG(arg);
	server_port = DEBUG_REMOTE_PORT;
	IP4_ADDR(&server_ipaddr, DEBUG_REMOTE_ADDR[0],DEBUG_REMOTE_ADDR[1], DEBUG_REMOTE_ADDR[2],DEBUG_REMOTE_ADDR[3]);
	
	while (1) 
	{
		debugtcp_clientconn = netconn_new(NETCONN_TCP);  //创建一个TCP链接
		err = netconn_connect(debugtcp_clientconn,&server_ipaddr,server_port);//连接服务器
		if(err != ERR_OK)  netconn_delete(debugtcp_clientconn); //返回值不等于ERR_OK,删除tcp_clientconn连接
		else if (err == ERR_OK)    //处理新连接的数据
		{ 
			struct netbuf *recvbuf;
			debugtcp_clientconn->recv_timeout = 10;
			netconn_getaddr(debugtcp_clientconn,&loca_ipaddr,&loca_port,1); //获取本地IP主机IP地址和端口号
			printf("连接上服务器%d.%d.%d.%d,本机端口号为:%d\r\n",DEBUG_REMOTE_ADDR[0],DEBUG_REMOTE_ADDR[1], DEBUG_REMOTE_ADDR[2],DEBUG_REMOTE_ADDR[3],loca_port);
			while(1)
			{
				if((debug_tcp_client_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA) //有数据要发送
				{
					err = netconn_write(debugtcp_clientconn ,debug_tcp_client_sendbuf,strlen((char*)debug_tcp_client_sendbuf),NETCONN_COPY); //发送tcp_server_sentbuf中的数据
					if(err != ERR_OK)
					{
						printf("发送失败\r\n");
					}
					debug_tcp_client_flag &= ~LWIP_SEND_DATA;
				}
					
				if((recv_err = netconn_recv(debugtcp_clientconn,&recvbuf)) == ERR_OK)  //接收到数据
				{	
					OS_CRITICAL_ENTER(); //关中断
					memset(debug_tcp_client_recvbuf,0,DEBUG_TCP_CLIENT_RX_BUFSIZE);  //数据接收缓冲区清零
					for(q=recvbuf->p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
					{
						//判断要拷贝到TCP_CLIENT_RX_BUFSIZE中的数据是否大于TCP_CLIENT_RX_BUFSIZE的剩余空间，如果大于
						//的话就只拷贝TCP_CLIENT_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
						if(q->len > (DEBUG_TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(debug_tcp_client_recvbuf+data_len,q->payload,(DEBUG_TCP_CLIENT_RX_BUFSIZE-data_len));//拷贝数据
						else memcpy(debug_tcp_client_recvbuf+data_len,q->payload,q->len);
						data_len += q->len;  	
						if(data_len > DEBUG_TCP_CLIENT_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出	
					}
					OS_CRITICAL_EXIT();  //开中断
					data_len=0;  //复制完成后data_len要清零。					
					printf("%s\r\n",debug_tcp_client_recvbuf);
					netbuf_delete(recvbuf);
				}else if(recv_err == ERR_CLSD)  //关闭连接
				{
					netconn_close(debugtcp_clientconn);
					netconn_delete(debugtcp_clientconn);
					printf("服务器%d.%d.%d.%d断开连接\r\n",DEBUG_REMOTE_ADDR[0],DEBUG_REMOTE_ADDR[1], DEBUG_REMOTE_ADDR[2],DEBUG_REMOTE_ADDR[3]);
					break;
				}
			}
		}
	}

	
	
	
	
	
	
	
	
	
	
	
	
//	while(1)
//	{
//	
//	  if(1 == DP83848_GetPHYlinkStatus())
//	  {
//		  if(2 != lwipdev.dhcpstatus)	//DHCP成功
//			{
//				OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err); 
//			}
//			else
//			{
//				
//			}
//	  }
//		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);
//		
//	}
	
	
}



//任务控制块
OS_TCB	DEBUG_TcpClientTaskTCB;
//任务堆栈
CPU_STK DEBUG_TCPCLIENT_TASK_STK[DEBUG_TCPCLIENT_STK_SIZE];

//创建调试TCP客户端线程
//返回值:0 TCP客户端创建成功
//		其他 TCP客户端创建失败
u8 DEBUG_TCP_Client_Init(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	OS_CRITICAL_ENTER();//进入临界区
	//创建TCP客户端任务
	OSTaskCreate((OS_TCB 	* )&DEBUG_TcpClientTaskTCB,		
				 (CPU_CHAR	* )"debug_tcp_client task", 		
                 (OS_TASK_PTR )debug_tcp_client_thread, 			
                 (void		* )0,					
                 (OS_PRIO	  )DEBUG_TCPCLIENT_PRIO,     
                 (CPU_STK   * )&DEBUG_TCPCLIENT_TASK_STK[0],	
                 (CPU_STK_SIZE)DEBUG_TCPCLIENT_STK_SIZE/10,	
                 (CPU_STK_SIZE)DEBUG_TCPCLIENT_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);
	OS_CRITICAL_EXIT();	//退出临界区
	return err;
}


void DebugETH_init(void)
{
  OS_ERR err;
  while(DEBUG_TCP_Client_Init()) 									//初始化tcp_client(创建tcp_client线程)
	{
		DEBUG_Printf("DEBUG TCP Client failed!!\r\n"); 		//tcp创建失败
		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err); 
	}	
	
}




void DEBUG_Init(void)
{
	
#ifdef SCOMM_DEBUG
  DebugScomm_init();
#endif
	
#ifdef ETH_DEBUG
	DebugETH_init();
#endif

}

