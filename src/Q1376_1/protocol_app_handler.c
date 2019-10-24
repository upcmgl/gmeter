///////////////////////////////////////////////////////////////////////////////
//
//   Q/GDW  1376.1 报文处理器  
//
///////////////////////////////////////////////////////////////////////////////


#include "../main_include.h"
#include "protocol_app_handler.h"
#include "app_3761.h"
#include "app_var_3761.h"
#include "ErtuParam.h"



INT8U  ACD;


   void  app_init_response(objRequest *pRequest,objResponse *pResp);
   void  process_frame_read(objRequest *pRequest,objResponse *pResp);
   void  process_frame_reset(objRequest *pRequest,objResponse *pResp);
   void  process_frame_readRelay(objRequest  *pRequest,objResponse *pResp);
   void  process_frame_readhis(objRequest *pRequest,objResponse *pResp);
   void  process_frame_set(objRequest *pRequest,objResponse *pResp);
   void  process_frame_query(objRequest *pRequest,objResponse *pResp);
   void  process_read_config_info(objRequest *pRequest,objResponse *pResp);
   void  process_vendor_debug(objRequest *pRequest,objResponse *pResp);
   void  process_test(objRequest *pRequest,objResponse *pResp);
   void  process_frame_ask_slave(objRequest *pRequest,objResponse *pResp);
   void  process_frame_ftp(objRequest *pRequest,objResponse *pResp);
   BOOLEAN process_remote_update_nm(objRequest *pRequest,objResponse *pResp);
   void  process_read_event(objRequest *pRequest,objResponse *pResp);
   void  process_frame_ctrl(objRequest *pRequest,objResponse *pResp);
   void  process_frame_ack(objRequest *pRequest,objResponse *pResp);
   void  process_frame_readTask(objRequest *pReqeust,objResponse *pResp);
   void  process_product_test(objRequest *pRequest,objResponse *pResp);
//void check_ertu_cascade(objRequest *pRequest,objResponse *pResp);
BOOLEAN check_ertu_address(objRequest *pRequest,objResponse *pResp);
BOOLEAN auth_verify_mac(INT8U *frame,objRequest  *pRequest,objResponse *pResp);
BOOLEAN check_is_cy_timing_frame(INT8U* frame,INT16U len);



/*+++
  功能：  判断是否已经接收到了报文头部
  备注：
          1)要求frame中不包含多余字节，即首字节必须符合标准。
---*/
void app_protocol_handler(objRequest* pRequest,objResponse *pResp)
{
void app_prepare_request(objRequest *pRequest);
char info[20];
//    if(get_system_flag(SYS_KEY_WAKEUP,SYS_FLAG_BASE))//按键唤醒时，如果有报文，则供电时间延长
//    {
//        gSystemCtrl.battery_start_time = system_get_tick10ms();
//    }
//    if((pRequest->frame[POS_AFN]!=AFN_FTP) && (pRequest->frame[POS_AFN]!=AFN_DEBUG))
//    {
//        if(is_debug_enabled()  && is_monitor_frame(MONTIOR_3761))
//        {
//            record_log_frame(pRequest->frame,pRequest->frame_len,LOG_DEBUG);
//        }
//    }

//预处理request对象
    app_prepare_request(pRequest);
    //初始化response对象
    app_init_response(pRequest,pResp);

    //检查下行报文的终端地址是否有效
#ifndef __ONENET__
    if(FALSE == check_ertu_address(pRequest,pResp))
    {

       return ;
    }
#endif    



    if(pRequest->userDataLen<4)
    {
        return ;//报文没有数据域，不合法，扔掉
    }
    switch(pRequest->frame[POS_AFN])
    {
    case  AFN_ACK:        //确认/否认报文
             process_frame_ack(pRequest,pResp);
             return;
    case  AFN_RESET:      //复位命令
             process_frame_reset(pRequest,pResp);
             break;
    case  AFN_RELAY:      //数据转发
             process_frame_readRelay(pRequest,pResp);
             break;

    case  AFN_READCUR:    //读当前数据
             process_frame_read(pRequest,pResp);
             break;

    case  AFN_READHIS:    //读历史数据
             process_frame_readhis(pRequest,pResp);
             break;
    case  AFN_SET:        //设置参数
             process_frame_set(pRequest,pResp);
             break;

    case  AFN_READEVENT:  //请求3类数据
//             process_read_event(pRequest,pResp);
             break;
    
    case  AFN_QUERY:      //查询参数
             process_frame_query(pRequest,pResp);
             break;
//    case  AFN_READTASK:  //请求任务数据
//             process_frame_readTask(pRequest,pResp);
//             break;
//    case  AFN_CTRL:     //控制命令
//             process_frame_ctrl(pRequest,pResp);
//             break;
//    #ifdef __ENABLE_ESAM2__
//    case  AFN_AUTH:      //密钥协商
//            #ifndef __SOFT_SIMULATOR__
//             process_frame_auth(pRequest,pResp);
//            #endif
//             break;
//    #endif
    case  AFN_READCFG:  //请求终端配置
             process_read_config_info(pRequest,pResp);
             break;   
        #ifndef __SOFT_SIMULATOR__
    case  AFN_FTP:   //更新终端程序
             process_frame_ftp(pRequest,pResp);
             break;
        #endif


    //TODO:继续处理该函数
//    case AFN_ASK_SLAVE: //请求被级联终端主动上报
//    case AFN_ASK_SLAVE_NEXT:
//             process_frame_ask_slave(pRequest,pResp);
//             break;
//    case AFN_DEBUG: //厂家调试
//             process_vendor_debug(pRequest,pResp);
//             break;
//    case AFN_TEST://链路接口检测
//            process_test(pRequest,pResp);
//            break;
//    case AFN_PRODUCT_TEST:
//            process_product_test(pRequest,pResp);
//            break;
    default:
             app_send_NAK(pRequest,pResp);
             break;
    }


    return ;
}
/*+++
  功能：预处理请求对象结构体
  参数：
  返回：

  描述：


---*/
void app_prepare_request(objRequest *pRequest)
{
    //提取用户有效数据区的地址和数据长度
    pRequest->pos = POS_DATA;
    if(pRequest->frame_len>16)
    {
        pRequest->userDataLen = pRequest->frame_len - 16; //考虑到应用层处理时的方便性，userDataLen仅包含应用层用户数据域
    }
    else
    {
        pRequest->userDataLen = 0;
    }
    #ifdef __RTUADDR_IS_6BYTE__
    pRequest->userDataLen -= 2;
    #endif
   //提取命令帧的序列域，用来判断报文是否有时间标签等信息
    if(pRequest->frame[POS_SEQ]>>7 )
    {

       //请求时标
       pRequest->param = & pRequest->frame[pRequest->frame_len - 8];

       //有效的用户数据长度减区时间标签
       if(pRequest->userDataLen > 6)
       {
           pRequest->userDataLen -= 6;
       }

    }
    else
    {
        pRequest->param = NULL;
    }
    pRequest->rseq =  pRequest->frame[POS_SEQ] & 0x0F;
}
/*+++
  功能：初始化响应结构
  参数：
  返回：
        INT8U *frameData  数据区头
  描述：
       1）设置响应头
       2）提取用户数据长度
       3）设置响应缓冲区长度

---*/
void app_init_response(objRequest *pRequest,objResponse *pResp)
{
    //INT8U get_cur_seq(void);
    void  app_set_max_response_length(objRequest *pRequest,objResponse *pResp);

    //提取命令帧的数据域前的字节为应答帧头
    mem_cpy(pResp->frame, pRequest->frame,POS_DATA);


    pResp->channel = pRequest->channel;
//    pResp->pChannel= pRequest->pChannel;
//    pResp->CallBack = pRequest->CallBack;
//    pResp->protocol_type = pRequest->protocol_type;
    //设置响应帧的最大可用位置
    app_set_max_response_length(pRequest,pResp);
    //响应报文的处理位置
    pResp->pos = POS_DATA;

    //提取命令帧的序列域，用来判断报文是否有时间标签等信息
    if(pRequest->frame[POS_SEQ]>>7 )
    {
       //有效用户数据长度减去时间标签长度
       pResp->max_reply_pos -= 6;
    }

    //清除广播标志
    pResp->work_flag[WORK_FLAG_CMD_FLAG] &= ~MSA_BROADCAST;

    //提取响应帧序号,作为响应帧的第一帧序号 
    pResp->work_flag[WORK_FLAG_SEQ_RSEQ] &= 0xF0;
    pResp->work_flag[WORK_FLAG_SEQ_RSEQ] += (pRequest->rseq++ & 0x0F);
    pResp->work_flag[WORK_FLAG_SEQ_RSEQ] &= ~MASK_CON;
    if(pResp->channel == CHANNEL_FLASH)
    {
        gActiveSendAck.check_seq = pResp->work_flag[WORK_FLAG_SEQ_RSEQ]&0x0F;
        gActiveSendAck.has_acked = FALSE;
        if(gActiveSendAck.need_check_ack)
        {
            pResp->work_flag[WORK_FLAG_SEQ_RSEQ] |= MASK_CON;
    }
    }

    //默认单帧
    pResp->work_flag[WORK_FLAG_IS_FIRST_FRAME] = TRUE;
    //设置控制域C的传输方向
    pResp->frame[POS_CTRL] |= CTRLFUNC_SET_DIR;
    pResp->frame[POS_CTRL] &= 0xF0;
    //清除控制域C的ACD位
    pResp->frame[POS_CTRL] &= CTRLFUNC_CLR_ACD;
    //若是主动上报，则不清除控制域的PRM
    if(pResp->channel != CHANNEL_FLASH)
    {
    pResp->frame[POS_CTRL] &= CTRLFUNC_CLR_PRM;
    pResp->frame[POS_SEQ] &= ~0x10;   //去掉CON
    }

    /*
    #ifndef __PROVICE_SHANGHAI__
    if(ACD  &&  (system_flag & SYS_FRAME_ACD_FLAG) )
    {
       pResp->frame[POS_CTRL] |=  CTRLFUNC_SET_ACD;
       //有效用户数据长度减去事件计数器
       PResp->max_reply_pos -= 2;
    }
    #endif
    */



    //根据AFN设置响应帧功能码，需要分启动站和从动站，单帧响应的可以在响应时处理
//    if(pResp->frame[POS_CTRL] & CTRLFUNC_SET_PRM)
//    {
//        
//    }
//    else
    {
          if((pRequest->frame[POS_CTRL] & 0x0F) == PRM_CTRLFUNC_TEST)
          {
               pResp->frame[POS_CTRL] |= CTRLFUNC_TEST;
          }
          else if(((pRequest->frame[POS_CTRL] & 0x0F) == PRM_CTRLFUNC_CURDATA)|| ((pRequest->frame[POS_CTRL] & 0x0F) == PRM_CTRLFUNC_HISDATA))
          {
               pResp->frame[POS_CTRL] |= CTRLFUNC_USERDATA;
          }
          else if((pRequest->frame[POS_CTRL] & 0x0F) ==PRM_CTRLFUNC_RESET)
          {
               pResp->frame[POS_CTRL] |= CTRLFUNC_CONFIRM;
          }
          else
          {
               if(pResp->channel != CHANNEL_FLASH)
               {
                    pResp->frame[POS_CTRL] |= CTRLFUNC_USERDATA;
               }
               else
               {
                    pResp->frame[POS_CTRL] |= PRM_CTRLFUNC_USERDATA;
               }
          }

    }

}

