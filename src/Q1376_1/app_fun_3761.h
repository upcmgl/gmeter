#ifndef __APP_FUN_3761_H__
#define __APP_FUN_3761_H__
#include "../main_include.h"
#include "app_3761.h"
#include "plms_read_c2.h"

typedef union{
     INT8U     bcd_datetime[6];
     struct{
       INT8U       second;
       INT8U       minute;
       INT8U       hour;
       INT8U       day;
       INT8U       week_month;
       INT8U       year;
     };
}DATAFMT_01;

void set_DATAFMT_01(DATAFMT_01 *data);
//void set_DATAFMT_15(DATAFMT_15 *data);
void double_to_fmt02(double dbVal,INT8U bcd2[2]);
void  ul2fmt02(INT32U ul, INT8U e,INT8U bcd2[2]);
void  int32s2fmt02(INT32S int32s, INT8U e,INT8U bcd2[2]);
BOOLEAN fmt02_to_ul(INT8U BCD2[2],INT32U *ul);
BOOLEAN fmt02_to_int32S(INT8U BCD2[2],INT32S *int32s);
BOOLEAN fmt02_to_int32u(INT8U BCD2[2],INT32U *int32u);
BOOLEAN fmt03_to_ul(INT8U BCD2[4],INT32S *ul);
BOOLEAN fmt03_to_int64s(INT8U BCD2[4],INT64S *int64s);
BOOLEAN ul2fmt03(INT32S ul,INT8U bcd2[4]);
BOOLEAN  fmt02_to_double(INT8U BCD2[2],double *dbVal);
void  fromFMT06toFMT09(INT8U *fmt06,INT8U *fmt09);
void  fromFMT11toFMT14(INT8U *fmt11,INT8U *fmt14);
float  fmt07_to_float(INT8U *fmt07);
INT16S  fmt05_to_int(INT8U fmt05[2]);
INT16S  int_to_fmt05(INT8U fmt05[2],INT16S value);
BOOLEAN  fmt09_to_double(INT8U *fmt09,double *db_val);
INT32S fmt09_to_INT32S(INT8U *fmt09);
//void assign_td_value(CommandDate *cmdDate,INT8U *td,INT8U count);
void v_97_to_format07(INT8U voltage[2]);
void i_97_to_format25(INT8U i[3]);
void pywg_97_to_format09(INT8U pwg[3]);
INT16U toDA(INT16U iVal);
INT16U DTtoInt(INT16U DT);
void check_A1_week(INT8U *bcd);
//void get_port_baudrate_parity(INT16U *BaudRate,INT8U *parity,PORT_COMM_CTRL port_ctrl);
void   set_event_datetime(INT8U *event);
INT8U crc16_param(INT8U *data,INT16U datalen);
void zfpgj2zjfpg(INT8U *data,INT8U datalen,INT8U *fl_count);
void convert_phy_data_zfpgj2zjfpg(INT8U *data,INT8U datalen);
void convert_phy_data_zfg2zjfpg(INT8U *data,INT8U datalen);
INT16U  get_fn_len(objRequest *pRequest,tagFN_ITEM *pFn);

#endif 