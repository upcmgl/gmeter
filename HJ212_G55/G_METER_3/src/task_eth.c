//==================================================
// 以太网处理任务
//==================================================
#include "main_include.h"

//#include "HardwareProfile_T.h"
#define THIS_IS_STACK_APPLICATION
#define _ETH

#define __ETH_POWER_SAVE__    //启用以太网节能模式

#ifdef __ROUTER_TEST__
#undef __ETH_POWER_SAVE__
#endif

// Include all headers for any enabled TCPIP Stack functions
#include "TCPIP Stack/TCPIP.h"
#include "TCPIP Stack/ENCX24J600.h"
// Declare AppConfig structure and some other supporting stack variables
APP_CONFIG AppConfig,ETH_Config;
BYTE AN0String[8];
BYTE vDeviceResetFlag=FALSE;   //device has to reset flag.
BYTE ETH_CHANGE_FLAG = 0;
#if defined STACK_USE_GENERIC_UDP_CLIENT_EXAMPLE || defined __HEBEI_GMETER__
#if defined(STACK_USE_GPRS_PPP)
WORD  GPRS_UDP_Buf_Begin=0,GPRS_UDP_Buf_End = 0;
BYTE  GPRS_UDP_Data_Buf[UDP_DATA_BUF_SIZE];
UDP_SOCKET	My_GPRS_UDPSocket= INVALID_SOCKET;
#endif
WORD  ETH_UDP_Buf_Begin=0,ETH_UDP_Buf_End = 0;
BYTE  ETH_UDP_Data_Buf[UDP_DATA_BUF_SIZE];
UDP_SOCKET	My_ETH_UDPSocket= INVALID_SOCKET;
#endif

TCP_SOCKET	My_ETH_TCP_Socket = INVALID_SOCKET;
TCP_SOCKET	My_GPRS_TCP_Socket = INVALID_SOCKET;
TCP_SOCKET	My_ETH_SERVER_Socket = INVALID_SOCKET;
TCP_SOCKET	My_GPRS_SERVER_Socket = INVALID_SOCKET;
TCP_SOCKET	My_BACK_SERVER_Socket = INVALID_SOCKET;

TCP_SOCKET	My_FTP_COMMAND_Socket = INVALID_SOCKET;
TCP_SOCKET	My_FTP_DATA_Socket = INVALID_SOCKET;

