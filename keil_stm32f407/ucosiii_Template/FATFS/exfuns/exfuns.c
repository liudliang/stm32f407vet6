#include "exfuns/exfuns.h"
#include "malloc.h"
//#include "usart.h"
#include "Debug.h"
#include "string.h"

	   
//////////////////////////////////////////////////////////////////////////////////	 
//功能：FATFS 扩展代码，文件操作api  
//作者：zyf
//创建日期:2020/05/29
//版本：V1.0	
//描述：
//历史：
////////////////////////////////////////////////////////////////////////////////// 							  


#define FILE_MAX_TYPE_NUM		7	//最多FILE_MAX_TYPE_NUM个大类
#define FILE_MAX_SUBT_NUM		4	//最多FILE_MAX_SUBT_NUM个小类

 //文件类型列表
u8*const FILE_TYPE_TBL[FILE_MAX_TYPE_NUM][FILE_MAX_SUBT_NUM]=
{
{"BIN"},			//BIN文件
{"LRC"},			//LRC文件
{"NES"},			//NES文件
{"TXT","C","H"},	//文本文件
{"WAV","MP3","APE","FLAC"},//支持的音乐文件
{"BMP","JPG","JPEG","GIF"},//图片文件
{"AVI"},			//视频文件
};
///////////////////////////////公共文件区,使用malloc的时候////////////////////////////////////////////
FATFS *fs[_VOLUMES];//逻辑磁盘工作区.	 
//FIL *file;	  		//文件1
//FIL *ftemp;	  		//文件2.
//UINT br,bw;			//读写变量
//FILINFO fileinfo;	//文件信息
//DIR dir;  			//目录

//u8 *fatbuf;			//SD卡数据缓存区
///////////////////////////////////////////////////////////////////////////////////////



FATFS *exfuns_GetfsArea(u8 fsnum)
{
	return fs[fsnum];
}

////为exfuns申请内存
////返回值:0,成功
////1,失败
//u8 exfuns_init(void)
//{
//	u8 i;
//	for(i=0;i<_VOLUMES;i++)
//	{
//		fs[i]=(FATFS*)mymalloc(SRAMIN,sizeof(FATFS));	//为磁盘i工作区申请内存	
//		if(!fs[i])break;
//	}
////	file=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//为file申请内存
////	ftemp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//为ftemp申请内存
////	fatbuf=(u8*)mymalloc(SRAMIN,512);				//为fatbuf申请内存
//	if(i==_VOLUMES)//&&file&&ftemp&&fatbuf)
//		return 0;  //申请有一个失败,即失败.
//	else 
//		return 1;	
//}