/*+++
  功能： 设置响应报文允许的最大长度
  备注： 该接口可以被覆盖.
---*/

void  app_set_max_response_length(objRequest *pRequest,objResponse *pResp)
{
    pResp->max_reply_pos = SIZE_OF_APP_PROTOCOL_FRAME - 2;
//    if(pResp->channel == CHANNEL_INFRA  )
//    {
//         pResp->max_reply_pos = 300;//如果是红外口，则定义报文最大长度为300字节
//    }
//    else
    {
//        if(!get_system_flag(SYS_ESAM,SYS_FLAG_BASE))
        {
            pResp->max_reply_pos = 916;//远程口，应答缓冲区最大支持916字节
        }
//        else if(pRequest->frame[POS_AFN]==AFN_AUTH)//在开启ESAM时只有身份认证报文使用全部缓冲区  单相G表是否需要身份认证  __GMETER__
//        {
//    	    pResp->max_reply_pos = MAX_SIZE_PLMSDTP - 2;
//        }
    }

    #if defined(STACK_USE_SMS)
    if(pResp->channel==CHANNEL_SMS)
    {
	if(REMOTE_SMS.smsframetype==1)
	pResp->max_reply_pos = 138;
    }
    #endif
	//浙江，任务上报时如果，网络不通则生成短信报文
   	#ifdef  __SMS_REPORT__
        if(1!=g_eth_conn_flag && !REMOTE_GPRS.tcp_ok && CHANNEL_FLASH==resp->channel)
		{
	    		resp->max_reply_pos = 138;
		}
	#endif



	//如果是UDP则，限制包的大小为540字节，2011.3.31
     /*
     if((pResp->channel==CHANNEL_REMOTE) && (REMOTE_SMS.gprs_mode==1) )
	{
	    	pResp->max_reply_pos = 540;
	}

 	//初始化响应通道为1，2011.5.17 by zyg
	if(1==REMOTE_SMS.work_mode)//客户端模式
    	resp->channel_idx=1;
	else if(2==REMOTE_SMS.work_mode)//服务器模式
    	resp->channel_idx=2;
    //混合模式，不初始化，由发送者设置

    //2011.3.16 by zyg

            */
#ifdef __USE_NB_COAP__
    if(pResp->channel == PROTOCOL_CHANNEL_REMOTE  )
    {
        pResp->max_reply_pos = SIZE_OF_APP_PROTOCOL_FRAME_COAP - 2;
    }
#endif
}



/*+++
   功能：计算校验位，设置控制域
   参数：INT8U *frame      [IN]   帧
        INT16U *framelen   [OUT]   帧总长度
---*/
void app_encodeFrame(objResponse *pResp)
{
    INT16U i;
    INT16U frameLen;
    INT8U cs;


    //确保帧帧起始符号正确
    pResp->frame[0]=0x68;
    pResp->frame[5]=0x68;

    //提取用户数据区长度
    frameLen = bin2_int16u(pResp->frame+1);
    frameLen >>=2;

    //计算校验位
    cs=0;
    frameLen += 6;   //加上固定长度的报文头
    for(i=6;i<frameLen;i++)  cs += pResp->frame[i];
    pResp->frame[frameLen++]=cs;

    //设置结束字符
    pResp->frame[frameLen++]=0x16;

    pResp->frame_len = frameLen;

}