BYTE ENC424J600_RST_FLAG=0;
BYTE ENC424J600_CHECK_FLAG=0;
/*********************************************************************
 * Function:        void InitAppConfig(void)
 *
 * PreCondition:    MPFSInit() is already called.
 *
 * Input:           None
 *
 * Output:          Write/Read non-volatile config variables.
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
// MAC Address Serialization using a MPLAB PM3 Programmer and 
// Serialized Quick Turn Programming (SQTP). 
// The advantage of using SQTP for programming the MAC Address is it
// allows you to auto-increment the MAC address without recompiling 
// the code for each unit.  To use SQTP, the MAC address must be fixed
// at a specific location in program memory.  Uncomment these two pragmas
// that locate the MAC address at 0x1FFF0.  Syntax below is for MPLAB C 
// Compiler for PIC18 MCUs. Syntax will vary for other compilers.
//#pragma romdata MACROM=0x1FFF0
static ROM BYTE SerializedMACAddress[6] = {MY_DEFAULT_MAC_BYTE1, MY_DEFAULT_MAC_BYTE2, MY_DEFAULT_MAC_BYTE3, MY_DEFAULT_MAC_BYTE4, MY_DEFAULT_MAC_BYTE5, MY_DEFAULT_MAC_BYTE6};
//#pragma romdata
void get_eth_ipinfo(void);

static void InitAppConfig(void)
{
	AppConfig.base_type = 0xff;
	ETH_Config.Flags.bIsDHCPEnabled = TRUE;
	ETH_Config.Flags.bInConfigMode = TRUE;
	memcpypgm2ram((void*)&ETH_Config.MyMACAddr, (ROM void*)SerializedMACAddress, sizeof(AppConfig.MyMACAddr));
//	{
//		_prog_addressT MACAddressAddress;
//		MACAddressAddress.next = 0x157F8;
//		_memcpy_p2d24((char*)&AppConfig.MyMACAddr, MACAddressAddress, sizeof(AppConfig.MyMACAddr));
//	}

    //配置IP地址参数 
    get_eth_ipinfo();
   

	// SNMP Community String configuration
	#if defined(STACK_USE_SNMP_SERVER)
	{
		BYTE i;
		static ROM char * ROM cReadCommunities[] = SNMP_READ_COMMUNITIES;
		static ROM char * ROM cWriteCommunities[] = SNMP_WRITE_COMMUNITIES;
		ROM char * strCommunity;
		
		for(i = 0; i < SNMP_MAX_COMMUNITY_SUPPORT; i++)
		{
			// Get a pointer to the next community string
			strCommunity = cReadCommunities[i];
			if(i >= sizeof(cReadCommunities)/sizeof(cReadCommunities[0]))
				strCommunity = "";

			// Ensure we don't buffer overflow.  If your code gets stuck here, 
			// it means your SNMP_COMMUNITY_MAX_LEN definition in TCPIPConfig.h 
			// is either too small or one of your community string lengths 
			// (SNMP_READ_COMMUNITIES) are too large.  Fix either.
			if(strlenpgm(strCommunity) >= sizeof(ETH_Config.readCommunity[0]))
				while(1);
			
			// Copy string into AppConfig
			strcpypgm2ram((char*)ETH_Config.readCommunity[i], strCommunity);

			// Get a pointer to the next community string
			strCommunity = cWriteCommunities[i];
			if(i >= sizeof(cWriteCommunities)/sizeof(cWriteCommunities[0]))
				strCommunity = "";

			// Ensure we don't buffer overflow.  If your code gets stuck here, 
			// it means your SNMP_COMMUNITY_MAX_LEN definition in TCPIPConfig.h 
			// is either too small or one of your community string lengths 
			// (SNMP_WRITE_COMMUNITIES) are too large.  Fix either.
			if(strlenpgm(strCommunity) >= sizeof(ETH_Config.writeCommunity[0]))
				while(1);

			// Copy string into AppConfig
			strcpypgm2ram((char*)ETH_Config.writeCommunity[i], strCommunity);
		}
	}
	#endif

	memcpypgm2ram(ETH_Config.NetBIOSName, (ROM void*)MY_DEFAULT_HOST_NAME, 16);
	FormatNetBIOSName(ETH_Config.NetBIOSName);

	#if defined(ZG_CS_TRIS)
		// Load the default SSID Name
		if (sizeof(MY_DEFAULT_SSID_NAME) > sizeof(ETH_Config.MySSID))
		{
			ZGErrorHandler((ROM char *)"ETH_Config.MySSID[] too small");
		}
		memcpypgm2ram(ETH_Config.MySSID, (ROM void*)MY_DEFAULT_SSID_NAME, sizeof(MY_DEFAULT_SSID_NAME));
	#endif

	#if defined(EEPROM_CS_TRIS)
	{
		BYTE c;
		
	    // When a record is saved, first byte is written as 0x60 to indicate
	    // that a valid record was saved.  Note that older stack versions
		// used 0x57.  This change has been made to so old EEPROM contents
		// will get overwritten.  The AppConfig() structure has been changed,
		// resulting in parameter misalignment if still using old EEPROM
		// contents.
		XEEReadArray(0x0000, &c, 1);
	    if(c == 0x60u)
		    XEEReadArray(0x0001, (BYTE*)&ETH_Config, sizeof(ETH_Config));
	    else
	        SaveAppConfig();
	}
	#elif defined(SPIFLASH_CS_TRIS)
	{
		BYTE c;
		
		SPIFlashReadArray(0x0000, &c, 1);
		if(c == 0x60u)
			SPIFlashReadArray(0x0001, (BYTE*)&ETH_Config, sizeof(AppConfig));
		else
			SaveAppConfig();
	}
	#endif

    config_eth_ipinfo();
}

#if defined(EEPROM_CS_TRIS) || defined(SPIFLASH_CS_TRIS)
void SaveAppConfig(void)
{
	// Ensure adequate space has been reserved in non-volatile storage to 
	// store the entire AppConfig structure.  If you get stuck in this while(1) 
	// trap, it means you have a design time misconfiguration in TCPIPConfig.h.
	// You must increase MPFS_RESERVE_BLOCK to allocate more space.
	#if defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2)
		if(sizeof(AppConfig) > MPFS_RESERVE_BLOCK)
			while(1);
	#endif

	#if defined(EEPROM_CS_TRIS)
	    XEEBeginWrite(0x0000);
	    XEEWrite(0x60);
	    XEEWriteArray((BYTE*)&AppConfig, sizeof(AppConfig));
    #else
	    SPIFlashBeginWrite(0x0000);
	    SPIFlashWrite(0x60);
	    SPIFlashWriteArray((BYTE*)&AppConfig, sizeof(AppConfig));
    #endif
}
#endif
void enc_j600_power_process(void)
{
   static BYTE state = 0;
   static BYTE minute=0xff;
   static INT32U timer;
   static BYTE eth_connect_flag=0;
   static INT32U timer_1;
   
   if(eth_connect_flag == 0)
   {
      if(g_eth_conn_flag)
      {
         eth_connect_flag = 1;
         timer_1 = get_core_time();
      }
      else return ;
   }
   else
   {
      if(minute_elapsed(timer_1) > 600)
      {
         eth_connect_flag = 0;
      }
   }

   if(g_eth_conn_flag)
   {
      state = 0;
      return ;
   }

   if(ENC424J600_CHECK_FLAG)
   {
      state = 0;
      return ;
   }  

   if(minute == datetime[MINUTE])
      return ;
   minute = datetime[MINUTE];
   
   if(state == 0)
   {
      timer = get_core_time();
      state = 1;
   }
   else if(state == 1)
   {
      if(second_elapsed(timer) > 150)
      {
         timer = get_core_time();
         state ++;
         ENC424J600_RST_FLAG = 1;
         vDeviceResetFlag = TRUE;
      }
   }
   else if(state == 2)
   {
      if(second_elapsed(timer) > 300)
      {
         timer = get_core_time();
         state  ++;
         ENC424J600_RST_FLAG = 1;
         vDeviceResetFlag = TRUE;
      }
   }
   else
   {
      if(minute_elapsed(timer) > 120)
      {
         timer = get_core_time();
         ENC424J600_RST_FLAG = 1;
         vDeviceResetFlag = TRUE;
         state = 0;
      }
   }
}
void task_eth(void)
{
   INT32U mac_link_timer;
	static DWORD t = 0;
   INT8U heart_cycle;
   BOOL MACLinkFlag=1;
   INT8U lastsecond=0; 
   INT8U former_second; 
   static INT8U last_second=0xff;
   static INT8U PHSTAT_LLSTAT;
    ClrTaskWdt();

    #ifdef __ENABLE_CLOCK_FIX__
      check_wait_fix_clock();
    #endif

    //設置通道類別
    resp_eth.channel = CHANNEL_ETH;
    #if defined(__PROVICE_GUIZHOU__)   ||  defined(__QGDW_FK2005__)
    resp_eth.QGDW_TYPE = QGDW_2004;
    #else
    resp_eth.QGDW_TYPE = QGDW_376_1;
    #endif 

    check_ctrl_heat();
  //  switch_on_veth();

    //DelayNmSec(8000);
    #if defined(STACK_USE_GPRS_PPP)
    if(8!=GPRS_MODEL)  
       gprs_power_on();
    #endif
       DelayNmSec(500); //需要确保加电了一会。

    if(TCB_TaskEth!=NULL)
    {
        os_task_protect(TCB_TaskEth->taskNo);
    }

	// Initialize stack-related hardware components that may be 
	// required by the UART configuration routines
    TickInit();
    
    UDPInit();
    
	#if defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2)
	MPFSInit();
	#endif

	// Initialize Stack and application related NV variables into AppConfig.
	InitAppConfig();


	// Initialize core stack layers (MAC, ARP, TCP, UDP) and
	// application modules (HTTP, SNMP, etc.)
    StackInit();

    RUN_TASK_ETH:

    ENC424J600_RST_FLAG=0;
    g_eth_conn_flag = 0;
    mac_link_timer = get_core_time();

	// Now that all items are initialized, begin the co-operative
	// multitasking loop.  This infinite loop will continuously 
	// execute all stack-related tasks, as well as your own
	// application's functions.  Custom functions should be added
	// at the end of this loop.
    // Note that this is a "co-operative mult-tasking" mechanism
    // where every task performs its tasks (whether all in one shot
    // or part of it) and returns so that other tasks can do their
    // job.
    // If a task needs very long time to do its job, it must be broken
    // down into smaller pieces so that other tasks can have CPU time.

    fread_ertu_params(EEADDR_HEARTCYCLE,&heart_cycle,1);   //读取心跳设置
    config_eth_ipinfo();
    ENC424J600_RST_FLAG = 0;
 #ifdef __POWER_CTRL__    
    check_g14_former_status();
 #endif   
    
    event_switch_state(TRUE);
    while(1)
    {
        ClrTaskWdt();
        tpos_power_charge_service();
        #ifdef __POWER_CTRL__ 
        process_pulse_meter();
        #endif
        check_g14_rtc_adj();
        event_switch_state(FALSE);
        if(ETH_CHANGE_FLAG)
        {
           ETH_CHANGE_FLAG = 0;
           get_eth_ipinfo();
           config_eth_ipinfo();
        } 
        if(ENC424J600_CHECK_FLAG)
        {
	        ENC424J600_RST_FLAG = 1;
	        ENC424J600_CHECK_FLAG = 0;
	        PHSTAT_LLSTAT = 0xaa;
	        vDeviceResetFlag = FALSE;
	    } 
        check_j600_state(); //通过写读寄存器,判断j600目前工作是否正常
        enc_j600_power_process();   //里面包含了断电处理，为后备措施
	    if(ENC424J600_RST_FLAG)
	    {
           if(last_second != datetime[SECOND])
           {
              if((PHSTAT_LLSTAT == 0x00)||(PHSTAT_LLSTAT == 0xff))
              {
                   //Led_ShowInfor(0,"ENC_J600_RST!");
		           ENC424J600_RST_FLAG = 0;
		           if(vDeviceResetFlag == TRUE)
		               reset_eth_device();  //以太网芯片断电
           vDeviceResetFlag = FALSE;  
		           InitAppConfig();
		           ETH_Init();
		           PHSTAT_LLSTAT = 0xaa;
		            #ifndef __USE_DUAL_GPRS_CLIENT__
		           g_eth_conn_flag=0;
		           REMOTE_CLIENT.ETH_STATUS = ETH_SOCKET_CLOSE;
		           #endif
           goto RUN_TASK_ETH;
        }
              else
              {
                  PHSTAT_LLSTAT  <<= 1;
                  if(get_link_state())
                     PHSTAT_LLSTAT |= 0x01;        
              }
              last_second = datetime[SECOND];
           }
        }
        else
        {
           PHSTAT_LLSTAT = 0xaa;
        }
        //如果已经掉电，则在这里检测是否已经来电了
        /*
        if(system_flag  & SYS_POWER_LOST)
        {
           //确保调度时钟在运行。
           T1CON = 0xC030;
          #if !defined(STACK_USE_GPRS_PPP)
              DelayNmSec(5000); 
              continue; 
         #else
            MACEDPowerDown();
            MACLinkFlag=0;
            g_eth_conn_flag=0;
            REMOTE_CLIENT.ETH_STATUS = ETH_SOCKET_CLOSE;
           #endif
        }
        */
        StackTask();
      #if defined(__ETH_POWER_SAVE__)
      #warning "开启以太网节能模式...."
      if(! MY_MACIsLinked())
      {
        if((MACLinkFlag==1)&&(second_elapsed(mac_link_timer)>10))
         {
            MACEDPowerDown();
            MACLinkFlag=0;
             #ifndef __USE_DUAL_GPRS_CLIENT__
            g_eth_conn_flag=0;
            REMOTE_CLIENT.ETH_STATUS = ETH_SOCKET_CLOSE;
            #endif
         }
         else
         {
           /*
            #if 0
            if((GPRS_MODEL & 0x1F) != 0x1F)
            #endif
           */
            {
            if((datetime[SECOND] % 30 ==0) && (lastsecond !=datetime[SECOND]))
            {
                MACPowerUp();
                MACLinkFlag=1;
                //REMOTE_CLIENT.eth_connect_wait = 0 ;
                REMOTE_CLIENT.eth_mac_link = 1;
                lastsecond =datetime[SECOND];
            }
         }
         }
         REMOTE_CLIENT.eth_mac_link = 0;
      }
      else
      {
        if((MACLinkFlag==0) &&  !(system_flag  & SYS_POWER_LOST))
         {
            MACPowerUp();
            MACLinkFlag=1;
            REMOTE_CLIENT.eth_connect_wait = 0 ;
         }
         mac_link_timer = get_core_time();
         REMOTE_CLIENT.eth_mac_link = 1;
      }

      #endif   
        // This task performs normal stack task including checking
        // for incoming packet, type of packet and calling
        // appropriate stack entity to process it.
      //如果以太网无法没有连接，则需要设置连接主站的IP及端口
		#if defined(STACK_USE_GENERIC_UDP_CLIENT_EXAMPLE) || defined __HEBEI_GMETER__
		set_Stack_type(2);
		GenericUDPClient();
		set_Stack_type(0);
		#endif
		
        // This tasks invokes each of the core stack application tasks
