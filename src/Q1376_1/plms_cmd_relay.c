/***************************************************************

    DL/T 698-200X G表===主站下行报文处理任务

    中继转发命令处理 AFN=0x10

    F1:     透明转发
    F9:     转发主站直接对电表的抄读数据命令
    F10:    转发主站直接对电表的遥控跳闸/允许合闸命令
    F11:    转发主站直接对电表的遥控送电命令



***************************************************************/
#include "app_3761.h"
#include "app_var_3761.h"
#include "protocol_app_handler.h"
#include "../task_meter.h"


void  process_readRelay_F1(objRequest  *pRequest,objResponse *pResp);
void  process_readRelay_F9(objRequest  *pRequest,objResponse *pResp);
void  process_readRelay_F10(objRequest  *pRequest,objResponse *pResp);
void  process_readRelay_F10F11(objRequest  *pRequest,objResponse *pResp,INT16U fn);
BOOLEAN  process_readRelay_F12(objRequest  *pRequest,objResponse *pResp);
void process_readRelay_F13(objRequest  *pRequest,objResponse *pResp);

//检查是否为标准的645报文
BOOLEAN check_cmd_is_GB645(INT8U *frame,BOOLEAN is_update_cy_meterNo_cs);
BOOLEAN check_cmd_is_3761(INT8U *frame);
BOOLEAN check_cmd_is_645_cast(INT8U *frame,INT16U *pos);

//载波监控645报文
INT16U plc_readRelay_GB645(INT8U *frame,INT16U cmd_len,INT8U *resp,INT8U max_resp_len,INT16U max_wait_time);

void clear_all_batch_transparent_meter_task(INT16U pi);
INT16U set_param_batch_transparent_meter_task(INT16U spot_idx,INT16U meter_seq,INT8U *params);
BOOLEAN query_param_batch_transparent_meter_task(INT16U spot_idx,objRequest *pRequest,objResponse *pResp,INT16U *from_seq,INT16U* req_bytes);


/*+++
   功能：数据转发  F1,F9,F10,F11   -P0
　　　
    参数:
       INT8U *Reply 应答帧工作区
       INT16U frameLen  命令帧的数据单元域长度
       INT8U *frameData 命令帧的数据单元域、

    描述：2008-03-28 如果是F2-P0，并且是从串口收到的命令，则数据体转发到远程通道上。
          这个功能用于调试报文。
          也可在现场用来上传其他内容。

---*/
void  process_frame_readRelay(objRequest  *pRequest,objResponse *pResp)
{
    #ifdef __BATCH_TRANSPARENT_METER_TASK__
    INT8U *frameData;
    INT16U pn;
    #endif
    INT16U fn;
    #ifdef __BATCH_TRANSPARENT_METER_TASK__
    INT16U frameLen,pos,pos_resp;
    INT16U pn_idx;
    INT16U meter_seq,from_seq,req_bytes;
    INT8U more_frame_flag;
    #endif

    ClrTaskWdt();
    //设置控制码：DIR=1,PRM=0, FUNC=8  响应报文，用户数据
    pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_USERDATA;

//    //检查命令附加信息域的口令
//    if(! check_frame_pw(frameLen,frameData) )
//    {
//         return;
//    }

    #ifdef __BATCH_TRANSPARENT_METER_TASK__
    frameData = pRequest->frame + pRequest->pos;
    frameLen = pRequest->userDataLen;
    #endif

    //下行报文数据单元标识
    #ifdef __BATCH_TRANSPARENT_METER_TASK__
    pn=bin2_int16u(pRequest->frame + pRequest->pos   );
    #endif
    fn=bin2_int16u(pRequest->frame + pRequest->pos +2);

    //执行不同的命令转发
    pRequest->pos += 4;
    if(fn == DT_F1)
    {
        //透明转发
        process_readRelay_F1(pRequest,pResp);
    }
    else if(fn == DT_F9)
    {
        //电表抄读
        process_readRelay_F9(pRequest,pResp);
    }
    else if(fn == DT_F10)
    {
//        if((pRequest->frame[26]==0xAA) || (pRequest->frame[26]==0x55)) //不带中继级别
//            process_readRelay_F10F11(pRequest,pResp,fn);//转发主站直接对电表的遥控跳闸/允许合闸命令
//        else
//            process_readRelay_F10(pRequest,pResp);

    }
    else if(fn == DT_F11)
    {
        //转发主站直接对电表的遥控送电命令
        //process_readRelay_F10F11(pRequest,pResp,fn);
    }
    #ifdef __ENABLE_ESAM2__
    else if(fn == DT_F12)
    {
        if(process_readRelay_F12(pRequest,pResp))
        {
            app_send_ACK(pRequest,pResp);
        }
        else
        {
            app_send_NAK(pRequest,pResp);
        }
        return ;
    }
    else if(fn == DT_F13)
    {
        process_readRelay_F13(pRequest,pResp);
    }
    #endif //#ifdef __ENABLE_ESAM2__
    #ifdef __BATCH_TRANSPARENT_METER_TASK__
    else if(fn == DT_F305)
    {
        pos = 0;
        while(pos < frameLen)
        {
            pn = bin2_int16u(frameData+pos);
            fn = bin2_int16u(frameData+pos+2);
            pos += 4;
            if(fn == DT_F305)
            {
                clear_all_batch_transparent_meter_task(pn);
            }
            else break;
        }
        app_send_ACK(pRequest,pResp);
    }
    else if(fn == DT_F306)
    {
        pos = 0;
        while(pos < frameLen)
        {
            pn = bin2_int16u(frameData+pos);
            fn = bin2_int16u(frameData+pos+2);
            pos += 4;
            if(fn == DT_F306)
            {
                pn_idx = trans_set_pn_2_pnidx(pn);

                meter_seq = meter_spot_2_meter_seq_map[pn_idx];
                //MASK配置序号
                meter_seq &= 0x7FF;

                if((meter_seq > 0) && (meter_seq <= MAX_METER_COUNT))
                {
                    pos += set_param_batch_transparent_meter_task(pn_idx,meter_seq,frameData+pos);
                }
                else
                {
                    app_send_NAK(pRequest,pResp);
                    return;
                }
            }
            else break;
        }
        app_send_ACK(pRequest,pResp);
    }
    else if(fn == DT_F307)
    {
        pos = 0;
        pos_resp = pResp->pos;
        req_bytes = 0;
        //resp->max_reply_pos = 200;

        while(pos < frameLen)
        {
            from_seq = 0;
            req_bytes = 0;

            pn = bin2_int16u(frameData+pos);
            fn = bin2_int16u(frameData+pos+2);
AGAIN:
            if(fn == DT_F307)
            {
                //设置数据标识
                set_pn_fn(pResp->frame,&(pResp->pos),pn,fn);

                pn_idx = trans_set_pn_2_pnidx(pn);

                more_frame_flag = query_param_batch_transparent_meter_task(pn_idx,pRequest,pResp,&from_seq,&req_bytes);

                if (more_frame_flag)
                {
                    //移动应答报文当前位置
                    if(FALSE == app_send_response_frame(pRequest,pResp,FALSE) ) return;

                    pResp->pos = pos_resp;

                    goto AGAIN;
                }
            }
            else break;

            //pos_resp += itemDataLen;
            pos += 4 + req_bytes;
            pRequest->pos += 4 + req_bytes;
        }

        if(pResp->pos > pos_resp)
        {
            app_send_response_frame(pRequest,pResp,TRUE);
        }
    }
    #endif //#ifdef __BATCH_TRANSPARENT_METER_TASK__
    else
    {
       //不能识别的转发任务，发送否定报文
        app_send_NAK(pRequest,pResp);
    }
}



