#ifndef __PROTOCOL_APP_CONST_DEF_H__
#define __PROTOCOL_APP_CONST_DEF_H__

#define APDU_LINK_REQUEST              0x01      //预连接请求      	    	[1]	 LINK-Request
#define APDU_LINK_RESPONSE             0x81      //预连接响应       	    [129]LINK-Response

#define APDU_SECURITY_REQUEST          0x10      //安全认证请求        	[4]	SecurityRequest
#define APDU_SECURITY_RESPONSE         0x90      //安全认证响应     	    [144]	SecurityResponse

#define CLIENT_APDU_CONNECT           0x02      //建立应用连接请求    	[2]	CONNECT-Request
#define CLIENT_APDU_RELEASE           0x03      // 断开应用连接请求    	[3]	RELEASE-Request
#define CLIENT_APDU_GET               0x05      //读取请求            	[5]	GET-Request
#define CLIENT_APDU_SET               0x06      //设置请求            	[6]	SET-Request
#define CLIENT_APDU_ACTION            0x07      //操作请求            	[7]	ACTION-Request
#define CLIENT_APDU_REPORT            0x08     //上报应答            	[8]	REPORT-Response
#define CLIENT_APDU_PROXY             0x09     //代理请求            	[9]	PROXY-Request


#define SERVER_APDU_CONNECT           0x82      //建立应用连接请求    	[130]	CONNECT-Response
#define SERVER_APDU_RELEASE_RESP      0x83      //断开应用连接响应 	    [131]	RELEASE-Response
#define SERVER_APDU_RELEASE_NOTI      0x84      //断开应用连接通知  	[132]	RELEASE-Notification
#define SERVER_APDU_GET               0x85      //读取响应         	    [133]	GET-Response
#define SERVER_APDU_SET               0x86      //设置响应       	    [134]	SET-Response
#define SERVER_APDU_ACTION            0x87      //操作响应        	    [135]	ACTION-Response
#define SERVER_APDU_REPORT            0x88      //上报通知        	    [136]	REPORT-Notification
#define SERVER_APDU_PROXY             0x89     //代理响应          	    [137]	PROXY-Response
#define SERVER_APDU_ERROR             0xEE     //错误响应          	    [238]	ERROR-Response

#define GET_REQUEST_NORMAL                0x01      //1)	读取一个对象属性；
#define GET_REQUEST_NORMAL_LIST           0x02      //2)	读取多个对象属性；
#define GET_REQUEST_RECORD                0x03      //3)	读取一个记录型对象属性；
#define GET_REQUEST_RECORD_LIST           0x04      //4)	读取多个记录型对象属性；
#define GET_REQUEST_NEXT                  0x05      //5)	读取分帧响应的下一个数据块，仅在被请求的数据响应不能在一个GET.response APDU中传输时才使用
#define GET_FLASH_CONTENT                 0x06      //6）   读取flash内容。

#define GET_RESPONSE_NORMAL                0x01      //1)	读取一个对象属性；
#define GET_RESPONSE_NORMAL_LIST           0x02      //2)	读取多个对象属性；
#define GET_RESPONSE_RECORD                0x03      //3)	读取一个记录型对象属性；
#define GET_RESPONSE_RECORD_LIST           0x04      //4)	读取多个记录型对象属性；
#define GET_RESPONSE_NEXT                  0x05      //5)	分帧响应一个数据块

#define  SET_REQUEST_NORMAL                 0x01     //设置一个对象属性请求
#define  SET_REQUEST_NORMA_LLIST            0x02     //设置多个对象属性请求
#define  SET_THEN_GET_REQUEST_NORMAL_LIST   0x03     //设置后读取多个对象属性请求

#define ACTION_REQUEST                       0x01     //操作一个对象方法请求
#define ACTION_REQUEST_LIST                  0x02     //操作多个对象方法请求
#define ACTION_THEN_GET_REQUEST_NORMAL_LIST  0x03     //操作多个对象方法后读取多个对象属性请求

