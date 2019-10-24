#include "main_include.h"
#include "dev_driver.h"
#include "drv_gpio.h"

#define CLEAR_BIT(VAL,BIT) (VAL)&=~(BIT)

#define ENABLE_FLASH_SPI      do{tpos_mutexPend(&SIGNAL_EASM); pio_set_pin_group_low(PORT_SPI_FLASH_CS, PIN_SPI_FLASH_CS);}while(0)
#define DISABLE_FLASH_SPI     do{pio_set_pin_group_high(PORT_SPI_FLASH_CS, PIN_SPI_FLASH_CS); tpos_mutexFree(&SIGNAL_EASM);}while(0)

#define ENABLE_FLASH_WP       {pio_set_pin_group_low(PORT_SPI_FLASH_WP, PIN_SPI_FLASH_WP);}
#define DISABLE_FLASH_WP      {pio_set_pin_group_high(PORT_SPI_FLASH_WP, PIN_SPI_FLASH_WP);}

#define FLASH_CMD_READ    0x03
#define FLASH_CMD_WREN    0x06
#define FLASH_CMD_WRDI    0x04
#define FLASH_CMD_RDSR    0x05
#define FLASH_CMD_RDCR    0x15
#define FLASH_CMD_WRSR    0x01
#define FLASH_CMD_SE      0x20
//#define FLASH_CMD_BE      0xD8
#define FLASH_CMD_PP      0x02
#define FLASH_CMD_RDID    0x9F

//for W25...
#define FLASH_CMD_RDSR1    0x35
#define FLASH_CMD_WRSR1    0x31
#define FLASH_CMD_RDSR2    0x15
#define FLASH_CMD_WRSR2    0x11

uint8_t  nor_flash_id[3];
INT32U FLASH_SECTOR_COUNT =0;
INT8U FLASH_TYPE= FLASH_TYPE_MX;


void _flash_wait_idle(void);
void _flash_enable_write(void);
void _flash_disable_write(void);

void drv_flash_init(void)
{
    
    DISABLE_FLASH_SPI;
    ENABLE_FLASH_WP;
    
    drv_flash_spi_init();
    
}
void drv_flash_readID(uint8_t id[3])
{
    ENABLE_FLASH_SPI;
    drv_flash_spi_write_byte(FLASH_CMD_RDID);
    drv_flash_spi_read_array(id,3);
    DISABLE_FLASH_SPI;
}
void drv_flash_write_enable(void)
{
    ENABLE_FLASH_SPI;
    drv_flash_spi_write_byte(FLASH_CMD_WREN);
    DISABLE_FLASH_SPI;
}
void drv_flash_write_disable(void)
{
    ENABLE_FLASH_SPI;
    drv_flash_spi_write_byte(FLASH_CMD_WRDI);
    DISABLE_FLASH_SPI;
}
/*status ¼Ä´æÆ÷
 * bit0 WIP   program/erase/write status register in process   1:busy 0:idle
 * bit1 WEL   Write Enable Latch  set to ¡°1¡± by the WREN instruction  1:enable 0:disable
 * bit2~bit5  BP  block protect default 0
 * bit6       QE  default 0
 * bit7       SRWD together with WP
 */
uint8_t drv_flash_read_status_reg(void)
{
    uint8_t status;
    ENABLE_FLASH_SPI;
    drv_flash_spi_write_byte(FLASH_CMD_RDSR);
    status = drv_flash_spi_read_byte();
    DISABLE_FLASH_SPI;
    return status;
}

