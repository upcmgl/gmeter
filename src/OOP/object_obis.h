#ifndef __PROTOCOL_OBJECT_OBIS_H__
#define __PROTOCOL_OBJECT_OBIS_H__

#define CLASS_EVENT              7      //事件接口类
#define CLASS_INPUT_OUTPUT      22      //输入输出设备接口类
#define CLASS_HOLD               9      //冻结数据接口类
#define CLASS_SUB_EVENT         24      //分项事件接口类
#define CLASS_STAT               14      //区间统计接口
#define CLASS_AVG                15      //累加平均接口类
#define CLASS_MAX_MIN            16      //极值工具接口类

#define OBIS_CUSTOM_INFO        0xFFFD    //厂家自定义信息
#define OBIS_DEBUG_INFO         0xFFFE    //调试信息



#define OBIS_EVENT_CONFIG       0x9001    //事件上报
#define OBIS_DC_PARAM           0x9210    //直流模拟量参数
#define OBIS_SWITCH             0x9211    //状态量输入参数
#define OBIS_RECTIME            0x9221    //抄表时段参数

//参变量类对象
#define OBIS_DATETIME           0x4000    //电能表日期时间
#define OBIS_COMM_ADDR          0x4001    //通信地址
#define OBIS_SERVER_ADDR        0x202A    //目标服务器地址
#define OBIS_METER_NO           0x4002    //表号
#define OBIS_CUST_NO            0x4003    //客户编号
#define OBIS_GPS_SEAT           0x4004    //设备地理位置
#define OBIS_GROUP_ADDR         0x4005    //组地址
#define OBIS_CLOCK              0x4006    //时钟源
#define OBIS_LCD                0x4007    //LCD
#define OBIS_TIMEZONE_SWITCH    0x4008  //两套时区表切换时间
#define OBIS_TIMESEG_SWITCH     0x4009  //两套时段表切换时间
#define OBIS_FL_PRICE_SWITCH    0x400A  //两套分时费率切换时间
#define OBIS_TIMEZONE_SEG_NUM      0x400C   //时区时段数
#define OBIS_HOLIDAY_TABLE         0x4011   //公共假日表
#define OBIS_WEEKEND_FLAG          0x4012   //周休日特征字
#define OBIS_WEEKEND_TIMESEG       0x4013   //周休日?用的日时段表号
#define OBIS_CUR_TIMEZONE_TABLE 0x4014	  //当前套日时区表
#define OBIS_BAK_TIMEZONE_TABLE 0x4015	  //备用套日时区表
#define OBIS_CUR_FL_TABLE       0x4016	  //当前套日时段表
#define OBIS_BAK_FL_TABLE       0x4017	  //备用套日时段表
#define OBIS_CUR_FL_PRICE       0x4018	  //当前套费率电价
#define OBIS_BAK_FL_PRICE       0x4019	  //备用套费率电价
#define OBIS_CUR_JT_PRICE       0x401A      //当前套阶梯电价
#define OBIS_BAK_JT_PRICE       0x401B      //备用套阶梯电价
#define OBIS_EXCLUDED           0x4024    //剔除
#define OBIS_CJQ_UPDATE_RESULT  0x4025    //采集器升级结果
#define OBIS_V_RATE             0x4030    //电压合格率
#define OBIS_ASSET_NO           0x4103    //资产管理编码
#define OBIS_YGZH_FLAG          0x4112      //有功组合特征字
#define OBIS_WGZH_1             0x4113      //无功组合特征字1
#define OBIS_WGZH_2             0x4114      //无功组合特征字2
#define OBIS_SETTLEMENT_DAY     0x4116    //结算日
#define OBIS_METER_TYPE         0x410B      //电表型号
#define OBIS_ROUTER             0x4200    //路由表
#define OBIS_ROUTER_INFO        0x4201    //路由信息单元

#define OBIS_CASCADE            0x4202    //级联通讯数据
//#define OBIS_COMM_STATE         0x4203    //终端上行通信状态参数
#define OBIS_BROADCAST_TIME     0x4204    //终端广播校时时间


