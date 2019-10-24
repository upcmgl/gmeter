#include "../main_include.h"

/*+++
  功能: 2字节二进制转换为INT16U,低字节在前

---*/
void int16u2_bin(INT16U val,INT8U bin[2])
{
    bin[0] = val & 0xFF;
    bin[1] = val>>8;
}

void int32u2_bin(INT32U val,INT8U bin[4])
{
    bin[0] = val;
    bin[1] = val>>8;
    bin[2] = val>>16;
    bin[3] = val>>24;
}
void int32u2_bin_reserve(INT32U val,INT8U bin[4])
{
    bin[3] = val;
    bin[2] = val>>8;
    bin[1] = val>>16;
    bin[0] = val>>24;
}

/*+++
  功能: 2字节二进制转换为INT16U,低字节在前

---*/
INT16U  bin2_int16u(INT8U bin[2])
{
   INT16U val;

   val = bin[1];
   val <<=8;
   val += bin[0];

   return val;
}



/*+++
  功能: 4字节二进制转换为INT32U,低字节在前
  
---*/
INT32U  bin2_int32u(INT8U bin[4])
{
   INT16S idx;
   INT32U val;

   val = 0;
   for(idx=3;idx>=0;idx--)
   {
       val<<=8;
       val += bin[idx];
   }
   return val;
}

//
//  把字节数据变成BCD格式
//
INT8U byte2BCD(INT8U bVal)
{
    INT8U ch;

    ch = ((bVal/10)<<4) + (bVal%10);

    return ch;
}
/*+++
  功能：将BCD码数据转换为整数
  参数：  unsigned char *bcdnum BCD码数据
         unsigned char bytes   BCD码数据字节数
         int multi    数值转换后需要的倍数或除数,正数，乘，负数，除
         unsigned long  *lVal  [OUT] 转换结果
  返回值: 
---*/
void BCD2Ulong(unsigned char *bcdnum,unsigned char bytes,INT16S multi,INT32U *lResult)
{
    unsigned char i,ch;

    *lResult=0;
    for(i=0;i<bytes;i++)
    {
       ch = bcdnum[bytes-1-i];
 
       //检查是否是正确的BCD数据
        //if(ch>0x99)
        if(is_valid_bcd(&ch,1) == FALSE)
        {
            ch=0x00;
        }

       *lResult = (*lResult)*100+ (ch>>4)*10+(ch&0x0F);
    }
    if(multi < 0 ) (*lResult) /= (-multi);
    else           (*lResult) *= multi;    
}

INT8U BCD2byte(INT8U bcd)
{
   return ((bcd>>4) & 0x000F) * 10 + ( bcd & 0x000F );
}

