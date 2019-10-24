//#include "app.h"
#include "app_var_3761.h"

tagGlobalEVENT g_event;
//电表档案
METER_DOCUMENT  gmeter_doc;
INT32S   read_c1_time_out;
tagActiveSendAck gActiveSendAck;//主动上报数据的确认机制
INT8U meter_status[14];//电表运行状态字



