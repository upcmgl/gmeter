#ifndef __APP_PROTOCOL_ONENET_H__
#define	__APP_PROTOCOL_ONENET_H__
#include "../main_include.h"
#include "EdpKit.h"
#ifdef __cplusplus
extern "C" {
#endif
INT8U OneNetData[1024],oneNetstr[1024],oneNetstr2[1024];
INT8U edp_version[7];
void app_protocol_handler_edp(objRequest* Request,objResponse *Response);
void registerDevice();
INT8U  edp_ping();

BOOLEAN reportConfigToOnenet();
INT8U reportHoldDayEnergyToOnenet(INT8U flag);
INT8U reportyEnergyBlockToOnenet();
INT8U responseUpdataResToOnenet(INT8U flag);
void first_poweron_flash_rebuilt();
void handleUnsuccessReportDataRead( );
void handleUnsuccessReportDataWrite(INT8U *buf,INT16U len);
void powerOnRebuildWaningInfo();
void warning_report_handler(INT16U *warningMask);
INT8U report_warning_to_onenet(INT8U flag,INT8U value);
INT8U report645CmdResultToOnenet();
INT64U rand_wait_report_time_index[40];
void first_poweron_flash_wait_report_rebuilt();
void handle_wait_report_data_read();
void handle_wait_report_data_write(INT8U *buf,INT16U len);
INT16U getrand();  //得到随机数。
void second_connet_onenet();
void first_login_onenet();
void vmeter_edp_warning_report();
struct edp_updata_info   //升级信息块，升级前需要保存在flash中。
{
	INT8U updata_from[6]; //升级前版本号
	INT8U updata_from_hex[3];
	INT8U updata_to[6];    //升级后版本号
	INT8U updata_to_hex[3];
	INT8U updata_plat;    //希望升级的硬件平台  G55或PIC
	INT8U updata_moduleType; //升级的模块类型 ，单、三相
}edp_updata_INFO;
#ifdef __ZHEJIANG_ONENET_EDP__
void  app_check_meter_record_event(INT16U* warningMask);
INT8U reportMonthDayEnergyToOnenet();
#endif
#ifdef __cplusplus
}
#endif

#endif