//        StackApplications();

		// Process application specific tasks here.
		// For this demo app, this will include the Generic TCP 
		// client and servers, and the SNMP, Ping, and SNMP Trap
		// demos.  Following that, we will process any IO from
		// the inputs on the board itself.
		// Any custom modules or processing you need to do should
		// go here.
		#if defined(STACK_USE_GENERIC_TCP_CLIENT_EXAMPLE)
		set_Stack_type(1);
		GenericTCPClient();
		set_Stack_type(0);
		#endif
		
		#if defined(STACK_USE_GENERIC_TCP_SERVER_EXAMPLE)
		set_Stack_type(1);
		GenericTCPServer();
		set_Stack_type(0);
		#endif
		
		#ifdef STACK_USE_FTP_CLIENT
		set_Stack_type(1);
		FTPClient();
		set_Stack_type(0);
		#endif
		
		#if defined(STACK_USE_ICMP_CLIENT)
		PingDemo();
		#endif
		
		#if defined(STACK_USE_SNMP_SERVER) && !defined(SNMP_TRAP_DISABLED)
		SNMPTrapDemo();
		if(gSendTrapFlag)
			SNMPSendTrap();
		#endif
      /*
         //如果以太网连接到了主站，则空闲超过5分钟，发送1个心跳报文。
        if(g_eth_conn_flag)  //以太网客户机连接主站标志
        {
            if(REMOTE_GPRS.req_eth)                  //申请借道以太网通道发送
            {
               mem_cpy(resp_eth.buffer,resp_remote.buffer,eth_req_sendlen);
               eth_send_buffer((TCP_SOCKET *)(resp_eth.handle),resp_eth.buffer,eth_req_sendlen);
               REMOTE_GPRS.req_eth = 0;
            }
            else
            {
              if(heart_cycle == 0 || heart_cycle > 30) heart_cycle = 5;
              if(minute_elapsed(g_eth_last_comm_time) >= heart_cycle)
              {
                 if(1 == g_eth_conn_flag)
                 {
                    g_eth_conn_flag |= 0x80;
                    fread_ertu_params(EEADDR_HEARTCYCLE,&heart_cycle,1);   //读取心跳设置      
                    //请求远程应答报文处理数据结构使用权
                    OSMutexPend(&SIGNAL_RESP_ETH);      
					     resp_eth.channel     = CHANNEL_ETH;
                    send_active_test_frame(&resp_eth,DT_F3);
                    OSMutexFree(&SIGNAL_RESP_ETH);
                 }
              }
            }
        }
      */  
	}

   
}
		

