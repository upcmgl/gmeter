/*
 * drv_uart.c
 *
 * Created: 2018/12/11 星期二 11:50:49
 *  Author: Administrator
 */ 

#include "asf.h"
#include "os.h"
#include "drv_uart.h"


tagUART_CTL uart_ctl[MAX_UART_CHN];
void bsp_uart_init( void )
{
    //FLEXCOMM 3
    uart_ctl[UART_ID_DEBUG].obj = USART3;
    uart_ctl[UART_ID_DEBUG].flexcom = FLEXCOM3;
    uart_ctl[UART_ID_DEBUG].irq = FLEXCOM3_IRQn;
    uart_ctl[UART_ID_DEBUG].pdc = PDC_USART3;

    //FLEXCOMM 2
    uart_ctl[UART_ID_METER].obj = USART2;
    uart_ctl[UART_ID_METER].flexcom = FLEXCOM2;
    uart_ctl[UART_ID_METER].irq = FLEXCOM2_IRQn;
    uart_ctl[UART_ID_METER].pdc = PDC_USART2;

    //FLEXCOMM 4
    uart_ctl[UART_ID_GPRS].obj = USART4;
    uart_ctl[UART_ID_GPRS].flexcom = FLEXCOM4;
    uart_ctl[UART_ID_GPRS].irq = FLEXCOM4_IRQn;
    uart_ctl[UART_ID_GPRS].pdc = PDC_USART4;

    //FLEXCOMM 7
    uart_ctl[UART_ID_IR].obj = USART7;
    uart_ctl[UART_ID_IR].flexcom = FLEXCOM7;
    uart_ctl[UART_ID_IR].irq = FLEXCOM7_IRQn;
    uart_ctl[UART_ID_IR].pdc = PDC_USART7;
}

void drv_uart_init(INT8U id, INT32U baud, INT8U parity)
{
    sam_usart_opt_t uart_opt;
    uart_opt.channel_mode = US_MR_CHMODE_NORMAL;
    uart_opt.char_length = US_MR_CHRL_8_BIT;
    uart_opt.irda_filter = 0;
    uart_opt.stop_bits = US_MR_NBSTOP_1_BIT;
    uart_opt.baudrate =   baud;
    if(parity == PARITY_NONE)
    {
        uart_opt.parity_type = US_MR_PAR_NO;
    }
    else if(parity == PARITY_ODD)
    {
        uart_opt.parity_type = US_MR_PAR_ODD;
    }  
    else
    {
        uart_opt.parity_type = US_MR_PAR_EVEN;
    }

    usart_disable_writeprotect(uart_ctl[id].obj);
    usart_init_rs232(uart_ctl[id].obj , &uart_opt, SYS_FREQ);
    //usart_enable_writeprotect(uart_ctl[id].obj);
    
    NVIC_EnableIRQ(uart_ctl[id].irq);
}

void drv_uart_pin_init(Pio * pio, INT32U tx_pin, INT32U rx_pin, INT8U pin_fun)
{
    pio->PIO_WPMR = 0x50494F00;
    pio->PIO_PDR = tx_pin|rx_pin;//禁止gpio功能
    pio->PIO_PUER = rx_pin;//|tx_pin;//接收上拉
    pio->PIO_SODR = tx_pin;//输出使能
    pio->PIO_OER = tx_pin;//输出
    pio->PIO_ODR = rx_pin;//输入
    if(pin_fun&0x01)
    {
       pio->PIO_ABCDSR[0] |= (tx_pin|rx_pin); 
    }
    else
    {
       pio->PIO_ABCDSR[0] &= ~(tx_pin|rx_pin); 
    }
    if(pin_fun&0x02)
    {
        pio->PIO_ABCDSR[1] |= (tx_pin|rx_pin);
    }
    else
    {
        pio->PIO_ABCDSR[1] &= ~(tx_pin|rx_pin);
    }
    
    //pio->PIO_WPMR = 0x50494F01;
}

#include "dev_driver.h"

