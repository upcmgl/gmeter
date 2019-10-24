#ifndef __APP_VAR_3761_H__
#define __APP_VAR_3761_H__

#include "OOP/oop_type_def.h"
//#include "../main_include.h"
#include "../main_include.h"
//#include "oop_type_def.h"


#define SYS_FLAG_BASE  0
#define MAX_SYSTEM_FLAG_TYPE  1

//通信速率及通信端口号：
// 	D0~D4编码表示电能表、交流采样装置接入终端的通信端口号，数值范围1~31。
// 	D5~D7编码表示电能表、交流采样装置接入终端的通信波特率， 0~7依次表示300、600、1200、2400、4800、7200、9600、19200。
typedef union{
  struct{
    INT8U port:5;
    INT8U baud:3;
  };
  INT8U  value;
}BAUD_PORT_PARAM;

//有功电能示值的整数位及小数位个数：
// 	D0~D1：编码表示通信接入的电能表的有功电能示值的小数位个数，数值范围0~3依次表示1~4位小数。
// 	D2~D3：编码表示通信接入的电能表的有功电能示值的整数位个数，数值范围0~3依次表示4~7位整数。
typedef union{
   struct{
     INT8U  fraction:2;
     INT8U  integer:2;
     INT8U  v_block:1;
        INT8U is_F210:1;  //保存档案时传递参数，不真正保存
        INT8U porior_item:1; //湖南预抄断相数据切换数据项使用   #ifdef __PLC_PRIOR_F26_SWITCH__
        INT8U reserved:1;
   };
   INT8U  value;
}MBIT_INFO_PARAM;

//用户分类及电表分类
typedef union{
    INT8U  value;
    struct{
       INT8U  meter_class:4;  //电表分类
       INT8U  user_class:4;   //用户分类
    };
}METER_CLASS_PARAM;

// 测量点档案信息,整合设置参数F10
#define SIZEOF_METER_DOCUMENT   27
typedef union{
    INT8U  value[27];
    struct{
        INT8U          meter_idx[2];  //配置序号
        INT8U          spot_idx[2];   //所属测量点号
        BAUD_PORT_PARAM      baud_port;     //速率及端口
        INT8U          protocol;      //电表协议
        INT8U          meter_no[6];   //电表地址
        INT8U          password[6];   //通信密码
        INT8U          fl_count;      //费率个数
        MBIT_INFO_PARAM      mbit_info;     //表位数
        INT8U          rtu_no[6];     //采集器地址
        METER_CLASS_PARAM    meter_class;   //用户分类及电表分类
    };
}METER_DOCUMENT;     //27字节

//终端主动上报（I、II和III类数据）确认机制的处理
typedef struct{
    INT32U last_send_timer;
    INT16U timeout_second;
    INT8U  repeat_time;
    INT8U  check_seq;
    BOOLEAN need_check_ack;
    BOOLEAN has_acked;
    BOOLEAN has_send;
}tagActiveSendAck;

typedef struct{
    INT8U single_addr_counter[4];//单地址应用协商计数器
    INT8U report_counter[4];//主动上报计数器 
    INT8U broadcast_no[4];//应用广播通信序列号
}ESAM_COUNTER;
typedef union{
    INT8U value[4];
    struct{
    INT8U esam_id[8];
    INT8U esam_ver[4];
    INT8U symmetry_ver[16];
    INT8U cert_ver[2];
    INT8U session_valid_time[4];
    INT8U session_left_time[4];
    ESAM_COUNTER counter;
    INT8U msa_cert_no[16];
    INT8U dev_cert_no[16];
    };
}ESAM_INFO;

typedef struct{
    INT16U mark;
    INT16U count;
    INT8U  object_list[MAX_ESAM_OI_COUNT][5];
}ESAM_OI_LIST;

#if 0
typedef struct{
    INT32U  oad;            //数据类别
    INT32U  item_07_block;  //07抄表数据项块
    INT32U  item_07;        //07抄表数据项
    INT16U  offset;         //数据存储偏移量
    INT8U   item_count;     //数据长度
    INT8U   data_len;       //数据长度
    INT16U  block_data_len; //数据长度
    INT16U  page_id;
    INT16U  page_count;
}tagREADITEM;


typedef struct{
    tagREADITEM *read_item_list;
    INT8U *read_mask;
    INT8U read_item_count;
    INT8U read_mask_size;
    INT16U obis;
    INT16U patch_count; //补抄次数
}READ_LIST_PROP;
#endif