#define OBIS_DEV_JZQ            0x4300    //集中器
#define OBIS_DEV_ENERGY         0x4306    //电能表
#define OBIS_DEV_WATER          0x4307    //水表
#define OBIS_DEV_GAS            0x4308    //气表
#define OBIS_DEV_HEAT           0x4309    //热表

#define OBIS_LINK               0x4400    //应用连接
#define OBIS_LINK_PASS          0x4401    //身份认证的口令

#define OBIS_GPRS1               0x4500    //gprs1
#define OBIS_GPRS2               0x4501    //gprs2
#define OBIS_NET1               0x4510    //NET1
#define OBIS_NET2               0x4511    //NET2
#define OBIS_NET3               0x4512    //NET3
#define OBIS_NET4               0x4513    //NET4
#define OBIS_NET5               0x4514    //NET5
#define OBIS_NET6               0x4515    //NET6
#define OBIS_NET7               0x4516    //NET7
#define OBIS_NET8               0x4517    //NET8


//采集监控类对象
#define OBIS_METER_TABLE        0x6000      //采集档案配置表
#define OBIS_METER_INFO         0x6001      //采集档案配置单元
#define OBIS_SEARCH_METER_TABLE 0x6002      //搜表对象
#define OBIS_SEARCH_METER_INFO  0x6003      //搜表信息
#define OBIS_MAINNODE_CHANGE    0x6004      //一个跨台区结果
#define OBIS_DATA_PROP_TABLE    0x6010      //对象属性数据描述符集合
#define OBIS_DATA_PROP          0x6011      //数据属性描述
#define OBIS_TASK_TABLE         0x6012      //任务配置表
#define OBIS_TASK               0x6013      //任务配置信息
#define OBIS_NORMAL_TASK_TABLE  0x6014      //普通采集方案集
#define OBIS_NORMAL_TASK        0x6015      //普通采集方案
#define OBIS_EVENT_TASK_TABLE   0x6016      //事件采集方案集
#define OBIS_EVENT_TASK         0x6017      //事件采集方案
#define OBIS_RELAY_TASK_TABLE   0x6018      //透明方案集
#define OBIS_RELAY_TASK         0x6019      //透明方案
#define OBIS_RELAY_RESULT_TABLE 0x601A      //透明方案结果集
#define OBIS_RELAY_RESULT       0x601B      //透明方案结果
#define OBIS_REPORT_TASK_TABLE  0x601C      //上报方案集
#define OBIS_REPORT_TASK        0x601D      //上报方案
#define OBIS_RULE_TABLE         0x601E      //采集规则库
#define OBIS_RULE               0x601F      //采集规则
#define OBIS_EXTEND_RULE_TABLE  0x6036      //扩展采集规则库
#define OBIS_EXTEND_RULE        0x6037      //扩展采集规则单元



#define OBIS_EVENT_COLUMN_PROP_TABLE    0x6020      //事件属性库
#define OBIS_EVENT_COLUMN_PROP          0x6021      //事件属性

#define OBIS_ACQ_STATUS_TABLE          0x6032      //采集状态表
#define OBIS_ACQ_STATUS                0x6033      //采集状态

#define OBIS_ACQ_MONITOR_TABLE          0x6034      //采集任务监控集
#define OBIS_ACQ_MONITOR                0x6035      //采集任务监控单元

#define OBIS_ACQ_BEGIN_TD               0x6040      //采集启动时标
#define OBIS_ACQ_END_TD                 0x6041      //采集成功时标
#define OBIS_ACQ_SAVE_TD                0x6042      //采集存储时标

//集合类对象
#define OBIS_FILE_TABLE             0x7000    //文件集合
#define OBIS_FILE                   0x7001    //文件
#define OBIS_SCRIPT_TABLE           0x7010    //脚本集合
#define OBIS_SCRIPT                 0x7011    //脚本
#define OBIS_SCRIPT_RESULT_TABLE    0x7012    //脚本结果集合
#define OBIS_SCRIPT_RESULT          0x7013    //脚本结果
#define OBIS_EXT_VAR_TABLE          0x7100    //扩展变量对象集合
#define OBIS_EXT_PARAM_TABLE        0x7101    //扩展参变量对象集合


