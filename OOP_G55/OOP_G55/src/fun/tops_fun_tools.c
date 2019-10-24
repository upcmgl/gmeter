#include "../main_include.h"
#include "asf.h"

#define __METER_SIMPLE__

const INT8U  update_watermark[8] = { 'T','O','P','S','C','O','M','M' };

BOOLEAN read_datfile_header(DATHEADER *header, INT8U *buffer)
{
    INT16U datalen;
    INT16U pos;
    INT16U idx;
    INT16U len;
    INT8U  count;
    INT8U  cs;
    INT8U  tag;
    INT8U  taglen;

    //初始化文件头
    mem_set(header, sizeof(DATHEADER), 0xFF);

    //读取文件头
    len = 512;
    //fread_array_dat(0,buffer,len,file_type);
   //1.首先验证水印
    if (compare_string(buffer, (INT8U*)update_watermark, sizeof(update_watermark)) != 0)
    {
        return FALSE;
    }
    pos = sizeof(update_watermark);
    //2.其次验证检验和
    datalen = bin2_int16u(buffer + pos);
    pos += 2;
    if (len < pos + datalen)
    {
        return FALSE;
    }
    cs = 0;
    for (idx = 0;idx < datalen + pos;idx++)
    {
        cs += buffer[idx];

    }
    if (buffer[datalen + pos] != cs)
    {
        return FALSE;
    }
    //如果没有任何可处理元素，则返回真
    if (0 == datalen)
    {
        return FALSE;
    }
    //3.枚举所有tag，并检查tag值
    count = buffer[pos++];
    //如果没有任何可处理元素，则返回真
    if (0 == count)
    {
        return FALSE;
    }
    for (idx = 0;idx < count;idx++)
    {
        tag = buffer[pos++];
        taglen = buffer[pos++];

        switch (tag)
        {
        case 0://文件类别
            header->filetype = bin2_int16u(buffer + pos);
            break;
        case 1://文件版本
            header->filever = bin2_int16u(buffer + pos);
            break;
        case 2://文件长度
            header->filesize = bin2_int32u(buffer + pos);
            break;
        case 3://文件md5值
            mem_cpy(header->md5, buffer + pos, taglen);//限制TAGLEN
            break;
        case 4://加密算法
            header->encryptno = buffer[pos];
            break;
        case 5://升级文件类别
            header->updatettype = buffer[pos];
            break;
        case 6://升级特征字
            if (taglen <= sizeof(header->updatetspec))
            {
                mem_cpy(header->updatetspec, buffer + pos, taglen);//限制TAGLEN
            }
            break;
        case 7://升级版本号
            if (taglen <= sizeof(header->updatetver))
            {
                mem_cpy(header->updatetver, buffer + pos, taglen);//限制TAGLEN
            }
            break;
        case 8://升级版本日期
            if (taglen <= sizeof(header->updatetdate))
            {
                mem_cpy(header->updatetdate, buffer + pos, taglen);//限制TAGLEN
            }
            break;
        case 9://省份特征
            if (taglen <= sizeof(header->areacode))
            {
                mem_cpy(header->areacode, buffer + pos, taglen);//限制TAGLEN
            }
            break;
        case 11://附加文件总大小
            if (taglen == 4)
            {
                header->attachsize = bin2_int32u(buffer + pos);
            }
            break;
        case 12://附加文件列表，每个文件大小
            if (taglen == 20)
            {
                mem_cpy(header->attachfile, buffer + pos, taglen);//限制TAGLEN
            }
            break;
        case 13://设备特征
            if (taglen <= sizeof(header->device_code_flag))
            {
                mem_cpy(header->device_code_flag, buffer + pos, taglen);//限制TAGLEN
            }
            break;
        }
        pos += taglen;

    }	
    
    if(header->filetype != FILEFLAG_TYPE) return FALSE;

    return TRUE;
}

