#ifndef __TASK_METER_H__
#define __TASK_METER_H__

#define POS_GB645_FLAG1       0
#define POS_GB645_METERNO     1
#define POS_GB645_FLAG2       7
#define POS_GB645_CTRL        8
#define POS_GB645_DLEN        9
#define POS_GB645_ITEM       10
#define POS_GB645_97_DATA    12
#define POS_GB645_07_DATA    14

#define OOP_POS_BEGIN     0       //起始符
#define OOP_POS_LEN       1       //长度
#define OOP_POS_CTRL      3       //控制码
#define OOP_POS_ADDR      4       //地址域

typedef struct{
    INT32U item_times;
    INT8U  check_offset1;
    INT8U  check_len1;
    INT32U item_end;
    INT8U  check_offset2;
    INT8U  check_len2;    
    INT8U  instance;
}tagResourceIdEvent;

INT16U app_trans_send_meter_frame(INT8U* frame,INT16U frame_len,INT8U* resp,INT16U max_resp_len,INT16U max_wait_time_10ms);
void push_to_meter_frame(uint8_t *frame,uint16_t frame_len);
INT16U  unwrap_gb645_frame(INT8U *gb645,INT16U frameLen);
INT8U make_gb645_2007_frame(INT8U *frame,INT8U *meter_no,INT8U ctrl,INT32U item,INT8U *data,INT8U datalen);
INT16U  decode_gb645_frame(INT8U *gb645,INT16U frameLen);
void task_meter_service(void);
#ifdef __EDP__
INT32U edp_ping_start_time;
INT32U edp_warningTime;
#endif
#endif