/*扩展645 表号AABBCCDDEEFF 设置是否使用采集器地址抄
从376.1AFN=10数据转发F1：透明转发完成
转发端口=31
取消采集器：FE FE FE 68 FF EE DD CC BB AA 68 14 02 00 00 E1 16
成功返回：  FE FE FE 68 FF EE DD CC BB AA 68 94 00 5F 16
异常返回：  FE FE FE 68 FF EE DD CC BB AA 68 D4 00 9F 16
启用采集器：FE FE FE 68 FF EE DD CC BB AA 68 14 02 11 11 03 16
成功返回：同上
异常返回：同上
查询状态：  FE FE FE 68 FF EE DD CC BB AA 68 11 02 22 22 22 16
成功返回：68 FF EE DD CC BB AA 68 91 02 00 00 5E 16
          68 FF EE DD CC BB AA 68 91 02 11 11 80 16
*/
INT16U process_plc_cjq_valid(INT8U *frameData,INT8U *resp)
{
    INT8U flag,chg_flag,buffer[20],buffer_len;

    buffer_len = Hex2Binary("FFEEDDCCBBAA",12,buffer);
    if(compare_string(frameData+1,buffer,buffer_len) != 0) return 0;

    flag = 0;
    chg_flag = 0;
    //不使用采集器地址抄读
    buffer_len = Hex2Binary("68FFEEDDCCBBAA6814020000E116",28,buffer);
    if(compare_string(frameData,buffer,buffer_len) == 0)
    {
//        if ((system_flag & SYS_PLC_REC_CJQ) == 0)
//        {
//        system_flag |= SYS_PLC_REC_CJQ;
//            chg_flag = 1;
//        }
        flag = 1;
    }
    //使用采集器地址抄读
    buffer_len = Hex2Binary("68FFEEDDCCBBAA68140211110316",28,buffer);
    if(compare_string(frameData,buffer,buffer_len) == 0)
    {
//        if(system_flag & SYS_PLC_REC_CJQ)
//        {
//        system_flag &= ~SYS_PLC_REC_CJQ;
//            chg_flag = 1;
//        }
        flag = 1;
    }
    //查询采集器地址状态
    buffer_len = Hex2Binary("68FFEEDDCCBBAA68110222222216",28,buffer);
    if(compare_string(frameData,buffer,buffer_len) == 0)
    {
        flag = 2;
    }
    
    if(flag == 1)
    {
        if (chg_flag == 1)
        {
//        if(system_flag & SYS_PLC_REC_CJQ)
//        {
//            flag = 0xAA;
//        }
//        else
//        {
//            flag = 0x55;
//        }
        //To do
        /*
        write_fmByte(FMADDR_PLC_CJQ_MODE,flag);/
        //更改了标识,执行初始化路由操作
        wait_rectask_exec(RECMETER_TASK_INIT,200);  //单位：100ms
            //执行重新抄表
            GPLC.rec_control.re_rec_flag = 1;   //重新抄表,重新抄日冻结
            wait_rectask_exec(RECMETER_TASK_REDO,200); //单位：100ms
        */
        }
        return Hex2Binary("68FFEEDDCCBBAA6894005F16",24,resp);
    }
    else if(flag == 2)
    {
//        if(system_flag & SYS_PLC_REC_CJQ)
//        {
//            return Hex2Binary("68FFEEDDCCBBAA68910200005E16",28,resp);
//        }
//        else
//        {
//            return Hex2Binary("68FFEEDDCCBBAA68910211118016",28,resp);
//        }
    }
    return 0;
}

/*+++
  功能：透明转发任务
  参数：
        PLMS_RESP *Reply 应答帧工作区
        INT8U *frameData 命令帧的数据单元域，从数据标识PNFN之后开始
  返回：
        无
  描述：
      载波口，如果是645报文，则转换为监控报文格式进行转发。任务转换为监控任务.
      
      1）下行报文：
          数据内容                 数据格式        单位         字节数
          ------------------------------------------------------------
          [0]通信端口号                 BIN                        1
          [1]通信控制字                 BS8                        1
          [2]接收等待报文超时时间        BIN           10ms/1s     1
          [3]接收等待字节超时时间        BIN           10ms        1
          [4]转发内容长度n              BIN           字节　       2
          [6]转发内容　　              　　　　　　　　　　　　　　n

      2）上行报文
          数据内容                 数据格式        单位         字节数
          ------------------------------------------------------------
          通信端口号                 BIN                        1
          转发长度n                 BIN           字节　        2
          转发内容　　              　　　　　　　　　　　　  　n　
      3） 端口1 ：载波通道
      4） 端口2 ：抄表485
      5)  端口3 : 级联485
      6)  端口4 : RS232
      7)  端口5 : 红外
      8)  端口6 : 远程通道

---*/
int16_t gprs_send_cmd_wait_OK(uint8_t *cmd,uint8_t *resp,uint16_t max_resp_len,uint16_t timeout_10ms);
void  process_readRelay_F1(objRequest  *pRequest,objResponse *pResp)
{
    #ifdef __POWER_CTRL_I__
    INT32U timer1;
    INT32U timer2;
    INT16S recv_data;
    #endif
    INT8U *frameData;
    INT8U *respData;
    INT16U framepos;
    INT16U pn,fn;
    INT16U BaudRate;
    INT16U cmd_len;
    INT16U max_resp_len;
    INT16U idx;
    INT16U pos_recv_len;
    INT16U TotalTimeOut;
    INT16U IdleTimeOut;
    INT8U  Parity;
    INT8U  Port;
    INT8U cast_flag;
//    BOOLEAN cast_time;
    PORT_COMM_CTRL   PortCtrl;
    INT8U recvLen;
    TRANS_READ_PARAM param;
    cast_flag = 0;
//    cast_time = FALSE;

    frameData = pRequest->frame + pRequest->pos;
    //设置应答报文起始位置
    pResp->pos = POS_DATA;

    //通信端口
    Port = frameData[0];

    //通信控制frameData[1]
    PortCtrl.port_ctrl = frameData[1];

    //通信控制字的波特率和校验方式
    //get_port_baudrate_parity(&BaudRate,&Parity,PortCtrl);

    //等待接收报文超时时间　10ms
    if(frameData[2] & 0x80)
    {
       TotalTimeOut = (frameData[2] & 0x7F) * 100;
    }
    else
    {
       TotalTimeOut = frameData[2] ;
    }

    //等待接收字节超时时间　ms
    IdleTimeOut =  10 * frameData[3];


    //转发内容长度  frameData[4][5]
    #if defined(__QGDW_FK2005__)|| defined(__PROVICE_NEIMENG__)
    cmd_len = frameData[4];
    framepos = 5;
    #else
    cmd_len = frameData[5];
    cmd_len = (cmd_len <<8) + frameData[4];
    framepos = 6;
    #endif

    //设置数据标识
    pn = DA_P0;
    fn = DT_F1;
    set_pn_fn(pResp->frame, &pResp->pos, pn, fn);


    #if !defined(__QGDW_FK2005__)
    //应答端口
    pResp->frame[pResp->pos++] = Port;

    //应答长度
    pos_recv_len = pResp->pos;
    pResp->pos +=2;
    #else
    //应答长度
    pos_recv_len = pResp->pos;
    pResp->pos +=1;
    #endif

    param.baud  = BaudRate;
    param.cs_bit  = Parity;
    param.data_bit =8;
    param.stop_bit =1;
    param.recv_frame_time_out = TotalTimeOut;
    param.recv_byte_time_out =  IdleTimeOut;
    respData =pResp->frame+pResp->pos;
    max_resp_len=SIZE_OF_APP_PROTOCOL_FRAME - pResp->pos;

    if(cmd_len > (pRequest->userDataLen - RELAY_MINUS_LEN))//userDataLen用户数据区减去前面的PnFn、转发控制字等内容共10个字节
    {
       app_send_NAK(pRequest,pResp);
       return;
    }
    //根据不同端口处理,目前支持载波、485、红外、GPRS
    //协议支持376.1、376.2、645
    if((Port == COMMPORT_485_REC) || (Port == COMMPORT_485_CAS)  || (Port == COMMPORT_485_CY) || (Port == 29))
    {
        cmd_len = app_trans_send_meter_frame(frameData+framepos,cmd_len,respData,max_resp_len,300);
    }
	 else if(Port == COMMPORT_GPRS)//转GPRS 串口，目的用来远程调试GPRS模块，发送AT指令
	 {
      frameData+=framepos;
      idx=0;
      recvLen=0;
      while(idx<cmd_len)
      {
         if(idx>0)
         {   
            if(((frameData[idx-1]==0x0D) && (frameData[idx]==0x0A)) || (idx==cmd_len-1))
            {
         		 if(idx+1>2)
                {
                   //gprs_send_buffer(frameData,idx+1); //发送指令
            	   //recvLen+=gprs_receive_response(read_uart_gprs,resp->buffer+pos,400,30);//接收指令
				   //recvLen+=get_gprs_cmd_resp(frameData,idx+1,resp->buffer+pos,3000);
                   #ifdef __SOFT_SIMULATOR__
                   recvLen += 0;  
                   #else
                    //暂时屏蔽
                   //recvLen+=  ex_get_cmd_resp(frameData,idx+1,respData,TotalTimeOut);
                   recvLen+= gprs_send_cmd_wait_OK(frameData,respData,0xFF,TotalTimeOut);
                   #endif
                   cmd_len-=(idx+1);
                   frameData+=(idx+1);
                   idx=0;
                   continue;
               }
            }
         }
         idx++;
      }
      cmd_len=recvLen;
	 }
    else
    {
          //暂不支持其他串口的透明转发
          app_send_NAK(pRequest,pResp);
          return;
    }


    #if defined(__QGDW_FK2005__)
    pResp->frame[pos_recv_len] = cmd_len;
    #else
    pResp->frame[pos_recv_len] = cmd_len;
    pResp->frame[pos_recv_len+1] = cmd_len>>8;
    #endif
    pResp->pos += cmd_len;

       //发送响应祯，结束=TRUE
    app_send_response_frame(pRequest,pResp,TRUE);

//    if(cast_flag)
//    {
//        //执行透传的广播指令
//        frameData[framepos+pos_645-1] = cast_len;
//        frameData[framepos+pos_645-2] = 0x02;
//        wait_rectask_exec_param(RECMETER_TASK_CAST_CMD,3000,frameData+framepos+pos_645-2,cast_len+2,resp->buffer+pos,cast_time);
//    }
}


