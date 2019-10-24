#include "app_3761.h"
#include "ErtuParam.h"
#include "../main_include.h"


//=====================================================================================
//
//                    终端配置信息
//
//=====================================================================================
//移到ErtuParam.h
//#define SOFT_VER_YEAR  0x18
//#define SOFT_VER_MONTH 0x11
//#define SOFT_VER_DAY   0x02
//
//
//#define SOFT_VER_MAIN   '1'
//#define SOFT_VER_DOT    '.'
//#define SOFT_VER_MIN1   '0'
//#define SOFT_VER_MIN2   '2'
//
//#define HARDWARE_VER_H   '1'
//#define HARDWARE_VER_DOT '.'
//#define HARDWARE_VER_L1  '0'
//#define HARDWARE_VER_L2  '1'

struct _DataItem
{
  INT16U fn;  //信息类标识
  INT32U eeAddr;  //EEPROM中的存放位置
  INT16U dataLen; //D0-D11为参数长度, 低字节为长度,高字节为测量点属性
};

const struct _DataItem DataItemList[]  =
{
   //-------------终端----------------
   {DT_F1, EEADDR_SET_F1,   6},    //终端通信参数
   {DT_F3, EEADDR_SET_F3,  28},    //主站IP地址和端口以及APN
   {DT_F4, EEADDR_SET_F4,  16},    //主站电话号码和短信中心号码
   {DT_F7, EEADDR_SET_F7,  64},    //终端IP地址和端口
   {DT_F8, EEADDR_SET_F8,   8},    //终端上行通信工作方式（以太专网或虚拟专网）
   {DT_F9, EEADDR_SET_F9,  16},    //终端事件记录配置设置
   {DT_F10,EEADDR_SET_F10, 33},    //终端电能表/交流采样装置配置参数,
   {DT_F16,EEADDR_SET_F16, 64},    //虚拟专网用户名及密码

//   {DT_F25,EEADDR_SET_F25,  11},     //测量点基本参数
//   {DT_F26,EEADDR_SET_F26,  57},     //测量点限值参数
//   {DT_F27,PIM_METER_F27,  24},     //测量点铜损，铁损参数
//   {DT_F28,PIM_METER_F28,   4},     //测量点功率因数分段限值
//   {DT_F29,PIM_METER_F29,   12},     //终端当地电能表显示号
//   {DT_F30,PIM_REC_FLAG,    1},     //终端台区集中抄表停抄/投抄设置
//   {DT_F31,PIM_AUX_NODES,  121},    //载波从节点附属节点地址,按最大20个附属节点设计
//
//   {DT_F33,EEADDR_SET_F33,  110},    //终端台区集中抄表运行参数设置：支持抄表端口3个
//   {DT_F34,EEADDR_SET_F34,  186},    //与终端接口的通信模块的参数设置:支持31个端口
//   {DT_F35,EEADDR_SET_F35,  256},    //终端台区集中抄表重点户设置，按位对应表示测量点序号，共可以表示2048个表
//   {DT_F36,EEADDR_SET_F36,   4},    //通信流量保护门限
//   {DT_F37,EEADDR_SET_F37,  23},   //终端级联通信参数：支持4个
//   {DT_F38,EEADDR_SET_F38,  32},   //1类数据配置设置（16*16*32 =8192字节）
//   {DT_F39,EEADDR_SET_F39,  32},   //2类数据配置设置（16*16*32 =8192字节）
//   {DT_F57,EEADDR_SET_F57,   3},    //终端声音告警允许?禁止设置
//   {DT_F58,EEADDR_SET_F58,   1},    //终端自动保电参数
//   {DT_F59,EEADDR_SET_F59,   5},    //电能表异常判别阈值设定
//
//    //------------任务号 4个任务---------------------------
//   {DT_F65,EEADDR_SET_F65,   1029},    //定时发送1类数据任务设置 1029*4=65856
//   {DT_F66,EEADDR_SET_F66,   1029},    //定时发送2类数据任务设置 1029*4=65856
//   {DT_F67,EEADDR_SET_F67,    1},     //定时发送1类数据任务启动/停止设置 1*4
//   {DT_F68,EEADDR_SET_F68,    1},     //定时发送2类数据任务启动/停止设置 1*4
//
   #ifdef __RTUADDR_IS_6BYTE__
   {DT_F89,EEADDR_CFG_DEVID, 6},     //终端地址
   #else
   {DT_F89,EEADDR_CFG_DEVID, 4},     //终端地址
   #endif
//
//   {DT_F97,EEADDR_SET_F97,    25},    //停电数据采集配置参数(存储位置考虑了最多10个测量点，实际设置只可以最多设置3个)
//   {DT_F98,EEADDR_SET_F98,  13},    //停电事件甄别限值参数
//
//    {DT_F104,PIM_METER_F104,2},        //曲线冻结密度参数
//    #ifdef __METER_EVENT_GRADE__
//    {DT_F105,PIM_METER_F105,1},        //电能表事件分级归类参数
//    {DT_F106,EEADDR_NONE,1},        //电能表事件分级参数
//    {DT_F107,EEADDR_SET_F107,2},        //电能表事件分级周期参数
//    #endif
   {DT_F301,EEADDR_SET_F301,264},    //域名登录参数
   {DT_F701,EEADDR_SET_F701,4},    //电表密码
   {DT_F702,EEADDR_SET_F702,5},    //拉合闸 有效标志1字节+起始结束时间 4字节

};


/*+++
   功能： 查找设备参数代码的地址和数据长度
   返回值： TRUE/FALSE
---*/
BOOLEAN  GetParamAddrLen(
        INT16U fn,    //[IN] 信息类编码
        INT32U *addr,    //[OUT]存储地址
        INT16U *len)     //[OUT]数据项长度
{
   INT16U idx,min,max;
   
   *len=0;
   min=0;
   max = sizeof(DataItemList) / sizeof(struct _DataItem) -1;
   while(min<=max)
   {

      idx=(min+max)>>1;   //除2
      if(fn == DataItemList[idx].fn)
      {
           *addr = DataItemList[idx].eeAddr;
           *len  = DataItemList[idx].dataLen & 0x0FFF;  //低12字节为数据项长度
           return TRUE;
      }
      else if(fn > DataItemList[idx].fn)
      {
         min = idx+1;
      }
      else 
      {
         if(idx==0) break; // 比最小值还要小
         max = idx-1;              
      }
   } 
   return FALSE;
}




//F1：终端版本信息,读取时需要更新下设备编号
const INT8U  ertu_cfg_verinfo[LEN_ERTU_CFG_VERINFO]=
{
   'T','O','P','S',                             //厂商代号          ASCII	4

   '0','0', '0','0','0','0','0','0',            //设备编号          ASCII	8


   SOFT_VER_MAIN,SOFT_VER_DOT,SOFT_VER_MIN1,SOFT_VER_MIN2, //终端软件版本号    ASCII	4



   SOFT_VER_DAY,SOFT_VER_MONTH,SOFT_VER_YEAR,   //终端软件发布日期：日月年  数据格式20	3

   'G','5','5','0','0','0','0','0','0','4','M', //终端配置容量信息码	ASCII	11

   '1','3','0','1',                             //终端通信规约版本号	ASCII	4

   'HARDWARE_VER_H','HARDWARE_VER_DOT','HARDWARE_VER_L1','HARDWARE_VER_L2',  //终端硬件版本号	ASCII	4

   0x01,0x08,0x17,                              //终端硬件发布日期：日月年	数据格式20	3
};

//获得集中器参数值的实际长度，有一些参数不是定长的，先按最大长度保存（多保存了没有关系），然后再更新长度

