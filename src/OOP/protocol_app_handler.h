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


void  app_init_response(objRequest *pRequest,objResponse *pResp);
BOOLEAN check_ertu_address(objRequest *pRequest,objResponse *pResp);
BOOLEAN check_timetag(objRequest *pRequest);
BOOLEAN check_frame_invalid(objRequest *pRequest);
void send_error_response(objRequest  *pRequest,objResponse *pResp,INT8U dar);

void app_send_ReplyFrame(objResponse *pResp);
void app_send_Response(objResponse *pResp,BOOLEAN insert_flag);
void app_setFrameLenThenSendFrame(objRequest  *pRequest,objResponse *pResp);
void app_setFrameLenThenSendFrame_exec(objRequest  *pRequest,objResponse *pResp);
BOOLEAN app_send_response_frame(objRequest  *pRequest,objResponse *pResp,INT8U last_frame);
void app_run_init(void);


#endif       /* __PROTOCOL_APP_HANDLER_Q376_1_H__ */

