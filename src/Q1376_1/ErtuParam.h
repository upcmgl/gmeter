#include "app_var_3761.h"
#include "app_3761.h"


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
#define SOFT_VER_YEAR  0x19
#define SOFT_VER_MONTH 0x06
#define SOFT_VER_DAY   0x22


#define SOFT_VER_MAIN   '1'
#define SOFT_VER_DOT    '.'
#define SOFT_VER_MIN1   '0'
#define SOFT_VER_MIN2   '2'

#define HARDWARE_VER_H   '1'
#define HARDWARE_VER_DOT '.'
#define HARDWARE_VER_L1  '0'
#define HARDWARE_VER_L2  '1'


#define MAX_TASK_C1       4

#define MAX_TASK_C2       4





//*******************************************************
//
//       终端参数规划
//       主要和AFN=04H关联
//       参数存储的规划为：内容+预留2字节

//       内容：根据参数的内容定义长度
//       预留：根据参数的意义来考虑后续可能的扩展，暂定孔洞大小
//
//       注释说明：字节长度=内容+预留
//
//
//*******************************************************

//终端的基本参数信息，占用4000字节  不可初始化
#define  EEADDR_BASIC_PARAM_START        0
#define  EEADDR_NONE             0xFFFFFF

#define  EEADDR_CFG_DEVID        (EEADDR_BASIC_PARAM_START)     //8=6+2：终端地址(最大6字节)(预留2字节--待定)
  #ifdef __RTUADDR_IS_6BYTE__
    #define  EEADDR_CFG_AREACODE        (EEADDR_CFG_DEVID + 1)  //3：行政区划代码
    #define  EEADDR_CFG_RTUADDR         (EEADDR_CFG_DEVID + 4)  //3：终端地址
  #else
    #define  EEADDR_CFG_AREACODE        (EEADDR_CFG_DEVID + 1)  //2：行政区划代码
    #define  EEADDR_CFG_RTUADDR         (EEADDR_CFG_DEVID + 3)  //2：终端地址
  #endif
#define  EEADDR_CFG_VENDOR          (EEADDR_CFG_DEVID + 8)         //6=4+2：厂商代码
#define  EEADDR_SOFT_VER	       (EEADDR_CFG_VENDOR+6)	//6=4+2 软件版本号
#define  EEADDR_SOFT_DATE	       (EEADDR_SOFT_VER+6)		//5=3+2 软件版本发布日期
#define  EEADDR_CAPACITY	       (EEADDR_SOFT_DATE+5)		//13=11+2 容量信息码
#define  EEADDR_PROTOCOL_VER	       (EEADDR_CAPACITY+13)		//6=4+2 协议版本号
#define  EEADDR_HARDWARE_VER	       (EEADDR_PROTOCOL_VER+6)		//6=4+2 硬件版本号
#define  EEADDR_HARDWARE_DATE	       (EEADDR_HARDWARE_VER+6)		//5=3+2 硬件版本发布日期
#define  GPRS_INFO    ( EEADDR_HARDWARE_DATE+ 5)      //66=64+2,GPRS信息
    #define   GPRS_INFO_VENDOR   (GPRS_INFO +  5)   //20,VENDOR
    #define   GPRS_INFO_VERINFO  (GPRS_INFO + 25)   //20,VER_INFO
    #define   GPRS_INFO_SIM_ID   (GPRS_INFO + 45)   //20,SIM_ID
#define  EEADDR_SIMCARD_TYPE    (GPRS_INFO + 66)               //3=1+2,SIM卡运营商类型
#define  EEADDR_GPRS_IMSI       (EEADDR_SIMCARD_TYPE + 3)      //17=15+2,SIM卡IMSI号
#define  EEADDR_GPRS_VERSION  (EEADDR_GPRS_IMSI + 17)   //31=29+2, 远程模块版本信息及版本类型