/*+++
  功能：电表抄读
  参数：
        objRequest  *pRequest
        objResponse *pResp
  返回：
        无
  描述：
      1.目前不支持采集器下电表

---*/
//TODO需要考虑采集器下电表
void process_readRelay_F9(objRequest  *pRequest,objResponse *pResp)
{

    INT8U *frameData;
    INT8U *dl645frame;
    INT32U item=0;
    INT16U wait_time;
    INT8U  *rec_rsp,rec_resplen;
    INT8U  Port,protocol_type;
    INT8U  relay_depth;
    INT8U  cmd_len,resp_len;
    #ifdef __PROVICE_JILIN__
    INT8U idx;
    #endif
    INT8U  meterno[6]={0};

    frameData  = pRequest->frame + pRequest->pos;
    dl645frame = pRequest->frame+ pRequest->frame_len;//如果已经超出最大缓冲区会出问题
    //设置数据标识
    set_pn_fn(pResp->frame, &pResp->pos, DA_P0, DT_F9);

    //通信端口
    Port = *frameData++;
    //应答：终端通信端口号
    pResp->frame[pResp->pos++] = Port;
    //应答：转发目标地址
    relay_depth = frameData[0];
    mem_cpy(pResp->frame+pResp->pos,frameData+1+6*relay_depth,6);
    mem_cpy(meterno,frameData+1+6*relay_depth,6);
    pResp->pos += 6;
    protocol_type=frameData[7+6*relay_depth];

    //数据标识

    mem_cpy(&item,frameData+8+6*relay_depth,4);
    mem_cpy(pResp->frame+pResp->pos+2,frameData+8+6*relay_depth,4);



    wait_time= 60*100;//60秒,单位为10ms
    resp_len =4;
    rec_rsp =   pResp->frame+pResp->pos+6;
    
    if(protocol_type==0)
    {
        //cmd_len=make_gb645_read_frame(dl645frame,meterno,item,NULL,0);    97表不需要支持
    }
    else
    {
        cmd_len=make_gb645_2007_frame(dl645frame,meterno,0x11,item,NULL,0);
    }
    //if(memory_fast_index_find_node_no(Port,meterno,&meter_idx,&rec_pos,&router_protocol,NULL))
    {
        //meter_idx &= FAST_IDX_MASK;
        //if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
        {
            //fread_meter_params(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
            //if(Port == COMMPORT_485_CAS)
            //{
            //    old_func=Channel_rs485_2.function;
            //    Channel_rs485_2.function=PORT_FUN_READMETER;
            //}
            rec_resplen=app_trans_send_meter_frame(dl645frame,cmd_len,rec_rsp,300,wait_time);
//            if((Port == COMMPORT_485_CAS) && (old_func!=1))
//            {
//                channel_rs485_2_init();
//            }
            if(rec_resplen>0)
            {
                rec_resplen = decode_gb645_frame(rec_rsp,rec_resplen);
                if(rec_resplen >= 4)
                {
                    pResp->frame[pResp->pos++] = TRANS_RECV_DATA;

                    if(rec_rsp[0]==0x91)
                    {
                        rec_resplen-= 6;
                        #ifdef __PROVICE_JILIN__ //吉林检测数据内容不减33
                        for(idx=0;idx<rec_resplen;idx++) rec_rsp[6+idx] += 0x33;
                        #endif
                        mem_cpy(rec_rsp,rec_rsp+6,rec_resplen);
                    }
                    else
                    {
                        rec_resplen -= 4;
                        #ifdef __PROVICE_JILIN__ //吉林检测数据内容不减33
                        for(idx=0;idx<rec_resplen;idx++) rec_rsp[4+idx] += 0x33;
                        #endif
                        mem_cpy(rec_rsp,rec_rsp+4,rec_resplen);
                    }
                }
                pResp->frame[pResp->pos++] = rec_resplen + resp_len;
                pResp->pos += rec_resplen + resp_len;
            }
            else
            {
                pResp->frame[pResp->pos++] =  TRANS_TIMEOUT;
                pResp->frame[pResp->pos++] =  4;
                pResp->pos+=4;
            }
        }

    }
    //发送响应祯，结束=TRUE
    app_send_response_frame(pRequest,pResp,TRUE);
}
/*+++
  功能：批量电表抄读，13扩展的
  参数：
        objRequest  *pRequest
        objResponse *pResp
  返回：
        无
  描述：
      1.目前不支持采集器下电表

---*/
//TODO需要考虑采集器下电表
void process_readRelay_F10(objRequest  *pRequest,objResponse *pResp)
{
    INT8U *frameData;
    INT8U *dl645frame;
    INT32U item=0,timer_cur;
    INT16U wait_time;
    INT8U  *rec_rsp,rec_resplen;
    INT8U  Port,protocol_type,item_count,idx_1;
    INT8U  relay_depth;
    INT8U  cmd_len,resp_len;
    #ifdef __PROVICE_JILIN__ 
    INT8U idx;
    #endif
    INT8U  meterno[6]={0};
    INT16U frame_pos;//,trans_count_pos;
    INT8U result_pos;
    INT16U frame_len_pos;
    INT16U temp_645_pos = 1000;


    timer_cur=system_get_tick10ms();

    rec_rsp = pResp->frame + temp_645_pos;
    frameData  = pRequest->frame + pRequest->pos;
    dl645frame = pRequest->frame+ pRequest->frame_len;//如果已经超出最大缓冲区会出问题
    //设置数据标识
    set_pn_fn(pResp->frame, &pResp->pos, DA_P0, DT_F10);

    //通信端口
    Port = *frameData++;
    //应答：终端通信端口号
    pResp->frame[pResp->pos++] = Port;
    //应答：转发目标地址
    relay_depth = frameData[0];
    mem_cpy(pResp->frame+pResp->pos,frameData+1+6*relay_depth,6);
    mem_cpy(meterno,frameData+1+6*relay_depth,6);
    pResp->pos += 6;
    //记录协议类型
    protocol_type=frameData[7+6*relay_depth];
//    trans_count_pos = 8;
    //数据标识
    item_count = frameData[8+6*relay_depth];//数据项个数
    frame_pos = 9+6*relay_depth;
    result_pos =   pResp->pos;//应答结果位置
    pResp->frame[result_pos] = 0;
    pResp->frame[result_pos+1] = 0;
    pResp->pos += 2;
    //检查电表是否存在
      #ifdef  __PROVICE_HENAN__
      if(Port == COMMPORT_485_CY) Port = COMMPORT_485_REC;
      else if(Port == 29) Port = COMMPORT_485_CY;
      else if(Port == COMMPORT_485_REC) Port = COMMPORT_485_CAS ;
      #endif

    
    for(idx_1=0;idx_1<item_count;idx_1++)
    {
        frame_len_pos =   pResp->pos;//应答数据项长度位置
        pResp->pos++;
        mem_cpy(&item,frameData+frame_pos,4);
        mem_cpy(pResp->frame+pResp->pos,frameData+frame_pos,4);
        frame_pos+=4;
        pResp->pos += 4;
        if(pResp->frame[result_pos] != TRANS_RECV_DATA)
        {
                 pResp->frame[result_pos] =  TRANS_ERR;
        }
        pResp->frame[result_pos+1] += 1;
        pResp->frame[frame_len_pos] =  4;
        if(pResp->pos >= 700)
        {
            app_send_response_frame(pRequest,pResp,TRUE);
        }
        continue;
    }
    app_send_response_frame(pRequest,pResp,TRUE);
    return;



    for(idx_1=0;idx_1<item_count;idx_1++)
    {
        if(second_elapsed(timer_cur) > 60)
        {
            app_send_response_frame(pRequest,pResp,TRUE);
            return;
        }
        frame_len_pos =   pResp->pos;
        pResp->pos++;
        mem_cpy(&item,frameData+frame_pos,4);
        mem_cpy(pResp->frame+pResp->pos,frameData+frame_pos,4);
        frame_pos+=4;
        pResp->pos += 4;
        wait_time= 60*100;//60秒,单位为10ms
        resp_len =4;
        
        if(protocol_type==0)
        {
            //cmd_len=make_gb645_read_frame(dl645frame,meterno,item,NULL,0);
        }
        else
        {
            //cmd_len=make_gb645_2007_read_frame(dl645frame,meterno,item,NULL,0);
            cmd_len=make_gb645_2007_frame(dl645frame,meterno,0x11,item,NULL,0);
        }
        //rec_resplen=app_trans_send_meter_frame(Port,NULL,&meter_doc,dl645frame,cmd_len,rec_rsp,300,wait_time);
        if(rec_resplen>0)
        {
            rec_resplen = decode_gb645_frame(rec_rsp,rec_resplen);
            //这里需要增加长度验证，避免超过最大报文
            if(rec_resplen >= 4)
            {
               pResp->frame[result_pos] = TRANS_RECV_DATA;
               pResp->frame[result_pos+1] += 1;
                //转发个数
                //trans_count++;
                //pResp->frame[result_pos+1] = trans_count;

               if(rec_rsp[0]==0x91)
               {
                    rec_resplen-= 6;
                    #ifdef __PROVICE_JILIN__ //吉林检测数据内容不减33
                    for(idx=0;idx<rec_resplen;idx++) rec_rsp[6+idx] += 0x33;
                    #endif
                    mem_cpy(pResp->frame+pResp->pos,rec_rsp+6,rec_resplen);
               }
               else
               {
                    rec_resplen -= 4;
                    #ifdef __PROVICE_JILIN__ //吉林检测数据内容不减33
                    for(idx=0;idx<rec_resplen;idx++) rec_rsp[4+idx] += 0x33;
                    #endif
                    mem_cpy(pResp->frame+pResp->pos,rec_rsp+4,rec_resplen);
               }
            }
            else if(rec_resplen == 0)
            {
                if(pResp->frame[result_pos] != TRANS_RECV_DATA)
                {
                    pResp->frame[result_pos] = TRANS_RECV_NAK;
                }
                pResp->frame[result_pos+1] += 1;
            }

             pResp->frame[frame_len_pos] = rec_resplen + resp_len;
             pResp->pos += rec_resplen;
             if(pResp->pos >= 700)
             {
                 //tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
                 app_send_response_frame(pRequest,pResp,TRUE);
             }
             continue;
             //pResp->pos += rec_resplen + resp_len;
        }
        else
        {
            if(pResp->frame[result_pos] != TRANS_RECV_DATA)
            {
             pResp->frame[result_pos] =  TRANS_TIMEOUT;
            }
            pResp->frame[result_pos+1] += 1;
             pResp->frame[frame_len_pos] =  4;
             //pResp->pos+=4;
             if(pResp->pos >= 700)
             {
                 //tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
                 app_send_response_frame(pRequest,pResp,TRUE);
             }
             continue;
        }
    }
    //发送响应祯，结束=TRUE
    app_send_response_frame(pRequest,pResp,TRUE);

}

