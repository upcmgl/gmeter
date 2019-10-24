/*********************************************************************
 *
 *                  CoreTimer lib function
 *
 *********************************************************************/

#include "tpos_typedef.h"
#include "tpos.h"
#include "tpos_cpu.h"
#include "tpos_time.h"
#include "os.h"

#include "samg55j19.h"
#include "core_cm4.h"

#include "tpos_include.h"//#define SYS_FREQ                65536000u
#include "dev_var.h"

#define TPOS_TIMER_INSTANCE     TC_INSTANCE(TMR_ID_OS)
#define TPOS_TIMER_CHANNEL      TC_CHANNEL(TMR_ID_OS)   //tmr0

#define ID_TMR_TPOS ID_TC5
#define IRQ_TMR_TPOS    TC5_IRQn
#define TC_TOPS_Handler TC5_Handler


static void _tpos_timer_isr_handler(void);

static t_fun_callback _tpos_schedule_handler_callback = NULL;
static t_fun_callback _tpos_10ms_isr_callback = NULL;


unsigned int tpos_read_schedule_timer(void)
{
    unsigned int timer;

    timer = TPOS_TIMER_INSTANCE->TC_CHANNEL[TPOS_TIMER_CHANNEL].TC_CV;

    return timer;
}

unsigned int tpos_read_schedule_timer_status(void)
{
    unsigned int status;

    status = TPOS_TIMER_INSTANCE->TC_CHANNEL[TPOS_TIMER_CHANNEL].TC_SR;

    return status;
}

/*********************************************************************
 * Core Timer Control Interrupt Functions
 *
 * Function:        void OpenCoreTimer(unsigned int period)
 *
 * PreCondition:    None
 *
 * Inputs:			period - 32 bit value for the timer
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 ********************************************************************/
void tpos_schedule_timer_open(void)
{
    system_peripheral_clock_enable(ID_TMR_TPOS);
    
    TPOS_TIMER_INSTANCE->TC_WPMR = 0x54494d00;
    //deinit
    TPOS_TIMER_INSTANCE->TC_CHANNEL[TPOS_TIMER_CHANNEL].TC_CCR = 0x00000002;
    tpos_read_schedule_timer_status();

    
    TPOS_TIMER_INSTANCE->TC_BMR = 0;
    
    TPOS_TIMER_INSTANCE->TC_CHANNEL[TPOS_TIMER_CHANNEL].TC_CMR = 0x00000002|1<<14;
    TPOS_TIMER_INSTANCE->TC_CHANNEL[TPOS_TIMER_CHANNEL].TC_RC = ((SYS_FREQ/ 32 / 500) - 1);//2ms
    TPOS_TIMER_INSTANCE->TC_CHANNEL[TPOS_TIMER_CHANNEL].TC_RB = 0xffff;
    TPOS_TIMER_INSTANCE->TC_CHANNEL[TPOS_TIMER_CHANNEL].TC_RA = 0xffff;
    TPOS_TIMER_INSTANCE->TC_CHANNEL[TPOS_TIMER_CHANNEL].TC_IDR = 0xffffffff;
    TPOS_TIMER_INSTANCE->TC_CHANNEL[TPOS_TIMER_CHANNEL].TC_IER = 0x00000010;

    TPOS_TIMER_INSTANCE->TC_WPMR = 0x54494d01;

    TPOS_TIMER_INSTANCE->TC_CHANNEL[TPOS_TIMER_CHANNEL].TC_CCR = 0x00000005;


    NVIC_ClearPendingIRQ(IRQ_TMR_TPOS);
    NVIC_SetPriority(IRQ_TMR_TPOS, 0xFF);//NVIC->IP[24] = 0xff;
    NVIC_EnableIRQ(IRQ_TMR_TPOS);//NVIC->ISER[0] = 1<<24;
}

/******************************************************************************
 * Function:        unsigned int ReadCoreTimer(void)
 *
 * Description:		Returns the current Core Timer value.
 *
 * PreCondition:    None
 *
 * Inputs:			None
 *
 * Output:          Count value.
 *
 * Example:			ReadCoreTimer()
 *
 *****************************************************************************/


/*********************************************************************
 * Core Timer
 *
 * Function:        void UpdateCoreTimer(unsigned int period)
 *
 * PreCondition:    None
 *
 * Inputs:			period - 32 bit value for the timer
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 ********************************************************************/
void tpos_clr_schedule_timer(void)
{
    TPOS_TIMER_INSTANCE->TC_CHANNEL[TPOS_TIMER_CHANNEL].TC_CCR = 0x00000005;
}

void tpos_schedule_restart(void)
{
    NVIC_EnableIRQ(IRQ_TMR_TPOS);
}

