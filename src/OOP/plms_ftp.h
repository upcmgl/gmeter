#ifndef __PLMS_FTP_H__
#define __PLMS_FTP_H__

//升级类别定义，目前按新376.1文件传输的文件标识定义
#define FILEFLAG_CLEAR            0x00    //清除传输文件
#define FILEFLAG_DEV_PROG         0x01    //终端升级文件

BOOLEAN read_datfile_header(DATHEADER *header, INT8U *buffer);
BOOLEAN check_update_header(DATHEADER *header,INT8U fileflag);
void update_download_record(INT16U sec_count,INT16U cur_sec,INT16U sec_len);
BOOLEAN check_download_record(INT8U *buffer);
BOOLEAN datfile_update(DATHEADER *header, INT8U *resetFlag,INT8U *buffer);
BOOLEAN check_datfile(DATHEADER *header, INT8U *buffer);

#endif