//文件传输类对象
#define OBIS_SLICING_TRANSMIT   0xF000    //分帧传输
#define OBIS_BLOCK_TRANSMIT     0xF001    //分块传输
#define OBIS_EXTEND_TRANSMIT    0xF002    //扩展传输

//安全类对象
#define OBIS_ESAM           0xF100    //ESAM
#define OBIS_ESAM_PARAM     0xF101    //ESAM

//输入输出设备类对象
#define OBIS_IO_RS232           0xF200    //串口
#define OBIS_IO_RS485           0xF201    //RS485
#define OBIS_IO_INFRA           0xF202    //红外
#define OBIS_IO_INPUT           0xF203    //开关量输入
#define OBIS_IO_DC              0xF204    //直流模拟量
#define OBIS_IO_OUTPUT          0xF205    //继电器输出
#define OBIS_IO_ALAERT          0xF206    //告警输出
#define OBIS_IO_FUNC            0xF207    //多功能端子
#define OBIS_IO_AC              0xF208    //交流采样
#define OBIS_IO_PLC             0xF209    //载波/微功率无线接口
#define OBIS_IO_PULSE           0xF20A    //脉冲输入设备
#define OBIS_IO_BLUETOOTH       0xF20B    //蓝牙设备
#define OBIS_IO_230M            0xF20C    //230M无线专网
#define OBIS_SLAVEUNIT           0xF300  //自动轮显

#define OBIS_MTR_EVT_LOST_V         0x3000  //电能表失压事件
#define OBIS_MTR_EVT_UNDERVOLTAGE   0x3001  //电能表欠压事件
#define OBIS_MTR_EVT_OVERVOLTAGE    0x3002  //电能表过压事件
#define OBIS_MTR_EVT_SEVER_PHASE    0x3003  //电能表断相事件
#define OBIS_MTR_EVT_POWERONOFF     0x3011  //电能表掉电事件
#define OBIS_EVT_COMMODULTE_CHANGE    0x3030   //通信模块变更事件

#define OBIS_EVT_INIT               0x3100  //终端初始化事件
#define OBIS_EVT_VER_CHANGE         0x3101  //终端版本变更事件
#define OBIS_EVT_STATUS_CHANGE      0x3104  //终端状态量变位事件
#define OBIS_EVT_METER_CLOCK        0x3105  //电能表时钟超差事件
#define OBIS_EVT_POWER_ONOFF        0x3106  //终端停/上电事件
#define OBIS_EVT_ESAM_ERROR         0x3109  //终端消息认证错误事件
#define OBIS_EVT_DEV_ERROR          0x310A  //终端故障记录
#define OBIS_EVT_AMOUNT_DOWN        0x310B  //电能表示度下降事件
#define OBIS_EVT_AMOUNT_OVER        0x310C  //电能量超差事件
#define OBIS_EVT_AMOUNT_FLY         0x310D  //电能表飞走事件
#define OBIS_EVT_AMOUNT_STOP        0x310E  //电能表停走事件
#define OBIS_EVT_READMETER_FAIL     0x310F  //终端抄表失败事件
#define OBIS_EVT_FLOW_OVER          0x3110  //月通信流量超限事件
#define OBIS_EVT_NEW_METER          0x3111  //发现未知电能表事件
#define OBIS_EVT_MAINNODE_CHANGE    0x3112  //跨台区电能表事件
#define OBIS_EVT_SET_CLOCK          0x3114  //终端对时事件
#define OBIS_EVT_REMOTE_CTRL        0x3115  //遥控跳闸记录
#define OBIS_EVT_CONTROL_LINE       0x3117  //输出回路接入状态变位事件记录
#define OBIS_EVT_PROGRAM            0x3118  //终端编程记录
#define OBIS_EVT_I_EXCEPT           0x3119  //终端电流回路异常事件
#define OBIS_EVT_METER_STATUS_CHG   0x311A  //电能表在网状态切换事件
#define OBIS_EVT_METER_SET_CLOCK    0x311B  //终端对电表校时记录
#define OBIS_EVT_METER_MONITOR      0x311C  //电能表数据变更监控记录

