/*
 * drv_esam_spi.c
 *
 * Created: 2018/12/19 星期三 11:45:37
 *  Author: Administrator
 */ 

#include "os.h"
#include "dev_var.h"
#include "drv_esam_spi.h"
#include "tops_fun_date.h"
#include "tops_fun_convert.h"
#include "tops_fun_mem.h"
#include "tops_fun_tools.h"
#include "tops_fun_common.h"
#include "dev_driver.h"
/*
 * note:
 *总线工作方式采用 MODE 3， 时钟极性(CPOL=1)， 串行同步时钟的空闲状态为高电平， 时钟相位(CPHA=1 )， 在串行同步时钟的下降沿转换数据，上升沿采样数据。
 * MISO 引脚的 bit0 为半位宽，信号将于 SCK 引脚上升沿发出的 900ns 后结束，针对软件模拟的 SPI 接口，将存在来不及采样 bit0 数据的问题，建议接口设备在发送 SCK 上升沿前，采样 MISO 引脚信号。
 *芯片上电复位期间， SSN 引脚保持高电平， MISO 引脚保持输入状态， MOSI 引脚输出低电平。
 *接口设备发送完数据 Len2 后，需要加 100μ s 时间间隔，再发送 DATA。
 *接收字节间最长的等待时间为 3s。
 *SCK 推荐速率 2MHz。
 *在 SSN 置高后， T-ESAM 将进入低功耗状态，高电平保持时间至少 10us 以上，才能置低。接口设备将 SSN 再次置低后，需等待 50us 以上时间，才能开始发送数据。
 *发送数据字节之间延时需大于 15us。
*/

#define  ENABLE_ESAM_SPI_CS     do{pio_set_pin_group_low(PORT_SPI_ESAM_CS, PIN_SPI_ESAM_CS);}while(0)
#define  DISABLE_ESAM_SPI_CS    do{pio_set_pin_group_high(PORT_SPI_ESAM_CS, PIN_SPI_ESAM_CS);}while(0)
#define  SPI_ID_ESAM SPI5

#ifdef __OOP_G_METER__
#define __OOP_ESAM__
#endif 

static inline void spi_send(INT8U byte)
{
    drv_spi_write(SPI_ID_ESAM, byte);
}    
static inline INT8U spi_recv(void)
{
    return drv_spi_read(SPI_ID_ESAM);
}

//unsigned char   ESAM_DEVID[8];
unsigned char   ESAM_RAND[8];
//unsigned char   ESAM_CERT_STATE;

void drv_esam_spi_Initialize(void)
{

}
void drv_esam_spi_high_Initialize(void)
{

}
void drv_esam_spi_DeInitialize(void)
{

}