/*+++
  功能：通过Socket发送数据
  参数：
        TCP_SOCKET	*pMySocket,
        INT8U       *buffer,
        INT16U       len  
---*/
/*
void eth_send_buffer(TCP_SOCKET	*pMySocket,INT8U *buffer,INT16U len)
{


    WORD wMaxPut;
    WORD wLen;
    WORD wait_count;

    wait_count = 0;
    display.footer_status.net.for_sec=datetime[SECOND];
    do
    {
        ClrTaskWdt();
       //得到当前可以发送的最大尺寸
       wMaxPut = TCPIsPutReady(*pMySocket);	// Get TCP TX FIFO free space

       if(wMaxPut > 0)
       {
          wLen = (len   > wMaxPut) ? wMaxPut : len;
    
          //发送
          wLen  = TCPPutArray(*pMySocket, buffer,wLen);  

          if(wLen == 0) break; //网络不可用了。
          else
          {
             TCPFlush(*pMySocket);
          }

    
          //计算剩余待发送字节数 
          len -= wLen;
          buffer += wLen;
       }
       else
       {
          if( wait_count++ > 10000) break;
       }

        // This task performs normal stack task including checking
        // for incoming packet, type of packet and calling
        // appropriate stack entity to process it.
        StackTask();
        // This tasks invokes each of the core stack application tasks
        StackApplications();
    }while(len > 0);
 
}
*/

