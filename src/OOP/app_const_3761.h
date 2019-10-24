#ifndef __APP_CONSTDEF_H__
#define __APP_CONSTDEF_H__

#define FUNC_MASK_CLIENT    0x80       //设置传送方向
#define FUNC_MASK_DENY      0x40       //设置否定标志

#define CTRLFUNC_SET_DIR        0x80       //设置传输方向位DIR=1    使用|
#define CTRLFUNC_CLR_DIR        0x7F       //设置传输方向位DIR=0    使用&
#define CTRLFUNC_CLR_PRM        0xBF       //设置启动标志位PRM=0    使用&
#define CTRLFUNC_SET_PRM        0x40       //设置启动标志位PRM=1    使用|
#define CTRLFUNC_CLR_ACD        0xDF       //设置帧要求访问位ACD=0  使用&
#define CTRLFUNC_SET_ACD        0x20       //设置帧要求访问位ACD=1  使用|
#define CTRLFUNC_CLR_FCB        0xDF       //设置帧计数位FCB=0      使用&
#define CTRLFUNC_SET_FCB        0x20       //设置帧计数位FCB=1      使用|
#define CTRLFUNC_CLR_FCV        0xEF       //设置帧计数有效位FCV=0      使用&
#define CTRLFUNC_SET_FCV        0x10       //设置帧计数有效位FCV=1      使用|

//上报补抄日冻结每帧数据个数
#define ENCRYPT_NONE  0
#define ENCRYPT_TOPS1  1
#define ENCRYPT_TOPS2  2

#define CHANNEL_REMOTE  0x01
#define CHANNEL_RS232   0x02
//#define CHANNEL_INFRA   0x04
#define CHANNEL_FLASH   0x08
#define CHANNEL_STACK   0x10
#define CHANNEL_ETH     0x20
#define CHANNEL_SMS     0x40
#define CHANNEL_NONE    0x80
//以上通道不够用，由于没有按位使用，因此增加新通道，不按位定义
#define CHANNEL_GPRS_SERVER    0x81
#define CHANNEL_GPRS_CLIENT    0x82
#define CHANNEL_ETH_SERVER    0x83
#define CHANNEL_ETH_CLIENT     0x84
#define CHANNEL_BEIDOU        0x85

#define LEN_ERTU_CFG_VERINFO        41

//采集信息管理总共占存储空间11K
#define MAX_TASK_COUNT                    20
#define MAX_NORMAL_PLAN_COUNT             20
#define MAX_EVENT_PLAN_COUNT              10
#define MAX_METER_DOC_SIZE                200
#define MAX_TASK_INFO_SIZE                100
#define MAX_NORMAL_PLAN_SIZE              400
#define MAX_EVENT_PLAN_SIZE               400
#define ACQ_CTRL_INFO_SATRT               8000
#define ACQ_METER_DOC_INFO                ACQ_CTRL_INFO_SATRT
#define ACQ_TASK_INDEX                    (ACQ_METER_DOC_INFO+900)
#define ACQ_NORMAL_PALM_INDEX             (ACQ_TASK_INDEX+25)
#define ACQ_EVENT_PLAN_INDEX              (ACQ_NORMAL_PALM_INDEX+15)
#define ACQ_TASK_INFO                     (ACQ_EVENT_PLAN_INDEX+15+45)    //45预留
#define ACQ_NORMAL_PLAN_INFO              (ACQ_TASK_INFO+MAX_TASK_COUNT*MAX_TASK_INFO_SIZE)
#define ACQ_EVENT_PLAN_INFO               (ACQ_NORMAL_PLAN_INFO+MAX_NORMAL_PLAN_COUNT*MAX_NORMAL_PLAN_SIZE)
#define ACQ_CTRL_INFO_END                 (ACQ_EVENT_PLAN_INFO+MAX_EVENT_PLAN_COUNT*MAX_EVENT_PLAN_SIZE)
//ESAM_OAD_LIST总共占存储空间1K
#define MAX_ESAM_OI_COUNT                 64
#define MAX_ESAM_PARAM_SIZE               1000
#define ESAM_PARAM_START                  ACQ_CTRL_INFO_END
#define ESAM_PARAM_END                    (ESAM_PARAM_START+MAX_ESAM_PARAM_SIZE)

#define EVENT_STORE_MAX                         15
#define TERMINAL_POWER_ONOFF                    (ESAM_PARAM_END+2)  //终端停上电。 这里我们从1000开始存储终端停上电的信息。
#define DAY_BYTES                               (TERMINAL_POWER_ONOFF+sizeof(StrTerminalPowerOnOffEventBlock)+2)//日流量 2个字节
#define MONTH_BYTES								(DAY_BYTES+2) //月流量 8个字节
#ifdef __OOP_GMETER_ERTU__
/************************************************************************/
/* 上电重构非侵入式终端中互感器参数（12块电表的表地）                        */
/************************************************************************/
#define ERTU_USER_NO		12
INT8U ertu_send_meter[100];
INT8U ertu_resp_meter[1000];
struct meter_no_set_from_ertu_st
{
	INT8U meter_index[12];
	INT8U meter_no[12][6];
	INT8U voltage_phase[12];	
}meter_no_set_from_ertu; 
INT8U read_meter_MS_set[12][6];	   //主站下发的欲读取的电表的地址。
INT8U read_meter_MS_set_mask[12];
INT8U seprate_frame_array[1000];
INT16U seprate_frame_len;
INT8U need_to_seperate;  //如果oad是46000300和46000400，需要标志分帧。本次条报文回复一次。
INT8U read_ertu_present_mask[12];
#endif
			
#endif