BOOLEAN is_valid_bcd_ext(INT8U *bcd,INT8U bytes)
{
   INT8U i;
   for(i=0;i<bytes;i++)
   {
      if((bcd[i] & 0x0F) > 0x0A) return FALSE;
      if((bcd[i] & 0xF0) > 0xA0) return FALSE;
   }
   return TRUE;
}
/*+++
  功能：检查是否为有效的BCD数据
  参数：
  返回：
       TRUE 有效
---*/
BOOLEAN is_valid_bcd(INT8U *bcd,INT8U bytes)
{
   INT8U i;
   for(i=0;i<bytes;i++)
   {
      if((bcd[i] & 0x0F) > 0x09) return FALSE;
      if((bcd[i] & 0xF0) > 0x90) return FALSE;
   }
   return TRUE;
}
//BCD码转换为无符号整数
INT32U  bcd2u32(INT8U *bcdnum,INT8U bytes,BOOLEAN *valid_bcd)
{
    INT32U ul;
    INT8U  i,ch;

    ul=0;
    *valid_bcd = TRUE;
    for(i=0;i<bytes;i++)
    {
       ch = bcdnum[bytes-1-i];

       //检查是否是正确的BCD数据
        if(is_valid_bcd(&ch,1) == FALSE)
       {
         ch=0x00;
         *valid_bcd = FALSE;
         return ch;
       }

       ul = ul*100+ (ch>>4)*10+(ch&0x0F);
    }
    return ul;
}
INT64U  bcd2u64(INT8U *bcdnum,INT8U bytes,BOOLEAN *valid_bcd)
{
    INT64U ul;
    INT8U  i,ch;

    ul=0;
    *valid_bcd = TRUE;
    for(i=0;i<bytes;i++)
    {
       ch = bcdnum[bytes-1-i];

       //检查是否是正确的BCD数据
        if(is_valid_bcd(&ch,1) == FALSE)
       {
         ch=0x00;
         *valid_bcd = FALSE;
         return ch;
       }

       ul = ul*100+ (ch>>4)*10+(ch&0x0F);
    }
    return ul;
}
/*+++
   功能: BCD码转换为INT16U,如果非BCD码，则按默认值返回
---*/
INT16U bcd2_int16u_def(INT8U *bcdnum,INT16U default_val)
{
    INT32U ret;
    BOOLEAN valid_bcd;
    ret=bcd2u32(bcdnum,2,&valid_bcd);
    if(valid_bcd==FALSE)
    {
     ret=default_val;
    }
    return ret;
}
/*+++
   功能: 把INT32U转换为指定字节长度的BCD码，低在前
   参数：
---*/
void ul2bcd(INT32U ul,INT8U *bcdnum,INT8U bytes)
{
    INT8U i;
    INT8U ch;

    for(i=0;i<bytes;i++)
    {
       ch = ul % 100;
       bcdnum[i] = byte2BCD(ch);
       ul /= 100;
    }
}
void u64_2fmt03(INT64U u64,INT8U *bcdnum)
{
    INT8U i;
    INT8U ch;
    INT8U mwh=0; 
    if(u64 > 9999999)
    {
        u64 /= 1000;
        mwh = 1;
    }
    for(i=0;i<4;i++)
    {
       ch = u64 % 100;
       bcdnum[i] = byte2BCD(ch);
       u64 /= 100;
    }
    bcdnum[3] &= 0x0F;
    if(mwh)
    {
        bcdnum[3] |= 0x40;
    }
}

/*+++
  功能： BCD 数据移位，相当于乘10或除10
  参数： INT8U *bcd  BCD数据
        INT8U len   BCD数据字节长度
        int shift           移位数(相当于乘10或除10)
           
---*/
void  BCDNumberShift(INT8U *bcd,INT8U len,int shift)
{
     INT32U ul;
     BOOLEAN is_valid_bcd;
     
     ul = bcd2u32(bcd,len,&is_valid_bcd);
     
     
     if(FALSE == is_valid_bcd)
         return ;
     if(shift > 0)
     {
        while(shift > 0)
        {
          ul *= 10;
          shift --;
        }
     }
     else          
     {
        shift = - shift;
        while(shift > 0)
        {
          ul /= 10;
          shift --;
        }
     }
     ul2bcd(ul,bcd,len);
}





/*+++
  功能：将BCD码数据转换为整数
  参数：  INT8U *bcdnum BCD码数据
         INT8U bytes   BCD码数据字节数
         int multi    数值转换后需要的倍数或除数,正数，乘，负数，除
         unsigned long  *lVal  [OUT] 转换结果
  返回值: 
---*/
void   bcd2Double(INT8U *bcdnum,INT8U bytes,int multi,double *dbl)
{
    BOOLEAN is_valid_bcd;

    if(bcdnum[0]==0xEE)   
    {
         //错误的BCD数据
         *dbl = 0x00;
         return;
    }

    *dbl=bcd2u32(bcdnum,bytes,&is_valid_bcd);
 
    if(multi < 0 )  *dbl /=  (-multi);
    else            *dbl *= multi;
}
/*+++
   功能：把HEX字符串转换为整数
   参数：char *hex 字符数据,长度不为偶数时,前面加零补齐
   描述：字符数据前面可以有空格，数字必须连续，之后可以有空格或结束
---*/
INT32U  hex2uint(INT8S *hex)
{
    INT32U val;

    val= 0;
    
    //首先，剔出字符数字前面的空格
    while(*hex!=0)
    {
       if(*hex==' ') 
            hex++;
       else 
            break;
    } 
    
    //然后，对字符数据从高位到低位依次进行转换，十进制，碰到第一个非数字结束
    while(*hex!=0)
    {          
        if( (*hex>='0') && (*hex<='9') )
        {
            val = (val<<4) + ((*hex)-'0');
        }
        else if( (*hex>='A') && (*hex<='F') )
        {
	        val = (val<<4) + ((*hex)-'A'+58-'0');
        }
        else
        {
           break;
        }
        hex++; 
    } 
    return val;
}