/*+++
  功能：通过Socket接收一个报文
  参数:
        TCP_SOCKET	*pMySocket,
        INT8U       *buffer,
        INT16U      buffer_size
  返回：
        INT16U 接收报文长度   0:没有接收到报文  

  描述：
        1）先接收报文头,即先读取10个字节，格式68 XX XX XX XX 68 判断是否是合法的报文头.
        2) 继续接收剩余的报文，如果接收超时，则返会0
        3) 接收到足够字节后，检查结束符和校验位，正确则返回报文长度，否则返回0
---*/
/*
INT16U  eth_recv_cmd_frame(TCP_SOCKET	*pMySocket,INT8U *buffer,INT16U buffer_size)
{
    WORD  wMaxGet,idx,recv_bytes;
    INT16U frameLen,leftLen,trid_time;
    INT8U  cs;

        //读取报文头,读取1个字符
    do{   
  	   wMaxGet = EX_TCPIsGetReady(*pMySocket);	// 小于6个字节，一定不是一帧完整的报文，或者还没收完整，返回
       if(wMaxGet < 10) return 0;
       wMaxGet = EX_TCPGetArray(*pMySocket,buffer, 1);
       if(wMaxGet<1) return 0;
    }while(buffer[0]!=0x68);

    wMaxGet = EX_TCPGetArray(*pMySocket,&buffer[1], 5);
    if(wMaxGet < 5) return 0;

    if((buffer[1]==buffer[3])&&(buffer[2]==buffer[4])&&(buffer[5]==0x68) )     
    {
    frameLen = buffer[2];
    frameLen <<= 8;
    frameLen += buffer[1]; 
    frameLen >>=2;  
    //报文总长度:数据区长度+报文头+校验+结束符
    frameLen += 8;
    }
    else
    {
       return 0;
    }
    recv_bytes = 6;
    //检查长度结构是否有效
    if( frameLen > buffer_size ) return 0;
    while(1)
    {
   
        //应该继续接收报文长度
        leftLen = frameLen - recv_bytes;

       wMaxGet = TCPGetArray(*pMySocket, buffer+recv_bytes, leftLen);   
       if(wMaxGet > 0)
       {
          recv_bytes += wMaxGet;
          if(recv_bytes == frameLen)
          {
             //检查接收报文是否有效
             if(buffer[recv_bytes-1] != 0x16)
                return 0;  
 
             //校验位检查
             cs = 0;
             for(idx=6;idx<frameLen-2;idx++) cs += buffer[idx];
             if(cs != buffer[recv_bytes-2])
                 return 0;
             return frameLen;
          }
       }
       else if(trid_time++>3000)
       {
           trid_time = 0;
           //接收发生问题。
           return 0;
       }  
       else DelayNmSec(10);  
       
       // This task performs normal stack task including checking
        // for incoming packet, type of packet and calling
        // appropriate stack entity to process it.
        StackTask();
        
        // This tasks invokes each of the core stack application tasks
        StackApplications();
    }    

    //报文接收失败
    return 0;
}
*/
/*+++
   功能： 配置以太网地址
   参数：
          无 
   描述：
        1）如果没有设置参数F7，则赋予默认值192.168.10.1,默认端口8000。默认连接服务器地址192.168.10.21 端口2021
        2）读取参数F7，
---*/
void get_eth_ipinfo(void)
{
    INT8U           *ptr;
    tagERTU_ETH_IP  eth_ip;
    INT8U           proxy_pass_len;

    //首先设置下默认值 
	ETH_Config.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2<<8ul | MY_DEFAULT_IP_ADDR_BYTE3<<16ul | MY_DEFAULT_IP_ADDR_BYTE4<<24ul;
	ETH_Config.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2<<8ul | MY_DEFAULT_MASK_BYTE3<<16ul | MY_DEFAULT_MASK_BYTE4<<24ul;
	ETH_Config.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2<<8ul | MY_DEFAULT_GATE_BYTE3<<16ul | MY_DEFAULT_GATE_BYTE4<<24ul;
	ETH_Config.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2<<8ul  | MY_DEFAULT_PRIMARY_DNS_BYTE3<<16ul  | MY_DEFAULT_PRIMARY_DNS_BYTE4<<24ul;
	ETH_Config.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2<<8ul  | MY_DEFAULT_SECONDARY_DNS_BYTE3<<16ul  | MY_DEFAULT_SECONDARY_DNS_BYTE4<<24ul;
    eth_server_port = ETH_SERVER_PORT;

    #ifdef __ROUTER_TEST__

    AppConfig.Flags.bIsDHCPEnabled = FALSE;  //取消DHCP
    AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
    AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
    return 
    #endif
    //读取参数F7
    fread_ertu_params(EEADDR_SET_F7,eth_ip.value,sizeof(tagERTU_ETH_IP));

    //检查参数F7，如果设置终端IP，则不进行设置。
    if( (eth_ip.local_ip > 0) && (eth_ip.local_ip < 0xFFFFFFFF) )  
    {   
        //用设置值替代缺省值
        ETH_Config.Flags.bIsDHCPEnabled = FALSE;  //取消DHCP

     	ETH_Config.MyIPAddr.Val = eth_ip.ip1 | eth_ip.ip2<<8ul | eth_ip.ip3<<16ul | eth_ip.ip4<<24ul;
    	ETH_Config.MyMask.Val = eth_ip.mask1 | eth_ip.mask2<<8ul | eth_ip.mask3<<16ul | eth_ip.mask4<<24ul;
        ETH_Config.MyGateway.Val = eth_ip.gate1 | eth_ip.gate2<<8ul | eth_ip.gate3<<16ul | eth_ip.gate4<<24ul; 

        ptr = eth_ip.b.proxy_user_pass;
        if(eth_ip.b.proxy_user_name_len < 20)
        {
           ptr += eth_ip.b.proxy_user_name_len;
           proxy_pass_len = *ptr++;
           if(proxy_pass_len < 20)  //用户密码长度
           {
               ptr += proxy_pass_len;     
               //侦听端口
               eth_server_port = ptr[0] + ptr[1]*0x100;
               if((eth_server_port == 0) || (eth_server_port == 0xFFFF)) eth_server_port = ETH_SERVER_PORT;
           }
        }
    }
    else
    {
       
		 ptr = eth_ip.b.proxy_user_pass;
		 if(eth_ip.b.proxy_user_name_len <= 20)
		 {
			 ptr += eth_ip.b.proxy_user_name_len;
			 proxy_pass_len = *ptr++;
			 if(proxy_pass_len <= 20)  //用户密码长度
			 {
				  ptr += proxy_pass_len;	  
				  //侦听端口
				  eth_server_port = ptr[0] + ptr[1]*0x100;
				  if((eth_server_port == 0) || (eth_server_port == 0xFFFF)) eth_server_port = ETH_SERVER_PORT;
			 }
		 }
       DHCPInit(0);
       if(!DHCPIsEnabled(0))
          DHCPEnable(0);
       ETH_Config.Flags.bIsDHCPEnabled = TRUE; //应用DHCP
       
    }
    ETH_Config.DefaultIPAddr.Val = ETH_Config.MyIPAddr.Val;
    ETH_Config.DefaultMask.Val = ETH_Config.MyMask.Val;
    ETH_Config.base_type = TYPE_ETH;
    AppConfig.base_type = 0xff; 
    //mem_cpy((void*)&AppConfig,(void*)&ETH_Config,sizeof(APP_CONFIG));
}
void set_eth_dns_ip(BYTE *ip_pri,BYTE *ip_sec,BOOLEAN force)
{
   if((ETH_Config.Flags.bIsDHCPEnabled != TRUE)||(TRUE == force))
   {
       ETH_Config.PrimaryDNSServer.Val = ip_pri[0]|(ip_pri[1]<<8)|(ip_pri[2]<<16)|(ip_pri[3]<<24);
       ETH_Config.SecondaryDNSServer.Val = ip_sec[0]|(ip_sec[1]<<8)|(ip_sec[2]<<16)|(ip_sec[3]<<24);
       AppConfig.base_type = 0xff;
   }
}
void config_eth_ipinfo(void)
{
    if(AppConfig.base_type==TYPE_ETH)
    {
       mem_cpy((void*)&ETH_Config,(void*)&AppConfig,sizeof(APP_CONFIG));
    }
    mem_cpy((void*)&AppConfig,(void*)&ETH_Config,sizeof(APP_CONFIG));
//   AppConfig.MyIPAddr.Val = ETH_Config.MyIPAddr.Val;
//   AppConfig.MyMask.Val = ETH_Config.MyMask.Val;
//   AppConfig.PrimaryDNSServer.Val = ETH_Config.PrimaryDNSServer.Val; 
}
DWORD get_eth_config_ip(void)
{
   if(ETH_Config.Flags.bIsDHCPEnabled == TRUE)
       return get_dhcp_tmp_ip();
   else if(AppConfig.base_type==TYPE_ETH)
       return AppConfig.MyIPAddr.Val;
   else
   return ETH_Config.MyIPAddr.Val;
}

