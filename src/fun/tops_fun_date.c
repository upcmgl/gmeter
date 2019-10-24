#include "../main_include.h"

//系统时间： 已经过去了多少秒
INT32U  second_elapsed(INT32U last_10ms_tick)
{
   last_10ms_tick =  system_get_tick10ms() - last_10ms_tick;

   last_10ms_tick = (last_10ms_tick ) / 100;

   return  last_10ms_tick;
}

//系统时间： 已经过去了多少10毫秒
INT32U  time_elapsed_10ms(INT32U last_10ms_tick)
{
   last_10ms_tick =  system_get_tick10ms() - last_10ms_tick;

   //last_10ms_tick = (last_10ms_tick ) / 100;

   return  last_10ms_tick;
}

INT32U  second_diff(INT32U cur_10ms_tick,INT32U begin_10ms_tick)
{
    cur_10ms_tick -= begin_10ms_tick;
    begin_10ms_tick =  cur_10ms_tick/ 100;

    return begin_10ms_tick;
}

//系统时间： 已经过去了多少分钟, 这里不能采用四舍五入，确保分钟是足够的。

INT32U  minute_elapsed(INT32U begin_10ms_tick)
{

   begin_10ms_tick =  system_get_tick10ms() - begin_10ms_tick;

    begin_10ms_tick /= 6000;
    return begin_10ms_tick;

}
//---------------------------------------------------------------------------
//  根据年-月-日 计算是星期几 
//  固定世纪为20世纪
//  星期日为0，星期1为1
//  2006-6-12 更改为星期1为1，星期日为7
//---------------------------------------------------------------------------
INT8U weekDay(INT8U year,INT8U month,INT8U day)
{
    //蔡勒（Zeller）公式。即w=y+[y/4]+[c/4]-2c+[26(m+1)/10]+d-1

    //公式中的符号含义如下，w：星期；c：世纪；y：年（两位数）；
    //  m：月（m大于等于3，小于等于14，
    // 即在蔡勒公式中，某年的1、2月要看作上一年的13、14月来计算，
    //  比如2003年1月1日要看作2002年的13月1日来计算）；
    //  d：日；[ ]代表取整，即只要整数部分。

    int tmpint;
    INT8U cent;


    cent = 20;


    if(month < 3)
    {
       month += 12;
       tmpint  = year + cent*100 -1;
       year = tmpint-cent*100;
       cent = tmpint/100;
    }

    tmpint = year;
    tmpint += (year>>2);
    tmpint += (cent >>2);
    tmpint += 26*(month+1)/10;

   tmpint += day -1;

   tmpint -= (cent <<1);

   //如果计算结果小于0，采用加7的方法将结果转正
   while(tmpint < 0) tmpint +=7;


   cent =  tmpint % 7;

   //2006-6-12 ,把星期日0更改为星期日7
   if(cent == 0) cent = 7;

   return cent;

}
BOOLEAN   isLeapYear(INT16U year)
{
    BOOLEAN res;

    res = FALSE;
    if(0 ==  (year%4))
    {
         if(year%100!=0) res = TRUE;

         if(0 == (year % 400)) res = TRUE;
    }
    return res;
}

//
//   得到某月的天数
// 
INT8U getMonthDays(INT16U year,INT8U month)
{
    if(month==2)
    {
       if(TRUE == isLeapYear(year)) return 29;
       return 28;
    }    
    if((month==4)  || (month==6)  || (month==9)  ||   (month==11))
    {
       return 30;
    } 
    return 31;
}




