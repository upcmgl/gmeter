/*
 * drv_gpio.c
 *
 * Created: 2018/12/11 星期二 11:54:49
 *  Author: Administrator
 */ 

#include "os.h"
#include "asf.h"
#include "drv_gpio.h"

#define DEFAULT_LV_LOW  0
#define DEFAULT_LV_HIGH 1

void bsp_io_init(void)
{
    system_peripheral_clock_enable(ID_PIOA);
    system_peripheral_clock_enable(ID_PIOB);
    
    pio_set_writeprotect(PIOA, FALSE);
    pio_set_writeprotect(PIOB, FALSE);
    
    pio_pull_up(PIOA, 0xFFFFFFFF, 0);
    pio_pull_up(PIOB, 0xFFFFFFFF, 0);
    pio_pull_down(PIOA, 0xFFFFFFFF, 0);
    pio_pull_down(PIOB, 0xFFFFFFFF, 0);
    
    //BAT
    pio_set_output(PORT_BAT_DISCHA_CTL, PIN_BAT_DISCHA_CTL, DEFAULT_LV_HIGH, 0, 0);
	pio_set_output(PORT_BAT_CHA_CTL, PIN_BAT_CHA_CTL, DEFAULT_LV_LOW, 0, 1);
    //CHA -- PWM
    //LVD,BAT，temp -- AD
    
    //GPRS  todo 默认电平
    
    pio_set_output(PORT_GPRS_RST, PIN_GPRS_RST, DEFAULT_LV_LOW, 1, 0);//输出0,开漏
	
    pio_set_output(PORT_GPRS_POWER_CTL, PIN_GPRS_POWER_CTL, DEFAULT_LV_HIGH, 0, 0);
    
    MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO5;//Select PB5
    pio_set_output(PORT_GPRS_ONOFF, PIN_GPRS_ONOFF, DEFAULT_LV_HIGH, 1, 0);//输出1，开漏
    pio_set_output(PORT_GPRS_HEAT, PIN_GPRS_HEAT, DEFAULT_LV_LOW, 0, 0);

    
    //LED
    pio_set_output(PORT_LED_RED, PIN_LED_RED, DEFAULT_LV_HIGH, 0, 0);
    pio_set_output(PORT_LED_GREEN, PIN_LED_GREEN, DEFAULT_LV_HIGH, 0, 0);
    pio_set_output(PORT_LED_LOCAL, PIN_LED_LOCAL, DEFAULT_LV_HIGH, 0, 0);
    
    //Meter-if pin
    pio_set_output(PORT_METER_STA, PIN_METER_STA, DEFAULT_LV_LOW, 1, 1);
    pio_set_input(PORT_METER_EVENT, PIN_METER_EVENT, 0);
    pio_set_input(PORT_METER_SET, PIN_METER_SET, 0);
    
    //SPI IF
    pio_set_output(PORT_SPI_FLASH_CS, PIN_SPI_FLASH_CS, DEFAULT_LV_HIGH, 0, 0);
    pio_set_output(PORT_SPI_FLASH_WP, PIN_SPI_FLASH_WP, DEFAULT_LV_HIGH, 0, 0);
    pio_set_output(PORT_SPI_FLASH_HOLD, PIN_SPI_FLASH_HOLD, DEFAULT_LV_HIGH, 0, 0);
    
    pio_set_output(PORT_SPI_ESAM_CS, PIN_SPI_ESAM_CS, DEFAULT_LV_HIGH, 0, 0);    // 原来是DEFAULT_LV_LOW
#ifdef OOP_GMETER    
    if(FILEFLAG_TYPE == FILEFLAG_TGP_GMTER_G55 )
    {
        pio_set_output(PORT_SPI_ESAM_CS, PIN_SPI_ESAM_CS, DEFAULT_LV_HIGH, 0, 0);//只有三相OOP有ESAM 
    }   
#endif
    //Idle pin..单相处理方法	
	#ifdef __METER_SIMPLE__
	#ifdef __NO_BATTERY__ //4G无超级电容
	pio_set_output(PIOB, bit(17), DEFAULT_LV_LOW, 0, 0);//输出0，推挽，不上拉
	#endif
	pio_set_output(PIOA, bit(9)|bit(15)|bit(19)|bit(21)|bit(22)|bit(28), DEFAULT_LV_LOW, 0, 0);//输出0，推挽，不上拉
	pio_set_output(PIOB, bit(0), DEFAULT_LV_LOW, 0, 0);//输出0，推挽，不上拉
	#endif
    //pio_set_output(PIOA, bit(0)|bit(1)|bit(26)|bit(25)|bit(24)|bit(2)|bit(21)|bit(22)|bit(15)|bit(28)|bit(17)|bit(19)|bit(9), DEFAULT_LV_LOW, 0, 0);//输出0，推挽，不上拉
    //pio_set_output(PIOB, bit(12)|bit(1)|bit(13)|bit(4)|bit(8)|bit(9)|bit(0)|bit(2), DEFAULT_LV_LOW, 0, 0);//输出0，推挽，不上拉  
    //三相处理
	pio_set_output(PIOA, bit(0)|bit(1)|bit(26)|bit(25)|bit(24)|bit(2), DEFAULT_LV_LOW, 0, 0);//输出0，推挽，不上拉
    pio_set_output(PIOB, bit(12)|bit(1)|bit(13)|bit(4)|bit(8)|bit(9), DEFAULT_LV_LOW, 0, 0);//输出0，推挽，不上拉
    //pio_set_writeprotect(PIOA, TRUE);
    //pio_set_writeprotect(PIOB, TRUE);
}

static INT8U bat_discha_state;
void bat_discha(bool en)
{
	if(en)
	{
		bat_discha_state = TRUE;
		pio_set_output(PORT_BAT_DISCHA_CTL, PIN_BAT_DISCHA_CTL, DEFAULT_LV_LOW, 0, 0);
	}
	else
	{
		bat_discha_state = FALSE;
		pio_set_output(PORT_BAT_DISCHA_CTL, PIN_BAT_DISCHA_CTL, DEFAULT_LV_HIGH, 0, 0);
	}
}

INT8U bat_get_discha_state(void)
{
	return bat_discha_state;
}
