bool drv_esam_spi_WriteArray_lowSpeed(uint8_t* data, uint16_t size)
{
    uint16_t idx;
    uint8_t lrc1;
    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);
    Delay_us(10);
    ENABLE_ESAM_SPI_CS;
    //cs拉低后,至少50us
    Delay_us(80);
    spi_send(0x55);
    Delay_us(15);
    lrc1 = 0;
    for(idx=0;idx<size;idx++)
    {
        spi_send(data[idx]);
        if(idx==5)
        {
            //Delay_us(100);
            Delay_us(10);
        }
        Delay_us(15);
        lrc1 ^= data[idx];
    }
    lrc1 = ~lrc1;
    spi_send(lrc1);
    Delay_us(10);
    DISABLE_ESAM_SPI_CS;
    //cs拉高后，最少10us
    Delay_us(15);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);
    return true;
}
bool drv_esam_spi_WriteArray_highSpeed(uint8_t* data, uint16_t size)
{
    uint16_t idx;
    uint8_t lrc1;
    uint32_t start_tick;
    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);
    Delay_us(10);
    ENABLE_ESAM_SPI_CS;
    //cs拉低后,至少10us
    Delay_us(20);
    spi_send(0x55);
    Delay_us(15);
    tpos_enterCriticalSection();
    lrc1 = 0;
    for(idx=0;idx<size;idx++)
    {
        //数据间隔大于660ns
        start_tick = SYSTICK_VAL;
        spi_send(data[idx]);
        lrc1 ^= data[idx];
        //700ns
        Delay_ns_from(start_tick, 700);
    }
    lrc1 = ~lrc1;
    spi_send(lrc1);
    tpos_leaveCriticalSection();
    //通讯结束，cs拉低时间大于3us
    Delay_us(5);
    DISABLE_ESAM_SPI_CS;
    //cs拉高后，最少10us
    Delay_us(10);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);
    return true;
}
bool drv_esam_spi_WriteArray(uint8_t* data, uint16_t size)
{
    if(gSystemInfo.esam_type==ESAM_OOP)
    {
        return drv_esam_spi_WriteArray_highSpeed(data,size);
    }
    return drv_esam_spi_WriteArray_lowSpeed(data,size);
}
static uint8_t drv_esam_spi_ReadByte_lowSpeed(void)
{
    //间隔1.5us
    Delay_us(1);  
    drv_spi_clear_old_data(SPI_ID_ESAM);
    return spi_recv();
}
static uint8_t drv_esam_spi_ReadByte_highSpeed(void)
{
    uint32_t start_tick;
    //间隔最小660ns
    start_tick = SYSTICK_VAL;
    drv_spi_clear_old_data(SPI_ID_ESAM);
    Delay_ns_from(start_tick, 700);
    return spi_recv();
}
static uint8_t drv_esam_spi_ReadByte(void)
{
    if(gSystemInfo.esam_type==ESAM_OOP)
    {
        return drv_esam_spi_ReadByte_highSpeed();
    }
    return drv_esam_spi_ReadByte_lowSpeed();
}
int16_t drv_esam_spi_ReadArray_lowSpeed(uint8_t* data, uint16_t max_len,uint8_t* headdata,INT16U *err_code)
{
    uint16_t idx;
    int16_t datalen=0;
    uint32_t timer;
    uint8_t status;
    uint8_t lrc2,recvlrc2;
    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);
    Delay_us(10);
    ENABLE_ESAM_SPI_CS;
    //cs拉低后,至少50us
    Delay_us(80);

    *err_code=0;
    timer = OS_TIME;
    pio_set_writeprotect(PORT_SPI_FLASH_MOSI, 0);
    pio_set_output(PORT_SPI_FLASH_MOSI, PIN_SPI_FLASH_MOSI, 0, 0, 0);
    
    do
    {
        status = drv_esam_spi_ReadByte();
        Delay_us(20);
        if(ms_elapsed(timer) > 3000)
        {
            Nop();
            datalen = -1;
            goto EASM_READ_END;
        }
    }while(status != 0x55);
    //读取CLA和INS
    headdata[0] = drv_esam_spi_ReadByte();
    headdata[1] = drv_esam_spi_ReadByte();
    *err_code=(headdata[0]<<8) | headdata[1];
    if((headdata[0] != 0x90) || (headdata[1] != 0x00))
    {
        //rs232_debug_info("\xEF\xEF",2);
        //rs232_debug_info(headdata,2);
        datalen = -1;
        goto EASM_READ_END;
    }
     //计算校验
    lrc2 = headdata[0] ^ headdata[1];
    //读取长度Len1和Len2，Len1代表长度的高字节，Len2代表长度的低字节。
    headdata[2] = drv_esam_spi_ReadByte();
    headdata[3] = drv_esam_spi_ReadByte();
    //rs232_debug_info("\xEE\xEE",2);
    //rs232_debug_info(headdata,4);
    datalen = (headdata[2]<<8) + headdata[3];
    if((datalen > 2048) || (datalen > max_len))  //长度最大不能超过2K 或 长度不能超出接收缓冲区的长度
    {
        //rs232_debug_info("\xEC\xEC",2);
        datalen = -1;
        goto EASM_READ_END;
    }
    //计算校验
    lrc2 ^= headdata[2] ^ headdata[3];
    for(idx=0;idx<datalen;idx++)
    {
        data[idx]=drv_esam_spi_ReadByte();
        lrc2 ^= data[idx];
    }
    lrc2 = ~lrc2;
    recvlrc2 = drv_esam_spi_ReadByte();
    if(lrc2 != recvlrc2)
    {
        datalen = -1;
        goto EASM_READ_END;  
    }