INT16U get_ertu_params_length(INT8U *frame,INT16U fn,INT16U itemDataLen)
{
    INT16U resp_len;
    INT16U pos;
    INT8U  i,j,tmp;
    resp_len = itemDataLen;
    switch(fn)
    {
    case DT_F41:
        pos = 0;
        resp_len = 1;
        for(j=0;j<3;j++)
        {
            if(frame[0]&(0x01<<j))
            {
                resp_len ++;
                pos++;
                tmp = frame[pos];
                for(i=0;i<8;i++)
                {
                    if(tmp&(0x01<<i))
                    {
                        resp_len+=2;
                        pos+=2;
                    }
                }
            }
        }
         break;
    case DT_F97:
        resp_len = frame[2]+frame[3]*0x100;
        break;
    case DT_F251:
        if(frame[0] > 140)
        {
            frame[0] = 0;
        }
        resp_len = frame[0]+1;
        break;
    case DT_F252:
//        if(frame[0] > 3)
//        {
//            frame[0] = 3;
//        }
        resp_len = 1;
        pos = 1;
        for(i=0;i<frame[0];i++)
        {
            pos += 8;
            resp_len += 8;
            tmp = frame[pos];
            resp_len += 1;
            pos += 1;
            resp_len += tmp;
            pos += tmp;
        }
        break;
    case DT_F301:
        resp_len = frame[8]+9;
        break;
    case DT_F303:
        resp_len = 6+1+frame[6]*(1+6+6+1);
        break;
    default:
        break;
    }
    return resp_len;
} 


/*+++
  功能: 设置终端类参数
  参数:
        INT8U *buffer,             [输入]缓冲区
        INT16U fn,                 [输入]单一的信息类
        INT8U *itemData,           [输入]数据
        INT16U *itemDataLen        [输出]数据项目长度
  返回:
        INT8U 设置结果
---*/
INT8U  set_ertu_params(INT16U fn,INT8U *itemData,INT16U *itemDataLen)
{
   INT32U   eeAddr;
   INT8U    error,itemData_tmp[50]={0};



   error = 0;

   switch(fn)
   {
   case DT_F796:
       error = set_ertu_custom_param(itemData,itemDataLen);
       break;
   default:
        //其他终端参数,固定参数长度,直接写入即可
		
        //提取数据单元的长度，存储地址
        if(TRUE == GetParamAddrLen(fn,&eeAddr,itemDataLen) )
        {
            if(EEADDR_NONE == eeAddr)
            {
                error = 1;
                break;
            }
            //写入参数
            *itemDataLen = get_ertu_params_length(itemData,fn,(*itemDataLen));
			//读出来比对是否变更
			if(DT_F3==fn)
			{
				fread_ertu_params(eeAddr,itemData_tmp,(*itemDataLen));
			}
            fwrite_ertu_params(eeAddr,itemData,(*itemDataLen));

            if(DT_F36==fn)
            {
//                fread_ertu_params(EEADDR_SET_F36,g_run_param.max_month_gprs_bytes,4);
//                if(gSystemCtrl.net_disconnect_flag==0xAA)
//                {
//                        gSystemCtrl.net_disconnect_flag=0x00;
//
//                }
//                if(gSystemCtrl.gprs_bytes_over_flag!=0)
//                {
//                    gSystemCtrl.gprs_bytes_over_flag =0x00;
//                    fwrite_array(FILEID_RUN_DATA,FMDATA_GPRS_BYTES_OVER_CHECK_EVENT,&gSystemCtrl.gprs_bytes_over_flag,1); 
//                }
            }
			else if(DT_F3 == fn)
			{
				if(compare_string(itemData,itemData_tmp,(*itemDataLen))!=0)
				{
					gSystemInfo.reconnect = 1;						
				}
			}
            else if(DT_F89 == fn)
            {
				mem_cpy(gSystemInfo.ertu_devid,itemData,CFG_DEVID_LEN);
            }
            else if(DT_F701 == fn)
            {
                mem_cpy(&g_app_run_data.password,itemData,4);
            }            
            else if(DT_F702 == fn)
            {
                mem_cpy(&g_app_run_data.meter_control.is_control_valid,itemData,sizeof(tagAPP_CONTROL));
            }
        }
        else
        {
            error=1;
            *itemDataLen = MAX_SIZE_PLMSDTP;
        }
        break;
   }
   return error;
}


/*+++
  功能：查询参数（报文途径）
  参数:
        INT16U pn
        INT16U fn
        INT8U *params  带参数的查询命令的内容
        INT8U *itemData  响应内容
        INT16U *itemDataLen   响应的长度
        QUERY_CTRL *query_ctrl 查询过程的结构体
        BOOLEAN *more_frame_flag   是否需分帧
  返回:
        查询结果
  描述：

---*/

INT8U process_frame_query_3761(INT16U fn,INT8U *itemData,INT16U *itemDataLen,QUERY_CTRL *query_ctrl)
{
    INT32U eeAddr;
    INT16U meter_count=0;
    INT8U error;
    error = 0;
    eeAddr = EEADDR_NONE;

    //查找参数的地址及长度信息
    if((fn!=0)&&(fn!=DT_F796) )
    {
        if(FALSE == GetParamAddrLen(fn,&eeAddr,itemDataLen) )
        {
            //不能理解某个信息类，则发送否定报文
            error = 1;
            return error;
        }
    }
    switch(fn)
    {
        case DT_F89:
        #ifndef __RTUADDR_IS_6BYTE__
            fread_ertu_params(EEADDR_CFG_DEVID,itemData,4);
            *itemDataLen = 4;
        #else
            fread_ertu_params(EEADDR_CFG_DEVID,itemData,6);
            *itemDataLen = 6;
        #endif
            break;
        case DT_F796:
            query_ertu_custom_param(itemData,itemDataLen,(INT8U*)query_ctrl);
            break;
        default:
            if(EEADDR_NONE==eeAddr  || 0x00==fn)  break;

            fread_ertu_params(eeAddr,itemData,(*itemDataLen));
            *itemDataLen = get_ertu_params_length(itemData,fn,(*itemDataLen));
            if(DT_F10 == fn)
            {
                mem_cpy(gmeter_doc.meter_no,gSystemInfo.meter_no,6);
                gmeter_doc.meter_idx[0] = 1;
                gmeter_doc.meter_idx[1] = 0;
                gmeter_doc.spot_idx[0] = 1;
                gmeter_doc.spot_idx[1] = 0;
                gmeter_doc.protocol = GB645_2007;
                gmeter_doc.baud_port.port = COMMPORT_485_CY;
                gmeter_doc.fl_count = 4;
                gmeter_doc.baud_port.port = 1;
				#ifndef __METER_SIMPLE__
                gmeter_doc.meter_class.meter_class = 2;//三相2 单相1
				#else
				gmeter_doc.meter_class.meter_class = 1;//三相2 单相1
				#endif
                gmeter_doc.meter_class.user_class = 5;
                itemData[0]=1;
                itemData[1]=0;
                mem_cpy(itemData+2,(INT8U *)&gmeter_doc,sizeof(METER_DOCUMENT));
                *itemDataLen = sizeof(METER_DOCUMENT)+2;

                   meter_count = bin2_int16u(query_ctrl->req);
                   //请求参数长度
                   query_ctrl->req_bytes = (meter_count<<1) + 2;
            }
            break;
    }
    return error;

}




INT8U process_frame_set_3761(INT16U fn,INT8U *itemData,INT16U *itemDataLen,INT16U DataLen)
{
    INT8U error=0;

    if(DT_F0 == fn)
    {
        //F0:终端逻辑地址,需要是唯一的数据数据标识
        if(DataLen == 4)
        {
            fwrite_ertu_params(EEADDR_CFG_DEVID,itemData,CFG_DEVID_LEN);
            mem_cpy(gSystemInfo.ertu_devid,itemData,CFG_DEVID_LEN);

            *itemDataLen = CFG_DEVID_LEN;

            if( (0xAA == *(itemData + *itemDataLen))   && (0x55 == *(itemData + *itemDataLen + 1)) )
            {
                //厂商代码
                fwrite_ertu_params(EEADDR_CFG_VENDOR,itemData + *itemDataLen + 2,4);
                *itemDataLen += 6;
            }
        }
        else
        {
            error=1;
            *itemDataLen = DataLen + 4;  //迫使循环结束
        }
    }
    else
    {
        error = set_ertu_params(fn,itemData,itemDataLen);
    }

    return error;
}



