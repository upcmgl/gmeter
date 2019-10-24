#ifndef __READMETER_OOP_HOLD_DATA_H__
#define	__READMETER_OOP_HOLD_DATA_H__

//#include "../main_include.h"
#include "readmeter_oop_data.h"
#define HOLD_LIST_COUNT        10
#define HOLD_MINUTE            0
#define HOLD_HOUR              1
#define HOLD_DAY               2
#define HOLD_ACCOUNT_DAY       3 /* 结算日 */
#define HOLD_MONTH             4 /* 月 */
#define HOLD_YEAR              5 /* 年 */
#define HOLD_TIME_ZONE         6
#define HOLD_DAY_SEG           7
#define HOLD_FL_PRICE          8
#define HOLD_LADDER            9


typedef union{
    INT8U value[2];
    struct{
        INT8U hold_save_idx[HOLD_LIST_COUNT][4];     //各种冻结的上一次位置
    }elmt;
}HOLD_IDX;

extern const READ_LIST_PROP hold_data_list[6];
#ifdef __PROVINCE_JIBEI_FUHEGANZHI__
extern const tagREADITEM  ELE_EQUIPMENT_HOLD_DATA_READ_ITEM_LIST[6];
void load_sensor_data_read_store();
#endif
//void read_meter_fixed_time_interval_hold_data(READ_PARAMS *read_param,INT8U meter_list_idx,BOOLEAN is_patch,INT8U hold_list_idx);
void read_meter_not_fixed_time_interval_hold_data(INT8U meter_list_idx,BOOLEAN is_patch,INT8U hold_list_idx);
INT8U find_hold_data_list(INT16U obis);
//INT16U read_hold_data(INT8U meter_list_idx,INT8U hold_list_idx,INT8U* resp,INT16U max_resp_len,INT16U hold_obis,INT16U rsd_obis,RSD* rsd,INT8U* rcsd);

BOOLEAN oad_search_in_hold_list(INT8U list_idx,INT8U oad[4],tagREADITEM *read_item);
#ifdef __PROVINCE_JIBEI_FUHEGANZHI__
BOOLEAN oad_search_in_hold_list_2(INT8U oad[4],tagREADITEM *read_item);
#endif
#endif