/*+++
  功能：计算指定日期与1900-1-1比过去的天数
  参数：
        INT16U year,    四位年，>=1900
        INT8U month,    1-12
        INT8U day       1-31
  返回：
        INT16U  相比1900-1-1过去的天数
  描述：
        1）年》2000，直接从2000年开始计算。
---*/
INT16U  getPassedDays(INT16U year,INT8U month,INT8U day)
{
   INT16U days,cur_year;

   days = 0;
   cur_year = 1900;
   if(year >= 2000)
   {
      days += 36525;            // 从1900到1999年的天数
      cur_year = 2000;
   }

   //累计经过的年的天数
   for(;cur_year < year; cur_year++)
   {
      if(TRUE == isLeapYear(cur_year)) days += 366;
      else   days += 365;
   }

   //累加到月的天数
   month &= 0x0F;    //(把月份限制到低字节)
   for(cur_year=1;cur_year<month;cur_year++)
   {
       days += getMonthDays(year,cur_year);
   }

   //累加天数
   days += (day-1);

   return days;
}
/*+++
  功能：计算与1900-1-1 0：0：0 计算经历的秒差值
  参数：
         INT8U    datetime[6]    HEX   秒-分-时-日-月-年
         INT16U   centry        世纪    1900,2000
---*/
INT64U   getPassedSeconds(tagDatetime *dt,INT16U centry)
{
    INT64U  passed_sec;
    INT16U  year;

    year = dt->year + centry;

    passed_sec  = dt->hour*3600;
	passed_sec += dt->minute*60;
	passed_sec += dt->second;

    passed_sec +=   (86400ULL) * getPassedDays(year,dt->month,dt->day);

    return passed_sec;
}
/*+++

 功能: 给日期增加天数
 描述: 1)增加的天数小于29天

---*/

void  date_add_days_ext(INT8U *year,INT8U *month,INT8U *day,INT8U days,INT16U centry)
{
    INT8U month_days;

     //给日直接加上要增加的天数
     *day += days;
    
     //判断是否超过了本月的最末一天
     month_days =  getMonthDays((INT16U)( centry+(*year)),*month);
     if(*day > month_days )
     {
          *day -= month_days;
          (*month)++;
          
           //检查月份是否超过了12月,达到了下一年
           if(*month > 12)
           {
               (*year)++;
               *month = 1;
           }
     }

}

/*+++

 功能: 给日期增加天数
 描述: 1)增加的天数小于29天

---*/

void  date_add_days(INT8U *year,INT8U *month,INT8U *day,INT8U days)
{
	date_add_days_ext(year,month,day,days,2000);
}
/*+++

 功能: 给时间增加固定秒数

---*/
void datetime_add_seconds(INT8U *year,INT8U *month,INT8U *day,INT8U *hour,INT8U *minute,INT8U *second,INT32U value)
{
    INT32U residue;

    residue=value;

    //增加秒，计算分钟数
    value=residue/60;
    residue=residue%60;

    *second+=residue;
    if(*second>=60)
    {
      *second-=60;
      value++;
    }

    //增加分钟，计算小时数
    residue=value;
    value=residue/60;
    residue=residue%60;

    *minute+=residue;
    if(*minute>=60)
    {
      *minute-=60;
       value++;
    }

    //增加小时，计算天数
    residue=value;
    value=residue/24;
    residue=residue%24;

    *hour+=residue;
    if(*hour>=24)
    {
      *hour-=24;
       value++;
    }
   //增加天数
   date_add_days(year,month,day,value);
}
/*+++

 功能: 给时间增加固定分钟数

---*/
void datetime_add_minutes(INT8U *year,INT8U *month,INT8U *day,INT8U *hour,INT8U *minute,INT32U value)
{
    INT32U residue;

    //增加分钟，计算小时数
    residue=value;
    value=residue/60;
    residue=residue%60;

    *minute+=residue;
    if(*minute>=60)
    {
      *minute-=60;
       value++;
    }

    //增加小时，计算天数
    residue=value;
    value=residue/24;
    residue=residue%24;

    *hour+=residue;
    if(*hour>=24)
    {
      *hour-=24;
       value++;
    }
   //增加天数
   date_add_days(year,month,day,value);
}
/*+++

 功能: 给日期增加月数
 描述: 1)增加的月数小于12

---*/

void  date_add_months(INT8U *year,INT8U *month,INT8U val)
{
    //INT8U month_days;

     *month += val;

     //检查月份是否超过了12月,达到了下一年
     if(*month > 12)
     {
         (*year)++;
         *month = 1;
     }

}

