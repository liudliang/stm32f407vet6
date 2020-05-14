#include "malloc.h"	   
//////////////////////////////////////////////////////////////////////////////////	 
//功能：内存管理 驱动代码	  
//作者：
//创建日期:2020/05/14
//版本：V1.0	
//描述：由正点原子demo修改而来，将demo中的外部SRAM内存池、内部CCM内存池去掉
//	分配原理：当指针 p 调用 malloc 申请内存的时候，先判断 p 要分配的内存块数（m），然后从第 n 项开
//始，向下查找，直到找到 m 块连续的空内存块（即对应内存管理表项为 0），然后将这 m 个内
//存管理表项的值都设置为 m（标记被占用），最后，把最后的这个空内存块的地址返回指针 p，
//完成一次分配。注意，如果当内存不够的时候（找到最后也没找到连续的 m 块空闲内存），则
//返回 NULL 给 p，表示分配失败。
//	释放原理：当 p 申请的内存用完，需要释放的时候，调用 free 函数实现。free 函数先判断 p 指向的内
//存地址所对应的内存块，然后找到对应的内存管理表项目，得到 p 所占用的内存块数目 m（内
//存管理表项目的值就是所分配内存块的数目），将这 m 个内存管理表项目的值都清零，标记释
//放，完成一次内存释放。
////////////////////////////////////////////////////////////////////////////////// 


//内存池(32字节对齐)
__align(32) u8 mem1base[MEM1_MAX_SIZE];													//内部SRAM内存池
//__align(32) u8 mem2base[MEM2_MAX_SIZE] __attribute__((at(0X68000000)));					//外部SRAM内存池
//__align(32) u8 mem3base[MEM3_MAX_SIZE] __attribute__((at(0X10000000)));					//内部CCM内存池
//内存管理表
u16 mem1mapbase[MEM1_ALLOC_TABLE_SIZE];													//内部SRAM内存池MAP
//u16 mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((at(0X68000000+MEM2_MAX_SIZE)));	//外部SRAM内存池MAP
//u16 mem3mapbase[MEM3_ALLOC_TABLE_SIZE] __attribute__((at(0X10000000+MEM3_MAX_SIZE)));	//内部CCM内存池MAP
//内存管理参数	   
const u32 memtblsize[SRAMBANK]={MEM1_ALLOC_TABLE_SIZE};//,MEM2_ALLOC_TABLE_SIZE,MEM3_ALLOC_TABLE_SIZE};	//内存表大小
const u32 memblksize[SRAMBANK]={MEM1_BLOCK_SIZE};//,MEM2_BLOCK_SIZE,MEM3_BLOCK_SIZE};					//内存分块大小
const u32 memsize[SRAMBANK]={MEM1_MAX_SIZE};//,MEM2_MAX_SIZE,MEM3_MAX_SIZE};							//内存总大小


//内存管理控制器
struct _m_mallco_dev mallco_dev=
{
	my_mem_init,						//内存初始化
	my_mem_perused,						//内存使用率
	mem1base,//mem2base,mem3base,			//内存池
	mem1mapbase,//mem2mapbase,mem3mapbase,//内存管理状态表
	0,//0,0,  		 					//内存管理未就绪
};



/*
** 函数名称: 
** 功能描述: 复制内存  
** 输　入: 
						*des:目的地址
						*src:源地址
						n:需要复制的内存长度(字节为单位)
** 输　出:  
** 全局变量:
** 描述: 
*/
void mymemcpy(void *des,void *src,u32 n)  
{  
  u8 *xdes=des;
	u8 *xsrc=src; 
  while(n--)
	{
		*xdes++=*xsrc++;
	}		
}  



/*
** 函数名称: 
** 功能描述: 设置内存  
** 输　入: 
						*s:内存首地址
						c :要设置的值
						count:需要设置的内存大小(字节为单位)
** 输　出:  
** 全局变量:
** 描述: 
*/
void mymemset(void *s,u8 c,u32 count)  
{  
  u8 *xs = s;  
  while(count--)
  {
		*xs++=c;
	}			
}	   



/*
** 函数名称: 
** 功能描述: 内存管理初始化  
** 输　入: 
						memx:所属内存块
** 输　出:  
** 全局变量:
** 描述: 
*/
void my_mem_init(u8 memx)  
{  
  mymemset(mallco_dev.memmap[memx], 0,memtblsize[memx]*2);//内存状态表数据清零  
	mymemset(mallco_dev.membase[memx], 0,memsize[memx]);	//内存池所有数据清零  
	mallco_dev.memrdy[memx]=1;								//内存管理初始化OK  
}  



