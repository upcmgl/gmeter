#ifndef __PLMS_READ_C1_H__
#define __PLMS_READ_C1_H__



//终端运行参数文件
#define FILE_RUN_PARA   FILEID_RUN_DATA

//转换函数BCD:X, BIN:B
#define MeterDate_3761FMT1		0x01	//电表日期转换函数:组合物理量(高字节)YYMMDDWW+HHMMSS(低字节)转换为3761A1
#define ProgrammDateTime_3761FMT1	0x02	//376.1 A.1 format YYWMMDDHHMMSS(BCD:YY DDHHMMSS, BIN:WMM(W:bit7-5,M:bit4,M:BCD) )
#define HarmonicFactor_3761FMT5	0x03	//谐波含量645: XX.XX% ->376.1 A.5 S(bit7)X(bit6-4)XX.X%
#define Voltage97_3761FMT7		0x04	//97_645 xxx -> 376.1 A.7 xxx.x
#define ReactivePower97_3761FMT9	0x05 	//97_645 xx.xx -> 376.1 A.9 xx.xxxx
#define PhaseLossingNum97_3761FMT10	0x06	//97_645 NNNN -> 376.1 A.10 xxxxxx
#define Energy_3761FMT14	0x07	//645 A.1XXXXXX.XX ->376.1 A.14XXXXXX.XXXX
#define DemandTime07_3761FMT17		0x08 	//07_645 YYMMDDhhmmss -> 376.1 A.17 MMDDhhmm
#define Current97_3761FMT25		0x09 	//97_645 xx.xx -> 376.1 A.25  xxx.xxx
#define CellWorkingTime97_3761FMT27		0x0A	// 97_645 NNNNNN -> 376.1 xxxxxxxx
#define ZTZ3_3761FMT  0x0B    //FN161中状态字3转换
#define PhaseB_3761FMT  0x0C  //载波表B相电流相角
#define PhaseC_3761FMT  0x0D  //载波表C相电流相角
#define MeterDate_3761FMT1_SH		0x0E    //上海一类F162处理
#define Energy_FMT14toFMT11	0x1F	//645 A.1XXXXXX.XXXX ->376.1 A.11XXXXXX.XX

//转换函数
#define RATE            0x0F
#define REC_DATE	0x10	//  终端时间-> 376.1 A.15
#define REC_DATE_SEC    0x11
#define MeterPowerStatus_3761FMT	0x12 //376.1 F161 电能表通断电状态
#define DXSJ_COMPLEMENT_3761FMT 0x13               //根据ABC相计算 断相总次数等
#define Datetime2FMT15          0x14   //YYWMMDDHHMMSS -> YYWMMDDHHMM
#define SetPhaseV               0x15   //默认电压相角0 120 240
#define A3toA2                  0x16   //3字节变成2字节
#define Datetime09toFMT15       0x17   //09规约的时标5字节
#define I3toI2                  0x18   //电流的格式转换
#define RATE_1                  0x19   //费率1
#define DELEWEEK                0x20   //去除星期
#define JSJL_HSJ                0x21   //校时记录后时间
#define JSJL_QSJ                0x22   //校时记录前时间
#define SYDL_ZDZ_2_376          0x23   //江苏剩余电流最大值 数据顺序倒换
#define DLSJK_FORMAT3to4        0x24   //江苏断路器电流数据转换
#define BbreakerIto376          0x25   //江苏剩余电流块电流数据转换
#define CbreakerIto376          0x26   //江苏剩余电流块电流数据转换
#define GetSYDL                 0x27   //江苏剩余电流块提取电流数据
#define GET_REC_DATE            0x28   //江苏曲线时间
#define GET_RQ                  0x29   //日期星期取出日期
#define AbreakerIto376          0x30   //江苏剩余电流块电流数据转换
#define S1C_JS_JLto3761         0x31   //校时记录提取校时前时间 江苏

