#include "main_include.h"
#include "stdio.h"
#include "string.h"

#define OS_TIME system_get_tick10ms()
uint8_t gprs_check_tcp_status(void);
static uint8_t gprs_cmd_buffer[200];
static uint8_t report_info[30];//report info header
volatile INT8S tcp_server_socket_id =-1, tcp_server_socket_id_old=-1 , server_has_on=0,need_reset = 0, check_socket_now=0;
volatile INT8S tcp_client_socket_id = -1;//榛樿浣跨敤1
#define MAX_SOCKET_ID 3 //A9600 socket id 1-3
int16_t gprs_send_cmd_wait_OK(uint8_t *cmd,uint8_t *resp,uint16_t max_resp_len,uint16_t timeout_10ms);
volatile INT16S has_data_to_read;
int16_t str2hex(INT8U *str, INT16U len, INT8U *hex);
//typedef struct{
//    uint8_t *send_ptr;
//    uint16_t send_len;
//    uint16_t recv_pos;
//    uint16_t read_pos;
//    uint8_t  recv_buf[SIZE_OF_APP_PROTOCOL_FRAME];
//    //uint8_t  send_buf[SIZE_OF_APP_PROTOCOL_FRAME];
//
//    //uint8_t  need_resp;//1 ：需要等待回复后再读取下一条
//    uint8_t  ref;
//    uint16_t mid;
//    uint8_t  result;
//    uint16_t objid;
//    int16_t  insid;
//    int16_t  resid;
//    uint8_t  type;
//    uint16_t len;
//    uint8_t  index;
//    uint8_t  flag;
//	uint8_t  msg_type;// 1 read 2 write 3 execute 4 notify
//    uint8_t  state;
//}tagWirelessObj;
tagWirelessObj GprsObj, GprsServerObj;
INT8U ex_cmd_apply, ex_cmd_allow;
//关闭低功耗PSM
uint8_t gprs_onenet_close_PSM(void)
{
    int8_t ret;

    sprintf((char *)gprs_cmd_buffer,"AT+CPSMS=0\r\n");
    ret = gprs_send_cmd_wait_OK(gprs_cmd_buffer, gprs_cmd_buffer, sizeof(gprs_cmd_buffer), 500);
    if(ret > 0)
    {
        ret = str_find(gprs_cmd_buffer, ret,(INT8U*) "OK\r\n", 4);
        if(ret >= 0)//
        {
            return 1;
        }
    }
    else if(ret == -2)
    {}
    else
    {
        return 0;
    }
    return 0;
}



//关闭低功耗eDRX
uint8_t gprs_onenet_close_eDRX(void)
{
    int8_t ret;

    sprintf((char *)gprs_cmd_buffer,"AT+CEDRXS=0,5\r\n");
    ret = gprs_send_cmd_wait_OK(gprs_cmd_buffer, gprs_cmd_buffer, sizeof(gprs_cmd_buffer), 500);
    if(ret > 0)
    {
        ret = str_find(gprs_cmd_buffer, ret,(INT8U*) "OK\r\n", 4);
        if(ret >= 0)//
        {
            return 1;
        }
    }
    else if(ret == -2)
    {}
    else
    {
        return 0;
    }
    return 0;
}
uint8_t gprs_onenet_set_eDRX(void)
{
    int8_t ret;

    sprintf((char *)gprs_cmd_buffer,"AT+NPTWEDRXS=1,5,0101,0001\r\n");//PTW 0101 15.36second eDRX 0001 20.48second
    ret = gprs_send_cmd_wait_OK(gprs_cmd_buffer, gprs_cmd_buffer, sizeof(gprs_cmd_buffer), 500);
    if(ret > 0)
    {
        ret = str_find(gprs_cmd_buffer, ret,(INT8U*) "OK\r\n", 4);
        if(ret >= 0)//
        {
            return 1;
        }
    }
    else if(ret == -2)
    {}
    else
    {
        return 0;
    }
    return 0;
}
//创建设备实体
uint8_t gprs_onenet_object_creat(void)
{
    int8_t ret;
    uint8_t data[82]={0},ip[4]={0};
	
	
    mem_set(gprs_cmd_buffer,200,0x00);
    mem_cpy(ip,gSystemInfo.ip1,4);
    //读取参数长度 长度为0或FF无参数 无参数时使用默认上线参数 
    fread_ertu_params(EEADDR_CONFIG_ONENET,data,1);
    if((data[0]==0)||(data[0]==0xFF))
    {
        //sprintf((char *)gprs_cmd_buffer,"AT+MIPLCREATE=57,130039F10003F2002B040011000000000000001A6E62696F746163632E6865636C6F7564732E636F6D3A35363833000131F300087100000000,0,57,0\r\n");//重庆公网    
        //sprintf((char *)gprs_cmd_buffer,"AT+MIPLCREATE=49,130031F10003F2002304001100000000000000123131322E31332E3136372E36333A35363833000131F300087100000000,0,49,0\r\n");//浙江公网
    	sprintf((char *)gprs_cmd_buffer,"AT+MIPLCREATEEX=\"112.13.167.63\",0\r\n");
    }
    else
    {
        //sprintf((char *)gprs_cmd_buffer,"AT+MIPLCREATE=64,130040F10003F200320400110010434D4E42494F544F4E454E45542E5A4A0000000000113138382E3130322E352E32373A35363833000131F300087100000000,0,64,0\r\n");//专网有APN
    	//sprintf((char *)gprs_cmd_buffer,"AT+MIPLCREATE=48,130030F10003F2002204001100000000000000113138382E3130322E352E32373A35363833000131F300087100000000,0,48,0\r\n");//专网无APN
    	//sprintf((char *)gprs_cmd_buffer,"AT+MIPLCREATEEX=\"188.102.2.14\",0\r\n");
    	if(check_is_valid_ipaddr(ip))
    	{
    		sprintf((char *)gprs_cmd_buffer,"AT+MIPLCREATEEX=\"%d.%d.%d.%d\",0\r\n",ip[0],ip[1],ip[2],ip[3]);	
    	}
    	else
    	{
    		sprintf((char *)gprs_cmd_buffer,"AT+MIPLCREATEEX=\"188.102.5.27\",0\r\n");
    	}
    }
    ret = gprs_send_cmd_wait_OK(gprs_cmd_buffer, gprs_cmd_buffer, sizeof(gprs_cmd_buffer), 1000);
    if(ret > 0)
    {
        ret = str_find(gprs_cmd_buffer, ret,(INT8U*) "OK\r\n", 4);
        if(ret >= 0)//
        {
            return 1;
        }
    }
    else if(ret == -2)
    {}
    else
    {
        return 0;
    }
    return 0;
}

//删除设备实体
uint8_t gprs_onenet_object_delete(INT16U ref)
{
    int8_t ret;

    sprintf((char *)gprs_cmd_buffer,"AT+MIPLDELETE=%d\r\n",ref);
    ret = gprs_send_cmd_wait_OK(gprs_cmd_buffer, gprs_cmd_buffer, sizeof(gprs_cmd_buffer), 300);
    if(ret > 0)
    {
        ret = str_find(gprs_cmd_buffer, ret,(INT8U*) "OK\r\n", 4);
        if(ret >= 0)//
        {
            return 1;
        }
    }
    else if(ret == -2)
    {}
    else
    {
        return 0;
    }
    return 0;
}
//订阅Object组配置流程
uint8_t gprs_onenet_object_observe(INT16U obj,INT8U instance,INT8U *state)
{
    int8_t ret;


    sprintf((char *)gprs_cmd_buffer,"AT+MIPLADDOBJ=0,%d,%d,\"%s\",0,0\r\n",obj,instance,state);
    ret = gprs_send_cmd_wait_OK(gprs_cmd_buffer, gprs_cmd_buffer, sizeof(gprs_cmd_buffer), 500);
    if(ret > 0)
    {
        ret = str_find(gprs_cmd_buffer, ret,(INT8U*) "OK\r\n", 4);
        if(ret >= 0)//
        {
            return 1;
        }
    }
    else if(ret == -2)
    {}
    else
    {
        return 0;
    }
    return 0;
}

uint8_t gprs_onenet_resource_observe(INT16U obj,INT16U res)
{
    int8_t ret;

    sprintf((char *)gprs_cmd_buffer,"AT+MIPLDISCOVERRSP=0,%d,1,4,\"%d\"\r\n",obj,res);
    ret = gprs_send_cmd_wait_OK(gprs_cmd_buffer, gprs_cmd_buffer, sizeof(gprs_cmd_buffer), 300);
    if(ret > 0)
    {
        ret = str_find(gprs_cmd_buffer, ret,(INT8U*) "OK\r\n", 4);
        if( ret >= 0)//
        {
            return 1;
        }
    }
    else if(ret == -2)
    {}
    else
    {
        return 0;
    }
    return 0;
}
uint8_t gprs_get_eid(void)
{
    int16_t resault;
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_CMI))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CEID\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }    

    if(resault >= 0)
    {
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 1;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_onenet_resource_observe_list(INT16U obj,INT8U *source)
{
    int8_t ret;

    sprintf((char *)gprs_cmd_buffer,"AT+MIPLDISCOVERRSP=0,%d,1,19,\"%s\"\r\n",obj,source);
    ret = gprs_send_cmd_wait_OK(gprs_cmd_buffer, gprs_cmd_buffer, sizeof(gprs_cmd_buffer), 300);
    if(ret > 0)
    {
        ret = str_find(gprs_cmd_buffer, ret,(INT8U*) "OK\r\n", 4);
        if( ret >= 0)//
        {
            return 1;
        }
    }
    else if(ret == -2)
    {}
    else
    {
        return 0;
    }
    return 0;
}
uint8_t gprs_onenet_resource_delete(INT16U obj)
{
    int8_t ret;

    sprintf((char *)gprs_cmd_buffer,"AT+MIPLDELOBJ=0,%d\r\n",obj);
    ret = gprs_send_cmd_wait_OK(gprs_cmd_buffer, gprs_cmd_buffer, sizeof(gprs_cmd_buffer), 300);
    if(ret > 0)
    {
        ret = str_find(gprs_cmd_buffer, ret,(INT8U*) "OK\r\n", 4);
        if( ret >= 0)//
        {
            return 1;
        }
    }
    else if(ret == -2)
    {}
    else
    {
        return 0;
    }
    return 0;
}
//查询当前驻留小区相关信息
uint8_t gprs_onenet_check_net(void)
{
    int8_t ret;

    sprintf((char *)gprs_cmd_buffer,"AT+NUESTATS\r\n");
    ret = gprs_send_cmd_wait_OK(gprs_cmd_buffer, gprs_cmd_buffer, sizeof(gprs_cmd_buffer), 200);
    if(ret > 0)
    {
        ret = str_find(gprs_cmd_buffer, ret,(INT8U*) "OK\r\n", 4);
        if( ret >= 0)//
        {
            return 1;
        }
    }
    else if(ret == -2)
    {}
    else
    {
        return 0;
    }
    return 0;
}

uint8_t update_rsrp_sinr_cellid(INT8U* buffer,INT16S len)
{
    int16_t pos=0,pos1=0,idx;
    char info[50],id[10];
    
    if(len>15)
    {
        pos = str_find(buffer,len,(uint8_t*)"CELL",4);
        if(pos < 0) return 0;

        buffer += (pos+4);
        len -= (pos+4);
        if(len <= 0)    return 0;
        pos = str_find(buffer,len,(uint8_t*)",",1);//earfcn
        if(pos < 0) return 0;
        
        buffer += (pos+4);
        len -= (pos+4);
        if(len <= 0)    return 0;
        pos = str_find(buffer,len,(uint8_t*)",",1);//phy cell id
        if(pos < 0) return 0;
        
        buffer += (pos+1);
        len -= (pos+1);
        if(len <= 0)    return 0;
        pos = str_find(buffer,len,(uint8_t*)",",1);//pri cell 
        if(pos < 0) return 0;
        
        buffer += (pos+1);
        len -= (pos+1);
        if(len <= 0)    return 0;
        pos = str_find(buffer,len,(uint8_t*)",",1); //rsrp  
        if(pos < 0) return 0;
        
        buffer += (pos+1);
        len -= (pos+1);  
        if(len <= 0)    return 0;
        pos1 = str_find(buffer,len,(uint8_t*)",",1); //rsrq  
        if(pos1 < 0) return 0;

        mem_set(gSystemInfo.rsrp, 4, 0);
        mem_cpy(gSystemInfo.rsrp, buffer, pos1);
        pos=pos1;
        
        mem_cpy(id, gSystemInfo.rsrp, 4);
        id[4]=0;
        sprintf(info, "\r\n RSRP:%s\r\n", id);
        system_debug_data(info, strlen(info));
                
        buffer += (pos+1);
        len -= (pos+1);  
        if(len <= 0)    return 0;
        pos = str_find(buffer,len,(uint8_t*)",",1); //rssi 
        if(pos < 0) return 0;
        
        buffer += (pos+1);
        len -= (pos+1);  
        if(len <= 0)    return 0;
        pos = str_find(buffer,len,(uint8_t*)",",1); //snr
        if(pos < 0) return 0;
        
        buffer += (pos+1);
        len -= (pos+1);  
        if(len <= 0)    return 0;
        pos1 = str_find(buffer,len,(uint8_t*)"\r\n",2); //snr 
        if(pos1 < 0) return 0;

        mem_set(gSystemInfo.sinr, 4, 0);
        mem_cpy(gSystemInfo.sinr, buffer, pos1);
           
        mem_cpy(id, gSystemInfo.sinr, 4);
        id[4]=0;
        sprintf(info, "\r\n SNR:%s\r\n", id);
        system_debug_data(info, strlen(info));
    }
    return 0;    
}

uint8_t gprs_get_rsrp_sinr_cellid(void)
{
	static uint32_t tmp=0;
	int16_t resault,pos,pos1;
    char info[50];
    char id[10];
	
    if(second_elapsed(tmp)<60)
    {
		return 0;
	}
	else
	{
		tmp = system_get_tick10ms();
	}
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+TUESTATS=\"CELL\"\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_CMI))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+NUESTATS=CELL\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }    
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        //resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+QCCID\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    else
    {
        //resault = gprs_send_cmd_wait_OK((uint8_t *)"AT$MYCCID\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    if(resault >= 0)
    {
        update_rsrp_sinr_cellid(gprs_cmd_buffer, resault);
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 1;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }

	
 	resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CEREG?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        if( (pos = str_find(gprs_cmd_buffer,resault,(uint8_t*)",", 1)) >= 0 )
        {
            if(pos = str_find(gprs_cmd_buffer + pos +1, resault-pos-1,(uint8_t*)"1", 1) >= 0 )
            {
                if((gSystemInfo.nouse_1376_3 == 0xAA)&&((gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG)
					||(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_CMI)))
                {
                    pos1 = 0;
                    if( (pos = str_find(gprs_cmd_buffer+pos1,resault-pos1,(uint8_t*)",", 1)) >= 0 )
                    {
                        pos1 += pos+1;
                        if(resault>pos1 && (pos = str_find(gprs_cmd_buffer+pos1,resault-pos1,(uint8_t*)",", 1)) >= 0 )
                        {
                           pos1 += pos+1;
                           if(resault>pos1 && (pos = str_find(gprs_cmd_buffer+pos1,resault-pos1,(uint8_t*)",", 1)) >= 0 )
                           {
                               pos1 += pos+1;
                               if(resault >= pos1+8)
                                   mem_cpy(gSystemInfo.cellid, gprs_cmd_buffer+pos1, 8);
							   return 1;
                           }
                        }
                    }
                }
            }
        }
    }
	else
	{
		return 0;
	}
	return 1;

}
uint8_t gprs_close_cfun(void)
{
    int16_t resault;
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CFUN=0\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),500);
    if(resault >= 0)
    {
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 1;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}











int16_t remote_read_byte(void)
{
    int16_t recv;
    
    if(GprsObj.read_pos == GprsObj.recv_pos)
    {
        return -1;
    }

    //system_debug_info("read remote\r\n");
    recv = GprsObj.recv_buf[GprsObj.read_pos];
    GprsObj.read_pos ++;
    if(GprsObj.read_pos >= sizeof(GprsObj.recv_buf))
    {
        GprsObj.read_pos = 0;
    }
    return recv;
}
int16_t remote_server_read_byte(void)
{
    int16_t recv;
    char info[20];
    
    if(GprsServerObj.read_pos == GprsServerObj.recv_pos)
    {
        return -1;
    }
    //sprintf(info, "r ser %d\r\n", GprsServerObj.read_pos);
    //system_debug_info(info);
    recv = GprsServerObj.recv_buf[GprsServerObj.read_pos];
    GprsServerObj.read_pos ++;
    if(GprsServerObj.read_pos >= sizeof(GprsServerObj.recv_buf))
    {
        GprsServerObj.read_pos = 0;
    }
    return recv;
}
void gprs_power_off(void)
{
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600))
    {
        if(tcp_client_socket_id > 0)
        {
            sprintf(gprs_cmd_buffer, "AT+MIPCLOSE=%d\r\n", tcp_client_socket_id);
            gprs_send_cmd_wait_OK((uint8_t *)gprs_cmd_buffer, NULL,0,100);
        }
        if(tcp_server_socket_id > 0)
        {
            sprintf(gprs_cmd_buffer, "AT+MIPCLOSE=%d\r\n", tcp_server_socket_id);
            gprs_send_cmd_wait_OK((uint8_t *)gprs_cmd_buffer, NULL,0,100);
        }
        
        gprs_send_cmd_wait_OK((uint8_t *)"AT+CPOF\r",NULL,0,100);
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        gprs_send_cmd_wait_OK((uint8_t *)"AT+QPOWD=1\r",NULL,0,100);
    }
    else 
    {
        gprs_send_cmd_wait_OK((uint8_t *)"AT$MYPOWEROFF\r",NULL,0,100);
    }
    gSystemInfo.ppp_is_ok = 0;
    gSystemInfo.tcp_link = 0;
    gSystemInfo.remote_client_linked = 0;
    DelayNmSec(10000);//15s蹇呴』瀹屽叏鍏虫満
    //A9600 鍏虫満鍛戒护鏈夋晥锛宱noff涓嶈捣浣滅敤锛屽鍔犲浣嶆搷浣滄浛浠ｅ紑鍏虫満
    if((gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600)||(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_M5310))
    {
        drv_pin_gprs_rst(0);
        DelayNmSec(1000); 
        drv_pin_gprs_rst(1);
//        drv_pin_gprs_onoff(0);
//        DelayNmSec(3200); //3绉
//        drv_pin_gprs_onoff(1);
    }
}
void gprs_power_on(void)
{
    drv_pin_gprs_onoff(0);
    DelayNmSec(1000); //1s卤100ms
#ifndef __METER_SIMPLE__
    GPRS_PCTRL_OUTPUT;
    GPRS_PCTRL_HIGH;
#endif
    drv_pin_gprs_onoff(1);
    DelayNmSec(10000);//10s蹇呴』瀹屾垚鍒濆鍖
}
int32_t find_first_num(INT8U *buf, INT16U len)
{
    INT16U i,find=0;
    INT32U num=0;
    for(i = 0; i<len; i++)
    {
        if(buf[i] >= '0' && buf[i]<= '9')
        {
            num = num*10;
            num += buf[i] - '0';
            find = 1;
        }
        else
        {
            if(find == 1)
                return num;
        }
    }
    return -1;
}
int32_t find_first_num_back(INT8U *buf, INT16U len)
{
    INT16U i,find=0;
    INT32U num=0;
    for(i = 0; i<len; i++)
    {
        num = num*10;
        if(buf[i] >= '0' && buf[i]<= '9')
        {
            num += buf[i] - '0';
            find = 1;
        }
        else
        {
            if(find == 1)
                return num;
        }
    }
    return -1;
}
INT16S find_last_char(INT8U *now_pos_ch, INT16U max_deep, char ch)
{
    INT16U pos=0;
    while(max_deep > 0)
    {
        if(*now_pos_ch  == ch)
            return pos;
        max_deep--;
        pos++;
    }
    return -1;
}
void  push_data_to_protocol_service(void)
{
    INT16U pos;

    RequestOnenet.recv_pos = 0;
    RequestOnenet.read_pos = 0;
    if(GprsObj.len > GprsObj.recv_pos)
    {
        pos = GprsObj.len-GprsObj.recv_pos;//后半段长度
        mem_cpy(RequestOnenet.recv_buf,GprsObj.recv_buf+(SIZE_OF_APP_PROTOCOL_FRAME-pos),pos);
        mem_cpy(RequestOnenet.recv_buf+pos,GprsObj.recv_buf,GprsObj.recv_pos);       
    }
    else
    {
        mem_cpy(RequestOnenet.recv_buf,GprsObj.recv_buf+(GprsObj.recv_pos-GprsObj.len),GprsObj.len);
    }
        
    RequestOnenet.ref = GprsObj.ref;
    RequestOnenet.mid = GprsObj.mid;
    RequestOnenet.result = GprsObj.result;
    RequestOnenet.objid = GprsObj.objid;
    RequestOnenet.insid = GprsObj.insid;
    RequestOnenet.resid = GprsObj.resid;
    RequestOnenet.type = GprsObj.type;
    RequestOnenet.len = GprsObj.len;
    RequestOnenet.index = GprsObj.index;
    RequestOnenet.flag = GprsObj.flag;
    RequestOnenet.msg_type = GprsObj.msg_type;
    RequestOnenet.state = REQUEST_STATE_WAIT_PROCESS;
    RequestOnenet.life_time = GprsObj.life_time;
    RequestOnenet.ackid = GprsObj.ackid;
    //将GprsObj置0
    GprsObj.len =0;
    GprsObj.recv_pos =0;
    ertu_month_bytes_add(RequestOnenet.len);
}
/*
 * 返回值 -1  没有收到命令响应 OK
 * 返回值 -2 收到ERROR
 * 返回值 >=0 收到OK,返回值为收到的报文长度
 */
