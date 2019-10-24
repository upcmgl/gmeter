#ifndef __TPOS_TIME_H__
#define __TPOS_TIME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tpos_typedef.h"

/* user specifics */

// #define CENTURY     9       // century
// #define MSECOND_H   8       // milisecond-high
// #define MSECOND_L   7       // milisecond-low
// #define WEEK        6       // week
// #define YEAR		5       // year
// #define MONTH		4       // month
// #define DAY  		3       // day
// #define HOUR		2       // hour
// #define MINUTE		1       // minute
// #define SECOND		0       // second

typedef union
{
	INT8U value[10];
	struct
	{
		INT8U second;
		INT8U minute;
		INT8U hour;
		INT8U day;
		INT8U month;
		INT8U year;
        INT8U weekday;
		INT8U msecond_h;
		INT8U msecond_l;
		INT8U century;
	};
}DateTime;

/* user specifics */

extern void   tpos_init_time(void);
extern INT8U  tpos_datetime(DateTime *date_time);
extern INT8U tpos_set_time(DateTime *date_time);
extern void   tpos_soft_rtc(void);
extern INT32U tpos_time_10ms(void);
extern INT32U tpos_elapsed_10ms(INT32U last_10ms_ticks);
extern INT32U tpos_elapsed_seconds(INT32U last_10ms_ticks);
extern INT32U tpos_elapsed_minutes(INT32U last_10ms_ticks);


extern void sleep(INT32U t);
extern void msleep(INT32U t);
extern void delay(INT32U t);
extern void tpos_delay_ms(INT32U  t) ;
extern INT32U tpos_time_ms(void);
#define OS_TIME tpos_time_ms()  //unit is ms, not 10ms
extern INT64U ms_elapsed(INT32U t) ;

void Delay_us(uint32_t us);//us延时，10ms定时器最多10*1000us,不要大于 9ms

void Delay_ns(uint32_t ns);//ns延时，10ms定时器最多10*1000us,不要大于 9ms

void Delay_ns_from(uint32_t from, uint32_t ns);//ns延时，10ms定时器最多10*1000us,不要大于 9ms

#ifdef __cplusplus
}
#endif

#endif /* __TPOS_TIME_H__ */
