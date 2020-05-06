#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 
#include "os.h" 



//#define USART_REC_LEN  			200  	//定义最大接收字节数 200
//#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
//	  	
//extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
//extern u16 USART_RX_STA;         		//接收状态标记	
////如果想串口中断接收，请不要注释以下宏定义
//void uart_init(u32 bound);


#define STM32F4XX  1 
#define UART_SUPORT_GPS_EN	1   //定义支持GPS

#define DEF_PORT_UART 0

#define COM1        0
#define COM2        1
#define COM3        2
#define COM4        3
#define COM5        4
#define COM6				5

	
#define SCREEN_COM  COM4
#define CARD_COM    COM2
#define METER_COM   COM5  
#define ISO_COM     COM3


#define ACMETER_COM COM6
#define MOD_COM     COM3
#define BACK_COM    COM1

enum HFControl							 
{
	HFC_NONE,
	HFC_CTRL,
};

#define N_UART_CHANNELS	2 	        // rs232/485 个数

#define	RS485TYPE	0
#define	RS232TYPE	1
#define	IRDATYPE	2			   //红外模式
#define	IR38KHZTYPE	3			   //38K调制型红外模式


#define SIO_BAUD_SET		0x1003	  	/* serial device I/O controls */
#define SIO_BAUD_GET		0x1004

#define SIO_HW_OPTS_SET		0x1005
#define SIO_HW_OPTS_GET		0x1006

#define SIO_MODE_SET		0x1007
#define SIO_MODE_GET		0x1008
#define SIO_AVAIL_MODES_GET	0x1009
#define SIO_START	0x2008 /*channel start*/
#define SIO_WORK_TYPE_SET	0x2009 /*type set(rs232 or rs485)*/
#define SIO_WORK_TYPE_GET	0x200A /*type Get(rs232 or rs485)*/

#define SIO_PARITY_BIT_SET	0x3001/*Parity set*/

#define SIO_STOP_BIT_SET	0x3003/*STOP BIT set*/

#define SIO_DATA_BIT_SET	0x3005/*DATA set*/

#define SIO_SEND_STATUS_GET		0x1020 //获得发送状态
#define SIO_SEND_BUF_FULL_STATUS_GET		0x1022 //获得发送缓冲满状态

#define SIO_USE_TYPE_SET		0x1040 //设置设备使用类型
#define SIO_USE_TYPE_GET		0x1041 //获得设备使用类型

#define SIO_GPS_SYN_TIME_GET		0x1051 //获得最新GPS同步时刻的系统给定的时间

#define SIO_GET_PORT_HANDLE     0x1060  //获得端口句柄
//

/* options to SIO_HW_OPTS_SET (ala POSIX), bitwise or'ed together */ 
#define CSIZE		0xc	/* bits 3 and 4 encode the character size */
#define CS5		0x0	/* 5 bits */
#define CS6		0x4	/* 6 bits */
#define CS7		0x8	/* 7 bits */
#define CS8		0xc	/* 8 bits */

#define HUPCL		0x10	/* hang up on last close */
#define STOPB		0x20	/* send two stop bits (else one) */

#define PARENB		0x1c0	/* parity detection setect */
#define PARODD		0x40	/* odd parity*/
#define PAREVEN		0x80	/* even parity  */
#define PARMASK		0xc0	/*Forced “1” stick parity*/
#define PARSPACE	0x100	/* Forced “0” stick parity  */



//#define 	MAX_UART_RCVBUF_LENTH    512    //   串口收发缓冲区大小
//#define   MAX_UART_SNDBUF_LENTH    512

#define   MAX_BACKRCV_LENGTH    700
#define   MAX_BACKSND_LENGTH    2048
 
typedef enum{
	UART_SEND_IDLE,//发送空闲
	UART_SEND_BUSY,//发送忙
}UART_SND_STA;//串口发送状态

//UART发送缓冲满状态
typedef enum{
	UART_SEND_BUF_NO_FULL,//发送不满
	UART_SEND_BUF_FULL,//发送满
}UART_SND_BUF_FULL_STA;//CAN口发送满状态