//parity = EVEN ！！
void debug_uart_init(INT32U baud)
{
	pmc_set_writeprotect(FALSE);
	pmc_disable_periph_clk(ID_FLEXCOM3);
    pmc_enable_periph_clk(ID_FLEXCOM3);
    
    drv_uart_pin_init(PORT_UART_DEBUG, PIN_UART_DEBUG_TX, PIN_UART_DEBUG_RX, PIN_FUN_A);
    
    flexcom_enable(uart_ctl[UART_ID_DEBUG].flexcom);
    flexcom_set_opmode(uart_ctl[UART_ID_DEBUG].flexcom, FLEXCOM_USART);
    
    drv_uart_init(UART_ID_DEBUG, baud, PARITY_EVEN);
    
    drv_uart_stop_rx(UART_ID_DEBUG);
	
	UartObjDebug.read_pos = 0;
    drv_uart_lanch_rx(UART_ID_DEBUG, UartObjDebug.recv_buf, sizeof(UartObjDebug.recv_buf));
}
//parity = EVEN ！！
void meter_uart_init(INT32U baud)
{
	pmc_set_writeprotect(FALSE);
	pmc_disable_periph_clk(ID_FLEXCOM2);
    pmc_enable_periph_clk(ID_FLEXCOM2);
    
    drv_uart_pin_init(PORT_UART_METER, PIN_UART_METER_TX, PIN_UART_METER_RX, PIN_FUN_A);
#ifdef __METER_TX_PULL_UP__
	PORT_UART_METER->PIO_PUER = PIN_UART_METER_TX;//发送上拉
#endif
    pio_set_writeprotect(PORT_UART_METER, FALSE);
    pio_set_multi_driver(PORT_UART_METER, PIN_UART_METER_TX, 1);//enable TX OpenDrain
    pio_pull_up(PORT_UART_METER, PIN_UART_METER_RX, 0);
    
    flexcom_enable(uart_ctl[UART_ID_METER].flexcom);
    flexcom_set_opmode(uart_ctl[UART_ID_METER].flexcom, FLEXCOM_USART);
    
    drv_uart_init(UART_ID_METER, baud, PARITY_EVEN);
    
    drv_uart_stop_rx(UART_ID_METER);
	
	UartObjMeter.read_pos = 0;
    drv_uart_lanch_rx(UART_ID_METER, UartObjMeter.recv_buf, sizeof(UartObjMeter.recv_buf));
}

//parity = NONE ！！
void gprs_uart_init(INT32U baud)
{
    pmc_set_writeprotect(FALSE);
	pmc_disable_periph_clk(ID_FLEXCOM4);
    pmc_enable_periph_clk(ID_FLEXCOM4);
    
    drv_uart_pin_init(PORT_UART_GPRS, PIN_UART_GPRS_TX, PIN_UART_GPRS_RX, PIN_FUN_A);
    pio_set_writeprotect(PORT_UART_GPRS, FALSE);
    pio_set_multi_driver(PORT_UART_GPRS, PIN_UART_GPRS_TX, 1);//enable TX OpenDrain
    pio_pull_up(PORT_UART_GPRS, PIN_UART_GPRS_TX, 0);//disable TX pullup

    
    flexcom_enable(uart_ctl[UART_ID_GPRS].flexcom);
    flexcom_set_opmode(uart_ctl[UART_ID_GPRS].flexcom, FLEXCOM_USART);
    
    drv_uart_init(UART_ID_GPRS, baud, PARITY_NONE);
    
    drv_uart_stop_rx(UART_ID_GPRS);
	
	UartObjGprs.read_pos = 0;
    drv_uart_lanch_rx(UART_ID_GPRS, UartObjGprs.recv_buf, sizeof(UartObjGprs.recv_buf));
}