void  process_frame_set(objRequest *pRequest,objResponse *pResp)
{
	INT16U pos_event;
    INT16U pn,fn;
    INT16U itemDataLen;
    INT16U pngroup[64] = {0};
    INT16U fngroup[8] = {0};
    INT16U tmp_pos;  //用于pn和fn组合时的处理
    INT8U result_flag,error=0;
    INT8U pngroupcount;
    INT8U fngroupcount;
    INT8U fn_idx;
    INT8U png_idx;
    INT8U fng_idx;
	INT8U event_record[EVENT_RECORD_SIZE];
	
    ClrTaskWdt();

    //TODO:控制码的处理，由统一的接口针对AFN和Fn等进行处理

    //设置控制码：DIR=1,PRM=0, FUNC=8  响应报文，用户数据
    pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_USERDATA;
    pResp->work_flag[WORK_FLAG_SEQ_RSEQ] |= MASK_FIR | MASK_FIN;
    pResp->work_flag[WORK_FLAG_SEQ_RSEQ] &= ~MASK_CON;

    //清除设置结果标志字节
    result_flag=0;

    pResp->pos = POS_DATA;
    pResp->frame[POS_AFN] = AFN_ACK;

    //首先按照按数据标识准备
    fn = DT_F3;
    pn = DA_P0;
    set_pn_fn(pResp->frame, &pResp->pos, pn, fn);
    pResp->frame[pResp->pos++]=AFN_SET;

    //读取F3:终端参数状态
    //fread_array(FILEID_RUN_DATA,PIM_C1_F3,BS248,31);


    //ERC3:参数变更记录
    event_record[EVENT_POS_ERC_LEN]=6;
    event_record[EVENT_POS_ERC_CONTENT]= pResp->frame[POS_MSA]>>1;  //启动站地址
    pos_event = EVENT_POS_ERC_CONTENT+1;

    while((pRequest->pos - POS_DATA) < pRequest->userDataLen)
    {

        //提取数据单元标识
        pn = bin2_int16u(pRequest->frame + pRequest->pos);
        fn = bin2_int16u(pRequest->frame + pRequest->pos + 2);

         //允许自定义设置终端地址，但不允许非第一个pnfn为0
        if((pn==0) && (fn==0) && ((pRequest->pos - POS_DATA)>0))
        {
            break;
        }

        //确认报文中的数据单元标识
        set_pn_fn(pResp->frame, &pResp->pos, pn, fn);

        pRequest->pos += 4;

        //信息点集合处理,处理时遵循先Fn，后Pn,从小到大顺序。
        pngroupcount = parse_pn_to_pngroup(pn,pngroup);
        fngroupcount = parse_fn_to_fngroup(fn,fngroup);

        error=0;
        tmp_pos = 0;
        for(png_idx=0;png_idx < pngroupcount;png_idx++)
        {
            pn =  pngroup[png_idx];
            //多pn循环处理时，移动pos指针保证fn处理的正确性
            pRequest->pos -= tmp_pos;
            tmp_pos = 0;
            for(fng_idx=0;fng_idx<fngroupcount;fng_idx++)
            {
                fn = fngroup[fng_idx];

                //设置参数标志位
                fn_idx = trans_set_fn_2_fnidx(fn);

                if(event_record[EVENT_POS_ERC_LEN] < EVENT_RECORD_SIZE-2)
                {
                    //参数变更事件记录：数据单元标识

                    mem_cpy(event_record+pos_event,(INT8U *)&pn,2);
                    pos_event += 2;
                    mem_cpy(event_record+pos_event,(INT8U *)&fn,2);
                    pos_event += 2;
                    event_record[EVENT_POS_ERC_LEN]+=4;
                }

                error = process_frame_set_3761(fn,pRequest->frame+pRequest->pos,(INT16U*)&itemDataLen,pRequest->pos-POS_DATA);

                if((error == 0)&&(g_meter_set_flag == 0x55))
                {
                    if(DT_F1 == fn)
                    {
                        set_bit_value(&(gSystemInfo.ertu_param_change),1,0);
                    }
                    if(DT_F3 == fn)
                    {
                        set_bit_value(&(gSystemInfo.ertu_param_change),1,1);
                    }
                    if(DT_F16 == fn)
                    {
                        set_bit_value(&(gSystemInfo.ertu_param_change),1,2);
                    }
                    if(DT_F89 == fn)
                    {
                        set_bit_value(&(gSystemInfo.ertu_param_change),1,3);
                    }
                }

                if(DT_F1 == fn)
                {
                    //4.读取心跳
                    fread_ertu_params(EEADDR_HEARTCYCLE,(INT8U*)&(gSystemInfo.heart_cycle),1);
                }
           
                if((pResp->channel == CHANNEL_RS232) && (DT_F3==fn))
                {
                    //set_system_flag(SYS_GPRS_CHG,SYS_FLAG_BASE);
                }

                //如果F7参数发生改变且不是以太网那么集中器重新装在F7参数
                if((pResp->channel!=CHANNEL_ETH_SERVER) && (pResp->channel!=CHANNEL_ETH_CLIENT) && (DT_F7==fn))
                {
                    //tops_platform_set_eth_update_param();
                }

                pRequest->pos += itemDataLen;
                tmp_pos += itemDataLen;
                if(error!=0) break;
            }// end of for(fng_idx...
            if(error!=0) break;
        }//end of for(png_idx....

        pResp->frame[pResp->pos++]=error;
        if(error==0)
            result_flag |= 0x01;
        else
            result_flag |= 0x02;
        if(error!=0) break;


    } // end of while..

    //参数设置成功后写终端参数设置状态F3
    if(error == 0)
    {
        //fwrite_array(FILEID_RUN_DATA,PIM_C1_F3,BS248,31);
    }


    //结束设置参数
    pn = DA_P0;
    if(result_flag !=0)
    {
        fn = (result_flag==1) ? DT_F1 :  DT_F2;
        pResp->pos=POS_DATA;
        set_pn_fn(pResp->frame, &pResp->pos, pn, fn);
    }


    app_setFrameLenThenSendFrame(pRequest,pResp);

  //  成功后，记录参数变更事件
   if(error == 0)
    event_params_set(event_record);

}