#define OBIS_EVT_POWER_CTRL         0x3200  //功控跳闸记录
#define OBIS_EVT_AMOUNT_CTRL        0x3201  //电控跳闸记录
#define OBIS_EVT_BUYFEE             0x3202  //购电参数设置记录
#define OBIS_EVT_AMOUNT_ALRAM       0x3203  //电控告警事件记录

#define OBIS_UNIT_NORAML             0x3301  //标准时间记录单元
#define OBIS_UNIT_PROGRAM            0x3302  //编程记录事件单元
#define OBIS_UNIT_NEW_METER          0x3303  //发现未知电表事件单元
#define OBIS_UNIT_MAINNODE_CHANGE    0x3304  //垮台区电能表事件单元
#define OBIS_UNIT_POWER_CTRL         0x3305  //功控跳闸记录单元
#define OBIS_UNIT_AMOUNT_CTRL        0x3306  //电控跳闸记录单元
#define OBIS_UNIT_AMOUNT_ALRAM       0x3307  //电控告警事件单元
#define OBIS_UNIT_XL_OVER            0x3308  //电能表需量超限事件单元
#define OBIS_UNIT_POWER_ONOFF        0x3309  //终端停上电记录单元
#define OBIS_UNIT_REMOTE_CTRL        0x330A  //遥控事件记录单元
#define OBIS_UNIT_YGAMOUNT_OVER      0x330B  //有功总电能量差动越限事件记录单元
#define OBIS_UNIT_EVENT_RESET        0x330C  //事件清零事件记录单元
#define OBIS_UNIT_METER_SETCLOCK     0x330D  //终端对电表校时记录单元
#define OBIS_UNIT_METER_STATUS_CHG   0x330E  //电能表在网状态切换事件单元
#define OBIS_UNIT_METER_MONITOR      0x330F  //电能表数据变更监控记录单元
#define OBIS_UNIT_COMMODULTE_CHANGE   0x3312  //通信模块变更事件单元

#define OBIS_UNIT_METER_CLOCK        0x3313  //电能表时钟超差事件记录单元

#define OBIS_NETFLOW            0x2200     //通信流量
#define OBIS_POWERTIME          0x2203     //供电时间
#define OBIS_RESETCOUNT         0x2204     //复位次数

#define OBIS_AMOUNT_ZXYG        0x0010    //正向有功电能
#define OBIS_AMOUNT_FXYG        0x0020    //反向有功电能
#define OBIS_AMOUNT_ZXWG        0x0030    //正向无功电能
#define OBIS_AMOUNT_FXWG        0x0040    //反向无功电能
#define OBIS_VOLTAGE            0x2000    //电压
#define OBIS_ELECTRICITY        0x2001    //电流
#define OBIS_V_ANGLE            0x2002    //电压相角
#define OBIS_VI_ANGLE           0x2003    //电压电流相角
#define OBIS_ACTIVE_POWER       0x2004    //有功功率
#define OBIS_REACTIVE_POWER     0x2005    //无功功率
#define OBIS_APPARENT_POWER     0x2006    //视在功率
#define OBIS_POWER_FACTOR       0x200A    //功率因数
#define OBIS_1XX_WG             0x0050    //一象限无功
#define OBIS_2XX_WG             0x0060    //二象限无功
#define OBIS_3XX_WG             0x0070    //三象限无功
#define OBIS_4XX_WG             0x0080    //四象限无功
#define OBIS_ZXYG_ZDXL          0x1010    //正向有功最大需量
#define OBIS_FXYG_ZDXL          0x1020    //反向有功最大需量
#define OBIS_ZXWG_ZDXL          0x1030    //组合无功1最大需量
#define OBIS_FXWG_ZDXL          0x1040    //组合无功2最大需量
#define OBIS_ClOCK_BATTERY_V    0x2011      //时钟电池电压
#define OBIS_ClOCK_BATTERY_T    0x2013      //时钟电池工作时间
#define OBIS_METER_STATE        0x2014    //电能表状态字

