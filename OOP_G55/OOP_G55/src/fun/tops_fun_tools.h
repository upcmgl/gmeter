#ifndef __TOPS_FUN_TOOLS_H__
#define __TOPS_FUN_TOOLS_H__

#include "dev_file.h"
//升级类别定义，目前按新376.1文件传输的文件标识定义
#define FILEFLAG_CLEAR            0x00    //清除传输文件
#define FILEFLAG_DEV_PROG         0x01    //终端升级文件

#define FILEFLAG_SGP_GMTER_G55       0xFA    //单相G表G55方案
#define FILEFLAG_TGP_GMTER_G55       0xFB    //三相G表G55方案

#define FILEFLAG_TYPE   FILEFLAG_TGP_GMTER_G55  //本设备是三相G表G55方案



//MX芯片，boot升级时采用如下规划
#define FLADDR_PROG           0
//主程序代码起始地址
#define FLADDR_PROG_START          (FLADDR_PROG+512)    //524*1024=536576
//主程序升级说明(256字节：前128字节用来表征是否烧写，后128字节表示是否烧写成功！)
#define FLADDR_PROG_FLAG           (FLADDR_PROG + (FLASH_SECTOR_UPDATE_END-1)*4096)      //128字节
//程序升级完毕区域,烧写前为0xFF,烧写后，变为0x669955AA
//升级检查条件： PROG_FLAG 为669955AA   PROG_FINISH是0xFF
#define FLADDR_PORG_FINISH         (FLADDR_PROG_FLAG  + 128)  //4字节
//#define FLADDR_PORG_HEADER         (FLADDR_PROG_FLAG  + 256)  //4字节
#define FLADDR_PORG_HEADER FLADDR_PROG

BOOLEAN read_datfile_header(DATHEADER *header, INT8U *buffer);
BOOLEAN check_update_header(DATHEADER *header,INT8U fileflag);
void update_download_record(INT16U sec_count,INT16U cur_sec,INT16U sec_len);
BOOLEAN check_download_record(INT8U *buffer);
BOOLEAN datfile_update(DATHEADER *header, INT8U *resetFlag,INT8U *buffer);
BOOLEAN check_datfile(DATHEADER *header, INT8U *buffer);
void decode_update(INT8U *buffer,INT16U len,INT32U addr);

#endif
 