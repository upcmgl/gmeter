#include "readmeter_oop_cur_data.h"
#include "gb_oop.h"
#include "../OOP/apdu_security.h"
#include "../main_include.h"
#if 1 //def __OOP_CJQ__

#define READ_MASK_CUR_DNL_SIZE    9
const INT8U READ_MASK_CUR_DNL[2][READ_MASK_CUR_DNL_SIZE] =
{
    {0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x1F},
};

//每个数据块的长度不要超过256字节
const tagREADITEM  READ_ITEM_LIST_CUR_DNL[] =
{
    //oad,      07抄表数据项块,    07抄表数据项,      存储偏移量, 数据项个数, 数据长度, 块长度, 存储起始page               占用page数量
    //
    {0x00000200,    0x0000FF00,    0x00000000,          0,          5,       28,     84,  PAGE_ID_CUR_DL_1,  PAGE_COUNT_CUR_DL_1},      //组合有功电能总及费率
    {0x00100200,    0x0001FF00,    0x00010000,         28,          5,       28,     84,  PAGE_ID_CUR_DL_1,  PAGE_COUNT_CUR_DL_1},      //正向有功电能总及费率
    {0x00200200,    0x0002FF00,    0x00020000,         56,          5,       28,     84,  PAGE_ID_CUR_DL_1,  PAGE_COUNT_CUR_DL_1},      //反向有功电能总及费率
    {0x00300200,    0x0003FF00,    0x00030000,          0,          5,       28,     84,  PAGE_ID_CUR_DL_2,  PAGE_COUNT_CUR_DL_2},      //组合无功1电能总及费率
    {0x00400200,    0x0004FF00,    0x00040000,         28,          5,       28,     84,  PAGE_ID_CUR_DL_2,  PAGE_COUNT_CUR_DL_2},      //组合无功2电能总及费率
    {0x00500200,    0x0005FF00,    0x00050000,         56,          5,       28,     84,  PAGE_ID_CUR_DL_2,  PAGE_COUNT_CUR_DL_2},      //第一象限无功电能总及费率
    {0x00600200,    0x0006FF00,    0x00060000,          0,          5,       28,     84,  PAGE_ID_CUR_DL_3,  PAGE_COUNT_CUR_DL_3},      //第二象限无功电能总及费率
    {0x00700200,    0x0007FF00,    0x00070000,         28,          5,       28,     84,  PAGE_ID_CUR_DL_3,  PAGE_COUNT_CUR_DL_3},      //第三象限无功电能总及费率
    
    {0x00800200,    0x0008FF00,    0x00080000,         56,          5,       28,     84,  PAGE_ID_CUR_DL_3,  PAGE_COUNT_CUR_DL_3},      //第四象限无功电能总及费率
    {0x00900200,    0x0009FF00,    0x00090000,          0,          5,        4,     82,  PAGE_ID_CUR_DL_4,  PAGE_COUNT_CUR_DL_4},      //正向视在电能总及费率
    {0x00A00200,    0x000AFF00,    0x000A0000,         20,          5,        4,     82,  PAGE_ID_CUR_DL_4,  PAGE_COUNT_CUR_DL_4},      //反向视在电能总及费率
    {0x00110200,    0x00150000,    0x00150000,         40,          1,        6,     82,  PAGE_ID_CUR_DL_4,  PAGE_COUNT_CUR_DL_4},      //A相正向有功电能
    {0x00120200,    0x00290000,    0x00290000,         46,          1,        6,     82,  PAGE_ID_CUR_DL_4,  PAGE_COUNT_CUR_DL_4},      //B相正向有功电能
    {0x00130200,    0x003D0000,    0x003D0000,         52,          1,        6,     82,  PAGE_ID_CUR_DL_4,  PAGE_COUNT_CUR_DL_4},      //C相正向有功电能
    {0x00210200,    0x00160000,    0x00160000,         58,          1,        6,     82,  PAGE_ID_CUR_DL_4,  PAGE_COUNT_CUR_DL_4},      //A相反向有功电能
    {0x00220200,    0x002A0000,    0x002A0000,         64,          1,        6,     82,  PAGE_ID_CUR_DL_4,  PAGE_COUNT_CUR_DL_4},      //B相反向有功电能
    
    {0x00230200,    0x003E0000,    0x003E0000,         70,          1,        6,     82,  PAGE_ID_CUR_DL_4,  PAGE_COUNT_CUR_DL_4},      //C相反向有功电能
    {0x00310200,    0x00170000,    0x00170000,         76,          1,        6,     82,  PAGE_ID_CUR_DL_4,  PAGE_COUNT_CUR_DL_4},      //A相组合无功1电能
    {0x00320200,    0x002B0000,    0x002B0000,          0,          1,        6,     30,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},      //B相组合无功1电能
    {0x00330200,    0x003F0000,    0x003F0000,          6,          1,        6,     30,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},      //C相组合无功1电能
    {0x00410200,    0x00180000,    0x00180000,         12,          1,        6,     30,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},      //A相组合无功2电能
    {0x00420200,    0x002C0000,    0x002C0000,         18,          1,        6,     30,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},      //B相组合无功2电能
    {0x00430200,    0x00400000,    0x00400000,         24,          1,        6,     30,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},		//C相组合无功2电能
    //{0x00510200,    0x00190000,    0x00190000,         20,          1,        4,     68,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},		//A相第一象限无功电能
    
    //{0x00520200,    0x002D0000,    0x002D0000,         24,          1,        4,     68,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},		//B相第一象限无功电能
    //{0x00530200,    0x00410000,    0x00410000,         28,          1,        4,     68,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},		//C相第一象限无功电能
    //{0x00610200,    0x001A0000,    0x001A0000,         32,          1,        4,     68,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},		//A相第二象限无功电能
    //{0x00620200,    0x002E0000,    0x002E0000,         36,          1,        4,     68,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},		//B相第二象限无功电能
    //{0x00630200,    0x00420000,    0x00420000,         40,          1,        4,     68,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},		//C相第二象限无功电能
    //{0x00710200,    0x001B0000,    0x001B0000,         44,          1,        4,     68,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},		//A相第三象限无功电能
    //{0x00720200,    0x002F0000,    0x002F0000,         48,          1,        4,     68,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},		//B相第三象限无功电能
    //{0x00730200,    0x00430000,    0x00430000,         52,          1,        4,     68,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},		//C相第三象限无功电能
    
    //{0x00810200,    0x001C0000,    0x001C0000,         56,          1,        4,     68,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},		//A相第四象限无功电能
    //{0x00820200,    0x00300000,    0x00300000,         60,          1,        4,     68,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},		//B相第四象限无功电能
    //{0x00830200,    0x00440000,    0x00440000,         64,          1,        4,     68,  PAGE_ID_CUR_DL_5,  PAGE_COUNT_CUR_DL_5},		//C相第四象限无功电能
    #if 0
    {0x00910200,    0x001D0000,    0x001D0000,          0,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},		//A相正向视在电能
    {0x00920200,    0x00310000,    0x00310000,          4,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},		//B相正向视在电能
    {0x00930200,    0x00450000,    0x00450000,          8,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},		//C相正向视在电能
    {0x00A10200,    0x001E0000,    0x001E0000,         12,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},		//A相反向视在电能
    {0x00A20200,    0x00320000,    0x00320000,         16,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},		//B相反向视在电能
    
    {0x00A30200,    0x00460000,    0x00460000,         20,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},		//C相反向视在电能
    {0x01100200,    0x00810000,    0x00810000,         24,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},      //正向有功基波总电能
    {0x01110200,    0x00950000,    0x00950000,         28,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},      //A相正向有功基波电能
    {0x01120200,    0x00A90000,    0x00A90000,         32,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},      //B相正向有功基波电能
    {0x01130200,    0x00BD0000,    0x00BD0000,         36,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},      //C相正向有功基波电能
    {0x01200200,    0x00820000,    0x00820000,         40,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},      //反向有功基波总电能
    {0x01210200,    0x00960000,    0x00960000,         44,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},      //A相反向有功基波电能
    {0x01220200,    0x00AA0000,    0x00AA0000,         48,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},      //B相反向有功基波电能
    
    {0x01230200,    0x00BE0000,    0x00BE0000,         52,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},      //C相反向有功基波电能
    {0x02100200,    0x00830000,    0x00830000,         56,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},      //正向有功谐波总电能
    {0x02110200,    0x00970000,    0x00970000,         60,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},      //A相正向有功谐波电能
    {0x02120200,    0x00AB0000,    0x00AB0000,         64,          1,        4,     68,  PAGE_ID_CUR_DL_6,  PAGE_COUNT_CUR_DL_6},      //B相正向有功谐波电能
    {0x02130200,    0x00BF0000,    0x00BF0000,          0,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //C相正向有功谐波电能
    {0x02200200,    0x00840000,    0x00840000,          4,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //反向有功谐波总电能
    {0x02210200,    0x00980000,    0x00980000,          8,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //A相反向有功谐波电能
    {0x02220200,    0x00AC0000,    0x00AC0000,         12,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //B相反向有功谐波电能
    
    {0x02230200,    0x00C00000,    0x00C00000,         16,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //C相反向有功谐波电能
    {0x03000200,    0x00850000,    0x00850000,         20,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //铜损有功总电能补偿量
    {0x03010200,    0x00990000,    0x00990000,         24,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //A相铜损有功电能补偿量
    {0x03020200,    0x00AD0000,    0x00AD0000,         28,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //B相铜损有功电能补偿量
    {0x03030200,    0x00C10000,    0x00C10000,         32,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //C相铜损有功电能补偿量
    {0x04000200,    0x00860000,    0x00860000,         36,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //铁损有功总电能补偿量
    {0x04010200,    0x009A0000,    0x009A0000,         40,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //A相铁损有功电能补偿量
    {0x04020200,    0x00AE0000,    0x00AE0000,         44,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //B相铁损有功电能补偿量
    
    {0x04030200,    0x00C20000,    0x00C20000,         48,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //C相铁损有功电能补偿量
    {0x05000200,    0x00800000,    0x00800000,         52,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //关联总电能
    {0x05010200,    0x00940000,    0x00940000,         56,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //A相关联电能
    {0x05020200,    0x00A80000,    0x00A80000,         60,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //B相关联电能
    {0x05030200,    0x00BC0000,    0x00BC0000,         64,          1,        4,     68,  PAGE_ID_CUR_DL_7,  PAGE_COUNT_CUR_DL_7},      //C相关联电能
    #endif
};
//需量类
#define READ_MASK_CUR_XL_SIZE    5
const INT8U READ_MASK_CUR_XL[2][READ_MASK_CUR_XL_SIZE] =
{
    {0x00,0x00,0x00,0x00,0x00},
    {0xFF,0xFF,0xFF,0xFF,0xFF},
};
const tagREADITEM READ_ITEM_LIST_CUR_XL[] =
{
    //oad,      07抄表数据项块,  07抄表数据项, 存储偏移量, 数据项个数, 数据长度, 块长度, 存储起始page               占用page数量
    {0x10100200,    0x0101FF00,    0x01010000,          0,          5,       78,    156,  PAGE_ID_CUR_XL_1,  PAGE_COUNT_CUR_XL_1},      //正向有功最大需量及发生时间（总及费率）
    {0x10200200,    0x0102FF00,    0x01020000,         78,          5,       78,    156,  PAGE_ID_CUR_XL_1,  PAGE_COUNT_CUR_XL_1},      //反向有功最大需量及发生时间（总及费率）

    {0x10300200,    0x0103FF00,    0x01030000,          0,          5,       78,    156,  PAGE_ID_CUR_XL_2,  PAGE_COUNT_CUR_XL_2},      //组合无功1最大需量及发生时间（总及费率）
    {0x10400200,    0x0104FF00,    0x01040000,         78,          5,       78,    156,  PAGE_ID_CUR_XL_2,  PAGE_COUNT_CUR_XL_2},      //组合无功2最大需量及发生时间（总及费率）

	#if 0
    {0x10500200,    0x0105FF00,    0x01050000,          0,          5,        8,     80,  PAGE_ID_CUR_XL_3,  PAGE_COUNT_CUR_XL_3},      //第一象限最大需量及发生时间（总及费率）
    {0x10600200,    0x0106FF00,    0x01060000,         40,          5,        8,     80,  PAGE_ID_CUR_XL_3,  PAGE_COUNT_CUR_XL_3},      //第二象限最大需量及发生时间（总及费率）

    {0x10700200,    0x0107FF00,    0x01070000,          0,          5,        8,     80,  PAGE_ID_CUR_XL_4,  PAGE_COUNT_CUR_XL_4},      //第三象限最大需量及发生时间（总及费率）
    {0x10800200,    0x0108FF00,    0x01080000,         40,          5,        8,     80,  PAGE_ID_CUR_XL_4,  PAGE_COUNT_CUR_XL_4},      //第四象限最大需量及发生时间（总及费率）

    {0x10900200,    0x0109FF00,    0x01090000,          0,          5,        8,     80,  PAGE_ID_CUR_XL_5,  PAGE_COUNT_CUR_XL_5},      //正向视在最大需量及发生时间（总及费率）
    {0x10A00200,    0x010AFF00,    0x010A0000,         40,          5,        8,     80,  PAGE_ID_CUR_XL_5,  PAGE_COUNT_CUR_XL_5},      //反向视在最大需量及发生时间（总及费率）

    {0x10110200,    0x01150000,    0x01150000,          0,          1,        8,     80,  PAGE_ID_CUR_XL_6,  PAGE_COUNT_CUR_XL_6},      //A相正向有功最大需量及发生时间
    {0x10120200,    0x01290000,    0x01290000,          8,          1,        8,     80,  PAGE_ID_CUR_XL_6,  PAGE_COUNT_CUR_XL_6},      //B相正向有功最大需量及发生时间
    {0x10130200,    0x013D0000,    0x013D0000,         16,          1,        8,     80,  PAGE_ID_CUR_XL_6,  PAGE_COUNT_CUR_XL_6},      //C相正向有功最大需量及发生时间
    {0x10210200,    0x01160000,    0x01160000,         24,          1,        8,     80,  PAGE_ID_CUR_XL_6,  PAGE_COUNT_CUR_XL_6},      //A相反向有功最大需量及发生时间
    {0x10220200,    0x012A0000,    0x012A0000,         32,          1,        8,     80,  PAGE_ID_CUR_XL_6,  PAGE_COUNT_CUR_XL_6},      //B相反向有功最大需量及发生时间
    {0x10230200,    0x013E0000,    0x013E0000,         40,          1,        8,     80,  PAGE_ID_CUR_XL_6,  PAGE_COUNT_CUR_XL_6},      //C相反向有功最大需量及发生时间
    {0x10310200,    0x01170000,    0x01170000,         48,          1,        8,     80,  PAGE_ID_CUR_XL_6,  PAGE_COUNT_CUR_XL_6},      //A相组合无功1最大需量及发生时间
    {0x10320200,    0x012B0000,    0x012B0000,         56,          1,        8,     80,  PAGE_ID_CUR_XL_6,  PAGE_COUNT_CUR_XL_6},      //B相组合无功1最大需量及发生时间
    {0x10330200,    0x013F0000,    0x013F0000,         64,          1,        8,     80,  PAGE_ID_CUR_XL_6,  PAGE_COUNT_CUR_XL_6},      //C相组合无功1最大需量及发生时间
    {0x10410200,    0x01180000,    0x01180000,         72,          1,        8,     80,  PAGE_ID_CUR_XL_6,  PAGE_COUNT_CUR_XL_6},      //A相组合无功2最大需量及发生时间

    {0x10420200,    0x012C0000,    0x012C0000,          0,          1,        8,     80,  PAGE_ID_CUR_XL_7,  PAGE_COUNT_CUR_XL_7},      //B相组合无功2最大需量及发生时间
    {0x10430200,    0x01400000,    0x01400000,          8,          1,        8,     80,  PAGE_ID_CUR_XL_7,  PAGE_COUNT_CUR_XL_7},      //C相组合无功2最大需量及发生时间
    {0x10510200,    0x01190000,    0x01190000,         16,          1,        8,     80,  PAGE_ID_CUR_XL_7,  PAGE_COUNT_CUR_XL_7},      //A相第一象限最大需量及发生时间
    {0x10520200,    0x012D0000,    0x012D0000,         24,          1,        8,     80,  PAGE_ID_CUR_XL_7,  PAGE_COUNT_CUR_XL_7},      //B相第一象限最大需量及发生时间
    {0x10530200,    0x01410000,    0x01410000,         32,          1,        8,     80,  PAGE_ID_CUR_XL_7,  PAGE_COUNT_CUR_XL_7},      //C相第一象限最大需量及发生时间
    {0x10610200,    0x011A0000,    0x011A0000,         40,          1,        8,     80,  PAGE_ID_CUR_XL_7,  PAGE_COUNT_CUR_XL_7},      //A相第二象限最大需量及发生时间
    {0x10620200,    0x012E0000,    0x012E0000,         48,          1,        8,     80,  PAGE_ID_CUR_XL_7,  PAGE_COUNT_CUR_XL_7},      //B相第二象限最大需量及发生时间
    {0x10630200,    0x01420000,    0x01420000,         56,          1,        8,     80,  PAGE_ID_CUR_XL_7,  PAGE_COUNT_CUR_XL_7},      //C相第二象限最大需量及发生时间
    {0x10710200,    0x011B0000,    0x011B0000,         64,          1,        8,     80,  PAGE_ID_CUR_XL_7,  PAGE_COUNT_CUR_XL_7},      //A相第三象限最大需量及发生时间
    {0x10720200,    0x012F0000,    0x012F0000,         72,          1,        8,     80,  PAGE_ID_CUR_XL_7,  PAGE_COUNT_CUR_XL_7},      //B相第三象限最大需量及发生时间

    {0x10730200,    0x01430000,    0x01430000,          0,          1,        8,     80,  PAGE_ID_CUR_XL_8,  PAGE_COUNT_CUR_XL_8},      //C相第三象限最大需量及发生时间
    {0x10810200,    0x011C0000,    0x011C0000,          8,          1,        8,     80,  PAGE_ID_CUR_XL_8,  PAGE_COUNT_CUR_XL_8},      //A相第四象限最大需量及发生时间
    {0x10820200,    0x01300000,    0x01300000,         16,          1,        8,     80,  PAGE_ID_CUR_XL_8,  PAGE_COUNT_CUR_XL_8},      //B相第四象限最大需量及发生时间
    {0x10830200,    0x01440000,    0x01440000,         24,          1,        8,     80,  PAGE_ID_CUR_XL_8,  PAGE_COUNT_CUR_XL_8},      //C相第四象限最大需量及发生时间
    {0x10910200,    0x011D0000,    0x011D0000,         32,          1,        8,     80,  PAGE_ID_CUR_XL_8,  PAGE_COUNT_CUR_XL_8},      //A相正向视在最大需量及发生时间
    {0x10920200,    0x01310000,    0x01310000,         40,          1,        8,     80,  PAGE_ID_CUR_XL_8,  PAGE_COUNT_CUR_XL_8},      //B相正向视在最大需量及发生时间
    {0x10930200,    0x01450000,    0x01450000,         48,          1,        8,     80,  PAGE_ID_CUR_XL_8,  PAGE_COUNT_CUR_XL_8},      //C相正向视在最大需量及发生时间
    {0x10A10200,    0x011E0000,    0x011E0000,         56,          1,        8,     80,  PAGE_ID_CUR_XL_8,  PAGE_COUNT_CUR_XL_8},      //A相反向视在最大需量及发生时间
    {0x10A20200,    0x01320000,    0x01320000,         64,          1,        8,     80,  PAGE_ID_CUR_XL_8,  PAGE_COUNT_CUR_XL_8},      //B相反向视在最大需量及发生时间
    {0x10A30200,    0x01460000,    0x01460000,         72,          1,        8,     80,  PAGE_ID_CUR_XL_8,  PAGE_COUNT_CUR_XL_8},      //C相反向视在最大需量及发生时间
    #endif
};
//变量类
#define READ_MASK_VARIABLE_SIZE    4
const INT8U READ_MASK_VARIABLE[2][READ_MASK_VARIABLE_SIZE] =
{
    {0xA7,0x03,0xC4,0x00,},
    {0xFF,0xFF,0xFF,0xFF,},
};
const tagREADITEM  READ_ITEM_LIST_VARIABLE[] =
{
    //oad,      07抄表数据项块,  07抄表数据项, 存储偏移量, 数据项个数, 数据长度, 块长度, 存储起始page               占用page数量
    {0x20000200,    0x0201FF00,    0x02010100,          0,          3,       12,    145,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //电压
    {0x20010200,    0x0202FF00,    0x02020100,         12,          4,       18,    145,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //电流
    //{0x20010200,    0x02800001,    0x02800001,         30,          1,        3,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //零线电流
    //{0x20020200,    0x0207FF00,    0x02070100,         18,          3,        2,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //电压相角
    //{0x20030200,    0x0207FF00,    0x02070100,         24,          3,        2,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //电流相角
    {0x20040200,    0x0203FF00,    0x02030000,         30,          4,       23,    145,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //有功功率
    {0x20050200,    0x0204FF00,    0x02040000,         53,          4,       23,    145,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //无功功率
    {0x20060200,    0x0205FF00,    0x02050000,         76,          4,       23,    145,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //视在功率
    //{0x20070200,    0x02800003,    0x02800003,         99,          1,        3,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //一分钟平均有功功率
    {0x200A0200,    0x0206FF00,    0x02060000,         99,          4,       15,    145,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //功率因数
    //{0x200B0200,    0x0208FF00,    0x02080100,         77,          3,        2,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //电压波形失真度
    //{0x200C0200,    0x0209FF00,    0x02090100,         83,          3,        2,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //电流波形失真度

	#if 0
    {0x200D0200,    0x020A01FF,    0x020A0101,          0,         21,        2,    126,  PAGE_ID_VARIABLE_2,  PAGE_COUNT_VARIABLE_2},      //A相电压谐波含有量（总及2…n次）
    {0x200D0300,    0x020A02FF,    0x020A0201,         42,         21,        2,    126,  PAGE_ID_VARIABLE_2,  PAGE_COUNT_VARIABLE_2},      //B相电压谐波含有量（总及2…n次）
    {0x200D0400,    0x020A03FF,    0x020A0301,         84,         21,        2,    126,  PAGE_ID_VARIABLE_2,  PAGE_COUNT_VARIABLE_2},      //C相电压谐波含有量（总及2…n次）
 
    {0x200E0200,    0x020B01FF,    0x020B0101,          0,         21,        2,    126,  PAGE_ID_VARIABLE_3,  PAGE_COUNT_VARIABLE_3},      //A相电流谐波含有量（总及2…n次）
    {0x200E0300,    0x020B02FF,    0x020B0201,         42,         21,        2,    126,  PAGE_ID_VARIABLE_3,  PAGE_COUNT_VARIABLE_3},      //B相电流谐波含有量（总及2…n次）
    {0x200E0400,    0x020B03FF,    0x020B0301,         84,         21,        2,    126,  PAGE_ID_VARIABLE_3,  PAGE_COUNT_VARIABLE_3},      //C相电流谐波含有量（总及2…n次）

    {0x200F0200,    0x02800002,    0x02800002,         89,          1,        2,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //电网频率
    {0x20100200,    0x02800007,    0x02800007,         91,          1,        2,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //表内温度
    {0x20110200,    0x02800008,    0x02800008,         93,          1,        2,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //时钟电池电压
    {0x20120200,    0x02800009,    0x02800009,         95,          1,        2,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //停电抄表电池电压
    {0x20130200,    0x0280000A,    0x0280000A,         97,          1,        4,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //时钟电池工作时间
	#endif
	{0x20140200,    0x040005FF,    0x04000501,        114,          7,       31,    145,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //电能表运行状态字 3+ 7*4
    //{0x20170200,    0x02800004,    0x02800004,        115,          1,        3,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //当前有功需量
    //{0x20180200,    0x02800005,    0x02800005,        118,          1,        3,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //当前无功需量
    //{0x20190200,    0x02800006,    0x02800006,        121,          1,        3,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //当前视在需量
    //{0x201A0200,    0x02800020,    0x02800020,        124,          1,        4,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //当前电价
    //{0x201B0200,    0x02800021,    0x02800021,        128,          1,        4,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //当前费率电价
    //{0x201C0200,    0x0280000B,    0x0280000B,        132,          1,        4,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //当前阶梯电价
    //{0x201D0200,    0x02800022,    0x02800022,        136,          1,        2,    138,  PAGE_ID_VARIABLE_1,  PAGE_COUNT_VARIABLE_1},      //安全认证剩余时长
};

//参变量类
#define READ_MASK_PARAMS_SIZE    2
const INT8U READ_MASK_PARAMS[2][READ_MASK_PARAMS_SIZE] =
{
    {0xFF,0x00,},
    {0xFF,0x00,},
};
const tagREADITEM  READ_ITEM_LIST_PARAMS[] =
{
    //oad,      07抄表数据项块,  07抄表数据项, 存储偏移量, 数据项个数, 数据长度, 块长度, 存储起始page               占用page数量
    {0x40000200,    0xFFFFFFFF,    0xFFFFFFFF,          0,          0,        9,     9,  PAGE_ID_PARAMS_1,  PAGE_COUNT_PARAMS_1},      //日期及星期   时间
    //{0x40010200,    0x04000401,    0x04000401,          0,          1,        6,     54,  PAGE_ID_PARAMS_1,  PAGE_COUNT_PARAMS_1},      //通信地址
    //{0x40020200,    0x04000402,    0x04000402,          6,          1,        6,     54,  PAGE_ID_PARAMS_1,  PAGE_COUNT_PARAMS_1},      //表号
    //{0x40030200,    0x0400040E,    0x0400040E,         12,          1,        6,     54,  PAGE_ID_PARAMS_1,  PAGE_COUNT_PARAMS_1},      //客户编号
    //{0xF0F0F0F0,    0x04001104,    0x04001104,         18,          1,        8,     54,  PAGE_ID_PARAMS_1,  PAGE_COUNT_PARAMS_1},      //主动上报模式字，oad是自己编的
    //{0x40040200,    0x0400040F,    0x0400040F,         26,          1,       11,     54,  PAGE_ID_PARAMS_1,  PAGE_COUNT_PARAMS_1},      //设备地理位置
};

const READ_LIST_PROP cur_data_list[] =
{
    {(tagREADITEM*)READ_ITEM_LIST_CUR_DNL,  (INT8U*)READ_MASK_CUR_DNL,  sizeof(READ_ITEM_LIST_CUR_DNL)/sizeof(tagREADITEM),  READ_MASK_CUR_DNL_SIZE,  0x0000,   0},
    {(tagREADITEM*)READ_ITEM_LIST_CUR_XL,   (INT8U*)READ_MASK_CUR_XL,   sizeof(READ_ITEM_LIST_CUR_XL)/sizeof(tagREADITEM),   READ_MASK_CUR_XL_SIZE,   0x1000,   0},
    {(tagREADITEM*)READ_ITEM_LIST_VARIABLE, (INT8U*)READ_MASK_VARIABLE, sizeof(READ_ITEM_LIST_VARIABLE)/sizeof(tagREADITEM), READ_MASK_VARIABLE_SIZE, 0x2000,   0},
    {(tagREADITEM*)READ_ITEM_LIST_PARAMS,   (INT8U*)READ_MASK_PARAMS,   sizeof(READ_ITEM_LIST_PARAMS)/sizeof(tagREADITEM),   READ_MASK_PARAMS_SIZE,   0x4000,   0},   //参变量类
};

BOOLEAN oad_search_in_cur_list(INT8U list_idx,INT8U oad[4],tagREADITEM *read_item)
{
	INT8U  idx_list;
	INT16U sub_idx;
	INT16U sub_cnt;
	tagREADITEM *item; 

	//idx_list = list_idx;
	for(idx_list=0;idx_list<sizeof(cur_data_list)/sizeof(READ_LIST_PROP);idx_list++)
	{
		sub_cnt = cur_data_list[idx_list].read_item_count;
		item    = cur_data_list[idx_list].read_item_list;
		for(sub_idx=0;sub_idx<sub_cnt;sub_idx++)
		{
			if(item[sub_idx].oad == cosem_bin2_int32u(oad))
			{
				mem_cpy((void *)read_item,(void *)&item[sub_idx],sizeof(tagREADITEM));
				return TRUE;
			}
		}
	}

	return FALSE;
}

//抄读电表实时数据
void read_meter_cur_data(INT16U obis,READ_PARAMS *read_param)
{
//    INT8U* data_ptr;
    //INT32U item;
    INT16U meter_data_start_page_id;
//    INT16U idx_item;
    INT8U mask_idx;
    INT8U result,len;//,idx_block;
//    INT8U idx_33;//item_block_idx,;
    METER_DOCUMENT meter_doc;
    INT8U *req_frm ;//[200] = {0};
	INT8U *resp_frm;//[500]  = {0};
	DateTime dt;
	INT16U req_len  = 0;
	INT16S resp_len = 0;
	GROUP_OAD_PARAMS *grp_oad_params;
	INT8U oad_cnt;
	
    mem_cpy((void *)&meter_doc,(void *)&gmeter_doc,sizeof(METER_DOCUMENT));
    if (isvalid_meter_addr(meter_doc.meter_no,TRUE) == FALSE) return;
    
    meter_data_start_page_id = PAGE_ID_METER_DATA_START;
    read_param->start_page   = PAGE_ID_METER_DATA_START;
	req_frm  = read_param->send_frame;
	resp_frm = read_param->recv_frame;
    
    tpos_datetime(&dt);

    //
    if(get_plan_oad_params(read_param,obis,0))
	{
		/*  */
		grp_oad_params = (GROUP_OAD_PARAMS *)&(read_param->norml_task_save_params.group_oad_params);
		oad_cnt = grp_oad_params->oad_count;
		/* TODO: */
		req_len = make_oop_cur_frame(req_frm, meter_doc.meter_no, oad_cnt, grp_oad_params->oad[0]);

		
		req_len = encode_readmeter_frame(req_frm,req_len);
		resp_len=app_trans_send_meter_frame(req_frm,req_len,resp_frm,800,5000);  //原来是300；
		resp_len = decode_readmeter_frame(resp_frm, resp_len);
		
		clr_process_mask(read_param);

		
		if(resp_len > 0)
		{
			/*
			 * 获取响应报文
			 */
			//resp_len = 5;
			parse_save_oop_data(read_param,FALSE, resp_frm, resp_len);
		}
	}
	return ;
}


#endif //#ifdef __OOP_CJQ__

