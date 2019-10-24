

#include "tpos.h"
#include "tpos_service.h"
#include "tpos_cpu.h"

// weak function.
void WEAK tpos_before_softReset(void)
{
    #ifndef __SOFT_SIMULATOR__
//    pin_ctrl_gprs_rst(PIN_LOW);
//    pin_ctrl_power_ctrl(PIN_LOW);
    #endif
}

void  tpos_softReset(void)
{
    tpos_before_softReset();
    tpos_cpu_rst();

    while(1);
}
/*+++
 *  function : get mutex.
 *  parameter:
 *         objMutex   *pMutex
 *         INT16U     timeout, 0 for no limit wait.
 ---*/
void  tpos_mutexPend(objMutex *pMutex)
{
     if(tpos_mutexGet(pMutex,0) == FALSE)
     {
         #ifdef __DEBUG
        // break into the debugger
        __asm__ volatile (" sdbbp 0");
         #endif
        tpos_softReset();
     }
}

/*+++
 * function : release mutex..
 * note :
 *      1) if mutex is idle after release, trigger the task schedule..
 ---*/
void tpos_mutexFree(objMutex *pMutex)
{
   if(!SYSTEM.tpos_running) return;

   tpos_enterCriticalSection();

   // mutex counter decrease..
   if(pMutex->e.signal) pMutex->e.signal--;
   if(pMutex->e.signal == 0)
   {
       // mutex counter equal to 0, mutex is idle now..
       pMutex->e.taskNo = 0;

       // if there is other task is waiting, trigger the task schedule..
       if(pMutex->e.waitFlag)
       {
             #ifndef __SOFT_SIMULATOR__
             pxCurrentTCB->task_dog = 0;
             tpos_TASK_YIELD();
            #endif
       }
   }
   tpos_leaveCriticalSection();
}

/*+++
 *  function : get mutex.
 *  parameter:
 *         objMutex   *pMutex
 *         INT16U     timeout, 0 for no limit wait.
 ---*/
INT8U tpos_mutexGet(objMutex *pMutex, INT32U timeout_10ms)
{
    OS_TCB  *pTCB;

    if(!SYSTEM.tpos_running) return TRUE;

    while(1)
    {
        tpos_enterCriticalSection();

        // if use mutex we can't use event.
        #ifndef __SOFT_SIMULATOR__
        pxCurrentTCB->wait_event1 = NULL;
        pxCurrentTCB->wait_event2 = NULL;
        #endif
        // Mutex is idle..
        if(0 == pMutex->e.signal)
        {
            pMutex->e.signal ++;
            #ifndef __SOFT_SIMULATOR__
            pMutex->e.taskNo = pxCurrentTCB->taskNo;
            #else
            pMutex->e.taskNo = get_cur_task_no();
            #endif
            pMutex->e.waitFlag = 0;
            #ifndef __SOFT_SIMULATOR__
            pxCurrentTCB->task_dog = 0;
            pxCurrentTCB->wait_mutex = NULL;
            pxCurrentTCB->wait_time = 0;
            #endif
            tpos_leaveCriticalSection();
            return TRUE;
        }

        // Mutex is busy, but is used by current task..
        #ifndef __SOFT_SIMULATOR__
        if(pMutex->e.taskNo  == pxCurrentTCB->taskNo )
        #else
        if(pMutex->e.taskNo  == get_cur_task_no() )
        #endif
        {
            pMutex->e.signal ++;    // Mutex nest.
            tpos_leaveCriticalSection();
            return TRUE;
        }
        
        #ifndef __SOFT_SIMULATOR__
        // Mutex is busy, and is used by other task..
        pxCurrentTCB->task_dog = 0;
        pxCurrentTCB->wait_mutex = pMutex;
        pxCurrentTCB->wait_time = timeout_10ms;

        // trigger task schedule..
        if(pxCurrentTCB->taskState ==  TPOS_TASKSTATE_STOP) return FALSE;

        // store current task control block point.
        pTCB = pxCurrentTCB;

        tpos_TASK_YIELD();
        #endif
        tpos_leaveCriticalSection();

        if(pMutex->e.signal > 0)
        {
            // Mutex still busy..
            if(timeout_10ms > 0)
            {
                // check the time out ..
                if(pTCB->task_dog  > timeout_10ms)
                {
                    return FALSE;
                }
            }
        }
    } // endof while(1)
    return FALSE;
}
/*+++
 *  function : try to get mutex, if fail return false..
 *  parameter : 
 *         objMutex   *pMutex
 ---*/