//UART用途类型
typedef enum{
	UART_USE_TYPE_NOMAL=0,//通用串口
	UART_USE_TYPE_GPS,//GPRS串口
}UART_USE_TYPE;//UART口用途类型

typedef struct
{
	uint8_t startflag;
	uint8_t head1;
	uint8_t head2;
	uint8_t count;
	uint32_t RevGpsSynTime;//最新收到GPS同步的时间
	uint8_t gpsbuf[50];
}gpsuart_t;	

typedef struct {
	void*		baseAddr;	          	// register base address
	void*		directionGPIO;	      // direction contrl pin's GPIO
	uint32_t	rs485ContrlPin;       // rs485  direction contrl pin
	uint8_t		devicType;		      	// 0= rs232 , 1= rs485*/
	uint32_t	defBuardRate;         // 默认波特率 
	uint16_t		defWordLength;        // 默认数据位 5-8 
	uint16_t		defStopbits;          // 默认停止位 1-2 
	uint16_t		defParity;            // 默认校验位
	uint16_t		HardwareFlowControl;  // 硬件流控 
	//20130904
	UART_USE_TYPE		useType;	//UART用途类型
} STM32F10x_SIO_CHAN_PARAM;	

#define IS_VALID_USE_TYPE(arg) ((arg == UART_USE_TYPE_NOMAL) || (arg == UART_USE_TYPE_GPS))



/* Device and Channel Structures */
typedef struct {
	OS_SEM*  uartLock;//OS_EVENT* 	uartLock;	                 		// devece mutex lock	
	void*				base;	                     		// register base address
	void*				directionGPIO;	              // direction contrl pin's GPIO
	uint32_t			rs485ContrlPin;               // rs485  direction contrl pin
	uint32_t			deviceID;                     // uart ID 
	uint8_t				deviceOpenFlag;	              // device flag opened
	uint8_t				devicType;		             		// 0= rs232 , 1= rs485
	int32_t   		baudRate;                     // baud rate 
	uint16_t				WordLength;        			 			// 默认数据位 5-8 
	uint16_t				Stopbits;          			 			// 默认停止位 1-2 
	uint16_t				Parity;            			 			// 默认校验位
	uint16_t				HardwareFlowControl;  		 		// 硬件流控 

	int32_t				sioMode;                     	// SIO mode
	int32_t   		options;                     	// SIO options
//	uint32_t 			read_status_mask;
	uint8_t				*SendBuf;  //[MAX_UART_SNDBUF_LENTH]; // send    buf
	uint8_t				*RcvBuf;    //[MAX_UART_RCVBUF_LENTH];  // receive buf
	uint32_t  		SendTimer;                  	 // 串口发送超时计数器
	UART_SND_STA	SendStatus;//串口发送控制器状态
	gpsuart_t Timegps;
	UART_USE_TYPE		useType;	//UART用途类型
} STM32F10x_UART_CHAN;


extern uint32_t UartHandler[];


// 硬件初始化
void sys_SerialHwInit(void);
//设备打开
extern int32_t Uart_Open(uint32_t deviceId);/*device ID*/ 
extern STM32F10x_UART_CHAN *GetChanHandler(uint8_t id);
//设备关闭
extern int32_t Uart_Close(uint32_t devDesc);/*device description*/	

//从设备读数据，返回读出的数据长度
extern int32_t Uart_Read(int32_t devDesc,	/*device description*/
				uint8_t *buf,	            /*save to buffer*/
				uint16_t maxSize          // 最大读取数量 
				);
//写入数据,返回成功写入长度
extern int32_t Uart_Write(int32_t devDesc,	/*device description*/
				uint8_t *buf,	/* buffer*/
				int32_t size	/*data size writed*/
				);

//串口操作功能
extern int Uart_IOCtrl(int32_t devDesc,	/*device description*/
			int32_t cmd,	/*request cmd*/
			void* arg	/*some argument*/
			);

//读空串口缓冲区
extern int32_t Uart_ClearReadBuf(int32_t devDesc);

//?????????			
extern uint16_t Uart_GetBytesNumInBuff(int32_t devDesc);
#ifdef  __cplusplus
}
#endif

#endif  /* _UART_H */