int16_t gprs_send_cmd_wait_OK(uint8_t *cmd,uint8_t *resp,uint16_t max_resp_len,uint16_t timeout_10ms)
{
    uint32_t timer;
    uint32_t byte_timer;
    uint16_t cmd_len, num, idx, recv_pos, event;
    int16_t  rx_data;
    static uint16_t last_pos=0;
    uint8_t  cmd_buffer[8];
    int16_t  resp_len, pos=0,pos1=0;
    int16_t  resault;
    uint8_t  state=0, report_id=0;
    tagWirelessObj *Obj;
    uint16_t report_pos=0;
    uint8_t report_state;//鍖哄垎澶氫釜涓婃姤鍛戒护锛屽垎鍒鐞
    uint8_t find_start=0;
    char info[30];

    Obj = &GprsObj;
    cmd_len = str_cmdlen(cmd);
    INT8U str[30];
    
    timer = system_get_tick10ms();
    
    while(tpos_mutexRequest(&SIGNAL_REMOTE_UART_SEND) == FALSE)
    {
        if((system_get_tick10ms()-timer) > MAX_REMOTE_DELAY_10MS)
        {
            return 0;
        }
        tpos_TASK_YIELD();
    }
    gprs_uart_send_buf(cmd,cmd_len);
    timer = system_get_tick10ms();
    mem_set(cmd_buffer,8,0);
    resp_len = 0;
    resault = -1;
    report_state = 0;
    mem_set(report_info, sizeof(report_info), 0);
    while(1)
    {
        rx_data = gprs_uart_read_byte();

        if(rx_data != -1)
        {
            byte_timer = system_get_tick10ms();
            if(gSystemInfo.gprs_debug_flag)
            {
                system_debug_data((INT8U*)&rx_data, 1);
            }
            //Report..
            if(1)//(report_len == 0)
            {
                if((report_state)&&(report_pos < sizeof(report_info)))
                {
                    timer = system_get_tick10ms();
                    if((gSystemInfo.nouse_1376_3 == 0xAA) && (gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_M5310))
                    {
                        if(Obj->msg_type == ONENET_READ)
                        {
                            report_info[report_pos++] = rx_data;

                                if(state==0)
                                {
                                    if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                    {
                                        Obj->ref = str2int(report_info);
                                        report_pos=0;
                                        state++;
                                        mem_set(report_info, sizeof(report_info), 0);
                                    }
                                }
                                else if(state==1)
                                {
                                    if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                    {
                                        Obj->mid = str2int(report_info);
                                        report_pos=0;
                                        state++;
                                        mem_set(report_info, sizeof(report_info), 0);
                                    }
                                }
                            
                                else if(state == 2)
                                {
                                    if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                    {
                                        Obj->objid = str2int(report_info);
                                        report_pos=0;
                                        state++;
                                        mem_set(report_info, sizeof(report_info), 0);
                                    }
                                }
                                else if(state == 3)
                                {
                                    if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                    {
                                        Obj->insid = str2int(report_info);
                                        report_pos=0;
                                        state++;
                                        mem_set(report_info, sizeof(report_info), 0);
                                    }
                                }
                                else if(state == 4)
                                {
                                    if((pos = str_find(report_info, report_pos, "\r\n", 1)) >= 0)
                                    {
                                        Obj->resid = str2int(report_info);
                                        report_pos=0;
                                        state=0;
                                        report_state=0;
                                        push_data_to_protocol_service();
                                        mem_set(report_info, sizeof(report_info), 0);
                                        gSystemInfo.remote_status = 0;
                                    }
                                }
                        }
                        else if(Obj->msg_type == ONENET_WRITE)
                        {
                            if(state!=7)
                            {
                            report_info[report_pos++] = rx_data;
                            }
                            if(state==0)
                            {
                                if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                {
                                        Obj->ref = str2int(report_info);
                                    report_pos=0;
                                state++;
                                mem_set(report_info, sizeof(report_info), 0);
                            }
                                }
                            else if(state == 1)
                            {
                                if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                {
                                        Obj->mid = str2int(report_info);
                                    report_pos=0;
                                state++;
                                mem_set(report_info, sizeof(report_info), 0);
                            }
                                }
                            
                            else if(state == 2)
                            {
                                if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                {
                                        Obj->objid = str2int(report_info);
                                    report_pos=0;
                                state++;
                                mem_set(report_info, sizeof(report_info), 0);
                            }
                                }
                            else if(state == 3)
                            {
                                if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                {
                                        Obj->insid = str2int(report_info);
                                    report_pos=0;
                                state++;
                                mem_set(report_info, sizeof(report_info), 0);
                            }
                                }
                            else if(state == 4)
                            {
                                if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                {
                                    Obj->resid = str2int(report_info);
                                    report_pos=0;
                                    state++;
                                    mem_set(report_info, sizeof(report_info), 0);
                                }
                            }
                            else if(state == 5)
                            {
                                if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                {
                                    Obj->type = str2int(report_info);
                                    report_pos=0;
                                    state++;
                                    mem_set(report_info, sizeof(report_info), 0);
                                }
                            }      
                            else if(state == 6)
                            {
                                if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                {
                                    Obj->len = str2int(report_info);
                                    num = Obj->len;
                                    report_pos=0;
                                    state++;
                                    mem_set(report_info, sizeof(report_info), 0);
                                }
                            }
                            else if(state == 7)
                            {
                                //长度大于512分帧时
                                if(last_pos!=0)
                                {
                                	Obj->recv_pos = last_pos;
                                	last_pos = 0;
                                }
                                Obj->recv_buf[Obj->recv_pos++] = rx_data;
                                if(Obj->recv_pos >= sizeof(Obj->recv_buf))
                                    Obj->recv_pos = 0;
                                num--;
                                if(num==0)
                                {
                                    state++;
                                    report_pos=0;
                                }
                            }  
                            else if(state == 8)
                            {
                                if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                {
                                        report_pos=0;
                                        mem_set(report_info, sizeof(report_info), 0);
                                        state++;
                                    }
                            } 
                            else if(state == 9)
                            {
                                if((pos = str_find(report_info, report_pos, "\r\n", 2)) >= 0)
                                {
                                        Obj->flag =  report_info[pos-1]-0x30;//用于标志flag这个标志位，1 第一条消息 2 中间消息 0 最后一条消息                                        
                                        if((Obj->flag == 1)||(Obj->flag == 2))
                                        {
                                            state =0;
                                            report_state = 0;
                                            last_pos = Obj->recv_pos;
                                        } 
                                        else
                                        {
                                    		Obj->index = str2int(report_info);
                                    		state=0;
                                			report_state = 0;
                                            Obj->len = Obj->recv_pos;
                                			push_data_to_protocol_service();
                                			mem_set(report_info, sizeof(report_info), 0);
                                			gSystemInfo.remote_status = 0;
                                        }
                            	}
                            }                            
                        }
                        
                        else if(Obj->msg_type == ONENET_EXECUTE)
                        {
                            report_info[report_pos++] = rx_data;


                                if(state==0)
                                {
                                    if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                    {
                                        Obj->mid = str2int(report_info);
                                        report_pos=0;
                                        state++;
                                        mem_set(report_info, sizeof(report_info), 0);
                                    }
                                }
                                else if(state == 1)
                                {
                                    if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                    {
                                        Obj->objid = str2int(report_info);
                                        report_pos=0;
                                        state=2;
                                        mem_set(report_info, sizeof(report_info), 0);
                                    }
                                }
                                else if(state == 2)
                                {
                                    if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                    {
                                        Obj->insid = str2int(report_info);
                                        report_pos=0;
                                        state=3;
                                        mem_set(report_info, sizeof(report_info), 0);
                                    }
                                }
                                else if(state == 3)
                                {
                                    if((pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                    {
                                        Obj->insid = str2int(report_info);
                                        report_pos=0;
                                        state=0;
                                        report_state=0;
                                        push_data_to_protocol_service();
                                        mem_set(report_info, sizeof(report_info), 0);
                                        gSystemInfo.remote_status = 0;
                                    }
                                }
                        }                        
                    }
                    continue;
                }
                else if(report_pos >= sizeof(report_info))
                {
                    report_pos = 0;
                }
                if(report_state == 0)
                {
                    if((gSystemInfo.nouse_1376_3 == 0xAA) && (gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_M5310))
                    {
                        find_start=18;
                        mem_cpy(report_info,report_info+1,find_start);//find ""
                        report_info[find_start] = rx_data;
                        #ifndef __ONENET__
                        pos = str_find(report_info, find_start, (uint8_t *)"+NSONMI:", 8);
                        if(pos != -1)
                        {
                            report_pos = 0;
                            report_state = 1;
                            state = 0;
                            mem_set(report_info, sizeof(report_info), 0);
                            timer = system_get_tick10ms();

                            continue;
                        }
                        pos = str_find(report_info,find_start , (uint8_t *)"+NSOCLI:", 8);
                        if(pos != -1)
                        {
                            report_pos = 0;
                            report_state = 2;
                            state = 0;
                            mem_set(report_info, sizeof(report_info), 0);
                            timer = system_get_tick10ms();

                            continue;
                        }
                        #else
                        if(str_find(report_info, find_start, (uint8_t *)"+MIPLREAD:", 10) >= 0)
                        {
                            state = 0;
                            report_state = 1;
                            report_pos = 0;
                            Obj->msg_type = ONENET_READ;
                            mem_set(report_info, sizeof(report_info), 0);
                            timer = system_get_tick10ms();
                            gSystemInfo.remote_status = 1;
                            continue;
                        }
                        else if(str_find(report_info, find_start,(uint8_t *) "+MIPLWRITE:", 11) >= 0)
                        {
                            state = 0;
                            report_state = 2;
                            Obj->msg_type = ONENET_WRITE;
                            report_pos = 0;
                            mem_set(report_info, sizeof(report_info), 0);
                            timer = system_get_tick10ms();
                            gSystemInfo.remote_status = 1;
                            continue;
                        }
                        else if(str_find(report_info, find_start, (uint8_t *)"+MIPLEXECUTE:", 13) >= 0)
                        {
                            state = 0;
                            report_state = 3;
                            Obj->msg_type = ONENET_EXECUTE;
                            report_pos = 0;
                            mem_set(report_info, sizeof(report_info), 0); 
                            timer = system_get_tick10ms();
                            gSystemInfo.remote_status = 1;
                            continue;
                        }
                        else if(pos = str_find(report_info, find_start, (uint8_t *)"+MIPLEVENT:", 11) >= 0)
                        {
                            gSystemInfo.remote_status = 1;
                            if((pos = str_find(report_info, find_start, ",", 1)) >= 0)
                            {
                                if((pos1 = str_find(report_info+pos, find_start-pos, "\r\n", 2)) >= 0)
                                {
                                    event = str2int(report_info+pos+1);
                                    if(event == 6)
                                    {
                                        gSystemInfo.tcp_link = 1;
                                    }
                                    else if((event == 7)||(event == 15))
                                    {
                                        gSystemInfo.tcp_link = 0;
                                    }
                                    else if(event==11)
                                    {
                                        tpos_mutexPend(&SIGNAL_SYSTEMINFO);
                                        gSystemInfo.check_update_tick = 0;
                                        tpos_mutexFree(&SIGNAL_SYSTEMINFO);
                                    }
                                    else if(event == 26)
                                    {
                                        //上报的确认
                                    }
                                    if((pos1 = str_find(report_info+pos, find_start, ",", 1)) >= 0)
                                    {
                                        Obj->ackid = str2int(report_info+pos1+1);
                                    }
                                    report_pos = 0;
                                    mem_set(report_info, sizeof(report_info), 0);                                    
                                }
                            }
                            report_pos = 0;
                            timer = system_get_tick10ms();
                            gSystemInfo.remote_status = 0;
                            continue;                        
                        
                        }
                        else if(str_find(report_info, find_start, (uint8_t *)"+MIPLOBSERVE:", 13) >= 0)
                        {
                            gSystemInfo.last_resource_ready = system_get_tick10ms(); 
                            report_pos = 0;
                            mem_set(report_info, sizeof(report_info), 0); 
                            timer = system_get_tick10ms();
                            continue;
                        }    
                        else if(str_find(report_info, find_start, (uint8_t *)"+MIPLDISCOVER:", 14) >= 0)
                        {
                            gSystemInfo.last_resource_ready = system_get_tick10ms(); 
                            report_pos = 0;
                            mem_set(report_info, sizeof(report_info), 0); 
                            timer = system_get_tick10ms();
                            continue;
                        }                          
                        #endif
                    }//M5310

                }//if(report_state == 0)
            }
            
            if(resp != NULL)
            {
                if(resp_len < max_resp_len)
                {
                    resp[resp_len] = rx_data;
                    resp_len++;
                }
            }
            else
            {
                mem_cpy(cmd_buffer,cmd_buffer+1,sizeof(cmd_buffer)-1);
                cmd_buffer[sizeof(cmd_buffer)-1] = rx_data;

                if(str_find(cmd_buffer,sizeof(cmd_buffer),(uint8_t *)"OK\r\n",4)!=-1)
                {
                    resault = 0;
                }
                if(str_find(cmd_buffer,sizeof(cmd_buffer),(uint8_t *)"ERROR\r\n",4)!=-1)
                {
                    resault = -2;
                }
            }
        }//if(rx_data != -1)
        if(resp == NULL)
        {
            if((system_get_tick10ms() - byte_timer) > 20)
            {
                if(resault != -1)
                {
                    tpos_mutexFree(&SIGNAL_REMOTE_UART_SEND);
                    return resault;
                }
            }
        }
        else if(resp_len > 0)
        {
            if((system_get_tick10ms() - byte_timer) > 20)
            {
                if(str_find(resp,resp_len,(uint8_t *)"OK\r\n",4)!=-1)
                {
                    tpos_mutexFree(&SIGNAL_REMOTE_UART_SEND);
                    return resp_len;
                }
                if(str_find(resp,resp_len,(uint8_t *)"ERROR\r\n",4)!=-1)
                {
                    tpos_mutexFree(&SIGNAL_REMOTE_UART_SEND);
                    return -2;
                }
            }
        }
        
        if((system_get_tick10ms() - timer) > timeout_10ms)
        {
            if((system_get_tick10ms() - byte_timer) > 20)
            {
                tpos_mutexFree(&SIGNAL_REMOTE_UART_SEND);
            return -1;
        }
        }
        if(rx_data == -1)
        {
            tpos_TASK_YIELD();
        }
    }
    tpos_mutexFree(&SIGNAL_REMOTE_UART_SEND);
    return -1;
}
uint8_t gprs_at_check_baudrate(uint32_t baudrate)
{
    gprs_uart_init(baudrate);
    if(gprs_send_cmd_wait_OK((uint8_t *)"AT\r\n",NULL,0,100) >= 0)
    {
        return 1;
    }
    if(gprs_send_cmd_wait_OK((uint8_t *)"AT\r\n",NULL,0,100) >= 0)
    {
        return 1;
    }
    if(gprs_send_cmd_wait_OK((uint8_t *)"AT\r\n",NULL,0,100) >= 0)
    {
        return 1;
    }
    return 0;
}
uint8_t gprs_at_check(void)
{
    if(gprs_at_check_baudrate(57600))
    {
        return 1;
    }
    if(gprs_at_check_baudrate(57600))
    {
        if(gprs_send_cmd_wait_OK((uint8_t *)"AT+IPR=115200\r\n",NULL,0,100) >= 0)
        {
            if(gprs_at_check_baudrate(115200))
            {
                gprs_send_cmd_wait_OK((uint8_t *)"AT&W\r\n",NULL,0,100);
                return 1;
            }
        }
    }
    if(gprs_at_check_baudrate(38400))
    {
        if(gprs_send_cmd_wait_OK((uint8_t *)"AT+IPR=115200\r\n",NULL,0,100) >= 0)
        {
            if(gprs_at_check_baudrate(115200))
            {
                gprs_send_cmd_wait_OK((uint8_t *)"AT&W\r\n",NULL,0,100);
                return 1;
            }
        }
    }
    if(gprs_at_check_baudrate(19200))
    {
        if(gprs_send_cmd_wait_OK((uint8_t *)"AT+IPR=115200\r\n",NULL,0,100) >= 0)
        {
            if(gprs_at_check_baudrate(115200))
            {
                gprs_send_cmd_wait_OK((uint8_t *)"AT&W\r\n",NULL,0,100);
                return 1;
            }
        }
    }
    if(gprs_at_check_baudrate(9600))
    {
        if(gprs_send_cmd_wait_OK((uint8_t *)"AT+IPR=57600\r\n",NULL,0,100) >= 0)
        {
            if(gprs_at_check_baudrate(57600))
            {
                gprs_send_cmd_wait_OK((uint8_t *)"AT&W\r\n",NULL,0,100);
                return 1;
            }
        }
    }
    if(gprs_at_check_baudrate(115200))
    {
        return 1;
    }
    return 0;
}
uint32_t gprs_at_check_routin()
{
	uint32_t baud=0,baud_set=115200;
	char cmd[20];
	while(1)
	{
		baud = 115200;
		if(gprs_at_check_baudrate(baud))
	    	break;
				baud = 57600;
		if(gprs_at_check_baudrate(baud))
	    	break;
				baud = 38400;
		if(gprs_at_check_baudrate(baud))
	    	break;
				baud = 19200;
		if(gprs_at_check_baudrate(baud))
	    	break;
				baud = 9600;
		if(gprs_at_check_baudrate(baud))
	    	break;
		baud = 0;
        break;
	}

	if(baud == 0) 
		return 0;
	if(baud == 115200)
		return baud;
	//或者直接获取 AT+IPR=?
	while(1)
	{
		sprintf(cmd, "AT+IPR=%ld\r\n", baud_set);
        if(gprs_send_cmd_wait_OK((uint8_t *)cmd,NULL,0,200) >= 0)
        {
            DelayNmSec(200);
            if(gprs_at_check_baudrate(baud_set))
            {
                gprs_send_cmd_wait_OK((uint8_t *)"AT&W\r\n",NULL,0,200);
                break;
            }
        }
		baud_set = baud_set/2;
		if(baud_set <= baud)
			break;
	}
    
    return baud_set;
}
uint8_t gprs_get_mytype(void)
{
    int16_t resault;
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT$MYTYPE?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        //保存MYTYPE
        
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        gSystemInfo.nouse_1376_3 = 0x55;
        return 1;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}


uint8_t update_model_type_info(uint8_t *buffer,uint16_t len)
{
    uint8_t resault=0;
    
    gSystemInfo.remote_model_type = REMOTE_MODEL_TYPE_UNKNOWN;
    if(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL)
    {
        if(str_find(buffer,len,(uint8_t*)"EC20",4) >= 0)
        {
            gSystemInfo.remote_model_type = REMOTE_MODEL_TYPE_EC20;
            resault = 1;
        }
        if(str_find(buffer,len,(uint8_t*)"EC21",4) >= 0)
        {
            gSystemInfo.remote_model_type = REMOTE_MODEL_TYPE_EC21;
            resault = 1;
        }
        if(str_find(buffer,len,(uint8_t*)"EC25",4) >= 0)
        {
            gSystemInfo.remote_model_type = REMOTE_MODEL_TYPE_EC25;
            resault = 1;
        }
    }
    else if(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG)
    {
        if(str_find(buffer,len,(uint8_t*)"A9600", 5) >= 0)
        {
            gSystemInfo.remote_model_type = REMOTE_MODEL_TYPE_A9600;
            resault = 1;
        }
        resault = 1;//宸茬粡鍦╟gmm涓洿鏂颁簡 //杩欓噷A9600鏄?KR9608
    }
    else if(str_find(buffer,len,(uint8_t*)"M5310", 5) >= 0)
    {
        gSystemInfo.remote_model_vendor = REMOTE_MODEL_VENDOR_CMI;
        gSystemInfo.remote_model_type = REMOTE_MODEL_TYPE_M5310;
    }
    return resault;
}
uint8_t update_model_vendor_info(uint8_t *buffer,uint16_t len)
{
    uint8_t resault=0;
    gSystemInfo.remote_model_vendor = REMOTE_MODEL_VENDOR_UNKNOWN;
    if(str_find(buffer,len,(uint8_t*)"Quec",4) >= 0)
    {
        gSystemInfo.remote_model_vendor = REMOTE_MODEL_VENDOR_QUECTEL;
        resault = 1;
    }
    if(str_find(buffer,len,(uint8_t*)"LONGSUNG",8) >= 0)
    {
        gSystemInfo.remote_model_vendor = REMOTE_MODEL_VENDOR_LONGSHANG;
        resault = 1;
    }
    return resault;
}
uint8_t update_mygmr_info(uint8_t *buffer,uint16_t len)
{
    uint8_t *str;
    int16_t pos,check_pos,start_pos,find_pos;
    uint8_t find_valid;
    uint8_t  idx,data[32],datapos;
    uint8_t  datalen[]={4,8,4,6,4,6};
    
    mem_set(gSystemInfo.mygmr_info,26,0);

    datapos=0;
    mem_set(data,sizeof(data),0x00);
    
    find_valid = 0;
    start_pos = 0;
    check_pos = str_find(buffer+start_pos,len,(uint8_t*)"\r\n",2);
    while(check_pos >= 0)
    {
        check_pos += 2;
        start_pos += check_pos;
        str = buffer + start_pos;
        if((len - start_pos) < 42)
        {
            break;
        }
        datapos = 0;
        check_pos = str_find(str,len-start_pos,(uint8_t*)"\r\n",2);
        if(check_pos == datalen[0])
        {
            mem_cpy(data+datapos,str,datalen[0]);
            datapos += datalen[0];
            find_pos = check_pos+2;
            str += find_pos;
            find_valid = 1;
            
            //找到了第一个长度，开始提取后边5个信息
            for(idx=1;idx<6;idx++)
            {
                if(len<(start_pos+find_pos+2))
                {
                    find_valid = 0;
                    break;
                }
                pos = str_find(str,len-start_pos-find_pos,(uint8_t*)"\r\n",2);
                if(pos == datalen[idx])
                {
                    mem_cpy(data+datapos,str,datalen[idx]);
                    datapos += datalen[idx];
                    find_pos += (pos+2); 
                    str += (pos+2);
                }
                else
                {
                    //有长度不匹配信息，重新从第一个长度开始查找
                    find_valid = 0;
                    break;
                }
            }
            if(find_valid)
            {
                break;
            }
        }
    }
    
	if(find_valid)
    {
        //发布日期与F9定义不一致，需要转换为BCD码
        Hex2Binary(data+16,6,data+16);
        mem_cpy(data+19,data+22,4);
        Hex2Binary(data+26,6,data+23);
        
        mem_cpy(gSystemInfo.mygmr_info,data,26);
        return 1;
    }
    return 0;
}
uint8_t gprs_get_mygmr(void)
{
    int16_t resault;
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT$MYGMR\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        //保存MYGMR
        update_mygmr_info(gprs_cmd_buffer,resault);
        update_model_vendor_info(gprs_cmd_buffer,resault);
        update_model_type_info(gprs_cmd_buffer,resault);
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        if(gSystemInfo.nouse_1376_3 == 0x55)
        {
            gSystemInfo.nouse_1376_3 = 0xAA;
        }
        return 1;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_get_gmr(void)
{
    int16_t resault;
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+GMR\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        //保存模块类型信息
        update_model_type_info(gprs_cmd_buffer,resault);
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 1;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_get_cgmm(void)
{
    int16_t resault;
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CGMM\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        //保存模块类型信息
        update_model_type_info(gprs_cmd_buffer,resault);
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 1;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_get_ati(void)
{
    int16_t resault;
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"ATI\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        //保存模块类型厂商信息
        update_model_vendor_info(gprs_cmd_buffer,resault);
        update_model_type_info(gprs_cmd_buffer,resault);
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 1;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_check_cimi(void)
{
    int16_t resault,pos;
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CIMI\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        if(str_find(gprs_cmd_buffer, sizeof(gprs_cmd_buffer), "NO SIM", 6) >= 0)//NO SIM
            return 0;
         pos = str_find(gprs_cmd_buffer,resault,(uint8_t*)"\r\n",2);
        if(resault-pos>15)
        {
            mem_cpy(gSystemInfo.imsi_info,gprs_cmd_buffer+pos+2,15);
            return 1;
        }
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 0;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_check_cpin(void)
{
    int16_t resault;
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CPIN?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        if(str_find(gprs_cmd_buffer, sizeof(gprs_cmd_buffer), "NO SIM", 6) >= 0)//NO SIM
            return 0;
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 0;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t update_myccid_info(INT8U* buffer,INT16U len)
{
    int16_t pos;
    uint8_t  sim_id[20];
    if(len>16)
    {
        pos = str_find(buffer,len,(uint8_t*)"\"",1);
        if(pos >= 0)
        {
            buffer += (pos+1);
            len -= (pos+1);
            pos = str_find(buffer,len,(uint8_t*)"\"",1);
            if(pos>0)
            {
                if(pos>=20)
                {
                    mem_cpy(sim_id,buffer,20);
                }
                else
                {
                     mem_cpy(sim_id,buffer,pos);
                }
                mem_cpy(gSystemInfo.myccid_info,sim_id,20);
                return 1;
           }
        } 
    }
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        pos = str_find(buffer,len,(uint8_t*)":",1);
        if(pos >= 0)
        {
            if(buffer[pos+1]==' ')
            {
                pos += 1;
            }
            buffer += (pos+1);
            len -= (pos+1);
            pos = str_find(buffer,len,(uint8_t*)"\r\n",2);
            if(pos>0)
            {
                if(pos>=20)
                {
                    mem_cpy(sim_id,buffer,20);
                }
                else
                {
                     mem_cpy(sim_id,buffer,pos);
                }
                mem_cpy(gSystemInfo.myccid_info,sim_id,20);
                return 1;
           }
        } 
    }
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_CMI))
    {
        pos = str_find(buffer,len,(uint8_t*)":",1);
        if(len-pos>20)
        {
            mem_cpy(gSystemInfo.myccid_info,buffer+pos+1,20);
            return 1;
        }
    }

    return 0;
}
uint8_t gprs_get_ccid(void)
{
    int16_t resault;
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CCID\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+QCCID\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_CMI))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+NCCID\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }    
    else
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT$MYCCID\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    if(resault >= 0)
    {
        //保存CCID
        update_myccid_info(gprs_cmd_buffer,resault);
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 1;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t update_imei_info(INT8U* buffer,INT16U len)
{
    int16_t pos;
    uint8_t  sim_id[20];
#ifndef __ONENET__
    if(len>15)
    {
        pos = str_find(buffer,len,(uint8_t*)"\"",1);
        if(pos >= 0)
        {
            buffer += (pos+1);
            len -= (pos+1);
            pos = str_find(buffer,len,(uint8_t*)"\"",1);
            if(pos>0)
            {
                if(pos==15)
                {
                    mem_cpy(sim_id,buffer,15);
                }
                else
                {
                     mem_cpy(sim_id,buffer,pos);
                }
                mem_cpy(gSystemInfo.imei_info,sim_id,15);
                return 1;
           }
        } 
    }
#else
    pos = str_find(buffer,len,(uint8_t*)":",1);
    if(len-pos>15)
    {
        mem_cpy(gSystemInfo.imei_info,buffer+pos+1,15);
        return 1;
    }
#endif
    return 0;
}
uint8_t gprs_get_imei(void)
{
    int16_t resault;
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_CMI))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CGSN=1\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    else
    {
       resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CGSN\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }   
    if(resault >= 0)
    {
        //保存imei
        update_imei_info(gprs_cmd_buffer,resault);
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 1;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_update_csq(void)
{
    int16_t resault;
    int16_t pos;
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CSQ\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        //保存CSQ
        pos =  str_find(gprs_cmd_buffer,resault,(uint8_t*)"+CSQ:",5);
        if(0 <= pos)
        {
           gSystemInfo.csq = str2int((INT8S *)gprs_cmd_buffer+pos+5);
        }
        return 1;
    }
    else if(resault == -2)
    {
        //鏀跺埌ERROR
        return 0;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_set_apn_and_call_net_longsung(void)
{
    int16_t resault;
    uint8_t idx;
    uint8_t pos;
    
    mem_cpy(gprs_cmd_buffer,(uint8_t *)"AT+MIPCALL=1",12);
    pos = 12;

    gprs_cmd_buffer[pos++]= ',';
    gprs_cmd_buffer[pos++]= '"';
    for(idx=0;idx<32;idx++)
    {
       if(gSystemInfo.apn[idx]==0x00)
       {
           break;
       }
       gprs_cmd_buffer[pos++]=gSystemInfo.apn[idx];
    }
    gprs_cmd_buffer[pos++]= '"';
    gprs_cmd_buffer[pos++]= ',';
    gprs_cmd_buffer[pos++]= '"';
    for(idx=0;idx<32;idx++)
    {
       if(gSystemInfo.user[idx]==0x00)
       {
           break;
       }
       gprs_cmd_buffer[pos++]=gSystemInfo.user[idx];
    }
    gprs_cmd_buffer[pos++]= '"';
    gprs_cmd_buffer[pos++]= ',';
    gprs_cmd_buffer[pos++]= '"';
    for(idx=0;idx<32;idx++)
    {
       if(gSystemInfo.pwd[idx]==0x00)
       {
           break;
       }
       gprs_cmd_buffer[pos++]=gSystemInfo.pwd[idx];
    }
    gprs_cmd_buffer[pos++]= '"'; 
    gprs_cmd_buffer[pos++]= '\r'; 
    gprs_cmd_buffer[pos++]= '\n'; 
    gprs_cmd_buffer[pos++]= 0; 
    resault = gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        //
        return 1;
    }
    else if(resault == -2)
    {
        //鏀跺埌ERROR
        return 1;
    }
    else //if(resault == -1)
    {
        //瓒呮椂
        return 0;
    }
    return 0;
}
uint8_t gprs_set_apn(void)
{
    int16_t resault;
    uint8_t idx;
    uint8_t pos;
    
    if((gSystemInfo.apn[0]==0x00)||(gSystemInfo.apn[0]==0xFF))
    {
        return 1;
    }
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        return 1;
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        mem_cpy(gprs_cmd_buffer,(uint8_t *)"AT+QICSGP=1,1,\"",15);
        pos = 15;
    }
    else
    {
        mem_cpy(gprs_cmd_buffer,(uint8_t *)"AT$MYNETCON=0,\"APN\",\"",21);
        pos = 21;
    }
    for(idx=0;idx<32;idx++)
    {
       if(gSystemInfo.apn[idx]==0x00)
       {
           break;
       }
       gprs_cmd_buffer[pos++]=gSystemInfo.apn[idx];
    }
    gprs_cmd_buffer[pos++]= '"';
    pos ++;
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        gprs_cmd_buffer[pos++]= ',';
        gprs_cmd_buffer[pos++]= '"';
        for(idx=0;idx<32;idx++)
        {
           if(gSystemInfo.user[idx]==0x00)
           {
               break;
           }
           gprs_cmd_buffer[pos++]=gSystemInfo.user[idx];
        }
        gprs_cmd_buffer[pos++]= '"';
        gprs_cmd_buffer[pos++]= ',';
        gprs_cmd_buffer[pos++]= '"';
        for(idx=0;idx<32;idx++)
        {
           if(gSystemInfo.pwd[idx]==0x00)
           {
               break;
           }
           gprs_cmd_buffer[pos++]=gSystemInfo.pwd[idx];
        }
        gprs_cmd_buffer[pos++]= '"';
        gprs_cmd_buffer[pos++]= ',';
        gprs_cmd_buffer[pos++]= '1';
    }
    gprs_cmd_buffer[pos++]= 0x0d;
    gprs_cmd_buffer[pos++]= 0x0A;
    gprs_cmd_buffer[pos++]= 0;
    resault = gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        //
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 1;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_set_user_pwd(void)
{
    int16_t resault;
    uint8_t idx;
    uint8_t j;
    
    if((gSystemInfo.user[0]==0x00)||(gSystemInfo.user[0]==0xFF))
    {
        return 1;
    }
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        return 1;
    }
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        return 1;
    }
    mem_cpy(gprs_cmd_buffer,(uint8_t*)"AT$MYNETCON=0,\"USERPWD\",\"",25);
    for(idx=0;idx<32;idx++)
    {
       if(gSystemInfo.user[idx]==0x00)
       {
           break;
       }
       gprs_cmd_buffer[25+idx]=gSystemInfo.user[idx];
    }
    gprs_cmd_buffer[25+idx]=',';
    for(j=0;j<32;j++)
    {
       if(gSystemInfo.pwd[j]==0x00)
       {
           break;
       }
       gprs_cmd_buffer[26+idx+j]=gSystemInfo.pwd[j];
    }
    gprs_cmd_buffer[26+idx+j]= '"';
    gprs_cmd_buffer[27+idx+j]= 0x0d;
    gprs_cmd_buffer[28+idx+j]= 0;
    resault = gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        //
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 0;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_check_creg(void)
{
    int16_t resault;
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CREG?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        if(gSystemInfo.nouse_1376_3 == 0xAA && (gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_CMI))
        {
            
        }
        else
        {
            if(  (-1 != str_find(gprs_cmd_buffer,resault,(uint8_t*)"1",1))
               ||(-1 != str_find(gprs_cmd_buffer,resault,(uint8_t*)"5",1)) )
            {
                return 1;
            }            
        }
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 0;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_check_cgreg(void)
{
    int16_t resault;
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CGREG?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        //
        if(  (-1 != str_find(gprs_cmd_buffer,resault,(uint8_t*)"1",1))
           ||(-1 != str_find(gprs_cmd_buffer,resault,(uint8_t*)"5",1)) )
        {
            return 1;
        }
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 0;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
// for NB
uint8_t gprs_check_cereg(void)
{ 
    int16_t resault,pos;

    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CEREG?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        if( (pos = str_find(gprs_cmd_buffer,resault,(uint8_t*)",", 1)) >= 0 )
        {
#ifndef __ONENET__
            if(str_find(gprs_cmd_buffer + pos +1, resault-pos-1,(uint8_t*)"1", 1) >= 0 )
#else
            if((str_find(gprs_cmd_buffer + pos +1, resault-pos-1,(uint8_t*)"1", 1) >= 0 )
                || (str_find(gprs_cmd_buffer + pos +1, resault-pos-1,(uint8_t*)"5", 1) >= 0 ))   
                
#endif
                return 1;
        }
        if(gSystemInfo.no_net_tick==0)//注册不到网络起始时间
        {
        	gSystemInfo.no_net_tick = system_get_tick10ms();
        }
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 0;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_set_netact(void)
{
    int16_t resault;
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_M5310))
    {
        return 1;
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        resault = gprs_set_apn_and_call_net_longsung();
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+QIACT=1\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    else
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT$MYNETACT=0,1\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    if(resault >= 0)
    {
        //
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 0;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_set_cereg(void)
{
    int16_t resault;
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CEREG=4\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    
    if(resault >= 0)
    {
        //
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 0;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t update_ppp_ip_info(INT8U* buffer,INT16U len)
{
    int16_t pos,pos1;
    INT8U ip[4];
    INT32U tmp;

    pos =  str_find(buffer,len,(uint8_t*)",1",2);
    if(0 <= pos)
    {
        pos += 2;
        pos1 =  str_find(buffer+pos,len-pos,(uint8_t*)"\"",1);
        pos += (pos1+1);
        tmp = str2int((INT8S *)buffer+pos);
        if(tmp > 255)
        {
            return 0;
        }
        pos1 =  str_find(buffer+pos,len-pos,(uint8_t*)".",1);
        if(pos1 > 3)
        {
            return 0;
        }
        ip[0] = tmp;
        
        pos += (pos1+1);
        tmp = str2int((INT8S *)buffer+pos);
        if(tmp > 255)
        {
            return 0;
        }
        pos1 =  str_find(buffer+pos,len-pos,(uint8_t*)".",1);
        if(pos1 > 3)
        {
            return 0;
        }
        ip[1] = tmp;
        
        pos += (pos1+1);
        tmp = str2int((INT8S *)buffer+pos);
        if(tmp > 255)
        {
            return 0;
        }
        pos1 =  str_find(buffer+pos,len-pos,(uint8_t*)".",1);
        if(pos1 > 3)
        {
            return 0;
        }
        ip[2] = tmp;
        
        pos += (pos1+1);
        tmp = str2int((INT8S *)buffer+pos);
        if(tmp > 255)
        {
            return 0;
        }
        pos1 =  str_find(buffer+pos,len-pos,(uint8_t*)"\"",1);
        if(pos1 > 3)
        {
            return 0;
        }
        ip[3] = tmp;
        mem_cpy(gSystemInfo.ppp_ip,ip,4);
        return 1;
    }
    return 0;
}
uint8_t update_ppp_ip_info_longsung(INT8U* buffer,INT16U len)
{
    int16_t pos,pos1;
    INT8U ip[4];
    INT32U tmp;

    pos =  str_find(buffer,len,(uint8_t*)"1,",2);
    if(0 <= pos)
    {
        pos += 2;

        tmp = str2int((INT8S *)buffer+pos);
        if(tmp > 255)
        {
            return 0;
        }
        pos1 =  str_find(buffer+pos,len-pos,(uint8_t*)".",1);
        if(pos1 > 3)
        {
            return 0;
        }
        ip[0] = tmp;
        
        pos += (pos1+1);
        tmp = str2int((INT8S *)buffer+pos);
        if(tmp > 255)
        {
            return 0;
        }
        pos1 =  str_find(buffer+pos,len-pos,(uint8_t*)".",1);
        if(pos1 > 3)
        {
            return 0;
        }
        ip[1] = tmp;
        
        pos += (pos1+1);
        tmp = str2int((INT8S *)buffer+pos);
        if(tmp > 255)
        {
            return 0;
        }
        pos1 =  str_find(buffer+pos,len-pos,(uint8_t*)".",1);
        if(pos1 > 3)
        {
            return 0;
        }
        ip[2] = tmp;
        
        pos += (pos1+1);
        tmp = str2int((INT8S *)buffer+pos);
        if(tmp > 255)
        {
            return 0;
        }

        ip[3] = tmp;
        mem_cpy(gSystemInfo.ppp_ip,ip,4);
        return 1;
    }
    return 0;
}
uint8_t gprs_check_netact(void)
{
    int16_t resault,pos;
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_M5310))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CGATT?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+MIPCALL?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+QIACT?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    else
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT$MYNETACT?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    if(resault >= 0)
    {
        if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_M5310))
        {
            if(( pos = str_find(gprs_cmd_buffer, resault, "+CGATT:", 7)) >= 0)
            {
                if(str2int(gprs_cmd_buffer + pos+ 7) == 1)
                {
                    gSystemInfo.cgatt=1;
//                    tpos_mutexPend(&SIGNAL_RUN_DATA);
//                    g_app_run_data.status |= CGATT_SUCCESS; 
//                    tpos_mutexFree(&SIGNAL_RUN_DATA);
                    return 1;
                }
            }
            gSystemInfo.cgatt=0;
            return 0;
        }
        //保存ip地址
        pos = str_find(gprs_cmd_buffer,resault,(uint8_t*)"1,",2);
        if(pos > 0)
        {
        	if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
				update_ppp_ip_info_longsung(gprs_cmd_buffer,resault);
            
			else
                update_ppp_ip_info(gprs_cmd_buffer,resault);
            return 1;
        }
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 0;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_set_tcp_param(uint8_t *ip,uint8_t *port)
{
    int16_t resault;
    uint32_t port_tmp;
    
    if((!check_is_valid_ipaddr(ip))|| (!check_is_valid_port(port)))
    {
        return 0;
    }
    port_tmp = port[1];
    port_tmp <<= 8;
    port_tmp += port[0];
    sprintf((char *)gprs_cmd_buffer,"AT$MYNETSRV=0,0,0,0,\"%d.%d.%d.%d:%ld\"\r\n",ip[0],ip[1],ip[2],ip[3],port_tmp);
    resault = gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        //
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 0;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_close_socket_longsung(uint8_t socket_id)
{
    int16_t ret;
    sprintf((char *)gprs_cmd_buffer,"AT+MIPCLOSE=%d\r\n", socket_id);
    ret = gprs_send_cmd_wait_OK(gprs_cmd_buffer, gprs_cmd_buffer, sizeof(gprs_cmd_buffer), 100);
    if(ret > 0)
    {
        if(str_find(gprs_cmd_buffer, ret,(INT8U*) "OK\r\n", 4) >= 0)//鍏抽棴鎴愬姛
        {
            return 1;
        }
    }
    else if(ret == -2)
    {}
    else
    {
        return 0;
    }
    return 0;
}
uint8_t gprs_close_tcp_server_longsung(void)
{
    int16_t ret;
    sprintf((char *)gprs_cmd_buffer,"AT+MIPSRVCLOSE\r\n");
    ret = gprs_send_cmd_wait_OK(gprs_cmd_buffer, gprs_cmd_buffer, sizeof(gprs_cmd_buffer), 100);
    if(ret > 0)
    {
        ret = str_find(gprs_cmd_buffer,(INT16U) ret,(INT8U*) "+MIPSRVCLOSE:0", 14);
        if( ret >= 0)//鍏抽棴鎴愬姛
        {
            return 1;
        }
    }
    else if(ret == -2)
    {}
    else
    {
        return 0;
    }
    return 0;
}
uint8_t gprs_start_tcp_server_longsung(INT8U* port)
{
    int16_t ret, pos, port_suc=0;
    uint8_t *c;
    uint16_t local_port = port[0] + port[1]*0x100;
    
    if(!check_is_valid_port(port))
        return 0;
    sprintf((char *)gprs_cmd_buffer,"AT+MIPLISTEN=0,%d\r\n", local_port);
    ret = gprs_send_cmd_wait_OK(gprs_cmd_buffer, NULL, sizeof(gprs_cmd_buffer), 100);
    if(ret >= 0)
    {
        return 1;
    }
    else if(ret == -2)
    {}
    else
    {
        return 0;
    }
    return 0;
}
uint8_t gprs_open_tcp_CMI(void)
{
    int16_t ret,pos;
    sprintf((char *)gprs_cmd_buffer,"AT+NSOCR=\"STREAM\",6,0,1\r\n");//receive control = 1, read data after indication
    ret = gprs_send_cmd_wait_OK(gprs_cmd_buffer, gprs_cmd_buffer, sizeof(gprs_cmd_buffer), 100);
    if(ret > 0)
    {
        ret = str_find(gprs_cmd_buffer,(INT16U) ret,(INT8U*) "\r\nOK", 4);
        if( ret >= 0)//鍏抽棴鎴愬姛
        {
            pos = ret-1;
            while(pos >= 0)
            {
                if(gprs_cmd_buffer[pos] >= '0' && gprs_cmd_buffer[pos] <= '6')
                {
                    tcp_client_socket_id = gprs_cmd_buffer[pos] - '0';
                    return 1;
                }
                pos -= 1;
            }
            return 0;
        }
    }
    else if(ret == -2)
    {}
    else
    {
        return 0;
    }
    return 0;
}
uint8_t gprs_connect_tcp_CMI(INT8U *ip, INT8U *port)
{
    uint32_t port_tmp;
    int16_t  rx_data,pos;
    uint32_t timer;
    uint8_t  cmd_buf[50];
    uint8_t data_valid;
    
    if((!check_is_valid_ipaddr(ip))|| (!check_is_valid_port(port)))
    {
        return 0;
    }
    port_tmp = port[1];
    port_tmp <<= 8;
    port_tmp += port[0];
    sprintf((char *)gprs_cmd_buffer,"AT+NSOCO=%d,%d.%d.%d.%d,%ld\r\n", tcp_client_socket_id, ip[0],ip[1],ip[2],ip[3], port_tmp);
    gprs_uart_send_buf(gprs_cmd_buffer,str_cmdlen(gprs_cmd_buffer));
    timer = system_get_tick10ms();
    data_valid = 0;
    mem_set(cmd_buf,sizeof(cmd_buf),0);
    while(1)
    {
        rx_data = gprs_uart_read_byte();
        if(rx_data != -1)
        {
            if(gSystemInfo.gprs_debug_flag)
            {
                system_debug_data((uint8_t*)&rx_data,1);
            }
            timer = system_get_tick10ms();
            if(data_valid == 0)
            {
                mem_cpy(cmd_buf,cmd_buf+1,sizeof(cmd_buf)-1);
                cmd_buf[sizeof(cmd_buf)-1] = rx_data;
                if(str_find(cmd_buf, sizeof(cmd_buf), (uint8_t*)"OK\r\n", 4) >= 0)
                {
                    data_valid = 1;
                    mem_set(cmd_buf,sizeof(cmd_buf),0);
                }
            }
            else if(data_valid == 1)
            {
                mem_cpy(cmd_buf,cmd_buf+1,sizeof(cmd_buf)-1);
                cmd_buf[sizeof(cmd_buf)-1] = rx_data;
            }
        }
        else
        {
            if(data_valid == 0)
            {
                if((system_get_tick10ms()-timer) > 300)
                {
                    return 0;
                }
            }
            else if(data_valid == 1)
            {
                if((system_get_tick10ms()-timer) > 100*153)
                {
                    return 0;
                }
                if((system_get_tick10ms()-timer) > 20)//check when rx_data=-1
                {
                    if(str_find(cmd_buf,sizeof(cmd_buf),(uint8_t*)"CONNECT", 7) >= 0)
                    {
                        if(str_find(cmd_buf,sizeof(cmd_buf),(uint8_t*)"OK",2)>=0)
                        {
                            return 1;
                        }
                        if((system_get_tick10ms()-timer) > 200)
                            return 0;
                    }
                    if( (pos=str_find(cmd_buf,sizeof(cmd_buf),(uint8_t*)"+NSOCLI:", 8)) >= 0)
                    {
                        if(str_find(cmd_buf+pos, sizeof(cmd_buf)-pos, (uint8_t*)"\r\n", 2)>=0)
                        {
                            if(str2int(cmd_buf+pos+8) == tcp_client_socket_id)
                                return 0;
                        }
                    }
                }
            }
            else
            {
                return 0;
            }
            tpos_TASK_YIELD();
        }
    }
    return 0;
}

uint8_t gprs_onenet_open_CMI(void)
{
    int16_t  rx_data,pos;
    uint32_t timer;
    uint8_t  cmd_buf[50];
    uint8_t data_valid,num=0;

    timer = system_get_tick10ms();
    if(gSystemInfo.is_debug == 1)
    {
        return 0;
    }
    while(tpos_mutexRequest(&SIGNAL_REMOTE_UART_SEND) == FALSE)
    {
        if((system_get_tick10ms()-timer) > MAX_REMOTE_DELAY_10MS)
        {
            return 0;
        }
        tpos_TASK_YIELD();
    }
    sprintf((char *)gprs_cmd_buffer,"AT+MIPLOPEN=0,3600,30\r\n");
    gprs_uart_send_buf(gprs_cmd_buffer,str_cmdlen(gprs_cmd_buffer));
    timer = system_get_tick10ms();
    data_valid = 0;
    mem_set(cmd_buf,sizeof(cmd_buf),0);
    while(1)
    {
        rx_data = gprs_uart_read_byte();
        if(rx_data != -1)
        {
            if(gSystemInfo.gprs_debug_flag)
            {
                system_debug_data((uint8_t*)&rx_data,1);
            }
            timer = system_get_tick10ms();
            if(data_valid == 0)
            {
                mem_cpy(cmd_buf,cmd_buf+1,sizeof(cmd_buf)-1);
                cmd_buf[sizeof(cmd_buf)-1] = rx_data;
                if(str_find(cmd_buf, sizeof(cmd_buf), (uint8_t*)"OK\r\n", 4) >= 0)
                {
                    data_valid = 1;
                    mem_set(cmd_buf,sizeof(cmd_buf),0);
                }
            }
            else if(data_valid == 1)
            {
                mem_cpy(cmd_buf,cmd_buf+1,sizeof(cmd_buf)-1);
                cmd_buf[sizeof(cmd_buf)-1] = rx_data;
            }
        }
        else
        {
            if(data_valid == 0)
            {
                if((system_get_tick10ms()-timer) > 300)  //3秒
                {
                    tpos_mutexFree(&SIGNAL_REMOTE_UART_SEND);
                    return 0;
                }
            }
            else if(data_valid == 1)
            {
                if((system_get_tick10ms()-timer) > 3000) //10秒
                {
                    tpos_mutexFree(&SIGNAL_REMOTE_UART_SEND);
                    return 0;
                }
                //if((system_get_tick10ms()-timer) > 20)//check when rx_data=-1
                {
                        if((pos = str_find(cmd_buf,sizeof(cmd_buf),(uint8_t*)"+MIPLEVENT:0,", 13)) >= 0)
                        {
                            if ((str_find(cmd_buf, sizeof (cmd_buf), (uint8_t*) "\r\n", 2)) >= 0) 
                            {
                                num = str2int(cmd_buf + pos + 13);
                                if (num == 6) 
                                {
                                    tpos_mutexFree(&SIGNAL_REMOTE_UART_SEND);
                                    return 1;
                                } 
                                else 
                                {
                                    tpos_mutexFree(&SIGNAL_REMOTE_UART_SEND);
                                    return 0;
                                }
                            }
                            
                        }
                }
            }
            tpos_TASK_YIELD();
        }
    }
    tpos_mutexFree(&SIGNAL_REMOTE_UART_SEND);
    return 0;
}
uint8_t gprs_connect_tcp_longsung(INT8U *ip, INT8U *port)
{
    uint32_t port_tmp;
    int16_t  rx_data;
    uint32_t timer;
    uint8_t  cmd_buf[50];
    uint8_t data_valid;
    
    if((!check_is_valid_ipaddr(ip))|| (!check_is_valid_port(port)))
    {
        return 0;
    }
    port_tmp = port[1];
    port_tmp <<= 8;
    port_tmp += port[0];
    sprintf((char *)gprs_cmd_buffer,"AT+MIPOPEN=%d,\"TCP\",\"%d.%d.%d.%d\",%ld,5000\r\n", tcp_client_socket_id, ip[0],ip[1],ip[2],ip[3], port_tmp);
    gprs_uart_send_buf(gprs_cmd_buffer,str_cmdlen(gprs_cmd_buffer));
    timer = system_get_tick10ms();
    data_valid = 0;
    mem_set(cmd_buf,sizeof(cmd_buf),0);
    while(1)
    {
        rx_data = gprs_uart_read_byte();
        if(rx_data != -1)
        {
            if(gSystemInfo.gprs_debug_flag)
            {
                system_debug_data((uint8_t*)&rx_data,1);
            }
            timer = system_get_tick10ms();
            if(data_valid == 0)
            {
                mem_cpy(cmd_buf,cmd_buf+1,sizeof(cmd_buf)-1);
                cmd_buf[sizeof(cmd_buf)-1] = rx_data;
                if(str_find(cmd_buf, sizeof(cmd_buf), (uint8_t*)"OK\r\n", 4) >= 0)
                {
                    data_valid = 1;
                    mem_set(cmd_buf,sizeof(cmd_buf),0);
                }
            }
            else if(data_valid == 1)
            {
                mem_cpy(cmd_buf,cmd_buf+1,sizeof(cmd_buf)-1);
                cmd_buf[sizeof(cmd_buf)-1] = rx_data;
                if(str_find(cmd_buf,sizeof(cmd_buf),(uint8_t*)"+MIPOPEN:", 9) >= 0)
                {
                    if(str_find(cmd_buf,sizeof(cmd_buf),(uint8_t*)",1",2)>=0)
                    {
                        return 1;
                    }
                    if(str_find(cmd_buf,sizeof(cmd_buf),(uint8_t*)",0",2)>=0)
                    {
                        return 0;
                    }
                }
            }
        }
        else
        {
            if(data_valid == 0)
            {
                if((system_get_tick10ms()-timer) > 300)
                {
                    return 0;
                }
            }
            else if(data_valid == 1)
            {
                if((system_get_tick10ms()-timer) > 100*153)
                {
                    return 0;
                }
                if((system_get_tick10ms()-timer) > 20)
                {
                    if(str_find(cmd_buf,sizeof(cmd_buf),(uint8_t*)"+MIPOPEN:", 9) >= 0)
                    {
                        if(str_find(cmd_buf,sizeof(cmd_buf),(uint8_t*)",1",2)>=0)
                        {
                            return 1;
                        }
                        if(str_find(cmd_buf,sizeof(cmd_buf),(uint8_t*)",0",2)>=0)
                        {
                            return 0;
                        }
                    }
                }
            }
            else
            {
                return 0;
            }
            tpos_TASK_YIELD();
        }
    }
    return 0;
}
uint8_t gprs_connect_tcp_quec(uint8_t *ip,uint8_t *port)
{
    uint32_t port_tmp;
    int16_t  rx_data;
    uint32_t timer;
    uint8_t  cmd_buf[20];
    uint8_t data_valid;
    
    if((!check_is_valid_ipaddr(ip))|| (!check_is_valid_port(port)))
    {
        return 0;
    }
    port_tmp = port[1];
    port_tmp <<= 8;
    port_tmp += port[0];
    sprintf((char *)gprs_cmd_buffer,"AT+QIOPEN=1,0,\"TCP\",\"%d.%d.%d.%d\",%ld,0,0\r\n",ip[0],ip[1],ip[2],ip[3],port_tmp);
    gprs_uart_send_buf(gprs_cmd_buffer,str_cmdlen(gprs_cmd_buffer));
    timer = system_get_tick10ms();
    data_valid = 0;
    mem_set(cmd_buf,sizeof(cmd_buf),0);
    while(1)
    {
        rx_data = gprs_uart_read_byte();
        if(rx_data != -1)
        {
            if(gSystemInfo.gprs_debug_flag)
            {
                system_debug_data((uint8_t*)&rx_data,1);
            }
            timer = system_get_tick10ms();
            if(data_valid == 0)
            {
                mem_cpy(cmd_buf,cmd_buf+1,5);
                cmd_buf[5] = rx_data;
                if(compare_string(cmd_buf,(uint8_t*)"\r\nOK\r\n",6)==0)
                {
                    data_valid = 1;
                    mem_set(cmd_buf,sizeof(cmd_buf),0);
                }
            }
            else if(data_valid == 1)
            {
                mem_cpy(cmd_buf,cmd_buf+1,sizeof(cmd_buf)-1);
                cmd_buf[sizeof(cmd_buf)-1] = rx_data;
            }
        }
        else
        {
            if(data_valid == 0)
            {
                if((system_get_tick10ms()-timer) > 300)
                {
                    return 0;
                }
            }
            else if(data_valid == 1)
            {
                if((system_get_tick10ms()-timer) > 100*153)
                {
                    return 0;
                }
                if((system_get_tick10ms()-timer) > 20)
                {
                    if(str_find(cmd_buf,sizeof(cmd_buf),(uint8_t*)"+QIOPEN:",8)>=0)
                    {
                        if(str_find(cmd_buf,sizeof(cmd_buf),(uint8_t*)"0,0",3)>=0)
                        {
                            return 1;
                        }
                        else
                        {
                            return 0;
                        }
                    }
                }
            }
            else
            {
                return 0;
            }
            tpos_TASK_YIELD();
        }
    }
    return 0;
}
uint8_t gprs_connect_tcp(void)
{
    int16_t resault;
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        return 1;
    }
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT$MYNETOPEN=0\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100*130); //130s//实测无法连接时，EC20模块2min才回复error，所以这里等待130s
    if(resault >= 0)
    {
        //
        return 1;
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 0;
    }
    else //if(resault == -1)
    {
        //超时
        return 0;
    }
    return 0;
}
uint8_t check_dns_process(void)
{
    uint8_t dns_ip1[4],dns_ip2[4];
    uint8_t domain_len;
    int16_t resault;
    int16_t rx_data;
    uint16_t pos;
    uint32_t timer;
    int16_t pos_x,pos_y;
    INT8U ip[4];
    INT32U tmp;
    
    if(    check_is_valid_ipaddr(gSystemInfo.ip1)||check_is_valid_ipaddr(gSystemInfo.ip2))
    {
        return 0;
    }
    if((!check_is_valid_port(gSystemInfo.port1)) && (!check_is_valid_port(gSystemInfo.port2)) )
    {
        return 0;
    }
    fread_ertu_params(EEADDR_SET_F301,dns_ip1,4);
    fread_ertu_params(EEADDR_SET_F301+4,dns_ip2,4);
    fread_ertu_params(EEADDR_SET_F301+8,&domain_len,1);
    if(domain_len > 50)
    {
        return 0;
    }
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        if(    check_is_valid_ipaddr(dns_ip1) || check_is_valid_ipaddr(dns_ip2) ) 
        {
            sprintf((char *)gprs_cmd_buffer,"AT+QIDNSCFG=1,\"%d.%d.%d.%d\",\"%d.%d.%d.%d\"\r\n",dns_ip1[0],dns_ip1[1],dns_ip1[2],dns_ip1[3],dns_ip2[0],dns_ip2[1],dns_ip2[2],dns_ip2[3]);
            resault = gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
            if(resault >= 0)
            {

            }
            else if(resault == -2)
            {
                //收到ERROR
                return 0;
            }
            else //if(resault == -1)
            {
                //超时
                return 0;
            }
        }

        mem_cpy(gprs_cmd_buffer,"AT+QIDNSGIP=1,\"",15);
        fread_ertu_params(EEADDR_SET_F301+9,gprs_cmd_buffer+15,domain_len);
        if(gprs_cmd_buffer[15+domain_len-1] == 0)
        {
            domain_len -= 1;
        }
        mem_cpy(gprs_cmd_buffer+15+domain_len,"\"\r\n",4);
        resault = gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
        if(resault >= 0)
        {

        }
        else if(resault == -2)
        {
            //收到ERROR
            return 0;
        }
        else //if(resault == -1)
        {
            //超时
            return 0;
        }

        timer = system_get_tick10ms();
        pos = 0;
        while(1)
        {
            rx_data = gprs_uart_read_byte();
            if(rx_data != -1)
            {
                if(gSystemInfo.gprs_debug_flag)
                {
                    system_debug_data((uint8_t*)&rx_data,1);
                }
                timer = system_get_tick10ms();
                if(pos < sizeof(gprs_cmd_buffer))
                {
                    gprs_cmd_buffer[pos] = rx_data;
                    pos ++;
                }
                if(pos > 15)
                {
                    if((gprs_cmd_buffer[pos-1]==0x0A)&&(gprs_cmd_buffer[pos-2]==0x0D))
                    {
                        if(str_find(gprs_cmd_buffer,pos,(uint8_t *)"+QIURC:",7)!=-1)
                        {
                            pos_x = str_find(gprs_cmd_buffer,pos,(uint8_t *)"\"dnsgip\",\"",10);
                            if(pos_x > 0)
                            {
                                pos_x += 10;
                                tmp = str2int((INT8S *)gprs_cmd_buffer+pos_x);
                                if(tmp > 255)
                                {
                                    return 0;
                                }
                                pos_y =  str_find(gprs_cmd_buffer+pos_x,pos-pos_x,(uint8_t*)".",1);
                                if(pos_y > 3)
                                {
                                    return 0;
                                }
                                ip[0] = tmp;

                                pos_x += (pos_y+1);
                                tmp = str2int((INT8S *)gprs_cmd_buffer+pos_x);
                                if(tmp > 255)
                                {
                                    return 0;
                                }
                                pos_y =  str_find(gprs_cmd_buffer+pos_x,pos-pos_x,(uint8_t*)".",1);
                                if(pos_y > 3)
                                {
                                    return 0;
                                }
                                ip[1] = tmp;

                                pos_x += (pos_y+1);
                                tmp = str2int((INT8S *)gprs_cmd_buffer+pos_x);
                                if(tmp > 255)
                                {
                                    return 0;
                                }
                                pos_y =  str_find(gprs_cmd_buffer+pos_x,pos-pos_x,(uint8_t*)".",1);
                                if(pos_y > 3)
                                {
                                    return 0;
                                }
                                ip[2] = tmp;

                                pos_x += (pos_y+1);
                                tmp = str2int((INT8S *)gprs_cmd_buffer+pos_x);
                                if(tmp > 255)
                                {
                                    return 0;
                                }
                                pos_y =  str_find(gprs_cmd_buffer+pos_x,pos-pos_x,(uint8_t*)".",1);
                                if(pos_y > 3)
                                {
                                    return 0;
                                }
                                ip[3] = tmp;
                                mem_cpy(gSystemInfo.ip1,ip,4);
                                mem_cpy(gSystemInfo.ip2,ip,4);
                                return 1;
                            }
                        }
                        pos = 0;
                    }
                }
            }
            if(pos == 0)
            {
                if((system_get_tick10ms()-timer)>9000)
                {
                    return 0;
                }
            }
            else
            {
                if((system_get_tick10ms()-timer)>100)
                {
                    return 0;
                }
            }
        }
    }
    return 0;
}
int16_t set_baud_M5310(void)
{
    INT32U tmr;
    if(gprs_send_cmd_wait_OK((uint8_t *)"AT+NATSPEED=115200,30,1\r\n", NULL, 0, 200) < 0)
        return 0;
    tmr = system_get_tick10ms();
    while(system_get_tick10ms() - tmr < 100*40)
    {
        if(gprs_at_check_baudrate(115200))
            break;
    }
    gprs_send_cmd_wait_OK((uint8_t *)"AT+NRB\r\n", NULL, 0, 200);
    
    return 1;
}
void check_firmware_longsung()
{
    int16_t result;
	
	result=gprs_send_cmd_wait_OK((uint8_t *)"AT+LSVER\r\n", gprs_cmd_buffer, sizeof(gprs_cmd_buffer),200);
}
void longshang_close_psm_edrx(void)
{
	int16_t result,pos,ok_count;
	
	ok_count=0;
	result=gprs_send_cmd_wait_OK((uint8_t *)"AT+CPSMS?\r\n", gprs_cmd_buffer, sizeof(gprs_cmd_buffer),200);
	pos=str_find(gprs_cmd_buffer, sizeof(gprs_cmd_buffer),"+CPSMS: 0",9);
	if(pos<0)
	{
		if(gprs_send_cmd_wait_OK((uint8_t *)"AT+CPSMS=0\r\n", NULL, sizeof(gprs_cmd_buffer),200) >= 0)
		{
			ok_count++;	
		}
	}
	result=gprs_send_cmd_wait_OK((uint8_t *)"AT+CEDRXS?\r\n", gprs_cmd_buffer, sizeof(gprs_cmd_buffer),200);
	pos=str_find(gprs_cmd_buffer, sizeof(gprs_cmd_buffer),"+CEDRXS: 0",10);
	if(pos<0)                                              
	{
		if(gprs_send_cmd_wait_OK((uint8_t *)"AT+CEDRXS=0\r\n", NULL, sizeof(gprs_cmd_buffer),200) >= 0)
		{
			ok_count++;	
		}
	}
	result=gprs_send_cmd_wait_OK((uint8_t *)"AT+CSCLK?\r\n", gprs_cmd_buffer, sizeof(gprs_cmd_buffer),200);
	pos=str_find(gprs_cmd_buffer, sizeof(gprs_cmd_buffer),"+CSCLK: 0",9);
	if(pos<0)
	{
		if(gprs_send_cmd_wait_OK((uint8_t *)"AT+CSCLK=0\r\n", NULL, sizeof(gprs_cmd_buffer),200) >= 0)
		{
			ok_count++;	
		}
	}
	result=gprs_send_cmd_wait_OK((uint8_t *)"AT+NVSETPM?\r\n", gprs_cmd_buffer, sizeof(gprs_cmd_buffer),200);
	pos=str_find(gprs_cmd_buffer, sizeof(gprs_cmd_buffer),"pm is 0\r\n",9);
	if(pos<0)
	{
		if(gprs_send_cmd_wait_OK((uint8_t *)"AT+NVSETPM=0\r\n", NULL, sizeof(gprs_cmd_buffer),200) >= 0)
		{
			ok_count++;	
		}
	}
	if(ok_count>0)
	{
	    if(gprs_send_cmd_wait_OK((uint8_t *)"AT&W\r\n", NULL, sizeof(gprs_cmd_buffer),200) >= 0)
		{
		 	gprs_power_off();
		 
		}
	}

}
void gprs_start_tcp(void);
uint8_t set_data_mode_longsung(void);
void gprs_start_work(void)
{
    uint8_t onoff_count=0,cfun_flag=0;
    uint8_t retry_flag=0;
    uint8_t try_count;
    uint8_t ip[4];
    uint8_t port[2];
    uint32_t baud;
    
    onoff_count=0;
    while(1)
    {
        ClrTaskWdt();
        gSystemInfo.tcp_link = 0;
        if(onoff_count++ > 10)
        {
            onoff_count=0;
            drv_pin_gprs_rst(0);
            DelayNmSec(1000);
            drv_pin_gprs_rst(1);
            DelayNmSec(3000);
        }

        gprs_power_on();
#ifdef __ONENET__  
        DelayNmSec(10000);//5310A开机等待10+10S
#endif
        if((baud = gprs_at_check_routin()) == 0)
        {
            gprs_power_off();
            continue;
        }
        #ifdef __ONENET__
        //3分钟约两次关机注册不到网络 关闭搜网30分钟
        if(onoff_count > 2)
        {
        	//system_debug_info("onoff 2 times");
        	while(second_elapsed(gSystemInfo.no_net_tick)<2000)	//1800+90*2
        	{
        		DelayNmSec(20000);
        		if(cfun_flag==0)
        		{
        			if(gprs_close_cfun()==1)
        			{
        				cfun_flag=1;
        			}
        		}		
        		//system_debug_info("delay 20s");		
        	}
        	cfun_flag=0;
        	onoff_count=0;
        	gSystemInfo.no_net_tick=0;
        	gprs_power_off();
        	//system_debug_info("delay 1800s");
        	continue;	
        }  
        #endif      
        gSystemInfo.nouse_1376_3 = 0;
        gSystemInfo.remote_model_type = 0;
        gSystemInfo.remote_model_vendor = 0;
        if(gprs_get_mytype() == 0)
            if(gprs_get_mytype() == 0)
                if(gprs_get_mytype() == 0)
                {
                    gprs_power_off();
                    continue;
                }
        if(gprs_get_mygmr() == 0)
            if(gprs_get_mygmr() == 0)
                if(gprs_get_mygmr() == 0)
                {
                    gprs_power_off();
                    continue;
                }
        if(gSystemInfo.nouse_1376_3 == 0xAA)
        {
            if(gprs_get_cgmm() == 0)
                if(gprs_get_cgmm() == 0)
                    if(gprs_get_cgmm() == 0)
                    {
                        gprs_power_off();
                        continue;
                    }
            if(gSystemInfo.remote_model_type != REMOTE_MODEL_TYPE_M5310)
            {
            if(gprs_get_ati() == 0)
                if(gprs_get_ati() == 0)
                    if(gprs_get_ati() == 0)
                    {
                        gprs_power_off();
                        continue;
                    }
            if(gprs_get_gmr() == 0)
                if(gprs_get_gmr() == 0)
                    if(gprs_get_gmr() == 0)
                    {
                        gprs_power_off();
                        continue;
                    }
            if(gprs_get_cgmm() == 0)
                if(gprs_get_cgmm() == 0)
                    if(gprs_get_cgmm() == 0)
                    {
                        gprs_power_off();
                        continue;
                    }                
            }//if(gSystemInfo.remote_model_type != REMOTE_MODEL_TYPE_M5310)

            
            if(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600)
            {
                longshang_close_psm_edrx();
                check_firmware_longsung();
            }
        }
        update_gprs_param();
        if(gprs_check_cimi() == 0)
            if(gprs_check_cimi() == 0)
                if(gprs_check_cimi() == 0)
                {
                    gprs_power_off();
                    continue;
                }        
        if(gprs_update_csq() == 0)
            if(gprs_update_csq() == 0)
                if(gprs_update_csq() == 0)
                {
                    gprs_power_off();
                    continue;
                }
        if(gprs_onenet_close_PSM() == 0)
            if(gprs_onenet_close_PSM() == 0)
                if(gprs_onenet_close_PSM() == 0)
                {
                    gprs_power_off();
                    continue;
                }
        if(gprs_onenet_close_eDRX() == 0)
            if(gprs_onenet_close_eDRX() == 0)
                if(gprs_onenet_close_eDRX() == 0)
                {
                    gprs_power_off();
                    continue;
                }        
        if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_M5310))
        {
            if(baud != 115200)
            {
                set_baud_M5310();
                //no continue
            }
            if(gprs_get_imei() == 0)
                if(gprs_get_imei() == 0)
                    if(gprs_get_imei() == 0)
                    {
                        gprs_power_off();
                        continue;
                    }
        if(gprs_get_ccid() == 0)
            if(gprs_get_ccid() == 0)
                if(gprs_get_ccid() == 0)
                {
                    gprs_power_off();
                    continue;
                }  
        	gprs_set_cereg();          
        }
        else
        {
        if(gprs_check_cpin() == 0)
            if(gprs_check_cpin() == 0)
                if(gprs_check_cpin() == 0)
                {
                    gprs_power_off();
                    continue;
                }
        if(gprs_get_ccid() == 0)
            if(gprs_get_ccid() == 0)
                if(gprs_get_ccid() == 0)
                {
                    gprs_power_off();
                    continue;
                }
        if(gprs_get_imei() == 0)
            if(gprs_get_imei() == 0)
                if(gprs_get_imei() == 0)
                {
                    gprs_power_off();
                    continue;
                }

        if(gprs_set_apn() == 0)
            if(gprs_set_apn() == 0)
                if(gprs_set_apn() == 0)
                {
                    gprs_power_off();
                    continue;
                }
        if(gprs_set_user_pwd() == 0)
            if(gprs_set_user_pwd() == 0)
                if(gprs_set_user_pwd() == 0)
                {
                    gprs_power_off();
                    continue;
                }            
        }
        
        try_count = 0;
        retry_flag = 0;
        while(1)
        {          
            if((gprs_check_creg() == 0)&&(gprs_check_cgreg() == 0)&&(gprs_check_cereg() == 0))
            {
                DelayNmSec(2000);
                try_count ++;
                if(try_count > 20)
                {
                    retry_flag = 1;
                    break;
                }
            }
            else
            {
                break;
            }
        }
        if(retry_flag)
        {
            gprs_power_off();
            continue;
        }
        if(gprs_set_netact() == 0)
        {
            DelayNmSec(2000);
            gprs_set_netact();
        }

        try_count = 0;
        retry_flag = 0;
        while(1)
        {
            if(gprs_check_netact() == 0)
            {
                DelayNmSec(1000);
                try_count ++;
                if(try_count > 20)
                {
                    retry_flag = 1;
                    break;
                }
            }
            else
            {
                break;
            }
        }
        if(retry_flag)
        {
            gprs_power_off();
            continue;
        }
        retry_flag = 0;
        //while(gSystemInfo.clock_ready == 0) 
        {
            ClrTaskWdt();
            DelayNmSec(1000);
            if(gprs_check_netact() == 0)
            {
                retry_flag = 1;
                break;
            }
            if((gprs_check_creg() == 0)&&(gprs_check_cgreg() == 0)&&(gprs_check_cereg() == 0))
            {
                retry_flag = 1;
                break;
            }
            if(gprs_update_csq() == 0)
            {
                retry_flag = 1;
                break;
            }
        }
        if(retry_flag)
        {
            gprs_power_off();
            continue;
        }
        retry_flag = 0;
//        while(1)
//        {
//            ClrTaskWdt();
//            update_gprs_param();
//            check_dns_process();
//            if(    (check_is_valid_ipaddr(gSystemInfo.ip1)&&check_is_valid_port(gSystemInfo.port1))
//                || (check_is_valid_ipaddr(gSystemInfo.ip2)&&check_is_valid_port(gSystemInfo.port2))  )
//            {
//                break;
//            }
//            DelayNmSec(1000);
//            if(gprs_check_netact() == 0)
//            {
//                retry_flag = 1;
//                break;
//            }
//            if((gprs_check_creg() == 0)&&(gprs_check_cgreg() == 0)&&(gprs_check_cereg() == 0))
//            {
//                retry_flag = 2;
//                break;
//            }
//            if(gprs_update_csq() == 0)
//            {
//                retry_flag = 3;
//                break;
//            }
//        }
        if(retry_flag)
        {
            gprs_power_off();
            continue;
        }
        gSystemInfo.ppp_is_ok = 1;
        //鎷ㄥ彿鎴愬姛锛屽彲浠ヨ繘琛宻ocket浠诲姟浜
        if(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG)
        {
            set_data_mode_longsung();
        }
        if((gSystemInfo.nouse_1376_3 == 0xAA) && (gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_CMI))
        {
            gprs_send_cmd_wait_OK((uint8_t *)"AT+CEREG=4\r\n", NULL,sizeof(gprs_cmd_buffer),100);
            gprs_get_ccid();
            gprs_get_eid();
        }
        break;
    }
}

void gprs_start_tcp(void)
{
    uint8_t ip[4];
    uint8_t port[2];
    GprsObj.send_len = 0;
    #ifndef __ONENET__
    if(check_is_valid_ipaddr(gSystemInfo.ip1)&&check_is_valid_port(gSystemInfo.port1))
    #endif
    {
        mem_cpy(ip,gSystemInfo.ip1,4);
        mem_cpy(port,gSystemInfo.port1,2);
        if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
        {
            if(gprs_connect_tcp_quec(ip,port))
            {
                gSystemInfo.login_status = 0;
                gSystemInfo.tcp_link = 1;
                return ;
            }
        }
        else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_CMI))
        {
            #ifndef __ONENET__
            if(gprs_open_tcp_CMI() > 0)
            {
                if(gprs_connect_tcp_CMI(ip,port))
                {
                    gSystemInfo.login_status = 0;
                    gSystemInfo.tcp_link = 1;
                    return ;
                }
            }
            #else
            //gprs_onenet_check_net();
			gprs_get_rsrp_sinr_cellid();
            DelayNmSec(500);

            //生产模式 只更新csq cfatt 写号过程避免模组复位
            while(gSystemInfo.is_debug==1)
            {
                gprs_update_csq();
                gprs_check_netact();
                DelayNmSec(3000);
            }

            if(gprs_onenet_object_creat()||gprs_onenet_object_creat()||gprs_onenet_object_creat())//创建设备实体
            {
                DelayNmSec(500);
#ifdef __METER_SIMPLE__
                #ifndef __DEBUG1__
              if(gprs_onenet_object_observe(ONENET_VOLTAGE,3,"100"))//订阅object 资源
                {
                    gprs_onenet_resource_observe_list(ONENET_VOLTAGE,"5700;5701");
                }
                
                if(gprs_onenet_object_observe(ONENET_CURRENT,4,"1001"))
                {
                    gprs_onenet_resource_observe_list(ONENET_CURRENT,"5700;5701");
                }
                if(gprs_onenet_object_observe(ONENET_POWER,8,"11000000"))
                {
                    gprs_onenet_resource_observe_list(ONENET_POWER,"5700;5701");
                }
                if(gprs_onenet_object_observe(ONENET_POWER_FACTOR,4,"1100"))
                {
                    gprs_onenet_resource_observe_list(ONENET_POWER_FACTOR,"5700");
                }
                if(gprs_onenet_object_observe(ONENET_ENERGY,9,"101011001"))
                        {
                    gprs_onenet_resource_observe_list(ONENET_ENERGY,"5805;5701");
                }
#endif
                if(gprs_onenet_object_observe(ONENET_METER_INFO,18,"111111111111111111"))
                {
                    gprs_onenet_resource_observe_list(ONENET_METER_INFO,"5750");
                } 
#ifndef __DEBUG1__
                if(gprs_onenet_object_observe(ONENET_METER_TIME,1,"1"))
                {
                    gprs_onenet_resource_observe_list(ONENET_METER_TIME,"5750");
                }              
                if(gprs_onenet_object_observe(ONENET_EVENT,11,"11110100000"))
                {
                    gprs_onenet_resource_observe_list(ONENET_EVENT,"5500");
                }
#endif
#else
                if(gprs_onenet_object_observe(ONENET_VOLTAGE,3,"111"))//订阅object 资源
                {
                    DelayNmSec(500);
                    gprs_onenet_resource_observe_list(ONENET_VOLTAGE,"5700;5701");
                    DelayNmSec(500);
                }
 
                if(gprs_onenet_object_observe(ONENET_CURRENT,4,"1111"))
                {
                    DelayNmSec(500);
                    gprs_onenet_resource_observe_list(ONENET_CURRENT,"5700;5701");
                    DelayNmSec(500);
                }
                if(gprs_onenet_object_observe(ONENET_POWER,8,"11111111"))
                {
                    DelayNmSec(500);
                    gprs_onenet_resource_observe_list(ONENET_POWER,"5700;5701");
                    DelayNmSec(500);
                }
                if(gprs_onenet_object_observe(ONENET_POWER_FACTOR,4,"1111"))
                {
                    DelayNmSec(500);
                    gprs_onenet_resource_observe_list(ONENET_POWER_FACTOR,"5700");
                    DelayNmSec(500);
                }
                if(gprs_onenet_object_observe(ONENET_ENERGY,9,"111111111"))
                        {
                            DelayNmSec(500);
                    gprs_onenet_resource_observe_list(ONENET_ENERGY,"5805;5701");
                    DelayNmSec(500);
                }

                if(gprs_onenet_object_observe(ONENET_METER_INFO,18,"111111111111111111"))
                {
                    DelayNmSec(500);
                    gprs_onenet_resource_observe_list(ONENET_METER_INFO,"5750");
                    DelayNmSec(500);
                }
 
                if(gprs_onenet_object_observe(ONENET_METER_TIME,1,"1"))
                {
                    DelayNmSec(500);
                    gprs_onenet_resource_observe_list(ONENET_METER_TIME,"5750");
                    DelayNmSec(500);
                }              
                if(gprs_onenet_object_observe(ONENET_EVENT,11,"11111111111"))
                {
                    DelayNmSec(500);
                    gprs_onenet_resource_observe_list(ONENET_EVENT,"5500");
                    DelayNmSec(500);
                }
#endif
                            if(gprs_onenet_open_CMI()==0) // 登陆onenet平台
                            {
                                if(gprs_onenet_open_CMI()==0) // 登陆onenet平台
                                {
                                    if(gprs_onenet_open_CMI()==0) // 登陆onenet平台
                                    {
                                        if(gSystemInfo.is_debug != 1)//生产进入调试状态 写卡及上线时登录不上时不复位
                                        {
                                            gprs_power_off();
                                        }
                                        return;
                                    }
                                }
                            }

                            //gSystemInfo.login_status = 1;
                            gSystemInfo.tcp_link = 1;
                            gSystemInfo.life_time = 0;
                            gSystemInfo.last_resource_ready = system_get_tick10ms();
                            return ;
                        }
 
            
            #endif
      
        }
        else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
        {
            tcp_client_socket_id = 1;
            while((tcp_client_socket_id == tcp_server_socket_id) || (tcp_client_socket_id == tcp_server_socket_id_old ))
            {
                tcp_client_socket_id++;
                if(tcp_client_socket_id > MAX_SOCKET_ID)
                {
                    gSystemInfo.ppp_is_ok = 0;
                    return ;
                }
            }
            if(gprs_connect_tcp_longsung(ip,port))
            {
                gSystemInfo.login_status = 0;//重置
                gSystemInfo.tcp_link = 1;
                return ;
            }
        }
        else
        {
            if(gprs_set_tcp_param(ip,port))
            {
                if(gprs_connect_tcp())
                {
                    gSystemInfo.login_status = 0;
                    gSystemInfo.tcp_link = 1;
                    return ;
                }
            }
        }
    }

    gprs_power_off();
}
void gprs_close_tcp_server(void)
{
     if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
     {
         if(gprs_close_tcp_server_longsung())
         {
             return;
         }
     }
}
uint8_t gprs_start_tcp_server(void)
{
    INT8U port[2];
    port[0] = gSystemInfo.listen_port[0];
    port[1] = gSystemInfo.listen_port[1];
#ifdef __PROVINCE_JIANGSU__
    int16u2_bin(5100, port);
#endif
     if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
     {
         if(gprs_start_tcp_server_longsung(port) > 0)
         {
             return 1;
         }
     }
	 return 0;
}
uint8_t gprs_close_socket(INT8U socket_id)
{
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        if(gprs_close_socket_longsung(socket_id))
        {
            return 1;
        }
    }
    return 0;
}
uint8_t gprs_read_app_data_1376_3(void)
{
    int16_t rx_data;
    uint32_t timer;
    uint16_t data_len;
    uint8_t cmd_buf[11];
    uint8_t data_valid;
    uint16_t recv_pos;
    
    if(RequestRemote.state == REQUEST_STATE_PROCESSING)
    {
        return 0;
    }
    if(gSystemInfo.tcp_link == 0)
    {
        return 0;
    }
    gprs_uart_send_buf((uint8_t*)"AT$MYNETREAD=0,500\r\n",20);
    timer = system_get_tick10ms();
    data_valid = 0;
    mem_set(cmd_buf,sizeof(cmd_buf),0);
    while(1)
    {
        rx_data = gprs_uart_read_byte();
        if(rx_data != -1)
        {
            if(gSystemInfo.gprs_debug_flag)
            {
                system_debug_data((uint8_t*)&rx_data,1);
            }
            timer = system_get_tick10ms();
            if(data_valid == 0)
            {
                //接收 $MYNETREAD:
                mem_cpy(cmd_buf,cmd_buf+1,10);
                cmd_buf[10] = rx_data;
                if(compare_string(cmd_buf,(uint8_t*)"AT$MYNETREA",11)==0)
                {
                    mem_set(cmd_buf,sizeof(cmd_buf),0);
                }
                if(compare_string(cmd_buf,(uint8_t*)"$MYNETREAD:",11)==0)
                {
                    data_valid = 1;
                }
            }
            else if(data_valid == 1)
            {
                if((rx_data == ' ')||(rx_data == 0x0d)||(rx_data == 0x0A))
                {
                    
                }
                else if((rx_data >= '0')&&(rx_data <= '5'))
                {
                    //socket_id
                    data_valid = 2;
                    data_len = 0;
                }
            }
            else if(data_valid == 2)
            {
                if(rx_data == ',')
                {
                    
                }
                else if((rx_data >= '0')&&(rx_data <= '9'))
                {
                    data_len *= 10;
                    data_len += (rx_data - '0');
                }
                else if(rx_data == 0x0D)
                {
                    
                }
                else if(rx_data == 0x0A)
                {
                    data_valid = 3;
                    recv_pos = GprsObj.recv_pos;
                    if((data_len == 0)||(data_len > 500))
                    {
                        data_valid = 0xFF;
                    }
                }
            }
            else if(data_valid == 3)
            {
                GprsObj.recv_buf[recv_pos] = rx_data;
                recv_pos ++;
                if(recv_pos >= sizeof(GprsObj.recv_buf))
                {
                    recv_pos = 0;
                }
                data_len --;
                if(data_len <= 0)
                {
                    data_valid = 4;
                }
            }
            else if(data_valid == 4)
            {
                mem_cpy(cmd_buf,cmd_buf+1,5);
                cmd_buf[5] = rx_data;
                if(compare_string(cmd_buf,(uint8_t*)"\r\nOK\r\n",6)==0)
                {
                    GprsObj.recv_pos = recv_pos;
                    if(data_len)
                    {
                        return 1;
                    }
                    return 0;
                }
            }
            else
            {
                
            }
        }
        else
        {
            if((data_valid == 0)||(data_valid == 1)||(data_valid == 4))
            {
                if((system_get_tick10ms()-timer) > 100)
                {
                    return 0;
                }
            }
            else if(data_valid == 3)
            {
                if((system_get_tick10ms()-timer) > 300)
                {
                    return 0;
                }
            }
            else
            {
                if((system_get_tick10ms()-timer) > 50)
                {
                    return 0;
                }
            }
            tpos_TASK_YIELD();
        }
    }
    return 0;
}
uint8_t gprs_read_app_data_CMI(void)
{
    int16_t rx_data;
    uint32_t timer;
    uint16_t data_len=0;
    uint8_t cmd_buf[10];
    uint8_t data_valid;
    uint16_t recv_pos,info_pos,pos,find_pos;
    INT8U data[2],index=0;
    
    if(RequestRemote.state == REQUEST_STATE_PROCESSING)
    {
        return 0;
    }
    if(gSystemInfo.tcp_link == 0)
    {
        return 0;
    }
    sprintf((char *)gprs_cmd_buffer,"AT+NSORF=%d,%d\r\n",tcp_client_socket_id, has_data_to_read);
    gprs_uart_send_buf(gprs_cmd_buffer, str_cmdlen(gprs_cmd_buffer));
    timer = system_get_tick10ms();
    data_valid = 0;
    mem_set(cmd_buf,sizeof(cmd_buf),0);
    while(1)
    {
        rx_data = gprs_uart_read_byte();
        if(rx_data != -1)
        {
            if(gSystemInfo.gprs_debug_flag)
            {
                system_debug_data((uint8_t*)&rx_data,1);
            }
            timer = system_get_tick10ms();
            
            if(data_valid == 0)
            {
                mem_cpy(cmd_buf,cmd_buf+1, sizeof(cmd_buf)-1);
                cmd_buf[sizeof(cmd_buf) -1] = rx_data;
                if(str_find(cmd_buf,sizeof(cmd_buf),(uint8_t*)"+NSORF:",7)>= 0)
                {
                    data_valid = 1;
                    data_len = 0;
                    info_pos = 0;
                }
                if(str_find(cmd_buf,sizeof(cmd_buf),(uint8_t*)"OK",2) >= 0)
                {
                    has_data_to_read = 0;
                    return 0;
                }
            }
            else
            {
                if(data_valid != 5)
                    gprs_cmd_buffer[info_pos++] = rx_data;
            }
            if(data_valid == 1)//socket id out
            {
                if((rx_data == ','))
                {
                    
                    data_valid ++;
                    find_pos = info_pos;
                }
            }
            else if(data_valid == 2)//ip out
            {
                if((rx_data == ','))
                {
                    
                    data_valid ++;
                    find_pos = info_pos;
                }
            }
            else if(data_valid == 3)//port out
            {
                if((rx_data == ','))
                {
                    
                    data_valid ++;
                    find_pos = info_pos;
                }
            }
            else if(data_valid == 4)//len out
            {
                if((rx_data == ','))
                {
                    data_len = str2int((INT8S*)gprs_cmd_buffer+find_pos);
                    data_len *= 2;
                    recv_pos = GprsObj.recv_pos;
                    data_valid ++;
                    find_pos = info_pos;  
                }
            }
            else if(data_valid == 5)//data out
            {
                data[index] = rx_data;
                index++;
                if(index > 1)
                {
                    index = 0;
                    str2hex(data, 2, (INT8U*)&rx_data);
                    GprsObj.recv_buf[recv_pos] = rx_data;
                    recv_pos ++;
                    if(recv_pos >= sizeof(GprsObj.recv_buf))
                    {
                        recv_pos = 0;
                    } 
                }
                data_len --;
                if(data_len <= 0)
                {
                    data_valid ++;
                    info_pos = 0;
                }
            }
            else if(data_valid == 6)//ok out ??
            {
                if(str_find(gprs_cmd_buffer,info_pos,(uint8_t*)"\r\nOK",4)>= 0)
                {
                    GprsObj.recv_pos = recv_pos;
                    has_data_to_read = 0;//clear flag

                    return 1;

                }
            }
            else
            {
                
            }
        }
        else
        {
            if(data_valid < 5)
            {
                if((system_get_tick10ms()-timer) > 100)
                {
                    return 0;
                }
            }
            else if(data_valid == 5)
            {
                if((system_get_tick10ms()-timer) > 300)
                {
                    return 0;
                }
            }
            else
            {
                if((system_get_tick10ms()-timer) > 50)
                {
                    return 0;
                }
            }
            tpos_TASK_YIELD();
        }
    }
    return 0;
}


