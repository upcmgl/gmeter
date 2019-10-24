#ifndef __GB_OOP_H__
#define __GB_OOP_H__
#include "readmeter_oop_data.h"
#include "../tpos/inc/tpos_typedef.h"

typedef union {
	INT8U value[20];
	struct{
		INT8U save_idx[4];
		INT8U tsk_start_time[8]; /* 类型+ 内容 datetime_s格式 ? 1C+ */
		INT8U save_time[8];      /* 类型+ 内容 datetime_s格式 ? 1C+ */
		INT8U success_time[8];   /* 类型+ 内容 datetime_s格式 ? 1C+ */
	};
}DATA_HEADER;
#define DATA_HEADER_SIZE   sizeof(DATA_HEADER) 
INT8U make_oop_cur_frame(INT8U *frame,INT8U *meter_no,INT8U oad_count,INT8U* oad_data);
INT16U make_oop_hold_frame(INT8U *frame,INT8U *rtu_no,INT8U *meter_no,OBJECT_ATTR_DESC1* oad,INT8U oad_count,INT8U* oad_data,BOOLEAN add_hold_time_oad,INT8U* hold_time,BOOLEAN is_oop_cjq);
INT8U make_oop_hold_begin_end_ti_frame(INT8U *frame,INT8U *meter_no,OBJECT_ATTR_DESC1* oad,OBJECT_ATTR_DESC1* query_oad,INT8U oad_count,INT8U* oad_data,BOOLEAN add_hold_time_oad,INT8U* hold_time,INT8U* last_time,INT8U* ti);
#endif //__GB_OOP_H__

