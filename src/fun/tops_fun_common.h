#ifndef __TOPS_FUN_COMMON_H__
#define __TOPS_FUN_COMMON_H__

INT16S  str_find(INT8U *buffer,INT16U buflen,INT8U *target,INT16U targetlen);
INT16U  str_cmdlen(INT8U *cmd);
BOOLEAN isvalid_meter_addr(INT8U meter_no[6],BOOLEAN is_meter);
void  swap_meter_addr(INT8U meter_no[6]);
void meteraddr2str(INT8U meter_no[6],INT8U *tmp_str);
BOOLEAN check_is_all_FF(INT8U *data,INT16U len);
BOOLEAN check_is_all_ch(INT8U *data,INT16U len,INT8U ch);
void sortdata(INT16U *array,INT16U val,INT8U len);
INT16U pppfcs16(INT16U fcs, INT8U *cp, INT16S len);
void fcs16(INT8U *frame, INT16S len);
INT16U fcs16_16u(INT8U *frame, INT16S len);
INT32U simple_random(void);
void app_softReset(void);
BOOLEAN check_is_valid_ipaddr(uint8_t *ip);
BOOLEAN check_is_valid_port(uint8_t *port);
void previous_dayhold_td(INT8U td[3]);
void previous_monthhold_td_BCD(INT8U td[2]);
void previous_dayhold_td(INT8U td[3]);
#endif
