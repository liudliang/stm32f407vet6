/****************************************Copyright (c)**************************************************

********************************************************************************************************/

//#define IN_QUEUE
#include "config.h"
#include "queue.h"

/*********************************************************************************************************
** 函数名称: QueueCreate
** 功能描述: 初始化数据队列
** 输　入: Buf      ：为队列分配的存储空间地址
**         SizeOfBuf：为队列分配的存储空间大小（字节）
**			dataSize: 队列每次处理数据大小 ADD
**         ReadEmpty：为队列读空时处理程序
**         WriteFull：为队列写满时处理程序
** 输　出: NOT_OK:参数错误
**         QUEUE_OK:成功
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
uint8 QueueCreate(void *Buf,
                          uint32 SizeOfBuf,
						  uint16 dataSize,
                          uint8 (* ReadEmpty)(),
                          uint8 (* WriteFull)()
                          )
{
#if OS_CRITICAL_METHOD == 3
	OS_CPU_SR  cpu_sr;
#endif  
    DataQueue *Queue;
    
    if (Buf != NULL && SizeOfBuf >= (sizeof(DataQueue)))        /* 判断参数是否有效 */
    {
        Queue = (DataQueue *)Buf;

        OS_ENTER_CRITICAL();
        Queue->DataSize = dataSize;                            /* 初始化结构体数据 */
        Queue->MaxData = (SizeOfBuf - (uint32)(((DataQueue *)0)->Buf)) / 
                         Queue->DataSize;               /* 计算队列可以存储的数据数目 */
        Queue->End = Queue->Buf + Queue->MaxData*Queue->DataSize;       /* 计算数据缓冲的结束地址 */
        Queue->Out = Queue->Buf;
        Queue->In = Queue->Buf;
        Queue->NData = 0;
        Queue->ReadEmpty = ReadEmpty;	
        Queue->WriteFull = WriteFull;

        OS_EXIT_CRITICAL();

        return QUEUE_OK;
    }
    else
    {
        return NOT_OK;
    }
}


/*********************************************************************************************************
** 函数名称: QueueRead
** 功能描述: 获取队列中的数据
** 输　入: Ret:存储返回的消息的地址
**         Buf:指向队列的指针
** 输　出: NOT_OK     ：参数错误
**         QUEUE_OK   ：收到消息
**         QUEUE_EMPTY：无消息
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
uint8 QueueRead(uint8 *Ret, VOID *Buf)
{
#if OS_CRITICAL_METHOD == 3
	OS_CPU_SR  cpu_sr;
#endif  
    uint8 err;
    volatile DataQueue *Queue;

    err = NOT_OK;
    if (Buf != NULL)                                            /* 队列是否有效 */
    {                                                           /* 有效 */
        Queue = (DataQueue *)Buf;
        
        OS_ENTER_CRITICAL();	
        
        if (Queue->NData > 0)                                   /* 队列是否为空 */
        {       
                                                /* 不空         */
            memcpy(Ret,(void *)Queue->Out,Queue->DataSize);				/* 数据出队     */
			//*Ret = Queue->Out[0];                               /* 数据出队     */
            Queue->Out = Queue->Out+Queue->DataSize;            /* 调整出队指针 */
            if (Queue->Out >= Queue->End)
            {
                Queue->Out = Queue->Buf;
            }
            Queue->NData--;                                     /* 数据减少      */
						
						err = QUEUE_OK;
        }
        else
        {                                                       /* 空              */
            err = QUEUE_EMPTY;
            if (Queue->ReadEmpty != NULL)                       /* 调用用户处理函数 */
            {
                err = Queue->ReadEmpty(Ret, Queue);
            }
        }

 	    OS_EXIT_CRITICAL();

    }
    return err;
}