/*+++
 功能： 发送响应数据帧
 参数： INT8U *Reply 响应帧,没有给定数据长度，附加域
       int pos              响应帧最后设置的数据位置

 备注：根据下行报文的时间标签以及当前ACD标志可能会增加附加域（EC,时间标签）
---*/
void app_setFrameLenThenSendFrame(objRequest  *pRequest,objResponse *pResp)
{
   //INT8U get_cur_seq(void);

   BOOLEAN allow_acd = TRUE;

      //如果是上海，则仅在心跳时，附带ACD位，其他不能带
   #ifndef __PROVICE_SHANGHAI__
   if(AFN_TEST == pResp->frame[POS_AFN])
   {
            allow_acd = FALSE;
       }
   #endif

   if(allow_acd==FALSE)
   {
      //2010-6-21  某些主站不能识别心跳报文的ACD，所以这里去掉！！(例如协同的主站)
      pResp->frame[POS_CTRL] &= ~CTRLFUNC_SET_ACD;
      pResp->frame[POS_SEQ]  &= ~MASK_TpV;
   }
   else
   {
     //1. 根据ACD值检查是否需要设置EC
//      #ifdef __PROVICE_SHANGHAI__
//      if(pResp->frame[POS_CTRL] & CTRLFUNC_SET_ACD)
//      #else
//      //if(ACD && (system_flag & SYS_FRAME_ACD_FLAG) )
//      if(g_event.acd)
//      #endif
//      {
//        pResp->frame[POS_CTRL] |= CTRLFUNC_SET_ACD;
//        //EC
//        //重要事件计数器EC1
//        pResp->frame[pResp->pos++] = g_event.ec1;
//
//        //一般事件计数器EC2
//        pResp->frame[pResp->pos++] = g_event.ec2;
//      }

     //2. 根据CMD_TPV
     if(pRequest->param)
     {
        pResp->work_flag[WORK_FLAG_SEQ_RSEQ] |= 0x80;  //set bit7=1
        mem_cpy(pResp->frame + pResp->pos,pRequest->param,6);
        pResp->pos+=6;
     }
   }


   //3. 设置应答帧中的数据域长度

   //3.1 减去固定长度帧报文头
   pResp->pos -= 6;

   //3.2 数据长度左移2位
   pResp->pos<<=2;

   //3.3  规约类型，按照命令的规约类型回答
//   #ifdef __QGXD_FK2012__
//   pRequest->protocol_type =QGXD_FK2012;
//   #elif defined  __QNMDW_FK2012__
//   pRequest->protocol_type = QNMDW_FK2012;
//   #elif defined __QGDW_FK2005__
//   pRequest->protocol_type = QGDW_2005;
//   #else
//   pRequest->protocol_type = QGDW_376_1;
//   #endif

   //pResp->pos += pRequest->protocol_type;
   pResp->pos += QGDW_376_1;

   //3.4 设置长度
   pResp->frame[1] =  pResp->pos;
   pResp->frame[2] =  pResp->pos >> 8;
   pResp->frame[3] =  pResp->frame[1];
   pResp->frame[4] =  pResp->frame[2];

   //4 设置响应帧序号
   pResp->frame[POS_SEQ] = pResp->work_flag[WORK_FLAG_SEQ_RSEQ];

   //5 响应帧序号加1,供多帧时发送使用
   pResp->work_flag[WORK_FLAG_SEQ_RSEQ] = (pResp->work_flag[WORK_FLAG_SEQ_RSEQ] +1 ) & 0x0F;
   //若是主动上报的分帧，且不是分帧的最后一帧(FIN=0)，则做全局帧序号的处理
   if((pResp->channel == CHANNEL_FLASH) && ((pResp->frame[POS_SEQ] & MASK_FIN) == 0x00))
   {
       //get_cur_seq();
   }
   #ifdef __PROVICE_TIANJIN__
   if((pResp->frame[POS_AFN] == AFN_QUERY) && (0 == bin2_int16u(pResp->frame + POS_DATA))
             && (DT_F89 == bin2_int16u(pResp->frame + POS_DATA + 2))&& (pResp->channel == CHANNEL_RS232))
    {
    }
    else
    #endif
    {
   //设置终端逻辑地址：行政区划码+终端地址
        mem_cpy(pResp->frame+POS_RT,gSystemInfo.ertu_devid,CFG_DEVID_LEN);
    }

   // 计算校验位
   app_encodeFrame(pResp);

   if(pResp->channel==CHANNEL_NONE)
   {

     return;
   }
    if((pRequest->frame[POS_AFN]!=AFN_FTP) && (pRequest->frame[POS_AFN]!=AFN_DEBUG))
    {
//        if(is_debug_enabled() && is_monitor_frame(MONTIOR_3761))
//        {
//            record_log_frame(pResp->frame,pResp->frame_len,LOG_DEBUG);
//        }
    }

   //6 发送响应帧
   app_send_ReplyFrame(pResp);

}


/*+++
  描述：  设置首祯，结束祯标志
          然后发送
  变量：
        SEQ_RSEQ
  返回：
         TRUE  继续处理，
         FALSE 停止处理
  
---*/
BOOLEAN app_send_response_frame(objRequest  *pRequest,objResponse *pResp,INT8U last_frame)
{
    //设置首帧、结束帧标志
    pResp->work_flag[WORK_FLAG_SEQ_RSEQ] &= 0x9F;  //clear bit5,6
    
    //设置结束帧标志位 
    if(last_frame)  pResp->work_flag[WORK_FLAG_SEQ_RSEQ] |=  MASK_FIN;
    
    if( pResp->work_flag[WORK_FLAG_IS_FIRST_FRAME] )
    {
        pResp->work_flag[WORK_FLAG_SEQ_RSEQ] |=  MASK_FIR;
        pResp->work_flag[WORK_FLAG_IS_FIRST_FRAME] = 0;
    }

     //发送响应对象
    app_setFrameLenThenSendFrame(pRequest,pResp);
    
    return TRUE;
}

/*+++
  功能： 发送响应对象。
  备注：
         1）响应对象在发送完成后，需要调用release释放。
         2）如果通道是虚拟的，则直接调用release.


---*/
void app_active_send_complete_callback(void)
{
    gActiveSendAck.has_send = TRUE;
    if(gActiveSendAck.need_check_ack == FALSE)
    {
        gActiveSendAck.has_acked = TRUE;
    }
    else
    {
        gActiveSendAck.last_send_timer = system_get_tick10ms();//这里更新一下发送时间吧，也可以不更新，可以商量
    }
}



/*++++
  功能： 发送全部确认帧
  参数：
---*/
void app_send_ACK(objRequest  *pRequest,objResponse *pResp)
{
    // clear prm =0
    // 否认：无所召唤的数据。
    pResp->frame[POS_CTRL] &= 0xF0;
    if((pRequest->frame[POS_CTRL] & 0x0F) ==PRM_CTRLFUNC_RESET)
    {
        pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_CONFIRM;
    }
    else
    {
        pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_USERDATA;
    }
    pResp->frame[POS_AFN] = AFN_ACK;
    pResp->work_flag[WORK_FLAG_SEQ_RSEQ]  |=  MASK_FIR|MASK_FIN;

    // PN = P0
    pResp->frame[POS_DATA] = 0x00;
    pResp->frame[POS_DATA+1] = 0x00;

    // FN = DT_F1  全部确认
    pResp->frame[POS_DATA+2] = DT_F1 & 0xFF;
    pResp->frame[POS_DATA+3] = DT_F1 >> 8;

    pResp->pos = POS_DATA+4;

    app_setFrameLenThenSendFrame(pRequest,pResp);
}

/*++++
  功能： 发送全部否认帧
  参数： GUN.SET
---*/
void  app_send_NAK(objRequest  *pRequest,objResponse *pResp)
{

    if(pRequest->channel == CHANNEL_FLASH)
    {
        //pResp->release(pResp);
        return;
    }
    // clear prm =0
    // 否认：无所召唤的数据。
    //pResp->frame[POS_CTRL] &= 0xF0;
    pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_NODATA; // 全部否认功能码为9  无所召唤的数据

    pResp->frame[POS_AFN] = AFN_ACK;
    pResp->work_flag[WORK_FLAG_SEQ_RSEQ]  |=  MASK_FIR|MASK_FIN;

    // PN = P0
    pResp->frame[POS_DATA] = 0x00;
    pResp->frame[POS_DATA+1] = 0x00;

    // FN = DT_F2  全部否认
    pResp->frame[POS_DATA+2] = DT_F2 & 0xFF;
    pResp->frame[POS_DATA+3] = DT_F2 >> 8;

    pResp->pos = POS_DATA+4;

    app_setFrameLenThenSendFrame(pRequest,pResp);
}


/*+++
   功能:将集合的信息类标示转换到标示数组中
    GUN.SET.fn  --> GUN.SET.fngroup
---*/
INT16U parse_fn_to_fngroup(INT16U fn,INT16U fngroup[8])
{
   INT8U ik,mask;
   INT16U fngroupcount;

   if(fn==DT_F0)
   {
       //fngroupcount=1;
       fngroup[0]=DT_F0;
       return 1; 
   }

   fngroupcount=0;
   for(ik=0;ik<8;ik++)
   {
      fngroup[ik]=0;
      mask = 1<<ik;
      if(fn & mask)
      {
         fngroup[fngroupcount++] = fn & (0xFF00|mask);
      } 
   }
   
   return fngroupcount;
}


