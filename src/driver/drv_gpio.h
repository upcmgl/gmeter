/*
 * drv_gpio.h
 *
 * Created: 2018/12/11 星期二 11:54:21
 *  Author: Administrator
 */ 


#ifndef DRV_GPIO_H_
#define DRV_GPIO_H_

#include "../asf.h"


//GPRS
#define PORT_GPRS_POWER_CTL PIOA
#define PIN_GPRS_POWER_CTL (1<<9)

#define PORT_GPRS_RST PIOA
#define PIN_GPRS_RST (1<<10)

#define PORT_GPRS_ONOFF PIOB
#define PIN_GPRS_ONOFF (1<<5)

#define PORT_GPRS_HEAT PIOB
#define PIN_GPRS_HEAT (1<<0)

//LED
#define PORT_LED_RED PIOB
#define PIN_LED_RED (1<<15)

#define PORT_LED_GREEN PIOB
#define PIN_LED_GREEN (1<<14)

#define PORT_LED_LOCAL PIOA
#define PIN_LED_LOCAL (1<<31)

//V
#define PORT_BAT_DISCHA_CTL    PIOA
#define PIN_BAT_DISCHA_CTL (1<<28)

#define PORT_BAT_CHA_CTL    PIOA
#define PIN_BAT_CHA_CTL (1<<15)

#define PORT_LVD    PIOA
#define PIN_LVD (1<<20)

#define PORT_V_BAT   PIOA
#define PIN_V_BAT (1<<17)

#define PORT_V_TEMP   PIOA
#define PIN_V_TEMP (1<<18)


//METER IF
#define PORT_METER_STA  PIOA
#define PIN_METER_STA   (1<<16)

#define PORT_METER_SET  PIOA
#define PIN_METER_SET   (1<<29)

#define PORT_METER_EVENT  PIOA
#define PIN_METER_EVENT   (1<<30)



//FLASH SPI

#define PORT_SPI_FLASH_CS  PIOA
#define PIN_SPI_FLASH_CS  (1<<11)

#define PORT_SPI_FLASH_WP  PIOB
#define PIN_SPI_FLASH_WP  (1<<3)

#define PORT_SPI_FLASH_HOLD  PIOB
#define PIN_SPI_FLASH_HOLD  (1<<2)

#define PORT_SPI_FLASH_CLK  PIOA
#define PIN_SPI_FLASH_CLK  (1<<14)

#define PORT_SPI_FLASH_MOSI  PIOA
#define PIN_SPI_FLASH_MOSI  (1<<13)

#define PORT_SPI_FLASH_MISO  PIOA
#define PIN_SPI_FLASH_MISO  (1<<12)

//ESAM SPI
#define PORT_SPI_ESAM_CS PIOA
#define PIN_SPI_ESAM_CS (1<<19)

#define PORT_POWER_CTRL_PWM PIOA
#define PIN_POWER_CTRL_PWM  (1<<1)

void bat_discha(bool en);
INT8U bat_get_discha_state(void);
#endif /* DRV_GPIO_H_ */