EASM_READ_END:
    Delay_us(10);
    DISABLE_ESAM_SPI_CS;
    //cs拉高后,至少10us
    Delay_us(15);
    
    pio_set_peripheral(PORT_SPI_FLASH_MOSI, PIO_PERIPH_A, PIN_SPI_FLASH_MOSI);
    pio_set_writeprotect(PORT_SPI_FLASH_MOSI, 1);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);

    return datalen;
}
int16_t drv_esam_spi_ReadArray_highSpeed(uint8_t* data, uint16_t max_len,uint8_t* headdata,INT16U *err_code)
{
    uint16_t idx;
    int16_t datalen=0;
    uint32_t timer;
    uint8_t status;
    uint8_t lrc2,recvlrc2;
    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);
    Delay_us(10);
    ENABLE_ESAM_SPI_CS;
    //cs拉低后,至少10us
    Delay_us(20);

    *err_code=0;
    timer = OS_TIME;
    pio_set_writeprotect(PORT_SPI_FLASH_MOSI, 0);
    pio_set_output(PORT_SPI_FLASH_MOSI, PIN_SPI_FLASH_MOSI, 0, 1, 0);
    do
    {
        status = drv_esam_spi_ReadByte();
        //查询间隔15us
        Delay_us(20);
        if(ms_elapsed(timer) > 2000)
        {
            Nop();
            datalen = -1;
            goto EASM_READ_END;
        }
    }while(status != 0x55);
    //读取CLA和INS
    headdata[0] = drv_esam_spi_ReadByte();
    headdata[1] = drv_esam_spi_ReadByte();
    *err_code=(headdata[0]<<8) | headdata[1];
    if((headdata[0] != 0x90) || (headdata[1] != 0x00))
    {
        //rs232_debug_info("\xEF\xEF",2);
        //rs232_debug_info(headdata,2);
        datalen = -1;
        goto EASM_READ_END;
    }
     //计算校验
    lrc2 = headdata[0] ^ headdata[1];
    //读取长度Len1和Len2，Len1代表长度的高字节，Len2代表长度的低字节。
    headdata[2] = drv_esam_spi_ReadByte();
    headdata[3] = drv_esam_spi_ReadByte();
    //rs232_debug_info("\xEE\xEE",2);
    //rs232_debug_info(headdata,4);
    datalen = (headdata[2]<<8) + headdata[3];
    if((datalen > 2048) || (datalen > max_len))  //长度最大不能超过2K 或 长度不能超出接收缓冲区的长度
    {
        //rs232_debug_info("\xEC\xEC",2);
        datalen = -1;
        goto EASM_READ_END;
    }
    tpos_enterCriticalSection();
    //计算校验
    lrc2 ^= headdata[2] ^ headdata[3];
    for(idx=0;idx<datalen;idx++)
    {
        data[idx]=drv_esam_spi_ReadByte();
        lrc2 ^= data[idx];
    }
    lrc2 = ~lrc2;
    recvlrc2 = drv_esam_spi_ReadByte();
    tpos_leaveCriticalSection();
    if(lrc2 != recvlrc2)
    {
        datalen = -1;
        goto EASM_READ_END;  
    }

EASM_READ_END:
    //通讯结束，cs拉低时间大于3us
    Delay_us(5);
    DISABLE_ESAM_SPI_CS;
    //cs拉高后,至少10us
    Delay_us(10);
    pio_set_peripheral(PORT_SPI_FLASH_MOSI, PIO_PERIPH_A, PIN_SPI_FLASH_MOSI);
    pio_set_writeprotect(PORT_SPI_FLASH_MOSI, 1);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);

    return datalen;
}
int16_t drv_esam_spi_ReadArray(uint8_t* data, uint16_t max_len,uint8_t* headdata,INT16U *err_code)
{
    if(gSystemInfo.esam_type==ESAM_OOP)
    {
        return drv_esam_spi_ReadArray_highSpeed(data,max_len,headdata,err_code);
    }
    return drv_esam_spi_ReadArray_lowSpeed(data,max_len,headdata,err_code);
}
/*+++
  功能： 获取芯片序列号8字节

  参数：
         INT8U *buffer  缓冲区
  返回：
         数据长度
  描述：
      发送：800E00020000
      返回：9000+LEN+ESAM序列号

 ---*/
uint16_t esam_get_esam_id(uint8_t *buffer,uint16_t max_len)
{
    uint16_t len,pos;
    uint8_t headdata[4];
    uint16_t err_code;
    
    pos = 0;
    buffer[pos++] = 0x80;
    buffer[pos++] = 0x0E;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x02;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);
    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);

    return len;
}
/*+++
  功能： 获取698esam芯片序列号8字节

  参数：
         INT8U *buffer  缓冲区
  返回：
         数据长度
  描述：
      发送：803600020000
      返回：9000+LEN+ESAM序列号

 ---*/