#define  EEADDR_SET_F1           (EEADDR_GPRS_VERSION+31)      //8=6+2：F1-终端通信参数
    #define  EEADDR_HEARTCYCLE        (EEADDR_SET_F1 + 5)       //1：心跳周期
#define  EEADDR_SET_F3           (EEADDR_SET_F1 + sizeof(tagSET_F1))           //50=44+6：F3-主站IP地址和端口(预留6字节--1个IP地址+端口)
    #define  EEADDR_APN               (EEADDR_SET_F3 + 50)      //34=32+2：APN  1376.1中APN是16字节，特殊处理
#define  EEADDR_SET_F7           (EEADDR_SET_F3 + sizeof(tagSET_F3))           //70=64+6：F7--终端IP地址和端口(预留6字节--1个IP地址+端口)
#define  EEADDR_SET_F8           (EEADDR_SET_F7 + sizeof(tagSET_F7))           //10=8+2：F8-终端上行通信方式(以太专网或虚拟专网)(预留2字节--待定)
#define  EEADDR_SET_F16          (EEADDR_SET_F8 + sizeof(tagSET_F8))          //66=64+2：F16--虚拟专网用户名、密码(预留2字节--待定)
#define  EEADDR_SET_F221         (EEADDR_SET_F16 + 66)         //3=1+2：F221-短信功能开启与禁用(预留2字节--待定)
#define  EEADDR_SET_F33          (EEADDR_SET_F221 + 3)         //220=110*2：F33-终端抄表运行参数(预留2字节--待定)，支持2个端口
#define  EEADDR_SET_F301         (EEADDR_SET_F33 + 220)         //267=1+264+2：F301-域名登录参数(预留2字节--待定)
#define  EEADDR_MES              (EEADDR_SET_F301+ 267)         //34 = 32+2 :MES码
#define  EEADDR_POWEROFF_RECORD  (EEADDR_MES+34)                //14 终端停上电记录
#define  EEADDR_METER_SETFLAG    (EEADDR_POWEROFF_RECORD+14)     //1 电表参数是否允许设置，0x55:允许设置，其他:不允许
#define  EEADDR_CONFIG_ONENET    (EEADDR_METER_SETFLAG+1)       //1+2 onenet上线参数  00或FF 公网 其他 专网
#define  EEADDR_SET_F701         (EEADDR_CONFIG_ONENET+3)       //4+1 电表密码 程序内部默认为12345602 使用默认密码时不需要设置
#define  EEADDR_SET_F702         (EEADDR_SET_F701+5)            //5+1 电表预置拉合闸时间参数 拉合闸 有效标志1字节+起始结束时间 4字节
#define  EEADDR_SWITCH_PARAM     (EEADDR_SET_F702+6)            //4+4+1 默认参数开关切换 
                                                                //D0：冀北环保645协议 特殊登陆心跳帧

#define EEADDR_BASIC_PARAM_CUR_END   (EEADDR_SET_F702 + 6)
#define EEADDR_BASIC_PARAM_END   8000UL

//#if  (EEADDR_BASIC_PARAM_CUR_END > EEADDR_BASIC_PARAM_END)
//    #error "EEADDR_BASIC_PARAM IS OVERFLOW"
//#endif





/************************************
设置终端参数   从8000UL开始 可初始化
*************************************/
#define  EEADDR_PARAM_INIT_START  (EEADDR_BASIC_PARAM_END)
//考虑到上述的参数文件的规划尚未完全，下列的和设置AFN=04H有关的，先另起存储地址
#define  EEADDR_SET_F4           (EEADDR_PARAM_INIT_START)           //24=16+8：F4-主站电话号码和短信中心号码(预留8字节--1个电话号码)
#define  EEADDR_SET_F9           (EEADDR_SET_F4 + sizeof(tagSET_F4))           //34=32+2：F9-终端事件记录配置(预留2字节--8个事件)
#define  EEADDR_SET_F10          (EEADDR_SET_F9 + sizeof(tagSET_F9))
#define  EEADDR_SET_F36          (EEADDR_SET_F10 + sizeof(METER_DOCUMENT))      //6=4+2：F36-终端上行通信流量门限(预留2字节--待定)
#define  EEADDR_FRAME_MONITOR    (EEADDR_SET_F36 + 6)         //2+2，报文监控开关 D0-gprs报文调试 D1-电表通信报文调试开关 D2-gprs的应用层报文调试开关
                                                                // D3-调试接口选择，0为debug串口输出，其他值为电表通信口输出 D4 时钟状态，0终端未取到时钟不能连接主站， D5~D15备用