/*+++
   功能:将集合的信息点标示转换到标示数组中
   参数：INT16U pn  原组合PN
         INT16U pngroup[8]  分解后的PN
   返回：分解数量
---*/
INT16U parse_pn_to_pngroup(INT16U pn,INT16U pngroup[8])
{
   INT8U ik,mask;
   INT16U pngroupcount;

   if(pn==DA_P0)
   {
       pngroup[0] = DA_P0;
       return 1;
   }
   pngroupcount=0;
   for(ik=0;ik<8;ik++)
   {
      pngroup[ik]=0;
      mask = 1<<ik;
      if(pn & mask)
      {
         pngroup[pngroupcount++] = pn & (0xFF00|mask);
      } 
   }
   return pngroupcount;
}



//把fn  转换为GUN.SET.fn_idx
//  转换后： DT_F1 == 1
//          DT_F2 == 2
//          DT_Fn == n
INT16U trans_set_fn_2_fnidx(INT16U fn)
{
    INT16U fn_idx;
    INT8U fn_low;


    fn_idx = fn>>8;
    fn_idx <<=3; //乘以8
    fn_low = fn & 0xFF;

    do
    {
      fn_idx ++;
      fn_low >>=1;
   }while(fn_low > 0);

   return fn_idx;
}



/*+++
 功能： 把PN  转换为pn_idx
 参数：
        INT16U pn  信息点，不能表示一组信息点，只能是单一的信息点。
 返回：
        INT16U  信息点序号，表示测量点号或总加组号
 描述:
         DA_P0==0
         DA_P1==1
         DA_Pn==n
---*/
INT16U  trans_set_pn_2_pnidx(INT16U pn)
{
    INT16U pn_idx;

    ClrTaskWdt();

    pn_idx = pn>>8;
    if(pn_idx == 0) return 0;


    //减1乘8
    pn_idx = (pn_idx-1)<<3;

    switch(pn & 0xFF)
    {
    case 0x01:  pn_idx+=1; break;
    case 0x02:  pn_idx+=2; break;
    case 0x04:  pn_idx+=3; break;
    case 0x08:  pn_idx+=4; break;
    case 0x10:  pn_idx+=5; break;
    case 0x20:  pn_idx+=6; break;
    case 0x40:  pn_idx+=7; break;
    case 0x80:  pn_idx+=8; break;
    default:
                return 0;
    }
    return pn_idx;    
}


/*+++
  功能：把测量点号PN转换为电表配置序号meter_seq
  参数：
        INT16U pn  测量点号
        BOOLEAN    *flag485  是否是485电表
  返回：
        电表序号，  错误返回0
---*/
INT16U  trans_pn_2_meter_seq(INT16U pn)
{
   INT16U spot_idx;

   //测量点
   spot_idx = trans_set_pn_2_pnidx(pn);


   //return trans_spot_idx_2_meter_idx(spot_idx);
   return 1;

}
        
/*+++
  功能：设置数据单元标识
  参数：
        Reply,&GUN.RELAY.pos,GUN.RELAY.pn,GUN.RELAY.fn
---*/
void  set_pn_fn(INT8U *Reply,INT16U *pos,INT16U pn,INT16U fn)
{
    Reply[(*pos)++]  = pn;
    Reply[(*pos)++]  = pn>>8;
    Reply[(*pos)++]  = fn;
    Reply[(*pos)++]  = fn>>8;
}

/*+++
  功能：设置数据单元标识,FK2005规约格式
  参数：
        Reply,&GUN.RELAY.pos,GUN.RELAY.pn,GUN.RELAY.fn
---*/
void  set_pn_fn_2005(INT8U *Reply,INT16U *pos,INT16U pn,INT16U fn)
{
    INT16U pn_tmp;
    
    pn_tmp = pn & 0xFF00;
    pn = pn & 0x00FF;
    
    switch(pn_tmp)
    {
        case 0x0100:  pn |= 0x0100; break;
        case 0x0200:  pn |= 0x0200; break;
        case 0x0300:  pn |= 0x0400; break;
        case 0x0400:  pn |= 0x0800; break;
        case 0x0500:  pn |= 0x1000; break;
        case 0x0600:  pn |= 0x2000; break;
        case 0x0700:  pn |= 0x4000; break;
        case 0x0800:  pn |= 0x8000; break;
        default:
            break;
    }
    
    Reply[(*pos)++]  = pn;
    Reply[(*pos)++]  = pn>>8;
    Reply[(*pos)++]  = fn;
    Reply[(*pos)++]  = fn>>8;
}

/*+++
  功能：根据FN的序号获取历史数据属性
---*/
INT8U  get_fnprop_from_fnidx(INT16U fn_idx)
{
    //F1~F8
    if(fn_idx <=8)  return PN_METER|LOCK_DAY;

    //F9~F12
    if(fn_idx <=12) return PN_METER|LOCK_METERDAY;

    //F13~F16
    if(fn_idx <17)  return 0;

    //F17~F24
    if(fn_idx <=24) return PN_METER|LOCK_MONTH;

    //F25~F32
    if(fn_idx <=32) return PN_METER|LOCK_DAY;

    //F33~F39
    if(fn_idx <=39) return  PN_METER|LOCK_MONTH;

    //F40
    if(fn_idx == 40)  return 0;


    //F41~F43
    if(fn_idx <=43) return  PN_METER|LOCK_DAY;

    //F44
    if(fn_idx == 44)  return PN_METER|LOCK_MONTH;;

    //F45~F48
    if(fn_idx <49)  return 0;

    //F49~F50
    if(fn_idx <51)  return PN_RTUA|LOCK_DAY;

    //F51~F52
    if(fn_idx <53)  return PN_RTUA|LOCK_MONTH;

    //F53
    if(fn_idx <54)  return PN_RTUA|LOCK_DAY;

    //F54
    if(fn_idx <55)  return PN_RTUA|LOCK_MONTH;

    //F55~F56
    if(fn_idx <57)  return 0;

    //F57~F59
    if(fn_idx <60)  return PN_AGT|LOCK_DAY;

    //F60~F62
    if(fn_idx <63)  return PN_AGT|LOCK_MONTH;

    //F63~F64
    if(fn_idx <65)  return 0;

    if(fn_idx<=112) return PN_METER|LOCK_CURVE;

    if(fn_idx<=128) return PN_METER|LOCK_DAY;

    if(fn_idx<=129) return PN_METER|LOCK_DAY;
    if(fn_idx<=130) return PN_METER|LOCK_MONTH;

    if(fn_idx <136) return  0;

    if(fn_idx<=148) return PN_METER|LOCK_CURVE;

    if(fn_idx <161) return  0;

    //F161~F168
    if(fn_idx <169) return PN_METER|LOCK_DAY;

    //F169~F176
    if(fn_idx <177) return PN_METER|LOCK_METERDAY;

    //F177~F184
    if(fn_idx <=184) return PN_METER|LOCK_MONTH;

    //F185~F188
    if(fn_idx <=188) return PN_METER|LOCK_DAY;

    //F189~F192
    if(fn_idx <=192) return PN_METER|LOCK_METERDAY;

    //F193~F196
    if(fn_idx <=196) return PN_METER|LOCK_MONTH;

    //F197~F200
    if(fn_idx <=200) return  0;

    //F201~F208
    if(fn_idx <=208) return PN_METER|LOCK_MONTH;

    //F209
    if(fn_idx <=209) return PN_METER|LOCK_DAY;

    //F210~F212
    if(fn_idx <=212) return 0;

    //F213~F216
    if(fn_idx <=216) return PN_METER|LOCK_MONTH;

    //F217	台区集中抄表载波主节点白噪声曲线	p0	Td_c	曲线
    //F218	台区集中抄表载波主节点色噪声曲线	p0	Td_c	曲线
    
    if(fn_idx <=218) return PN_RTUA | LOCK_CURVE;
    
    if(fn_idx <=220) return PN_METER | LOCK_CURVE;
    if(fn_idx <=221) return PN_METER | LOCK_DAY;
    if(fn_idx <=222) return PN_METER | LOCK_MONTH;

	#ifdef __METER_DAY_FREEZE_EVENT__ // 230~234 为冻结
	if(fn_idx <=234) return PN_METER | LOCK_DAY;
	#else
    if(fn_idx <=226) return PN_METER | LOCK_CURVE;

    if(fn_idx <=231) return PN_METER | LOCK_CURVE;
	#endif
    #ifdef __COUNTRY_ISRAEL__
    if(fn_idx == 237) return PN_METER | LOCK_DAY;
    #endif


    return 0;
}

