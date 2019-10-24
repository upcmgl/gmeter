/*
 * drv_adc.h
 *
 * Created: 2018/12/14 星期五 20:58:24
 *  Author: Administrator
 */ 


#ifndef DRV_ADC_H_
#define DRV_ADC_H_


void Task_ad_init( void );
void Task_ad_start( void );




INT16S drv_adc_get_LVD(void);
INT16S drv_adc_get_Vbat(void);
INT16S drv_adc_get_Temp(void);



#endif /* DRV_ADC_H_ */