/*+++
   功能： 查询参数
   参数:
   objRequest *pRequest 请求帧工作区
   objResponse *pResp   应答帧工作区

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

void  process_frame_query(objRequest *pRequest,objResponse *pResp)
{
    INT16U pn,fn;
    INT16U pngroup[64]={0};
    INT16U fngroup[8]={0};
    INT16U itemDataLen;
    INT16U pn_idx;
    QUERY_CTRL  query_ctrl;
    INT8U error;
    INT8U png_idx,fng_idx;
    INT8U pngroupcount;
    INT8U fngroupcount;
    BOOLEAN  more_frame_flag;


    ClrTaskWdt();
    //设置控制码：DIR=1,PRM=0, FUNC=8  响应报文，用户数据
    pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_USERDATA;


    //首次响应F10,F89标志
    query_ctrl.first_ctrl.first_flag = 0xFF;
    query_ctrl.from_seq = 0;
    //首帧标志置1
    pResp->work_flag[WORK_FLAG_IS_FIRST_FRAME]=1;


    /////////////////////////////////
    //    开始组织响应帧
    /////////////////////////////////

//RESP_PARAMS_QUERY:

    pResp->pos = POS_DATA;
    while((pRequest->pos - POS_DATA) < pRequest->userDataLen)
    {
       ClrTaskWdt();

       //提取数据单元标识
       pn = bin2_int16u(pRequest->frame + pRequest->pos);
       fn = bin2_int16u(pRequest->frame + pRequest->pos + 2);
       pRequest->pos += 4;

       //信息点集合处理,处理时遵循先Fn，后Pn,从小到大顺序。

       pngroupcount = parse_pn_to_pngroup(pn,pngroup);
       fngroupcount = parse_fn_to_fngroup(fn,fngroup);

       //循环处理PN,从小到大的顺序
       more_frame_flag = FALSE;
       for(png_idx=0;png_idx < pngroupcount;png_idx++)
       {
          //转换测量点为序号 DA_P0=0  DA_P1=1  DA_P2=2 DA_P3=3
          pn_idx = trans_set_pn_2_pnidx(pngroup[png_idx]);
          if(pn_idx > MAX_METER_COUNT)
          {
              continue;
          }
          //循环处理FN,从小到大的顺序
          for(fng_idx=0;fng_idx<fngroupcount;fng_idx++)
          {
             fn = fngroup[fng_idx];
             itemDataLen = 0;

             //设置数据标识
             if(fn != 0)
             {
                 set_pn_fn(pResp->frame, &pResp->pos, pngroup[png_idx], fn);
             }

             query_ctrl.req_pos = pRequest->pos;
             query_ctrl.req = pRequest->frame + pRequest->pos;
             query_ctrl.left_resp_len =  pResp->max_reply_pos - pResp->pos - 4;
             query_ctrl.req_bytes = 0;

             error = process_frame_query_3761(fn,pResp->frame+pResp->pos,&itemDataLen,(QUERY_CTRL*)&query_ctrl);

             if(error != 0)
             {
                 app_send_NAK(pRequest,pResp);
                 return;
             }
             //检查是否已经进入到多祯状态,这个状态的产生是因为信息类本身数据量很大，支持多次传输
             if(more_frame_flag==TRUE)
             {
                 if(POS_DATA == pResp->pos)
                 {
                      //不能够进行分祯传送
                      app_send_NAK(pRequest,pResp);
                      return;
                 }
                 //移动应答报文当前位置
                 pResp->pos += itemDataLen;

                 if(FALSE == app_send_response_frame(pRequest,pResp,0)) return;

                 pResp->pos = POS_DATA;
                 pRequest->pos=query_ctrl.req_pos;
                 //继续组帧
                 fng_idx--;
                 continue;
             }

             //判断是否超过最大用户数据有效长度，这个地方分祯，与上面的处理不同。
             //如超过，则进行多帧发送
             if(pResp->pos + itemDataLen + 4 > pResp->max_reply_pos)
             {
                 if(POS_DATA == pResp->pos-4)   //原来条件不会进入，因为已经加了一个PNFN ,2011.3.31 by zyg    if((POS_DATA == pos)  && (TRUE==first_frame_flag))
                 {
                      //只有首帧不能分帧时，才发送否认帧，否则，直接返回
                      if(TRUE==pResp->work_flag[WORK_FLAG_IS_FIRST_FRAME])
                      app_send_NAK(pRequest,pResp);
                      return;
                 }

                 //可以进行分祯传送,发送响应祯
                 if(FALSE == app_send_response_frame(pRequest,pResp,0)) return;
                 pResp->pos = POS_DATA;
                 pRequest->pos=query_ctrl.req_pos;
                 //继续组帧
                 fng_idx--;
                 continue;
             }
             //移动应答报文当前位置
             pResp->pos += itemDataLen;

             //移动请求参数位置
             pRequest->pos += query_ctrl.req_bytes;

          }//end of for(fng_idx...
       }//end of for(png_idx...
    } // end of while(idx...

    //发送响应祯
    app_send_response_frame(pRequest,pResp,TRUE);

}


/*+++
  功能：响应主站请求终端配置 [AFN=0x09]
  参数：
        PLMS_RESP *resp,            响应处理结构
        INT16U frameLen,            请求报文用户数据区长度
        INT8U *frameData    请求报文用户数据区
---*/
void  process_read_config_info(objRequest *pRequest,objResponse *pResp)
{
    INT16U pn,fn;
    INT16U itemDataLen;

    //设置控制码：DIR=1,PRM=0, FUNC=8  响应报文，用户数据
    pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_USERDATA;

    //首帧标志置1
    pResp->work_flag[WORK_FLAG_IS_FIRST_FRAME]=1;


    /////////////////////////////////
    //    开始组织响应帧
    /////////////////////////////////

    RESP_PARAMS_READCONFIG:

    pResp->pos = POS_DATA;
    while((pRequest->pos - POS_DATA) < pRequest->userDataLen)
    {
        //提取数据单元标识
        pn = bin2_int16u(pRequest->frame + pRequest->pos);
        fn = bin2_int16u(pRequest->frame + pRequest->pos + 2);
        pRequest->pos += 4;

        //检查信息点PN, 信息点只能是P0,如果不是,则继续检查下一个数据单元标识
        if(pn != DA_P0) continue;

        //检查信息类FN,信息类DT只能是第0组
        #ifdef __COUNTRY_ISRAEL__
        if ((fn & 0xFF00) && (fn != DT_F24))  continue;
        //#else
        //if(fn & 0xFF00) continue;
        #endif

        //计算本次读取的信息类的信息长度
        itemDataLen = compute_readcfg_fn_datalen(fn);
        if(itemDataLen == 0) continue;

        //判断是否超过最大用户数据有效长度
        //如超过，则进行多帧发送
        if(pResp->pos + itemDataLen + 4 > pResp->max_reply_pos)
        {
           //发送响应祯
           if(FALSE == app_send_response_frame(pRequest,pResp,0) ) return;
           pRequest->pos -= 4;

           //继续组帧
           goto RESP_PARAMS_READCONFIG;
        }

        //设置数据标识
        set_pn_fn(pResp->frame, &pResp->pos, pn, fn);

        //读取信息类FN的数据
        readcfg_fn_data(fn,pResp->frame + pResp->pos);
        if(fn==DT_F11)
        {
            //itemDataLen=readcfg_spot_info(pResp->frame+pResp->pos,pResp->max_reply_pos);
        }
        #ifdef __PROVICE_JIANGSU__
        //F12：本地通信模块版本信息(双模)
        if(fn == DT_F12)
        {
           itemDataLen = readcfg_plc_verinfo(pResp->frame+pResp->pos,pResp->max_reply_pos);
        }
        //F13：采集器版本信息
        if(fn == DT_F13)
        {
            itemDataLen = readcfg_cjq_verinfo(pRequest->frame + pRequest->pos,pResp->frame+pResp->pos,pResp->max_reply_pos);
        }
        #endif
		if(fn == DT_F249)
		{
			itemDataLen = app_get_F249_data(pResp->frame+pResp->pos);	
		}

        pResp->pos += itemDataLen;
    } // end of while(idx...

    //检查如果没有任何响应数据,则发送否定报文
    if(pResp->pos == POS_DATA)
    {
         app_send_NAK(pRequest,pResp);
    }
    else
    {
        //发送响应祯
        app_send_response_frame(pRequest,pResp,TRUE);
    }
}

