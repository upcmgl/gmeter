/**
 * \file
 *
 * \brief GCC startup file for ATSAMG55J19
 *
 * Copyright (c) 2017 Atmel Corporation, a wholly owned subsidiary of Microchip Technology Inc.
 *
 * \license_start
 *
 * \page License
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * \license_stop
 *
 */

#include "samg55j19.h"
#include "fpu.h"

/* Initialize segments */
extern uint32_t _sfixed;
extern uint32_t _efixed;
extern uint32_t _etext;
extern uint32_t _srelocate;
extern uint32_t _erelocate;
extern uint32_t _szero;
extern uint32_t _ezero;
extern uint32_t _sstack;
extern uint32_t _estack;

/** \cond DOXYGEN_SHOULD_SKIP_THIS */
int main(void);
/** \endcond */

void __libc_init_array(void);

/* Reset handler */
void Reset_Handler(void);

/* Default empty handler */
void Dummy_Handler(void);

/* Cortex-M4 core handlers */
void NonMaskableInt_Handler ( void ) __attribute__ ((weak, alias("Dummy_Handler1")));
void HardFault_Handler    ( void ) __attribute__ ((weak, alias("Dummy_Handler2")));
void MemoryManagement_Handler ( void ) __attribute__ ((weak, alias("Dummy_Handler3")));
void BusFault_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler4")));
void UsageFault_Handler   ( void ) __attribute__ ((weak, alias("Dummy_Handler5")));
void SVCall_Handler       ( void ) __attribute__ ((weak, alias("Dummy_Handler6")));
void DebugMonitor_Handler ( void ) __attribute__ ((weak, alias("Dummy_Handler7")));
void PendSV_Handler       ( void ) __attribute__ ((weak, alias("Dummy_Handler8")));
void SysTick_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler9")));

/* Peripherals handlers */
void SUPC_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void RSTC_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void RTC_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void RTT_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void WDT_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void PMC_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EFC_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void FLEXCOM7_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void FLEXCOM0_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void FLEXCOM1_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void PIOA_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void PIOB_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void PDMIC0_Handler       ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void FLEXCOM2_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void MEM2MEM_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void I2SC0_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void I2SC1_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void PDMIC1_Handler       ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void FLEXCOM3_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void FLEXCOM4_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void FLEXCOM5_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void FLEXCOM6_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TC0_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TC1_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TC2_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TC3_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TC4_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TC5_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void ADC_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void UHP_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void UDP_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void CRCCU_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));

