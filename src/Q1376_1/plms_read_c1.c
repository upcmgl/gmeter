/***************************************************************

    DL/T 1376.1  G表===主站下行报文处理任务

    请求1类数据 0x0C



***************************************************************/
#include "app_const_3761.h"
#include "../main_include.h"
#include "app_3761.h"
#include "ErtuParam.h"
#include "protocol_app_handler.h"


BOOLEAN get_read_c1data(objRequest *pRequest,objResponse *pResp,READ_CONTEXT *readC1);
INT16U app_get_fn_data_c1(objRequest *pRequest,objResponse *pResp);








/*****************************************************************
Fn数据结构表
typedef struct
{
    INT32U  item;         //数据项
    INT8U   offset;       //偏移量
    INT8U   len;          //数据长度 (上行协议长度1376.1)
    INT8U   num;          //块数据项个数 （暂时不需要使用）
    INT8U   prop;         //上N日字节位置  D0-D1（0：D0字节 MASK_DAYHOLD_D0 1 ：D1字节 2 ：D2字节
                            3：D4字节）
                            D2: 1 曲线数据  D3 非单相表数据项 NOT_SINGLE D3-D7空闲
    INT16U  format;       //转换函数 （07到1376.1转换函数）D15（1：非BCD 0：默认BCD）
                            NOT_BCD
}tagFN_ITEM;
******************************************************************/
const   tagFN_ITEM  C1_FN25_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	 //	终端抄表时间  最后
{	0x0203FF00,     5,	    12,	    1,	    0,                               0,},	 //	有功功率
{   0x0204FF00,     17,     12,     1,      NOT_SINGLE,                      0,},    //	无功功率
{   0x0206FF00,     29,     8,      1,      NOT_SINGLE,                      0,},    // 总功率因数
{   0x0201FF00,     37,     6,      1,      0,                               0,},    //电压
{   0x0202FF00,     43,     9,      1,      0,                               0,},    //电流
{   0x02800001,     52,     3,      1,      0,                               0,},    //零线电流
{   0x0205FF00,     55,     12,     1,      0,                               0,},    //视在功率
};

const   tagFN_ITEM  C1_FN27_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	 //	终端抄表时间  最后
{	0x04000102,     5,	    3,	    1,	    0,                               0,},	 //	电能表日历时钟
{   0x04000101,     8,      3,      1,      0,                               DELEWEEK,},    //电能表日历时钟
{   0x0280000A,     11,     4,      1,      0,                               0,},    // 电池工作时间
{   0x03300000,     15,     3,      1,      0,                               0,},    //编程总次数
{   0x03300001,     18,     6,      1,      0,                               0,},    //最近一次编程发生时刻
{   0x03300100,     24,     3,      1,      0,                               0,},    //电表清零总次数
{   0x03300101,     27,     6,      1,      0,                               0,},    //最近一次清零发生时刻
{   0x03300200,     33,     3,     1,       0,                               0,},    //需量清零总次数
{   0x03300201,     36,     6,     1,       0,                               0,},    //最近一次清零发生时刻
{   0x03300300,     42,     3,     1,       0,                               0,},    //事件清零总次数
{   0x03300301,     45,     6,     1,       0,                               0,},    //最近一次清零发生时刻
{   0x03300400,     51,     3,     1,       0,                               0,},    //校时总次数
{   0x03300401,     54,     6,     1,       0,                               JSJL_HSJ,},    //最近一次校时发生时刻
};

const   tagFN_ITEM  C1_FN28_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	 //	终端抄表时间  最后
{	0x80000009,     5,	    14,	    1,	    0,                               METER_STATUS,},	 //	电表运行状态字变位标志
{   0x040005FF,     19,     14,     1,       0,                               0,},    //电表运行状态字1
//{   0x04000502,     21,     2,     1,       0,                               0,},    //电表运行状态字2
//{   0x04000503,     23,     2,     1,       0,                               0,},    //电表运行状态字3
//{   0x04000504,     25,     2,     1,       0,                               0,},    //电表运行状态字4
//{   0x04000505,     27,     2,     1,       0,                               0,},    //电表运行状态字5
//{   0x04000506,     29,     2,     1,       0,                               0,},    //电表运行状态字6
//{   0x04000507,     31,     2,     1,       0,                               0,},    //电表运行状态字7
};
const   tagFN_ITEM  C1_FN33_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0001FF00,     6,      25,     1,      0,                               Energy_3761FMT14_BLOCK,},
{   0x0003FF00,     31,     20,     1,      NOT_SINGLE,                      0,},
{   0x0005FF00,     51,     20,     1,      NOT_SINGLE,                      0,},
{   0x0008FF00,     71,     20,     1,      NOT_SINGLE,                      0,},
};

const   tagFN_ITEM  C1_FN34_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0002FF00,     6,      25,     1,      0,                               Energy_3761FMT14_BLOCK,},
{   0x0004FF00,     31,     20,     1,      NOT_SINGLE,                      0,},
{   0x0006FF00,     51,     20,     1,      NOT_SINGLE,                      0,},
{   0x0007FF00,     71,     20,     1,      NOT_SINGLE,                      0,},
};

const   tagFN_ITEM  C1_FN41_ITEMS[] =
{
{	0x80000002,     0,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0001FF00,     1,      20,     1,      0,                               0,},
};

const   tagFN_ITEM  C1_FN42_ITEMS[] =
{
{	0x80000002,     0,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0003FF00,     1,      20,     1,      NOT_SINGLE,                      0,},
};

const   tagFN_ITEM  C1_FN43_ITEMS[] =
{
{	0x80000002,     0,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0002FF00,     1,      20,     1,      0,                               0,},
};

const   tagFN_ITEM  C1_FN44_ITEMS[] =
{
{	0x80000002,     0,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0004FF00,     1,      20,     1,      NOT_SINGLE,                      0,},
};

const   tagFN_ITEM  C1_FN45_ITEMS[] =
{
{	0x80000002,     0,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0001FF00,     1,      20,     1,      0,                               0,},
};

const   tagFN_ITEM  C1_FN46_ITEMS[] =
{
{	0x80000002,     0,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0003FF00,     1,      20,     1,      NOT_SINGLE,                      0,},
};

const   tagFN_ITEM  C1_FN47_ITEMS[] =
{
{	0x80000002,     0,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0002FF00,     1,      20,     1,      0,                               0,},
};

const   tagFN_ITEM  C1_FN48_ITEMS[] =
{
{	0x80000002,     0,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0004FF00,     1,      20,     1,      NOT_SINGLE,                      0,},
};

const   tagFN_ITEM  C1_FN105_ITEMS[] =
{
{	0x80000004,     0,	    2,	    1,	    0,                               0,},	//	小时冻结类数据时标Td_h
{   0x06100601,     1,      20,     1,      0,                               0,},   //小时冻结正向有功总电能量
};

const   tagFN_ITEM  C1_FN106_ITEMS[] =
{
{	0x80000004,     0,	    2,	    1,	    0,                               0,},	//	小时冻结类数据时标Td_h
{   0x06100603,     1,      20,     1,      NOT_SINGLE,                      0,},   //小时冻结正向无功总电能量
};

const   tagFN_ITEM  C1_FN107_ITEMS[] =
{
{	0x80000004,     0,	    2,	    1,	    0,                               0,},	//	小时冻结类数据时标Td_h
{   0x06100602,     1,      20,     1,      0,                               0,},   //小时冻结反向有功总电能量
};

const   tagFN_ITEM  C1_FN108_ITEMS[] =
{
{	0x80000004,     0,	    2,	    1,	    0,                               0,},	//	小时冻结类数据时标Td_h
{   0x06100604,     1,      20,     1,      NOT_SINGLE,                      0,},   //小时冻结反向无功总电能量
};

const   tagFN_ITEM  C1_FN109_ITEMS[] =
{
{	0x80000004,     0,	    2,	    1,	    0,                               0,},	//	小时冻结类数据时标Td_h
{   0x06100601,     1,      20,     1,      0,                               0,},   //小时冻结正向有功总电能示值
};

const   tagFN_ITEM  C1_FN110_ITEMS[] =
{
{	0x80000004,     0,	    2,	    1,	    0,                               0,},	//	小时冻结类数据时标Td_h
{   0x06100603,     1,      20,     1,      NOT_SINGLE,                      0,},   //小时冻结正向无功总电能示值
};

const   tagFN_ITEM  C1_FN111_ITEMS[] =
{
{	0x80000004,     0,	    2,	    1,	    0,                               0,},	//	小时冻结类数据时标Td_h
{   0x06100602,     1,      20,     1,      0,                               0,},   //小时冻结反向有功总电能示值
};

const   tagFN_ITEM  C1_FN112_ITEMS[] =
{
{	0x80000004,     0,	    2,	    1,	    0,                               0,},	//	小时冻结类数据时标Td_h
{   0x06100604,     1,      20,     1,      NOT_SINGLE,                      0,},   //小时冻结反向无功总电能示值
};

const   tagFN_ITEM  C1_FN129_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0001FF00,     6,      25,     1,      0,                               Energy_3761FMT14_BLOCK,},
};

const   tagFN_ITEM  C1_FN130_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0003FF00,     6,      20,     1,      NOT_SINGLE,                      0,},
};

const   tagFN_ITEM  C1_FN131_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0002FF00,     6,      25,     1,      0,                               Energy_3761FMT14_BLOCK,},
};

const   tagFN_ITEM  C1_FN132_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0004FF00,     6,      20,     1,      NOT_SINGLE,                      0,},
};

const   tagFN_ITEM  C1_FN133_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0005FF00,     6,      20,     1,      NOT_SINGLE,                      0,},
};

const   tagFN_ITEM  C1_FN134_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0006FF00,     6,      20,     1,      NOT_SINGLE,                      0,},
};

const   tagFN_ITEM  C1_FN135_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0007FF00,     6,      20,     1,      NOT_SINGLE,                      0,},
};

const   tagFN_ITEM  C1_FN136_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0008FF00,     6,      20,     1,      NOT_SINGLE,                      0,},
};

const   tagFN_ITEM  C1_FN137_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0001FF01,     6,      25,     1,      0,                               Energy_3761FMT14_BLOCK,},
};

const   tagFN_ITEM  C1_FN138_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0003FF01,     6,      20,     1,      0,                               0,},
};

const   tagFN_ITEM  C1_FN139_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0002FF01,     6,      25,     1,      0,                               Energy_3761FMT14_BLOCK,},
};

const   tagFN_ITEM  C1_FN140_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0004FF01,     6,      20,     1,      0,                               0,},
};

const   tagFN_ITEM  C1_FN141_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0005FF01,     6,      20,     1,      0,                               0,},
};

const   tagFN_ITEM  C1_FN142_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0006FF01,     6,      20,     1,      0,                               0,},
};

const   tagFN_ITEM  C1_FN143_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0007FF01,     6,      20,     1,      0,                               0,},
};

const   tagFN_ITEM  C1_FN144_ITEMS[] =
{
{	0x80000001,     0,	    5,	    1,	    0,                               0,},	//	终端抄表时间  最后
{	0x80000002,     5,	    1,	    1,	    0,                               RATE,},	//	费率数
{   0x0008FF01,     6,      20,     1,      0,                               0,},
};

