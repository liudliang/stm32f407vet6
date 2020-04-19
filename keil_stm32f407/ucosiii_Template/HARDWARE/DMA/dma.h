#ifndef __DMA_H
#define	__DMA_H	   
#include "sys.h"
	 

//void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx,u32 par,u32 mar,u16 ndtr);//≈‰÷√DMAx_CHx
void sys_ADC1_Config(void);
u16 *DMA_GetAdcAver(void);	   
#endif






























