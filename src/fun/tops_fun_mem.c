#include "../main_include.h"

/*+++
  功能：设置缓冲区数据
---*/

void   mem_set(void *buffer,INT16U len,INT8U value)
{
    INT8U *ch_buffer;

    ch_buffer = (INT8U *)buffer;
    while(len!=0)
    {
        *ch_buffer++ = value;
        len--;
    }
}

//内存复制
//注意一定要从左向右进行复制，因为有可能是同一数组内的复制。
void    mem_cpy(void *dst,void *src,INT16U len)
{
INT16U i;

   for(i=0;i<len;i++)
   {
       ((INT8U *)dst)[i] = ((INT8U *)src)[i];
   }

}
void mem_cpy_reverse(INT8U *dst,INT8U *src,INT16U len)
{
    INT16U idx;
    for(idx=0;idx<len;idx++)
    {
        dst[idx] = src[len-idx-1];
    }
}

//内存复制
//注意一定要从右向左进行复制，因为有可能是同一数组内的复制。
void mem_cpy_right(void *dst,void *src,INT16U len)
{
   INT16U i;

   for(i=0;i<len;i++)
   {
       ((INT8U *)dst)[len-1-i] = ((INT8U *)src)[len-1-i];
   }

}

//内存都是同一个字符
INT8U  mem_all_is(void *dst,INT16U len,  INT8U c)
{
   INT8U  *p;

   p = (INT8U *)dst;
   while(len--)
   {
        if((*p++) != c)    return FALSE;
   }
   return TRUE;
}

//内存都是同一个字符
INT8U  mem_is_same(void *dst,void * src,INT16U len)
{
   INT8U  *p;
   INT8U  *q;
   p = (INT8U *)dst;
   q = (INT8U *)src;
   while(len--)
   {
        if((*p++) != (*q++))    return FALSE;
   }
   return TRUE;
}
//字节内位顺序从D0到D7, DL698

#define BITSTRING_MASK   0x01


//字节内位顺序从D7~D0,南方电网规约,62056位串
//#define BITSTRING_MASK   0x80


/*++++
  功能：检查位串的某个位置是1或0
  参数：INT8U *BS           位串
       INT8U bitStringLen  位串字节长度
       int  bitpos                第？位     0:代表bit0 
  描述：bit0-bit7  顺序0-7
 
----*/
INT8U get_bit_value(INT8U *BS,INT16U bitStringlen,INT16U bitpos)
{
    INT8U  byte_no;
    if(bitpos > 7)
    {
        byte_no = bitpos >> 3;
        BS += byte_no;
        if(bitStringlen<=byte_no) return 0;
        //bitStringlen-=byte_no;//未使用的值
        bitpos &= 0x07;
    }
    return (*BS) & (BITSTRING_MASK<<bitpos);

}

//  获取位串中1的个数
INT32U get_bit_count(INT8U *BS,INT16U bitStringlen)
{
	INT32U idx_byte,count_bit;
	INT8U idx_bit;
	count_bit=0;
	for(idx_byte=0; idx_byte<bitStringlen; idx_byte++)
	{
		for(idx_bit=0; idx_bit<8; idx_bit++)
		{
			if(BS[idx_byte] & (1<<idx_bit))
				count_bit++;
		}
	}
return count_bit;

}

/*++++
  功能：设置位串的某个位置是1
  参数：INT8U *BS           位串
       INT8U bitStringLen  位串长度
       int  bitpos                第？位     0:代表bit0  
----*/

void set_bit_value(INT8U *BS,INT16U bitStringlen,INT16U bitpos)
{
   INT8U  byte_no;
   if(bitpos > 7)
   {
         byte_no = bitpos >> 3;
         BS += byte_no;
         if(bitStringlen<=byte_no) return;
         //bitStringlen-=byte_no;//未使用的值
         bitpos &= 0x07;
   }
   (*BS) |=   (BITSTRING_MASK<<bitpos);
}

/*++++
  功能：设置位串的某个位置是0
  参数：INT8U *BS           位串
       INT8U bitStringLen  位串长度
       int  bitpos                第？位     0:代表bit0  
----*/

void  clr_bit_value(INT8U *BS,INT16U bitStringlen,INT16U bitpos)
{
   INT8U  byte_no;
   if(bitpos > 7)
   {
         byte_no = bitpos >> 3;
         BS += byte_no;
         if(bitStringlen<=byte_no) return;
         //bitStringlen-=byte_no;//未使用的值
         bitpos &= 0x07;
   }
   (*BS) &=   ~(BITSTRING_MASK<<bitpos);
}

 //设置位串取反
void bit_value_opt_inversion(INT8U *BS,INT16U bitStringlen)
{
    INT16U idx;

    for(idx=0;idx<bitStringlen;idx++)
    {
        BS[idx] = ~ BS[idx];
    }
}

//两个位串进行与运算，结果放在第一个位串中
void bit_value_opt_and(INT8U *BS1,INT8U *BS2,INT16U bitStringlen)
{
    INT16U idx;

    for(idx=0;idx<bitStringlen;idx++)
    {
        BS1[idx] &= BS2[idx];
    }
}

//两个位串进行与运算，结果放在第一个位串中
void bit_value_opt_or(INT8U *BS1,INT8U *BS2,INT16U bitStringlen)
{
    INT16U idx;

    for(idx=0;idx<bitStringlen;idx++)
    {
        BS1[idx] |= BS2[idx];
    }
}
//将一个字节内位的顺序颠倒，max_len<=8
void bit_seq_reverse(INT8U *BS1,INT8U max_len)
{
    INT8U idx;
    INT8U BS2,mask;


    BS2=*BS1;
    *BS1=0;
    for(idx=0;idx<max_len;idx++)
    {
        mask=(1<<idx);
        if(BS2 & mask)
        {
            *BS1 |= 1<<(max_len-idx-1);
        }
    }
}