INT8U tpos_mutexRequest(objMutex *pMutex)
{
    if(!SYSTEM.tpos_running) return TRUE;

    tpos_enterCriticalSection();

    // if mutex is used, can't use event..
    #ifndef __SOFT_SIMULATOR__
    pxCurrentTCB->wait_event1 = NULL;
    pxCurrentTCB->wait_event2 = NULL;
    #endif
    // mutex is idle..
    if(0 == pMutex->e.signal)
    {
        pMutex->e.signal ++;
        #ifndef __SOFT_SIMULATOR__
        pMutex->e.taskNo = pxCurrentTCB->taskNo;
        #else
        pMutex->e.taskNo = get_cur_task_no();
        #endif
        pMutex->e.waitFlag = 0;
        #ifndef __SOFT_SIMULATOR__
        pxCurrentTCB->task_dog = 0;
        pxCurrentTCB->wait_mutex = NULL;
        pxCurrentTCB->wait_time = 0;
        #endif
        tpos_leaveCriticalSection();
        return TRUE;
    }

    // mutex is busy..
    #ifndef __SOFT_SIMULATOR__
    if(pMutex->e.taskNo  == pxCurrentTCB->taskNo )
    #else
    if(pMutex->e.taskNo  == get_cur_task_no() )
    #endif
    {
        pMutex->e.signal ++;    // mutex nest..
        tpos_leaveCriticalSection();
        return TRUE;
    }

    tpos_leaveCriticalSection();
    return FALSE;
}
//delay_ms < 57266

/*+++
 * function : current task sleep, unit 1ms..
 ---*/
void tpos_sleep(INT32U sleep_time)
{
    INT32U  wait_tick,tick_start;

    wait_tick = sleep_time / 10 + 1;

    if(SYSTEM.tpos_running)
    {
        // system is running..
        tpos_enterCriticalSection();
        #ifndef __SOFT_SIMULATOR__
        pxCurrentTCB->delay_tick = wait_tick;
        pxCurrentTCB->task_dog = 0;
        tpos_TASK_YIELD();
        #endif
        tpos_leaveCriticalSection();
    }
    else
    {
        // system is not running..
        #ifndef __SOFT_SIMULATOR__
        tick_start = system_get_tick10ms();
        while(system_get_tick10ms() - tick_start < wait_tick)
        {
            extern void ClrTaskWdt(void);
            ClrTaskWdt();
        }
        #else
        Sleep(sleep_time);
        #endif
    }
}


// clear task dog..
void  tpos_clrTaskWdt(void)
{
    if(!SYSTEM.tpos_running) return ;
    tpos_enterCriticalSection();
    #ifndef __SOFT_SIMULATOR__
    pxCurrentTCB->task_dog = 0;
    #endif
    tpos_leaveCriticalSection();
}

/*+++
 * function : wait for event happen..
 * parameter : 
 *          OS_EVENT        wait_event1
 *          OS_EVENT        wait_event2   
 *          OS_TICK10MS      time_out  
 * return : 
 *          TRUE   
 *          FALSE  time out quit.
 ---*/