uint16_t tesam_get_esam_id(uint8_t *buffer,uint16_t max_len)
{
    uint16_t len,pos;
    uint8_t headdata[4];
    uint16_t err_code;
    
    pos = 0;

    buffer[pos++] = 0x80;
    buffer[pos++] = 0x36;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x02;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;


    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);
    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);

    return len;
}
/*+++
  功能： 获取离线计数器4字节

  参数：
         INT8U *buffer  缓冲区
  返回：
         数据长度
  描述：
         发送：800E00030000
         返回：9000+LEN+离线计数器信息
 ---*/
uint16_t esam_get_offline_counter(uint8_t *buffer,uint16_t max_len)
{
    uint16_t  len,pos;
    uint8_t headdata[4];
    uint16_t err_code;
    
    pos = 0;

    buffer[pos++] = 0x80;
    buffer[pos++] = 0x0E;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x03;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);


    return len;
}

/*+++
  功能： 获取芯片状态信息1字节

  参数：
         INT8U *buffer  缓冲区
  返回：
         数据长度
  描述：
         发送：800E00050000
         返回：9000+LEN+芯片状态信息
 ---*/
uint16_t esam_get_chip_state(uint8_t *buffer,uint16_t max_len)
{
    uint16_t  len,pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    buffer[pos++] = 0x80;
    buffer[pos++] = 0x0E;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x05;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);

    return len;
}
/*+++
  功能： 获取698芯片密钥状态，根据对称密钥版本来判断

  参数：
         INT8U *buffer  缓冲区
  返回：
         数据长度
  描述：
         发送：803600040000
         返回：9000+LEN+芯片状态信息
 ---*/
uint16_t tesam_get_chip_state(uint8_t *buffer,uint16_t max_len)
{
    uint16_t  len,pos;
    uint8_t headdata[4];
    uint16_t err_code;
    
    pos = 0;

    buffer[pos++] = 0x80;
    buffer[pos++] = 0x36;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x04;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);

    return len;
}
/*+++
  功能： 获取密钥版本：8字节

  参数：
         INT8U *buffer  缓冲区
  返回：
         数据长度
  描述：
         发送：800E00060000
         返回：9000+LEN+密钥版本信息
 ---*/
uint16_t esam_get_key_version(uint8_t *buffer,uint16_t max_len)
{
    uint16_t  len,pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    buffer[pos++] = 0x80;
    buffer[pos++] = 0x0E;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x06;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);

    return len;
}

/*+++
  功能： 获取证书序列号

  参数：
         INT8U *buffer  缓冲区
  返回：
         数据长度
  描述：
      发送：8032 + P1 + 02 +0000
      返回：9000+LEN+证书序列号
      P1：根据T-ESAM当前状态设置；
          00：测试证书；
          01：正式证书；
      证书序列号：16字节
 ---*/
uint16_t esam_get_cert_id(uint8_t *buffer,uint16_t max_len)
{
    uint16_t  len,pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    buffer[pos++] = 0x80;
    buffer[pos++] = 0x32;
    buffer[pos++] = (gSystemInfo.ESAM_CERT_STATE > 1) ? 1 : gSystemInfo.ESAM_CERT_STATE;
    buffer[pos++] = 0x02;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);


    return len;
}

/*+++
  功能： 获取终端公钥

  参数：
         INT8U *buffer;  终端公钥
  返回：
         公钥长度为64字节
  描述：
        发送：802C00010000
        返回：9000+LEN+终端公钥
        终端生产非对称密钥对，返回公钥长度为64字节
---*/
uint16_t esam_get_public_key(uint8_t *buffer,uint16_t max_len)
{
    uint16_t  len,pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    buffer[pos++] = 0x80;
    buffer[pos++] = 0x2C;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x01;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);

    return len;
}

/*+++
  功能： 会话初始化/恢复

  参数：
         INT8U *data;    报文1
         INT8U *buffer;  报文2
  返回：
         报文2的长度
  描述：
      发送：84100000+LC+报文1+Time
      返回：9000+LEN+报文2
      发送报文1
会话ID为0，报文1：版本号(1字节)+会话 ID(1字节，00)+主站证书 (Ln)+ Eks1(R1) (16字节)+MAC1(4字节)+数字签名S1(64字节)
      Time取值：20130117111915 ~ 20500101010101，必须在此范围内；

 ---*/
