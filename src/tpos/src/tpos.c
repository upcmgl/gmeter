

#include "tpos_typedef.h"
#include "tpos.h"
#include "tpos_service.h"
#include "tpos_cpu.h"

#include "samg55j19.h"

/* Let the user override the pre-loading of the initial RA with the address of
prvTaskExitError() in case is messes up unwinding of the stack in the
debugger - in which case configTASK_RETURN_ADDRESS can be defined as 0 (NULL). */
#ifdef configTASK_RETURN_ADDRESS
	#define portTASK_RETURN_ADDRESS	configTASK_RETURN_ADDRESS
#else
	#define portTASK_RETURN_ADDRESS	prvTaskExitError
#endif


/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError( void );

/*-----------------------------------------------------------*/
OS_TCB   tpos_task_list[TASK_MAX_COUNT]={0};

/* Records the interrupt nesting depth.  This is initialised to one as it is
decremented to 0 when the first task starts. */
volatile OS_STK uxInterruptNesting = 0x01;

/* Stores the task stack pointer when a switch is made to use the system stack. */
OS_STK uxSavedTaskStackPointer = 0;

// Point to the current task control block..
OS_TCB * volatile pxCurrentTCB = NULL;
// Point to the next task control block, used in task switch..
OS_TCB * volatile pxNextTCB = NULL;

objSYSTEM  SYSTEM;

/* The stack used by interrupt service routines that cause a context switch. */
//#define SYSTEM_STACK_SIZE   256
//OS_STK xISRStack[ SYSTEM_STACK_SIZE ] = { 0 };

/* The top of stack value ensures there is enough space to store 6 registers on
the callers stack, as some functions seem to want to do this. */
//const OS_STK * const xISRStackTop = &( xISRStack[ SYSTEM_STACK_SIZE - 7 ] );

/*-----------------------------------------------------------*/
void tpos_init(void)
{
    INT8U i;
    
    SYSTEM.app_running = 0;
    SYSTEM.critical_nest = 0;
    SYSTEM.systick_in_10ms = 0;
    SYSTEM.task_count = 0;
    SYSTEM.tpos_running = 0;
    
    pxCurrentTCB = NULL;
    uxSavedTaskStackPointer = 0;
    
    for(i=0; i<TASK_MAX_COUNT; i++)
    {
        tpos_task_list[i].pTopOfStack = NULL;
        tpos_task_list[i].taskNo = 0;
        tpos_task_list[i].taskPriority = 0;
        tpos_task_list[i].taskState = 0;
        tpos_task_list[i].taskProperty = 0;
        tpos_task_list[i].pxStack = NULL;
        tpos_task_list[i].uxCriticalNesting = 0;
        tpos_task_list[i].task_dog = 0;
        tpos_task_list[i].wait_mutex = NULL;
        tpos_task_list[i].taskName = NULL;
        tpos_task_list[i].wait_time = 0;
        tpos_task_list[i].wait_event1 = 0;
        tpos_task_list[i].wait_event2 = 0;
        tpos_task_list[i].delay_tick = 0;
    }
}
/*+++
 * Function: TPOS， initialize the stack of process..
 ---*/
OS_STK *tpos_taskStackInit( OS_STK *pxTopOfStack, pTASK_CODE pxCode, void *pvParameters )
{
    OS_STK *p_stk;
	
    p_stk = pxTopOfStack;
    p_stk--;
    *p_stk = 0x89ABCDEF;
    p_stk = (OS_STK *)((OS_STK)(p_stk) & 0xFFFFFFF8);   //make 8 word aligment.  

	  //-----hard core operate stack------------ 
    *p_stk-- = 0x01000000;        //xPSR //status register
    *p_stk-- = ((OS_STK)pxCode);    //PC
    *p_stk-- = ((OS_STK)pxCode);    //r14(LR)
    *p_stk-- = 0x12;              //r12	
    *p_stk-- = 0x03;              //r3
    *p_stk-- = 0x02;              //r2
    *p_stk-- = 0x01;              //r1
    *p_stk-- = (OS_STK)pvParameters;      //r0
	
	  //-----software operate statck-------------
    *p_stk-- = 0x07;              //r7
    *p_stk-- = 0x06;              //r6
    *p_stk-- = 0x05;              //r5
    *p_stk-- = 0x04;              //r4
	
    *p_stk-- = 0x11;              //r11
    *p_stk-- = 0x10;              //r10
    *p_stk-- = 0x09;              //r9
    *p_stk   = 0x08;              //r8


    return ((OS_STK *)p_stk);
}
/*-----------------------------------------------------------*/
/*+++
 * Function : Crate task..
 * Note :
 *        pTask_Code  pTask         task entry.
 *        OS_STK      *pStack       the top of task stack.
 *        void        *parameter    parameter point, store in r0.
 ---*/
