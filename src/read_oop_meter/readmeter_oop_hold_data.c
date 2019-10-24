

#include "readmeter_oop_hold_data.h"
#include "gb_oop.h"
#include "../main_include.h"

#define READ_MASK_DAY_HOLD_DATA_SIZE    2
const INT8U READ_MASK_DAY_HOLD_DATA[2][READ_MASK_DAY_HOLD_DATA_SIZE] =
{
    {0x07,0x60},
    {0xFF,0x7F},
};
//每个数据块的长度不要超过249字节  264是否可行 TODO:????
// 存储格式 每个数据定义为
/*
 * 存储格式 每个数据定义为
 * flg 00 无数据  01  有数据 
 * 数据类型  + 内容  自解析获取
 * 比如 datetime_s  有数据  : 01+1c+7 总共占用9字节 
 */
const tagREADITEM  DAY_HOLD_DATA_READ_ITEM_LIST[] =
{
    //oad,           存储长度 ,  07抄表数据项,		存储偏移量, 数据项个数, 数据长度, 块长度, 存储起始page               占用page数量
    {0x20210200,    0x05060001,    0x05060001,          0,          1,        9,    111,  PAGE_ID_DAY_HOLD_1,  PAGE_COUNT_DAY_HOLD_1},      //(上1-62次)日冻结时间
    {0x00100200,    0x05060101,    0x05060101,          9,          5,       28,    111,  PAGE_ID_DAY_HOLD_1,  PAGE_COUNT_DAY_HOLD_1},      //正向有功电能总及费率
    {0x00200200,    0x05060201,    0x05060201,         37,          5,       28,    111,  PAGE_ID_DAY_HOLD_1,  PAGE_COUNT_DAY_HOLD_1},      //反向有功电能总及费率
    {0x20040200,    0x05061001,    0x05061001,         65,          4,       23,    111,  PAGE_ID_DAY_HOLD_1,  PAGE_COUNT_DAY_HOLD_1},      //(上1-62次)日冻结变量数据：有功功率（总及ABC）
    {0x20050200,    0xFFFFFFFF,    0xFFFFFFFF,         88,          4,       23,    111,  PAGE_ID_DAY_HOLD_1,  PAGE_COUNT_DAY_HOLD_1},      //(上1-62次)日冻结变量数据：无功功率（总及ABC） 该项不抄

    {0x00300200,    0x05060301,    0x05060301,          0,          5,       28,    168,  PAGE_ID_DAY_HOLD_2,  PAGE_COUNT_DAY_HOLD_2},      //组合无功1电能总及费率
    {0x00400200,    0x05060401,    0x05060401,         28,          5,       28,    168,  PAGE_ID_DAY_HOLD_2,  PAGE_COUNT_DAY_HOLD_2},      //组合无功2电能总及费率
    {0x00500200,    0x05060501,    0x05060501,         56,          5,       28,    168,  PAGE_ID_DAY_HOLD_2,  PAGE_COUNT_DAY_HOLD_2},      //第一象限无功电能总及费率
    {0x00600200,    0x05060601,    0x05060601,         84,          5,       28,    168,  PAGE_ID_DAY_HOLD_2,  PAGE_COUNT_DAY_HOLD_2},      //第二象限无功电能总及费率
    {0x00700200,    0x05060701,    0x05060701,        112,          5,       28,    168,  PAGE_ID_DAY_HOLD_2,  PAGE_COUNT_DAY_HOLD_2},      //第三象限无功电能总及费率
    {0x00800200,    0x05060801,    0x05060801,        140,          5,       28,    168,  PAGE_ID_DAY_HOLD_2,  PAGE_COUNT_DAY_HOLD_2},      //第四象限无功电能总及费率
    //最大需量  :
    {0x10100200,    0x05060901,    0x05060901,          0,          5,       78,    177,  PAGE_ID_DAY_HOLD_3,  PAGE_COUNT_DAY_HOLD_3},      //(上1-62次)日冻结正向有功最大需量及发生时间总及费率
    {0x10300200,    0x05060A01,    0x05060A01,         78,          5,       78,    177,  PAGE_ID_DAY_HOLD_3,  PAGE_COUNT_DAY_HOLD_3},      //(上1-62次)日冻结反向有功最大需量及发生时间总及费率
	// TODO:需要测试 暂时对应报文看起来不正确
    {0x202C0200,    0x05080201,    0x05080201,        156,          1,       15,    177,  PAGE_ID_DAY_HOLD_3,  PAGE_COUNT_DAY_HOLD_3},      //(上1-62次)日冻结剩余金额
    {0x202D0200,    0xFFFFFFFF,    0xFFFFFFFF,        171,          1,        6,    177,  PAGE_ID_DAY_HOLD_3,  PAGE_COUNT_DAY_HOLD_3},      //(上1-62次)日冻结透支金额
};

const tagREADITEM  MONTH_HOLD_DATA_READ_ITEM_LIST[] =
{
    //oad,           存储长度 ,  07抄表数据项, 存储偏移量, 数据项个数, 数据长度, 块长度, 存储起始page               占用page数量
    {0x20210200,    0x05060001,    0x05060001,          0,          1,        9,    111,  PAGE_ID_MONTH_HOLD_1,  PAGE_COUNT_MONTH_HOLD_1},      //(上1-62次)日冻结时间
    {0x00100200,    0x05060101,    0x05060101,          9,          5,       28,    111,  PAGE_ID_MONTH_HOLD_1,  PAGE_COUNT_MONTH_HOLD_1},      //正向有功电能总及费率
    {0x00200200,    0x05060201,    0x05060201,         37,          5,       28,    111,  PAGE_ID_MONTH_HOLD_1,  PAGE_COUNT_MONTH_HOLD_1},      //反向有功电能总及费率
    {0x20040200,    0x05061001,    0x05061001,         65,          4,       23,    111,  PAGE_ID_MONTH_HOLD_1,  PAGE_COUNT_MONTH_HOLD_1},      //(上1-62次)日冻结变量数据：有功功率（总及ABC）
    {0x20050200,    0xFFFFFFFF,    0xFFFFFFFF,         88,          4,       23,    111,  PAGE_ID_MONTH_HOLD_1,  PAGE_COUNT_MONTH_HOLD_1},      //(上1-62次)日冻结变量数据：无功功率（总及ABC） 该项不抄

    {0x00300200,    0x05060301,    0x05060301,          0,          5,       28,    168,  PAGE_ID_MONTH_HOLD_2,  PAGE_COUNT_MONTH_HOLD_2},      //组合无功1电能总及费率
    {0x00400200,    0x05060401,    0x05060401,         28,          5,       28,    168,  PAGE_ID_MONTH_HOLD_2,  PAGE_COUNT_MONTH_HOLD_2},      //组合无功2电能总及费率
    {0x00500200,    0x05060501,    0x05060501,         56,          5,       28,    168,  PAGE_ID_MONTH_HOLD_2,  PAGE_COUNT_MONTH_HOLD_2},      //第一象限无功电能总及费率
    {0x00600200,    0x05060601,    0x05060601,         84,          5,       28,    168,  PAGE_ID_MONTH_HOLD_2,  PAGE_COUNT_MONTH_HOLD_2},      //第二象限无功电能总及费率
    {0x00700200,    0x05060701,    0x05060701,        112,          5,       28,    168,  PAGE_ID_MONTH_HOLD_2,  PAGE_COUNT_MONTH_HOLD_2},      //第三象限无功电能总及费率
    {0x00800200,    0x05060801,    0x05060801,        140,          5,       28,    168,  PAGE_ID_MONTH_HOLD_2,  PAGE_COUNT_MONTH_HOLD_2},      //第四象限无功电能总及费率
    //最大需量需要修改 TODO:
    {0x10100200,    0x05060901,    0x05060901,          0,          5,       78,    156,  PAGE_ID_MONTH_HOLD_3,  PAGE_COUNT_MONTH_HOLD_3},      //(上1-62次)日冻结正向有功最大需量及发生时间总及费率
    {0x10200200,    0x05060A01,    0x05060A01,         78,          5,       78,    156,  PAGE_ID_MONTH_HOLD_3,  PAGE_COUNT_MONTH_HOLD_3},      //(上1-62次)日冻结反向有功最大需量及发生时间总及费率

    //{0x202C0200,    0x05080201,    0x05080201,         69,          1,       10,     79,  PAGE_ID_DAY_HOLD_1,  PAGE_COUNT_DAY_HOLD_1},      //(上1-62次)日冻结剩余金额
    //{0x202D0200,    0xFFFFFFFF,    0xFFFFFFFF,         75,          1,        4,     79,  PAGE_ID_DAY_HOLD_1,  PAGE_COUNT_DAY_HOLD_1},      //(上1-62次)日冻结透支金额
};

/* 无用?? */
const tagREADITEM  YEAR_HOLD_DATA_READ_ITEM_LIST[] =
{
    //oad,           存储长度 ,  07抄表数据项, 存储偏移量, 数据项个数, 数据长度, 块长度, 存储起始page               占用page数量
    {0xEEEEEEEE,    0x05060001,    0x05060001,          0,          1,        9,    111,  PAGE_ID_DAY_HOLD_1,  PAGE_COUNT_DAY_HOLD_1},      //
};

#define READ_MASK_HOUR_HOLD_DATA_SIZE    1
const INT8U READ_MASK_HOUR_HOLD_DATA[2][READ_MASK_HOUR_HOLD_DATA_SIZE] =
{
    {0x07},
    {0x07},
};
//每个数据块的长度不要超过249字节 
const tagREADITEM  HOUR_HOLD_DATA_READ_ITEM_LIST[] =
{
    //oad,      07抄表数据项块,  07抄表数据项, 存储偏移量, 数据项个数, 数据长度, 块长度, 存储起始page               占用page数量
    {0x20210200,    0x05040001,    0x05040001,          0,          1,        5,     13,  PAGE_ID_HOUR_HOLD,  PAGE_COUNT_HOUR_HOLD},      //(上1-62次)日冻结时间
    {0x00100200,    0x05040101,    0x05040101,          5,          1,        4,     13,  PAGE_ID_HOUR_HOLD,  PAGE_COUNT_HOUR_HOLD},      //正向有功电能总及费率
    {0x00200200,    0x05040201,    0x05040201,          9,          1,        4,     13,  PAGE_ID_HOUR_HOLD,  PAGE_COUNT_HOUR_HOLD},      //反向有功电能总及费率
		
};

