#include "main_include.h"
#include "stdio.h"
#include "string.h"

struct{
    uint8_t ver;
    uint8_t type;
    uint8_t tkl;
    uint8_t code;
    uint16_t mid;
    uint8_t Token[16];
    uint8_t *options;
    uint16_t options_len;
    uint8_t *payload;
    uint16_t payload_len;
}recv_coap,send_coap;


uint16_t coap_send_mid=0;
uint8_t  coap_token_app[16];
uint8_t  coap_tkl_app;


uint8_t gprs_cmd_buffer[200];
int16_t gprs_send_cmd_wait_OK(uint8_t *cmd,uint8_t *resp,uint16_t max_resp_len,uint16_t timeout_10ms);

struct{
    uint8_t *send_ptr;
    uint16_t send_len;
    uint16_t recv_pos;
    uint16_t read_pos;
    uint8_t  recv_buf[SIZE_OF_APP_PROTOCOL_FRAME];
}GprsObj;
int16_t remote_read_byte(void)
{
    int16_t recv;
    
    if(GprsObj.read_pos == GprsObj.recv_pos)
    {
        return -1;
    }
    recv = GprsObj.recv_buf[GprsObj.read_pos];
    GprsObj.read_pos ++;
    if(GprsObj.read_pos >= sizeof(GprsObj.recv_buf))
    {
        GprsObj.read_pos = 0;
    }
    return recv;
}
void gprs_power_off(void)
{
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        gprs_send_cmd_wait_OK((uint8_t *)"AT$QCPWRDN\r",NULL,0,100);
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        gprs_send_cmd_wait_OK((uint8_t *)"AT+QPOWD=1\r",NULL,0,100);
    }
    else
    {
        gprs_send_cmd_wait_OK((uint8_t *)"AT$MYPOWEROFF\r",NULL,0,100);
    }
    DelayNmSec(15000);//15s必须完全关机
}
void gprs_power_on(void)
{
    drv_pin_gprs_onoff(0);
    DelayNmSec(1000); //1s±100ms
    drv_pin_gprs_onoff(1);
    DelayNmSec(15000);//10s必须完成初始化
}

uint8_t coap_send_info[100+4*SIZE_OF_APP_PROTOCOL_FRAME_COAP+20];
uint8_t coap_report_info[4*SIZE_OF_APP_PROTOCOL_FRAME_COAP+20];
uint16_t coap_report_pos=0;
uint16_t coap_report_len=0;
/*
 * 返回值 -1  没有收到命令响应 OK
 * 返回值 -2 收到ERROR
 * 返回值 >=0 收到OK,返回值为收到的报文长度
 */
