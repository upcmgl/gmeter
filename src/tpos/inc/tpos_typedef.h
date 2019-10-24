/* 
 * File:   base_data_type.h
 * Author: Administrator
 *
 * Created on 2014-09-05
 */

#ifndef __TOPSCOMM_DEVP_BASE_DATA_TYPE_H__
#define	__TOPSCOMM_DEVP_BASE_DATA_TYPE_H__

#ifdef	__cplusplus
extern "C" {
#endif
	
#include "stdint.h"
    
typedef unsigned char    BOOLEAN;
typedef unsigned char    INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char    INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned short   INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   short   INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned int     INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   int     INT32S;                   /* Signed   32 bit quantity                           */
typedef unsigned long long   INT64U;
typedef long long   INT64S;
typedef float            FP32;                     /* Single precision floating point                    */
typedef double           FP64;                     /* Double precision floating point                    */


typedef unsigned int OS_STK;
typedef unsigned int *OS_EVENT_HANDLE;     // address of event data
typedef unsigned int OS_EVENT;
typedef unsigned int OS_TICK10MS;   //10ms tick number

typedef void (*t_fun_callback)(void);

#ifndef TRUE
#define TRUE  0x01
#endif

#ifndef FALSE
#define FALSE 0x00
#endif


#ifndef NULL
 #define NULL   0UL
#endif

#ifndef PIN_HIGH
#define PIN_HIGH                1      // high 
#endif
#ifndef PIN_LOW
#define PIN_LOW                 0      // low
#endif

#ifdef	__cplusplus
}
#endif

#ifndef __SOFT_SIMULATOR__
#define WEAK __attribute__(( weak ))
#else
#define WEAK
#endif 


#ifdef __SOFT_SIMULATOR__
typedef enum {

    USART_8N1 = 0x00,
    USART_8E1 = 0x01,
    USART_8O1 = 0x02,
    USART_9N1 = 0x03,
    USART_8N2 = 0x04,
    USART_8E2 = 0x05,
    USART_8O2 = 0x06,
    USART_9N2 = 0x07

} USART_LINECONTROL_MODE;
typedef INT8U USART_MODULE_ID;


typedef unsigned char    uint8_t;                    /* Unsigned  8 bit quantity                           */
typedef signed   char    int8_t;                    /* Signed    8 bit quantity                           */
typedef unsigned short   uint16_t;                   /* Unsigned 16 bit quantity                           */
typedef signed   short   int16_t;                   /* Signed   16 bit quantity                           */
typedef unsigned int     uint32_t;                   /* Unsigned 32 bit quantity                           */
typedef signed   int     int32_t;                   /* Signed   32 bit quantity                           */

#endif

#define NOP()            __asm("NOP")
#define Nop()            __asm("NOP")
#define DI()             __asm("CPSID i")    //disable irq
#define EI()             __asm("CPSIE i")    //enable irq

#define STRUCT_OFFSET(struct_object,e) (unsigned long)&(((struct_object*)0)->e)
#endif	/* __TOPSCOMM_DEVP_BASE_DATA_TYPE_H__ */

