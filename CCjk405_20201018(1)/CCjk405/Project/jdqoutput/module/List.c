/**
  ******************************************************************************
  * @file    List.c
  * @author  zqj
  * @version v1.0
  * @date    2018-07-09
  * @brief   
  ******************************************************************************
*/

/* ?????? */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "List.h"


 
/*定义个个链表头部*/
//static struct list *list_head = NULL;
 
//static int list_id = 0;
 
/**	将指定元素插入到链表尾部
  * 	head	: 表示要插入元素的链表的头地址
  *	list    : 表示要插入到链表中的元素
	* 返回插入的id,失败为-1
 */

int list_add(struct list **head,uint16 nsize,void *node)
{
	struct list *temp,*cmdstptr;
	DMEM *dynMemory = DynMemGet(sizeof(struct list)); 
	if( NULL == dynMemory) {
		return -1;
	}
	cmdstptr = dynMemory->addr;
	memset(cmdstptr,0,sizeof(struct list));
	
	cmdstptr->memptr = dynMemory;
	if( NULL == node )
	{
		return -1;
	}
	
  nsize = nsize > CMDSIZE ? CMDSIZE : nsize;
	cmdstptr->size = nsize;
	//memcpy(cmdstptr->cmdst,node,nsize);
	/* 判断链表是否为空 */
	if(NULL == *head)
	{
		/*为空*/
		*head = cmdstptr;
		 cmdstptr->id = 1;
		 memcpy(cmdstptr->cmdst,node,cmdstptr->size);
		(*head)->next = NULL;
	}
	else
	{
		/*不为空*/
		temp = *head;
		while(temp)
		{
			if(NULL == temp->next)
			{
				cmdstptr->id = (temp->id + 1 );
				memcpy(cmdstptr->cmdst,node,cmdstptr->size);
				temp->next = cmdstptr;
				cmdstptr->next = NULL;
				break;
			}
			temp = temp->next;
		}
	}
	return cmdstptr->id;
}

 
///**	指定元素从链表尾部删除
//  * 	head	: 表示要删除元素的链表头部地址
//  *	  id    :  表示要删除元素的标识
//  *	返回值  : 0-成功,-1-失败
//  */
int list_del(struct list **head, int id)
{
	struct list *temp, *p;
	temp = *head;
 
	if(NULL == temp)
	{
		/*链表为空*/
		//printf("链表为空!\n");
		return -1;
	}
	else
	{
		/*判断匹配的元素是否为链表头部的元素*/
		if(id == temp->id)		/*是链表头部*/
		{
			DynMemPut(temp->memptr);
			temp->memptr = NULL;
			*head = temp->next;
			return 0;
		}
		else					/*不是链表头部*/
		{
			while(temp->next)
			{
				p = temp;
				temp = temp->next;
 
				if(id == temp->id)
				{
					DynMemPut(temp->memptr);
					temp->memptr = NULL;
					p->next = temp->next;
					return 0;
				}
			}	
			return -1;
		}
	}
}
 
/**将指定的id的元素所定义的内容进行修改
  * 	head	: 表示要改变元素的链表的头部的地址
  *	  id    : 表示要改变元素的标识
  *	content : 表示要改变的内容
  *	返回值  : 0-成功,-1-失败
  */
int list_chg(struct list **head, int id, void *node)
{
	struct list *temp;
 
	temp = *head;	/*将链表的头部赋值给临时 变量*/
 
	while(temp)		/* 对链表进行轮询 */
	{
		if(id == temp->id)
		{
			memset(temp->cmdst, 0, sizeof(temp->cmdst));
			memcpy(temp->cmdst,node,temp->size);
			return 0;
		}
		temp = temp->next;
	}
	return -1;
}

/**将指定id的元素所定义的内容进行查找
  * 	head	: 表示要查询的元素的链表的头部地址
  *	  id    : 表示要查询的元素的标识
  *	返回值  : 0-成功,-1-失败
*/
int list_query(struct list **head, int id,void *node)
{
	struct list *temp;
 
	temp = *head;	
 
	while(temp)
	{
		if(id == temp->id)
		{
			if( NULL != node ) {
				memcpy(node,temp->cmdst,temp->size);
			}
			return 0;
		}
		temp = temp->next;
	}
 
	return -1;
}


