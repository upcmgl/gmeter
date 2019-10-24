/*
 * apdu_security.h
 *
 * Created: 2018/12/20 星期四 17:19:21
 *  Author: Administrator
 */ 


#ifndef APDU_SECURITY_H_
#define APDU_SECURITY_H_
#include "main_include.h"

BOOLEAN check_safe_config(INT16U obis,INT8U operate,INT8U *security_info);
INT16S req_object_attribute_esam(INT8U attri_id,INT8U index,INT8U *data,INT16U left_len);
void process_client_apdu_security(objRequest *pRequest,objResponse *pResp);
INT16U security_encode_apdu(objRequest *pRequest,objResponse *pResp);
INT16U encode_readmeter_frame(INT8U *frame,INT16U len);
INT16U decode_readmeter_frame(INT8U *frame,INT16U len);
#endif /* APDU_SECURITY_H_ */