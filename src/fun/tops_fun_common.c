#include "../main_include.h"
/*+++
  功能： 查找指定字符窜
----*/
INT16S  str_find(INT8U *buffer,INT16U buflen,INT8U *target,INT16U targetlen)
{
    INT16U pos;
    INT16U idx;

    for(pos=0;pos<buflen;pos++)
    {
        for(idx=0;idx<targetlen;idx++)
        {
           if(target[idx]!=buffer[idx]) break;
        }
        if(idx>=targetlen) return pos;
        buffer++;
    }
    return -1;
}



/*+++
  功能： ASCII命令的长度,以0结尾
---*/
INT16U  str_cmdlen(INT8U *cmd)
{
   INT16U len;

   len = 0;
   if(cmd == NULL) 	return len;
   while(*cmd)
   {
      len ++;
      cmd ++;
      if(len > 512) break;
   }
   return len;
}
/*+++
  功能：检查是否为合法的电表地址
  参数：
       INT8U meter_no[6]
       BOOLEAN is_meter   true 电表  false 采集器
  描述：
      1）电表地址可以为全00，采集器地址全00 无效
      2）电表地址不能有FF
      3）电表地址不能为全99
      4）电表地址不能为全AA
---*/
BOOLEAN isvalid_meter_addr(INT8U meter_no[6],BOOLEAN is_meter)    //__zy_set_meter__
{
    INT8U idx;
    INT8U zero_count;
    INT8U count_99;
    INT8U count_AA;

   zero_count = 0;
   count_99   = 0;
    count_AA = 0;

   for(idx=0;idx<6;idx++)
   {
      if(meter_no[idx]==0x00) zero_count++;
      if(meter_no[idx]==0x99) count_99++;
        #ifdef __ALLOW_LETTER_METER_NO__
        if((idx == 3) && (is_meter == TRUE)) continue;
        if((idx == 5) && (is_meter == TRUE)) continue;
        #endif
        if(meter_no[idx] == 0xAA)
      {
            count_AA++;
         continue;
      }
       #ifndef __ALLOW_LETTER_METER_NO__   // 上海 不判断 字母表号  2017-5-17  暂时（全部协议）

      if((meter_no[idx] & 0x0F) > 0x09) return FALSE;
      if((meter_no[idx] & 0xF0) > 0x90) return FALSE;
      #endif
   }

    if(is_meter == FALSE)
    {
        if(zero_count == 6) return FALSE;
    }
   
   if(count_99==6) return FALSE;
    if(count_AA == 6) return FALSE;
    
   return TRUE;
}

/*+++
 功能：交换电表地址的字节顺序
 参数：
       INT8U meter_no[6]  [输入/输出] 电表地址
 返回：
       无
       0<->5
       1<->4
       2<->3
---*/
void  swap_meter_addr(INT8U meter_no[6])
{
    INT8U tmp;
    INT8U idx;

    for(idx=0;idx<3;idx++)
    {
      tmp = meter_no[idx];
      meter_no[idx] = meter_no[5-idx];
      meter_no[5-idx] = tmp;
    }
}
//将6字节的表号转为12位的表号字符串

void meteraddr2str(INT8U meter_no[6],INT8U *tmp_str)
{
    INT8U i;
    for(i=0;i<6;i++)
    {
      if(((meter_no[5-i]>>4)&0x0f)>=0x0a)
	*tmp_str++ = '0';
      else
	*tmp_str++ = ((meter_no[5-i]>>4)&0x0f)+'0';
      if((meter_no[5-i]&0x0f)>=0x0a)
	*tmp_str++ = '0';
      else
	*tmp_str++ = (meter_no[5-i]&0x0f)+'0';
    }
    return;
}
/*+++
   功能：检查数据是否全是FF
   参数：
         INTU *data,   数据
         INT16U len    数据长度
   返回：
          TRUE   FALSE
---*/
BOOLEAN check_is_all_FF(INT8U *data,INT16U len)
{
   while(len > 0)
   {
      if(*data != 0xFF) return FALSE;
      data++;
      len --;
   }
   return TRUE;
}
/*+++
   功能：检查数据是否全是指定数据
   参数：
         INTU *data,   带检查缓冲区数据
         INT16U len    数据长度
         INT8U ch     目的字符
   返回：
          TRUE   FALSE
---*/
BOOLEAN check_is_all_ch(INT8U *data,INT16U len,INT8U ch)
{
   while(len > 0)
   {
      if(*data != ch) return FALSE;
      data++;
      len --;
   }
   return TRUE;
}
void sortdata(INT16U *array,INT16U val,INT8U len)
{
   INT8U idx,pos;
    	
   //查找当前值的插入位置
   pos=0xFF;
   for(idx=0;idx<len;idx++)
   {
       if(val>array[idx])
       {
           pos=idx;
           break;
       } 
   }
   if(pos!=0xFF)
   {
       for(idx=len-1;idx>pos;idx--)
       {
           array[idx]=array[idx-1];
       }
       array[pos]=val;  
   }

}
#define PPPINITFCS16 0xffff /* Initial FCS value */
#define PPPGOODFCS16 0xf0b8 /* Good final FCS value */

/*
* FCS lookup table as calculated by the table generator.
*/
const INT16U  /*__MYOSTEXT */fcstab[256] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,

    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

/*
* Calculate a new fcs given the current fcs and the new data.
*/
INT16U pppfcs16(INT16U fcs, INT8U *cp, INT16S len)
{


   while(len--)
   {
      fcs = (fcs >> 8) ^ fcstab[(fcs ^ *cp++) & 0xff];
   }
   return (fcs);
}

