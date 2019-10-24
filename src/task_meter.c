#include "main_include.h"



uint8_t meter_send_buffer[300];
uint8_t meter_recv_frame[300];
uint8_t meter_recv_flag=0;
uint16_t meter_recv_len=0;

uint8_t send_wait_response(uint32_t baudrate,uint8_t *frame,uint16_t frame_len,uint16_t time_out_10ms);

#ifdef __ONENET__
//顺序执行 instance为相对偏移位置
const tagResourceIdEvent  RESOURCE_ID_EVENT[] =
{
	//item_times,   check_offset1,  check_len1,     item_end,       check_offset2,  check_len2,    instance
	#ifdef __METER_SIMPLE__
	{0x03060000,    0,              3,              0x03060001,     6,              6,              2}, //停上电事件（辅助电源）
	{0x03110000,    0,              3,              0x03110001,     6,              6,              1}, //停上电事件（市电）
	{0x1B010001,    0,              3,              0x1B011201,     0,              6,              3}, //有功功率方向  ,这里指的是A相
	{0x19010001,    0,              3,              0x19012101,     0,              6,              5}, //A相过流
	//{ 0x040005FF,  14,  0xFFFFFFFF,     0xFFFFFFFF,      	3200,       0,      5500,              }, //时钟电池欠压
	#else
	{0x03060000,    0,              3,              0x03060001,     6,              6,              2}, //停上电事件（辅助电源）
	{0x03110000,    0,              3,              0x03110001,     6,              6,              1}, //停上电事件（市电）
	{0x1B010001,    0,              3,              0x1B011201,     0,              6,              3}, //有功功率方向  ,这里指的是A相
	//{0x03110000,    0,              3,              0x03110001,     0,              6,              4}, //无功功率方向

	{0x19010001,    0,              3,              0x19012101,     0,              6,              5}, //A相过流
	{0x19020001,    0,              3,              0x19022101,     0,              6,              6}, //B相过流
	{0x19030001,    0,              3,              0x19032101,     0,              6,              7}, //C相过流

	{0x13010001,    0,              3,              0x13012501,     0,              6,              8}, //A相断相
	{0x13020001,    0,              3,              0x13022501,     0,              6,              9}, //B相断相
	{0x13030001,    0,              3,              0x13032501,     0,              6,              10}, //C相断相
	//{ 0x040005FF,  14,  0xFFFFFFFF,     0xFFFFFFFF,      	3200,       0,      5500,              }, //时钟电池欠压
	#endif
};

void  app_check_meter_record_event(void)
{
	static INT32U last_tick=0;
	static INT8U report_once=0;
	INT32U last_res=0;
	INT8U idx,offset=0;
	INT8U request[30]={0},resp[80]={0};
	INT8U occur=0,state=0;
	tagResourceIdEvent* event;
	tagEVENT event_for,event_cur;

	if(gSystemInfo.tcp_link==0)
	{
		return;
	}
	if(gSystemInfo.login_status!=1)
	{
		return;
	}
	
	if(second_elapsed(last_tick)<30)//每30秒检查一次事件告警
	{
		return;
	}
	else
	{
		last_tick = system_get_tick10ms();
	}
	
	for(idx=0;idx<sizeof(RESOURCE_ID_EVENT)/sizeof(tagResourceIdEvent);idx++)
	{
		event = &RESOURCE_ID_EVENT[idx];
		mem_set(event_for.value,sizeof(event_for),0x00);
		mem_set(event_cur.value,sizeof(event_cur),0x00);
		//使用列表中的(instance-1)*4作为偏移地址 时钟电池欠压不需要存储
		fread_ertu_params(EEADDR_LAST_ENENT_TIMES+(event->instance-1)*4,event_for.value,4);
		
		if(app_read_his_item(event->item_times,request,resp,255, NULL,2000)==0)
		{
			continue;
		}
		mem_cpy(event_cur.times,request+event->check_offset1,event->check_len1);
		if(check_is_all_FF(event_for.times,3))
		{
			mem_cpy(event_for.times,event_cur.times,sizeof(tagEVENT));
			event_for.state=0x55;
			fwrite_ertu_params(EEADDR_LAST_ENENT_TIMES+(event->instance-1)*4,event_for.value,4);
			continue;
		}

		if(compare_string(event_for.times,event_cur.times,3)==0)
		{
			occur = 0;
			if(event_for.state!=0xAA)
			{
				continue;
			}
		}
		else
		{
			occur = 1;
		}

		if(app_read_his_item(event->item_end,request,resp,255,NULL,2000)==0)
		{
			continue;
		}
		//结束时间无效为已经发生
		if(check_is_all_ch(request+event->check_offset2,event->check_len2,0x00)
		||check_is_all_FF(request+event->check_offset2,event->check_len2))
		{
			event_cur.state=0xAA;
			state = 1;
		}
		//结束时间有效为已经恢复
		else
		{
			event_cur.state=0x55;
			state = 0;
		}
		
		//次数变更 或 发生恢复事件
		if(occur||(event_for.state!=event_cur.state))
		{
			if((gSystemInfo.csq==99)||(gSystemInfo.csq==0))
			{
				gSystemInfo.tcp_link=0;
				gSystemInfo.login_status = 0;
				return;
			}
			//report
			//如果次数变更 已经恢复需要上报发生和恢复
			gSystemInfo.allow_charge=0;//发送前关闭充电
			if((occur)&&(event_cur.state==0x55))
			{
				sprintf((char*)resp,"AT+MIPLNOTIFY=0,0,3200,%d,5500,5,1,\"1\",0,0\r\n",event->instance);
				remote_send_app_frame(resp,str_cmdlen(resp));
			}
			
			sprintf((char*)resp,"AT+MIPLNOTIFY=0,0,3200,%d,5500,5,1,\"%d\",0,0\r\n",event->instance,state);
			remote_send_app_frame(resp,str_cmdlen(resp));
			gSystemInfo.allow_charge=1;
			
			mem_cpy(event_for.value,event_cur.value,sizeof(event_for));
			fwrite_ertu_params(EEADDR_LAST_ENENT_TIMES+(event->instance-1)*4,event_for.value,4);
		}
	}
	return;

}
void app_check_meter_status(void)
{
	static INT32U last_tick=0;
	INT16U ackid =0;
	INT8U for_status[26]={0},cur_status[26]={0};
	INT8U request[100]={0};
	INT8U *resp;
	
	if(gSystemInfo.tcp_link==0)
	{
		return;
	}
	if(gSystemInfo.login_status!=1)
	{
		return;
	}
	//30秒检查一次事件
	if(second_elapsed(last_tick) < 30)
	{
		return;
	}
	else
	{
		last_tick = system_get_tick10ms();
	}
	
	
	fread_ertu_params(EEADDR_LAST_STATUS,for_status,sizeof(for_status));
	if(for_status[20]==0xFF)
	{
		mem_set(for_status,sizeof(for_status),0x00);
		fwrite_ertu_params(EEADDR_LAST_STATUS,for_status,sizeof(for_status));
	}
	
	resp = request+30;//预留30的请求报文长度
	if(app_read_his_item(0x040005FF,request,resp,255, NULL,2000)==0)
	{
		return;
	}
	else
	{
		memcpy(cur_status,request,20);
		if((cur_status[0]&0x04) != (for_status[0]&0x04))
		{
			if((gSystemInfo.csq==99)||(gSystemInfo.csq==0))
			{
				gSystemInfo.tcp_link=0;
				gSystemInfo.login_status = 0;
				return;
			}
			gSystemInfo.allow_charge=0;//发送前关闭充电
			mem_cpy(cur_status,request,sizeof(for_status));
			fwrite_ertu_params(EEADDR_LAST_STATUS,cur_status,sizeof(cur_status));
			if(cur_status[0]&0x04)
			{
				sprintf(request,"AT+MIPLNOTIFY=0,0,3200,0,5500,5,1,\"1\",0,0\r\n");
			}
			else
			{
				sprintf(request,"AT+MIPLNOTIFY=0,0,3200,0,5500,5,1,\"0\",0,0\r\n");
			}
			remote_send_app_frame(request,str_cmdlen(request));
			gSystemInfo.allow_charge=1;
		}
		//
		#ifndef __METER_SIMPLE__
		if((cur_status[0]&0x20) != (for_status[0]&0x20))
		{
			if((gSystemInfo.csq==99)||(gSystemInfo.csq==0))
			{
				gSystemInfo.tcp_link=0;
				gSystemInfo.login_status = 0;
				return;
			}
			mem_cpy(cur_status,request,sizeof(for_status));
			fwrite_ertu_params(EEADDR_LAST_STATUS,cur_status,sizeof(cur_status));
			if(cur_status[0]&0x04)
			sprintf(request,"AT+MIPLNOTIFY=0,0,3200,4,5500,5,1,\"1\",0,0\r\n");
			else
			sprintf(request,"AT+MIPLNOTIFY=0,0,3200,4,5500,5,1,\"0\",0,0\r\n");
			remote_send_app_frame(request,str_cmdlen(request));
			
		}
		#endif
	}
	
	
}
#endif