#define PROXY_GET_REQUEST_LIST               0x01   //代理读取多个服务器的多个对象属性请求
#define PROXY_GET_REQUEST_RECORD             0x02	//代理读取一个服务器的一个记录型对象属性请求
#define PROXY_SET_REQUEST_LIST               0x03   //代理设置多个服务器的多个对象属性请求
#define PROXY_SET_THEN_GET_REQUEST_LIST      0x04   //代理设置后读取多个服务器的多个对象属性请求
#define PROXY_ACTION_REQUEST_LIST            0x05   //代理操作多个服务器的多个对象方法请求
#define PROXY_ACTION_THEN_GET_REQUEST_LIST   0x06   //代理操作后读取多个服务器的多个对象方法和属性请求
#define PROXY_TRANS_COMMAND_REQUEST          0x07   //代理透明转发命令请求

#define REPORT_NORMAL_LIST                   0x01   //上报若干个对象属性
#define REPORT_RECORD_LIST                   0x02   //上报若干个记录型对象属性
#define REPORT_NOT_TRAN_DATA                 0x03   //上报透明数据

#define APDU_LINK_MNG_REQ               0x03      //链路管理
#define APDU_LINK_MNG_RESP              0x83      //链路管理
#define APDU_REQ_OBJECT_ATTRIBUTE_REQ     0x0B      //请求对象属性
#define APDU_REQ_OBJECT_ATTRIBUTE_RESP    0x8B      //请求对象属性
#define APDU_REQ_BATCH_OBJECT_ATTRIBUTE_REQ     0x0C      //批量请求对象属性
#define APDU_REQ_BATCH_OBJECT_ATTRIBUTE_RESP    0x8C      //批量请求对象属性
#define APDU_REQ_RECORD_OBJECT_ATTRIBUTE_REQ     0x0D      //请求记录型对象属性
#define APDU_REQ_RECORD_OBJECT_ATTRIBUTE_RESP    0x8D      //请求记录型对象属性
#define APDU_REQ_BATCH_RECORD_OBJECT_ATTRIBUTE_REQ    0x11      //批量请求记录型对象属性
#define APDU_REQ_COMPLEX_RECORD_OBJECT_ATTRIBUTE_REQ    0x12      //请求复合记录型对象属性
#define APDU_REQ_COMPLEX_RECORD_OBJECT_ATTRIBUTE_RESP   0x92      //请求复合记录型对象属性
#define APDU_REQ_SEPARATE_FRAME                  0x10      //请求分帧后续帧
#define APDU_REQ_DATA_REPORT1                   0x04      //数据上报-服务器通知批量记录
#define APDU_REQ_DATA_REPORT2                   0x05      //数据上报-服务器通知复合记录

#define APDU_EXEC_AND_REQ_RECORD_OBJECT_ATTRIBUTE_REQ     0x2E      //执行方法后读取记录型对象
#define APDU_EXEC_AND_REQ_RECORD_OBJECT_ATTRIBUTE_RESP    0xAE      //执行方法后读取记录型对象


#define APDU_EXEC_OBJECT_METHOD_REQ     0x29      //执行对象方法
#define APDU_EXEC_OBJECT_METHOD_RESP    0xA9      //执行对象方法
#define APDU_BATCH_EXEC_OBJECT_METHOD   0x2A      //批量执行对象方法
#define APDU_EXEC_READ_OBJECT_METHOD    0x2B      //执行后读取

#define APDU_SET_OBJECT_ATTRIBUTE               0x1F       //设置对象属性
#define APDU_SET_OBJECT_ATTRIBUTE_RESP          0x9F       //设置对象属性
#define APDU_BATCH_SET_OBJECT_ATTRIBUTE         0x20       //批量设置对象属性
#define APDU_BATCH_SET_OBJECT_ATTRIBUTE_RESP    0xA0       //批量设置对象属性
#define APDU_SETGET_OBJECT_ATTRIBUTE               0x21    //设置后读取对象属性
#define APDU_SETGET_OBJECT_ATTRIBUTE_RESP          0xA1    //设置后读取对象属性