INT16U fcs16_16u(INT8U *frame, INT16S len)
{
    INT16U trialfcs;

    /* add on output */
    trialfcs = pppfcs16( PPPINITFCS16, frame, len );
    trialfcs ^= 0xffff; /* complement */
    return trialfcs;
}

/*+++
   功能: 计算FCS校验
   参数: INT8U *frame  祯数据,从祯格式开始
         int len               代计算校验的字节数量
   描述: 校验值被放在祯数据的末尾2字节
---*/

void fcs16(INT8U *frame, INT16S len)
{
   INT16U trialfcs;

   /* add on output */
   trialfcs = pppfcs16( PPPINITFCS16, frame, len );
   trialfcs ^= 0xffff; /* complement */
   frame[len] = (trialfcs & 0x00ff); /* least significant byte first */
   frame[len+1] = ((trialfcs >> 8) & 0x00ff);
}

//算法：cpu当前执行指令数、开机时间
//INT32U simple_random(void)
//{
//   INT32U count;
//
//   DelayNmSec(7);
//   count=SYSTEM.systick_in_10ms;
//   count |= os_get_cp0count();
//   srand(count);
//   count = rand(); 
//
//   return count;
//
//}
void app_softReset(void)
{
#ifndef __SOFT_SIMULATOR__
   force_gprs_power_off();
   DelayNmSec(3000);//等待复位后，连接断开
   tpos_softReset();
#endif
}

/*+++
  功能：计算测量点电能量差值
  参数：
        INT8U *data,    电量示值列表
        INT16U points   点数
  返回：
        INT16U 数据长度
  描述：
       1）初始值放在最前面
       2）计算后差值的数量为 points-1
---*/
INT16U   compute_spot_amounts(INT8U *data,INT16U points)
{
   INT16U idx;
   INT32U init_value,cur_value;
   BOOLEAN  is_valid;

   //
   init_value = bcd2u32(data,4,&is_valid);
   if(is_valid == FALSE)
   {
       for(idx=1;idx<points;idx++)
       {
           mem_set(data+(idx-1)*4,4,0xEE);  //无效数据
       }
       return (points-1)<<2;    
   }
   for(idx=1;idx<points;idx++)
   {
      cur_value = bcd2u32(data+idx*4,4,&is_valid);
      if((is_valid)&&(cur_value >= init_value))
      {
        cur_value -= init_value;
        cur_value *= 100;   //数据格式，相差100倍
        ul2bcd(cur_value,data+(idx-1)*4,4);
      }
      else
      {
         mem_set(data+(idx-1)*4,4,0xEE);  //无效数据
      }
   }

   return (points-1)<<2;

}

BOOLEAN is_same_Td3(INT8U td1[3],INT8U td2[3])
{
    if(    (td1[0] != td2[0] )
        || (td1[1] != td2[1] )
        || (td1[2] != td2[2] ) ) return FALSE;
    return TRUE;
}

/*+++
  功能：计算抄表时标的日期，比请求时标早1日
  参数：
          INT8U td[3],          请求时标   日-月-年  BCD
  返回：
          无
  描述：

---*/
void previous_dayhold_td(INT8U td[3])
{
    INT8U  year,month,day;

    day = BCD2byte(td[0]);
    month = BCD2byte(td[1]);
    year = BCD2byte(td[2]);
    date_minus_days(&year,&month,&day,1);
    td[0] = byte2BCD(day);
    td[1] = byte2BCD(month);
    td[2] = byte2BCD(year);
}

/*+++
  功能：计算抄表时标的日期，比请求时标晚1日
  参数：
          INT8U td[3],          请求时标   日-月-年  BCD
          INT8U rec_td[3]
  返回：
          无
  描述：

---*/
void next_dayhold_td(INT8U td[3],INT8U rec_td[3])
{
    INT8U  year,month,day,monthday;

    mem_cpy(rec_td,td,3);

    day   = BCD2byte(rec_td[0]);

    day++;

    if(day <= 28)
    {
       rec_td[0] = byte2BCD(day);
}
    else
    {
       //检查是否大于本月的月末日
       year =   BCD2byte(rec_td[2]);
       month =  BCD2byte(rec_td[1]);
       monthday = getMonthDays(2000+year,month);
       if(day > monthday)
       {
            month ++;
            if(month > 12)
            {
               month = 1;
               year++;
            }
            rec_td[0] = 0x01;
            rec_td[1] = byte2BCD(month);
            rec_td[2] = byte2BCD(year);
       }
       else
       {
            rec_td[0] = byte2BCD(day);
       }
    }

}

/*+++
  功能：计算抄表时标的日期，比请求时标早1月
  参数：
          INT8U td[2],          请求时标   月-年  BCD
          INT8U rec_td[2]
  返回：
          无
  描述：

---*/
void previous_monthhold_td_BCD(INT8U td[2])
{
    INT8U  year,month;
    INT8U  rec_td[2];

    mem_cpy(rec_td,td,2);

    month = BCD2byte(rec_td[0]);
    month--;

    if((month == 0) || (month > 12))
    {
        td[0] = byte2BCD(12);
        year = BCD2byte(rec_td[1]) - 1;
        if(year > 99) year = 99;
        td[1] = byte2BCD(year);
    }
    else
    {
        td[0] = byte2BCD(month);
    }
}

BOOLEAN check_is_valid_ipaddr(uint8_t *ip)
{
    if(check_is_all_ch(ip,4,0x00))
    {
        return FALSE;
    }
    if(check_is_all_ch(ip,4,0xFF))
    {
        return FALSE;
    }
    return TRUE;
}
BOOLEAN check_is_valid_port(uint8_t *port)
{
    if(check_is_all_ch(port,2,0x00))
    {
        return FALSE;
    }
    if(check_is_all_ch(port,2,0xFF))
    {
        return FALSE;
    }
    return TRUE;
}
