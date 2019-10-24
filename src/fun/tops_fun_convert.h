#ifndef __TOPS_FUN_CONVERT_H__
#define __TOPS_FUN_CONVERT_H__

void int16u2_bin(INT16U val,INT8U bin[2]);
void int32u2_bin(INT32U val,INT8U bin[4]);
void int32u2_bin_reserve(INT32U val,INT8U bin[4]);//反着转换，比如01010605，按照05060101转成32U
INT16U  bin2_int16u(INT8U bin[2]);
INT32U  bin2_int32u(INT8U bin[4]);
INT8U byte2BCD(INT8U bVal);
INT8U BCD2byte(INT8U bcd);
void sortdata(INT16U *array,INT16U val,INT8U len);
BOOLEAN is_valid_bcd_ext(INT8U *bcd,INT8U bytes);
BOOLEAN is_valid_bcd(INT8U *bcd,INT8U bytes);
INT32U  bcd2u32(INT8U *bcdnum,INT8U bytes,BOOLEAN *valid_bcd);
INT64U  bcd2u64(INT8U *bcdnum,INT8U bytes,BOOLEAN *valid_bcd);
void ul2bcd(INT32U ul,INT8U *bcdnum,INT8U bytes);
void u64_2fmt03(INT64U ul,INT8U *bcdnum);
void   bcd2Double(INT8U *bcdnum, INT8U bytes,int multi,double *dbl);
INT32U  hex2uint(INT8S *hex);
void  bin2hex(INT8U *hex,INT8U ch);
INT32U  str2int(INT8S *str);
void to_bcd(INT8U *data,INT8U len);
void to_byte(INT8U *data,INT8U len);
INT16U   str2uint(INT8S *str);
void     str2ip(INT8S *str_server_ip, INT8U server_ip[4]);
INT16U  Hex2Binary(INT8U * hex,INT16U len, INT8U * buffer);
INT16U Str2UintBuff(INT8S *str,INT16U len,INT8U *buf);
void  BCDNumberShift(INT8U *bcd, INT8U len,int shift);
INT16U bcd2_int16u_def(INT8U *bcdnum,INT16U default_val);
void buffer_bin_to_bcd(INT8U *bin,INT8U *bcd,INT8U len);
void buffer_bcd_to_bin(INT8U *bcd,INT8U *bin,INT8U len);
BOOLEAN ascii_to_bcd(INT8U *ascii,INT8U *bcd,INT8U len);
INT8U cosem_int162_bin(INT16U val,INT8U bin[2]);
INT16U cosem_bin2_int16u(INT8U bin[2]);
INT8U cosem_int162_bcd(INT16U val,INT8U bin[2]);
INT16U cosem_bcd2_int16u(INT8U bin[2]);
INT8U cosem_int32u2_bin(INT32U val,INT8U bin[4]);
INT32U  cosem_bin2_int32u(INT8U bin[4]);
INT32U  cosem_bin2_unit(INT8U *bin,INT8U size);
void  cosem_bin2_int64u(INT8U bin[8],INT64U *val);
INT8U cosem_int64u2_bin(INT64U val,INT8U bin[8]);
void cosem_bin2_int64s(INT8U bin[8],INT64S *val);
INT8U get_datetime_from_cosem(INT8U *cosem_bin,INT8U *datetime_bin);
INT16U  get_cosem_data_len(INT8U *data,INT8U *len_size);
INT16U set_cosem_data_len(INT8U *data,INT16U len);

void  ip2str(INT8U  ip[4],INT8S *str_server_ip);
#endif
 