void tpos_schedule_stop(void)
{
    NVIC_DisableIRQ(IRQ_TMR_TPOS);
}

void tpos_TASK_YIELD(void)
{

    NVIC_SetPendingIRQ(IRQ_TMR_TPOS);
}

void tpos_schedule_handler_callback_reg(t_fun_callback fun_callback)
{
    _tpos_schedule_handler_callback = fun_callback;
}

void tpos_schedule_now(void)
{
    SCB->SHP[10] = 0xff;
    SCB->ICSR = 0x10000000;

	__asm volatile( "dsb" ::: "memory" );										
	__asm volatile( "isb" );
}

uint8_t dev_power_lost(void);
INT8U ad_is_rdy(void);
void ad_rdy_clear(void);
void TC_TOPS_Handler(void)
{
	tpos_read_schedule_timer_status();
    tpos_clr_schedule_timer();

    if(NULL != _tpos_schedule_handler_callback)
    {
        _tpos_schedule_handler_callback();
    }

    tpos_switch();        
}

/*+++
 *   功能：  系统软时钟模块，定时器模块，10ms 定时
 ---*/
void tpos_10ms_timer_open(void)
{
    SysTick->LOAD = SYS_FREQ/500 - 1;
	SysTick->VAL = 0;
	SysTick->CTRL = 0x06;
	SysTick->CTRL |= 0x01;
}

INT32U tpos_10ms_timer_count(void)
{
    INT32U value;
    
    value = SysTick->VAL;
    
    return value;
}

void tpos_10ms_isr_callback_reg(t_fun_callback fun_callback)
{
    _tpos_10ms_isr_callback = fun_callback;
}

void timer10ms_call(void );
volatile INT8U ii; 
volatile INT8U once=0; 
void SysTick_Handler(void)
{
    INT32U csr_value;
	    
    csr_value = SysTick->CTRL;
    if (csr_value){}//Coverity
	
	if(++ii > 4)
	{
		ii = 0;
		_tpos_timer_isr_handler();
		
		timer10ms_call();			
	}
	
	
	if(ad_is_rdy())
	{
		ad_rdy_clear();
		Task_ad_start();
	}
	if(dev_power_lost())
	{
		if((gSystemInfo.dev_temp >-25) && (gSystemInfo.dev_temp < 65) && (gSystemInfo.batter_power_time == 0))
		{
			
			bat_discha(TRUE);
			if(!once)
			{
				gSystemInfo.batter_power_time = OS_TIME;
				once = 1;
			}
		}
	}
	else
	{
		once = 0;
		gSystemInfo.batter_power_time = 0;
	}

}
INT8U addr[10];
static void _tpos_timer_isr_handler(void)
{
    void tpos_service_10ms(void);
    void tpos_pulse_10ms_process(void);

    SYSTEM.systick_in_10ms ++;
    ClrWdt();
    
    //操作系统的10ms处理
    if(TRUE == SYSTEM.tpos_running)
    {
        tpos_service_10ms();
    }
    
    tpos_soft_rtc();

    if(_tpos_10ms_isr_callback != NULL)
    {
        _tpos_10ms_isr_callback();
    }
}

void tpos_cpu_rst(void)
{
    SCB->AIRCR = 0x05FA0004; //SCB_AIRCR @ 0xE000ED0C in ARM Core.
}

extern OS_TCB * volatile pxCurrentTCB;
extern OS_TCB * volatile pxNextTCB;


void os_start(void)
{
	/* Start the first task.  This also clears the bit that indicates the FPU is
	in use in case the FPU was used before the scheduler was started - which
	would otherwise result in the unnecessary leaving of space in the SVC stack
	for lazy saving of FPU registers. */
    
	__asm volatile(
					" ldr r0, =0xE000ED08 	\n" /* Use the NVIC offset register to locate the stack. */
					" ldr r0, [r0] 			\n"
					" ldr r0, [r0] 			\n"
					" msr msp, r0			\n" /* Set the msp back to the start of the stack. */
					" mov r0, #0			\n" /* Clear the bit that indicates the FPU is in use, see comment above. */
					" msr control, r0		\n"
					" cpsie i				\n" /* Globally enable interrupts. */
					" cpsie f				\n"
					" dsb					\n"
					" isb					\n"
					" svc 0					\n" /* System call to start first task. */
					" nop					\n"
				);
}