INT16U app_trans_send_meter_frame(INT8U* frame,INT16U frame_len,INT8U* resp,INT16U max_resp_len,INT16U max_wait_time_10ms)
{
    uint32_t timer;
    uint16_t resp_len;
    
    resp_len = 0;
    timer = system_get_tick10ms();
    #ifndef __SOFT_SIMULATOR__
    while(tpos_mutexRequest(&SIGNAL_METER_UART) == FALSE)
    {
        if((system_get_tick10ms()-timer) > max_wait_time_10ms)
        {
            return 0;
        }
        tpos_TASK_YIELD();
    }
    #endif
    if(send_wait_response(0,frame,frame_len,300))//之前是80
    {
        if(max_resp_len < meter_recv_len)
        {
            resp_len = 0;
        }
        else
        {
            resp_len = meter_recv_len;
            mem_cpy(resp,meter_recv_frame,meter_recv_len);
        }
    }
    #ifndef __SOFT_SIMULATOR__
    tpos_mutexFree(&SIGNAL_METER_UART);
    #endif
    return resp_len;
}
void push_to_meter_frame(uint8_t *frame,uint16_t frame_len)
{
    if(meter_recv_flag)
    {
        return ;
    }
    OSMutexPend(&SIGNAL_METER);
    mem_cpy(meter_recv_frame,frame,frame_len);
    meter_recv_len = frame_len;
    meter_recv_flag = 1;
    OSMutexFree(&SIGNAL_METER);
}
volatile static INT32U meter_baud;
void send_meter_frame(uint32_t baudrate,uint8_t *frame,uint16_t frame_len)
{
    //if((baudrate !=0) && (baudrate != meter_baud))
	if((baudrate !=0))
    {
		meter_baud = baudrate;
        meter_uart_init(baudrate);
    }
    meter_uart_send_buf(frame,frame_len);
    meter_recv_flag = 0;
    if((gSystemInfo.meter_debug_flag)&&(gSystemInfo.debug_info_interface==0))
    {
        system_debug_data(frame,frame_len);
    }
}
uint8_t get_meter_response(uint16_t time_out_10ms)
{
    uint32_t timer;

        #ifdef __SOFT_SIMULATOR__
    snprintf(info,100,"%d ",system_get_tick10ms());
    debug_println(info);
        #endif
    timer = system_get_tick10ms();
    while((system_get_tick10ms()-timer) < time_out_10ms)
    {
        if(meter_recv_flag)
        {
            if((gSystemInfo.meter_debug_flag)&&(gSystemInfo.debug_info_interface==0))
            {
                system_debug_data(meter_recv_frame,meter_recv_len);
            }
            return 1;
        }
        tpos_TASK_YIELD();
    }
        #ifdef __SOFT_SIMULATOR__
    snprintf(info,100,"%d ",system_get_tick10ms());
    debug_println(info);
        #endif
    return 0;
}
uint8_t send_wait_response(uint32_t baudrate,uint8_t *frame,uint16_t frame_len,uint16_t time_out_10ms)
{
    uint8_t resault;
    
    resault = 0;
    tpos_mutexPend(&SIGNAL_METER_UART);
    send_meter_frame(baudrate,frame,frame_len);
    if(get_meter_response(time_out_10ms))
    {
        //
        resault = 1;
        gSystemInfo.local_status=1;
		gSystemInfo.noResponseCnt = 0;
		
    }
    else
    {
		meter_baud = 0;
		gSystemInfo.noResponseCnt++;
		if(gSystemInfo.noResponseCnt >20)
		{
    		gSystemInfo.local_status=0;
		}
    }
    tpos_mutexFree(&SIGNAL_METER_UART);
    return resault;
}

INT8U make_gb645_2007_frame(INT8U *frame,INT8U *meter_no,INT8U ctrl,INT32U item,INT8U *data,INT8U datalen)
{
    INT8U idx,pos;
    INT8U cs;

    pos=0;
    frame[pos++] = 0xFE;
    frame[pos++] = 0xFE;
    frame[pos++] = 0xFE;
    frame[pos++] = 0xFE;
    frame[pos++] = 0x68;
    frame[pos++] = meter_no[0];
    frame[pos++] = meter_no[1];
    frame[pos++] = meter_no[2];
    frame[pos++] = meter_no[3];
    frame[pos++] = meter_no[4];
    frame[pos++] = meter_no[5];
    frame[pos++] = 0x68;
    frame[pos++] = ctrl;
    frame[pos++] = 4+datalen;
    frame[pos++] = item;
    frame[pos++] = item>>8;
    frame[pos++] = item>>16;
    frame[pos++] = item>>24;

    for(idx=0;idx<datalen;idx++) frame[pos++]=data[idx];
    for(idx=14;idx<pos;idx++) frame[idx]+=0x33;
    cs = 0;
    for(idx=4;idx<pos;idx++) cs+=frame[idx];
    frame[pos++]=cs;
    frame[pos++]=0x16;
    return pos;
}
//拉合闸控制报文
INT8U make_gb645_2007_frame_ctrl(INT8U *frame,INT8U ctrl)
{
    INT8U idx,pos;
    INT8U cs;
    INT8U data[20]={0};
    tagDatetime datetime;
    
    pos=0;
    frame[pos++] = 0xFE;
    frame[pos++] = 0xFE;
    frame[pos++] = 0xFE;
    frame[pos++] = 0xFE;
    frame[pos++] = 0x68;
    frame[pos++] = gSystemInfo.meter_no[0];
    frame[pos++] = gSystemInfo.meter_no[1];
    frame[pos++] = gSystemInfo.meter_no[2];
    frame[pos++] = gSystemInfo.meter_no[3];
    frame[pos++] = gSystemInfo.meter_no[4];
    frame[pos++] = gSystemInfo.meter_no[5];
    frame[pos++] = 0x68;
    frame[pos++] = 0x1C;
    frame[pos++] = 4+4+8;

    if(is_valid_bcd(g_app_run_data.password,4))
    {
        mem_cpy(data,g_app_run_data.password,4);
    }
    else
    {
        data[0]=0x02;
        data[1]=0x56;
        data[2]=0x34;
        data[3]=0x12;
    }
    pos+=4;//password
    pos+=4;//用户权限    
    data[8]=ctrl;
    //有效时间
    os_get_datetime(&datetime);
    data[10]=byte2BCD(datetime.second);
    data[11]=byte2BCD(datetime.minute);
    data[12]=byte2BCD(datetime.hour);
    data[13]=byte2BCD(datetime.day);
    data[14]=byte2BCD(datetime.month);
    data[15]=byte2BCD(datetime.year+1);
    pos+=8;
    mem_cpy(frame+14,data,16);
    for(idx=14;idx<pos;idx++) frame[idx]+=0x33;
    cs = 0;
    for(idx=4;idx<pos;idx++) cs+=frame[idx];
    frame[pos++]=cs;
    frame[pos++]=0x16;
    return pos;
}