#define     EEADDR_TASK_REPORT_CYCLE    (EEADDR_FRAME_MONITOR+4)     //4+2 4字节BCD                         
#define     EEADDR_TASK_REPORT      (EEADDR_TASK_REPORT_CYCLE+6)     //36+20 任务上报参数 时间
    #define TASK_REPORT_MONTHHOLD_TIME     0    
    #define TASK_REPORT_DAYHOLD_TIME       TASK_REPORT_MONTHHOLD_TIME+sizeof(tagDatetime)
    #define TASK_REPORT_CURDATA_TIME       TASK_REPORT_DAYHOLD_TIME+sizeof(tagDatetime)
    #define TASK_REPORT_REPORT_FLAG        TASK_REPORT_CURDATA_TIME+sizeof(tagDatetime)
    #define TASK_REPORT_IDX                TASK_REPORT_REPORT_FLAG+2
    #define TASK_REPORT_ACK_IDX            TASK_REPORT_IDX+2
	#define     EEADDR_STAT_FLOW        (EEADDR_TASK_REPORT+56)        //11+2 数据流量统计
#ifdef __ONENET__
	#define     EEADDR_LAST_STATUS      (EEADDR_STAT_FLOW+13)          //28=20状态字+6时间+2 上一次状态字   每次变化后更新
	#define     EEADDR_LAST_ENENT_TIMES (EEADDR_LAST_STATUS+28)        //80=4*20  11个预留9 根据instance顺序计算偏移地址默认 如果instance变化需要注意偏移地址数据的初始化
#endif
#ifdef __EDP__

//#ifdef __ZHEJIANG_ONENET_EDP__  //存储了浙江事件的信息。

//#define     EEADDR_STAT_FLOW        (EEADDR_TASK_REPORT+56)        //11+2 数据流量统计
#define     EEADDR_LAST_STATUS      (EEADDR_STAT_FLOW+13)          //28=20状态字+6时间+2 上一次状态字   每次变化后更新
#define     EEADDR_LAST_ENENT_TIMES (EEADDR_LAST_STATUS+28)        //80=4*20  11个预留9 根据instance顺序计算偏移地址默认 如果instance变化需要注意偏移地址数据的初始化
//#endif