/*+++
  功能: 请求终端配置,计算信息类包含的数据长度
  参数:
        INT16U fn
  返回:
        INT16U 数据长度,如果某个信息点无,也包含其长度
  描述:
      F1: 终端版本信息  	  41
      F2: 输入输出及通信端口配置	  91	通信端口数量按10个考虑。
      F3: 其它配置      	  16
      F4: 终端支持的参数配置      32
      F5: 终端支持的控制配置      32
      F6: 终端支持的1类数据配置   32
      F7: 终端支持的2类数据配置   32
      F8: 终端支持的事件记录配置   8
      F9: 远程通讯模块版本信息    56
      F10: 本地通讯模块版本信息   15
---*/
INT16U compute_readcfg_fn_datalen(INT16U fn)
{
   INT16U  datalen;

   datalen = 0;

   #ifdef __COUNTRY_ISRAEL__
   if(fn == DT_F24) return 1;
   #endif

   if(fn == DT_F249)  return 51;


   if((fn & 0xFF00) == 0)
   {
   #ifdef __PROVICE_SHANXI__
   if(fn & DT_F1)  datalen += LEN_ERTU_CFG_VERINFO+6;//sizeof(ertu_cfg_verinfo)+6;
   #else
   if(fn & DT_F1)  datalen += LEN_ERTU_CFG_VERINFO;//sizeof(ertu_cfg_verinfo);
   #endif
//   if(fn & DT_F2)  datalen += sizeof(ertu_cfg_ioports);
//   if(fn & DT_F3)  datalen += sizeof(ertu_cfg_options);
//   if(fn & DT_F4)  datalen += sizeof(ertu_cfg_opt_set);
//   if(fn & DT_F5)  datalen += sizeof(ertu_cfg_opt_ctrl);
//   if(fn & DT_F6)  datalen += (sizeof(ertu_cfg_opt_c1data)-2)*6+2;
//   if(fn & DT_F7)  datalen += (sizeof(ertu_cfg_opt_c2data)-2)*6+2;
//   if(fn & DT_F8)  datalen += sizeof(ertu_cfg_opt_events);
   }
   else if((fn & 0xFF00) == 0x0100)
   {
       if(fn & DT_F1)  datalen += 46;
       if(fn & DT_F2)  datalen += 15;
       if(fn & DT_F3)  datalen += 4;   //TODO:山东扩展F11 终端有效测量点，先保证有一个长度返回
       if(fn & DT_F4)  datalen += 30;  //江苏F12  本地通信模块版本信息（双模）
       if(fn & DT_F5)  datalen += 8;   //江苏F13  采集器版本信息
   }
   return datalen;
}



/*+++
  功能:请求终端配置,读取信息类包含的数据
  参数:
       INT16U fn      信息类
       INT8U  resp    数据区
---*/

void readcfg_fn_data(INT16U fn,INT8U *resp)
{
    INT16U pos;
    #ifdef __PROVICE_SHANXI__
    INT8U  temp[30];
    #endif

   pos = 0;


    #ifdef __PROVICE_HEILONGJIANG__
    if(fn == DT_F249)  //黑龙江扩展
    {
       mem_cpy(resp+pos,gSystemInfo.ertu_devid,4);//read_fmArray(EEADDR_CFG_DEVID,resp+pos,4);
       pos += 4;
       resp[pos++] = 6;  //6专变终端3型(FKXA2X)；7专变终端3型(FKXA4X)；8专变终端3型(FCXA2X)

       if(get_system_flag(SYS_CY_PAHSE_LINE,SYS_FLAG_BASE))
       {
           resp[pos++] = 1;
       }
       else
       {
           resp[pos++] = 2;
       }
       resp[pos++] = 0;
       resp[pos++] = 0x22;
       
       if(gSystemInfo.eth_client_state == ETH_NET_STATE_LOGGED_IN)
       {
           resp[pos++] = 3;//1 GPRS，2 CDMA，3以太网，4, 230 M,5 EPON, 其他保留；
       }
       else if(REMOTE_COMMTYPE == REMOTECOMM_CDMA)
       {
           resp[pos++] = 2;//1 GPRS，2 CDMA，3以太网，4, 230 M,5 EPON, 其他保留；
       }
       else
       {
           resp[pos++] = 1;//1 GPRS，2 CDMA，3以太网，4, 230 M,5 EPON, 其他保留；
       }

       mem_cpy(resp+pos,gSystemInfo.gprs_ccid,20);//read_fmArray(GPRS_INFO_SIM_ID,resp+pos,20);
       for(idx=0;idx<20;idx++)
       {
          if((resp[pos+idx]>='0')&&(resp[pos+idx]<='9')
             || (resp[pos+idx]>='A')&&(resp[pos+idx]<='Z')
             || (resp[pos+idx]>='a')&&(resp[pos+idx]<='z'))
          {
          }
          else
            resp[pos+idx] = '0';
       }
       pos += 20;
       resp[pos++] = gSystemInfo.gprs_csq;//REMOTE_GPRS.csq;
       resp[pos++] = 4;
       mem_cpy(resp+pos,"XXXXXXXXXXXX",12);
       pos += 12;
       resp[pos++] = 0;
       resp[pos++] = 0;
       resp[pos++] = 0;
       resp[pos++] = 0;
       resp[pos++] = 0;
       resp[pos++] = 0;
       resp[pos++] = 0;
       resp[pos++] = get_readport_meter_count_from_fast_index(COMMPORT_485_REC)+get_readport_meter_count_from_fast_index(COMMPORT_485_CAS);//read_fmByte(FMADDR_485_MCOUNT);
       return ;
    }
    #endif

   if((fn & 0xFF00) == 0)
   {
       // F1: 终端版本信息  	  41
       if(fn & DT_F1)
       {
          read_C1_F1(resp+pos);
          pos += sizeof(ertu_cfg_verinfo);
          #ifdef __PROVICE_SHANXI__
          mem_cpy(temp,resp + 12,29);
          mem_cpy(resp + 18,temp,29);
          fread_ertu_params(EEADDR_VENDOR_CHIP,resp+12,6); //山西要求有终端厂家及芯片厂家
          if((resp[16]==0xFF) && (resp[17]==0xFF))
          {
                resp[16]='T';
                resp[17]='C';
          }
          pos +=6;
          #endif
       }
//       // F2: 输入输出及通信端口配置	  91	通信端口数量按10个考虑。
//       if(fn & DT_F2)
//       {
//          mem_cpy(resp+pos, (INT8U *)ertu_cfg_ioports,sizeof(ertu_cfg_ioports));
//          //处理mac地址
//          #if !defined(__SOFT_SIMULATOR__) && !defined(__QGDW_FK2005__)
//          mem_cpy(resp+pos+10,gSystemInfo.eth_mac_addr,6);
//          #endif
//          pos += sizeof(ertu_cfg_ioports);
//       }
//       // F3: 其它配置      	  16
//       if(fn & DT_F3)
//       {
//          mem_cpy(resp+pos, (INT8U *)ertu_cfg_options,sizeof(ertu_cfg_options));
//          pos += sizeof(ertu_cfg_options);
//       }
//       // F4: 终端支持的参数配置      32
//       if(fn & DT_F4)
//       {
//          mem_cpy(resp+pos, (INT8U *)ertu_cfg_opt_set,sizeof(ertu_cfg_opt_set));
//          pos += sizeof(ertu_cfg_opt_set);
//       }
//       // F5: 终端支持的控制配置      32
//       if(fn & DT_F5)
//       {
//          mem_cpy(resp+pos, (INT8U *)ertu_cfg_opt_ctrl,sizeof(ertu_cfg_opt_ctrl));
//          pos += sizeof(ertu_cfg_opt_ctrl);
//       }
//        // F6: 终端支持的1类数据配置   32
//        if(fn & DT_F6)
//        {
//          mem_cpy(resp+pos, (INT8U *)ertu_cfg_opt_c1data,sizeof(ertu_cfg_opt_c1data));
//          pos += sizeof(ertu_cfg_opt_c1data);
//            for(idx=0;idx<5;idx++)
//            {
//                mem_cpy(resp+pos,((INT8U *)ertu_cfg_opt_c1data)+2,sizeof(ertu_cfg_opt_c1data)-2);
//                pos += sizeof(ertu_cfg_opt_c1data)-2;
//            }
//        }
//        // F7: 终端支持的2类数据配置   32
//        if(fn & DT_F7)
//        {
//          mem_cpy(resp+pos, (INT8U *)ertu_cfg_opt_c2data,sizeof(ertu_cfg_opt_c2data));
//          pos += sizeof(ertu_cfg_opt_c2data);
//            for(idx=0;idx<5;idx++)
//            {
//                mem_cpy(resp+pos,((INT8U *)ertu_cfg_opt_c2data)+2,sizeof(ertu_cfg_opt_c2data)-2);
//                pos += sizeof(ertu_cfg_opt_c2data)-2;
//            }
//        }
//        // F8: 终端支持的事件记录配置   8
//        if(fn & DT_F8)
//        {
//          mem_cpy(resp+pos, (INT8U *)ertu_cfg_opt_events,sizeof(ertu_cfg_opt_events));
//          //pos += sizeof(ertu_cfg_opt_events);  //消告警
//        }
   }
   else if((fn & 0xFF00) == 0x0100)
   {
        // F9: 远程通讯模块版本信息   46
        if(fn & DT_F1)
        {
            mem_cpy(resp+pos,gSystemInfo.mygmr_info,sizeof(gSystemInfo.mygmr_info));
            pos += sizeof(gSystemInfo.mygmr_info);
            mem_cpy(resp+pos,gSystemInfo.myccid_info,sizeof(gSystemInfo.myccid_info));
            pos += sizeof(gSystemInfo.myccid_info);
        }
//        // F10: 本地通讯模块版本信息   15
//        if(fn & DT_F2)
//        {
//            mem_cpy(resp+pos, gSystemInfo.plc_ver_info,sizeof(gSystemInfo.plc_ver_info));
//            #ifdef __POWER_CTRL__
//            mem_set(resp+pos,15,0x00);
//            #endif
//        }
   }

}

