#ifndef __PLMS_READ_DATA_C1_H__
#define __PLMS_READ_DATA_C1_H__


//上N日字节位置
#define  MASK_DAYHOLD_D0    0x00
//#define  MASK_HOLD_D0    0x01
//#define  MASK_HOLD_D0    0x10
//#define  MASK_HOLD_D0    0x11
#define  MASK_CURVE         0x04




//Fn数据结构
typedef struct
{
    INT32U  item;         //数据项
    INT8U   offset;       //偏移量
    INT8U   len;          //数据长度 (上行协议长度1376.1)
    INT8U   num;          //块数据项个数 （暂时不需要使用）
    INT8U   prop;         //上N日字节位置  D0-D1（0：D0字节 1 ：D1字节 2 ：D2字节 3：D4字节）D2: 1 曲线数据   D3-D7空闲
    INT16U  format;       //转换函数 （07到1376.1转换函数）D15（1：非BCD 0：默认BCD）
}tagFN_ITEM;
typedef union{
    INT8U value[73];
    struct{
        INT8U  read_num;//抄读次数
        INT8U  read_point[16];//抄读点数，暂定最大抄读16次，按照3天每15min一个点，最大抄读7次
        INT8U  start_time[5];
        INT8U  data_len[16];
        INT8U  data_midu; 
        INT16U work_flag;
        INT16U  offset[16];
    };
}READ_FHJU;


INT16S  app_read_his_item(INT32U item,INT8U* request,INT8U* resp,INT16U resp_left_len,INT8U* item_tmp,INT16S left_time_ms);
INT16U  app_get_fn_data_c2(objRequest *pRequest,objResponse *pResp);
INT16U   convert_645_to_376(INT16U format,INT8U *pResp,INT8U *param);
void app_prcess_inter_data(objRequest *pRequest,objResponse *pResp,tagFN_ITEM *pFn);
INT32U  app_get_item_with_td(INT32U item,INT8U item_prop,INT8U *td,INT8U hold_flag);
INT8U trans_C2_curve_midu(READ_CONTEXT *readC2);
INT8U get_read_param(READ_FHJU* read_fhju,INT8U midu,INT8U point,INT16U total_len,INT8U point_len);
#endif