uint8_t gprs_read_app_data_quec(void)
{
    int16_t rx_data;
    uint32_t timer;
    uint16_t data_len=0;
    uint8_t cmd_buf[11];
    uint8_t data_valid;
    uint16_t recv_pos;
    
    if(RequestRemote.state == REQUEST_STATE_PROCESSING)
    {
        return 0;
    }
    if(gSystemInfo.tcp_link == 0)
    {
        return 0;
    }
    gprs_uart_send_buf((uint8_t*)"AT+QIRD=0,500\r\n",20);
    timer = system_get_tick10ms();
    data_valid = 0;
    mem_set(cmd_buf,sizeof(cmd_buf),0);
    while(1)
    {
        rx_data = gprs_uart_read_byte();
        if(rx_data != -1)
        {
            if(gSystemInfo.gprs_debug_flag)
            {
                system_debug_data((uint8_t*)&rx_data,1);
            }
            timer = system_get_tick10ms();
            if(data_valid == 0)
            {
                //接收 $MYNETREAD:
                mem_cpy(cmd_buf,cmd_buf+1,6);
                cmd_buf[6] = rx_data;
                if(compare_string(cmd_buf,(uint8_t*)"+QIRD: ",7)==0)
                {
                    data_valid = 1;
                    data_len = 0;
                }
            }
            else if(data_valid == 1)
            {
                if((rx_data >= '0')&&(rx_data <= '9'))
                {
                    data_len *= 10;
                    data_len += (rx_data - '0');
                }
                else if(rx_data == 0x0D)
                {
                    
                }
                else if(rx_data == 0x0A)
                {
                    data_valid = 3;
                    recv_pos = GprsObj.recv_pos;
                    if((data_len == 0)||(data_len > 500))
                    {
                        data_valid = 0xFF;
                    }
                }
            }
            else if(data_valid == 3)
            {
                GprsObj.recv_buf[recv_pos] = rx_data;
                recv_pos ++;
                if(recv_pos >= sizeof(GprsObj.recv_buf))
                {
                    recv_pos = 0;
                }
                data_len --;
                if(data_len <= 0)
                {
                    data_valid = 4;
                }
            }
            else if(data_valid == 4)
            {
                mem_cpy(cmd_buf,cmd_buf+1,5);
                cmd_buf[5] = rx_data;
                if(compare_string(cmd_buf,(uint8_t*)"\r\nOK\r\n",6)==0)
                {
                    GprsObj.recv_pos = recv_pos;
                    if(data_len)
                    {
                        return 1;
                    }
                    return 0;
                }
            }
            else
            {
                
            }
        }
        else
        {
            if((data_valid == 0)||(data_valid == 1)||(data_valid == 4))
            {
                if((system_get_tick10ms()-timer) > 100)
                {
                    return 0;
                }
            }
            else if(data_valid == 3)
            {
                if((system_get_tick10ms()-timer) > 300)
                {
                    return 0;
                }
            }
            else
            {
                if((system_get_tick10ms()-timer) > 50)
                {
                    return 0;
                }
            }
            tpos_TASK_YIELD();
        }
    }
    return 0;
}
uint8_t check_report(void)
{
    gprs_send_cmd_wait_OK(NULL, NULL, 0, 20);

    return 0;
}
uint8_t gprs_read_app_data_longsung(void)
{
    //gprs_send_cmd_wait_OK(NULL, NULL, 0, 20);
    return 0;
}
uint8_t gprs_read_app_data(void)
{
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        return gprs_read_app_data_quec();
    }
    else    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_M5310))
    {
        #ifndef  __ONENET__
        if(has_data_to_read)
            return gprs_read_app_data_CMI();
        else
            return 0;
        #else
            return 0;
        #endif

    }
    else    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        return gprs_read_app_data_longsung();
    }
    else
    {
        return gprs_read_app_data_1376_3();
    }
}
uint8_t gprs_server_read_app_data(void)
{
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        //return gprs_read_app_data_quec();
    }
    else    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        return gprs_read_app_data_longsung();
    }
    else
    {
        //return gprs_read_app_data_1376_3();
    }
    return 0;
}
uint8_t gprs_send_app_data_1376_3(void)
{
    int16_t  rx_data;
    uint32_t timer;
    uint16_t send_len;
    uint8_t  cmd_buf[12];
    uint8_t data_valid;
    uint16_t data_len;
    
    if(gSystemInfo.tcp_link == 0)
    {
        return 0;
    }
    if(GprsObj.send_len == 0)
    {
        return 0;
    }
    send_len = GprsObj.send_len;
    if(send_len > 1460)
    {
        send_len = 1460;
    }
    sprintf((char*)gprs_cmd_buffer,"AT$MYNETWRITE=0,%d\r",send_len);
    gprs_uart_send_buf(gprs_cmd_buffer,str_cmdlen(gprs_cmd_buffer));
    timer = system_get_tick10ms();
    data_valid = 0;
    mem_set(cmd_buf,sizeof(cmd_buf),0);
    while(1)
    {
        rx_data = gprs_uart_read_byte();
        if(rx_data != -1)
        {
            if(gSystemInfo.gprs_debug_flag)
            {
                system_debug_data((uint8_t*)&rx_data,1);
            }
            timer = system_get_tick10ms();
            if(data_valid == 0)
            {
                //接收 $MYNETWRITE:
                mem_cpy(cmd_buf,cmd_buf+1,11);
                cmd_buf[11] = rx_data;
                if(compare_string(cmd_buf,(uint8_t*)"AT$MYNETWRIT",12)==0)
                {
                    mem_set(cmd_buf,sizeof(cmd_buf),0);
                }
                if(compare_string(cmd_buf,(uint8_t*)"$MYNETWRITE:",12)==0)
                {
                    data_valid = 1;
                }
            }
            else if(data_valid == 1)
            {
                if((rx_data == ' ')||(rx_data == 0x0d)||(rx_data == 0x0A))
                {
                    
                }
                else if((rx_data >= '0')&&(rx_data <= '5'))
                {
                    //socket_id
                    data_valid = 2;
                    data_len = 0;
                }
            }
            else if(data_valid == 2)
            {
                if(rx_data == ',')
                {
                    
                }
                else if((rx_data >= '0')&&(rx_data <= '9'))
                {
                    data_len *= 10;
                    data_len += (rx_data - '0');
                }
                else if(rx_data == 0x0D)
                {
                    
                }
                else if(rx_data == 0x0A)
                {
                    data_valid = 3;
                    if((data_len == 0)||(data_len > 1460))
                    {
                        data_valid = 0xFF;
                    }
                    else
                    {
                        gprs_uart_send_buf(GprsObj.send_ptr,data_len);
                        mem_set(cmd_buf,sizeof(cmd_buf),0);
                    }
                }
            }
            else if(data_valid == 3)
            {
                mem_cpy(cmd_buf,cmd_buf+1,5);
                cmd_buf[5] = rx_data;
                if(compare_string(cmd_buf,(uint8_t*)"\r\nOK\r\n",6)==0)
                {
                    GprsObj.send_ptr += data_len;
                    if(GprsObj.send_len >= data_len)
                    {
                        GprsObj.send_len -= data_len;
                    }
                    else
                    {
                        GprsObj.send_len = 0;
                    }
                    if(GprsObj.send_len)
                    {
                        return 1;
                    }
                    return 0;
                }
            }
            else
            {
                
            }
        }
        else
        {
            if((data_valid == 0)||(data_valid == 2))
            {
                if((system_get_tick10ms()-timer) > 200)
                {
                    return 0;
                }
            }
            else if(data_valid == 3)
            {
                if((system_get_tick10ms()-timer) > 300)
                {
                    return 0;
                }
            }
            else
            {
                if((system_get_tick10ms()-timer) > 50)
                {
                    return 0;
                }
            }
            tpos_TASK_YIELD();
        }
    }
    return (GprsObj.send_len==0)?0:1;
}
uint8_t gprs_send_app_data_quec(void)
{
    int16_t  rx_data;
    uint32_t timer;
    uint16_t send_len;
    uint8_t  cmd_buf[12];
    uint8_t data_valid;
    uint16_t data_len;
    
    if(gSystemInfo.tcp_link == 0)
    {
        return 0;
    }
    if(GprsObj.send_len == 0)
    {
        return 0;
    }
    send_len = GprsObj.send_len;
    if(send_len > 1460)
    {
        send_len = 1460;
    }
    sprintf((char*)gprs_cmd_buffer,"AT+QISEND=0,%d\r",send_len);
    gprs_uart_send_buf(gprs_cmd_buffer,str_cmdlen(gprs_cmd_buffer));
    timer = system_get_tick10ms();
    data_valid = 0;
    mem_set(cmd_buf,sizeof(cmd_buf),0);
    while(1)
    {
        rx_data = gprs_uart_read_byte();
        if(rx_data != -1)
        {
            if(gSystemInfo.gprs_debug_flag)
            {
                system_debug_data((uint8_t*)&rx_data,1);
            }
            timer = system_get_tick10ms();
            if(data_valid == 0)
            {
                //接收 $MYNETWRITE:
                mem_cpy(cmd_buf,cmd_buf+1,11);
                cmd_buf[11] = rx_data;
                if(rx_data == '>')
                {
                    data_valid = 1;
                    data_len = send_len;
                    gprs_uart_send_buf(GprsObj.send_ptr,data_len);
                    mem_set(cmd_buf,sizeof(cmd_buf),0);
                }
            }
            else if(data_valid == 1)
            {
                mem_cpy(cmd_buf,cmd_buf+1,8);
                cmd_buf[8] = rx_data;
                if(compare_string(cmd_buf,(uint8_t*)"SEND OK\r\n",9)==0)
                {
                    GprsObj.send_ptr += data_len;
                    if(GprsObj.send_len >= data_len)
                    {
                        GprsObj.send_len -= data_len;
                    }
                    else
                    {
                        GprsObj.send_len = 0;
                    }
                    if(GprsObj.send_len)
                    {
                        return 1;
                    }
                    return 0;
                }
            }
            else
            {
                
            }
        }
        else
        {
            if((data_valid == 0)||(data_valid == 1))
            {
                if((system_get_tick10ms()-timer) > 300)
                {
                    return 0;
                }
            }
            else
            {
                if((system_get_tick10ms()-timer) > 50)
                {
                    return 0;
                }
            }
            tpos_TASK_YIELD();
        }
    }
    return (GprsObj.send_len==0)?0:1;
}
uint8_t set_data_mode_longsung(void)
{
    int16_t resault;
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+MIPMODE=0,0,0\r\n", NULL,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    { 
        return 1;
    }
    else if(resault == -2)
    {
    }
    else //if(resault == -1)
    {
        //瓒呮椂
        return 0;
    }
    return 0;
}
unsigned char hex_to_byte(char high, char low)
{
      unsigned char val=0;
      if(high >='0' && high <='9') val = high-'0';
      else if(high >='A' && high <='F') val = high-'A'+10;
      else  val = high -'a' + 10;
      val *=16;

      if(low >='0' && low <='9') val += low-'0';
      else if(low >='A' && low <='F') val += low-'A'+10;
      else  val += low -'a' + 10;

      return val;
}
int16_t str2hex(INT8U *str, INT16U len, INT8U *hex)
{
    INT16U i;
    if(len %2)  return -1;
    for(i=0; i<len; i+=2)
    {
        hex[i/2] = hex_to_byte(str[i], str[i+1]);
    }
}
int16_t hex2str(INT8U *hex, INT16U len, INT8U *str)
{
    INT8U temp;
    INT16U i;
    
    for(i=0; i<len; i++)
    {
        temp = hex[i]>>4;
        if(temp < 0x0a)
            str[i*2] = temp + '0';
        else
            str[i*2] = temp - 0x0a + 'a';
        
        temp = hex[i]&0x0f;
        if(temp < 0x0a)
            str[i*2+1] = temp + '0';
        else
            str[i*2+1] = temp - 0x0a + 'a';
    }
    
}
uint8_t gprs_send_app_data_CMI(uint8_t socket_id)
{
    int16_t  rx_data;
    uint32_t timer;
    uint16_t send_len;
    uint8_t  cmd_buf[20];
    uint8_t data_valid;
    uint16_t data_len;
    tagWirelessObj *Obj;
    int16_t pos,len,resault;
 	uint32_t val;
    char str[100];

    if(socket_id == tcp_client_socket_id)
    {
        if(gSystemInfo.tcp_link == 0)
            return 0;
        Obj = &GprsObj;
    }     
    else if(socket_id == tcp_server_socket_id)
    {
        if(gSystemInfo.remote_client_linked == 0)
            return 0;
        Obj = &GprsServerObj;
    } 
    else
        return 0;
    
    if(Obj->send_len == 0)
    {
        return 0;
    }
    send_len = Obj->send_len;
    if(send_len > 1500)
    {
        send_len = 1500;
    }
//    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+MIPSEND?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),200);
//    if(resault <0)
//	{
//		return 0;
//	}
//    sprintf(str,"+MIPSEND: %d,", socket_id);
//    pos = str_find(gprs_cmd_buffer,sizeof(gprs_cmd_buffer),str,str_cmdlen(str));
//	if(pos==-1)
//	{
//		return 0;
//	}
//	val=str2int(gprs_cmd_buffer+pos+str_cmdlen(str));
//	if(val==0)
//	{
//		return 0;
//	}
    pos = 0;
    
    sprintf((char*)gprs_cmd_buffer,"AT+NSOSD=%d,%d,", socket_id, send_len);
    gprs_uart_send_buf(gprs_cmd_buffer,str_cmdlen(gprs_cmd_buffer));
    timer = system_get_tick10ms();
    data_valid = 0;
    mem_set(cmd_buf,sizeof(cmd_buf),0);
    while(1)
    {
        rx_data = gprs_uart_read_byte();
        if(rx_data != -1)
        {
            if(gSystemInfo.gprs_debug_flag)
            {
                system_debug_data((uint8_t*)&rx_data,1);
            }
            timer = system_get_tick10ms();
            {
                mem_cpy(cmd_buf, cmd_buf+1, sizeof(cmd_buf)-1);
                cmd_buf[sizeof(cmd_buf)-1] = rx_data;
            }
        }
        else
        {
            if(pos >= 0 && gprs_uart_is_idle())
            {
                if(send_len-pos > 50)
                {
                    len = 50;
                }
                else
                {
                    len = send_len - pos;
                }
                if(len)
                {
                    hex2str(Obj->send_ptr+pos, len, str);
                    //hex2str(str, len*2, gprs_cmd_buffer);
                    //gprs_uart_send_buf(Obj->send_ptr+pos, len);
                    gprs_uart_send_buf(str, len*2);
                    timer = system_get_tick10ms();
                }
                else
                {
                    pos = -1;
                    data_len = send_len;
                    gprs_uart_send_buf("\r\n", 2);
                    timer = system_get_tick10ms();
                    continue;
                }

                pos = pos +len;
            }

            if(str_find(cmd_buf,sizeof(cmd_buf), (uint8_t*)"OK", 2) >=0)
            {
                Obj->send_ptr += data_len;
                if(Obj->send_len >= data_len)
                {
                    Obj->send_len -= data_len;
                }
                else
                {
                    Obj->send_len = 0;
                }
                if(Obj->send_len)
                {
                    return 1;
                }
                return 0;
            }

            if((system_get_tick10ms()-timer) > 200)
            {
                return 0;
            }

            tpos_TASK_YIELD();
        }
    }
    return (Obj->send_len==0)?0:1;
}