/*+++
  功能： 自定义设置终端参数(04F796)
  参数： INT8U *data,           参数内容
         INT16U *itemDataLen      参数长度
  返回：
         处理结果
  描述：
        1）data数据带CS，需对CS先处理
---*/
INT16U set_ertu_custom_param(INT8U *data,INT16U *itemDataLen)
{
    INT16U code;
    INT16U size;
    INT8U count;
    INT8U idx;
    INT8U crc;

    count=*data;
    data++;
    *itemDataLen=1;
    for(idx=0;idx<count;idx++)
    {
        code=bin2_int16u(data);
        size=bin2_int16u(data+2);
        data+=4;
        crc = crc16_param(data+1,size);
        if(crc!=data[0])
        {
            (*itemDataLen)+=(size+5);
            return 1;
        }

        set_custom_param(code,data+1);
        data+=(size+1);
        (*itemDataLen)+=(size+5);
    }
    return 0;
}


/*++++++++++++++++++++++++++++++++++
//描述：
//   1）data已经不带CS，纯粹的数据内容
+++++++++++++++++++++++++++++++++*/
INT16U set_custom_param(INT16U paramcode,INT8U *data)
{
    INT16U tmp;
    BOOLEAN error=FALSE;

    switch(paramcode)
    {
        case CONST_CFG_DEVID:				//1UL			//6	终端地址
            error = fwrite_ertu_params(EEADDR_CFG_DEVID,data,CFG_DEVID_LEN);
            mem_cpy(gSystemInfo.ertu_devid,data,CFG_DEVID_LEN);
            break;
        case CONST_CFG_VENDOR:				//2UL			//4	厂商代码
            error = fwrite_ertu_params(EEADDR_CFG_VENDOR,data,4);
            break;
//        case CONST_VENDOR_CHIP:				//3UL			//4	终端厂家（山西）
//            error = fwrite_ertu_params(EEADDR_VENDOR_CHIP,data,4);
//            break;
//        case CONST_VENDOR_CHIP2:			//4UL			//2芯片厂家（山西）
//            error = fwrite_ertu_params(EEADDR_VENDOR_CHIP+4,data,2);
//            break;
//        case CONST_SHANGHAI_VENDOR_FLAG:	//5UL			//2	版本标识（上海）
//            error = fwrite_ertu_params(EEADDR_SHANGHAI_VENDOR_FLAG,data,2);
//            break;
//        case CONST_SOFT_VER:				//6UL			//1+4	软件版本号
//            error = fwrite_ertu_params(EEADDR_SOFT_VER,data,4);
//            break;
//        case CONST_SOFT_DATE:				//7UL			//1+3	软件版本发布日期
//            error = fwrite_ertu_params(EEADDR_SOFT_DATE,data,3);
//            break;
//        case CONST_CAPACITY:				//8UL			//1+11	容量信息码
//            fwrite_ertu_params(EEADDR_CAPACITY,data,11);
//            break;
//        case CONST_PROTOCOL_VER:			//9UL			//1+4	协议版本号
//            error = fwrite_ertu_params(EEADDR_PROTOCOL_VER,data,4);
//            break;
//        case CONST_HARDWARE_VER:			//10UL		//1+4	硬件版本号
//            error = fwrite_ertu_params(EEADDR_HARDWARE_VER,data,4);
//            break;
//        case CONST_HARDWARE_DATE:			//11UL		//1+3	硬件版本发布日期
//            error = fwrite_ertu_params(EEADDR_HARDWARE_DATE,data,3);
//            break;
//        case CONST_ERTU_SWITCH:                        //12UL
//            //对于该开关的，存储时，认为0有效，设置和读取时均做取反操作，
//            bit_value_opt_inversion(data,ERTU_SWITCH_LEN);
//            error = fwrite_ertu_params(EEADDR_ERTU_SWITCH,data,ERTU_SWITCH_LEN);     //1+4   终端的编译开关，D0-日冻结时标抄读开关
//            break;
//        case CONST_DISPLAY_PWD:
//            fwrite_array(FILEID_RUN_PARAM, FLADDR_LOCAL_PASSWORD, data, 6);      //1+6，液晶显示密码
//            break;
//        case CONST_RS232_BAUDRATE:
//            fwrite_array(FILEID_RUN_PARAM,FLADDR_RS232_BAUDRATE,data,4);     //1+4,串口波特率
//            break;
//        case CONST_LOG_PARAM:
//            fwrite_array(FILEID_RUN_PARAM,FLADDR_LOGEVEL,data,2);        //1+2，日志参数:日志级别+日志输出方式
//            gSystemInfo.log_level=data[0];
//            gSystemInfo.log_out_type=data[1];
//            break;
        case CONST_FRAME_MONITOR:
            tmp = bin2_int16u(data);
            if(tmp&0x0004)
            {
                gSystemInfo.gprs_debug_flag = 1;
            }
            else
            {
                gSystemInfo.gprs_debug_flag = 0;
            }
            if(tmp&0x0008)
            {
                gSystemInfo.meter_debug_flag = 1;
            }
            else
            {
                gSystemInfo.meter_debug_flag = 0;
            }
            if(tmp&0x0001)
            {
                gSystemInfo.remote_app_debug_flag = 1;
            }
            else
            {
                gSystemInfo.remote_app_debug_flag = 0;
            }
            if(tmp&0x0010)
            {
                gSystemInfo.debug_info_interface = 1;
            }
            else
            {
                gSystemInfo.debug_info_interface = 0;
            }
            if(tmp&0x0020)
            {
                gSystemInfo.clock_ready = 1;
            }
            else
            {
                gSystemInfo.clock_ready = 0;
            }
            bit_value_opt_inversion(data,2);
            fwrite_ertu_params(EEADDR_FRAME_MONITOR, data, 2);      //2，报文监控
            break;
//        case CONST_RS232_REPORT:
//            fwrite_array(FILEID_RUN_PARAM, FLADDR_RS232_REPORT, data, 1);       //1+1，串口上报
//            gSystemInfo.rs232_report_type = data[0];
//            #ifndef __SOFT_SIMULATOR__
//            if(0xBB == gSystemInfo.rs232_report_type)
//            {
//                channel_gprs_rs232_init();
//            }
//            #endif
//            break;
//        case CONST_GPRS_BAUDRATE:
//            fwrite_array(FILEID_RUN_PARAM, FLADDR_GPRS_BAUDRATE, data, 4);   //4+1,GPRS串口波特率
//            #ifndef __SOFT_SIMULATOR__
//            value = data[0]+data[1]*0x100+data[2]*0x10000+data[3]*0x1000000;
//            set_gprs_uart_baudrate(value);
//            #endif
//            break;
//        case CONST_INFRA_BAUDRATE:
//            fwrite_array(FILEID_RUN_PARAM, FLADDR_INFRA_BAUDRATE, data, 4);   //4+1,红外口波特率
//            break;
//        case CONST_MNG485_BAUDRATE:
//            fwrite_array(FILEID_RUN_PARAM, FLADDR_RS4852_BAUDRATE, data, 4);   //4+1,485II波特率
//            break;
//        case CONST_GPRS_LOCK:
//            fwrite_array(FILEID_RUN_PARAM, FLADDR_GPRS_LOCK, data, 1);   //1+1,锁频状态
//            if(0xAA == data[0])
//                set_system_flag(SYS_LOCK_CHANNEL,SYS_FLAG_BASE);
//            else
//                clr_system_flag(SYS_LOCK_CHANNEL,SYS_FLAG_BASE);
//            break;
//        case CONST_PROVINCE_FEATURE:                                          //8,省份特征，ASCII码
//            fwrite_array(FILEID_RUN_PARAM, FLADDR_PROVINCE_FEATURE,data,8);
//            mem_cpy(gSystemInfo.province_feature,data,8);
//            break;
//        case CONST_REMOTE_TYPE:
//            fwrite_array(FILEID_RUN_PARAM, FLADDR_REMOTE_TYPE,data,1);
//            gSystemInfo.remote_type = data[0];
//            break;
        case CONST_MES:
            fwrite_ertu_params(EEADDR_MES,data,32);
            //record_log_code(LOG_SYS_SET_MES,NULL,0,LOG_ALL);//设置时记录一次日志，用于以后分析
            break;
//        case CONST_ERTU_PLMS_SWITCH:
//	        //对于该开关的，存储时，认为0有效，设置和读取时均做取反操作
//	        bit_value_opt_inversion(data,ERTU_PLMS_SWITCH_LEN);
//	        error = fwrite_ertu_params(EEADDR_ERTU_PLMS_SWITCH,data,ERTU_PLMS_SWITCH_LEN);   //1+4 终端参数控制字
//	        break;
		case CONST_DZC_METER_SWTICH:   //34 三相G表检测U盘 写USB.dat文件
			tmp = app_write_usb_data(NULL,0,data,10);
            break;		

        case CONST_CONFIG_ONENET:
            //设置ONENET上线参数
            fwrite_ertu_params(EEADDR_CONFIG_ONENET,data,1);
            gSystemInfo.tcp_link = 0;//切换网络就重新拨号
            break;
		case CONST_SWITCH_PARAM:
			fwrite_ertu_params(EEADDR_SWITCH_PARAM,data,4);
			break;
			
			
        default:
            error = TRUE;
            break;

    }

    return error;

}