OS_TCB *tpos_createTask(pTASK_CODE pTask,OS_STK *pStack,void *parameter,INT8S *taskName,INT8U taskProperty)
{
    OS_TCB *pTCB=NULL;
    INT8U i;
    
     //find a free OS_TCB from rtos_task_list
    for(i=0; i<TASK_MAX_COUNT;i++)
    {
        if(tpos_task_list[i].pTopOfStack == NULL) 
        {
            pTCB = &(tpos_task_list[i]);
            pTCB->taskNo = (INT8U)(i+1);
            break;
        }
    }
    
    if(pTCB==NULL) return NULL;

    pTCB->taskName = taskName;
    pTCB->taskProperty = taskProperty;

    SYSTEM.task_count ++;

    pTCB->pTopOfStack = tpos_taskStackInit(pStack,pTask,parameter);

    if(pxCurrentTCB == NULL) pxCurrentTCB = pTCB;

    return pTCB;
}
static void prvTaskExitError( void )
{
	/* A function that implements a task must not exit or attempt to return to
	its caller as there is nothing to return to.  If a task wants to exit it
	should instead call vTaskDelete( NULL ).

	Artificially force an assert() to be triggered if configASSERT() is
	defined, then stop here so application writers can catch the error. */
	//configASSERT( uxSavedTaskStackPointer == 0UL );
	//portDISABLE_INTERRUPTS();
	for( ;; );
}

/*+++
 *  Function : Start the tpos..
 ---*/
INT8U tpos_start(void) 
{
    extern void os_start(void);

    // Set the running flag.
    SYSTEM.tpos_running = TRUE;
    
    // Start the schedule timer.
    tpos_schedule_timer_open();
    
    // Start the first task.
    os_start();

    // Never get here!!!
    prvTaskExitError();

    return FALSE;
}

/*-----------------------------------------------------------*/
INT32U  os_get_systick_10ms(void)
{
    INT32U  tick10ms;

    tick10ms = SYSTEM.systick_in_10ms;

    return  tick10ms;
}

// Enter critical section, could nest, used for little code protect.
void tpos_enterCriticalSection(void)
{
    if(SYSTEM.tpos_running)
    {
        tpos_schedule_stop();//
        pxCurrentTCB->uxCriticalNesting++;
        
        // critical nest counter increase.
        SYSTEM.critical_nest ++;
    }
}

// Leave critical section, recovery schedule.
void tpos_leaveCriticalSection(void)
{
    if(SYSTEM.tpos_running)
    {
        // critical nest counter decrease.
        if(SYSTEM.critical_nest > 0)
        {
            SYSTEM.critical_nest --;
        }

        if(pxCurrentTCB->uxCriticalNesting > 0)
        {
            pxCurrentTCB->uxCriticalNesting--;
        }

        // still in critical section, then return.
        if(SYSTEM.critical_nest)
        {
            return;
        }

        // Leave critical section, recovery schedule.
        tpos_schedule_restart();//
    }
}

// Yield the right to use the CPU, trigger task schedule.
void tpos_yield(void)
{
    if(SYSTEM.tpos_running)
    {
        tpos_enterCriticalSection();

        // Feed the task dog.
        pxCurrentTCB->task_dog = 0;

        tpos_TASK_YIELD();
        
        tpos_leaveCriticalSection();
    }
}

/*+++
 *  Function : 10ms service, called by system tick interrupt.
 *  Note :
 *         1） system tick.
 ---*/
void tpos_service_10ms(void)
{
    INT16U  ti;

    // check tpos running flag.
    if(SYSTEM.tpos_running)
    {
        if(SYSTEM.task_count == 0)
        {
            tpos_softReset();
        }
    }
    else if(SYSTEM.systick_in_10ms < 2000)
    {
    // 20 seconds for system start..

    }
    
    // process the task state..
    for(ti=0;ti < TASK_MAX_COUNT; ti++)
    {
        // Task control block check..
        if(tpos_task_list[ti].pTopOfStack == NULL)
        {
            if(ti != SYSTEM.task_count)
            {
                //SYSTEM.task_count与tpos_task_list[ti].pTopOfStack不相符了，有变量有问题了
                tpos_softReset();
            }
            break;
        }
        
        // check task delay tick.
        if(tpos_task_list[ti].delay_tick > 0)
        {
            tpos_task_list[ti].delay_tick --;
        }

        // check task wait time..
        if(tpos_task_list[ti].wait_time > 0)
        {
           tpos_task_list[ti].wait_time --;
           if(tpos_task_list[ti].wait_time == 0)
           {
               // time out, cancel the waiting.
               tpos_task_list[ti].wait_mutex = NULL;
               tpos_task_list[ti].wait_event1 = NULL;
               tpos_task_list[ti].wait_event2 = NULL;
           }
        }
        
        // task dog..
        tpos_task_list[ti].task_dog ++;
        // five minutes..
        if(tpos_task_list[ti].task_dog > 30000)
        {
            tpos_softReset();
        }
    } // end of for(ti=0;;)

}