/*************************************************
QueueReadInInt -中断中调用读队列.

modify:rm

RETURN:		NOT_OK     ：参数错误
**         QUEUE_OK   ：收到消息
**         QUEUE_EMPTY：无消息
*************************************************/
uint8 QueueReadInInt(uint8 *Ret, void *Buf)
{
    uint8 err;
    volatile DataQueue *Queue;

    err = NOT_OK;
    if (Buf != NULL)                                            /* 队列是否有效 */
    {                                                           /* 有效 */
        Queue = (DataQueue *)Buf;
         
        if (Queue->NData > 0)                                   /* 队列是否为空 */
        {   
                                                    /* 不空         */
             memcpy(Ret,(void *)Queue->Out,Queue->DataSize);				/* 数据出队     */
			//*Ret = Queue->Out[0];                               /* 数据出队     */
            Queue->Out = Queue->Out+Queue->DataSize;             /* 调整出队指针 */
            if (Queue->Out >= Queue->End)
            {
                Queue->Out = (volatile uint8 *)Queue->Buf;
            }
            Queue->NData--;                                     /* 数据减少      */
            err = QUEUE_OK;
        }
        else
        {                                                       /* 空              */
            err = QUEUE_EMPTY;
            if (Queue->ReadEmpty != NULL)                       /* 调用用户处理函数 */
            {
                err = Queue->ReadEmpty(Ret, Queue);
            }
        }

    }
    return err;
}
/*********************************************************************************************************
** 函数名称: QueueWrite
** 功能描述: FIFO方式发送数据
** 输　入: Buf :指向队列的指针
**         Data:消息数据
** 输　出: NOT_OK   :参数错误
**         QUEUE_FULL:队列满
**         QUEUE_OK  :发送成功
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#ifndef EN_QUEUE_WRITE
#define EN_QUEUE_WRITE      0
#endif

#if EN_QUEUE_WRITE > 0

uint8 QueueWrite(VOID *Buf,uint8* Data)
{
#if OS_CRITICAL_METHOD == 3
	OS_CPU_SR  cpu_sr;
#endif  
    uint8 err;
    volatile DataQueue *Queue;

    err = NOT_OK;
    if (Buf != NULL)                                                    /* 队列是否有效 */
    {
        Queue = (DataQueue *)Buf;
        
        OS_ENTER_CRITICAL();

        if (Queue->NData < Queue->MaxData)                              /* 队列是否满  */
        {                                                               /* 不满        */
            memcpy((void *)Queue->In,Data,Queue->DataSize);											/* 数据入队*/            
            Queue->In = Queue->In+Queue->DataSize;                      /* 调整入队指针*/
            if (Queue->In >= Queue->End)
            {
                Queue->In = Queue->Buf;
            }
            Queue->NData++;                                             /* 数据增加    */
            err = QUEUE_OK;
        }
        else
        {                                                               /* 满           */
            err = QUEUE_FULL;
            if (Queue->WriteFull != NULL)                               /* 调用用户处理函数 */
            {
                err = Queue->WriteFull(Queue, Data, Q_WRITE_MODE);
            }
        }

	    OS_EXIT_CRITICAL();

    }
    return err;
}
#endif

/*************************************************
QueueWriteInInt -中断中调用写队列.

modify:rm

RETURN:		NOT_OK     ：参数错误
**         QUEUE_OK   ：收到消息
**         QUEUE_EMPTY：无消息
*************************************************/
uint8 QueueWriteInInt(VOID *Buf, uint8* Data)
{
    uint8 err;
    volatile  DataQueue *Queue;

    err = NOT_OK;
    if (Buf != NULL)                                                    /* 队列是否有效 */
    {
        Queue = ( DataQueue *)Buf;
        
        if (Queue->NData < Queue->MaxData)                              /* 队列是否满  */
        {                                                               /* 不满        */
					memcpy((void *)Queue->In,Data,Queue->DataSize);	/* 数据入队    */
					//Queue->In[0] = Data;                                        /* 数据入队    */
					//Queue->In++;                                                /* 调整入队指针*/
					Queue->In = Queue->In+Queue->DataSize;                        /* 调整入队指针*/

					if (Queue->In >= Queue->End)
					{
						Queue->In = (volatile uint8 *)Queue->Buf;
					}
					
					Queue->NData++;                                             /* 数据增加    */
					err = QUEUE_OK;
        }
        else
        {                                                               /* 满           */
            err = QUEUE_FULL;
            if (Queue->WriteFull != NULL)                               /* 调用用户处理函数 */
            {
                err = Queue->WriteFull(Queue, Data, Q_WRITE_MODE);
            }
        }

    }
    return err;
}