/*+++
  功能： 检查命令帧的信息摘要
  备注：
---*/
INT8U check_frame_pw(objRequest *pRequest)
{

   return TRUE;  
}

/*+++
   功能：复位命令  F1,F2,F3-P0
   参数:
       objRequest *pRequest 请求帧工作区
       objResponse *pResp 应答帧工作区

   下行报文：
           数据单元标识(DA=0)
           PW
           TP
   上行报文：
           确认/否认报文

---*/
void  process_frame_reset(objRequest *pRequest,objResponse *pResp)
{
    INT16U pn,fn;
    INT16U reset_cmd;
//    INT16U pos;

    ClrTaskWdt();



     //数据单元标识
    pn = bin2_int16u(pRequest->frame +  pRequest->pos);
    fn = bin2_int16u(pRequest->frame +  pRequest->pos + 2);

 	#ifdef __PROVICE_CHONGQING__
	if(get_system_flag(SYS_RUN_STATUS,SYS_FLAG_BASE) )
	{
		app_send_NAK(pRequest,pResp);
        return;
	}
	#endif
//    reset_cmd=0xFF;

    if((pn != DA_P0) || (fn > DT_F4))
    {
        app_send_NAK(pRequest,pResp);
        return;
    }

    reset_cmd = fn;


    app_send_ACK(pRequest,pResp);

    if(reset_cmd!=DT_F1)
    {
        //app_begin_reset();
        //switch(reset_cmd)
        //{
            //case DT_F2:
                //app_data_reset();
                //break;
            //case DT_F3:
                //before_app_reset();
                //app_param_reset_default();
                //break;
            //case DT_F4:	
                //before_app_reset();
                //app_param_reset();
                //break;
        //}
        //DelayNmSec(5000);
        //ndfat_release_lock_file_system();
        //app_softReset();
        //app_end_reset();
		app_softReset();
    }
    else
    {
            //record_log_code(LOG_SYS_INIT_RESET,NULL,0,LOG_ALL);//初始化软件复位
            #ifndef __PROVICE_HUNAN__
            DelayNmSec(5000);
            app_softReset();
            #else
            app_softReset();
            #endif
    }
    //if need reset ertu, then delay 5s.
         //需要执行复位操作,根据当前ACD状态，设置软复位标志字。
//         system_flag |= SYS_PRG_RESET;
//         if(gprs_tcp_is_connected())
//         {
//             gprs_client_tcp_disconnect();
//             DelayNmSec(1000);
//         }
//         //延时，等待通信模块发送报文到主站
//         DelayNmSec(3000);
//         record_sys_operate(OP_SYS_INIT_RESET);//初始化复位
//
//

}


/*+++

  功能：处理主终端的巡检
  描述：
      1）从终端有上报数据，则上报，否则
      2）主终端否定请求被级联终端主动上报报文
      3）浙江扩展和原376.1，都需要先调用发送上报报文.
---*/
void process_frame_ask_slave(objRequest *pRequest,objResponse *pResp)
{
    INT16U pos,fn;
    #ifdef __PROVICE_ZHEJIANG_TODO__
    INT8U pn;
    #endif

    #ifdef __PROVICE_ZHEJIANG_TODO__
    pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_USERDATA;
    pResp->work_flag[WORK_FLAG_SEQ_RSEQ] |= MASK_FIR | MASK_FIN;
    pResp->work_flag[WORK_FLAG_SEQ_RSEQ] &= ~MASK_CON;
    pos=POS_DATA;
    // 1、主终端发AFN=18 F1来查询从终端是否有数据上报。
    // 2、从终端应该回复AFN=18 F1  AA：无数据 55：有数据
    // 3、如果从终端有数据（返回F1： 55），则通过AFN=18 F2请求从终端数据。
    // 4、从终端上报一帧数据。
    // 5、重复步骤1，直到上报从终端F1：AA
    if(AFN_ASK_SLAVE_NEXT == pRequest->frame[POS_AFN])
    {
        //数据单元标识
        pn = bin2_int16u(pRequest->frame + pRequest->pos);
        fn = bin2_int16u(pRequest->frame + pRequest->pos + 2);
        if(DT_F1 == fn)
        {
            set_pn_fn(pResp->frame, &pos, pn, fn);
            pResp->frame[pos++] =  send_report_frames(TRUE,FALSE) ?  0x55 : 0xAA;
            pResp->pos = pos;
            app_setFrameLenThenSendFrame(pRequest,pResp);
        }
        else
        {
            //如果有上报报文，则报告后返回,否则，根据请求类别进行应答。
            send_report_frames(TRUE,TRUE);
        }
        return;
    }  
    #endif
   
    //如果有上报报文，则报告后返回,否则，根据请求类别进行应答。
    //if(send_report_frames(TRUE,TRUE)) return;


    //应答：
    //设置控制码：DIR=1,PRM=0, FUNC=8 响应报文，用户数据
    pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_USERDATA;
    pResp->work_flag[WORK_FLAG_SEQ_RSEQ] |= MASK_FIR | MASK_FIN;
    pResp->work_flag[WORK_FLAG_SEQ_RSEQ] &= ~MASK_CON;
    pos=POS_DATA;

   // if(AFN_ASK_SLAVE_NEXT == frame[POS_AFN])
   // {
        
  //  }
  //  else
  //  {
       pResp->frame[POS_AFN] = AFN_ACK;
       fn = DT_F2; //全部否认
       set_pn_fn(pResp->frame, &pos, DA_P0, fn);
   // }
    pResp->pos = pos;
    app_setFrameLenThenSendFrame(pRequest,pResp);
}


/*+++

  功能： 厂家调试
  参数：
  返回：
  描述：
      1)F1: 读取文件数据   PN=P0，数据单元内容：文件号(INT16U), 起始偏移（32位地址，低字节在前），数据长度（INT16U）
      2)F2：初始化命令 PN=P0,数据单元内容：类别：1-系统日志
      3)F3:写命令    PN=P0，数据单元内容：文件号(INT16U), 起始偏移（32位地址，低字节在前），数据长度（INT16U）
---*/
void  process_vendor_debug(objRequest *pRequest,objResponse *pResp)
{
    INT16U pi,fi;
    INT8U ret=0;


    //is_first_frame = TRUE;
    pResp->work_flag[WORK_FLAG_IS_FIRST_FRAME] = TRUE;
	//设置控制码：DIR=1,PRM=0, FUNC=8  响应报文，用户数据
	pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_USERDATA;

    //提取数据单元标识
    pi = bin2_int16u(pRequest->frame +  pRequest->pos);
    fi = bin2_int16u(pRequest->frame +  pRequest->pos + 2);
    pRequest->pos += 4;
    set_pn_fn(pResp->frame, &pResp->pos, pi, fi);
    switch(fi)
    {
    case DT_F1:  //读取文件数据
          //pResp->pos += cmd_read_system_data(pRequest->frame + pRequest->pos,pResp->frame + pResp->pos);
	 	  if((pRequest->frame[pRequest->pos + 1] == 2) || (pRequest->frame[pRequest->pos + 1] == 3))
		  {
            app_send_ACK(pRequest,pResp);
			return;
		 }
         break;
    case DT_F2:
        //ret=cmd_extend_system_data(pRequest->frame + pRequest->pos);
        if(ret == 0)
        {
            app_send_ACK(pRequest,pResp);
        }
        else
        {
            app_send_NAK(pRequest,pResp);
        }
        return;
    case DT_F3:
        //ret=cmd_write_system_data(pRequest->frame + pRequest->pos);
        if(ret > 0)
        {
            app_send_ACK(pRequest,pResp);
        }
        else
        {
            app_send_NAK(pRequest,pResp);
        }
        return;
    default:
         app_send_NAK(pRequest,pResp);
         return;
    }

    app_send_response_frame(pRequest,pResp,TRUE);
}



