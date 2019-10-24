#ifndef __PROTOCOL_APP_HANDLER_Q376_1_H__
#define __PROTOCOL_APP_HANDLER_Q376_1_H__
#include "../main_include.h"



//请求一类数据工作变量
typedef struct{
   INT16U     meter_seq;
   INT16U     meter_seq_485;
   BOOLEAN    flag485;
   BOOLEAN    need_do_default;
}VAR_REQ_C1;



INT16U dl645_readitem(INT16U meter_idx,INT8U *frame,INT32U item,INT8U protocol,INT8U *meter_no);
void app_encodeFrame(objResponse *pResp);
void app_send_ReplyFrame(objResponse *pResp);
//void app_send_ReplyFrame_urgent(objResponse *pResp);
void app_send_Response(objResponse *pResp,BOOLEAN insert_flag);
void app_send_ACK(objRequest  *pRequest,objResponse *pResp);
void app_send_NAK(objRequest  *pRequest,objResponse *pResp);
void app_setFrameLenThenSendFrame(objRequest  *pRequest,objResponse *pResp);
BOOLEAN app_send_response_frame(objRequest  *pRequest,objResponse *pResp,INT8U last_frame);

INT8U  get_fnprop_from_fnidx(INT16U fn_idx);
INT16U  flag485_idx(INT16U meter_seq);
void  set_pn_fn(INT8U *Reply,INT16U *pos,INT16U pn,INT16U fn);
INT16U  trans_set_pn_2_pnidx(INT16U pn);
INT16U trans_set_fn_2_fnidx(INT16U fn);
INT16U get_meter_idx_from_meter_addr(INT8U *addr);
INT16U parse_pn_to_pngroup(INT16U pn,INT16U pngroup[8]);
INT16U parse_pn_to_pngroup_2005(INT16U pn,INT16U pngroup[8]);
INT16U parse_fn_to_fngroup(INT16U fn,INT16U fngroup[8]);
INT8U  check_frame_pw(objRequest *pRequest);
//INT16U  trans_spot_idx_2_meter_idx(INT16U spot_idx);
BOOLEAN get_next_pn(INT16U pn,INT16U *pn_new,INT16U *pn_idx);
BOOLEAN meter_exist(INT16U spot_idx);

#endif       /* __PROTOCOL_APP_HANDLER_Q376_1_H__ */

