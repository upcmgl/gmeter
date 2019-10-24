/***************************************************************

    DL/T 1376.1  G表 ===主站下行报文处理任务

    1)请求2类数据 0x0D


***************************************************************/
#include "app_const_3761.h"
#include "../main_include.h"
#include "app_3761.h"
#include "protocol_app_handler.h"   
#include "app_var_3761.h"  


void  process_frame_readhis(objRequest *pRequest,objResponse *pResp);
BOOLEAN get_hisread_data(objRequest *pRequest,objResponse *pResp,READ_CONTEXT *readC2);

INT8U trans_C2_curve_midu(READ_CONTEXT *readC2);
INT8U get_read_param(READ_FHJU* read_fhju,INT8U midu,INT8U point,INT16U total_len,INT8U point_len);





/*****************************************************************
Fn数据结构表
typedef struct
{
    INT32U  item;         //数据项
    INT8U   offset;       //偏移量
    INT8U   len;          //数据长度 (上行协议长度1376.1)
    INT8U   num;          //块数据项个数 （暂时不需要使用）
    INT8U   prop;         //上N日字节位置  D0-D1（0：D0字节 1 ：D1字节 2 ：D2字节
                            3：D4字节）D2: 1 曲线数据   D3-D7空闲
    INT16U  format;       //转换函数 （07到1376.1转换函数）D15（1：非BCD 0：默认BCD）
                            NOT_BCD
}tagFN_ITEM;
******************************************************************/
const   tagFN_ITEM  C2_FN1_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    0,                      0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060101,     9,      25,     1,      0,                      Energy_3761FMT14_BLOCK,},
{   0x05060301,     34,     20,     1,      0,                      0,},
{   0x05060501,     54,     20,     1,      0,                      0,},
{   0x05060801,     74,     20,     1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN2_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    0,                      0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060201,     9,      25,     1,      0,                      Energy_3761FMT14_BLOCK,},
{   0x05060401,     34,     20,     1,      0,                      0,},
{   0x05060601,     54,     20,     1,      0,                      0,},
{   0x05060701,     74,     20,     1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN3_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    0,                      0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0xFFFFFFFF,     9,      15,     1,      0,                      0,},
{   0x05060901,     24,     20,     1,      0,                      0,},
{   0xFFFFFFFF,     44,     15,     1,      0,                      0,},
{   0x05060A01,     59,     20,     1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN4_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    0,                      0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0xFFFFFFFF,     9,      15,     1,      0,                      0,},
{   0x05060801,     24,     20,     1,      0,                      0,},
{   0xFFFFFFFF,     44,     15,     1,      0,                      0,},
{   0x05060B01,     59,     20,     1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN5_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    0,                      0,},	//	日冻结类数据时标Td_d
{	0x80000002,     3,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060101,     4,      20,     1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN6_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    0,                      0,},	//	日冻结类数据时标Td_d
{	0x80000002,     3,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060301,     4,      20,     1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN7_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    0,                      0,},	//	日冻结类数据时标Td_d
{	0x80000002,     3,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060201,     4,      20,     1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN8_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    0,                      0,},	//	日冻结类数据时标Td_d
{	0x80000002,     3,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060401,     4,      20,     1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN9_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    0,                      0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060101,     9,      25,     1,      0,                      Energy_3761FMT14_BLOCK,},
{   0x05060301,     34,     20,     1,      0,                      0,},
{   0x05060501,     54,     20,     1,      0,                      0,},
{   0x05060801,     74,     20,     1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN10_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    0,                      0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060201,     9,      25,     1,      0,                      Energy_3761FMT14_BLOCK,},
{   0x05060401,     34,     20,     1,      0,                      0,},
{   0x05060601,     54,     20,     1,      0,                      0,},
{   0x05060701,     74,     20,     1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN11_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    0,                      0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0xFFFFFFFF,     9,      15,     1,      0,                      0,},
{   0x01010000,     24,     20,     1,      0,                      0,},
{   0xFFFFFFFF,     44,     15,     1,      0,                      0,},
{   0x01030000,     59,     20,     1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN12_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    0,                      0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0xFFFFFFFF,     9,      15,     1,      0,                      0,},
{   0x01020000,     24,     20,     1,      0,                      0,},
{   0xFFFFFFFF,     44,     15,     1,      0,                      0,},
{   0x01040000,     59,     20,     1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN17_ITEMS[] =
{
{	0x80000006,     0,	    2,      1,	    0,                      0,},	//	月冻结类数据时标Td_d
{	0x80000001,     2,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     7,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x0001FF01,     8,      25,     1,      0,                      Energy_3761FMT14_BLOCK,},
{   0x0003FF01,     33,     20,     1,      0,                      0,},
{   0x0005FF01,     53,     20,     1,      0,                      0,},
{   0x0008FF01,     73,     20,     1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN18_ITEMS[] =
{
{	0x80000006,     0,	    2,      1,	    0,                      0,},	//	月冻结类数据时标Td_d
{	0x80000001,     2,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     7,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x0002FF01,     8,      25,     1,      0,                      Energy_3761FMT14_BLOCK,},
{   0x0004FF01,     33,     20,     1,      0,                      0,},
{   0x0006FF01,     53,     20,     1,      0,                      0,},
{   0x0007FF01,     73,     20,     1,      0,                      0,},
};

//const   tagFN_ITEM  C2_FN19_ITEMS[] =
//{
//{	0x80000006,     0,	    2,      1,	    0,                      0,},	//	月冻结类数据时标Td_d
//{	0x80000001,     2,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
//{	0x80000002,     7,	    1,	    1,	    0,                      RATE,},	//	费率数
//{   0xFFFFFFFF,     8,      15,     1,      0,                      0,},
//{   0x01010000,     23,     20,     1,      0,                      0,},
//{   0xFFFFFFFF,     43,     15,     1,      0,                      0,},
//{   0x01030000,     58,     20,     1,      0,                      0,},
//};
//
//const   tagFN_ITEM  C2_FN20_ITEMS[] =
//{
//{	0x80000005,     0,	    2,      1,	    0,                      0,},	//	月冻结类数据时标Td_d
//{	0x80000001,     2,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
//{	0x80000002,     7,	    1,	    1,	    0,                      RATE,},	//	费率数
//{   0xFFFFFFFF,     8,      15,     1,      0,                      0,},
//{   0x01020000,     23,     20,     1,      0,                      0,},
//{   0xFFFFFFFF,     43,     15,     1,      0,                      0,},
//{   0x01040000,     58,     20,     1,      0,                      0,},
//};

//const   tagFN_ITEM  C2_FN21_ITEMS[] =
//{
//{	0x80000005,     0,	    2,      1,	    0,                      0,},	//	月冻结类数据时标Td_d
//{	0x80000002,     2,	    1,	    1,	    0,                      RATE,},	//	费率数
//{   0x0001FF01,     3,      20,     1,      0,                      0,},
//};
//
//const   tagFN_ITEM  C2_FN22_ITEMS[] =
//{
//{	0x80000005,     0,	    2,      1,	    MASK_DAYHOLD_D0,        0,},	//	月冻结类数据时标Td_d
//{	0x80000002,     2,	    1,	    1,	    0,                      RATE,},	//	费率数
//{   0x0003FF01,     3,      20,     1,      0,                      Energy_3761FMT14_BLOCK,},
//};
//
//const   tagFN_ITEM  C2_FN23_ITEMS[] =
//{
//{	0x80000005,     0,	    2,      1,	    MASK_DAYHOLD_D0,        0,},	//	月冻结类数据时标Td_d
//{	0x80000002,     2,	    1,	    1,	    0,                      RATE,},	//	费率数
//{   0x0002FF01,     3,      20,     1,      0,                      Energy_3761FMT14_BLOCK,},
//};
//
//const   tagFN_ITEM  C2_FN24_ITEMS[] =
//{
//{	0x80000005,     0,	    2,      1,	    MASK_DAYHOLD_D0,        0,},	//	月冻结类数据时标Td_d
//{	0x80000002,     2,	    1,	    1,	    0,                      RATE,},	//	费率数
//{   0x0004FF01,     3,      20,     1,      0,                      Energy_3761FMT14_BLOCK,},
//};

const   tagFN_ITEM  C2_FN32_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    0,                      0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{   0x13000001,     8,      3,      1,      0,                      0,},    //总断相次数
{   0x13010001,    11,      3,      1,      0,                      0,},    //A相断相次数    
{   0x13020001,    12,      3,      1,      0,                      0,},    //B相断相次数
{   0x13030001,    17,      3,      1,      0,                      0,},    //C相断相次数
{   0x13000002,    20,      3,      1,      0,                      0,},    //断相累计时间
{   0x13010002,    23,      3,      1,      0,                      0,},    //A断相累计时间
{   0x13020002,    26,      3,      1,      0,                      0,},    //B断相累计时间
{   0x13030002,    29,      3,      1,      0,                      0,},    //C断相累计时间
{   0x13000101,    32,      4,      1,      0,                      0,},    //最近一次断相起始时刻
{   0x13010101,    36,      4,      1,      0,                      0,},    //A相最近断相起始时刻
{   0x13020101,    40,      4,      1,      0,                      0,},    //B相最近断相起始时刻
{   0x13020101,    44,      4,      1,      0,                      0,},    //C相最近断相起始时刻
{   0x13000201,    48,      4,      1,      0,                      0,},    //最近一次断相结束时刻
{   0x13012501,    52,      4,      1,      0,                      0,},    //A相最近断相结束时刻
{   0x13022501,    56,      4,      1,      0,                      0,},    //B相最近断相结束时刻
{   0x13032501,    60,      4,      1,      0,                      0,},    //C相最近断相结束时刻
};


const   tagFN_ITEM  C2_FN81_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100300,     7,      3,        0xFF,   0,                             0,},   //有功功率
};

const   tagFN_ITEM  C2_FN82_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100301,     7,      3,        0xFF,   0,                             0,},   //有功功率
};
const   tagFN_ITEM  C2_FN83_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100302,     7,      3,        0xFF,   0,                             0,},   //有功功率
};
const   tagFN_ITEM  C2_FN84_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100303,     7,      3,        0xFF,   0,                             0,},   //有功功率
};
const   tagFN_ITEM  C2_FN85_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100400,     7,      3,        0xFF,   NOT_SINGLE,                    0,},   //无功功率
};

const   tagFN_ITEM  C2_FN86_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100401,     7,      3,        0xFF,   NOT_SINGLE,                    0,},   //无功功率
};

const   tagFN_ITEM  C2_FN87_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100402,     7,      3,        0xFF,   NOT_SINGLE,                    0,},   //无功功率
};

const   tagFN_ITEM  C2_FN88_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100403,     7,      3,        0xFF,   NOT_SINGLE,                    0,},   //无功功率
};

const   tagFN_ITEM  C2_FN89_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100101,     7,      2,        0xFF,   0,                             0,},   //电压数据
};
const   tagFN_ITEM  C2_FN90_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100102,     7,      2,        0xFF,   0,                             0,},   //电压数据
};
const   tagFN_ITEM  C2_FN91_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100103,     7,      2,        0xFF,   0,                             0,},   //电压数据
};
const   tagFN_ITEM  C2_FN92_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100201,     7,      3,        0xFF,   0,                             0,},   //电流数据
};
const   tagFN_ITEM  C2_FN93_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100202,     7,      3,        0xFF,   0,                             0,},   //电流数据
};
const   tagFN_ITEM  C2_FN94_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100203,     7,      3,        0xFF,   0,                             0,},   //电流数据
};
const   tagFN_ITEM  C2_FN97_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100601,     7,      4,        0xFF,   0,                             0,},   //有功电能量
};

const   tagFN_ITEM  C2_FN98_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100603,     7,      4,        0xFF,   NOT_SINGLE,                    0,},   //无功电能量
};

const   tagFN_ITEM  C2_FN99_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100602,     7,      4,        0xFF,   NOT_SINGLE,                    0,},   //反向有功电能量
};

const   tagFN_ITEM  C2_FN101_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100601,     7,      4,        0xFF,   0,                             0,},   //正向有功电能示值
};

const   tagFN_ITEM  C2_FN102_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100603,     7,      4,        0xFF,   0,                             0,},   //正向有功电能示值
};
const   tagFN_ITEM  C2_FN103_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100602,     7,      4,        0xFF,   0,                             0,},   //反向有功电能示值
};
const   tagFN_ITEM  C2_FN104_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100604,     7,      4,        0xFF,   0,                             0,},   //反向有功电能示值
};
const   tagFN_ITEM  C2_FN105_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100500,     7,      2,        0xFF,   0,                             0,},   //总功率因数
};

const   tagFN_ITEM  C2_FN106_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100501,     7,      2,        0xFF,   0,                             0,},   //A相功率因数
};

const   tagFN_ITEM  C2_FN107_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100502,     7,      2,        0xFF,   0,                             0,},   //A相功率因数
};

const   tagFN_ITEM  C2_FN108_ITEMS[] =
{
{	0x80000007,     0,	    7,	      1,	  0,                             0,},	//	曲线类数据时标Td_c
{   0x06100503,     7,      2,        0xFF,   0,                             0,},   //A相功率因数
};

