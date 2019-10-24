/*
 * drv_misc.c
 *
 * Created: 2018/12/14 星期五 19:42:05
 *  Author: Administrator
 */ 
#include "asf.h"
#include "os.h"

/* TIMER    */
pfun_no_para tc_callback[6] = 
{
    0,  //TC0-0
    0,  //TC0-1
    0,  //TC0-2
    0,  //TC1-0
    0,  //TC1-1
    0   //TC1-2 
};


//AD -TC0-2
void tc_general_init(INT8U chn , INT32U t)//t - unit is us
{
    Tc *tc;
    INT32U rc,rc_pre,cmr,Hz;

    system_peripheral_clock_enable(ID_TC0+chn);
    tc= TC_INSTANCE(chn);
    chn = TC_CHANNEL(chn);
    
    cmr = 0;
    rc_pre = 2;
    Hz = 1000000/t;

    rc = (( SYS_FREQ /rc_pre /Hz ) - 1);
    while(rc > 30000)
    {
        if(cmr >= 3)
        {
            return ;//ERROR!!
        }
        cmr = cmr+1;
        rc = ((( SYS_FREQ/(rc_pre * 4 * cmr) ) /Hz) - 1);
    }

    tc->TC_WPMR = 0x54494d00;
    tc->TC_BMR = 0;   
    
    tc->TC_CHANNEL[chn].TC_RC = rc;
    tc->TC_CHANNEL[chn].TC_CMR = cmr|1<<14;
    
    tc->TC_CHANNEL[chn].TC_IDR = 0xffffffff;
    tc->TC_CHANNEL[chn].TC_IER = 0x00000010;

    //tc->TC_WPMR = 0x54494d01;
    
    NVIC_ClearPendingIRQ(TC0_IRQn+chn);
    NVIC_SetPriority(TC0_IRQn+chn, 0);
    NVIC_EnableIRQ(TC0_IRQn+chn);
}
/*
    paras:  AB-     0-TIOA; 1-TIOB
            duty    duty:100=low-level:ALL
*/
void tc_pwm_init(INT8U chn, INT8U AB, INT32U Hz, INT8U duty)
{
    Tc *tc;
    INT32U rAB, rc,rc_pre,cmr;

    system_peripheral_clock_enable(ID_TC0+chn);
    tc= TC_INSTANCE(chn);
    chn = TC_CHANNEL(chn);      
    
    cmr = 0;
    rc_pre = 2;

    rc = (( SYS_FREQ /rc_pre /Hz ) - 1);
    while(rc > 30000)
    {
        if(cmr >= 3)
        {
            return ;//ERROR!!       
        }
        cmr = cmr+1;
        rc = ((( SYS_FREQ/(rc_pre * 4 * cmr) ) /Hz) - 1);
    }

    rAB = rc*duty/100;

    tc->TC_WPMR = 0x54494d00;
    tc->TC_BMR = 0;
    
    cmr = cmr|0x02<<13|1<<15;//enable waveform 10 mode
    if(AB == PWM_PORT_SEL_A)//TIOA
    {
        cmr = cmr|1<<10;//TIOA  default:0=TIOB
    }
    cmr = cmr|1<<16|2<<18;//enable waveform ,RA/B SET ,RC Clear
    tc->TC_CHANNEL[chn].TC_CMR = cmr; 
    
    if(AB == PWM_PORT_SEL_A)//TIOA
    {
        tc->TC_CHANNEL[chn].TC_RA = rAB;
    }
    else//TIOB
    {
        tc->TC_CHANNEL[chn].TC_RB = rAB;
    }
    tc->TC_CHANNEL[chn].TC_RC = rc;

    tc->TC_CHANNEL[chn].TC_IDR = 0xffffffff;
    tc->TC_CHANNEL[chn].TC_IER = 0x00000010;

    tc->TC_WPMR = 0x54494d01;

    NVIC_EnableIRQ(TC0_IRQn+chn);
    NVIC_SetPriority(TC0_IRQn+chn, 0);
}