#define     EEADDR_TIME_INTERVAL   (EEADDR_LAST_ENENT_TIMES+80)  //2 修改设备采集数据的时间间隔 默认3分钟
#define     EEADDR_UPDATE_FLAG     (EEADDR_TIME_INTERVAL+2)  //1  启动升级flag 0xAA 开始 0xBB 正在升级 0xCC升级完成
#define     EEADDR_NEW_VERSION  (EEADDR_UPDATE_FLAG+1)  //11  tops4+新版本
#define     EEADDR_OLD_VERSION     (EEADDR_NEW_VERSION+11)  //11 tops4+老版本
#define     EEADDR_UPDATE_SUCESS   (EEADDR_OLD_VERSION+11)  //1  升级成功标志 0
#define     EEADDR_IP_PORT_ONENET       (EEADDR_UPDATE_SUCESS+1)  //4+2 onenet IP地址和端口
#define     EEADDR_IP_PORT_UPDATE       (EEADDR_IP_PORT_ONENET+6)  //4+2 模块升级跳转 IP地址和端口
#define     EEADDR_OVERFLOW_WARNING_A     (EEADDR_IP_PORT_UPDATE +6)  //4字节，过流告警次数存储 A相。
#define     EEADDR_OVERFLOW_WARNING_B     (EEADDR_OVERFLOW_WARNING_A +4)  //4字节，过流告警次数存储 B相。
#define     EEADDR_OVERFLOW_WARNING_C     (EEADDR_OVERFLOW_WARNING_B +4)  //4字节，过流告警次数存储 C相。
#define     EEADDR_OPENMETER_WARNING    (EEADDR_OVERFLOW_WARNING_C +4) //4字节 开表盖告警次数存储
#define     EEADDR_CLEANNMETER_WARNING    (EEADDR_OPENMETER_WARNING +4) //4字节 清零告警次数存储
#define     EEADDR_EDP_RAND_BEGIN         (EEADDR_CLEANNMETER_WARNING+4) // 1字节，用于设置随机延后时间范围的开始时间， 可能的设置时间段为10，20中的 10 ;  /*代表随机延后的起始时间为10分钟，结束时间20分钟*/ 
#define     EEADDR_EDP_RAND_END            (EEADDR_EDP_RAND_BEGIN + 1)   // 1字节，用于设置随机延后时间范围的结束时间， 可能的设置时间段为10，20中的20；
#define     EEADDR_UPDATA_INFO          (EEADDR_EDP_RAND_END+1)     //sizeof(struct edp_updata_info)  字节，用于保存平台下发的升级版本。比如20191007代表版本号20191007版本。



#define     EEADDR_REPORT_UNSUCESS  (EEADDR_UPDATE_SUCESS +1) //sizeof(struct strReportUnsuceed) 存储未成功上报数据索引
#endif

//#define   EEADDR_SET_F97          (EEADDR_SET_F36 + 6)           //27=25+2：F97-停电数据采集配置参数
//#define   EEADDR_SET_F98          (EEADDR_SET_F97 + 27)          //15=13+2：F98-停电事件甄别限值参数
//#define   EEADDR_SET_F67          (EEADDR_SET_F66 + 65856)       //6=4+2：F67-定时上报I类数据任务启动/停止设置(预留2字节--待定)
//#define   EEADDR_SET_F68          (EEADDR_SET_F67 + 64)          //6=4+2：F68-定时上报II类数据任务启动/停止设置(预留2字节--待定)
//#define   EEADDR_C1_F7    (PIM_C1_F5 + 2)         //2,  终端事件计数器当前值
//#define   EEADDR_C1_F8    (PIM_C1_F7 + 2)         //8,  终端事件标志状态
//#define   EEADDR_C1_F10   (ABANDON_PIM_C1_F9 + 2)         //8,  终端当日(零点开始),当月(每月1日零点开始)通信流量,  分钟保存
//#define   EEADDR_DAY_RST_COUNT       (PIM_ERTU_XLOST + 450)               //SAVE_POINT_NUMBER_DAY_HOLD*7, 终端日复位次数
//#define   EEADDR_DAY_BYTES           (FLADDR_DAY_RST_COUNT + SAVE_POINT_NUMBER_DAY_HOLD*7)        //SAVE_POINT_NUMBER_DAY_HOLD*7, 终端日流量统计
//#define   EEADDR_DAY_POWER_TIME  (FLADDR_DAY_BYTES + SAVE_POINT_NUMBER_DAY_HOLD*7)            //SAVE_POINT_NUMBER_DAY_HOLD*7, 终端日供电次数
//#define   EEADDR_MONTH_RST_COUNT   (FLADDR_DAY_POWER_TIME + SAVE_POINT_NUMBER_DAY_HOLD*7)        //SAVE_POINT_NUMBER_MONTH_HOLD*6, 终端月复位次数
//#define   EEADDR_MONTH_BYTES   (FLADDR_MONTH_RST_COUNT + SAVE_POINT_NUMBER_MONTH_HOLD*6)        //SAVE_POINT_NUMBER_MONTH_HOLD*6, 终端月流量统计
//#define   EEADDR_MONTH_POWER_TIME  (FLADDR_MONTH_BYTES + SAVE_POINT_NUMBER_MONTH_HOLD*6)            //SAVE_POINT_NUMBER_MONTH_HOLD*6, 终端月供电次数
//#define   EEADDR_SET_F65          (EEADDR_SET_F98 + 15)        //4116=1029*4：F65-定时上报I类数据任务设置
//#define   EEADDR_SET_F66          (EEADDR_SET_F65 + )   //4116=1029*4：F66-定时上报II类数据任务设置
//#define   MAX_METER_EVENT_RECORD_COUNT         512*50 ？？            //电表事件记录    预留 暂不确定