/*+++

 功能: 给时间增加固定小时数

---*/
void datetime_add_hours(INT8U *year,INT8U *month,INT8U *day,INT8U *hour,INT32U value)
{
    INT32U residue;


    //增加小时，计算天数
    residue=value;
    value=residue/24;
    residue=residue%24;

    *hour+=residue;
    if(*hour>=24)
    {
      *hour-=24;
       value++;
    }
   //增加天数
   date_add_days(year,month,day,value);
}
void cosem_datetime_add_ti(INT8U *cosem_bin,INT8U ti[3])
{
    INT8U datetime_bin[6]={0};
    INT8U pos=0;
    INT16U year;
    INT32U seconds;

    year=cosem_bin[0]*0x100+cosem_bin[1];
    datetime_bin[YEAR]       = year-2000;
    datetime_bin[MONTH]      = cosem_bin[2];
    datetime_bin[DAY]        = cosem_bin[3];
    datetime_bin[HOUR]       = cosem_bin[4];
    datetime_bin[MINUTE]     = cosem_bin[5];
    datetime_bin[SECOND]     = cosem_bin[6];

    switch(ti[0])
    {
    case 0:
        seconds=cosem_bin2_int16u(ti+1);
        datetime_add_seconds(&datetime_bin[YEAR],&datetime_bin[MONTH],&datetime_bin[DAY],&datetime_bin[HOUR],&datetime_bin[MINUTE],&datetime_bin[SECOND],seconds);
        break;
    case 1:
        seconds=(cosem_bin2_int16u(ti+1))*60;
        datetime_add_seconds(&datetime_bin[YEAR],&datetime_bin[MONTH],&datetime_bin[DAY],&datetime_bin[HOUR],&datetime_bin[MINUTE],&datetime_bin[SECOND],seconds);
        break;
    case 2:
        seconds=(cosem_bin2_int16u(ti+1))*3600;
        datetime_add_seconds(&datetime_bin[YEAR],&datetime_bin[MONTH],&datetime_bin[DAY],&datetime_bin[HOUR],&datetime_bin[MINUTE],&datetime_bin[SECOND],seconds);
        break;
    case 3:
        seconds=cosem_bin2_int16u(ti+1);
        date_add_days(&datetime_bin[YEAR],&datetime_bin[MONTH],&datetime_bin[DAY],seconds);
        break;
    case 4:
        seconds=cosem_bin2_int16u(ti+1);
        datetime_bin[MONTH]++;
        if(datetime_bin[MONTH]>12)
        {
            datetime_bin[MONTH]=1;
            datetime_bin[YEAR]++;
        }
        break;
    default:
        return;
    }

    year=2000+datetime_bin[YEAR];
    cosem_bin[pos++] = year>>8;
    cosem_bin[pos++] = year;
    cosem_bin[pos++] = datetime_bin[MONTH];
    cosem_bin[pos++] = datetime_bin[DAY];
    cosem_bin[pos++] = datetime_bin[HOUR];
    cosem_bin[pos++] = datetime_bin[MINUTE];
    cosem_bin[pos++] = datetime_bin[SECOND];
}

void date_minus_days(INT8U *year,INT8U *month,INT8U *day,INT8U days)
{
     INT16S what_day;
     INT8U month_days;

     //给日直接减去的天数,可能出现负值
     what_day = *day;
     what_day -= days;

     if(what_day >= 1)
     {
         *day = what_day;
         return;
     }

     //日期减到了上个月
     (*month) --;
     if((*month) == 0x00)
     {
         //上一年的12月
         *month = 12;
         if((*year) > 0) (*year)--;
     }
     month_days =  getMonthDays((INT16U)(2000+(*year)),*month);
    
     *day = month_days + what_day;
}

void   date_minus_months(INT8U *year,INT8U *month,INT8U months)
{
     INT16S what_month;

     what_month = *month;
     what_month -= months;

     if((*month) == 0x00)
     {
         //上一年的12月
         *month = 12;
         if((*year) > 0) (*year)--;
     }
     *month = what_month;
}

