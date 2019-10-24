#ifndef __TOPS_FUN_STR_H__
#define __TOPS_FUN_STR_H__


INT16S  compare_string(INT8U *str1,INT8U *str2,INT16U len);
INT16S  compare_string_reverse(INT8U *str1,INT8U *str2,INT16U len);
void mem_cpy_reverse(INT8U *dst,INT8U *src,INT16U len);
void buffer_reverse(INT8U *dst,INT16U len);
void trimspace(INT8U *str,INT16U size);
INT16S  str_find_again(INT8U *buffer,INT16U buflen,INT8U *target,INT16U targetlen,INT8U againcount);
INT16S  str_find_reverse_non(INT8U *buffer,INT16U buflen,INT8S c);
INT16U str_fill(INT8U *buffer,INT16U buflen,INT8S fill_c,INT16U width_len);
void str_nprintf(char *buffer,INT16U size,const char * format,...);
void str_printf(char *buffer,const char * format,...);
INT8S *str_cpy(INT8S *s1, INT8S *s2);
//#define sprintf(stream,fmt,a...) sprintf(stream,fmt,##a)
//void str_printf(char *buffer,const char * format,...);
#endif

 