int16_t gprs_send_cmd_wait_OK(uint8_t *cmd,uint8_t *resp,uint16_t max_resp_len,uint16_t timeout_10ms)
{
    uint32_t timer;
    uint32_t byte_timer;
    uint16_t cmd_len;
    int16_t  rx_data;
    uint8_t  cmd_buffer[8];
    int16_t  resp_len;
    int16_t  resault;
    
    cmd_len = str_cmdlen(cmd);
    gprs_uart_send_buf(cmd,cmd_len);
    timer = system_get_tick10ms();
    mem_set(cmd_buffer,sizeof(cmd_buffer),0);
    resp_len = 0;
    resault = -1;
    while(1)
    {
        rx_data = gprs_uart_read_byte();
        
        if(rx_data != -1)
        {
            byte_timer = system_get_tick10ms();
            
            if(coap_report_len == 0)
            {
                if((coap_report_pos)&&(coap_report_pos < sizeof(coap_report_info)))
                {
                    coap_report_info[coap_report_pos++] = rx_data;
                    if((coap_report_info[coap_report_pos-1]==0x0a)&&(coap_report_info[coap_report_pos-2]==0x0d))
                    {
                        if(gSystemInfo.gprs_debug_flag)
                        {
                            system_debug_data(coap_report_info,coap_report_pos);
                        }
                        coap_report_len = coap_report_pos;
                        coap_report_pos = 0;
                    }
                    continue;
                }
                else if(coap_report_pos >= sizeof(coap_report_info))
                {
                    coap_report_pos = 0;
                }
                if(coap_report_pos == 0)
                {
                    if(    (resault != -1)
                         ||((resault == -1)&&(resp_len<16)))
                    {
                        mem_cpy(coap_report_info,coap_report_info+1,14);
                        coap_report_info[14] = rx_data;
                        if(str_find(coap_report_info,15,(uint8_t *)"\r\n+LSIPRCOAP:1,",15)!=-1)
                        {
                            coap_report_pos = 15;
                            timer = system_get_tick10ms();
                            continue;
                        }
                    }
                }
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
        }
        if(resp == NULL)
        {
            if((system_get_tick10ms() - byte_timer) > 20)
            {
                if(resault != -1)
                {
                    if(gSystemInfo.gprs_debug_flag)
                    {
                        system_debug_data(cmd_buffer,sizeof(cmd_buffer));
                    }
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
                    if(gSystemInfo.gprs_debug_flag)
                    {
                        system_debug_data(resp,resp_len);
                    }
                    return resp_len;
                }
                if(str_find(resp,resp_len,(uint8_t *)"ERROR\r\n",4)!=-1)
                {
                    if(gSystemInfo.gprs_debug_flag)
                    {
                        system_debug_data(resp,resp_len);
                    }
                    return -2;
                }
                if(gSystemInfo.gprs_debug_flag)
                {
                    system_debug_data(resp,resp_len);
                }
            }
        }
        
        if((system_get_tick10ms() - timer) > timeout_10ms)
        {
            return -1;
        }
        if(rx_data == -1)
        {
            tpos_TASK_YIELD();
        }
        
    }
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
    if(gprs_at_check_baudrate(115200))
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
        if(gprs_send_cmd_wait_OK((uint8_t *)"AT+IPR=115200\r\n",NULL,0,100) >= 0)
        {
            if(gprs_at_check_baudrate(115200))
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
    else
    {
        if(str_find(buffer,len,(uint8_t*)"A9500",4) >= 0)
        {
            gSystemInfo.remote_model_type = REMOTE_MODEL_TYPE_A9500;
            resault = 1;
        }
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
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT$MYGMR?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
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
uint8_t gprs_check_cpin(void)
{
    int16_t resault;
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CPIN?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
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
    return 0;
}
uint8_t gprs_get_ccid(void)
{
    int16_t resault;
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9500))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+ICCID\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+QCCID\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
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
    
    if(len>15)
    {
        pos = str_find(buffer,len,(uint8_t*)"\x0d\x0a",2);
        if(pos >= 0)
        {
            buffer += (pos+2);
            len -= (pos+2);
            pos = str_find(buffer,len,(uint8_t*)"\x0d\x0a",2);
            if(pos>0)
            {
                if(pos==15)
                {
                    mem_cpy(sim_id,buffer,15);
                    mem_cpy(gSystemInfo.imei_info,sim_id,15);
                    return 1;
                }
            }
        } 
    }
    return 0;
}
uint8_t gprs_get_imei(void)
{
    int16_t resault;
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CGSN\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    
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
uint8_t gprs_set_apn(void)
{
    int16_t resault;
    uint8_t idx;
    uint8_t pos;
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        return 1;
        mem_cpy(gprs_cmd_buffer,(uint8_t *)"AT+LSIPPROFILE=1,\"",18);
        pos = 18;
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
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        return 0;
    }
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CREG?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
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
uint8_t gprs_check_cereg(void)
{
    int16_t resault;
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CEREG?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        //
        if(  (-1 != str_find(gprs_cmd_buffer,resault,(uint8_t*)"1,1",3))
           ||(-1 != str_find(gprs_cmd_buffer,resault,(uint8_t*)"4,1",3)) )
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
uint8_t gprs_check_psrat(void)
{
    int16_t resault;
    
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+PSRAT\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        //
        if(  (-1 != str_find(gprs_cmd_buffer,resault,(uint8_t*)"1",1))
           ||(-1 != str_find(gprs_cmd_buffer,resault,(uint8_t*)"2",1)) )
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
uint8_t gprs_set_netact(void)
{
    int16_t resault;
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+LSIPCALL=1\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),300);
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
uint8_t gprs_set_cgreg(void)
{
    int16_t resault;
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CGREG=1\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
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
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CEREG=1\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
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
uint8_t update_ppp_ip_info_longshang(INT8U* buffer,INT16U len)
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
        pos1 =  str_find(buffer+pos,len-pos,(uint8_t*)"\x0d\x0a",2);
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
uint8_t gprs_check_netact(void)
{
    int16_t resault;
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+LSIPCALL?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
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
        //
        if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
        {
            if(-1 != str_find(gprs_cmd_buffer,resault,(uint8_t*)"1,",2))
            {
                update_ppp_ip_info_longshang(gprs_cmd_buffer,resault);
                return 1;
            }
        }
        else if(-1 != str_find(gprs_cmd_buffer,resault,(uint8_t*)",1",2))
        {
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
uint8_t gprs_set_udp_param(uint8_t *ip,uint8_t *port)
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
    sprintf((char *)gprs_cmd_buffer,"AT+LSIPOPEN=1,5000,\"%d.%d.%d.%d\",%ld,1\r\n",ip[0],ip[1],ip[2],ip[3],port_tmp);
    resault = gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        if(str_find(gprs_cmd_buffer,resault,(uint8_t *)"1,1",3)!=-1)
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
/*
    0                   1                   2                   3   
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1   
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
 * |Ver| T |  TKL  |      Code     |          Message ID           |   
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
 * |   Token (if any, TKL bytes) ...    
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
 * |   Options (if any) ...    
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
 * |1 1 1 1 1 1 1 1|    Payload (if any) ...    
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * ver      2bits固定为01
 * T（Type）2bits 00~11 CON，NON，ACK，RST
 * TKL（Token Len） 4bits 目前使用0~8；9~15保留
 * Code 8bits = 3bits.5bits 
 *              Class 0   request
 *              【0.01】GET：获得某资源
 *              【0.02】POST：创建某资源
 *              【0.03】PUT：更新某资源
 *              【0.04】DELETE：删除某资源
 *              Class 2   success
 *              【2.01】Created
 *              【2.02】Deleted
 *              【2.03】Valid
 *              【2.04】Changed
 *              【2.05】Content。类似于HTTP 200 OK
 *              Class 4 client error
 *              【4.00】Bad Request 请求错误，服务器无法处理。类似于HTTP 400
 *              【4.01】Unauthorized 没有范围权限。类似于HTTP 401
 *              【4.02】Bad Option 请求中包含错误选项
 *              【4.03】Forbidden 服务器拒绝请求。类似于HTTP 403
 *              【4.04】Not Found 服务器找不到资源。类似于HTTP 404
 *              【4.05】Method Not Allowed 非法请求方法。类似于HTTP 405
 *              【4.06】Not Acceptable 请求选项和服务器生成内容选项不一致。类似于HTTP 406
 *              【4.12】Precondition Failed 请求参数不足。类似于HTTP 412
 *              【4.15】Unsuppor Conten-Type 请求中的媒体类型不被支持。类似于HTTP 415
 *              Class 5 server error
 *              【5.00】Internal Server Error 服务器内部错误。类似于HTTP 500
 *              【5.01】Not Implemented 服务器无法支持请求内容。类似于HTTP 501
 *              【5.02】Bad Gateway 服务器作为网关时，收到了一个错误的响应。类似于HTTP 502
 *              【5.03】Service Unavailable 服务器过载或者维护停机。类似于HTTP 503
 *              【5.04】Gateway Timeout 服务器作为网关时，执行请求时发生超时错误。类似于HTTP 504
 *              【5.05】Proxying Not Supported 服务器不支持代理功能
 * Message ID 16bits 报文编号。每个CoAP消息都有一个消息ID，在一次会话中ID总是保持不变，但在此会话结束后该ID会被回收利用
 * Token      标识符具体内容，通过TKL指定Token长度
 * Option     报文选项，包括CoAP主机、端口号、资源路径、资源参数等。类似于HTTP请求头
 * 
                 +--------+------------------+-----------+
                 | Number | Name             | Reference |
                 +--------+------------------+-----------+
                 |      0 | (Reserved)       | [RFC7252] |
                 |      1 | If-Match         | [RFC7252] |
                 |      3 | Uri-Host         | [RFC7252] |
                 |      4 | ETag             | [RFC7252] |
                 |      5 | If-None-Match    | [RFC7252] |
                 |      7 | Uri-Port         | [RFC7252] |
                 |      8 | Location-Path    | [RFC7252] |
                 |     11 | Uri-Path         | [RFC7252] |
                 |     12 | Content-Format   | [RFC7252] |
                 |     14 | Max-Age          | [RFC7252] |
                 |     15 | Uri-Query        | [RFC7252] |
                 |     17 | Accept           | [RFC7252] |
                 |     20 | Location-Query   | [RFC7252] |
                 |     35 | Proxy-Uri        | [RFC7252] |
                 |     39 | Proxy-Scheme     | [RFC7252] |
                 |     60 | Size1            | [RFC7252] |
                 |    128 | (Reserved)       | [RFC7252] |
                 |    132 | (Reserved)       | [RFC7252] |
                 |    136 | (Reserved)       | [RFC7252] |
                 |    140 | (Reserved)       | [RFC7252] |
                 +--------+------------------+-----------+
 * type12 content format
 * +--------------------------+----------+----+------------------------+
   | Media type               | Encoding | ID | Reference              |
   +--------------------------+----------+----+------------------------+
   | text/plain;              | -        |  0 | [RFC2046] [RFC3676]    |
   | charset=utf-8            |          |    | [RFC5147]              |
   | application/link-format  | -        | 40 | [RFC6690]              |
   | application/xml          | -        | 41 | [RFC3023]              |
   | application/octet-stream | -        | 42 | [RFC2045] [RFC2046]    |
   | application/exi          | -        | 47 | [REC-exi-20140211]     |
   | application/json         | -        | 50 | [RFC7159]              |
   +--------------------------+----------+----+------------------------+
 * 11111111   CoAP报文和具体负载之间的分隔符
 * Payload
*/
uint8_t gprs_send_coap(void)
{
    //设置ver
    sprintf((char*)gprs_cmd_buffer,"AT+LSIPCOAPSET=\"Ver\",\"3%d\"\r\n",send_coap.ver);
    if(gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100) < 0)
    {
        sprintf((char*)gprs_cmd_buffer,"AT+LSIPCOAPSET=\"Ver\",\"3%d\"\r\n",send_coap.ver);
        if(gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100) < 0)
        {
            sprintf((char*)gprs_cmd_buffer,"AT+LSIPCOAPSET=\"Ver\",\"3%d\"\r\n",send_coap.ver);
            if(gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100) < 0)
            {
                return 0;
            }
        }
    }
    //设置Type
    sprintf((char*)gprs_cmd_buffer,"AT+LSIPCOAPSET=\"Type\",\"3%d\"\r\n",send_coap.type);
    if(gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100) < 0)
    {
        return 0;
    }
    //设置TKL
    sprintf((char*)gprs_cmd_buffer,"AT+LSIPCOAPSET=\"TKL\",\"3%d\"\r\n",send_coap.tkl);
    if(gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100) < 0)
    {
        return 0;
    }
    //设置Code
    sprintf((char*)gprs_cmd_buffer,"AT+LSIPCOAPSET=\"Code\",\"3%d3%d3%d\"\r\n",send_coap.code/100,send_coap.code%100/10,send_coap.code%10);
    if(gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100) < 0)
    {
        return 0;
    }
    //设置Message ID
    sprintf((char*)gprs_cmd_buffer,"AT+LSIPCOAPSET=\"MID\",\"3%d3%d3%d3%d3%d\"\r\n",send_coap.mid/10000,send_coap.mid%10000/1000,send_coap.mid%1000/100,send_coap.mid%100/10,send_coap.mid%10);
    if(gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100) < 0)
    {
        return 0;
    }
    //设置Token
    mem_cpy(gprs_cmd_buffer,"AT+LSIPCOAPSET=\"Token\",\"",24);
    mem_cpy(gprs_cmd_buffer+24,send_coap.Token,send_coap.tkl*2);
    mem_cpy(gprs_cmd_buffer+24+send_coap.tkl*2,"\"\r\n",4);
    if(gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100) < 0)
    {
        return 0;
    }
    //设置Options 48
    if((send_coap.options != NULL)&&(send_coap.options_len))
    {
        mem_cpy(gprs_cmd_buffer,(uint8_t *)"AT+LSIPCOAPSET=\"Options\",\"",26);
        mem_cpy(gprs_cmd_buffer+26,send_coap.options,send_coap.options_len);
        mem_cpy(gprs_cmd_buffer+26+send_coap.options_len,"\"\r\n",4);
        if(gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100) < 0)
        {
            return 0;
        }
    }
    if((send_coap.payload != NULL)&&(send_coap.payload_len))
    {
        mem_cpy(coap_send_info,(uint8_t *)"AT+LSIPCOAPSET=\"Payload\",\"",26);
        mem_cpy(coap_send_info+26,send_coap.payload,send_coap.payload_len);
        mem_cpy(coap_send_info+26+send_coap.payload_len,"\"\r\n",4);
        if(gprs_send_cmd_wait_OK(coap_send_info,coap_send_info,sizeof(coap_send_info),100) < 0)
        {
            return 0;
        }
    }
    //send
    if(gprs_send_cmd_wait_OK((uint8_t *)"AT+LSIPCOAPSEND=1\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100) < 0)
    {
        return 0;
    }
    return 1;
}
void process_coap_recv_frame(void)
{
    int16_t pos;
    int16_t idx;
    uint16_t data_idx;
    uint8_t coap_tag[10];
    
    mem_set(&recv_coap,sizeof(recv_coap),0xFF);
    pos = 0;
    while(pos < coap_report_len)
    {
        if(coap_report_info[pos]==',')
        {
            pos ++;
            mem_set(coap_tag,sizeof(coap_tag),0);
            for(idx=0;idx<sizeof(coap_tag);idx++)
            {
                if(coap_report_info[pos+idx] == '=')
                {
                    break;
                }
                coap_tag[idx] = coap_report_info[pos+idx];
            }
            if(idx<sizeof(coap_tag))
            {
                //找到一个命令
                if((idx==4)&&(compare_string(coap_tag,(uint8_t*)"type",4)==0))
                {
                    pos += 5;
                    recv_coap.type = coap_report_info[pos]-'0';
                    pos ++;
                }
                else if((idx==4)&&(compare_string(coap_tag,(uint8_t*)"code",4)==0))
                {
                    pos += 5;
                    recv_coap.code = coap_report_info[pos]-'0';
                    pos ++;
                    if(coap_report_info[pos] == '.')
                    {
                        pos ++;
                        recv_coap.code <<= 5;
                        recv_coap.code += (coap_report_info[pos]-'0');
                        pos ++;
                    }
                }
                else if((idx==3)&&(compare_string(coap_tag,(uint8_t*)"mid",3)==0))
                {
                    pos += 4;
                    recv_coap.mid = coap_report_info[pos]-'0';
                    pos ++;
                    while(  (coap_report_info[pos]>='0') &&(coap_report_info[pos]<='9'))
                    {
                        recv_coap.mid *= 10;
                        recv_coap.mid += (coap_report_info[pos]-'0');
                        pos ++;
                    }
                }
                else if((idx==5)&&(compare_string(coap_tag,(uint8_t*)"token",5)==0))
                {
                    pos += 6;
                    recv_coap.tkl = 0;
                    while(    ((coap_report_info[pos]>='0') &&(coap_report_info[pos]<='9'))
                            ||((coap_report_info[pos]>='a') &&(coap_report_info[pos]<='f'))
                            ||((coap_report_info[pos]>='A') &&(coap_report_info[pos]<='F')) )
                    {
                        recv_coap.Token[recv_coap.tkl] = coap_report_info[pos];
                        recv_coap.tkl ++;
                        pos ++;
                    }
                    recv_coap.tkl /= 2;
                }
                else if((idx==6)&&(compare_string(coap_tag,(uint8_t*)"option",6)==0))
                {
                    pos += 7;
                    recv_coap.options = coap_report_info+pos;
                    recv_coap.options_len = 0;
                    while(    ((coap_report_info[pos]>='0') &&(coap_report_info[pos]<='9'))
                            ||((coap_report_info[pos]>='a') &&(coap_report_info[pos]<='f'))
                            ||((coap_report_info[pos]>='A') &&(coap_report_info[pos]<='F')) )
                    {
                        recv_coap.options_len ++;
                        pos ++;
                    }
                }
                else if((idx==7)&&(compare_string(coap_tag,(uint8_t*)"payload",7)==0))
                {
                    pos += 8;
                    recv_coap.payload = coap_report_info+pos;
                    recv_coap.payload_len = 0;
                    while(    ((coap_report_info[pos]>='0') &&(coap_report_info[pos]<='9'))
                            ||((coap_report_info[pos]>='a') &&(coap_report_info[pos]<='f'))
                            ||((coap_report_info[pos]>='A') &&(coap_report_info[pos]<='F')) )
                    {
                        recv_coap.payload_len ++;
                        pos ++;
                    }
                    recv_coap.payload_len = Hex2Binary(recv_coap.payload,recv_coap.payload_len,recv_coap.payload);
                    if(recv_coap.payload_len != 0xFFFF)
                    {
                        if(recv_coap.payload[0] == 0x02)
                        {
                            recv_coap.payload ++;
                            recv_coap.payload_len --;
                            if((recv_coap.payload[0]*0x100+recv_coap.payload[1]+2) == recv_coap.payload_len)
                            {
                                recv_coap.payload += 2;
                                recv_coap.payload_len -= 2;
                                recv_coap.payload_len = Hex2Binary(recv_coap.payload,recv_coap.payload_len,recv_coap.payload);
                                if(recv_coap.payload_len != 0xFFFF)
                                {
                                    tpos_enterCriticalSection();
                                    for(data_idx=0;data_idx<recv_coap.payload_len;data_idx++)
                                    {   
                                        GprsObj.recv_buf[GprsObj.recv_pos] = recv_coap.payload[data_idx];
                                        GprsObj.recv_pos ++;
                                        if(GprsObj.recv_pos >= sizeof(GprsObj.recv_buf))
                                        {
                                            GprsObj.recv_pos = 0;
                                        }
                                    }
                                    tpos_leaveCriticalSection();
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                pos ++;
            }
        }
        else
        {
            pos ++;
        }
    }
    if(recv_coap.type == 0)
    {
        if(recv_coap.code == ((0<<5)+1))
        {
            coap_tkl_app = recv_coap.tkl;
            mem_cpy(coap_token_app,recv_coap.Token,16);
            gSystemInfo.tcp_link = 1;
        }
        
        mem_set(&send_coap,sizeof(send_coap),0);
        send_coap.ver = 1;
        send_coap.type = 2;
        send_coap.tkl = recv_coap.tkl;
        mem_cpy(send_coap.Token,recv_coap.Token,16);
        send_coap.code = (2<<5) + 4;
        send_coap.mid = recv_coap.mid;
        send_coap.options = NULL;
        send_coap.options_len = 0;
        send_coap.payload = NULL;
        send_coap.payload_len = 0;
        gprs_send_coap();
    }
}
uint8_t gprs_coap_register(void)
{
    uint32_t tmp;
    uint32_t timer;
    uint8_t coap_options[80];
    
    mem_set(&send_coap,sizeof(send_coap),0);
    send_coap.ver = 1;
    send_coap.type = 0;
    send_coap.tkl = 8;
    
    mem_cpy(send_coap.Token,"546F707300000000",16);
    tmp = system_get_tick10ms();
    send_coap.Token[8] = '3';
    send_coap.Token[9] = tmp%10 + '0';
    send_coap.Token[10] = '3';
    send_coap.Token[11] = tmp/10%10 + '0';
    send_coap.Token[12] = '3';
    send_coap.Token[13] = tmp/100%10 + '0';
    send_coap.Token[14] = '3';
    send_coap.Token[15] = tmp/1000%10 + '0';
    
    send_coap.code = 2;
    coap_send_mid = tmp&0xFFFF;
    send_coap.mid = coap_send_mid++;
    mem_cpy(coap_options,"b1740172112a3d0565703d000102030405060708090A0B0C0D0E",54);
    Binary2HexString(gSystemInfo.imei_info,15,coap_options+22);
    send_coap.options_len = 52;
    send_coap.options = coap_options;
    send_coap.payload = NULL;
    send_coap.payload_len = 0;
    timer = system_get_tick10ms();
    if(gprs_send_coap())
    {
        while(second_elapsed(timer)<300)
        {
            gprs_send_cmd_wait_OK(NULL,NULL,0,100);
            if(coap_report_len)
            {
                process_coap_recv_frame();
                Nop();
                coap_report_len = 0;
                mem_set(coap_report_info,sizeof(coap_report_info),0);
                if(gSystemInfo.tcp_link)
                {
                    return 1;
                }
            }
            DelayNmSec(10);
        }
    }
    return 0;
}
void gprs_start_work(void)
{
    uint8_t onoff_count=0;
    uint8_t retry_flag=0;
    uint8_t try_count;
    uint8_t ip[4];
    uint8_t port[2];
    
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
        if(gprs_at_check() == 0)
        {
            gprs_power_off();
            continue;
        }
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
            if(gprs_get_ati() == 0)
                if(gprs_get_ati() == 0)
                    if(gprs_get_ati() == 0)
                    {
                        gprs_power_off();
                        continue;
                    }
            if(   (gSystemInfo.remote_model_vendor != REMOTE_MODEL_VENDOR_LONGSHANG)
                ||(gSystemInfo.remote_model_type != REMOTE_MODEL_TYPE_A9500))
            {
                continue;
            }
        }
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
        if(gprs_update_csq() == 0)
            if(gprs_update_csq() == 0)
                if(gprs_update_csq() == 0)
                {
                    gprs_power_off();
                    continue;
                }
        update_gprs_param();
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
        if(gprs_set_cgreg() == 0)
            if(gprs_set_cgreg() == 0)
                if(gprs_set_cgreg() == 0)
                {
                    gprs_power_off();
                    continue;
                }
        if(gprs_set_cereg() == 0)
            if(gprs_set_cereg() == 0)
                if(gprs_set_cereg() == 0)
                {
                    gprs_power_off();
                    continue;
                }
        try_count = 0;
        retry_flag = 0;
        while(1)
        {
            //if((gprs_check_creg() == 0)&&(gprs_check_cgreg() == 0)&&(gprs_check_cereg() == 0))
            if(gprs_check_cereg() == 0)
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
        gprs_check_psrat();
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
//            if((gprs_check_creg() == 0)&&(gprs_check_cgreg() == 0))
            if(gprs_check_cereg() == 0)
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
        while(1)
        {
            ClrTaskWdt();
            update_gprs_param();
            if(    (check_is_valid_ipaddr(gSystemInfo.ip1)&&check_is_valid_port(gSystemInfo.port1))
                || (check_is_valid_ipaddr(gSystemInfo.ip2)&&check_is_valid_port(gSystemInfo.port2))  )
            {
                break;
            }
            DelayNmSec(1000);
            if(gprs_check_netact() == 0)
            {
                retry_flag = 1;
                break;
            }
//            if((gprs_check_creg() == 0)&&(gprs_check_cgreg() == 0))
            if(gprs_check_cereg() == 0)
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
        if(check_is_valid_ipaddr(gSystemInfo.ip1)&&check_is_valid_port(gSystemInfo.port1))
        {
            mem_cpy(ip,gSystemInfo.ip1,4);
            mem_cpy(port,gSystemInfo.port1,2);
            if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
            {
                if(gprs_set_udp_param(ip,port))
                {
                    gSystemInfo.login_status = 0;
                    gSystemInfo.tcp_link = 0;
                    if(gprs_coap_register())
                    {
                        return ;
                    }
                }
            }
        }
        if(check_is_valid_ipaddr(gSystemInfo.ip2)&&check_is_valid_port(gSystemInfo.port2))
        {
            mem_cpy(ip,gSystemInfo.ip2,4);
            mem_cpy(port,gSystemInfo.port2,2);
            if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
            {
                if(gprs_set_udp_param(ip,port))
                {
                    gSystemInfo.login_status = 0;
                    gSystemInfo.tcp_link = 0;
                    if(gprs_coap_register())
                    {
                        return ;
                    }
                }
            }
        }
        gprs_power_off();
    }
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
uint8_t gprs_read_app_data_longshang(void)
{
    gprs_send_cmd_wait_OK(NULL,NULL,0,100);
    if(coap_report_len)
    {
        process_coap_recv_frame();
        coap_report_len = 0;
        mem_set(coap_report_info,sizeof(coap_report_info),0);
    }
    return 0;
}
uint8_t gprs_read_app_data(void)
{
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        return gprs_read_app_data_longshang();
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        return gprs_read_app_data_quec();
    }
    else
    {
        return gprs_read_app_data_1376_3();
    }
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
uint8_t gprs_send_app_data_longshang(void)
{
    uint16_t app_send_len;
    uint16_t coap_send_len;
    
    if(gSystemInfo.tcp_link == 0)
    {
        return 0;
    }
    if(GprsObj.send_len == 0)
    {
        return 0;
    }
    app_send_len = GprsObj.send_len;
    if(app_send_len > (SIZE_OF_APP_PROTOCOL_FRAME_COAP+10))
    {
        app_send_len = SIZE_OF_APP_PROTOCOL_FRAME_COAP+10;
    }
    
    mem_set(&send_coap,sizeof(send_coap),0);
    send_coap.ver = 1;
    send_coap.type = 1;
    send_coap.tkl = coap_tkl_app;
    mem_cpy(send_coap.Token,coap_token_app,16);
    send_coap.code = (2<<5) + 5;
    send_coap.mid = coap_send_mid++;
    send_coap.options = NULL;
    if(send_coap.options == NULL)
    {
        send_coap.options_len = 0;
    }
    else
    {
        send_coap.options_len = str_cmdlen(send_coap.options);
    }
    coap_send_len = Binary2HexString(GprsObj.send_ptr,app_send_len,coap_send_info+100+2*SIZE_OF_APP_PROTOCOL_FRAME_COAP+3);
    coap_send_info[100+2*SIZE_OF_APP_PROTOCOL_FRAME_COAP] = 0x01;
    coap_send_info[100+2*SIZE_OF_APP_PROTOCOL_FRAME_COAP+1] = coap_send_len>>8;
    coap_send_info[100+2*SIZE_OF_APP_PROTOCOL_FRAME_COAP+2] = coap_send_len;
    coap_send_len += 3;
    coap_send_len = Binary2HexString(coap_send_info+100+2*SIZE_OF_APP_PROTOCOL_FRAME_COAP,coap_send_len,coap_send_info+100);
    send_coap.payload = coap_send_info+100;
    send_coap.payload_len = coap_send_len;
    
    if(gprs_send_coap())
    {
        GprsObj.send_len -= app_send_len;
        GprsObj.send_ptr += app_send_len;
    }
    
    return (GprsObj.send_len==0)?0:1;
}
uint8_t gprs_send_app_data(void)
{
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        return gprs_send_app_data_longshang();
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        return gprs_send_app_data_quec();
    }
    else
    {
        return gprs_send_app_data_1376_3();
    }
}
uint8_t gprs_check_tcp_status(void)
{
    int16_t resault;
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        return 1;
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+QISTATE?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
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
        //超时
        return 0;
    }
    return 0;
}
uint8_t gprs_check_net_status(void)
{
    static uint32_t timer=0;
    
    if(second_elapsed(timer)<10)
    {
        return 0;
    }
    timer = system_get_tick10ms();
    if(gprs_check_netact() == 0)
    {
        gSystemInfo.tcp_link = 0;
        gSystemInfo.login_status = 0;
        return 0;
    }
    if(gprs_check_tcp_status() == 0)
    {
        gSystemInfo.tcp_link = 0;
        gSystemInfo.login_status = 0;
        return 0;
    }
    gprs_update_csq();
    return 1;
}
void remote_send_app_frame(uint8_t *frame,uint16_t frame_len)
{
    uint32_t timer;

    //江苏混合模式 延时5秒等待连接TCP
    #ifdef __PROVICE_JIANGSU__
    timer = system_get_tick10ms();
    while(gSystemInfo.tcp_link == 0)
    {
        if((system_get_tick10ms() - timer) > 500)
        {
            return;
        }
    }
    #else
    if(gSystemInfo.tcp_link == 0)
    {
        return ;
    }
    #endif
    if(gSystemInfo.login_status == 0)
    {
        return ;
    }
    timer = system_get_tick10ms();
    while(GprsObj.send_len)
    {
        if((system_get_tick10ms() - timer) > 60000)
        {
            //10分钟，放弃吧
            return ;
        }
        DelayNmSec(10);
        if(gSystemInfo.tcp_link == 0)
        {
            return ;
        }
        if(gSystemInfo.login_status == 0)
        {
            return ;
        }
    }
    GprsObj.send_ptr = frame;
    GprsObj.send_len = frame_len;
    timer = system_get_tick10ms();
    while(GprsObj.send_len)
    {
        if((system_get_tick10ms() - timer) > 60000)
        {
            //10分钟，放弃吧
            return ;
        }
        DelayNmSec(10);
        if(gSystemInfo.tcp_link == 0)
        {
            return ;
        }
        if(gSystemInfo.login_status == 0)
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
void gprs_check_active_test(void)
{
    static uint8_t try_count=0;
    
    if(RequestRemote.recv_success_flag)
    {
        gSystemInfo.login_status = 1;
        try_count = 0;
    }
    if(gSystemInfo.login_status == 0)
    {
        if(try_count < 6)
        {
            if(second_elapsed(RequestRemote.send_time_in_10ms) > 60)
            {
                gprs_send_active_test(1);
                try_count ++;
            }
        }
        else
        {
            gSystemInfo.tcp_link = 0;
        }
    }
    else
    {
        if(try_count == 0)
        {
            if(second_elapsed(RequestRemote.recv_time_in_10ms) > gSystemInfo.heart_cycle*60)
            {
                gprs_send_active_test(0);
                try_count ++;
            }
        }
        else
        {
            if(try_count < 6)
            {
                if(second_elapsed(RequestRemote.send_time_in_10ms) > 60)
                {
                    gprs_send_active_test(0);
                    try_count ++;
                }
            }
            else
            {
                gSystemInfo.tcp_link = 0;
            }
        }
    }
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
        gSystemInfo.heart_cycle = 5;
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
                
    if(    ((check_is_valid_ipaddr(gSystemInfo.ip1)==FALSE)||(check_is_valid_port(gSystemInfo.port1)==FALSE))
        && ((check_is_valid_ipaddr(gSystemInfo.ip2)==FALSE)||(check_is_valid_port(gSystemInfo.port2)==FALSE))  )
    {
        //没有设置主站ip地址和端口的时候默认电平物联网平台的调试接口
        gSystemInfo.ip1[0] = 180;
        gSystemInfo.ip1[1] = 101;
        gSystemInfo.ip1[2] = 147;
        gSystemInfo.ip1[3] = 115;
        gSystemInfo.port1[0] = 5683&0xFF;
        gSystemInfo.port1[1] = 5683>>8;
        mem_cpy(gSystemInfo.ip2,gSystemInfo.ip1,4);
        mem_cpy(gSystemInfo.port2,gSystemInfo.port2,2);
    }
    
    if((buf[12] == 0)||(buf[12] == 0xFF))
    {
        gSystemInfo.apn[0] = 0;
    }
    else
    {
        mem_cpy(gSystemInfo.apn,buf+12,16);
    }
    fread_ertu_params(EEADDR_SET_F16,buf,64);
    if((buf[0] == 0)||(buf[0] == 0xFF))
    {
        gSystemInfo.user[0] = 0;
    }
    else
    {
        mem_cpy(gSystemInfo.user,buf,32);
    }
    if((buf[32] == 0)||(buf[32] == 0xFF))
    {
        gSystemInfo.pwd[0] = 0;
    }
    else
    {
        mem_cpy(gSystemInfo.pwd,buf+32,32);
    }
    fread_ertu_params(EEADDR_CFG_DEVID,buf,4);
    mem_cpy(gSystemInfo.ertu_devid,buf,4);
}
void task_1376_3_service(void)
{
    uint8_t has_work;
    
    gSystemInfo.is_nb_net = 1;
    gSystemInfo.is_coap_net = 1;
    
    meter_uart_init(115200);
    gprs_uart_init(115200);
    drv_pin_gprs_rst(1);
    drv_pin_gprs_onoff(1);
    DelayNmSec(3000);//等待上电稳定
    
    update_gprs_param();
    
    gprs_start_work();
    gprs_send_active_test(1);
    while(1)
    {
        ClrTaskWdt();
        has_work = 0;
        gprs_check_active_test();
        gprs_check_net_status();
        if(gSystemInfo.tcp_link == 0)
        {
            GprsObj.send_len = 0;
            gprs_power_off();
            gprs_start_work();
            gprs_send_active_test(DT_F1);
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
