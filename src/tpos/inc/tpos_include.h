#ifndef     _TPOS_INCLUDE_H_
#define     _TPOS_INCLUDE_H_
//-------------------------------------------------

#include "tpos_typedef.h"



#define SYS_FREQ        65536000u

#define SYSTEM_FREQ     SYS_FREQ
#define system_freq     SYS_FREQ
#define SYSTEM_CLOCK    (SYS_FREQ/100)
#define system_clock    (SYS_FREQ/100)



typedef INT32U OS_STK;
typedef INT16S (*P_READ)(void);    



#define EnIsr_PendSv    {if(os_task_next != os_cur_task){SCB_SHPR3_PSV  = 0xc0;	SCB_ICSR_PENDSVSET = 1;}}
#define task_to_plc     {os_task_next = TASK_INDEX_TCC; EnIsr_PendSv} 	
#define task_to_app     {os_task_next = TASK_INDEX_TCA; EnIsr_PendSv}  	

#define TASK_TO(x)    {os_task_next = (x); EnIsr_PendSv}

//task index for two task.
#define TASK_INDEX_TCC   0
#define TASK_INDEX_TCA   1


void ClrWdt( void );
#define  Reset()          tpos_softReset()


//--------------------------------------------------
#define bit(x)  (1<<x)


#define __EMPTY_FLASH_VALUE  0xFFFFFFFF
#define __ALLAA_FLASH_VALUE  0xAAAAAAAA


#define  FLASH_INT     INT64U 


#define FLASH_CMD_WRITE_WORD    0
#define FLASH_CMD_ERASE_PAGE    1
#define FLASH_CMD_ERASE_CHIP    2


//--------------------------------------------------
#endif  //_TPOS_INCLUDE_H_
//--------------------------------------------------