/*+++

 功能: 给时间减去固定秒数

---*/
void datetime_minus_seconds(INT8U *year,INT8U *month,INT8U *day,INT8U *hour,INT8U *minute,INT8U *second,INT32U value)
{
    INT32U count;
    INT32U residue;
  
    residue=value;

    value = residue / 86400;
    residue = residue % 86400;
    count = (INT32U)*hour * 3600 + (INT32U)*minute * 60 + (INT32U)*second;
    if(count < residue) 
    {
        count += 86400;
        value ++; 
}

    count = count - residue;
    *hour = count / 3600;
    count = count - (INT32U)*hour * 3600;
    *minute = count / 60;
    *second = count - (INT32U)*minute * 60;

    //减去天数
    date_minus_days(year,month,day,value);
}
/*+++

 功能: 给时间减去固定分钟数

---*/
void datetime_minus_minutes(INT8U *year,INT8U *month,INT8U *day,INT8U *hour,INT8U *minute,INT32U value)
{
    INT32U count;
    INT32U residue;
  
    residue=value;

    value = residue / 1440;
    residue = residue % 1440;
    count = *hour * 60 + *minute;
    if(count < residue) 
    {
        count += 1440;
        value ++; 
    }

    count -= residue;
    *hour = count / 60;
    *minute = count - *hour * 60;

    //减去天数
    date_minus_days(year,month,day,value);
}
/*+++

 功能: 给时间减去固定小时数

---*/
void datetime_minus_hours(INT8U *year,INT8U *month,INT8U *day,INT8U *hour,INT32U value)
{
    INT32U count;
    INT32U residue;
  
    residue=value;

    value = residue / 24;
    residue = residue % 24;
    count = *hour;
    if(count < residue) 
    {
        count += 24;
        value ++; 
    }

    count -= residue;
    *hour = count;

    //减去天数
    date_minus_days(year,month,day,value);
}
//
//   命令时间增加有效时间,不可能超过1天的分钟
//

void commandDateAddMinute(CommandDate *cmdDate,INT8U minute)
{
    INT16U monthdays;

    cmdDate->minute += minute;

    if(cmdDate->minute >= 60)
    {
        cmdDate->hour += cmdDate->minute / 60;
        cmdDate->minute = cmdDate->minute % 60;
    }
    if(cmdDate->hour >= 24)
    {
        cmdDate->day +=  cmdDate->hour / 24;
        cmdDate->hour =  cmdDate->hour % 24;
    }

    //检查是否打了月末，如果是则转到下月的1日
    monthdays = getMonthDays(cmdDate->year,cmdDate->month);
    if(cmdDate->day > monthdays)
    {
        cmdDate->month++;
        cmdDate->day = 1;
    }
    if(cmdDate->month > 12)
    {
        cmdDate->month=1;
        cmdDate->year++;
    }
}

/*+++
  功能：得到昨天
  参数：
         INT8U  yesterday[3]  BIN  日-月-年
  返回：
          无
---*/
void get_yesterday(INT8U yesterday[3])
{
    tagDatetime datetime;
    
    os_get_datetime(&datetime);
    yesterday[0] = datetime.day;
    yesterday[1] = datetime.month;
    yesterday[2] = datetime.year;

    date_minus_days(&yesterday[2],&yesterday[1],&yesterday[0],1);
}

/*+++
  功能：得到前天
  参数：
         INT8U  yesterday[3]  BIN  日-月-年
  返回：
          无
---*/
void get_before_yesterday(INT8U yesterday[3])
{
    tagDatetime datetime;
    
    os_get_datetime(&datetime);
    yesterday[0] = datetime.day;
    yesterday[1] = datetime.month;
    yesterday[2] = datetime.year;

    date_minus_days(&yesterday[2],&yesterday[1],&yesterday[0],2);
}

/*+++
  功能：得到上月
  参数：
         INT8U  former_month[2]  BIN  月-年
  返回：
          无
---*/
void get_former_month(INT8U former_month[2])
{
    tagDatetime datetime;
    
    os_get_datetime(&datetime);
    if(datetime.month==1)
    {
        former_month[0] = 12;
        former_month[1] = datetime.year-1;
    }
    else
    {
        former_month[0] = datetime.month-1;
        former_month[1] = datetime.year;
    }
}