#define OBIS_CUR_YGXL           0x2017      //当前有功需量
#define OBIS_CUR_WGXL           0x2018      //当前无供需量
#define OBIS_CUR_SZXL           0x2019      //当前视在需量
#define OBIS_V_UNBALANCE        0x2026      //电压不平衡
#define OBIS_I_UNBALANCE        0x2027      //电流不平衡
#define OBIS_AMPERE_VALUE       0x2029      //安时值

#define OBIS_METER_V_RATE_A     0x2131    //电能A相电压合格率

#define OBIS_WALLET_FILE        0x202C      //（当前）钱包文件
#define OBIS_OVERDRAFT_AMOUNT   0x202D      //（当前）透支金额
#define OBIS_CUMULATIVE_AMOUNT  0x202E      //累计购电金额

#define OBIS_EVENT_BEGIN_TIME       0x201E      //事件发生时间
#define OBIS_EVENT_END_TIME         0x2020      //事件结束时间
#define OBIS_EVENT_SEQ              0x2022      //事件记录序号
#define OBIS_EVENT_SOURCE           0x2024      //事件发生源
#define OBIS_EVENT_REPORT_STATUS    0x3300      //通道上报状态

#define OBIS_HOLD_TIME             0x2021      //冻结时间
#define OBIS_HOLD_SEQ              0x2023      //冻结记录序号


//控制相关
#define OBIS_ADDGROUP_1         0x2301    //总加组1
#define OBIS_ADDGROUP_2         0x2302    //总加组2
#define OBIS_ADDGROUP_3         0x2303    //总加组3
#define OBIS_ADDGROUP_4         0x2304    //总加组4
#define OBIS_ADDGROUP_5         0x2305    //总加组5
#define OBIS_ADDGROUP_6         0x2306    //总加组6
#define OBIS_ADDGROUP_7         0x2307    //总加组7
#define OBIS_ADDGROUP_8         0x2308    //总加组8

#define OBIS_PULSE_METER_1         0x2401    //脉冲计量点1
#define OBIS_PULSE_METER_2         0x2402    //脉冲计量点2
#define OBIS_PULSE_METER_3         0x2403    //脉冲计量点3
#define OBIS_PULSE_METER_4         0x2404    //脉冲计量点4
#define OBIS_PULSE_METER_5         0x2405    //脉冲计量点5
#define OBIS_PULSE_METER_6         0x2406    //脉冲计量点6
#define OBIS_PULSE_METER_7         0x2407    //脉冲计量点7
#define OBIS_PULSE_METER_8         0x2408    //脉冲计量点8

#define OBIS_ACC_WAR_FLOW           0x2500   //累计水（热）流量
#define OBIS_ACC_GAS_FLOW           0x2501  //累计气流量
#define OBIS_ACC_HOT_FLOW           0x2502  //累计热量
#define OBIS_HOT_POWER              0x2503  //热功率
#define OBIS_ACC_WORK_TIME          0x2504  //累计工作时间
#define OBIS_WARTER_TEMPERATURE     0x2505  //水温
#define OBIS_STATUS_ST              0x2506  //（仪表）状态ST