extern uint8_t datetime[];
uint16_t esam_session_init(uint8_t *data,uint8_t *buffer,uint16_t max_len)
{
    uint16_t  len,pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    len = data[0];
    len += data[1]<<8;
    len += 7;

    if((len + 6) > max_len) return 0;

    buffer[pos++] = 0x84;
    buffer[pos++] = 0x10;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = len>>8;     //LC高字节
    buffer[pos++] = len;        //LC低字节
    mem_cpy(buffer+pos,data+2,len-7);
    pos += len-7;
    buffer[pos++] = 0x20;       //time
    
    buffer[pos++] = byte2BCD(datetime[YEAR]);
    buffer[pos++] = byte2BCD(datetime[MONTH]);
    buffer[pos++] = byte2BCD(datetime[DAY]);
    buffer[pos++] = byte2BCD(datetime[HOUR]);
    buffer[pos++] = byte2BCD(datetime[MINUTE]);
    buffer[pos++] = byte2BCD(datetime[SECOND]);

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);


    return len;
}

/*+++
  功能： 会话密钥协商

  参数：
         INT8U *data;    长度(2字节) + 报文3
         INT8U *buffer;  报文4
  返回：
         报文4的长度
  描述：
        发送：84120000+LC+报文3
        返回：9000+LEN+报文4
        LC：报文3的长度， LC为2字节；
        报文3如下：
          密文M1(113字节)+
          Eks2(R2)（16字节）+
          主站证书序列号（16字节）+
          有效状态(1字节）+
          签名S3（64字节）+
          MAC2(4字节)+
          签名S4(64字节)
        报文4如下：
          R3(16) + MAC3(4)
---*/
uint16_t esam_session_key_consult(uint8_t *data,uint8_t *buffer,uint16_t max_len)
{
    uint16_t  len,pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    len = data[0];
    len += data[1]<<8;

    if((len + 6) > max_len) return 0;

    buffer[pos++] = 0x84;
    buffer[pos++] = 0x12;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = len>>8;     //LC高字节
    buffer[pos++] = len;        //LC低字节
    mem_cpy(buffer+pos,data+2,len);
    pos += len;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);

    return len;
}



/*+++
  功能： 对称密钥更新

  参数：
         INT8U *data;    证书更新密文
         INT8U *buffer;  缓冲区
  返回：
         报文4的长度
  描述：
        发送：84240100+LC+密钥密文数据
        返回：9000+0000
---*/
bool esam_symmetric_key_update(uint8_t *data,uint8_t *buffer,uint16_t max_len)
{
    int16_t len;
    uint16_t pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    len = ((uint16_t)data[0]) * 32 + 1;

    if((len + 6) > max_len) return 0;

    buffer[pos++] = 0x84;
    buffer[pos++] = 0x24;
    buffer[pos++] = 0x01;
    buffer[pos++] = 0x00;
    buffer[pos++] = len>>8;
    buffer[pos++] = len;
    mem_cpy(buffer+pos,data,len);
    pos += len;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);
    if(len==-1)
    {
        return false;
    }

    if((headdata[0] == 0x90) && (headdata[1] == 0x00)) return true;
    else return false;
}

/*+++
  功能： 证书更新

  参数：
         INT8U *data;    证书更新密文
         INT8U *buffer;  缓冲区
  返回：
         报文4的长度
  描述：
        发送：8430+证书更新密文
        返回：9000+0000
---*/
bool esam_ca_certificate_update(uint8_t *data,uint8_t *buffer,uint16_t max_len)
{
    int16_t len;
    uint16_t pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    len = data[0];
    len += data[1]<<8;

    if((len + 2) > max_len) return 0;

    buffer[pos++] = 0x84;
    buffer[pos++] = 0x30;
    mem_cpy(buffer+pos,data+2,len);
    pos += len;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);

    if(len==-1)
    {
        return false;
    }
    if((headdata[0] == 0x90) && (headdata[1] == 0x00)) return true;
    else return false;
}

