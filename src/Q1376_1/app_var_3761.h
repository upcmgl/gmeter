#ifndef __APP_VAR_3761_H__
#define __APP_VAR_3761_H__

#include "../main_include.h"
//#include "app_dev_const.h"
//#include "app_const_3761.h"
//#include "app_var.h"
//#include "tpos_readport_common.h"

#define MAX_ERC_NO              64
//INT8U __attribute__ ((aligned(4))) g_temp_buffer[4096];         //全局临时变量缓冲区
INT8U   g_temp_buffer[100];
typedef struct{
     INT8U   acd;
     INT8U   ec1;         //重要事件计数器
     INT8U   ec2;         //一般事件计数器
     INT8U   alarm_event;//显示告警使用
     INT8U   ercflag[8]; //F8事件变更标志位BS64
     INT8U   event_ec[4];//32位的事件计数器
     INT8U   report_flag[256];
     INT8U   report_idx;
}tagGlobalEVENT;
extern tagGlobalEVENT g_event;
/*
//==============================================================================
//数据格式14:  XXXXXX.XXXX
typedef struct{
     INT8U   value[5];
}DATAFMT_14;


//数据格式11:  XXXXXX.XX
typedef struct{
     INT8U   value[4];
}DATAFMT_11;

*/
//数据格式15
/*
typedef union{
    INT8U   value[5];
    struct{
       INT8U   bcd_minute;
       INT8U   bcd_hour;
       INT8U   bcd_day;
       INT8U   bcd_month;
       INT8U   bcd_year;
    };
}DATAFMT_15;
*/
/*
//中文信息头
typedef union{
   INT8U  value;
   struct{
      INT8U  number:4;  //信息编号
      INT8U  type:4;    //信息分类： 0:普通信息， 1：重要信息
   };
}CTRLP_NOTE_INFO_TYPE;

#define NOTE_INFO_LEN  202
typedef union{
    INT8U  vlaue[2];
    struct{
       CTRLP_NOTE_INFO_TYPE   info_type;
       INT8U                  info_len;
    };
}CTRL_NOTE_INFO;
*/
//参数F1：终端上行通信口通信参数
typedef union{
    INT8U value[6];
    struct{
        INT8U    machine_delaytime;   //终端数传机延时时间RTS，单位20ms
        INT8U    send_delaytime;      //终端作为启动站允许发送传输延时时间，单位min
        INT16U   resp_timeout;        //终端等待从动站响应的超时时间和重发次数
        INT8U    confirm_service;     //需要主站确认的通信服务(CON=1)的标识
        INT8U    heartcycle;          //心跳周期，单位min
    };
}SET_F1;