/*+++
  功能：得到参数中上月
  参数：
         INT8U  former_month[2]  BIN  月-年
  返回：
          无
---*/
void get_former_month_from_param(INT8U former_month[2])
{
    if(former_month[0]==1)
    {
        former_month[0] = 12;
        former_month[1] = former_month[1]-1;
    }
    else
    {
        former_month[0] = former_month[0]-1;
    }
}

/*+++
  功能：判断是否为昨天,即上日
  参数：
        INT8U year,          BIN
        INT8U month,         BIN
        INT8U day            BIN
  返回：
        TRUE / FALSE
---*/
BOOLEAN is_yesterday(INT8U year,INT8U month,INT8U day)
{
   INT8U yesterday[3];

   get_yesterday(yesterday);

   if(  (day != yesterday[2]) ||  (month != yesterday[1])  || (year != yesterday[0]) )
   {
      return FALSE;
   }
   return TRUE;
}
//
//   命令时间增加有效时间
//

void commandDateMinusMinute(CommandDate *cmdDate,INT8U minute)
{
    INT8U monthdays;


    cmdDate->minute -= minute;
    if(cmdDate->minute < 0)
    {
        cmdDate->minute += 60;
        cmdDate->hour --;
    }
    if(cmdDate->hour < 0)
    {
        cmdDate->hour += 24;
        cmdDate->day--;
    }
    if(cmdDate->day <= 0)
    {
        if(cmdDate->month >1)
           monthdays = getMonthDays(cmdDate->year,cmdDate->month-1);
        else
           monthdays = 31;
        cmdDate->month--;
        cmdDate->day+=monthdays;
    }
    if(cmdDate->month <=0)
    {
       cmdDate->month=12;
       cmdDate->year--;
    }
}
//该函数和上面的函数传参是带有限定条件的，hour需要在24以内，程序中并未做判定不严谨
void commandDateMinusHour(CommandDate *cmdDate,INT8U hour)
{
    INT8U monthdays;


    cmdDate->hour -= hour;
    if(cmdDate->hour < 0)
    {
        cmdDate->hour += 24;
        cmdDate->day--;
    }
    if(cmdDate->day <= 0)
    {
        if(cmdDate->month >1)
           monthdays = getMonthDays(cmdDate->year,cmdDate->month-1);
        else
           monthdays = 31;
        cmdDate->month--;
        cmdDate->day+=monthdays;
    }
    if(cmdDate->month <=0)
    {
       cmdDate->month=12;
       cmdDate->year--;
    }
}

//
//  比较两个命令日期的先后：  =0 相等  <0 cmdDate1 比cmdDate2早  > 0 cmdDate1 比cmdDate2晚
// 
INT16S commandDateCompare(CommandDate *cmdDate1,CommandDate *cmdDate2)
{


    if(cmdDate1->year < cmdDate2->year) return -1;
    else if(cmdDate1->year > cmdDate2->year) return 1;
 
    if(cmdDate1->month < cmdDate2->month) return -1;
    else if(cmdDate1->month > cmdDate2->month) return 1;
    
    if(cmdDate1->day < cmdDate2->day) return -1;
    else if(cmdDate1->day > cmdDate2->day) return 1;
   
    if(cmdDate1->hour < cmdDate2->hour) return -1;
    else if(cmdDate1->hour > cmdDate2->hour) return 1;

    if(cmdDate1->minute < cmdDate2->minute) return -1;
    else if(cmdDate1->minute > cmdDate2->minute) return 1;
    return 0;
}
INT16S DateTimeCompare(tagDatetime *Date1,tagDatetime *Date2)
{

    if(Date1->year < Date2->year ) return -1;
    else if(Date1->year > Date2->year ) return 1;
 
    if(Date1->month < Date2->month ) return -1;
    else if(Date1->month > Date2->month ) return 1;
    
    if(Date1->day < Date2->day ) return -1;
    else if(Date1->day > Date2->day ) return 1;
   
    if(Date1->hour < Date2->hour ) return -1;
    else if(Date1->hour > Date2->hour ) return 1;

    if(Date1->minute < Date2->minute ) return -1;
    else if(Date1->minute > Date2->minute ) return 1;

    if(Date1->second < Date2->second ) return -1;
    else if(Date1->second > Date2->second ) return 1;

    return 0;
}