const   tagFN_ITEM  C2_FN161_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060101,     9,     25,      1,      0,                      Energy_3761FMT14_BLOCK,},
};

const   tagFN_ITEM  C2_FN162_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060301,     9,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN163_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060201,     9,     25,      1,      0,                      Energy_3761FMT14_BLOCK,},
};

const   tagFN_ITEM  C2_FN164_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060401,     9,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN165_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060501,     9,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN166_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060601,     9,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN167_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060701,     9,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN168_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060801,     9,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN169_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060101,     9,     25,      1,      0,                      Energy_3761FMT14_BLOCK,},
};

const   tagFN_ITEM  C2_FN170_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060301,     9,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN171_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060201,     9,     25,      1,      0,                      Energy_3761FMT14_BLOCK,},
};

const   tagFN_ITEM  C2_FN172_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060401,     9,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN173_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060501,     9,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN174_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060601,     9,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN175_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060701,     9,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN176_ITEMS[] =
{
{	0x80000005,     0,	    3,      1,	    MASK_DAYHOLD_D0,        0,},	//	日冻结类数据时标Td_d
{	0x80000001,     3,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     8,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x05060801,     9,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN177_ITEMS[] =
{
{	0x80000005,     0,	    2,      1,	    MASK_DAYHOLD_D0,        0,},	//	月冻结类数据时标Td_d
{	0x80000001,     2,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     7,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x0001FF01,     8,     25,      1,      0,                      Energy_3761FMT14_BLOCK,},
};

const   tagFN_ITEM  C2_FN178_ITEMS[] =
{
{	0x80000005,     0,	    2,      1,	    MASK_DAYHOLD_D0,        0,},	//	月冻结类数据时标Td_d
{	0x80000001,     2,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     7,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x0003FF01,     8,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN179_ITEMS[] =
{
{	0x80000005,     0,	    2,      1,	    MASK_DAYHOLD_D0,        0,},	//	月冻结类数据时标Td_d
{	0x80000001,     2,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     7,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x0002FF01,     8,     25,      1,      0,                      Energy_3761FMT14_BLOCK,},
};

const   tagFN_ITEM  C2_FN180_ITEMS[] =
{
{	0x80000005,     0,	    2,      1,	    MASK_DAYHOLD_D0,        0,},	//	月冻结类数据时标Td_d
{	0x80000001,     2,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     7,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x0004FF01,     8,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN181_ITEMS[] =
{
{	0x80000005,     0,	    2,      1,	    MASK_DAYHOLD_D0,        0,},	//	月冻结类数据时标Td_d
{	0x80000001,     2,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     7,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x0005FF01,     8,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN182_ITEMS[] =
{
{	0x80000005,     0,	    2,      1,	    MASK_DAYHOLD_D0,        0,},	//	月冻结类数据时标Td_d
{	0x80000001,     2,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     7,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x0006FF01,     8,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN183_ITEMS[] =
{
{	0x80000005,     0,	    2,      1,	    MASK_DAYHOLD_D0,        0,},	//	月冻结类数据时标Td_d
{	0x80000001,     2,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     7,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x0007FF01,     8,     20,      1,      0,                      0,},
};

const   tagFN_ITEM  C2_FN184_ITEMS[] =
{
{	0x80000005,     0,	    2,      1,	    MASK_DAYHOLD_D0,        0,},	//	月冻结类数据时标Td_d
{	0x80000001,     2,	    5,	    1,	    0,                      0,},	//	终端抄表时间  最后
{	0x80000002,     7,	    1,	    1,	    0,                      RATE,},	//	费率数
{   0x0008FF01,     8,     20,      1,      0,                      0,},
};

INT8U trans_C2_curve_midu(READ_CONTEXT *readC2)
{
    INT8U midu;
    
    switch(readC2->Td[5])
    {
        case 1:
        midu = 1;
            break;
        case 2:
        midu = 2;
            break;
        case 3:
        midu = 4;
            break;
        default:
        midu = 1;
            break;
    }
    return midu;
}

INT8U get_read_param(READ_FHJU* read_fhju,INT8U midu,INT8U point,INT16U total_len,INT8U point_len)
{
    INT8U idx,idx2;
    INT16U tmp_len2;
    total_len *= midu;
    for(idx2=0;idx2<15;idx2++)
    {
        if((180-idx2*12)%(midu*point_len) == 0)
        {
            tmp_len2 = 180-idx2*12;
            break;
        }
    }
    if(tmp_len2 != 0)
    {
        read_fhju->read_num = total_len/tmp_len2;    
        if(total_len%tmp_len2 != 0)
        {
            read_fhju->read_num++;
        }
    }
    else
    {
        read_fhju->read_num = 0;
    }

    for(idx=0;idx<read_fhju->read_num;idx++)
    {
        if(total_len >= tmp_len2*(idx+1))
        {
            if(point_len != 0)
            {
                read_fhju->read_point[idx] = tmp_len2/point_len;
                read_fhju->offset[idx] = idx*tmp_len2;
            }
            else
            {
                read_fhju->read_point[idx] = 0;
                read_fhju->offset[idx] = idx*tmp_len2;
            }
        }
        else
        {
            if(point_len != 0)
            {
                read_fhju->read_point[idx] = (total_len - tmp_len2*idx)/point_len;
                read_fhju->offset[idx] = idx*tmp_len2;
            }
            else
            {
                read_fhju->read_point[idx] = 0;
                read_fhju->offset[idx] = idx*tmp_len2;
            }
        }
    }
    return 1;
}
/*+++
   功能： 请求2类数据
   参数:  INT8U *Reply 应答帧工作区
         INT16U frameLen  命令帧的数据单元域长度
         INT8U *frameData 命令帧的数据单元域
   下行报文：
         数据单元标识1
         数据单元1  (Td1)
         ...
         数据单元标识n
         数据单元n (Tdn)
         TP
   上行报文：
         数据单元标识1
         数据单元1
         ...
         数据单元标识n
         数据单元n
         EC 
         Tp
---*/

void  process_frame_readhis(objRequest *pRequest,objResponse *pResp)
{
    READ_CONTEXT  readC2;
    INT16U flag=0;

    INT16U pn_tmp;//临时保存PN
    INT16U pn_idx;//临时保存所有pn的索引

    ClrTaskWdt();
    pRequest->context = (INT8U *)&readC2;
    //如果不是主动上报,写入FLASH的情况
    //if(pResp->channel!=CHANNEL_FLASH)
    {
           //设置控制码：DIR=1,PRM=0, FUNC=8  响应报文，用户数据
           pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_USERDATA;
    }

    //首帧标志置1
    readC2.is_first_frame = TRUE;
    //readC2.is_first_td = TRUE;

    //一个数据项目的多桢发送标志复位
    readC2.check_flag &= ~SENDFRAME_FLAG;
    read_c1_time_out = TIME_OUT_C1;//超时时间初始化

    /////////////////////////////////
    //    开始组织响应帧
    /////////////////////////////////

    pRequest->pos = POS_DATA;

    while((pRequest->pos - POS_DATA) < pRequest->userDataLen)
    {
       ClrTaskWdt();

       //数据单元标识 
       readC2.pn = bin2_int16u(pRequest->frame + pRequest->pos);
       readC2.fn = bin2_int16u(pRequest->frame + pRequest->pos + 2);
       pRequest->pos += 4;
       flag=0;
       
       //提取命令报文的数据单元的长度，存储地址
       if((readC2.check_flag & SENDFRAME_FLAG) ==0x00)
       {
          readC2.pos_fn_td = pRequest->pos;
          mem_cpy(readC2.Td,pRequest->frame+pRequest->pos,7);
         #ifdef __BATCH_TRANSPARENT_METER_CYCLE_TASK__
          readC2.param = pRequest->frame+pRequest->pos; //__BATCH_TRANSPARENT_METER_CYCLE_TASK__
          readC2.seq = 0; //zy加的
         #endif

            //浙江扩展的全部测量点支持，2011.3.20 by zyg
            //#ifdef  __PROVICE_ZHEJIANG__
            #if(1)   // 新规范规定改了，都要查是否全部测量点
             pn_tmp=readC2.pn;
             pn_idx=0;//初始化测量点序号为0
             #if defined(__QNMDW_FK2012__)
             while(get_next_pn_neimeng(pn_tmp,&(readC2.pn),&pn_idx,readC2.fn,1))
             #else
             while(get_next_pn(pn_tmp,&(readC2.pn),&pn_idx))
             #endif
             {
               if(FALSE == get_hisread_data(pRequest,pResp,&readC2) )
               {
                   if(readC2.is_first_frame)
                   {
                      app_send_NAK(pRequest,pResp);
                   }
                   return;
               }
               else
               {
                 #ifdef __PROVICE_NEIMENG__
                 if(pn_tmp == 0xFFFF)
                 #else
                 if(pn_tmp == 0x00FF)
                 #endif
                 {
                   flag++;//说明读取历史数据成功，由于Pn为FF的情况是在get_hisread_data之前进行处理，造成  readC2.pos_fn_td计算错误，后续FN无法读取，这里添加flag进行判断
                 }
               }
             }
            #else
            if(FALSE == get_hisread_data(pReqeust,&readC2) )
            {
               if(readC2.is_first_frame)
               {
                  app_send_NAK(pRequest,pResp);
               }
               return;
            }
          #endif
       }
        #ifdef __PROVICE_NEIMENG__
        if(pn_tmp == 0xFFFF)
        #else
        if(pn_tmp == 0x00FF)
        #endif
       {
           pRequest->pos = (readC2.pos_fn_td - pRequest->pos)/flag+pRequest->pos;
       }
       else
       {
           pRequest->pos = readC2.pos_fn_td;  //越过请求的时标
       }

       //复位多桢发送标志
       readC2.check_flag &= ~SENDFRAME_FLAG;

    }

    if((pResp->pos == POS_DATA)  && (readC2.is_first_frame == TRUE))
    {
        //应答报文没有一个数据项目，并且这是首个应答祯，则发送否定报文
        app_send_NAK(pRequest,pResp);
    }
    else
    {
       //发送响应祯，结束=TRUE
       app_send_response_frame(pRequest,pResp,TRUE);
    }
}

/*+++
  功能：读取历史数据
  参数：
        PLMS_RESP   *resp        应答报文
        INT16U      *resp_pos    应答报文的起始位置
        READ_HIS    *hisread     读取历史数据的访问结构
  返回：
        TRUE  / FALSE
  描述:
       1) 先按照不拆分PN,FN进行响应
       2) 如果需要分祯,则按照最小PiFi重新进行响应

---*/
BOOLEAN get_hisread_data(objRequest *pRequest,objResponse *pResp,READ_CONTEXT *readC2)
{
   INT16U   pi,fi;
   #ifdef __PROVICE_SHANGHAI__
   INT16U   max_left_len;
   #endif
   INT16U   pngroup[64];
   INT16U   fngroup[8];
    INT16U   datalen;
    #ifdef __PRECISE_TIME__
    INT16U   meter_seq_start;
    #endif
    //INT16U   need_more_frame;   //TODO:此处有问题，需要处理告警
    #ifdef __PROVICE_ANHUI__
    INT16U   meter_seq;
    #endif
    #ifdef __ZXYG_EQUAL_TO_ZXYG_ADD_FXYG__
   INT16U   fx_len;
    #endif
   #ifdef __PROVICE_FUJIAN__
   CommandDate cmddate1,cmddate2;
   INT8U    first_td[7];
   #endif
   INT8U    fng_idx;
   INT16U   png_idx,pngroupcount;
   INT8U    fngroupcount;
   INT8U    td_len;
    #if defined(__REDUCE_FLOW__)
   INT8U    check_pos,check_len,check_idx;  // 检查是否为EE，为EE则抛弃数据
    BOOLEAN check_flag;
    #endif
    BOOLEAN  is_pifi_mode;
    #ifdef __BATCH_TRANSPARENT_METER_CYCLE_TASK__
    BOOLEAN f306_clear_flag;
    #endif
    #if defined (__QNMDW_FK2012__)
    BOOLEAN  is_plc = FALSE;
    #endif
    INT8U    td[7]={0};           
    #ifdef __ZXYG_EQUAL_TO_ZXYG_ADD_FXYG__
   METER_CLASS_PARAM meter_class;
    #endif

    #if defined(__REDUCE_FLOW__)
   check_pos = 0;
   check_len = 0;  //为0的时候不检查数据是否EE
    #endif

   //初始按照整体PNFN响应
   is_pifi_mode = FALSE;

   GET_HISREAD_DATA_AGAIN:

    #ifdef __ZXYG_EQUAL_TO_ZXYG_ADD_FXYG__
   meter_class.value = 0;
    #endif

   //填写应答报文的数据标识
   //信息点集合处理,处理时遵循先Fn，后Pn,从小到大顺序。
   pngroupcount = parse_pn_to_pngroup(readC2->pn,pngroup);
   fngroupcount = parse_fn_to_fngroup(readC2->fn,fngroup);

   #if defined (__QNMDW_FK2012__)
   //蒙西需根据Fn的不同调整Pn，直接判断第一个Fn即可
   fi = fngroup[0];
   if(fi >=  DT_F153)
   {
       //低压测量点，和376.1格式一致
       is_plc = TRUE;
   }
   else
   {
       //格式和FK2005格式一致
       pngroupcount = parse_pn_to_pngroup_2005(readC2->pn,pngroup);
       is_plc = FALSE;
   }
   #endif

  /* #ifdef __PROVICE_HUBEI__
   if((readC2->pn == 0xFFFF) && (readC2->fn == DT_F801 || readC2->fn == DT_F802))
   {
        pngroupcount = 1;
        meter_seq = 1;
        pngroup[0] = 0xFFFF;
   }
   #endif
  */
   #if defined(__REDUCE_FLOW__)
   is_pifi_mode = TRUE;
   #endif

   //if((pngroupcount==1) && (fngroupcount==1)) is_pifi_mode = TRUE;
   is_pifi_mode = TRUE;

   //pos = pResp->pos;
   if(FALSE == is_pifi_mode)
   {
       set_pn_fn(pResp->frame, &pResp->pos, readC2->pn, readC2->fn);
     //pResp->pos+=4; //??
   }

   #ifdef __PROVICE_FUJIAN__
   //福建要求对未来时标的数据恢复否认，判断第一个数据项的时标是否是未来，是的话，会否认，否则按照正常回复
   mem_cpy(first_td,readC2->Td,7);
   #endif

   readC2->need_more_frame = FALSE;
   #ifdef __BATCH_TRANSPARENT_METER_CYCLE_TASK__
   f306_clear_flag = FALSE;  //__BATCH_TRANSPARENT_METER_CYCLE_TASK__
   #endif
   //循环处理PN,从小到大的顺序
   td_len = 0;
//   if((readC2->pn == 0xFF00)||((gAppInfo.hubei_801 == 1)&&(readC2->pn == 0xFFFF)))
//   {
//        pngroupcount = MAX_METER_COUNT;  //todo 目前仅支持了255个测量点
//   }
   for(png_idx=0;png_idx < pngroupcount;png_idx++)
   {
//       if((readC2->pn == 0xFF00)||((gAppInfo.hubei_801 == 1)&&(readC2->pn == 0xFFFF)))
//       {
//           fread_array(png_idx+1,PIM_METER_DOC,(INT8U *)&(readC2->meter_doc),sizeof(METER_DOCUMENT));
//           if((png_idx+1) != bin2_int16u(readC2->meter_doc.meter_idx))  continue;
//           pi = (png_idx)/8+1;
//           pi = pi<< 8;
//           pi |= 0x01<<((png_idx)%8);
//       }
//       else
       {
           pi = pngroup[png_idx];
       }
       //readC2->pn_idx = pi;

       #ifdef __BATCH_TRANSPARENT_METER_CYCLE_TASK__
       f306_clear_flag = TRUE;
       #endif

       if(pi!=DA_P0)
       {

//            if(readC2->meter_seq!=0)//todo增加总加组的判断
//            {
//              fread_array(readC2->meter_seq,PIM_METER_DOC,(INT8U *)&(readC2->meter_doc),sizeof(METER_DOCUMENT)); //todo未查到档案 返回ERR
//            }
            //总加组可以排除
            if((readC2->fn == DT_F57)||(readC2->fn == DT_F58)||(readC2->fn == DT_F59)||(readC2->fn == DT_F60)||(readC2->fn == DT_F61)||(readC2->fn == DT_F62)||(readC2->fn == DT_F65)||(readC2->fn == DT_F66)||(readC2->fn == DT_F73)||(readC2->fn == DT_F74)||(readC2->fn == DT_F75)||(readC2->fn == DT_F76))
            {
                  continue;
            }

       }

       //循环处理FN,从小到大的顺序
      for(fng_idx=0;fng_idx<fngroupcount;fng_idx++)
      {
         fi = fngroup[fng_idx];
         readC2->work_flag = 0;

         if(is_pifi_mode==TRUE)
         {
            set_pn_fn(pResp->frame, &pResp->pos, pi, fi); 

         }
         if(readC2->need_more_frame == FALSE)
         {
           mem_cpy(td,readC2->Td,7);  //add liujing 20101011  如果用hisread->Td的话，一个数据单元标识有多个测量点时，曲线数据就会有问题，取数据时，对hisread->Td做了修改造成上行帧不正确
         }
         else
         {
            readC2->need_more_frame = FALSE;
         }

            #ifdef __BATCH_TRANSPARENT_METER_CYCLE_TASK__
            if ((f306_clear_flag) && (fi == DT_F306))
            {
                readC2->seq = 1;    //分帧使用
                f306_clear_flag = FALSE;
            }
            #endif
            
         switch(fi)
         {
         //readC2->work_flag 标识抄读数据类型。日电能量 ：01 月电能量 02 曲线电能量 03 日冻结数据 04 月冻结 05 曲线 10  抄表日 06  小时冻结 07
         //总加组 （日、月、曲线）总加组 08    当日电能量  0E  当月电能量 0F
         //小时冻结总加组 09  抄表日使用上已结算日数据项 0C
         //白噪声 色噪声 0D
         case DT_F1:  //日冻结正向有/无功电能示值、一/四象限无功电能示值（总、费率1~M）
            pRequest->item_count = sizeof(C2_FN1_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN1_ITEMS;
            readC2->work_flag = 0x04;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len =3;
                    #if defined(__REDUCE_FLOW__)
            check_pos = 6;
                    #endif
            break;
         case DT_F2:  //日冻结反向有/无功电能示值,二/三象限无功电能示值
            pRequest->item_count = sizeof(C2_FN2_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN2_ITEMS;
            readC2->work_flag = 0x04;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len =3;
                    #if defined(__REDUCE_FLOW__)
            check_pos = 6;
                    #endif
            break;
         //case DT_F3:  //日冻结正向最大需量及发生时间（总、费率1~M）
            //pRequest->item_count = sizeof(C2_FN3_ITEMS)/sizeof(tagFN_ITEM);
            //pRequest->fn = (INT8U *)&C2_FN3_ITEMS;
            //readC2->work_flag = 0x04;
            //datalen = app_get_fn_data_c2(pRequest,pResp);
            //td_len =3;
                    //#if defined(__REDUCE_FLOW__)
            //check_pos = 6;
                    //#endif
            //break;
         //case DT_F4:  //日冻结反向最大需量及发生时间（总、费率1~M）
            //pRequest->item_count = sizeof(C2_FN4_ITEMS)/sizeof(tagFN_ITEM);
            //pRequest->fn = (INT8U *)&C2_FN4_ITEMS;
            //readC2->work_flag = 0x04;
            //datalen = app_get_fn_data_c2(pRequest,pResp);
            //td_len =3;
                    //#if defined(__REDUCE_FLOW__)
            //check_pos = 6;
                    //#endif
            //break;
         case DT_F5: //日冻结正向有功电能量（总及费率）
            pRequest->item_count = sizeof(C2_FN5_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN5_ITEMS;
            readC2->work_flag = 0x01;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len =3;
                    #if defined(__REDUCE_FLOW__)
            check_pos = 1;
                    #endif
            break;
         case DT_F6: //日冻结正向无功电能量（总及费率）
            pRequest->item_count = sizeof(C2_FN6_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN6_ITEMS;
            readC2->work_flag = 0x01;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len =3;
                    #if defined(__REDUCE_FLOW__)
            check_pos = 1;
                    #endif
            break;
         case DT_F7: //日冻结反向有功电能量（总及费率）
            pRequest->item_count = sizeof(C2_FN7_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN7_ITEMS;
            readC2->work_flag = 0x01;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len =3;
                    #if defined(__REDUCE_FLOW__)
            check_pos = 1;
                    #endif
            break;
         case DT_F8: //日冻结反向无功电能量（总及费率）
            pRequest->item_count = sizeof(C2_FN8_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN8_ITEMS;
            readC2->work_flag = 0x01;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len =3;
                    #if defined(__REDUCE_FLOW__)
            check_pos = 1;
                    #endif
            break;
         case DT_F9:  //抄表日冻结: F1
//            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
//                return FALSE;
            readC2->work_flag = 0x06;
            pRequest->item_count = sizeof(C2_FN1_ITEMS)/sizeof(tagFN_ITEM);
            #ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
            pRequest->fn = (INT8U *)&C2_FN9_ITEMS;
            #else
            pRequest->fn = (INT8U *)&C2_FN1_ITEMS;
            #endif
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len =3;
                    #if defined(__REDUCE_FLOW__)
            check_pos = 6;
                    #endif
            break;
         case DT_F10: //              F2
//            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
//                return FALSE;
            readC2->work_flag = 0x06;
            pRequest->item_count = sizeof(C2_FN2_ITEMS)/sizeof(tagFN_ITEM);
            #ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
            pRequest->fn = (INT8U *)&C2_FN10_ITEMS;
            #else
            pRequest->fn = (INT8U *)&C2_FN2_ITEMS;
            #endif
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len =3;
                    #if defined(__REDUCE_FLOW__)
            check_pos = 6;
                    #endif
            break;
         //case DT_F11: //              F3
////            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
////                return FALSE;
            //readC2->work_flag = 0x06;
            //pRequest->item_count = sizeof(C2_FN3_ITEMS)/sizeof(tagFN_ITEM);
            //pRequest->fn = (INT8U *)&C2_FN3_ITEMS;
            //datalen = app_get_fn_data_c2(pRequest,pResp);
            //td_len =3;
                    //#if defined(__REDUCE_FLOW__)
            //check_pos = 6;
                    //#endif
            //break;
         //case DT_F12: //              F4
////            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
////                return FALSE;
            //readC2->work_flag = 0x06;
            //pRequest->item_count = sizeof(C2_FN4_ITEMS)/sizeof(tagFN_ITEM);
            //pRequest->fn = (INT8U *)&C2_FN4_ITEMS;
            //datalen = app_get_fn_data_c2(pRequest,pResp);
            //td_len =3;
                    //#if defined(__REDUCE_FLOW__)
            //check_pos = 6;
                    //#endif
            //break;
//         #ifdef __QGDW_FK2005__
//         case DT_F13: //组合有功电能示值（总、费率1~M）	测量点号	Td_d
//         case DT_F165: //组合有功电能示值（总、费率1~M）	测量点号	Td_d
//            pRequest->item_count = sizeof(C2_FN13_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN13_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_pos = 6;
//                    #endif
//            break;
//        #else
//         case DT_F13:
//            return FALSE;
//        #endif
//         case DT_F14:
//         case DT_F15:
//         case DT_F16:
//            return FALSE;
         case DT_F17: //月冻结 电能示值
            pRequest->item_count = sizeof(C2_FN17_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN17_ITEMS;
            readC2->work_flag = 0x05;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            #ifdef __ZXYG_EQUAL_TO_ZXYG_ADD_FXYG__
            if ((meter_class.value != 0x02) && (meter_class.value != 0x62) && (meter_class.value != 0x71) && (meter_class.value != 0x72)&& (datalen != 0))
            {
                pRequest->pResp->pos += datalen;
                pRequest->phy_count = sizeof(C2_FN179_ITEMS)/sizeof(tagFN_ITEM_C2);
                pRequest->fn = (INT8U *)&C2_FN179_ITEMS;
                fx_len = get_fn_data_c2(pRequest,pRequest->pResp);
                pRequest->pResp->pos -= datalen;
                computer_zfxyg_add(pRequest->pResp->frame+pRequest->pResp->pos+8,pRequest->pResp->frame+pRequest->pResp->pos+datalen+8,pRequest->pResp->frame[pRequest->pResp->pos+7],5);
            }
            #endif
            td_len =2;
                    #if defined(__REDUCE_FLOW__)
            check_pos = 6;
                    #endif
            break;
         case DT_F18:
            pRequest->item_count = sizeof(C2_FN18_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN18_ITEMS;
            readC2->work_flag = 0x05;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len =2;
                    #if defined(__REDUCE_FLOW__)
            check_pos = 6;
                    #endif
            break;
         //case DT_F19: //月冻结　最大需量
            //pRequest->item_count = sizeof(C2_FN19_ITEMS)/sizeof(tagFN_ITEM);
            //pRequest->fn = (INT8U *)&C2_FN19_ITEMS;
            //readC2->work_flag = 0x05;
            //datalen = app_get_fn_data_c2(pRequest,pResp);
            //td_len =2;
                    //#if defined(__REDUCE_FLOW__)
            //check_pos = 6;
                    //#endif
            //break;
         //case DT_F20:
            //pRequest->item_count = sizeof(C2_FN20_ITEMS)/sizeof(tagFN_ITEM);
            //pRequest->fn = (INT8U *)&C2_FN20_ITEMS;
            //readC2->work_flag = 0x05;
            //datalen = app_get_fn_data_c2(pRequest,pResp);
            //td_len =2;
                    //#if defined(__REDUCE_FLOW__)
            //check_pos = 6;
                    //#endif
            //break;
         //case DT_F21: //月冻结正向有功电能量（总及费率）
            //pRequest->item_count = sizeof(C2_FN21_ITEMS)/sizeof(tagFN_ITEM);
            //pRequest->fn = (INT8U *)&C2_FN21_ITEMS;
            //readC2->work_flag = 0x02;
            //datalen = app_get_fn_data_c2(pRequest,pResp);
            //#ifdef __ZXYG_EQUAL_TO_ZXYG_ADD_FXYG__
            //if ((meter_class.value != 0x02) && (meter_class.value != 0x62) && (meter_class.value != 0x71) && (meter_class.value != 0x72)&& (datalen != 0))
            //{
                //pRequest->pResp->pos += datalen;
                //pRequest->phy_count = sizeof(C2_FN23_ITEMS)/sizeof(tagFN_ITEM_C2);
                //pRequest->fn = (INT8U *)&C2_FN23_ITEMS;
                //readC2->work_flag = 0x02;
                //fx_len = get_fn_data_c2(pRequest,pRequest->pResp);
                //pRequest->pResp->pos -= datalen;
                //computer_zfxyg_add(pRequest->pResp->frame+pRequest->pResp->pos+3,pRequest->pResp->frame+pRequest->pResp->pos+datalen+3,pRequest->pResp->frame[pRequest->pResp->pos+2],4);
            //}
            //#endif
            //td_len =2;
                    //#if defined(__REDUCE_FLOW__)
            //check_pos = 1;
                    //#endif
            //break;
         //case DT_F22: //月冻结正向无功电能量（总及费率）
            //pRequest->item_count = sizeof(C2_FN22_ITEMS)/sizeof(tagFN_ITEM);
            //pRequest->fn = (INT8U *)&C2_FN22_ITEMS;
            //readC2->work_flag = 0x02;
            //datalen = app_get_fn_data_c2(pRequest,pResp);
            //td_len =2;
                    //#if defined(__REDUCE_FLOW__)
            //check_pos = 1;
                    //#endif
            //break;
         //case DT_F23: //月冻结反向有功电能量（总及费率）
            //pRequest->item_count = sizeof(C2_FN23_ITEMS)/sizeof(tagFN_ITEM);
            //pRequest->fn = (INT8U *)&C2_FN23_ITEMS;
            //readC2->work_flag = 0x02;
            //datalen = app_get_fn_data_c2(pRequest,pResp);
            //td_len =2;
                    //#if defined(__REDUCE_FLOW__)
            //check_pos = 1;
                    //#endif
            //break;
         //case DT_F24: //月冻结反向无功电能量（总及费率）
            //pRequest->item_count = sizeof(C2_FN24_ITEMS)/sizeof(tagFN_ITEM);
            //pRequest->fn = (INT8U *)&C2_FN24_ITEMS;
            //readC2->work_flag = 0x02;
            //datalen = app_get_fn_data_c2(pRequest,pResp);
            //td_len =2;
                    //#if defined(__REDUCE_FLOW__)
            //check_pos = 1;
                    //#endif
            //break;
//         case DT_F25: //日总及分相最大有功功率发生时间
//         case DT_F26: //日总及分相最大需量及发生时间
//         case DT_F27: //日电压统计数据
//         case DT_F28: //日不平衡统计
//         case DT_F29:  //日电流越限统计
//         case DT_F30:  //日视在功率统计
//         #ifndef __QGDW_FK2005__
////         #ifdef __STAT_LOADRATE__
//         case DT_F31:  //日负载率统计
////         #endif
//         #endif
//         case DT_F43:  //日功率因数区段统计
//         case DT_F41: //电容器投入累计时间和次数
//         case DT_F42: //日月电容器累计补偿的无功电能量
//         case DT_F211: //日冻结三相电压、电流不平衡度95%概率值
//         case DT_F129: //日冻结直流模拟量越限累计时间，最大最小值及发生时间
//               td_len = 3;
//               datalen = read_his_spot_dayhold(pResp->frame+pResp->pos,pi,fi,readC2->Td);
//               //((READ_CONTEXT *)(pRequest->context))->pos_fn_td += 3;//这里加TD长度是否影响组合PNFN？？？？
//                    #if defined(__REDUCE_FLOW__)
//               check_pos = 0;
//                    #endif
//              #ifdef __PROVICE_HUNAN__
//                    #if defined(__REDUCE_FLOW__)
//              if(fi == DT_F129) check_len = 0xFF;
//              #endif
//                    #endif
//               break;
//         #if defined(__QGDW_FK2005__)
//         case DT_F31://日冻结电能表断相数据
//         #endif
         case DT_F32: //日冻结电能表断相数据
            pRequest->item_count = sizeof(C2_FN32_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN32_ITEMS;
            readC2->work_flag = 0x04;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len =3;
            #if defined __QGDW_376_2009_PROTOCOL__
            datalen = convert_cur_spot_2009_value(fi,pRequest->pResp->frame+pRequest->pResp->pos,datalen);
            #endif
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
//         case DT_F33: //月冻结总及分相最大有功功率及发生时间
//         case DT_F34: //月冻结总及分相最大需量及发生时间
//         case DT_F35: //月冻结月电压统计数据
//         case DT_F36: //月不平衡统计
//         case DT_F37:  //月电流越限统计
//         case DT_F38:  //月视在功率统计
////         #ifdef __STAT_LOADRATE__
//         case DT_F39:  //月负载率统计
////         #endif
//         case DT_F44:  //月功率因数区段统计
//         case DT_F212:  //月冻结三相电压、电流不平衡度95%概率值
//         case DT_F130: //月冻结直流模拟量越限累计时间，最大最小值及发生时间
//               td_len = 2;
//               datalen = read_his_spot_monthhold(pResp->frame+pResp->pos,pi,fi,readC2->Td);
//               //((READ_CONTEXT *)(pRequest->context))->pos_fn_td += 2;
//                    #if defined(__REDUCE_FLOW__)
//               check_pos = 0;
//                    #endif
//              #ifdef __PROVICE_HUNAN__
//                    #if defined(__REDUCE_FLOW__)
//              if(fi == DT_F130) check_len = 0xFF;
//              #endif
//                    #endif
//               break;
//        case DT_F49://终端日供电时间，日复位累计次数
//            datalen = get_f49_data_c2(pRequest,pResp);
//            td_len =3;
//            break;
//         case DT_F50://日冻结终端日控制统计数据
//            datalen = get_f50_data_c2(pRequest,pResp);
//            td_len = 3;
//            break;
//         case DT_F51://终端月供电时间，月复位累计次数
//            datalen = get_f51_data_c2(pRequest,pResp);
//            td_len =2;
//            break;
//         case DT_F52://月冻结终端日控制统计数据
//            datalen = get_f52_data_c2(pRequest,pResp);
//            td_len = 2;
//            break;
//         case DT_F53://日冻结终端日通信流量
//            datalen = get_f53_data_c2(pRequest,pResp);
//            td_len =3;
//            break;
//         case DT_F54://月冻结月通信流量
//            datalen = get_f54_data_c2(pRequest,pResp);
//            td_len =2;
//            break;
//		#ifdef __PROVICE_CHONGQING__
//		case DT_F55://日冻结终端日集中抄表统计
//            datalen = get_f55_data_c2(pRequest,pResp);
//            td_len =3;
//            break;
//		case DT_F247://日冻结网络运行统计
//            datalen = get_f247_data_c2(pRequest,pResp);
//            td_len =3;
//            break;
//		#endif
//         case DT_F57:
//            pRequest->item_count = sizeof(C2_FN57_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN57_ITEMS;
//            readC2->work_flag = 0x08;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 0;
//                    #endif
//            break;
//         case DT_F58:
//            pRequest->item_count = sizeof(C2_FN58_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN58_ITEMS;
//            readC2->work_flag = 0x08;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 1;
//                    #endif
//            break;
//         case DT_F59:
//            pRequest->item_count = sizeof(C2_FN59_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN59_ITEMS;
//            readC2->work_flag = 0x08;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 1;
//                    #endif
//            break;
//         case DT_F60://月冻结功率统计
//            datalen = get_f60_data_c2(pRequest,pResp);
//            td_len =2;
//            break;
//         case DT_F61:
//            pRequest->item_count = sizeof(C2_FN61_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN61_ITEMS;
//            readC2->work_flag = 0x08;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 2;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 1;
//                    #endif
//            break;
//         case DT_F62:
//            pRequest->item_count = sizeof(C2_FN62_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN62_ITEMS;
//            readC2->work_flag = 0x08;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 2;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 1;
//                    #endif
//            break;
//         case DT_F65:
//         case DT_F66:
//            mem_cpy(pResp->frame+pResp->pos,td,2);
//            mem_set(pResp->frame+pResp->pos+2,6,0xEE);
//            datalen = 8;
//            td_len = 2;
//            break;
//         case DT_F73:
//            pRequest->item_count = sizeof(C2_FN73_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN73_ITEMS;
//            readC2->work_flag = 0x08;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = datalen - td_len;
//                    #endif
//            break;
//         case DT_F74:
//            pRequest->item_count = sizeof(C2_FN74_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN74_ITEMS;
//            readC2->work_flag = 0x08;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = datalen - td_len;
//                    #endif
//            break;
//         case DT_F75:
//            pRequest->item_count = sizeof(C2_FN75_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN75_ITEMS;
//            readC2->work_flag = 0x08;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = datalen - td_len;
//                    #endif
//            break;
//         case DT_F76:
//            pRequest->item_count = sizeof(C2_FN76_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN76_ITEMS;
//            readC2->work_flag = 0x08;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = datalen - td_len;
//                    #endif
//            break;
         case DT_F81:
        #ifdef __QGDW_FK2005__
         case DT_F176:
        #endif
            pRequest->item_count = sizeof(C2_FN81_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN81_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F82:
            pRequest->item_count = sizeof(C2_FN82_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN82_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F83:
            pRequest->item_count = sizeof(C2_FN83_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN83_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F84:
            pRequest->item_count = sizeof(C2_FN84_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN84_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F85:
        #ifdef __QGDW_FK2005__
         case DT_F177:
        #endif
            pRequest->item_count = sizeof(C2_FN85_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN85_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F86:
            pRequest->item_count = sizeof(C2_FN86_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN86_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F87:
            pRequest->item_count = sizeof(C2_FN87_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN87_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F88:
            pRequest->item_count = sizeof(C2_FN88_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN88_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
        case DT_F89:
            pRequest->item_count = sizeof(C2_FN89_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN89_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F90:
            pRequest->item_count = sizeof(C2_FN90_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN90_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F91:
            pRequest->item_count = sizeof(C2_FN91_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN91_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F92:
            pRequest->item_count = sizeof(C2_FN92_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN92_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F93:
            pRequest->item_count = sizeof(C2_FN93_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN93_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F94:
            pRequest->item_count = sizeof(C2_FN94_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN94_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
//         case DT_F95:
//            pRequest->item_count = sizeof(C2_FN95_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN95_ITEMS;
//            readC2->work_flag = 0x0A;
//            datalen = app_get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = datalen - td_len;
//                    #endif
//            break;
         case DT_F97:
            pRequest->item_count = sizeof(C2_FN101_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN101_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
//         case DT_F98:
//            pRequest->item_count = sizeof(C2_FN102_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN102_ITEMS;
//            readC2->work_flag = 0x03;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = datalen - td_len;
//                    #endif
//            break;
//         case DT_F99:
//            pRequest->item_count = sizeof(C2_FN103_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN103_ITEMS;
//            readC2->work_flag = 0x03;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = datalen - td_len;
//                    #endif
//            break;
//         case DT_F100:
//            pRequest->item_count = sizeof(C2_FN104_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN104_ITEMS;
//            readC2->work_flag = 0x03;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = datalen - td_len;
//                    #endif
//            break;
         case DT_F101:
         #ifdef __QGDW_FK2005__
         case DT_F172:
         #endif
            pRequest->item_count = sizeof(C2_FN101_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN101_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F102:
         #ifdef __QGDW_FK2005__
         case DT_F174:
         #endif
            pRequest->item_count = sizeof(C2_FN102_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN102_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
            #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
            #endif
            break;
         case DT_F103:
         #ifdef __QGDW_FK2005__
         case DT_F173:
         #endif
            pRequest->item_count = sizeof(C2_FN103_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN103_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F104:
         #ifdef __QGDW_FK2005__
         case DT_F175:
         #endif
            pRequest->item_count = sizeof(C2_FN104_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN104_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
            #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
            #endif
            break;
         case DT_F105:
        #ifdef __QGDW_FK2005__
         case DT_F178:
        #endif
            pRequest->item_count = sizeof(C2_FN105_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN105_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F106:
            pRequest->item_count = sizeof(C2_FN106_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN106_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F107:
            pRequest->item_count = sizeof(C2_FN107_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN107_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
         case DT_F108:
            pRequest->item_count = sizeof(C2_FN108_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN108_ITEMS;
            readC2->work_flag = 0x0A;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 7;
                    #if defined(__REDUCE_FLOW__)
            check_len = datalen - td_len;
                    #endif
            break;
//        #ifndef __QGDW_FK2005__
//         case DT_F109:
//            if(readC2->meter_doc.baud_port.port == COMMPORT_485_CY)
//            {
//                pRequest->item_count = sizeof(C2_FN109_CY_ITEMS)/sizeof(tagFN_ITEM_C1);
//                pRequest->fn = (INT8U *)&C2_FN109_CY_ITEMS;
//                datalen = get_fn_data_c2_F219F109F110(pRequest,pResp);
//            }
//            else
//            {
//                pRequest->item_count = sizeof(C2_FN109_ITEMS)/sizeof(tagFN_ITEM_C1);
//                pRequest->fn = (INT8U *)&C2_FN109_ITEMS;
//                datalen = get_fn_data_c2_F219F109F110(pRequest,pResp);
//            }
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = datalen - td_len;
//                    #endif
//            break;
//        #else
//         case DT_F109:
//         case DT_F145:
//             
//            pRequest->item_count = sizeof(C2_FN109_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN109_ITEMS;
//            datalen = get_fn_data_c2_F219F109F110(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = datalen - td_len;
//                    #endif
//            break;
//        #endif
//        #ifndef __QGDW_FK2005__
//         case DT_F110:
//            if(readC2->meter_doc.baud_port.port == COMMPORT_485_CY)
//            {
//                pRequest->item_count = sizeof(C2_FN110_CY_ITEMS)/sizeof(tagFN_ITEM_C1);
//                pRequest->fn = (INT8U *)&C2_FN110_CY_ITEMS;
//                datalen = get_fn_data_c2_F219F109F110(pRequest,pResp);
//            }
//            else
//            {
//                pRequest->item_count = sizeof(C2_FN110_ITEMS)/sizeof(tagFN_ITEM_C1);
//                pRequest->fn = (INT8U *)&C2_FN110_ITEMS;
//                datalen = get_fn_data_c2_F219F109F110(pRequest,pResp);
//            }
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = datalen - td_len;
//                    #endif
//            break;
//        #else
//         case DT_F110:
//         case DT_F179:
//            pRequest->item_count = sizeof(C2_FN110_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN110_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 0;
//                    #endif
//            break;
//        #endif
//		#ifdef __GF_NBQ_DATA_READ_CURVE__
//         case DT_F111: //逆变器曲线数据
//            pRequest->item_count = sizeof(C2_FN111_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN111_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//            break;
//        #endif
//         case DT_F113:
//         case DT_F114:
//         case DT_F115:
//         case DT_F116:
//         case DT_F117:
//         case DT_F118:
//         case DT_F121:
//         case DT_F122:
//         case DT_F123:
////            datalen = get_f49_data_c2(pRequest,pResp);
//              td_len = 3;
//              datalen = read_his_spot_harmonic(pResp->frame+pResp->pos,pi,fi,readC2->Td);
//              //((READ_CONTEXT *)(pRequest->context))->pos_fn_td += 3;//这里加TD长度是否影响组合PNFN？？？？
//                    #if defined(__REDUCE_FLOW__)
//              check_pos = 0;
//                    #endif
//              #ifdef __PROVICE_HUNAN__
//                    #if defined(__REDUCE_FLOW__)
//              check_len = 0;
//              #endif
//                    #endif
//              break;
//         #if defined (__QGDW_376_2013_PROTOCOL__)
//         case DT_F124:
//         case DT_F125:
//         case DT_F126:
//              td_len = 3;
//              datalen = read_his_F124F125F126(pResp->frame+pResp->pos,pi,fi,readC2->Td);
//              //((READ_CONTEXT *)(pRequest->context))->pos_fn_td += 3;//这里加TD长度是否影响组合PNFN？？？？
//                    #if defined(__REDUCE_FLOW__)
//              check_pos = 6;
//                    #endif
//              break;
//         #endif
//        #ifndef __QGDW_FK2005__
//         case DT_F145:
//            pRequest->item_count = sizeof(C2_FN145_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN145_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 0;
//                    #endif
//            break;
//        #endif
//         case DT_F146:
//            pRequest->item_count = sizeof(C2_FN146_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN146_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 0;
//                    #endif
//            break;
//         case DT_F147:
//            pRequest->item_count = sizeof(C2_FN147_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN147_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 0;
//                    #endif
//            break;
//         case DT_F148:
//            pRequest->item_count = sizeof(C2_FN148_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN148_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 0;
//                    #endif
//            break;
//         #ifdef __QGDW_FK2005__
//         case DT_F153:
//            pRequest->item_count = sizeof(C2_FN153_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN153_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 0;
//                    #endif
//            break;
//        #else
//         case DT_F153:
//            pRequest->item_count = sizeof(C2_FN153_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN153_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 0;
//                    #endif
//            break;
//         #endif
//         case DT_F154:
//            pRequest->item_count = sizeof(C2_FN154_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN154_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//            break;
//         case DT_F155:
//            pRequest->item_count = sizeof(C2_FN155_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN155_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//            break;
//         case DT_F156:
//            pRequest->item_count = sizeof(C2_FN156_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN156_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//            break;
//         case DT_F157:
//            pRequest->item_count = sizeof(C2_FN157_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN157_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 2;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//            break;
//         case DT_F158:
//            pRequest->item_count = sizeof(C2_FN158_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN158_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 2;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//            break;
//         case DT_F159:
//            pRequest->item_count = sizeof(C2_FN159_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN159_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 2;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//            break;
//         case DT_F160:
//            pRequest->item_count = sizeof(C2_FN160_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN160_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 2;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//            break;
         case DT_F161: //正向有功电能示值（总、费率1~M）	测量点号	Td_d
         case DT_F801:
            pRequest->item_count = sizeof(C2_FN161_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN161_ITEMS;
            readC2->work_flag = 0x04;
            datalen = app_get_fn_data_c2(pRequest,pResp);

            #ifdef __ZXYG_EQUAL_TO_ZXYG_ADD_FXYG__
            if ((meter_class.value != 0x02) && (meter_class.value != 0x62) && (meter_class.value != 0x71) && (meter_class.value != 0x72)&& (datalen != 0))
            {
                pRequest->pResp->pos += datalen;
                pRequest->phy_count = sizeof(C2_FN163_ITEMS)/sizeof(tagFN_ITEM_C1);
                pRequest->fn = (INT8U *)&C2_FN163_ITEMS;
                fx_len = get_fn_data_c2(pRequest,pRequest->pResp);
                pRequest->pResp->pos -= datalen;
                computer_zfxyg_add(pRequest->pResp->frame+pRequest->pResp->pos+9,pRequest->pResp->frame+pRequest->pResp->pos+datalen+9,pRequest->pResp->frame[pRequest->pResp->pos+8],5);
            }
            #endif
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
                    #if defined(__REDUCE_FLOW__)
            check_pos = 6;
                    #endif          
            break;
         case DT_F162: //正向无功电能示值     日冻结
            pRequest->item_count = sizeof(C2_FN162_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN162_ITEMS;
            readC2->work_flag = 0x04;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F163: //反向有功电能示值     日冻结
            pRequest->item_count = sizeof(C2_FN163_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN163_ITEMS;
            readC2->work_flag = 0x04;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F164: //反向无功电能示值     日冻结
            pRequest->item_count = sizeof(C2_FN164_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN164_ITEMS;
            readC2->work_flag = 0x04;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         #ifndef __QGDW_FK2005__
         case DT_F165: //正向有功电能示值（总、费率1~M）	测量点号	Td_d
            pRequest->item_count = sizeof(C2_FN165_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN165_ITEMS;
            readC2->work_flag = 0x04;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         #endif
         case DT_F166: //正向无功电能示值     日冻结
            pRequest->item_count = sizeof(C2_FN166_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN166_ITEMS;
            readC2->work_flag = 0x04;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F167: //反向有功电能示值     日冻结
            pRequest->item_count = sizeof(C2_FN167_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN167_ITEMS;
            readC2->work_flag = 0x04;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F168: //反向无功电能示值     日冻结
            pRequest->item_count = sizeof(C2_FN168_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN168_ITEMS;
            readC2->work_flag = 0x04;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         #ifndef __QGDW_FK2005__
         case DT_F169:  //抄表日正向有功电能示值（总、费率1~M）	测量点号	Td_d
//            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
//                return FALSE;
            readC2->work_flag = 0x04;
            #ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
            pRequest->phy_count = sizeof(C2_FN169_ITEMS)/sizeof(tagFN_ITEM_C1);
            pRequest->fn = (INT8U *)&C2_FN169_ITEMS;
            #else
            pRequest->item_count = sizeof(C2_FN161_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN161_ITEMS;
            #endif
            datalen = app_get_fn_data_c2(pRequest,pResp);
            #ifdef __ZXYG_EQUAL_TO_ZXYG_ADD_FXYG__
            if ((meter_class.value != 0x02) && (meter_class.value != 0x62) && (meter_class.value != 0x71) && (meter_class.value != 0x72)&& (datalen != 0))
            {
                pRequest->pResp->pos += datalen;
                pRequest->phy_count = sizeof(C2_FN163_ITEMS)/sizeof(tagFN_ITEM_C1);
                pRequest->fn = (INT8U *)&C2_FN163_ITEMS;
                fx_len = get_fn_data_c2(pRequest,pRequest->pResp);
                pRequest->pResp->pos -= datalen;
                computer_zfxyg_add(pRequest->pResp->frame+pRequest->pResp->pos+9,pRequest->pResp->frame+pRequest->pResp->pos+datalen+9,pRequest->pResp->frame[pRequest->pResp->pos+8],5);
            }
            #endif
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F170: //正向无功电能示值     日冻结
//            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
//                return FALSE;
            readC2->work_flag = 0x04;
            pRequest->item_count = sizeof(C2_FN162_ITEMS)/sizeof(tagFN_ITEM);
            #ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
            pRequest->fn = (INT8U *)&C2_FN170_ITEMS;
            #else
            pRequest->fn = (INT8U *)&C2_FN162_ITEMS;
            #endif
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
        #endif
         case DT_F171:   //反向有功电能示值     日冻结
//            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
//                return FALSE;
            readC2->work_flag = 0x04;
            pRequest->item_count = sizeof(C2_FN163_ITEMS)/sizeof(tagFN_ITEM);
            #ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
            pRequest->fn = (INT8U *)&C2_FN171_ITEMS;
            #else
            pRequest->fn = (INT8U *)&C2_FN163_ITEMS;
            #endif
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         #ifndef __QGDW_FK2005__
         case DT_F172:  //反向无功电能示值     日冻结
//            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
//                return FALSE;
            readC2->work_flag = 0x04;
            pRequest->item_count = sizeof(C2_FN164_ITEMS)/sizeof(tagFN_ITEM);
            #ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
            pRequest->fn = (INT8U *)&C2_FN172_ITEMS;
            #else
            pRequest->fn = (INT8U *)&C2_FN164_ITEMS;
            #endif
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F173: //正向有功电能示值（总、费率1~M）	测量点号	Td_d
//            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
//                return FALSE;
            readC2->work_flag = 0x04;
            #ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
            pRequest->phy_count = sizeof(C2_FN173_ITEMS)/sizeof(tagFN_ITEM_C1);
            pRequest->fn = (INT8U *)&C2_FN173_ITEMS;
            #else
            pRequest->item_count = sizeof(C2_FN165_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN165_ITEMS;
            #endif
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F174:
//            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
//                return FALSE;
            readC2->work_flag = 0x04;
            #ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
            pRequest->phy_count = sizeof(C2_FN174_ITEMS)/sizeof(tagFN_ITEM_C1);
            pRequest->fn = (INT8U *)&C2_FN174_ITEMS;
            #else
            pRequest->item_count = sizeof(C2_FN166_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN166_ITEMS;
            #endif
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F175:
//            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
//                return FALSE;
            readC2->work_flag = 0x04;
            #ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
            pRequest->phy_count = sizeof(C2_FN175_ITEMS)/sizeof(tagFN_ITEM_C1);
            pRequest->fn = (INT8U *)&C2_FN175_ITEMS;
            #else
            pRequest->item_count = sizeof(C2_FN167_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN167_ITEMS;
            #endif
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F176:
//            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
//                return FALSE;
            readC2->work_flag = 0x04;
            #ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
            pRequest->phy_count = sizeof(C2_FN176_ITEMS)/sizeof(tagFN_ITEM_C1);
            pRequest->fn = (INT8U *)&C2_FN176_ITEMS;
            #else
            pRequest->item_count = sizeof(C2_FN168_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN168_ITEMS;
            #endif
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F177: //月冻结 电能示值
            pRequest->item_count = sizeof(C2_FN177_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN177_ITEMS;
            readC2->work_flag = 0x05;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            #ifdef __ZXYG_EQUAL_TO_ZXYG_ADD_FXYG__
            if ((meter_class.value != 0x02) && (meter_class.value != 0x62) && (meter_class.value != 0x71) && (meter_class.value != 0x72)&& (datalen != 0))
            {
                pRequest->pResp->pos += datalen;
                pRequest->phy_count = sizeof(C2_FN179_ITEMS)/sizeof(tagFN_ITEM_C2);
                pRequest->fn = (INT8U *)&C2_FN179_ITEMS;
                fx_len = get_fn_data_c2(pRequest,pRequest->pResp);
                pRequest->pResp->pos -= datalen;
                computer_zfxyg_add(pRequest->pResp->frame+pRequest->pResp->pos+8,pRequest->pResp->frame+pRequest->pResp->pos+datalen+8,pRequest->pResp->frame[pRequest->pResp->pos+7],5);
            }
            #endif
            td_len = 2;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F178:
            pRequest->item_count = sizeof(C2_FN178_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN178_ITEMS;
            readC2->work_flag = 0x05;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 2;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F179: //月冻结　最大需量
            pRequest->item_count = sizeof(C2_FN179_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN179_ITEMS;
            readC2->work_flag = 0x05;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 2;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         #endif
         case DT_F180:
            pRequest->item_count = sizeof(C2_FN180_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN180_ITEMS;
            readC2->work_flag = 0x05;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 2;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F181: //月冻结正向有功电能量（总及费率）
            pRequest->item_count = sizeof(C2_FN181_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN181_ITEMS;
            readC2->work_flag = 0x05;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 2;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F182: //月冻结正向无功电能量（总及费率）
            pRequest->item_count = sizeof(C2_FN182_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN182_ITEMS;
            readC2->work_flag = 0x05;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 2;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F183: //月冻结反向有功电能量（总及费率）
            pRequest->item_count = sizeof(C2_FN183_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN183_ITEMS;
            readC2->work_flag = 0x05;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 2;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         case DT_F184: //月冻结反向无功电能量（总及费率）
            pRequest->item_count = sizeof(C2_FN184_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN184_ITEMS;
            readC2->work_flag = 0x05;
            datalen = app_get_fn_data_c2(pRequest,pResp);
            td_len = 2;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
//         case DT_F185: //日冻结正向有功最大需量及发生时间
//            pRequest->item_count = sizeof(C2_FN185_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN185_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 6;
//                    #endif
//            break;
//         case DT_F186: //日冻结正向无功最大需量及发生时间
//            pRequest->item_count = sizeof(C2_FN186_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN186_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 6;
//                    #endif
//            break;
//         case DT_F187: //日冻结反向有功最大需量及发生时间
//            pRequest->item_count = sizeof(C2_FN187_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN187_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 6;
//                    #endif
//            break;
//         case DT_F188: //日冻结反向无功最大需量及发生时间
//            pRequest->item_count = sizeof(C2_FN188_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN188_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 6;
//                    #endif
//            break;
//         case DT_F189: //抄表日冻结正向无功电能量（总及费率）
//            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
//                return FALSE;
//            readC2->work_flag = 0x06;
//            pRequest->item_count = sizeof(C2_FN185_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN185_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 6;
//                    #endif
//            break;
//         case DT_F190: //抄表日冻结反向有功电能量（总及费率）
//            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
//                return FALSE;
//            readC2->work_flag = 0x06;
//            pRequest->item_count = sizeof(C2_FN186_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN186_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 6;
//                    #endif
//            break;
//         case DT_F191: //抄表日冻结反向无功电能量（总及费率）
//            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
//                return FALSE;
//            readC2->work_flag = 0x06;
//            pRequest->item_count = sizeof(C2_FN187_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN187_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 6;
//                    #endif
//            break;
//         case DT_F192: //抄表日冻结反向无功电能量（总及费率）
//            if(get_f33_rec_day(readC2->meter_doc.baud_port.port,readC2->Td)==FALSE)
//                return FALSE;
//            readC2->work_flag = 0x06;
//            pRequest->item_count = sizeof(C2_FN188_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN188_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 6;
//                    #endif
//            break;
//         case DT_F193: //月冻结反向有功电能量（总及费率）
//            pRequest->item_count = sizeof(C2_FN193_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN193_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 2;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 6;
//                    #endif
//            break;
//         case DT_F194: //月冻结反向无功电能量（总及费率）
//            pRequest->item_count = sizeof(C2_FN194_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN194_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 2;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 6;
//                    #endif
//            break;
//         case DT_F195: //月冻结正向无功电能量（总及费率）
//            pRequest->item_count = sizeof(C2_FN195_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN195_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 2;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 6;
//                    #endif
//            break;
//         case DT_F196: //月冻结反向有功电能量（总及费率）
//            pRequest->item_count = sizeof(C2_FN196_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN196_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 2;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 6;
//                    #endif
//            break;
//         case DT_F201:
//         case DT_F202:
//         case DT_F203:
//         case DT_F204:
//         case DT_F205:
//         case DT_F206:
//         case DT_F207:
//         case DT_F208:
//            return FALSE;
//         case DT_F209:
//            pRequest->item_count = sizeof(C2_FN209_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN209_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//            break;
//        #ifdef __QGDW_FK2005__
//         case DT_F170:
//        #endif
//         case DT_F210:
//            pRequest->item_count = sizeof(C2_FN210_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN210_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//            break;
//         case DT_F213:
//            pRequest->item_count = sizeof(C2_FN213_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN213_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 2;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//            break;
//         case DT_F214:
//            pRequest->item_count = sizeof(C2_FN214_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN214_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 2;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//            break;
//         case DT_F215:
//            pRequest->item_count = sizeof(C2_FN215_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN215_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 2;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//            break;
//         case DT_F216:
//            pRequest->item_count = sizeof(C2_FN216_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN216_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 2;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//            break;
//         #ifdef __READ_PLC_NOISE__
//         case DT_F217:  //白噪声曲线
//            readC2->work_flag = 0x0D;
//            pRequest->item_count = sizeof(C2_FN217_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN217_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = datalen - td_len;
//                    #endif
//            break;
//         case DT_F218:  //色噪声曲线
//            readC2->work_flag = 0x0D;
//            pRequest->item_count = sizeof(C2_FN218_ITEMS)/sizeof(tagFN_ITEM_C1);
//            pRequest->fn = (INT8U *)&C2_FN218_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = datalen - td_len;
//                    #endif
//            break;
//         #else
//         case DT_F217:  //白噪声曲线
//         case DT_F218:  //色噪声曲线
//                return false;
//         #endif
//         case DT_F219:
//            pRequest->item_count = sizeof(C2_FN219_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN219_ITEMS;
//            datalen = get_fn_data_c2_F219F109F110(pRequest,pResp);
//            td_len = 7;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 0;
//                    #endif
//            break;
//        #ifdef __PROVICE_SICHUAN__
//        case DT_F220:
//            pRequest->item_count = sizeof(C2_FN220_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN220_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//            break;
//        #endif
//         #ifdef __PROVICE_HEBEI__
//         case DT_F225:
//            pRequest->item_count = sizeof(C2_FN225_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN225_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//             break;
//         #endif
//         #ifdef __QGDW_FK2005__
//         case DT_F231:
//             //get_f49_data_c2
//            datalen = get_fk2005_f231_data_c2(pRequest,pResp);
//            td_len =3;
//             break;
//         #endif
//
//		 #ifdef __METER_DAY_FREEZE_EVENT__
//		 case DT_F230://开表盖
//		 case DT_F231://开端钮盖
//		 case DT_F232:// 跳闸
//                #ifndef __PROVICE_JIANGSU__
//		 case DT_F233://合闸
//		 case DT_F234://电表清零
//                #endif
//            datalen = get_freeze_event_fn_data_c2(pRequest,pResp,fi);
//            td_len =3;
//             break;
//		 #endif
//
//               #ifdef  __PROVICE_JIANGSU__
//                case DT_F233:
//                    pRequest->item_count = sizeof(C2_FN233_ITEMS)/sizeof(tagFN_ITEM);
//                    pRequest->fn = (INT8U *)&C2_FN233_ITEMS;
//                    datalen = get_fn_data_c2(pRequest,pResp);
//                    td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//                    check_len = 5;
//                    #endif
//                    break;
//                case DT_F234:
//                    pRequest->item_count = sizeof(C2_FN234_ITEMS)/sizeof(tagFN_ITEM);
//                    pRequest->fn = (INT8U *)&C2_FN234_ITEMS;
//                    datalen = get_fn_data_c2_F219F109F110(pRequest,pResp);
//                    td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//                    check_len = datalen - td_len;
//                    #endif
//                    break;
//                case DT_F235:
//                    pRequest->item_count = sizeof(C2_FN235_ITEMS)/sizeof(tagFN_ITEM);
//                    pRequest->fn = (INT8U *)&C2_FN235_ITEMS;
//                    datalen = get_fn_data_c2(pRequest,pResp);
//                    td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//                    check_len = 5;
//                    #endif
//                    break;
//               #endif
//
//       #ifdef __PROVICE_SHANGHAI__  //2015-11-26
//       #ifdef __POWER_CTRL__
//         case DT_F235:
//         case DT_F236:
//         case DT_F237:
//              td_len = 7;
//              datalen = read_sh_ertu_voltage_curve(pResp->frame+pResp->pos,readC2->pn,readC2->fn,td,max_left_len);
//              readC2->need_more_frame = td[6];
//              break;
//          #endif
//         case DT_F241:
//              td_len = 3;
//              datalen = read_sh_C2_F241(pResp->frame+pResp->pos,readC2->Td,max_left_len);
//            break;
//         case DT_F242:
//              td_len = 2;
//              datalen = read_sh_C2_F242(pResp->frame+pResp->pos,readC2->Td,max_left_len);
//            break;
//         case DT_F243:
//              td_len = 3;
//              datalen = read_sh_C2_F243(pResp->frame+pResp->pos,readC2->Td,max_left_len);
//            break;
//      #endif
//      case DT_F245:
//            pRequest->item_count = sizeof(C2_FN245_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN245_ITEMS;
//            datalen = get_fn_data_c2(pRequest,pResp);
//
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 5;
//                    #endif
//            break;
//         case DT_F246:
//            pRequest->item_count = sizeof(C2_FN246_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C2_FN246_ITEMS;
//            readC2->work_flag = 0x04;
//            datalen = get_fn_data_c2(pRequest,pResp);
//            td_len = 3;
//                    #if defined(__REDUCE_FLOW__)
//            check_len = 0;
//                    #endif
//            break;
         #ifdef __PROVICE_GANSU__
         case DT_F247:
            pRequest->item_count = sizeof(C2_FN247_ITEMS)/sizeof(tagFN_ITEM);
            pRequest->fn = (INT8U *)&C2_FN247_ITEMS;
            datalen = get_fn_data_c2(pRequest,pResp);
            td_len = 3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 5;
                    #endif
            break;
         #endif
         #ifdef __VOLTAGE_MONITOR__
         case DT_F250: //日冻结电压监视越限数据
             datalen = read_C2_F250(pResp->frame+pResp->pos,((READ_CONTEXT *)(pRequest->context))->Td,SIZE_OF_APP_PROTOCOL_FRAME - pResp->pos);
             td_len = 3;
                    #if defined(__REDUCE_FLOW__)
             check_len = 5;
                    #endif
             break;
         #endif
         #ifdef __INSTANT_FREEZE__
         case DT_F300:
                 td_len = 7;
                 datalen = get_f300_data_c2(pRequest,pResp);
                    #if defined(__REDUCE_FLOW__)
                 check_len = 5;
                    #endif
                 break;
         #endif
         #ifdef __BATCH_TRANSPARENT_METER_CYCLE_TASK__
         case DT_F306:
                td_len = 11;
                datalen = read_C2_F306(pi,pResp->frame+pResp->pos,readC2->param,max_left_len,&(readC2->seq));
                readC2->need_more_frame = readC2->seq;
                break;
         #endif //#ifdef __BATCH_TRANSPARENT_METER_CYCLE_TASK__
         #ifdef __PRECISE_TIME__
         case DT_F321:
                 td_len = 3;
                 datalen = read_C2_F321(pRequest,pResp);
                    #if defined(__REDUCE_FLOW__)
                 check_len = 0;
                    #endif
                 break;
         case DT_F322:
                 td_len = 3;
                 if(pi==0) pResp->pos = 14;
                 datalen = read_C2_F322(pRequest,pi,pResp,&meter_seq_start);
                    #if defined(__REDUCE_FLOW__)
                 check_len = 0;
                    #endif
                 if(pi==0)
                 {
                        readC2->need_more_frame = meter_seq_start;  //没有处理好分帧的问题！pn=0的时候 没用的代码，是否是readC2->need_more_frame？？？？？？？？？？？？？？
                 }
                 break;
         #endif
		#ifdef __HUNAN_NEW_RECORDING__//__PROVICE_HUNAN__
		case DT_F400:
			datalen = get_f400_data_c2(pRequest,pResp);
            td_len =3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 0xFF;
                    #endif
			break;
		case DT_F401:
			datalen = get_f401_data_c2(pRequest,pResp);
            td_len =3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 0xFF;
                    #endif
			break;
            //#if( (defined __ZhuZhou_CITY__) || (defined __PROVICE_HUNAN__) )
        case DT_F402:
			datalen = get_f402_data_c2(pRequest,pResp);
            td_len =3;
                    #if defined(__REDUCE_FLOW__)
            check_len = 0xFF;
                    #endif
			break;
            //#endif
		#endif

         default:
              datalen = 0;

              td_len=3;                     //datalen==0时，设置td_len有何作用?
              //月冻结
              if( (fi >= DT_F17 && fi <= DT_F24) || (fi >= DT_F33 && fi <= DT_F39)
                    || (fi == DT_F44) || (fi == DT_F46) || (fi == DT_F51) || (fi == DT_F52)
                    || (fi == DT_F54) || (fi >= DT_F60 && fi <= DT_F62) || (fi == DT_F65)
                    || (fi == DT_F66) || (fi == DT_F130) || (fi >= DT_F157 && fi <= DT_F160)
                    || (fi >= DT_F177 && fi <= DT_F184) || (fi >= DT_F193 && fi <= DT_F196)
                    || (fi >= DT_F201 && fi <= DT_F208) || (fi >= DT_F213 && fi <= DT_F216) )
              {
                 td_len = 2;
              }
              //曲线
              else if( (fi >= DT_F73 && fi <= DT_F76) || (fi >= DT_F81 && fi <= DT_F95)
              || (fi >= DT_F97 && fi <= DT_F110) || (fi == DT_F138) || (fi >= DT_F145 && fi <= DT_F148)
              || (fi == DT_F217) || (fi == DT_F218) )
              {
                 td_len = 7;
              }
              //return  FALSE;
              break;
         }

         #if defined(__REDUCE_FLOW__)
         if(check_len != 0xFF)
         {
            check_len = datalen-td_len-check_pos;
         }
         else
         check_len = 0;
         if((datalen > 0) && (check_len > 0))
         {
            check_flag = FALSE;
            for(check_idx=0;check_idx<(check_len);check_idx++)
            {
                if (pResp->frame[pResp->pos+td_len+check_pos+check_idx] != REC_DATA_IS_DEFAULT)
                {
                    check_flag = TRUE;
                    break;
                }
            }
            if(check_flag == FALSE)
                datalen = 0;
         }
         #endif

            #ifdef __PROVICE_ANHUI__
            //有效测量点回复数据，无效测量点回复否认
            if(pi>0)
            {
            meter_seq = trans_pn_2_meter_seq(pi);
            if(meter_seq == 0) datalen = 0;
            }
            #endif

         if(datalen == 0)
         {
            //fn_mask =  fi & 0x00FF;
            //readC2->fn &= ~fn_mask;

            if(TRUE == is_pifi_mode)
            {
               pResp->pos -= 4;
            }
            else
            {
               datalen = readC2->pos_pnfn;
               set_pn_fn(pResp->frame, &datalen, readC2->pn, readC2->fn);     //如果该fn无数据单元，则在数据单元标识中去掉
            }
            //必须放在这里，因为前面可能改变其值！
            datalen = 0;
         }

        pResp->pos += datalen;
         if((pResp->pos  > pResp->max_reply_pos)  || readC2->need_more_frame)
         {
             if(is_pifi_mode==TRUE)
             {
                //如果一个报文都无法容纳数据标识,则返回失败
                if(pResp->pos == POS_DATA) return FALSE;           //该条件只有在datalen=0的时候可能成立

                //恢复到上个处理位置if(!need_more_frame )
                if(!readC2->need_more_frame || (readC2->need_more_frame && pResp->pos  > pResp->max_reply_pos ))       //如果已经进行了手工分帧，则不需要再进行分帧,但是如果手工分帧后pos  > resp->max_reply_pos，那么还需要分帧
                {
                  pResp->pos -= (datalen + 4);
                }
                if(pResp->pos == POS_DATA) return FALSE;//如果分帧后位置回到了初始位置，说明缓冲区无法容纳数据项，则首帧返回否认帧，其他放弃处理
                fng_idx--;

                //先行发送响应祯, 结束=FALSE
                //pResp->pos = pos;
                if(FALSE == app_send_response_frame(pRequest,pResp,FALSE) ) return FALSE;
                
                //DelayNmSec(1000);

                //初始化响应报文位置
                pResp->pos = POS_DATA;
                 //pResp->pos=pos;
                //继续组织报文
                continue;
             }
             else
             {
                //重新进行响应
                is_pifi_mode = TRUE;
               // *resp_pos = POS_DATA;

                //重新组织响应报文
                goto GET_HISREAD_DATA_AGAIN;
             }
         }
      }
   }

   //修改报文当前响应位置
   //pResp->pos = pos;

    #ifdef __DAYHOLD_GTOUP_REPEAT_TD__
    readC2->pos_fn_td += td_len * pngroupcount;
    #else
    readC2->pos_fn_td += td_len;
    #endif

   return TRUE;
}

/******************************************
历史数据项转换函数
 * 根据需要提取的日期来转换对应数据项
 * item         转换前数据项
 * item_prop    数据项转换位置
 * td           时标
 * hod_flag     冻结类型 work_flag 日冻结 月冻结
 * 返回 转换后的数据项
*****************************************/
INT32U  app_get_item_with_td(INT32U item,INT8U item_prop,INT8U *td,INT8U hold_flag)
{
    INT32U new_item=0;
    INT32U day_num,day1,day2;
    tagDatetime datetime;

    os_get_datetime(&datetime);

    //获取上N日日冻结数据项 单相表支持62天
    if((hold_flag == 0x04)||(hold_flag == 0x01))
    {
        day1 = getPassedDays(2000+datetime.year,datetime.month,datetime.day);
        #ifdef __PROVICE_JILIN__
        day2 = getPassedDays(2000+BCD2byte(td[2]),BCD2byte(td[1]),BCD2byte(td[0]))-1;
        #else
        day2 = getPassedDays(2000+BCD2byte(td[2]),BCD2byte(td[1]),BCD2byte(td[0]));
        #endif

        if((day1 > day2) && (day1 <= day2+62))
        {
            day_num = day1 - day2;
        }
        else
        {
            return 0xFFFFFFFF;
        }

    }
    else if((hold_flag == 0x05)||(hold_flag == 0x02))
    {
        day1 = datetime.year*12 + datetime.month;
        #ifdef __PROVICE_JILIN__
        day2 = BCD2byte(td[1])*12 + BCD2byte(td[0])-1;
        #else
        day2 = BCD2byte(td[1])*12 + BCD2byte(td[0]);
        #endif

        if((day1 > day2) && (day1 <= day2+12))
        {
            day_num = day1 - day2;
        }
        else
        {
            return 0xFFFFFFFF;
        }
    }
    else if(hold_flag == 0x0E)//当日电能量
    {
        if(item==0x0001FF00)
        {
            return 0x05060101;
        }
        else if(item==0x0002FF00)
        {
            return 0x05060201;
        }
        else if(item==0x0003FF00)
        {
            return 0x05060301;
        }
        else if(item==0x0004FF00)
        {
            return 0x05060401;
        }				
        else 
        {
            return 0xFFFFFFFF;
        }
    }
    else if(hold_flag == 0x0F)//当月电能量
    {
        if(item==0x0001FF00)
        {
            return 0x0001FF01;
        }
        else if(item==0x0002FF00)
        {
            return 0x0002FF01;
        }
        else if(item==0x0003FF00)
        {
            return 0x0003FF01;
        }
        else if(item==0x0004FF00)
        {
            return 0x0004FF01;
        }				
        else
        {
            return 0xFFFFFFFF;
        }
    }
    else
    {
        return item;
    }
    
    new_item = item & (~(0x000000FF<<(item_prop&0x03)));
    return  new_item|((day_num)<<(item_prop&0x03));
}





/**************************
历史数据项抄读接口
 * 返回值 需要判断数据正确性，未验证报文返回数据长度、内容的正确性
**************************/
INT16S  app_read_his_item(INT32U item,INT8U* request,INT8U* resp,INT16U resp_left_len,INT8U* item_tmp,INT16S left_time_ms)
{
    INT32U start_time;
    INT16U len=0;
    INT8U req_len=0,idx;
    INT8U req[50]={0};

    start_time = system_get_tick10ms();
    if(item_tmp != NULL)
    {
        if(item_tmp[0]!=0)
        {
        	req_len = make_gb645_2007_frame(req,(INT8U*)&(gmeter_doc.meter_no),0x11,item,item_tmp,6);
        }
        else
        {
        	req_len = make_gb645_2007_frame(req,(INT8U*)&(gmeter_doc.meter_no),0x11,item,resp,0);        
        }
    }
    else
    {
        req_len = make_gb645_2007_frame(req,(INT8U*)&(gmeter_doc.meter_no),0x11,item,resp,0);
    }

    if(left_time_ms < 50)
    {
        return 0;
    }
    //是否考虑重发多次抄读 抄读接口是否有重发？
    //暂不考虑块抄改单抄
    for(idx=0;idx<1;idx++)
    {
        len = app_trans_send_meter_frame(req,req_len,resp,0xFF,left_time_ms);
        left_time_ms -= time_elapsed_10ms(start_time);
    }

    if(len == 0)
    {
        return 0;
    }

    len = unwrap_gb645_frame(resp,len);

    //判断数据项正确性
    if((len))
    {
        if(((item>>16) ==  0x00000610) && (len >11))
        {
            mem_cpy(request,resp+6+5,len-6-5);
        }
        else
        {
            mem_cpy(request,resp+6,len-6);
        }
    }
    else
    {
        return 0;
    }

    if(((item>>16) ==  0x00000610) && (len >11))
    {
         return len-6-5;
    }
    else
    {
        return len-6;
    }
}



/*************************
  内部数据处理接口
*************************/
void app_prcess_inter_data(objRequest *pRequest,objResponse *pResp,tagFN_ITEM *pFn)
{
    INT32U  cur_num,for_num;
    INT8U idx;
    BOOLEAN is_valid1,is_valid2;
    tagDatetime datetime,datetime_tmp;
    READ_CONTEXT *readC2;
    
    readC2 =  (READ_CONTEXT *)(pRequest->context);
    
    if(pFn->item == 0x80000001) //
    {
        os_get_datetime(&datetime);
		mem_cpy(&datetime_tmp,&datetime,sizeof(tagDatetime));
        if(pFn->format == REC_DATE_SEC)
        {
            for(idx=0;idx<6;idx++)
            {
                pResp->frame[pResp->pos+pFn->offset+idx] = byte2BCD(datetime_tmp.value[SECOND+idx]);
            }    
        }
        else
        {
            for(idx=0;idx<5;idx++)
            {
                pResp->frame[pResp->pos+pFn->offset+idx] = byte2BCD(datetime_tmp.value[MINUTE+idx]);
            }    
        }             
    }
    if(pFn->item == 0x80000002) //todo费率根据存储读取
    {
        pResp->frame[pResp->pos+pFn->offset] = 4;
    }
    if(pFn->item == 0x80000005)
    {
        mem_cpy(pResp->frame + pResp->pos+pFn->offset,&(readC2->Td),3);
    }
    else if(pFn->item == 0x80000006)
    {
        mem_cpy(pResp->frame + pResp->pos+pFn->offset,&(readC2->Td),2);
    }
    else if(pFn->item==0x80000007)
    {
        mem_cpy(pResp->frame + pResp->pos+pFn->offset,readC2->Td,7);
    }
    else if(pFn->item == 0x8000000C)                //FN216
    {
        for(idx=0; idx<5; idx++)       //4费率
        {
            cur_num = bcd2u32((pResp->frame+pResp->pos+pFn->offset+33+5*idx),5,&is_valid1);
            for_num = bcd2u32((pResp->frame+pResp->pos+pFn->offset+8+5*idx),5,&is_valid2);
            if(is_valid1 && is_valid2)
            {
                if(cur_num > for_num) cur_num = cur_num - for_num;
                else cur_num = 0;
            }
            else cur_num = 0;

            ul2bcd(cur_num,(pResp->frame+pResp->pos+pFn->offset+33+5*idx),5);
        }

    }
    else if(pFn->item == 0x8000000D)
    {
        mem_set(pResp->frame + pResp->pos+pFn->offset,0x00,4);
    }
    return;

}

/**************
二类数据Fn抄读接口
**************/
INT16U app_get_fn_data_c2(objRequest *pRequest,objResponse *pResp)
{
    INT32U item,data_rec=0,data_cur=0;
    INT16U require_len,left_len,data_len=0,res_len_tmp=0;
    INT8U idx,idx_item,idx_point,block_num,res_len=0;
    INT8U *pread,*pbuffer,*param=NULL;
    INT8U td_first[7],td[4],td_tmp[4];
    BOOLEAN  is_valid1,is_valid2;
    READ_CONTEXT *readC2;
    tagFN_ITEM *pFn;
    tagDatetime datetime;    
    READ_FHJU read_fhju;
    INT32U datatime_add;
    INT8U item_tmp[6] = {0};
    INT8U second = 0;
    INT8U year;
    INT8U month;
    INT8U day;
    INT8U hour;
    INT8U minute;
    
    os_get_datetime(&datetime);
    
    pFn = (tagFN_ITEM *)pRequest->fn;
    readC2 =  (READ_CONTEXT *)(pRequest->context);
    
    if(pRequest->item_count == 0) return 0;
    read_c1_time_out = read_c1_time_out - (system_get_tick10ms() - pRequest->recv_time_in_10ms);
    
    mem_cpy(td,readC2->Td,7);
    //read_fhju.data_midu = 0;
    mem_set(read_fhju.value,sizeof(READ_FHJU),0);
    require_len = get_fn_len(pRequest,pFn);
    if(require_len + pResp->pos > pResp->max_reply_pos) //超过帧长时要分帧
    {
        readC2->need_more_frame=TRUE;
        return 0;
    }
    
    //使用较大的缓冲区 pResp pRequest  pread 返回数据  pbuffer 抄读645报文的临时缓冲区 =<255字节 
    if(require_len < pRequest->frame_len)
    {
        pread = pResp->frame+pResp->pos+require_len;
        pbuffer =  pRequest->frame+pRequest->frame_len;
        left_len =  SIZE_OF_APP_PROTOCOL_FRAME - require_len;
    }
    else
    {
        pread = pRequest->frame+pRequest->frame_len;
        pbuffer =  pResp->frame+pResp->pos+require_len;
        left_len =  SIZE_OF_APP_PROTOCOL_FRAME - pRequest->frame_len;
    }
    
    require_len += pResp->pos;
    read_fhju.work_flag = readC2->work_flag;
    for(idx_item=pRequest->item_count;idx_item>0;idx_item--)
    {
        pFn = &((tagFN_ITEM*)(pRequest->fn))[idx_item-1];
        
        //内部数据项处理 特殊处理可以统一在app_prcess_inter_data处理
        if(pFn->item & 0x80000000)
        {
            app_prcess_inter_data(pRequest,pResp,pFn);
            (void)convert_645_to_376(pFn->format,pResp->frame + pResp->pos+pFn->offset,param);//param 可以用来传参数
            
            data_len +=  pFn->len;
            pResp->frame_len+=pFn->len;
            continue;
        }
        

        if(0xFF==pFn->num)
        {
            read_fhju.data_midu = trans_C2_curve_midu(readC2);
            get_read_param((READ_FHJU*)read_fhju.value,read_fhju.data_midu,readC2->Td[6],require_len - pResp->pos - 7,pFn->len);            
        }
        else
        {
            read_fhju.read_num = 1;
        }
        mem_cpy(read_fhju.start_time,readC2->Td,5);
        for(block_num=0;block_num<read_fhju.read_num;block_num++) 
        {
            if(2==readC2->work_flag)
            {
				#ifdef __PROVICE_NEIMENG__
                get_next_curve_td(td,TRUE); //获取曲线下一个时间点
                #else
                #endif
                if(block_num == 0)
                {
                    mem_cpy(td_first,td,7);
                    #ifdef __PROVICE_SHANGHAI__
                    if(readC2->work_flag == 0x03)
                    {
                        get_next_curve_td(td,TRUE);
                    }
                    #endif
                }
            } 

            //读取数据
            item = app_get_item_with_td(pFn->item,pFn->prop,td,readC2->work_flag);
            item_tmp[0] = read_fhju.read_point[block_num];
            to_bcd(item_tmp,1);
            mem_cpy(&item_tmp[1],read_fhju.start_time,5);
            if(read_fhju.data_midu)
            {
                if(block_num >0)
                {
                    mem_set(pResp->frame+pResp->pos+pFn->offset+read_fhju.offset[block_num]/read_fhju.data_midu,require_len-pResp->pos-read_fhju.offset[block_num]/read_fhju.data_midu,REC_DATA_IS_DEFAULT);
                }
                else
                {
                    mem_set(pResp->frame+pResp->pos+pFn->offset,require_len-pResp->pos,REC_DATA_IS_DEFAULT);
                }
            }
            else
            {
                mem_set(pResp->frame+pResp->pos+pFn->offset,pFn->len,REC_DATA_IS_DEFAULT); 
            }
            if(item!=0xFFFFFFFF)
            {
                res_len = app_read_his_item(item,pread,pbuffer,left_len,item_tmp,read_c1_time_out);
            }
            else
            {
                res_len = 0;
            }
            
            //电量计算
            if(((readC2->work_flag == 0x01)||(readC2->work_flag == 0x02))&&(res_len!=0))
            {
                if(readC2->work_flag == 0x01)
                {
                    mem_cpy(td_tmp,td,4);
                    previous_dayhold_td(td_tmp);
                }
                else
                {
                    mem_cpy(td_tmp,td,2);
                    previous_monthhold_td_BCD(td_tmp);
                }
                item = app_get_item_with_td(pFn->item,pFn->prop,td_tmp,readC2->work_flag);
                if(item!=0xFFFFFFFF)
                {
                    res_len_tmp = app_read_his_item(item,pread+res_len,pbuffer,left_len,item_tmp,read_c1_time_out);
                }
                else
                {
                    res_len_tmp = 0;
                }
                if(res_len_tmp!=0)
                {
                    for(idx=0;idx<5;idx++)
                    {
                        data_cur = bcd2u32(pread+idx*4,4,&is_valid1);     //当前值
                        data_rec = bcd2u32(pread+res_len+idx*4,4,&is_valid2);    //初始值
                        if((data_cur >= data_rec) && (is_valid1) && (is_valid2))
                        {
                            data_cur -= data_rec;
                            ul2bcd(data_cur,pread+idx*4,4);
                        }
                    }
                }
                else
                {
                    res_len = 0;//上一日、月无数据
                }
            }


            if(res_len)
            {
                if(pFn->format)// 非BCD判断？
                {
                    res_len = convert_645_to_376(pFn->format,pread,param);
                }
                if(res_len)
                {
                    if(!(NOT_FL_DATA & pFn->prop))
                    {
                        //todo  对数据内容BCD的判断     回复数据大于5字节的合法报文的处理？ 是否使用res_len回填
                        if(read_fhju.data_midu)
                        {
                            if(((pFn->item>>8) == 0x00061001) || ((pFn->item>>8) == 0x00061005))
                            {
                                for(idx_point=0;idx_point<read_fhju.read_point[block_num]/read_fhju.data_midu;idx_point++)
                                {
                                    if(2*idx_point*read_fhju.data_midu>=res_len)
                                    {
                                        mem_set(pResp->frame+pResp->pos+pFn->offset+read_fhju.offset[block_num]/read_fhju.data_midu+2*idx_point ,2, REC_DATA_IS_DEFAULT);
                                    }
                                    else if(check_is_all_FF(pread+2*read_fhju.data_midu*idx_point, 2))
                                    {
                                        mem_set(pResp->frame+pResp->pos+pFn->offset+read_fhju.offset[block_num]/read_fhju.data_midu+2*idx_point ,2, REC_DATA_IS_DEFAULT);
                                    }
                                    else
                                    {
                                        mem_cpy(pResp->frame+pResp->pos+pFn->offset+read_fhju.offset[block_num]/read_fhju.data_midu+2*idx_point,pread+2*read_fhju.data_midu*idx_point,2);
                                    }
                                }
                            }
                            else if(((pFn->item>>8) == 0x00061002) || ((pFn->item>>8) == 0x00061003) || ((pFn->item>>8) == 0x00061004) || ((pFn->item>>8) == 0x00061008))
                            {
                                for(idx_point=0;idx_point<read_fhju.read_point[block_num]/read_fhju.data_midu;idx_point++)
                                {
                                    if(3*idx_point*read_fhju.data_midu>=res_len)
                                    {
                                        mem_set(pResp->frame+pResp->pos+pFn->offset+read_fhju.offset[block_num]/read_fhju.data_midu+3*idx_point, 3, REC_DATA_IS_DEFAULT);
                                    }
                                    else if(check_is_all_FF(pread+3*read_fhju.data_midu*idx_point, 3))
                                    {
                                        mem_set(pResp->frame+pResp->pos+pFn->offset+read_fhju.offset[block_num]/read_fhju.data_midu+3*idx_point, 3, REC_DATA_IS_DEFAULT);
                                    }
                                    else
                                    {
                                        mem_cpy(pResp->frame+pResp->pos+pFn->offset+read_fhju.offset[block_num]/read_fhju.data_midu+3*idx_point,pread+3*read_fhju.data_midu*idx_point,3);
                                    }
                                }
                            }
                            else if(((pFn->item>>8) == 0x00061006) || ((pFn->item>>8) == 0x00061007))
                            {
                                for(idx_point=0;idx_point<read_fhju.read_point[block_num]/read_fhju.data_midu;idx_point++)
                                {
                                    if(4*idx_point*read_fhju.data_midu>=res_len)
                                    {
                                        mem_set(pResp->frame+pResp->pos+pFn->offset+read_fhju.offset[block_num]/read_fhju.data_midu+4*idx_point, 4, REC_DATA_IS_DEFAULT);
                                    }
                                    else if(check_is_all_FF(pread+4*read_fhju.data_midu*idx_point, 4))
                                    {
                                        mem_set(pResp->frame+pResp->pos+pFn->offset+read_fhju.offset[block_num]/read_fhju.data_midu+4*idx_point, 4, REC_DATA_IS_DEFAULT);
                                    }
                                    else
                                    {
                                        mem_cpy(pResp->frame+pResp->pos+pFn->offset+read_fhju.offset[block_num]/read_fhju.data_midu+4*idx_point,pread+4*read_fhju.data_midu*idx_point,4);
                                    }
                                }
                            }
                        }
                        else
                        {
                            mem_cpy(pResp->frame+pResp->pos+pFn->offset,pread,pFn->len);
                        }
                    }
                    else
                    {
                        mem_cpy(pResp->frame+pResp->pos+pFn->offset,pread,pFn->len);
                    }
                }
            }
            if(readC2->work_flag == 0x0A)
            {
                pResp->frame_len += pFn->len*read_fhju.read_point[block_num]/read_fhju.data_midu;
                data_len += pFn->len*read_fhju.read_point[block_num]/read_fhju.data_midu;
            }
            else
            {
                pResp->frame_len += pFn->len;
                data_len += pFn->len;
            }
            
            datatime_add = (INT32U)(read_fhju.read_point[block_num])*15*60;
            
            year = BCD2byte(read_fhju.start_time[4]);
            month = BCD2byte(read_fhju.start_time[3]);
            day = BCD2byte(read_fhju.start_time[2]);
            hour = BCD2byte(read_fhju.start_time[1]);
            minute = BCD2byte(read_fhju.start_time[0]);
            
            datetime_add_seconds(&year, &month, &day, &hour, &minute, &second, datatime_add);
        
            read_fhju.start_time[4] = byte2BCD(year);
            read_fhju.start_time[3] = byte2BCD(month);
            read_fhju.start_time[2] = byte2BCD(day);
            read_fhju.start_time[1] = byte2BCD(hour);
            read_fhju.start_time[0] = byte2BCD(minute);
        }
    }
    return data_len;
}



/**************
数据转换接口
 * format:  转换格式名
 * pResp：  数据转换缓冲区
 * param：  数据处理参数 无参数赋值为NULL
 * 返回值为数据长度
 * 要考虑回复了单费率数据的处理
**************/
INT16U   convert_645_to_376(INT16U format,INT8U *pResp,INT8U *param)
{
    INT16U len=0;
    INT8U data[50]={0};
    INT8U idx;
    
    if(!format)
    {
        return len;
    }
    //是否全部判断BCD 可能耽误时间？
    if(format & NOT_BCD)
    {

    }

    switch(format & 0x7FFF)
    {
        case REC_DATE:
            //set_event_datetime(pResp);
            break;

        case Energy_3761FMT14_BLOCK:
            for(idx=0;idx<5;idx++)
            {
                mem_cpy(data+1+idx*5,pResp+idx*4,4);
            }
            mem_cpy(pResp,data,25);
            len = 25;
            break;
        case DELEWEEK:
            mem_cpy(data,pResp+1,3);
            mem_cpy(pResp,data,3);
            len = 3;
            break;
        case JSJL_HSJ:
            mem_cpy(data,pResp+10,6);
			mem_cpy(pResp,data,6);
            len = 6;
            break;
//        case REC_DATE_SEC:
//            os_get_datetime(&datetime);
//            data[0] = byte2BCD(datetime.value[SECOND]);
//            mem_cpy(&data[1],pResp,5);
//            mem_cpy(pResp,data,6);
//            len = 6;
//            break;            
        case METER_STATUS:
            mem_cpy(data,meter_status,14);
            for(idx=0;idx<7;idx++)
            {
                if((!check_is_all_ch(pResp+idx+14,2,0xEE))&&(!check_is_all_ch(pResp+idx+14,2,0xFF))
                &&(data[idx]!=0xEE)&&(data[idx]!=0xFF)
                &&(data[idx+1]!=0xEE)&&(data[idx+1]!=0xFF))
                {
                    data[idx] ^= pResp[idx+14];
                    data[idx+1] ^= pResp[idx+1+14];
                }
                else
                {
                    mem_set(data+idx,2,REC_DATA_IS_DEFAULT);
                }
            }
            mem_cpy(pResp,data,14);
            mem_cpy(meter_status,pResp+14,14);
            len = 14;
            break;

                   
        default:
            break;


    }
    return len;

}

