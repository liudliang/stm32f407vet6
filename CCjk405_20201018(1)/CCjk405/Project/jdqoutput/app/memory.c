//memory.c

#include "memory.h"

 
#define DMEM_BLOCK_SIZE         40       /*块大小，根据本项目配置好，请慎重修改 */
#define DMEM_BLOCK_NUM          30        /*块数量*/
#define DMEM_TOTAL_SIZE         (DMEM_BLOCK_SIZE*DMEM_BLOCK_NUM)  
 
typedef enum
{
    DMEM_FREE   = 0,
    DMEM_USED   = 1,
}DMEM_USED_ITEM;
 
typedef struct
{
    DMEM_USED_ITEM   used;
    uint16         blk_s;
    uint16         blk_num;
}DMEM_APPLY;
 
typedef struct
{
    DMEM_USED_ITEM  tb_blk[DMEM_BLOCK_NUM];
    DMEM            tb_user[DMEM_BLOCK_NUM];
    DMEM_APPLY      tb_apply[DMEM_BLOCK_NUM];
    uint16        apply_num;
    uint16        blk_num;
}DMEM_STATE;
 
static uint8_t DMEMORY[DMEM_TOTAL_SIZE];

//    DMEM_USED_ITEM  tb_blk[DMEM_BLOCK_NUM];
//    DMEM            tb_user[DMEM_BLOCK_NUM];
//    DMEM_APPLY      tb_apply[DMEM_BLOCK_NUM];
//    uint16        apply_num;
//    uint16        blk_num;

static DMEM_STATE DMEMS = {{(DMEM_USED_ITEM)0},{(DMEM_USED_ITEM)0},{(DMEM_USED_ITEM)0},0,0};
DMEM *DynMemGet(uint32 size)
{
    uint16 loop = 0;
    uint16 find = 0;
    uint16 blk_num_want = 0;
    DMEM * user = NULL;
    DMEM_APPLY *apply = NULL;
    
    if(size == 0)               {   return NULL;    }
    if(size > DMEM_TOTAL_SIZE)  {   return NULL;    }
    if(size > (DMEM_BLOCK_NUM - DMEMS.blk_num) * DMEM_BLOCK_SIZE)   {   return NULL;    }
    if(DMEMS.apply_num >= DMEM_BLOCK_NUM)   {   return NULL;    }
    
    blk_num_want = (size + DMEM_BLOCK_SIZE - 1) / DMEM_BLOCK_SIZE;
    
    for(loop = 0; loop < DMEM_BLOCK_NUM; loop++)
    {
        if(DMEMS.tb_apply[loop].used == DMEM_FREE)
        {
            apply = &DMEMS.tb_apply[loop];
            user = &DMEMS.tb_user[loop];
            user->tb = loop;
            user->size = blk_num_want * DMEM_BLOCK_SIZE;
            break;
        }
    }
    
  
    if(loop == DMEM_BLOCK_NUM)  {   return NULL;    }
    
    for(loop = 0; loop < DMEM_BLOCK_NUM; loop++)
    {
        if(DMEMS.tb_blk[loop] == DMEM_FREE)
        {
            for(find = 1; (find < blk_num_want) && (loop + find < DMEM_BLOCK_NUM); find ++)
            {
                if(DMEMS.tb_blk[loop + find] != DMEM_FREE)
                {
                    break;
                }
            }
            if(find >= blk_num_want)
            {
                user->addr = DMEMORY + loop * DMEM_BLOCK_SIZE;
                apply->blk_s = loop;                   
                apply->blk_num = blk_num_want;
                for(find = 0 ; find < apply->blk_num; find++)
                {
                    DMEMS.tb_blk[loop + find] = DMEM_USED;
                }
                apply->used = DMEM_USED;
                DMEMS.apply_num += 1;
                DMEMS.blk_num += blk_num_want;
                
                return user;
            }
            else
            {
                loop += find;
            }
        }
    }
    
    return NULL;
}
 
void DynMemPut(DMEM *user)
{
    uint16 loop = 0;
    
    if(NULL == user)    {   return; }
    
    for(loop = DMEMS.tb_apply[user->tb].blk_s; loop < DMEMS.tb_apply[user->tb].blk_s + DMEMS.tb_apply[user->tb].blk_num; loop++)
    {
        DMEMS.tb_blk[loop] = DMEM_FREE;
        DMEMS.blk_num -= 1;
    }
    DMEMS.tb_apply[user->tb].used = DMEM_FREE;
    DMEMS.apply_num -= 1;
}