uint8_t drv_flash_read_reg(INT8U reg_read_cmd)
{
    uint8_t status;
    ENABLE_FLASH_SPI;
    drv_flash_spi_write_byte(reg_read_cmd);
    status = drv_flash_spi_read_byte();
    DISABLE_FLASH_SPI;
    return status; 
}
// 
uint8_t drv_flash_write_reg(INT8U reg_write_cmd, INT8U val)
{
    _flash_enable_write();// SET WEL=1
    
    ENABLE_FLASH_SPI;
    drv_flash_spi_write_byte(reg_write_cmd);
    drv_flash_spi_write_byte(val);
    DISABLE_FLASH_SPI;
    
    _flash_wait_idle();// wait busy=0                
    _flash_disable_write();//SET WEL=0                          
}
//uint8_t drv_flash_read_cfg_reg(void)
//{
//    uint8_t cfg;
//    ENABLE_FLASH_SPI;
//    drv_flash_spi_write_byte(FLASH_CMD_RDCR);
//    cfg = drv_flash_spi_read_byte();
//    DISABLE_FLASH_SPI;
//    return cfg;
//}
void drv_flash_write_status_reg(uint8_t status[2])
{
    ENABLE_FLASH_SPI;
    drv_flash_spi_write_byte(FLASH_CMD_WRSR);
    drv_flash_spi_write_array(status,2);
    DISABLE_FLASH_SPI;
}
void drv_flash_addr_data_read(uint32_t addr,uint8_t cmd,uint8_t *data,uint16_t len)
{
    ENABLE_FLASH_SPI;
    drv_flash_spi_write_byte(cmd);
    drv_flash_spi_write_byte(addr>>16);
    drv_flash_spi_write_byte(addr>>8);
    drv_flash_spi_write_byte(addr&0xFF);
    drv_flash_spi_read_array(data,len);
    DISABLE_FLASH_SPI;
}
uint8_t drv_flash_addr_data_read_check(uint32_t addr,uint8_t cmd,uint8_t *data,uint16_t len)
{
    uint32_t idx;
    
    ENABLE_FLASH_SPI;
    drv_flash_spi_write_byte(cmd);
    drv_flash_spi_write_byte(addr>>16);
    drv_flash_spi_write_byte(addr>>8);
    drv_flash_spi_write_byte(addr&0xFF);
    for(idx=0;idx<len;idx++)
    {
        if(data[idx] != drv_flash_spi_read_byte())
        {
            DISABLE_FLASH_SPI;
            return 0;
        }
    }
    DISABLE_FLASH_SPI;
    return 1;
}
void drv_flash_addr_data_write(uint32_t addr,uint8_t cmd,uint8_t *data,uint16_t len)
{
    ENABLE_FLASH_SPI;
    drv_flash_spi_write_byte(cmd);
    drv_flash_spi_write_byte(addr>>16);
    drv_flash_spi_write_byte(addr>>8);
    drv_flash_spi_write_byte(addr&0xFF);
    drv_flash_spi_write_array(data,len);
    DISABLE_FLASH_SPI;
}


void _flash_wait_idle(void)
{
    uint8_t status;
    
    do{
        status = drv_flash_read_status_reg();
    }while(status & 0x01);
}
void _flash_enable_write(void)
{
    uint8_t status;
   
    _flash_wait_idle();
    status = drv_flash_read_status_reg();
    while((status & 0x02)==0)
    {
        drv_flash_write_enable();
        status = drv_flash_read_status_reg();
    }
}
void _flash_disable_write(void)
{
    uint8_t status;
   
    _flash_wait_idle();
    status = drv_flash_read_status_reg();
    while(status & 0x02) // wel -> 0
    {
        drv_flash_write_disable();
        status = drv_flash_read_status_reg();
    }
}



