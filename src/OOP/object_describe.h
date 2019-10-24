#ifndef __OOP_OBJECT_DESCRIBE_H__
#define	__OOP_OBJECT_DESCRIBE_H__
#include "protocol_app_const_def.h"

typedef struct{
  INT8U index;      //索引
  INT8U data_type;  //数据类型
  INT16U len;
}tagObject_Index;

typedef struct{
  INT8U Attri_Id;   //属性编号
  INT8U data_type;  //数据类型
  INT16U len;
  tagObject_Index *index_list;  //索引表
  INT8U index_list_size;
  INT8U right;
}tagObject_Attribute;

typedef struct{
  INT16U obis;                   //OBIS
  INT16U attri_max_len;         //属性最大长度
  INT32U offset;                //偏移地址
  tagObject_Attribute  *attri_list;            //属性表
  INT8U  attri_list_size;            //属性表大小
}tagParamObject;

#define READ_ONLY 0x01
#define WRITE_ONLY 0x02

//GPRS配置参数
#define OA_MASTER_IP             0 //主用IP地址
#define OA_MASTER_PORT           1 //主用端口

//终端ip配置参数
#define OA_IP_CONFIG_MODE        0 //终端IP
#define OA_DEV_IP                1 //终端IP
#define OA_SUBNET_MASK           2 //子网掩码
#define OA_GETEWAY               3 //网关地址
#define OA_PPPOE_USER            4 //用户名
#define OA_PPPOE_PWD             5 //密码


#define OA_WORK_MODE             0 //工作模式
#define OA_ONLINE_KIND           1 //在线方式
#define OA_CONNECT_KIND          2 //连接方式
#define OA_CONNECT_USE_MODE      3 //连接方式应用模式
#define OA_LISTEN_PORT_LIST      4//11 //侦听端口列表
#define OA_APN                   5 //APN
#define OA_USER                  6 //用户名
#define OA_PASSWORD              7 //密码
#define OA_PROXY_IP              8 //代理服务器地址
#define OA_PROXY_PORT            9 //代理端口
#define OA_TIMEOUT_RESEND_COUNT  10 //超时时间及重发次数
#define OA_HEART_CYCLE           11 //心跳周期

////////////////////////////////////////////////////////////////////
////短信通讯参数
#define OA_SMS_CENTER_NO            0   //短信中心号码
#define OA_MSA_SIM_NO               1   //主站号码
#define OA_DEST_NO                  2   //短信通知目的号码

//无线公网通信属性表
#define OA_GPRS_PARAM                       0   //150，通信配置
#define OA_GPRS_MASTER_PARAM                1   //60，主站通信参数表
#define OA_SMS_PARAM                        2   //200，短信通信参数
#define OA_GPRS_VER                         3   //50，版本信息
#define OA_ALLOW_PROTOCOL                   4   //20，支持规约列表
#define OA_GRPS_CCID                        5   //30，SIM卡的ICCID
#define OA_GRPS_IMSI                        6   //20，IMSI
#define OA_GPRS_CSQ                         7   //3，信号强度
#define OA_GPRS_SIM_ID                      8   //20，sim卡号码
#define OA_GPRS_IP                          9   //20，拨号ip

//电表地址属性表
#define OA_METER_ADDR                       0   //10，电表地址

//厂家调试属性表
#define OA_CFG_VENDO                       0    //厂商代码
#define OA_FRAME_TYPE                      1    //日志类型
#define CONST_MES						   16    //主板MES号，板级检测时设置，永久保留FFFD1800

//ESAM参数
#define OA_SAFE_MODE                       0
#define OA_READ_MODE                       6

BOOLEAN get_param_attribute(INT8U attri_id,const tagObject_Attribute *attr_list,INT8U attr_list_size,tagObject_Attribute *attri);
BOOLEAN get_param_object(INT16U obis,tagParamObject *object);
INT32U get_object_offset(INT16U obis);
INT16U get_attribute_offset(const tagObject_Attribute *attr_list,INT8U attr_list_size,INT8U attr_id);
INT16U get_index_offset(const tagObject_Index *index_list,INT8U index,INT8U count);
INT16U calculate_ms_size(INT8U *data,INT8U data_type);
INT16U calculate_ms_size_without_type(INT8U *data,INT8U data_type);
INT16U get_object_attribute_item_len(INT8U *data,INT8U data_type);
INT16U get_object_attribute_item_value(INT8U *data,INT8U data_type,BOOLEAN only_value,INT8U find_index,INT8U **retdata,INT8U *attr_head_size);
BOOLEAN check_attribute_valid(INT8U *data,INT16U len,INT8U object_type,const tagObject_Attribute *attr_list,INT8U attr_list_size);

INT8U get_data_type_len(INT8U data_type);
INT16U get_oop_data_type_len(INT8U *data,INT8U data_type);
INT16U get_attribute_inbuffer(INT8U index,INT8U *data_buffer,INT8U *attribute,INT16U max_len,BOOLEAN only_value,INT8U parent_index);

INT16U get_apdu_app_link_request_size(INT8U *data,INT16U max_len);
INT16U get_apdu_link_response_size(INT8U *data,INT16U max_len);
INT16U get_apdu_get_request_size(INT8U *data,INT16U max_len);
INT16U get_apdu_set_request_size(INT8U *data,INT16U max_len);
INT16U get_apdu_action_request_size(INT8U *data,INT16U max_len);
INT16U get_apdu_report_response_size(INT8U *data,INT16U max_len);
INT16U get_apdu_proxy_request_size(INT8U *data,INT16U max_len);
INT16U get_apdu_security_request_size(INT8U *data,INT16U max_len);
INT16U get_apdu_security_response_size(INT8U *data,INT16U max_len);

void default_esam_oi_list(INT16U obis,INT8U mask,INT16U safe_config);
INT32U get_system_flag(INT32U flag,INT8U type);
void   set_system_flag(INT32U flag,INT8U type);
void  clr_system_flag(INT32U flag,INT8U type);
void  clr_system_flag_type(INT8U type);
void  clr_system_flag_all();

void delete_esam_oi_list(INT8U oi[2]);
void update_esam_oi_list(INT8U data[4]);
INT16U get_esam_safe_config(INT16U obis);
void load_esam_oi_list(void);

#endif
 