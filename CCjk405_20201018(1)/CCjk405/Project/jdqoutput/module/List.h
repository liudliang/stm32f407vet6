/**
  ******************************************************************************
  * @file    List.h
  * @author  zqj
  * @version v1.0
  * @date    2018-07-09
  * @brief   
  ******************************************************************************
*/
#ifndef __LIST_H_0001
#define __LIST_H_0001
#include "typedefs.h"
#include "memory.h"




#define  CMDSIZE  14     // > sizeof(CMD_LIST)
/*定义一个链表的结构体指针*/
struct list {
	uint32 id;				      /*标识这个元素方便查找*/
	DMEM * memptr;
	uint16 size;
	uint8 cmdst[CMDSIZE];		/*链表包含的元素*/
	struct list *next;	/*指向下一个链表的指针*/
};


extern int list_add(struct list **head,uint16 nsize,void *node);

extern int list_add(struct list **head,uint16 nsize,void *node);

extern int list_del(struct list **head, int id);

extern int list_chg(struct list **head, int id, void *node);

extern int list_query(struct list **head, int id,void *node);






#endif