//设置模块状态信息到电表中 方便检测  
BOOLEAN update_meter_status(INT32U status)
{
	INT8U frame[30]={0},resp[30]={0},len=0,meter_no[6]={0};
	INT8U data[15]={0xA2,0x68,0x81,0x76,0x88,0x76,0x68,0x90,0x76,0x80,0x70};
	INT8U cs=0,pos=0,idx=0;
	
	mem_cpy(data+11,&status,4);
	mem_cpy(meter_no,gSystemInfo.meter_no,6);
	len = make_gb645_2007_frame(frame,meter_no,0x1F,0x04C80406,data,15);
	len = app_trans_send_meter_frame(frame,len,resp,0xFF,200);
//	if(resp[8]==0x9F)
//{
//		return TRUE;
//	}
	
	pos=0;
	frame[pos++]=0xFE;
	frame[pos++]=0xFE;
	frame[pos++]=0xFE;
	frame[pos++]=0xFE;			
	frame[pos++]=0x68;
	mem_cpy(frame+pos,meter_no,6);
	pos+=6;
	frame[pos++]=0x68;
	frame[pos++]=0x2F;
	frame[pos++]=0x07;
	frame[pos++]=0x00;
	frame[pos++]=0x37;
	frame[pos++]=0x33;
	frame[pos++]=0x3A;
	mem_cpy(frame+pos,&status,4);
	for(idx=pos;idx<pos+4;idx++) frame[idx]+=0x33;
	pos+=4;
	for(idx=0;idx<pos;idx++) cs+=frame[idx];
	frame[pos++]=cs;
	frame[pos++]=0x16; 
	mem_set(resp,30,0x00);
	len = app_trans_send_meter_frame(frame,pos,resp,0xFF,200);
//	if((resp[8]==0xAF)&&(resp[9]==0x00)&&(resp[10]==0x00))
//	{
//		return TRUE;
//}
//	return FALSE;		
    return TRUE;	
}


INT8U make_oop_get_normal_via_Taddr(INT8U *frame,INT8U *oad)
{
    INT8U pos=0;
    INT8U meter_no[6];

    mem_set(meter_no,6,0xAA);

    frame[OOP_POS_BEGIN] = 0x68;
    frame[OOP_POS_LEN]   = 0x00;
    frame[OOP_POS_LEN+1] = 0x00;
    frame[OOP_POS_CTRL]  = 0x43;//CTRLFUNC_PRM_SET|CTRLFUNC_FC_REQ_RESP
    pos = OOP_POS_ADDR;
    frame[pos++] = 0x05 | (0x01<<6);      //地址类型
    mem_cpy(frame+pos,meter_no,6);
    pos += 6;
    frame[pos++] = 0x22;                  //客户机地址CA
    frame[pos++] = 0x00;                  //HCS
    frame[pos++] = 0x00;                  //
    frame[pos++] = 0x05;                  //APDU-读取
    frame[pos++] = 0x01;                  //choice-普通读取
    frame[pos++] = 0x02;                  //PIID
    mem_cpy(frame+pos,oad,4);
    pos += 4;
    frame[pos++] = 0x00;                 //tp
    //长度
    frame[OOP_POS_LEN] = (pos+2-1);
    frame[OOP_POS_LEN+1] = ((pos+2-1)>>8);
    //计算HCS校验位
    fcs16(frame+OOP_POS_LEN,11);
    //计算FCS校验位
    fcs16(frame+OOP_POS_LEN,pos-1);
    pos += 2;
    frame[pos++]=0x16;

    mem_cpy_right(frame+4,frame,pos);
    mem_set(frame,4,0xFE);
    pos += 4;

    return pos;
}

#ifdef __OOP_G_METER__
uint8_t get_meter_data_via_oop(uint8_t *frame,INT8U *oad,uint8_t *data)
{
    uint8_t  addr_len,len,pos;

    addr_len = (frame[OOP_POS_ADDR]&0x0F)+1;
    pos = OOP_POS_CTRL+addr_len+5;
    if((frame[OOP_POS_CTRL] == 0xC3)&&(frame[pos] == 0x85))
    {
        pos += 3;
        if(compare_string(oad,frame+pos,4)!=0)
        {
            return 0;
        }
        pos += 4;
        if(frame[pos++]!=1)
        {
            return 0;
        }
        len = get_oop_data_type_len(frame+pos+1,frame[pos])+1;
        mem_cpy(data,frame+pos,len);
        return len;
    }
    return 0;
}

