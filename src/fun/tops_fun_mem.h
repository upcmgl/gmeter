#ifndef __TOPS_FUN_MEM_H__
#define __TOPS_FUN_MEM_H__

INT8U get_bit_value(INT8U *BS,INT16U bitStringlen,INT16U bitpos);
void set_bit_value(INT8U *BS,INT16U bitStringlen,INT16U bitpos);
void  clr_bit_value(INT8U *BS,INT16U bitStringlen,INT16U bitpos);
void bit_value_opt_inversion(INT8U *BS,INT16U bitStringlen);
void bit_value_opt_and(INT8U *BS1,INT8U *BS2,INT16U bitStringlen);
void bit_value_opt_or(INT8U *BS1,INT8U *BS2,INT16U bitStringlen);
void   mem_set(void *buffer,INT16U len,INT8U value);
void    mem_cpy(void *dst,void *src,INT16U len);
void mem_cpy_right(void *dst,void *src,INT16U len);
INT8U  mem_all_is(void *dst,INT16U len,  INT8U c);
INT8U  mem_is_same(void *dst,void * src,INT16U len);
void bit_seq_reverse(INT8U *BS1,INT8U max_len);
void mem_cpy_reverse(INT8U *dst,INT8U *src,INT16U len);
#endif 