/*********************************************************************************************************
** 函数名称: QueueWriteFront
** 功能描述: LIFO方式发送数据
** 输　入: Buf:指向队列的指针
**         Data:消息数据
** 输　出: QUEUE_FULL:队列满
**         QUEUE_OK:发送成功
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#ifndef EN_QUEUE_WRITE_FRONT
#define EN_QUEUE_WRITE_FRONT    0
#endif

#if EN_QUEUE_WRITE_FRONT > 0

uint8 QueueWriteFront(VOID *Buf, uint8* Data)
{
    uint8 err;
    DataQueue *Queue;

    err = NOT_OK;
    if (Buf != NULL)                                                    /* 队列是否有效 */
    {
        Queue = (DataQueue *)Buf;
        
        OS_ENTER_CRITICAL();
        
        if (Queue->NData < Queue->MaxData)                              /* 队列是否满  */
        {                                                               /* 不满 */
            Queue->Out=-Queue->DataSize;                                /* 调整出队指针 */
			//Queue->Out--;                                               /* 调整出队指针 */
            if (Queue->Out < Queue->Buf)
            {
                Queue->Out = Queue->End - 1;
            }
						memcpy(Queue->Out,Data,Queue->DataSize);
            //Queue->Out[0] = Data;                                       /* 数据入队     */
            Queue->NData++;                                             /* 数据数目增加 */
            err = QUEUE_OK;
        }
        else
        {                                                               /* 满           */
            err = QUEUE_FULL;
            if (Queue->WriteFull != NULL)                               /* 调用用户处理函数 */
            {
                err = Queue->WriteFull(Queue, Data, Q_WRITE_FRONT_MODE);
            }
        }
        OS_EXIT_CRITICAL();
    }
    return err;
}

#endif

/*********************************************************************************************************
** 函数名称: QueueNData
** 功能描述: 取得队列中数据数
** 输　入: Buf:指向队列的指针
** 输　出: 消息数
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#ifndef EN_QUEUE_NDATA
#define EN_QUEUE_NDATA    0
#endif

#if EN_QUEUE_NDATA > 0

uint16 QueueNData(VOID *Buf)
{
#if OS_CRITICAL_METHOD == 3
	OS_CPU_SR  cpu_sr;
#endif  
    uint16 temp;
    
    temp = 0;                                                   /* 队列无效返回0 */
    if (Buf != NULL)
    {
        OS_ENTER_CRITICAL();
        temp = ((DataQueue *)Buf)->NData;
        OS_EXIT_CRITICAL();
    }
    return temp;
}

#endif

/*********************************************************************************************************
** 函数名称: QueueSize
** 功能描述: 取得队列总容量
** 输　入: Buf:指向队列的指针
** 输　出: 队列总容量
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
**
** 作　者: 陈明计
** 日　期: 2003年7月2日
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#ifndef EN_QUEUE_SIZE
#define EN_QUEUE_SIZE    0
#endif

#if EN_QUEUE_SIZE > 0

        uint16 QueueSize(VOID *Buf)
{
#if OS_CRITICAL_METHOD == 3
	OS_CPU_SR  cpu_sr;
#endif  
    uint16 temp;
    
    temp = 0;                                                   /* 队列无效返回0 */
    if (Buf != NULL)
    {
        OS_ENTER_CRITICAL();
        temp = ((DataQueue *)Buf)->MaxData;
        OS_EXIT_CRITICAL();
    }
    return temp;
}

#endif

/*********************************************************************************************************
** 函数名称: OSQFlush
** 功能描述: 清空队列
** 输　入: Buf:指向队列的指针
** 输　出: 无
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#ifndef EN_QUEUE_FLUSH
#define EN_QUEUE_FLUSH    0
#endif

#if EN_QUEUE_FLUSH > 0

        void QueueFlush(VOID *Buf)
{
    DataQueue *Queue;
    
    if (Buf != NULL)                                                /* 队列是否有效 */
    {                                                               /* 有效         */
        Queue = (DataQueue *)Buf;
        OS_ENTER_CRITICAL();
        Queue->Out = Queue->Buf;
        Queue->In = Queue->Buf;
        Queue->NData = 0;                                           /* 数据数目为0 */
        OS_EXIT_CRITICAL();
    }
}

#endif

//判断缓冲是否满
//返回：0-不满，1-满
uint16 IsQueueFull(VOID *Buf)
{
#if OS_CRITICAL_METHOD == 3
	OS_CPU_SR  cpu_sr;
#endif  
    uint16 bRet=FALSE;
	DataQueue *Queue;                                                   /* 队列无效返回0 */
    if (Buf != NULL)                                                    /* 队列是否有效 */
    {
		OS_ENTER_CRITICAL();
        Queue = (DataQueue *)Buf;
		if (Queue->NData >= Queue->MaxData)
    	{//满
			bRet = TRUE;		
		}
        OS_EXIT_CRITICAL();
    }
    return bRet;
}

/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/
