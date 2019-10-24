/*
 * drv_misc.h
 *
 * Created: 2018/12/14 星期五 19:42:23
 *  Author: Administrator
 */ 


#ifndef DRV_MISC_H_
#define DRV_MISC_H_


void tc_adc_enable(void);

void tc_callback_reg(INT8U id, pfun_no_para fun);

void drv_ir_tx_init(INT32U baud);

void drv_ir_lanch_tx(INT8U en);

void tc_ir_tx(INT8U x);

void drv_bat_cha_init();

void drv_bat_cha_start(INT8U state);

void drv_cha_pwm_width(INT8U width);

#define TC_INSTANCE(id)  ((id < 3)? TC0: TC1)
#define TC_CHANNEL(id)  ((id < 3)? id:(id-3))

#define PWM_PORT_SEL_A  0
#define PWM_PORT_SEL_B  1

#endif /* DRV_MISC_H_ */