//C8之后规划四表集抄转换函数
#define DataCJT_to_3761FMT      0xC8//CJT188转376.1
#define CJT_SQDB_TO_3761        0xC9//水汽表转376.1
#define CJT_RB_TO_3761          0xCA//热表转376.1
#define CJT_MODBUS_TO_3761      0xCB// modbus转376.1
#define MODBUS_EE_TO_FALSEDATA  0xCC//水门表空数据填充
#define CJT_SHUIMEN_TO_3761     0xCD//水门协议转376.1
#define CJT_WUHAN_TO_3761       0xCE//武汉水表转376.1
#define CJT_EN1434_RB_TO_3761   0xCF// EN1434转376.1
#define XY645_TO_3761           0xD1//兴源水表645协议转376.1
#define CJT_HUBEI645_TO_3761    0xD2//
#define CJT_SQDB_TO_3761JIANGXI 0xD3//江西发现的燃气表，CJT188协议，但是回复数据只有6字节 ，3个字节的整数，无小数位
#define CJT_NINGBO_TO_3761      0xD4//
#define CJT_LIXIN_TO_3761       0xD5//
#define XINGYUAN_TO_3761        0xD6
#define CJT_FENXI645_RB_TO_3761 0xD7
#define FUJIAN645_TO_3761       0xD8
#define CJT_645_97_RB_TO_3761   0xD9
#define CJT_RB_TO_3761_reverse  0xDA//热表转376.1,回复报文单位在数据前的情况，永阳热表
#define CJT_DAYHOLD_TO_RECDAYHOLD      0xDB
#define CJT_JIAJIENENG_RB_TO_3761      0xDC
#define CJT_SQDB_TO_3761_HN            0xDD
#define CJT_DAYHOLD_TO_RECDAYHOLD_HN   0xDE
#define CJT_SQDB_SHORT_TO_3761         0xDF
#define SR8009_TO_3761                 0xE1 //中间有定义了
#define CJT_SHANKE_TO_3761             0xE2
#define CJT_CHONGQING_TO_3761          0xE3
#define CJT_HANGZHOU_TO_3761           0xE4
#define CJT_MODBUS_RQB_TO_3761         0xE5
#define CJT_CHENGUANG_TO_3761          0xE6
#define CJT_SHANDONG_F28_TO_3761       0xE7
#define CJT_DAYHOLD_TO_RECDAYHOLD_RB     0xE8
#define CJT_DAYHOLD_TO_RECDAYHOLD_RB_HN  0xE9
#define CJT_SQDB_TO_3761_SH_RQB          0xEA
#define CJT_SQDB_TO_3761_SH_SB           0xEB
#define CJT_SQDB_TO_3761_0DF1_SZ         0xEC
#define CJT_SQDB_TO_3761_0DF161_SZ       0xED
#define CJT_SQDB_TO_3761_0CF129_SZ       0xEE
#define CJT_JINGDA_TO_3761               0xEF
#define CJT_CHENGUANG_NEW_TO_3761        0xF0
#define MAX_RES_SIZE    1000

//起始字节
#define PHY_LEN          4
#define RES_LEN          1
//#define RES_DATA       5


//使用缓冲区标志
#define RES_BUF   0x01
#define REQ_BUF   0x02


#define  PIM_METER_DOC 0
//数据项块标识
#define   BLOCK_MARK 0x30
//抄表标识
#define   BLOCK_FIRST     0x00
#define   SINGLE         0x01
#define   BLOCK           0x02


//国标645-1997报文位置定义
#define POS_GB645_FLAG1       0
#define POS_GB645_METERNO     1
#define POS_GB645_FLAG2       7
#define POS_GB645_CTRL        8
#define POS_GB645_DLEN        9
#define POS_GB645_ITEM       10
#define POS_GB645_97_DATA    12
#define POS_GB645_07_DATA    14

//物理量地址名
#define CUR_YGZ      0000
#define CUR_WGZ      0001
#define CUR_Q1_WGZ   0002
#define CUR_Q4_WGZ   0003
#define DATE_TIME    0004
#define CUR_P_YG     0005
#define CUR_P_WG     0006
#define CUR_P_SZ     0007//视在功率
#define CUR_Q        0008//功率因数
#define CUR_V        0009
#define CUR_I        0010
#define CUR_I_LX     0011//零线电流


//一类数据超时时间
#define  TIME_OUT_C1    6000  //单位 10ms


void get_work_info(INT8U *resp);
void  process_frame_read(objRequest *pRequest,objResponse *pResp);
BOOLEAN get_read_c1data(objRequest *pRequest,objResponse *pResp,READ_CONTEXT *readC1);
INT16U app_get_fn_data_c1(objRequest *pRequest,objResponse *pResp);
INT16U read_C1_F1(INT8U *resp);

#endif       /* __PLMS_READ_C1_H__ */