#define READ_MASK_MINUTE_HOLD_DATA_SIZE    2
const INT8U READ_MASK_MINUTE_HOLD_DATA[2][READ_MASK_MINUTE_HOLD_DATA_SIZE] =
{
    {0xFF,0xFF},
    {0xFF,0xFF},
};
//每个数据块的长度不要超过249字节 
const tagREADITEM  MINUTE_HOLD_DATA_READ_ITEM_LIST[] =
{
    //oad,      07抄表数据项块,      07抄表数据项,   存储偏移量, 数据项个数, 数据长度, 块长度, 存储起始page               占用page数量
    {0x20210200,    0xFFFF0001,    0xFFFF0001,          0,          1,        9,    156,  PAGE_ID_MINUTE_HOLD,  PAGE_COUNT_MINUTE_HOLD},      //冻结时间    
    {0x00100200,    0x0001FF00,    0x00010000,          9,          5,       28,    156,  PAGE_ID_MINUTE_HOLD,  PAGE_COUNT_MINUTE_HOLD},      //正向有功电能总及费率
    {0x00200200,    0x0002FF00,    0x00020000,         37,          5,       28,    156,  PAGE_ID_MINUTE_HOLD,  PAGE_COUNT_MINUTE_HOLD},      //反向有功电能总及费率
    {0x20000200,    0x0201FF00,    0x02010100,         65,          3,       12,    156,  PAGE_ID_MINUTE_HOLD,  PAGE_COUNT_MINUTE_HOLD},      //电压
    {0x20010200,    0x0202FF00,    0x02020100,         77,          4,       18,    156,  PAGE_ID_MINUTE_HOLD,  PAGE_COUNT_MINUTE_HOLD},      //电流
//    {0x20010200,    0x02800001,    0x02800001,         95,          1,        3,    110,  PAGE_ID_MINUTE_HOLD,  PAGE_COUNT_MINUTE_HOLD},      //零线电流
    {0x20040200,    0x0203FF00,    0x02030000,         95,          4,       23,    156,  PAGE_ID_MINUTE_HOLD,  PAGE_COUNT_MINUTE_HOLD},      //有功功率
    {0x20050200,    0x0204FF00,    0x02040000,        118,          4,       23,    156,  PAGE_ID_MINUTE_HOLD,  PAGE_COUNT_MINUTE_HOLD},      //无功功率
    
//    {0x20060200,    0x0205FF00,    0x02050000,         87,          4,        3,    110,  PAGE_ID_MINUTE_HOLD,  PAGE_COUNT_MINUTE_HOLD},      //视在功率
//    {0x20070200,    0x02800003,    0x02800003,         99,          1,        3,    110,  PAGE_ID_MINUTE_HOLD,  PAGE_COUNT_MINUTE_HOLD},      //一分钟平均有功功率
    {0x200A0200,    0x0206FF00,    0x02060000,        141,          4,       15,    156,  PAGE_ID_MINUTE_HOLD,  PAGE_COUNT_MINUTE_HOLD},      //功率因数
};