/*+++

  功能： 链路接口检测
  参数：
  返回：
  描述：
      1)F3: 目前仅处理心跳
---*/
void  process_test(objRequest *pRequest,objResponse *pResp)
{
    INT16U pos;
    INT16U pi,fi;
//    BOOLEAN is_first_frame;

    pos = POS_DATA;

    //is_first_frame = TRUE;
    pResp->work_flag[WORK_FLAG_IS_FIRST_FRAME] = TRUE;

    //提取数据单元标识
    pi = bin2_int16u(pRequest->frame +  pRequest->pos);
    fi = bin2_int16u(pRequest->frame +  pRequest->pos + 2);
    pRequest->pos += 4;
    set_pn_fn(pResp->frame, &pos, pi, fi);
    switch(fi)
    {
    case DT_F3:  //心跳
        app_send_ACK(pRequest,pResp);
        return;
    default:
        app_send_NAK(pRequest,pResp);
        return;
    }
}

/*+++

  功能： 生产检测扩展
  参数：
  返回：
  描述：
 *    1)F1~F8：液晶测试
 *    2)F9~F16：指示灯测试(II型集中器)
---*/
void process_product_test(objRequest *pRequest,objResponse *pResp)
{
    
    INT16U pi,fi;


    pResp->pos = POS_DATA;

    //is_first_frame = TRUE;
    pResp->work_flag[WORK_FLAG_IS_FIRST_FRAME] = TRUE;
    //设置控制码：DIR=1,PRM=0, FUNC=8  响应报文，用户数据
	pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_USERDATA;

    //提取数据单元标识
    pi = bin2_int16u(pRequest->frame +  pRequest->pos);
    fi = bin2_int16u(pRequest->frame +  pRequest->pos + 2);
    pRequest->pos += 4;
    set_pn_fn(pResp->frame, &pResp->pos, pi, fi);
    switch(fi)
    {
    case DT_F9:  //指示灯控制
         //暂预留了一个字节用于后续可能的扩展
         pRequest->pos ++;
         app_send_ACK(pRequest,pResp);
         #if defined (__SGRID_HARDWARE_II__)|| defined(__NGRID_HARDWARE_II__)
         output_led_test_status();
         #endif
         return;
    default:
         app_send_NAK(pRequest,pResp);
         return;
    }
}
//void  app_send_active_test(bool login,objResponse *pResp)
//{
//   //INT8U get_cur_seq(void);
//
//   BOOLEAN allow_acd = TRUE;
//   INT8U idx;
//   INT8U cur_seq;
//   INT16U fn;
//
//   fn = (login==true)?DT_F3:DT_F1;
//   
//    pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_SET_PRM |PRM_CTRLFUNC_TEST;
//
//    pResp->frame[POS_AFN] = AFN_TEST;
//    cur_seq = 1;//get_cur_seq();
//    pResp->work_flag[WORK_FLAG_SEQ_RSEQ]  =  MASK_FIR|MASK_FIN|MASK_CON | cur_seq;
//
//    // PN = P0
//    pResp->frame[POS_DATA] = 0x00;
//    pResp->frame[POS_DATA+1] = 0x00;
//
//    //FN
//    pResp->frame[POS_DATA+2] = fn & 0xFF;
//    pResp->frame[POS_DATA+3] = fn >> 8;
//
//    pResp->pos = POS_DATA+4;
//    #ifdef __QNMDW_FK2012__
//    for(idx=0;idx<6;idx++)
//    {
//        pResp->frame[pResp->pos] = byte2BCD( datetime[idx] );
//        if(idx == 4)
//        {
//            pResp->frame[pResp->pos] |= (datetime[WEEKDAY] << 5);   //星期放在月的高3位
//        }
//        pResp->pos++;
//    }
//    for(idx=0;idx<4;idx++)
//    {
//        pResp->frame[pResp->pos] = 0xFF;
//        pResp->pos++;
//    }
//    #elif ((defined __YUNNAN_FK__) || (defined __QGDW_376_2009_PROTOCOL__)) //标签用__QGDW_FK2005__有重要冲突，应该是和FK2012冲突，未查找
//    {
//
//    }
//    #else
////    if(fn == DT_F3)
////    {
////       for(idx=0;idx<6;idx++)
////       {
////          pResp->frame[pResp->pos]= byte2BCD( datetime[idx] );
////          if(idx == 4)
////          {
////            pResp->frame[pResp->pos] |= (datetime[WEEKDAY] << 5);  //星期放在月的高3位
////          }
////          pResp->pos ++;
////       }
////    }
//    #endif
//
//      //如果是上海，则仅在心跳时，附带ACD位，其他不能带
//   #ifndef __PROVICE_SHANGHAI__
//   if(AFN_TEST == pResp->frame[POS_AFN])
//   {
//            allow_acd = FALSE;
//   }
//   #endif
//
//   if(allow_acd==FALSE)
//   {
//      //2010-6-21  某些主站不能识别心跳报文的ACD，所以这里去掉！！(例如协同的主站)
//      pResp->frame[POS_CTRL] &= ~CTRLFUNC_SET_ACD;
//      pResp->frame[POS_SEQ]  &= ~MASK_TpV;
//   }
//   else
//   {
//     //1. 根据ACD值检查是否需要设置EC
////      #ifdef __PROVICE_SHANGHAI__
////      if(pResp->frame[POS_CTRL] & CTRLFUNC_SET_ACD)
////      #else
//      //if(ACD && (system_flag & SYS_FRAME_ACD_FLAG) )
////      if(g_event.acd)
////      #endif
////      {
////        pResp->frame[POS_CTRL] |= CTRLFUNC_SET_ACD;
////        //EC
////        //重要事件计数器EC1
////        pResp->frame[pResp->pos++] = g_event.ec1;
////
////        //一般事件计数器EC2
////        pResp->frame[pResp->pos++] = g_event.ec2;
////      }
//
//   }
//
//   //3.1 减去固定长度帧报文头
//   pResp->pos -= 6;
//
//   //3.2 数据长度左移2位
//   pResp->pos<<=2;
//
//   //3.3  规约类型，按照命令的规约类型回答
//   #ifdef __QGXD_FK2012__
//   pResp->protocol_type =QGXD_FK2012;
//   #elif defined  __QNMDW_FK2012__
//   pResp->protocol_type = QNMDW_FK2012;
//   #elif defined __QGDW_FK2005__
//   pResp->protocol_type = QGDW_2005;
//   #else
//   //pResp->protocol_type = QGDW_376_1;
//   #endif
//   pResp->pos += QGDW_376_1;
//   //3.4 设置长度
//   pResp->frame[1] =  pResp->pos;
//   pResp->frame[2] =  pResp->pos >> 8;
//   pResp->frame[3] =  pResp->frame[1];
//   pResp->frame[4] =  pResp->frame[2];
//
//   //4 设置响应帧序号（和任务上报共用一个帧序号）
//   pResp->frame[POS_SEQ] = pResp->work_flag[WORK_FLAG_SEQ_RSEQ];
//
//
//   //设置终端逻辑地址：行政区划码+终端地址
//   mem_cpy(pResp->frame+POS_RT,gSystemInfo.ertu_devid,CFG_DEVID_LEN);
//   pResp->frame[POS_MSA] = 0;
//
//   // 计算校验位
//   app_encodeFrame(pResp);
//
//   if(pResp->channel==CHANNEL_NONE)
//   {
//
//     return;
//   }
//   //6 发送响应帧
//   if(!login)
//   {
//       //app_send_ReplyFrame_urgent(pResp);
//   }
//   else
//   {
//       app_send_ReplyFrame(pResp);
//   }
//}