uint8_t gprs_send_app_data_CMI_onenet(void)
{
    int16_t  rx_data;
    uint32_t timer;
    uint16_t send_len,event=0;
    uint8_t  cmd_buf[20]={0};
    uint8_t data_valid,pos1=0,err=0;
    uint16_t data_len;
    tagWirelessObj *Obj;
    int16_t pos,len,resault;
 	uint32_t val;


    if(gSystemInfo.tcp_link == 0)
        return 0;
    Obj = &GprsObj;
    
    if(Obj->send_len == 0)
    {
        return 0;
    }
    send_len = Obj->send_len;
    if(send_len > 500)//本次上报不超过500bytes
    {
        send_len = 500;
    }
    pos = 0;
     
    if(gprs_send_cmd_wait_OK(Obj->send_ptr,NULL,0,200) != -2) 
    {
    Obj->send_len=0;
    return 1;
    }
    else
    {
        gSystemInfo.tcp_link = 0;
        Obj->send_len = 0;
    }
}
uint8_t gprs_readresp_app_data_CMI_onenet(void)
{
    int16_t  rx_data;
    uint32_t timer;
    uint16_t send_len;
    uint8_t  cmd_buf[20]={0};
    uint8_t data_valid;
    uint16_t data_len;
    tagWirelessObj *Obj;
    int16_t pos,len,resault;
 	uint32_t val;
    char str[100]={0};


    if(gSystemInfo.tcp_link == 0)
        return 0;
    //Obj = &GprsObj;
    Obj = &RequestOnenet;

    if(Obj->state != REQUEST_STATE_PROCESSED)
    {
        return 0;
    }
    
    send_len = Obj->send_len;
    if(send_len > 500)//本次上报不超过1000bytes
    {
        send_len = 500;
    }
    pos = 0;
    if(send_len)
    {
        if(Obj->type==STRING_DATA)
        {
            sprintf((char*)str,"AT+MIPLREADRSP=0,%ld,%d,%4d,%d,%ld,%d,%d,\"", Obj->mid,Obj->result,Obj->objid,Obj->insid,Obj->resid,Obj->type,send_len);
        }
        else
        {
            sprintf((char*)str,"AT+MIPLREADRSP=0,%ld,%d,%4d,%d,%ld,%d,%d,", Obj->mid,Obj->result,Obj->objid,Obj->insid,Obj->resid,Obj->type,send_len);        
        }
    }
    else
    {
        sprintf((char*)str,"AT+MIPLREADRSP=0,%ld,%d\r\n", Obj->mid,Obj->result);
    }
    gprs_uart_send_buf(str,str_cmdlen(str));
    timer = system_get_tick10ms();
    data_valid = 0;
    mem_set(cmd_buf,sizeof(cmd_buf),0);
    while(1)
    {
        rx_data = gprs_uart_read_byte();
        if(rx_data != -1)
        {
            if(gSystemInfo.gprs_debug_flag)
            {
                system_debug_data((uint8_t*)&rx_data,1);
            }
            timer = system_get_tick10ms();
            {
                mem_cpy(cmd_buf, cmd_buf+1, sizeof(cmd_buf)-1);
                cmd_buf[sizeof(cmd_buf)-1] = rx_data;
            }
        }
        else
        {
            if(pos >= 0 && gprs_uart_is_idle())
            {
                if(send_len-pos > 50)
                {
                    len = 50;
                }
                else
                {
                    len = send_len - pos;
                }
                if(len)
                {
                    //hex2str(Obj->send_ptr+pos, len, str);//应用层转换为string
                    //hex2str(str, len*2, gprs_cmd_buffer);
                    //gprs_uart_send_buf(Obj->send_ptr+pos, len);
                    gprs_uart_send_buf(Obj->send_ptr+pos, len);
                    timer = system_get_tick10ms();
                }
                else
                {
                    pos = -1;
                    data_len = send_len;
                    if(Obj->result==1)
                    {
                        if(Obj->type == STRING_DATA)
                        {
                            gprs_uart_send_buf("\",0,0\r\n",10);
                        }
                        else
                        {
                            gprs_uart_send_buf(",0,0\r\n",10);
                        }
                    }
                    timer = system_get_tick10ms();
                    continue;
                }

                pos = pos +len;
            }

            if(str_find(cmd_buf,sizeof(cmd_buf), (uint8_t*)"OK", 2) >=0)
            {
                Obj->send_ptr += data_len;
                if(Obj->send_len >= data_len)
                {
                    Obj->send_len -= data_len;
                }
                else
                {
                    Obj->send_len = 0;
                }
                if(Obj->send_len)
                {
                    return 1;
                }
                return 0;
            }

            if((system_get_tick10ms()-timer) > 200)
            {
                return 0;
            }

            tpos_TASK_YIELD();
        }
    }
    return (Obj->send_len==0)?0:1;
}