/* Exception Table */
__attribute__ ((section(".vectors")))
const DeviceVectors exception_table = {

        /* Configure Initial Stack Pointer, using linker-generated symbols */
        .pvStack = (void*) (&_estack),

        .pfnReset_Handler              = (void*) Reset_Handler,
        .pfnNMI_Handler         = (void*) NonMaskableInt_Handler,
        .pfnHardFault_Handler          = (void*) HardFault_Handler,
        .pfnMemManage_Handler   = (void*) MemoryManagement_Handler,
        .pfnBusFault_Handler           = (void*) BusFault_Handler,
        .pfnUsageFault_Handler         = (void*) UsageFault_Handler,
        .pfnReserved1_Handler                  = (void*) (0UL), /* Reserved */
        .pfnReserved2_Handler                  = (void*) (0UL), /* Reserved */
        .pfnReserved3_Handler                  = (void*) (0UL), /* Reserved */
        .pfnReserved4_Handler                  = (void*) (0UL), /* Reserved */
        .pfnSVC_Handler             = (void*) SVCall_Handler,
        .pfnDebugMon_Handler       = (void*) DebugMonitor_Handler,
        .pfnReserved5_Handler                  = (void*) (0UL), /* Reserved */
        .pfnPendSV_Handler             = (void*) PendSV_Handler,
        .pfnSysTick_Handler            = (void*) SysTick_Handler,

        /* Configurable interrupts */
        .pfnSUPC_Handler               = (void*) SUPC_Handler,   /* 0  Supply Controller */
        .pfnRSTC_Handler               = (void*) RSTC_Handler,   /* 1  Reset Controller */
        .pfnRTC_Handler                = (void*) RTC_Handler,    /* 2  Real-time Clock */
        .pfnRTT_Handler                = (void*) RTT_Handler,    /* 3  Real-time Timer */
        .pfnWDT_Handler                = (void*) WDT_Handler,    /* 4  Watchdog Timer */
        .pfnPMC_Handler                = (void*) PMC_Handler,    /* 5  Power Management Controller */
        .pfnEFC_Handler                = (void*) EFC_Handler,    /* 6  Embedded Flash Controller */
        .pfnFLEXCOM7_Handler           = (void*) FLEXCOM7_Handler, /* 7  Flexible Serial Communication */
        .pfnFLEXCOM0_Handler           = (void*) FLEXCOM0_Handler, /* 8  Flexible Serial Communication */
        .pfnFLEXCOM1_Handler           = (void*) FLEXCOM1_Handler, /* 9  Flexible Serial Communication */
        .pvReserved10                  = (void*) (0UL),          /* 10 Reserved */
        .pfnPIOA_Handler               = (void*) PIOA_Handler,   /* 11 Parallel Input/Output Controller */
        .pfnPIOB_Handler               = (void*) PIOB_Handler,   /* 12 Parallel Input/Output Controller */
        .pfnPDMIC0_Handler             = (void*) PDMIC0_Handler, /* 13 Pulse Density Modulation Interface Controller */
        .pfnFLEXCOM2_Handler           = (void*) FLEXCOM2_Handler, /* 14 Flexible Serial Communication */
        .pfnMEM2MEM_Handler            = (void*) MEM2MEM_Handler, /* 15 Memory to Memory */
        .pfnI2SC0_Handler              = (void*) I2SC0_Handler,  /* 16 Inter-IC Sound Controller */
        .pfnI2SC1_Handler              = (void*) I2SC1_Handler,  /* 17 Inter-IC Sound Controller */
        .pfnPDMIC1_Handler             = (void*) PDMIC1_Handler, /* 18 Pulse Density Modulation Interface Controller */
        .pfnFLEXCOM3_Handler           = (void*) FLEXCOM3_Handler, /* 19 Flexible Serial Communication */
        .pfnFLEXCOM4_Handler           = (void*) FLEXCOM4_Handler, /* 20 Flexible Serial Communication */
        .pfnFLEXCOM5_Handler           = (void*) FLEXCOM5_Handler, /* 21 Flexible Serial Communication */
        .pfnFLEXCOM6_Handler           = (void*) FLEXCOM6_Handler, /* 22 Flexible Serial Communication */
        .pfnTC0_Handler                = (void*) TC0_Handler,    /* 23 Timer Counter */
        .pfnTC1_Handler                = (void*) TC1_Handler,    /* 24 Timer Counter */
        .pfnTC2_Handler                = (void*) TC2_Handler,    /* 25 Timer Counter */
        .pfnTC3_Handler                = (void*) TC3_Handler,    /* 26 Timer Counter */
        .pfnTC4_Handler                = (void*) TC4_Handler,    /* 27 Timer Counter */
        .pfnTC5_Handler                = (void*) TC5_Handler,    /* 28 Timer Counter */
        .pfnADC_Handler                = (void*) ADC_Handler,    /* 29 Analog-to-Digital Converter */
        .pfnARM_Handler                  = (void*) (0UL),          /* 30 Reserved */
        .pvReserved31                  = (void*) (0UL),          /* 31 Reserved */
        .pvReserved32                  = (void*) (0UL),          /* 32 Reserved */
        .pvReserved33                  = (void*) (0UL),          /* 33 Reserved */
        .pvReserved34                  = (void*) (0UL),          /* 34 Reserved */
        .pvReserved35                  = (void*) (0UL),          /* 35 Reserved */
        .pvReserved36                  = (void*) (0UL),          /* 36 Reserved */
        .pvReserved37                  = (void*) (0UL),          /* 37 Reserved */
        .pvReserved38                  = (void*) (0UL),          /* 38 Reserved */
        .pvReserved39                  = (void*) (0UL),          /* 39 Reserved */
        .pvReserved40                  = (void*) (0UL),          /* 40 Reserved */
        .pvReserved41                  = (void*) (0UL),          /* 41 Reserved */
        .pvReserved42                  = (void*) (0UL),          /* 42 Reserved */
        .pvReserved43                  = (void*) (0UL),          /* 43 Reserved */
        .pvReserved44                  = (void*) (0UL),          /* 44 Reserved */
        .pvReserved45                  = (void*) (0UL),          /* 45 Reserved */
        .pvReserved46                  = (void*) (0UL),          /* 46 Reserved */
        .pfnUHP_Handler                = (void*) UHP_Handler,    /* 47 USB Host Port */
        .pfnUDP_Handler                = (void*) UDP_Handler,    /* 48 USB Device Port */
        .pfnCRCCU_Handler              = (void*) CRCCU_Handler   /* 49 Cyclic Redundancy Check Calculation Unit */
};