/*+++
  功能：电表阀控
  参数：
        objRequest  *pRequest
        objResponse *pResp
        INT16U fn
  返回：
        无
  描述：
      1.目前不支持采集器下电表
---*/
//TODO需要考虑采集器下电表
//void  process_readRelay_F10F11(objRequest  *pRequest,objResponse *pResp,INT16U fn)
//{
//   INT8U *frameData;
//   INT8U *dl645frame;
//   INT16U meter_idx,rec_pos,wait_time;
//   INT8U  *rec_rsp,rec_resplen;
//   INT8U  Port,ctrl_flag;
//   INT8U  relay_depth;
//   INT8U  cmd_len,router_protocol,pwd_len;
//   INT8U  meterno[6]={0},meter_pwd[8]={0};
//   METER_DOCUMENT meter_doc;
//
//   frameData  = pRequest->frame + pRequest->pos;
//   if(pRequest->frame_len+100>SIZE_OF_APP_PROTOCOL_FRAME)
//   {
//        dl645frame = pResp->frame+ 100;
//   }
//   else
//   {
//        dl645frame = pRequest->frame+ pRequest->frame_len;
//   }
//   //设置数据标识
//   set_pn_fn(pResp->frame, &pResp->pos, DA_P0, fn);
//
//   //通信端口
//   Port = *frameData++;
//   //应答：终端通信端口号
//   pResp->frame[pResp->pos++] = Port;
//   //应答：转发目标地址
//   relay_depth = frameData[0];
//   mem_cpy(pResp->frame+pResp->pos,frameData+1+6*relay_depth,6);
//   mem_cpy(meterno,frameData+1+6*relay_depth,6);
//   pResp->pos += 6;
//   ctrl_flag=frameData[7+6*relay_depth];
//   pwd_len=frameData[8+6*relay_depth];
//   if(pwd_len>8)
//   {
//        pwd_len =8;
//   }
//   mem_cpy(meter_pwd,frameData+9+6*relay_depth,pwd_len);
//
//
//
//    wait_time= 60*100;//60秒,单位为10ms
////    resp_len =4;
//    rec_rsp =   pResp->frame+pResp->pos+6;
//    pResp->frame[pResp->pos++] =  TRANS_ERR;
//    pResp->frame[pResp->pos++] =  ctrl_flag;
//
//    cmd_len=make_gb645_2007_ctrl_frame(dl645frame,meterno,ctrl_flag,meter_pwd,pwd_len,NULL);
//
//    if(memory_fast_index_find_node_no(Port,meterno,&meter_idx,&rec_pos,&router_protocol,NULL))
//    {
//        meter_idx &= FAST_IDX_MASK;
//        if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
//        {
//            fread_meter_params(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
//            rec_resplen=app_trans_send_meter_frame(Port,NULL,&meter_doc,dl645frame,cmd_len,rec_rsp,300,wait_time);
//            if(rec_resplen>10)
//            {
//                pResp->frame[pResp->pos-2]=(rec_rsp[POS_GB645_CTRL] & 0x40) ? TRANS_RECV_NAK : TRANS_RECV_ACK;
//                #ifdef __YUNNAN_METER_CONTROL__ //云南电表拉合闸
//                if(pResp->frame[pResp->pos-2] == TRANS_RECV_NAK )//如果回复了否认，就要更换协议
//                {
//                 cmd_len= make_gb645_2007_ctrl_frame_via_write(dl645frame,meterno,ctrl_flag,meter_pwd,pwd_len,0);  //云南新规约电表跳闸
//                 rec_resplen=app_trans_send_meter_frame(Port,NULL,&meter_doc,dl645frame,cmd_len,rec_rsp,300,wait_time);
//                   if(rec_resplen>10)
//                   {
//                        pResp->frame[pResp->pos-2]=(rec_rsp[POS_GB645_CTRL] & 0x40) ? TRANS_RECV_NAK : TRANS_RECV_ACK;
//
//                        if(pResp->frame[pResp->pos-2] == TRANS_RECV_NAK)//如果回复了否认，就要更换协议
//                        {
//                          cmd_len= make_gb645_2007_ctrl_frame_via_write(dl645frame,meterno,ctrl_flag,meter_pwd,pwd_len,1);
//                          rec_resplen=app_trans_send_meter_frame(Port,NULL,&meter_doc,dl645frame,cmd_len,rec_rsp,300,wait_time);
//                            if(rec_resplen>10)
//                            {
//                             pResp->frame[pResp->pos-2]=(rec_rsp[POS_GB645_CTRL] & 0x40) ? TRANS_RECV_NAK : TRANS_RECV_ACK;
//                            }
//                            else
//                            {
//                            pResp->frame[pResp->pos-2] =  TRANS_TIMEOUT;
//                            }
//                        }
//                   }
//                   else
//                   {
//                    pResp->frame[pResp->pos-2] =  TRANS_TIMEOUT;
//                   }
//                }
//                #endif
//            }
//            else
//            {
//                 pResp->frame[pResp->pos-2] =  TRANS_TIMEOUT;
//            }
//        }
//    }
//    //发送响应祯，结束=TRUE
//    app_send_response_frame(pRequest,pResp,TRUE);
//
//}