#define APDU_AGENT_SET_OBJECT_ATTRIBUTE          0x22    //代理设置对象属性
#define APDU_AGENT_SETGET_OBJECT_ATTRIBUTE       0x23    //代理设置后读取对象属性
#define APDU_AGENT_GET_OBJECT_ATTRIBUTE          0x0E   //代理请求对象属性
#define APDU_AGENT_GET_RECORD_OBJECT_ATTRIBUTE   0x0F   //代理请求记录型对象属性
#define APDU_AGENT_EXEC_OBJECT_METHOD            0x2C   //代理执行对象方法
#define APDU_AGENT_EXEC_READ_OBJECT_METHOD       0x2E   //代理执行后读取
#define APDU_EVENT_NOTIFY1                        0x04   //事件及数据通知
#define APDU_EVENT_NOTIFY2                        0x05   //事件及数据通知


#define CTRLFUNC_DIR_SET        0x80       //设置传输方向位DIR=1    使用|
#define CTRLFUNC_DIR_CLR        0x7F       //设置传输方向位DIR=0    使用&
#define CTRLFUNC_PRM_SET        0x40       //设置启动标志位PRM=1    使用|
#define CTRLFUNC_PRM_CLR        0xBF       //设置启动标志位PRM=0    使用&
#define CTRLFUNC_SC_SET         0x08       //设置扰码标志位SC=1     使用|
#define CTRLFUNC_SC_CLR         0xF7       //设置扰码标志位SC=0     使用&
#define CTRLFUNC_FC_LINK_MNG    0x01       //设置功能码        使用|
#define CTRLFUNC_FC_REQ_RESP    0x03       //设置功能码        使用|
#define CTRLFUNC_FC_REPORT      0x04       //设置功能码        使用|
#define CTRL_FUNC_USERDATA      0x03        //用户数据

//#define ATTRIBUTE_ALL           0xFF
////#define MAX_EVENT_COUNT         0x10       //目前支持最大事件数量
//#define SFF_FIRST_FRAME         0x01       //分帧标志，首帧标志
//#define SFF_LAST_FRAME          0x02       //分帧标志，末帧标志
//#define SFF_BLOCK_KIND          0x04       //分帧标志,按块分帧
//
//#define SERVER_ADDR_TYPE_FIXED  0xA0       //链路层服务器地址固定长度
//#define SERVER_ADDR_TYPE_VAR    0xB0       //链路层服务器地址可变长度

//数据错在结果DAR
#define DAR_SECCESS                         0       //成功
#define DAR_HARDWARE_FAULT                  1       //硬件失效
#define DAR_TEMPORARY_FAILURE               2       //暂时失效
#define DAR_READ_WRITE_DENIED               3       //拒绝读写
#define DAR_OBJECT_UNDEFINED                4       //对象未定义
#define DAR_OBJECT_CLASS_INCONSISTENT       5       //对象接口类不符合
#define DAR_OBJECT_UNAVAILABLE              6       //对象不存在
#define DAR_TYPE_UNMATCHED                  7       //类型不匹配
#define DAR_SCOPE_OF_ACCESS_VIOLATED        8       //越界
#define DAR_DATA_BLOCK_UNAVAILABLE          9       //数据块不可用
#define DAR_LONG_GET_ABORTED                10      //分帧传输已取消
#define DAR_NO_LONG_GET_IN_PROGRESS         11      //不处于分帧传输状态
#define DAR_LONG_WRITE_ABORTED              12      //块写取消
#define DAR_NO_LONG_WRITE_IN_PROGRESS       13      //不存在块写状态
#define DAR_DATA_BLOCK_NUMBER_INVALID       14      //数据块序号无效
#define DAR_UNAUTHORIZED_ACCESS             15      //密码错误、未授权
#define DAR_COMM_RATE_NO_CHANGE             16      //通信速率不能更改
#define DAR_YEAR_TIME_ZONE_COUT_OVER        17      //年时区数超
#define DAR_DAY_TIME_INTERVAL_COUNT_VOER    18      //日时段数超
#define DAR_RATE_EXCEEDED                   19      //费率数超
#define DAR_SECURITY_AUTH_UNMATCHED         20      //安全认证不匹配
#define DAR_DUPLICATE_RECHARGE              21      //重复充值
#define DAR_ESAM_VERIFY_FAIL                22      //ESAM验证失败
#define DAR_SECURITY_AUTH_FAIL              23      //安全认证失败
#define DAR_CUST_NO_UNMATCHED               24      //客户编号不匹配
#define DAR_RECHARGE_COUNT_ERROR            25      //充值次数错误
#define DAR_BUY_FEE_OVER                    26      //购电超囤积
#define DAR_ADDRESS_EXCEPT                  27      //地址异常
#define DAR_SYMMETRIC_DECRYPT_ERROR         28      //对称解密错误
#define DAR_ASYMMETRIC_DECRYPT_ERROR        29     //非对称解密错误
#define DAR_SIGNATURE_ERROR                 30      //签名错误
#define DAR_METER_HANGING                   31      //电能表挂起
#define DAR_TIME_TAG_INVALID                32      //时间标签无效
#define DAR_TIME_OUT                        33      //请求超时
#define DAR_ESAM_P1P2_ERROR                 34      //ESAM的P1P2不正确
#define DAR_ESAM_LC_ERROR                   35      //ESAM的LC错误
#define DAR_OTHER_REASON                    255     //其它