/**
 * \brief This is the code that gets called on processor reset.
 * To initialize the device, and call the main() routine.
 */
void Reset_Handler(void)
{
        uint32_t *pSrc, *pDest;
        volatile uint32_t addr;

        /* Initialize the relocate segment */
        pSrc = &_etext;
        pDest = &_srelocate;

        if (pSrc != pDest) {
                for (; pDest < &_erelocate;) {
                        *pDest++ = *pSrc++;
                }
        }
        addr = (uint32_t) &_sstack;
        addr = (uint32_t) &_estack;
        addr = (uint32_t) &_ezero;
        /* Clear the zero segment */
        for (pDest = &_szero; pDest < &_ezero;) {
                *pDest++ = 0;
        }

        /* Set the vector table base address */
        pSrc = (uint32_t *) & _sfixed;
        SCB->VTOR = ((uint32_t) pSrc & SCB_VTOR_TBLOFF_Msk);

        /* Initialize the C library */
        __libc_init_array();
        

        /* Branch to main function */
        main();

        /* Infinite loop */
        while (1);
}

/**
 * \brief Default interrupt handler for unused IRQs.
 */
void Dummy_Handler4(void)
{
        while (1) {
        }
}
void Dummy_Handler1(void)
{
    while (1) {
    }
}
void HardFault_Handler_C(unsigned long * hardfault_args, unsigned int lr_value);
//Hard Fault
void Dummy_Handler2(void)
{
    while (1) {
            __asm volatile (
    "	bx r14							\n"
    "	.align 4						\n"
    );
    }
   
   __asm volatile (
        "   TST    LR, #4       \n"      //;测试LR寄存器的bit2是否为1
        "   ITE    EQ           \n"
        "   MRSEQ  R0, MSP      \n"
        "   MRSNE  R0, PSP      \n"
        "   MOV    R1, LR       \n"
        "   ldr r2, =HardFault_Handler_C \n"
        "   bx     r2   \n"
        "	.align 4						\n"
//41         ;如果LR的bit2是0，表示在发生HardFault之前，程序使用的是主进程堆栈Main Stack
//42         ;否则LR的bit2是1，说明在发生HF之前，程序使用是进程堆栈Process Stack
//43     
//44       ;把栈的值放入R0，把LR的值放入R1，调用C函数
);   
}

void Dummy_Handler3(void)
{
    while (1) {
    }
}

void Dummy_Handler5(void)
{
    while (1) {
    }
}

void Dummy_Handler6(void)
{
    while (1) {
    }
}
void Dummy_Handler7(void)
{
    while (1) {
    }
}

void Dummy_Handler8(void)
{
    while (1) {
    }
}

void Dummy_Handler9(void)
{
    while (1) {
    }
}

void Dummy_Handler10(void)
{
    while (1) {
    }
}

void Dummy_Handler(void)
{
    while (1) {
    }
}