BOOLEAN check_cmd_is_3761(INT8U *frame)
{
	INT16U i,frameLen=0;
	INT8U cs=0;
	if(frame[0]!= 0x68 || frame[5]!= 0x68)return FALSE;
	if(frame[1]!= frame[3] || frame[2]!= frame[4])return FALSE;
	//计算报文长度
	frameLen = frame[2];
	frameLen <<= 8;
	frameLen += frame[1]; 
	frameLen >>=2;	  
	frameLen+=8;  //报文总长度

	//验证长度
	if(frameLen>MAX_SIZE_PLMSDTP)return FALSE;
	//验证cs
	for(i=6;i<frameLen-2;i++)
	   cs+=frame[i];
	if(cs!=frame[frameLen-2])return FALSE;
	return TRUE;
}
/*+++
  功能： 检查报文是否是标准的645报文
  参数：
         INT8U   *frame   命令报文
  返回：
          TRUE / FALSE

---*/
BOOLEAN check_cmd_is_GB645(INT8U *frame,BOOLEAN is_update_cy_meterNo_cs)
{
   INT16U idx,i;
   BOOLEAN head_ok;
   INT8U len,cs;
   head_ok=FALSE;
   cs=0;
   for(idx=0;idx<5;idx++)
   {
      if( (frame[idx]==0x68)  && (frame[idx+7] == 0x68))
	  {
	  	head_ok=TRUE;
            if(is_update_cy_meterNo_cs)
            {
                mem_set(frame+idx+1,6,0xAA);
            }
		break;
   }
   }
   if(FALSE==head_ok)return FALSE;
   //验证长度
   len=frame[idx+9];
   if(0x16!=frame[idx+11+len])return FALSE;
   //验证CS
    for(i=0;i<len+10;i++) cs+=frame[idx+i];
    if(is_update_cy_meterNo_cs) frame[idx+10+len] = cs; 
   if(cs==frame[idx+10+len]) return TRUE;
   return FALSE;
}

INT8U check_cmd_is_645_cast(INT8U *frame,INT16U *pos)
{
   INT16U idx;
   BOOLEAN head_ok;
   
   head_ok=FALSE;
    *pos = 0;
   for(idx=0;idx<5;idx++)
   {
      if( (frame[idx]==0x68)  && (frame[idx+7] == 0x68))
	  {
	  	head_ok=TRUE;
		break;
        }
   }
    if(FALSE==head_ok) return 0;
    *pos = idx;
   frame += idx;
   for(idx=1;idx<=6;idx++)
   {
        if(frame[idx]!= 0x99) return 0;
   }
    
    #ifdef __ROADLAMP_CTRL__
    return frame[9] + 12;
    #endif

    if(frame[8] == 0x08)
    {
        if(frame[9] != 6 && frame[9] != 14) return 0;
        return frame[9] + 12;
    }
    else if(frame[8] == 0x1E)
    {
     return frame[9] + 12;
    }
    else if(frame[8] == 0x16)
   {
        if(frame[9] != 4) return FALSE;
        if(check_is_all_ch(frame+10,4,0xEE) || check_is_all_ch(frame+10,4,0xFF))
       {
            return frame[9] + 12;
       }
   }

    return 0;
}

/*+++
  功能：计算报文的校验和
  参数：
        INT8U *frame   645报文，无前导字节
  返回:
        无
  描述：
        68 XX XX XX XX XX XX 68 C L .... CS 16
        0  1  2  3  4  5  6  7  8 9

---*/
void  GB645_compute_checksum(INT8U *frame)
{
   INT16U idx;
   INT8U cs;

   cs = 0;
   for(idx=0;idx<frame[9]+10;idx++) cs += frame[idx];
   frame[frame[9]+10] = cs;
   frame[frame[9]+11] = 0x16;
}

//#ifndef __POWER_CTRL__
/*+++
  功能：通过路由板执行标准645报文的载波监控
  参数：
         INT8U *frame  645报文
         INT8U *via_meter_no   参考表地址，有效时不能从报文中提取地址
  返回：
         响应长度

  描述：
        1）报文前面的0xFF去掉
        2) 分为2007和1997规约两种,需要分别对待。
        3) 检查电表是否为采集器下所属电表，如果是，则需要按照采集器的模式执行。
        4）电表档案没有的，按照载波电表对待。
        5) 68 XX XX XX XX XX XX 68 C L
           0  1  2   3  4  5  6  7 8 9
        6) 因为是路由板，所以先检测电表档案，

---*/
//TODO需要考虑采集器下电表
//INT16U plc_readRelay_GB645(INT8U *frame,INT16U cmd_len,INT8U *resp,INT8U max_resp_len,INT16U max_wait_time)
//{
////     INT32U time_start;
//     INT16U meter_idx;//,pos,resp_len,save_pos;
////     INT16U wait_time;
//     INT16U rec_pos;
//     METER_DOCUMENT meter_doc;
//     INT8U meter_no[6];//,rtu_no[6];
//     INT8U router_protocol;  //采集器标志
//
////     pos = 1;
//
//     //去掉报文的前导词
//     while(*frame != 0x68)
//     {
//        frame++;
//        cmd_len--;
//     }
//
//	 //如果表号为AABBCDDEEFF，则为通过645报文转发376.1报文
//	 if(0==compare_string(frame+1,"\xFF\xEE\xDD\xCC\xBB\xAA",6))
//	 {
//
//		//return process_relay_3761(frame+10,resp,channel);
//
//	 }
//
//    mem_cpy(meter_no,frame+1,6);
//    if(memory_fast_index_find_node_no(COMMPORT_PLC,meter_no,&meter_idx,&rec_pos,&router_protocol,NULL))
//    {
//        meter_idx &= FAST_IDX_MASK;
//        if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
//        {
//            fread_meter_params(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
//            return app_trans_send_meter_frame(COMMPORT_PLC,NULL,&meter_doc,frame,cmd_len,resp,max_resp_len,max_wait_time);
//        }
//
//
//    }
//    return 0;
//
//}
//#endif //#ifndef __POWER_CTRL__


