/*********************************************************************
 * Configuration Bit settings
 * SYSCLK = 65.536 MHz 
 ********************************************************************/

#include "samg55j19.h"
#include "core_cm4.h"

#include "asf.h"

#include "tpos_typedef.h"
#include "tpos.h"
#include "tpos_cpu.h"
#include "tpos_service.h"

#include "main_include.h"

void bsp_init(void);
void app_init(void);
void task_sys( void );
void task_protocol_service( void );
void task_1376_3_service( void );
void task_meter_service( void );
void task_protocol_handler( void );
void task_usb( void);

#define SIZE_STK_TASK_METER  	1024
#define SIZE_STK_TASK_PROTOCOL  1024
#define SIZE_STK_TASK_GPRS  	1024
#define SIZE_STK_TASK_HANDLER  	1024
#define SIZE_STK_TASK_SYS  		256
#define SIZE_STK_TASK_USB  		1024

OS_STK  stk_task_protocol_service[SIZE_STK_TASK_PROTOCOL];
OS_STK  stk_task_1376_3_service[SIZE_STK_TASK_GPRS];
OS_STK  stk_task_meter_service[SIZE_STK_TASK_METER];
OS_STK  stk_task_protocol_handler[SIZE_STK_TASK_HANDLER ];
OS_STK  stk_task_sys[SIZE_STK_TASK_SYS];
OS_STK  stk_task_usb[SIZE_STK_TASK_USB];

OS_TCB  *TCB_Task_Meter;
OS_TCB  *TCB_Task_Protocol;
OS_TCB  *TCB_Task_Gprs;
OS_TCB  *TCB_Task_Handler;
OS_TCB  *TCB_Task_sys;
OS_TCB  *TCB_Task_usb;


static void app_task_create(void)
{
    TCB_Task_sys        = tpos_createTask(task_sys, stk_task_sys + (sizeof(stk_task_sys)/sizeof(OS_STK)), NULL, (INT8S *)"sys_service", TPOS_TASKPROPERTY_USER);
    TCB_Task_Protocol   = tpos_createTask(task_protocol_service, stk_task_protocol_service + (sizeof(stk_task_protocol_service)/sizeof(OS_STK)), NULL, (INT8S *)"pro_service", TPOS_TASKPROPERTY_USER);
	TCB_Task_Gprs       = tpos_createTask(task_1376_3_service, stk_task_1376_3_service + (sizeof(stk_task_1376_3_service)/sizeof(OS_STK)), NULL, (INT8S *)"pro_3763", TPOS_TASKPROPERTY_USER);
    TCB_Task_Meter      = tpos_createTask(task_meter_service, stk_task_meter_service + (sizeof(stk_task_meter_service)/sizeof(OS_STK)), NULL, (INT8S *)"meter_service", TPOS_TASKPROPERTY_USER);
    TCB_Task_usb        = tpos_createTask(task_usb, stk_task_usb + (sizeof(stk_task_usb)/sizeof(OS_STK)), NULL, (INT8S *)"task_usb", TPOS_TASKPROPERTY_USER);
	TCB_Task_Handler    = tpos_createTask(task_protocol_handler, stk_task_protocol_handler + (sizeof(stk_task_protocol_handler)/sizeof(OS_STK)), NULL, (INT8S *)"pro_handler", TPOS_TASKPROPERTY_USER);
    
    
}
int main(void){


    /* Initialize the SAM system */
    SystemInit();
    //system_init_clock();
	SUPC->SUPC_SMMR = 0x01<<8 | 0x09<<0 | 1<<12;   //´ò¿ªVDDIO monitor µçÑ¹2.69V
    bsp_init();
    
    EI();
	app_init();
    tpos_10ms_timer_open();
    
    drv_flash_init();
    while(nor_flash_detect()==0);
    
    tpos_init();
    app_task_create();

    tpos_start();


    while (1);   
    return 0;
}
/*  BSP Init */
void bsp_io_init(void);
void bsp_uart_init( void );

void bsp_init( void )
{
    bsp_io_init();
    bsp_uart_init();
    
    LED_green_on(0);
    LED_red_on(0);
    LED_local_on(0);

}

/* APP Level Init */
void app_init(void)
{  
	gSystemInfo.lvd = 0xFFFF;
    /* WDT */
    #define WDT_RELOAD_VAL  (10*1000/4)
    wdt_init(WDT, WDT_MR_WDRSTEN|WDT_MR_WDDBGHLT|WDT_MR_WDIDLEHLT, WDT_RELOAD_VAL, WDT_RELOAD_VAL);
    wdt_restart(WDT);
    
    /* UARTs */    
    gprs_uart_init(115200);
	//debug_uart_init(115200);
    debug_uart_init(9600);
    meter_uart_init(9600);
    ir_init(1200);  
    
    
    /* ADC */
    Task_ad_init();
    
    /* BAT CHA */
    drv_bat_cha_init();
	bat_discha(FALSE);
    
    /* USB */
    uhc_start();
}
INT8U ir_buf[3]={1,2,3};   
void Task_bat_cha(void);
void timer10ms_call(void )  //run in TC-interrupt-process
{
    static INT8U CNT_100ms, CNT_1S;
	
	//Task_ad_start(); 
	//if(dev_power_lost())
	//{
		//if((gSystemInfo.dev_temp >-25) && (gSystemInfo.dev_temp < 65))
		//{
			//bat_discha(TRUE);
			//gSystemInfo.batter_power_time = OS_TIME;
		//}
	//}

	//if(bat_get_discha_state())
	//{
		//if(dev_power_lost() && (ms_elapsed(gSystemInfo.batter_power_time) > 30*1000))
		//{
			//bat_discha(FALSE);
			////DelayNmSec(15000);
		//}
	//}
    if(++CNT_100ms > 10)
    {
        CNT_100ms = 0;
        //100ms
         
           
        if(++CNT_1S >= 10)
        {
            CNT_1S = 0;
            //1S
            //LED_local_toggle();   
        }
    }
}
INT32U tt;
INT8U check_usb( void);
void task_sys( void)
{
    while(1)
    {
        ClrTaskWdt();
        //LED 
        LED_Task(); 
        
        bat_charge_service();
		
        if(bat_get_discha_state())
        {
	        if(dev_power_lost() && (ms_elapsed(gSystemInfo.batter_power_time) > 60*1000))
	        {
		        bat_discha(FALSE);
		        DelayNmSec(15000);
#ifdef DEBUG
	system_debug_info("==================need to restart====================");
#endif				
	     		app_softReset();
	        }
        }
			
        tpos_yield();
    }  
}

void ClrWdt(void)
{
    wdt_restart(WDT);
}