//
//   HEX码  分[0]-时[1]-日[2]-月[3]-年[4]
void setCommandDate(CommandDate *cmdDate,INT8U *cmdDateBytes)
{

    cmdDate->year = 2000 + cmdDateBytes[4];
    cmdDate->month = cmdDateBytes[3];
    cmdDate->day =  cmdDateBytes[2];
    cmdDate->hour =  cmdDateBytes[1];
    cmdDate->minute =  cmdDateBytes[0];

}
//
//   BCD码  分[0]-时[1]-日[2]-月[3]-年[4]
void setCommandBCDDate(CommandDate *cmdDate,INT8U *cmdDateBytes)
{
    cmdDate->year = 2000 + BCD2byte(cmdDateBytes[4]);
    cmdDate->month = BCD2byte(cmdDateBytes[3]);
    cmdDate->day =  BCD2byte(cmdDateBytes[2]);
    cmdDate->hour =  BCD2byte(cmdDateBytes[1]);
    cmdDate->minute =  BCD2byte(cmdDateBytes[0]);

}

/*+++
 功能：检查日期是否为今天，BCD   日-月-年
---*/
BOOLEAN is_today_bcd(INT8U bcd_date[3])
{
    tagDatetime datetime;
    
    os_get_datetime(&datetime);
    if(BCD2byte(bcd_date[0]) != datetime.day)   return FALSE;
    if(BCD2byte(bcd_date[1]) != datetime.month) return FALSE;
    if(BCD2byte(bcd_date[2]) != datetime.year)  return FALSE;

    return TRUE;
}

BOOLEAN is_today(INT8U date[3])
{
    tagDatetime datetime;
    
    os_get_datetime(&datetime);
    if(date[0] != datetime.day)   return FALSE;
    if(date[1] != datetime.month) return FALSE;
    if(date[2] != datetime.year)  return FALSE;

    return TRUE;

}


void set_today_bcd(INT8U date[3])
{
    tagDatetime datetime;
    
    os_get_datetime(&datetime);
    date[0] = byte2BCD(datetime.day);
    date[1] = byte2BCD(datetime.month);
    date[2] = byte2BCD(datetime.year);

}

//设置:
BOOLEAN set_today_bcd4(INT8U date[4])
{
    tagDatetime datetime;
    
    os_get_datetime(&datetime);
    date[0] = byte2BCD(datetime.minute);
    date[1] = byte2BCD(datetime.hour);
    date[2] = byte2BCD(datetime.day);
    date[3] = byte2BCD(datetime.month);
    return TRUE;
}
BOOLEAN is_former_month_bcd(INT8U date[2])
{
   INT8U for_month,for_year;
    tagDatetime datetime;
    
    os_get_datetime(&datetime);

   if(datetime.month == 1)
   {
      for_month = 12;
      for_year = datetime.year - 1;
   }
   else
   {
      for_month = datetime.month - 1;
      for_year = datetime.year;
   }
   if(BCD2byte(date[0]) != for_month) return FALSE;
   if(BCD2byte(date[1]) != for_year)  return FALSE;

   return TRUE;

}