#define OBIS_REMOTE_CTRL        0x8000    //遥控
#define OBIS_PROTECT            0x8001    //保电
#define OBIS_ALARM_FEE          0x8002    //催费告警
#define OBIS_CHINESE_INFO       0x8003    //中文信息
#define OBIS_CHINESE_INFO_VIP   0x8004    //重要中文信息
#define OBIS_SECURITY_VALUE     0x8100    //保安定值
#define OBIS_PCTRL_TIMESEG      0x8101    //功控时段
#define OBIS_PCTRL_ALARM_TIME   0x8102    //功控告警时间
#define OBIS_TIMESEG_CTRL       0x8103    //时段功控
#define OBIS_RELAX_CTRL         0x8104    //厂休功控
#define OBIS_STOPWORK_CTRL      0x8105    //营业报停控
#define OBIS_FLOATDOWN_CTRL     0x8106    //当前功率下浮控
#define OBIS_BUYFEE_CTRL        0x8107    //购电控
#define OBIS_MONTHFEE_CTRL      0x8108    //月电控
#define OBIS_TIMESEG_CFG_UNIT   0x8109    //时段功控配置单元
#define OBIS_RELAY_CFG_UNIT     0x810A    //厂休控配置单元
#define OBIS_STOPWORK_CFG_UNIT  0x810B    //营业报停控配置单元
#define OBIS_BUYFEE_CFG_UNIT    0x810C    //购电控配置单元
#define OBIS_MONTHFEE_CFG_UNIT  0x810D    //月电控配置单元
#define OBIS_CTRL_OBJ           0x810E    //控制对象
#define OBIS_RELAY_TURN         0x810F    //跳闸轮次
#define OBIS_FCTRL_VALUE        0x8110    //电控定值

#define OBIS_INSTANTANEOUS_HOLD     0x5000      //瞬时冻结
#define OBIS_SEC_HOLD               0x5001      //秒冻结
#define OBIS_MINUTE_HOLD            0x5002      //分钟冻结
#define OBIS_HOUR_HOLD              0x5003      //小时冻结
#define OBIS_DAY_HOLD               0x5004      //日冻结
#define OBIS_SETTLEMENT_DAY_HOLD    0x5005      //结算日冻结
#define OBIS_MONTH_HOLD             0x5006      //月冻结
#define OBIS_YEAR_HOLD              0x5007      //年冻结
#define OBIS_TZONE_SWITCH_HOLD      0x5008      //时区表切换冻结
#define OBIS_DAY_PERIOD_HOLD        0x5009      //日时段表切换冻结
#define OBIS_ELECTRICITY_HOLD       0x500A      //费率电价切换冻结
#define OBIS_LADDER_SWITCH_HOLD     0x500B      //阶梯切换冻结
#define OBIS_MON_VOLTAGE_HOLD       0x5010      //电压合格率月冻结
#define OBIS_LADDER_SETTLE_HOLD     0x5011      //阶梯结算冻结


#define OBIS_STAT_MINUTE            0x2100	//分钟区间统计	统计周期单位为分钟
#define OBIS_STAT_HOUR              0x2101	//小时区间统计	统计周期单位为小时
#define OBIS_STAT_DAY               0x2102	//日区间统计	统计周期单位为日
#define OBIS_STAT_MONTH             0x2103	//月区间统计	统计周期单位为月
#define OBIS_STAT_YEAR              0x2104	//年区间统计 	统计周期单位为年
#define OBIS_AVG_MINUTE             0x2110	//分钟平均	统计周期单位为分钟
#define OBIS_AVG_HOUR               0x2111	//小时平均	统计周期单位为时
#define OBIS_AVG_DAY                0x2112	//日平均	统计周期单位为日
#define OBIS_AVG_MONTH              0x2113	//月平均	统计周期单位为月
#define OBIS_AVG_YEAR               0x2114	//年平均	统计周期单位为年
#define OBIS_MAX_MIN_MINUTE         0x2120	//分钟极值	统计周期单位为分钟
#define OBIS_MAX_MIN_HOUR           0x2121	//小时极值	统计周期单位为时
#define OBIS_MAX_MIN_DAY            0x2122	//日极值	统计周期单位为日
#define OBIS_MAX_MIN_MONTH          0x2123	//月极值	统计周期单位为月
#define OBIS_MAX_MIN_YEAR           0x2124	//年极值	统计周期单位为年

#endif