void  bin2hex(INT8U *hex,INT8U ch)
{
   INT8U c;
   c= ch >>4;
   if(c<10) hex[0]='0'+c;
   else     hex[0]='A'+c-10;

   c= ch & 0x0F;
   if(c<10) hex[1]='0'+c;
   else     hex[1]='A'+c-10;
}
INT16U  Binary2HexString(INT8U * bin,INT16U len, INT8U * buffer)
{
    INT16U idx;
    for(idx=0;idx<len;idx++)
    {
        bin2hex(&buffer[2*idx],bin[idx]);
    }
    return 2*len;
}

/*+++
  功能： 从字符串中得到数值
  参数：
          char *str;
  返回：
          数值
---*/
INT32U  str2int(INT8S *str)
{
   INT32U value,idx;
   BOOLEAN  start;

   value = 0;

   //数值前面的空格可以掠过
   start = TRUE;
   for(idx=0;idx<10;idx++)
   {
      if(*str==' ')
      {
         if(!start) break;
      }
      else if((*str < '0')  || (*str > '9') )
      {
         break;
      }
      else
      {
        start = FALSE;
        value *= 10;
        value += *str-'0';
      }
      str++;
   }
   return value;
}
void buffer_bin_to_bcd(INT8U *bin,INT8U *bcd,INT8U len)
{
   while(len > 0)
   {
      *bcd =  byte2BCD(*bin);
      bcd++;
      bin++;
      len --;
   }
}
void buffer_bcd_to_bin(INT8U *bcd,INT8U *bin,INT8U len)
{
   while(len > 0)
   {
      *bin =  BCD2byte(*bcd);
      bcd++;
      bin++;
      len --;
   }
}
//把数据转换为BCD格式
void to_bcd(INT8U *data,INT8U len)
{
   while(len > 0)
   {
      *data =  byte2BCD(*data);
      data++;
      len --;
   }
}
//把数据转换为byte格式
void to_byte(INT8U *data,INT8U len)
{
   while(len > 0)
   {
      *data =  BCD2byte(*data);
      data++;
      len --;
   }
}
/*+++
   功能：把字符串转换为整数
   参数：char *str 字符数据
   描述：字符数据前面可以有空格，数字必须连续，之后可以有空格或结束
---*/
INT16U   str2uint(INT8S *str)
{
    unsigned int val;

    val= 0;
    
    //首先，剔出字符数字前面的空格
    while(*str!=0)
    {
       if(*str==' ') 
            str++;
       else 
            break;
    } 
    
    //然后，对字符数据从高位到低位依次进行转换，十进制，碰到第一个非数字结束
    while(*str!=0)
    {          
        if( (*str>='0') && (*str<='9') )
        {
            val = val *10 + ((*str)-'0');
        }
        else
        {
           break;
        }
        str++; 
    } 
    return val;
}

INT16U Str2UintBuff(INT8S *str,INT16U len,INT8U *buf)
{
    INT16U pos,iii;
    INT8S ch1,ch2;
    
    pos = 0;
    for(iii=0;iii<len;iii+=2)
    {
        ch1 = str[iii];
        ch2 = str[iii+1];

        if(ch1 <='9') ch1 -= '0';
        else          ch1 -= 'A'-10;

        if(ch2 <='9') ch2 -= '0';
        else          ch2 -='A'-10;

        buf[pos++] = (ch1 << 4) + ch2;
    }
    return pos;
}

