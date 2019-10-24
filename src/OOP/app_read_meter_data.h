/*
 * app_read_meter_data.h
 *
 * Created: 2018/12/20 星期四 17:21:47
 *  Author: Administrator
 */ 


#ifndef APP_READ_METER_DATA_H_
#define APP_READ_METER_DATA_H_

extern INT8U transIndex;
void init_separate_frame_info(INT8U apdu_service_code);
void process_get_meter_attribute(objRequest *pRequest,objResponse *pResp);
void process_get_meter_record(objRequest *pRequest,objResponse *pResp);
INT8U process_prepare_separate_frame_info(objRequest *pRequest,objResponse *pResp,SEPARATE_FRAME *separate_frame,INT16U frame_id);
//INT8U read_acq_plan_data_from_meter(objRequest *pRequest,objResponse *pResp,RSD *rsd,INT8U *rcsd,INT16U left_len,SEPARATE_FRAME *separate_frame);

INT16U make_meter_data_from_flash(INT32U *master_oad,INT32U *oad,INT8U oad_cnt,INT8U *hold_time,INT8U *buffer,INT8U *task_start_time_frq,INT8U *DayHoldFlag,INT8U *data_type,INT8U *is_cur_data_req);
void change_day_hold_serach_time(INT8U *time,INT8U flag);

#endif /* APP_READ_METER_DATA_H_ */