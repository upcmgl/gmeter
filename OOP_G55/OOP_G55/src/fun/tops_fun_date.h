#ifndef __TOPS_FUN_DATE_H__
#define __TOPS_FUN_DATE_H__

//#include "dev_var.h"

#define WEEKDAY     9       //??
#define CENTURY     8       //????
#define MSECOND_H   7       //???????????10????
#define MSECOND_L   6       //???????????10????
#define YEAR		5       //??????
#define MONTH		4       //?
#define DAY  		3       //?
#define HOUR		2       //?
#define MINUTE		1       //?
#define SECOND		0       //?

INT32U  second_elapsed(INT32U last_10ms_tick);
INT32U  time_elapsed_10ms(INT32U last_10ms_tick);
INT32U  second_diff(INT32U cur_10ms_tick,INT32U begin_10ms_tick);
INT32U  minute_elapsed(INT32U begin_10ms_tick);
INT8U weekDay(INT8U year,INT8U month,INT8U day);
BOOLEAN   isLeapYear(INT16U year);
INT8U getMonthDays(INT16U year,INT8U month);
INT16U  getPassedDays(INT16U year,INT8U month,INT8U day);
INT64U   getPassedSeconds(tagDatetime *dt,INT16U centry);
void  date_add_days(INT8U *year,INT8U *month,INT8U *day,INT8U days);
void   date_minus_days(INT8U *year,INT8U *month,INT8U *day,INT8U days);
void   date_minus_months(INT8U *year,INT8U *month,INT8U months);
void datetime_add_seconds(INT8U *year,INT8U *month,INT8U *day,INT8U *hour,INT8U *minute,INT8U *second,INT32U value);
void datetime_add_minutes(INT8U *year,INT8U *month,INT8U *day,INT8U *hour,INT8U *minute,INT32U value);
void  date_add_months(INT8U *year,INT8U *month,INT8U val);
void datetime_add_hours(INT8U *year,INT8U *month,INT8U *day,INT8U *hour,INT32U value);
void datetime_minus_seconds(INT8U *year,INT8U *month,INT8U *day,INT8U *hour,INT8U *minute,INT8U *second,INT32U value);
void datetime_minus_minutes(INT8U *year,INT8U *month,INT8U *day,INT8U *hour,INT8U *minute,INT32U value);
void datetime_minus_hours(INT8U *year,INT8U *month,INT8U *day,INT8U *hour,INT32U value);
void commandDateAddMinute(CommandDate *cmdDate,INT8U minute) ;
void commandDateMinusMinute(CommandDate *cmdDate,unsigned char minute);
void get_yesterday(INT8U yesterday[3]);
void get_before_yesterday(INT8U yesterday[3]);
void get_former_month(INT8U former_month[2]);
void get_former_month(INT8U former_month[2]);
void commandDateMinusMinute(CommandDate *cmdDate,INT8U minute);
INT16S commandDateCompare(CommandDate *cmdDate1,CommandDate *cmdDate2);
INT16S DateTimeCompare(tagDatetime *Date1,tagDatetime *Date2);
void setCommandDate(CommandDate *cmdDate,INT8U *cmdDateBytes);
void setCommandBCDDate(CommandDate *cmdDate,INT8U *cmdDateBytes);
BOOLEAN is_today_bcd(INT8U bcd_date[3]);
BOOLEAN is_today(INT8U date[3]);
void set_today_bcd(INT8U date[3]);
BOOLEAN set_today_bcd4(INT8U date[4]);
BOOLEAN is_former_month_bcd(INT8U date[2]);
void RTCBCD2DateTime(tagDatetime *dt,INT8U *nValue);
void  DateTime2RTCBCD(tagDatetime *dt,INT8U *nValue);
void  DateTime2645BCD(tagDatetime *dt,INT8U *nValue);
BOOLEAN compare_time_second(tagDatetime *t1,tagDatetime *t2,INT8U s);
INT32U   getPassedMinutes(tagDatetime *dt,INT16U centry);
INT32U   getPassedHours(tagDatetime *dt,INT16U centry);
INT32U   diff_sec_between_dt(tagDatetime *dt1,tagDatetime *dt2);
INT32U   diff_min_between_dt(tagDatetime *dt1,tagDatetime *dt2);
void assign_td_value(CommandDate *cmdDate,INT8U *td,INT8U count);
void  addSeconds_at1900(tagDatetime *dt,INT64U value);
void commandDateMinusHour(CommandDate *cmdDate,INT8U hour);
void cosem_datetime_add_ti(INT8U *cosem_bin,INT8U ti[3]);
void sys_datetime2cosem_datetime(INT8U *cosem_bin);
INT16U set_cosem_datetime_s(INT8U *cosem_bin,INT8U *datetime_bin);
#endif