#define READ_MASK_ACCOUNT_DAY_HOLD_DATA_SIZE    9
const INT8U READ_MASK_ACCOUNT_DAY_HOLD_DATA[2][READ_MASK_ACCOUNT_DAY_HOLD_DATA_SIZE] =
{
    {0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,},
    {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x3F,},
};
//每个数据块的长度不要超过249字节  结算日 抄表日 
const tagREADITEM  ACCOUNT_DAY_HOLD_DATA_READ_ITEM_LIST[] =
{
    //oad,      07抄表数据项块,  07抄表数据项, 存储偏移量, 数据项个数, 数据长度, 块长度, 存储起始page               占用page数量
    {0x20210200,    0x00FE0001,    0x00FE0001,          0,          1,        9,    121,  PAGE_ID_ACCOUNT_DAY_HOLD_1,  PAGE_COUNT_ACCOUNT_DAY_HOLD_1},      //(上1-12结算日)冻结时间
    //{0x00000200,    0x0000FF01,    0x00000001,          9,          5,       28,    225,  PAGE_ID_ACCOUNT_DAY_HOLD_1,  PAGE_COUNT_ACCOUNT_DAY_HOLD_1},      //组合有功电能总及费率
    {0x00100200,    0x0001FF01,    0x00010001,          9,          5,       28,    121,  PAGE_ID_ACCOUNT_DAY_HOLD_1,  PAGE_COUNT_ACCOUNT_DAY_HOLD_1},      //正向有功电能总及费率
    {0x00200200,    0x0002FF01,    0x00020001,         37,          5,       28,    121,  PAGE_ID_ACCOUNT_DAY_HOLD_1,  PAGE_COUNT_ACCOUNT_DAY_HOLD_1},      //反向有功电能总及费率
    {0x00300200,    0x0003FF01,    0x00030001,         65,          5,       28,    121,  PAGE_ID_ACCOUNT_DAY_HOLD_1,  PAGE_COUNT_ACCOUNT_DAY_HOLD_1},      //组合无功1电能总及费率
    {0x00400200,    0x0004FF01,    0x00040001,         93,          5,       28,    121,  PAGE_ID_ACCOUNT_DAY_HOLD_1,  PAGE_COUNT_ACCOUNT_DAY_HOLD_1},      //组合无功2电能总及费率

    #if 0
    {0x00500200,    0x0005FF01,    0x00050001,        105,          5,        4,    225,  PAGE_ID_ACCOUNT_DAY_HOLD_1,  PAGE_COUNT_ACCOUNT_DAY_HOLD_1},      //第一象限无功电能总及费率
    {0x00600200,    0x0006FF01,    0x00060001,        125,          5,        4,    225,  PAGE_ID_ACCOUNT_DAY_HOLD_1,  PAGE_COUNT_ACCOUNT_DAY_HOLD_1},      //第二象限无功电能总及费率
    
    {0x00700200,    0x0007FF01,    0x00070001,        145,          5,        4,    225,  PAGE_ID_ACCOUNT_DAY_HOLD_1,  PAGE_COUNT_ACCOUNT_DAY_HOLD_1},      //第三象限无功电能总及费率
    {0x00800200,    0x0008FF01,    0x00080001,        165,          5,        4,    225,  PAGE_ID_ACCOUNT_DAY_HOLD_1,  PAGE_COUNT_ACCOUNT_DAY_HOLD_1},      //第四象限无功电能总及费率
    {0x00900200,    0x0009FF01,    0x00090001,        185,          5,        4,    225,  PAGE_ID_ACCOUNT_DAY_HOLD_1,  PAGE_COUNT_ACCOUNT_DAY_HOLD_1},      //正向视在电能总及费率
    {0x00A00200,    0x000AFF01,    0x000A0001,        205,          5,        4,    225,  PAGE_ID_ACCOUNT_DAY_HOLD_1,  PAGE_COUNT_ACCOUNT_DAY_HOLD_1},      //反向视在电能总及费率
    {0x00110200,    0x00150001,    0x00150001,          0,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //A相正向有功电能
    {0x00120200,    0x00290001,    0x00290001,          4,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //B相正向有功电能
    {0x00130200,    0x003D0001,    0x003D0001,          8,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //C相正向有功电能
    {0x00210200,    0x00160001,    0x00160001,         12,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //A相反向有功电能
    
    {0x00220200,    0x002A0001,    0x002A0001,         16,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //B相反向有功电能
    {0x00230200,    0x003E0001,    0x003E0001,         20,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //C相反向有功电能
    {0x00310200,    0x00170001,	   0x00170001,         24,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},	    //A相组合无功1电能
    {0x00320200,    0x002B0001,	   0x002B0001,         28,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},	    //B相组合无功1电能
    {0x00330200,    0x003F0001,	   0x003F0001,         32,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},	    //C相组合无功1电能
    {0x00410200,    0x00180001,	   0x00180001,         36,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},	    //A相组合无功2电能
    {0x00420200,    0x002C0001,    0x002C0001,         40,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//B相组合无功2电能
    {0x00430200,    0x00400001,    0x00400001,         44,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//C相组合无功2电能
    
    {0x00510200,    0x00190001,    0x00190001,         48,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//A相第一象限无功电能
    {0x00520200,    0x002D0001,    0x002D0001,         52,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//B相第一象限无功电能
    {0x00530200,    0x00410001,    0x00410001,         56,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//C相第一象限无功电能
    {0x00610200,    0x001A0001,    0x001A0001,         60,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//A相第二象限无功电能
    {0x00620200,    0x002E0001,    0x002E0001,         64,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//B相第二象限无功电能
    {0x00630200,    0x00420001,    0x00420001,         68,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//C相第二象限无功电能
    {0x00710200,    0x001B0001,    0x001B0001,         72,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//A相第三象限无功电能
    {0x00720200,    0x002F0001,    0x002F0001,         76,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//B相第三象限无功电能
    
    {0x00730200,    0x00430001,    0x00430001,         80,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//C相第三象限无功电能
    {0x00810200,    0x001C0001,    0x001C0001,         84,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//A相第四象限无功电能
    {0x00820200,    0x00300001,    0x00300001,         88,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//B相第四象限无功电能
    {0x00830200,    0x00440001,    0x00440001,         92,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//C相第四象限无功电能
    {0x00910200,    0x001D0001,    0x001D0001,         96,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//A相正向视在电能
    {0x00920200,    0x00310001,    0x00310001,        100,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//B相正向视在电能
    {0x00930200,    0x00450001,    0x00450001,        104,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//C相正向视在电能
    {0x00A10200,    0x001E0001,    0x001E0001,        108,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//A相反向视在电能
    
    {0x00A20200,    0x00320001,    0x00320001,        112,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//B相反向视在电能
    {0x00A30200,    0x00460001,    0x00460001,        116,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},		//C相反向视在电能
    {0x01100200,    0x00810001,    0x00810001,        120,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //正向有功基波总电能
    {0x01110200,    0x00950001,    0x00950001,        124,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //A相正向有功基波电能
    {0x01120200,    0x00A90001,    0x00A90001,        128,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //B相正向有功基波电能
    {0x01130200,    0x00BD0001,    0x00BD0001,        132,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //C相正向有功基波电能
    {0x01200200,    0x00820001,    0x00820001,        136,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //反向有功基波总电能
    {0x01210200,    0x00960001,    0x00960001,        140,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //A相反向有功基波电能
    
    {0x01220200,    0x00AA0001,    0x00AA0001,        144,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //B相反向有功基波电能
    {0x01230200,    0x00BE0001,    0x00BE0001,        148,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //C相反向有功基波电能
    {0x02100200,    0x00830001,    0x00830001,        152,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //正向有功谐波总电能
    {0x02110200,    0x00970001,    0x00970001,        156,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //A相正向有功谐波电能
    {0x02120200,    0x00AB0001,    0x00AB0001,        160,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //B相正向有功谐波电能
    {0x02130200,    0x00BF0001,    0x00BF0001,        164,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //C相正向有功谐波电能
    {0x02200200,    0x00840001,    0x00840001,        168,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //反向有功谐波总电能
    {0x02210200,    0x00980001,    0x00980001,        172,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //A相反向有功谐波电能
    
    {0x02220200,    0x00AC0001,    0x00AC0001,        176,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //B相反向有功谐波电能
    {0x02230200,    0x00C00001,    0x00C00001,        180,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //C相反向有功谐波电能
    {0x03000200,    0x00850001,    0x00850001,        184,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //铜损有功总电能补偿量
    {0x03010200,    0x00990001,    0x00990001,        188,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //A相铜损有功电能补偿量
    {0x03020200,    0x00AD0001,    0x00AD0001,        192,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //B相铜损有功电能补偿量
    {0x03030200,    0x00C10001,    0x00C10001,        196,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //C相铜损有功电能补偿量
    {0x04000200,    0x00860001,    0x00860001,        200,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //铁损有功总电能补偿量
    {0x04010200,    0x009A0001,    0x009A0001,        204,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //A相铁损有功电能补偿量
    
    {0x04020200,    0x00AE0001,    0x00AE0001,        208,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //B相铁损有功电能补偿量
    {0x04030200,    0x00C20001,    0x00C20001,        212,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //C相铁损有功电能补偿量
    {0x05000200,    0x00800001,    0x00800001,        216,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //关联总电能
    {0x05010200,    0x00940001,    0x00940001,        220,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //A相关联电能
    {0x05020200,    0x00A80001,    0x00A80001,        224,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //B相关联电能
    {0x05030200,    0x00BC0001,    0x00BC0001,        228,          1,        4,    232,  PAGE_ID_ACCOUNT_DAY_HOLD_2,  PAGE_COUNT_ACCOUNT_DAY_HOLD_2},      //C相关联电能
    #endif
};

#define READ_MASK_CHANGE_HOLD_DATA_SIZE    2
const INT8U READ_MASK_CHANGE_HOLD_DATA[2][READ_MASK_CHANGE_HOLD_DATA_SIZE] =
{
    {0x07,0x00},
    {0xFF,0x1F},
};
//每个数据块的长度不要超过249字节       时区表冻结
const tagREADITEM  TIME_ZONE_HOLD_DATA_READ_ITEM_LIST[] =
{
    //oad,      07抄表数据项块,         07抄表数据项, 存储偏移量, 数据项个数, 数据长度, 块长度, 存储起始page               占用page数量
    {0x20210200,    0x05020001,    0x05020001,          0,          1,        5,    269,  PAGE_ID_TIME_ZONE_HOLD,  PAGE_COUNT_TIME_ZONE_HOLD},      //(上1-12结算日)冻结时间
    {0x00100200,    0x05020101,    0x05020101,          5,          5,        4,    269,  PAGE_ID_TIME_ZONE_HOLD,  PAGE_COUNT_TIME_ZONE_HOLD},      //正向有功电能总及费率
    {0x00200200,    0x05020201,    0x05020201,         25,          5,        4,    269,  PAGE_ID_TIME_ZONE_HOLD,  PAGE_COUNT_TIME_ZONE_HOLD},      //反向有功电能总及费率
    {0x00300200,    0x05020301,    0x05020301,         45,          5,        4,    269,  PAGE_ID_TIME_ZONE_HOLD,  PAGE_COUNT_TIME_ZONE_HOLD},      //组合无功1电能总及费率
    {0x00400200,    0x05020401,    0x05020401,         65,          5,        4,    269,  PAGE_ID_TIME_ZONE_HOLD,  PAGE_COUNT_TIME_ZONE_HOLD},      //组合无功2电能总及费率
    {0x00500200,    0x05020501,    0x05020501,         85,          5,        4,    269,  PAGE_ID_TIME_ZONE_HOLD,  PAGE_COUNT_TIME_ZONE_HOLD},      //第一象限无功电能总及费率
    {0x00600200,    0x05020601,    0x05020601,        105,          5,        4,    269,  PAGE_ID_TIME_ZONE_HOLD,  PAGE_COUNT_TIME_ZONE_HOLD},      //第二象限无功电能总及费率
    {0x00700200,    0x05020701,    0x05020701,        125,          5,        4,    269,  PAGE_ID_TIME_ZONE_HOLD,  PAGE_COUNT_TIME_ZONE_HOLD},      //第三象限无功电能总及费率
    
    {0x00800200,    0x05020801,    0x05020801,        145,          5,        4,    269,  PAGE_ID_TIME_ZONE_HOLD,  PAGE_COUNT_TIME_ZONE_HOLD},      //第四象限无功电能总及费率
    {0x10100200,    0x05020901,    0x05020901,        165,          5,        8,    269,  PAGE_ID_TIME_ZONE_HOLD,  PAGE_COUNT_TIME_ZONE_HOLD},      //正向有功最大需量及发生时间总及费率
    {0x10200200,    0x05020A01,    0x05020A01,        205,          5,        8,    269,  PAGE_ID_TIME_ZONE_HOLD,  PAGE_COUNT_TIME_ZONE_HOLD},      //反向有功最大需量及发生时间总及费率
    {0x20040200,    0x05021001,    0x05021001,        245,          4,        3,    269,  PAGE_ID_TIME_ZONE_HOLD,  PAGE_COUNT_TIME_ZONE_HOLD},      //(上1-2次)两套时区表切换变量数据：有功功率（总及ABC）
    {0x20050200,    0xFFFFFFFF,    0x05021001,        257,          4,        3,    269,  PAGE_ID_TIME_ZONE_HOLD,  PAGE_COUNT_TIME_ZONE_HOLD},      //(上1-2次)两套时区表切换变量数据：无功功率（总及ABC）
};

//每个数据块的长度不要超过293字节       日时段表切换冻结
const tagREADITEM  DAY_SEG_HOLD_DATA_READ_ITEM_LIST[] =
{
    //oad,      07抄表数据项块,  07抄表数据项, 存储偏移量, 数据项个数, 数据长度, 块长度, 存储起始page               占用page数量
    {0x20210200,    0x05030001,    0x05030001,          0,          1,        5,    269,  PAGE_ID_DAY_SEG_HOLD,  PAGE_COUNT_DAY_SEG_HOLD},      //(上1-12结算日)冻结时间
    {0x00100200,    0x05030101,    0x05030101,          5,          5,        4,    269,  PAGE_ID_DAY_SEG_HOLD,  PAGE_COUNT_DAY_SEG_HOLD},      //正向有功电能总及费率
    {0x00200200,    0x05030201,    0x05030201,         25,          5,        4,    269,  PAGE_ID_DAY_SEG_HOLD,  PAGE_COUNT_DAY_SEG_HOLD},      //反向有功电能总及费率
    {0x00300200,    0x05030301,    0x05030301,         45,          5,        4,    269,  PAGE_ID_DAY_SEG_HOLD,  PAGE_COUNT_DAY_SEG_HOLD},      //组合无功1电能总及费率
    {0x00400200,    0x05030401,    0x05030401,         65,          5,        4,    269,  PAGE_ID_DAY_SEG_HOLD,  PAGE_COUNT_DAY_SEG_HOLD},      //组合无功2电能总及费率
    {0x00500200,    0x05030501,    0x05030501,         85,          5,        4,    269,  PAGE_ID_DAY_SEG_HOLD,  PAGE_COUNT_DAY_SEG_HOLD},      //第一象限无功电能总及费率
    {0x00600200,    0x05030601,    0x05030601,        105,          5,        4,    269,  PAGE_ID_DAY_SEG_HOLD,  PAGE_COUNT_DAY_SEG_HOLD},      //第二象限无功电能总及费率
    {0x00700200,    0x05030701,    0x05030701,        125,          5,        4,    269,  PAGE_ID_DAY_SEG_HOLD,  PAGE_COUNT_DAY_SEG_HOLD},      //第三象限无功电能总及费率
    {0x00800200,    0x05030801,    0x05030801,        145,          5,        4,    269,  PAGE_ID_DAY_SEG_HOLD,  PAGE_COUNT_DAY_SEG_HOLD},      //第四象限无功电能总及费率
    {0x10100200,    0x05030901,    0x05030901,        165,          5,        8,    269,  PAGE_ID_DAY_SEG_HOLD,  PAGE_COUNT_DAY_SEG_HOLD},      //正向有功最大需量及发生时间总及费率
    {0x10200200,    0x05030A01,    0x05030A01,        205,          5,        8,    269,  PAGE_ID_DAY_SEG_HOLD,  PAGE_COUNT_DAY_SEG_HOLD},      //反向有功最大需量及发生时间总及费率
    {0x20040200,    0x05031001,    0x05031001,        245,          4,        3,    269,  PAGE_ID_DAY_SEG_HOLD,  PAGE_COUNT_DAY_SEG_HOLD},      //(上1-2次)两套时区表切换变量数据：有功功率（总及ABC）
    {0x20050200,    0xFFFFFFFF,    0x05031001,        257,          4,        3,    269,  PAGE_ID_DAY_SEG_HOLD,  PAGE_COUNT_DAY_SEG_HOLD},      //(上1-2次)两套时区表切换变量数据：无功功率（总及ABC）
};

//每个数据块的长度不要超过293字节       费率电价切换冻结
const tagREADITEM  FL_PRICE_HOLD_DATA_READ_ITEM_LIST[] =
{
    //oad,      07抄表数据项块,  07抄表数据项, 存储偏移量, 数据项个数, 数据长度, 块长度, 存储起始page               占用page数量
    {0x20210200,    0x05050001,    0x05050001,          0,          1,        5,    269,  PAGE_ID_FL_PRICE_HOLD,  PAGE_COUNT_FL_PRICE_HOLD},      //(上1-12结算日)冻结时间
    {0x00100200,    0x05050101,    0x05050101,          5,          5,        4,    269,  PAGE_ID_FL_PRICE_HOLD,  PAGE_COUNT_FL_PRICE_HOLD},      //正向有功电能总及费率
    {0x00200200,    0x05050201,    0x05050201,         25,          5,        4,    269,  PAGE_ID_FL_PRICE_HOLD,  PAGE_COUNT_FL_PRICE_HOLD},      //反向有功电能总及费率
    {0x00300200,    0x05050301,    0x05050301,         45,          5,        4,    269,  PAGE_ID_FL_PRICE_HOLD,  PAGE_COUNT_FL_PRICE_HOLD},      //组合无功1电能总及费率
    {0x00400200,    0x05050401,    0x05050401,         65,          5,        4,    269,  PAGE_ID_FL_PRICE_HOLD,  PAGE_COUNT_FL_PRICE_HOLD},      //组合无功2电能总及费率
    {0x00500200,    0x05050501,    0x05050501,         85,          5,        4,    269,  PAGE_ID_FL_PRICE_HOLD,  PAGE_COUNT_FL_PRICE_HOLD},      //第一象限无功电能总及费率
    {0x00600200,    0x05050601,    0x05050601,        105,          5,        4,    269,  PAGE_ID_FL_PRICE_HOLD,  PAGE_COUNT_FL_PRICE_HOLD},      //第二象限无功电能总及费率
    {0x00700200,    0x05050701,    0x05050701,        125,          5,        4,    269,  PAGE_ID_FL_PRICE_HOLD,  PAGE_COUNT_FL_PRICE_HOLD},      //第三象限无功电能总及费率
    {0x00800200,    0x05050801,    0x05050801,        145,          5,        4,    269,  PAGE_ID_FL_PRICE_HOLD,  PAGE_COUNT_FL_PRICE_HOLD},      //第四象限无功电能总及费率
    {0x10100200,    0x05050901,    0x05050901,        165,          5,        8,    269,  PAGE_ID_FL_PRICE_HOLD,  PAGE_COUNT_FL_PRICE_HOLD},      //正向有功最大需量及发生时间总及费率
    {0x10200200,    0x05050A01,    0x05050A01,        205,          5,        8,    269,  PAGE_ID_FL_PRICE_HOLD,  PAGE_COUNT_FL_PRICE_HOLD},      //反向有功最大需量及发生时间总及费率
    {0x20040200,    0x05051001,    0x05051001,        245,          4,        3,    269,  PAGE_ID_FL_PRICE_HOLD,  PAGE_COUNT_FL_PRICE_HOLD},      //(上1-2次)两套时区表切换变量数据：有功功率（总及ABC）
    {0x20050200,    0xFFFFFFFF,    0x05051001,        257,          4,        3,    269,  PAGE_ID_FL_PRICE_HOLD,  PAGE_COUNT_FL_PRICE_HOLD},      //(上1-2次)两套时区表切换变量数据：无功功率（总及ABC）
};

//每个数据块的长度不要超过290字节       费率电价切换冻结
const tagREADITEM  LADDER_HOLD_DATA_READ_ITEM_LIST[] =
{
    //oad,      07抄表数据项块,      07抄表数据项,    存储偏移量, 数据项个数, 数据长度, 块长度, 存储起始page               占用page数量
    {0x20210200,    0x05070001,    0x05070001,          0,          1,        5,    269,  PAGE_ID_LADDER_HOLD,  PAGE_COUNT_LADDER_HOLD},      //(上1-12结算日)冻结时间
    {0x00100200,    0x05070101,    0x05070101,          5,          5,        4,    269,  PAGE_ID_LADDER_HOLD,  PAGE_COUNT_LADDER_HOLD},      //正向有功电能总及费率
    {0x00200200,    0x05070201,    0x05070201,         25,          5,        4,    269,  PAGE_ID_LADDER_HOLD,  PAGE_COUNT_LADDER_HOLD},      //反向有功电能总及费率
    {0x00300200,    0x05070301,    0x05070301,         45,          5,        4,    269,  PAGE_ID_LADDER_HOLD,  PAGE_COUNT_LADDER_HOLD},      //组合无功1电能总及费率
    {0x00400200,    0x05070401,    0x05070401,         65,          5,        4,    269,  PAGE_ID_LADDER_HOLD,  PAGE_COUNT_LADDER_HOLD},      //组合无功2电能总及费率
    {0x00500200,    0x05070501,    0x05070501,         85,          5,        4,    269,  PAGE_ID_LADDER_HOLD,  PAGE_COUNT_LADDER_HOLD},      //第一象限无功电能总及费率
    {0x00600200,    0x05070601,    0x05070601,        105,          5,        4,    269,  PAGE_ID_LADDER_HOLD,  PAGE_COUNT_LADDER_HOLD},      //第二象限无功电能总及费率
    {0x00700200,    0x05070701,    0x05070701,        125,          5,        4,    269,  PAGE_ID_LADDER_HOLD,  PAGE_COUNT_LADDER_HOLD},      //第三象限无功电能总及费率
    {0x00800200,    0x05070801,    0x05070801,        145,          5,        4,    269,  PAGE_ID_LADDER_HOLD,  PAGE_COUNT_LADDER_HOLD},      //第四象限无功电能总及费率
    {0x10100200,    0x05070901,    0x05070901,        165,          5,        8,    269,  PAGE_ID_LADDER_HOLD,  PAGE_COUNT_LADDER_HOLD},      //正向有功最大需量及发生时间总及费率
    {0x10200200,    0x05070A01,    0x05070A01,        205,          5,        8,    269,  PAGE_ID_LADDER_HOLD,  PAGE_COUNT_LADDER_HOLD},      //反向有功最大需量及发生时间总及费率
    {0x20040200,    0x05071001,    0x05071001,        245,          4,        3,    269,  PAGE_ID_LADDER_HOLD,  PAGE_COUNT_LADDER_HOLD},      //(上1-2次)两套时区表切换变量数据：有功功率（总及ABC）
    {0x20050200,    0xFFFFFFFF,    0x05071001,        257,          4,        3,    269,  PAGE_ID_LADDER_HOLD,  PAGE_COUNT_LADDER_HOLD},      //(上1-2次)两套时区表切换变量数据：无功功率（总及ABC）
};

const READ_LIST_PROP hold_data_list[] =
{
    {(tagREADITEM*)MINUTE_HOLD_DATA_READ_ITEM_LIST,    (INT8U*)READ_MASK_MINUTE_HOLD_DATA, sizeof(MINUTE_HOLD_DATA_READ_ITEM_LIST)/sizeof(tagREADITEM),    READ_MASK_MINUTE_HOLD_DATA_SIZE, 0x5002,   0,},  //分钟冻结
    {(tagREADITEM*)HOUR_HOLD_DATA_READ_ITEM_LIST,      (INT8U*)READ_MASK_HOUR_HOLD_DATA,   sizeof(HOUR_HOLD_DATA_READ_ITEM_LIST)/sizeof(tagREADITEM),      READ_MASK_HOUR_HOLD_DATA_SIZE,   0x5003,  48,},  //小时冻结
    {(tagREADITEM*)DAY_HOLD_DATA_READ_ITEM_LIST,       (INT8U*)READ_MASK_DAY_HOLD_DATA,    sizeof(DAY_HOLD_DATA_READ_ITEM_LIST)/sizeof(tagREADITEM),       READ_MASK_DAY_HOLD_DATA_SIZE,    0x5004,   3,},  //日冻结
    {(tagREADITEM*)ACCOUNT_DAY_HOLD_DATA_READ_ITEM_LIST, (INT8U*)READ_MASK_ACCOUNT_DAY_HOLD_DATA, sizeof(ACCOUNT_DAY_HOLD_DATA_READ_ITEM_LIST)/sizeof(tagREADITEM), READ_MASK_ACCOUNT_DAY_HOLD_DATA_SIZE, 0x5005, 12,}, //结算日冻结
    {(tagREADITEM*)MONTH_HOLD_DATA_READ_ITEM_LIST,       (INT8U*)READ_MASK_DAY_HOLD_DATA,    sizeof(MONTH_HOLD_DATA_READ_ITEM_LIST)/sizeof(tagREADITEM),   READ_MASK_DAY_HOLD_DATA_SIZE,    0x5006,   3,},  // 月冻结
    {(tagREADITEM*)YEAR_HOLD_DATA_READ_ITEM_LIST,       (INT8U*)READ_MASK_DAY_HOLD_DATA,    sizeof(YEAR_HOLD_DATA_READ_ITEM_LIST)/sizeof(tagREADITEM),     READ_MASK_DAY_HOLD_DATA_SIZE,    0x5007,   3,},  // 年冻结
    //{(tagREADITEM*)TIME_ZONE_HOLD_DATA_READ_ITEM_LIST, (INT8U*)READ_MASK_CHANGE_HOLD_DATA, sizeof(TIME_ZONE_HOLD_DATA_READ_ITEM_LIST)/sizeof(tagREADITEM), READ_MASK_CHANGE_HOLD_DATA_SIZE, 0x5008,   2,},  //时区表切换冻结
    //{(tagREADITEM*)DAY_SEG_HOLD_DATA_READ_ITEM_LIST,   (INT8U*)READ_MASK_CHANGE_HOLD_DATA, sizeof(DAY_SEG_HOLD_DATA_READ_ITEM_LIST)/sizeof(tagREADITEM),   READ_MASK_CHANGE_HOLD_DATA_SIZE, 0x5009,   2,},  //日时段表切换冻结
    //{(tagREADITEM*)FL_PRICE_HOLD_DATA_READ_ITEM_LIST,  (INT8U*)READ_MASK_CHANGE_HOLD_DATA, sizeof(FL_PRICE_HOLD_DATA_READ_ITEM_LIST)/sizeof(tagREADITEM),  READ_MASK_CHANGE_HOLD_DATA_SIZE, 0x500A,   2,},  //费率电价切换冻结
    //{(tagREADITEM*)LADDER_HOLD_DATA_READ_ITEM_LIST,    (INT8U*)READ_MASK_CHANGE_HOLD_DATA, sizeof(LADDER_HOLD_DATA_READ_ITEM_LIST)/sizeof(tagREADITEM),    READ_MASK_CHANGE_HOLD_DATA_SIZE, 0x500B,   2,},  //阶梯切换冻结
};

#ifdef __PROVINCE_JIBEI_FUHEGANZHI__   //面向对象的数据交换扩展协议（适用于负荷感知数据召测）冀北别墅区项目扩展4600 OI。
const tagREADITEM  ELE_EQUIPMENT_HOLD_DATA_READ_ITEM_LIST[] =
{
    //oad,           存储长度 ,     扩展4600 OI,		存储偏移量, 数据项个数, 数据长度, 块长度, 存储起始page               占用page数量
    {0x20210200,    0x05060001,    0x46000500,          0,          1,        3,    33,  0,  MAX_SECTOR_ELECTRICAL_EQUIPMENT_MINUTE_DATA},      //冻结时间,最后一个字节用于说明电器设备运行状态。
    {0x20040200,    0x05060101,    0x46000600,          3,          1,       6,    33,  0,  MAX_SECTOR_ELECTRICAL_EQUIPMENT_MINUTE_DATA},      //有功功率
    {0x20050200,    0x05060201,    0x46000700,         9,          1,       6,      33,  0,  MAX_SECTOR_ELECTRICAL_EQUIPMENT_MINUTE_DATA},      //无功功率
    {0x20010200,    0x05061001,    0x46000800,         15,          1,       6,    33,  0,  MAX_SECTOR_ELECTRICAL_EQUIPMENT_MINUTE_DATA},      //电流
    {0x00100200,    0xFFFFFFFF,    0x46000900,         21,          1,       6,    33,  0,  MAX_SECTOR_ELECTRICAL_EQUIPMENT_MINUTE_DATA},      //正向有功
	{0x00100200,    0xFFFFFFFF,    0x46000A00,         27,          1,       6,    33,  0,  MAX_SECTOR_ELECTRICAL_EQUIPMENT_MINUTE_DATA},      //反向有功	
};



#endif

BOOLEAN oad_search_in_hold_list(INT8U list_idx,INT8U oad[4],tagREADITEM *read_item)
{
	INT8U  idx_list;
	INT16U sub_idx;
	INT16U sub_cnt;
	tagREADITEM *item; 
#ifdef __PROVINCE_JIBEI_FUHEGANZHI__	
	INT8U oad_tmp[4];
	if (list_idx!=0xFF)
	{
#endif
	idx_list = list_idx;
	//for(idx_list=0;idx_list<sizeof(hold_data_list)/sizeof(READ_LIST_PROP);idx_list++)
	{
		sub_cnt = hold_data_list[idx_list].read_item_count;
		item    = hold_data_list[idx_list].read_item_list;
		for(sub_idx=0;sub_idx<sub_cnt;sub_idx++)
		{
			if(item[sub_idx].oad == cosem_bin2_int32u(oad))
			{
				//
				//*page_id = item[sub_idx].page_id;
				mem_cpy((void *)read_item,(void *)&item[sub_idx],sizeof(tagREADITEM));
				return TRUE;
			}
		}
	}
#ifdef __PROVINCE_JIBEI_FUHEGANZHI__
	}
		for(sub_idx=0;sub_idx<6;sub_idx++)
		{
			mem_cpy_reverse(oad_tmp,(INT8U*)&(ELE_EQUIPMENT_HOLD_DATA_READ_ITEM_LIST[sub_idx].item_07),4);
			if((oad_tmp[0]==oad[0])&&(oad_tmp[1]==oad[1])&&(oad_tmp[2]==oad[2]))
			{
					mem_cpy((void *)read_item,(void *)&ELE_EQUIPMENT_HOLD_DATA_READ_ITEM_LIST[sub_idx],sizeof(tagREADITEM));
					return TRUE;
			}
			
		}
#endif

	return FALSE;
}
#ifdef	__PROVINCE_JIBEI_FUHEGANZHI__
BOOLEAN oad_search_in_hold_list_2(INT8U oad[4],tagREADITEM *read_item)
{
	INT8U  idx_list;
	INT16U sub_idx;
	INT16U sub_cnt;
	tagREADITEM *item; 
	INT8U oad_tmp[4];
	for(sub_idx=0;sub_idx<6;sub_idx++)
	{
		mem_cpy_reverse(oad_tmp,(INT8U*)&(ELE_EQUIPMENT_HOLD_DATA_READ_ITEM_LIST[sub_idx].item_07),4);
		if((oad_tmp[0]==oad[0])&&(oad_tmp[1]==oad[1])&&(oad_tmp[2]==oad[2]))
		{
			mem_cpy((void *)read_item,(void *)&ELE_EQUIPMENT_HOLD_DATA_READ_ITEM_LIST[sub_idx],sizeof(tagREADITEM));
			return TRUE;
		}
		
	}
	return FALSE;
}
#endif
void read_meter_fixed_time_interval_hold_data(INT16U obis,READ_PARAMS *read_param)
{
    INT8U* data_ptr;
    INT32U item,save_idx;
    CommandDate cmddate_meter,cmddate_td;
    INT16U meter_data_start_page_id;
    INT16U idx_item;
    INT8U result,len;
    INT8U idx_33;
    INT8U idx_tmp;
    INT8U idx_list = 0;
    INT8U min_td[5];
    METER_DOCUMENT meter_doc;
    HOLD_IDX all_hold_idx;
	INT8U oad_cnt;
	GROUP_OAD_PARAMS *grp_oad_params;
	INT8U *req_frm ;//[200] = {0};
	INT8U *resp_frm;//[500]  = {0};
	INT16U req_len  = 0;
	INT16S resp_len = 0;
	INT8U  resp_oad[32][4];
	DateTime dt;
	INT8U hold_list_idx = 0xFF;  //定位的是存储中的哪张表
	INT8U last_td[7] = {0};
	INT8U hold_td[7] = {0};

	/* find the table */
	for(idx_list=0;idx_list<sizeof(hold_data_list)/sizeof(READ_LIST_PROP);idx_list++)   //这里从大表中得到具体的位置
	{
		if(obis == hold_data_list[idx_list].obis )
		{
			hold_list_idx = idx_list;
		}
	}

	mem_cpy((void *)&meter_doc,(void *)&gmeter_doc,sizeof(METER_DOCUMENT));
	
    if (hold_list_idx >= HOLD_LIST_COUNT) return;
    if (isvalid_meter_addr(meter_doc.meter_no,TRUE) == FALSE) return;
    
    save_idx = 0;
    meter_data_start_page_id = PAGE_ID_METER_DATA_START;// +meter_list_idx*MAX_ONE_METER_DATA_PAGE_COUNT;
    read_param->start_page   = PAGE_ID_METER_DATA_START;
	req_frm  = read_param->send_frame;
	resp_frm = read_param->recv_frame;
    #if 1 //  test 
    tpos_datetime(&dt);
    #endif
	// 1
	//while(1)
	{
		if(get_plan_oad_params(read_param,obis,hold_list_idx))
		{
			/*  */
			grp_oad_params = (GROUP_OAD_PARAMS *)&(read_param->norml_task_save_params.group_oad_params);
			oad_cnt = grp_oad_params->oad_count;
			/* TODO: */
			switch(obis)
			{
				case 0x5002: /* 分钟冻结 */
				case 0x5003: /* 小时冻结 */
					//if(gTaskPlanAttribute[read_param->idx_tsk_attr].read_num)
					{
						//cosem_datetime_add_ti(read_param->last_td, gTaskPlanAttribute[read_param->idx_tsk_attr].acq_content);
					}
					set_cosem_datetime_s(last_td, gTaskPlanAttribute[read_param->idx_tsk_attr].last_cycle);
					set_cosem_datetime_s(hold_td, gTaskPlanAttribute[read_param->idx_tsk_attr].cycle_date);
					for(idx_tmp=0;idx_tmp<gTaskPlanAttribute[read_param->idx_tsk_attr].read_num;idx_tmp++)
					{
						cosem_datetime_add_ti(last_td, gTaskPlanAttribute[read_param->idx_tsk_attr].acq_content);
					}
					if(compare_string(last_td, hold_td, 7) < 0)
					{
						//mem_cpy(read_param->hold_td,last_td,7);
						req_len = make_oop_hold_frame(req_frm,NULL,meter_doc.meter_no,&(read_param->oad),oad_cnt,
			                    grp_oad_params->oad[0],TRUE,last_td,FALSE);
			            gTaskPlanAttribute[read_param->idx_tsk_attr].read_num++;
			        }
			        else
			        {
			        	gTaskPlanAttribute[read_param->idx_tsk_attr].read_num = 0;
			        	clr_process_mask(read_param);
			        	return;
			        }
					break;
				case 0x5004://日冻结: 添加时间
					set_cosem_datetime_s(read_param->hold_td, dt.value);
					req_len = make_oop_hold_frame(req_frm,NULL,meter_doc.meter_no,&(read_param->oad),oad_cnt,
			                    grp_oad_params->oad[0],TRUE,read_param->hold_td,FALSE);
			        
			        break;
			    case 0x5005: //结算日,结算日是否需要改成日冻结转存???
			    	set_cosem_datetime_s(read_param->hold_td, dt.value);
					req_len = make_oop_hold_frame(req_frm,NULL,meter_doc.meter_no,&(read_param->oad),oad_cnt,
			                    grp_oad_params->oad[0],TRUE,read_param->hold_td,FALSE);
			    	break;
			   	case 0x5006: //月冻结，已经是1日 
			    	set_cosem_datetime_s(read_param->hold_td, dt.value);
					req_len = make_oop_hold_frame(req_frm,NULL,meter_doc.meter_no,&(read_param->oad),oad_cnt,
			                    grp_oad_params->oad[0],TRUE,read_param->hold_td,FALSE);
			    	break;
			    default:// clr mask and return 
			    	clr_process_mask(read_param);
			    	return;
			}
			req_len = encode_readmeter_frame(req_frm,req_len);
			resp_len=app_trans_send_meter_frame(req_frm,req_len,resp_frm,800,5000);  //原来是300；
			resp_len = decode_readmeter_frame(resp_frm, resp_len);
			switch(obis)
			{
				case 0x5002: /* 分钟冻结 */
				case 0x5003: /* 小时冻结 */
					//if()
					{
						//clr_process_mask(read_param);
					}
					break;
				default:
					clr_process_mask(read_param);
					break;
			}
			
			if(resp_len > 0)
			{
				/*
				 * 获取响应报文
				 */
				//resp_len = 5;
				parse_save_oop_data(read_param,TRUE, resp_frm, resp_len);
			}
		}
		else
		{
			//break;
		}
	}
	return ;
}
#ifdef __PROVINCE_JIBEI_FUHEGANZHI__
void save_load_sensor_data(READ_PARAMS *read_param,DateTime *dt,INT8U *frame,INT16U frame_len);
void save_normal_list_load_sensor_data(READ_PARAMS *read_param,INT8U *frame,INT16U frame_len,DateTime *dt);
//INT8U load_buffer_send[300],load_buffer_recv[300];
READ_PARAMS load_sensor_read_param;
static INT8U  new_cycle =0;
// 		static INT8U equipment_mask[4]={0xFF,0xFF,0xFF,0xFF};
		
void load_sensor_data_read_store()  //负荷感知存储
{
	METER_DOCUMENT meter_doc;
	INT8U oad_data[24];
	static INT8U equipment_mask[4]={0},readMeterBegin =1;
	INT8U oad_idx =0,equip_cnt =0;
	INT8U resp_len =0,req_len =0;
	static  readmeterloop =0;
	DateTime dt;
	mem_set(&load_sensor_read_param,sizeof(READ_PARAMS),0x00); //将全局变量初始化为0；
	tpos_datetime(&dt);
	if((new_cycle != ((dt.minute-1)/15+1))&&(dt.minute>0))  //延迟1分钟进行数据召测,原来是15分钟 (((dt.minute-1) %15 ==0) &&(new_cycle != (dt.minute-1)/15+1))
	{
 		new_cycle = (dt.minute-1)/15+1;
		mem_set(equipment_mask,4,0xFF);
		readMeterBegin =0;

	}
	dt.minute -=1;	   //存储按照0,15,30,45 来存储。
	if((compare_string(equipment_mask,"\x00\x00\x00\x00",4) !=0) &&(readMeterBegin ==0)) //不等于0，说明还有设备的数据没有读回来。
	{
		for(equip_cnt =0;equip_cnt<30;equip_cnt++)
		{
			//if(equip_cnt == 30)
			//{
				//clr_bit_value(equipment_mask, 8, equip_cnt);
				//clr_bit_value(equipment_mask, 8, equip_cnt+1);
			//}
			if(get_bit_value(equipment_mask, 4, equip_cnt))
			{
				//clr_bit_value(equipment_mask, 8, equip_cnt);
				system_debug_info("\r\n*****************need to read sensor load**********************\r\n");
				if(equip_cnt <30)				
				{
					readMeterBegin =1;
					break;
				}
				else
				{
					clr_bit_value(equipment_mask, 8, equip_cnt);
					clr_bit_value(equipment_mask, 8, equip_cnt+1);
				}
			}	
		}
	
		if(equip_cnt<30) //说明需要去抄表了，表中带有30个设备的数据
		{
			for(oad_idx=0;oad_idx<6;oad_idx++)
			{
				mem_cpy_reverse(oad_data+4*oad_idx,&ELE_EQUIPMENT_HOLD_DATA_READ_ITEM_LIST[oad_idx].item_07,4);
				*(oad_data+4*oad_idx+3) =equip_cnt +1;
				mem_cpy(load_sensor_read_param.norml_task_save_params.group_oad_params.oad[oad_idx],oad_data+4*oad_idx,4);
				load_sensor_read_param.norml_task_save_params.group_oad_params.oad_count++;
				load_sensor_read_param.norml_task_save_params.group_oad_params.resp_byte_num[oad_idx] = ELE_EQUIPMENT_HOLD_DATA_READ_ITEM_LIST[oad_idx].data_len;
				mem_cpy(load_sensor_read_param.norml_task_save_params.group_oad_params.save_off[oad_idx],&ELE_EQUIPMENT_HOLD_DATA_READ_ITEM_LIST[oad_idx].offset,2);
				load_sensor_read_param.norml_task_save_params.group_oad_params.save_size  = ELE_EQUIPMENT_HOLD_DATA_READ_ITEM_LIST[oad_idx].block_data_len;
				load_sensor_read_param.norml_task_save_params.group_oad_params.page_id  = ELE_EQUIPMENT_HOLD_DATA_READ_ITEM_LIST[oad_idx].page_id +equip_cnt*MAX_SECTOR_ELECTRICAL_EQUIPMENT_MINUTE_DATA;
				load_sensor_read_param.norml_task_save_params.group_oad_params.page_cnt =ELE_EQUIPMENT_HOLD_DATA_READ_ITEM_LIST[oad_idx].page_count;
			}
//			load_sensor_read_param.hold_td
			load_sensor_read_param.start_page = SECTOR_ELECTRICAL_EQUIPMENT_MINUTE_DATA_START;
			req_len = make_oop_cur_frame(load_sensor_read_param.send_frame,gSystemInfo.meter_no,6, oad_data);
			//for(readmeterloop =0;readmeterloop<3;readmeterloop++)   //在对非侵入式数据进行读取的时候，有时候读不回来，需要重复读几次，这里最多读3次。
			//{
				//resp_len=app_trans_send_meter_frame(load_sensor_read_param.send_frame,req_len,load_sensor_read_param.recv_frame,300,5000);  //原来是300；
				//if(resp_len>0)
				//{
					//break;
				//}
//#ifdef DEBUG
			//system_debug_info("need to read more");
//#endif				
			//}
			
			resp_len=app_trans_send_meter_frame(load_sensor_read_param.send_frame,req_len,load_sensor_read_param.recv_frame,300,5000);  //原来是300；
			if(resp_len>0)
			{
				 clr_bit_value(equipment_mask, 8, equip_cnt);
				 readMeterBegin = 0;
				 readmeterloop =0;
			}
			else
			{
				if(readmeterloop++ >3)
				{
					readmeterloop =0;
					clr_bit_value(equipment_mask, 8, equip_cnt);
					
				}
				readMeterBegin = 0;
				return ;
			}
			save_load_sensor_data(&load_sensor_read_param,&dt,load_sensor_read_param.recv_frame,resp_len);
			system_debug_info("\r\n*****************need to read sensor load read ok **********************\r\n");
			return ;
		}

	}
	
}
void save_load_sensor_data(READ_PARAMS *read_param,DateTime *dt,INT8U *frame,INT16U frame_len)
{
	INT8U pos =0;
	INT8U type =0;
	pos = get_frame_header_length_gb_oop(frame);
	if(frame[pos++] == SERVER_APDU_GET)
	{
		type = frame[pos];// APDU
		switch(type)
		{
			case GET_RESPONSE_NORMAL:
			break;
			case GET_RESPONSE_NORMAL_LIST:
			save_normal_list_load_sensor_data(read_param,frame,frame_len,dt);
			break;
		}
	}
}
void save_normal_list_load_sensor_data(READ_PARAMS *read_param,INT8U *frame,INT16U frame_len,DateTime *dt)
{
	/*  */
	GROUP_OAD_PARAMS *params;
	INT8U *buf;
	INT32U save_idx;
	INT16U pos = 0;
	INT16U expect_len = 0;
	INT16U data_len = 0;
	INT16U data_offset = 0;
	INT16U offset = 0;
	INT16U save_len = 0;
	INT16U cp_len;
	INT8U  col_cnt = 0;
	INT8U  idx_col;
	INT8U  hold_time[7] = {0};
	HEADER_TD hdr_td;
	DateTime cur_dt;
	METER_DOCUMENT meter_doc;
	INT8U tmp[150] ={0} ;
	mem_cpy((void *)&meter_doc,(void *)&gmeter_doc,sizeof(METER_DOCUMENT));
	
	tpos_datetime(&cur_dt);
	

	
	params = &(read_param->norml_task_save_params.group_oad_params);
	buf = read_param->send_frame;/* 借用 不行再改掉 */

	
	pos = get_frame_header_length_gb_oop(frame);
	pos += 1; /* get response */
	pos += 2;  //apdu和piid
	col_cnt = frame[pos++];
	#ifdef __DEBUG_
	system_debug_data(frame+pos,frame_len-pos);
	#endif
	mem_set(buf,300,0xFF);
	/* 比对oad并拷贝对应数据 */
	for(idx_col=0;idx_col<col_cnt;idx_col++)
	{
		if(compare_string(frame+pos,params->oad[idx_col],4)!=0)
		{
			return;
		}
		pos += 4;
		if(frame[pos++] != 1)
		{
			continue;
		}
		/* 获取长度信息 */
		data_len = get_object_attribute_item_len(frame+pos,frame[pos]);
		expect_len = params->resp_byte_num[idx_col];
		offset     = bin2_int16u(params->save_off[idx_col]);
		if(data_len == 1)
		{
			/* 没有数据 */
			buf[offset] = 0xFF;
		}
		else// 处理正常
		{
			//
			if(data_len >(expect_len-1))
			{
				cp_len = expect_len - 1 ;
			}
			else
			{
				cp_len = data_len;
			}
			buf[offset] = 1;//有数据
			mem_cpy(buf+offset+1,frame+pos,cp_len);
		}
		if( save_len <= (offset+expect_len) )
		{
			save_len = offset + expect_len;
		}
		pos += data_len;
		//	    offset+=data_len;
	}
	//
	#ifdef __DEBUG_
	system_debug_data(buf, save_len);
	#endif
//	get_datetime_from_cosem(read_param->hold_td, hold_time);
	mem_cpy(hold_time,dt,7);
	save_idx = getPassedDays(2000+hold_time[YEAR],hold_time[MONTH],hold_time[DAY]);

	save_idx *= 96;
	save_idx += hold_time[HOUR] *4;
	save_idx += hold_time[MINUTE] /15;

	hdr_td.tsk_start_time[0] = DT_DATETIME_S;
	hdr_td.save_time[0]      = DT_DATETIME_S;
	hdr_td.success_time[0]   = DT_DATETIME_S;
//	mem_cpy(hdr_td.tsk_start_time+1,dt,7);
	set_cosem_datetime_s(hdr_td.tsk_start_time+1, dt);
	set_cosem_datetime_s(hdr_td.success_time+1, cur_dt.value);
//	mem_cpy(hdr_td.save_time+1,dt,7);
	set_cosem_datetime_s(hdr_td.save_time+1, dt);


	write_flash_data_from_save_idx(hdr_td, save_idx, data_offset, buf, save_len,
		params->save_size, read_param->start_page+params->page_id, params->page_cnt);
		read_flash_data_from_save_idx(save_idx,data_offset , tmp, 150,
				params->save_size, read_param->start_page+params->page_id,params->page_cnt);

	//write_flash_data(hdr_td,meter_doc.meter_no, data_offset, buf, save_len,
	//params->save_size, read_param->start_page+params->page_id, params->page_cnt);
}
#endif
#if 0
void read_meter_not_fixed_time_interval_hold_data(INT8U meter_list_idx,BOOLEAN is_patch,INT8U hold_list_idx)
{
    INT8U* data_ptr;
    INT32U item,save_idx;
    INT16U meter_data_start_page_id,idx_cycle;
    INT16S idx_item,idx_patch;
    INT8U mask_idx;
    INT8U result,len;
    INT8U idx_33,patch_count;
    HOLD_IDX all_hold_idx;

    if (meter_list_idx >= MAX_METER_COUNT) return;
    if (hold_list_idx >= HOLD_LIST_COUNT) return;
    if (MeterList[meter_list_idx].elmt.protocol != METERPROTOCOL_GB645_2007) return;
    if (isvalid_meter_addr(MeterList[meter_list_idx].elmt.meter_no,TRUE) == FALSE) return;

    meter_data_start_page_id = PAGE_ID_METER_DATA_START+meter_list_idx*MAX_ONE_METER_DATA_PAGE_COUNT;
    read_flash_data(MeterList[meter_list_idx].elmt.meter_no,0,all_hold_idx.value,sizeof(HOLD_IDX),sizeof(HOLD_IDX),
            PAGE_ID_HOLD_IDX+meter_data_start_page_id,PAGE_COUNT_HOLD_IDX);
    //save_idx = bin2_int32u(all_hold_idx.elmt.hold_save_idx[hold_list_idx]);
    mask_idx = MeterList[meter_list_idx].elmt.is_single_phase ? 0 : 1;
    patch_count = is_patch ? hold_data_list[hold_list_idx].patch_count : 1;

    for(idx_patch=patch_count;idx_patch>0;idx_patch--)
    {
        if (GRECRS485.is_break) break;
        save_idx = bin2_int32u(all_hold_idx.elmt.hold_save_idx[hold_list_idx]);
        for(idx_item=0;idx_item<hold_data_list[hold_list_idx].read_item_count;idx_item++)
        {                    
            if (GRECRS485.is_break) break;
            if (get_bit_value(hold_data_list[hold_list_idx].read_mask+mask_idx*hold_data_list[hold_list_idx].read_mask_size,hold_data_list[hold_list_idx].read_mask_size,idx_item))
            {
                if (hold_data_list[hold_list_idx].read_item_list[idx_item].item_07_block == 0xFFFFFFFF) continue;
                if (hold_data_list[hold_list_idx].read_item_list[idx_item].oad == 0x20210200) //冻结时间
                {
                    item = hold_data_list[hold_list_idx].read_item_list[idx_item].item_07_block+idx_patch-1;
                    #ifdef __SOFT_SIMULATOR__
                    snprintf(info,100,"*** item = %08X",item);
                    debug_println(info);
                    #endif
                    result = read_GB645_item(MeterList[meter_list_idx].elmt.meter_no,item,NULL,0,MeterList[meter_list_idx].elmt.baudrate,MeterList[meter_list_idx].elmt.protocol);
                    if (result == GB645_RESP_OK) //保存
                    {
                        data_ptr = GRECRS485.frame+POS_GB645_DATA;
                        len = GRECRS485.frame[POS_GB645_DLEN];
                        for(idx_33=0;idx_33<len;idx_33++) data_ptr[idx_33] -= 0x33;
                        if ((len > 4) && (bin2_int32u(data_ptr) == item))
                        {
                            data_ptr += 4;
                            len -= 4;
                            if (hold_list_idx == HOLD_ACCOUNT_DAY)
                            {
                                #ifdef __SOFT_SIMULATOR__
                                snprintf(info,100,"*** td = %02X-%02X-%02X %02X:%02X",data_ptr[3],data_ptr[2],data_ptr[1],data_ptr[0],0);
                                debug_println(info);
                                #endif

                                if ((BCD2byte(data_ptr[2]) == 0) || (BCD2byte(data_ptr[2]) > 12)) break;
                                if ((BCD2byte(data_ptr[1]) == 0) || (BCD2byte(data_ptr[1]) > 31)) break;
                                data_ptr--;
                                len++;
                                data_ptr[0] = 0;//补一个字节
                            }
                            else 
                            {
                                #ifdef __SOFT_SIMULATOR__
                                snprintf(info,100,"*** td = %02X-%02X-%02X %02X:%02X",data_ptr[4],data_ptr[3],data_ptr[2],data_ptr[1],data_ptr[0]);
                                debug_println(info);
                                #endif

                                if ((BCD2byte(data_ptr[3]) == 0) || (BCD2byte(data_ptr[3]) > 12)) break;
                                if ((BCD2byte(data_ptr[2]) == 0) || (BCD2byte(data_ptr[2]) > 31)) break;
                            }
                            if (len > hold_data_list[hold_list_idx].read_item_list[idx_item].item_count*hold_data_list[hold_list_idx].read_item_list[idx_item].data_len)
                            {
                                len = hold_data_list[hold_list_idx].read_item_list[idx_item].item_count*hold_data_list[hold_list_idx].read_item_list[idx_item].data_len;
                            }

                            if (is_patch)
                            {
                                for(idx_cycle=0;idx_cycle<hold_data_list[hold_list_idx].patch_count;idx_cycle++)
                                {
                                    read_flash_data_from_save_idx(save_idx-idx_cycle,MeterList[meter_list_idx].elmt.meter_no,
                                            hold_data_list[hold_list_idx].read_item_list[idx_item].offset,GRECRS485.frame,
                                            hold_data_list[hold_list_idx].read_item_list[idx_item].item_count*hold_data_list[hold_list_idx].read_item_list[idx_item].data_len,
                                            hold_data_list[hold_list_idx].read_item_list[idx_item].block_data_len,
                                            hold_data_list[hold_list_idx].read_item_list[idx_item].page_id+meter_data_start_page_id,
                                            hold_data_list[hold_list_idx].read_item_list[idx_item].page_count);
                                    if (compare_string(GRECRS485.frame,data_ptr,5) == 0) break;
                                }
                                if (idx_cycle < hold_data_list[hold_list_idx].patch_count)  save_idx = save_idx - idx_cycle;
                            }

                            read_flash_data_from_save_idx(save_idx,MeterList[meter_list_idx].elmt.meter_no,
                                    hold_data_list[hold_list_idx].read_item_list[idx_item].offset,GRECRS485.frame,
                                    hold_data_list[hold_list_idx].read_item_list[idx_item].item_count*hold_data_list[hold_list_idx].read_item_list[idx_item].data_len,
                                    hold_data_list[hold_list_idx].read_item_list[idx_item].block_data_len,
                                    hold_data_list[hold_list_idx].read_item_list[idx_item].page_id+meter_data_start_page_id,
                                    hold_data_list[hold_list_idx].read_item_list[idx_item].page_count);
                            if (compare_string(GRECRS485.frame,data_ptr,hold_data_list[hold_list_idx].read_item_list[idx_item].data_len) != 0)
                            {
                                save_idx++;
                                //写数据
                                write_flash_data_from_save_idx(save_idx,MeterList[meter_list_idx].elmt.meter_no,
                                    hold_data_list[hold_list_idx].read_item_list[idx_item].offset,data_ptr,len,
                                    hold_data_list[hold_list_idx].read_item_list[idx_item].block_data_len,
                                    hold_data_list[hold_list_idx].read_item_list[idx_item].page_id+meter_data_start_page_id,
                                    hold_data_list[hold_list_idx].read_item_list[idx_item].page_count);
                                int32u2_bin(save_idx,all_hold_idx.elmt.hold_save_idx[hold_list_idx]);
                                write_flash_data(MeterList[meter_list_idx].elmt.meter_no,0,all_hold_idx.value,sizeof(HOLD_IDX),sizeof(HOLD_IDX),
                                        PAGE_ID_HOLD_IDX+meter_data_start_page_id,PAGE_COUNT_HOLD_IDX);
                            }
                        }
                        else break;
                    }
                    else break;
                }
                else
                {
                    read_flash_data_from_save_idx(save_idx,MeterList[meter_list_idx].elmt.meter_no,
                            hold_data_list[hold_list_idx].read_item_list[idx_item].offset,GRECRS485.frame,
                            hold_data_list[hold_list_idx].read_item_list[idx_item].item_count*hold_data_list[hold_list_idx].read_item_list[idx_item].data_len,
                            hold_data_list[hold_list_idx].read_item_list[idx_item].block_data_len,
                            hold_data_list[hold_list_idx].read_item_list[idx_item].page_id+meter_data_start_page_id,
                            hold_data_list[hold_list_idx].read_item_list[idx_item].page_count);
                    if (check_is_all_ch(GRECRS485.frame,hold_data_list[hold_list_idx].read_item_list[idx_item].item_count*hold_data_list[hold_list_idx].read_item_list[idx_item].data_len,0xFF) == TRUE)
                    {
                        item = hold_data_list[hold_list_idx].read_item_list[idx_item].item_07_block+idx_patch-1;
                        #ifdef __SOFT_SIMULATOR__
                        snprintf(info,100,"*** item = %08X    idx_item = %d",item,idx_item);
                        debug_println(info);
                        #endif
                        result = read_GB645_item(MeterList[meter_list_idx].elmt.meter_no,item,NULL,0,MeterList[meter_list_idx].elmt.baudrate,MeterList[meter_list_idx].elmt.protocol);
                        if (result == GB645_RESP_OK) //保存
                        {
                            data_ptr = GRECRS485.frame+POS_GB645_DATA;
                            len = GRECRS485.frame[POS_GB645_DLEN];
                            for(idx_33=0;idx_33<len;idx_33++) data_ptr[idx_33] -= 0x33;
                            if ((len > 4) && (bin2_int32u(data_ptr) == item))
                            {
                                data_ptr += 4;
                                len -= 4;
                                if ((hold_data_list[hold_list_idx].read_item_list[idx_item].oad == 0x20040200)
                                && (hold_data_list[hold_list_idx].read_item_list[idx_item+1].oad == 0x20050200)
                                && (hold_data_list[hold_list_idx].read_item_list[idx_item+1].item_07_block == 0xFFFFFFFF))
                                {
                                    if (len > hold_data_list[hold_list_idx].read_item_list[idx_item].item_count*hold_data_list[hold_list_idx].read_item_list[idx_item].data_len*2)
                                    {
                                        len = hold_data_list[hold_list_idx].read_item_list[idx_item].item_count*hold_data_list[hold_list_idx].read_item_list[idx_item].data_len*2;
                                    }
                                }
                                else
                                {
                                    if (len > hold_data_list[hold_list_idx].read_item_list[idx_item].item_count*hold_data_list[hold_list_idx].read_item_list[idx_item].data_len)
                                    {
                                        len = hold_data_list[hold_list_idx].read_item_list[idx_item].item_count*hold_data_list[hold_list_idx].read_item_list[idx_item].data_len;
                                    }
                                }
                                write_flash_data_from_save_idx(save_idx,MeterList[meter_list_idx].elmt.meter_no,
                                        hold_data_list[hold_list_idx].read_item_list[idx_item].offset,data_ptr,len,
                                        hold_data_list[hold_list_idx].read_item_list[idx_item].block_data_len,
                                        hold_data_list[hold_list_idx].read_item_list[idx_item].page_id+meter_data_start_page_id,
                                        hold_data_list[hold_list_idx].read_item_list[idx_item].page_count);
                            }
                        }
                    }
                }
            }
        }
    }
}

//#if 0
INT8U find_hold_data_list(INT16U obis)
{
    INT8U idx;
    for(idx=0;idx<HOLD_LIST_COUNT;idx++)
    {
        if (hold_data_list[idx].obis == obis)
        {
            return idx;
        }
    }
    return 0xFF;
}

INT8U find_hold_item_list(INT8U hold_list_idx,OBJECT_ATTR_DESC* oad,INT8U mask_idx)
{
    INT16U idx_item;

    for(idx_item=0;idx_item<hold_data_list[hold_list_idx].read_item_count;idx_item++)
    {
        if (get_bit_value(hold_data_list[hold_list_idx].read_mask+mask_idx*hold_data_list[hold_list_idx].read_mask_size,hold_data_list[hold_list_idx].read_mask_size,idx_item) == FALSE) continue;

        if (((hold_data_list[hold_list_idx].read_item_list[idx_item].oad >> 16) == ((oad->elmt.object_id[0]<<8)+oad->elmt.object_id[1]))
        && (oad->elmt.attribute_id.elmt.attribute_idx == ((hold_data_list[hold_list_idx].read_item_list[idx_item].oad>>8) & 0xFF)))
        {
            return idx_item;
        }
    }
    return hold_data_list[hold_list_idx].read_item_count;
}

INT16U read_hold_data(INT8U meter_list_idx,INT8U hold_list_idx,INT8U* resp,INT16U max_resp_len,INT16U hold_obis,INT16U rsd_obis,RSD* rsd,INT8U* rcsd)
{
    OBJECT_ATTR_DESC* rcsd_oad;
    INT8U *count;
    INT32U begin_save_idx,end_save_idx,add_count;
    CommandDate cmd_date;
    INT16U meter_data_start_page_id;
    INT16U idx_item,pos,idx,obis,idx_count,len;
    INT16U count_row_pos;
    HOLD_IDX all_hold_idx;
    INT8U hold_date[5];
    INT8U buffer[60];
    INT8U oop_buffer[80];
    INT8U mask_idx,count_row;

    if (meter_list_idx >= MAX_METER_COUNT) return 0;
    if (hold_list_idx >= HOLD_LIST_COUNT) return 0;

    pos = 0;
    begin_save_idx = 0;
    end_save_idx = 0;
    add_count = 1;
    mem_set(hold_date,5,0xFF);
    mem_set(buffer,60,0xFF);
    
    meter_data_start_page_id = PAGE_ID_METER_DATA_START+meter_list_idx*MAX_ONE_METER_DATA_PAGE_COUNT;
    read_flash_data(MeterList[meter_list_idx].elmt.meter_no,0,all_hold_idx.value,sizeof(HOLD_IDX),sizeof(HOLD_IDX),
            PAGE_ID_HOLD_IDX+meter_data_start_page_id,PAGE_COUNT_HOLD_IDX);
    mask_idx = MeterList[meter_list_idx].elmt.is_single_phase ? 0 : 1;

    write_resp_data_a(meter_list_idx,resp,rcsd,(rcsd[0] * 5 + 1),max_resp_len);
    write_resp_data_b(meter_list_idx,resp,1,max_resp_len); //DAR
    count_row = 0;
    count_row_pos = link_layer_framing[meter_list_idx].pos+link_layer_framing[meter_list_idx].resp_pos;
    write_resp_data_b(meter_list_idx,resp,255,max_resp_len); //count_row,先写成FF

    if (rsd->chioce == 9)
    {
        begin_save_idx = bin2_int32u(all_hold_idx.elmt.hold_save_idx[hold_list_idx]);
        if ((hold_list_idx == HOLD_MINUTE) && (hold_obis == 0x5003))
        {
            begin_save_idx = begin_save_idx/4*4;
            begin_save_idx -= rsd->obis_2021.interval*4;
            end_save_idx = begin_save_idx+1;
        }
        else
        {
            begin_save_idx -= rsd->obis_2021.interval;
            end_save_idx = begin_save_idx+1;
        }
    }
    else if (rsd->chioce == 1)
    {
        if (rsd_obis == 0x2021)
        {
            if (hold_list_idx == HOLD_MINUTE)    //15分钟冻结
            {
                begin_save_idx = getPassedDays(rsd->obis_2021.begin_data.year,rsd->obis_2021.begin_data.month,rsd->obis_2021.begin_data.day);
                begin_save_idx *= 96;
                begin_save_idx += rsd->obis_2021.begin_data.hour*4;
                if (hold_obis == 0x5003)
                {

                }
                else
                {
                    begin_save_idx += rsd->obis_2021.begin_data.minute / 15;
                }
                end_save_idx = begin_save_idx+1;
            }
            else if (hold_list_idx == HOLD_HOUR)
            {
                begin_save_idx = getPassedDays(rsd->obis_2021.begin_data.year,rsd->obis_2021.begin_data.month,rsd->obis_2021.begin_data.day);
                begin_save_idx *= 24;
                begin_save_idx += rsd->obis_2021.begin_data.hour;
                end_save_idx = begin_save_idx+1;
            }
            else if (hold_list_idx == HOLD_DAY)
            {
                begin_save_idx = getPassedDays(rsd->obis_2021.begin_data.year,rsd->obis_2021.begin_data.month,rsd->obis_2021.begin_data.day);
                end_save_idx = begin_save_idx+1;
            }
            else
            {
                end_save_idx = bin2_int32u(all_hold_idx.elmt.hold_save_idx[hold_list_idx])+1;
                if (end_save_idx > hold_data_list[hold_list_idx].patch_count)
                {
                    begin_save_idx = end_save_idx - hold_data_list[hold_list_idx].patch_count;
                }
                else
                {
                    begin_save_idx = 0;
                }
            }
        }
        else
        {
            begin_save_idx = rsd->obis_2023.begin_data;
            end_save_idx = begin_save_idx+1;
        }
    }
    else if (rsd->chioce == 2)
    {
        if (rsd_obis == 0x2021)
        {
            if (hold_list_idx == HOLD_MINUTE)    //15分钟冻结
            {
                begin_save_idx = getPassedDays(rsd->obis_2021.begin_data.year,rsd->obis_2021.begin_data.month,rsd->obis_2021.begin_data.day);
                begin_save_idx *= 96;
                begin_save_idx += rsd->obis_2021.begin_data.hour * 4;
                if (hold_obis == 0x5003)
                {
                
                }
                else
                {
                    begin_save_idx += rsd->obis_2021.begin_data.minute / 15;
                }
                end_save_idx = getPassedDays(rsd->obis_2021.end_data.year,rsd->obis_2021.end_data.month,rsd->obis_2021.end_data.day);
                end_save_idx *= 96;
                end_save_idx += rsd->obis_2021.end_data.hour * 4;
                if (hold_obis == 0x5003)
                {
                
                }
                else
                {
                    end_save_idx += rsd->obis_2021.end_data.minute / 15;
                }
                switch(rsd->obis_2021.unit)
                {
                case 1: //分
                    if (hold_obis == 0x5003)
                        add_count = 4; 
                    else
                        add_count = rsd->obis_2021.interval/15;
                    break;
                case 2: //时
                    add_count = rsd->obis_2021.interval*4;
                    break;
                case 3: //日
                    add_count = rsd->obis_2021.interval*96;
                    break;
                case 4:
                case 5:
                    add_count = end_save_idx;
                    break;
                }
                if (add_count == 0) add_count = 1;
            }
            else if (hold_list_idx == HOLD_HOUR)
            {
                begin_save_idx = getPassedDays(rsd->obis_2021.begin_data.year,rsd->obis_2021.begin_data.month,rsd->obis_2021.begin_data.day);
                begin_save_idx *= 24;
                begin_save_idx += rsd->obis_2021.begin_data.hour;
                end_save_idx = getPassedDays(rsd->obis_2021.end_data.year,rsd->obis_2021.end_data.month,rsd->obis_2021.end_data.day);
                end_save_idx *= 24;
                end_save_idx += rsd->obis_2021.end_data.hour;
                switch(rsd->obis_2021.unit)
                {
                case 2: //时
                    add_count = rsd->obis_2021.interval;
                    break;
                case 3: //日
                    add_count = rsd->obis_2021.interval*24;
                    break;
                case 4:
                case 5:
                    add_count = end_save_idx;
                    break;
                }
                if (add_count == 0) add_count = 1;
            }
            else if (hold_list_idx == HOLD_DAY)
            {
                begin_save_idx = getPassedDays(rsd->obis_2021.begin_data.year,rsd->obis_2021.begin_data.month,rsd->obis_2021.begin_data.day);
                end_save_idx = getPassedDays(rsd->obis_2021.end_data.year,rsd->obis_2021.end_data.month,rsd->obis_2021.end_data.day);
            }
            else
            {
                end_save_idx = bin2_int32u(all_hold_idx.elmt.hold_save_idx[hold_list_idx])+1;
                if (end_save_idx > hold_data_list[hold_list_idx].patch_count)
                {
                    begin_save_idx = end_save_idx - hold_data_list[hold_list_idx].patch_count;
                }
                else
                {
                    begin_save_idx = 0;
                }
            }
        }
        else
        {
            begin_save_idx = rsd->obis_2023.begin_data;
            end_save_idx = rsd->obis_2023.end_data;
        }
    }

    while(begin_save_idx < end_save_idx)
    {
        //读取冻结时间
        read_flash_data_from_save_idx(begin_save_idx,MeterList[meter_list_idx].elmt.meter_no,
                hold_data_list[hold_list_idx].read_item_list[0].offset,hold_date,
                hold_data_list[hold_list_idx].read_item_list[0].item_count*hold_data_list[hold_list_idx].read_item_list[0].data_len,
                hold_data_list[hold_list_idx].read_item_list[0].block_data_len,
                hold_data_list[hold_list_idx].read_item_list[0].page_id+meter_data_start_page_id,
                hold_data_list[hold_list_idx].read_item_list[0].page_count);
        if (is_valid_bcd(hold_date,hold_data_list[hold_list_idx].read_item_list[0].data_len))
        {
            switch(hold_list_idx)
            {
            case HOLD_MINUTE:
            case HOLD_HOUR:
            case HOLD_DAY:
                break;
            default:
                if (rsd->chioce == 1)
                {
                    if (rsd_obis == 0x2021)
                    {
                        cmd_date.year = 2000 + BCD2byte(hold_date[4]);
                        cmd_date.month = BCD2byte(hold_date[3]);
                        cmd_date.day = BCD2byte(hold_date[2]);
                        cmd_date.hour = BCD2byte(hold_date[1]);
                        cmd_date.minute = BCD2byte(hold_date[0]);
                        if (commandDateCompare(&cmd_date,&(rsd->obis_2021.begin_data)) != 0)
                        {
                            begin_save_idx++;
                            continue;
                        }
                    }
                }
                else if (rsd->chioce == 2)
                {
                    if (rsd_obis == 0x2021)
                    {
                        cmd_date.year = 2000 + BCD2byte(hold_date[4]);
                        cmd_date.month = BCD2byte(hold_date[3]);
                        cmd_date.day = BCD2byte(hold_date[2]);
                        cmd_date.hour = BCD2byte(hold_date[1]);
                        cmd_date.minute = BCD2byte(hold_date[0]);
                        if ((commandDateCompare(&cmd_date,&(rsd->obis_2021.begin_data)) < 0) || (commandDateCompare(&cmd_date,&(rsd->obis_2021.end_data)) >= 0))
                        {
                            begin_save_idx++;
                            continue;
                        }
                    }
                }
                break;
            }
            for(idx=0;idx<rcsd[0];idx++)
            {
                rcsd_oad = (OBJECT_ATTR_DESC*)(rcsd+1+idx*5+1);
                obis = (rcsd_oad->elmt.object_id[0]<<8) + rcsd_oad->elmt.object_id[1];
                if (obis == 0x2021)
                {
                    len = set_oop_data_format(rcsd_oad,oop_buffer,hold_date,5);
                    if (len > 0)
                    {
                        write_resp_data_a(meter_list_idx,resp,oop_buffer,len,max_resp_len);
                    }
                    else
                    {
                        write_resp_data_b(meter_list_idx,resp,DT_NULL,max_resp_len);
                    }
                }
                else if (obis == 0x2023)
                {
                    len = set_oop_data_format(rcsd_oad,oop_buffer,(INT8U*)&begin_save_idx,4);
                    if (len > 0)
                    {
                        write_resp_data_a(meter_list_idx,resp,oop_buffer,len,max_resp_len);
                    }
                    else
                    {
                        write_resp_data_b(meter_list_idx,resp,DT_NULL,max_resp_len);
                    }
                }
                else
                {
                    idx_item = find_hold_item_list(hold_list_idx,rcsd_oad,mask_idx);
                    if (idx_item < hold_data_list[hold_list_idx].read_item_count)
                    {
                        read_flash_data_from_save_idx(begin_save_idx,MeterList[meter_list_idx].elmt.meter_no,
                                hold_data_list[hold_list_idx].read_item_list[idx_item].offset,buffer,
                                hold_data_list[hold_list_idx].read_item_list[idx_item].item_count*hold_data_list[hold_list_idx].read_item_list[idx_item].data_len,
                                hold_data_list[hold_list_idx].read_item_list[idx_item].block_data_len,
                                hold_data_list[hold_list_idx].read_item_list[idx_item].page_id+meter_data_start_page_id,
                                hold_data_list[hold_list_idx].read_item_list[idx_item].page_count);
                        if (get_oop_data_if_class_is_array(rcsd_oad))
                        {
                            if (rcsd_oad->elmt.index_id == 0)
                            {
                                pos = 0;
                                if (((rcsd_oad->elmt.object_id[0]<<8)+rcsd_oad->elmt.object_id[1]) == 0x202C) //这货是一个结构体，不是数组；而且645的格式是  XXXX  XXXXXX.XX XXXXXX.XX  购电次数 剩余金额 透支金额
                                {
                                    oop_buffer[pos++] = DT_STRUCTURE;
                                    oop_buffer[pos++] = 2;
                                    len = set_oop_data_format(rcsd_oad,oop_buffer+pos,buffer+2,4);
                                    if (len > 0) pos += len; else oop_buffer[pos++] = DT_NULL;
                                    len = set_oop_data_format(rcsd_oad,oop_buffer+pos,buffer,2);
                                    if (len > 0) pos += len; else oop_buffer[pos++] = DT_NULL;
                                }
                                else
                                {
                                    oop_buffer[pos++] = DT_ARRAY;
                                    count = oop_buffer+pos;
                                    oop_buffer[pos++] = 0;
                                    for(idx_count=0;idx_count<hold_data_list[hold_list_idx].read_item_list[idx_item].item_count;idx_count++)
                                    {
                                        len = set_oop_data_format(rcsd_oad,oop_buffer+pos,buffer+idx_count*hold_data_list[hold_list_idx].read_item_list[idx_item].data_len,hold_data_list[hold_list_idx].read_item_list[idx_item].data_len);
                                        if (len > 0)
                                        {
                                            pos += len;
                                            (*count)++;
                                        }
                                        else break;
                                    }
                                }
                                write_resp_data_a(meter_list_idx,resp,oop_buffer,pos,max_resp_len);
                            }
                            else if (((rcsd_oad->elmt.object_id[0]<<8)+rcsd_oad->elmt.object_id[1]) == 0x202C) //这货是一个结构体，不是数组；
                            {
                                if (rcsd_oad->elmt.index_id == 1) //剩余金额
                                {
                                    len = set_oop_data_format(rcsd_oad,oop_buffer,buffer+2,4);
                                }
                                else if (rcsd_oad->elmt.index_id == 2) //购电次数
                                {
                                    len = set_oop_data_format(rcsd_oad,oop_buffer,buffer,2);
                                }
                                else len = 0;
                                if (len > 0)
                                {
                                    write_resp_data_a(meter_list_idx,resp,oop_buffer,len,max_resp_len);
                                }
                                else
                                {
                                    write_resp_data_b(meter_list_idx,resp,DT_NULL,max_resp_len);
                                }
                            }
                            else if (rcsd_oad->elmt.index_id <= hold_data_list[hold_list_idx].read_item_list[idx_item].item_count)
                            {
                                len = set_oop_data_format(rcsd_oad,oop_buffer,buffer+(rcsd_oad->elmt.index_id-1)*hold_data_list[hold_list_idx].read_item_list[idx_item].data_len,hold_data_list[hold_list_idx].read_item_list[idx_item].data_len);
                                if (len > 0)
                                {
                                    write_resp_data_a(meter_list_idx,resp,oop_buffer,len,max_resp_len);
                                }
                                else
                                {
                                    write_resp_data_b(meter_list_idx,resp,DT_NULL,max_resp_len);
                                }
                            }
                            else
                            {
                                write_resp_data_b(meter_list_idx,resp,DT_NULL,max_resp_len);
                            }
                        }
                        else
                        {
                            len = set_oop_data_format(rcsd_oad,oop_buffer,buffer,hold_data_list[hold_list_idx].read_item_list[idx_item].data_len);
                            if (len > 0)
                            {
                                write_resp_data_a(meter_list_idx,resp,oop_buffer,len,max_resp_len);
                            }
                            else
                            {
                                write_resp_data_b(meter_list_idx,resp,DT_NULL,max_resp_len);
                            }
                        }
                    }
                    else
                    {
                        write_resp_data_b(meter_list_idx,resp,DT_NULL,max_resp_len);
                    }
                }
            }
            count_row++;
        }
        begin_save_idx += add_count;
    }

    if (link_layer_framing[meter_list_idx].flag.elmt.frame_flag)
    {
        write_resp_data_frame_buffer_pos(meter_list_idx,count_row_pos,&count_row,1);
    }
    else
    {
        mem_cpy(resp+count_row_pos,&count_row,1);
    }

    return pos;
}
#endif
//#endif //#ifdef __OOP_CJQ__