/*+++
  功能： 发送内部认证

  参数：
         INT8U *data;    主站生成随机数R4，R4为16字节
         INT8U *buffer;  ER4，ER4：R4密文，16字节
  返回：
         报文4的长度
  描述：
        发送：842600000010+R4
        返回：9000+LEN+ER4
---*/
uint16_t esam_internal_auth(uint8_t *data,uint8_t *buffer,uint16_t max_len)
{
    uint16_t len,pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    buffer[pos++] = 0x84;
    buffer[pos++] = 0x26;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x10;
    mem_cpy(buffer+pos,data,16);
    pos += 16;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);


    return len;
}

/*+++
  功能： 发送外部认证

  参数：
         INT8U *data;    主站生成随机数R5密文ER5，下发外部认证命令 16字节
         INT8U *buffer;  缓冲区
  返回：
         报文4的长度
  描述：
        发送：842800000010+ER5
        返回：9000+0000
---*/
bool esam_external_auth(uint8_t *data,uint8_t *buffer,uint16_t max_len)
{
    int16_t len;
    uint16_t pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    buffer[pos++] = 0x84;
    buffer[pos++] = 0x28;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x10;
    mem_cpy(buffer+pos,data,16);
    pos += 16;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);

    if(len==-1)
    {
        return false;
    }
    if((headdata[0] == 0x90) && (headdata[1] == 0x00)) return true;
    else return false;
}

/*+++
  功能： 取随机数

  参数：
         INT8U len    随机数长度
         INT8U *buffer;  随机数
  返回：
         随机数长度
  描述：
        发送：801A10000000
        返回：9000+LEN+随机数
---*/
uint16_t esam_get_random_num(uint8_t randonlen,uint8_t *buffer,uint16_t max_len)
{
    uint16_t len,pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    buffer[pos++] = 0x80;
    buffer[pos++] = 0x1A;
    buffer[pos++] = randonlen;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);


    return len;
}

/*+++
  功能： 证书状态切换

  参数：
         INT8U *data;    主站计算R6密文和MAC6，发送证书状态切换命令
         INT8U *buffer;  缓冲区
  返回：
         报文4的长度
  描述：
        发送：842A+P1+000014+ER6+MAC6
        返回：9000+0000
        ER6：R6密文，16字节
        P1：主站下发切换状态
            00从测试证书切换到正式证书
            01从正式证书切换到测试证书
        MAC6：4字节
---*/
bool esam_cert_state_change(uint8_t *data,uint8_t *buffer,uint16_t max_len)
{
    int16_t len;
    uint16_t pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    buffer[pos++] = 0x84;
    buffer[pos++] = 0x2A;
    buffer[pos++] = data[0];
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x14;
    mem_cpy(buffer+pos,data+1,20);
    pos += 20;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);

    if(len==-1)
    {
        return false;
    }
    if((headdata[0] == 0x90) && (headdata[1] == 0x00)) return true;
    else return false;
}

/*+++
  功能： 置离线计数器

  参数：
         INT8U *data;    主站发送重置离线计数器密文、MAC
         INT8U *buffer;  缓冲区
  返回：
         报文4的长度
  描述：
        发送：842000000014+离线计数器密文+MAC
        返回：9000+0000
---*/
bool esam_set_offline_counter(uint8_t *data,uint8_t *buffer,uint16_t max_len)
{
    int16_t len;
    uint16_t pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    buffer[pos++] = 0x84;
    buffer[pos++] = 0x20;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x14;
    mem_cpy(buffer+pos,data,20);
    pos += 20;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);

    if(len==0)
    {
        return false;
    }
    if((headdata[0] == 0x90) && (headdata[1] == 0x00)) return true;
    else return false;
}

/*+++
  功能： 转加密授权

  参数：
         INT8U *data;    主站发送转加密授权数据,28字节随机数+4字节MAC
         INT8U *buffer;  缓冲区
  返回：
         报文4的长度
  描述：
        发送：801600000020+转加密数据
        返回：9000+0000
---*/
bool esam_change_data_authorize(uint8_t *data,uint8_t *buffer,uint16_t max_len)
{
    int16_t len;
    uint16_t pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    buffer[pos++] = 0x80;
    buffer[pos++] = 0x16;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x20;
    mem_cpy(buffer+pos,data,32);
    pos += 32;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);

    if(len==-1)
    {
        return false;
    }

    if((headdata[0] == 0x90) && (headdata[1] == 0x00)) return true;
    else return false;
}

