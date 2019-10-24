///////////////////////////////////////////////////////////////////////////////
//
//   Q/GDW  1376.1 报文处理器  
//
///////////////////////////////////////////////////////////////////////////////


#include "../main_include.h"
#include "protocol_app_handler.h"


INT8U  ACD;


INT8U process_client_apdu_get(objRequest *pRequest,objResponse *pResp);
INT8U process_client_apdu_set(objRequest *pRequest,objResponse *pResp);
INT8U process_client_apdu_action(objRequest *pRequest,objResponse *pResp,INT8U *rest_flag);
void process_trans_frame_2_meter(objRequest *pRequest,objResponse *pResp);
void process_trans_frame_2_meter_ext(objRequest *pRequest,objResponse *pResp);

/*+++
  功能：  判断是否已经接收到了报文头部
  备注：
          1)要求frame中不包含多余字节，即首字节必须符合标准。
---*/
void app_protocol_handler(objRequest* pRequest,objResponse *pResp)
{
    void app_prepare_request(objRequest *pRequest);
    void process_client_apdu_proxy(objRequest *pRequest,objResponse *pResp);
    INT8U apdu_service_code;
    INT8U reset_flag = 0;
    
    //检查下行报文的终端地址是否有效
    if(FALSE == check_ertu_address(pRequest,pResp))
    {
       return ;
    }
    //预处理request对象
    app_prepare_request(pRequest);

    //初始化response对象
    app_init_response(pRequest,pResp);


    if(!check_frame_invalid(pRequest))
    {
        send_error_response(pRequest,pResp,DAR_OTHER_REASON);
        return;
    }
    (void)check_timetag(pRequest);//这里仅提取相关变量，不做处理，在具体服务中做相应检查处理
    apdu_service_code=pRequest->frame[pRequest->userdata_pos];
    
    switch(apdu_service_code)   //APDU
    {
    case APDU_LINK_RESPONSE:
        return;
    case CLIENT_APDU_GET:
		if(process_client_apdu_get(pRequest,pResp))
		{
			return;
		}
        break;
    case CLIENT_APDU_SET:
		if(process_client_apdu_set(pRequest,pResp))
		{
			return;
		}
        break;
    case CLIENT_APDU_ACTION:
        if(process_client_apdu_action(pRequest,pResp,&reset_flag))
        {
			app_send_response_frame(pRequest,pResp,TRUE);
            #ifndef __SOFT_SIMULATOR__
            if(reset_flag) //如果需要复位
            {
                //延时是为了确保最后的应答报文可以发送出去！！
               // DelayNmSec(2000);
                if(pResp->channel == PROTOCOL_CHANNEL_REMOTE)
                {
                     //GPRS多等待5秒
                   //  DelayNmSec(5000);
                }
				if(pResp->channel == PROTOCOL_CHANNEL_DEBUG)
				{
					//GPRS多等待5秒
				//	DelayNmSec(5000);
				}
                  app_softReset();
            }
            #endif
           // app_send_response_frame(pRequest,pResp,TRUE);
			//更新全局变量日冻结类状态
			form_hold_state();
            return;
        }
        break;
    case CLIENT_APDU_PROXY:
        process_client_apdu_proxy(pRequest,pResp);
        return;
//    case CLIENT_APDU_REPORT://
//    case CLIENT_APDU_CONNECT:
//    case CLIENT_APDU_RELEASE:
    #if defined __ENABLE_ESAM2__
    case APDU_SECURITY_REQUEST:
        process_client_apdu_security(pRequest,pResp);
        break;
    #endif
//    case APDU_SECURITY_RESPONSE:
    default:
		send_error_response(pRequest,pResp,DAR_OTHER_REASON);
		return;
    }
    //转发给电表
    process_trans_frame_2_meter_ext(pRequest,pResp);
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
    pRequest->userdata_pos=(pRequest->frame[OOP_POS_ADDR]& 0x0F)+9;

    pRequest->pos = pRequest->userdata_pos;
    if(pRequest->frame_len>pRequest->userdata_pos+3)
    {
        pRequest->userDataLen = pRequest->frame_len - pRequest->userdata_pos-3; //考虑到应用层处理时的方便性，userDataLen仅包含应用层用户数据域
    }
    else
    {
        pRequest->userDataLen = 0;
    }
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
//#define SIZE_OF_APP_PROTOCOL_FRAME 2100
void app_init_response(objRequest *pRequest,objResponse *pResp)
{
    pResp->pos=0x00;
    pResp->frame[pResp->pos++]=0x68;
    pResp->frame[pResp->pos++]=0x00;
    pResp->frame[pResp->pos++]=0x00;
    pResp->frame[pResp->pos++]=CTRLFUNC_DIR_SET | CTRLFUNC_PRM_SET |  CTRL_FUNC_USERDATA;
    pResp->frame[pResp->pos++]=gSystemInfo.devid_len-1;
    mem_cpy_reverse(pResp->frame+OOP_POS_ADDR+1,gSystemInfo.ertu_devid,gSystemInfo.devid_len); //这里我设置表号来读取
   // mem_cpy_reverse(pResp->frame+OOP_POS_ADDR+1,gSystemInfo.meter_no,gSystemInfo.devid_len);
    pResp->pos+=gSystemInfo.devid_len;
    pResp->frame[pResp->pos++]=pRequest->frame[pRequest->userdata_pos-3];//主站地址
    pResp->pos+=2;
    pResp->userdata_pos=pResp->pos;
    pResp->channel = pRequest->channel;
    pResp->max_reply_pos = SIZE_OF_APP_PROTOCOL_FRAME - 10;

}

INT16U get_apdu_len(INT8U *data,INT16U max_len)
{
    switch(data[0])
    {
        case APDU_LINK_RESPONSE:
            return get_apdu_link_response_size(data,max_len);
        case CLIENT_APDU_GET:
            return get_apdu_get_request_size(data,max_len);
        case CLIENT_APDU_SET:
            return get_apdu_set_request_size(data,max_len);
        case CLIENT_APDU_ACTION:
            return get_apdu_action_request_size(data,max_len);
        case CLIENT_APDU_PROXY:
            return get_apdu_proxy_request_size(data,max_len);
        case CLIENT_APDU_REPORT:
            return get_apdu_report_response_size(data,max_len);
        case CLIENT_APDU_CONNECT:
            return get_apdu_app_link_request_size(data,max_len);
        case CLIENT_APDU_RELEASE:
            return 2;
        case APDU_SECURITY_REQUEST:
            return 0;
    }
    return 0;
}

BOOLEAN check_ertu_address(objRequest *pRequest,objResponse *pResp)
{
    INT8U devid[6];
    INT8U addr_len,addr_type,logic_addr;

    //首先检查行政区划代码A1是否正确，否则不予响应
    mem_set(devid,6,0x00);
    mem_cpy_reverse(devid,gSystemInfo.ertu_devid,gSystemInfo.devid_len);

    addr_type = (pRequest->frame[OOP_POS_ADDR]>>6)&0x03;
    logic_addr = (pRequest->frame[OOP_POS_ADDR]>>4)&0x03;
    addr_len=pRequest->frame[OOP_POS_ADDR]& 0x0F;
    pRequest->userdata_pos=addr_len+9;
    
    if(logic_addr != 0)	//逻辑地址只支持0
    {
        return FALSE;
    }
    if(addr_type==0)   //只支持单地址
    {
		if(((addr_len+1)==gSystemInfo.devid_len)&&(compare_string(devid,pRequest->frame+OOP_POS_ADDR+1,gSystemInfo.devid_len) == 0))
		{
			return TRUE;
		}
    }
	else if(addr_type==1)
	{
		return TRUE;
	}
    return FALSE;
}

BOOLEAN check_timetag(objRequest *pRequest)
{
    INT16U len,timeout;
    INT8U send_datetime[10]={0},unit;
    
    //如果是分帧，则需要组成完整帧后才能检查时间标签
    if(pRequest->frame[OOP_POS_CTRL] & 0x20)
    {
        return TRUE;
    }
    switch(pRequest->frame[pRequest->userdata_pos])
    {
        case APDU_LINK_RESPONSE:
        case APDU_SECURITY_REQUEST:
             return TRUE;
    }
    len=get_apdu_len(pRequest->frame+pRequest->userdata_pos ,pRequest->frame_len-pRequest->userdata_pos);
    pRequest->addition_pos=pRequest->userdata_pos;
    if(len>0)
    {
        pRequest->addition_pos=pRequest->userdata_pos+len;
    }
    if(len>0)
    {

       //请求时标
       pRequest->param = & pRequest->frame[pRequest->addition_pos];
       //有效的用户数据长度减区时间标签
       pRequest->userDataLen = pRequest->frame_len-pRequest->userdata_pos-3;

       //验证时间标签有效性
       if(pRequest->param[0]==0)
       {
            return TRUE;
       }
       if(pRequest->userDataLen>9)
       {
            pRequest->userDataLen-=9;
       }

       mem_cpy(send_datetime,&pRequest->param[1],7);
       unit=pRequest->param[8];
       timeout=cosem_bin2_int16u(pRequest->param+9);
       if(timeout == 0)
       {
           return TRUE;
       }
       if(unit==1)
       {
            timeout*=60;
       }
       else if(unit==2)
       {
            timeout*=3600;
       }
    }
    else
    {
        pRequest->param = NULL;
    }
    return TRUE;
}

BOOLEAN check_frame_invalid(objRequest *pRequest)
{
    INT16U pos;
    INT16U max_len;
    BOOLEAN timetag;
    INT8U *data;

    pos=0;
    timetag=FALSE;
    data=pRequest->frame+pRequest->userdata_pos;
    max_len=pRequest->frame_len-pRequest->userdata_pos;
    if(pRequest->frame_len<=pRequest->userdata_pos)
    {
        return FALSE;
    }
    switch(data[0])
    {
        case APDU_LINK_RESPONSE:
            pos=get_apdu_link_response_size(data,max_len);
            break;
        case CLIENT_APDU_GET:
            pos= get_apdu_get_request_size(data,max_len);
            timetag=TRUE;
            break;
        case CLIENT_APDU_SET:
            pos= get_apdu_set_request_size(data,max_len);
            timetag=TRUE;
            break;
        case CLIENT_APDU_ACTION:
            pos= get_apdu_action_request_size(data,max_len);
            timetag=TRUE;
            break;
        case CLIENT_APDU_PROXY:
            pos= get_apdu_proxy_request_size(data,max_len);
            timetag=TRUE;
            break;
        case CLIENT_APDU_REPORT:
            pos= get_apdu_report_response_size(data,max_len);
            timetag=TRUE;
            break;
        case CLIENT_APDU_CONNECT:
            pos= get_apdu_app_link_request_size(data,max_len);
            timetag=TRUE;
            break;
        case CLIENT_APDU_RELEASE:
            pos= 2;
            timetag=TRUE;
            break;
        case APDU_SECURITY_REQUEST:
            pos= get_apdu_security_request_size(data,max_len);
            break;
        case APDU_SECURITY_RESPONSE:
            pos= get_apdu_security_response_size(data,max_len);
            break;
        default:
            pos=0;
            break;
    }
    if(pos==0)
    {
        return FALSE;
    }
    if(timetag)
    {
        if(data[pos++]==1)
        {
            pos+=10;
        }
    }
    if(pos+3==max_len)
    {
        return TRUE;
    }
    return FALSE;
}
void send_error_response(objRequest  *pRequest,objResponse *pResp,INT8U dar)
{
    //设置控制码：DIR=1,PRM=0, FUNC=8  响应报文，用户数据
    pResp->frame[OOP_POS_CTRL] = CTRLFUNC_DIR_SET | CTRLFUNC_PRM_SET |  CTRL_FUNC_USERDATA;
    pResp->frame[pResp->pos++]=SERVER_APDU_ERROR;
    pResp->frame[pResp->pos++]=pRequest->frame[pRequest->userdata_pos+1];
    pResp->frame[pResp->pos++]=dar;
	pRequest->addition_pos = 0;

    app_setFrameLenThenSendFrame(pRequest,pResp);

}

/*+++
 功能： 发送响应数据帧
 参数： INT8U *Reply 响应帧,没有给定数据长度，附加域
       int pos              响应帧最后设置的数据位置

 备注：根据下行报文的时间标签以及当前ACD标志可能会增加附加域（EC,时间标签）
---*/
void app_setFrameLenThenSendFrame_exec(objRequest  *pRequest,objResponse *pResp)
{
    INT8U get_cur_seq(void);
    INT8U head_len;


    //3. 设置应答帧中的数据域长度

    pResp->frame[OOP_POS_BEGIN]=0x68;
    //3.4 设置长度
    pResp->frame[OOP_POS_LEN] =  pResp->pos-1+2;
    pResp->frame[OOP_POS_LEN+1] = (pResp->pos-1+2) >> 8;

    //设置终端逻辑地址：行政区划码+终端地址
    pResp->frame[OOP_POS_ADDR]=gSystemInfo.devid_len-1;
    mem_set(pResp->frame+OOP_POS_ADDR+1,gSystemInfo.devid_len,0x00);
    mem_cpy_reverse(pResp->frame+OOP_POS_ADDR+1,gSystemInfo.ertu_devid,gSystemInfo.devid_len);
    head_len=3+gSystemInfo.devid_len+2;
    fcs16(pResp->frame+OOP_POS_LEN,head_len);
    //计算FCS校验位
    fcs16(pResp->frame+OOP_POS_LEN,pResp->pos-1);
    pResp->pos += 2;
    pResp->frame[pResp->pos++] = 0x16;
    pResp->frame_len = pResp->pos;

    if(pResp->channel==CHANNEL_NONE)
    {
        return;
    }

    app_send_ReplyFrame(pResp);
}

void process_addition(objRequest  *pRequest,objResponse *pResp)
{
    if((pResp->frame[pResp->userdata_pos]!=APDU_LINK_REQUEST)
    && (pResp->frame[pResp->userdata_pos]!=APDU_LINK_RESPONSE)
    && (pResp->frame[pResp->userdata_pos]!=APDU_SECURITY_REQUEST)
    && (pResp->frame[pResp->userdata_pos]!=APDU_SECURITY_RESPONSE))
    {
        pResp->frame[pResp->pos++]=0;
        if((pRequest->addition_pos>0) &&(pRequest->frame[pRequest->addition_pos]!=0))
        {
            mem_cpy(pResp->frame+pResp->pos,pRequest->frame+pRequest->addition_pos,11);
            pResp->pos+=11;
        }
        else
        {
            pResp->frame[pResp->pos++]=0;
        }
    }
}

void app_setFrameLenThenSendFrame(objRequest  *pRequest,objResponse *pResp)
{
    process_addition(pRequest,pResp);
    #if defined __ENABLE_ESAM2__
    if(pRequest->is_security)//如果是安全传输则，以安全传输帧响应
    {
        pResp->pos=security_encode_apdu(pRequest,pResp);
    }
    #endif
    app_setFrameLenThenSendFrame_exec(pRequest,pResp);
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

uint16_t app_make_active_test_frame(uint8_t *frame,uint8_t is_first)
{
    tagDatetime datetime;
    INT16U len,pos,pos_hcs,val16;
    INT8U req_type;

	os_get_datetime(&datetime);

    frame[OOP_POS_BEGIN] = 0x68;
    frame[OOP_POS_CTRL] = CTRLFUNC_SET_DIR |  CTRLFUNC_FC_LINK_MNG;
    len = 0;
    frame[OOP_POS_ADDR]=gSystemInfo.devid_len-1;
    mem_set(frame+OOP_POS_ADDR+1,7,0x00);

    mem_cpy_reverse(frame+OOP_POS_ADDR+1,gSystemInfo.ertu_devid,gSystemInfo.devid_len);
    pos_hcs = OOP_POS_ADDR + gSystemInfo.devid_len+1+1;
    pos = pos_hcs + 2;
    
    if(is_first)
    {
        req_type = 0;
    }
    else
    {
        req_type = 1;
    }

    frame[pos++] = APDU_LINK_REQUEST;       //APDU
    frame[pos++] = 1;           			//PIID-ACD
    frame[pos++] = req_type;                //请求类别

	frame[pos++] = gSystemInfo.heart_cycle[0];  //心跳周期
	frame[pos++] = gSystemInfo.heart_cycle[1];
	
    val16=2000+datetime.year;
    frame[pos++] = val16>>8;
    frame[pos++] = val16;
    frame[pos++] = datetime.month;
    frame[pos++] = datetime.day;
    frame[pos++] = datetime.weekday;
    frame[pos++] = datetime.hour;
    frame[pos++] = datetime.minute;
    frame[pos++] = datetime.second;
    frame[pos++] = datetime.msecond_h;
    frame[pos++] = datetime.msecond_l;

    len = pos + 2 - 1;
    frame[OOP_POS_LEN] = len;
    frame[OOP_POS_LEN+1] = len >> 8;
    //计算HCS
    fcs16(frame+OOP_POS_LEN,pos_hcs-1);
    //计算FCS
    fcs16(frame+OOP_POS_LEN,pos-1);
    pos += 2;
    frame[pos++] = 0x16;
    return pos;
}

void app_init_esam(void)
{
    extern void esam_get_info(void);
    INT8U tmp_flag[6]={0};
    tagParamObject object;

    #ifdef __ENABLE_ESAM2__
    esam_get_info();
    #endif
    if(!get_param_object(OBIS_ESAM_PARAM,&object))
    {
        return;
    }
    (void)get_oad_value(OBIS_ESAM_PARAM,2,0,tmp_flag,&object);
    (void)get_oad_value(OBIS_ESAM_PARAM,9,0,tmp_flag+3,&object);

    if((tmp_flag[0] == 1) &&(tmp_flag[1] == DT_ENUM) && (tmp_flag[2] == 1) )
    {
        set_system_flag(SYS_ESAM,SYS_FLAG_BASE);     //使用ESAM的MAC校验
    }
    else
    {
        clr_system_flag(SYS_ESAM,SYS_FLAG_BASE);   //不使用ESAM的MAC校验
    }

    if((tmp_flag[3] == 1) &&(tmp_flag[4] == DT_ENUM) && (tmp_flag[5] == 1) )
    {
        set_system_flag(SYS_ESAM_READMETER,SYS_FLAG_BASE);     //使用ESAM抄表
    }
    else
    {
        clr_system_flag(SYS_ESAM_READMETER,SYS_FLAG_BASE);   //不使用ESAM抄表
    }

    #ifdef __SOFT_SIMULATOR__
        clr_system_flag(SYS_ESAM,SYS_FLAG_BASE);   //不使用ESAM的MAC校验
        clr_system_flag(SYS_ESAM_READMETER,SYS_FLAG_BASE);   //不使用ESAM抄表
    #endif
}

void init_default_esam_oi_list(void)
{
    default_esam_oi_list(0x0FFF,3,SMW_PROXY_GET|SMW_GET_NONCODE);                                                       //0ZZZ	当前电能
    default_esam_oi_list(0x1FFF,3,SMW_PROXY_GET|SMW_GET_NONCODE_MAC);                                                   //1ZZZ	最大需量
    default_esam_oi_list(0x2FFF,3,SMW_ACTION_NONCODE_MAC|SMW_GET_NONCODE_MAC);                                          //2ZZZ	变量
    default_esam_oi_list(0x3FFF,3,SMW_ACTION_ENCODE_MAC|SMW_SET_ENCODE_MAC|SMW_GET_NONCODE_MAC);                        //3ZZZ	事件
    default_esam_oi_list(0x4FFF,3,SMW_SET_ENCODE_MAC|SMW_GET_NONCODE_MAC);                                              //4ZZZ  参变量,使用方法操作参变量的，未默认安全模式子

    default_esam_oi_list(OBIS_DATETIME,0,SMW_PROXY_GET|SMW_ACTION_NONCODE|SMW_SET_ENCODE_MAC|SMW_GET_NONCODE);          //4000    日期时间
    default_esam_oi_list(OBIS_COMM_ADDR,0,SMW_PROXY_GET|SMW_ACTION_NONCODE|SMW_SET_ENCODE_MAC|SMW_GET_NONCODE);         //4001	  通信地址
    default_esam_oi_list(OBIS_CUST_NO,0,SMW_PROXY_GET|SMW_SET_NONCODE_MAC|SMW_GET_NONCODE_MAC);                         //4003	  客户编号
    default_esam_oi_list(OBIS_CUR_FL_PRICE,0,SMW_PROXY_GET|SMW_ACTION_NONCODE|SMW_SET_ENCODE_MAC|SMW_GET_NONCODE_MAC);  //4018	  当前套费率电价

    default_esam_oi_list(OBIS_METER_TABLE       ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x6000      //采集档案配置表
    default_esam_oi_list(OBIS_METER_INFO        ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x6001      //采集档案配置单元
    default_esam_oi_list(OBIS_SEARCH_METER_TABLE,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x6002      //搜表对象
    default_esam_oi_list(OBIS_SEARCH_METER_INFO ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x6003      //搜表信息
    default_esam_oi_list(OBIS_MAINNODE_CHANGE   ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x6004      //一个跨台区结果
    default_esam_oi_list(OBIS_TASK_TABLE        ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x6012      //任务配置表
    default_esam_oi_list(OBIS_TASK              ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x6013      //任务配置信息
    default_esam_oi_list(OBIS_NORMAL_TASK_TABLE ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x6014      //普通采集方案集
    default_esam_oi_list(OBIS_NORMAL_TASK       ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x6015      //普通采集方案
    default_esam_oi_list(OBIS_EVENT_TASK_TABLE  ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x6016      //事件采集方案集
    default_esam_oi_list(OBIS_EVENT_TASK        ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x6017      //事件采集方案
    default_esam_oi_list(OBIS_RELAY_TASK_TABLE  ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x6018      //透明方案集
    default_esam_oi_list(OBIS_RELAY_TASK        ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x6019      //透明方案
    default_esam_oi_list(OBIS_RELAY_RESULT_TABLE,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x601A      //透明方案结果集
    default_esam_oi_list(OBIS_RELAY_RESULT      ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x601B      //透明方案结果
    default_esam_oi_list(OBIS_REPORT_TASK_TABLE ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x601C      //上报方案集
    default_esam_oi_list(OBIS_REPORT_TASK       ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_PROXY_SET|SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);//0x601D      //上报方案

    default_esam_oi_list(OBIS_FILE_TABLE         ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_ACTION_NONCODE|SMW_GET_NONCODE);         //0x7000	文件集合
    default_esam_oi_list(OBIS_FILE               ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_ACTION_NONCODE|SMW_GET_NONCODE);         //0x7001    文件
    default_esam_oi_list(OBIS_SCRIPT_TABLE       ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_ACTION_NONCODE|SMW_GET_NONCODE);         //0x7010    脚本集合
    default_esam_oi_list(OBIS_SCRIPT             ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_ACTION_NONCODE|SMW_GET_NONCODE);         //0x7011    脚本
    default_esam_oi_list(OBIS_SCRIPT_RESULT_TABLE,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_ACTION_NONCODE|SMW_GET_NONCODE);         //0x7012    脚本结果集合
    default_esam_oi_list(OBIS_SCRIPT_RESULT      ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_ACTION_NONCODE|SMW_GET_NONCODE);         //0x7013    脚本结果
    default_esam_oi_list(OBIS_EXT_VAR_TABLE      ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_ACTION_NONCODE|SMW_GET_NONCODE);         //0x7100    扩展变量对象集合
    default_esam_oi_list(OBIS_EXT_PARAM_TABLE    ,0,SMW_PROXY_ACTION|SMW_PROXY_GET|SMW_ACTION_NONCODE|SMW_GET_NONCODE);         //0x7101    扩展参变量对象集合
    default_esam_oi_list(OBIS_SLICING_TRANSMIT   ,0,SMW_ACTION_ENCODE_MAC|SMW_SET_ENCODE_MAC|SMW_GET_NONCODE);                  //0xF000	分帧传输
    default_esam_oi_list(OBIS_BLOCK_TRANSMIT     ,0,SMW_ACTION_ENCODE|SMW_SET_ENCODE|SMW_GET_NONCODE);                          //0xF001	分块传输
    default_esam_oi_list(OBIS_EXTEND_TRANSMIT    ,0,SMW_ACTION_NONCODE|SMW_SET_NONCODE|SMW_GET_NONCODE);                        //0xF002	扩展传输
    default_esam_oi_list(OBIS_ESAM               ,0,SMW_PROXY_GET|SMW_ACTION_ENCODE_MAC|SMW_SET_NONCODE_MAC|SMW_GET_NONCODE);   //0xF100	ESAM
    default_esam_oi_list(OBIS_ESAM_PARAM         ,0,SMW_PROXY_GET|SMW_ACTION_ENCODE|SMW_SET_ENCODE_MAC|SMW_GET_NONCODE);        //0xF101	安全模式参数
}

void init_default_task_plan();

void app_run_init(void)
{   
    uint32_t offset;
    uint8_t data[10];
    //读取终端地址
    mem_set(data,10,0x00);
    offset = get_object_offset(OBIS_COMM_ADDR);
    fread_ertu_params(offset,data,10);
    gSystemInfo.devid_len = 6;
    if((data[0]==DT_OCTETSTRING)&&(data[1]<=6))
    {
        mem_cpy(gSystemInfo.ertu_devid,data+2,data[1]);
		gSystemInfo.devid_len=data[1];
    }
    else
    {
        mem_cpy(gSystemInfo.ertu_devid,"\x22\x22\x22\x22\x22\x22",6);
    }
    //报文监控控制字
    load_ertu_param(OBIS_CUSTOM_INFO,16,0);
    #ifdef __ENABLE_ESAM2__
    init_default_esam_oi_list();
    load_esam_oi_list();
    app_init_esam();
    #endif
    form_hold_state();
 	//TerminalPowerOnOffEvent.eventIndex =0x1;
	//fwrite_ertu_params(TERMINAL_POWER_ONOFF,&TerminalPowerOnOffEvent,sizeof(StrTerminalPowerOnOffEventBlock));
	//TerminalPowerOnOffEvent.eventIndex =0x0;
	fread_ertu_params(TERMINAL_POWER_ONOFF,&TerminalPowerOnOffEvent,sizeof(StrTerminalPowerOnOffEventBlock));
	if(TerminalPowerOnOffEvent.eventIndex>16)  //标号排序从1开始，1到15；
	{
		TerminalPowerOnOffEvent.eventIndex = 0;
		fwrite_ertu_params(TERMINAL_POWER_ONOFF,&TerminalPowerOnOffEvent,sizeof(StrTerminalPowerOnOffEventBlock));

	}
	if(TerminalPowerOnOffEvent.eventIndex == 0) //模块第一次上电；
	{
		TerminalPowerOnOffState = 1;
	}
	else
	{
		TerminalPowerOnOffState =TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventSource[1];
		if(TerminalPowerOnOffState == 0xFF)
		{
			TerminalPowerOnOffState = 0;
		}
	}
	
	//更新流量数据到全局变量。
	fread_ertu_params(MONTH_BYTES,&gSystemInfo.month_bytes,8);
	if(gSystemInfo.month_bytes == 0xFFFFFFFFFFFFFFFF)
	{
		gSystemInfo.month_bytes =0;
		fwrite_ertu_params(MONTH_BYTES,&gSystemInfo.month_bytes,8);
	}
	fread_ertu_params(DAY_BYTES,&gSystemInfo.day_bytes,2);
	if(gSystemInfo.day_bytes == 0xFFFF)
	{
		gSystemInfo.day_bytes =0;
		fwrite_ertu_params(DAY_BYTES,&gSystemInfo.day_bytes,2);
	}
#ifdef __PROVIENCE_LIAONING_OOP__ 	
	/**上电检测任务方案有没有，如果没有那么需要使用默认配置，即将gtask和gplan 中的数据拷贝到内存中去**/
	init_default_task_plan();
#endif
}
#ifdef __PROVIENCE_LIAONING_OOP__  
void init_default_task_plan()
{
	INT32U offset =0,pos = 0,addr =0;
	INT8U tmp[4],idx =0;
	INT16U max_size =0,len =0;
	INT8U max_count =0;
	INT8U  acq_ctrl_flag =0;	
	offset = ACQ_TASK_INFO;
	pos=0;
	addr=offset;
	fread_ertu_params(addr,tmp,4);
	if((compare_string(tmp,"\xFF\xFF\xFF\xFF",4) ==0) ||(tmp[0]!=0x02)||(tmp[1]!=0x0C))  //需要使用默认配置
	{
		offset=ACQ_TASK_INFO;
		max_size=MAX_TASK_INFO_SIZE;
		max_count=MAX_TASK_COUNT;
		acq_ctrl_flag =1;
		len = MAX_TASK_INFO_SIZE-1;
		for(idx =0;idx <G_TASK_PLAN_CNT;idx ++)
		{
			add_acq_ctrl_info(gtask[idx],len,max_size,max_count,offset,acq_ctrl_flag);
			fread_ertu_params(addr,tmp,4);
		}
		
		offset=ACQ_NORMAL_PLAN_INFO;        
		max_size=MAX_NORMAL_PLAN_SIZE;
        max_count=MAX_NORMAL_PLAN_COUNT;
        acq_ctrl_flag =2;
		len = 200;
		for(idx =0;idx <G_TASK_PLAN_CNT;idx ++)
		{
			add_acq_ctrl_info(gplan[idx],len,max_size,max_count,offset,acq_ctrl_flag);
		}		

	}
}

#endif