/*  */
#if 0
typedef union{
	INT8U value;
	struct{
		INT8U attribute_idx:5;
		INT8U attribute_spec:3;
	};
}ATTRIBUTE_ID1;
typedef union{
	INT8U value[4];
	struct{
		INT8U object_id[2];
		ATTRIBUTE_ID1 attribute_id;
		INT8U index_id;
	};
}OBJECT_ATTR_DESC1;

#define NORMAL_TASK_OAD_GROUP_MASK_SIZE        4U
#define NORMAL_TASK_GROUP_MAX_OAD_COUNT       (NORMAL_TASK_OAD_GROUP_MASK_SIZE*8)
typedef union{
    INT8U value[4];
    struct{
        INT8U oad_count;
        INT8U oad[NORMAL_TASK_GROUP_MAX_OAD_COUNT][4];/* OAD 需要抄读OAD存在这，组帧和处理数据的时候使用?? */
        INT8U save_off[NORMAL_TASK_GROUP_MAX_OAD_COUNT][2]; /* 存储的偏移位置 */
        INT8U mask_idx[NORMAL_TASK_GROUP_MAX_OAD_COUNT]; /* 掩码的位置信息，抄读成功后根据此信息清除?? */
        /* 预计应答长度 */
        INT8U resp_byte_num[NORMAL_TASK_GROUP_MAX_OAD_COUNT];
        OBJECT_ATTR_DESC1 master_oad;
    };
}GROUP_OAD_PARAMS;
typedef union{
    INT8U value[4];
    struct{
        INT8U plan_id;
        INT8U plan_type;
        INT8U depth[2];
        INT8U z_size[2];
        INT8U process_mask[8]; /* OAD处理掩码  */
        INT8U plan_oad[64][4];/* 方案的OAD先存到这里吧 */
        INT8U max_oad_cnt;// oad 最大数量
        INT8U oad_idx;// oad 处理位置 下次从之后开始处理
        GROUP_OAD_PARAMS group_oad_params;
        INT8U record_point_count; //每帧可以抄读几个点的数据
        INT8U cycle_record_count;
        INT8U last_time;
        INT8U time_interval[3];
        INT8U save_opt;//存储时标选项
    };
}NORMAL_TASK_SAVE_PARAMS;

typedef union{
    INT8U value[2];
    struct{
        INT8U                   task_idx[2];
        INT8U                   plan_type;
        INT8U                   idx_mask;
        INT8U                   idx_oad_group;
        INT8U                   idx_oad;
        //OBJECT_ATTR_DESC        oad;
        union{
            struct{
                NORMAL_TASK_SAVE_PARAMS norml_task_save_params;
                //READ_ITEM               read_item;
                INT8U                   normal_task_oad_mask[NORMAL_TASK_OAD_GROUP_MASK_SIZE];
                INT8U                   idx_oad_start;
                INT8U                   read_oad_count;
            };
        };
        union{
            
            //OAD_SAVE_PARAMS         oad_save_params;
            //OAD_SAVE_PARAMS_1       oad_save_params_1;
        };
        INT8U                   last_td[7];        //日月年
        INT8U                   hold_td[7];        //日月年
        //READ_MASK               read_mask;         //抄读掩码

        INT8U send_frame[300];
        INT8U recv_frame[500];
    };
}READ_PARAMS;


extern READ_PARAMS     read_params;
#endif
extern METER_DOCUMENT  gmeter_doc;
extern INT32S   read_c1_time_out;//数据超时变量
extern tagActiveSendAck gActiveSendAck;//主动上报数据的确认机制

extern ESAM_INFO g_esam_info;
extern ESAM_OI_LIST g_esam_oi_list;
extern ESAM_OI_LIST g_esam_oi_list_default;
extern INT32U sys_flag[MAX_SYSTEM_FLAG_TYPE];  //系统标志字,暂未使用

extern INT16U  frame_receive;

#ifdef __PROVIENCE_LIAONING_OOP__  //顺序如下 实时，日冻结,15分钟曲线
#define  G_TASK_PLAN_CNT  5
extern  INT8U gtask[G_TASK_PLAN_CNT][200];
extern  INT8U gplan[G_TASK_PLAN_CNT][200];
#endif
#endif