typedef union{
    INT8U value[8];
    struct{
        SET_F1   F1;                  //F1的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F1;

//参数F2：终端上行通信口无线中继转发设置
typedef union{
    INT8U    value[33];
    struct{
        INT8U    ertu_number;         //被转发的终端地址数n
        INT16U   ertu_addr[16];       //被转发终端地址
    };
}SET_F2;

typedef union{
    INT8U    value[35];
    struct{
        SET_F2   F2;                  //F2的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F2;

//参数F3： 终端主站IP地址和端口
//APN,1376.1长度为16字节
typedef union{
    INT8U   value[44];
    struct{
        INT8U   ip1[4];
        INT8U   port1[2];
        INT8U   ip2[4];
        INT8U   port2[2];
        INT8U   APN[32];
     };
     struct{
        INT32U   msa_ip1;
        INT16U   msa_port1;
        INT32U   msa_ip2;
        INT16U   msa_port2;
        INT8U    gprs_APN[32];
     }w;
}SET_F3;

typedef union{
    INT8U    value[50];
    struct{                 //参数的校验值，采用累加和的计算得出
        SET_F3   F3;                  //F3的参数
        INT8U    reserve[6];          //预留
    };
}tagSET_F3;

//参数F4：主站电话号码和短信中心号码
typedef union{
    INT8U    value;
    struct{
        INT8U    number1:4;    //号码1
        INT8U    number2:4;    //号码2
    };
}NUMBER;     //参数F4的号码格式

typedef union{
    INT8U    value[16];
    struct{
        NUMBER    phone_number[8];    //主站电话号码
        NUMBER    sms_number[8];      //短信中心号码
    };
}SET_F4;

typedef union{
    INT8U    value[24];
    struct{
        SET_F4   F4;                  //F4的参数
        INT8U    reserve[8];          //预留
  };
}tagSET_F4;

//参数F5：终端上行通信消息认证参数
typedef union{
    INT8U    value[3];
    struct{
        INT8U    message_plan;        //消息认证方案号
        INT16U   plan_param;         //消息认证方案参数
    };
}SET_F5;

typedef union{
    INT8U    value[5];
    struct{
        SET_F5   F5;                  //F5的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F5;

//参数F6：终端组地址设置
#ifdef __RTUADDR_IS_6BYTE__
typedef union{
    INT8U    value[32];
    struct{
        INT8U   group_addr[8][4];       //终端组地址，共8组
    };    
}SET_F6;

typedef union{
    INT8U    value[35];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F6  F6;                   //F6的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F6;

typedef union{
    INT8U    value[16];
    struct{
        INT16U   group_addr[8];       //终端组地址，共8组
    };
}SET_F6_1;

typedef union{
    INT8U    value[19];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F6_1  F6;                   //F6的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F6_1;
#else
typedef union{
    INT8U    value[32];
    struct{
        INT8U   group_addr[8][4];       //终端组地址，共8组
    };    
}SET_F6_1;

typedef union{
    INT8U    value[19];
    struct{
        SET_F6_1  F6;                   //F6的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F6_1;

typedef union{
    INT8U    value[16];
    struct{
        INT16U   group_addr[8];       //终端组地址，共8组
    };
}SET_F6;

typedef union{
    INT8U    value[18];
    struct{
        SET_F6  F6;                   //F6的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F6;
#endif
//参数F7： 终端IP地址和端口
typedef union{
    INT8U  value[12];
    struct{
       //终端IP地址
       INT8U   ip1;
       INT8U   ip2;
       INT8U   ip3;
       INT8U   ip4;
       //子网掩码
       INT8U   mask1;
       INT8U   mask2;
       INT8U   mask3;
       INT8U   mask4;
       //网关地址
       INT8U   gate1;
       INT8U   gate2;
       INT8U   gate3;
       INT8U   gate4;
       //代理服务器
       INT8U   proxy_type;
       INT8U   proxy_ip1;
       INT8U   proxy_ip2;
       INT8U   proxy_ip3;
       INT8U   proxy_ip4;
       INT8U   proxy_port[2];
       INT8U   proxy_con_type;
       INT8U   proxy_user_name_len;
       INT8U   proxy_user_name[20];
       INT8U   proxy_pass_len;
       INT8U   proxy_pass[20];
       //终端侦听端口
       INT8U   listen_port[2];
    };
    struct{
       INT32U   local_ip;
       INT32U   mask_ip;
       INT32U   gate_ip;
    };
    struct{
       INT8U   ip_info[20];
       INT8U   proxy_user_name_len;
       INT8U   proxy_user_pass[43];
    }b; 
}tagERTU_ETH_IP;


typedef union{
   INT8U  value[64];
   struct{
     tagERTU_ETH_IP  eth_ip;
   };
}SET_F7;

typedef union{
    INT8U  value[70];
    struct{
        SET_F7  F7;                   //F7的参数
        INT8U    reserve[6];          //预留
    };
}tagSET_F7;

//参数F8：终端上行通信虚拟专网工作方式
typedef union{
    INT8U  value[8];
    struct{
        INT8U  work_mode;              //公网通信模块（GPRS或CDMA）工作模式   BIN  1
        INT8U  redail_interval[2];     //永久在线、时段在线模式重拨间隔       BIN  2
        INT8U  passive_dailcount;      //被动激活模式重拨次数                 BIN  1
        INT8U  passive_offline_time;   //被动激活模式连续无通信自动断线时间   BIN  1
        INT8U  time_segments[3];       //时段在线模式允许在线时段标志         BS24 3
    };
    struct{
        INT8U  online_mode:2;  //在线模式 1--永久  2--被动  3--时段
        INT8U  tmp1:2;
        INT8U  serv_mode:2;  //服务器模式：  0-混合模式  1--客户机模式  2--服务器模式
        INT8U  tmp2:1;
        INT8U  conn_mode:1;//连接模式 0-TCP 1-UDP
    }mode;
}SET_F8;

typedef union{
    INT8U  value[11];
    struct{
        SET_F8   F8;
        INT8U    reserve[2];          //预留
    };
}tagSET_F8;

//参数F9：终端事件记录配置
typedef union{
    INT8U    value[32];
    struct{
        INT8U    event_valid[16];       //事件记录有效标识位，按位表示
        INT8U    evnet_VIP[16];          //事件重要性等级标识位，按位表示
    };
}SET_F9;

typedef union{
    INT8U    value[34];
    struct{
        SET_F9   F9;
        INT8U    reserve[2];          //预留
    };
}tagSET_F9;

//F11：终端脉冲配置(8路)
typedef union{
    INT8U    value[5];
    struct{
        INT8U    pulse_port;          //脉冲输入端口号
        INT8U    meter_idx;           //所属测量点号
        INT8U    pulse_attr;          //脉冲属性，BS8格式，
        INT16U   pulse_const;         //电能表常数k
    };
}PULSE_PARAM;       //脉冲配置的参数

typedef union{
    INT8U    value[40];
    struct{
        PULSE_PARAM  pulse[8];       //脉冲配置参数
    };
}SET_F11;

typedef union{
    INT8U    value[42];
    struct{
        SET_F11  F11;                 //F11的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F11;

//F12：终端状态量输入参数
typedef union{
    INT8U    value[2];
    struct{
        INT8U    state_access;        //状态量接入标识位，BS8格式，按位表示
        INT8U    state_attr;          //状态量属性标识位，BS8格式，按位表示
    };
}SET_F12;

typedef union{
    INT8U    value[5];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F12  F12;                 //F12的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F12;

//F13：终端电压/电流模拟量配置参数(8路)
typedef union{
    INT8U    value[3];
    struct{
        INT8U    ad_port;             //电压/电流模拟量输入端口号
        INT8U    meter_idx;           //所属测量点号
        INT8U    ad_attr;             //电压/电流模拟量属性
    };
}AD_PARAM;     //模拟量配置的参数

typedef union{
    INT8U    value[24];
    struct{
        AD_PARAM  ad[8];              //电压/电流模拟量配置参数
    };
}SET_F13;

typedef union{
    INT8U    value[28];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F13  F13;                 //F13的参数
        INT8U    reserve[3];          //预留
    };
}tagSET_F13;

//总加标志的结构体
typedef union{
    INT8U    value;
    struct{
        INT8U spot_idx:6;     //测量点号
        INT8U dir:1;        //测量点参与总加计算是正向还是反向，0-正向；1-反向
        INT8U flag:1;          //总加运算的运算符标志，0-加；1-减
    };
}tagOP_FLAG;

//终端总加组配置参数，协议对应的数据格式
typedef union{
    INT8U  value[10];
    struct{
       INT8U   agp;           //总加组序号1-8
       INT8U   spot_count;    //测量点数量(0-8)
       tagOP_FLAG   op_flag[8];    //总加标志
    };
}tagAGP_SET;


typedef union{
    INT8U    value[83];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        tagAGP_SET   set_f14[8];      //测量点最大64，按照测量点对应总加组的方式存储
        INT8U    reserve[2];          //预留
    };
}tagSET_F14;

//F15：有功总电能量差动越限事件参数设置
typedef union{
    INT8U    value;
    struct{
        INT8U    time_interval:2;     //电能量的时间跨度,0：60min；1：30min；2：15min
        INT8U    reserve:5;           //预留
        INT8U    contrast_method:1;   //对比方法标识，0：相对对比；1：绝对对比
    };
}DIFF_PARAM;      //参与差动的电能量的时间区间及对比方法标识

typedef union{
    INT8U    value[9];
    struct{
        INT8U    diff_id;              //有功总电能量差动组序号
        INT8U    contrast_agp;         //对比的总加组序号
        INT8U    refer_agp;            //参照的总加组序号
        DIFF_PARAM  diff_param;        //参与差动的电能量的时间区间及对比方法标识
        INT8U    relative_deviation;   //差动越限相对偏差值
        INT32U   absolute_deviation;   //差动越限绝对偏差值
    };
}SET_F15;

typedef union{
    INT8U    value[75];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F15  set_f15[8];          //F15参数，差动组配置数量最大8
        INT8U    reserve[2];          //预留
    };
}tagSET_F15;

//F16：虚拟专网用户名、密码
typedef union{
    INT8U    value[64];
    struct{
        INT8U    net_username[32];     //虚拟专网用户名
        INT8U    net_password[32];      //虚拟专网密码
    };
}SET_F16;


typedef union{
    INT8U    value[67];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F16  set_f16;             //F16的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F16;

//F17：终端保安定值
typedef union{
    INT8U    value[2];
    struct{
        INT8U    security_pyg[2];     //终端保安定值，数据格式2
    };
}SET_F17;

typedef union{
    INT8U    value[5];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F17  set_f17;             //F17的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F17;

//F18：终端功控时段
typedef union{
    INT8U    value;
    struct{
        INT8U    power_ctrl_time1:2;             //时段1，30min
        INT8U    power_ctrl_time2:2;             //时段2，30min
        INT8U    power_ctrl_time3:2;             //时段3，30min
        INT8U    power_ctrl_time4:2;             //时段4，30min
    };
}POWER_CTRL_TIME;

typedef union{
    INT8U    value[12];
    struct{
        POWER_CTRL_TIME  power_ctrl_time[12];          //功控时段，30min，共48个时段
    };
}SET_F18;

typedef union{
    INT8U    value[15];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F18  set_f18;             //F18的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F18;

//F19：终端时段功控定值浮动系数
typedef union{
    INT8U    value;
    struct{
        INT8U    power_ctrl_float_coff;     //时段功控定值浮动系数
    };
}SET_F19;

typedef union{
    INT8U    value[4];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F19  set_f19;             //F19的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F19;

//F20：终端月电能量控定值浮动系数
typedef union{
    INT8U    value;
    struct{
        INT8U    energy_ctrl_float_coff;     //月电能量控定值浮动系数
    };
}SET_F20;

typedef union{
    INT8U    value[4];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F20  set_f20;             //F20的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F20;

//F21：终端电能量费率时段和费率数

//F22：终端电能量费率

//F23：终端催费告警参数
typedef union{
    INT8U    value[3];
    struct{
        INT8U    fee_alarm_flag[3];     //催费告警允许/禁止标识位,BS8格式,D0~D23按顺序对应表示0~23点，置1表示告警
    };
}SET_F23;

typedef union{
    INT8U    value[6];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F23  set_f23;             //F23的参数值
        INT8U    reserve[2];          //预留
    };
}tagSET_F23;

//F25：测量点基本参数（电表参数）
//为方便读取测量点PT，CT定义数据结构
typedef union{
    INT8U    value[4];
    struct{
       INT16U  pt;     //电压互感器倍率
       INT16U  ct;     //电流互感器倍率
    };
}SPOT_PTCT;

typedef union{
    INT8U    value[11];
    struct{
        SPOT_PTCT  spot_ptct;        //PT、CT参数
        INT16U     v_rated;          //额定电压，单位V
        INT8U      i_vated;          //额定电流，单位A
        INT8U      p_vated[3];       //额定负荷，单位kVA
        INT8U      connect_mode;     //电源接线方式，BS8格式，D0~D1表示测量点电源接线方式，D2~D3表示单相表接线相，D4~D7备用
    };
}SET_F25;

typedef union{
    INT8U    value[14];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F25  set_f25;             //F25的参数值
        INT8U    reserve[2];          //预留
    };
}tagSET_F25;

//F26：测量点限值参数（电表参数）
typedef union{
    INT8U  value[5];
    struct{
       INT8U   set_max[2];          //门限
       INT8U   last_time;           //持续时间
       INT8U   back_coff[2];        //恢复系数
    };
}SET_F26_OVER_V;

typedef union{
    INT8U  value[6];
    struct{
       INT8U   set_max[3];          //门限
       INT8U   last_time;           //持续时间
       INT8U   back_coff[2];        //恢复系数
    };
}SET_F26_OVER_I;


typedef union{
   INT8U value[57];
   struct{
      INT8U           v_max[2];       //电压合格上限
      INT8U           v_min[2];       //电压合格下限
      INT8U           v_lost[2];      //电压断相门限
      SET_F26_OVER_V  v_over_maxmax;  //电压上上限
      SET_F26_OVER_V  v_over_minmin;  //电压下下限
      SET_F26_OVER_I  i_over_maxmax;  //相电流上上限
      SET_F26_OVER_I  i_over_max;     //相电流上限
      SET_F26_OVER_I  in_over_max;    //零电流上限
      SET_F26_OVER_I  s_over_maxmax;  //视在功率超上上限
      SET_F26_OVER_I  s_over_max;     //视在功率超上限
      SET_F26_OVER_V  v_unb_over;     //电压不平衡
      SET_F26_OVER_V  i_unb_over;     //电流不平衡
      INT8U           lost_v_last_time;  //连续失压时间限值
   };
}SET_F26;

typedef union{
    INT8U    value[60];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F26  set_f26;             //F26的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F26;


//F27：测量点铜损、铁损参数（电表参数）

//F28：测量点功率因数分段限值（电表参数）

//F29：终端当地电能表显示号（电表参数）

//F30：台区集中抄表停抄/投抄设置（电表参数）
typedef union{
    INT8U    value;
    struct{
        INT8U    rec_flag;          //集中抄表停抄/投抄设置
    };
}SET_F30;

typedef union{
    INT8U    value[4];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F30  set_f30;            //F30的参数值
        INT8U    reserve[2];          //预留
    };
}tagSET_F30;


//F31：从节点附属节点地址

//F33：终端抄表运行参数设置
typedef union{
    INT8U    value[110];
    struct{
        INT8U    port;          // 终端通信端口号	BIN	1	本次设置的
        INT8U    run_ctrl[2];   // 台区集中抄表运行控制字	BS16	2
        INT8U    rec_days[4];   // 抄表日-日期	BS32	4
        INT8U    rec_time[2];   // 抄表日-时间	数据格式19	2
        INT8U    cycle;         // 抄表间隔         BIN     1
        INT8U    cast_time[3];  // 对电表广播校时定时时间	数据格式18	3
        INT8U    seg_count;       // 抄表时段数量
        INT8U    rec_timeseg[96]; // 抄表时段信息   时-分
    };
    struct{
        INT8U    tmp[13];            //其他
        INT8U    seg_count;          // 抄表时段数量
        INT8U    rec_timeseg[24][4]; //24个时段
    }seg;
}SET_F33;

typedef union{
    INT8U    value[113];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F33  set_f33;             //F33的参数
        INT8U    reserve[2];          //预留
    };
}tagSET_F33;




typedef struct{
    INT8U    rec_days[4];        // 抄表日-日期	BS32	4
    INT8U    seg_count;          // 抄表时段数量
    INT8U    rec_timeseg[24][4]; //24个时段
}JCSET_F801;

typedef union{
    INT8U value[4];
    struct{
        INT8U    year;
        INT8U    month;
        INT8U    day;
        INT8U    hour;
        INT8U    v[6];
        INT8U    i[9];
    };
}CURVE_V_I;


//参数： F33，  DL698_41_0928 版本定义
typedef struct{
    INT8U    port;            // 终端通信端口号	BIN	1	本次设置的
    INT8U    run_ctrl[2];     // 台区集中抄表运行控制字	BS16	2
    INT8U    rec_timeseg[12]; // 抄表时段信息  15分钟间隔，用96个位标记禁止或允许，1==禁止
    INT8U    rec_days[4];     // 抄表日-日期	BS32	4
    INT8U    rec_time[2];     // 抄表日-时间	数据格式19	2
    INT8U    cycle;           // 抄表间隔         BIN     1
    INT8U    cast_time[3];    // 对电表广播校时定时时间	数据格式18	3
}SET_F33_DL698_41_0928;


//参数: 终端支持三个可以设置的抄表端口: 1--载波口   2--抄表485口   3--级联485口(如果设置为级联,则不可用)
//按照协议进行对应端口的支持，1--交采485；2--抄表485；3--级联485；31--载波口
//typedef struct{
//    JCSET_F33  port_set[4];
//}SET_F33_ALL;


//F34：集中器下行通信模块的参数设置

//F35：台区集中抄表重点户设置

//F36：终端上行通信流量门限设置

//F37：终端级联通信参数

//级联/被级联标志、及其对应的被级联/级联的终端个数
typedef union{
   INT8U value;
   struct{
      INT8U  count:4;        //当slave=1时，count=1,表示级联方终端。
      INT8U  reserved:3;
      INT8U  slave:1;        //0"表示级联方，"1"表示被级联方
   };
}CASCADE_FLAG;


//通信控制字
typedef union{
    INT8U   port_ctrl;
    struct{
       INT8U   databits:2;    //数据位数
       INT8U   parity:1;      //偶校验
       INT8U   has_parity:1;  //有无校验位
       INT8U   stopbits:1;    //停止位
       INT8U   baudrate:3;    //波特率
    };
}PORT_COMM_CTRL;


typedef union{
   INT8U  value[23];
   struct{
       INT8U           port;             //终端级联通信端口号	BIN		1
       PORT_COMM_CTRL  ctrl;             //终端级联通信控制字	BS8		1
       INT8U           frame_timeout;    //接收等待报文超时时间	BIN	100ms	1
       INT8U           byte_timeout;     //接收等待字节超时时间	BIN	10ms	1
       INT8U           try_count;        //级联方（主动站）接收失败重发次数	BIN	次	1
       INT8U           check_interval;   // 级联巡测周期	BIN	min	1
       CASCADE_FLAG    cascade_flag;     //级联/被级联标志、及其对应的被级联/级联的终端个数n	BIN		1
       INT8U           ertu_addr[4][4];  //4个被级联的终端行政区划码/级联方的终端行政区划码	BCD		2
   };
}SET_F37;

typedef union{
    INT8U    value[28];
    struct{
        INT8U    cs;                  //参数的校验值，采用累加和的计算得出
        SET_F37  set_f37;             //F37的参数
        INT8U    reserve[4];          //预留，一个终端地址的长度
    };
}tagSET_F37;

//F38：I类数据配置设置

//F39：II类数据配置设置

//F41：时段功控定值
typedef union{
    INT8U    value[17];
    struct{
        INT8U    timeseg_flag;  //时段号，D0~D7按顺序对位表示第1~第8时段
        INT8U    timeseg1_power_ctrl_value[2];    //时段1功率定值
        INT8U    timeseg2_power_ctrl_value[2];    //时段2功率定值
        INT8U    timeseg3_power_ctrl_value[2];    //时段3功率定值
        INT8U    timeseg4_power_ctrl_value[2];    //时段4功率定值
        INT8U    timeseg5_power_ctrl_value[2];    //时段5功率定值
        INT8U    timeseg6_power_ctrl_value[2];    //时段6功率定值
        INT8U    timeseg7_power_ctrl_value[2];    //时段7功率定值
        INT8U    timeseg8_power_ctrl_value[2];    //时段8功率定值
    };
}TIMESEG_CTRL_PLAN;

typedef union{
    INT8U    value[52];
    struct{
        INT8U    timeseg_plan_flag;   //方案标识,D0~D2按位表示1~3套方案，置1有效
        TIMESEG_CTRL_PLAN    timeseg_plan_value[3];
    };
}SET_F41;

typedef union{
    INT8U    value[55];
    struct{
        INT8U    cs;
        SET_F41  set_f41;
        INT8U    reserve[2];
    };
}tagSET_F41;

//F42：厂休功控参数
typedef union{
    INT8U    value[6];
    struct{
        INT8U   relax_ctrl_set_value[2];    //厂休控定值
        INT8U   limit_power_begin_time[2];  //限电起始时间
        INT8U   limit_power_duration;       //限电持续时间(范围为1~48)
        INT8U   limit_power_day;            //每周限电日:D1~D7表示星期一~星期日，D0=0
    };
}SET_F42;

typedef union{
    INT8U    value[9];
    struct{
        INT8U    cs;
        SET_F42  set_f42;
        INT8U    reserve[2];
    };
}tagSET_F42;

//F43：功率控制的功率计算滑差时间
typedef struct{
    INT8U    cal_power_winsize;   //功率控制的功率计算滑差时间(数值范围：1~60)
}SET_F43;

typedef union{
    INT8U    value[4];
    struct{
        INT8U    cs;
        SET_F43  set_f43;
        INT8U    reserve[2];
    };
}tagSET_F43;

//F44：营业报停控参数
typedef union{
    INT8U    value[8];
    struct{
        INT8U  stopwork_ctrl_begin_time[3];    //报停起始时间
        INT8U  stopwork_ctrl_end_time[3];      //报停结束时间
        INT8U  stopwork_ctrl_set_value[2];     //报停控功率定值
    };
}SET_F44;

typedef union{
    INT8U    value[11];
    struct{
        INT8U    cs;
        SET_F44  set_f44;
        INT8U    reserve[2];
    };
}tagSET_F44;

//F45：功控轮次设定
typedef struct{
    INT8U    power_ctrl_sw_flag;    //功控轮次设定
}SET_F45;

typedef union{
    INT8U    value[4];
    struct{
        INT8U    cs;
        SET_F45  set_f45;
        INT8U    reserve[2];
    };
}tagSET_F45;

#if ((defined __QGDW_376_2013_PROTOCOL__) && (!defined __QGDW_FK2005__))
//F46：月电量控定值
typedef union{
    INT8U    value[5];
    struct{
        INT8U  monthfee_ctrl_set_value[4];   //月电量控定值
        INT8U  alarm_limit_coff;             //报警门限值系数
    };
}SET_F46;
#else
//F46：月电量控定值
typedef union{
    INT8U    value[4];
    struct{
        INT8U  monthfee_ctrl_set_value[4];   //月电量控定值
    };
}SET_F46;
#endif
#if ((defined __QGDW_376_2013_PROTOCOL__) && (!defined __QGDW_FK2005__))
typedef union{
    INT8U    value[8];
    struct{
        INT8U    cs;
        SET_F46  set_f46;
        INT8U    reserve[2];
    };
}tagSET_F46;
#else
typedef union{
    INT8U    value[7];
    struct{
        INT8U    cs;
        SET_F46  set_f46;
        INT8U    reserve[2];
    };
}tagSET_F46;
#endif

//F47：购电量(费)控参数
typedef union{
    INT8U    value[17];
    struct{
        INT8U    buyfee_number[4];      //购电单号
        INT8U    buyfee_flag;           //追加/刷新标识
        INT8U    buyfee_value[4];       //购电量(费)值
        INT8U    alarm_limit_value[4];   //报警门限值
        INT8U    switch_limit_value[4];  //跳闸门限值
    };
}SET_F47;

typedef union{
    INT8U    value[20];
    struct{
        INT8U    cs;
        SET_F47  set_f47;
        INT8U    reserve[2];
    };
}tagSET_F47;

//F48：电控轮次设定
typedef struct{
    INT8U  fee_ctrl_sw_flag;    //电控轮次设定
}SET_F48;

typedef union{
    INT8U    value[4];
    struct{
        INT8U    cs;
        SET_F48  set_f48;
        INT8U    reserve[2];
    };
}tagSET_F48;

//F49：功控告警时间
typedef struct{
    INT8U  power_ctrl_alarm_time[8];    //功控告警时间,8个轮次
}SET_F49;

typedef union{
    INT8U    value[11];
    struct{
        INT8U    cs;
        SET_F49  set_f49;
        INT8U    reserve[2];
    };
}tagSET_F49;

//F57：终端声音告警允许/禁止设置

//F58：终端自动保电参数

//F59：电能表异常判别阀值设定

//F60：谐波限值

//F61：直流模拟量接入参数

//F65：定时上报I类数据任务设置

//F66：定时上报II类数据任务设置

//F67：定时上报I类数据任务启动/停止设置

//F68：定时上报II类数据任务启动/停止设置

//F73：电容器参数

//F74：电容器投/切运行参数

//F75：电容器保护参数

//F76：电容器投/切控制方式

//F81：直流模拟量输入变化

//F82：直流模拟量限值

//F83：直流模拟量冻结参数

//F89：终端逻辑地址

//F90：设置无线通信参数

//F91：终端地理位置信息
//F97:停电数据采集配置参数
typedef union
{
    INT8U value;
    struct{
        INT8U is_use:1;     //是否有效
        INT8U is_random:1;  //是否随机
        INT8U reserve:6;    //备用
    };
}tagPOWER_ONOFF_READ_FLAG;
typedef union{
    INT8U    value[13];
    struct{
        tagPOWER_ONOFF_READ_FLAG    read_flag; //停电数据采集标志
        INT8U    read_interval_time;           //停电事件读取时间间隔
        INT8U    read_max_time;                //停电事件读取时间限值
        INT8U    meter_count[2];               //要读取停电事件电能表个数,最大支持4个
        INT8U    meter_list[8];                //要读取电能表测量点号,最大支持4个
    };
}tagSET_F97;
//F98：停电事件甄别限值参数
typedef union{
    INT8U    value[13];
    struct{
        INT8U    power_lost_min_time[2];                 //停电时间最小有效间隔
        INT8U    power_lost_max_time[2];                 //停电时间最大有效间隔
        INT8U    start_stop_time_deviation[2];           //停电事件起止时间偏差限值
        INT8U    time_interval_deviation[2];             //停电事件时间区段偏差限值
        INT8U    power_off_v_limit[2];                   //终端停电发生电压限值
        INT8U    power_on_v_limit[2];                    //终端停电恢复电压限值
        INT8U    report_flag;                            //停电事件主动上报标志位
    };
}tagSET_F98;
typedef struct
{
    tagSET_F98  power_onoff_param;
    tagSET_F97  power_onoff_config;
}tagPowerOnoffEvent;
//F150：测量点状态

//采集器下关联的电表信息
typedef union{
    INT8U  value[128];
    struct{
       INT16U  meter_seq[64];
    };
}CJQ_METER_INFO;


//
//通信速率及通信端口号：
// 	D0~D4编码表示电能表、交流采样装置接入终端的通信端口号，数值范围1~31。
// 	D5~D7编码表示电能表、交流采样装置接入终端的通信波特率， 0~7依次表示300、600、1200、2400、4800、7200、9600、19200。
//
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

//
// 测量点档案信息,整合设置参数F10
//

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




//
//  电表抄读信息
//
typedef struct{
    INT8U       actphase;       // 载波抄读通信相位	BS8	1
    INT8U       plc_q;           // 载波信号品质	BS8	1
}METER_REC_STATE;   //18字节


//添加节点的节点信息
typedef union{
   INT8U   value[9];
   struct{
      INT8U node_no[6];
      INT8U node_seq[2];
      INT8U node_sepc;
   };
}ROUTER_NODE_ITEM;

#ifdef __376_2_2013__
typedef union{
    INT8U value[7];
    struct{
        INT8U node_no[6];
        INT8U node_sepc;
    };
}ROUTER_NODE_ITEM_NEW;
#endif

//批量添加节点信息,5个节点添加
typedef union{
   INT8U value[46];
   struct{
        INT8U count;
      ROUTER_NODE_ITEM  node[5];
   };
    #ifdef __376_2_2013__
    struct{
        INT8U count;
        ROUTER_NODE_ITEM_NEW node[5];
    }b;
    #endif
}ROUTER_ADD_5NODE_INFO;

//删除载波从节点节点
typedef union{
   INT8U value[7];
   struct{
      INT8U count;     //这里恒取1
      INT8U node_no[6];
   };
}ROUTER_DEL_NODE_INFO;

typedef union{
   INT8U value[7];
   struct{
      INT8U count;
      INT8U node_no[10][6];
   };
}ROUTER_NOTICE_NODE_INFO;




//电表抄读中继与相位信息
typedef union{
   INT8U   value;
   struct{
      INT8U   readphase:2;    //抄读相位
      INT8U   relay:5;        //中继深度 0-15
      INT8U   flag:1;         //成功标志
   };
}RELAYPHASE;

//电表抄读中继与相位信息
typedef union{
    INT8U   value;
    struct{
        INT8U is_meter_event:1;    //是否抄读电表事件数据项   __METER_EVENT_GRADE__
        INT8U is_curve:1;          //是否抄读电表曲线数据项   __METER_CURVE_READ__
        INT8U is_report_meter_event:1;     //是否抄读电表曲线数据项   __METER_CURVE_READ__
        INT8U tmp:5;               //备用
    };
}REC_CTRL_INFO;



//抄表时段
typedef union
{
    INT8U value[4];
    struct{
       INT8U  from_hour;
       INT8U  from_minute;
       INT8U  end_hour;
       INT8U  end_minute;
    };
}REC_TIMESEG;





//DL/T698  数据格式01
/*
typedef union{
     INT8U     bcd_datetime[6];
     struct{
       INT8U       second;
       INT8U       minute;
       INT8U       hour;
       INT8U       day;
       INT8U       week_month;
       INT8U       year;
     };
}DATAFMT_01;
 */
/*
//重点表抄读数据
typedef union{
    INT8U  value[8];
    struct{
        INT8U  year;
        INT8U  month;
        INT8U  day;
        INT8U  hour;
        INT8U  zxygz[4];
        #if defined (__PLC_READ_FXYG__) || defined(__PLC_READ_VIP_FYG_WG__)
        INT8U  fxygz[4];
        #endif
        #ifdef __PLC_READ_VIP_FYG_WG__
        INT8U  zxwgz[4];
        INT8U  fxwgz[4];
        #endif
        #if defined(__PLC_READ_VIP_F25__) || defined(__PLC_READ_VIP_LOAD_RECORD__)
        INT8U p_yg[12];      //总,A,B,C相有功功率  数据格式09  4*3
        INT8U p_wg[12];      //总,A,B,C相无功功率  数据格式09  4*3
        INT8U Voltage[6];    //A,B,C相电压  数据格式07   3*2
        INT8U I[12];         //A,B,C,N相电流  数据格式25   4*3
        INT8U Q[8];          //总,A,B,C相功率因数   4*2
        #endif
    };
}C_VIP_ZXYGZ;

//电压表一天曲线
typedef union{
    INT8U  value[7];
    struct{
        INT8U  rec_date[5];
        INT8U  Voltage_A[2];

    };
}C_VOLTMETER;
typedef union{
    INT8U  value[23];
    struct{
        INT8U  rec_date[5];
        INT8U  Block[15];

    };
}C_VOLTMETER_BLOCK;  //负荷记录的格式
typedef union{
    INT8U  value[37];
    struct{
        INT8U  rec_date[5];
        INT8U  Block[24];
        INT8U  glys[8];
    };
}C_POWER_BLOCK;  //负荷记录的格式

typedef union{
    INT8U  value[9];
    struct{
        INT8U  rec_date[5];
        INT8U  CURVE_YWG[4];

    };
}C_CURVE_PATCH;

//重点表抄读数据

#if (defined(__PLC_READ_FXYG__) && defined(__PLC_READ_VIP_F25__))
  //包含反向有功和F25的定义 
  typedef union{
     INT8U  value[8];
     struct{
        INT8U  year;
        INT8U  month;
        INT8U  day;
        INT8U  hour;
        INT8U  zxygz[4];

        //__PLC_READ_FXYG__
        INT8U  fxygz[4]; 
    
        //__PLC_READ_VIP_F25__
        INT8U p_yg[12];      //总,A,B,C相有功功率  数据格式09  4*3
        INT8U p_wg[12];      //总,A,B,C相无功功率  数据格式09  4*3
        INT8U Voltage[6];    //A,B,C相电压  数据格式07   3*2
        INT8U I[12];         //A,B,C,N相电流  数据格式25   4*3 
     };
  }C_VIP_ZXYGZ;
#elif defined(__PLC_READ_FXYG__)
   //包含反向有功的定义
   typedef union{
     INT8U  value[8];
     struct{
        INT8U  year;
        INT8U  month;
        INT8U  day;
        INT8U  hour;
        INT8U  zxygz[4];

        //__PLC_READ_FXYG__
        INT8U  fxygz[4];     
    };
   }C_VIP_ZXYGZ;
#elif defined(__PLC_READ_VIP_F25__)
   //包含F25的定义
   typedef union{
      INT8U  value[8];
      struct{
           INT8U  year;
           INT8U  month;
           INT8U  day;
           INT8U  hour;
           INT8U  zxygz[4];

           //__PLC_READ_VIP_F25__
           INT8U p_yg[12];      //总,A,B,C相有功功率  数据格式09  4*3
           INT8U p_wg[12];      //总,A,B,C相无功功率  数据格式09  4*3
           INT8U Voltage[6];    //A,B,C相电压  数据格式07   3*2
           INT8U I[12];         //A,B,C,N相电流  数据格式25   4*3 
      };
   }C_VIP_ZXYGZ;
#elif defined(__PROVICE_JIANGSU__)
typedef union{
    INT8U  value[8];
    struct{
        INT8U  year;
        INT8U  month;
        INT8U  day;
        INT8U  hour;
        INT8U  zxygz[4];
        INT8U  zxwgz[4];
        INT8U  fxygz[4];
        INT8U  fxwgz[4];    
    };
}C_VIP_ZXYGZ;
#else
     //标准定义
     typedef union{
         INT8U  value[8];
         struct{
            INT8U  year;
            INT8U  month;
            INT8U  day;
            INT8U  hour;
            INT8U  zxygz[4];
      };
     }C_VIP_ZXYGZ;

#endif   */


//自定义：GPRS信息
typedef union{
   INT8U value[66];
   struct{
      INT8U csq;           //信号强度
      INT8U ber;           //误码率
      INT8U op;            //当前操作
      INT8U gprs_end_state;         //最终状态
      char          temp;          //当前温度
      INT8U mac[6];        //mac地址
      INT8U g14_ver[8];    //g14版本信息，V3.0没有
      INT8U ctrl_ver[5];   //控制模块版本信息
      INT8U soft_ver[7];   //内部版本
      INT8U provice[3];   //省份特征字，V3.0暂时没有
      INT8U ram_status;   //内存是否损坏,AA-正常 0-损坏
      INT8U osc_status;   //主晶振是否异常，0xAA-正常 1-异常
      INT8U rtcc_status;   //RTCC工作状态，0xAA-正常 1-异常
      INT8U error_status;//D0-交采通讯故障,1为故障 D1-时钟故障状态 D2-蓝牙通讯故障 D3-ESAM通讯故障
      INT8U check_key;                //工装检测按键bit7-2：6个按键（1检测到，0未检测到），bit1-0: 01 开启按键检测；
      INT8U reserved[5];      //保留
      INT8U sim_id[20];    //SIM卡ID
      INT8U djb_ver[16]; //蓝牙版本 
      INT8U gprs_type[3];//模块类型，字符
      INT8U gprs_imsi[15];//imsi
      INT8U config_ver[2];           //配置脚本文件版本
      INT8U config_provice[4];       //配置脚本文件省份代码
      INT8U bad_block_count[2];       //Flash中坏块数量
      INT8U esam_type;//01-13规范esam，02-698 esam，其它-未知
   };
}tagWORKINFO;
/*
typedef union{
    INT8U  value[2];
    struct{
        INT16U   meter_idx:11;
        INT16U   rec_flag:5;
    };
}PATCH_REC_FLAG;

typedef union{
    INT8U  value[5];
    struct{
        INT8U item[4];
        INT8U level;
    };
}PARAM_F106;  //#ifdef __METER_EVENT_GRADE__

////////////////////////////////////////////////////////////////////
//
//         查询用数据结构
//
////////////////////////////////////////////////////////////////////
*/
typedef union
{
   INT8U  first_flag;
   struct{
      INT8U  first_F10_flag:1;
      INT8U  first_F89_flag:1;
      INT8U  first_F200_flag:1;
      INT8U  first_F112_flag:1;
      INT8U  first_F106_flag:1;   //__METER_EVENT_GRADE__
      INT8U  first_F306_flag:1;   //__BATCH_TRANSPARENT_METER_CYCLE_TASK__
      INT8U  first_F310_flag:1;
      INT8U  reserved:1;
   };
}QUERY_FIRST_CTRL;

typedef struct{
     INT8U              *req;         //请求参数
     INT16U             req_bytes;    //处理的请求参数字节长度
     INT16U             from_seq;     //起始处理位置
     INT16U             req_pos;      //起始的请求报文位置
     INT16U             left_resp_len; //当前可以响应的报文空间
     QUERY_FIRST_CTRL   first_ctrl;  //首次处理标志
}QUERY_CTRL;

/*
//从界面设置测量点参数
typedef union{
      INT8U   value[29];
      struct{
             INT16U       set_count;    //本次配置数量，这里填1
             INT16U       meter_seq;   //配置序号，界面上输入
             INT16U       spot_idx;    //测量点号，界面上输入，默认为电表序号
             BAUD_PORT    baud_port;     //速率及端口
             INT8U        protocol;       //电表协议
             INT8U        meter_no[6];    //电表地址，低字节在前
             INT8U        meter_pass[6];   //电表通信密码，默认为FFFFFFFFFF
             INT8U        meter_fl_count;  //电能表费率个数
             INT8U        meter_frac_info;  //电表示值整数位及小数位个数信息
             INT8U        rtu_no[6];       //所属采集器地址，485电表不需要，载波表可能需要，默认填00
             METER_CLASS meter_class;    //用户大类号，小类号，选择输入。
      };
}DISSET_SPOT_PARAMS;



//以色列扩充数据类型
typedef union{
   INT8U value[85];
   struct{
      INT8U          rec_date[5];
      INT8U          data[80];
   }b;
}C2_F13,C2_F14;

typedef union{
   INT8U value[53];
   struct{
      INT8U          rec_date[5];
      INT8U          data[48];
   }b;
}C2_F15;

//一类数据，F127，电表事件记录状态
typedef union{
   INT8U  value[7];
   struct{
      INT8U          rec_date[5];
      INT8U          data[2];
   }b;
}C1_F127;

//以色列当前数据
typedef union{
   INT8U  value[90];
   struct{
      INT8U          rec_date[5];
      INT8U          zdl[4];
      INT8U          jdl[4];
      INT8U          fdl[4];
      INT8U          pdl[4];
      INT8U          gdl[4];
      INT8U          syje[4];
      INT8U          event_flag[2];
      INT8U          ua[2];
      INT8U          ub[2];
      INT8U          uc[2];
      INT8U          ia[3];
      INT8U          ib[3];
      INT8U          ic[3];
      INT8U           q[2];
      INT8U           f[2];
      INT8U          wg1[20];
      INT8U          wg2[20];
   }b;
}C1_CURDATA;

//以色列无功数据
typedef union{
   INT8U  value[45];
   struct{
      INT8U          rec_date[5];
      INT8U          wg1[20];
      INT8U          wg2[20];
   }b;
}C2_WGDATA;

//巴西小时冻结数据
typedef union{
    INT8U  value[129];
    struct{
        INT8U          rec_date[5];
        INT8U          load_record1[15];
        INT8U          load_record2[15];
        INT8U          load_record3[15];
        INT8U          load_record4[15];
        INT8U load_record5[16];
        INT8U load_record6[16];
        INT8U load_record7[16];
        INT8U load_record8[16];
    };
}HOUR_LOAD_RECORD;

typedef union{
    INT8U  value[258];
    struct{
        INT8U   td[3];
        INT8U   meter_count[2];
        INT8U   vip_count;
        INT8U   fail_count[2];
        INT8U   rec_status[250];
   };
}C2_F55;

typedef union{
    INT8U  value[28];
    struct{
        INT8U   rec_date[5];
        INT8U   v_a[2];
        INT8U   v_b[2];
        INT8U   v_c[2];
        INT8U   i_a[3];
        INT8U   i_b[3];
        INT8U   i_c[3];
        INT8U   zxyg[4];
        INT8U   fxyg[4];
   };
}C2_LOAD_RECORD;

typedef union{
    INT8U  value[23];
    struct{
        INT8U   td[3];
        INT8U   meter_count[10][2];
   };
}C2_F56;

typedef union{
    INT8U value[15];
    struct{
      INT8U td[3];
      INT8U  p_max[2];
      INT8U  p_max_time[3];
      INT8U  p_min[2];
      INT8U  p_min_time[3];
      INT8U  p_zero_time[2];
    };
}C2_F57;

typedef union{
    INT8U value[14];
    struct{
      INT8U td[2];
      INT8U  p_max[2];
      INT8U  p_max_time[3];
      INT8U  p_min[2];
      INT8U  p_min_time[3];
      INT8U  p_zero_time[2];
    };
}C2_F60;


typedef union{
    INT8U  value[9];
    struct{
        INT8U   td[3];
        INT8U   min_csq;
        INT8U   min_time[2];
        INT8U   max_csq;
        INT8U   max_time[2];
   };
}C2_F247;

typedef union{
    INT8U  value[6];
    struct{
        INT8U port;
        INT8U flag;
        INT8U bcd_start_time[3];
        INT8U exec_hour;
    };
}SET_F111; //__PLC_NET_JIANGSU__

//线损计算标志字
typedef union{
    INT8U value;
    struct{
       INT8U  sale_amount:1;   //售电量计算标志  1：已计算
       INT8U  sum_amount:1;    //总电量读取标志：1：已读取
       INT8U  time_seg:1;      //是否在时段内    1： 时段外计算   0：时段内计算
    };
}tagXLOSTCALFLAG;

//块抄标识
typedef union{
    INT8U  value[7];
    struct{
        INT8U   fl_flag:5;          //按位记录支持的费率数,目前最大支持4费率 1为不支持 0为支持
        INT8U   block_flag:3;  //记录是否支持块抄  111为未探测  001为块抄 000为单抄
        INT8U fl_time[6];    //探测次数 8次
    };
}METER_BLOCK_FLAG;

typedef union{
    INT8U value[3];
    struct{
    INT8U   flag;
    INT8U   seq;
    INT8U   port;
    };
}G14_ACK;

typedef union{
    INT8U  value[9];
    struct{
        INT8U create_date[3];    //下发任务的时间
        INT8U task_format[2];    //任务格式
        INT8U task_type;         //任务类型
        INT8U task_data_len[2];
        INT8U exec_type;         //1:部分电表 2：全部电表；
    };
}BATCH_TASK_HEADER_XIAO;

typedef union{
    INT8U  value[7];
    struct{
        INT8U create_date[3];    //下发任务的时间
        INT8U task_format[2];    //任务格式
        INT8U task_type;         //任务类型
        INT8U task_data_len[2];
        INT8U exec_type;         //1:部分电表 2：全部电表；
        INT8U common_er[32];     //通用密文；
        INT8U task_data[256];    //任务数据最大为256
        INT8U meter_count[2];
    };
}BATCH_TASK_HEADER;

typedef union{
    INT8U  value[7];
    struct{
        INT8U task_state;        //0xAA:有任务，未执行；A0：成功；0A:失败； 其他 ：无效
        INT8U task_ctrl[4];      
        INT8U meter_no[8];
        INT8U meter_er[32];      //密文
        INT8U meter_rondon[4];   //电表随机数
        INT8U meter_esam_id[8];  //电表esam序号
    };
}BATCH_TASK_AUTH;    //身份认证

typedef union{
    INT8U  value[7];
    struct{
        INT8U task_state;        //0xAA:有任务，未执行；A0：成功；0A:失败； 其他 ：无效
        INT8U task_ctrl[4];
        INT8U meter_no[8];
        INT8U meter_er[32];      //密文
    };
}BATCH_TASK_TIME;   //对时任务

typedef union{
    INT16U  value;
    struct{
        INT16U  meter_idx:11;
        INT16U  tmp:4;
        INT16U  rec_task_flag:1;
    };
}METER_IDX;

typedef union{
    INT8U  value[6];
    struct{
        METER_IDX meter_idx;
        INT8U time[2];
        INT8U voltage[2];
    };
}VOLTAGE_MONITOR;   //电压监视

typedef union{
    INT8U value;
    struct{
        INT8U type:3;
        INT8U is_huali:2;
        INT8U tmp:2;
        INT8U is_type:1;
    };
}SWITCH_FLAG;

typedef struct{
    INT32U count[2][15];
    INT8U  ptr;
}PULSECOUNTER;

typedef struct{
   INT32U timer;
   INT8U  flag;
   INT8U  meter_no[6];
}COMMPORT485_CAS_TO_CY;

typedef  union{
     INT8U  value[400];
     struct{   
        INT8U save_F1_4[63];
        INT8U save_F16[64];
        INT8U save_F8[8];
        INT8U save_F7[64];
     };
}INIT_DATA;


typedef struct
{
    INT8U *task_info;
    INT16U task_info_len;
    INT8U *resp;
    INT16U resp_len;
    INT8U port; //抄表端口
    INT8U taskid;//紧急执行任务id
    INT8U max_wait_time;//最大等待时间
    PORT_COMM_CTRL port_ctrl;
    BOOLEAN is_delay;
}RECTASK_URGENT_EXEC_OBJECT;


typedef struct
{
    INT8U cur_date[3];          //统计时间
    INT8U day_power_time[4];    //当日供电时间
    INT8U month_power_time[4];  //当月供电时间
}tagAPP_STAT_RUNTIME;
typedef struct
{
    INT8U cur_date[3];          //统计时间
    INT8U day_bytes[4];         //日通信流量
    INT8U month_bytes[4];       //月通信流量
}tagAPP_GPRS_BYTES;
typedef struct{
    INT8U  pulse_count[5][4];
    INT8U  meter_value[5][4];
    INT8U  max_xl[5][3];
    INT8U  max_xl_time[5][4];
}tagPulseMeterSaveData;
typedef struct
{
    INT8U               cur_time[6];//当前系统时间
    tagAPP_GPRS_BYTES   gprs_bytes;  //GPRS流量统计
    tagAPP_STAT_RUNTIME runtime_stat;//运行时间统计
    tagPulseMeterSaveData pulse_meter_save_data[MAX_PULSE_PORT_COUNT];//脉冲表需要保存的数据
}tagAPP_RUN_DATA;

typedef struct
{
    INT8U   max_month_gprs_bytes[4];//月流量限值
}tagAPP_RUN_PARAM;

typedef struct
{
    INT32U phy;
    INT32U item;
    INT16U left_len;
    INT16U meter_seq;
    INT16U flag;//对齐
    INT8U  len;//test
    INT8U  block_prop;//D0~D1 表示块抄单抄类型 00：优先块抄，可尝试单抄  01：固定块抄 02 固定单抄   默认00
    INT8U  *resp;

}READ_PHY_C1;



//总加组相关
typedef struct{
    INT32S agp_p_yg[MAX_ADDGROUP_COUNT][60];
//    INT32S agp_p_wg[MAX_ADDGROUP_COUNT][60];//无功功率目前用不到，只要有一个当前值就可以了，用下面的单个值
    INT32S agp_p_wg[MAX_ADDGROUP_COUNT];
    INT8U  cur_ptr;
}tagAGP_P;

#define BATCH_TRANS_METER_TASK_FLAG_NO_EXEC             0     //有任务,未执行
#define BATCH_TRANS_METER_TASK_FLAG_CANT_EXEC           1     //不能执行
#define BATCH_TRANS_METER_TASK_FLAG_EXEC_NO_RESP        2     //已执行，未响应
#define BATCH_TRANS_METER_TASK_FLAG_FINISH              3     //执行完成
#define BATCH_TRANS_METER_TASK_FLAG_BEFORE_EXCEPTION    4     //前面任务回复异常应答
#define BATCH_TRANS_METER_TASK_FLAG_RESP_LONG           5     //执行完成,回复报文太长
#define BATCH_TRANS_METER_TASK_FLAG_TIME_OUT            6     //执行完成,超时

typedef union{
    INT8U value;
    struct{
        INT8U report:1;
        INT8U wait_read:1;
        INT8U task_end:1;
        INT8U deny:1;
        INT8U node_prior:1;  //__BATCH_TRANSPARENT_METER_TASK_COST_CONTROL__
        INT8U tmp:3;
    };
}BATCH_TRANSPARENT_METER_TASK_FALG;

typedef union{
    INT8U value[4];
    struct{
        INT8U task_id[2];
        INT8U task_level;   //0：保留；1：高于日冻结抄读；2：低于日冻结抄读
        BATCH_TRANSPARENT_METER_TASK_FALG ctrl_flag;  //D0-上报标志位  D1-抄读等待标志位  D2~D7备用
        #ifdef __BATCH_TRANSPARENT_METER_TASK_TMP1__
        INT8U vaild_minute[2];  // 0-不限制有效时长  其它：任务执行有效时长，单位为分钟
        #else
        INT8U vaild_minute;  // 0-不限制有效时长  其它：任务执行有效时长，单位为分钟
        #endif
        INT8U z_count;
        INT8U count;
        INT8U begin_time[6];
        INT8U end_time[6];
        INT8U flag;         //0xAA 有任务
    };
}BATCH_TRANSPARENT_METER_TASK_HEADER;       //__BATCH_TRANSPARENT_METER_TASK__

#define BATCH_TRANSPARENT_METER_TASK_FRAME_LEN         160        //每条报文的长度
#define BATCH_TRANSPARENT_METER_TASK_FRAME_MAX_COUNT   15         //每个测量点缓存报文的最大条数

typedef union{
    INT8U value[4];
    struct{
        INT8U flag;  //FE:有任务；未执行  FC：不能执行  F8：执行完成
        INT8U frame_seq;
        INT8U req_frame_len;
        INT8U req_frame[BATCH_TRANSPARENT_METER_TASK_FRAME_LEN];
        INT8U resp_frame_len;
        INT8U resp_date[6];
        INT8U resp_frame[BATCH_TRANSPARENT_METER_TASK_FRAME_LEN];
    };
}BATCH_TRANSPARENT_METER_TASK;       //__BATCH_TRANSPARENT_METER_TASK__


typedef union{
    INT8U value[10];
    struct{
        INT8U flag;
        INT8U cycle;
        INT8U meter_date[4];
        INT8U meter_error[4];
    };
}PRECISE_TIME_METER_CTRL; //__PRECISE_TIME__

#define MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM                        1
#define BATCH_TRANSPARENT_METER_CYCLE_TASK_FRAME_LEN             200          //每条报文的长度
#define BATCH_TRANSPARENT_METER_CYCLE_TASK_FRAME_MAX_COUNT         8          //每个测量点缓存报文的最大条数
#define BATCH_TRANSPARENT_METER_CYCLE_TASK_RESP_FRAME_MAX_COUNT   10          //每个测量点缓存应答报文的最大条数
typedef union{
    INT8U value[4];
    struct{
        INT8U task_id;  //FE:有任务；未执行  FC：不能执行  F8：执行完成
        INT8U task_flag;
        INT8U begin_time[5];
        INT8U unit;
        INT8U frog;
        INT8U end_time[5];
        INT8U protocol;
        INT8U resp_byte_num;
        INT8U req_frame_len;
    };
}BATCH_TRANSPARENT_METER_CYCLE_TASK_HEADER;        //__BATCH_TRANSPARENT_METER_CYCLE_TASK__

typedef union{
    INT8U value[4];
    struct{
        BATCH_TRANSPARENT_METER_CYCLE_TASK_HEADER header;
        INT8U req_frame[BATCH_TRANSPARENT_METER_CYCLE_TASK_FRAME_LEN];
    };
}BATCH_TRANSPARENT_METER_CYCLE_TASK_CMD;        //__BATCH_TRANSPARENT_METER_CYCLE_TASK__

typedef union{
    INT8U value[4];
    struct{
        INT8U cycle_date[5];
        INT8U resp_frame_len;
        INT8U resp_date[5];
        INT8U resp_frame[BATCH_TRANSPARENT_METER_CYCLE_TASK_FRAME_LEN];
    };
}BATCH_TRANSPARENT_METER_CYCLE_TASK_REPS;       //__BATCH_TRANSPARENT_METER_CYCLE_TASK__
*/


//有功电能示值的整数位及小数位个数：
// 	D0~D1：编码表示通信接入的电能表的有功电能示值的小数位个数，数值范围0~3依次表示1~4位小数。
// 	D2~D3：编码表示通信接入的电能表的有功电能示值的整数位个数，数值范围0~3依次表示4~7位整数。


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


//typedef union
//{
//    INT8U value;
//    struct{
//        INT8U set_address:1;  //设置蓝牙地址
//        INT8U update_version:1;  //需要查询DJB芯片版本
//        INT8U update_ad_data:1;  //需要读取AD采样数据
//        INT8U reset_flag:1;      //需要复位djb芯片
//        INT8U bat_ad_flag:1;     //备用电池采样标志
//        INT8U reserve:3;
//    };
//}tagDJB_TASK_FLAG;
//typedef union
//{
//    INT8U value;
//    struct{
//        INT8U has_battery:1;   //是否有备用电池标志
//        INT8U battery_full:1;  //电池是否充满
//        INT8U battery_supply:1;//是否是备用电池供电状态
//        INT8U reserve:5;
//    };
//}tagBAT_STATE;

typedef struct{
    INT32U item;                //数据项
    INT16U baud;                //波特率
    INT8U  cs_bit;              //校验位
    INT8U  data_bit;            //数据位
    INT8U  stop_bit;            //停止位
    INT8U  recv_frame_time_out; //接收等待报文超时时间
    INT8U  recv_byte_time_out;  //接收等待字节超时时间
}TRANS_READ_PARAM;



extern METER_DOCUMENT  gmeter_doc;
extern INT32S   read_c1_time_out;//数据超时变量
extern tagActiveSendAck gActiveSendAck;//主动上报数据的确认机制
extern INT8U meter_status[14];//电表运行状态字
#endif