/*+++
  功能：获取主站IP地址和端口
  参数：
        INT8U *ip   【输出】  获取到的IP地址 
        INT8U *port 【输出】  获取到的端口
  返回：
        无
  描述：
        使用eth_connet_msa_count进行主用备用IP地址切换
---*/
void eth_get_server_ip_port(INT8U *ip,INT16U *port)
{ 
   INT32U ip2;//第二地址需要人工转换，读出来不正确！！！
   INT8U idx;
   fread_ertu_params(EEADDR_SET_F3,ertu_msa_F3.value,sizeof(tagSET_F3));
   for(idx=0;idx<2;idx++)
   {
      eth_connet_msa_count ++;
   
      ip2 = bin2_int32u(ertu_msa_F3.ip2);
      //mem_set(eth_server_name,16,0x00);
      if(eth_connet_msa_count & 0x01)
      {
          //--主用IP
          if((ertu_msa_F3.w.msa_ip1==0) || (ertu_msa_F3.w.msa_ip1==0xFFFFFFFF)) continue;
          
          //snprintf(eth_server_name,16,"%d.%d.%d.%d",ertu_msa_F3.ip1[0],ertu_msa_F3.ip1[1],ertu_msa_F3.ip1[2],ertu_msa_F3.ip1[3]);
          mem_cpy(ip,&(ertu_msa_F3.w.msa_ip1),4);
          byte2int16(ertu_msa_F3.port1,port);
      }
     
      //备用IP
      if((ip2==0) || (ip2==0xFFFFFFFF)) continue;
          
       //snprintf(eth_server_name,16,"%d.%d.%d.%d",ertu_msa_F3.ip2[0],ertu_msa_F3.ip2[1],ertu_msa_F3.ip2[2],ertu_msa_F3.ip2[3]);
          mem_cpy(ip,&(ertu_msa_F3.ip2),4);
   	    byte2int16(ertu_msa_F3.port2,port);
   }

} 
void update_eth_ip_info(void)
{
  if(ETH_CHANGE_FLAG == 0)
    ETH_CHANGE_FLAG = 1; 
} 