#ifdef __HJ_212__
#define  EEADDR_HJ212_PASSWD      (EEADDR_STAT_FLOW+13)     //6个字节。
#endif
#define EEADDR_INIT_CUR_END                  (EEADDR_LAST_ENENT_TIMES+80)
#define EEADDR_UNCS_PARAM_END                23999UL
//#if EEADDR_UNCS_PARAM_CUR_END > EEADDR_UNCS_PARAM_END
//    #error "EEADDR_UNCS_PARAM IS OVERFLOW"
//#endif

///////////////////////////////////////////////////////////////////////////////////////////
//
//         电表数据规划
//
//         普通单相电表,最多可以是4费率单相电能表
//
//         保存 93日的日冻结数据    12月的月冻结数据
//
//////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------测量点档案及参数-----------------------
//  测量点档案
#define  PIM_METER_DOC                0        //27,电表配置信息
#define        PIM_METER_BAUDPORT     4
#define        PIM_METER_PROTOCOL     5
#define        PIM_METER_NO           6
#define        PIM_METER_RTU         20
#define        PIM_METER_USERTYPE    26
#define  PIM_REC_FLAG               (PIM_READ_MASK + sizeof(READ_MASK))        //1,  F30:停抄/投抄设置
#define  PIM_AUX_NODES              (PIM_REC_FLAG + 1)          //13, F31:载波从节点附属节点地址,按最多20个附属节点设计,20140411改为最大2个附属节点


BOOLEAN  GetParamAddrLen(INT16U fn,INT32U *addr,INT16U *len);
INT8U  check_curve_fn_list(INT8U fn);
INT8U  fwrite_ertu_params(INT32U addr,INT8U *data,INT16U len);
INT8U  fread_ertu_params(INT32U addr,INT8U *data,INT16U len);
INT8U  fwrite_meter_params(INT16U meter_idx,INT32U addr,INT8U *data,INT16U len);
INT8U  fread_meter_params(INT16U meter_idx,INT32U addr,INT8U *data,INT16U len);
INT8U  fwrite_meter_data(INT16U meter_idx,INT32U offset,INT8U *data,INT16U len);
INT8U  fread_meter_data(INT16U meter_idx,INT32U offset,INT8U *data,INT16U len);

extern const INT8U ertu_cfg_verinfo[LEN_ERTU_CFG_VERINFO];

extern const INT8U  update_watermark[8];

INT8U check_device_code(DATHEADER *header);
INT16U compute_readcfg_fn_datalen(INT16U fn);
void readcfg_fn_data(INT16U fn,INT8U *resp);
INT8U  set_ertu_params(INT16U fn,INT8U *itemData,INT16U *itemDataLen);
INT16U set_custom_param(INT16U paramcode,INT8U *data);
INT16U set_ertu_custom_param(INT8U *data,INT16U *itemDataLen);
void query_ertu_custom_param(INT8U *buffer,INT16U *itemDataLen,INT8U *query_ctrl);
INT16U read_custom_param(INT16U paramcode,INT8U *data);    
INT16U app_get_F249_data(INT8U* resp);