/*+++
  功能: 将BCD格式时间转为字节格式
  参数: 
      INT8U *dt 目标转换值
      INT8U *nValue 待转换值，带星期 从低到高格式为:秒-分-时-星期-日-月-年
  描述: 
---*/
void RTCBCD2DateTime(tagDatetime *dt,INT8U *nValue)
{
     dt->second = BCD2byte(nValue[0]);
     dt->minute = BCD2byte(nValue[1]);
     dt->hour =   BCD2byte(nValue[2]);
     dt->weekday =BCD2byte(nValue[3]);	
     dt->day =    BCD2byte(nValue[4]);
     dt->month =  BCD2byte(nValue[5]);
     dt->year =   BCD2byte(nValue[6]);
}
void  DateTime2RTCBCD(tagDatetime *dt,INT8U *nValue)
{
     nValue[0]=byte2BCD(dt->second);
     nValue[1]=byte2BCD(dt->minute);
     nValue[2]=byte2BCD(dt->hour);
     nValue[3]=byte2BCD(dt->weekday);	
     nValue[4]=byte2BCD(dt->day);
     nValue[5]=byte2BCD(dt->month);
     nValue[6]=byte2BCD(dt->year);
}
void  DateTime2645BCD(tagDatetime *dt,INT8U *nValue)
{
     nValue[0]=byte2BCD(dt->second);
     nValue[1]=byte2BCD(dt->minute);
     nValue[2]=byte2BCD(dt->hour);
     nValue[3]=byte2BCD(dt->day);
     nValue[4]=byte2BCD(dt->month);
     nValue[5]=byte2BCD(dt->year);
}
//比较两个时间，相差秒数是否大于s秒,s<60
BOOLEAN compare_time_second(tagDatetime *t1,tagDatetime *t2,INT8U s)
{
    INT32U   passed_min1;
    INT32U   passed_min2;
    INT16U   s1,s2;
    BOOLEAN is_t1;

    s1=t1->second;
    s2=t2->second;

    t1->second=0;
    t2->second=0;

    passed_min1 =  getPassedMinutes(t1,2000);
    passed_min2 =  getPassedMinutes(t2,2000);

    t1->second=s1;
    t2->second=s2;

    if(passed_min1 > passed_min2)
    {
       is_t1 = TRUE;
       passed_min1= (passed_min1 - passed_min2);
    }
    else
    {
       is_t1 = FALSE;
       passed_min1=(passed_min2 - passed_min1);
    }

    if(passed_min1 > 1) return TRUE;//那么肯定超过60s

    if(passed_min1 == 0) //只有秒不等，其他都相等
    {
        if(t1->second>t2->second)
           s1=(INT16U)(t1->second-t2->second);
        else
           s1=(INT16U)(t2->second-t1->second);
    }
    else if(is_t1)  //t1大 ，必须t1-t2
    {
           s1=(INT16U)(t1->second+60-t2->second);
    }
    else   //t2大 ，必须t2-t1
    {
           s1=(INT16U)(t2->second+60-t1->second);
    }

    return (s1>s);

}
/*+++
  功能：计算与1900-1-1 0：0：0 计算经历的分钟差值
  参数：
         INT8U    datetime[6]    HEX   秒-分-时-日-月-年
         INT16U   centry        世纪    1900,2000
---*/
INT32U   getPassedMinutes(tagDatetime *dt,INT16U centry)
{
    INT32U  passed_min;
    INT16U  year;

    year = (INT16U)(dt->year + centry);

    passed_min = dt->hour*60 + dt->minute;
    if(dt->second > 30)  passed_min ++;

    passed_min +=   (1440UL) * getPassedDays(year,dt->month,dt->day);

    return passed_min;
}
/*+++
  功能：计算与1900-1-1 0：0：0 计算经历的小时差值
  参数：
         INT8U    datetime[6]    HEX   秒-分-时-日-月-年
         INT16U   centry        世纪    1900,2000
---*/
INT32U   getPassedHours(tagDatetime *dt,INT16U centry)
{
    INT32U  passed_hour;
    INT16U  year;

    year = (INT16U)(dt->year + centry);

    passed_hour = dt->hour;

    passed_hour +=   (24UL) * getPassedDays(year,dt->month,dt->day);


    return passed_hour;
}
/*+++
  功能：计算两个日期的秒差
  参数：
         INT8U dt1[6],
         INT8U dt2[6]
  返回：
         INT32U 秒差值
---*/
INT32U   diff_sec_between_dt(tagDatetime *dt1,tagDatetime *dt2)
{
    INT64U   passed_sec1;
    INT64U   passed_sec2;

    passed_sec1 =  getPassedSeconds(dt1,2000);
    passed_sec2=  getPassedSeconds(dt2,2000);

    if(passed_sec1 > passed_sec2)
    {
       return (INT32U)(passed_sec1 - passed_sec2);
    }
    return (INT32U)(passed_sec2 - passed_sec1);
}
/*+++
  功能：计算两个日期的分钟差
  参数：
         INT8U dt1[6],
         INT8U dt2[6]
  返回：
         INT32U 分钟差值
---*/
INT32U   diff_min_between_dt(tagDatetime *dt1,tagDatetime *dt2)
{
    INT32U   passed_min1;
    INT32U   passed_min2;

    passed_min1 =  getPassedMinutes(dt1,2000);
    passed_min2 =  getPassedMinutes(dt2,2000);

    if(passed_min1 > passed_min2)
    {
       return (passed_min1 - passed_min2);
    }
    return (passed_min2 - passed_min1);
}
/*+++
  功能：根据cmdDate设置td
---*/
void assign_td_value(CommandDate *cmdDate,INT8U *td,INT8U count)
{
       td[0] = byte2BCD(cmdDate->minute);
       td[1] = byte2BCD(cmdDate->hour);
       td[2] = byte2BCD(cmdDate->day);
       td[3] = byte2BCD(cmdDate->month);
       td[4] = byte2BCD((INT8U)(cmdDate->year-2000));
}

