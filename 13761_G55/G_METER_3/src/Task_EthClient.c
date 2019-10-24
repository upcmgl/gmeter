#include "main_include.h"
#include "Stack_Interface.h"
#ifdef STACK_USE_GPRS_PPP
/*++++++++++++++++++++++++++++++++++++++++++++++++++

  功能： 协议栈使用的以太网远程任务
      1.该任务包括了 PRS以太网的TCP/UDP CLIENT
 +++++++++++++++++++++++++++++++++++++++++++++++++++*/

void ETH_Clent_Idle(void)
{
    #ifdef __PATCH_REC_DH_REPORT__
    report_patch_dayhold();
    #endif
             #ifdef __PRECISE_TIME__
             check_precice_time(3);
             #endif
}
void ETH_Logon_Process(void)
{	
	INT16U frameLen = 0;
   if(is_available_eth())
	{
	
		frameLen = TCP_recv_cmd_frame_eth(cmd_buffer_eth,10000,MAX_SIZE_PLMSDTP);
   }
   #if defined STACK_USE_GENERIC_UDP_CLIENT_EXAMPLE
	else if(is_available_udp_eth())
	{
		frameLen = recv_cmd_frame(read_udp_eth,2000,cmd_buffer_eth, MAX_SIZE_PLMSDTP,2);
   }
   #endif
	//如果收到数据，则处理数据
	if(frameLen)
	{
		if(g_login_flag==2)//登录时只要收到报文，就认为登录成功
		{
			g_login_flag=1;
		}
		if(REMOTE_CLIENT.ETH_STATUS == ETH_CHECK_LOGIN)
		{
		
            g_eth_conn_time=0;
            g_eth_disconn_time=0;
			   REMOTE_CLIENT.ETH_STATUS = ETH_LOGON;
			   #ifndef __USE_DUAL_GPRS_CLIENT__
            g_eth_conn_flag = 1;
            #endif
		}
		//请求远程应答报文处理数据结构使用权
		OSMutexPend(&SIGNAL_RESP_ETH);	  
		resp_eth.channel	 = CHANNEL_ETH;
		resp_eth.cmd_flag = 0x00;
		resp_eth.handle = (INT32U) &My_ETH_TCP_Socket; 
		process_CommandFrame(cmd_buffer_eth,&resp_eth);
		REMOTE_CLIENT.eth_idle_start = get_core_time();
        REMOTE_CLIENT.eth_20sec_idle = get_core_time();
        #ifndef __USE_DUAL_GPRS_CLIENT__
 		g_eth_conn_flag = 1; 
 		#endif
		OSMutexFree(&SIGNAL_RESP_ETH);
	}
	else
	{
		   //连接成功后，如果1分钟未收到主站下发报文，则认为连接失败，则需要重新连接
		  if(REMOTE_CLIENT.ETH_STATUS == ETH_CHECK_LOGIN)
		  {
            if( second_elapsed(REMOTE_CLIENT.eth_idle_start) >= 60 )
            {
                  REMOTE_CLIENT.ETH_STATUS = ETH_SOCKET_CLOSE;
						return ;
            }
         }
         #ifndef __USE_DUAL_GPRS_CLIENT__
		  if(REMOTE_GPRS.req_eth)						 //申请借道以太网通道发送
		  {
			  mem_cpy(resp_eth.buffer,resp_remote.buffer,eth_req_sendlen);
			  TCP_send_buffer_eth(resp_eth.buffer,eth_req_sendlen);
			  REMOTE_GPRS.req_eth = 0;
		  }
		  else
		  #endif
		  {

				//处理心跳
				if(REMOTE_CLIENT.heart_cycle == 0 || REMOTE_CLIENT.heart_cycle > 30) REMOTE_CLIENT.heart_cycle = 5;
				if(minute_elapsed(REMOTE_CLIENT.eth_idle_start) >= REMOTE_CLIENT.heart_cycle)
				{
                   #ifndef __USE_DUAL_GPRS_CLIENT__
				   if(1 == g_eth_conn_flag)
				   #endif
				   {
					  //g_eth_conn_flag |= 0x80;
					  fread_ertu_params(EEADDR_HEARTCYCLE,&(REMOTE_CLIENT.heart_cycle),1);	 //读取心跳设置 	 
					  //请求远程应答报文处理数据结构使用权
					  OSMutexPend(&SIGNAL_RESP_ETH);	  
					  resp_eth.channel 	= CHANNEL_ETH;
		 			  resp_eth.channel_idx = 1;
					  send_active_test_frame(&resp_eth,DT_F3);
                      DelayNmSec(500);  
					  OSMutexFree(&SIGNAL_RESP_ETH);
				   }
				}
                else if((second_elapsed(REMOTE_CLIENT.eth_idle_start)>=20) && (second_elapsed(REMOTE_CLIENT.eth_20sec_idle)>=20))
                {
                    ETH_Clent_Idle();
                    REMOTE_CLIENT.eth_20sec_idle=get_core_time();
                }
		  	}
	
	}



}

