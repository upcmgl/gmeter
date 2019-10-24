#include "main_include.h"
#include "stdio.h"
#include "string.h"


uint8_t gprs_check_tcp_status(void);
static uint8_t gprs_cmd_buffer[200];
static uint8_t report_info[30];//report info header
volatile INT8S tcp_server_socket_id =-1, tcp_server_socket_id_old=-1 , server_has_on=0,need_reset = 0, check_socket_now=0;
volatile INT8S tcp_client_socket_id = -1, tcp_client_socket_id2 = -1;//榛樿浣跨敤1
#define MAX_SOCKET_ID 3 //A9600 socket id 1-3
int16_t gprs_send_cmd_wait_OK(uint8_t *cmd,uint8_t *resp,uint16_t max_resp_len,uint16_t timeout_10ms);
INT8U check_dns(void);
volatile INT16S has_data_to_read;
int16_t str2hex(INT8U *str, INT16U len, INT8U *hex);


tagWirelessObj GprsObj, GprsObj2, GprsServerObj;
int16_t remote_read_byte(void)
{
    int16_t recv;
    
    if(GprsObj.read_pos == GprsObj.recv_pos)
    {
        return -1;
    }
    LED_green_toggle_start();//Comm-led on
    //system_debug_info("read remote\r\n");
    recv = GprsObj.recv_buf[GprsObj.read_pos];
    GprsObj.read_pos ++;
    if(GprsObj.read_pos >= sizeof(GprsObj.recv_buf))
    {
        GprsObj.read_pos = 0;
    }
    return recv;
}
int16_t remote_read_byte2(void)
{
    int16_t recv;
    
    if(GprsObj2.read_pos == GprsObj2.recv_pos)
    {
        return -1;
    }
    LED_green_toggle_start();//Comm-led on
    //system_debug_info("read remote\r\n");
    recv = GprsObj2.recv_buf[GprsObj2.read_pos];
    GprsObj2.read_pos ++;
    if(GprsObj2.read_pos >= sizeof(GprsObj2.recv_buf))
    {
        GprsObj2.read_pos = 0;
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
    LED_green_toggle_start();//Comm-led on
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
        if(tcp_client_socket_id2 > 0)
        {
            sprintf(gprs_cmd_buffer, "AT+MIPCLOSE=%d\r\n", tcp_client_socket_id2);
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
    DelayNmSec(10000);

    if(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600)
    {
        drv_pin_gprs_rst(0);
        DelayNmSec(1000); 
        drv_pin_gprs_rst(1);
    }
				#ifdef __debug_diaoxian__
				os_get_datetime(&datetime);
				system_debug_info("poweroff ");
				system_debug_data(&datetime,6);
				system_debug_info("\n");
				#endif
}
void gprs_power_on(void)
{
    drv_pin_gprs_onoff(0);
    DelayNmSec(1100); //1s卤100ms
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

/*
 * 返回值 -1  没有收到命令响应 OK
 * 返回值 -2 收到ERROR
 * 返回值 >=0 收到OK,返回值为收到的报文长度
 */
int16_t gprs_send_cmd_wait_OK(uint8_t *cmd,uint8_t *resp,uint16_t max_resp_len,uint16_t timeout_10ms)
{
    uint32_t timer;
    uint32_t byte_timer;
    uint16_t cmd_len, num, idx, recv_pos;
    int16_t  rx_data;
    uint8_t  cmd_buffer[8];
    int16_t  resp_len, pos=0,pos1;
    int16_t  resault;
    uint8_t  state=0, report_id=0;
    tagWirelessObj *Obj;
    uint16_t report_pos=0;
    uint8_t report_state;//鍖哄垎澶氫釜涓婃姤鍛戒护锛屽垎鍒鐞
    uint8_t find_start=0;
    char info[30];
	cmd_len = str_cmdlen(cmd);
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
                    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600))
                    {
                        if(report_state == 1)
                        {//+MIPACCEPT=1,10.247.168.205,33068
                            report_info[report_pos++] = rx_data; 
                            if(state == 0)
                            {
                                if((rx_data >= '1') && (rx_data<= ('0' + MAX_SOCKET_ID)))//A9600 id 1-3
                                {
                                    if(tcp_server_socket_id > 0)
                                    {
                                        tcp_server_socket_id_old = tcp_server_socket_id;
                                    }
                                    tcp_server_socket_id = rx_data - '0';
                                    if(tcp_server_socket_id == tcp_server_socket_id_old)
                                    {
                                        tcp_server_socket_id_old = -1;
                                    }

                                    gSystemInfo.remote_client_linked = 1;
                                    server_has_on = 1;
                                    RequestRemoteServer.recv_time_in_10ms = system_get_tick10ms();
                                    state = 1;
                                    report_pos = 0;//skip out 
                                    report_state = 0;
                                    GprsServerObj.send_len = 0;
                                } 
                            } 
                        }
                        else if(report_state == 2)
                        {//+MIPDATA: < socket_id>,<len>,<data>
                            if(state != 2)
                                report_info[report_pos++] = rx_data;
                            if(state == 0)
                            {
                                pos = str_find(report_info, report_pos,(INT8U*) ",", 1);
                                if(pos >= 0)
                                {
                                    report_id = str2int((INT8S*)report_info);
                                    pos++;
                                    state = 1;
                                    if(report_id == tcp_client_socket_id)
                                        Obj = &GprsObj;
                                    else if(report_id == tcp_server_socket_id)
                                        Obj = &GprsServerObj;
                                    else
                                    {
                                        report_pos = 0;
                                        report_state = 0;
                                    }

                                }    
                            }
                            else if(state == 1)
                            {
                                pos1 = str_find(report_info+pos, report_pos - pos,(INT8U*) ",", 1);
                                if(pos1 >= 0)
                                {
                                    num=0;
                                    for(idx =0; idx< (pos1); idx++)
                                    {
                                        if((report_info[pos+ idx] >= '0') && (report_info[pos+idx] <= '9'))
                                        {
                                            num = num*10;
                                            num += report_info[pos+ idx]-'0';
                                        }
                                    }
                                    if(num >= sizeof(Obj->recv_buf))//??
                                    {
                                        report_pos = 0;//invalid restart
                                        report_state = 0;
                                    }
                                    state = 2;
                                    recv_pos = Obj->recv_pos;
                                    report_pos = 1;

                                    //sprintf(info, "obj id:%d,len%d\r\n", report_id, num);
                                    //if(gSystemInfo.gprs_debug_flag)
                                    //    system_debug_info(info);
                                }
                            }
                            else if(state == 2)
                            {
                                Obj->recv_buf[recv_pos++] = rx_data;
                                if(recv_pos >= sizeof(Obj->recv_buf))
                                    recv_pos = 0;
                                num--;

                                if(num == 0)
                                {
                                    Obj->recv_pos = recv_pos;//16浣嶈祴鍊煎簲璇ヤ笉鐢ㄥ叧閿尯
                                    report_pos = 0;//skip out
                                    report_state = 0;

                                    //sprintf(info, "obj recv:%d,read%d\r\n", Obj->recv_pos, Obj->read_pos);
                                    //if(gSystemInfo.gprs_debug_flag)
                                    //    system_debug_info(info);
                                }

                            }  
                        }
                    }//if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600))
                    if((gSystemInfo.nouse_1376_3 == 0xAA) && (gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_M5310))
                    {
                        if(report_state == 1)
                        {
                            report_info[report_pos++] = rx_data;
                            if(report_pos > 3)
                            {
                                if(str_find(report_info, report_pos, "\r\n", 2) >= 0)
                                {
                                    if((recv_pos = str_find(report_info, report_pos, ",", 1)) >= 0)
                                    {
                                        //find_first_num(report_info, report_pos)  -- socket id
                                        INT16S read = find_first_num(report_info+recv_pos, report_pos);
                                        if(read > 0)
                                            has_data_to_read = read;
                                    }
                                    report_state = 0;
                                }
                            }
                        }
                        else if(report_state == 2)
                        {
                            if(rx_data >= '0' && rx_data <= '9')
                            {
                                if((rx_data - '0') == tcp_client_socket_id)
                                    gSystemInfo.tcp_link = 0;
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
                    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600))//((resault != -1) || ((resault == -1)&&(resp_len<16)))
                    {
                        find_start=12;
                        mem_cpy(report_info,report_info+1,find_start);//find ""
                        report_info[find_start] = rx_data;
                        pos = str_find(report_info,find_start , (uint8_t *)"+MIPACCEPT=", 11);
                        if(pos != -1)
                        {
                            report_pos = 0;
                            report_state = 1;//+LSIPACCEPT
                            state = 0;
                            mem_set(report_info, sizeof(report_info), 0);
                            timer = system_get_tick10ms();

                            continue;
                        }
                        pos = str_find(report_info, find_start, (uint8_t *)"+MIPDATA:", 9);
                        if(pos != -1)
                        {
                            report_pos = 0;
                            report_state = 2;//+LSIPRTCP
                            state = 0;
                            mem_set(report_info, sizeof(report_info), 0);
                            timer = system_get_tick10ms();

                            continue;
                        }
                        pos = str_find(report_info, find_start, (uint8_t *)"+MIPCLOSE:", 10);
                        if(pos != -1)
                        {
                            check_socket_now = 1;
                            continue;
                        }
                    }//if(A9600)
                    else if((gSystemInfo.nouse_1376_3 == 0xAA) && (gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_M5310))
                    {
                        find_start=10;
                        mem_cpy(report_info,report_info+1,find_start);//find ""
                        report_info[find_start] = rx_data;
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
                        pos = str_find(report_info, find_start, (uint8_t *)"+NSOCLI:", 8);
                        if(pos != -1)
                        {
                            report_pos = 0;
                            report_state = 2;
                            state = 0;
                            mem_set(report_info, sizeof(report_info), 0);
                            timer = system_get_tick10ms();

                            continue;
                        }
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
                    return resp_len;
                }
                if(str_find(resp,resp_len,(uint8_t *)"ERROR\r\n",4)!=-1)
                {
                    return -2;
                }
            }
        }
        
        if((system_get_tick10ms() - timer) > timeout_10ms)
        {
            if(system_get_tick10ms() - byte_timer > 20)
            {
                return -1;
            }
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
        baud = 9600;
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
		
		baud = 0;
        break;
	}

	if(baud == 0) 
		return 0;
	if(baud == 115200)
		return baud;

	while(1)
	{
		sprintf(cmd, "AT+IPR=%d\r\n", baud_set);//pic24 %ld pic32%d
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
uint8_t gprs_check_cimi(void)  //at+cimi 保存imsi
{
    int16_t resault;
    INT16S pos =0,pos_end =0;
	INT8U idx =0,imsi =0;
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CIMI\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        if(str_find(gprs_cmd_buffer, sizeof(gprs_cmd_buffer), "NO SIM", 6) >= 0)//NO SIM
            return 0;
		else
		{
			pos = str_find(gprs_cmd_buffer, sizeof(gprs_cmd_buffer), "\x0d\x0d\x0a", 3);
			pos+=3;			
			pos_end = pos + str_find(gprs_cmd_buffer+pos , sizeof(gprs_cmd_buffer)-pos, "\x0d\x0a", 2);
			for(idx =pos;idx<pos_end;idx++)
			{
				if((gprs_cmd_buffer[idx]>='0')&&(gprs_cmd_buffer[idx]<='9'))
				{
					gSystemInfo.imsi_info[imsi++] =gprs_cmd_buffer[idx];
				}
			}
			
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
uint8_t update_imei_info(INT8U* buffer,INT16U len)  //at+cgsn 保存imei
{
    int16_t pos;
	INT8U findImei=0;
    uint8_t  sim_id[20] ={0},sim_id_pos =0;
    
    if(len>15)
    {
		if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
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
		else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
		{
			while(pos<len)
			{
				if(buffer[pos] >'0' && buffer[pos] <'9')
				{
					findImei =1;
					break;
				}
				else
				{
					pos++;
				}
			}
			while(pos<len)
			{
				if(buffer[pos] >='0' && buffer[pos] <='9')
				{
					*(sim_id + sim_id_pos++) = *(buffer+pos++);
				}
				else
				{
					break;
				}
			}
			if(findImei)
			{
				mem_cpy(gSystemInfo.imei_info,sim_id,15);
				return 1;
			}
			
		}
		else if((gSystemInfo.nouse_1376_3 == 0x00)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
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
uint8_t gprs_close_report(void)
{
	int16_t resault;
	int16_t pos;
	
	resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+MIPMODE=0,1,0\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
	if(resault >= 0)
	{
		
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
uint8_t gprs_get_sysinfo(void)
{
    int16_t resault;

    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT$MYSYSINFO\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);

    if(resault >= 0)
    {
        return 1;
    }
    else if(resault == -2)
    {
        return 1;
    }
    else
    {
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
    gprs_cmd_buffer[pos]= '"';
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

uint8_t gprs_set_auth(INT8U auth)
{
    int16_t resault;

    sprintf((char *)gprs_cmd_buffer, (const char*)"AT$MYNETCON=0,\"AUTH\",%d\r\n", auth);

    resault = gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
        return 1;
    }
    else if(resault == -2)
    {
        return 0;
    }
    else
    {
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
//补充的函数用来获得小区cellID
uint8_t gprs_check_creg_cell(void)
{
	int16_t resault,resault2 =0;
	volatile pos1=0,pos2=0,idx=0;
	resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+CREG=2\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
	if(resault >= 0)
	{
		resault2 = gprs_send_cmd_wait_OK((uint8_t *)"AT+CREG?\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
		if(resault2>0)
		{
			if((pos1 = str_find(gprs_cmd_buffer,resault2,(uint8_t*)"\",\"",3))!=-1)
			{
				pos1 +=3;
				gSystemInfo.cell_id =0;
				if((pos2 = str_find(gprs_cmd_buffer+pos1,resault2-pos1,(uint8_t*)"\",",2))!=-1)
				{
					for(idx=pos1;idx<(pos1+pos2);idx++)
					{
						gSystemInfo.cell_id = (gSystemInfo.cell_id<<4) +((gprs_cmd_buffer[idx]>'A')?(gprs_cmd_buffer[idx]-'A'+0xA):(gprs_cmd_buffer[idx]-'0'));
					}
				}

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
            if(str_find(gprs_cmd_buffer + pos +1, resault-pos-1,(uint8_t*)"1", 1) >= 0 )
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
uint8_t gprs_set_netact2(void)
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
		resault = gprs_send_cmd_wait_OK((uint8_t *)"AT$MYNETACT=1,1\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
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
        resault = gprs_send_cmd_wait_OK((uint8_t *)"AT$MYNETACT=0,1\r\n",gprs_cmd_buffer,sizeof(gprs_cmd_buffer),500);
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
                    return 1;
            }
            return 0;
        }
        //保存ip地址
        pos = str_find(gprs_cmd_buffer,resault,(uint8_t*)"1,",2);
        if(pos > 0)
        {
        	//if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
				//update_ppp_ip_info_longsung(gprs_cmd_buffer,resault);
            //
			//else
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
uint8_t gprs_set_tcp_param(uint8_t id, uint8_t *ip,uint8_t *port)
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
    sprintf((char *)gprs_cmd_buffer,"AT$MYNETSRV=0,%d,0,0,\"%d.%d.%d.%d:%d\"\r\n",id,ip[0],ip[1],ip[2],ip[3],port_tmp);
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
uint8_t gprs_set_tcp_param_domain(uint8_t *domain, uint8_t *port)
{
    int16_t resault;
    uint32_t port_tmp;
    
    if((strlen(domain) == 0) || !check_is_valid_port(port))
    {
        return 0;
    }
    port_tmp = port[1];
    port_tmp <<= 8;
    port_tmp += port[0];
    sprintf((char *)gprs_cmd_buffer,"AT$MYNETSRV=0,0,0,0,\"%s:%d\"\r\n",(char *)domain,port_tmp);
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
uint8_t gprs_set_dns_server(uint8_t *ip1,uint8_t *ip2)
{
    int16_t resault;
    uint32_t port_tmp;
    INT8U f1=0,f2=0;

    if( check_is_valid_ipaddr(ip1)) 
        f1=1;
    if(check_is_valid_ipaddr(ip2) )
        f2=1;
    
    if(!f1 && !f2)  
        return 0;
    
    if(f1 != f2)
    {
        if(!f1){
            mem_cpy(ip1, ip2, 4);
        }
        if(!f2){
            mem_cpy(ip2, ip1, 4);
        }
    }
    
    sprintf((char *)gprs_cmd_buffer,"AT$MYNETDNS=0,\"%d.%d.%d.%d\",\"%d.%d.%d.%d\"\r\n",ip1[0],ip1[1],ip1[2],ip1[3],ip2[0],ip2[1],ip2[2],ip2[3]);
    resault = gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100);
    if(resault >= 0)
    {
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
    sprintf((char *)gprs_cmd_buffer,"AT+NSOCO=%d,%d.%d.%d.%d,%d\r\n", tcp_client_socket_id, ip[0],ip[1],ip[2],ip[3], port_tmp);
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
    sprintf((char *)gprs_cmd_buffer,"AT+MIPOPEN=%d,\"TCP\",\"%d.%d.%d.%d\",%d,5000\r\n", tcp_client_socket_id, ip[0],ip[1],ip[2],ip[3], port_tmp);
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
    sprintf((char *)gprs_cmd_buffer,"AT+QIOPEN=1,0,\"TCP\",\"%d.%d.%d.%d\",%d,0,0\r\n",ip[0],ip[1],ip[2],ip[3],port_tmp);
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
uint8_t gprs_connect_tcp(uint8_t id)
{
    int16_t resault;
    
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        return 1;
    }
    sprintf(gprs_cmd_buffer,"AT$MYNETOPEN=%d\r\n", id);
    resault = gprs_send_cmd_wait_OK(gprs_cmd_buffer,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),100*130); //130s//实测无法连接时，EC20模块2min才回复error，所以这里等待130s
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
#ifndef __OOP_G_METER__
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
#endif
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
    uint8_t onoff_count=0;
    uint8_t retry_flag=0;
    uint8_t try_count;
    uint8_t ip[4];
    uint8_t port[2];
    uint32_t baud;
    uint8_t AUTH_mode=1;	//AUTH：鉴权类型， 0： NONE； 1： PAP； 2： CHAP，4G默认为2
    
    onoff_count=0;
    while(1)
    {
#ifdef __EDP__
		storeUnsuccessData();    //存储未上报成功的数据。
		handle_wait_report_data_read();  //等待上报的队列，在这里等待上报。
#endif		
        ClrTaskWdt();
		gSystemInfo.reconnect = 0;
        gSystemInfo.tcp_link = 0;
        if(onoff_count++ > 10)
        {
            onoff_count=0;
            drv_pin_gprs_rst(0);
            DelayNmSec(2000);
            drv_pin_gprs_rst(1);
            DelayNmSec(3000);
        }
        gprs_power_on();
        if((baud = gprs_at_check_routin()) == 0)
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
 		//gprs_close_report(); 
        if(gprs_update_csq() == 0)
            if(gprs_update_csq() == 0)
                if(gprs_update_csq() == 0)
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
		if(gSystemInfo.remote_model_type != REMOTE_MODEL_TYPE_A9600)
		{
    		if(AUTH_mode == 1)
    			AUTH_mode = 2;
    		else
    			AUTH_mode = 1;
			if(gprs_set_auth(AUTH_mode) == 0)
				if(gprs_set_auth(AUTH_mode) == 0)
					if(gprs_set_auth(AUTH_mode) == 0)
					{
						gprs_power_off();
						continue;
					}  
		}
        try_count = 0;
        retry_flag = 0;
        while(1)
        {          
            if((gprs_check_cgreg() == 0)&&(gprs_check_creg() == 0)&&(gprs_check_cereg() == 0))
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
		if(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL)
		{
 			gprs_check_creg_cell();
	//		gprs_check_creg();
		}	
        if(retry_flag)
        {
            gprs_power_off();
            continue;
        }
        //AT$MYSYSINFO //查询当前网络状态 电信、移动；2/3/4G
        if(gprs_get_sysinfo() == 0)
        	if(gprs_get_sysinfo() == 0)
        		if(gprs_get_sysinfo() == 0)
                {
                    gprs_power_off();
                    continue;
                }
        //NETACT..
        try_count=0;
        retry_flag = 0;
		DelayNmSec(5000);
        while(1)
        {
        	if(gprs_set_netact() == 0)
        	{
				if(++try_count >= 3)
				{
					retry_flag=1;
					break;
				}
				DelayNmSec(2000);
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
        while(1)
        {
            ClrTaskWdt();
            update_gprs_param();

            if(    (check_is_valid_ipaddr(gSystemInfo.ip1)&&check_is_valid_port(gSystemInfo.port1))
                || (check_is_valid_ipaddr(gSystemInfo.ip2)&&check_is_valid_port(gSystemInfo.port2))  )
            {
                break;
            }
            //if((gSystemInfo.nouse_1376_3==0xAA) && (gSystemInfo.remote_model_vendor==REMOTE_MODEL_VENDOR_QUECTEL))
			if(gSystemInfo.remote_model_vendor==REMOTE_MODEL_VENDOR_QUECTEL)
            {
                 if(check_dns())
                     break;               
            }

            DelayNmSec(3000);
            if(gprs_check_netact() == 0)
            {
                retry_flag = 1;
                break;
            }
            if((gprs_check_creg() == 0)&&(gprs_check_cgreg() == 0)&&(gprs_check_cereg() == 0))
            {
                retry_flag = 2;
                break;
            }
            if(gprs_update_csq() == 0)
            {
                retry_flag = 3;
                break;
            }
        }
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
            //gprs_send_cmd_wait_OK((uint8_t *)"AT+NATSPEED=57600\r\n", NULL,sizeof(gprs_cmd_buffer),100);
        }
        break;
		

		
    }
}

void gprs_get_dns_domin(INT8U *buf, INT8U *domain_len)
{
#ifdef __13761__
    fread_ertu_params(EEADDR_SET_F301+8,domain_len,1);
    if(buf)
        fread_ertu_params(EEADDR_SET_F301+9,buf,*domain_len);
#endif
}
void gprs_get_dns_server(INT8U *ip1, INT8U *ip2)
{
#ifdef __13761__
     fread_ertu_params(EEADDR_SET_F301,ip1,4);
     fread_ertu_params(EEADDR_SET_F301+4,ip2,4);
#endif
}
INT8U check_dns(void)
{
    INT8U dns1[4],dns2[4];
    INT8U domain_len;
    gprs_get_dns_server(dns1, dns2);
    if(!check_is_valid_ipaddr(dns1) && !check_is_valid_ipaddr(dns2))
        return 0;
    gprs_get_dns_domin(NULL, &domain_len);
    if(domain_len == 0)
        return 0;
    
    return 1;
}
void gprs_start_tcp1(void)
{
    uint8_t ip[4];
    uint8_t port[2];
    INT8U dns1[4],dns2[4];
    INT8U domain[250];
    INT8U domain_len;
    GprsObj.send_len = 0;
#ifdef __EDP__
	INT8U ip_port[6];
#endif	
    //EC20 DNS
    //if((gSystemInfo.nouse_1376_3 == 0xAA) && (gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL) && check_dns())
	if((gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL) && check_dns())
    {
        gprs_get_dns_server(dns1, dns2);
        gprs_get_dns_domin(domain, &domain_len);
        mem_cpy(port,gSystemInfo.port1,2);
        if(gprs_set_tcp_param_domain(domain, port))
        {
            if(gprs_set_dns_server(dns1, dns2))
            {
                if(gprs_connect_tcp(0))
                {
                    gSystemInfo.login_status = 0;
                    gSystemInfo.tcp_link = 1;

                    return ;
                }
            }
        }
    }
    
    if(gSystemInfo.tcp_valid)
    {
        mem_cpy(ip,gSystemInfo.ip1,4);
        mem_cpy(port,gSystemInfo.port1,2);
//#ifdef __EDP__
             //////添加的mgl
             //if(gSystemInfo.update==0xAA)
             //{/*
	             //ip[0] = 49;ip[1] = 4; ip[2] = 70; ip[3] = 70;
	             //port[0] = 0xb8;port[1] = 0x1A;//6840  0x1ab8
	             ////port[0] = 0xda;port[1] = 0x13;//5082  0x13da
				 //*/
				////fread_ertu_params(EEADDR_IP_PORT_UPDATE, ip_port,6);
				////mem_cpy(ip,ip_port,4);
				////mem_cpy(port,ip_port+4,2);
			 	 //ip[0] = 219;ip[1] = 147; ip[2] = 26; ip[3] = 62;
	             //port[0] = 0xD9;port[1] = 0x13;//6840  0x1ab8
             //}
             //else
             //{
				 ////fread_ertu_params(EEADDR_IP_PORT_ONENET, ip_port,6);
				 ////mem_cpy(ip,ip_port,4);
				 ////mem_cpy(port,ip_port+4,2);
	             //ip[0] = 183;ip[1] = 230; ip[2] = 40; ip[3] = 39;
	             //port[0] = 0x6c;port[1] = 0x3;
             //}
//#endif					
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
            if(gprs_open_tcp_CMI() > 0)
            {
                if(gprs_connect_tcp_CMI(ip,port))
                {
                    gSystemInfo.login_status = 0;
                    gSystemInfo.tcp_link = 1;
                    return ;
                }
            }
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
            if(gprs_set_tcp_param(0, ip,port))
            {
                if(gprs_connect_tcp(0))
                {
                    gSystemInfo.login_status = 0;
                    gSystemInfo.tcp_link = 1;
#ifdef __HJ_212__
				gSystemInfo.login_status =1;
#endif
                    return ;
                }
            }
        }
    }
}
void gprs_start_tcp2(void)
{
    uint8_t ip[4];
    uint8_t port[2];
    INT8U dns1[4],dns2[4];
    INT8U domain[250];
    INT8U domain_len;
    GprsObj.send_len = 0;

    if(gSystemInfo.tcp_valid2)
    {
        mem_cpy(ip,gSystemInfo.ip2,4);
        mem_cpy(port,gSystemInfo.port2,2);
        if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
        {
            if(gprs_connect_tcp_quec(ip,port))
            {
                gSystemInfo.login_status2 = 0;
                gSystemInfo.tcp_link2 = 1;
                return ;
            }
        }
        else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_CMI))
        {
            if(gprs_open_tcp_CMI() > 0)
            {
                if(gprs_connect_tcp_CMI(ip,port))
                {
                    gSystemInfo.login_status2 = 0;
                    gSystemInfo.tcp_link2 = 1;
                    return ;
                }                    
            }
        }
        else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
        {
            tcp_client_socket_id2 = 1;
            while((tcp_client_socket_id2 == tcp_server_socket_id) || (tcp_client_socket_id2 == tcp_server_socket_id_old ))
            {
                tcp_client_socket_id2++;
                if(tcp_client_socket_id2 > MAX_SOCKET_ID)
                {
                    gSystemInfo.ppp_is_ok = 0;
                    return ;
                }
            }
            if(gprs_connect_tcp_longsung(ip,port))
            {
                gSystemInfo.login_status2 = 0;
                gSystemInfo.tcp_link2 = 1;
                return ;
            }
        }
        else
        {
            if(gprs_set_tcp_param(1,ip,port))
            {
                if(gprs_connect_tcp(1))
                {
                    gSystemInfo.login_status2 = 0;
                    gSystemInfo.tcp_link2 = 1;
                    return ;
                }
            }
        }
    }

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
    volatile uint16_t data_len;
    uint8_t cmd_buf[11];
    volatile uint8_t data_valid;
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
            if(gSystemInfo.gprs_debug_flag && (data_valid==3))
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
                    if(data_len >SIZE_OF_APP_PROTOCOL_FRAME)
                    {
                        data_valid = 0xFF;
                    }
					if(data_len == 0)
					{
						data_valid = 4;//return
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
                mem_cpy(cmd_buf,cmd_buf+1,3);
                cmd_buf[3] = rx_data;
                if(compare_string(cmd_buf,(uint8_t*)"OK\r\n",4)==0)
                {
                    GprsObj.recv_pos = recv_pos;
                    if(data_len) 
                    {
                        return 1;
                    }
					if(data_len > 1000)
					{
						data_len = 100;
					}
					data_valid = 0;
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
                if((system_get_tick10ms()-timer) > 20)
                {
                    return 0;
                }
            }
            tpos_TASK_YIELD();
        }
    }
    return 0;
}
uint8_t gprs_read_app_data_1376_3_2(void)
{
    int16_t rx_data;
    uint32_t timer;
    uint16_t data_len;
    uint8_t cmd_buf[11];
    uint8_t data_valid;
    uint16_t recv_pos;
    
    if(RequestRemote2.state == REQUEST_STATE_PROCESSING)
    {
        return 0;
    }
    if(gSystemInfo.tcp_link2 == 0)
    {
        return 0;
    }
    gprs_uart_send_buf((uint8_t*)"AT$MYNETREAD=1,500\r\n",20);
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
                    recv_pos = GprsObj2.recv_pos;
                    if((data_len == 0)||(data_len > 500))
                    {
                        data_valid = 0xFF;
                    }
                }
            }
            else if(data_valid == 3)
            {
                GprsObj2.recv_buf[recv_pos] = rx_data;
                recv_pos ++;
                if(recv_pos >= sizeof(GprsObj2.recv_buf))
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
                    GprsObj2.recv_pos = recv_pos;
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
    gprs_send_cmd_wait_OK(NULL, NULL, 0, 20);
    return 0;
}
uint8_t gprs_read_app_data_longsung_active(void)
{
    uint32_t timer;
    uint32_t byte_timer;
    uint16_t cmd_len, num, idx, recv_pos;
    int16_t  rx_data;
    uint8_t  cmd_buffer[8];
    int16_t  resp_len, pos=0,pos1,pos2 =0;
    int16_t  resault;
    uint8_t  state=0, report_id=0;
    tagWirelessObj *Obj;
    uint16_t report_pos=0;
    uint8_t report_state;//鍖哄垎澶氫釜涓婃姤鍛戒护锛屽垎鍒鐞
    uint8_t find_start=0;
    char info[30];
	gprs_uart_send_buf((uint8_t*)"AT+MIPDR=0\r\n",12);
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
				    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600))
				    {
					    if(report_state == 1)
					    {//+MIPACCEPT=1,10.247.168.205,33068
						    report_info[report_pos++] = rx_data;
						    if(state == 0)
						    {
							    if((rx_data >= '1') && (rx_data<= ('0' + MAX_SOCKET_ID)))//A9600 id 1-3
							    {
								    if(tcp_server_socket_id > 0)
								    {
									    tcp_server_socket_id_old = tcp_server_socket_id;
								    }
								    tcp_server_socket_id = rx_data - '0';
								    if(tcp_server_socket_id == tcp_server_socket_id_old)
								    {
									    tcp_server_socket_id_old = -1;
								    }

								    gSystemInfo.remote_client_linked = 1;
								    server_has_on = 1;
								    RequestRemoteServer.recv_time_in_10ms = system_get_tick10ms();
								    state = 1;
								    report_pos = 0;//skip out
								    report_state = 0;
								    GprsServerObj.send_len = 0;
							    }
						    }
					    }
					    else if(report_state == 2)
					    {//+MIPDATA: < socket_id>,<data_index>,<len>,<data>
						    if(state != 3)
						    report_info[report_pos++] = rx_data;
						    if(state == 0)
						    {
							    pos = str_find(report_info, report_pos,(INT8U*) ",", 1);
							    if(pos >= 0)
							    {
								    report_id = str2int((INT8S*)report_info);
								    pos++;
								    state = 1;
								    if(report_id == tcp_client_socket_id)
								    Obj = &GprsObj;
								    else if(report_id == tcp_server_socket_id)
								    Obj = &GprsServerObj;
								    else
								    {
									    report_pos = 0;
									    report_state = 0;
								    }

							    }
						    }
							else if(state ==1)
							{
								pos2 = str_find(report_info+pos, report_pos-pos,(INT8U*) ",", 1);
								if(pos2>0)
								{
									pos += ++pos2;
									//pos2++;
									state = 2;
								}
								//else
								//{
									//report_pos = 0;
									//report_state = 0;									
								//}
							}
						    else if(state == 2)
						    {
							    pos1 = str_find(report_info+pos, report_pos - pos,(INT8U*) ",", 1);
							    if(pos1 >= 0)
							    {
								    num=0;
								    for(idx =0; idx< (pos1); idx++)
								    {
									    if((report_info[pos+ idx] >= '0') && (report_info[pos+idx] <= '9'))
									    {
										    num = num*10;
										    num += report_info[pos+ idx]-'0';
									    }
								    }
								    if(num >= sizeof(Obj->recv_buf))//??
								    {
									    report_pos = 0;//invalid restart
									    report_state = 0;
								    }
									if(num ==0)
									{
										report_pos = 0;//skip out
										report_state = 0;
									}
								    state = 3;
								    recv_pos = Obj->recv_pos;
								    report_pos = 1;

								    //sprintf(info, "obj id:%d,len%d\r\n", report_id, num);
								    //if(gSystemInfo.gprs_debug_flag)
								    //    system_debug_info(info);
							    }
						    }
						    else if(state == 3)
						    {
							    Obj->recv_buf[recv_pos++] = rx_data;
							    if(recv_pos >= sizeof(Obj->recv_buf))
							    recv_pos = 0;
							    num--;

							    if(num == 0)
							    {
								    Obj->recv_pos = recv_pos;//16浣嶈祴鍊煎簲璇ヤ笉鐢ㄥ叧閿尯
								    report_pos = 0;//skip out
								    report_state = 0;

								    //sprintf(info, "obj recv:%d,read%d\r\n", Obj->recv_pos, Obj->read_pos);
								    //if(gSystemInfo.gprs_debug_flag)
								    //    system_debug_info(info);
							    }

						    }
					    }
				    }//if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600))
				    if((gSystemInfo.nouse_1376_3 == 0xAA) && (gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_M5310))
				    {
					    if(report_state == 1)
					    {
						    report_info[report_pos++] = rx_data;
						    if(report_pos > 3)
						    {
							    if(str_find(report_info, report_pos, "\r\n", 2) >= 0)
							    {
								    if((recv_pos = str_find(report_info, report_pos, ",", 1)) >= 0)
								    {
									    //find_first_num(report_info, report_pos)  -- socket id
									    INT16S read = find_first_num(report_info+recv_pos, report_pos);
									    if(read > 0)
									    has_data_to_read = read;
								    }
								    report_state = 0;
							    }
						    }
					    }
					    else if(report_state == 2)
					    {
						    if(rx_data >= '0' && rx_data <= '9')
						    {
							    if((rx_data - '0') == tcp_client_socket_id)
							    gSystemInfo.tcp_link = 0;
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
				    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600))//((resault != -1) || ((resault == -1)&&(resp_len<16)))
				    {
					    find_start=12;
					    mem_cpy(report_info,report_info+1,find_start);//find ""
					    report_info[find_start] = rx_data;
						pos = str_find(report_info,find_start , (uint8_t *)"+", 1);
						if(pos == -1)
						{
							continue;
						}
					    pos = str_find(report_info,find_start , (uint8_t *)"+MIPACCEPT=", 11);
					    if(pos != -1)
					    {
						    report_pos = 0;
						    report_state = 1;//+LSIPACCEPT
						    state = 0;
						    mem_set(report_info, sizeof(report_info), 0);
						    timer = system_get_tick10ms();

						    continue;
					    }
					    //pos = str_find(report_info, find_start, (uint8_t *)"+MIPDATA:", 9);
					    //if(pos != -1)
					    //{
						    //report_pos = 0;
						    //report_state = 2;//+LSIPRTCP
						    //state = 0;
						    //mem_set(report_info, sizeof(report_info), 0);
						    //timer = system_get_tick10ms();
//
						    //continue;
					    //}
						 pos = str_find(report_info, find_start, (uint8_t *)"+MIPDR:", 7);
					    if(pos != -1)
					    {
						    report_pos = 0;
						    report_state = 2;//+LSIPRTCP
						    state = 0;
						    mem_set(report_info, sizeof(report_info), 0);
						    timer = system_get_tick10ms();

						    continue;
					    }						
					    pos = str_find(report_info, find_start, (uint8_t *)"+MIPCLOSE:", 10);
					    if(pos != -1)
					    {
						    check_socket_now = 1;
						    continue;
					    }
				    }//if(A9600)
				    else if((gSystemInfo.nouse_1376_3 == 0xAA) && (gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_M5310))
				    {
					    find_start=10;
					    mem_cpy(report_info,report_info+1,find_start);//find ""
					    report_info[find_start] = rx_data;
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
					    pos = str_find(report_info, find_start, (uint8_t *)"+NSOCLI:", 8);
					    if(pos != -1)
					    {
						    report_pos = 0;
						    report_state = 2;
						    state = 0;
						    mem_set(report_info, sizeof(report_info), 0);
						    timer = system_get_tick10ms();

						    continue;
					    }
				    }//M5310
			    }//if(report_state == 0)
		    }
		}
	    if((system_get_tick10ms() - timer) > 10)
	    {
		    if(system_get_tick10ms() - byte_timer > 10)
		    {
			    return -1;
		    }
	    }
	    if(rx_data == -1)
	    {
		    tpos_TASK_YIELD();
	    }
    }
    return -1;	
	
}
uint8_t gprs_read_app_data(void)
{
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        return gprs_read_app_data_quec();
    }
    else    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_M5310))
    {
        if(has_data_to_read)
            return gprs_read_app_data_CMI();
        else
            return 0;
    }
    else    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        return gprs_read_app_data_longsung_active();
    }
    else
    {
        gprs_read_app_data_1376_3();
        gprs_read_app_data_1376_3_2();
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
uint8_t gprs_send_app_data_1376_3_2(void)
{
    int16_t  rx_data;
    uint32_t timer;
    uint16_t send_len;
    uint8_t  cmd_buf[12];
    uint8_t data_valid;
    uint16_t data_len;
    
    if(gSystemInfo.tcp_link2 == 0)
    {
        return 0;
    }
    if(GprsObj2.send_len == 0)
    {
        return 0;
    }
    
    send_len = GprsObj2.send_len;
    if(send_len > 1460)
    {
        send_len = 1460;
    }
    sprintf((char*)gprs_cmd_buffer,"AT$MYNETWRITE=1,%d\r",send_len);
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
                        gprs_uart_send_buf(GprsObj2.send_ptr,data_len);
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
                    GprsObj2.send_ptr += data_len;
                    if(GprsObj2.send_len >= data_len)
                    {
                        GprsObj2.send_len -= data_len;
                    }
                    else
                    {
                        GprsObj2.send_len = 0;
                    }
                    if(GprsObj2.send_len)
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
    return (GprsObj2.send_len==0)?0:1;
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
    resault = gprs_send_cmd_wait_OK((uint8_t *)"AT+MIPMODE=0,1,0\r\n", NULL,sizeof(gprs_cmd_buffer),100);
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
//unsigned char hex_to_byte(char high, char low)
//{
      //unsigned char val=0;
      //if(high >='0' && high <='9') val = high-'0';
      //else if(high >='A' && high <='F') val = high-'A'+10;
      //else  val = high -'a' + 10;
      //val *=16;
//
      //if(low >='0' && low <='9') val += low-'0';
      //else if(low >='A' && low <='F') val += low-'A'+10;
      //else  val += low -'a' + 10;
//
      //return val;
//}
//int16_t str2hex(INT8U *str, INT16U len, INT8U *hex)
//{
    //INT16U i;
    //if(len %2)  return -1;
    //for(i=0; i<len; i+=2)
    //{
        //hex[i/2] = hex_to_byte(str[i], str[i+1]);
    //}
//}
//int16_t hex2str(INT8U *hex, INT16U len, INT8U *str)
//{
    //INT8U temp;
    //INT16U i;
    //
    //for(i=0; i<len; i++)
    //{
        //temp = hex[i]>>4;
        //if(temp < 0x0a)
            //str[i*2] = temp + '0';
        //else
            //str[i*2] = temp - 0x0a + 'a';
        //
        //temp = hex[i]&0x0f;
        //if(temp < 0x0a)
            //str[i*2+1] = temp + '0';
        //else
            //str[i*2+1] = temp - 0x0a + 'a';
    //}
    //
//}
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
#ifdef DEBUG
	system_debug_info("******1****");
#endif	
    if(resault <0)
	{
		return 0;
	}
#ifdef DEBUG
	system_debug_info("******2****");
#endif	
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
				if(str_find(cmd_buf,sizeof(cmd_buf), (uint8_t*)"OK\r\n", 4) >=0)  //如果找到了ｏｋ，那么退出
				{		
#ifdef DEBUG
	system_debug_info("******find ok will return****");
#endif						
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
            }
        }
        else
        {
#ifdef DEBUG
	system_debug_info("******3****");
#endif			
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
#ifdef DEBUG
	system_debug_info("******4****");
#endif 				
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
#ifdef DEBUG
	system_debug_info("******5****");
#endif                
                if((system_get_tick10ms()-timer) > 200)
                {
#ifdef DEBUG
	system_debug_info("******6****");
#endif 					
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
        return gprs_send_app_data_CMI(tcp_client_socket_id);
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        return gprs_send_app_data_longsung(tcp_client_socket_id);
    }
    else
    {
        gprs_send_app_data_1376_3();
    }
}
uint8_t gprs_send_app_data2(void)
{
    if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_QUECTEL))
    {
        return gprs_send_app_data_quec();
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_CMI))
    {
        return gprs_send_app_data_CMI(tcp_client_socket_id);
    }
    else if((gSystemInfo.nouse_1376_3 == 0xAA)&&(gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_LONGSHANG))
    {
        return gprs_send_app_data_longsung(tcp_client_socket_id);
    }
    else
    {
        gprs_send_app_data_1376_3_2();
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
#warning "todo"
}
uint8_t gprs_check_tcp_status(void)
{
    int16_t resault;
    int16_t pos, pos1;
    uint8_t id, tcp_link, tcp_link2 , remote_link, find;
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
              
             
            return 1;
        }
        else
        {
            tcp_link = 0;   tcp_link2 = 0;
            if(( pos = str_find(gprs_cmd_buffer,resault,(uint8_t *)"$MYNETOPEN:",11) ) != -1)
            {
                if(( pos = str_find(gprs_cmd_buffer+pos+11,5,(uint8_t *)"0,",2) ) != -1)
                {
                     tcp_link=1;
                }
            }
            if(( pos = str_find(gprs_cmd_buffer+pos,resault-pos,(uint8_t *)"$MYNETOPEN:",11) ) != -1)
            {
                if(( pos = str_find(gprs_cmd_buffer+pos+11,5,(uint8_t *)"1,",2) ) != -1)
                {
                    tcp_link2=1;
                }
            }
            if(!tcp_link)
            {
                gSystemInfo.tcp_link = 0;
                gSystemInfo.login_status = 0;
            }
            if(!tcp_link2)
            {
                gSystemInfo.tcp_link2 = 0;
                gSystemInfo.login_status2 = 0;
            }
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
    
    if(second_elapsed(timer)<10)
    {
        return 0;
    }
    timer = system_get_tick10ms();
    if(gprs_check_netact() == 0)
    {
        gSystemInfo.ppp_is_ok = 0;
        gSystemInfo.tcp_link = 0;
		gSystemInfo.tcp_link2 = 0;
        gSystemInfo.login_status = 0;
		gSystemInfo.login_status2 = 0;
        return 0;
    }
    if(gprs_check_tcp_status() == 0)
    {
        //gSystemInfo.tcp_link = 0;
        //gSystemInfo.login_status = 0;
        return 0;
    }

    if(gprs_check_server_state() == 0)
    {
        gprs_start_tcp_server();
    }
    gprs_update_csq();
    if((gSystemInfo.nouse_1376_3 == 0xAA) && (gSystemInfo.remote_model_vendor == REMOTE_MODEL_VENDOR_CMI))
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
#ifndef __EDP__
    if(gSystemInfo.login_status == 0)
    {
        return ;
    }
#endif
    #endif
    timer = system_get_tick10ms();
    
    while(tpos_mutexRequest(&SIGNAL_REMOTE_UART) == FALSE)
    {
#ifdef DEBUG
		system_debug_info("no useful mutex");
#endif
        if((system_get_tick10ms()-timer) > MAX_REMOTE_DELAY_10MS)
        {
            return;
        }
        tpos_TASK_YIELD();
    }
#ifdef DEBUG
	system_debug_info("can find useful mutex");
#endif	
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
    tpos_enterCriticalSection();
    GprsObj.send_ptr = frame;
    GprsObj.send_len = frame_len;
    tpos_leaveCriticalSection();
    
    timer = system_get_tick10ms();
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
    LED_green_toggle_start();//Comm-led on
    
    tpos_mutexFree(&SIGNAL_REMOTE_UART);
#ifdef __ZHEJIANG_ONENET_EDP__
	ertu_month_bytes_add(frame_len);
#endif

}
void remote_send_app_frame2(uint8_t *frame,uint16_t frame_len)
{
    uint32_t timer;

    //江苏混合模式 延时50秒等待连接TCP和拨号
    #ifdef __PROVINCE_JIANGSU__
    timer = system_get_tick10ms();
    while(gSystemInfo.login_status2 == 0)
    {
        if((system_get_tick10ms() - timer) > 50*100)
        {
            return;
        }
    }
    DelayNmSec(2000);
    #else
    if(gSystemInfo.tcp_link2 == 0)
    {
        return ;
    }
    
    if(gSystemInfo.login_status2 == 0)
    {
        return ;
    }
    #endif
    timer = system_get_tick10ms();
    
    while(tpos_mutexRequest(&SIGNAL_REMOTE_UART) == FALSE)
    {
        if((system_get_tick10ms()-timer) > MAX_REMOTE_DELAY_10MS)
        {
            return;
        }
        tpos_TASK_YIELD();
    }
    while(GprsObj2.send_len)
    {
        if((system_get_tick10ms() - timer) > 300*100)
        {
            //5分钟，放弃吧
            tpos_mutexFree(&SIGNAL_REMOTE_UART);
            return ;
        }
        DelayNmSec(10);
        if(gSystemInfo.tcp_link2 == 0)
        {
            tpos_mutexFree(&SIGNAL_REMOTE_UART);
            return ;
        }
        if(gSystemInfo.login_status2 == 0)
        {
            tpos_mutexFree(&SIGNAL_REMOTE_UART);
            return ;
        }
    }
    tpos_enterCriticalSection();
    GprsObj2.send_ptr = frame;
    GprsObj2.send_len = frame_len;
    tpos_leaveCriticalSection();
    
    timer = system_get_tick10ms();
    while(GprsObj2.send_len)
    {
        if((system_get_tick10ms() - timer) > 300*100)
        {
            //5分钟，放弃吧
            tpos_mutexFree(&SIGNAL_REMOTE_UART);
            return ;
        }
        DelayNmSec(10);
        if(gSystemInfo.tcp_link2 == 0)
        {
            tpos_mutexFree(&SIGNAL_REMOTE_UART);
            return ;
        }
        if(gSystemInfo.login_status2 == 0)
        {
            tpos_mutexFree(&SIGNAL_REMOTE_UART);
            return ;
        }
    }
    LED_green_toggle_start();//Comm-led on
    
    tpos_mutexFree(&SIGNAL_REMOTE_UART);
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
    
    tpos_enterCriticalSection();
    GprsServerObj.send_ptr = frame;
    GprsServerObj.send_len = frame_len;
    tpos_leaveCriticalSection();
    
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
    LED_green_toggle_start();//Comm-led on

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
void gprs_send_active_test2(uint8_t is_first)
{
    uint16_t app_make_active_test_frame(uint8_t *frame,uint8_t is_first);
    uint16_t frame_len;
    
    if(GprsObj2.send_len)
    {
        return ;
    }

    RequestRemote2.recv_success_flag = 0;
    RequestRemote2.send_time_in_10ms = system_get_tick10ms();
    frame_len = app_make_active_test_frame(gprs_cmd_buffer+150,is_first);
    GprsObj2.send_len = frame_len;
    GprsObj2.send_ptr = gprs_cmd_buffer+150;
    gprs_send_app_data2();
}
void gprs_check_active_test(void)
{
    static uint8_t try_count=0;
    char info[30];
    if(gSystemInfo.tcp_link == 0)
    {
        try_count=0;//try 6 times after each connection 
        return;
    }
    if(RequestRemote.recv_success_flag)
    {
        gSystemInfo.login_status = 1;
        try_count = 0;
    }
    if(gSystemInfo.login_status == 0)
    {
        if(try_count < 6)
        {
         #ifdef __HEBEI_HUANBAO__
            if(second_elapsed(RequestRemote.send_time_in_10ms) > 60) //主站不回复登录，但是会每分钟召测，收到主站报文认为已经登录
         #else 
            if(second_elapsed(RequestRemote.send_time_in_10ms) > 20)
         #endif  
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
#ifndef __PROVINCE_JIANGSU__
    else
    {
        if(try_count == 0)
        {
            #ifdef __OOP_G_METER__
            if(second_elapsed(RequestRemote.recv_time_in_10ms) >cosem_bin2_int16u(gSystemInfo.heart_cycle))
            #else
            if(second_elapsed(RequestRemote.recv_time_in_10ms) > gSystemInfo.heart_cycle*60)
            #endif
            {
                gprs_send_active_test(0);
                try_count ++;
            }
        }
        else
        {
            if(try_count < 6)
            {
         #ifdef __HEBEI_HUANBAO__
            if(second_elapsed(RequestRemote.send_time_in_10ms) > 60) //主站不回复登录，但是会每分钟召测，收到主站报文认为已经登录
         #else 
                if(second_elapsed(RequestRemote.send_time_in_10ms) > 20)
         #endif                 
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
#endif
    if(gSystemInfo.tcp_link == 0)
    {
        try_count=0;
    }
}
void gprs_check_active_test2(void)
{
    static uint8_t try_count=0;
    char info[30];
    if(gSystemInfo.tcp_link2 == 0)
    {
        try_count=0;//try 6 times after each connection
        return;
    }
    if(RequestRemote2.recv_success_flag)
    {
        gSystemInfo.login_status2 = 1;
        try_count = 0;
    }
    if(gSystemInfo.login_status2 == 0)
    {
        if(try_count < 6)
        {
         #ifdef __HEBEI_HUANBAO__
            if(second_elapsed(RequestRemote.send_time_in_10ms) > 60) //主站不回复登录，但是会每分钟召测，收到主站报文认为已经登录
         #else 
            if(second_elapsed(RequestRemote.send_time_in_10ms) > 20)
         #endif 
            {
                gprs_send_active_test2(1);
                try_count ++;
            }
        }
        else
        {
            gSystemInfo.tcp_link2 = 0;
        }
    }
    #ifndef __PROVINCE_JIANGSU__
    else
    {
        if(try_count == 0)
        {
            #ifdef __OOP_G_METER__
            if(second_elapsed(RequestRemote2.recv_time_in_10ms) >cosem_bin2_int16u(gSystemInfo.heart_cycle))
            #else
            if(second_elapsed(RequestRemote2.recv_time_in_10ms) > gSystemInfo.heart_cycle*60)
            #endif
            {
                gprs_send_active_test2(0);
                try_count ++;
            }
        }
        else
        {
            if(try_count < 6)
            {
         #ifdef __HEBEI_HUANBAO__
            if(second_elapsed(RequestRemote.send_time_in_10ms) > 60) //主站不回复登录，但是会每分钟召测，收到主站报文认为已经登录
         #else 
            if(second_elapsed(RequestRemote.send_time_in_10ms) > 20)
         #endif 
                {
                    gprs_send_active_test2(0);
                    try_count ++;
                }
            }
            else
            {
                gSystemInfo.tcp_link2 = 0;
            }
        }
    }
    #endif
    if(gSystemInfo.tcp_link2 == 0)
    {
        try_count=0;
    }
}
void force_gprs_power_off(void)
{
    
}

#ifdef __OOP_G_METER__
void update_gprs_param(void)
{
    INT16U get_oad_value(INT16U obis,INT8U attri_id, INT8U index, INT8U *data,tagParamObject *object);  
    uint8_t len;
    uint8_t buf[100];
    uint32_t offset;
    tagParamObject object;

    get_param_object(OBIS_GPRS1,&object);
    //心跳
    mem_set(buf,100,0x00);
    len = get_oad_value(OBIS_GPRS1,2,12,buf,&object);
    mem_cpy(buf,buf+1,len);
    if((len == 4)&&(buf[0]==DT_LONGUNSIGNED))
    {
        mem_cpy(gSystemInfo.heart_cycle,buf+1,2);
    }
    else
    {
        gSystemInfo.heart_cycle[0] = 0x01;
        gSystemInfo.heart_cycle[1] = 0x2C;
    }
    //IP 端口
    mem_set(buf,100,0x00);
    mem_set(gSystemInfo.ip1,4,0xFF);
    mem_set(gSystemInfo.port1,2,0xFF);
    mem_set(gSystemInfo.ip2,4,0xFF);
    mem_set(gSystemInfo.port2,2,0xFF);
    len = get_oad_value(OBIS_GPRS1,3,0,buf,&object);
    mem_cpy(buf,buf+1,len);
    if((len>0)&&(buf[0]==DT_ARRAY))
    {
        if(buf[1]>0)
        {
            if((buf[2]==DT_STRUCTURE)&&(buf[3]==2))
            {
                if((buf[4]==DT_OCTETSTRING)&&(buf[5]==4))
                {
                    mem_cpy(gSystemInfo.ip1,buf+6,4);
                }
                if(buf[10]==DT_LONGUNSIGNED)
                {
                    mem_cpy_reverse(gSystemInfo.port1,buf+11,2);
                }
            }
            if((buf[13]==DT_STRUCTURE)&&(buf[14]==2))
            {
                if((buf[15]==DT_OCTETSTRING)&&(buf[16]==4))
                {
                    mem_cpy(gSystemInfo.ip2,buf+17,4);
                }
                if(buf[21]==DT_LONGUNSIGNED)
                {
                    mem_cpy_reverse(gSystemInfo.port2,buf+22,2);
                }
            }
        }
    }
	//目的是对双主站ip和port的有效性进行判断
	if(check_is_valid_ipaddr(gSystemInfo.ip1)&&check_is_valid_port(gSystemInfo.port1))
	{
		gSystemInfo.tcp_valid=1;
	}
	if(check_is_valid_ipaddr(gSystemInfo.ip2)&&check_is_valid_port(gSystemInfo.port2))
	{
		gSystemInfo.tcp_valid2=1;
	}
	if((compare_string(gSystemInfo.ip1,gSystemInfo.ip2,4)==0)&&(compare_string(gSystemInfo.port1,gSystemInfo.port2,2)==0))
	{
		gSystemInfo.tcp_valid2=0;
	}
	
    //APN
    mem_set(buf,100,0x00);
    len = get_oad_value(OBIS_GPRS1,2,6,buf,&object);
    mem_cpy(buf,buf+1,len);
    if(buf[0] == DT_VISIBLESTRING)
    {
        mem_cpy(gSystemInfo.apn,buf+2,buf[1]);
    }
    else
    {
        mem_cpy(gSystemInfo.apn,(uint8_t*)"CMNET",5);
    }
    //用户名
    mem_set(buf,100,0x00);
    len = get_oad_value(OBIS_GPRS1,2,7,buf,&object);
    mem_cpy(buf,buf+1,len);
    if(buf[0] == DT_VISIBLESTRING)
    {
        mem_cpy(gSystemInfo.user,buf+2,buf[1]);
    }
    else
    {
        mem_cpy(gSystemInfo.user,(uint8_t*)"CARD",4);
    }
    //密码
    mem_set(buf,100,0x00);
    len = get_oad_value(OBIS_GPRS1,2,8,buf,&object);
    mem_cpy(buf,buf+1,len);
    if(buf[0] == DT_VISIBLESTRING)
    {
        mem_cpy(gSystemInfo.pwd,buf+2,buf[1]);
    }
    else
    {
        mem_cpy(gSystemInfo.pwd,(uint8_t*)"CARD",4);
    }
    //终端地址
    gSystemInfo.devid_len = 6;
    offset = get_object_offset(OBIS_COMM_ADDR);
    fread_ertu_params(offset,buf,10);
    gSystemInfo.devid_len = 6;
    if((buf[0]==DT_OCTETSTRING)&&(buf[1]==6))
    {
        mem_cpy(gSystemInfo.ertu_devid,buf+2,6);
    }
    else
    {
        mem_set(buf,6,0x22);
        mem_cpy(gSystemInfo.ertu_devid,buf,6);
    }
	
#ifdef   __OOP_GMETER_ERTU__
		updata_gprs_param_from_ertu();
#endif
}
#else   //#ifdef __OOP_G_METER__
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
    #ifdef __HEBEI_HUANBAO__
	//if(gSystemInfo.switch_param & HUANBAO_JIBEI)
	{
		if(gSystemInfo.heart_cycle != 1)
		{
			buf[5] = 1;
			gSystemInfo.heart_cycle = 1;
			fwrite_ertu_params(EEADDR_SET_F1,buf,6);
		}
	}
    
    #endif
    fread_ertu_params(EEADDR_SET_F3,buf,28);
#ifdef __EDP__
             ////添加的mgl
             if(gSystemInfo.update==0xAA)
             {
	             buf[0] = 49;buf[1] = 4; buf[2] = 70; buf[3] = 70;
	             buf[4] = 0xb8;buf[5] = 0x1A;//6840  0x1ab8
	             //port[0] = 0xda;port[1] = 0x13;//5082  0x13da
				//fread_ertu_params(EEADDR_IP_PORT_UPDATE, ip_port,6);
				//mem_cpy(ip,ip_port,4);
				//mem_cpy(port,ip_port+4,2);
			 	 //buf[0] = 219;buf[1] = 147; buf[2] = 26; buf[3] = 62;
	             //buf[4] = 0xD9;buf[5] = 0x13;//6840  0x1ab8
             }
             else
             {
				 //fread_ertu_params(EEADDR_IP_PORT_ONENET, ip_port,6);
				 //mem_cpy(ip,ip_port,4);
				 //mem_cpy(port,ip_port+4,2);
	             buf[0] = 183;buf[1] = 230; buf[2] = 40; buf[3] = 39;
	             buf[4] = 0x6c;buf[5] = 0x3;
             }
#endif		
    mem_cpy(gSystemInfo.ip1,buf,4);
    mem_cpy(gSystemInfo.port1,buf+4,2);
    mem_cpy(gSystemInfo.ip2,buf+6,4);
    mem_cpy(gSystemInfo.port2,buf+10,2);
		
	if(check_is_valid_ipaddr(gSystemInfo.ip1)&&check_is_valid_port(gSystemInfo.port1))
	{
		gSystemInfo.tcp_valid=1;	
	}
	if(check_is_valid_ipaddr(gSystemInfo.ip2)&&check_is_valid_port(gSystemInfo.port2))
	{
		gSystemInfo.tcp_valid2=1;	
	}	
	if((compare_string(gSystemInfo.ip1,gSystemInfo.ip2,4)==0)&&(compare_string(gSystemInfo.port1,gSystemInfo.port2,2)==0))
	{
		gSystemInfo.tcp_valid2=0;		
	}
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
    fread_ertu_params(EEADDR_CFG_DEVID,buf,CFG_DEVID_LEN);
    mem_cpy(gSystemInfo.ertu_devid,buf,CFG_DEVID_LEN);
}
#endif  //#ifdef __OOP_G_METER__ #else
#ifdef __EDP__
void  report_erc1_update_info(void)
{
	INT8U cs=0,i,tmp;
	INT8U tmp_updata_version[4];
	INT8U frame[48]={0x68,0xA2,0x00,0xA2,0x00,0x68,0xE4,0x03,0x81,0x01,
		0x80,0x00,0x0E,0xF1,0x00,0x00,0x01,0x00,0x03,0x00,
		0x02,0x03,0x01,0x0E,0x00,0x07,0x10,0x03,0x19,0x02,
		0x31,0x2E,0x30,0x31,0x31,0x2E,0x30,0x32,
	0x03,0x00,0x00,0x40,0x00,0x07,0x10,0x05,0x15,0x16};
	fread_ertu_params(EEADDR_UPDATA_INFO,&edp_updata_INFO,sizeof(struct edp_updata_info));
	frame[7]=gSystemInfo.ertu_devid[0];
	frame[8]=gSystemInfo.ertu_devid[1];
	frame[9]=gSystemInfo.ertu_devid[2];
	frame[10]=gSystemInfo.ertu_devid[3];
	if(FILEFLAG_TYPE ==FILEFLAG_SGP_GMTER_G55)
	{	
		frame[30]=0x31;//单相1 三相3
	}
	if(FILEFLAG_TYPE ==FILEFLAG_TGP_GMTER_G55)
	{	
		frame[30]=0x33;//单相1 三相3
	}	
	frame[31]=0x32;
	frame[32]=0x30;
	frame[33]=0x30;
	//    frame[33]=0x31;
	
	frame[34]=0xFF;
	frame[35]=edp_updata_INFO.updata_to_hex[0];
	frame[36]=edp_updata_INFO.updata_to_hex[1];
	frame[37]=edp_updata_INFO.updata_to_hex[2];

	
	//计算校验位
	cs=0;
	for(i=6;i<46;i++)  cs += frame[i];
	frame[46]=cs;
	GprsObj.send_len = 48;
	GprsObj.send_ptr = frame;
	gprs_send_app_data();
}
//******************************在为注册网络的时候需要将没上报成功的数据存储到flash中*******************************//
void storeUnsuccessData()
{
	switch(autoReportType)   //对从app_protocol_onenet中来的命令数据进行解析
	{
#ifdef DEBUG
		if(autoReportType)
		{
			system_debug_info("\r\n************in outline state need to store data ******************\r\n");
		}
#endif
		case 7:      //当前电量
			cmdType =0;
			wait_report_flag =1;
			while(!reportHoldDayEnergyToOnenet(0));
			wait_report_flag =0;
			autoReportType = 0;
			break;
		case 11:
			cmdType =0;
			wait_report_flag=1;
			while(!reportHoldDayEnergyToOnenet(1)); //日冻结
			wait_report_flag=0;
			autoReportType = 0;
			break;
		case 12:    //645透抄结果返回。
			cmdType =0;
			//			report645CmdResultToOnenet();
			break;
		case 14:    //电能量数据块
			cmdType =0;
			wait_report_flag =1;
			while(!reportyEnergyBlockToOnenet());
			wait_report_flag =0;
			autoReportType = 0;
			break;
		default:
			break;
	}
	switch(warning_report_cmd)
	{
		case 1:
			while(!report_warning_to_onenet(1,0));
			warning_report_cmd =0;
			break;
		case 2:
			while(!report_warning_to_onenet(2,0));
			warning_report_cmd =0;
			break;
		case 3:
			while(!report_warning_to_onenet(3,0));
			warning_report_cmd =0;
			break;
		default:
			break;
	}

}
#endif
uint32_t tmr;

void task_1376_3_service(void)
{
    uint8_t has_work;
    INT8U dns1[4],dns2[4];
    INT8U domain[200];
    INT8U domain_len;
#ifdef __EDP__	
    INT16S len=0;
	INT8U str1[100],str2[100];
	INT8U idx=0,idx1=0,idx2=0;
	INT8U flag = 0;
	uint8_t cnt=0,up=0,tmp=0;
	INT32U update_start_Time =0;
//	static INT32U  startTime =0;
#endif    

		
//  gSystemInfo.clock_ready = 1;
    gSystemInfo.meter_debug_flag = 0;
    gSystemInfo.gprs_debug_flag =  0;
    RequestUart.recv_success_flag = 1;  
    gSystemInfo.debug_info_interface = 0;

    
    drv_pin_gprs_onoff(1);
    drv_pin_gprs_rst(0);
    DelayNmSec(1000);
    drv_pin_gprs_rst(1);

#ifdef __OOP_GMETER_ERTU__
	// DelayNmSec(5000);
	while(gSystemInfo.app_is_ready==0)
	{
		DelayNmSec(1000);
	}
#endif
    has_work = 0;
#ifdef __EDP__
    //如果需要升级 跳转升级服务器
    fread_ertu_params(EEADDR_UPDATE_FLAG,&gSystemInfo.update,1);
#endif	
    update_gprs_param();


#ifdef __EDP__
	 //读不到资产编号无法注册网络，这里只能一直等到读到编号
	 for(idx=0;idx<10;idx++)
	 {
		 len = app_read_his_item(0x04000403,str1,str2,255,NULL,2000);//str1中存在着我们要读取的值；读资产编号
		 //len = app_read_his_item(0x04000403,str2,str2+40,255,NULL,2000);//str1中存在着我们要读取的值；读资产编号
		 //if(compare_string(str1,str2,32)!=0)
		 //{
			 //continue;
		 //}
		 if(len)
		 {
			 for(idx1 =0;idx1<22;idx1++)
			 {
				 gSystemInfo.managementNum[idx1] = str1[31-idx1];//test code
			 }
			 break;
		 }
	 }
	 system_debug_data(gSystemInfo.managementNum,22);
	 
	 gSystemInfo.managementNum[22] =0x00;   //资产管理号字符串结束符。
	 for(idx1=0;idx1<22;idx1++)
	 {
		 if( (gSystemInfo.managementNum[idx1] > '9') || (gSystemInfo.managementNum[idx1] < '0') )
		 {
			 break;
		 }
	 }
	 if(idx1 != 22)//出现不合法的值
	 {
		 system_debug_info("managementNum error");
		 mem_set(gSystemInfo.managementNum, 22, '0');
		 for(idx1 =0;idx1<6;idx1++)
		 {
			 gSystemInfo.managementNum[ idx1*2] = (gSystemInfo.meter_no[5-idx1]>>4) + '0';
			 gSystemInfo.managementNum[ idx1*2+1] = (gSystemInfo.meter_no[5-idx1]&0x0f) + '0';
		 }
	 }
	 //一直读不到复位
	 if(idx==10)
	 {
		 app_softReset();
	 }
#endif   
    while(((gSystemInfo.tcp_link == 0) && (gSystemInfo.tcp_link2 == 0)) || gSystemInfo.reconnect)
    {
        gprs_start_work();
        gprs_start_tcp1();
        gprs_start_tcp2();
        DelayNmSec(1000);//
        if(gprs_check_tcp_status() == 0)
        {
            //gSystemInfo.tcp_link = 0;
            //gSystemInfo.login_status = 0;
        }

    }
#ifndef __HJ_212__   //HJ212 doesn't need to send login frame. 
    if(gSystemInfo.tcp_link)
    {
#ifndef __EDP__
       gprs_send_active_test(1);
       RequestRemote.recv_time_in_10ms = system_get_tick10ms();
#else
		if(gSystemInfo.update == 0xAA)  //升级跳转时发的376.1 登录帧
		{
		       gprs_send_active_test(1);
		       RequestRemote.recv_time_in_10ms = system_get_tick10ms();
		}
#endif		


    }
    if(gSystemInfo.tcp_link2)
    {
#ifndef __EDP__		
        gprs_send_active_test2(1);
        RequestRemote2.recv_time_in_10ms = system_get_tick10ms();
#endif		
    }
#endif    
#ifdef __PROVINCE_JIANGSU__    
    gprs_start_tcp_server();
#endif

#ifdef __EDP__ 
	if(gSystemInfo.update!=0xAA)    //不是0xAA 说明不是在升级中。
	{

		//mgl测试用添加。注册设备,因为用到资产管理编号
		edp_ping_start_time =system_get_tick10ms();		
		registerDevice();

#ifdef DEBUG
		system_debug_info("\r\n *********already finish register device******:");
#endif		
		DelayNmSec(1000);
	}

#endif

    while(1)
    {
        ClrTaskWdt();
        has_work = 0;
		
#ifndef __EDP__	|| __HJ_212__	//HJ212 doesn't need to send login frame. 
        gprs_check_active_test();
        gprs_check_active_test2();
#endif		
#ifdef __EDP__
	 if(gSystemInfo.update==0xAA)
	 {
		
		 gprs_check_active_test();
		 
		 if(up==0)
		 {
			 update_start_Time =tpos_time_10ms(); //开始升级时间
			 report_erc1_update_info();
			 up=1;
		 }
		 if(tpos_elapsed_minutes(update_start_Time)>10)
		 {
			 flag = 0xDD;
			 fwrite_ertu_params(EEADDR_UPDATE_FLAG,&flag,1);
			 gSystemInfo.update =flag;
#ifdef DEBUG
			system_debug_info("=====time is arrive we need to restart=======");
#endif			 
			 app_softReset();
		 }
		 if(dev_power_lost())
		 {
			 system_debug_info("\r\ninto power lost");
			 flag = 0xDD;
			 fwrite_ertu_params(EEADDR_UPDATE_FLAG,&flag,1);
			 gSystemInfo.update =flag;
		 }
	 }
#endif
        gprs_check_net_status();
        #ifdef __PROVINCE_JIANGSU__

        //only one client
        //if(tcp_server_socket_id_old > 0)
        //{
            //if(gprs_close_socket(tcp_server_socket_id_old) > 0)
                //tcp_server_socket_id_old = -1;
        //}
        //if(need_reset)
        //{
            //need_reset = 0;
            //gSystemInfo.ppp_is_ok = 0;
        //}
        //
        //check_report();
        //
        //if((gSystemInfo.has_data_report == 0) && (gSystemInfo.ppp_is_ok == 0))
        //{
            ////GprsObj.send_len = 0;
            //gprs_power_off();
            //gprs_start_work();
            //gprs_start_tcp_server();
        //}
//
        //if(second_elapsed(RequestRemoteServer.recv_time_in_10ms) > (10*gSystemInfo.heart_cycle*60))//10鍊嶅績璺冲懆鏈
        //{
            //gprs_power_off();
            //gprs_start_work();
            //gprs_send_active_test(1); 
            //gprs_start_tcp_server();
        //}
        //
        //if(gSystemInfo.has_data_report && (gSystemInfo.tcp_link==0) )//闇?瑕佽繛鎺ヤ富绔
        //{
            //gprs_check_active_test();//clear count
            //if(cnt > 3)
                //gSystemInfo.ppp_is_ok = 0;
            //if(gSystemInfo.ppp_is_ok == 0)
            //{
                //GprsObj.send_len = 0;
                //gprs_power_off();
                //while(gSystemInfo.tcp_link == 0)
                //{
                    //gprs_start_work();
                    //gprs_start_tcp();
                    //DelayNmSec(1000);//
                    //if(gprs_check_tcp_status() == 0)
                    //{
                        //gSystemInfo.tcp_link = 0;
                        //gSystemInfo.login_status = 0;
                    //}
                //}
                //gprs_start_tcp_server();
            //}
            //else
            //{
                //gprs_start_tcp();
                //cnt++;
            //}
            //DelayNmSec(1000);
            //if(gprs_check_tcp_status() == 0)
            //{
                //gSystemInfo.tcp_link = 0;
                //gSystemInfo.login_status = 0;
            //}
//
            //if(gSystemInfo.tcp_link)
            //{
                //cnt = 0;
                //gprs_send_active_test(1);
                //RequestRemote.recv_time_in_10ms = system_get_tick10ms();
            //}
        //}
        //
        //if(check_socket_now)
        //{
            //check_socket_now=0;
            //gprs_check_tcp_status();
        //}
        //
        //if(gSystemInfo.remote_client_linked)
        //{
            //if(second_elapsed(RequestRemoteServer.recv_time_in_10ms) > 120)
            //{
                //gprs_close_socket(tcp_server_socket_id);
                //gprs_check_tcp_status();
            //}
            //else
            //{
                ////宸茬粡寤虹珛tcp server杩炴帴
                //has_work += gprs_server_read_app_data();
                //has_work += gprs_server_send_app_data();                 
            //}
        //}
//
        //if(gSystemInfo.tcp_link )
        //{
            //if(second_elapsed(RequestRemote.recv_time_in_10ms) > 120)
            //{
                //gprs_close_socket(tcp_client_socket_id);
                //DelayNmSec(500);
                //if(gprs_check_tcp_status() == 0)
                //{
                    //gSystemInfo.tcp_link = 0;
                    //gSystemInfo.login_status = 0;
                //}
            //}
            //else
            //{
                ////宸茬粡寤虹珛tcp杩炴帴
                //has_work += gprs_read_app_data();
                //if(gSystemInfo.login_status)
                //{
                    //has_work += gprs_send_app_data();
                //}
            //} 
        //}   
        
        #else   //__PROVINCE_JIANGSU__
        if(gSystemInfo.nouse_1376_3 == 0xAA)
        {
          //  if(gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_M5310 || gSystemInfo.remote_model_type == REMOTE_MODEL_TYPE_A9600)
            //    check_report();
        }
		
		//其中一个有效连接失败达到24小时就重启模组，平时更新cnt和timer
		if(gSystemInfo.login_status && gSystemInfo.login_status2)
		{
			gSystemInfo.relink_cnt = 0;
			gSystemInfo.relink_timer = OS_TIME;
		}
		
        if(gSystemInfo.reconnect 
		|| !gSystemInfo.ppp_is_ok 
		|| (!gSystemInfo.tcp_link  && !gSystemInfo.tcp_link2) 
		|| ((gSystemInfo.relink_cnt > 24 ) && (gSystemInfo.tcp_link != gSystemInfo.tcp_link2)))
        {
			if(gSystemInfo.gprs_debug_flag)
			{
				system_debug_info("reconnect ");
				system_debug_data(&gSystemInfo.reconnect,1);
				system_debug_info("\n");
				system_debug_info("ppp_is_ok ");
				system_debug_data(&gSystemInfo.ppp_is_ok,1);
				system_debug_info("\n");
				system_debug_info("tcp_link ");
				system_debug_data(&gSystemInfo.tcp_link,1);
				system_debug_info("\n");
				system_debug_info("tcp_link2 ");
				system_debug_data(&gSystemInfo.tcp_link2,1);
				system_debug_info("\n");
				system_debug_info("relink_cnt ");
				system_debug_data(&gSystemInfo.relink_cnt,1);
				system_debug_info("\n");
			}
            GprsObj.send_len = 0;
            GprsObj2.send_len = 0;
            gSystemInfo.relink_timer = OS_TIME;
            gSystemInfo.relink_cnt = 0;
            gprs_power_off();
            while((gSystemInfo.tcp_link == 0) && (gSystemInfo.tcp_link2 == 0))
            {
                gprs_start_work();
                gprs_start_tcp1();
                gprs_start_tcp2();
            }
            if(gSystemInfo.tcp_link)
            {
#ifdef __EDP__
				if(gSystemInfo.update==0xAA)	  //如果掉线了edp也需要重新上线。
				{
					gprs_send_active_test(TRUE);
				}
				else
				{
					edp_ping_start_time =system_get_tick10ms(); //在运行过程中掉线了，那么极有可能是edp老在发送报文导致阻塞了，所以提前把这种异常去掉。
					registerDevice();
					//DelayNmSec(1000);
					//while(!reportConfigToOnenet()); //上报设备配置信息。
					gSystemInfo.edp_second_connet = 1;
				}

#ifdef DEBUG
				system_debug_info("\r\n *********already finish register device3******:");
#endif

#else
				gprs_send_active_test(TRUE);
#endif
            }
            if(gSystemInfo.tcp_link2)
            {
                gprs_send_active_test2(TRUE);
            }      
        }
        
        if((gSystemInfo.tcp_link == 0) && gSystemInfo.tcp_valid)
        {
            if(gSystemInfo.relink_cnt == 0 
                || (gSystemInfo.relink_cnt==1 && ms_elapsed(gSystemInfo.relink_timer)> 1*60*1000)
                || (gSystemInfo.relink_cnt==2 && ms_elapsed(gSystemInfo.relink_timer)> 30*60*1000)
                || (gSystemInfo.relink_cnt==3 && ms_elapsed(gSystemInfo.relink_timer)> 60*60*1000))
            {
                gSystemInfo.relink_timer = OS_TIME;
                GprsObj.send_len = 0;
                gprs_start_tcp1();
                if(gSystemInfo.tcp_link)
                {
#ifdef __EDP__		
				if(gSystemInfo.update==0xAA)	  //如果掉线了edp也需要重新上线。
				{
					 gprs_send_active_test(TRUE);
				}		
				else
				{
					registerDevice();
				}

#ifdef DEBUG
				system_debug_info("\r\n *********already finish register device2******:");
#endif
		
#else			
                    gprs_send_active_test(TRUE);
#endif	
                }   
				else
				{
					gSystemInfo.relink_cnt++;
			if(gSystemInfo.gprs_debug_flag)
			{
				system_debug_info("relink_cnt ");
				system_debug_data(&gSystemInfo.relink_cnt,1);
				system_debug_info("\n");
				system_debug_info("relink_timer ");
				system_debug_data(&gSystemInfo.relink_timer,1);
				system_debug_info("\n");
			}		
				}               
            }
        }
        
        if((gSystemInfo.tcp_link2 == 0) && gSystemInfo.tcp_valid2)
        {
            if(gSystemInfo.relink_cnt == 0
            || (gSystemInfo.relink_cnt==1 && ms_elapsed(gSystemInfo.relink_timer)> 1*60*1000)
            || (gSystemInfo.relink_cnt==2 && ms_elapsed(gSystemInfo.relink_timer)> 30*60*1000)
            || (gSystemInfo.relink_cnt==3 && ms_elapsed(gSystemInfo.relink_timer)> 60*60*1000))
            {
                gSystemInfo.relink_timer = OS_TIME;
                GprsObj2.send_len = 0;
                gprs_start_tcp2();
                if(gSystemInfo.tcp_link2)
                {
                    gprs_send_active_test2(TRUE);
                }
				else
				{
					gSystemInfo.relink_cnt++;
			if(gSystemInfo.gprs_debug_flag)
{
			system_debug_info("relink_cnt ");
			system_debug_data(&gSystemInfo.relink_cnt,1);
			system_debug_info("\n");
			system_debug_info("relink_timer ");
			system_debug_data(&gSystemInfo.relink_timer,1);
			system_debug_info("\n");
}					
				} 
            }
        }

        //已经建立tcp连接
        has_work += gprs_read_app_data();	
        if(gSystemInfo.login_status )
        {
            has_work += gprs_send_app_data();
        }
        if(gSystemInfo.login_status2)
        {
            has_work += gprs_send_app_data2();
        }

        #endif  //__PROVINCE_JIANGSU__
        if(has_work == 0)
        {
            DelayNmSec(10);
        }
        else
        {
          //  tpos_TASK_YIELD();
        }
    }
}