#define SYS_POWER_LOST             0x00000001   //电源掉电标志
#define SYS_GPRS_CHG               0x00000002   //GPRS参数重新设置
#define SYS_RST_GPRS               0x00000004   //GPRS复位标志
#define SYS_PRG_RESET              0x00000008   //系统软件复位标志
#define SYS_PLC_REC_CJQ            0x00000010   //载波抄表时是否屏蔽采集器号码    0:按照采集器方式抄表  1：屏蔽档案中的采集器地址，按照表号抄表
#define SYS_ESAM                   0x00000020   //是否使用ESAM的mac验证   0:不使用;1:使用
#define SYS_CY_METER               0x00000040   //是否有交采硬件   1:有;0:没有
#define SYS_READ_485_CY            0x00000040   //是否有交采硬件   1:有;0:没有
#define SYS_PROG_KEY               0x00000080   //是否按了编程键
#define SYS_CY_RESET               0x00000100   //交采发生了复位   1：发生 0：未发生
#define SYS_CY_CLR_DATA            0x00000200   //交采发生了清零事件   1：发生 0：未发生
#define SYS_CY_CLOCK_LOST          0x00000400   //交采时钟丢失  1:发生  0:未发生
#define SYS_KEY_WAKEUP             0x00000800  //按键唤醒状态  1-在按键唤醒状态，0-不在按键唤醒状态
#define SYS_CY_PAHSE_LINE          0x00001000   //相线标志，0-三相四 1-三相三
#define SYS_FRAME_ACD_FLAG         0x00002000   //报文中ACD标志允许标志
#define SYS_CLOCK_LOST	  	       0x00004000   		//时钟失效   1：发生 0：未发生
#define SYS_CLOCK_DOUBT	  	       0x00008000   		//时钟质疑   1：发生 0：未发生
#define SYS_RUN_STATUS             0x00010000   //集中器投运状态 1:投运 0:未投  重庆扩展 __PROVICE_CHONGQING__
#define SYS_ESAM_READMETER         0x00020000   //ESAM抄表

//
//
//#define PRJ_NORMAL_TASK                         1   //普通采集方案
//#define PRJ_EVENT_TASK                          2   //事件采集方案
//#define PRJ_RELAY_TASK                          3   //透明方案
//#define PRJ_REPORT_TASK                         4   //上报方案
//#define PRJ_SCRIPT_TASK                         5   //脚本方案

typedef struct{
    INT8U *request_frame;
    INT8U *resp_frame;
    INT8U *security_info;
    INT8U *param;
    INT16U request_pos;
    INT16U request_data_pos;
    INT16U resp_pos;
    INT16U left_len;
    INT16U separate_flalg_pos;
    INT16U addition_pos;
    INT16U userDataLen;
    INT16U frame_len;
    INT8U  query_record_count;//默认查询记录数，0为全部，否则为指定记录数，目前用于事件上报，每次只报一条事件 
}OBJECT_REQ_CONTEXT;
#endif

