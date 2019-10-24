#ifndef __DEV_CONST_H__
#define __DEV_CONST_H__

//#define __HEBEI_HUANBAO__  //河北环保局 支持645上行 及特殊心跳帧
//#define __MX_UPDATE__
//#define __CTRL_METER__  预置参数跳合闸
//#define __NO_BATTERY__ //电池电压 单相4G无超级电容
//#define __METER_SIMPLE__
//#define  __BAT_PWM__

#ifdef __OOP_G_METER__
#define MAX_FRAME_HEADER_SIZE  14
#else
#define MAX_FRAME_HEADER_SIZE  10
#endif

#define MAX_REMOTE_DELAY_10MS  2000
//模块型号定义
#define  REMOTE_MODEL_TYPE_UNKNOWN 0
#define  REMOTE_MODEL_TYPE_EC20    1
#define  REMOTE_MODEL_TYPE_EC21    2
#define  REMOTE_MODEL_TYPE_EC25    3
#define  REMOTE_MODEL_TYPE_A9500   4
#define  REMOTE_MODEL_TYPE_A9600   5
#define  REMOTE_MODEL_TYPE_M5310   6


//模块厂商定义
#define  REMOTE_MODEL_VENDOR_UNKNOWN   0
#define  REMOTE_MODEL_VENDOR_QUECTEL   1
#define  REMOTE_MODEL_VENDOR_LONGSHANG 2
#define  REMOTE_MODEL_VENDOR_CMI       3    //China Mobile Internet


#define ONENET_READ    1
#define ONENET_WRITE   2
#define ONENET_EXECUTE 3
#define ONENET_NOTIFY  4
#define ONENET_EVENT_ID 5

#define STRING_DATA      0x0100
#define OPAQUE_DATA      0x0200
#define INTERGER_DATA    0x0300
#define FLOAT_DATA       0x0400
#define BOOL_DATA        0x0500
#define HEX_STR_DATA     0x0600

#define KWH_FLOAT    1
#define KWH_STRING   2
#define KVarH_STRING 3
#define V_STRING     4
#define V_FLOAT      5
#define A_STRING     6
#define A_FLOAT      7
#define P_FLOAT      8
#define KW_STRING    9
#define KVar_STRING  10
#define FACTOR_FLOAT 11

//扩展的3332转换函数
#define METER_INDEX    12   //电能表资产编号
#define METER_TYPE     13   //电能表类型
#define MODULE_IMEI    14   //IMEI
#define MODULE_ICCID   15   //ICCID
#define MODULE_IMSI   16   //IMSI
#define MODULE_CSQ     17   //信号强度等级
#define METER_ADDR     18   //电表通信地址
#define NOTIFY_CYCLE    19   //主动上报周期
#define SOFT_VERSION   20   //厂家软件版本号
#define HARD_VERSION   21   //厂家硬件版本号
#define VENDER_NO       22   //厂家编号
#define ONENET_TIME    23   //日期、星期（其中0代表星期天）及时间
#define POWER_SUPPLY   24   //当前供电状态
#define DATA_FLOW       25   // 数据流量
#define REMOTE_UPDATE   26  //远程升级 
#define SZDC_QY         27 //时钟电池欠压
#define SD_TSD          28 //市电停上电
#define FZDY_TSD        29 //辅助电源停上电
#define YGGL_FX         30 //有功功率方向
#define WGGL_FX         31 //无功功率方向
#define A_GL            32 //A相过流
#define B_GL            33 //B相过流
#define C_GL            34 //C相过流
#define A_DX            35 //A相断相
#define B_DX            36 //B相断相
#define C_DX            37 //C相断相
#define KWH_FLOAT_MINUS 38 //带符号位电能示值（组合有功、组合无功）
#define RSRP            39 //RSRP
#define SINR            40 //信噪比
#define eNODEB          41 //小区eNodeB cell id前四位
#define CELL_ID         42 //cell id 
#define KWH_BCD2STRING  43 //电能示值BCD转STRING
#define V_BCD2STRING    44 //电压BCD转STRING
#define I_BCD2STRING    45 //电流BCD转STRING
#define P_BCD2STRING    46 //功率BCD转STRING
#define F_BCD2STRING    47 //功率因数BCD转STRING



#define ONENET_VOLTAGE          3316
#define ONENET_CURRENT          3317
#define ONENET_POWER            3328
#define ONENET_POWER_FACTOR     3329
#define ONENET_ENERGY           3331
#define ONENET_METER_INFO       3332
#define ONENET_METER_TIME       3333
#define ONENET_EVENT            3200

//定义resource ID 
#define ONENET_SENSOR_VALUE     5700
#define ONENET_UNITS            5701
#define ONENET_RESOURCE_APP     5500


#define DAYHOLD_DATA    0x1000 
#define CYCLE_DATA      0x2000
#define MONTHHOLD_DATA  0x4000 


#define READ_METER_SUCCESS  0x00000001 //电表通信成功
#define CGATT_SUCCESS  0x00000002 //模块注网成功
#define LOGIN_SUCCESS  0x00000004 //已登录
#endif