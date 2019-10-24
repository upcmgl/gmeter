
#include "tpos_include.h"
#include "tpos_service.h"
#include "tpos_time.h"
#include "tpos_cpu.h"

/* all internal functions */


/* all internal variables */
//static DateTime   _datetime;
DateTime   _datetime;

static void cmn_mem_cpy(void *dst,void *src,INT16U len)
{
   INT16U i;

   for(i=0;i<len;i++)
   {
       ((INT8U *)dst)[i] = ((INT8U *)src)[i];
   }

}

static INT16S  cmn_compare_string(INT8U *str1,INT8U *str2,INT16U len)
{
    while(len > 0)
    {
        if(*str1 < *str2) return -1;
        if(*str1 > *str2) return 1;
        str1++;
        str2++;
        len--;
    }
    return 0;
}



void tpos_init_time(void)
{
	tpos_enterCriticalSection();

    _datetime.msecond_h = 0;
    _datetime.msecond_l = 0;
    _datetime.second = 0;
    _datetime.minute = 0;
    _datetime.hour   = 0;
    _datetime.weekday   = 6;
    _datetime.day    = 1;
    _datetime.month  = 1;
    _datetime.year   = 15;

	tpos_leaveCriticalSection();
}

INT8U tpos_set_time(DateTime *date_time)
{
    _datetime.msecond_l = 0x00;
    _datetime.msecond_h = 0x00;
    
    cmn_mem_cpy(&_datetime, date_time, sizeof(DateTime));
    
    return TRUE;
}

INT8U tpos_datetime(DateTime *date_time)
{
    DateTime date_time1, date_time2;
    INT8U i;

    tpos_enterCriticalSection();

    for(i=0; i<3; i++)
    {
    	cmn_mem_cpy(&date_time1, &_datetime, sizeof(DateTime));
    	cmn_mem_cpy(&date_time2, &_datetime, sizeof(DateTime));

    	if(0 == cmn_compare_string((INT8U *)&date_time1, (INT8U *)&date_time2, sizeof(DateTime)))
    	{
    		cmn_mem_cpy(date_time, &date_time1, sizeof(DateTime));
    		tpos_leaveCriticalSection();
    		return TRUE;
    	}
    }

    tpos_leaveCriticalSection();

    return FALSE;
}


void tpos_soft_rtc(void)
{
    INT16U leap_year;
    INT16U msec;
    
   
    msec = _datetime.msecond_h;
    msec <<= 8;
    msec += _datetime.msecond_l;

    /* milisecond */
    msec += 10;
     
    
    if(msec >= 1000)
    {
        msec = 0;
        _datetime.second++;
    }

    _datetime.msecond_h = msec>>8;

    _datetime.msecond_l = msec;
  
//return ;
    /* second */
    if(_datetime.second >= 60)
    {
        _datetime.second = 0;

        /* minute */
        _datetime.minute++;
		if(_datetime.minute >= 60)
		{
			_datetime.minute =0;
		
			_datetime.hour++;
			/* hour */
			if(_datetime.hour >= 24)
			{
				_datetime.hour = 0;

				/* day, month */
				_datetime.day++;
				if(_datetime.day > 28)
				{
					if(_datetime.month == 2)
					{
						leap_year = (INT16U)(2000 + _datetime.year);

						if((leap_year%400 == 0)  ||
						   ((leap_year%4  == 0)  &&
							(leap_year%100 != 0)))
						{
							if(_datetime.day > 29)
							{
							   _datetime.day = 1;
							   _datetime.month++;
							}
						}
						else
						{
							_datetime.day = 1;
							_datetime.month++;
						}
					}
					else if((_datetime.month == 4) || (_datetime.month == 6) ||
							(_datetime.month == 9) || (_datetime.month == 11))
					{
						if(_datetime.day > 30)
						{
							_datetime.day = 1;
							_datetime.month++;
						}
					}
					else if(_datetime.day > 31)
					{
						_datetime.day = 1;
						_datetime.month++;
					}

					/* year */
					if(_datetime.month > 12)
					{
						_datetime.month = 1;
						_datetime.year++;
					}
				}
			}
		}
    }
}

extern INT32U os_get_systick_10ms(void);

INT32U tpos_time_10ms(void)
{
	INT32U current_10ms_ticks;
	current_10ms_ticks = os_get_systick_10ms();

	return current_10ms_ticks;
}

INT32U tpos_elapsed_10ms(INT32U last_10ms_ticks)
{
	INT32U elapsed_10ms_ticks;
	elapsed_10ms_ticks = tpos_time_10ms() - last_10ms_ticks;

	return elapsed_10ms_ticks;
}

INT32U tpos_elapsed_seconds(INT32U last_10ms_ticks)
{
	INT32U elapsed_10ms_ticks;
	elapsed_10ms_ticks = tpos_time_10ms() - last_10ms_ticks;

	return elapsed_10ms_ticks / 100;
}

INT32U tpos_elapsed_minutes(INT32U last_10ms_ticks)
{
	INT32U elapsed_10ms_ticks;
	elapsed_10ms_ticks = tpos_time_10ms() - last_10ms_ticks;

	return elapsed_10ms_ticks / 6000;
}

//void tpos_delay_msec(INT32U delay)
//{
    //tpos_sleep(delay);
//}


void delay(INT32U t)
{
    tpos_sleep(t*1000);
}

void sleep(INT32U t)
{
    tpos_sleep(t*1000);
}

void tpos_delay_ms(INT32U t)
{
    tpos_sleep(t);
}

void msleep(INT32U t)
{
    tpos_sleep(t);
}
INT32U tpos_time_ms()
{
    INT32U now = os_get_systick_10ms();
    
    return now;
}

INT64U ms_elapsed(INT32U t)
{
    INT32U now = OS_TIME;
    
    return 10*(now - t); 
} 

void Delay_us(uint32_t us)//us延时，10ms定时器最多10*1000us,不要大于 9ms
{
    int32_t tick, base_val, end_val;

    base_val = SYSTICK_VAL;
    end_val = base_val - SYSTICK_US(us);//count down
    tick = base_val;
    while(1)
    {
        tick = SYSTICK_VAL;
        if(tick > base_val)
        {
            tick -= SYSTICK_MAX_VAL;
        }
        if(tick < end_val)
        {
            break;
        }
    }
}

void Delay_ns(uint32_t ns)//ns延时，10ms定时器最多10*1000us,不要大于 9ms
{
    int32_t tick, base_val, end_val;

    base_val = SYSTICK_VAL;
    end_val = base_val - SYSTICK_NS(ns);//count down
    tick = base_val;
    while(1)
    {
        tick = SYSTICK_VAL;
        if(tick > base_val)
        {
            tick -= SYSTICK_MAX_VAL;
        }
        if(tick < end_val)
        {
            break;
        }
    }
}
void Delay_ns_from(uint32_t from, uint32_t ns)//ns延时，10ms定时器最多10*1000us,不要大于 9ms
{
    int32_t tick, base_val, end_val;

    base_val = from;
    end_val = base_val - SYSTICK_NS(ns);//count down
    tick = base_val;
    while(1)
    {
        tick = SYSTICK_VAL;
        if(tick > base_val)
        {
            tick -= SYSTICK_MAX_VAL;
        }
        if(tick < end_val)
        {
            break;
        }
    }
}