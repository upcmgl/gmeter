#ifndef __READMETER_OOP_CUR_DATA_H__
#define	__READMETER_OOP_CUR_DATA_H__
#include "readmeter_oop_data.h"

#define CUR_LIST_COUNT        4
#define CUR_DNL               0
#define CUR_XL                1
#define CUR_VARIABLE          2
#define CUR_PARAMS            3

void read_meter_cur_data(INT16U obis,READ_PARAMS *read_param);
//INT16U read_cur_data(INT8U meter_list_idx,OBJECT_ATTR_DESC* oad,INT8U* resp,INT16U max_resp_len,INT8U cur_data_list_idx);
BOOLEAN read_report_mode_word(INT8U meter_list_idx,INT8U* buffer);
BOOLEAN oad_search_in_cur_list(INT8U list_idx,INT8U oad[4],tagREADITEM *read_item);
#endif