void SVCall_Handler( void )
{
    __asm volatile (
    "	ldr	r3, =pxCurrentTCB		    \n" /* Restore the context. */
    "	ldr r1, [r3]					\n" /* Use pxCurrentTCBConst to get the pxCurrentTCB address. */
    "	ldr r0, [r1]					\n" /* The first item in pxCurrentTCB is the task top of stack. */
    "	ldmia r0!, {r4-r11}		        \n" /* Pop the registers that are not automatically saved on exception entry and the critical nesting count. */
    "	msr psp, r0						\n" /* Restore the task stack pointer. */
    "	isb								\n"
    "	orr r14, r14, #13    			\n"
    "	bx r14							\n"
    "									\n"
    "	.align 4						\n"
    );
}



void PendSV_Handler( void )
{
    __asm volatile
    (
    "	mrs r0, psp							\n"
    "	isb									\n"
    "										\n"
    "	ldr	r3, =pxCurrentTCB			    \n" /* Get the location of the current TCB. */
    "	ldr	r2, [r3]						\n"
    "										\n"
    "	stmdb r0!, {r4-r11}			        \n" /* Save the core registers. */
    "	str r0, [r2]						\n" /* Save the new top of stack into the first member of the TCB. */
    "										\n"
    "	stmdb sp!, {r0}					\n"
	"	dsb									\n"
	"	isb									\n"
    "	    								\n"
    "	ldr	r2, =pxNextTCB				    \n" /* Get the location of the next TCB. */
    "	ldr r0, [r2]						\n" 
    "	str r0, [r3]						\n" /* Update the current TCB. */
    "	    								\n"
    "	ldmia sp!, {r0}			    		\n"
    "										\n"
    "	ldr r1, [r3]						\n" /* The first item in pxCurrentTCB is the task top of stack. */
    "	ldr r0, [r1]						\n"
    "										\n"
    "	ldmia r0!, {r4-r11}			\n" /* Pop the core registers. */
    "										\n"
    "	msr psp, r0							\n"
    "	isb									\n"
    "										\n"
    "										\n"
    "	bx r14								\n"
    "										\n"
    "	.align 4							\n"
    );
}


static void driver_set_regulator_trim_value(void)
{
    //#define EFC_FCMD_STUI    0x0E  //!< Start unique ID
    //#define EFC_FCMD_SPUI    0x0F  //!< Stop unique ID
    //INT32U unique_id[32];
    //INT32U trim_value;
    //INT32U supcPwr;
    //uint8_t trim_value1; 
    //supcPwr = SUPC->SUPC_PWMR;
    ///* Get the trim value from unique ID area */
    //efc_perform_read_sequence(EFC, EFC_FCMD_STUI, EFC_FCMD_SPUI,unique_id, 32);
    //trim_value = (unique_id[16]);
    //trim_value1 =   (trim_value & 0x0F);
    //
    //SUPC->SUPC_PWMR = 0x5AFF0100 | (trim_value1 << 9);
    
    SUPC->SUPC_SMMR = 0x01<<8 | 0x09<<0 | 1<<12;   //打开VDDIO monitor 电压2.69V
}

/** 
 * Initialize clock.
 */
void system_init_clock(void)
{
    INT32U tmp_val;
    INT8U i;
	/* Set FWS for embedded Flash access according to operating frequency 70MHz*/
    EFC->EEFC_FMR = EEFC_FMR_FWS(6)|EEFC_FMR_CLOE;

    //driver_set_regulator_trim_value();
    
    // Enable 32.768KHz external crystal oscillator..
    SUPC->SUPC_CR = 0xA5000008;
    
    //Reset Control
    RSTC->RSTC_MR = 0xA5000301;
    
    // unprotect..
    PMC->PMC_WPMR = 0x504D4300;    
    
    //32768 SCLK Monitor..
    //PMC->CKGR_MOR = 1<<26|0<<24| 1<<3 ;
    //PMC->CKGR_MOR |= 0<<4;//8M

    // set PLLA output 65.536MHz PLLACK..
    PMC->CKGR_PLLAR = 0x07CF0801;
    // wait PLLA Lock..
    while(!(PMC->PMC_SR&0x00000002));

    // Change Master Clock to PLLACK..
    PMC->PMC_MCKR = 0x00000002;
    // wait Master Clock ready..
    while(!(PMC->PMC_SR&0x00000008));

    // set programmable clock register..
    for(i=0; i<8; i++)
    {
        PMC->PMC_PCK[i] = 0x00000002;
        while(!(PMC->PMC_SR&(0x00000100<<i)));
    }
}

void system_peripheral_clock_enable(INT8U periph_id)
{
    // unprotect..
    PMC->PMC_WPMR = 0x504D4300;
    if(periph_id < 32)
    {
        PMC->PMC_PCER0 = 0x00000001<<periph_id;
    }
    else if(periph_id < 64)
    {
        PMC->PMC_PCER1 = 0x00000001<<(periph_id-32);
    }
    else
    {
        return;
    }
}



