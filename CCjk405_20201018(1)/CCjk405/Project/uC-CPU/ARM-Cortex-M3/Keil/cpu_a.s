;********************************************************************************************************
;                                               uC/CPU
;                                    CPU CONFIGURATION & PORT LAYER
;
;                          (c) Copyright 2004-2007; Micrium, Inc.; Weston, FL
;
;               All rights reserved.  Protected by international copyright laws.
;
;               uC/CPU is provided in source form for FREE evaluation, for educational
;               use or peaceful research.  If you plan on using uC/CPU in a commercial
;               product you need to contact Micrium to properly license its use in your
;               product.  We provide ALL the source code for your convenience and to
;               help you experience uC/CPU.  The fact that the source code is provided
;               does NOT mean that you can use it without paying a licensing fee.
;
;               Knowledge of the source code may NOT be used to develop a similar product.
;
;               Please help us continue to provide the Embedded community with the finest
;               software available.  Your honesty is greatly appreciated.
;********************************************************************************************************


;********************************************************************************************************
;
;                                            CPU PORT FILE
;
;                                              Cortex-M3
;                                            IAR C Compiler
;
; Filename      : cpu_a.asm
; Version       : V1.17
; Programmer(s) : JJL
;********************************************************************************************************


;********************************************************************************************************
;                                           EXPORT FUNCTIONS
;********************************************************************************************************

        EXPORT  CPU_IntDis
        EXPORT  CPU_IntEn

        EXPORT  CPU_SR_Save
        EXPORT  CPU_SR_Restore

;********************************************************************************************************
;                                      CODE GENERATION DIRECTIVES
;********************************************************************************************************

;        RSEG CODE:CODE:NOROOT(2)
         PRESERVE8
         AREA    RESET, CODE, READONLY


;$PAGE
;********************************************************************************************************
;                                    DISABLE and ENABLE INTERRUPTS
;
; Description: Disable/Enable interrupts.
;
; Prototypes : void  CPU_IntDis(void);
;              void  CPU_IntEn (void);
;********************************************************************************************************

CPU_IntDis
        CPSID   I
        BX      LR


CPU_IntEn
        CPSIE   I
        BX      LR


;********************************************************************************************************
;                                      CRITICAL SECTION FUNCTIONS
;
; Description : Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking, the 
;               state of the interrupt disable flag is stored in the local variable 'cpu_sr' & interrupts
;               are then disabled ('cpu_sr' is allocated in all functions that need to disable interrupts).
;               The previous interrupt state is restored by copying 'cpu_sr' into the CPU's status register.
;
; Prototypes  : CPU_SR  CPU_SR_Save   (void);
;               void    CPU_SR_Restore(CPU_SR cpu_sr);
;
; Note(s)     : (1) These functions are used in general like this:
;
;                   void  Task (void *p_arg)
;                   {
;                                                               /* Allocate storage for CPU status register */
;                   #if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
;                       CPU_SR  cpu_sr;
;                   #endif
;
;                            :
;                            :
;                       CPU_CRITICAL_ENTER();                   /* cpu_sr = CPU_SR_Save();                  */
;                            :
;                            :
;                       CPU_CRITICAL_EXIT();                    /* CPU_SR_Restore(cpu_sr);                  */
;                            :
;                            :
;                   }
;
;               (2) CPU_SR_Restore() is implemented as recommended by Atmel's application note :
;
;                      "Disabling Interrupts at Processor Level"
;********************************************************************************************************

CPU_SR_Save
        MRS     R0, PRIMASK                 ; set prio int mask to mask all (except faults)
        CPSID   I
        BX      LR


CPU_SR_Restore                                    ; See Note #2.
        MSR     PRIMASK, R0
        BX      LR


;$PAGE
;********************************************************************************************************
;                                     CPU ASSEMBLY PORT FILE END
;********************************************************************************************************

        END