#ifdef __ENABLE_ESAM2__
/*+++
  功能：对批量电表下发任务命令
  参数：
        PLMS_RESP *Reply 应答帧工作区
        INT8U *frameData 命令帧的数据单元域，从数据标识PNFN之后开始
  返回：
        无
  描述：

---*/
BOOLEAN process_readRelay_F12(objRequest  *pRequest,objResponse *pResp)
{
    INT8U *frameData;
    INT32U offset;
    INT16U len,file_id,meter_idx,rec_pos,idx;
    INT16U meter_count;
//    INT8U  *rec_rsp,rec_resplen,rec_result;
    INT8U  router_protocol,port,task_type;
//    INT8U  relay_depth;
//    INT8U  resp_len;
    INT8U  meter_no[8]={0};
    union{
        BATCH_TASK_HEADER task_header;
        BATCH_TASK_AUTH task_auth;
        BATCH_TASK_TIME task_time;
    }var;
//    BOOLEAN has_task;

    port = COMMPORT_PLC;
    frameData =pRequest->frame+pRequest->pos;
    if(frameData[2] == 0) file_id = FILEID_METER_BATCH_TASK_0;
    else if (frameData[2] == 1) file_id = FILEID_METER_BATCH_TASK_1;
    else return FALSE;

    file_delete(file_id);  //先删除任务文件，不在叠加任务。

    //先读出电表数量
    fread_array(file_id,0,var.task_header.value,sizeof(BATCH_TASK_HEADER));
    meter_count = bin2_int16u(var.task_header.meter_count);
    if(meter_count > MAX_METER_COUNT) meter_count = 0;

    mem_set(var.task_header.value,sizeof(BATCH_TASK_HEADER),0xFF);
    mem_cpy_reverse(var.task_header.task_format,frameData,2);
    frameData += 2;
    var.task_header.task_type = frameData[0];
    frameData ++;
    mem_cpy(var.task_header.task_data_len,frameData,2);
    frameData += 2;
    len = bin2_int16u(var.task_header.task_data_len);
    if(len > 256)  //目前任务数据最大定义了256
    {
        return FALSE;
    }
    mem_cpy_reverse(var.task_header.task_data,frameData,len);
    frameData += len;

//    has_task = FALSE;
           
    var.task_header.create_date[0] = datetime[YEAR];
    var.task_header.create_date[1] = datetime[MONTH];
    var.task_header.create_date[2] = datetime[DAY];
           
    //电表数量
    mem_cpy(var.task_header.meter_count,frameData,2);
    len = bin2_int16u(frameData);
    frameData += 2;
    if(var.task_header.task_type == 0) //身份认证
    {
        if((len == 1) && (frameData[0] == 0xAA) && (frameData[1] == 0xAA)
            && (frameData[2] == 0xAA) && (frameData[3] == 0xAA)
            && (frameData[4] == 0xAA) && (frameData[5] == 0xAA)
            && (frameData[6] == 0xAA) && (frameData[7] == 0xAA))
        {
            //全部电表时，数量设置为FFFF
            mem_set(var.task_header.meter_count,2,0xFF);
            
            var.task_header.exec_type = 2;
            frameData += 8;
            mem_cpy_reverse(var.task_header.common_er,frameData,32);
            frameData += 32;
            file_delete(file_id);  //删除任务文件
//            has_task = TRUE;
        }
        else
        {
            var.task_header.exec_type = 1;
            meter_count += len;
            var.task_header.meter_count[0] = meter_count;
            var.task_header.meter_count[1] = meter_count>>8;
        }
    }
    else
    {
        meter_count += len;
        var.task_header.meter_count[0] = meter_count;
        var.task_header.meter_count[1] = meter_count>>8;
    }
    //写任务头
    fwrite_array(file_id,0,var.task_header.value,sizeof(BATCH_TASK_HEADER));
    if (var.task_header.exec_type == 2)
    {
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_esam,COMMPORT_485_REC);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_esam,COMMPORT_485_CAS);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_esam,COMMPORT_PLC);
        set_batch_meter_task_redo(port);
    }

    //处理电表数据
    task_type = var.task_header.task_type;
    if((var.task_header.task_type == 1) || (var.task_header.exec_type == 1))
    {
        for(idx=0;idx<len;idx++)
        {
            mem_set(var.task_auth.value,sizeof(BATCH_TASK_AUTH),0xFF);
            var.task_auth.task_state = 0xAA;
            mem_cpy_reverse(var.task_auth.meter_no,frameData,8);
            mem_cpy_reverse(var.task_auth.meter_er,frameData+8,32);
            mem_cpy_reverse(meter_no,var.task_auth.meter_no,8);
            //TODO需要修改该函数，不传递端口
            if(memory_fast_index_find_node_no_no_port(meter_no,&meter_idx,&rec_pos,&router_protocol,&port,NULL) == TRUE)
            {
                meter_idx &= FAST_IDX_MASK;
                if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;
                
                //has_task = TRUE;
                set_batch_meter_task_redo(port);
                offset = (task_type == 0) ? sizeof(BATCH_TASK_AUTH) : sizeof(BATCH_TASK_TIME);
                offset *= meter_idx;
                offset += sizeof(BATCH_TASK_HEADER);
                fwrite_array(file_id,offset,var.task_auth.value,(task_type == 0) ? sizeof(BATCH_TASK_AUTH) : sizeof(BATCH_TASK_TIME));
                set_bit_value(read_meter_flag_esam,READ_FLAG_BYTE_NUM,meter_idx);
            }
            frameData += 40;
        }
    }
    return TRUE;
}
/*+++
  功能：查询对批量电表下发任务的执行状态
  参数：
        PLMS_RESP *Reply 应答帧工作区
        INT8U *frameData 命令帧的数据单元域，从数据标识PNFN之后开始
  返回：
        无
  描述：

---*/
//void process_readRelay_F13(objRequest  *pRequest,objResponse *pResp)
//{
//    INT8U *frameData;
//    INT32U offset;
//    INT16U file_id,meter_idx,meter_count_pos;
//    INT16U count,meter_count;
//    INT8U exec_type;
//    union{
//        BATCH_TASK_HEADER task_header;
//        BATCH_TASK_AUTH task_auth;
//        BATCH_TASK_TIME task_time;
//    }var;
//    METER_DOCUMENT meter_doc;
//
//    frameData =pRequest->frame+pRequest->pos;
//
//    /*if(read_fmByte(FMADDR_METER_SET_CLOCK_FALG) == 0xAA) //禁止
//    {
//        set_pn_fn(resp->buffer, &pos, DA_P0, DT_F13);
//        resp->buffer[pos++] = frameData[0];
//        resp->buffer[pos++] = 0;
//        resp->buffer[pos++] = 0;
//        resp->buffer[pos++] = 0;
//        resp->buffer[pos++] = 0;
//        SetFrameLenThenSendFrame(resp,pos);
//        return;
//    }*/
//
//    if(frameData[0] == 0) file_id = FILEID_METER_BATCH_TASK_0;
//    else if(frameData[0] == 1) file_id = FILEID_METER_BATCH_TASK_1;
//    else
//    {
//        set_pn_fn(pResp->frame, &pResp->pos, DA_P0, DT_F13);
//        pResp->frame[pResp->pos++] = frameData[0];
//        pResp->frame[pResp->pos++] = 0;
//        pResp->frame[pResp->pos++] = 0;
//        pResp->frame[pResp->pos++] = 0;
//        pResp->frame[pResp->pos++] = 0;
//        app_send_response_frame(pRequest,pResp,TRUE);//发送响应祯，结束=TRUE
//        return;
//    }
//
//    fread_array(file_id,0,var.task_header.value,sizeof(BATCH_TASK_HEADER));
//    if(var.task_header.task_type != frameData[0])
//    {
//        set_pn_fn(pResp->frame, &pResp->pos, DA_P0, DT_F13);
//        pResp->frame[pResp->pos++] = frameData[0];
//        pResp->frame[pResp->pos++] = 0;
//        pResp->frame[pResp->pos++] = 0;
//        pResp->frame[pResp->pos++] = 0;
//        pResp->frame[pResp->pos++] = 0;
//        app_send_response_frame(pRequest,pResp,TRUE);//发送响应祯，结束=TRUE
//        return;
//    }
//
//    exec_type = var.task_header.exec_type;
//
//    //设置数据标识
//    set_pn_fn(pResp->frame, &pResp->pos, DA_P0, DT_F13);
//
//    pResp->frame[pResp->pos++] = frameData[0];
//    //统计总数
//    if((frameData[0] == 0) && (exec_type == 2))
//    {
//        count = 0;
//        for(meter_idx=1;meter_idx<=MAX_METER_COUNT;meter_idx++)
//        {
//            if(FALSE == file_exist(meter_idx) ) continue;
//            fread_array(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
//            if(meter_idx != bin2_int16u(meter_doc.meter_idx)) continue;
//            if(bin2_int16u(meter_doc.spot_idx) == 0) continue;
//            if(bin2_int16u(meter_doc.spot_idx) > MAX_METER_COUNT) continue;
//            if((frameData[0] == 0) && (exec_type == 2)) //全部电表
//            {
//                count++;
//            }
//        }
//    }
//    else
//    {
//        count = var.task_header.meter_count[0];
//        count += var.task_header.meter_count[1]<<8;
//    }
//
//    /*for(meter_idx=1;meter_idx<=MAX_METER_COUNT;meter_idx++)
//    {
//        if(FALSE == file_exist(meter_idx) ) continue;
//        fread_array(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
//        if(meter_idx != bin2_int16u(meter_doc.meter_idx)) continue;
//        if(bin2_int16u(meter_doc.spot_idx) == 0) continue;
//        if(bin2_int16u(meter_doc.spot_idx) > MAX_METER_COUNT) continue;
//        if((frameData[0] == 0) && (exec_type == 2)) //全部电表
//        {
//            count++;
//        }
//        else
//        {
//            offset = (frameData[0] == 0) ? sizeof(BATCH_TASK_AUTH) : sizeof(BATCH_TASK_TIME);
//            offset *= meter_idx;
//            offset += sizeof(BATCH_TASK_HEADER);
//            fread_array(file_id,offset,var.task_auth.value,(frameData[0] == 0) ? sizeof(BATCH_TASK_AUTH) : sizeof(BATCH_TASK_TIME));
//            if((var.task_auth.task_state == 0xAA) || (var.task_auth.task_state == 0x0A) || (var.task_auth.task_state == 0xA0))
//            {
//                count++;
//            }
//        }
//    }*/
//    
//    //批量执行对电表的任务中电表总数量
//    pResp->frame[pResp->pos++] = count;
//    pResp->frame[pResp->pos++] = count>>8;
//    //批量执行对电表的任务中已成功的电表数量
//    meter_count = 0;
//    meter_count_pos=pResp->pos;
//    pResp->frame[pResp->pos++] = meter_count;
//    pResp->frame[pResp->pos++] = meter_count>>8;
//
//    if(count > 0)
//    {
//        for(meter_idx=1;meter_idx<=MAX_METER_COUNT;meter_idx++)
//        {
//            if(FALSE == file_exist(meter_idx) ) continue;
//            fread_array(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
//            if(meter_idx != bin2_int16u(meter_doc.meter_idx)) continue;
//            if(bin2_int16u(meter_doc.spot_idx) == 0) continue;
//            if(bin2_int16u(meter_doc.spot_idx) > MAX_METER_COUNT) continue;
//
//            if((pResp->pos + 8) > (pResp->max_reply_pos - 20))
//            {
//                pResp->frame[meter_count_pos]  = meter_count;
//                pResp->frame[meter_count_pos+1]  = meter_count>>8;
//                (void)app_send_response_frame(pRequest,pResp,FALSE); //发送响应祯，结束=TRUE
//
//                pResp->pos = POS_DATA+9;
//                meter_count = 0;
//            }
//
//            offset = (frameData[0] == 0) ? sizeof(BATCH_TASK_AUTH) : sizeof(BATCH_TASK_TIME);
//            offset *= meter_idx;
//            offset += sizeof(BATCH_TASK_HEADER);
//            fread_array(file_id,offset,var.task_auth.value,(frameData[0] == 0) ? sizeof(BATCH_TASK_AUTH) : sizeof(BATCH_TASK_TIME));
//            if(var.task_auth.task_state == 0xA0)
//            {
//                meter_count++;     
//                mem_cpy(pResp->frame+pResp->pos,var.task_auth.meter_no,8);
//                pResp->pos += 8;
//            }
//        }
//    }
//    pResp->frame[meter_count_pos] = meter_count;
//    pResp->frame[meter_count_pos+1] = meter_count>>8;
//
//    app_send_response_frame(pRequest,pResp,TRUE);//发送响应祯，结束=TRUE
//}
#endif //#ifdef __ENABLE_ESAM2__