uint8_t get_meter_no_via_oop(uint8_t *frame,uint8_t *meter_no)
{
    uint8_t len=0,addr_len;
    uint8_t oad[4]={0x40,0x01,0x02,0x00},data[10];
    uint32_t offset;


    len = get_meter_data_via_oop(frame,oad,data);
    if((len>1)&&(len<=10))
    {
        if(data[0]!=DT_OCTETSTRING)
        {
            return 0;
        }
        addr_len = data[1];
        if(addr_len!=6)
        {
            return 0;
        }
	
        mem_cpy_reverse(meter_no,data+2,6);
#ifndef __PROVICE_SHANDONG__			
        offset = get_object_offset(OBIS_COMM_ADDR);
        fwrite_ertu_params(offset,data,8);
	    ////
		mem_cpy(gSystemInfo.ertu_devid,data+2,data[1]);
		gSystemInfo.devid_len=data[1];
#endif
        return 1;
    }
    return 0;
}
#endif
uint8_t unzip_gb645_data(uint8_t *frame,uint32_t check_item,uint8_t *data)
{
    uint32_t item;
    uint8_t  len;
    uint8_t  tmp;
    uint8_t  idx;
    
    if((frame[POS_GB645_CTRL]&0xC0)!=0x80)
    {
        return 0;
    }
    len = frame[POS_GB645_DLEN];
    if(len<=4)
    {
        return 0;
    }
    tmp = frame[POS_GB645_ITEM+3];
    tmp -= 0x33;
    item = tmp;
    item <<= 8;
    tmp = frame[POS_GB645_ITEM+2];
    tmp -= 0x33;
    item += tmp;
    item <<= 8;
    tmp = frame[POS_GB645_ITEM+1];
    tmp -= 0x33;
    item += tmp;
    item <<= 8;
    tmp = frame[POS_GB645_ITEM+0];
    tmp -= 0x33;
    item += tmp;
    if(item != check_item)
    {
        return 0;
    }
    for(idx=0;idx<len-4;idx++)
    {
        data[idx] = frame[POS_GB645_07_DATA+idx]-0x33;
    }
    return (len-4);
}
uint8_t get_meter_no_via_645(uint8_t *frame,uint32_t read_item,uint8_t *meter_no)
{
    uint32_t item;
    uint8_t  tmp;
    uint8_t  idx;
    
    if((frame[POS_GB645_CTRL] == 0x91)&&(frame[POS_GB645_DLEN] >= 10))
    {
        tmp = frame[POS_GB645_ITEM+3];
        tmp -= 0x33;
        item = tmp;
        item <<= 8;
        tmp = frame[POS_GB645_ITEM+2];
        tmp -= 0x33;
        item += tmp;
        item <<= 8;
        tmp = frame[POS_GB645_ITEM+1];
        tmp -= 0x33;
        item += tmp;
        item <<= 8;
        tmp = frame[POS_GB645_ITEM+0];
        tmp -= 0x33;
        item += tmp;
        if(item != read_item)
        {
            return 0;
        }
        for(idx=0;idx<6;idx++)
        {
            tmp = frame[POS_GB645_07_DATA+idx];
            tmp -= 0x33;
            meter_no[idx] = tmp;
        }
        return 1;
    }
    return 0;
}
#ifndef __OOP_G_METER__
void check_read_meter_no(void)
{
    uint16_t app_make_active_test_frame(uint8_t *frame,uint8_t is_first);
    uint16_t frame_len;
    uint8_t  meter_no[6];
    
    while(1)
    {
        mem_set(meter_no,6,0xAA);
        //先尝试07读取通信地址
        frame_len = make_gb645_2007_frame(meter_send_buffer,meter_no,0x11,0x04000401,NULL,0);
       


		
        if(send_wait_response(9600,meter_send_buffer,frame_len,50))
        {
            //
            if(get_meter_no_via_645(meter_recv_frame,0x04000401,meter_no))
            {
                gmeter_doc.baud_port.baud = 6;
                break ;
            }
        }
        if(send_wait_response(4800,meter_send_buffer,frame_len,50))
        {
            //
            if(get_meter_no_via_645(meter_recv_frame,0x04000401,meter_no))
            {
                gmeter_doc.baud_port.baud = 4;
                break ;
            }
        }
        if(send_wait_response(2400,meter_send_buffer,frame_len,50))
        {
            //
            if(get_meter_no_via_645(meter_recv_frame,0x04000401,meter_no))
            {
                gmeter_doc.baud_port.baud = 3;
                break ;
            }
        }
        if(send_wait_response(1200,meter_send_buffer,frame_len,50))
        {
            //
            if(get_meter_no_via_645(meter_recv_frame,0x04000401,meter_no))
            {
                gmeter_doc.baud_port.baud = 2;
                break ;
            }
        }
        

        
       // 调试需要，发送1376.1的登录和心跳
        RequestUart.recv_success_flag = 0;
        frame_len = app_make_active_test_frame(meter_send_buffer,1);
        if(send_wait_response(115200,meter_send_buffer,frame_len,100) == 0)
        {
            if(RequestUart.recv_success_flag)
            {
                //发送1376.1的登录帧有响应，处于调试状态
                while(1)
                {
                    gSystemInfo.is_debug = 1;
                    if((second_elapsed(RequestUart.recv_time_in_10ms)) > 290)//生产检测中避免发送
                    {
                        RequestUart.recv_success_flag = 0;
                        frame_len = app_make_active_test_frame(meter_send_buffer,0);
                        if(send_wait_response(0,meter_send_buffer,frame_len,100) == 0)
                        {
                            if(RequestUart.recv_success_flag == 0)
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    DelayNmSec(10);
                    tpos_TASK_YIELD();
                }
                gSystemInfo.is_debug = 0;
            }
        }
    }
    
    OSMutexPend(&SIGNAL_SYSTEMINFO);
    mem_cpy(gSystemInfo.meter_no,meter_no,6);
    mem_cpy(gmeter_doc.meter_no,gSystemInfo.meter_no,6);
    gSystemInfo.app_is_ready = 1;
	//#ifdef __PROVICE_JILIN__   ////晚上给OOOP 吉林处理的时候暂时注掉
    //if((gmeter_doc.meter_no[0] != gSystemInfo.ertu_devid[2])||(gmeter_doc.meter_no[1] != gSystemInfo.ertu_devid[3])
     //||(gmeter_doc.meter_no[2] != gSystemInfo.ertu_devid[4])||(gmeter_doc.meter_no[3] != gSystemInfo.ertu_devid[5]))
    //{
    	//gSystemInfo.ertu_devid[2] = gmeter_doc.meter_no[0];
    	//gSystemInfo.ertu_devid[3] = gmeter_doc.meter_no[1];
    	//gSystemInfo.ertu_devid[4] = gmeter_doc.meter_no[2];
    	//gSystemInfo.ertu_devid[5] = gmeter_doc.meter_no[3];
        //fwrite_ertu_params(EEADDR_CFG_DEVID+2,gSystemInfo.ertu_devid+2,CFG_DEVID_LEN-2);
    //}
    //#endif
    OSMutexFree(&SIGNAL_SYSTEMINFO);
   
    tpos_mutexPend(&SIGNAL_RUN_DATA);
    g_app_run_data.status |= READ_METER_SUCCESS;
    tpos_mutexFree(&SIGNAL_RUN_DATA); 
//	update_meter_status(g_app_run_data.status);
}
#endif
#ifdef __OOP_G_METER__
void check_read_meter_no(void)
{
    uint16_t app_make_active_test_frame(uint8_t *frame,uint8_t is_first);
    uint16_t frame_len;
    uint8_t  meter_no[6]={0},oad[4];
   
    while(1)
    {
        //读取通信地址       
        mem_cpy(oad,"\x40\x1\x2\x0",4);
        frame_len = make_oop_get_normal_via_Taddr(meter_send_buffer,oad);


        if(send_wait_response(9600,meter_send_buffer,frame_len,50))
        {
            //
            if(get_meter_no_via_oop(meter_recv_frame,meter_no))
            {
                gmeter_doc.baud_port.baud = 6;
                break ;
            }
        }
        if(send_wait_response(4800,meter_send_buffer,frame_len,50))
        {
            //
            if(get_meter_no_via_oop(meter_recv_frame,meter_no))
            {
                gmeter_doc.baud_port.baud = 4;
                break ;
            }
        }
        if(send_wait_response(2400,meter_send_buffer,frame_len,50))
        {
            //
            if(get_meter_no_via_oop(meter_recv_frame,meter_no))
            {
                gmeter_doc.baud_port.baud = 3;
                break ;
            }
        }
        if(send_wait_response(1200,meter_send_buffer,frame_len,50))
        {
            //
            if(get_meter_no_via_oop(meter_recv_frame,meter_no))
            {
                 gmeter_doc.baud_port.baud = 2;
                 break ;
            }
        }
        
        //调试需要，发送1376.1的登录和心跳
        RequestUart.recv_success_flag = 0;
        frame_len = app_make_active_test_frame(meter_send_buffer,1);
        if(send_wait_response(115200,meter_send_buffer,frame_len,100) == 0)
        {
            if(RequestUart.recv_success_flag)
            {
                //发送1376.1的登录帧有响应，处于调试状态
                while(1)
                {
                    if((second_elapsed(RequestUart.recv_time_in_10ms)) > 180)//生产检测中避免发送
                    {
                        RequestUart.recv_success_flag = 0;
                        frame_len = app_make_active_test_frame(meter_send_buffer,0);
                        if(send_wait_response(0,meter_send_buffer,frame_len,100) == 0)
                        {
                            if(RequestUart.recv_success_flag == 0)
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
					DelayNmSec(100);
					tpos_TASK_YIELD();
                }
            }
        }
    }
    
    OSMutexPend(&SIGNAL_SYSTEMINFO);
    mem_cpy(gSystemInfo.meter_no,meter_no,6);
    mem_cpy(gmeter_doc.meter_no,gSystemInfo.meter_no,6);
    gSystemInfo.app_is_ready = 1;
	//#ifdef __PROVICE_JILIN__
    //if((gmeter_doc.meter_no[0] != gSystemInfo.ertu_devid[2])||(gmeter_doc.meter_no[1] != gSystemInfo.ertu_devid[3])
     //||(gmeter_doc.meter_no[2] != gSystemInfo.ertu_devid[4])||(gmeter_doc.meter_no[3] != gSystemInfo.ertu_devid[5]))
    //{
    	//gSystemInfo.ertu_devid[2] = gmeter_doc.meter_no[0];
    	//gSystemInfo.ertu_devid[3] = gmeter_doc.meter_no[1];
    	//gSystemInfo.ertu_devid[4] = gmeter_doc.meter_no[2];
    	//gSystemInfo.ertu_devid[5] = gmeter_doc.meter_no[3];
        //fwrite_ertu_params(EEADDR_CFG_DEVID+2,gSystemInfo.ertu_devid+2,CFG_DEVID_LEN-2);
    //}
    //#endif
    OSMutexFree(&SIGNAL_SYSTEMINFO);
	//OOP电表支持新旧协议其中一个设置
    tpos_mutexPend(&SIGNAL_RUN_DATA);
    g_app_run_data.status |= READ_METER_SUCCESS;
    tpos_mutexFree(&SIGNAL_RUN_DATA);
}

void read_gprs_param_from_oop_meter(void)
{
    uint16_t frame_len;
    uint8_t  oad[4],buffer[65];
    tagParamObject object;
    uint8_t pos,count,pos1=0;
    
    mem_cpy(oad,"\x45\x0\x2\x0",4);
    frame_len = make_oop_get_normal_via_Taddr(meter_send_buffer,oad);
   
    if(send_wait_response(0,meter_send_buffer,frame_len,50))
    {
        if(get_meter_data_via_oop(meter_recv_frame,oad,meter_recv_frame)>1)
        {
            mem_set(buffer,sizeof(buffer),0x00);
            (void)get_param_object(OBIS_GPRS1,&object);
            set_oad_value(OBIS_GPRS1,2,0,meter_recv_frame,&object,buffer);
        }
    }
    
    mem_cpy(oad,"\x45\x0\x3\x0",4);
    frame_len = make_oop_get_normal_via_Taddr(meter_send_buffer,oad);
    
    if(send_wait_response(0,meter_send_buffer,frame_len,50))
    {
        if(get_meter_data_via_oop(meter_recv_frame,oad,meter_recv_frame)>1)
        {
            pos=0;
            mem_set(buffer,sizeof(buffer),0x00);
            if((meter_recv_frame[0]==DT_ARRAY)&&(meter_recv_frame[1]>0))
            {
                pos += 2;
                count = meter_recv_frame[1];
                if((meter_recv_frame[pos]==DT_STRUCTURE)&&(meter_recv_frame[pos+1]==2)&&(meter_recv_frame[pos+2]==DT_OCTETSTRING)&&(meter_recv_frame[pos+3]==4))
                {
                    pos+=8;
                    if(meter_recv_frame[pos++]==DT_LONGUNSIGNED)
                    {
                        pos+=2;
                        buffer[0] = DT_ARRAY;
                        buffer[1] = 1;
                        mem_cpy(buffer+2,meter_recv_frame+2,pos-2);
                        if(count>1)
                        {
                            pos1 = pos;
                            if((meter_recv_frame[pos]==DT_STRUCTURE)&&(meter_recv_frame[pos+1]==2)&&(meter_recv_frame[pos+2]==DT_OCTETSTRING)&&(meter_recv_frame[pos+3]==4))
                            {
                                pos+=8;
                                if(meter_recv_frame[pos++]==DT_LONGUNSIGNED)
                                {
                                    pos+=2;
                                    buffer[1] = 2;
                                    mem_cpy(buffer+pos1,meter_recv_frame+pos1,pos-pos1);
                                    mem_cpy(meter_recv_frame,buffer,pos);
                                }
                            }                
                        }
                        (void)get_param_object(OBIS_GPRS1,&object);
                        set_oad_value(OBIS_GPRS1,3,0,meter_recv_frame,&object,buffer);
                    }
                }
            }
        }
    }    
}

uint8_t update_ertu_datetime(uint8_t force_flag)
{
    static uint32_t timer;
    uint16_t year;

    uint16_t frame_len;
    uint8_t  meter_no[6],oad[4];
    tagDatetime datetime;
	static tagDatetime dt_for_flow_prev,dt_for_flow_next; //为日月流量变化定义时间。
	static INT8U  flow_change_flag =0;
    if((minute_elapsed(timer) < 10)&&(force_flag==0))
    {
        return 0;
    }

    timer = system_get_tick10ms();
    mem_set(meter_no,6,0xAA);
    mem_set(datetime.value,sizeof(tagDatetime),0);
    mem_cpy(oad,"\x40\x0\x2\x0",4);
    frame_len = make_oop_get_normal_via_Taddr(meter_send_buffer,oad);
    OSMutexPend(&SIGNAL_METER_UART);     //保护meter_recv_frame
    if(send_wait_response(0,meter_send_buffer,frame_len,50))
    {
        if(get_meter_data_via_oop(meter_recv_frame,oad,meter_recv_frame)>1)
        {
            year = cosem_bin2_int16u(meter_recv_frame+1);
            if((meter_recv_frame[0] == DT_DATETIME_S)&&(year>2000))
            {
				datetime.century =year/100;
                datetime.year = year-2000;
                datetime.month = meter_recv_frame[3];
                datetime.day = meter_recv_frame[4];
                datetime.hour= meter_recv_frame[5];
                datetime.minute = meter_recv_frame[6];
                datetime.second = meter_recv_frame[7];
            }
            else
            {
                OSMutexFree(&SIGNAL_METER_UART);
                return 0;
            }
        }
        else
        {
            OSMutexFree(&SIGNAL_METER_UART);
            return 0;
        }
    }
    else
    {
        OSMutexFree(&SIGNAL_METER_UART);
        return 0;
    }
    OSMutexFree(&SIGNAL_METER_UART);

    #ifndef __SOFT_SIMULATOR__
    tpos_set_time(&datetime);
    #endif
    gSystemInfo.clock_ready = 1;
    LED_local_on(1);
	if(flow_change_flag ==0)   //初次上电先不存储，之后每过日就写进去
	{ 
		flow_change_flag =1;
		tpos_datetime(&dt_for_flow_prev);
		tpos_datetime(&dt_for_flow_next);		
	}
	else
	{
		tpos_datetime(&dt_for_flow_next);
		if((dt_for_flow_prev.day !=dt_for_flow_next.day)||(dt_for_flow_prev.month !=dt_for_flow_next.month))
		{
			if(dt_for_flow_prev.month ==dt_for_flow_next.month)
			{
				gSystemInfo.day_bytes =0;
				fwrite_ertu_params(MONTH_BYTES,&gSystemInfo.month_bytes,8);			
//				system_debug_info("day is change month is same");
			}
			else
			{
				gSystemInfo.month_bytes =0;
				fwrite_ertu_params(MONTH_BYTES,&gSystemInfo.month_bytes,8);
//				system_debug_info("day is change month is change");
			}
			tpos_datetime(&dt_for_flow_prev);	
		}
	}
	
    return 1;
}
#else
uint8_t update_ertu_datetime(uint8_t force_flag)
{
    static uint32_t timer;
    INT32U  monthflow=0;
    uint16_t frame_len;
    uint8_t  meter_no[6];
    tagDatetime datetime;
    
    if((minute_elapsed(timer) < 10)&&(force_flag==0))
    {
        return 0;
    }
    //检查升级状态每10分钟检查一次 超过30分钟没有收到报文认为升级失败 取消升级状态
    if(gSystemInfo.update_begin_tick!=0)
    {
        if(second_elapsed(gSystemInfo.update_begin_tick)>1800)
        {
            gSystemInfo.update=0;
            gSystemInfo.update_begin_tick=0;
            //system_debug_info("update=0 5");
        }
    }
    timer = system_get_tick10ms();
    mem_set(meter_no,6,0xAA);
    mem_set(datetime.value,sizeof(tagDatetime),0);
    frame_len = make_gb645_2007_frame(meter_send_buffer,meter_no,0x11,0x04000101,NULL,0);
    OSMutexPend(&SIGNAL_METER_UART);
    if(send_wait_response(0,meter_send_buffer,frame_len,200))
    {
        if(unzip_gb645_data(meter_recv_frame,0x04000101,meter_recv_frame)>3)
        {
            datetime.year = BCD2byte(meter_recv_frame[3]);
            datetime.month = BCD2byte(meter_recv_frame[2]);
            datetime.day = BCD2byte(meter_recv_frame[1]);
            datetime.weekday = BCD2byte(meter_recv_frame[0]);
        }
        else
        {
            OSMutexFree(&SIGNAL_METER_UART);
            return 0;
        }
    }
    else
    {
        OSMutexFree(&SIGNAL_METER_UART);
        return 0;
    }
    OSMutexFree(&SIGNAL_METER_UART);
    
    frame_len = make_gb645_2007_frame(meter_send_buffer,meter_no,0x11,0x04000102,NULL,0);
    OSMutexPend(&SIGNAL_METER_UART);
    if(send_wait_response(0,meter_send_buffer,frame_len,200))
    {
        if(unzip_gb645_data(meter_recv_frame,0x04000102,meter_recv_frame)>=3)
        {
            datetime.hour = BCD2byte(meter_recv_frame[2]);
            datetime.minute = BCD2byte(meter_recv_frame[1]);
            datetime.second = BCD2byte(meter_recv_frame[0]);
        }
        else
        {
            OSMutexFree(&SIGNAL_METER_UART);
            return 0;
        }
    }
    else
    {
        OSMutexFree(&SIGNAL_METER_UART);
        return 0;
    }
    OSMutexFree(&SIGNAL_METER_UART);
	
    if(datetime.month != g_app_run_data.gprs_bytes.cur_date[1]) //这里的意思是如果新的一月我们需要清零。
    {
        memset(&g_app_run_data.gprs_bytes,0x00,sizeof(tagAPP_GPRS_BYTES));   
		fwrite_ertu_params(EEADDR_STAT_FLOW,&g_app_run_data.gprs_bytes,sizeof(tagAPP_GPRS_BYTES));   //每天我们写一次数据		   
    }
    if(0!=compare_string(g_app_run_data.gprs_bytes.cur_date,&(datetime.day),3))
    {
    	mem_cpy(g_app_run_data.gprs_bytes.cur_date,&(datetime.day),3);
    	fwrite_ertu_params(EEADDR_STAT_FLOW,&g_app_run_data.gprs_bytes,sizeof(tagAPP_GPRS_BYTES));   //每天我们写一次数据
    }
    
    
    #ifndef __SOFT_SIMULATOR__
    tpos_set_time(&datetime);
    #endif
    gSystemInfo.clock_ready = 1;
    LED_local_on(1);
    return 0;
}
void update_ertu_param(void)
{
    uint16_t frame_len,data_len;
    INT32U addr;
    uint8_t address_len;
	uint8_t tmp_address[4]={0};
    uint8_t set_flag = 0;

    //DT_F89:终端地址
    frame_len = make_gb645_2007_frame(meter_send_buffer,gSystemInfo.meter_no,0x11,0x04C80604,NULL,0);
    if(send_wait_response(0,meter_send_buffer,frame_len,50))
    {
        if(GetParamAddrLen(DT_F89,&addr,&data_len))
        {
            if(unzip_gb645_data(meter_recv_frame,0x04C80604,meter_recv_frame)>0)
            {
                address_len = meter_recv_frame[0];
				#ifdef __RTUADDR_IS_6BYTE__
				data_len = 2;
				#endif
                if(address_len == data_len)				
                {
                    set_flag = 0x55;
                    #ifdef __RTUADDR_IS_6BYTE__
					mem_cpy(meter_recv_frame+3,gmeter_doc.meter_no,4);
					data_len = 6;
					#else
					mem_cpy(tmp_address,meter_recv_frame+3,2);
					mem_cpy(tmp_address+2,meter_recv_frame+1,2);
					mem_cpy(meter_recv_frame+1,tmp_address,4);
					#endif
					set_ertu_params(DT_F89,meter_recv_frame+1,&data_len);
                }
            }
        }
    }

    //DT_F1:终端上行通信口通信参数
    frame_len = make_gb645_2007_frame(meter_send_buffer,gSystemInfo.meter_no,0x11,0x04C80601,NULL,0);
    if(send_wait_response(0,meter_send_buffer,frame_len,50))
    {
        if(GetParamAddrLen(DT_F1,&addr,&data_len))
        {
            if(unzip_gb645_data(meter_recv_frame,0x04C80601,meter_recv_frame)==data_len)
            {
                if(!check_is_all_FF(meter_recv_frame,data_len))
                {
                    set_flag = 0x55;
                    set_ertu_params(DT_F1,meter_recv_frame,&data_len);
                }
            }
        }
    }

    //DT_F3:主站IP地址和端口
    frame_len = make_gb645_2007_frame(meter_send_buffer,gSystemInfo.meter_no,0x11,0x04C80602,NULL,0);
    if(send_wait_response(0,meter_send_buffer,frame_len,50))
    {
        if(GetParamAddrLen(DT_F3,&addr,&data_len))
        {
            if(unzip_gb645_data(meter_recv_frame,0x04C80602,meter_recv_frame)==data_len)
            {
                if(!check_is_all_FF(meter_recv_frame,data_len))
                {
                    set_flag = 0x55;
                    set_ertu_params(DT_F3,meter_recv_frame,&data_len);
                }
            }
        }
    }

    //DT_F16:虚拟专网用户名、密码
    frame_len = make_gb645_2007_frame(meter_send_buffer,gSystemInfo.meter_no,0x11,0x04C80603,NULL,0);
    if(send_wait_response(0,meter_send_buffer,frame_len,50))
    {
        if(GetParamAddrLen(DT_F16,&addr,&data_len))
        {
            if(unzip_gb645_data(meter_recv_frame,0x04C80603,meter_recv_frame)==data_len)
            {
                if(!check_is_all_FF(meter_recv_frame,data_len))
                {
                    set_flag = 0x55;
                    set_ertu_params(DT_F16,meter_recv_frame,&data_len);
                }
            }
        }
    }

    if(set_flag == 0x55)
    {
        g_meter_set_flag = 0x55;
        fwrite_ertu_params(EEADDR_METER_SETFLAG,&g_meter_set_flag,1);
    }
}
#endif
/*+++
  功能：把标准645报文变换为 C L Data  并且Data做减33处理
  参数：
         INT8U *gb645  标准645报文 ，第一个字符0x68
         INT16U frameLen   报文长度
  返回：
         处理后长度
  描述：
        1）对于抄读的否定报文，返回长度0
---*/
INT16U  unwrap_gb645_frame(INT8U *gb645,INT16U frameLen)
{
    INT16U idx;
    //resp:  68 XX XX XX XX XX XX 68 C DL
    if(gb645[POS_GB645_CTRL] & 0x40) frameLen = 0;  //处理否定报文

    if(frameLen > 8)
    {
           frameLen = gb645[9] + 2;       //加上2字节的C和L。
           mem_cpy(gb645,gb645+8,frameLen);
           for(idx=2;idx<frameLen;idx++) gb645[idx]-=0x33;
    }
    return frameLen;
}
/*+++
  功能：把标准645报文变换为 C L Data  并且Data做减33处理
  参数：
         INT8U *gb645  标准645报文 ，第一个字符0x68
         INT16U frameLen   报文长度
  返回：
         处理后长度
  描述：
        1）对于抄读的否定报文，返回长度0
---*/
INT16U  decode_gb645_frame(INT8U *gb645,INT16U frameLen)
{
    INT16U idx;
    //resp:  68 XX XX XX XX XX XX 68 C DL
    if(gb645[POS_GB645_CTRL] & 0x40) frameLen = 0;  //处理否定报文

    if(frameLen > 8)
    {
           frameLen = gb645[9] + 2;       //加上2字节的C和L。
           mem_cpy(gb645,gb645+8,frameLen);
           for(idx=2;idx<frameLen;idx++) gb645[idx]-=0x33;
    }
    return frameLen;
}

void ertu_rtc_service(void)
{
    static uint32_t timer=0;
    
    if(gSystemInfo.clock_ready == 0)
    {
        if(second_elapsed(timer)>3)
        {
            update_ertu_datetime(1);
        }
    }
    else
    {
        update_ertu_datetime(0);
    }
}
#ifndef __OOP_G_METER__
void update_meter_param(void)
{
    uint8_t idx,data_pos,frame_len;
    uint8_t *itemData=NULL,*tmp_addr=NULL;
    uint32_t item;
    static uint8_t try_count[4] = {0};

    if(g_meter_set_flag != 0x55)    return;
    if(gSystemInfo.ertu_param_change != 0)
    {
        for(idx=0;idx<4;idx++)
        {
            if(get_bit_value(&(gSystemInfo.ertu_param_change),1,idx))
            {
                itemData = meter_send_buffer+200;
                data_pos=0;
                itemData[data_pos++] = 0x02;
                itemData[data_pos++] = 0x00;
                itemData[data_pos++] = 0x00;
                itemData[data_pos++] = 0x00;
                itemData[data_pos++] = 0x78;
                itemData[data_pos++] = 0x56;
                itemData[data_pos++] = 0x34;
                itemData[data_pos++] = 0x12;
                switch(idx)
                {
                    case 0:         //DT_F1
                    item = 0x04C80601;
                    fread_ertu_params(EEADDR_SET_F1,itemData+data_pos,6);
                    data_pos += 6;
                    break;
                    case 1:         //DT_F3
                    item = 0x04C80602;
                    fread_ertu_params(EEADDR_SET_F3,itemData+data_pos,28);
                    data_pos += 28;
                    break;
                    case 2:         //DT_F16
                    item = 0x04C80603;
                    fread_ertu_params(EEADDR_SET_F16,itemData+data_pos,64);
                    data_pos += 64;
                    break;
                    case 3:         //DT_F89
                    item = 0x04C80604;
                    tmp_addr = itemData+90;
                    #ifndef __RTUADDR_IS_6BYTE__
                        itemData[data_pos++] = 4;
                        fread_ertu_params(EEADDR_CFG_DEVID,tmp_addr,4);
                        mem_cpy(itemData+data_pos,tmp_addr+2,2);
                        mem_cpy(itemData+data_pos+2,tmp_addr,2);
                        data_pos += 4;
                    #else
                        itemData[data_pos++] = 2;
                        fread_ertu_params(EEADDR_CFG_DEVID,itemData+data_pos,2);
                        data_pos += 2;
                    #endif
                    break;
                }
                frame_len = make_gb645_2007_frame(meter_send_buffer,gSystemInfo.meter_no,0x14,item,itemData,data_pos);

                if(send_wait_response(0,meter_send_buffer,frame_len,50))
                {
                    if(meter_recv_frame[POS_GB645_CTRL]==0x94)
                    {
                        try_count[idx] = 0;
                        clr_bit_value(&(gSystemInfo.ertu_param_change),1,idx);
                    }
                    else
                    {
                        try_count[idx]++;
                        if(try_count[idx]>=3)
                        {
                            try_count[idx] = 0;
                            clr_bit_value(&(gSystemInfo.ertu_param_change),1,idx);
                        }
                    }
                }
            }
        }
    }
}
#endif
//下面我们在这里去统计GPRS的流量
INT32U get_ertu_month_bytes(void)
{
    INT32U month_bytes=0;
    tagDatetime datetime;
    
    
    os_get_datetime(&datetime);
    //tpos_enterCriticalSection();
    if(check_is_all_FF(g_app_run_data.gprs_bytes.cur_date,3) || check_is_all_ch(g_app_run_data.gprs_bytes.cur_date,3,0x00))
    {
        mem_cpy(g_app_run_data.gprs_bytes.cur_date,&datetime+DAY,3);
    }
    mem_cpy(&month_bytes,g_app_run_data.gprs_bytes.month_bytes,4);
    //tpos_leaveCriticalSection();
    return month_bytes;
}
void set_ertu_month_bytes(INT32U month_bytes)
{
    tpos_enterCriticalSection();
    mem_cpy(g_app_run_data.gprs_bytes.month_bytes,&month_bytes,4);
    tpos_leaveCriticalSection();
}

//预置拉合闸时段处理
void app_swtich_on_off_meter(void)
{
    INT8U idx=0,ctrl=0,len=0;
    INT8U frame[50]={0},resp[50]={0};
    static INT8U flag=0;
    tagDatetime datetime;
    
    os_get_datetime(&datetime);
    for(idx=0;idx<1;idx++)
    {
        if((datetime.hour == BCD2byte(g_app_run_data.meter_control.turn_off[1]))&&
            (datetime.minute == BCD2byte(g_app_run_data.meter_control.turn_off[0]))&&(flag!=0xAA))
        {
            //跳闸
            ctrl = 0x1A;
        }
        if((datetime.hour == BCD2byte(g_app_run_data.meter_control.turn_on[1]))&&
            (datetime.minute == BCD2byte(g_app_run_data.meter_control.turn_on[0]))&&(flag!=0x55))
        {
            //合闸
            ctrl = 0x1C;
        }
        if(ctrl)
        {
            len = make_gb645_2007_frame_ctrl(frame,ctrl);
            for(idx=0;idx<5;idx++)
            {
                len = app_trans_send_meter_frame(frame,len,resp,50,100);
                if((len)&&(resp[8]==0x9C))
                {
                    if(ctrl==0x1A)
                    {
                        flag = 0xAA;
                    }
                    else
                    {
                        flag = 0x55;
                    }       
                    break;
                }
            }
        }
    
    }

}



 //设置电表信号强度和在线状态  上线触发设置 获取gprs模块强度时触发设置
 //信号强度指示
 // 0x0000  空
 // 0x0100  信号强度1格     < 11
 // 0x0200  信号强度2格     < 16
 // 0x0203  信号强度3格     < 20
 // 0x0204  信号强度4格     >=20
 void  set_meter_csq(void)
 {
	 INT16U baudrate;
	 INT8U csq,frame[60],frameLen;
	 INT8U data[10]={0},count,data_pos=0;

	 if(gSystemInfo.csq < 11 )
	 csq = 1;
	 else if(gSystemInfo.csq < 16 )
	 csq = 2;
	 else if(gSystemInfo.csq < 20 )
	 csq = 3;
	 else if(gSystemInfo.csq < 99)
	 csq = 4;
	 else
	 csq = 0;
	 
	 data[data_pos++]= 0x02;//使用电表默认密码123456 权限02 避免电表密码闭锁
	 data[data_pos++]= 0x56;
	 data[data_pos++]= 0x34;
	 data[data_pos++]= 0x12;
	 data[data_pos++]= 0x00;
	 data[data_pos++]= 0x00;
	 data[data_pos++]= 0x00;
	 data[data_pos++]= 0x00;
	 data[data_pos] = csq;
	 if(gSystemInfo.login_status)
	 {
		 data[data_pos++] |= 0x80;
	 }
	 else
	 {
		 data[data_pos++] &= 0x7F;	 
	 }
	 frameLen = make_gb645_2007_frame(frame,gSystemInfo.meter_no,0x14,0x04001301,data,data_pos);
	 count = 2;
	 while(count--)
	 {
		 if(app_trans_send_meter_frame(frame,frameLen,frame+30,0xFF,100)>0) break;
	 }
 }
 
#ifdef __OOP_G_METER__
extern void read_meter_fixed_time_interval_hold_data(READ_PARAMS *read_param,INT8U meter_list_idx,BOOLEAN is_patch,INT8U hold_list_idx);
void rebuilt_task_plan_index();
#endif

INT8U buf[4096];


void task_meter_service(void )
{
 INT32U time_tick=0,last_status=0,last_tick=0,last_minute=0;
 INT8U i,err=0,status=0; 
 tagDatetime datetime;  
    DateTime cur_dt;
INT8U event_report_idx=0;
#ifdef __HJ_212__
	INT32U HJ212_time_last=0;
	INT8U HJ212_poweron=0;
	INT16U HJ212_frame_len=0;
#endif
    last_status = g_app_run_data.status;  
    check_read_meter_no();
    update_ertu_datetime(1);

#if !defined(__ONENET__) && !defined(__OOP_G_METER__)
    update_ertu_param();
	
	#endif
    #ifdef __OOP_G_METER__ 
	//read_gprs_param_from_oop_meter();
	#endif
    get_ertu_month_bytes();   //得到GPRS数据流量
#ifdef __EDP
	edp_warningTime =system_get_tick10ms();   //上电需要拿到edp 告警上报的初始时间。
	#ifdef  __BEIJING_ONENET_EDP__
		powerOnRebuildWaningInfo();   //上电重建告警信息状态，将数据存储在全局变量gsysteminfo 里。
	#endif
#endif
#ifdef __OOP_G_METER__
	rebuilt_task_plan_index();
	INT32U tick = system_get_tick10ms();
	//read_meter_fixed_time_interval_hold_data(&read_params,1, FALSE, 0);
	check_read_cycle_one_task();
#ifdef   __OOP_GMETER_ERTU__
	  fetch_meter_no_from_ertu();
#endif			  
#endif
	//=================================================================================================//
	//INT8U buf[200];
	//for(int i=0;i<21;i++)
	//{
		//nor_flash_read_data(FLASH_SECTOR_METER_DATA_START+PAGE_ID_MINUTE_HOLD+i,0,buf,4096);
		//system_debug_data(&i,4);
		//system_debug_info("\r\n=====================================================\r\n");
		//system_debug_data(buf,4096);
		//system_debug_info("\r\n=====================================================\r\n");
	//}


	//==================================================================================================//
    while(1)
    {
#ifdef __OOP_G_METER__
		tpos_datetime(&cur_dt);

		/* 方案任务变化，等待超过某个时间30S吧 */
		if( (gSystemInfo.plan_task_chg) && (diff_sec_between_dt(&cur_dt, &gSystemInfo.dt) > 30) )
		{
			delete_acq_data(FLASH_SECTOR_METER_DATA_START,FLASH_SECTOR_METER_DATA_START+MAX_ONE_METER_DATA_PAGE_COUNT);
			gSystemInfo.plan_task_chg = 0;
			rebuilt_task_plan_index();
			check_read_cycle_one_task();
		}
#endif
        ClrTaskWdt();
        ertu_rtc_service();
#ifdef __OOP_G_METER__
		prepare_read_oop_data_cycle(&read_params);
#ifdef 	__PROVINCE_JIBEI_FUHEGANZHI__
		load_sensor_data_read_store();
#endif
#endif
//分钟级??
        if(((system_get_tick10ms()-last_tick)>100*60)||(gSystemInfo.login_status!=status))
        {
#ifndef  __OOP_G_METER__
			set_meter_csq();		   //设置信号强度。
#endif
			status = gSystemInfo.login_status;
			//tpos_mutexPend(&SIGNAL_RUN_DATA);
        	//if(last_status!=g_app_run_data.status)
        	//{
				//if(update_meter_status(g_app_run_data.status))
				//{
					//last_status = g_app_run_data.status;
				//}
        	//}
        	//tpos_mutexFree(&SIGNAL_RUN_DATA);
        	last_tick = system_get_tick10ms();	
        }
        
#if !defined(__ONENET__) && !defined(__OOP_G_METER__)
        update_meter_param();
#endif
//#ifdef __ZHEJIANG_ONENET_EDP__// 浙江edp上报事件处理
        //app_check_meter_record_event();
    ////    app_check_meter_status();  //用来上报时钟电压
//#endif
        #ifdef __CTRL_METER__  //
        if(g_app_run_data.meter_control.is_control_valid==1)
        {
            app_swtich_on_off_meter();
        }
        #endif

#ifdef __EDP__  //处理edp的上报告警和间隔上报数据。
	vmeter_edp_warning_report();
#endif
#ifdef __HJ_212__
	if((gSystemInfo.login_status==1)&&(!HJ212_poweron))
	{
		if((system_get_tick10ms()-HJ212_time_last)>100*20*1)
		{  
			//HJ212_frame_len = HJ212_ertu_request_time();
			//remote_send_app_frame(HJ212_request_buf,HJ212_frame_len);
			HJ212_time_last = system_get_tick10ms();
			
			//HJ212_frame_len = report_212_monitor_xinhe();
			//remote_send_app_frame(HJ212_request_buf,HJ212_frame_len);
			HJ212_frame_len = report_212_poweroff_xinhe();
			remote_send_app_frame(HJ212_request_buf,HJ212_frame_len);			
		}
	  HJ212_poweron=1;

	}
	//主动上报
	//停上电上报
	

#endif
       

 #ifdef __OOP_G_METER__
		powerOnOff();
		if(gSystemInfo.tcp_link) //登上tcp后开始上报事件
		{
			for(event_report_idx=0;event_report_idx<15;event_report_idx++)
			{
				if(TerminalPowerOnOffEvent.event[event_report_idx].has_report ==0xAA)
				{
					reportOnOffEvent(event_report_idx);
					TerminalPowerOnOffEvent.event[event_report_idx].has_report = 0xBB;
					fwrite_ertu_params(TERMINAL_POWER_ONOFF,&TerminalPowerOnOffEvent,sizeof(StrTerminalPowerOnOffEventBlock));
				}
			}
		}
 #endif    
        tpos_TASK_YIELD();
	 
    }
}