/*+++
  功能: 读取扩展的终端参数F796
  参数：
        BOOLEAN flag  读取时是否需带检验位，TRUE 带校验
  返回:

---*/
INT16U read_custom_param(INT16U paramcode,INT8U *data)
{
    INT16U size;
    BOOLEAN error;

    error= FALSE;
    size=0;

    switch(paramcode)
    {
        case CONST_CFG_DEVID:				//1UL			//6	终端地址
            error = fread_ertu_params(EEADDR_CFG_DEVID,data,CFG_DEVID_LEN);
            size=CFG_DEVID_LEN+1;
            break;
        case CONST_CFG_VENDOR:				//2UL			//4	厂商代码
            error = fread_ertu_params(EEADDR_CFG_VENDOR,data,4);
            if(error || check_is_all_FF(data,4))
            {
                mem_cpy(data,(INT8U *)ertu_cfg_verinfo,4);
                error = FALSE;
            }
            size=5;
            break;
//        case CONST_VENDOR_CHIP:				//3UL			//4	终端厂家（山西）
//            error = fread_ertu_params(EEADDR_VENDOR_CHIP,data,4);
//            size=5;
//            break;
//        case CONST_VENDOR_CHIP2:			//4UL			//2芯片厂家（山西）
//            error = fread_ertu_params(EEADDR_VENDOR_CHIP+4,data,2);
//            size=3;
//            break;
//        case CONST_SHANGHAI_VENDOR_FLAG:	//5UL			//2	版本标识（上海）
//            error = fread_ertu_params(EEADDR_SHANGHAI_VENDOR_FLAG,data,2);
//            size=3;
//            break;
//        case CONST_SOFT_VER:				//6UL			//4+1	软件版本号
//            error = fread_ertu_params(EEADDR_SOFT_VER,data,4);
//            if(error || check_is_all_FF(data,4))
//            {
//                mem_cpy(data,(INT8U *)ertu_cfg_verinfo+12,4);
//                error = FALSE;
//            }
//            size=5;
//            break;
//        case CONST_SOFT_DATE:				//7UL			//3+1	软件版本发布日期
//            error = fread_ertu_params(EEADDR_SOFT_DATE,data,3);
//            if(error || check_is_all_FF(data,3))
//            {
//                mem_cpy(data,(INT8U *)ertu_cfg_verinfo+16,3);
//                error = FALSE;
//            }
//            size=4;
//            break;
//        case CONST_CAPACITY:				//8UL			//11+1	容量信息码
//            error = fread_ertu_params(EEADDR_CAPACITY,data,11);
//            if(error || check_is_all_FF(data,11))
//            {
//                mem_cpy(data,(INT8U *)ertu_cfg_verinfo+19,11);
//                if(NandFlashCfgInfo.block_count==2048)
//                {
//                    data[7]='2';
//                    data[8]='5';
//                    data[9]='6';
//                }
//                error = FALSE;
//            }
//            size=12;
//            break;
//        case CONST_PROTOCOL_VER:			//9UL			//4+1	协议版本号
//            error = fread_ertu_params(EEADDR_PROTOCOL_VER,data,4);
//            if(error || check_is_all_FF(data,4))
//            {
//                mem_cpy(data,(INT8U *)ertu_cfg_verinfo+30,4);
//                error = FALSE;
//            }
//            size=5;
//            break;
//        case CONST_HARDWARE_VER:			//10UL		//4+1	硬件版本号
//            error = fread_ertu_params(EEADDR_HARDWARE_VER,data,4);
//            if(error || check_is_all_FF(data,4))
//            {
//                mem_cpy(data,(INT8U *)ertu_cfg_verinfo+34,4);
//                if(mem_is_same(data,"3.01",4))
//                {
//                    //版本信息是3.01的话，说明是通用版本，这里根据实际硬件版本做修正
//                    if(gSystemInfo.hardware_version == 0x02)   
//                    {
//                        mem_cpy(data,"3.02",4);
//                    }
//                }
//                error = FALSE;
//            }
//            size=5;
//            break;
//        case CONST_HARDWARE_DATE:			//11UL		//3+1	硬件版本发布日期
//            error = fread_ertu_params(EEADDR_HARDWARE_DATE,data,3);
//            if(error || check_is_all_FF(data,3))
//            {
//                mem_cpy(data,(INT8U *)ertu_cfg_verinfo+38,3);
//                if(mem_is_same(data,"\x02\x01\x15",3))
//                {
//                    //版本信息是3.01的话，说明是通用版本，这里根据实际硬件版本做修正
//                    if(gSystemInfo.hardware_version == 0x02)
//                    {
//                        mem_cpy(data,"\x16\x02\x17",3);
//                    }
//                }
//                error = FALSE;
//            }
//            size=4;
//            break;
//        case CONST_ERTU_SWITCH:
//            error = fread_ertu_params(EEADDR_ERTU_SWITCH,data,ERTU_SWITCH_LEN);     //4+1   终端的编译开关，D0-日冻结时标抄读开关
//            //对于该开关的，存储时，认为0有效，设置和读取时均做取反操作，
//            if(error)
//            {
//                mem_set(data,ERTU_SWITCH_LEN,0xFF);
//                error = FALSE;
//            }
//            bit_value_opt_inversion(data,ERTU_SWITCH_LEN);
//            size=ERTU_SWITCH_LEN + 1;
//            break;
//        case CONST_DJB_VER:                                                //16+1  蓝牙的版本信息
//            mem_cpy(data,gSystemInfo.djb_ver_info,16);
//            error = FALSE;
//            size = 17;
//            break;
//        case CONST_LOG_PARAM:                                           //2+1  日志参数:日志级别+日志输出方式
//            fread_array(FILEID_RUN_PARAM,FLADDR_LOGEVEL,data,2);
//            error = FALSE;
//            size = 2+1;
//            break;
//        case CONST_DISPLAY_PWD:                                         //6+1  液晶显示密码
//            fread_array(FILEID_RUN_PARAM,FLADDR_LOCAL_PASSWORD,data,6);
//            error = FALSE;
//            size = 6+1;
//            break;
//        case CONST_RS232_BAUDRATE:                                      //4+1  串口波特率
//            fread_array(FILEID_RUN_PARAM,FLADDR_RS232_BAUDRATE,data,4);
//            error = FALSE;
//            size = 4+1;
//            break;
        case CONST_FRAME_MONITOR:
            fread_ertu_params(EEADDR_FRAME_MONITOR, data, 2);      //1+2，报文监控
            bit_value_opt_inversion(data,2);
            error = FALSE;
            size = 2+1;
            break;
//        case CONST_RS232_REPORT:
//            fread_array(FILEID_RUN_PARAM, FLADDR_RS232_REPORT, data, 1);       //1+1，串口上报
//            error = FALSE;
//            size = 1+1;
//            break;
//        case CONST_GPRS_BAUDRATE:
//            fread_array(FILEID_RUN_PARAM, FLADDR_GPRS_BAUDRATE, data, 4);   //4+1,GPRS串口波特率
//            error = FALSE;
//            size = 4+1;
//            break;
//        case CONST_INFRA_BAUDRATE:
//            fread_array(FILEID_RUN_PARAM, FLADDR_INFRA_BAUDRATE, data, 4);   //4+1,红外口波特率
//            error = FALSE;
//            size = 4+1;
//            break;
//        case CONST_MNG485_BAUDRATE:   //对应485II波特率
//            fread_array(FILEID_RUN_PARAM, FLADDR_RS4852_BAUDRATE, data, 4);   //4+1,485II波特率
//            error = FALSE;
//            size = 4+1;
//            break;
//        case CONST_GPRS_LOCK:
//            fread_array(FILEID_RUN_PARAM, FLADDR_GPRS_LOCK, data, 1);   //1+1,锁频状态
//            error = FALSE;
//            size = 1+1;
//            break;
//        case CONST_PROVINCE_FEATURE:                                 //8,省份特征
//            fread_array(FILEID_RUN_PARAM, FLADDR_PROVINCE_FEATURE, data, 8);
//            error = FALSE;
//            size = 8+1;
//            break;
//        case CONST_REMOTE_TYPE:                           //1,远程通道类型
//            fread_array(FILEID_RUN_PARAM, FLADDR_REMOTE_TYPE, data, 1);
//            error = FALSE;
//            size = 1+1;
//            break;
        case CONST_MES:
            fread_ertu_params(EEADDR_MES, data, 32);
            error = FALSE;
            size = 32+1;
            break;
//        case CONST_ERTU_PLMS_SWITCH:
//            error = fread_ertu_params(EEADDR_ERTU_PLMS_SWITCH,data,ERTU_PLMS_SWITCH_LEN);     //4+1   终端参数控制字，D0-GPRS参数是否被修改
//            //对于该开关的，存储时，认为0有效，设置和读取时均做取反操作，
//            if(error)
//            {
//                mem_set(data,ERTU_PLMS_SWITCH_LEN,0xFF);
//                error = FALSE;
//            }
//            bit_value_opt_inversion(data,ERTU_PLMS_SWITCH_LEN);
//            size=ERTU_PLMS_SWITCH_LEN + 1;
//            break;
//        case CONST_GPRS_AUTH_TYPE:
//            fread_array(FILEID_RUN_PARAM, FLADDR_GPRS_AUTH_TYPE, data, 1);   //1+1,鉴权类型，55-pap，AA-chap，其他默认
//            error = FALSE;
//            size = 1+1;
//            break;

        case CONST_DZC_METER_SWTICH:	//34UL	三相G表检测U盘 写U盘USB.dat文件数据 读出数据验证 
			size = app_read_usb_data(NULL,0,data,10);
			if(size)
			{
				size = 10+1;
			}
			else
			{
				size=0;
			}
            break;		
        case CONST_CONFIG_ONENET:
            //设置ONENET上线参数
            fread_ertu_params(EEADDR_CONFIG_ONENET,data,1);
            size = 1+1;
            break;
		case CONST_SWITCH_PARAM:
			fread_ertu_params(EEADDR_SWITCH_PARAM,data,4);
			size = 4+1;
			break;			
			
			
        default:
            size = 1;
            error = TRUE;
            break;
    }
//    if(flag)
//    {
//        crc = crc16_param(data,(size-1));
//        if(error)
//        {
//            mem_set(data,size,0xFF);
//            crc=0xAA;
//            size = 0;
//        }
//        else
//        {
//            mem_cpy_right(data+1,data,(size-1));
//        }
//        data[0]=crc;
//    }
//    else
//    {
//        size -= 1;
//        if(error)
//        {
//            mem_set(data,size,0xFF);
//            size = 0;
//        }
//    }
    return size;

}



