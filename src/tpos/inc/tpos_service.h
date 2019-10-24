#ifndef ___TOPS_SERVICE_H__
#define ___TOPS_SERVICE_H__

#include "tpos.h"
#include "tpos_typedef.h"

extern void    tpos_softReset(void);
#define sys_reset() tpos_softReset()    //使用void delay_reset(void);延时复位
extern void    tpos_clrTaskWdt(void);
extern void    tpos_sleep(INT32U sleep_time);
extern void    tpos_abs_delay_ms(INT32U delay_ms);
extern INT8U   tpos_taskLock(void);
extern INT8U   tpos_taskUnLock(void);

extern void    tpos_mutexPend(objMutex *pMutex);
extern void    tpos_mutexFree(objMutex *pMutex);
extern INT8U   tpos_mutexRequest(objMutex *pMutex);
extern INT8U   tpos_mutexGet(objMutex *pMutex, INT32U timeout_10ms);
extern INT8U   tpos_wait2Event(OS_EVENT_HANDLE wait_event1,OS_EVENT_HANDLE wait_event2,OS_TICK10MS  timeout_10ms);
extern INT8U   tpos_waitEvent(OS_EVENT_HANDLE wait_event,OS_TICK10MS  timeout_10ms);
extern INT32U  system_get_tick10ms(void);

#define OSMutexPend tpos_mutexPend
#define OSMutexFree tpos_mutexFree
#endif
