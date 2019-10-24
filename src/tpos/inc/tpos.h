#ifndef __TOPSCOMM_RTOS_MACRO_H__
#define __TOPSCOMM_RTOS_MACRO_H__

/* System include files */

#include "tpos_typedef.h"

typedef void (*pTASK_CODE)( void );

typedef void (*pfun_no_para)( void );
typedef void (*pfun_with_para)( void *);

#define TASK_MAX_COUNT       8

#define tpos_NOP()    { Nop();}


#define TPOS_TASKSTATE_STOP         0xAA
#define TPOS_TASKSTATE_PAUSE        0x55
#define TPOS_TASKSTATE_NORMAL       0x00

#define TPOS_TASKPROPERTY_SYS       0x01    // System task.
#define TPOS_TASKPROPERTY_USER      0x02    // User task.


/*+++
 *   Mutex : Scramble mechanism.
 ---*/
typedef union{
     unsigned short   mutex;
    struct{
      unsigned short   signal:7;     // 0: mutex is free now.  otherwise is busy.
      unsigned short   taskNo:8;     // task number of the task which is using the mutex..
      unsigned short   waitFlag:1;   // flag of the mutex being scrambled, 0£º no,  1£º yes.
    }e;
}objMutex;

typedef struct tskTaskControlBlock
{
	volatile OS_STK	*pTopOfStack;		/*< Points to the location of the last item placed on the tasks stack.  THIS MUST BE THE FIRST MEMBER OF THE TCB STRUCT. */

	//xListItem				xGenericListItem;	/*< The list that the state list item of a task is reference from denotes the state of that task (Ready, Blocked, Suspended ). */
	//xListItem				xEventListItem;		/*< Used to reference a task from an event list. */
    INT8U           taskNo;
	INT8U      	taskPriority;			/*< The priority of the task.  0 is the lowest priority. */
    INT8U           taskState;                    // task state.
    INT8U           taskProperty;                 // task property.
	OS_STK		*pxStack;			/*< Points to the start of the stack. */
	//signed char				pcTaskName[ configMAX_TASK_NAME_LEN ];/*< Descriptive name given to the task when created.  Facilitates debugging only. */

	OS_STK uxCriticalNesting; /*< Holds the critical section nesting depth for ports that do not maintain their own count in the port layer. */

    INT32U              task_dog;
    objMutex            *wait_mutex;
    INT8S               *taskName;            // task name..
    OS_TICK10MS         wait_time;            // Mutex or Event max wait time..
    OS_EVENT_HANDLE     wait_event1;
    OS_EVENT_HANDLE     wait_event2;
    INT16U              delay_tick;           // task delay time..

#if ( configUSE_APPLICATION_TASK_TAG == 1 )
    pdTASK_HOOK_CODE pxTaskTag;
#endif

#if ( configGENERATE_RUN_TIME_STATS == 1 )
    unsigned long ulRunTimeCounter;			/*< Stores the amount of time the task has spent in the Running state. */
#endif
} OS_TCB;



typedef struct{
    INT8U            task_count;             // task count.
    INT8U            tpos_running;           // system running flag.
    INT8U            critical_nest;          // Critical area nest counter.
    INT8U            app_running;            // Application running flag.
    volatile INT32U  systick_in_10ms;        // unit 10ms,  since system started.
}objSYSTEM;

extern OS_TCB tpos_task_list[TASK_MAX_COUNT];
extern OS_TCB * volatile pxCurrentTCB;
extern objSYSTEM  SYSTEM;

extern void tpos_init(void);
extern INT8U tpos_start(void);
extern INT32U  os_get_systick_10ms(void);
extern void tpos_enterCriticalSection(void);
extern void tpos_leaveCriticalSection(void);
extern void tpos_yield(void);
extern void tpos_service_10ms(void);
extern void tpos_switch(void);
extern INT8U get_cur_task_no(void);
extern OS_TCB *tpos_createTask(pTASK_CODE pTask,OS_STK *pStack,void *parameter,INT8S *taskName,INT8U taskProperty);
extern void tpos_dbg_tcb_output(void);

#endif /* __TOPSCOMM_RTOS_MACRO_H__ */