/*+++
  功能：字符格式的IP地址转换为4字节IP地址
  参数：char *str_server_ip  字符格式的IP  例如“222.41.144.45”
        INT8U server_ip[4]  4字节IP地址
  
---*/
void   str2ip(INT8S *str_server_ip, INT8U server_ip[4])
{
     unsigned int val;
     INT8U idx;
     
     for(idx=0;idx<4;idx++)
     {
        //转换当前字节
        val = str2uint(str_server_ip);
        server_ip[idx] = val;
        
        //转移到下一字节地址的起始位置，特征为找到字符'.'之后的第一个
        while(*str_server_ip !=0)
        {
           if(*str_server_ip == '.') 
           {
              str_server_ip++;
              break;
           } 
           str_server_ip++;
        }          
     }
}

INT16U  Hex2Binary(INT8U * hex,INT16U len, INT8U * buffer)
{
       INT16U i;

       char ch;
       if(hex==NULL) return 0;

       for(i=0;i<len;i+=2)
       {
           ch=hex[i];
           *buffer = 0;
           if(ch >='0' && ch <='9') *buffer += ch-'0';
           else if(ch >='a' && ch <='f')  *buffer += ch-'a'+10;
           else if(ch >='A' && ch <='F')  *buffer += ch-'A'+10;
           else  return -1;  // 错误的HEX
           *buffer *= 16;

           ch=hex[i+1];
           if(ch >='0' && ch <='9') *buffer += ch-'0';
           else if(ch >='a' && ch <='f')  *buffer += ch-'a'+10;
           else if(ch >='A' && ch <='F')  *buffer += ch-'A'+10;
           else  return -1;  // 错误的HEX
           buffer++;
       }
       return len/2;
} 

/*****************************************************
 * 功能：
 *     将ASCII码转化为BCD格式
 * 描述：
 *     该函数，暂时只将数字形式的ASCII码转化为BCD格式
 * 参数：
 *     INT8U *ascii  ASCII码的buffer
 *     INT8U *bcd    转化后的BCD码的buffer
 *     INT8U len     ASCII码对应的buffer长度
 * 返回值：
 *     BOOLEAN  成功转化了返回TRUE，否则返回FALSE
 * 备注：
 *     1）因为ASCII码的2个字节，才能构成1个BCD的数字，所以len原则上要考虑2的倍数的处理
 *****************************************************/
BOOLEAN ascii_to_bcd(INT8U *ascii,INT8U *bcd,INT8U len)
{
    INT8U ch;
    INT8U idx;
    
    ch = 0;
    idx= 0;
    
    for(idx = 0;idx < (len / 2);idx++)
    {
        //非数字的ASCII直接返回FALSE
        if((ascii[idx*2] < 0x30) || (ascii[idx*2] > 0x39))
            return FALSE;
        if((ascii[idx*2+1] < 0x30) || (ascii[idx*2+1] > 0x39))
            return FALSE;
        ch = ascii[idx*2] - 0x30;
        ch <<= 4;
        ch += ascii[idx*2+1] - 0x30;
        bcd[idx] = ch;
    }
    
    return TRUE;

}

//oop转换函数
INT8U cosem_int162_bin(INT16U val,INT8U bin[2])
{
    bin[1] = val;
    bin[0] = val>>8;
    return 2;
}
INT16U cosem_bin2_int16u(INT8U bin[2])
{
   INT16U val;

   val = bin[0];
   val <<=8;
   val += bin[1];

   return val;
}
INT8U cosem_int162_bcd(INT16U val,INT8U bin[2])
{
   INT8U ch;
   ch = val % 100;
   bin[1] = byte2BCD(ch);
   val /= 100;

   ch = val % 100;
   bin[0] = byte2BCD(ch);

    return 2;
}
INT16U cosem_bcd2_int16u(INT8U bin[2])
{
   INT16U val;
   INT8U ch;
   val=0;
   
   ch=bin[0];
   val=val*100+ (ch>>4)*10+(ch&0x0F);
   ch=bin[1];
   val=val*100+ (ch>>4)*10+(ch&0x0F);

   return val;
}
INT8U cosem_int32u2_bin(INT32U val,INT8U bin[4])
{
    bin[3] = val;
    bin[2] = val>>8;
    bin[1] = val>>16;
    bin[0] = val>>24;
    return 4;
}
INT32U  cosem_bin2_int32u(INT8U bin[4])
{
   INT16S idx;
   INT32U val;

   val = 0;
   for(idx=0;idx<4;idx++)
   {
       val<<=8;
       val += bin[idx];
   }
   return val;
}
INT32U  cosem_bin2_unit(INT8U *bin,INT8U size)
{
   INT16S idx;
   INT32U val;

   val = 0;
   for(idx=0;idx<size;idx++)
   {
       val<<=8;
       val += bin[idx];
   }
   return val;
}