//parity = EVEN ！！
void ir_init(INT32U baud)
{
    //ir rx
    pmc_set_writeprotect(FALSE);
	pmc_disable_periph_clk(ID_FLEXCOM7);
    pmc_enable_periph_clk(ID_FLEXCOM7);
    
    drv_uart_pin_init(PORT_UART_IR, 0, PIN_UART_IR_RX, PIN_FUN_B);
    
    flexcom_enable(uart_ctl[UART_ID_IR].flexcom);
    flexcom_set_opmode(uart_ctl[UART_ID_IR].flexcom, FLEXCOM_USART);
    
    drv_uart_init(UART_ID_IR, baud, PARITY_EVEN);
    
    drv_uart_stop_rx(UART_ID_IR);
	
	UartObjIr.read_pos = 0;
    drv_uart_lanch_rx(UART_ID_IR, UartObjIr.recv_buf, sizeof(UartObjIr.recv_buf));
    //ir tx
    drv_ir_tx_init(baud);
}
void ir_lanch_rx()
{
    drv_uart_lanch_rx(UART_ID_IR, UartObjIr.recv_buf, sizeof(UartObjIr.recv_buf));
}
void ir_stop_rx()
{
    drv_uart_stop_rx(UART_ID_IR);
}

INT16U drv_uart_get_recv_pos(INT8U id)
{
    Pdc * pdc = uart_ctl[id].pdc;
    INT16U pos;
    
    pos = (uart_ctl[id].recv_buf_len -  pdc->PERIPH_RCR);
    
    return pos;
}
void drv_uart_lanch_tx(INT8U id, INT8U *buf, INT16U len)
{
    Pdc * pdc = uart_ctl[id].pdc;
    Usart * uart = uart_ctl[id].obj;
    
    usart_enable_tx(uart);
    

    pdc->PERIPH_TPR = (INT32U)buf;
    pdc->PERIPH_TCR = len;
    pdc->PERIPH_PTCR = PERIPH_PTCR_TXTEN|PERIPH_PTCR_TXCBDIS;
    
    usart_enable_interrupt(uart, US_IER_TXEMPTY);   
}

void drv_uart_stop_tx(INT8U id)
{
    Pdc * pdc = uart_ctl[id].pdc;
    Usart * uart = uart_ctl[id].obj;

    usart_disable_tx(uart);
    usart_disable_interrupt(uart, US_IER_TXEMPTY);

    pdc->PERIPH_TCR = 0;
    pdc->PERIPH_PTCR = PERIPH_PTCR_TXTDIS;
}

void drv_uart_lanch_rx(INT8U id, INT8U *buf, INT16U max_len)
{
    Pdc * pdc = uart_ctl[id].pdc;
    Usart * uart = uart_ctl[id].obj;
    
    usart_enable_rx(uart);
    
    pdc->PERIPH_RPR = (INT32U)buf;
    pdc->PERIPH_RCR = max_len;
    pdc->PERIPH_RNPR = (INT32U)buf;
    pdc->PERIPH_RNCR = max_len;
    pdc->PERIPH_PTCR = PERIPH_PTCR_RXTEN|PERIPH_PTCR_RXCBEN;
    
    uart_ctl[id].recv_buf_len = max_len;
	
}

void drv_uart_stop_rx(INT8U id)
{
    Pdc * pdc = uart_ctl[id].pdc;
    Usart * uart = uart_ctl[id].obj;

    usart_disable_rx(uart);

    pdc->PERIPH_RCR = 0;
    pdc->PERIPH_PTCR = PERIPH_PTCR_RXTDIS;
}

void __attribute__((weak)) uart_debug_send_complete(void)
{
    
}
void __attribute__((weak)) uart_meter_send_complete(void)
{
    
}
void __attribute__((weak)) uart_gprs_send_complete(void)
{
    
}
void __attribute__((weak)) uart_ir_send_complete(void)
{
    
}
//中断函数
void FLEXCOM3_Handler( void )//debug uart
{
    INT32U status = usart_get_status(USART3);
    if(status & US_CSR_TXEMPTY) //发送完成
    {
        drv_uart_stop_tx(UART_ID_DEBUG);
        
        uart_debug_send_complete();
    }
}

void FLEXCOM2_Handler( void )//meter uart
{
    INT32U status = usart_get_status(USART2);
    if(status & US_CSR_TXEMPTY) //发送完成
    {
        drv_uart_stop_tx(UART_ID_METER);
                
       uart_meter_send_complete();
    }
}

void FLEXCOM4_Handler( void )//gprs uart
{
    INT32U status = usart_get_status(USART4);
    if(status & US_CSR_TXEMPTY) //发送完成
    {
        drv_uart_stop_tx(UART_ID_GPRS);

        uart_gprs_send_complete();
    }
}