BOOLEAN check_update_header(DATHEADER *header,INT8U fileflag)
{
   if(fileflag == FILEFLAG_DEV_PROG)
   {
        if(header->filetype == FILEFLAG_TYPE) return TRUE;
		return FALSE;
   }
   return TRUE;
}
void update_download_record(INT16U sec_count,INT16U cur_sec,INT16U sec_len)
{
    tagDatetime datetime;
    INT16U temp=0;
	INT8U val=0;

    //写总段数
    fread_array_update_info(FLADDR_PROG_TOTAL_SEC_COUNT,(INT8U *)&temp,2);
    if(temp == 0xFFFF)
    {
        os_get_datetime(&datetime);
        fwrite_array_update_info(FLADDR_PROG_TOTAL_SEC_COUNT,(INT8U *)&sec_count,2);
        fwrite_array_update_info(FLADDR_UPDATE_TIME,datetime.value,6);
    }
    //如果是最后一个段，则写末段的长度，否则写首段或中间段长度（首段、中间段长度固定，末段长度不固定）
    if(cur_sec == sec_count-1 )
    {
        fread_array_update_info(FLADDR_PROG_END_FRAME_LEN,(INT8U *)&temp,2);
        if(temp == 0xFFFF)
        {
            fwrite_array_update_info(FLADDR_PROG_END_FRAME_LEN,(INT8U *)&sec_len,2);
        }
    }
    else
    {
        fread_array_update_info(FLADDR_PROG_FRAME_LEN,(INT8U *)&temp,2);
        if(temp == 0xFFFF)
        {
            fwrite_array_update_info(FLADDR_PROG_FRAME_LEN,(INT8U *)&sec_len,2);
        }
    }
    //写每段的下载情况
    fread_array_update_info(FLADDR_PROG_DOWNLOAD_RECORD+cur_sec/8,&val,1);
    clr_bit_value(&val,1,cur_sec %8);
    fwrite_array_update_info(FLADDR_PROG_DOWNLOAD_RECORD+cur_sec/8,&val,1);
}
BOOLEAN check_download_record(INT8U *buffer)
{
    INT16U sec_count;
	INT16U idx;
    INT16U count;
    INT16U ps_count;

    sec_count = 0;
    //总段数
    fread_array_update_info(FLADDR_PROG_TOTAL_SEC_COUNT,(INT8U *)&sec_count,2);
    
    if((sec_count == 0xFFFF) || (sec_count == 0x00) || (sec_count>4096))
    {
        return FALSE;
    }

    count=sec_count/8;
    //所有段下载标记
    fread_array_update_info(FLADDR_PROG_DOWNLOAD_RECORD,buffer,count+1);

    for(idx=0;idx<count;idx++)
    {
       if(buffer[idx] != 0x00)
       {
       		return FALSE;
       }
    }
    
    if(sec_count % 8 != 0)
    {
        ps_count=(sec_count % 8) ;
		for(idx=0;idx<ps_count;idx++)
        {
    		if(get_bit_value(buffer+count,1,idx))
            {
             	return FALSE;
            }
        }
    }
    return TRUE;
}
BOOLEAN datfile_update(DATHEADER *header, INT8U *resetFlag,INT8U *buffer)
{
    BOOLEAN check_md5, resault;
   union{
      INT8U info[32];
      INT32U u32[8];
      INT16U u16[16];
    }var;
    *resetFlag = FALSE;
    resault = FALSE;
    
    check_md5 = check_datfile(header, buffer);
    if (check_md5)
    {
        *resetFlag = TRUE;
        resault = TRUE;
    }
    return resault;
}
void toggle_LED_ERR(void);
/*+++
 功能： 根据DAT文件头信息，验证文件是否合法，这里仅验证字体MD5文件
 参数：
        INT8U fileflag    文件标识
        DATHEADER *header 文件头结构
        INT8U *buffer     用于读取文件的缓冲区，缓冲区大小必须大于2064字节
        INT8U file_type  0-终端文件  其它-usb文件
返回值
    TRUE-文件验证成功 FALSE-文件验证失败
---*/
#define PIM_DATA                    512  
BOOLEAN check_datfile(DATHEADER *header, INT8U *buffer)
{
    INT32U   progLen, progStart;
    INT16U i, len;
    MD5_CTX md5_ctx;
    INT16U  prog_page_size, ret;
    INT8U  hash[16] = { 0 };
    static INT8U led_count = 0;

    //根据文件长度，读取文件，并解密，计算MD5
    if (header->filesize == 0xFFFFFFFF) return FALSE;
    if (header->filesize > 0x100000) return FALSE;//文件长度超过1M则返回失败
 
    if(header->filetype != FILEFLAG_TYPE) return FALSE;
    
    //header->encryptno
    progLen = header->filesize;
       //解密并计算MD5
       //每次读取代码的长度
    prog_page_size = 512;
	progStart = PIM_DATA;

    MD5Init(&md5_ctx);

    while (progLen > 0)
    {
        if (progLen >= prog_page_size)
            len = prog_page_size;
        else
            len = progLen;
        ret = fread_array_update_data( progStart, buffer, len);
        if (ret != FILE_SUCCESS)
        {
            break;
        }
		
        //解密
        //加解密算法1:使用"moc.mmocspot.www"对原文进行循环异或
        if (header->encryptno == ENCRYPT_TOPS1)
        {
            decode_update(buffer, len, progStart-PIM_DATA);
            }
        MD5Update(&md5_ctx, buffer, len);
        progStart += len;
        progLen -= len;
        if ((led_count++) > 20)
        {
            led_count = 0;
			toggle_LED_ERR();
        }
    }
    MD5Final(hash, &md5_ctx);
    for (i = 0;i < 16;i++)
    {
        if (header->md5[i] != hash[i])
        {
            return FALSE;
        }
    }
    return TRUE;

}