/*+++
  功能： MAC验证

  参数：
         INT8U kid  密钥ID
         INT8U *data;    主站发送转加密授权数据,28字节随机数+4字节MAC
         INT8U *buffer;  缓冲区
  返回：
         报文4的长度
  描述：
        发送：80FA+03+KeyID+0000
        返回：9000+0000
              KeyID：终端T-ESAM加密密钥标识，具体定义见KeyID分配关系表。
        发送：80FA+0200+LC+Data
        返回：9000+LEN+MAC1
              LC：Data的长度（2字节）
              终端对比MAC与MAC1，如果校验失败，则取随机数
        发送：801A+08+000000
        返回：9000+LEN+Random
              Random：8字节随机数
              终端上传确认帧或否认帧，否认帧中含有随机数Random
---*/
bool esam_verify_mac2(uint8_t kid,uint8_t *data,uint16_t datalen,uint8_t *buffer,uint16_t max_len,uint8_t mac[4])
{
    int16_t len;
    uint16_t pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    buffer[pos++] = 0x80;
    buffer[pos++] = 0xFA;
    buffer[pos++] = 0x03;
    buffer[pos++] = kid;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);
    if(len==-1)
    {
        return false;
    }

    if((headdata[0] != 0x90) || (headdata[1] != 0x00)) return false;

    pos = 0;
    buffer[pos++] = 0x80;
    buffer[pos++] = 0xFA;
    buffer[pos++] = 0x02;
    buffer[pos++] = 0x00;
    buffer[pos++] = datalen>>8;
    buffer[pos++] = datalen;
    mem_cpy(buffer+pos,data,datalen);
    pos += datalen;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);
    
    if(len==-1)
    {
        return false;
    }

    if(len == 4)
    {
        if((mac[0] == buffer[0]) && (mac[1] == buffer[1]) && (mac[2] == buffer[2]) && (mac[3] == buffer[3])) return true;
    }

    if(8 == esam_get_random_num(8,buffer,max_len)) mem_cpy(ESAM_RAND,buffer,8);
    else mem_set(ESAM_RAND,8,0xFF);

    return false;
}

/*+++
  功能： 组广播

  参数：
         INT8U kid1  密钥ID
         INT8U kid2  密钥ID
         INT8U *data;    主站发送转加密授权数据,28字节随机数+4字节MAC
         INT8U *buffer;  缓冲区
  返回：
         报文4的长度
  描述：
        发送：8022+KID1+KID2+LC+(00+TotalTN+130202224622+Data)+ 20130202224622+ 05
        返回：9000+LEN+明文
            KID1(1字节)：MAC加密密钥的KID；
            KID2(1字节)：MAC初始值的KID；
                         具体参见KID分配关系表
            LC：后续所有数据的长度2字节；
            TotalTN(2)：组地址；
            Data(Var)：明文+MAC
        终端返回确认帧或否认帧
---*/
bool esam_group_broadcast(uint8_t kid1,uint8_t kid2,uint16_t group_id,uint8_t *data,uint16_t datalen,uint8_t *buffer,uint16_t max_len,uint8_t mac[4])
{
    int16_t len;
    uint16_t pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    buffer[pos++] = 0x80;
    buffer[pos++] = 0x22;
    buffer[pos++] = kid1;
    buffer[pos++] = kid2;
    buffer[pos++] = 0x00;  //LC
    buffer[pos++] = 0x00;  //LC
    buffer[pos++] = 0x00;
    buffer[pos++] = group_id>>8;
    buffer[pos++] = group_id;
    buffer[pos++] = 0x13;
    buffer[pos++] = 0x02;
    buffer[pos++] = 0x02;
    buffer[pos++] = 0x22;
    buffer[pos++] = 0x46;
    buffer[pos++] = 0x22;
    mem_cpy(buffer+pos,data,datalen);
    pos += datalen;
    mem_cpy(buffer+pos,mac,4);
    pos += 4;
    buffer[pos++] = 0x20;
    buffer[pos++] = 0x13;
    buffer[pos++] = 0x02;
    buffer[pos++] = 0x02;
    buffer[pos++] = 0x22;
    buffer[pos++] = 0x46;
    buffer[pos++] = 0x22;
    buffer[pos++] = 0x05;
    //LC
    buffer[4] = (pos-6)>>8;
    buffer[5] = pos-6;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);
    if(len==-1)
    {
        return false;
    }

    if((headdata[0] != 0x90) || (headdata[1] != 0x00)) return false;

    return true;
}