uint8_t gprs_send_app_data_longsung(uint8_t socket_id)
{
    int16_t  rx_data;
    uint32_t timer;
    uint16_t send_len;
    uint8_t  cmd_buf[20];
    uint8_t data_valid;
    uint16_t data_len;
    tagWirelessObj *Obj;
    int16_t pos,resault;
 	uint32_t val;
    char str[20];
    
    if(socket_id == tcp_client_socket_id)
    {
        if(gSystemInfo.tcp_link == 0)
            return 0;
        Obj = &GprsObj;
    }
        
    else if(socket_id == tcp_server_socket_id)
    {
        if(gSystemInfo.remote_client_linked == 0)
            return 0;
        Obj = &GprsServerObj;
    } 
    else
        return 0;
    
    if(Obj->send_len == 0)
    {
        return 0;
    }
    send_len = Obj->send_len;
    if(send_len > 1500)
    {
        send_len = 1500;
    }
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+MIPSEND?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),200);
    if(resault <0)
	{
		return 0;
	}
    sprintf(str,"+MIPSEND: %d,", socket_id);
    pos = str_find(gprs_cmd_buffer,sizeof(gprs_cmd_buffer),str,str_cmdlen(str));
	if(pos==-1)
	{
		return 0;
	}
	val=str2int(gprs_cmd_buffer+pos+str_cmdlen(str));
	if(val==0)
	{
		return 0;
	}
    
    sprintf((char*)gprs_cmd_buffer,"AT+MIPSEND=%d,%d\r\n", socket_id, send_len);
    gprs_uart_send_buf(gprs_cmd_buffer,str_cmdlen(gprs_cmd_buffer));
    timer = system_get_tick10ms();
    data_valid = 0;
    mem_set(cmd_buf,sizeof(cmd_buf),0);
    while(1)
    {
        rx_data = gprs_uart_read_byte();
        if(rx_data != -1)
        {
            if(gSystemInfo.gprs_debug_flag)
            {
                system_debug_data((uint8_t*)&rx_data,1);
            }
            timer = system_get_tick10ms();
            if(data_valid == 0)
            {
                //鎺ユ敹 $MYNETWRITE:
                mem_cpy(cmd_buf,cmd_buf+1,11);
                cmd_buf[11] = rx_data;
                if(rx_data == '>')
                {
                    data_valid = 1;
                    data_len = send_len;
                    gprs_uart_send_buf(Obj->send_ptr, data_len);
                    mem_set(cmd_buf, sizeof(cmd_buf), 0);
                }
            }
            else
            {
                mem_cpy(cmd_buf, cmd_buf+1, sizeof(cmd_buf)-1);
                cmd_buf[sizeof(cmd_buf)-1] = rx_data;
            }
        }
        else
        {
            if((data_valid == 0)||(data_valid == 1))
            {
                if(data_valid == 1)
                {
                    if(gprs_uart_is_idle() == 1)
                    {
                        gprs_cmd_buffer[0] = 26;//[CTRL+z]
                        gprs_uart_send_buf(gprs_cmd_buffer,1);
                        data_valid = 2;
                        timer = system_get_tick10ms();
                    }
                }
                if((system_get_tick10ms()-timer) > 200)
                {
                    return 0;
                }
            }
            else
            {
                if(str_find(cmd_buf,sizeof(cmd_buf), (uint8_t*)"OK\r\n", 4) >=0)
                {
                    Obj->send_ptr += data_len;
                    if(Obj->send_len >= data_len)
                    {
                        Obj->send_len -= data_len;
                    }
                    else
                    {
                        Obj->send_len = 0;
                    }
                    if(Obj->send_len)
                    {
                        return 1;
                    }
                    return 0;
                }
                
                if((system_get_tick10ms()-timer) > 200)
                {
                    return 0;
                }
            }
            tpos_TASK_YIELD();
        }
    }
    return (Obj->send_len==0)?0:1;
}

