#ifndef __HJ_212_H__
#define __HJ_212_H__

//#include <string.h>
//#include <stdio.h>
#include "main_include.h"
//#define INT8U   unsigned char
//#define INT16U  unsigned int
#include "os.h"
#define  TIME_GET_EQUIPMENT			 1011    //提取现场机时间
#define  TIME_CORRECT_SET			 1012    //中心端监控平台设置智能数据采集传输终端时间
#define  TIME_CORRECT_REQUEST		 1013	//智能数据采集传输终端时间校准请求
#define  TIME_INTERVAL_REAL_DATA	 1061    //实时数据间隔
#define  TIME_INTERVAL_REAL_DATA_SET 1062	 //设置实时数据间隔
#define  TIME_INTERVAL_MINU_DATA	 1063    //实时数据间隔
#define  TIME_INTERVAL_MINU_DATA_SET 1064	 //设置实时数据间隔
#define  TIME_PASSWD_SET		     1071    //设置现场机密码

#define  REPORT_REAL_DATE			 2011	//上传实时数据
#define  REPORT_RUN_STAT			 2021    //上传设施运行状态数据
#define	 RESP_REQUEST				 9011	//请求应答
#define  RESP_NOTIFY				 9013    //通知应答
#define  RESP_DATA					 9014    //数据应答
struct
{	
	INT16U ST;
	INT16U CN;
	INT8U passwd[6];
	INT8U MN[24];	
}gSystemHJ212;
INT8U HJ212_request_buf[1000];
INT8U HJ212_handle_state;
INT16U  make_data_seg(INT8U* buf, const INT16U ST, const INT16U CN, const INT8U *passwd, const  INT8U* MN, INT8U  Flag, INT16U PNUM, INT16U PNO, const INT8U* CP);
void makeHJ212Frame(INT8U *buf, const INT8U *data_seg, INT16U data_seg_len, INT16U *frameLen);
INT8U is_HJ212_frame_ready(const INT8U *frame,INT16U frame_len);
void app_protocol_handler_HJ212(objRequest* pRequest,objResponse *pResp);
/************************************************************************/
/* 现场机请求校时                                                        */
/************************************************************************/
INT8S HJ212_ertu_request_time();
INT16U report_212_monitor_xinhe();
INT16U 	report_212_poweroff_xinhe();
#endif