FSFILE * usb_open_file(INT8U file_type, FSFILE *file)
{
    char    *filename;
    FRESULT ret;

    switch(file_type)
    {
	    case FILEFLAG_DEV_PROG:   //程序文件
        {
            if(FILEFLAG_TYPE == FILEFLAG_SGP_GMTER_G55)//单相G表
            {
                filename="0:G55_S_GMETER.dat";
            }
            else if(FILEFLAG_TYPE == FILEFLAG_TGP_GMTER_G55)//三相G表
            {
                filename="0:G55_GMETER.dat";
            }
            else
            {
                filename=NULL;
            }
        }
            break;
        default:
            filename=NULL;
            break;
    }
    if(filename == NULL) 
    {
        return NULL;         
    }

    ret = f_open_app(file, filename, FA_READ);
    if(ret != FR_OK)
    {
        return NULL;
    }

     return file;
}
BOOLEAN usb_file_exist(INT8U file_type, FSFILE* file)
{
    FSFILE *myFile;
    myFile=usb_open_file(file_type, file);
    if(myFile != NULL)
    {
        f_close(myFile);
    }  
    
    return (myFile!=NULL);
}

void decode_update(INT8U *buffer,INT16U len,INT32U addr)
{
    INT8U TOPS_ENCRYPT_1_STR[] = { 'm','o','c','.','m','m','o','c','s','p','o','t','.','w','w','w' };
    INT8U encrypt_idx;
	INT8U  encrypt_len= sizeof(TOPS_ENCRYPT_1_STR), k;
	INT32U i;
    
    encrypt_idx= addr%encrypt_len;
    
    for (i = 0;i < len;i++)
    {
        k =  (addr+i);
        k |= (addr+i) >> 8;
        k |= (addr+i) >> 16;
        k |= (addr+i) >> 24;

        buffer[i] = buffer[i] ^ (TOPS_ENCRYPT_1_STR[encrypt_idx++] - 0x30) ^ k;
        if (encrypt_idx >= encrypt_len)
        {
            encrypt_idx = 0;
        }
    }
}

void usb_download_program(FSFILE * myFile,INT8U *data_buffer)
{
      INT32S  len;
      INT32U  datalen;
      INT8U   idx1,down_flag;
	  INT32U addr;
      
      UINT count;
      FRESULT ret;

        //循环读取文件并保存  
        datalen = 0;
        down_flag = 0;
        addr     = 0;
        do
        {         
           ClrTaskWdt();
           //len = FSfread(data_buffer,1,512,myFile);
           ret = f_read(myFile, data_buffer, 512, &count);
           if(ret != FR_OK)
           {
               f_close(myFile);
               return;
           } 
           len = count;
           if(len > 0)
           {
    		 if(addr==0)
			 {
			 	fwrite_array_update_data(FLADDR_PORG_HEADER,data_buffer,512);
			 }
             else
			 {
				//加密存入NOR_FLASH，烧写到内部flash再解密
				fwrite_array_update_data(addr,data_buffer,512);
			 }
             addr += len;
             datalen += len;
           }
           //告警灯一个闪烁，表示U盘已经插入了。
           down_flag ++;
           if( (down_flag&0x0F) == 0x0F)      
           {
                   //闪烁告警灯
                LED_comm_toggle();
           } 
       }while(len > 0);

}
INT8U usb_download_dat(INT8U file_type, FSFILE * file)
{
    DATHEADER update_header;
    BOOLEAN sucess;
    INT8U resetFlag;
    
    resetFlag =FALSE;
    delete_update_file();

	update_object.usbFile=usb_open_file(file_type, file);
	usb_download_program(update_object.usbFile,update_object.buffer);
    //读取文件头
	fread_array_update_data(FLADDR_PORG_HEADER,update_object.buffer,512);
    sucess=read_datfile_header(&update_header,update_object.buffer);

    //验证MD5升级文件
    if(sucess )
    {        
		if(update_object.usbFile!=NULL)
		{
	        if(datfile_update(&update_header,&resetFlag,update_object.buffer)==FALSE)
			{

			}
		}
    }
    else
    {
    }
	if(update_object.usbFile!=NULL)
    {
			f_close(update_object.usbFile);
    }
    return resetFlag;
}
/*+++
 功能： 处理U盘上的升级文件
 参数：
        无
---*/
BOOLEAN usb_file_process(void)
{
    INT8U   idx;
    FSFILE file;
    BOOLEAN  reset_flag=FALSE;

    for(idx = 0; idx<1; idx++)
    {
        switch(idx)
        {
        case 0:   //程序文件
                if(usb_file_exist(FILEFLAG_DEV_PROG, &file))
                {
                    //record_sys_operate(OP_SYS_MAIN_UPDATE);  
                    reset_flag=usb_download_dat(FILEFLAG_DEV_PROG, &file);
                }
                break;
        default:
                break;
        }
    }

    return reset_flag;
}
