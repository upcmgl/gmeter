/*
 * drv_uart.h
 *
 * Created: 2018/12/11 星期二 11:53:00
 *  Author: Administrator
 */ 


#ifndef DRV_UART_H_
#define DRV_UART_H_


typedef enum{
    UART_ID_DEBUG,
    UART_ID_METER,
    UART_ID_GPRS,
    UART_ID_IR,
    
    MAX_UART_CHN
}tagUART_ID;
typedef enum{
    PARITY_NONE,
    PARITY_ODD,
    PARITY_EVEN,
    
    MAX_PARITY_TYPE
}tagUART_PARITY;


typedef enum{
    PIN_FUN_A,
    PIN_FUN_B,
    PIN_FUN_C,
    PIN_FUN_D
    
}atgPIN_FUN;
typedef struct{
    INT16U recv_buf_len;
    Usart * obj;
    Flexcom *flexcom;
    Pdc * pdc;
    IRQn_Type irq;
    
}tagUART_CTL;
extern tagUART_CTL uart_ctl[MAX_UART_CHN];

#define PORT_UART_DEBUG PIOA
#define PIN_UART_DEBUG_TX   (1<<3)
#define PIN_UART_DEBUG_RX   (1<<4)

#define PORT_UART_METER PIOA
#define PIN_UART_METER_TX   (1<<6)
#define PIN_UART_METER_RX   (1<<5)

#define PORT_UART_GPRS PIOB
#define PIN_UART_GPRS_TX   (1<<10)
#define PIN_UART_GPRS_RX   (1<<11)

#define PORT_UART_IR PIOA
#define PIN_UART_IR_RX   (1<<27)



// IR
#define PORT_IR_TX  PIOA
#define PIN_IR_TX   (1<<23)



void debug_uart_init(INT32U baud);

void meter_uart_init(INT32U baud);

void gprs_uart_init(INT32U baud);

void ir_init(INT32U baud);

void ir_lanch_rx(void);

void ir_stop_rx(void);

INT16U drv_uart_get_recv_pos(INT8U id);

void drv_uart_lanch_tx(INT8U id, INT8U *buf, INT16U len);

void drv_uart_stop_tx(INT8U id);

void drv_uart_lanch_rx(INT8U id, INT8U *buf, INT16U max_len);

void drv_uart_stop_rx(INT8U id);


extern void uart_debug_send_complete(void);
extern void uart_meter_send_complete(void);
extern void uart_gprs_send_complete(void);
extern void uart_ir_send_complete(void);


#endif /* DRV_UART_H_ */