#ifdef __BATCH_TRANSPARENT_METER_TASK__
INT16U set_param_batch_transparent_meter_task(INT16U spot_idx,INT16U meter_seq,INT8U *params)
{
    BATCH_TRANSPARENT_METER_TASK_HEADER *header_param;
    CommandDate cmddate;
    INT16U pos,valid_minute;
    union{
        BATCH_TRANSPARENT_METER_TASK_HEADER header;
        BATCH_TRANSPARENT_METER_TASK frame;
    }var;
    INT8U idx,count_save,count_add,idx1,seq;
    BOOLEAN is_save,is_find;
    INT8U meter_no[6],node_idx;

    pos = 0;
    is_save = FALSE;
    count_save = 0;

    header_param = (BATCH_TRANSPARENT_METER_TASK_HEADER*)params;

    if (header_param->count > BATCH_TRANSPARENT_METER_TASK_FRAME_MAX_COUNT) return 0;

    tpos_mutexPend(&SIGNAL_BATCH_SET);

    fread_array(meter_seq,PIM_BATCH_TRANSPARENT_METER_TASK,var.header.value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER));
    if (compare_string(var.header.task_id,header_param->task_id,2) == 0)
    {
        count_save = (var.header.count > BATCH_TRANSPARENT_METER_TASK_FRAME_MAX_COUNT) ? 0 : var.header.count;
    }

    #ifdef __BATCH_TRANSPARENT_METER_TASK_DEBUG__
    OSMutexPend(&SIGNAL_RS232);
    INT8U debug_msg[50];
    mem_set(debug_msg,50,0);
    snprintf(debug_msg,50,"task_id1 = %02X%02X , task_id2 = %02X%02X ",var.header.task_id[0],var.header.task_id[1],header_param->task_id[0],header_param->task_id[1]);
    rs232_send_buffer(debug_msg,50);
    mem_set(debug_msg,50,0);
    snprintf(debug_msg,50,"meter_seq = %d , is_save = %d ",meter_seq,is_save);
    rs232_send_buffer(debug_msg,50);
    OSMutexFree(&SIGNAL_RS232);
    #endif    

    //任务标志写为无效
    if (header_param->count == 0)
    {
        var.header.flag == 0xFF;
        var.header.count == 0xFF;
        fwrite_array(meter_seq,PIM_BATCH_TRANSPARENT_METER_TASK,var.header.value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER));

        #ifdef __BATCH_TRANSPARENT_METER_TASK_DEBUG__
        OSMutexPend(&SIGNAL_RS232);
        rs232_send_buffer("clear_header",12);
        OSMutexFree(&SIGNAL_RS232);
        #endif
    }

    #ifdef __BATCH_TRANSPARENT_METER_TASK_TMP1__
    pos += 8;
    #else
    pos += 7;
    #endif

    for(idx=0;idx<header_param->count;idx++)
    {
        is_find = FALSE;
        for(idx1=0;idx1<count_save;idx1++)
        {
            fread_array(meter_seq,PIM_BATCH_TRANSPARENT_METER_TASK+sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER)+idx1*sizeof(BATCH_TRANSPARENT_METER_TASK)+1,&seq,1);
            if(params[pos] == seq)
            {
                is_find = TRUE;
                break;
            }
        }

        mem_set(var.frame.value,sizeof(BATCH_TRANSPARENT_METER_TASK),0xFF);

        if(params[pos+1] > BATCH_TRANSPARENT_METER_TASK_FRAME_LEN)
        {
            var.frame.flag = BATCH_TRANS_METER_TASK_FLAG_CANT_EXEC;
            var.frame.frame_seq = params[pos];
        }
        else
        {
            var.frame.flag = BATCH_TRANS_METER_TASK_FLAG_NO_EXEC;
            mem_cpy(&(var.frame.frame_seq),params+pos,params[pos+1]+2);
        }
        
        if ((!is_find) && (count_save < BATCH_TRANSPARENT_METER_TASK_FRAME_MAX_COUNT))
        {
            fwrite_array(meter_seq,PIM_BATCH_TRANSPARENT_METER_TASK+sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER)+count_save*sizeof(BATCH_TRANSPARENT_METER_TASK),var.frame.value,sizeof(BATCH_TRANSPARENT_METER_TASK));
            count_save++;
            is_save = TRUE;

            #ifdef __BATCH_TRANSPARENT_METER_TASK_DEBUG__
            OSMutexPend(&SIGNAL_RS232);
            rs232_send_buffer("write_frame",11);
            OSMutexFree(&SIGNAL_RS232);
            #endif
        }
        pos += params[pos+1]+2;
    }

    if(is_save)
    {
        #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
        set_bit_value(priority_node.map,256,meter_seq);
        #endif

        mem_set(header_param->begin_time,6,0xFF);
        #ifdef __BATCH_TRANSPARENT_METER_TASK_TMP1__
        valid_minute = bin2_int16u(header_param->vaild_minute);
        #else
        valid_minute = header_param->vaild_minute;
        #endif
        if (valid_minute > 0)
        {
            setCommandDate(&cmddate,(INT8U*)(datetime+1));
            while(valid_minute > 0)
            {
                if(valid_minute > 255)
                {
                    commandDateAddMinute(&cmddate,255);
                    valid_minute -= 255;
                }
                else
                {
                    commandDateAddMinute(&cmddate,valid_minute);
                    valid_minute -= valid_minute;
                }
            }
            assign_td_value(&cmddate,header_param->begin_time,5);
        }
        mem_set(header_param->end_time,6,0xFF);
        header_param->flag = 0xAA;
        header_param->count = count_save;
        fwrite_array(meter_seq,PIM_BATCH_TRANSPARENT_METER_TASK,header_param->value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER));

        #ifdef __BATCH_TRANSPARENT_METER_TASK_DEBUG__
        OSMutexPend(&SIGNAL_RS232);
        rs232_send_buffer("write_header",12);
        OSMutexFree(&SIGNAL_RS232);
        #endif

        if (header_param->task_level == 0)
        {
            set_bit_value(COST_CONTROL_FAIL_FLAG,256,meter_seq);
        }
        if (header_param->ctrl_flag.node_prior)
        {
            #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
            set_bit_value(priority_node.map,256,meter_seq);

            fread_array(FILEID_RUN_DATA,FMDATA_TRANS_TASK_PRIORITY_NODE+(meter_seq/8),&idx,1);
            idx &= ~(0x01<<(meter_seq%8));
            fwrite_array(FILEID_RUN_DATA,FMDATA_TRANS_TASK_PRIORITY_NODE+(meter_seq/8),&idx,1);

            if ((portContext_plc.batch_meter_ctrl.check_close_event_report == 0) && (portContext_plc.batch_meter_ctrl.ctrl_close_event_report == 0))
            {
                portContext_plc.batch_meter_ctrl.check_close_event_report = 1;
            }
            #endif
        }
        else
        {
            fread_array(FILEID_RUN_DATA,FMDATA_TRANS_TASK_PRIORITY_NODE+(meter_seq/8),&idx,1);
            idx |= (0x01<<(meter_seq%8));
            fwrite_array(FILEID_RUN_DATA,FMDATA_TRANS_TASK_PRIORITY_NODE+(meter_seq/8),&idx,1);
        }

        if ((spot_idx > 0) && (spot_idx <= MAX_METER_COUNT))
        {
            fread_array(FILEID_RUN_DATA,FMDATA_F305+((spot_idx-1)/8),&idx,1);
            idx &= ~(0x01<<((spot_idx-1)%8));
            fwrite_array(FILEID_RUN_DATA,FMDATA_F305+((spot_idx-1)/8),&idx,1);
        }

        if (portContext_plc.batch_meter_ctrl.is_set_router_fail == 0)
        {
            portContext_plc.batch_meter_ctrl.is_set_router_fail = 1;
            portContext_plc.batch_meter_ctrl.is_restart_read_meter = 1;
            portContext_plc.batch_meter_ctrl.is_not_check_meter_doc = 1;
            #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
            portContext_plc.batch_meter_ctrl.is_restart_node_yugao = 1;
            #endif
        }
    }
    portContext_plc.batch_meter_time_10ms = os_get_systick_10ms();

    tpos_mutexFree(&SIGNAL_BATCH_SET);

    return pos;
}