void SubTask_Eth_Client(void)
{
   INT16U port;
   INT8U ip[4];


	switch(REMOTE_CLIENT.ETH_STATUS)
	{
	   case ETH_NONE:
         //暂停以太网任务
         break;
      case ETH_SOCKET_OPEN://连接主站
         #ifdef __USE_DUAL_GPRS_CLIENT__
         if(!GPRS_PPP_is_ok())
         {
             break;
         }    
        #endif
         //以太网连接，时间控制策略
         if(REMOTE_CLIENT.eth_connect_wait >0 )
         {
            if( second_elapsed(REMOTE_CLIENT.eth_connect_fail_time) >= 60 )
            {
                REMOTE_CLIENT.eth_connect_wait--;  
   				REMOTE_CLIENT.eth_connect_fail_time = get_core_time();
            }
            break;
         }
        #ifndef __REMOTE_ETH_PRIORITY__
        //如果是GPRS优先，则GPRS连接时以太网不连接，GPRS断开1分钟后，以太网才连接
        if(REMOTE_GPRS.tcp_ok)
        {
            REMOTE_CLIENT.eth_connect_wait = 1;
            break;
        }
        #endif    
         REMOTE_CLIENT.eth_connect_wait = 0 ;
			if(1==REMOTE_SMS.gprs_mode)//udp
			{
			#if defined STACK_USE_GENERIC_UDP_CLIENT_EXAMPLE
				if(eth_udp_is_closed())
				{
    				#ifdef __USE_DUAL_GPRS_CLIENT__
    				if(gprs_get_server(ip,&port,1))
    				#else
					if(gprs_get_server(ip,&port,REMOTE_CLIENT.eth_server_idx))
					#endif
					{
					eth_udp_set(ip,port,7800);
	 				EX_UDP_Socket_Open(1);//eth udp
    	            } 				
					
	 			}
			#endif
			}
			else		//tcp
			{
				if(eth_tcp_is_closed())
				{
    				#ifdef __USE_DUAL_GPRS_CLIENT__
    				if(gprs_get_server(ip,&port,1))
    				#else
					if(gprs_get_server(ip,&port,REMOTE_CLIENT.eth_server_idx))
					#endif
					{
					EX_TCPSocketSetIPPort(TCP_PURPOSE_ETH_CLIENT,ip,port);
					    EX_TCP_Socket_Open(TCP_PURPOSE_ETH_CLIENT);
					}
					else
					{ 
    					if(TRUE == eth_get_dns_host_name(port))
    					{
					EX_TCP_Socket_Open(TCP_PURPOSE_ETH_CLIENT);
					    }    
					}
					
	 			}
			}
			break;
		case ETH_SOCKET_CLOSE://仅断开TCP、UDP连接
			if(1==REMOTE_SMS.gprs_mode)//udp
			{
			#if defined STACK_USE_GENERIC_UDP_CLIENT_EXAMPLE
				eth_udp_close();
			#endif
			}
			else
			{
				EX_TCP_Socket_Close(TCP_PURPOSE_ETH_CLIENT);
			}
         REMOTE_CLIENT.ETH_STATUS = ETH_SOCKET_OPEN;//重新连接
 			break;
  		case ETH_SOCKET_CONNECTED://GPRS已连接，混合模式下，每次拨号后，连接需要登录，其它情况每次连接均需要登录
			//发送登录报文,登录后只要收到主站下发的任何报文，则认为登录成功
            OSMutexPend(&SIGNAL_RESP_ETH);
			   resp_eth.channel     = CHANNEL_ETH;
            resp_eth.handle = (INT32U) &My_ETH_TCP_Socket;
            send_active_test_frame(&resp_eth,DT_F1);
            OSMutexFree(&SIGNAL_RESP_ETH);
   			REMOTE_CLIENT.ETH_STATUS = ETH_CHECK_LOGIN;
			   REMOTE_CLIENT.eth_idle_start = get_core_time();
			g_login_flag=2;

			break;
		case ETH_CHECK_LOGIN://检查是否登录
		case ETH_LOGON://终端已经登陆
			ETH_Logon_Process();
			break;
		default:
			break;
	}//end switch(REMOTE_CLIENT.THE_STATUS)


}
void SubTask_Eth_Server(void)
{
    INT16U  frameLen;

    frameLen=0;
    
    if(eth_server_is_closed())
    {
    
        get_server_port();
        EX_TCPServerSetPort(TCP_PURPOSE_ETH_SERVER,eth_server_port);
        eth_server_tcp_open();
    
    }
    if(is_available_eth_server())
    {
        frameLen = TCP_recv_cmd_frame_eth_server(cmd_buffer_eth,10000,MAX_SIZE_PLMSDTP);
    }
    
    if(frameLen >0)
    {
    
    	OSMutexPend(&SIGNAL_RESP_ETH);	  
    	resp_eth.channel	 = CHANNEL_ETH_SERVER;
        resp_eth.channel_idx =2;
        
        process_CommandFrame(cmd_buffer_eth,&resp_eth);
        unlight_LED_MSA(DIR_SEND);
        OSMutexFree(&SIGNAL_RESP_ETH);
    }
}
void SubTask_Other(void)
{

}
/*++++
 功能： 协议栈使用的远程通道以太网客户端任务
 参数：
        无
 返回：
        无
 描述：

----*/
void task_eth_client(void)
{
 
   REMOTE_CLIENT.ETH_STATUS  = ETH_SOCKET_OPEN;
   REMOTE_CLIENT.eth_server_idx = 0;
   REMOTE_CLIENT.eth_connect_wait = 0;
	
   g_eth_conn_time = 0;
   g_eth_disconn_time = 0;
   g_eth_conn_flag = 0;


	
   ClrTaskWdt();
   DelayNmSec(20000);

   g_eth_conn_flag = 0;
   fread_ertu_params(EEADDR_HEARTCYCLE,&(REMOTE_CLIENT.heart_cycle),1);   //读取心跳设置
    #ifdef __ENABLE_CLOCK_FIX__
      check_wait_fix_clock();
    #endif
    #ifndef __REMOTE_ETH_PRIORITY__
    //如果是GPRS优先，则给GPRS一分钟连接时间
        REMOTE_CLIENT.eth_connect_wait = 1;
    #endif

   for(;;)
   {
       ClrTaskWdt();
       SubTask_Eth_Client();

   }

}