/*+++
  功能：检查下行报文的终端地址是否具备响应条件
  参数：
        INT8U *frame,         命令报文
        PLMS_RESP *resp       响应器
  返回：
         TRUE / FALSE
  描述：
        1）与通讯模块之间通讯时，终端地址为5个0x00

---*/
BOOLEAN check_ertu_address(objRequest *pRequest,objResponse *pResp)
{
//    INT32U  eeAddr;
    INT32U rtuid;
    INT16U  group_id=0;
    INT8U devid[6]={0};

    pResp->work_flag[WORK_FLAG_IS_CAST] = 0;
    pResp->work_flag[WORK_FLAG_CMD_FLAG] = 0;

    //首先检查行政区划代码A1是否正确，否则不予响应
    mem_cpy(devid,gSystemInfo.ertu_devid,CFG_DEVID_LEN);

    // 如果终端地址不符，检查是否为行政区划的广播地址
    if( (devid[0] != pRequest->frame[POS_RT]) || (devid[1] != pRequest->frame[POS_RT+1]) )
    {
        #ifdef __PROVICE_HEBEI__
        //河北广播地址0x22114433
        if(((pRequest->frame[POS_RT] == 0xFF) && (pRequest->frame[POS_RT+1] == 0xFF)) ||
           ((pRequest->frame[POS_RT] == 0x11) && (pRequest->frame[POS_RT+1] == 0x22)))
        #else
         //山东地区广播地址0x9999FFFF 
        if(((pRequest->frame[POS_RT] == 0xFF) && (pRequest->frame[POS_RT+1] == 0xFF)) ||
           ((pRequest->frame[POS_RT] == 0x99) && (pRequest->frame[POS_RT+1] == 0x99)))
        #endif    
            pResp->work_flag[WORK_FLAG_CMD_FLAG] |= MSA_BROADCAST;
        else
            return FALSE;
    }

    //然后检查终端地址域是否正确，否则不予响应
    #ifdef __RTUADDR_IS_6BYTE__
    rtuid = bin2_int32u(pRequest->frame+POS_UID);
    #else
    rtuid = bin2_int16u(pRequest->frame+POS_UID);
    #endif
    #ifndef __PROVICE_JIANGSU__
    if(rtuid==0)
    {
        #ifndef __PROVICE_JIANGXI__
        return FALSE;   //无效地址
        #endif
    }
    #endif
    if(pRequest->frame[POS_MSA] & 0x01)
    {
        //组地址
#ifdef __POWER_CTRL__
        //组地址的话需要检查是否在剔除状态
        if(gPowerCtrl.CtrlSetStatus.specStatus & 0x02)
        {
           if((pRequest->frame[POS_AFN] != AFN_CTRL)
               ||(pRequest->frame[POS_DATA]!= 0x00)||(pRequest->frame[POS_DATA+1]!= 0x00)
               ||(pRequest->frame[POS_DATA+2]!= 0x40)||(pRequest->frame[POS_DATA+3]!= 0x03)               )
           {
                return FALSE;
           }
        }
#endif
        //return FALSE;
    #if defined (__RTUADDR_IS_6BYTE__)
        if(rtuid == 0xFFFFFFFF)
    #elif defined (__PROVICE_HEBEI__) 
        if((rtuid == 0xFFFF)||(rtuid == 0x4433))     //广播地址
    #else
        if(rtuid == 0xFFFF)     //广播地址
    #endif
        {
            pResp->work_flag[WORK_FLAG_IS_CAST] = 1;
            return TRUE;
        }

//        fread_ertu_params(EEADDR_SET_F6,var.value,16);
//        for(idx=0;idx<8;idx++)
//        {
//            group_id = var.F6[idx][1]*0x100+var.F6[idx][0];
//            if(group_id == 0) continue; //组地址为0，不是组地址
//            if(group_id == rtuid) break;
//        }
        if(group_id == rtuid)
        {
            pResp->work_flag[WORK_FLAG_IS_CAST] = 1;
            return TRUE;
        }
        else
        {
            return FALSE;   //无效地址
        }
 /*
        if(CMD_BROADCAST) return FALSE;

        eeAddr = EEADDR_F6;
        for(idx=0;idx<8;idx++)
        {
            read_fmArray(eeAddr,(INT8U *)&group_id,2);
            if(group_id == 0) continue; //组地址为0，不是组地址
            if(group_id == rtuid) break;
        }
        if(group_id != rtuid) return FALSE;
  */
    }
    else 
    {
        #if defined (__RTUADDR_IS_6BYTE__)
        if( (pResp->work_flag[WORK_FLAG_CMD_FLAG] & MSA_BROADCAST)  && (rtuid != 0xFFFFFFFF) ) return FALSE;
        #elif defined (__PROVICE_HEBEI__)
        if( (pResp->work_flag[WORK_FLAG_CMD_FLAG] & MSA_BROADCAST)  && ((rtuid != 0xFFFF)&&(rtuid != 0x4433)) ) return FALSE;
        #else
        if( (pResp->work_flag[WORK_FLAG_CMD_FLAG] & MSA_BROADCAST)  && (rtuid != 0xFFFF) ) return FALSE;
        #endif
        //单地址 
        if(compare_string(devid+2,pRequest->frame+POS_UID,CFG_DEVID_LEN-2) != 0)
        //if( (devid[2] != frame[POS_UID]) || (devid[3] != frame[POS_UID+1]) )
        {
             //最后检查是否是广播地址
            #if defined (__RTUADDR_IS_6BYTE__)
            if(rtuid != 0xFFFFFFFF)  return FALSE;
            #elif defined (__PROVICE_HEBEI__)
            if((rtuid != 0xFFFF)&&(rtuid != 0x4433))  return FALSE;
            #else
             if(rtuid != 0xFFFF)  return FALSE;
            #endif

        }
    }
    return TRUE;
}

/*+++
   功能：确认/否认报文
   参数:
       INT8U *Reply 应答帧工作区
       INT16U frameLen  命令帧的数据单元域长度
       INT8U *frameData 命令帧的数据单元域

   下行报文：
           数据单元标识(DA=0)
           PW
           TP
---*/
void process_frame_ack(objRequest  *pRequest,objResponse *pResp)
{
    if((gActiveSendAck.need_check_ack == TRUE)&&(gActiveSendAck.check_seq == (pResp->frame[POS_SEQ] & 0x0F)))
    {
        #ifdef __ENABLE_4852_ERC14_REPORT__
        if(pRequest->channel == CHANNEL_485_2)
    	{
                g_erc14_report = 0;
                Channel_rs485_2.dir = 0;//接收态
    	}
        #endif
        gActiveSendAck.has_acked = TRUE;
    }

    //如果是链路状态响应,则也不能处理
    //if( (resp->buffer[POS_CTRL] & 0x0F) == CTRLFUNC_TEST) return;

    //TODO:掉电上报状态检查
//    if(system_flag & SYS_POWER_LOST)
//    {
//         //上报掉电事件标志位
//        if(system_flag & SYS_POWER_LOST_RPT)
//        {
//           //检查序号是否相符。
//           if(RPT_POWER_LOST.b.rtp_seq == pResp->frame[POS_SEQ])
//           {
//              system_flag &= ~SYS_POWER_LOST_RPT;   //清除上报标志位.
//           }
//        }
//    }

    //上报补抄日冻结等待确认帧
    #ifdef __PATCH_REC_DH__
    if(report_patch.wait_confirm.is_wait_confirm == 1)
    {
        if(report_patch.seqtime.seq == (pResp->frame[POS_SEQ] & 0x0F))
        {
            report_patch.wait_confirm.is_wait_confirm = 0;
            report_patch.wait_confirm.is_confirm = 1;
        }
    }
    #endif

    return;
}

/*
从所有有效测量点中获取下一个测量点
参数1：原始DA标识
参数2，目的DA标识
参数3，所有测量点中所属第几组
返回值：TRUE-表示当前存在有效DA,FALSE-标识当前不存在有效DA，不需要再调用该函数进行获取
*/
BOOLEAN get_next_pn(INT16U pn,INT16U *pn_new,INT16U *pn_idx)
{
   INT8U val;

   if(*pn_idx>=MAX_METER_COUNT)return FALSE;
   if((pn!=0x00FF) && (pn!=0xFFFF))//不是所有测量点，则不需要处理
   {
       *pn_new=pn;
       *pn_idx=MAX_METER_COUNT;
       return TRUE;
   }
   //处理思路，一次读取8个测量点（1个字节），如果该字节存在有效测量点则，返回该组测量点，否则继续读取，直到读完
   while(*pn_idx<MAX_METER_COUNT)
   {
      (*pn_idx)++;
      val=meter_exist((*pn_idx));
      if(val==0)continue;//没有有效测量点，继续读取

       //转换按位标识测量点为实际DA
       val=0;
       set_bit_value(&val,1,(*pn_idx-1)%8);
       *pn_new=((INT8U)((*pn_idx-1)/8+1))<<8;
       *pn_new|=val;
       return TRUE;
    }

    return FALSE;
}