uint8_t nor_flash_detect(void)
{
    uint8_t id[3];
    uint8_t try_count;
    uint8_t resault = 0;
    uint8_t status[3];
    
    tpos_mutexPend(&SIGNAL_FLASH);
    while(1)
    {
        drv_flash_readID(id);
        if((id[0] == 0xC2)&&(id[1]==0x20)&&(id[2]==0x16))
        {
            nor_flash_id[0] = id[0];
            nor_flash_id[1] = id[1];
            nor_flash_id[2] = id[2];
            FLASH_SECTOR_COUNT = 1024;
            FLASH_TYPE = FLASH_TYPE_MX;
            resault = 1;
        }
        if((id[0] == 0xEF)&&((id[1]==0x40) || (id[1]==0x70))&&(id[2]==0x17))
        {
            nor_flash_id[0] = id[0];
            nor_flash_id[1] = id[1];
            nor_flash_id[2] = id[2];
            FLASH_SECTOR_COUNT = 2048;
            FLASH_TYPE = FLASH_TYPE_W25;
            resault = 1;
        }
        if(try_count++ > 50)
        {
            break;
        }
    }
    if(resault)
    {
        if(FLASH_TYPE == FLASH_TYPE_MX)
        {
        //µÈ´ýflash¿ÕÏÐ£¬ÉèÖÃstatus¼Ä´æÆ÷
        _flash_wait_idle();
        status[0] = drv_flash_read_status_reg();
        while(status[0] & 0x7C)
        {
            DISABLE_FLASH_WP;
            //±£Ö¤Ã»ÓÐ QE ¡¢ BP
            _flash_enable_write();
            status[0] = drv_flash_read_status_reg();
            status[0] &= ~0x7C;
            status[1] = 0;
            drv_flash_write_status_reg(status);
            
            _flash_wait_idle();
            _flash_disable_write();
            status[0] = drv_flash_read_status_reg();
            ENABLE_FLASH_WP;
        }
    }
        if(FLASH_TYPE == FLASH_TYPE_W25)
        {
            _flash_wait_idle();
            status[0] = drv_flash_read_reg(FLASH_CMD_RDSR);
            status[1] = drv_flash_read_reg(FLASH_CMD_RDSR1);
            status[2] = drv_flash_read_reg(FLASH_CMD_RDSR2);
            while((status[0] & 0x7c) || (status[1] & 0x40) || (status[2] & 0x4))
            {
                DISABLE_FLASH_WP;
                
                CLEAR_BIT(status[1], 0x1);//clear SRL 
                CLEAR_BIT(status[0], 0x80);//clear SRP
                CLEAR_BIT(status[1], 0x2);//clear QE
                drv_flash_write_reg(FLASH_CMD_WRSR, status[0]);
                drv_flash_write_reg(FLASH_CMD_WRSR1, status[1]);
                
                
                CLEAR_BIT(status[0], 0x80| 0x7c);
                CLEAR_BIT(status[1], 0x03| 0x42);
                CLEAR_BIT(status[2], 0x4);
                drv_flash_write_reg(FLASH_CMD_WRSR, status[0]);
                drv_flash_write_reg(FLASH_CMD_WRSR1, status[1]);
                drv_flash_write_reg(FLASH_CMD_WRSR2, status[2]);
                
                ENABLE_FLASH_WP;
                
                status[0] = drv_flash_read_reg(FLASH_CMD_RDSR);
                status[1] = drv_flash_read_reg(FLASH_CMD_RDSR1);
                status[2] = drv_flash_read_reg(FLASH_CMD_RDSR2);  
            } 
        }
    }

    tpos_mutexFree(&SIGNAL_FLASH);
    return resault;
}
uint16_t nor_flash_read_data(uint16_t sector,uint16_t offset,uint8_t *data,uint16_t len)
{
    uint32_t addr;
    
    if(sector >= FLASH_SECTOR_COUNT)
    {
        return 0;
    }
    if(offset > FLASH_SECTOR_SIZE)
    {
        return 0;
    }
    
    tpos_mutexPend(&SIGNAL_FLASH);
    
    addr = sector;
    addr *= FLASH_SECTOR_SIZE;
    addr += offset;
    
FLASH_READ_TRY_AGAIN:
    _flash_wait_idle();
    drv_flash_addr_data_read(addr,FLASH_CMD_READ,data,len);
    
    if(drv_flash_addr_data_read_check(addr,FLASH_CMD_READ,data,len) == 0)
    {
        goto FLASH_READ_TRY_AGAIN;
    }
    tpos_mutexFree(&SIGNAL_FLASH);
    return len;
}
uint16_t nor_flash_erase_page(uint16_t sector)
{
    uint8_t status;
    uint32_t addr;
    
    if(sector >= FLASH_SECTOR_COUNT)
    {
        return 0;
    }
    
    tpos_mutexPend(&SIGNAL_FLASH);
    
    addr = sector;
    addr *= FLASH_SECTOR_SIZE;

    DISABLE_FLASH_WP;
    _flash_wait_idle();
    _flash_enable_write();
    status = drv_flash_read_status_reg();
    drv_flash_addr_data_write(addr,FLASH_CMD_SE,NULL,0);
    _flash_wait_idle();
    _flash_disable_write();
    _flash_wait_idle();
    ENABLE_FLASH_WP;
    tpos_mutexFree(&SIGNAL_FLASH);
    return 1;
}
uint16_t nor_flash_write_data(uint16_t sector,uint16_t offset,uint8_t *data,uint16_t len)
{
    uint16_t write_len;
    uint32_t addr;
    uint16_t all_len=len;
    
    if(sector >= FLASH_SECTOR_COUNT)
    {
        return 0;
    }
    if(offset > FLASH_SECTOR_SIZE)
    {
        return 0;
    }
    
    tpos_mutexPend(&SIGNAL_FLASH);
    
    addr = sector;
    addr *= FLASH_SECTOR_SIZE;
    addr += offset;
    
    DISABLE_FLASH_WP;
    do
    { 
        write_len = 0x100 - (addr & 0x000000FF);
        if(write_len > len ) write_len = len;

        _flash_wait_idle();
        _flash_enable_write();
        drv_flash_addr_data_write(addr,FLASH_CMD_PP,data,write_len);
        data += write_len;
        addr += write_len;
        len -= write_len;
    }while(len > 0);
    _flash_wait_idle();
    _flash_disable_write();
    _flash_wait_idle();
    ENABLE_FLASH_WP;
    tpos_mutexFree(&SIGNAL_FLASH);
    return (all_len-len);
}