BOOLEAN query_param_batch_transparent_meter_task(INT16U spot_idx,objRequest *pRequest,objResponse *pResp,INT16U *from_seq,INT16U *req_bytes)
{
    INT32U offset;
    INT8U *Reply,*req;
    INT16U pos;
    INT16U  meter_idx;
    INT8U req_seq,req_seq_0;
    INT8U count,count_resp,count_save,idx,idx1,idx_start,idx1_start;
    INT8U   data[6];
    BATCH_TRANSPARENT_METER_TASK_HEADER header;

    Reply = pResp->frame+pResp->pos;
    req = pRequest->frame+pRequest->pos;
    #ifdef __BATCH_TRANSPARENT_METER_TASK_TMP1__
    mem_set(Reply,8,0x00);
    #else
    mem_set(Reply,7,0x00);
    #endif
    (*req_bytes) = req[0]+1;
    //query_ctrl->req_bytes = query_ctrl->req[0]+1;
    #ifdef __BATCH_TRANSPARENT_METER_TASK_TMP1__
    pos = 8;
    #else
    pos = 7;
    #endif
    count_resp = 0;
    req_seq_0 = 255;

    //get meter_seq
    meter_idx = meter_spot_2_meter_seq_map[spot_idx];
    //MASK 配置序号
    meter_idx &= 0x7FF;

    if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT) && (file_exist(meter_idx)))
    {
        fread_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK,header.value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER));
        count_save = header.count;
    }
    else header.count = 255;
    
    if (header.count > BATCH_TRANSPARENT_METER_TASK_FRAME_MAX_COUNT)
    {
        #ifdef __BATCH_TRANSPARENT_METER_TASK_TMP1__
        pResp->pos += 8;
        #else
        pResp->pos += 7;
        #endif
        return FALSE;
    }
    mem_cpy(Reply,header.task_id,7);
    #ifdef __BATCH_TRANSPARENT_METER_TASK_TMP1__
    Reply[6] = count_save;    //报文总数
    #else
    Reply[5] = count_save;    //报文总数
    #endif

    count = (req[0] == 0) ? 1 : req[0];
    idx_start = (req[0] == 0) ? 0 : (*from_seq);
    idx1_start = (req[0] == 0) ? (*from_seq) : 0;

    for(idx=idx_start;idx<count;idx++)
    {
        if(req[0] != 0)
        {
            req_seq = req[idx+1];
        }
        
        for(idx1=idx1_start;idx1<count_save;idx1++)
        {
            offset = PIM_BATCH_TRANSPARENT_METER_TASK+sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER)+idx1*sizeof(BATCH_TRANSPARENT_METER_TASK);
            fread_array(meter_idx,offset,data,2);
            if ((data[1] == req_seq) || (req[0] == 0))
            {
                req_seq = data[1];
                if ((pos + 2) > (pResp->max_reply_pos - pResp->pos))
                {
                    pResp->pos += (pos > 2) ? pos : 0;
    #ifdef __BATCH_TRANSPARENT_METER_TASK_TMP1__
                    Reply[7] = count_resp;
    #else
                    Reply[6] = count_resp;
    #endif
                    (*from_seq) = (req[0] == 0) ? idx1 : idx;
                    return TRUE;
                }

                switch(data[0])
                {
                case BATCH_TRANS_METER_TASK_FLAG_NO_EXEC:                 //有任务,未执行
                case BATCH_TRANS_METER_TASK_FLAG_CANT_EXEC:            //不能执行
                case BATCH_TRANS_METER_TASK_FLAG_BEFORE_EXCEPTION:     //前面任务回复异常应答
                case BATCH_TRANS_METER_TASK_FLAG_RESP_LONG:            //执行完成,回复报文太长
                case BATCH_TRANS_METER_TASK_FLAG_EXEC_NO_RESP:         //已执行,未响应
                case BATCH_TRANS_METER_TASK_FLAG_TIME_OUT:             //执行完成,超时
                case BATCH_TRANS_METER_TASK_FLAG_FINISH:                   //执行完成
                    Reply[pos++] = req_seq;
                    fread_array(meter_idx,offset+2,data,1);
                    if(data[0] > BATCH_TRANSPARENT_METER_TASK_FRAME_LEN) Reply[pos++] = 0;
                    else Reply[pos++] = data[0];
                    break;
                default:
                    continue;
                }
            
                count_resp++;

                if(Reply[pos-1] > 0)
                {
                    if ((pos + Reply[pos-1]) > (pResp->max_reply_pos - pResp->pos))
                    {
                        pResp->pos += ((pos-2) > 2) ? (pos-2) : 0;
    #ifdef __BATCH_TRANSPARENT_METER_TASK_TMP1__
                        Reply[7] = count_resp - 1;
    #else
                        Reply[6] = count_resp - 1;
    #endif
                        (*from_seq) = (req[0] == 0) ? idx1 : idx;
                        return TRUE;
                    }
                    else
                    {
                        fread_array(meter_idx,offset+3,Reply+pos,Reply[pos-1]);
                        pos += Reply[pos-1];
                    }
                }
                if (req[0] != 0) break;
            }
        }
    }

    pResp->pos += pos;
    #ifdef __BATCH_TRANSPARENT_METER_TASK_TMP1__
    Reply[7] = count_resp;
    #else
    Reply[6] = count_resp;
    #endif

    return FALSE;
}

void clear_all_batch_transparent_meter_task(INT16U pi)
{
    INT16U meter_idx,pn_idx;
    INT8U idx;
    INT8U buffer[256];
    BATCH_TRANSPARENT_METER_TASK_HEADER header;

    pn_idx = trans_set_pn_2_pnidx(pi);

    tpos_mutexPend(&SIGNAL_BATCH_SET);

    if (pn_idx == 0)
    {
        for(meter_idx=1;meter_idx<=MAX_METER_COUNT;meter_idx++)
        {
            if (file_exist(meter_idx))
            {
                fread_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK,header.value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER));
                if ((header.count > 0) && (header.count <= BATCH_TRANSPARENT_METER_TASK_FRAME_MAX_COUNT))
                {
                    mem_set(header.value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER),0xFF);
                    fwrite_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK,header.value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER));
                }
            }
        }

        mem_set(buffer,256,0xFF);
        fwrite_array(FILEID_RUN_DATA,FMDATA_F305,buffer,255);
        #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
        fwrite_array(FILEID_RUN_DATA,FMDATA_TRANS_TASK_PRIORITY_NODE,buffer,256);
        #endif
        
        portContext_plc.batch_meter_ctrl.is_set_router_fail = 0;
        portContext_plc.batch_meter_ctrl.is_restart_read_meter = 0;
        mem_set(COST_CONTROL_FAIL_FLAG,256,0x00);  //__BATCH_TRANSPARENT_METER_TASK_COST_CONTROL__
        #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
        mem_set(priority_node.value,sizeof(PRIORITY_NODE),0x00);
        #endif
    }
    else
    {
        //get meter_seq
        meter_idx = meter_spot_2_meter_seq_map[pn_idx];
        //MASK配置序号
        meter_idx &= 0x7FF;

        if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
        {
            fread_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK,header.value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER));
            if ((header.count > 0) && (header.count<= BATCH_TRANSPARENT_METER_TASK_FRAME_MAX_COUNT))
            {
                mem_set(header.value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER),0xFF);
                fwrite_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK,header.value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER));
            }
        
            if ((pn_idx > 0) && (pn_idx <= MAX_METER_COUNT))
            {
                fread_array(FILEID_RUN_DATA,FMDATA_F305+((pn_idx-1)/8),buffer,1);
                buffer[0] |= (0x01<<((pn_idx-1)%8));
                fwrite_array(FILEID_RUN_DATA,FMDATA_F305+((pn_idx-1)/8),buffer,1);
            }
            #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
            fread_array(FILEID_RUN_DATA,FMDATA_TRANS_TASK_PRIORITY_NODE+((meter_idx)/8),buffer,1);
            buffer[0] |= (0x01<<((meter_idx)%8));
            fwrite_array(FILEID_RUN_DATA,FMDATA_TRANS_TASK_PRIORITY_NODE+((meter_idx)/8),buffer,1);

            clr_bit_value(priority_node.map,256,meter_idx);
            #endif
        }
    }

    tpos_mutexFree(&SIGNAL_BATCH_SET);
}
#endif //#ifdef __BATCH_TRANSPARENT_METER_TASK__