uint8_t gprs_send_app_data(void)
{
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        return gprs_send_app_data_quec();
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_CMI))
    {
#ifndef __ONENET__
        return gprs_send_app_data_CMI(tcp_client_socket_id);
#else
            return gprs_send_app_data_CMI_onenet();
#endif
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        return gprs_send_app_data_longsung(tcp_client_socket_id);
    }
    else
    {
        return gprs_send_app_data_1376_3();
    }
}
uint8_t gprs_server_send_app_data(void)
{
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        //return gprs_send_app_data_quec();
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
       return gprs_send_app_data_longsung(tcp_server_socket_id);
    }
    else
    {
        //return gprs_send_app_data_1376_3();
    }
    return 0;

}
uint8_t gprs_check_tcp_status(void)
{
    int16_t resault;
    int16_t pos, pos1;
    uint8_t id, tcp_link, remote_link, find;
    if(server_has_on != 0)
        server_has_on ++;
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+QISTATE?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+MIPOPEN?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_CMI))
    {
        return 1;
    }
    else
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT$MYNETOPEN?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    if(resault >= 0)
    {
        //
        if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
        {
            if(str_find(gprs_cmd_buffer,resault,(uint8_t *)"2,1,0,0",7)!=-1)
            {
                return 1;
            }
        }
        else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
        {
            pos1 = 0;
            tcp_link = 0;
            remote_link = 0;
            find = 0;
            id =0;
            pos1 = str_find(gprs_cmd_buffer, resault,(uint8_t *)"+MIPOPEN:", 9);
            if(pos1 > 0)
            {
                pos1 += 9;
                while(pos1 < resault)
                {
                    if(gprs_cmd_buffer[pos1] >= '0' && gprs_cmd_buffer[pos1] <= '9')
                    {
                        id = id*10;
                        id += gprs_cmd_buffer[pos1] - '0';
                        find = 1;
                    }
                    else
                    {
                        if(find)
                        {
                            if(id == 0)
                            {
                                tcp_link = 0;
                                remote_link = 0;
                                break;
                            }
                            if(id == tcp_client_socket_id)
                                tcp_link = 1;
                            else if(id == tcp_server_socket_id)
                                remote_link = 1;
                        }
                        find = 0;
                        id = 0;
                    }
                    pos1++;
                }
                gSystemInfo.tcp_link = tcp_link;
                gSystemInfo.remote_client_linked = remote_link;
                if(server_has_on == 3)
                {
                    if(remote_link == 0)
                        need_reset = 1;
                    
                    server_has_on = 0;
                }
                if(gSystemInfo.tcp_link == 0)
                {
                    gSystemInfo.login_status = 0;
                    tcp_client_socket_id = 0;
                }  
                if(gSystemInfo.remote_client_linked == 0)
                {
                   tcp_server_socket_id = -1;
                }
            }
              
            
//            pos1 = 0;
//            tcp_link = 0;
//            remote_link = 0;
//            while((pos1 + 10) < resault)
//            {
//                pos = str_find(gprs_cmd_buffer+pos1, resault-pos1,(uint8_t *)"+MIPSTATE:",10);
//                if(pos != -1)
//                {
//                    id = str2int(gprs_cmd_buffer + pos1 + pos +10);
//                    if(id == tcp_client_socket_id)
//                        tcp_link = 1;
//                    else if(id == tcp_server_socket_id)
//                        remote_link = 1;
//                }
//                pos1 += pos;
//                pos1 += 12;
//            }
//            gSystemInfo.tcp_link = tcp_link;
//            gSystemInfo.remote_client_linked = remote_link;
//            if(gSystemInfo.tcp_link == 0)
//            {
//                gSystemInfo.login_status = 0;
//            }  
            return 1;
        }
        else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_CMI))
        {
            return 1;
//            if((pos1 = str_find(gprs_cmd_buffer,resault,(uint8_t*)"+MIPLEVENT:0,", 13)) >= 0)
//            {
//                if ((str_find(gprs_cmd_buffer, resault, (uint8_t*) "\r\n", 2)) >= 0) 
//                {
//                    pos1 = str2int(gprs_cmd_buffer + pos1 + 13);
//                    if (pos1 == 6) 
//                    {
//                        gSystemInfo.tcp_link = 1;
//                        return 1;
//                    } 
//                    else 
//                    {
//                        gSystemInfo.tcp_link = 0;
//                        return 0;
//                    }
//                }
//            }            
        }
        else
        {
            if(str_find(gprs_cmd_buffer,resault,(uint8_t *)"0,",2)!=-1)
            {
                return 1;
            }
        }
    }
    else if(resault == -2)
    {
        //收到ERROR
        return 0;
    }
    else //if(resault == -1)
    {
        //瓒呮椂
        return 0;
    }
    return 0;
}
uint8_t gprs_check_server_state(void)
{
    int16_t resault;
    int16_t pos, pos1;
    uint8_t id, tcp_link, remote_link;
    
    return 1;
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+MIPLISTEN?\r\n", gprs_cmd_buffer, sizeof(gprs_cmd_buffer), 100);
    }
    else
    {
        return 0;
    }
    if(resault >= 0)
    {
        if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600))
        {
            if(str_find(gprs_cmd_buffer,resault,(uint8_t *)"+MIPLISTEN:", 11) != -1)
            {
                return 1;
            }
        }
    }
    else if(resault == -2)//鏀跺埌ERROR
    {  
        return 0;
    }
    else //if(resault == -1)//瓒呮椂
    {
        return 0;
    }
    return 0;
}
uint8_t gprs_check_net_status(void)
{
    static uint32_t timer=0;
    char info[30];
    
    if(second_elapsed(timer)<30)
    {
        return 0;
    }
    timer = system_get_tick10ms();
    
    if(gprs_check_netact() == 0)
        if(gprs_check_netact() == 0)
            if(gprs_check_netact() == 0)
            {
                gSystemInfo.ppp_is_ok = 0;
                gSystemInfo.tcp_link = 0;
                gSystemInfo.login_status = 0;
                return 0;
            }
    if(gprs_check_tcp_status() == 0)
        if(gprs_check_tcp_status() == 0)
            if(gprs_check_tcp_status() == 0)
    {
        gSystemInfo.tcp_link = 0;
        gSystemInfo.login_status = 0;
        return 0;
    }
    //妫?鏌ヤ竴涓嬬洃鍚湇鍔″櫒
    if(gprs_check_server_state() == 0)
    {
        gprs_start_tcp_server();
    }
    gprs_update_csq();
    if((gSystemInfo.nouse_1376_3 == 0xAA) && (gSystemInfo.remote_model_vendor = REMOTE_MODEL_VENDOR_CMI))
    {
        gprs_send_cmd_wait_OK((uint8_t *)"AT+CEREG?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
//    sprintf(info, "meter no:%x %x %x %x %x %x\r\n", gSystemInfo.meter_no[0],gSystemInfo.meter_no[1],gSystemInfo.meter_no[2]\
//                                                   ,gSystemInfo.meter_no[3],gSystemInfo.meter_no[4],gSystemInfo.meter_no[5]);
//    if(gSystemInfo.gprs_debug_flag)
//        system_debug_info(info);
    return 1;
}

void remote_send_app_frame(uint8_t *frame,uint16_t frame_len)
{
    uint32_t timer;

    //江苏混合模式 延时50秒等待连接TCP和拨号
    #ifdef __PROVINCE_JIANGSU__
    timer = system_get_tick10ms();
    while(gSystemInfo.login_status == 0)
    {
        if((system_get_tick10ms() - timer) > 50*100)
        {
            return;
        }
    }
    DelayNmSec(2000);
    #else
    if(gSystemInfo.tcp_link == 0)
    {
        return ;
    }
    
    if(gSystemInfo.login_status == 0)
    {
        return ;
    }
    #endif
    timer = system_get_tick10ms();
    
    while(tpos_mutexRequest(&SIGNAL_REMOTE_UART) == FALSE)
    {
        if((system_get_tick10ms()-timer) > MAX_REMOTE_DELAY_10MS)
        {
            return 0;
        }
        tpos_TASK_YIELD();
    }
    
    while(GprsObj.send_len)
    {
        if((system_get_tick10ms() - timer) > 300*100)
        {
            //5分钟，放弃吧
            tpos_mutexFree(&SIGNAL_REMOTE_UART);
            return ;
        }
        DelayNmSec(10);
        if(gSystemInfo.tcp_link == 0)
        {
            tpos_mutexFree(&SIGNAL_REMOTE_UART);
            return ;
        }
        if(gSystemInfo.login_status == 0)
        {
            tpos_mutexFree(&SIGNAL_REMOTE_UART);
            return ;
        }
    }
    GprsObj.send_ptr = frame;
    GprsObj.send_len = frame_len;
    
    timer = system_get_tick10ms();
    
    while(GprsObj.send_len)
    {
        gSystemInfo.remote_status = 1;
        if((system_get_tick10ms() - timer) > 300*100)
        {
            //5分钟，放弃吧
            tpos_mutexFree(&SIGNAL_REMOTE_UART);
            return ;
        }
        DelayNmSec(10);
        if(gSystemInfo.tcp_link == 0)
        {
            tpos_mutexFree(&SIGNAL_REMOTE_UART);
            return ;
        }
        if(gSystemInfo.login_status == 0)
        {
            tpos_mutexFree(&SIGNAL_REMOTE_UART);
            return ;
        }
    }
    gSystemInfo.remote_status = 0;
    tpos_mutexFree(&SIGNAL_REMOTE_UART);
    ertu_month_bytes_add(frame_len);   //这里注意的是每接受到一个字节就加len；
}
void remote_server_send_app_frame(uint8_t *frame,uint16_t frame_len)
{
        uint32_t timer;

    if(gSystemInfo.remote_client_linked == 0)
    {
        return ;
    }
    timer = system_get_tick10ms();

    while(GprsServerObj.send_len)
    {
        if((system_get_tick10ms() - timer) > 60000)
        {
            //10鍒嗛挓锛屾斁寮冨惂
            return ;
        }
        DelayNmSec(10);
        if(gSystemInfo.remote_client_linked == 0)
        {
            return ;
        }

    }

    GprsServerObj.send_ptr = frame;
    GprsServerObj.send_len = frame_len;
    timer = system_get_tick10ms();
    while(GprsServerObj.send_len)
    {
        if((system_get_tick10ms() - timer) > 60000)
        {
            //10鍒嗛挓锛屾斁寮冨惂
            return ;
        }
        DelayNmSec(10);
        if(gSystemInfo.remote_client_linked == 0)
        {
            return ;
        }

    }

}
void gprs_send_active_test(uint8_t is_first)
{
    uint16_t app_make_active_test_frame(uint8_t *frame,uint8_t is_first);
    uint16_t frame_len;
    
    if(GprsObj.send_len)
    {
        return ;
    }

    RequestRemote.recv_success_flag = 0;
    RequestRemote.send_time_in_10ms = system_get_tick10ms();
    frame_len = app_make_active_test_frame(gprs_cmd_buffer+150,is_first);
    GprsObj.send_len = frame_len;
    GprsObj.send_ptr = gprs_cmd_buffer+150;
    gprs_send_app_data();
}

//AT+MIPLUPDATE=<ref>,<lifetime>,<withObjectFlag>
//lifetime < 10 为一天
uint8_t gprs_send_active_test_onenet(void)
{
    int16_t  rx_data;
    uint32_t timer;
    uint8_t  cmd_buf[20];
    uint8_t step=0;
    char str[100]={0};

    if(GprsObj.send_len)
    {
        return 0;
    }
    
    //sprintf(str,"AT+MIPLUPDATE=0,%d,1\r\n",gSystemInfo.heart_cycle*60); //0，0，1 为1天  单位 秒
    sprintf(str,"AT+MIPLUPDATE=0,4200,0\r\n"); //0，0，1 为1天  单位 秒
    gprs_uart_send_buf(str,str_cmdlen(str));
    timer = system_get_tick10ms();
    mem_set(cmd_buf,sizeof(cmd_buf),0);
    while(1)
    {
        rx_data = gprs_uart_read_byte();
        if(rx_data != -1)
        {
            if(gSystemInfo.gprs_debug_flag)
            {
                system_debug_data((uint8_t*)&rx_data,1);
            }
            timer = system_get_tick10ms();
            {
                mem_cpy(cmd_buf, cmd_buf+1, sizeof(cmd_buf)-1);
                cmd_buf[sizeof(cmd_buf)-1] = rx_data;
            }
        }
        else
        {
            if(step == 0)
            {
                if(str_find(cmd_buf,sizeof(cmd_buf), (uint8_t*)"OK", 2) >=0)
                {
                    step = 1;
                }
            }
            else if(step == 1)
            {
                if(str_find(cmd_buf,sizeof(cmd_buf), (uint8_t*)"+MIPLEVENT:0,11", 15) >=0)
                {
                    gSystemInfo.life_time = system_get_tick10ms();
                    return 1;
                }
                if(str_find(cmd_buf,sizeof(cmd_buf), (uint8_t*)"+MIPLEVENT:0,15", 15) >=0)
                {
                    gSystemInfo.tcp_link = 0;
                    return 0;
                }                
            }

            if((system_get_tick10ms()-timer) > 500) //判断5秒超时
            {
                return 0;
            }

            tpos_TASK_YIELD();
        }
    }
}

void gprs_check_active_test(void)
{
    static INT32U timer=0;
    static uint8_t try_count=0,hour=0;
    INT8U res=0;
    tagDatetime datetime;

    if(gSystemInfo.tcp_link == 0)                                                                                                   
    {
        try_count=0;//try 6 times after each connection
        gSystemInfo.login_status = 0;
        return;
    }

    #ifdef __ONENET__
    if((gSystemInfo.tcp_link==1)&&(gSystemInfo.login_status==0))
    {
    if((second_elapsed(gSystemInfo.last_resource_ready) > 30)&&(gSystemInfo.last_resource_ready!=0))
    {
        res=gprs_send_cmd_wait_OK((uint8_t *)"AT+MIPLNOTIFY=0,0,3332,13,5750,1,1,\"0\",0,0\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);  
        if(res>=0)   
        {   
        	gSystemInfo.login_status = 1;
        	try_count = 0;
        }
    }
    }

    return;
    #else
    if(RequestRemote.recv_success_flag)
    {
        gSystemInfo.login_status = 1;
        try_count = 0;
    }    
    #endif
    
    if(gSystemInfo.login_status == 0)
    {
        if(try_count < 6)
        {
            if(second_elapsed(gSystemInfo.life_time) > 20)
            {
                #ifndef __ONENET__
                gprs_send_active_test(1);
                #else
                gprs_send_active_test_onenet();
                #endif
                try_count ++;                
            }
        }
        else
        {
            gSystemInfo.tcp_link = 0;
        }
    }
#ifndef __PROVINCE_JIANGSU__
    else
    {
        if(try_count == 0)
        {
            #ifndef __ONENET__
            if(second_elapsed(RequestRemote.recv_time_in_10ms) > gSystemInfo.heart_cycle*60)
            #else
            //if(second_elapsed(gSystemInfo.life_time) > (gSystemInfo.heart_cycle*60-30) ||  (gSystemInfo.life_time == 0)) //预留了30秒余量
            if(second_elapsed(gSystemInfo.life_time) > (24*60*60-30) ||  (gSystemInfo.life_time == 0))
            #endif
            {
                #ifndef __ONENET__
                gprs_send_active_test(0);
                #else
                gprs_send_active_test_onenet();
                #endif
                try_count ++;
            }
        }
        else
        {
            if(try_count < 6)
            {
                if(second_elapsed(gSystemInfo.life_time) > 20)
                {
                    #ifndef __ONENET__
                    gprs_send_active_test(0);
                    #else
                    gprs_send_active_test_onenet();
                    #endif
                    try_count ++;
                }
            }
            else
            {
                gSystemInfo.tcp_link = 0;
            }
        }
    }
#endif
    if(gSystemInfo.tcp_link == 0)
    {
        try_count=0;
    }
}
void force_gprs_power_off(void)
{
    
}
#include "Q1376_1/ErtuParam.h"
void update_gprs_param(void)
{
    uint8_t buf[100];
    
    fread_ertu_params(EEADDR_SET_F1,buf,6);
    if((buf[5] == 0)||(buf[5]==0xFF))
    {
        gSystemInfo.heart_cycle = 60;
    }
    else
    {
        gSystemInfo.heart_cycle = buf[5];
    }
    fread_ertu_params(EEADDR_SET_F3,buf,28);
    mem_cpy(gSystemInfo.ip1,buf,4);
    mem_cpy(gSystemInfo.port1,buf+4,2);
    mem_cpy(gSystemInfo.ip2,buf+6,4);
    mem_cpy(gSystemInfo.port2,buf+10,2);
    if((buf[12] == 0)||(buf[12] == 0xFF))
    {
//        mem_cpy(gSystemInfo.apn,(uint8_t*)"CMNET",5);
    }
    else
    {
        mem_cpy(gSystemInfo.apn,buf+12,16);
    }
    fread_ertu_params(EEADDR_SET_F16,buf,64);
    if((buf[0] == 0)||(buf[0] == 0xFF))
    {
//        mem_cpy(gSystemInfo.user,(uint8_t*)"CARD",4);
    }
    else
    {
        mem_cpy(gSystemInfo.user,buf,32);
    }
    if((buf[32] == 0)||(buf[32] == 0xFF))
    {
//        mem_cpy(gSystemInfo.pwd,(uint8_t*)"CARD",4);
    }
    else
    {
        mem_cpy(gSystemInfo.pwd,buf+32,32);
    }
    fread_ertu_params(EEADDR_CFG_DEVID,buf,4);
    mem_cpy(gSystemInfo.ertu_devid,buf,4);
    fread_ertu_params(EEADDR_SET_F8,buf,8);
    #ifdef __PROVINCE_JIANGSU__
    if( ((buf[0]>>4)&0x03) == 0)
    {
        gSystemInfo.online_mode = ONLINE_MODE_MIXED;
    }
    else
    #endif
    {
        gSystemInfo.online_mode = ONLINE_MODE_CLIENT;
    }
    gSystemInfo.ackid=0;
}
INT16U  ex_get_cmd_resp(INT8U *cmd, INT16U cmd_len, INT8U *resp, INT32U timeout)//timeout unit = s
{
   INT32U t;
   INT16S len;
   
   if(cmd_len > (sizeof(gprs_cmd_buffer)-5))
       return;
   t = OS_TIME;
   ex_cmd_apply = 0xAA;
   
   while(second_elapsed(t) < timeout)
   {
       if(ex_cmd_allow == 0xAA)
           break;
       DelayNmSec(200);
   }
   mem_cpy(gprs_cmd_buffer, cmd, cmd_len);
   gprs_cmd_buffer[cmd_len] = 0;
   gprs_cmd_buffer[cmd_len+1] = '\r';
   gprs_cmd_buffer[cmd_len+2] = '\n';
   len = gprs_send_cmd_wait_OK((uint8_t *)gprs_cmd_buffer, resp, sizeof(gprs_cmd_buffer), 200);
   
   ex_cmd_allow = 0xA0;
   if(len <0)
       len = 0;
   
   return len;
}
uint32_t tmr;
void task_1376_3_service(void)
{
    uint8_t has_work;

    //gSystemInfo.clock_ready = 1;
    //gSystemInfo.gprs_debug_flag = 1;
    //gSystemInfo.debug_info_interface = 1;
    //meter_uart_init(115200);

    gSystemInfo.is_nb_net = 1;
    gSystemInfo.is_coap_net = 1;
    gSystemInfo.no_net_tick = 0;
    
    gprs_uart_init(57600);
    
    drv_pin_gprs_onoff(1);
    drv_pin_gprs_rst(0);
    DelayNmSec(1000);
    drv_pin_gprs_rst(1);
    
//    DelayNmSec(3200);//绛夊緟涓婄數绋冲畾,A9600澶嶄綅鍚庡緢蹇氨鍙互鍝嶅簲AT鍛戒护
    has_work = 0;
    update_gprs_param();
    while(gSystemInfo.tcp_link == 0)
    {
        gprs_start_work();
        gprs_start_tcp();
        DelayNmSec(1000);//
        if(gprs_check_tcp_status() == 0)
        {
            gSystemInfo.tcp_link = 0;
            gSystemInfo.login_status = 0;
        }
    }
    #ifndef __ONENET__
    gprs_send_active_test(1);
    #endif
    RequestRemote.recv_time_in_10ms = system_get_tick10ms();
#ifdef __PROVINCE_JIANGSU__
    gprs_start_tcp_server();
#endif

    while(1)
    {
        ClrTaskWdt();
        has_work = 0;
        gprs_check_active_test();
        gprs_check_net_status();
		gprs_get_rsrp_sinr_cellid();
        if(ex_cmd_apply == 0xAA)
        {
            ex_cmd_allow= 0xAA;
            ex_cmd_apply = 0;
            while(ex_cmd_allow != 0xA0)
            {
                DelayNmSec(200);
            }
            ex_cmd_allow = 0;
        }        
        if(gSystemInfo.nouse_1376_3 == 0xAA)
        {
            if(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_M5310 || gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600)
                check_report();
        }
        
        if(gSystemInfo.tcp_link == 0)
        {
            GprsObj.send_len = 0;
            gprs_power_off();
            while(gSystemInfo.tcp_link == 0)
            {
                gprs_start_work();
                gprs_start_tcp();
            }
            #ifndef __ONENET__
            gprs_send_active_test(DT_F1);
            #else
            gprs_send_active_test_onenet();
            #endif
        }
        else
        {
            //已经建立tcp连接
            has_work += gprs_read_app_data();
            if(gSystemInfo.login_status)
            {
                has_work += gprs_send_app_data();
            }
        }

        if(has_work == 0)
        {
            DelayNmSec(200);
        }
        else
        {
            tpos_TASK_YIELD();
        }
    }
}