/* APP IF */
#define ID_IR   TMR_ID_IR
#define ID_IR_TX    TMR_ID_IR_PWM
#define ID_BAT  TMR_ID_BAT_CHA_PWM
/*
    IR - timer      TC0-ch0
    IR - pwm-tx     TC0-ch1
    
*/
//IR -TC0 -TIOA1
void tc_ir_callback(void);
void tc_ir_toogle(void);
void drv_ir_tx_init(INT32U baud)
{
    tc_general_init(ID_IR, 1000000/baud);
    tc_callback_reg(ID_IR, tc_ir_callback);
    
    tc_pwm_init(TMR_ID_IR_PWM, PWM_PORT_SEL_A, 38000, 33);//TC0-chn1, TIOA, 38K Hz, dutyH:dutyL=1:2 

    pio_set_writeprotect(PORT_IR_TX, FALSE);
    pio_set_output(PORT_IR_TX, PIN_IR_TX, 0, 0, 0);
	
}
void tc_ir_toogle()
{
    pio_toggle_pin_group(PORT_IR_TX, PIN_IR_TX);
}
void drv_ir_lanch_tx(INT8U enable)
{
    if(enable)
    {
       tc_start(TC_INSTANCE(ID_IR), TC_CHANNEL(ID_IR)); 
    }
    else
    {
        tc_stop(TC_INSTANCE(ID_IR), TC_CHANNEL(ID_IR));
    }
}
void tc_ir_tx(INT8U x)
{
    if(x)
    {
        tc_stop(TC_INSTANCE(ID_IR_TX), TC_CHANNEL(ID_IR_TX));//stop pwm
        pio_set_writeprotect(PORT_IR_TX, FALSE);
        pio_set_output(PORT_IR_TX, PIN_IR_TX, 0, 0, 0);
    }
    else
    {
        pio_set_writeprotect(PIOA, FALSE);
        pio_set_peripheral(PORT_IR_TX, PIO_PERIPH_B, PIN_IR_TX);

        tc_start(TC_INSTANCE(ID_IR_TX), TC_CHANNEL(ID_IR_TX));//start pwm
    }
}

/* BAT CHA*/
extern void Task_bat_cha(void );


void drv_cha_pwm_width(INT8U width)
{
	#ifdef  __BAT_PWM__
	pio_set_writeprotect(PIOA, FALSE);
	pio_set_peripheral(PORT_POWER_CTRL_PWM, PIO_PERIPH_B, PIN_POWER_CTRL_PWM);
	tc_pwm_init(TMR_ID_BAT_CHA_PWM, PWM_PORT_SEL_B, 38000, width);//TC0-chn1, TIOA, 38K Hz
	tc_start(TC_INSTANCE(ID_BAT), TC_CHANNEL(ID_BAT));//start pwm
	#endif
}
void drv_bat_cha_init()
{
    #ifndef  __BAT_PWM__
	tc_general_init(ID_BAT, 200);//200us
    tc_callback_reg(ID_BAT, Task_bat_cha);
	#else
	//drv_cha_pwm_width(1);
	#endif
}
void drv_bat_cha_start(INT8U state)
{
	#ifndef  __BAT_PWM__
	if(state)
	{
		#ifdef __METER_SIMPLE_NB__
		pio_set_pin_group_high(PORT_BAT_CHA_CTL,PIN_BAT_CHA_CTL);
		#else
		tc_start(TC_INSTANCE(ID_BAT), TC_CHANNEL(ID_BAT));
		#endif
	}
	else
	{
		#ifdef __METER_SIMPLE_NB__
		pio_set_pin_group_low(PORT_BAT_CHA_CTL,PIN_BAT_CHA_CTL);
		#else
		tc_stop(TC_INSTANCE(ID_BAT), TC_CHANNEL(ID_BAT));
		#endif
	}
	#endif

}


void tc_callback_reg(INT8U id, pfun_no_para fun)
{
    if(id > 5)
        return;
    tc_callback[id] = fun;
}

INT32U tmr_read_status(INT8U id)
{
    volatile INT32U status;
    status = TC_INSTANCE(id)->TC_CHANNEL[TC_CHANNEL(id)].TC_SR;
}
void TC0_Handler()
{
    tmr_read_status(0);
    if(tc_callback[0])
    {
        tc_callback[0]();
    }
}
void TC1_Handler()
{
    tmr_read_status(1);
    if(tc_callback[1])
    {
        tc_callback[1]();
    }
}
void TC2_Handler()
{
    tmr_read_status(2);
    if(tc_callback[2])
    {
        tc_callback[2]();
    }
}
void TC3_Handler()
{
    tmr_read_status(3);
    if(tc_callback[3])
    {
        tc_callback[3]();
    }
}
void TC4_Handler()
{
    tmr_read_status(4);
    if(tc_callback[4])
    {
        tc_callback[4]();
    }
}

//TC5  TC1












