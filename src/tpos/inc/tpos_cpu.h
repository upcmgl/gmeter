/*********************************************************************
 *
 *                  CoreTimer lib function
 *
 *********************************************************************/
#ifndef __TPOS_CPU_H__
#define __TPOS_CPU_H__

#include "tpos_include.h"

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
extern void tpos_schedule_timer_open(void);

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
extern unsigned int tpos_read_schedule_timer(void);

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
extern void tpos_clr_schedule_timer(void);

extern void tpos_schedule_restart(void);

extern void tpos_schedule_stop(void);

extern void tpos_TASK_YIELD(void);

extern void tpos_schedule_handler_callback_reg(t_fun_callback fun_callback);

extern void tpos_schedule_now(void);

/*+++
 *   功能：  系统软时钟模块，定时器模块，10ms 定时
 ---*/

extern void tpos_10ms_timer_open(void);

extern INT32U tpos_10ms_timer_count(void);

extern void tpos_10ms_isr_callback_reg(t_fun_callback fun_callback);

extern void tpos_cpu_rst(void);

//for delay_us
#define SYSTICK_VAL         tpos_10ms_timer_count()
#define SYSTICK_MAX_VAL     (SYS_FREQ/100)
static inline INT32U SYSTICK_US(INT32U us)
{
    return (us*(SYS_FREQ/1000)/1000);
}
static inline INT32U SYSTICK_NS(INT32U ns)
{
    return (ns*(SYS_FREQ/1000)/1000000);
}      

void system_init_clock(void);
void system_peripheral_clock_enable(INT8U periph_id);


#endif  //ifndef