void tpos_switch(void)
{
    int i,task_idx;
    
    //here uxTaskNo start from 1,  so it pointer to next task already.
    //task_idx = os_cur_task + 1;   
    task_idx = pxCurrentTCB->taskNo;

    for(i=0; i< SYSTEM.task_count; i++)
    {
        if(task_idx >= SYSTEM.task_count) task_idx = 0;
        pxNextTCB =  &(tpos_task_list[task_idx]);
        task_idx ++;
        
        // task is waiting for mutex..
        if(pxNextTCB->wait_mutex != NULL)
        {
            if(pxNextTCB->wait_mutex->e.signal)
            {
                continue;
            }
        }
        
        // task is waiting for event..
        if(pxNextTCB->wait_event1  && (*(pxNextTCB->wait_event1) == 0) ) continue;
        if(pxNextTCB->wait_event2  && (*(pxNextTCB->wait_event2) == 0) ) continue;

        
        // task is in delay..
        if(pxNextTCB->delay_tick) continue;

        // task is in the pause state.
        if(pxNextTCB->taskState != TPOS_TASKSTATE_NORMAL) continue;

        // task can be schedule..
        break;
    }

    if(i >=SYSTEM.task_count)
    {
        // there is no task can be schedule, run the first task..
        pxNextTCB =  &(tpos_task_list[0]);
    }

    if(pxNextTCB != pxCurrentTCB)
    {
        tpos_schedule_now();
    }
}

// get current task number..
INT8U get_cur_task_no(void)
{
    if(pxCurrentTCB!=NULL)
    {
        return pxCurrentTCB->taskNo;
    }
    return 0;

}


void HardFault_Handler_C(unsigned long * hardfault_args, unsigned int lr_value)  //hardfault_args存的是栈的值，lr_value是LR的值
{
     unsigned long stacked_r0;
     unsigned long stacked_r1;
     unsigned long stacked_r2;
     unsigned long stacked_r3;
     unsigned long stacked_r12;
     unsigned long stacked_lr;
     unsigned long stacked_pc;
     unsigned long stacked_psr;
     unsigned long cfsr;
     unsigned long bus_fault_address;
     unsigned long memmanage_fault_address;
 
     bus_fault_address       = SCB->BFAR;   //BusFaultAddrReg
     memmanage_fault_address = SCB->MMFAR;  //MemManageAddrReg
     cfsr                    = SCB->CFSR;   //ConfigerableFaultStatusReg
 
     stacked_r0  = ((unsigned long) hardfault_args[0]);  //Stack指向地址的内容，按照自动进栈出栈时的顺序
     stacked_r1  = ((unsigned long) hardfault_args[1]);
     stacked_r2  = ((unsigned long) hardfault_args[2]);
     stacked_r3  = ((unsigned long) hardfault_args[3]);
     stacked_r12 = ((unsigned long) hardfault_args[4]);
     stacked_lr  = ((unsigned long) hardfault_args[5]);
     stacked_pc  = ((unsigned long) hardfault_args[6]);
     stacked_psr = ((unsigned long) hardfault_args[7]);
//76 
//77     printf ("[HardFault]\n");
//78     printf ("- Stack frame:\n");
//79     printf (" R0  = %x\n", stacked_r0);
//80     printf (" R1  = %x\n", stacked_r1);
//81     printf (" R2  = %x\n", stacked_r2);
//82     printf (" R3  = %x\n", stacked_r3);
//83     printf (" R12 = %x\n", stacked_r12);
//84     printf (" LR  = %x\n", stacked_lr);
//85     printf (" PC  = %x\n", stacked_pc);  //注意这个是在HardFault发生前的PC地址，说明运行到这里发生HF【定位HF发生的指令】
//86     printf (" PSR = %x\n", stacked_psr);
//87     printf ("- FSR/FAR:\n");
//88     printf (" CFSR = %x\n", cfsr);
//89     printf (" HFSR = %x\n", SCB->HFSR);  //HardFaultStatusReg
//90     printf (" DFSR = %x\n", SCB->DFSR);  //DebugFaultStatusReg
//91     printf (" AFSR = %x\n", SCB->AFSR);  //Auxiliary Fault Status Reg 附加的..具体看Cortex M3 Technical Reference Manual
//92     if (cfsr & 0x0080) printf (" MMFAR = %x\n", memmanage_fault_address);
//93     if (cfsr & 0x8000) printf (" BFAR = %x\n", bus_fault_address);
//94     printf ("- Misc\n");
//95     printf (" LR/EXC_RETURN= %x\n", lr_value);    
}




