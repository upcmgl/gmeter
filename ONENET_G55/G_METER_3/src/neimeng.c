#if defined(__PROVICE_NEIMENG__)
#include "main_include.h"
#include "neimeng.h"
#include "TaskPlmsdtp_nm.h"

/*
内蒙古协议处理程序
*/

/*+++
 功能：设置F85：配置电表有效性和重点用户标志
 参数:
        INT8U *itemData,           [输入]数据
        INT16U *itemDataLen        [输出]数据项目长度
  返回:
        INT8U 设置结果
  F85定义：
      本次配置标志	BIN	1
      本次配置内容	BIN	256	本次配置
  描述：
       1)需要与F30、F35同步
---*/

INT8U set_ertu_F85_nm(INT8U *itemData,INT16U *itemDataLen)
{
    INT16U idx,pos;
    INT8U flag,is_use,data[41];

    flag=itemData[0];
    data[0]=0;
    pos=0;
    for(idx=0;idx<MAX_METER_COUNT;idx++)
    {
      if(0==flag)//电表有效性
      {
            //F30
            is_use=get_bit_value(&itemData[idx/8+1],1,idx%8);  // 1-表示有效 0-表示无效
            is_use=is_use?0:1;
            set_spot_params((idx+1),PIM_REC_FLAG,&is_use,1);//0-表示有效 1-表示无效

      }else//重点户标志
      {
           is_use=get_bit_value(&itemData[idx/8+1],1,idx%8);
           if(is_use)
           {
               data[pos++]=meter_idx2_seq(idx+1);
               data[pos++]=(meter_idx2_seq(idx+1)>>8);
               data[0]++;
               if(data[0]>=20)break;//最大支持20个重点用户
           }
      }
    }
    if(1==flag)
    {
	    fwrite_ertu_params(EEADDR_SET_F35,data,41);
    }
    return OK;  //返回成功

}
/*+++
  功能：查询F85参数
  参数：
        INT8U  *data  电表有效性或重点户标志[输出]
        QUERY_CTRL *query_ctrl  用于查询控制的变量，
  返回：
        INT16U  参数长度
  描述：
        1）根据查询参数，返回电能表有效标志或重点用户标志
---*/
INT16U query_ertu_F85_nm(INT8U *data,QUERY_CTRL *query_ctrl)
{
    INT16U idx,meter_idx;
    INT8U is_use,temp[41],count;
    data[0]=query_ctrl->req[0];
    mem_set(data+1,256,0x00);
    //电表有效性
    if(0==query_ctrl->req[0])
    {
       for(idx=0;idx<MAX_METER_COUNT;idx++)
       {
            get_spot_params((idx+1),PIM_REC_FLAG,&is_use,1);//0-表示有效 1-表示无效
            if(is_use == 0)
            {
            	set_bit_value(&data[idx/8+1],1,idx%8);  // 1-表示有效 0-表示无效
            }
       }

    }
    else //重点户标志
    {
	    fread_ertu_params(EEADDR_SET_F35,temp,41);
        count=temp[0];
        if(count>20)count=20;
          for(idx=0;idx<count;idx++)
          {
               byte2int16(temp+idx*2+1,&meter_idx);
               set_bit_value(&data[(meter_idx-1)/8+1],1,(meter_idx-1)%8);  // 1-表示有效 0-表示无效
          }

    
    }
    query_ctrl->req_bytes++;
    return 256;
}
/*
根据载波抄表起始时间和抄表间隔，计算抄表时段

*/
void calc_seg_time()
{
    JCSET_F33  SET_F33;
    INT16U  offset;
    INT8U idx;
    INT8U start_time[2];
    offset=sizeof(JCSET_F33)*3;

    //读取台区集中抄表运行参数设置:载波通道
    fread_ertu_params(EEADDR_SET_F33+offset,(INT8U *)&SET_F33,sizeof(JCSET_F33));

    start_time[0]=SET_F33.seg.rec_timeseg[0][0];
    start_time[1]=SET_F33.seg.rec_timeseg[0][1];
    for(idx=0;idx<24;idx++)
    {
        if(start_time[0]+SET_F33.cycle*(idx+1)>24)break;
        SET_F33.seg.rec_timeseg[0][0]=start_time[0]+SET_F33.cycle*idx;
        SET_F33.seg.rec_timeseg[0][1]=start_time[1];

        SET_F33.seg.rec_timeseg[1][0]=start_time[0]+SET_F33.cycle*(idx+1);
        if(start_time[1]==0)
            SET_F33.seg.rec_timeseg[1][1]=59;
        else
            SET_F33.seg.rec_timeseg[1][1]=start_time[1]-1;
    }




}


