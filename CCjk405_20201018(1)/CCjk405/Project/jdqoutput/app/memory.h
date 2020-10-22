#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "typedefs.h"
#include "stdio.h"
#include "string.h"
#include "includes.h"

typedef struct
{
    void    *addr;
    uint32 size;
    uint16  tb;
}DMEM;
 
DMEM *DynMemGet(uint32_t size);
 
void DynMemPut(DMEM *pDmem);
 
 
#endif //__MEMORY_H__