const   tagFN_ITEM  C1_FN162_ITEMS[] =
{
{	0x80000001,     0,	    6,	    1,	    0,                               REC_DATE_SEC,},	 //	终端抄表时间  最后
{	0x04000102,     6,	    3,	    1,	    0,                               0,},	 //	电能表日历时钟
{   0x04000101,     9,      3,      1,      0,                               DELEWEEK,},    //电能表日历时钟
};

/*+++
   功能： 请求1类数据
   参数:
   下行报文：
         数据单元标识1
         ...
         数据单元标识n
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

void  process_frame_read(objRequest *pRequest,objResponse *pResp)
{
    READ_CONTEXT  readC1;
    INT16U pn_tmp;//临时保存PN
    INT16U pn_idx;//临时保存所有pn的索引

    pRequest->context = (INT8U *)&readC1;

    ClrTaskWdt();


    //如果不是主动上报,写入FLASH的情况
    if(pResp->channel!=CHANNEL_FLASH)
    {
         //设置控制码：DIR=1,PRM=0, FUNC=8  响应报文，用户数据
         pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_USERDATA;
    }


    //首帧标志置1
   readC1.is_first_frame = TRUE;

    //一个数据项目的多桢发送标志复位
    readC1.check_flag &= ~SENDFRAME_FLAG;

    //处理一帧报文前，先清除该标志，说明还未请求预抄数据项

    read_c1_time_out = TIME_OUT_C1;//超时时间初始化


    /////////////////////////////////
    //    开始组织响应帧
    /////////////////////////////////
    pRequest->pos = POS_DATA;

    while((pRequest->pos - POS_DATA) <  pRequest->userDataLen)
    {
        ClrTaskWdt();

        //提取数据单元标识
        readC1.pn = bin2_int16u(pRequest->frame +  pRequest->pos);
        readC1.fn = bin2_int16u(pRequest->frame +  pRequest->pos + 2);
        pRequest->pos += 4;

        if((readC1.check_flag & SENDFRAME_FLAG) ==0x00)
        {

           #ifdef __BATCH_TRANSPARENT_METER_TASK__
            if (readC1.fn == DT_F306)
            {
                mem_set(readC1.Td,7,0);     //利用这个分帧
            }
            #endif

             pn_tmp=readC1.pn;
             pn_idx=0;//初始化测量点序号为0

            while(get_next_pn(pn_tmp,&(readC1.pn),&pn_idx))
            {
                if(FALSE == get_read_c1data(pRequest,pResp,&readC1) )
                {
                    if(readC1.is_first_frame)
                    {
                        app_send_NAK(pRequest,pResp);
                    }
                    return;
                }
            }

        }
        //如果请求数据单元有数据

       //复位多桢发送标志
       readC1.check_flag &= ~SENDFRAME_FLAG;
    }
    if((pResp->pos == POS_DATA)  && (readC1.is_first_frame == TRUE))
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
  功能：读取制定PNFN的1类数据
  参数：
        PLMS_RESP   *resp        应答报文
        INT16U      *resp_pos    应答报文的起始位置
        READ_CONTEXT    *readC1     读取数据的访问结构
  返回：
        TRUE  / FALSE
  描述:
       1) 先按照不拆分PN,FN进行响应
       2) 如果需要分祯,则按照最小PiFi重新进行响应
       3) 分帧处理两种方式 ：
         a）  fn读取处理前判断预计长度是否应该分帧，如果需要分帧，置位readC1->need_more_frame，返回长度为0. 对于单个fn长度可能分帧的情况一定要在fn函数内判断分帧。
        if(fn的长度 + pResp->pos > pResp->max_reply_pos)
        {
                ((READ_CONTEXT *)(pRequest->context))->need_more_frame=TRUE;
                return 0;
        }
         b） 对单个fn长度不会导致分帧的情况，可以fn内不判断分帧，返回数据后判断分帧，回退datalen+4。但是不能处理单pnFn长度就分帧的情况，会回复否认。（不推荐）
---*/
BOOLEAN get_read_c1data(objRequest *pRequest,objResponse *pResp,READ_CONTEXT *readC1)
{
    extern INT16U read_C1_F1(INT8U *resp);
    extern INT16U read_c1_f14(INT8U *buffer);
    INT16U   pi,fi,fi_idx;
    INT16U   pngroup[64];
    INT16U   fngroup[8];
    INT16U   fn_mask;
    INT16U   datalen;
    INT8U    fng_idx;
    INT16U   png_idx,pngroupcount;
    INT8U    fngroupcount;
    METER_CLASS_PARAM meter_class;//大小类
    #if defined(__REDUCE_FLOW__)
    INT8U    check_pos,check_len,check_idx;  // 检查是否为EE，为EE则抛弃数据
    #endif
    BOOLEAN  is_pifi_mode;
    #if defined(__REDUCE_FLOW__)
    BOOLEAN  check_flag;
    #endif
    #if defined(__QNMDW_FK2012__)
    BOOLEAN  is_plc;
    #endif
    //(READ_CONTEXT *)(pRequest->context);

    ClrTaskWdt();
    #if defined(__REDUCE_FLOW__)
    check_pos = 0;
    check_len = 0;  //为0的时候不检查数据是否EE
    #endif
    //read_c1_time_out = TIME_OUT_C1;

    //初始按照整体PNFN响应
    is_pifi_mode = FALSE;
GET_READ_C1DATA_AGAIN:

    meter_class.value = 0;

    //信息点集合处理,处理时遵循先Fn，后Pn,从小到大顺序。
    pngroupcount = parse_pn_to_pngroup(readC1->pn,pngroup);
    fngroupcount = parse_fn_to_fngroup(readC1->fn,fngroup);


   #if defined(__REDUCE_FLOW__)
   is_pifi_mode = TRUE;
   #endif

   //if((pngroupcount==1) && (fngroupcount==1)) is_pifi_mode = TRUE;
   is_pifi_mode = TRUE;

    //填写应答报文的数据标识
    //pos = pResp->pos;
    if(FALSE == is_pifi_mode)
    {
        readC1->pos_pnfn = pResp->pos;
        set_pn_fn(pResp->frame, &pResp->pos, readC1->pn, readC1->fn);
    }

    readC1->need_more_frame = FALSE;
//    if((readC1->pn == 0xFF00)||((gAppInfo.hubei_801 == 1)&&(readC1->pn == 0xFFFF)))
//    {
//        pngroupcount = MAX_METER_COUNT;  //todo 目前仅支持了255个测量点
//    }
   //循环处理PN,从小到大的顺序
    for(png_idx=0;png_idx < pngroupcount;png_idx++)
    {

        pi = pngroup[png_idx];
      
        if(pi != DA_P0)// && readC1->pn!=0xFFFF)//Pn==0xFFFF为全部测量点，内蒙古用来标识所有测量点，2011.4.27 by zyg
        {
            //readC1->meter_seq = trans_pn_2_meter_seq(pi);

            //readC1->pn_idx = pi;
            //fread_array(readC1->meter_seq,PIM_METER_DOC,(INT8U *)&(readC1->meter_doc),sizeof(METER_DOCUMENT)); //todo未查到档案 返回ERR
            //if(COMMPORT_485_CY == readC1->meter_doc.baud_port.port)
            //{
            //   mem_set(readC1->meter_doc.meter_no,6,0xAA);
           //    readC1->meter_doc.protocol = GB645_2007;
            //}

        }
        else
        {
            //readC1->meter_seq = 1;   //这个可以兼用作分帧处理的起始值
        }

      //循环处理FN,从小到大的顺序
      for(fng_idx=0;fng_idx<fngroupcount;fng_idx++)
      {
         fi = fngroup[fng_idx];
         readC1->work_flag = 0;

         if(is_pifi_mode==TRUE)
         {
            set_pn_fn(pResp->frame, &pResp->pos, pi, fi);
         }

         //当前剩余可用的响应空间
         readC1->left_resp_len = pResp->max_reply_pos - pResp->pos;
         readC1->need_more_frame = FALSE;

         //判断fn是否预抄  默认F38配置了或者多pn和多fn时预抄
         fi_idx = trans_set_fn_2_fnidx(fi);
         //meter_class.value = ((READ_CONTEXT*)(pRequest->context))->meter_doc.meter_class.value;

         switch(fi)
         {
         //readC2->work_flag 标识抄读数据类型。日电能量 ：01 月电能量 02 曲线电能量 03 日冻结数据 04 月冻结 05 曲线 10  抄表日 06  小时冻结 07
         //总加组 （日、月、曲线）总加组 08    当日电能量  0E  当月电能量 0F
         //小时冻结总加组 09  抄表日使用上已结算日数据项 0C
         //白噪声 色噪声 0D         
         case DT_F1:  //终端版本信息
              datalen = read_C1_F1(pResp->frame+pResp->pos);
              break;
         case DT_F2:  //终端日历时钟
              datalen = 6;
              set_DATAFMT_01((DATAFMT_01*)(pResp->frame+pResp->pos));
              break;
              //读当前时间
//         case DT_F3: //终端参数状态
//              datalen = 31;
//              fread_array(FILE_RUN_PARA,PIM_C1_F3,pResp->frame+pResp->pos,31);
//              //1代表无效，0代表有效，因此此处进行取反
//              bit_value_opt_inversion(pResp->frame+pResp->pos,31);
//              #if defined(__PROVICE_HEILONGJIANG__)
//              //黑龙江台体检测，初始化也要求参数F3进行设置，因此默认参数F3一直是设置的
//              pResp->frame[pResp->pos] |= 0x04;
//              #endif
//              break;
//         case DT_F4:  //终端上行通信状态
//              datalen = 1;
//              fread_array(FILEID_RUN_PARAM,PIM_C1_F4,pResp->frame+pResp->pos,1);
//              break;
//        #ifdef __POWER_CTRL__
//         case DT_F5:  //终端控制设置状态
//             datalen = get_ctrl_set_status(pResp->frame+pResp->pos);
//              break;
//         case DT_F6:  //终端当前控制状态
//             datalen = get_cur_ctrl_status(pResp->frame+pResp->pos);
//              break;
//        #endif
//         case DT_F7:   //终端事件计数器当前值
//
//              datalen = 2;
//              //fread_array(FILE_RUN_PARA,PIM_C1_F7,pResp->frame+pos,2);
//              mem_cpy(pResp->frame+pResp->pos,&g_event.ec1,2);
//              break;
//
//         case DT_F8:    //终端事件标志状态
//              datalen = 8;
//              //fread_array(FILE_RUN_PARA,PIM_C1_F8,pResp->frame+pos,8);
//              mem_cpy(pResp->frame+pResp->pos,g_event.ercflag,8);
//              break;
//
//         case DT_F9:  //终端状态量及变位标志
//             datalen =2;
//              fread_array(FILEID_RUN_PARAM,PIM_C1_F9,pResp->frame+pResp->pos,datalen);
//              tmp=0;
//              fwrite_array(FILEID_RUN_PARAM,PIM_C1_F9+1,(INT8U*)&tmp,1);
//              break;
//         case DT_F10:    //当日通信流量,当日零点开始统计,当月流量统计
//              datalen = 8;
//              int32u2_bin(get_ertu_day_bytes(),pResp->frame+pResp->pos);
//              int32u2_bin(get_ertu_month_bytes(),pResp->frame+pResp->pos+4);
//              break;
//         case DT_F11:   //终端集中抄表状态信息,4个端口
//              datalen = get_c1_F11_info(pResp->frame+pResp->pos);
//              break;
//         case DT_F13:   //搜索到的电能表信息
//              datalen = get_F13(pResp->frame+pResp->pos,&(readC1->meter_seq),readC1->left_resp_len);
//              if(readC1->meter_seq > 0) readC1->need_more_frame = TRUE;
//              break;
         case DT_F799: //GPRS信息   读当前数据
                datalen = sizeof(tagWORKINFO);
                get_work_info(pResp->frame+pResp->pos);
              break;
//        #ifdef __PROVICE_JIANGSU__
//         case DT_F15:
//             datalen =2;
//             if(gSystemInfo.gprs_csq > 31)
//             {
//                 mem_set(pResp->frame+pResp->pos,2,0xEE);
//             }
//             else
//             {
//                 ul2bcd(113-2*gSystemInfo.gprs_csq,pResp->frame+pResp->pos,2);
//                 pResp->frame[pResp->pos+1] |= 0x80;
//             }
//             break;
//        #endif
//         case DT_F12: //GPRS信息
//              datalen =2;
//              fread_array(FILEID_RUN_PARAM,PIM_C1_F12,pResp->frame+pResp->pos,datalen);
//              tmp=0;
//              fwrite_array(FILEID_RUN_PARAM,PIM_C1_F12+1,(INT8U*)&tmp,1);
//              break;
         case DT_F14://文件传输未收到的数据
              datalen=read_c1_f14(pResp->frame+pResp->pos);
              break;
//         case DT_F17: //总加组组当前有功功率
//              pRequest->item_count = sizeof(C1_FN17_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN17_ITEMS;
//              readC1->work_flag = 0x08;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 0;
//                #endif
//              break;
//         case DT_F18: //总加组当前无功功率
//              pRequest->item_count = sizeof(C1_FN18_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN18_ITEMS;
//              readC1->work_flag = 0x08;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 0;
//                #endif
//              break;
//         case DT_F19:  //当日总加组有功电能量
//              pRequest->item_count = sizeof(C1_FN19_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN19_ITEMS;
//              readC1->work_flag = 0x08;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 1;
//                #endif
//              break;
//         case DT_F20:  //当日总加组无功电能量
//              pRequest->item_count = sizeof(C1_FN20_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN20_ITEMS;
//              readC1->work_flag = 0x08;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 1;
//                #endif
//              break;
//         case DT_F21:  //当月总加组有功电能量
//              pRequest->item_count = sizeof(C1_FN21_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN21_ITEMS;
//              readC1->work_flag = 0x08;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 1;
//                #endif
//              break;
//         case DT_F22:  //当月总加组无功电能量
//              pRequest->item_count = sizeof(C1_FN22_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN22_ITEMS;
//              readC1->work_flag = 0x08;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 1;
//                #endif
//              break;
//         case DT_F23:
//              pRequest->item_count = sizeof(C1_FN23_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN23_ITEMS;
//              readC1->work_flag = 0x08;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 0;
//                #endif
//              break;
//         case DT_F24:
//              fread_array(FILEID_AGP+trans_set_pn_2_pnidx(pi)-1,PIM_AGP_FLOATDOWN_CTRL_HOLDPYG,pResp->frame+pResp->pos,2);
//              if((pResp->frame[pResp->pos]==0xFF)&&(pResp->frame[pResp->pos+1]==0xFF))
//              {
//                  mem_set(pResp->frame+pResp->pos,2,0xEE);
//              }
//              datalen = 2;
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 0;
//                #endif
//              break;
         case DT_F25:
              #ifdef __PROVICE_JIANGSU__
              readC1->work_flag = 0x0A;
              #endif
              #if defined (__NGRID_HARDWARE_II__)
              if((COMMPORT_485_CY == readC1->meter_doc.baud_port.port) || (readC1->pn == 0))
              {
                  mem_set(pResp->frame+pResp->pos,67,REC_DATA_IS_DEFAULT);
                  tmp = ger_ertu_cur_voltage();
                  set_event_datetime(pResp->frame+pResp->pos);
                  ul2bcd(tmp,pResp->frame+pResp->pos+37,2);
                  datalen = 67;
              }
              else
              #endif
              {
                  pRequest->item_count = sizeof(C1_FN25_ITEMS)/sizeof(tagFN_ITEM);
                  pRequest->fn = (INT8U *)&C1_FN25_ITEMS;
                  datalen = app_get_fn_data_c1(pRequest,pResp);
                  #if defined(__QGDW_FK2005__)
                  datalen = convert_cur_spot_value(fi,pResp->frame+pResp->pos,datalen);
                  #endif
              }
                #if defined(__REDUCE_FLOW__)
                check_pos = 5;
                #endif
              break;
//         case DT_F26:
//              pRequest->item_count = sizeof(C1_FN26_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN26_ITEMS;
//              #ifdef __PROVICE_JIANGSU__
//              readC1->work_flag = 0x0A;
//              #endif
//              datalen = get_fn_data(pRequest,pResp);
//              #if defined (__QGDW_FK2005__)
//              datalen = convert_cur_spot_value(fi,pResp->frame+pResp->pos,datalen);
//              #endif
//              #if defined __QGDW_376_2009_PROTOCOL__
//              datalen = convert_cur_spot_2009_value(fi,pResp->frame+pResp->pos,datalen);
//              #endif
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 5;
//                #endif
//              break;
         case DT_F27:
              pRequest->item_count = sizeof(C1_FN27_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN27_ITEMS;
              #ifdef __PROVICE_JIANGSU__
              readC1->work_flag = 0x0A;
              #endif
              datalen = app_get_fn_data_c1(pRequest,pResp);
              #if defined(__QGDW_FK2005__) &&(!defined (__PROVICE_NEIMENG__))
              datalen = convert_cur_spot_value(fi,pResp->frame+pResp->pos,datalen);
              #endif
              #if defined(__REDUCE_FLOW__)
              check_pos = 5;
              #endif
              break;
//         case DT_F28:
//              #ifdef __PROVICE_JIANGSU__
//              readC1->work_flag = 0x0A;
//              #endif
//              pRequest->item_count = sizeof(C1_FN28_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN28_ITEMS;
//              if(readC1->work_flag == 0x0A)
//              {
//                  datalen = get_fn_data(pRequest,pResp);
//              }
//              else
//              {
//              datalen = get_fn_xl_data(pRequest,pResp);
//              }
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 5;
//                #endif
//              break;
//         case DT_F29:
//              pRequest->item_count = sizeof(C1_FN29_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN29_ITEMS;
//              datalen = get_fn_xl_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 5;
//                #endif
//              break;
//         case DT_F30:
//              pRequest->item_count = sizeof(C1_FN30_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN30_ITEMS;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 5;
//                #endif
//              break;
//         case DT_F31:
//              pRequest->item_count = sizeof(C1_FN31_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN31_ITEMS;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 5;
//                #endif
//              break;
//         case DT_F32:
//              pRequest->item_count = sizeof(C1_FN32_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN32_ITEMS;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 5;
//                #endif
//              break;
         case DT_F33:  //当前正向有/无功电能示值、一/四象限无功电能示值（总、费率1~M）
                pRequest->item_count = sizeof(C1_FN33_ITEMS)/sizeof(tagFN_ITEM);
                pRequest->fn = (INT8U *)&C1_FN33_ITEMS;
                datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
         case DT_F34:
                pRequest->item_count = sizeof(C1_FN34_ITEMS)/sizeof(tagFN_ITEM);
                pRequest->fn = (INT8U *)&C1_FN34_ITEMS;
                datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
//         case DT_F35:
//              pRequest->item_count = sizeof(C1_FN35_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN35_ITEMS;
//              datalen = get_fn_xl_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
//         case DT_F36:
//              pRequest->item_count = sizeof(C1_FN36_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN36_ITEMS;
//              datalen = get_fn_xl_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
//         case DT_F37:
//              pRequest->item_count = sizeof(C1_FN37_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN37_ITEMS;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
//         case DT_F38:
//              pRequest->item_count = sizeof(C1_FN38_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN38_ITEMS;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
//         case DT_F39:
//              pRequest->item_count = sizeof(C1_FN39_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN39_ITEMS;
//              datalen = get_fn_xl_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
//         case DT_F40:
//              pRequest->item_count = sizeof(C1_FN40_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN40_ITEMS;
//              datalen = get_fn_xl_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
         case DT_F41:
              pRequest->item_count = sizeof(C1_FN41_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN41_ITEMS;
              readC1->work_flag = 0x0E;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 1;
                #endif
              break;
         case DT_F42:
              pRequest->item_count = sizeof(C1_FN42_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN42_ITEMS;
              readC1->work_flag = 0x0E;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 1;
                #endif
              break;
         case DT_F43:
              pRequest->item_count = sizeof(C1_FN43_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN43_ITEMS;
              readC1->work_flag =0x0E;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 1;
                #endif
              break;
         case DT_F44:
              pRequest->item_count = sizeof(C1_FN44_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN44_ITEMS;
              readC1->work_flag =0x0E;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 1;
                #endif
              break;
//         case DT_F45:
//              pRequest->item_count = sizeof(C1_FN45_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN45_ITEMS;
//              readC1->work_flag =0x02;
//              datalen = get_fn_spot_amounts(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 1;
//                #endif
//              break;
//         case DT_F46:
//              pRequest->item_count = sizeof(C1_FN46_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN46_ITEMS;
//              readC1->work_flag =0x02;
//              datalen = get_fn_spot_amounts(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 1;
//                #endif
//              break;
//         case DT_F47:
//              pRequest->item_count = sizeof(C1_FN47_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN47_ITEMS;
//              readC1->work_flag =0x02;
//              datalen = get_fn_spot_amounts(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 1;
//                #endif
//              break;
//         case DT_F48:
//              pRequest->item_count = sizeof(C1_FN48_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN48_ITEMS;
//              readC1->work_flag =0x02;
//              datalen = get_fn_spot_amounts(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 1;
//                #endif
//              break;
//         case DT_F49:     //电压电流相角
//              if(readC1->meter_doc.baud_port.port == COMMPORT_485_CY)
//              {
//                pRequest->item_count = sizeof(C1_FN49_CY_ITEMS)/sizeof(tagFN_ITEM);
//                pRequest->fn = (INT8U *)&C1_FN49_CY_ITEMS;
//              }
//              else
//              {
//                pRequest->item_count = sizeof(C1_FN49_ITEMS)/sizeof(tagFN_ITEM);
//                pRequest->fn = (INT8U *)&C1_FN49_ITEMS;
//              }
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 0;
//                #endif
//              break;
//         #ifdef __QGDW_FK2005__
//         case DT_F50: //当前组合有功电能示值（费率）
//         case DT_F133: //低压表当前组合有功电能示值（费率）
//              pRequest->item_count = sizeof(C1_FN50_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN50_ITEMS;
//              datalen = get_fn_data(pRequest,pResp);
//              break;
//         #endif
//         case DT_F57: //当前谐波有效值
//         case DT_F58: //当前谐波含有率
//             datalen = read_spot_harmonic(&req_var,pi,fi,pResp->frame+pResp->pos);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 1;
//                #endif
//             break;
///*        case DT_F58: //谐波
//              pRequest->item_count = sizeof(C1_FN58_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN58_ITEMS;
//              datalen = get_fn_data(pRequest,pResp);
//              break;
//*/
//         #ifndef __SOFT_SIMULATOR__
//         case DT_F73:
//              datalen = 2;
//              #if defined (__NEIMENG_HARDWARE__) ||defined (__HUNAN_TTU__)
//              read_dcad_value(pResp->frame+pResp->pos,pi,tmp_str);
//              #else
//              mem_set(pResp->frame+pResp->pos,2,0); 
//              #endif
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 0;
//                #endif
//              break;
//         #endif
//         case DT_F81:  //小时冻结总加有功功率
//              pRequest->item_count = sizeof(C1_FN81_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN81_ITEMS;
//              readC1->work_flag = 0x09;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F82:  //小时冻结总加无功功率
//              pRequest->item_count = sizeof(C1_FN82_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN82_ITEMS;
//              readC1->work_flag = 0x09;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F83:  //小时冻结总加有功电能量
//              pRequest->item_count = sizeof(C1_FN83_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN83_ITEMS;
//              readC1->work_flag = 0x09;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F84:  //小时冻结总加无功电能量
//              pRequest->item_count = sizeof(C1_FN84_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN84_ITEMS;
//              readC1->work_flag = 0x09;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F89: //小时冻结有功功率
//              pRequest->item_count = sizeof(C1_FN89_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN89_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F90: //小时冻结A相有功功率
//              pRequest->item_count = sizeof(C1_FN90_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN90_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F91:  //小时冻结B相有功功率
//              pRequest->item_count = sizeof(C1_FN91_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN91_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F92: //小时冻结C相有功功率
//              pRequest->item_count = sizeof(C1_FN92_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN92_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F93: //小时冻结无功功率
//              pRequest->item_count = sizeof(C1_FN93_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN93_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F94: //小时冻结A相无功功率
//              pRequest->item_count = sizeof(C1_FN94_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN94_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F95:  //小时冻结B相无功功率
//              pRequest->item_count = sizeof(C1_FN95_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN95_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F96: //小时冻结C相无功功率
//              pRequest->item_count = sizeof(C1_FN96_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN96_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F97: //小时冻结A相电压
//              pRequest->item_count = sizeof(C1_FN97_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN97_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F98: //小时冻结B相电压
//              pRequest->item_count = sizeof(C1_FN98_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN98_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F99: //小时冻结C相电压
//              pRequest->item_count = sizeof(C1_FN99_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN99_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F100: //小时冻结A相电流
//              pRequest->item_count = sizeof(C1_FN100_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN100_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//              #if defined(__QGDW_FK2005__)
//              datalen = convert_cur_spot_value(fi,pResp->frame+pResp->pos,datalen);
//              #endif
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F101: //小时冻结B相电流
//              pRequest->item_count = sizeof(C1_FN101_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN101_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//              #if defined(__QGDW_FK2005__)
//              datalen = convert_cur_spot_value(fi,pResp->frame+pResp->pos,datalen);
//              #endif
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F102: //小时冻结C相电流
//              pRequest->item_count = sizeof(C1_FN102_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN102_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//              #if defined(__QGDW_FK2005__)
//              datalen = convert_cur_spot_value(fi,pResp->frame+pResp->pos,datalen);
//              #endif
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F103: //小时冻结N相电流
//              pRequest->item_count = sizeof(C1_FN103_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN103_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//              #if defined(__QGDW_FK2005__)
//              datalen = convert_cur_spot_value(fi,pResp->frame+pResp->pos,datalen);
//              #endif
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         //F105-F108, 电能量，从曲线中提取 0,15,30,45,60,计算差值
//         case DT_F105:  //小时冻结正向有功总电能量
//              pRequest->item_count = sizeof(C1_FN105_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN105_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_spot_amounts(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F106:  //小时冻结正向无功总电能量
//              pRequest->item_count = sizeof(C1_FN106_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN106_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_spot_amounts(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F107:  //小时冻结反向有功总电能量
//              pRequest->item_count = sizeof(C1_FN107_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN107_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_spot_amounts(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F108:  //小时冻结反向无功总电能量
//              pRequest->item_count = sizeof(C1_FN108_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN108_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_spot_amounts(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//
//         case DT_F109:  //小时冻结正向有功总电能示值（总，费率）
//              pRequest->item_count = sizeof(C1_FN109_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN109_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F110:  //小时冻结正向无功总电能示值（总，费率）
//              pRequest->item_count = sizeof(C1_FN110_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN110_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F111:  //小时冻结反向有功总电能示值（总，费率）
//              pRequest->item_count = sizeof(C1_FN111_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN111_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F112:  //小时冻结反向无功总电能示值（总，费率）
//              pRequest->item_count = sizeof(C1_FN112_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN112_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F113: //小时冻结总功率因数
//              pRequest->item_count = sizeof(C1_FN113_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN113_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F114: //小时冻结A相总功率因数
//              pRequest->item_count = sizeof(C1_FN114_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN114_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F115:  //小时冻结B相总功率因数
//              pRequest->item_count = sizeof(C1_FN115_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN115_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
//         case DT_F116: //小时冻结C相总功率因数
//              pRequest->item_count = sizeof(C1_FN116_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN116_ITEMS;
//              readC1->work_flag = 0x07;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 2;
//                #endif
//              break;
         case DT_F129: //当前正向有功电能示值（费率）
         case DT_F801:                      
         #ifdef __ZXYG_EQUAL_TO_ZXYG_ADD_FXYG__
              if ((meter_class.value != 0x02) && (meter_class.value != 0x62) && (meter_class.value != 0x71) && (meter_class.value != 0x72)
              && (((READ_CONTEXT*)(pRequest->context))->meter_doc.protocol == GB645_2007))
              {
                    pRequest->item_count = sizeof(C1_FN129_ITEMS_SH)/sizeof(tagFN_ITEM);
                    pRequest->fn = (INT8U *)&C1_FN129_ITEMS_SH;
              }
              else
              {
                    pRequest->item_count = sizeof(C1_FN129_ITEMS)/sizeof(tagFN_ITEM);
                    pRequest->fn = (INT8U *)&C1_FN129_ITEMS;
              }
              #endif
              #if defined (__PROVICE_GANSU__)
              if(meter_class.meter_class == 6)
              {
                    pRequest->item_count = sizeof(C1_FN129_GANSU_ITEMS)/sizeof(tagFN_ITEM);
                    pRequest->fn = (INT8U *)&C1_FN129_GANSU_ITEMS;
              }
              else
              {
                    pRequest->item_count = sizeof(C1_FN129_ITEMS)/sizeof(tagFN_ITEM);
                    pRequest->fn = (INT8U *)&C1_FN129_ITEMS;
              }
              #else
                  #ifdef __READ_EDMI_METER__
                  if(readC1->meter_doc.protocol == METER_EDMI)
                  {
                  if(readC1->work_flag == 0x0A)
                    {
              pRequest->item_count = sizeof(C1_FN129_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN129_ITEMS;
                    }
                    else
                    {
                      pRequest->item_count = sizeof(C1_FN129_EDMI_ITEMS)/sizeof(tagFN_ITEM);
                      pRequest->fn = (INT8U *)&C1_FN129_EDMI_ITEMS;
                  }
                }
                  else
                  #endif
                  {
              pRequest->item_count = sizeof(C1_FN129_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN129_ITEMS;
                  }
              #endif
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
         case DT_F130: //当前正向无功电能示值（费率）
              #ifdef __READ_EDMI_METER__
              if(readC1->meter_doc.protocol == METER_EDMI)
              {
                    if(readC1->work_flag == 0x0A)
                    {
              pRequest->item_count = sizeof(C1_FN130_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN130_ITEMS;
                    }
                    else
                    {
                  pRequest->item_count = sizeof(C1_FN130_EDMI_ITEMS)/sizeof(tagFN_ITEM);
                  pRequest->fn = (INT8U *)&C1_FN130_EDMI_ITEMS;
              }
                }
              else
              #endif
              {
                  pRequest->item_count = sizeof(C1_FN130_ITEMS)/sizeof(tagFN_ITEM);
                  pRequest->fn = (INT8U *)&C1_FN130_ITEMS;
              }
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
         case DT_F131:
              #ifdef __READ_EDMI_METER__
              if(readC1->meter_doc.protocol == METER_EDMI)
              {
                    if(readC1->work_flag == 0x0A)
                    {
              pRequest->item_count = sizeof(C1_FN131_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN131_ITEMS;
                    }
                    else
                    {
                        pRequest->item_count = sizeof(C1_FN131_EDMI_ITEMS)/sizeof(tagFN_ITEM);
                        pRequest->fn = (INT8U *)&C1_FN131_EDMI_ITEMS;
                    }
              }
              else
              #endif
              {
              pRequest->item_count = sizeof(C1_FN131_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN131_ITEMS;
              }
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
         case DT_F132:
              #ifdef __READ_EDMI_METER__
              if(readC1->meter_doc.protocol == METER_EDMI)
              {
                    if(readC1->work_flag == 0x0A)
                    {
                        pRequest->item_count = sizeof(C1_FN132_ITEMS)/sizeof(tagFN_ITEM);
                        pRequest->fn = (INT8U *)&C1_FN132_ITEMS;
                    }
                    else
                    {
                  pRequest->item_count = sizeof(C1_FN132_EDMI_ITEMS)/sizeof(tagFN_ITEM);
                  pRequest->fn = (INT8U *)&C1_FN132_EDMI_ITEMS;
              }
                }
              else
              #endif
              {
              pRequest->item_count = sizeof(C1_FN132_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN132_ITEMS;
              }
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
        #ifndef __QGDW_FK2005__
         case DT_F133:
              pRequest->item_count = sizeof(C1_FN133_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN133_ITEMS;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
        #endif
         case DT_F134:
              pRequest->item_count = sizeof(C1_FN134_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN134_ITEMS;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
         case DT_F135:
              pRequest->item_count = sizeof(C1_FN135_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN135_ITEMS;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
         case DT_F136:
              pRequest->item_count = sizeof(C1_FN136_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN136_ITEMS;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
         case DT_F137:
              #ifdef __ZXYG_EQUAL_TO_ZXYG_ADD_FXYG__
              if ((meter_class.value != 0x02) && (meter_class.value != 0x62) && (meter_class.value != 0x71) && (meter_class.value != 0x72))
              {
                    pRequest->item_count = sizeof(C1_FN137_ITEMS_SH)/sizeof(tagFN_ITEM);
                    pRequest->fn = (INT8U *)&C1_FN137_ITEMS_SH;
              }
              else
              {
                    pRequest->item_count = sizeof(C1_FN137_ITEMS)/sizeof(tagFN_ITEM);
                    pRequest->fn = (INT8U *)&C1_FN137_ITEMS;
              }
              #else
              pRequest->item_count = sizeof(C1_FN137_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN137_ITEMS;
              #endif
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
         case DT_F138:
              pRequest->item_count = sizeof(C1_FN138_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN138_ITEMS;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
         case DT_F139:
              pRequest->item_count = sizeof(C1_FN139_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN139_ITEMS;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
         case DT_F140:
              pRequest->item_count = sizeof(C1_FN140_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN140_ITEMS;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
         case DT_F141:
              pRequest->item_count = sizeof(C1_FN141_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN141_ITEMS;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
         case DT_F142:
              pRequest->item_count = sizeof(C1_FN142_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN142_ITEMS;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
         case DT_F143:
              pRequest->item_count = sizeof(C1_FN143_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN143_ITEMS;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
         case DT_F144:
              pRequest->item_count = sizeof(C1_FN144_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN144_ITEMS;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
//         case DT_F145:
//              pRequest->item_count = sizeof(C1_FN145_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN145_ITEMS;
//              datalen = get_fn_xl_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
//         case DT_F146:
//              pRequest->item_count = sizeof(C1_FN146_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN146_ITEMS;
//              datalen = get_fn_xl_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
//         case DT_F147:
//              pRequest->item_count = sizeof(C1_FN147_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN147_ITEMS;
//              datalen = get_fn_xl_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
//         case DT_F148:
//              pRequest->item_count = sizeof(C1_FN148_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN148_ITEMS;
//              datalen = get_fn_xl_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
//         case DT_F149:
//              pRequest->item_count = sizeof(C1_FN149_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN149_ITEMS;
//              datalen = get_fn_xl_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
//         case DT_F150:
//              pRequest->item_count = sizeof(C1_FN150_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN150_ITEMS;
//              datalen = get_fn_xl_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
//         case DT_F151:
//              pRequest->item_count = sizeof(C1_FN151_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN151_ITEMS;
//              datalen = get_fn_xl_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
//         case DT_F152:
//              pRequest->item_count = sizeof(C1_FN152_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN152_ITEMS;
//              datalen = get_fn_xl_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
//         case DT_F161:
//              pRequest->item_count = sizeof(C1_FN161_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN161_ITEMS;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 5;
//                #endif
//              break;
         case DT_F162:
              pRequest->item_count = sizeof(C1_FN162_ITEMS)/sizeof(tagFN_ITEM);
              pRequest->fn = (INT8U *)&C1_FN162_ITEMS;
              datalen = app_get_fn_data_c1(pRequest,pResp);
                #if defined(__REDUCE_FLOW__)
                check_pos = 6;
                #endif
              break;
//         case DT_F165:
//              pRequest->item_count = sizeof(C1_FN165_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN165_ITEMS;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 5;
//                #endif
//              break;
//         case DT_F166:
//              pRequest->item_count = sizeof(C1_FN166_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN166_ITEMS;
//              datalen = get_fn_xl_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 5;
//                #endif
//              break;
//         case DT_F167:
//              pRequest->item_count = sizeof(C1_FN167_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN167_ITEMS;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 5;
//                #endif
//              break;
//         case DT_F168:
//              pRequest->item_count = sizeof(C1_FN168_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN168_ITEMS;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
//         case DT_F169:
//              mem_set(pResp->frame+pResp->pos,1,readC1->meter_doc.baud_port.port);
//              mem_set(pResp->frame+pResp->pos+1,1,0x00);
//              datalen = 2;
//              break;
//         case DT_F170:
//              datalen = get_c1_F170(pRequest,pResp->frame+pResp->pos);
//            break;
//         case DT_F171:   //搜索到的电能表信息
//              datalen = get_F13(pResp->frame+pResp->pos,&(readC1->meter_seq),readC1->left_resp_len);
//              if(readC1->meter_seq > 0) readC1->need_more_frame = TRUE;
//              break;
//        #ifndef __PROVICE_NEIMENG__
//         case DT_F177:
//              pRequest->item_count = sizeof(C1_FN177_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN177_ITEMS;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 6;
//                #endif
//              break;
//        #endif
//         case DT_F178:
//              pRequest->item_count = sizeof(C1_FN178_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN178_ITEMS;
//              datalen = get_fn_data(pRequest,pResp);
//              break;
//         #if  defined (__QGDW_376_2013_PROTOCOL__)
//         case DT_F179:
//         #ifndef __PROVICE_JIANGSU__
//         case DT_F180:
//         #endif
//            datalen = read_cur_spot_F179F180(&req_var,pi,fi,pResp->frame+pResp->pos);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 5;
//                #endif
//             break;
//        #endif
//        #ifdef __GF_NBQ_DATA_READ_CURVE__
//        case DT_F181:
//              pRequest->item_count = sizeof(C1_FN181_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN181_ITEMS;
//              datalen = get_fn_data(pRequest,pResp);
//              break;
//        #endif
//        #ifdef __PROVICE_JIANGSU__
//        case DT_F186:
//            mem_cpy(pResp->frame+pResp->pos, gSystemInfo.plc_ver_info+6,9);
////            fread_ertu_params(EEADDR_SYS_ROUTER_VER, pResp->frame+pResp->pos,9);
//            datalen = 9;
//            break;
//         #endif
//
             case DT_F199:
                 mem_set(pResp->frame+pResp->pos,19,0x00);
                 #ifndef __METER_SIMPLE__
                 mem_cpy(pResp->frame+pResp->pos,(INT8U*)&gSystemInfo.bat_mon,2);
                 #endif
//                 mem_cpy(pResp->frame+pResp->pos+4,gSystemInfo.clock_battrey_ex,2);
                 if(gSystemInfo.battery_charge==0xAA)
                 {
                    pResp->frame[pResp->pos+8]=1;
                 }
                 #ifdef __METER_SIMPLE__
                 mem_cpy(pResp->frame+pResp->pos+9,&gSystemInfo.bat_mon,2);
                 #endif
                 mem_cpy(pResp->frame+pResp->pos+13,&gSystemInfo.lvd,2);
                 datalen=19;
                 break;
//         #ifdef __PROVICE_JIANGSU__
//         case DT_F201:
//              pRequest->item_count = sizeof(C1_FN201_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN201_ITEMS;
//              readC1->work_flag = 0x0A;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 5;
//                #endif
//              break;
//         case DT_F202:
//              pRequest->item_count = sizeof(C1_FN202_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN202_ITEMS;
//              readC1->work_flag = 0x0A;
//              datalen = get_fn_data(pRequest,pResp);
//                #if defined(__REDUCE_FLOW__)
//                check_pos = 5;
//                #endif
//              break;
//         #endif
//        #ifdef __QGDW_FK2005__
//         case DT_F208:
//              pRequest->item_count = sizeof(C1_FN208_ITEMS)/sizeof(tagFN_ITEM);
//              pRequest->fn = (INT8U *)&C1_FN208_ITEMS;
//              datalen = get_fn_data(pRequest,pResp);
//            break;
//         case DT_F209:
//              datalen = read_C1_F209_sim_card(pRequest,pResp);
//            break;
//        #endif
//
//    #ifdef __PROVICE_SHANGHAI__
//#ifdef __POWER_CTRL__
//            case DT_F191:
//                datalen = get_sh_C1_F191(pResp->frame+pResp->pos);
//                break;
//            #endif
//         case DT_F201:
//                datalen= meter_read_fail_count(pResp->frame+pResp->pos,readC1->left_resp_len);
//	      break;
//	 case DT_F202:
//            //readC1->req_param_size = 4;
//                datalen = meter_read_fail_list(pResp->frame+pResp->pos,readC1->param,readC1->left_resp_len);
//	      if((readC1->param[0]!=0)||(readC1->param[1]!=0))
//            readC1->need_more_frame = TRUE;
//	      break; 
//         case DT_F241:
//              datalen = read_sh_C1_F241(pResp->frame+pResp->pos,255);
//            break;
//         case DT_F242:
//              datalen = read_sh_C1_F242(&req_var,pResp->frame+pResp->pos,255);
//            break;
//         case DT_F243:
//              datalen = read_sh_C1_F243(pResp->frame+pResp->pos,255);
//            break;
//      #endif
//      #if defined(__PROVICE_JILIN__) || defined(__PROVICE_JILIN_FK__)
//         case DT_F201:
//             datalen =2;
//             if(gSystemInfo.gprs_csq > 31)
//             {
//                 mem_set(pResp->frame+pResp->pos,2,0xEE);
//             }
//             else
//             {
//                 ul2bcd(113-2*gSystemInfo.gprs_csq,pResp->frame+pResp->pos,2);
//                 pResp->frame[pResp->pos+1] |= 0x80;
//             }
//             break;
//      #endif
//         case DT_F245:
//            datalen=read_C1_F245(pRequest,pResp);
//            break;
//         case DT_F246:
//            pRequest->item_count = sizeof(C1_FN246_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C1_FN246_ITEMS;
//            readC1->work_flag = 0x0B;
//            datalen = get_fn_data(pRequest,pResp);
//            break;
//         case DT_F247:
//#ifndef __SOFT_SIMULATOR__
//            datalen=get_ertu_standby_battery_voltage();
//            if(datalen<BAT_INVALID_VALUE)
//            {
//                pResp->frame[pResp->pos]= 0;
//            }
//            else
//            {
//                pResp->frame[pResp->pos]= 1;
//            }
//            datalen=1;
//#endif
//            break;
//         case DT_F251:
//            pRequest->item_count = sizeof(C1_FN251_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C1_FN251_ITEMS;
//            readC1->work_flag = 0x0B;
//            datalen = get_fn_data(pRequest,pResp);
//            break;
//            #ifdef __PRECISE_TIME__
//            /*
//            case DT_F248:
//                datalen = read_precise_time_F248(resp->buffer+pos,readC1->param,readC1->left_resp_len);
//                if((readC1->param[0]!=0)||(readC1->param[1]!=0))
//                {
//                    need_more_frame = TRUE;
//                }
//                break;
//                */
//            case DT_F249:
//                datalen = read_precise_time_F249(pResp->frame+pResp->pos);
//                break;
//                /*
//            case DT_F250:
//                datalen = read_precise_time_F250(&req_var,readC1->param,resp->buffer+pos,readC1->left_resp_len);
//                if((readC1->param[0]!=0)||(readC1->param[1]!=0))
//                {
//                    need_more_frame = TRUE;
//                }
//                break;
//                */
//            #endif
//
//         case DT_F252:
//            pRequest->item_count = sizeof(C1_FN252_ITEMS)/sizeof(tagFN_ITEM);
//            pRequest->fn = (INT8U *)&C1_FN252_ITEMS;
//            datalen = get_fn_data(pRequest,pResp);
//            break;
//          case DT_F253:
//          pRequest->item_count = sizeof(C1_FN253_ITEMS)/sizeof(tagFN_ITEM);
//          pRequest->fn = (INT8U *)&C1_FN253_ITEMS;
//          datalen = get_fn_data(pRequest,pResp);
//          break;
//         #ifdef __BATCH_TRANSPARENT_METER_TASK__
//         case DT_F305:
//             fread_array(FILEID_RUN_DATA,FMDATA_F305,pResp->frame+pResp->pos,255);
//             datalen = 255;
//             break;
//         case DT_F306:
//             datalen = query_param_batch_transparent_meter_task_exec_result(trans_set_pn_2_pnidx(pi),
//                    readC1->param,readC1->Td,
//                    pResp->frame+pResp->pos,
//                    readC1->left_resp_len,&(readC1->req_param_size));
//             readC1->need_more_frame = readC1->Td[1];
//             break;
//         #endif
//
//         case DT_F310:
//            datalen = read_C1_F310(pResp->frame+pResp->pos);
//            break;
//        #ifdef __F2040_GET_GPRS_INFO__
//         case DT_F2040://GPRS\CDMA通信信息
//              datalen=get_c1_F2040(pResp->frame+pResp->pos);
//              break;
//        #endif
//         #ifdef __MESSAGE_SEND_RECEIVE_RECORD__
//         case DT_F324:
//             datalen = read_C1_F324(trans_set_pn_2_pnidx(pi),pResp->frame+pResp->pos);
//             break;
//        case DT_F325:
//             datalen = read_C1_F325(trans_set_pn_2_pnidx(pi),pResp->frame+pResp->pos);
//             break;
//         #endif
         case 0x00:
            break;
         default:
              //req_var.need_do_default = TRUE;
              //datalen = 0;
              //break;
              return  FALSE;
         }
        //处理一个fn后文，清除该标志，说明预抄时对于未抄到数据项需要点抄


         #if defined(__REDUCE_FLOW__)   //数据长度大于0并且检查长度大于0时，检查是否全为EE
         check_len = datalen-check_pos;
         if((datalen > 0) && (check_len > 0))
         {
            check_flag = FALSE;
            for(check_idx=0;check_idx<check_len;check_idx++)
            {
                if(pResp->frame[pResp->pos+check_pos+check_idx] != REC_DATA_IS_DEFAULT)
                {
                    check_flag = TRUE;
                    break;
                }
            }
            if(check_flag == FALSE)
                req_var.need_do_default = TRUE;
         }
         #endif

         //无数据项目处理
         if(datalen == 0)
         {
              fn_mask =  fi & 0x00FF;
              readC1->fn &= ~fn_mask;
              if(TRUE == is_pifi_mode)
              {
                 pResp->pos -= 4;
              }
              else
              {
                 datalen = readC1->pos_pnfn;
                 set_pn_fn(pResp->frame, &datalen, readC1->pn, readC1->fn);     //如果该fn无数据单元，则在数据单元标识中去掉
              }
              //必须放在这里，因为前面可能改变其值！
              datalen = 0;
         }

         pResp->pos += datalen;
         if(pResp->pos  > pResp->max_reply_pos || readC1->need_more_frame)
         {
            if(is_pifi_mode==TRUE)
            {
               //如果一个报文都无法容纳数据标识,则返回失败
               if(pResp->pos == POS_DATA) return FALSE;           //该条件只有在datalen=0的时候可能成立

               //恢复到上个处理位置if(!need_more_frame )
               if(((!readC1->need_more_frame )&& pResp->pos  > pResp->max_reply_pos ) || readC1->need_more_frame ) //长度是多pn、fn回退数据和pnfn  如果当前帧只有一个fn 不回退pnfn 在fn内分帧 置位分帧标志//如果已经进行了手工分帧，则不需要再进行分帧,但是如果手工分帧后pos  > resp->max_reply_pos，那么还需要分帧
               {
                  if((pRequest->userDataLen/4>1)|| (pngroupcount >1) ||(fngroupcount>1))
                  {
                    pResp->pos -= (datalen + 4);
                  }
                  else if(datalen == 0)  //单pnfn data为0一帧都放不下 直接否认  data 不为0 需要分帧
                  {
                     pResp->pos -= 4;
                  }
               }
                if(pResp->pos == POS_DATA)return FALSE;//如果分帧后位置回到了初始位置，说明缓冲区无法容纳数据项，则首帧返回否认帧，其他放弃处理
                fng_idx--;

                //先行发送响应祯, 结束=FALSE
                //pResp->pos = pos;
                if(FALSE == app_send_response_frame(pRequest,pResp,FALSE) ) return FALSE;

                //初始化响应报文位置
                pResp->pos = POS_DATA;

                //继续组织报文
                continue;
             }
             else
             {
                //重新进行响应
                is_pifi_mode = TRUE;

                //重新组织响应报文
                goto GET_READ_C1DATA_AGAIN;
             }
         }
      }
   }

   return TRUE;
}


/*+++
  功能：获取工作状态信息
---*/
void get_work_info(INT8U *resp)
{
    tagWORKINFO *ptr_work_info;
    
    ptr_work_info = (tagWORKINFO *)resp;
    ptr_work_info->temp=gSystemInfo.dev_temp;

    #ifdef __ONENET__ //空中写卡要附着网络
    if(gSystemInfo.cgatt==1)
    {
        ptr_work_info->csq=gSystemInfo.csq;
    }
    else
    {
        ptr_work_info->csq=0;
    }      
    #else
    ptr_work_info->csq=gSystemInfo.csq;
    #endif
    ptr_work_info->ber=gSystemInfo.remote_model_type;
    ptr_work_info->gprs_end_state=gSystemInfo.login_status;
    ptr_work_info->gprs_type[0]=gSystemInfo.remote_model_vendor;
    ptr_work_info->gprs_type[1]=gSystemInfo.is_nb_net;
    ptr_work_info->gprs_type[2]=gSystemInfo.is_coap_net;
    mem_cpy(ptr_work_info->gprs_imsi,gSystemInfo.imei_info,15);
    mem_cpy(ptr_work_info->sim_id,gSystemInfo.myccid_info,20);


//    //mem_cpy(ptr_work_info->mac,gSystemInfo.eth_mac_addr,6);
//    {
//       mem_set( ptr_work_info->ctrl_ver,5,'X');
//    }
//    mem_cpy(ptr_work_info->soft_ver,(void*)internal_soft_verinfo,7);
//    mem_cpy(ptr_work_info->djb_ver,gSystemInfo.djb_ver_info,16);
//    mem_cpy(ptr_work_info->bad_block_count,&NDFAT.bad_blocks_count,2);
//    ptr_work_info->ram_status = 0x00;
//    if(devp_ram_status == 0xAAAAAAAA)
//    {
//        ptr_work_info->ram_status = 0xAA;
//    }
//    ptr_work_info->osc_status=gSystemInfo.primary_osc_status;
//
//    #ifdef __SOFT_SIMULATOR__
//    ptr_work_info->rtcc_status = 0xAA;
//    #else
//    ptr_work_info->rtcc_status = PLIB_RTCC_RTCSecondGet(RTCC_ID_0);
//    #endif
//    load_config_ver(ptr_work_info);
//    ptr_work_info->error_status=0x00;
//    ptr_work_info->check_key= gSystemInfo.check_key;
//    //gSystemInfo.check_key = 0;
//    if(check_load_clock_flag())
//    {
//        ptr_work_info->error_status|=0x02;
//    }
//    ptr_work_info->esam_type=gSystemInfo.esam_type;

}

//INT16U get_c1_F11_info(INT8U* data)
//{
//    LLVC_REC_STATE *rec_state;
////    FAST_INDEX fast_index;
//    INT16U node_count;//,meter_seq;
//    INT16U meter_idx,plc_read_count,idx;
//    INT16U port2_read_count,port3_read_count,port1_read_count;
//    INT8U value,is_F35,metrer_port;
//    INT8U seq_spec[2];
//    #ifdef __PROVICE_HENAN__
//    INT8U jc_llvc[sizeof(LLVC_REC_STATE)];
//    #endif
//#ifdef __POWER_CTRL__
//        int16u2_bin(get_readport_meter_count_from_fast_index(COMMPORT_485_CY),llvc_rec_state[COMMPORT_485_CY-1].total_count);//交采重新统计
//        int16u2_bin(get_readport_meter_count_from_fast_index(COMMPORT_485_REC),llvc_rec_state[COMMPORT_485_REC-1].total_count);//485I重新统计
//        int16u2_bin(get_readport_meter_count_from_fast_index(COMMPORT_485_CAS),llvc_rec_state[COMMPORT_485_CAS-1].total_count);//485II重新统计
//        if(bin2_int16u(llvc_rec_state[COMMPORT_485_CY-1].total_count) == 0)//交采不存在
//        {
//            llvc_rec_state[COMMPORT_485_CY-1].currec_flag.value = 0x00;//未抄表
//            mem_set(llvc_rec_state[COMMPORT_485_CY-1].begin_datetime,6,0x00);
//            mem_set(llvc_rec_state[COMMPORT_485_CY-1].end_datetime,6,0x00);
//        }
//        if(bin2_int16u(llvc_rec_state[COMMPORT_485_REC-1].total_count) == 0)//485I不存在
//        {
//            llvc_rec_state[COMMPORT_485_REC-1].currec_flag.value = 0x00;//未抄表
//            mem_set(llvc_rec_state[COMMPORT_485_REC-1].begin_datetime,6,0x00);
//            mem_set(llvc_rec_state[COMMPORT_485_REC-1].end_datetime,6,0x00);
//        }
//        if(bin2_int16u(llvc_rec_state[COMMPORT_485_CAS-1].total_count) == 0)//485II不存在
//        {
//            llvc_rec_state[COMMPORT_485_CAS-1].currec_flag.value = 0x00;//未抄表
//            mem_set(llvc_rec_state[COMMPORT_485_CAS-1].begin_datetime,6,0x00);
//            mem_set(llvc_rec_state[COMMPORT_485_CAS-1].end_datetime,6,0x00);
//        }
//        port1_read_count = 0;
//        port2_read_count = 0;
//        port3_read_count = 0;
//        llvc_rec_state[COMMPORT_485_CY-1].read_vip_count = 0;
//        llvc_rec_state[COMMPORT_485_REC-1].read_vip_count = 0;
//        llvc_rec_state[COMMPORT_485_CAS-1].read_vip_count = 0;
//
//        node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;
//        for(idx=0;idx<node_count;idx++)
//        {
//            tpos_enterCriticalSection();
//            seq_spec[0]=fast_index_list.fast_index[idx].seq_spec[0];
//            seq_spec[1]=fast_index_list.fast_index[idx].seq_spec[1];
//            metrer_port=fast_index_list.fast_index[idx].port;
//            tpos_leaveCriticalSection();
//
//            meter_idx = bin2_int16u(seq_spec) & FAST_IDX_MASK;
//
//            if(!file_exist(meter_idx)) continue;
//
//             //判断是否是重点表
//              fread_ertu_params(EEADDR_SET_F35+(meter_idx/8),&value,1);
//              is_F35 = ((~value) & (0x01<<(meter_idx%8))) ? TRUE : FALSE;
//
//            if(metrer_port == COMMPORT_485_REC)
//            {
//                if(check_F161_day_hold(meter_idx,1,FALSE))
//                {
//                   port2_read_count ++ ;
//
//                   if(is_F35)
//                   {
//                   llvc_rec_state[COMMPORT_485_REC-1].read_vip_count ++;
//                   }
//                }
//
//            }
//            if(metrer_port == COMMPORT_485_CAS)
//            {
//                if(check_F161_day_hold(meter_idx,1,FALSE))
//                {
//                   port3_read_count ++ ;
//                   if(is_F35)
//                   {
//                   llvc_rec_state[COMMPORT_485_CAS-1].read_vip_count ++;
//                   }
//                }
//
//            }
//            if(metrer_port == COMMPORT_485_CY)
//            {
//                if(check_F161_day_hold(meter_idx,1,FALSE))
//                {
//                   port1_read_count ++ ;
//                   if(is_F35)
//                   {
//                    llvc_rec_state[COMMPORT_485_CY-1].read_vip_count ++;
//                   }
//                }
//
//            }
//
//        }
//#else
//    llvc_rec_state[COMMPORT_PLC_REC-1].port = COMMPORT_PLC;
//    int16u2_bin(get_readport_meter_count_from_fast_index(COMMPORT_PLC),llvc_rec_state[COMMPORT_PLC_REC-1].total_count);//再统计一次，有可能不插路由
//    int16u2_bin(get_readport_meter_count_from_fast_index(COMMPORT_485_CY),llvc_rec_state[COMMPORT_485_CY-1].total_count);//交采重新统计
//    int16u2_bin(get_readport_meter_count_from_fast_index(COMMPORT_485_REC),llvc_rec_state[COMMPORT_485_REC-1].total_count);//485I重新统计
//    int16u2_bin(get_readport_meter_count_from_fast_index(COMMPORT_485_CAS),llvc_rec_state[COMMPORT_485_CAS-1].total_count);//485II重新统计
//    if(bin2_int16u(llvc_rec_state[COMMPORT_PLC_REC-1].total_count)==0)//载波不存在
//    {
//        llvc_rec_state[COMMPORT_PLC_REC-1].currec_flag.value = 0x00;//未抄表
//        mem_set(llvc_rec_state[COMMPORT_PLC_REC-1].begin_datetime,6,0x00);
//        mem_set(llvc_rec_state[COMMPORT_PLC_REC-1].end_datetime,6,0x00);
//    }
//    if(bin2_int16u(llvc_rec_state[COMMPORT_485_CY-1].total_count) == 0)//交采不存在
//    {
//        llvc_rec_state[COMMPORT_485_CY-1].currec_flag.value = 0x00;//未抄表
//        mem_set(llvc_rec_state[COMMPORT_485_CY-1].begin_datetime,6,0x00);
//        mem_set(llvc_rec_state[COMMPORT_485_CY-1].end_datetime,6,0x00);
//    }
//    if(bin2_int16u(llvc_rec_state[COMMPORT_485_REC-1].total_count) == 0)//485I不存在
//    {
//        llvc_rec_state[COMMPORT_485_REC-1].currec_flag.value = 0x00;//未抄表
//        mem_set(llvc_rec_state[COMMPORT_485_REC-1].begin_datetime,6,0x00);
//        mem_set(llvc_rec_state[COMMPORT_485_REC-1].end_datetime,6,0x00);
//    }
//    if(bin2_int16u(llvc_rec_state[COMMPORT_485_CAS-1].total_count) == 0)//485II不存在
//    {
//        llvc_rec_state[COMMPORT_485_CAS-1].currec_flag.value = 0x00;//未抄表
//        mem_set(llvc_rec_state[COMMPORT_485_CAS-1].begin_datetime,6,0x00);
//        mem_set(llvc_rec_state[COMMPORT_485_CAS-1].end_datetime,6,0x00);
//    }
//
//    plc_read_count = 0;
//    port1_read_count = 0;
//    port2_read_count = 0;
//    port3_read_count = 0;
//    llvc_rec_state[COMMPORT_485_CY-1].read_vip_count = 0;
//    llvc_rec_state[COMMPORT_PLC_REC-1].read_vip_count = 0;
//    llvc_rec_state[COMMPORT_485_REC-1].read_vip_count = 0;
//    llvc_rec_state[COMMPORT_485_CAS-1].read_vip_count = 0;
//
//    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;
//    for(idx=0;idx<node_count;idx++)
//    {
//        tpos_enterCriticalSection();
//        seq_spec[0]=fast_index_list.fast_index[idx].seq_spec[0];
//        seq_spec[1]=fast_index_list.fast_index[idx].seq_spec[1];
//        metrer_port=fast_index_list.fast_index[idx].port;
//        tpos_leaveCriticalSection();
//
//        meter_idx = bin2_int16u(seq_spec) & FAST_IDX_MASK;
//
//        if(!file_exist(meter_idx)) continue;
//
//        //判断是否是重点表
//        fread_ertu_params(EEADDR_SET_F35+(meter_idx/8),&value,1);
//        is_F35 = ((~value) & (0x01<<(meter_idx%8))) ? TRUE : FALSE;
//
//        if(metrer_port == COMMPORT_PLC)
//        {
//            if(check_F161_day_hold(meter_idx,1,FALSE))
//            {
//               plc_read_count ++ ;
//
//               if(is_F35)
//               {
//              llvc_rec_state[COMMPORT_PLC_REC-1].read_vip_count ++;
//               }
//            }
//
//        }
//        if(metrer_port == COMMPORT_485_REC)
//        {
//            if(check_F161_day_hold(meter_idx,1,FALSE))
//            {
//               port2_read_count ++ ;
//
//               if(is_F35)
//               {
//                    llvc_rec_state[COMMPORT_485_REC-1].read_vip_count ++;
//               }
//            }
//
//        }
//        if(metrer_port == COMMPORT_485_CAS)
//        {
//            if(check_F161_day_hold(meter_idx,1,FALSE))
//            {
//               port3_read_count ++ ;
//               if(is_F35)
//               {
//                    llvc_rec_state[COMMPORT_485_CAS-1].read_vip_count ++;
//               }
//            }
//
//        }
//        if(metrer_port == COMMPORT_485_CY)
//        {
//            if(check_F161_day_hold(meter_idx,1,FALSE))
//            {
//               port1_read_count ++ ;
//               if(is_F35)
//               {
//                    llvc_rec_state[COMMPORT_485_CY-1].read_vip_count ++;
//               }
//            }
//
//        }
//
//    }
//    int16u2_bin(plc_read_count,llvc_rec_state[COMMPORT_PLC_REC-1].read_count); //要注意端口数量不同
//#endif
////重新计算成功日冻结的数量
//
//    int16u2_bin(port2_read_count,llvc_rec_state[COMMPORT_485_REC-1].read_count);
//    int16u2_bin(port3_read_count,llvc_rec_state[COMMPORT_485_CAS-1].read_count);
//    int16u2_bin(port1_read_count,llvc_rec_state[COMMPORT_485_CY-1].read_count);
//
//
//    data[0] = COUNT_OF_READPORT;
//    mem_cpy(data+1,llvc_rec_state,sizeof(LLVC_REC_STATE)*COUNT_OF_READPORT);
//    rec_state = (LLVC_REC_STATE*)(data+1);
//
//    for(idx=0;idx<COUNT_OF_READPORT;idx++)
//    {
//        set_byte2BCD(rec_state->begin_datetime,6);
//         if(rec_state->begin_datetime[3] == 0)rec_state->begin_datetime[3] =0x01;
//         if(rec_state->begin_datetime[4] == 0)rec_state->begin_datetime[4] =0x01;
//        check_A1_week(rec_state->begin_datetime);
//        set_byte2BCD(rec_state->end_datetime,6);
//        if(rec_state->end_datetime[3] == 0)rec_state->end_datetime[3] = 0x01;
//        if(rec_state->end_datetime[4] == 0)rec_state->end_datetime[4] = 0x01;
//        check_A1_week(rec_state->end_datetime);
//        rec_state++;
//    }
//
//    #ifdef __PROVICE_HENAN__
//    mem_cpy(jc_llvc,data+1,sizeof(LLVC_REC_STATE));
//    mem_cpy(data+1,data+1+sizeof(LLVC_REC_STATE),2*sizeof(LLVC_REC_STATE));
//    data[1] = 1;
//    data[1+sizeof(LLVC_REC_STATE)] = 2;
//    jc_llvc[0] = 0x1D;
//    mem_cpy(data+1+2*sizeof(LLVC_REC_STATE),jc_llvc,sizeof(LLVC_REC_STATE));
//    #endif
//
//    return sizeof(LLVC_REC_STATE)*COUNT_OF_READPORT+1;
//}



//INT16U get_c1_F170(objRequest *pRequest,INT8U* data)
//{
//    INT16U meter_idx;
//    INT8U read_datetime[5];
//    INT8U datalen;
//
//    meter_idx = ((READ_CONTEXT *)(pRequest->context))->meter_seq;
//
//    datalen = app_readdata_cur_data(meter_idx,0x0000AEC0,read_datetime,data,60,TRUE);
//    if(datalen > 0) return datalen;
//    mem_set(data,sizeof(METER_READ_INFO),0xEE);
//    return sizeof(METER_READ_INFO);
//}

//
///*+++
//  功能：当前日、月电能量
//
//
//---*/
//INT16U  get_fn_spot_amounts(objRequest *pRequest,objResponse *pResp)
//{
//    INT32U left_time_10ms=0,data_cur,data_rec;
//    INT16U data_len,require_len,data_pos;//预计长度
//    INT8U rate,idx,block_num;//,pread_pos;
//    INT8U idx_tmp;
//    INT8U *pread,*preq,td[7];
//    INT8U yesterday[3],rec_datetime[5],former_month[2];
//    BOOLEAN  is_valid1,is_valid2;
//    tagFN_ITEM *pFn;
//    READ_PHY_C1  read_phy_c1,read_phy_rec;
//    METER_DOCUMENT *meter_doc;
//    tagPROTOCOL_LIB item_lib,*pLib;
//    READ_CONTEXT *readC1;
//
//    if(pRequest->context==NULL)
//    {
//        return 0;
//    }
//    readC1 =  (READ_CONTEXT *)(pRequest->context);
//    if(readC1->meter_seq==0)
//    {
//        return 0;
//    }
//
//    td[5]= 0x03;   //60分钟一次
//    td[6]= 0x02;   //2个点
//    data_len = 0;
//
//    pFn = (tagFN_ITEM *)pRequest->fn;
//    pLib = (tagPROTOCOL_LIB *)&item_lib;
//    meter_doc =  &readC1->meter_doc;
//    rate = 4;
//
//    //判断使用较大的buffer
//    //todo 计算fn预计长度是否< 剩余缓冲区长度  剩余长度不够返回  分帧
//
//    if(0 == pRequest->item_count)
//    {
//        return data_len;//todo
//    }
//
//    //缓冲区不足预计长度 需要分帧
//    require_len = get_fn_len(pRequest,pFn);
//    if(require_len + pResp->pos>pResp->max_reply_pos) //todo需要加上cs等长度
//    {
//        readC1->need_more_frame=TRUE;
//        return 0;
//    }
//    require_len += pResp->pos;
//
//    data_pos =  pResp->pos;
//    for(idx=pRequest->item_count;idx>0;idx--)
//    {
//
//        pFn = &((tagFN_ITEM*)(pRequest->fn))[idx-1];
//
//        //
//        if(pFn->phy & 0x80000000)
//        {
//            if(pFn->phy == 0x80000002) //todo费率根据存储读取
//            {
//                mem_set(pResp->frame + data_pos+pFn->offset,1,rate);
//            }
//            if(pFn->phy == 0x80000003) //  小时冻结
//            {
//                //上1小时的15分钟位置起
//                mem_set(pResp->frame + data_pos+pFn->offset, 1, td[HOUR-1]);//HOUR-1对应  时
//                mem_set(pResp->frame + data_pos+pFn->offset+1, 1, td[5]); //冻结密度  15分钟
//            }
//            if(pFn->phy == 0x80000005)
//            {
//                mem_cpy(pResp->frame + data_pos+pFn->offset,&(readC1->Td),3);
//            }
//            else if(pFn->phy == 0x80000006)
//            {
//                mem_cpy(pResp->frame + data_pos+pFn->offset,&(readC1->Td),2);
//            }
//            else if(pFn->phy == FK2005_XSDJ_SJ_MD_XN)
//            {                  
//                #ifdef __QGDW_FK2005__
//                mem_set(pResp->frame + data_pos+pFn->offset, 1, (td[HOUR-1] | (td[5]<<6)));//HOUR-1对应  时
//                #endif
//            }
//            convert_phy_to_376(pFn->format&0x7FFF,pResp->frame + data_pos+pFn->offset);//
//            data_len +=  pFn->len;
//            pResp->frame_len+=pFn->len;
//            continue;
//        }
//
//        //
//        for(block_num=0;block_num<pFn->num;block_num++)
//        {
//            read_phy_c1.phy = pFn->phy;
//            read_phy_c1.meter_seq = readC1->meter_seq;
//            read_phy_rec.phy =  pFn->phy_son;
//
//            //使用较大的缓冲区 pResp pRequest
//            if(require_len < pRequest->frame_len)
//            {
//                pread = pResp->frame+require_len;
//                preq =  pRequest->frame+pRequest->frame_len;
//                read_phy_c1.left_len =  SIZE_OF_APP_PROTOCOL_FRAME - require_len;
//                mem_set(pread,read_phy_c1.left_len,DATA_DEFAULT); //初始化fn预计长度
//                read_phy_c1.resp = pread;
//            }
//            else
//            {
//                pread = pRequest->frame+pRequest->frame_len;
//                preq =  pResp->frame+require_len;
//                read_phy_c1.left_len =  SIZE_OF_APP_PROTOCOL_FRAME - pRequest->frame_len;
//                mem_set(pread,read_phy_c1.left_len,DATA_DEFAULT);
//                read_phy_c1.resp = pread;
//            }
//
//            if(FALSE == get_data_item_bydate(read_phy_c1.phy,meter_doc,pLib,0))//if(FALSE == get_data_item(read_phy_c1.phy,meter_doc->protocol,pLib))  //todo  97及其他规约
//            {
//                mem_set(pResp->frame+data_pos+pFn->offset+pFn->len*block_num,pFn->len,REC_DATA_IS_DENY);
//                pResp->frame_len += pFn->len;
//                data_len += pFn->len;
//                continue;
//
//            }
//
//            if(readC1->work_flag == 0x07)     //小时冻结电能量
//            {
//                data_len +=  get_hour_hold_spot_amounts_phy((readC1->meter_seq), read_phy_c1.phy, td, (pResp->frame+data_pos+pFn->offset), (read_phy_c1.left_len>255? 255:read_phy_c1.left_len),pFn,readC1);
//
//                break;    //Fn中当前条 的 读取数据后已处理、填充
//            }
//            else
//            read_phy_c1.len = read_real_phy(preq,pread,read_phy_c1.phy,meter_doc,left_time_10ms,read_phy_c1.left_len);
//            //读取冻结值
//            if(readC1->work_flag == 0x01)
//            {
//                get_yesterday(yesterday);
//                to_bcd(yesterday,3);
//                read_phy_rec.len = read_his_phy(preq,pread+read_phy_c1.len,read_phy_rec.phy,read_phy_c1.meter_seq,yesterday,rec_datetime,read_phy_c1.left_len,0,meter_doc);
//            }
//            else
//            {
//                get_former_month(former_month);
//                to_bcd(former_month,2);
//                read_phy_rec.len = read_his_phy(preq,pread+read_phy_c1.len,read_phy_rec.phy,read_phy_c1.meter_seq,former_month,rec_datetime,read_phy_c1.left_len,1,meter_doc);
//            }
//            if((pLib->item_prop&0x3F) != 0)
//            {
//                pLib->len = pLib->len/(pLib->item_prop&0x3F);
//            }
//            //否认或者超时
//            if((read_phy_c1.len==0)||(read_phy_rec.len==0)||((read_c1_time_out<0)&&(read_phy_c1.len==0)))
//            {
//                if((read_phy_c1.phy&0x0000003F) == 63)
//                {
//                    mem_set(pResp->frame+data_pos+pFn->offset+pFn->len*block_num,pFn->len*(pLib->item_prop&0x3F),REC_DATA_IS_DENY);
//                    pResp->frame_len += pFn->len*(pLib->item_prop&0x3F);
//                    data_len += pFn->len*(pLib->item_prop&0x3F);
//                    break;
//                }
//                else
//                {
//                    mem_set(pResp->frame+data_pos+pFn->offset+pFn->len*block_num,pFn->len,REC_DATA_IS_DENY);
//                    pResp->frame_len += pFn->len;
//                    data_len += pFn->len;
//                    read_phy_c1.phy++;//块内物理量默认连续
//                    continue;
//                }
//            }
//
//            //计算电能量
//            for(idx_tmp = 0; idx_tmp < 5; idx_tmp++)
//            {
//                data_cur = bcd2u32(pread+(PHY_LEN+RES_LEN+4)*idx_tmp+5,4,&is_valid1);     //初值
//                data_rec = bcd2u32(pread+read_phy_c1.len+(PHY_LEN+RES_LEN+4)*idx_tmp+5,4,&is_valid2);    //当前值
//
//                if((is_valid1 && is_valid2)&&(data_cur >= data_rec))
//                {
//                    data_cur -= data_rec;                   //差值
//                    data_cur *= 100;                      //格式A.13
//                    ul2bcd(data_cur,pResp->frame+data_pos+pFn->offset+pFn->len*idx_tmp,4);
//                }
//                else
//                {
//                    mem_set(pResp->frame+data_pos+pFn->offset+pFn->len*idx_tmp,4,REC_DATA_IS_DEFAULT);
//                }
//                block_num ++;
//            }
//            pResp->frame_len += (pFn->len*5);
//            data_len += (pFn->len*5);
//        }
//
//    }
//    return  data_len;
//}
//
//INT8U get_respose_count(INT8U *pread,INT16U len)
//{
//    INT16U idx;
//    INT8U  num;
//
//    if(len<5)
//        return 0;
//    num = 0;
//    for(idx=0;idx<len;(idx+=(5+pread[idx+4])))
//    {
//        if(pread[idx+4]!=0)
//        {
//            num++;
//        }
//        else
//            return num;
//    }
//    return num;
//}
//

INT8U trans_curve_midu(INT8U midu)
{
    switch(midu)
    {
    case 15: return 1;
    case 30: return 2;
    case 60: return 3;
    case 5: return 254;
    case 1: return 255;
    }
    return 0;
}


/***************************
一类数据Fn抄读接口
***************************/
INT16U app_get_fn_data_c1(objRequest *pRequest,objResponse *pResp)
{
    INT32U item,data_rec=0,data_cur=0;
    INT16U require_len,left_len,data_len=0;
    INT8U idx,idx_item,block_num,pfn_num,res_len=0,res_len_tmp=0;
    INT8U *pread,*pbuffer,*param=NULL;
    INT8U td_tmp[4]={0};
    BOOLEAN  is_valid1,is_valid2;
    READ_CONTEXT *readC1;
    tagFN_ITEM *pFn;
    tagDatetime datetime;
    INT8U item_tmp[6] = {0};
    
    os_get_datetime(&datetime);
    
    pFn = (tagFN_ITEM *)pRequest->fn;
    readC1 =  (READ_CONTEXT *)(pRequest->context);
    
    if(pRequest->item_count == 0) return 0;
    read_c1_time_out = read_c1_time_out - (system_get_tick10ms() - pRequest->recv_time_in_10ms);

    require_len = get_fn_len(pRequest,pFn);
    if(require_len + pResp->pos > pResp->max_reply_pos) //超过帧长时要分帧
    {
        readC1->need_more_frame=TRUE;
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
        

        pfn_num = 1;
        for(block_num=0;block_num<pfn_num;block_num++) 
        {
            //读取数据
            if(item!=0xFFFFFFFF)
            {
                res_len = app_read_his_item(pFn->item,pread,pbuffer,left_len,item_tmp,read_c1_time_out);
            }
            else
            {
                res_len = 0;
            }

            mem_set(pResp->frame+pResp->pos+pFn->offset,pFn->len,REC_DATA_IS_DEFAULT);
            //电量计算
            if(((readC1->work_flag == 0x0E)||(readC1->work_flag == 0x0F))&&(res_len!=0))
            {
                item = app_get_item_with_td(pFn->item,pFn->prop,td_tmp,readC1->work_flag);
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
                    for(idx=0; idx<pFn->len ; idx++)
                    {
                        if(is_valid_bcd(pread+idx, 1) == FALSE)
                        {
                            pread[idx] = 0xEE;
                        }        
                    }
                    if(!(NOT_FL_DATA & pFn->prop))
                    {
                        //todo  对数据内容BCD的判断     回复数据大于5字节的合法报文的处理？ 是否使用res_len回填
                        mem_cpy(pResp->frame+pResp->pos+pFn->offset,pread,pFn->len);
                    }
                    else
                    {
                        mem_cpy(pResp->frame+pResp->pos+pFn->offset,pread,pFn->len);
                    }
                }
            }
            pResp->frame_len += pFn->len;
            data_len += pFn->len;
        
        }
    }
    return data_len;
}


INT16U read_C1_F1(INT8U *resp)
{
    INT16U size;

    mem_cpy(resp, (INT8U *)ertu_cfg_verinfo,41);

    //厂商代码
    //size=read_custom_param(CONST_CFG_VENDOR,param);
    if(size > 0)
    {
       //mem_cpy(resp,param,4);
    }

    //需要更新下设备编号
    bin2hex(resp+4,gSystemInfo.ertu_devid[1]);
    bin2hex(resp+6,gSystemInfo.ertu_devid[0]);
    bin2hex(resp+8,gSystemInfo.ertu_devid[3]);
    bin2hex(resp+10,gSystemInfo.ertu_devid[2]);

//    //软件版本号 CONST_SOFT_VER 4
//    size=read_custom_param(CONST_SOFT_VER,param);
//    if(size > 0)
//    {
//        mem_cpy(resp+12,param,4);
//    }
//
//    //软件发布日期：日月年  CONST_SOFT_DATE 3
//    size=read_custom_param(CONST_SOFT_DATE,param);
//    if(size > 0)
//    {
//        mem_cpy(resp+16,param,3);
//      }
//
//    //终端的容量信息码  CONST_CAPACITY  11
//    size=read_custom_param(CONST_CAPACITY,param);
//    if(size > 0)
//    {
//        mem_cpy(resp+19,param,11);
//    }
//
//    #ifndef __QGDW_FK2005__
//    //通信协议版本号 CONST_PROTOCOL_VER 4
//    size=read_custom_param(CONST_PROTOCOL_VER,param);
//    if(size > 0)
//    {
//        mem_cpy(resp+30,param,4);
//      }
//
//    //硬件版本号  CONST_HARDWARE_VER 4
//    size=read_custom_param(CONST_HARDWARE_VER,param);
//    if(size > 0)
//    {
//        mem_cpy(resp+34,param,4);
//    }
//
//    //硬件发布日期 CONST_HARDWARE_DATE 3
//    size=read_custom_param(CONST_HARDWARE_DATE,param);
//    if(size > 0)
//    {
//        mem_cpy(resp+38,param,3);
//    }


    return LEN_ERTU_CFG_VERINFO;

}