INT8U  tpos_wait2Event(OS_EVENT_HANDLE wait_event1,OS_EVENT_HANDLE wait_event2,OS_TICK10MS  timeout_10ms)
{
    OS_TCB  *pTCB;

    if(!SYSTEM.tpos_running) return TRUE;
    #ifdef __SOFT_SIMULATOR__
        return FALSE;
    #endif
    
    while(1)
    {
        tpos_enterCriticalSection();

        // can't use mutex if we use event..
        pxCurrentTCB->wait_mutex = NULL;

        // event is ready..
        if( ( wait_event1 && (*wait_event1) )  ||  ( wait_event2 && (*wait_event2) )  )
        {
            pxCurrentTCB->task_dog = 0;
            pxCurrentTCB->wait_event1 = NULL;
            pxCurrentTCB->wait_event2 = NULL;
            tpos_leaveCriticalSection();
            return TRUE;
        }

        // event is not ready, hang up the task and wait..
        pxCurrentTCB->task_dog = 0;
        pxCurrentTCB->wait_event1 = wait_event1;
        pxCurrentTCB->wait_event2 = wait_event2;
        pxCurrentTCB->wait_time = timeout_10ms;

        // trigger the task schedule..
        if(pxCurrentTCB->taskState ==  TPOS_TASKSTATE_STOP) return FALSE;

        // store the current task control block..
        pTCB = pxCurrentTCB;

        #ifndef __SOFT_SIMULATOR__
            tpos_TASK_YIELD();
        #endif
        tpos_leaveCriticalSection();

        if( ( wait_event1 && (*wait_event1) )  ||  ( wait_event2 && (*wait_event2) )  )
        {
            // event is ready..
            continue;
        }
        else
        {
            // event is not ready, check time out..
            if(timeout_10ms > 0)
            {
                if(pTCB->task_dog  > timeout_10ms)
                {
                    return FALSE;
                }
            }
        }
    } // end of while(1)
    return FALSE;
}

/*+++
 * function : wait for event happen..
 * parameter : 
 *          OS_EVENT        wait_event
 *          OS_TICK10MS      time_out  
 * return : 
 *          TRUE   
 *          FALSE  time out quit.
 ---*/
INT8U  tpos_waitEvent(OS_EVENT_HANDLE wait_event,OS_TICK10MS  timeout_10ms)
{
    return tpos_wait2Event(wait_event,NULL,timeout_10ms);
}

INT8U  tpos_taskLock(void)
{
    #ifndef __SOFT_SIMULATOR__
    INT8U task_id;
    if(!SYSTEM.tpos_running) return TRUE;
    
    // lock user task, system task and current task still running..
    for(task_id=0;task_id<SYSTEM.task_count;task_id++)
    {
        if((tpos_task_list[task_id].taskProperty==TPOS_TASKPROPERTY_USER) && (tpos_task_list[task_id].taskNo !=pxCurrentTCB->taskNo)  )
        {
            if( tpos_task_list[task_id].taskState== TPOS_TASKSTATE_NORMAL)
            {
                tpos_task_list[task_id].taskState = TPOS_TASKSTATE_PAUSE;
            }
        }
    }
    #endif
    return TRUE;
}

INT8U  tpos_taskUnLock(void)
{
    #ifndef __SOFT_SIMULATOR__
    INT8U task_id;
     if(!SYSTEM.tpos_running) return TRUE;
     
    // unlock user task, system task and current task still running..
    for(task_id=0;task_id<SYSTEM.task_count;task_id++)
    {
        if((tpos_task_list[task_id].taskProperty==TPOS_TASKPROPERTY_USER) && (tpos_task_list[task_id].taskNo !=pxCurrentTCB->taskNo)  )
        {
            if( tpos_task_list[task_id].taskState== TPOS_TASKSTATE_PAUSE)
            {
                tpos_task_list[task_id].taskState = TPOS_TASKSTATE_NORMAL;
            }
        }
    }
    return TRUE;
    #endif
}

void tpos_Soft2msHandler(void)
{
   
}

INT32U  system_get_tick10ms(void)
{
    INT32U  tick10ms;

    tick10ms = SYSTEM.systick_in_10ms;

//    #ifdef __SOFT_SIMULATOR__
//    tick10ms=GetTickCount()/10;
//    #endif
    return  tick10ms;
}



