/*
 * router2.0.c
 *
 * Created: 2018-9-11 9:15:11
 * Author : fuyoutao
 */ 


//#include "samg55j19.h"
//#include "core_cm4.h"
//
//#include "asf.h"
//
//#include "tpos_typedef.h"
//#include "tpos.h"
//#include "tpos_cpu.h"
//#include "tpos_service.h"
//
//OS_STK stk_task_service[256];
//OS_STK stk_task_test[256];
//
//OS_STK stk_task_3762_protocol[256];
//OS_STK stk_task_read_meter[256];
//OS_STK stk_task_system_service[256];
//OS_STK stk_task_channel[256];
//
//
//void task_3762_ptotocol(void * para);
//void task_read_meter(void * para);
//void task_system_service(void * para);
//void task_channel(void * para);
//void task_update(void *parameter);
//
//static void task_service(void *parameter);
//void task1_test(void *parameter);
//static void app_task_create(void);
//void app_init(void);
//
//
//int app_main(void)
//{
    ///* Initialize the SAM system */
    //SystemInit();
    //system_init_clock();
    //EI();
    //tpos_10ms_timer_open();
    //tpos_init();
    //
    //app_init();
//
    ////app_task_create();
//
    ////tpos_start();
    ///* Replace with your application code */
    //while (1) 
    //{
        //task_3762_ptotocol(0);
        //task_read_meter(0);
        //task_system_service(0);
        //task_channel(0);
        //task_update(0);
    //}
	//
	//return 0;
//}
//
//
//
//static void task_service(void *parameter)
//{
    ////while(TRUE)
    //{
        //tpos_clrTaskWdt();
//
        ////tpos_yield();
    //}
//}
//
//void task1_test(void *parameter)
//{
    ////while(1)
    //{
        ////tpos_clrTaskWdt();
//
        //// TODO: user code...
        //NOP();
        //NOP();
        //NOP();
        //NOP();
        //
        ////tpos_yield();
    //}
//}
//volatile INT32U  * SCB_HFSR = 0xE000ED2C;
//
//static void app_task_create(void)
//{
    //
    //tpos_createTask(task_service,stk_task_service + (sizeof(stk_task_service)/sizeof(OS_STK)),NULL,(INT8S *)"SYS_SERVICE",TPOS_TASKPROPERTY_SYS);
    ////tpos_createTask(task1_test, stk_task_test + (sizeof(stk_task_test)/sizeof(OS_STK)),NULL,(INT8S *)"TASK_TEST",TPOS_TASKPROPERTY_USER);
    //
    //SCB_HFSR = 0xE000ED2C;
    //
    //tpos_createTask(task_3762_ptotocol, stk_task_3762_protocol + (sizeof(stk_task_3762_protocol)/sizeof(OS_STK)),NULL,(INT8S *)"TASK_3762",TPOS_TASKPROPERTY_USER);
    //tpos_createTask(task_read_meter, stk_task_read_meter + (sizeof(stk_task_read_meter)/sizeof(OS_STK)),NULL,(INT8S *)"TASK_READ_METER",TPOS_TASKPROPERTY_USER);
    ////tpos_createTask(task_system_service, stk_task_system_service + (sizeof(stk_task_system_service)/sizeof(OS_STK)),NULL,(INT8S *)"TASK_SYSTEM_SERVICES",TPOS_TASKPROPERTY_USER);
    ////tpos_createTask(task_channel, stk_task_channel + (sizeof(stk_task_channel)/sizeof(OS_STK)),NULL,(INT8S *)"TASK_CHANNEL",TPOS_TASKPROPERTY_USER);
//}
//
//void  task_system_service_init(void);
//void task_channel_init(void);
//void task_3762_ptotocol_init(void);
//void task_read_meter_init(void);
//void task_update_init(void);
//void app_init(void)
//{
    //task_system_service_init();
    //task_channel_init();
    //task_3762_ptotocol_init();
    //task_read_meter_init();
    //task_update_init();
//}
