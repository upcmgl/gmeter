#ifndef __DEV_DRIVER_H__
#define __DEV_DRIVER_H__

#define MAX_RX_BUFFER_LENGTH   500
typedef struct{
    uint8_t *send_ptr;
    uint16_t send_len;
    uint16_t recv_pos;
    uint16_t read_pos;
    uint8_t  recv_buf[MAX_RX_BUFFER_LENGTH];
}tagUartObj;
extern volatile tagUartObj  UartObjMeter,UartObjIr;
#define MAX_RX_BUFFER_LENGTHGPRS    2200 //注意这里原来是500，现在改成1500
typedef struct{
    uint8_t *send_ptr;
    uint16_t send_len;
    uint16_t recv_pos;
    uint16_t read_pos;
    uint8_t  recv_buf[MAX_RX_BUFFER_LENGTHGPRS];
}tagUartObjGPRS;
extern volatile tagUartObjGPRS UartObjGprs,UartObjDebug;


void drv_pin_gprs_rst(uint8_t pin_level);
void drv_pin_gprs_onoff(uint8_t pin_level);
void drv_pin_gprs_heat_en(uint8_t en);

void drv_flash_spi_init (void);
void drv_flash_spi_write_byte(uint8_t data);
void drv_flash_spi_write_array(uint8_t *data,uint16_t len);
uint8_t drv_flash_spi_read_byte(void);
void drv_flash_spi_read_array(uint8_t *data,uint16_t len);

void drv_spi_write(Spi *spi, INT8U byte);
INT8U drv_spi_read(Spi *spi);
void drv_spi_clear_old_data(Spi *spi);

void TMR2_Initialize (void);

void gprs_uart_init (INT32U baudrate);
int16_t gprs_uart_read_byte(void);
uint8_t gprs_uart_send_buf(uint8_t *data,uint16_t len);
uint8_t gprs_uart_is_idle(void);

void meter_uart_init (INT32U baudrate);
int16_t meter_uart_read_byte(void);
uint8_t meter_uart_send_buf(uint8_t *data,uint16_t len);
uint8_t meter_uart_is_idle(void);


void system_debug_info(char* info);
void system_debug_data(uint8_t* data,uint16_t data_len);
void debug_uart_init (INT32U baudrate);
int16_t debug_uart_read_byte(void);
uint8_t debug_uart_send_buf(uint8_t *data,uint16_t len);
uint8_t debug_uart_is_idle(void);

int16_t ir_read_byte(void);
uint8_t ir_send_buf(uint8_t *data,uint16_t len);
void ir_send_complete( void );
uint8_t ir_is_idle(void);

uint8_t dev_power_lost(void);
void bat_charge_service(void);

//LEDs
void LED_Task( void );
void LED_local_on(INT8U on);
void LED_local_toggle(void);
void LED_green_on(INT8U on);
void LED_green_toggle(void);
void LED_comm_toggle(void);
void LED_green_toggle_start(void);
void LED_red_on(INT8U on);
void LED_red_toggle(void);

void ertu_month_bytes_add(INT32U bytes);















#endif