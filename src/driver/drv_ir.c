/*
 * drv_ir.c
 *
 * Created: 2018/12/17 星期一 13:44:56
 *  Author: Administrator
 */ 
#include "os.h"
#include "dev_driver.h"

extern volatile tagUartObj  UartObjIr;
static INT8U bit_state;
void tc_ir_callback()
{
    static INT8U bit_pos, parity;
    
    if(UartObjIr.send_len == 0)
    {
        drv_ir_lanch_tx(0);
        ir_send_complete();
        return;
    }
    switch(bit_state)
    {
        case 0:
            tc_ir_tx(0);
            parity = 0;//EVEN prity
            bit_pos = 0;
            bit_state ++;
        break;
        case 1:
            if(UartObjIr.send_ptr[0] & (1<<bit_pos))
            {
                tc_ir_tx(1);
                parity++;
            }
            else
            {
                tc_ir_tx(0);
            }
            bit_pos++;
            
            if(bit_pos >= 8)
            {
                bit_state++;
            }
        break;
        case 2:
            if(parity&0x01)
            {
                tc_ir_tx(1);
            }
            else
            {
                tc_ir_tx(0);
            }
            bit_state++;
        break;
        case 3:
            tc_ir_tx(1);
            UartObjIr.send_ptr++;
            UartObjIr.send_len--;
            bit_state=0;
        break;  
        default:
        UartObjIr.send_len = 0;
        bit_state = 0;
        break;
    }
}