//以太网远程任务
//这里包括以太网客户端任务和以太网服务器任务，使用状态机实现
void task_eth_remote(void)
{
   INT32U remote_start;
   ClrTaskWdt();

   REMOTE_CLIENT.ETH_STATUS  = ETH_SOCKET_OPEN;
   REMOTE_CLIENT.eth_server_idx = 0;
   REMOTE_CLIENT.eth_connect_wait = 0;

   g_eth_conn_time = 0;
   g_eth_disconn_time = 0;
   g_eth_conn_flag = 0;

   DelayNmSec(20000);

   g_eth_conn_flag = 0;
   fread_ertu_params(EEADDR_HEARTCYCLE,&(REMOTE_CLIENT.heart_cycle),1);   //读取心跳设置
    #ifdef __ENABLE_CLOCK_FIX__
      check_wait_fix_clock();
    #endif
    #ifndef __REMOTE_ETH_PRIORITY__
    //如果是GPRS优先，则给GPRS一分钟连接时间
        REMOTE_CLIENT.eth_connect_wait = 1;
    #endif

    remote_start = get_core_time();


	for(;;)
	{
		ClrTaskWdt();
		SubTask_Eth_Client();
		SubTask_Eth_Server();
        SubTask_Other();
	}


}

void task_udp_report(void)
{
   INT8U ip[4];
   INT16U port;
   INT32U timer;
   
   INT16U pos;
   INT16S rx_data;
   
    #ifdef __ENABLE_CLOCK_FIX__
      check_wait_fix_clock();
    #endif

    //要扩展参数设置udp上报主站的ip地址和端口，这里测试使用
    //get_udp_report_param(ip,port);
    ip[0] = 219;
    ip[1] = 147;
    ip[2] = 26;
    ip[3] = 62;
    port = 2014;
    for(;;)
    {
        ClrTaskWdt();
		
        if(!REMOTE_GPRS.ppp_ok)
        {
            DelayNmSec(100);
            continue;
        }
		if(gprs_udp_is_closed())
		{
            //要扩展参数设置udp上报主站的ip地址和端口，这里测试使用
            //if(get_udp_report_param(ip,port) == FALSE)
            //{
            //    DelayNmSec(100);
            //    continue;
            //}  
            
            ip[0] = 219;
            ip[1] = 147;
            ip[2] = 26;
            ip[3] = 62;
            port = 2014;  
            
    		gprs_udp_set(ip,port,7801);
			gprs_udp_open();
		}
		else
		{
    	    if(second_elapsed(timer) > 30)
    	    {
        	    timer = get_core_time();
        	    //sprintf(cmd_buffer_eth,"udp report test:%02d-%02d-%02d  %02d:%02d:%02d",datetime[YEAR],datetime[MONTH],datetime[DAY],datetime[HOUR],datetime[MINUTE],datetime[SECOND]);
        	    //gprs_udp_send_buffer(cmd_buffer_eth,35);
        	}
        }      	
	    if(is_available_udp_gprs())
	    {
    	    pos = 0;
    	    while(1)
    	    {
		        rx_data = read_udp_gprs(20);
		        if(rx_data != -1)
		        {
    		        cmd_buffer_eth[pos++] = rx_data;
    		    }
    		    else
    		    {
        		    //接收完成
        		    break;
        		}
        	}
        	if(pos)
        	{
            	
                //OSMutexPend(&SIGNAL_RS232);
                //rs232_send_buffer(cmd_buffer_eth,pos);  
                //OSMutexFree(&SIGNAL_RS232);
            }   	
        }
   }


}
#endif