void  cosem_bin2_int64u(INT8U bin[8],INT64U *val)
{
   INT16S idx;
   INT64U value;

   value = 0;
   for(idx=0;idx<8;idx++)
   {
       value<<=8;
       value += bin[idx];
   }

   *val = value;
}
INT8U cosem_int64u2_bin(INT64U val,INT8U bin[8])
{
    INT8S idx;
    for(idx=7;idx>=0;idx--)
    {
        bin[idx]=(INT8U)(val);
        val>>=8;
    }
    return 8;
}

void cosem_bin2_int64s(INT8U bin[8],INT64S *val)
{
   INT16S idx;
   INT64S value;

   value = 0;
   for(idx=0;idx<8;idx++)
   {
       value<<=8;
       value += (INT64S)bin[idx];
   }

   *val = value;
}

INT8U get_datetime_from_cosem(INT8U *cosem_bin,INT8U *datetime_bin)
{
    INT16U year;

    year=cosem_bin[0]*0x100+cosem_bin[1];

    datetime_bin[YEAR]       = year-2000;
    datetime_bin[MONTH]      = cosem_bin[2];
    datetime_bin[DAY]        = cosem_bin[3];
    datetime_bin[HOUR]       = cosem_bin[4];
    datetime_bin[MINUTE]     = cosem_bin[5];
    datetime_bin[SECOND]     = cosem_bin[6];

    return 6;
}

INT16U  get_cosem_data_len(INT8U *data,INT8U *len_size)
{
    INT16U num;

    if(data[0] & 0x80)
    {
        num=cosem_bin2_unit(data+1,data[0]& 0x7F);
        *len_size=(data[0]& 0x7F)+1;
    }
    else
    {
        num=data[0];
        *len_size=1;
    }

    return num;

}
INT16U set_cosem_data_len(INT8U *data,INT16U len)
{

    if(len>=0x80)
    {
        data[0]=0x81;
        if(len>255)
        {
            data[0]++;
            data[1]=len>>8;
            data[2]=len;
            return 3;
        }
        else
        {
            data[1]=len;
            return 2;
        }

    }
    else
    {
        data[0]=len;
    }
    return 1;
}
/*+++
功能：字符格式的IP地址转换为4字节IP地址
参数：char *str_server_ip  字符格式的IP  例如“222.41.144.45”
INT8U ip[4]  4字节IP地址

---*/
void  ip2str(INT8U  ip[4],INT8S *str_server_ip)
{
INT8U val =0;
INT8U idx,server_ip[4];

mem_cpy(server_ip, ip,4);
for(idx=0;idx<4;idx++)
{
if( (server_ip[idx]/100) != 0)
{
*(str_server_ip) = (server_ip[idx]/100)+'0';
server_ip[idx] =server_ip[idx] - (*(str_server_ip)-'0')*100;
str_server_ip++;
val=1;
}
if( (server_ip[idx]/10) != 0 ||val)
{
*(str_server_ip) = (server_ip[idx]/10)+'0';
server_ip[idx] =server_ip[idx] - (*(str_server_ip)-'0')*10;
str_server_ip++;
val=1;
}

if( (server_ip[idx]) != 0 ||val)
{
*(str_server_ip) = (server_ip[idx])+'0';
str_server_ip++;
}
if(idx<3)
*(str_server_ip++) = '.';
}
*(str_server_ip ) =0x0;  //最后结束符
}
