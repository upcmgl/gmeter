/* 
 * File:   tpos_config.h
 *
 * TPOS 的配置文件
 *
 */

#ifndef TPOS_CONFIG_H
#define	TPOS_CONFIG_H

//#include <xc.h>

//任务调度中断级别，必须是1
#define configKERNEL_INTERRUPT_PRIORITY                         0x01

//系统服务允许的最大中断级别，目前也必须是1。  暂不支持从中断中调用服务函数。
#define configMAX_SYSCALL_INTERRUPT_PRIORITY                    0x01

//定义关键任务区的实现方式， 使用DI,EI 指令方式的话， 定义取值1  否则定义0！！！！目前不能使用1，只能定义为0
#define configCRITICAL_SECTION_METHOD_DI_EI                      0

//任务狗的看护周期,单位 10ms
#define configMAX_TASK_SOFT_DOG_10MS                            30000


//PIC32MZ  CPO_STATUS  寄存器
// 0: IE    1: EXL   2: ERL   3: R    4: UM   5~7: R    8~9: IM1..IM0
// 10~16:  IPL(IM8..IM2)     17:CEE    18: IPL(IM9)
// 19:NMI   20:SR  21:TS   22:BEV   23~24:R   25:RE  26:FR   27:RP  28~31:CU3..CU0

#define MCU_IPL_SHIFT				( 10UL )

//中断级别只关心CEE以下部分。
#define MCU_ALL_IPL_BITS			( 0x7FUL << MCU_IPL_SHIFT )

//系统服务允许的中断级别
#define MCU_SYSCALL_IPL_BITS                   (configMAX_SYSCALL_INTERRUPT_PRIORITY << MCU_IPL_SHIFT)

//允许任务中断:  执行后允许发生任务调度
//#define tpos_ENABLE_TASK_INTERRUPTS()					\
//{									\
        //uint32_t ulStatus;					 	\
									//\
	///* Unmask all interrupts. */					\
	//ulStatus = _CP0_GET_STATUS();					\
	//ulStatus &= ~MCU_ALL_IPL_BITS;					\
	//_CP0_SET_STATUS( ulStatus );					\
//}
//
////禁止任务中断：  执行后不再发生任务调度
//#define tpos_DISABLE_TASK_INTERRUPTS()					\
//{									\
	//uint32_t ulStatus;						\
									//\
	//ulStatus = _CP0_GET_STATUS();					\
	//ulStatus &= ~MCU_ALL_IPL_BITS;					\
	//_CP0_SET_STATUS( ( ulStatus | MCU_SYSCALL_IPL_BITS) ) ;        \
//}
//
//#define tpos_TASK_YIELD()						\
//{									\
       //PLIB_INT_SourceFlagSet(INT_ID_0, INT_SOURCE_TIMER_CORE);         \
//}

#endif	/* TPOS_CONFIG_H */

