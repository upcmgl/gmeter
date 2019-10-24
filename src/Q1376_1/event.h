/*
 * event.h
 *
 * Created: 2019-4-19 20:32:18
 *  Author: mgl
 */ 

#include "main_include.h"
#ifndef EVENT_H_
#define EVENT_H_
#define INVALID_EC_SEQ                   0xFFFFFFFF

#define EVENT_POS_SEQ                    0      //4,事件序号
#define EVENT_POS_RPT_FLAG               4      //1,事件标志,D0-表示重要事件/一般事件,D1表示是否需要上报D2表示是否已经上报
#define EVENT_POS_EC                     5      //1,发生时的事件计数器
#define EVENT_POS_ERC_NO                 6      //1,事件代码
#define EVENT_POS_ERC_LEN                7      //1事件长度
#define EVENT_POS_ERC_DATA               8      //n事件内容
#define EVENT_POS_ERC_TIME               8      //5，事件发生时间
#define EVENT_POS_ERC_CONTENT            13      //N，事件内容

#define EVENT_MAX_SAVE_COUNT             15    //单个事件最大存储个数
#define EVENT_RECORD_SIZE               152

#define EVENT_HEAD_LEN                   6      //事件记录头长度
#define G_METER_ERC_RECORD_MAX        64        //ERC记录个数暂时定义为64个标准协议41个,
#define EVENT_ERC_SECTOR_MAX          70       //ERC事件记录sector个数的最大值

#define EVENT_TYPE_BASE         0x00    //普通事件
#define EVENT_TYPE_VIP          0x01    //重要事件
#define EVENT_TYPE_ALL          0x02    //普通及重要事件

typedef struct
{
	
}tagERCF1;
typedef struct
{
	
}tagERCF2;
typedef struct
{
	INT8U sector_index[G_METER_ERC_RECORD_MAX];             //记录每个erc事件所处的sector，范围是0到39
	INT8U erc_cnt[G_METER_ERC_RECORD_MAX];					//记录每个erc事件的个数
	INT8U sector_free_index[EVENT_ERC_SECTOR_MAX-G_METER_ERC_RECORD_MAX];        //记录了自由sector的序号，范围是0到39
}tagERCF3;
typedef struct 
{
	INT8U  ec1;                       //重要事件个数
	INT8U  ec2;                       //一般事件个数
	tagERCF1 ERC_F1;
	tagERCF2 ERC_F2;
	tagERCF3 ERC_F3;
}tagEVETN;
typedef struct
{
	INT8U erc_no;
	INT8U erc_cnt;
}tagERC_96bits_region;


INT8S reconstructe_eventERC_index();

void save_event_record(INT8U *event,INT8U event_flag);
void  event_params_set(INT8U *event);             //ERC3

#endif /* EVENT_H_ */