//为exfuns申请内存
//返回值:0,成功
//1,失败
u8 exfuns_init(u8 disknum)
{
	fs[disknum]=(FATFS*)mymalloc(SRAMIN,sizeof(FATFS));	//为磁盘i工作区申请内存	
	if(NULL == fs[disknum])
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


//将小写字母转为大写字母,如果是数字,则保持不变.
u8 char_upper(u8 c)
{
	if(c<'A')return c;//数字,保持不变.
	if(c>='a')return c-0x20;//变为大写.
	else return c;//大写,保持不变
}	      
//报告文件的类型
//fname:文件名
//返回值:0XFF,表示无法识别的文件类型编号.
//		 其他,高四位表示所属大类,低四位表示所属小类.
u8 f_typetell(u8 *fname)
{
	u8 tbuf[5];
	u8 *attr='\0';//后缀名
	u8 i=0,j;
	while(i<250)
	{
		i++;
		if(*fname=='\0')break;//偏移到了最后了.
		fname++;
	}
	if(i==250)return 0XFF;//错误的字符串.
 	for(i=0;i<5;i++)//得到后缀名
	{
		fname--;
		if(*fname=='.')
		{
			fname++;
			attr=fname;
			break;
		}
  	}
	strcpy((char *)tbuf,(const char*)attr);//copy
 	for(i=0;i<4;i++)tbuf[i]=char_upper(tbuf[i]);//全部变为大写 
	for(i=0;i<FILE_MAX_TYPE_NUM;i++)	//大类对比
	{
		for(j=0;j<FILE_MAX_SUBT_NUM;j++)//子类对比
		{
			if(*FILE_TYPE_TBL[i][j]==0)break;//此组已经没有可对比的成员了.
			if(strcmp((const char *)FILE_TYPE_TBL[i][j],(const char *)tbuf)==0)//找到了
			{
				return (i<<4)|j;
			}
		}
	}
	return 0XFF;//没找到		 			   
}	 

//得到磁盘剩余容量
//drv:磁盘编号("0:"/"1:")
//total:总容量	 （单位KB）
//free:剩余容量	 （单位KB）
//返回值:0,正常.其他,错误代码
u8 exf_getfree(u8 *drv,u32 *total,u32 *free)
{
	FATFS *fs1;
	u8 res;
  u32 fre_clust=0, fre_sect=0, tot_sect=0;
  //得到磁盘信息及空闲簇数量
  res =(u32)f_getfree((const TCHAR*)drv, (DWORD*)&fre_clust, &fs1);
  if(res==0)
	{											   
	    tot_sect=(fs1->n_fatent-2)*fs1->csize;	//得到总扇区数
	    fre_sect=fre_clust*fs1->csize;			//得到空闲扇区数	   
#if _MAX_SS!=512				  				//扇区大小不是512字节,则转换为512字节
		tot_sect*=fs1->ssize/512;
		fre_sect*=fs1->ssize/512;
#endif	  
		*total=tot_sect>>1;	//单位为KB
		*free=fre_sect>>1;	//单位为KB 
 	}
	return res;
}	


/*
*********************************************************************************************************
*	函 数 名: exfuns_ViewRootDir
*	功能说明: 显示根目录下的文件名
*	形    参：path:路径
*	返 回 值: 
*********************************************************************************************************
*/
u8 exfuns_ViewRootDir(char *path)
{
	FRESULT result;
	DIR *DirInf = mymalloc(SRAMIN,sizeof(DirInf));		//为DirInf申请内存
	FILINFO *FileInf = mymalloc(SRAMIN,sizeof(FileInf));		//为FileInf申请内存
	
	if((NULL == DirInf) || (NULL == FileInf) )
	{
		DEBUG_Printf("malloc ViewRootDir failed!");
		goto ViewRootDir_END;
	}
	
	
	uint32_t cnt = 0;
	char lfname[100];

	/* 打开根文件夹 */
	result = f_opendir(DirInf, path);//"/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败 (%d)\r\n", result);
		goto ViewRootDir_END;
	}

	/* 读取当前文件夹下的文件和目录 */
	FileInf->lfname = lfname;
	FileInf->lfsize = 256;

	printf("属性        |  文件大小 | 短文件名 | 长文件名\r\n");
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(DirInf,FileInf); 		/* 读取目录项，索引会自动下移 */
		if (result != FR_OK || FileInf->fname[0] == 0)
		{
			break;
		}

		if (FileInf->fname[0] == '.')
		{
			continue;
		}

		/* 判断是文件还是子目录 */
		if (FileInf->fattrib & AM_DIR)
		{
			printf("(0x%02d)目录  ", FileInf->fattrib);
		}
		else
		{
			printf("(0x%02d)文件  ", FileInf->fattrib);
		}

		/* 打印文件大小, 最大4G */
		printf(" %10ld", FileInf->fsize);   //单位：字节

		printf("  %s |", FileInf->fname);	/* 短文件名 */

		printf("  %s\r\n", (char *)FileInf->lfname);	/* 长文件名 */
	}
	
ViewRootDir_END:
	myfree(SRAMIN,DirInf);//释放内存
	myfree(SRAMIN,FileInf);//释放内存
	
	return result;
}


//带锁的文件写操作
u8 SST_Rd_BufferWrite(u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite, const TCHAR* path)
{
	OS_ERR err;
	u32 bytesToWrite = 0;
	FIL* file=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//为file申请内存
	if(NULL == file)
	{
		myfree(SRAMIN,file);//释放内存
		DEBUG_Printf("malloc writefile_ptr failed!");
		return 0;
	}
	
	
	OSSchedLock(&err);
	
	f_open(file, path, FA_WRITE);	
	f_lseek(file, WriteAddr);	
	f_write(file, pBuffer, NumByteToWrite, &bytesToWrite);
	f_sync(file);
	f_close(file);
	
	OSSchedUnlock(&err);
	
	myfree(SRAMIN,file);//释放内存
	
	return 1;
}


//带锁的文件读操作
u8 SST_Rd_BufferRead(u8 *recv, u32 adr, u16 size, const TCHAR* path)
{
	OS_ERR err;
	u32 numOfReadBytes = 0;
	FIL* file=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//为file申请内存
	if(NULL == file)
	{
		myfree(SRAMIN,file);//释放内存
		DEBUG_Printf("malloc readfile_ptr failed!");
		return 0;
	}
	

	OSSchedLock(&err);
	
	f_open(file, path, FA_READ);	
	f_lseek(file, adr);
	f_read(file, recv, size, &numOfReadBytes);
	f_close(file);
	
	OSSchedUnlock(&err);
	
	myfree(SRAMIN,file);//释放内存
	
	return 1;
}