/*+++
  功能：测量点是否有效
  参数:
        INT16U spot_idx  测量点号
  返回:
        查询结果

  描述:

---*/
BOOLEAN meter_exist(INT16U spot_idx)
{
  //INT16U meter_idx;

  //meter_idx = meter_spot_2_meter_seq_map[spot_idx];
  //if(meter_idx & 0x8000)
  {
    return TRUE;
  }
  //return FALSE;
}

uint16_t app_make_active_test_frame(uint8_t *frame,uint8_t is_first)
{
    INT8U get_cur_seq(void);
    tagDatetime datetime;
    INT8U cur_seq;
    INT16U fn;
    INT16U pos;
    INT8U     protocol_type;
    INT16U frameLen;
    INT16U i;
    INT8U cs;
    
    os_get_datetime(&datetime);
	#ifdef __HEBEI_HUANBAO__
	    frame[0]=0xFF;
	    mem_cpy_reverse(frame+1,gmeter_doc.meter_no,6);
	    frameLen = 7;
	    return frameLen;
	#else
		fn = is_first?DT_F1:DT_F3;
   
		frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_SET_PRM |PRM_CTRLFUNC_TEST;

		frame[POS_AFN] = AFN_TEST;
		// PN = P0
		frame[POS_DATA] = 0x00;
		frame[POS_DATA+1] = 0x00;

		//FN
		frame[POS_DATA+2] = fn & 0xFF;
		frame[POS_DATA+3] = fn >> 8;

		pos = POS_DATA+4;
   
		if(fn == DT_F3)
		{
			frame[pos++]= byte2BCD( datetime.second );
			frame[pos++]= byte2BCD( datetime.minute );
			frame[pos++]= byte2BCD( datetime.hour );
			frame[pos++]= byte2BCD( datetime.day );
			frame[pos++]= byte2BCD( datetime.month )|(datetime.weekday<<5);
			frame[pos++]= byte2BCD( datetime.year );
		}

	//    if(g_event.acd)
	//    {
	//        frame[POS_CTRL] |= CTRLFUNC_SET_ACD;
	//        //EC
	//        //重要事件计数器EC1
	//        frame[pos++] = g_event.ec1;
	//
	//        //一般事件计数器EC2
	//        frame[pos++] = g_event.ec2;
	//    }

		//3.1 减去固定长度帧报文头
		pos -= 6;

		//3.2 数据长度左移2位
		pos<<=2;

		//3.3  规约类型，按照命令的规约类型回答
		#ifdef __QGXD_FK2012__
		protocol_type =QGXD_FK2012;
		#elif defined  __QNMDW_FK2012__
		protocol_type = QNMDW_FK2012;
		#elif defined __QGDW_FK2005__
		protocol_type = QGDW_2005;
		#else
		protocol_type = QGDW_376_1;
		#endif
		pos += protocol_type;
		//3.4 设置长度
		frame[1] =  pos;
		frame[2] =  pos >> 8;
		frame[3] =  frame[1];
		frame[4] =  frame[2];

		//4 设置响应帧序号（和任务上报共用一个帧序号）
		cur_seq = 1;//get_cur_seq();
		frame[POS_SEQ] = MASK_FIR|MASK_FIN|MASK_CON | cur_seq;


		//设置终端逻辑地址：行政区划码+终端地址
		mem_cpy(frame+POS_RT,gSystemInfo.ertu_devid,CFG_DEVID_LEN);
		frame[POS_MSA] = 0;

		//确保帧帧起始符号正确
		frame[0]=0x68;
		frame[5]=0x68;

		//提取用户数据区长度
		frameLen = bin2_int16u(frame+1);
		frameLen >>=2;

		//计算校验位
		cs=0;
		frameLen += 6;   //加上固定长度的报文头
		for(i=6;i<frameLen;i++)  cs += frame[i];
		frame[frameLen++]=cs;

		//设置结束字符
		frame[frameLen++]=0x16;

		return frameLen;
	#endif
}

void app_run_init(void)
{
    INT16U tmp=0;
    INT8U data[2]={0};
#ifdef __EDP__
	INT8U edp_interval =0;
	INT8U rand_report_begin =0,rand_report_end =0;	
#endif
#ifdef __HJ_212__
	INT8U passwd[7]={0};
#endif

//INT8U tmp_updata_verison[16];
//tmp_updata_verison[11] =0x89;
//fwrite_ertu_params(EEADDR_CLEANNMETER_WARNING,&tmp_updata_verison,16);
//tmp_updata_verison[11] =0x00;
//fread_ertu_params(EEADDR_CLEANNMETER_WARNING,&tmp_updata_verison,16);
    //读取终端地址
    fread_ertu_params(EEADDR_CFG_DEVID,gSystemInfo.ertu_devid,CFG_DEVID_LEN);
    fread_ertu_params(EEADDR_METER_SETFLAG,&g_meter_set_flag,1);
    #ifdef __SOFT_SIMULATOR__
    g_meter_set_flag = 0x55;
    #endif

    gSystemInfo.has_data_report = 0x00;
    gSystemInfo.is_debug = 0;//默认非调试
    read_custom_param(CONST_FRAME_MONITOR, data);      //2，报文监控
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
        //gSystemInfo.is_debug = 1;//底座上线开关 兼做生产模式 onenet底座调试时默认不上线和复位模组 
    }
    else
    {
        gSystemInfo.clock_ready = 0;
        //gSystemInfo.is_debug = 0;
    }

    fread_ertu_params(EEADDR_STAT_FLOW,&g_app_run_data.gprs_bytes,sizeof(tagAPP_GPRS_BYTES));   //上电首先将nor flash中的流量读出来。
    fread_ertu_params(EEADDR_SET_F702,&g_app_run_data.meter_control.is_control_valid,sizeof(tagAPP_CONTROL));//拉合闸参数
    fread_ertu_params(EEADDR_SET_F701,&g_app_run_data.password,4);//电表密码
#ifdef __ONENET__
    fread_ertu_params(EEADDR_TASK_REPORT,task.value,sizeof(tagDatetime)*2);
#endif
    gSystemInfo.update = 0;//默认是0 
    gSystemInfo.update_begin_tick=0;
	
	gSystemInfo.filename = NULL;
	gSystemInfo.offset = 0;
	gSystemInfo.data = NULL;
	gSystemInfo.len = 0;
	gSystemInfo.usb_write = 0;	
#ifdef __EDP__
   for(int i =0;i<5;i++)
   {
	   nor_flash_erase_page(FLASH_EDP_RAND_REPORT_STORE_START+i);
   }
	fread_ertu_params(EEADDR_TIME_INTERVAL,&edp_interval,1);
	if(edp_interval ==0xFF)
	{
		edp_interval =3;
	}
	gSystemInfo.edp_report_interval =edp_interval ;
	first_poweron_flash_rebuilt();
	first_poweron_flash_wait_report_rebuilt();
	fwrite_ertu_params(EEADDR_EDP_RAND_BEGIN,&rand_report_begin,1);
	fwrite_ertu_params(EEADDR_EDP_RAND_END,&rand_report_end,1);
	if((rand_report_begin == 0xFF)||(rand_report_begin == 0x00))
	{
		gSystemInfo.edp_rand_report_begin =1;   //随机延时开始时间，默认值
	}
	else
	{
		gSystemInfo.edp_rand_report_begin =(INT8U)rand_report_begin;	
	}
	if((rand_report_end == 0xFF)||(rand_report_end == 0x00))
	{
		gSystemInfo.edp_rand_report_end = 3;//随机延时结束时间，默认值
	}
	else
	{
		gSystemInfo.edp_rand_report_end =(INT8U)rand_report_end;
	}
   
#endif
#ifdef __HJ_212__
	fread_ertu_params(EEADDR_HJ212_PASSWD,passwd,6);
	if(mem_all_is(passwd,6,0xFF))
	{
		mem_cpy(passwd,"\0x31\x32\x33\x34\x35\x36",6);
	}
	mem_cpy(gSystemInfo.passwd,passwd,6);
#endif
	//fread_ertu_params(EEADDR_SWITCH_PARAM,&gSystemInfo.switch_param,4);
	//if(gSystemInfo.switch_param == 0xFFFFFFFF)
	//{
		//gSystemInfo.switch_param = 0;
		//fwrite_ertu_params(EEADDR_SWITCH_PARAM,&gSystemInfo.switch_param,4);	
	//}
}