void set_byte2BCD(INT8U* data,INT16U len)
{
    INT16U idx;
    for(idx=0;idx<len;idx++)
    {
       data[idx] = byte2BCD(data[idx]);
    }
}
/*+++
  功能：在1900-1-1 0：0：0 基础上增加固定秒数后的日期
  参数：
         INT8U    datetime[6]    输出   秒-分-时-日-月-年
         INT64U   value     输入   秒数
---*/
void  addSeconds_at1900(tagDatetime *dt,INT64U value)
{
    INT32U  residue;
    INT32U  val;    
    INT16U  year;
    INT8U   day,temp;    
 
	dt->second	=0;
	dt->minute	=0;
	dt->hour	=0;
	dt->day		=1;
	dt->month	=1;
	dt->year	=0;


    //增加秒，计算分钟数
    val=value/60;
    temp=value%60;
    dt->second+=temp;

    //增加分钟，计算小时数
    residue=val;
    val=residue/60;
    temp=residue%60;
    dt->minute+=temp;

    //增加小时，计算天数
    residue=val;
    val=residue/24;
    temp=residue%24;
    dt->hour+=temp;

    
    year = 1900;
    if(val>36525)
	{
		val-=36525;   // 从1900到1999年的天数
		year = 2000;	
	}
    while(val>0)
	{
	    day=28;
		if(val<day)
		{
			day=val;
		}
		val-=day;
		//增加天数
	    date_add_days_ext(&dt->year,&dt->month,&dt->day,day,year);
		
 	}   
}

void sys_datetime2cosem_datetime(INT8U *cosem_bin)
{
    INT16U year;
    tagDatetime datetime;

    os_get_datetime(&datetime);
    year=datetime.century*100+datetime.year;
    cosem_bin[0]=year>>8;
    cosem_bin[1]=year;
    cosem_bin[2]=datetime.month;
    cosem_bin[3]=datetime.day;
    cosem_bin[4]=datetime.hour;
    cosem_bin[5]=datetime.minute;
    cosem_bin[6]=datetime.second;
}
INT16U set_cosem_datetime_s(INT8U *cosem_bin,INT8U *datetime_bin)
{
	INT16U pos,year;

	pos =0;
	year=2000+datetime_bin[YEAR];

	cosem_bin[pos++] = year>>8;
	cosem_bin[pos++] = year;
	cosem_bin[pos++] = datetime_bin[MONTH];
	cosem_bin[pos++] = datetime_bin[DAY];
	cosem_bin[pos++] = datetime_bin[HOUR];
	cosem_bin[pos++] = datetime_bin[MINUTE];
	cosem_bin[pos++] = datetime_bin[SECOND];
	
	return pos;
}