void query_ertu_custom_param(INT8U *buffer,INT16U *itemDataLen,INT8U *param)
{
    INT16U code;
    INT16U size;
    INT8U *data;
    INT8U count;
    INT8U idx;
    //BOOLEAN check_crc;
    QUERY_CTRL *query_ctrl;
    
    query_ctrl = (QUERY_CTRL *)param;
    
    count=*(query_ctrl->req);
    query_ctrl->req++;
    query_ctrl->req_bytes += count * 2 + 1;

    buffer[0]=count;
    buffer++;

    *itemDataLen=0;
    for(idx=0;idx<count;idx++)
    {
        code=bin2_int16u(query_ctrl->req);
        mem_cpy(buffer,query_ctrl->req,2);
        query_ctrl->req+=2;
        buffer+=2;

        data=buffer+2;
        //check_crc = TRUE;
        size=read_custom_param(code,data+1);
        //如果终端不支持该参数，则返回的长度为0，给主站返回长度为1,仅包含校验码
        if(size==0)
        {
            size =1;
        }
        buffer[0]=size-1;
        buffer[1]=(size-1)>>8;
        buffer+=2;
        buffer+=size;
        *itemDataLen+=(size+4);
    }
    if(*itemDataLen>0)
    {
        (*itemDataLen)++;
    }

}

INT16U app_get_F249_data(INT8U* resp)
{
	INT8U pos=0,idx=0;
	
	mem_set(resp,60,0x00);
	fread_ertu_params(EEADDR_CFG_DEVID,resp+pos,4);
	pos += 4;
	resp[pos++] = 6;  //6专变终端3型(FKXA2X)；7专变终端3型(FKXA4X)；8专变终端3型(FCXA2X)
	resp[pos++] = 2;  //d1 d0 数值1~3依次表示三相三线、三相四线、单相二线，0为备用
	resp[pos++] = 0;
	resp[pos++] = 0x22;
	resp[pos++] = 1;//1 GPRS，2 CDMA，3以太网，4, 230 M,5 EPON, 其他保留；
	mem_cpy(resp+pos,gSystemInfo.myccid_info,20);
	for(idx=0;idx<20;idx++)
	{
		if((resp[pos+idx]>='0')&&(resp[pos+idx]<='9')
		|| (resp[pos+idx]>='A')&&(resp[pos+idx]<='Z')
		|| (resp[pos+idx]>='a')&&(resp[pos+idx]<='z'))
		{
		}
		else
		resp[pos+idx] = '0';
	}
	pos += 20;
	resp[pos++] = gSystemInfo.csq;
	resp[pos++] = 4;
	mem_cpy(resp+pos,"XXXXXXXXXXXX",12);
	pos += 12;
	resp[pos++] = 0;
	resp[pos++] = 0;
	resp[pos++] = 0;
	resp[pos++] = 0;
	resp[pos++] = 0;
	resp[pos++] = 0;
	resp[pos++] = 0;
	resp[pos++] = 1;
	return pos;

}