/*+++
  功能： 获得电表身份认证密文

  参数：
         INT8U *data;    主站发送转加密授权数据,28字节随机数+4字节MAC
         INT8U *buffer;  缓冲区
  返回：
         报文4的长度
  描述：
        发送：841C00P2+LC+身份认证任务中的电表密钥密文+分散因子+R1
        返回：9000+LEN+ER0
              LC：后续数据的总长度，2字节
              电表密钥密文：32字节
              分散因子：当下发任务命令数据中的表号为实际表号时， P2设置为01，分散因子为0000+6字节表号；
                        当下发任务命令数据中的表号为8字节AA时， P2设置为02，分散因子为任务命令中的任务数据（2字节）+表号+0000+表号
              ER0：16字节密文
---*/
uint16_t esam_identity_authentication(uint8_t P2,uint8_t *data,uint8_t datalen,uint8_t *buffer,uint16_t max_len)
{
    uint16_t len,pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    buffer[pos++] = 0x84;
    buffer[pos++] = 0x1C;
    buffer[pos++] = 0x00;
    buffer[pos++] = P2;
    buffer[pos++] = 0;//datalen>>8;由于类型本事就为INT8U
    buffer[pos++] = datalen;
    mem_cpy(buffer+pos,data,datalen);
    pos += datalen;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);


    return len;
}

/*+++
  功能： 获得电表对时密文

  参数：
         INT8U *data;    YYMMDDWWhhmmss+任务数据（含数据标识）+对时任务中电表密钥密文+ R2
         INT8U *buffer;  缓冲区
  返回：
         报文4的长度
  描述：
        发送：8418+任务格式（0404）+005A+任务类型(01)+05+0032+0005+07+YYMMDDWWhhmmss+任务数据（含数据标识）+对时任务中电表密钥密文+ R2
        返回：9000+LEN+数据标识（4字节）+电表对时任务密文+4字节MAC
              YYMMDDWWhhmmss为终端当前时间。
              数据标识（4字节）为DL/T645-2007电能表通信协议中的数据标识DI3、DI2、DI1、DI0。
---*/
uint16_t esam_set_time(uint8_t cur_datatime[7],uint8_t task_data[40],uint8_t meter_er[32],uint8_t meter_rdn[4],uint8_t *buffer,uint16_t max_len)
{
    uint16_t len,pos;
    uint8_t headdata[4];
    uint16_t err_code;

    pos = 0;

    buffer[pos++] = 0x84;
    buffer[pos++] = 0x18;
    buffer[pos++] = 0x04;
    buffer[pos++] = 0x04;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x5A;
    buffer[pos++] = 0x01;  //任务类型
    buffer[pos++] = 0x05;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x32;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x05;
    buffer[pos++] = 0x07;
    mem_cpy(buffer+pos,cur_datatime,7);
    pos += 7;
    mem_cpy(buffer+pos,task_data,40);
    pos += 40;
    mem_cpy(buffer+pos,meter_er,32);
    pos += 32;
    mem_cpy(buffer+pos,meter_rdn,4);
    pos += 4;

    //获取信号量
    tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    tpos_mutexFree(&SIGNAL_EASM);

    return len;
}

/*+++
  功能： 上电时读取esam信息

  参数：
         INT8U *data;    主站发送转加密授权数据,28字节随机数+4字节MAC
         INT8U *buffer;  缓冲区
  返回：
         报文4的长度
  描述：
        发送：801600000020+转加密数据
        返回：9000+0000
---*/
#ifndef  __OOP_ESAM__
void esam_get_info(void)
{
    uint8_t buffer[20];
    uint8_t len;

    mem_set(gSystemInfo.ESAM_DEVID,8,0xFF);
    gSystemInfo.ESAM_CERT_STATE = 0xFF;
    len = esam_get_esam_id(buffer,20);
    if(len == 8) mem_cpy_reverse(gSystemInfo.ESAM_DEVID,buffer,8);

    len = esam_get_chip_state(buffer,20);
    if(len == 1) gSystemInfo.ESAM_CERT_STATE = buffer[0];
}
#endif