/*
** 函数名称: 
** 功能描述: 获取内存使用率
** 输　入: 
						memx:所属内存块
						size:要分配的内存大小(字节)
** 输　出:  使用率(0~100) 
** 全局变量:
** 描述: 
*/
u8 my_mem_perused(u8 memx)  
{  
  u32 used=0;  
  u32 i;  
  for(i=0;i<memtblsize[memx];i++)  
  {  
    if(mallco_dev.memmap[memx][i])used++; 
  } 
  return (used*100)/(memtblsize[memx]);  
}  



/*
** 函数名称: 
** 功能描述: 内存分配(内部调用)
** 输　入: 
						memx:所属内存块
						size:要分配的内存大小(字节)
** 输　出:  0XFFFFFFFF,代表错误;其他,内存偏移地址  
** 全局变量:
** 描述: 
*/
u32 my_mem_malloc(u8 memx,u32 size)  
{  
  signed long offset=0;  
  u32 nmemb;	//需要的内存块数  
	u32 cmemb=0;//连续空内存块数
  u32 i;  
  if(!mallco_dev.memrdy[memx])
	{
		mallco_dev.init(memx);//未初始化,先执行初始化
	}		
  if(size==0)
	{
		return 0XFFFFFFFF;//不需要分配
	}
  nmemb=size/memblksize[memx];  	//获取需要分配的连续内存块数
  if(size%memblksize[memx])
	{
		nmemb++;
	}		
  for(offset=memtblsize[memx]-1;offset>=0;offset--)//搜索整个内存控制区  
  {     
		if(!mallco_dev.memmap[memx][offset])
		{
			cmemb++;//连续空内存块数增加
		}
		else
		{
			cmemb=0;								//连续内存块清零
		}
		if(cmemb==nmemb)							//找到了连续nmemb个空内存块
		{
      for(i=0;i<nmemb;i++)  					//标注内存块非空 
      {  
        mallco_dev.memmap[memx][offset+i]=nmemb;  
      }  
      return (offset*memblksize[memx]);//返回偏移地址  
		}
  }  
  return 0XFFFFFFFF;//未找到符合分配条件的内存块  
}  



/*
** 函数名称: 
** 功能描述: 释放内存(内部调用) 
** 输　入: 
						memx:所属内存块
						offset:内存地址偏移
** 输　出:  0,释放成功;1,释放失败; 
** 全局变量:
** 描述: 
*/
u8 my_mem_free(u8 memx,u32 offset)  
{  
  int i;  
  if(!mallco_dev.memrdy[memx])//未初始化,先执行初始化
	{
		mallco_dev.init(memx);    
    return 1;//未初始化  
  }  
  if(offset<memsize[memx])//偏移在内存池内. 
  {  
    int index=offset/memblksize[memx];			//偏移所在内存块号码  
    int nmemb=mallco_dev.memmap[memx][index];	//内存块数量
    for(i=0;i<nmemb;i++)  						//内存块清零
    {  
      mallco_dev.memmap[memx][index+i]=0;  
    }  
    return 0;  
  }
	else 
	{
		return 2;//偏移超区了.
	}		
}  



/*
** 函数名称: 
** 功能描述: 释放内存(外部调用) 
** 输　入: 
						memx:所属内存块
						ptr:内存首地址
** 输　出:  
** 全局变量:
** 描述: 
*/
void myfree(u8 memx,void *ptr)  
{  
	u32 offset;   
	if(ptr==NULL)
	{
		return;//地址为0.
	}		
 	offset=(u32)ptr-(u32)mallco_dev.membase[memx];     
  my_mem_free(memx,offset);	//释放内存      
}  



/*
** 函数名称: 
** 功能描述: 分配内存(外部调用)
** 输　入: 
						memx:所属内存块
						size:内存大小(字节)
** 输　出:  分配到的内存首地址.
** 全局变量:
** 描述: 
*/
void *mymalloc(u8 memx,u32 size)  
{  
  u32 offset;   
	offset=my_mem_malloc(memx,size);  	   	 	   
  if(offset==0XFFFFFFFF)
	{
		return NULL;
	}		
  else 
	{
		return (void*)((u32)mallco_dev.membase[memx]+offset);
	}		
}  



/*
** 函数名称: 
** 功能描述: 重新分配内存(外部调用)
** 输　入: 
						memx:所属内存块
						*ptr:旧内存首地址
						size:要分配的内存大小(字节)	
** 输　出:  新分配到的内存首地址.
** 全局变量:
** 描述: 
*/
void *myrealloc(u8 memx,void *ptr,u32 size)  
{  
  u32 offset;    
  offset=my_mem_malloc(memx,size);   	
  if(offset==0XFFFFFFFF)
	{
		return NULL;
	}		
  else  
  {  									   
	  mymemcpy((void*)((u32)mallco_dev.membase[memx]+offset),ptr,size);	//拷贝旧内存内容到新内存   
    myfree(memx,ptr);  											  		//释放旧内存
    return (void*)((u32)mallco_dev.membase[memx]+offset);  				//返回新内存首地址
  }  
}