/*
集中器管理表数量
说明：统计载波表大小类电表数量的方法，每次在返回数量列表中查询该大小类，如果电表数量为FF则说明该类无效，
*/
INT16U read_c1_f10(INT8U *data,READ_HIS *readC1)
{
    INT16U spot_idx;
    INT16U meter_seq;
    INT16U meter_idx;
    INT16U meter_count;
    INT16U plcmeter_count;
    INT16U idx,temp_count;
    INT8U meter485_count,class_count;
    METER_DOCUMENT meter_doc;
    INT8U PLC_METER_COUNT_POS=6;

    meter_count=0;
    meter485_count=0;
    plcmeter_count=0;
    class_count=0;
    for(meter_seq=1;meter_seq<MAX_METER_COUNT;meter_seq++)
    {

        if(FALSE == file_exist(meter_seq))     continue;

        //读取配置信息
        fread_array(meter_seq,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
        //验证电表有效性
        meter_idx = bin2_int16u(meter_doc.meter_idx);
        spot_idx  = bin2_int16u(meter_doc.spot_idx);

        if(spot_idx == 0) continue;
        if(meter_seq != meter_idx) continue;
         meter_count++;
        if((meter_doc.baud_port.port>=2) && (meter_doc.baud_port.port<=5))
        {
         	meter485_count++;
        }
        if(meter_doc.baud_port.port==31)
        {
           plcmeter_count++;
           for(idx=0;idx<0xFF;idx++)
           {
              if((data[PLC_METER_COUNT_POS+idx*3+1]==0xFF) && (data[PLC_METER_COUNT_POS+idx*3+2]==0xFF))break;//没找到
              if(data[PLC_METER_COUNT_POS+idx*3]!=meter_doc.meter_class.value) continue;//不是需要的
              if(idx+1>class_count)
              {
              	class_count=idx+1;
              }
              break;
           }
           data[PLC_METER_COUNT_POS+idx*3+1]=meter_doc.meter_class.value;
           temp_count=bin2_int16u(data+PLC_METER_COUNT_POS+idx*3+1);
           if(temp_count=0xFF)
           {
           		temp_count++;
           }
           temp_count++;
           data[PLC_METER_COUNT_POS+idx*3+1]=temp_count;
           data[PLC_METER_COUNT_POS+idx*3+2]=(temp_count>>8);
        }
        
    }
    data[0]=meter_count;
    data[1]=meter_count>>8;
    data[2]=meter485_count;
    data[3]=plcmeter_count;
    data[4]=plcmeter_count>>8;
    data[5]=class_count;
    return  (class_count*3+6);
}
/*
集中器工作状态
由于485和载波是分开的，因此目前只取载波
*/
INT16U read_c1_f11(INT8U *data,READ_HIS *readC1)
{
   INT16U pos,count1,count2;
   LLVC_REC_STATE  plc_rec_state;
   pos=0;
   read_fmArray(FMADDR_F11+1,&plc_rec_state,sizeof(LLVC_REC_STATE));
   if(plc_rec_state.currec_flag.finish==1)
   {
       byte2int16(plc_rec_state.total_count,&count1);
       byte2int16(plc_rec_state.read_count,&count2);
       if(count1==count2)
           data[pos++]=1;
       else
           data[pos++]=2;
   }else
   {
           data[pos++]=0;
   }
   //抄读数据起始时间
   data[pos++]=plc_rec_state.begin_datetime.year;
   data[pos++]=(plc_rec_state.begin_datetime.week_month & 0x1F);
   data[pos++]=plc_rec_state.begin_datetime.day;
   data[pos++]=plc_rec_state.begin_datetime.hour;
   data[pos++]=plc_rec_state.begin_datetime.minute;
   data[pos++]=plc_rec_state.begin_datetime.second;

   //抄读数据结束时间
   data[pos++]=plc_rec_state.end_datetime.year;
   data[pos++]=(plc_rec_state.end_datetime.week_month & 0x1F);
   data[pos++]=plc_rec_state.end_datetime.day;
   data[pos++]=plc_rec_state.end_datetime.hour;
   data[pos++]=plc_rec_state.end_datetime.minute;
   data[pos++]=plc_rec_state.end_datetime.second;

   return pos;
}



/*+++
  功能：负控2004，设置参数F9
  参数：
        INT8U *itemData  F9参数
  返回：
        无
  描述：
        1）仅适用于485表和交采表，即前64个测量点
        2）设置时删除多余测量点的
---*/

void set_ertu_f9(INT8U *itemData)
{
   INT16U itemLen;
   DISSET_SPOT_PARAMS meter_doc;
   INT8U old_meter_count,new_meter_count,idx,meter_idx;

   fread_array(FILEID_QGDW_NM,EEADDR_SET_F9_NM,&old_meter_count,1);

   if(old_meter_count>64)old_meter_count=64;

   fwrite_array(FILEID_QGDW_NM,EEADDR_SET_F9_NM,itemData,4);

   new_meter_count=itemData[0];//电能表/交流采样装置配置总块数

   while(new_meter_count<old_meter_count)
   {
         meter_idx=new_meter_count+1;
         //删除测量点
         meter_doc.set_count=1;
         meter_doc.meter_seq=meter_idx;
         meter_doc.spot_idx=0;
           //如果以太网空闲，则使用以太网缓冲区，否则使用串口缓冲区
          if(1 != g_eth_conn_flag)
          {

           //请求以太网应答报文处理数据结构使用权
           OSMutexPend(&SIGNAL_RESP_ETH);
           set_ertu_meters(resp_eth.buffer,(INT8U*)&meter_doc,&itemLen);
           OSMutexFree(&SIGNAL_RESP_ETH);
          }
          else
          {
           OSMutexPend(&SIGNAL_RESP_LOCAL);
           set_ertu_meters(resp_local.buffer,(INT8U*)&meter_doc,&itemLen);
           OSMutexFree(&SIGNAL_RESP_LOCAL);
          }
          new_meter_count++;
   }

}

/*+++
  功能：设置终端电能表/交流采样装置配置参数[F10]
  参数:
        INT8U *buffer,        [缓冲区]，大小足够500字节
        INT8U *params
        INT16U *itemDataLen    处理掉的参数长度
  返回:
        设置结果

  描述:
  		1.由于内蒙古F10参数和376.1不一样，为了保持与国网的一致性，使用规约转换的方法，将内蒙古规约转换376.1
      2.由于内蒙古F10的测量点最大为64个，因此前64个测量点为RS485测量点
      3.由于内蒙古没有删除测量点功能，要删除只能设置测量点个数，因此改变个数时要删除多余的测量点
---*/
INT8U set_ertu_meters_nm(INT8U *buffer,INT8U *params,INT16U *itemDataLen)
{
    INT32U fmAddr;
    INT16U idx,pos;
    INT16U count;          //本次配置数量
    INT16U meter_idx;      //配置序号1~63
    INT16U spot_idx;      //测量点号1~63， 0：删除
    INT16U itemLen;
    DISSET_SPOT_PARAMS meter_doc;
    INT8U temp;

    pos=0;
    //本次电能表/交流采样装置配置数量
    count=params[pos++];

    *itemDataLen = count * 17 + 1;

    mem_set(buffer,PIM_CUR_ZXYG,0x00);

    //依次处理每个配置
    for(idx=0; idx<count; idx++)
    {
        //序号
        meter_idx=params[pos++];

        //判断序号是否合理
        if( (meter_idx == 0 ) || (meter_idx > 64) )
        {
            return ERR;   //返回错误
        }
        //测量点号
        spot_idx=params[pos++];

        //判断测量点号是否合理
        if( spot_idx > 64 )
        {
            return ERR;  //返回错误
        }

        //电表档案
        meter_doc.set_count=1;
        meter_doc.meter_seq=meter_idx;

        meter_doc.spot_idx=spot_idx;
        meter_doc.baud_port.value=params[pos++];
        meter_doc.protocol =params[pos++];
        mem_cpy(meter_doc.meter_no,params+pos,6);pos+=6;
        mem_cpy(meter_doc.meter_pass,params+pos,6);pos+=6;
        temp=params[pos++];
        meter_doc.meter_fl_count=(temp >> 4) & 0x0F;
        meter_doc.meter_frac_info=temp;
        mem_set(meter_doc.rtu_no,6,0xFF);
        meter_doc.meter_class.value = 0;
        //如果以太网空闲，则使用以太网缓冲区，否则使用串口缓冲区
       if(1 != g_eth_conn_flag)
       {

        //请求以太网应答报文处理数据结构使用权
        OSMutexPend(&SIGNAL_RESP_ETH);
        set_ertu_meters(resp_eth.buffer,(INT8U*)&meter_doc,&itemLen);
        OSMutexFree(&SIGNAL_RESP_ETH);
       }
       else
       {
        OSMutexPend(&SIGNAL_RESP_LOCAL);
        set_ertu_meters(resp_local.buffer,(INT8U*)&meter_doc,&itemLen);
        OSMutexFree(&SIGNAL_RESP_LOCAL);
       }
    }

    return OK;  //返回成功
}
/*+++
  功能：设置终端电能表/交流采样装置配置参数[F96]
  参数:
        INT8U *buffer,        [缓冲区]，大小足够500字节
        INT8U *params
        INT16U *itemDataLen    处理掉的参数长度
  返回:
        设置结果

  描述:
  		1.测量点号重65开始，前64个给F10使用
---*/
INT8U set_ertu_meters_F96(INT8U *buffer,INT8U *params,INT16U *itemDataLen)
{
    INT32U fmAddr;
    INT16U idx,pos;
    INT16U count;          //本次配置数量
    INT16U meter_idx;      //配置序号1~63
    INT16U spot_idx;      //测量点号1~63， 0：删除
    INT16U itemLen;
    DISSET_SPOT_PARAMS meter_doc;
    INT8U temp;

    pos=0;
    //本次电能表/交流采样装置配置数量
    count=bin2_int16u(params+pos);
    pos+=2;

    *itemDataLen = count * 27 + 1;

    //依次处理每个配置
    for(idx=0; idx<count; idx++)
    {
        //序号
       meter_idx=bin2_int16u(params+pos);
       pos+=2;

        //判断序号是否合理
        if( (meter_idx == 0 ) || (meter_idx > 64) )
        {
            return ERR;   //返回错误
        }
        //测量点号
       spot_idx=bin2_int16u(params+pos);
       pos+=2;

        //判断测量点号是否合理
        if( spot_idx > 64 )
        {
            return ERR;  //返回错误
        }

        //电表档案
        meter_doc.set_count=1;
        meter_doc.meter_seq=meter_idx+65;

        meter_doc.spot_idx=spot_idx+65;
        meter_doc.baud_port.value=params[pos++];
        meter_doc.protocol =params[pos++];
        mem_cpy(meter_doc.meter_no,params+pos,6);pos+=6;
        mem_cpy(meter_doc.meter_pass,params+pos,6);pos+=6;
        meter_doc.meter_fl_count=params[pos++];
        meter_doc.meter_frac_info=params[pos++];
        mem_cpy(meter_doc.rtu_no,params+pos,6);pos+=6;
        meter_doc.meter_class.value = params[pos++];
        //如果以太网空闲，则使用以太网缓冲区，否则使用串口缓冲区
       if(1 != g_eth_conn_flag)
       {

        //请求以太网应答报文处理数据结构使用权
        OSMutexPend(&SIGNAL_RESP_ETH);
        set_ertu_meters(resp_eth.buffer,(INT8U*)&meter_doc,&itemLen);
        OSMutexFree(&SIGNAL_RESP_ETH);
       }
       else
       {
        OSMutexPend(&SIGNAL_RESP_LOCAL);
        set_ertu_meters(resp_local.buffer,(INT8U*)&meter_doc,&itemLen);
        OSMutexFree(&SIGNAL_RESP_LOCAL);
       }
    }

    return OK;  //返回成功
}

/*+++
  功能：负控2004，查询电表配置信息,没有单独存储，根据376.1进行转换
  参数：
        INT8U *data,
        INT16U max_len      可用最大长度
  返回：
        INT16U  返回数据长度
---*/
BOOLEAN query_meter_info_nm(INT8U *data,INT16U *itemDataLen,INT16U max_len)
{
   INT16U pos,idx,meter_idx;
   INT8U  meter_count;
   union{
      INT8U  str[4];
      struct{
         INT8U   meter_count;
         INT8U   pulse_count;
         INT8U   analog_count;
         INT8U   agp_count;
      }d;
   }var;
    METER_DOCUMENT meter_doc;
   *itemDataLen=0;
   //读取配置数量
   fread_array(FILEID_QGDW_NM,EEADDR_SET_F9_NM, var.str,4);

   meter_count = var.d.meter_count;

   if(meter_count > MAX_485METER_COUNT) meter_count  = 0;
   if(meter_count*17+1>max_len)return TRUE;//无法容纳

   data[0] = 0;
   pos = 1;
   for(idx=0;idx<meter_count;idx++)
   {
        if(FALSE == file_exist(idx+1))     continue;

        //读取配置信息
        fread_array(idx+1,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
        byte2int16(meter_doc.meter_idx,&meter_idx);
        if(meter_idx==idx+1)
        {
            data[0] ++;
            data[pos++]=meter_doc.meter_idx[0];
            data[pos++]=meter_doc.spot_idx[0];
            data[pos++]=meter_doc.baud_port.value;
            data[pos++]=meter_doc.protocol;
            mem_cpy(data+pos,meter_doc.meter_no,6);pos+=6;
            mem_cpy(data+pos,meter_doc.password,6);pos+=6;
            data[pos++]=(meter_doc.fl_count<<4) | (meter_doc.mbit_info.value & 0x0F);
            
            //if(pos + 17 > max_len) break;
        }
   }
   *itemDataLen=pos;
   return FALSE;
}
/*+++
  功能：查询参数F10
  参数：
        INT8U *Reply,                [输出] 应答缓冲区
        INT16U *itemDataLen,         [输出] 本次应答报文长度
        QUERY_CTRL *query_ctrl       [输入] 请求参数
  返回：
        是否还有数据需要应答 TRUE/FALSE
  描述：
         1）查询参数
         本次查询数量n	BIN	2
         本次查询的第1个对象序号	BIN	2
          … …	……	……
         本次查询的第n个对象序号	BIN	2
       2)   修改itemDataLen为处理的字节长度
       3) query_ctrl.from_seq 为处理请求的起始位置
---*/
BOOLEAN query_ertu_meters_F96(INT8U *Reply,INT16U *itemDataLen,QUERY_CTRL *query_ctrl)
{
    INT16U idx;
    INT16U pos;
    INT16U req_seq;
    INT16U meter_idx;
    INT16U meter_count;
    INT16U resp_count;
    INT8U meter_doc_len;
    METER_DOCUMENT meter_doc;
    BOOLEAN flag;

    pos = 0;
    resp_count=0;
    flag = FALSE;
    
    //本次电能表/交流采样装置配置数量
    byte2int16(query_ctrl->req,  &meter_count);
    query_ctrl->req+=2;
    Reply[pos++] = 0;
    Reply[pos++] = 0;

    for(idx=0;idx<meter_count;idx++)
    {
        if(idx<query_ctrl->from_seq)
        {
           query_ctrl->req +=2;
           continue;
        }

        //请求的序号
        byte2int16(query_ctrl->req,  &req_seq);
        query_ctrl->req += 2;
        req_seq+=65;//低压表重65开始

        if(FALSE == file_exist(req_seq))     continue;

        //读取配置信息
        meter_doc_len = sizeof(METER_DOCUMENT);
        fread_array(req_seq,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));


        //检查是否已经配置过了,配置序号相符即可认为已经配置过了！
        byte2int16(meter_doc.meter_idx,  &meter_idx);
        if(meter_idx == req_seq)
        {
            //检查应答缓冲区是否可以容纳下响应数据
            if( (pos+meter_doc_len) > query_ctrl->left_resp_len)
            {
               flag = TRUE;
               break;
            }
            //更改meter_doc.序号和测量点号
            meter_doc.meter_idx[0]-=65;
            meter_doc.spot_idx[0]-=65;
            //可以容纳下
            mem_cpy(Reply+pos,(INT8U *)&meter_doc,meter_doc_len);
            pos += meter_doc_len;
            resp_count ++;
            query_ctrl->from_seq = idx+1;
        }
    }

    //重新填写本次配置数量
    Reply[0] = resp_count;
    Reply[1] = resp_count>>8;

    //本次响应实际长度
    *itemDataLen = pos;

    //请求参数长度
    query_ctrl->req_bytes = (meter_count<<1) + 2;

    //循环结束只有两种可能：1）循环结束，2）超过响应空间
    //这两种结果由flag进行表征。
    return flag;
}
/*+++
 功能：设置主站IP地址和端口，需要做一个转换。
 参数:
        INT8U *itemData,           [输入]数据
        INT16U *itemDataLen        [输出]数据项目长度
  返回:
        INT8U 设置结果
  描述：
       2004规约中 ，有网管IP地址和端口，代理ＩＰ地址和端口。
       处理时把这个内容直接去掉，然后保存到集中器参数文件中。

---*/
INT8U set_ertu_msa_info_nm(INT8U *itemData,INT16U *itemDataLen)
{
    tagSET_F3_nm *setF3;

    setF3  = (tagSET_F3_nm *)itemData;


    //先保存网关地址端口及代理地址及端口
    fwrite_array(FILEID_QGDW_NM,EEADDR_SET_F3_NM_EXT,setF3->d.gate_ipport,12);
   
    //修改参数结构。
    mem_cpy(setF3->n.apn,setF3->d.apn,16);

    //保存
    fwrite_ertu_params(EEADDR_SET_F3,itemData,sizeof(tagSET_F3));

    *itemDataLen = sizeof(tagSET_F3_nm);

    return  OK;
}


/*+++
  功能：负控2004，查询主站参数
  参数：
        INT8U  *data  主站参数[输出]
  返回：
        INT16U  参数长度
---*/
INT16U query_nm_msa_info(INT8U *data)
{
   tagSET_F3 F3;
   tagSET_F3_nm *F3_nm;

   F3_nm = (tagSET_F3_nm *)data;
   fread_ertu_params(EEADDR_SET_F3,F3.value,sizeof(tagSET_F3));

   //主用IP，备用IP
   mem_set(F3_nm->value,sizeof(tagSET_F3_nm),0x00);
   mem_cpy(F3_nm->value,F3.value,12);

   //APN
   mem_cpy(F3_nm->d.apn,F3.APN,16);

   //读取网关地址端口及代理地址及端口
   fread_array(FILEID_QGDW_NM,EEADDR_SET_F3_NM_EXT,F3_nm->d.gate_ipport,12);
   
   return sizeof(tagSET_F3_nm);
}


/*+++
 功能：设置抄表日参数，需要做一个转换。
 参数:
        INT8U *itemData,           [输入]数据
        INT16U *itemDataLen        [输出]数据项目长度
  返回:
        INT8U 设置结果
  描述：
       1)把参数内容保存到抄表端口的F33中(QGDW376.1)。

---*/
INT8U set_ertu_recday_info_nm(INT8U *itemData,INT16U *itemDataLen)
{
    JCSET_F33  SET_F33;
    INT16U  offset;

    offset=sizeof(JCSET_F33);
    //读取台区集中抄表运行参数设置:抄表RS485通道
    fread_ertu_params(EEADDR_SET_F33+offset,(INT8U *)&SET_F33,sizeof(JCSET_F33));

    SET_F33.port = COMMPORT_485_REC;
    mem_cpy(SET_F33.rec_days,itemData,6);
    *itemDataLen = 6;
    fwrite_ertu_params(EEADDR_SET_F33+offset,(INT8U *)&SET_F33,sizeof(JCSET_F33));

    return OK;
}

/*+++
  功能：负控2004，查询抄表日参数
  参数：
        INT8U  *data  抄表日[输出]
  返回：
        INT16U  参数长度
  描述：
        1）从QGDW376.1的F33中取出。固定从抄表485口的取出.
---*/
INT16U query_nm_recday_info(INT8U *data)
{
    JCSET_F33  SET_F33;
    INT16U  offset;

    offset=sizeof(JCSET_F33);
    //读取台区集中抄表运行参数设置:抄表RS485通道
    fread_ertu_params(EEADDR_SET_F33+offset,(INT8U *)&SET_F33,sizeof(JCSET_F33));

    mem_cpy(data,SET_F33.rec_days,6);

    return 6;
}



/*+++
  功能：负控2004，查询电表配置信息
  参数：
        INT8U *data,
        INT16U max_len      可用最大长度
  返回：
        INT16U  返回数据长度
---*/
INT16U query_nm_meter_info(INT8U *data,INT16U max_len)
{
   INT16U pos,idx;
   INT8U  meter_count;
   union{
      INT8U  str[4];
      struct{
         INT8U   meter_count;
         INT8U   pulse_count;
         INT8U   analog_count;
         INT8U   agp_count;
      }d;
      struct{
          INT8U   meter_idx;       //配置序号
          INT8U   spot_idx;        //测量点号
          INT8U   meter_info[15];
      }e;
   }var;

   //读取配置数量
   fread_array(FILEID_QGDW_NM,EEADDR_SET_F9_NM, var.str,4);

   meter_count = var.d.meter_count;

   if(meter_count > MAX_485METER_COUNT) meter_count  = 0;

   data[0] = 0;
   pos = 1;
   for(idx=0;idx<meter_count;idx++)
   {
      fread_array(FILEID_QGDW_NM,EEADDR_SET_F10_NM+idx*17,var.str,17);

      if( (var.e.meter_idx+1) == idx)
      {
          data[0] ++;
          mem_cpy(data+pos,var.str,17);
          pos+=17;
          if(pos + 17 > max_len) break;
      }
   }

   return pos;
}


/*+++
 功能：设置抄表间隔
 参数:
        INT8U *itemData,           [输入]数据
        INT16U *itemDataLen        [输出]数据项目长度
        INT8U port_type            [输入]端口类别 0-482/交采  1-载波
  返回:
        INT8U 设置结果
  描述：
       1)把参数内容保存到抄表端口的F33中(QGDW376.1)。

---*/
INT8U set_ertu_recinterval_info_nm(INT8U *itemData,INT16U *itemDataLen,INT8U port_type)
{
    JCSET_F33  SET_F33;
    INT16U  offset;

    offset=sizeof(JCSET_F33);
    if(1==port_type)offset=sizeof(JCSET_F33)*3;

    //读取台区集中抄表运行参数设置
    fread_ertu_params(EEADDR_SET_F33+offset,(INT8U *)&SET_F33,sizeof(JCSET_F33));

    SET_F33.port = COMMPORT_485_REC;
    SET_F33.cycle=itemData[0];
    *itemDataLen = 1;
    fwrite_ertu_params(EEADDR_SET_F33+offset,(INT8U *)&SET_F33,sizeof(JCSET_F33));
    if(1==port_type)calc_seg_time();//如果载波抄表间隔改变了，则需要重新计算抄表时段
    return OK;
}

/*+++
  功能：负控2004，查询抄表间隔
  参数：
        INT8U  *data  抄表间隔[输出]
        INT8U  port_type端口类型[输入]
  返回：
        INT16U  参数长度
  描述：
        1）从QGDW376.1的F33中取出。从指定通道读取.
---*/
INT16U query_nm_recinterval_info(INT8U *data,INT8U port_type)
{
    JCSET_F33  SET_F33;
    INT16U  offset;

    offset=sizeof(JCSET_F33);
    if(1==port_type)offset=sizeof(JCSET_F33)*3;

    //读取台区集中抄表运行参数设置
    fread_ertu_params(EEADDR_SET_F33+offset,(INT8U *)&SET_F33,sizeof(JCSET_F33));

    data[0]=SET_F33.cycle;
    return 1;
}



/*+++
 功能：设置F33抄表时段1开始时间
 参数:
        INT8U *itemData,           [输入]数据
        INT16U *itemDataLen        [输出]数据项目长度
  返回:
        INT8U 设置结果
  描述：
       1)把参数内容保存到抄表端口的F33中(QGDW376.1)。

---*/
INT8U set_ertu_recstarttime_info_nm(INT8U *itemData,INT16U *itemDataLen)
{
    JCSET_F33  SET_F33;
    INT16U  offset;

    offset=sizeof(JCSET_F33)*3;
    //读取台区集中抄表运行参数设置:载波通道
    fread_ertu_params(EEADDR_SET_F33+offset,(INT8U *)&SET_F33,sizeof(JCSET_F33));

    SET_F33.port = COMMPORT_PLC_REC;
    SET_F33.seg.rec_timeseg[0][0]=itemData[0];
    SET_F33.seg.rec_timeseg[0][1]=itemData[1];
    *itemDataLen = 2;
    fwrite_ertu_params(EEADDR_SET_F33+offset,(INT8U *)&SET_F33,sizeof(JCSET_F33));
    calc_seg_time();//如果载波抄表间隔改变了，则需要重新计算抄表时段

    return OK;
}

/*+++
  功能：负控2004，查询载波抄表时段1开始时间
  参数：
        INT8U  *data  抄表间隔[输出]
  返回：
        INT16U  参数长度
  描述：
        1）从QGDW376.1的F33中取出。固定从载波通道口的取出.
---*/
INT16U query_nm_recstarttime_info(INT8U *data)
{
    JCSET_F33  SET_F33;
    INT16U  offset;
    offset=sizeof(JCSET_F33)*3;

    //读取台区集中抄表运行参数设置:抄表载波通道
    fread_ertu_params(EEADDR_SET_F33+offset,(INT8U *)&SET_F33,sizeof(JCSET_F33));

    data[0]=SET_F33.seg.rec_timeseg[0][0];
    data[1]=SET_F33.seg.rec_timeseg[0][1];
    return 2;
}

INT16U query_ertu_f27(INT16U pn_idx,INT8U *itemData)
{
   INT8U len;
   fread_array(FILEID_QGDW_NM,EEADDR_SET_F27_NM+pn_idx*65,itemData,65);
   len=itemData[0]*2+1;
   if(len>65)
   {
      itemData[0]=0;
      len=1;
   }
   return len;
}
INT16U set_ertu_f27(INT16U pn_idx,INT8U *itemData)
{
   INT8U len;
   len=itemData[0]*2+1;
   if(len>0)
   {
      fwrite_array(FILEID_QGDW_NM,EEADDR_SET_F27_NM+pn_idx*65,itemData,len);
   }
   return len;

}
INT16U get_F27F28_C1_nm(INT8U *data,C1F27 *cur_F27,C1F28 *cur_F28)
{
   INT16U pos;
   INT8U meter_status;
   INT8U run_status;

   pos=0;
   meter_status=0;
   run_status=0;
   
   //需量积算方式
   if(get_bit_value((INT8U*)(cur_F28->status+7),1,1))
      set_bit_value(&meter_status,1,1);
   
   //电池电压
   if(get_bit_value((INT8U*)(cur_F28->status+7),1,2))
      set_bit_value(&meter_status,1,2);

   //有功电能方向
   if(get_bit_value((INT8U*)(cur_F28->status+7),1,4))
      set_bit_value(&meter_status,1,4);
   
   //无功电能方向
   if(get_bit_value((INT8U*)(cur_F28->status+7),1,5))
      set_bit_value(&meter_status,1,5);
   
   //A相断电
   if(get_bit_value((INT8U*)(cur_F28->status+10),1,7))
      set_bit_value(&run_status,1,0);
   //B相断电
   if(get_bit_value((INT8U*)(cur_F28->status+11),1,7))
      set_bit_value(&run_status,1,1);
   //C相断电
   if(get_bit_value((INT8U*)(cur_F28->status+12),1,7))
      set_bit_value(&run_status,1,2);

   //A相过压
    if(get_bit_value((INT8U*)(cur_F28->status+10),1,2))
       set_bit_value(&run_status,1,4);
    //B相过压
    if(get_bit_value((INT8U*)(cur_F28->status+11),1,2))
       set_bit_value(&run_status,1,5);
    //C相过压
    if(get_bit_value((INT8U*)(cur_F28->status+12),1,2))
       set_bit_value(&run_status,1,6);

   //终端抄表时间,电能表日历
   mem_cpy((INT8U*)(data+pos),(INT8U*)cur_F27->read_date,11);
   pos+=11;
   //电表运行状态字
   data[pos++]=meter_status;
   //电网状态字
   data[pos++]=run_status;
   //最近一次编程时间
   mem_cpy((INT8U*)(data+pos),(INT8U*)(cur_F27->last_program_date+1),3);
   data[pos+3]=(cur_F27->last_program_date[4] & 0x1F);
   pos+=4;
   //最近一次最大需量清零时间
   mem_cpy((INT8U*)(data+pos),(INT8U*)(cur_F27->last_xlclear_date+1),3);
   data[pos+3]=(cur_F27->last_xlclear_date[4] & 0x1F);
   pos+=4;
  //编程次数 
  mem_cpy((INT8U*)(data+pos),(INT8U*)cur_F27->program_count,2);
  pos+=2;
  //最大需量清零次数
  mem_cpy((INT8U*)(data+pos),(INT8U*)cur_F27->xlclear_count,2);
  pos+=2;
  //电池工作时间
  mem_cpy((INT8U*)(data+pos),(INT8U*)cur_F27->battery_time,3);
  pos+=3;
  
  return pos;


}

/*+++
  功能：设置谐波限值
---*/
INT8U set_ertu_F60_nm(INT8U *params,INT16U *itemDataLen)
{
   INT8U temp[42];

   mem_set(temp,sizeof(temp),0);
   mem_cpy(temp,params,6);

   fwrite_ertu_params(EEADDR_SET_F60,temp,sizeof(temp));

   fwrite_ertu_params(EEADDR_SET_F60+42,params+6,38);
   
   *itemDataLen = 80;
   return 0;

}
/*+++
  功能：查询谐波限值
---*/
INT16U query_ertu_F60_nm(INT8U *data)
{

   fread_ertu_params(EEADDR_SET_F60,data,6);
   fread_ertu_params(EEADDR_SET_F60+42,data+6,38);

   return 80;

}
/*+++
  功能：设置终端电能量费率时段和费率数,这里转换2004规约为376.1规约
---*/
INT8U set_ertu_F21_nm(INT8U *params,INT16U *itemDataLen)
{
   INT8U temp[49];
   INT8U idx;

   for(idx=0;idx<24;idx++)
   {
      temp[idx*2]=(params[idx] & 0x0F);
      temp[idx*2+1]=((params[idx] >>4)& 0x0F);
   }
   temp[48]=params[24];
   fwrite_ertu_params(EEADDR_SET_F21,temp,sizeof(temp));
   
   *itemDataLen = 25;
   return 0;


}
/*+++
  功能：查询终端电能量费率时段和费率数
---*/
INT16U query_ertu_F21_nm(INT8U *data)
{
   INT8U temp[49];
   INT8U idx;

   fread_ertu_params(EEADDR_SET_F21,temp,49);
   for(idx=0;idx<24;idx++)
   {
      data[idx]=(temp[idx*2+1]<<4);
      data[idx]|=temp[idx*2];
   }
   data[24]=temp[48];

   return 25;

}
/*+++
  功能：设置终端电能量费率
---*/
INT8U set_ertu_F22_nm(INT8U *params,INT16U *itemDataLen)
{
   INT8U fl_num;

   fl_num=14;
   fwrite_ertu_params(EEADDR_SET_F22,&fl_num,1);

   fwrite_ertu_params(EEADDR_SET_F22+1,params,56);

   *itemDataLen = 56;

   return 0;

}
/*+++
  功能：查询终端电能量费率
---*/
INT16U query_ertu_F22_nm(INT8U *data)
{

   fread_ertu_params(EEADDR_SET_F22+1,data,56);

   return 56;

}
/*+++
  功能：设置测量点基本参数
---*/
INT16U set_ertu_F25_nm(INT16U spot_idx,INT8U *params)
{
   INT8U temp[11];

   mem_set(temp,sizeof(temp),0);
   mem_cpy(temp,params,7);
   temp[10]=params[7];

   fwrite_array(spot_idx,PIM_METER_F25,temp,sizeof(temp));
   
   return  8;


}
/*+++
  功能：查询测量点基本参数
---*/
INT16U query_ertu_F25_nm(INT16U spot_idx,INT8U *data)
{

   fread_array(spot_idx,PIM_METER_F25,data,11);
   data[7]=data[10];
   return 8;

}
/*+++
  功能：设置测量点限值参数
---*/
INT16U set_ertu_F26_nm(INT16U spot_idx,INT8U *params)
{
   INT8U temp[57];
   INT8U pos,idx;
   pos=0;
   idx=0;

   mem_set(temp,sizeof(temp),0);

   mem_cpy(temp,params,8);
   pos+=8;
   idx+=8;
   //电压上上限
   temp[pos++]=0x02;
   temp[pos++]=0x50;
   temp[pos++]=0x01;
   //电压下下限
   temp[pos++]=params[idx++];
   temp[pos++]=params[idx++];
   temp[pos++]=0x02;
   temp[pos++]=0x50;
   temp[pos++]=0x01;
   //相电流上上限（过流门限）
   ConvertIfromNm(temp+pos,params+idx);
   pos+=3;
   idx+=2;
   temp[pos++]=0x02;
   temp[pos++]=0x50;
   temp[pos++]=0x01;
   //相电流上限（额定电流门限）
   ConvertIfromNm(temp+pos,params+idx);
   pos+=3;
   idx+=2;
   temp[pos++]=0x02;
   temp[pos++]=0x50;
   temp[pos++]=0x01;
   //零序电流上限
   ConvertIfromNm(temp+pos,params+idx);
   pos+=3;
   idx+=2;
   temp[pos++]=0x02;
   temp[pos++]=0x50;
   temp[pos++]=0x01;
   //视在功率上上限 
   temp[pos++]=params[idx++];
   temp[pos++]=params[idx++];
   temp[pos++]=params[idx++];
   temp[pos++]=0x02;
   temp[pos++]=0x50;
   temp[pos++]=0x01;

   //视在功率上限 
   temp[pos++]=params[idx++];
   temp[pos++]=params[idx++];
   temp[pos++]=params[idx++];
   temp[pos++]=0x02;
   temp[pos++]=0x50;
   temp[pos++]=0x01;
   //三相电压不平衡限值
   temp[pos++]=params[idx++];
   temp[pos++]=params[idx++];
   temp[pos++]=0x02;
   temp[pos++]=0x50;
   temp[pos++]=0x01;

   //三相电流不平衡限值
   temp[pos++]=params[idx++];
   temp[pos++]=params[idx++];
   temp[pos++]=0x02;
   temp[pos++]=0x50;
   temp[pos++]=0x01;
   //连续失压时间限值
   temp[pos++]=params[idx++];
   
   fwrite_array(spot_idx,PIM_METER_F26,temp,pos);
   
   return idx;


}
/*+++
  功能：查询测量点限值参数
---*/
INT16U query_ertu_F26_nm(INT16U spot_idx,INT8U *data)
{
   INT8U temp[57];
   INT8U pos,idx;

   pos=0;
   idx=0;

   fread_array(spot_idx,PIM_METER_F26,temp,57);
   //   电压合格上限   电压合格下限   电压断相门限   电压上上限（过压门限）
   mem_cpy(data,temp,8);
   pos+=11;
   idx+=8;

   //电压下下限
   data[idx++]=temp[pos++];  
   data[idx++]=temp[pos++];  
   pos+=3;
   
   //相电流上上限（过流门限）
   ConvertI2Nm(data+idx,temp+pos);
   pos+=6;
   idx+=2;
   //相电流上限（额定电流门限）
   ConvertI2Nm(data+idx,temp+pos);
   pos+=6;
   idx+=2;
   //零序电流上限
   ConvertI2Nm(data+idx,temp+pos);
   pos+=6;
   idx+=2;
   //视在功率上上限 
   data[idx++]=temp[pos++];  
   data[idx++]=temp[pos++];  
   data[idx++]=temp[pos++];  
   pos+=3;
   //视在功率上限 
   data[idx++]=temp[pos++];  
   data[idx++]=temp[pos++];  
   data[idx++]=temp[pos++];  
   pos+=3;
   //三相电压不平衡限值
   data[idx++]=temp[pos++];  
   data[idx++]=temp[pos++];  
   pos+=3;
   //三相电流不平衡限值
   data[idx++]=temp[pos++];  
   data[idx++]=temp[pos++];  
   pos+=3;
   //连续失压时间限值
   data[idx++]=temp[pos++];  
  return idx;

}
/*+++
  功能：设置直流模拟量输入变比
---*/
INT8U set_ertu_F81_nm(INT8U *params,INT16U *itemDataLen)
{
   double dval;
   INT8U temp[4];

   mem_set(temp,4,0);
   if(fmt02_to_double(params,&dval))
   {
      OSTASK_PROTECT_BEGIN; 
      dval*=20;   
      OSTASK_PROTECT_END;
      double_to_fmt02(dval,temp+2);
   }
   fwrite_ertu_params(EEADDR_SET_F81,temp,4);
   
   *itemDataLen = 2;
   return 0;


}
/*+++
  功能：查询直流模拟量输入变比
---*/
INT16U query_ertu_F81_nm(INT8U *data)
{
   double dval;
   INT8U temp[2];

   fread_ertu_params(EEADDR_SET_F81+2,temp,2);
   if(fmt02_to_double(temp,&dval))
   {
      OSTASK_PROTECT_BEGIN; 
      dval/=20;
      OSTASK_PROTECT_END;
      double_to_fmt02(dval,temp);

   }
   data[0]=temp[0];
   data[1]=temp[1];
   return 2;

}
INT16U query_nm_F14(INT8U *Reply)
{
   INT8U idx,len,buffer[80],count,pos;

   //读取当前参数配置
   fread_ertu_params(EEADDR_SET_F14,buffer,80);

   //看有效配置数量
   count = 0;
   pos = 1;
   for(idx=0;idx<MAX_ADDGROUP_COUNT;idx++)
   {
       if(buffer[idx*10] == (idx+1) )
       {
           count ++;
           len =  buffer[idx*10+1]+2;
           mem_cpy(Reply+pos,buffer+idx*10,len);
           pos += len;
       }
   }
   Reply[0] = count;
   return pos;
}
/*+++
 功能：误差对时
 参数: INT16s T   误差时间
 返回： 成功OK
        其他
---*/
INT8U  set_ertu_time_t(INT16S T)
{
    INT8U bcd[7],mytime[6];
    INT8U weekofday; 

    if(0==T)return 0;

    mem_cpy((void*)mytime,(void*)datetime,6);
    if(T<0)
      datetime_minus_seconds(&mytime[YEAR],&mytime[MONTH],&mytime[DAY],&mytime[HOUR],&mytime[MINUTE],&mytime[SECOND],T*-1);
    else
      datetime_add_seconds(&mytime[YEAR],&mytime[MONTH],&mytime[DAY],&mytime[HOUR],&mytime[MINUTE],&mytime[SECOND],T);


    weekofday =  weekDay(mytime[YEAR],mytime[MONTH],mytime[DAY]);

    bcd[0] = byte2BCD(mytime[SECOND]);         //秒
    bcd[1] = byte2BCD(mytime[MINUTE]);         //分
    bcd[2] = byte2BCD(mytime[HOUR]);        //时
    bcd[3] = byte2BCD(weekofday);             //星期
    bcd[4] = byte2BCD(mytime[DAY]);         //日
    bcd[5] = byte2BCD(mytime[MONTH]);      //月
    bcd[6] = byte2BCD(mytime[YEAR]);         //年

    return set_ertu_time(bcd,TRUE,TRUE);
}
/*+++
  功能：内蒙古扩展功能，对电表进行拉闸、合闸控制
  参数：
       INT8U *frameData,      数据单元
       INT16U *itemDataLen    输出，数据单元实际处理的长度
---*/
INT8U nm_ctrl_meter(INT8U *frameData,INT16U *itemDataLen)
{
   INT16U meter_seq;
   INT8U  error;
   error=0;
   *itemDataLen=7;
   //电能表配置序号
   meter_seq = frameData[0] + frameData[1]*0x100;


   return error;
}
/*+++
  功能：读取打包文件信息
  参数：
       INT16U *sec_count        [输出] 总段数
       INT16U *sec_data_size    [输出] 每段数据长度
---*/
void ftp_read_file_info(INT16U *sec_count,INT16U *sec_data_size)
{
	INT32U	filesize;
    INT8U filehead[]={'T','O','P','S','C','O','M','M'};
    INT8U buffer[8];

    *sec_count=0;
    *sec_data_size=1;

    //先读取文件水印，检查打包文件是否存在
	os_spansion_readArray(FLADDR_FILE_INFO,buffer,8);//写文件水印
	if(0 == compare_string(filehead,buffer,8))
    {
        //读取文件打包信息
		os_spansion_readArray(FLADDR_FILE_INFO+38,buffer,8);//文件打包信息
        mem_cpy(sec_data_size,buffer,2);
        mem_cpy(&filesize,buffer+2,4);
        OSTASK_PROTECT_BEGIN;
        *sec_count=filesize / *sec_data_size;
        if(filesize % *sec_data_size !=0)
        {
            *sec_count++;
        }
     	OSTASK_PROTECT_END;
    }
}
/*+++
  功能：更新每数据包的使用标志
  参数：
       IINT16U package_no        第几个数据包
---*/
void ftp_update_package_flag(INT16U package_no)
{
	INT8U val;
    
    os_spansion_readArray(FLADDR_PACKAGE_RECORD+package_no/8,&val,1);
    set_bit_value(&val,1,package_no %8);
    os_spansion_writeArray(FLADDR_PACKAGE_RECORD+package_no/8,&val,1);
}

/*+++
  功能：内蒙古远程升级F1
  参数：
       PLMS_RESP *resp        应答结构体变量
       INT16U frameLen       数据单元的长度
       INT8U *frameData,      数据单元
---*/
void process_ftp_nm_F1(PLMS_RESP *resp,INT16U frameLen,INT8U *frameData)
{
    INT16U pos,idx,idx1;
    INT16U pn,fn;
    INT16U nCount;
    INT16U fileDataLen;
    INT16U sec_count;
    INT16U cur_sec;
    INT16U sec_data_size;
    INT32U progAddr;
    INT32U startAddr;
    INT32U progLen;

    INT8U fileFlag,fileProp,fileCmd;
    INT8U resetFlag;

    idx=0;
    
    byte2int16(frameData,&nCount);  //总段数
    idx+=2;
    progAddr = bin2_int32u(frameData+idx);   //第i段标识
    startAddr = progAddr;        //当前段起始地址
    idx+=4;
    byte2int16(frameData+idx,&fileDataLen);  //第i段数据长度Lf
    idx+=2;

    fn = DT_F2; //全部否认
    resetFlag=FALSE;

    if(fileCmd==0)   //写程序代码到存储器
    {
       fn = DT_F1;
       
      // 设置程序开始字节地址
      progAddr += FLADDR_PROG_START;

      //如果是起始地址，则擦除整个程序代码区，共9个扇区
      if(FLADDR_PROG_START == progAddr)
      {
         os_spansion_erase_sector(0,9);
      }

       //保存数据

       if(fileDataLen < 0x8000)
       {
          #ifdef __DEBUG_RECINFO__
          snprintf(info,100,"rtsp_prog: addr=%d   len=%d",progAddr,fileDataLen);
          println_info(info);
          #endif

          //对695的数据进行解密
          #ifdef __32MX695F512L_H
           for(idx1=0;idx1<fileDataLen;idx1++)
           {
              frameData[idx+idx1] = frameData[idx+idx1] ^ __RTSP_SPEC__;
           }
          #endif

          os_spansion_writeArray(progAddr,frameData+idx,fileDataLen);
       }
       else
       {
          //利用frameData作为缓冲区，这个缓冲区可以使用700个字节，
          mem_set(frameData+idx,656,frameData[idx]);
          fileDataLen &=0x7FFF;

          #ifdef __DEBUG_RECINFO__
          snprintf(info,100,"rtsp_prog: addr=%d   len=%d",progAddr,fileDataLen);
          println_info(info);
          #endif

          while(fileDataLen>0)
          {
             nCount =  fileDataLen;
             if(nCount > 656) nCount = 656;
             os_spansion_writeArray(progAddr,frameData+idx,nCount);
             progAddr += nCount;
             fileDataLen -= nCount;
          }
       }
   }


    
    ENDFTP:

    set_pn_fn(resp->buffer, &pos, pn, fn);

    //应答确认数据

    //由于协议中未定义文件编码具体格式，因此暂时用文件标识和文件属性代替
     resp->buffer[pos++] = fileFlag;
     resp->buffer[pos++] = fileProp;


     ftp_read_file_info(&sec_count,&sec_data_size);
     //根据当前地址计算，第几段
     cur_sec=startAddr / sec_data_size;
     if(startAddr % sec_data_size !=0)
     {
       cur_sec++;
     }
     //总块数
     resp->buffer[pos++] = sec_count;
     resp->buffer[pos++] = sec_count>>8;
     //当前块数
     resp->buffer[pos++] = cur_sec;
     resp->buffer[pos++] = cur_sec>>8;

     //更新断点续传标志
     ftp_update_package_flag(cur_sec);

    SetFrameLenThenSendFrame(resp,pos);

}
/*+++
  功能：内蒙古远程升级F2,读取断点续传信息
  参数：
       PLMS_RESP *resp        应答结构体变量
       INT16U frameLen       数据单元的长度
       INT8U *frameData,      数据单元
---*/
void process_ftp_nm_F2(PLMS_RESP *resp,INT16U frameLen,INT8U *frameData)
{
	INT32U	filesize,cur_offset;
    INT16U 	pos,idx,data_len,sec_count,read_count,index;
    INT8U filehead[]={'T','O','P','S','C','O','M','M'};
    INT8U buffer[36];
    INT8U break_point;
    
    pos = POS_DATA;
    break_point=0;
    
    set_pn_fn(resp->buffer, &pos, DA_P0, DT_F2);
    //先读取文件水印，检查打包文件是否存在
	os_spansion_readArray(FLADDR_FILE_INFO,buffer,8);//写文件水印
	if(0 == compare_string(filehead,buffer,8))
    {
    	resp->buffer[pos++] =  1;//文件存在
        //读取文件打包信息
		os_spansion_readArray(FLADDR_FILE_INFO+8,buffer,36);//文件打包信息
        mem_cpy(&data_len,buffer+30,2);
        mem_cpy(&filesize,buffer+32,4);
        if(filesize>537600)//如果文件长度大于实际允许长度，则不允许升级
        {
            filesize=0;
        }
        if((data_len>=2048) || (data_len==0))
        {
            data_len=512;
        }
        OSTASK_PROTECT_BEGIN;
        sec_count=filesize/data_len;
        if(filesize % data_len !=0)
        {
            sec_count++;
        }
        read_count=sec_count/32;
        if(sec_count % 32 !=0)
        {
            read_count++;
        }
        
     	OSTASK_PROTECT_END;
        
        mem_cpy(resp->buffer+pos+4,buffer,30);//软件版本信息
        //读取断点续传信息
        for(idx=0;idx<read_count;idx++)
        {
            //一次读32字节
			os_spansion_readArray(FLADDR_PACKAGE_RECORD+idx*32,buffer,32);
            //按位检查，如果该为不为0，说明没有下载
			for(index=0;index<256;index++)
            {
            	cur_offset=idx*256+index;
                if(0 !=get_bit_value(buffer,32,index))
                {
			        mem_cpy(resp->buffer+pos,&cur_offset,4);//下一个数据段偏移
                    break_point=2;
                    break;
                }
                //如果超出文件总段数，则说明已经下载完成
            	if(cur_offset>=sec_count)
                {
                   break_point=1;
                   break;
                }
            }
            //如果已经找到了断点或已经下载完毕，则无须再继续查找
            if(break_point>0)
            {
                   break;
            }
		}
    }
    else
    {
        mem_set(resp->buffer+pos,5,0);
        pos+=5;
        mem_set(resp->buffer+pos,30,0xFF);//软件版本信息
        pos+=30;
    }
    SetFrameLenThenSendFrame(resp,pos);



}
/*+++
  功能：内蒙古远程升级F3,复位文件下载
  参数：
       PLMS_RESP *resp        应答结构体变量
       INT16U frameLen       数据单元的长度
       INT8U *frameData,      数据单元
---*/
void process_ftp_nm_F3(PLMS_RESP *resp,INT16U frameLen,INT8U *frameData)
{
    INT16U 	pos;
    
    pos = POS_DATA;
	os_spansion_erase_sector(0,9);
    set_pn_fn(resp->buffer, &pos, DA_P0, DT_F3);
    resp->buffer[pos++] =  0x55;//55表示成功 AA表示失败
    SetFrameLenThenSendFrame(resp,pos);

}
/*+++
  功能：内蒙古远程升级F4,文件打包
  参数：
       PLMS_RESP *resp        应答结构体变量
       INT16U frameLen       数据单元的长度
       INT8U *frameData,      数据单元
---*/
void process_ftp_nm_F4(PLMS_RESP *resp,INT16U frameLen,INT8U *frameData)
{
    INT16U 	pos;
    INT8U filehead[]={'T','O','P','S','C','O','M','M'};
    pos = POS_DATA;
	//存储打包信息到FLADDR_FILE_INFO
    if(frameLen==256)
    {
    	os_spansion_writeArray(FLADDR_FILE_INFO+8,frameData,frameLen);
        os_spansion_writeArray(FLADDR_FILE_INFO,filehead,8);//写文件水印
    }
    set_pn_fn(resp->buffer, &pos, DA_P0, DT_F4);
    resp->buffer[pos++] =  0x55;//55表示成功 AA表示失败
    SetFrameLenThenSendFrame(resp,pos);

}

void default_param_neimeng(void)
{




}
#endif


