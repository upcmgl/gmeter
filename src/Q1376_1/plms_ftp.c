#include "../main_include.h"
#include "app_const_3761.h"
#include "app_3761.h"
#include "protocol_app_handler.h"   
#include "plms_ftp.h"
#include "tops_fun_tools.h"


BOOLEAN process_ftp_F1_new(objRequest *pRequest,objResponse *pResp);
//BOOLEAN read_datfile_header(DATHEADER *header, INT8U *buffer);
//BOOLEAN check_update_header(DATHEADER *header,INT8U fileflag);
//void update_download_record(INT16U sec_count,INT16U cur_sec,INT16U sec_len);
//BOOLEAN check_download_record(INT8U *buffer);
//BOOLEAN datfile_update(DATHEADER *header, INT8U *resetFlag,INT8U *buffer);
//BOOLEAN check_datfile(DATHEADER *header, INT8U *buffer);
void  process_frame_ftp(objRequest *pRequest,objResponse *pResp)
{
    INT16U pn,fn;
    INT8U resetFlag;
#ifdef __EDP__
    INT8U flag = 0;
#endif
	
    //设置控制码：DIR=1,PRM=0, FUNC=8  响应报文，用户数据
    pResp->frame[POS_CTRL] |=  CTRLFUNC_USERDATA;

    //提取数据单元标识
    pn=bin2_int16u(pRequest->frame +  pRequest->pos);
    fn=bin2_int16u(pRequest->frame +  pRequest->pos+2);
    pRequest->pos+=4;

    if ((pn == DA_P0)&&(fn == DT_F1))
    {
        resetFlag =process_ftp_F1_new(pRequest,pResp);
        if(resetFlag) //如果需要复位
        {
            //延时是为了确保最后的应答报文可以发送出去！！
            DelayNmSec(2000);
            if(pResp->channel == PROTOCOL_CHANNEL_REMOTE)
            {
                 //GPRS多等待5秒
                 DelayNmSec(5000);
            }
 #ifdef __EDP__
            if(gSystemInfo.update==0xAA)
            {
	            flag=0xCC;
	            fwrite_ertu_params(EEADDR_UPDATE_FLAG, &flag,1);
				fread_ertu_params(EEADDR_UPDATA_INFO,&edp_updata_INFO,sizeof(struct edp_updata_info));
            }
#endif			
            app_softReset();
        }
    }
}

BOOLEAN process_ftp_F1_new(objRequest *pRequest,objResponse *pResp)
{
    INT8U *frameData;
    INT16U idx=0;
    INT16U pn,fn;
    INT16U nCount;
    INT16U fileDataLen;
//    INT16U sec_count;
    INT16U cur_sec=0;
//    INT16U sec_data_size;
    INT32U progAddr;
//    INT32U progLen;
    INT32U respflag;
    INT32U tag_cur_seg;
//	INT16U file_id,attach_idx;

    INT8U fileFlag,fileProp,fileCmd;
    INT8U resetFlag;
    BOOLEAN sucess;

    frameData = pRequest->frame + pRequest->pos;
    
    fileFlag = frameData[idx++];      //文件标识
    fileProp = frameData[idx++];      //文件属性
    fileCmd  = frameData[idx++];      //文件指令

    nCount=bin2_int16u(frameData+idx);  //总段数
    idx+=2;
    tag_cur_seg = bin2_int32u(frameData+idx);   //第i段标识
    idx+=4;
    fileDataLen=bin2_int16u(frameData+idx);  //第i段数据长度Lf
    idx+=2;

    //判断是国网升级方式，还是浙江升级方式
    if( (fileFlag == 0x01) && (fileProp == 0x00) && (fileCmd == 0) )
    {
        if(tag_cur_seg == 0)
        {
        }
    }

    if(fileDataLen==0)fileDataLen=512;

    progAddr = tag_cur_seg * fileDataLen ;
    //如果为末尾帧则为最后一段，否则根据偏移量和长度计算当前段
    if(0x01 == fileProp)
    {
        fread_array_update_info(FLADDR_PROG_FRAME_LEN,(INT8U *)&cur_sec,2);
        progAddr = tag_cur_seg * cur_sec;
        cur_sec = nCount-1;
    }
    else
    {
        cur_sec = tag_cur_seg;
    }
    respflag = tag_cur_seg;

    

    pn = DA_P0;
    fn = DT_F1; //全部否认
    pResp->pos = POS_DATA;
    resetFlag = FALSE;
    switch(fileCmd)
    {
        case FILECMD_FRAME://报文方式升级
            //不做任何处理
            //if(!(resp->is_cast))
            {
                update_object.update_group_flag =FALSE;
            }
            break;
        case FILECMD_FTP: //FTP方式升级
            //提取升级文件、账号/密码，跳转至FTP升级程序
            respflag = 0xFFFFFFFF;//检查发现文件错误
            break;

        case FILECMD_GROUP://组地址升级
            //设置组地址升级标志，收到F1升级清除命令或以单地址下载或升级完成，则退出组地址升级
            update_object.update_group_flag =1;
            break;
        default:
            break;
    }
    if(FILEFLAG_CLEAR == fileFlag)
    {
        respflag=0x00;
        delete_update_file();
    }
    else if(FILECMD_FRAME == fileCmd)
    {
            //如果是第一个段，则清除历史下载信息
            if(0==tag_cur_seg)
            {
                    //验证字节传输顺序
                    if(compare_string(frameData+idx,(INT8U*)update_watermark,sizeof(update_watermark)) == 0)
                    {
                            update_object.is_transmit_reverse=FALSE;
                            mem_cpy(update_object.buffer,frameData+idx,512);
                            sucess=read_datfile_header(&update_header,update_object.buffer);
                    }
                    else
                    {
                            update_object.is_transmit_reverse=TRUE;
                            if(fileDataLen>=512)
                            {
                                mem_cpy_reverse(update_object.buffer,frameData+idx+fileDataLen-512,512);
                            }
                            sucess=read_datfile_header(&update_header,update_object.buffer);
                    }
                    //验证升级版本是否符合要求
                    if(!sucess ||(FALSE==check_update_header(&update_header,fileFlag)))
                    {
                        respflag = 0xFFFFFFFF;//检查发现文件错误
                    }
                    delete_update_file();
            }
            if(respflag!=0xFFFFFFFF)
            {
                //存储升级文件
                if(update_object.is_transmit_reverse)
                {
                    // 不使用临时变量，直接利用resp的buffer,
                    mem_cpy_reverse(pResp->frame+pResp->pos,frameData+idx,fileDataLen);
                    fwrite_array_update_data(progAddr,pResp->frame+pResp->pos,fileDataLen);
                }
                else
                {
                    // 不使用临时变量，直接利用resp的buffer,
                    mem_cpy(pResp->frame+pResp->pos,frameData+idx,fileDataLen);
                    fwrite_array_update_data(progAddr,pResp->frame+pResp->pos,fileDataLen);
                }
                update_download_record(nCount,cur_sec,fileDataLen);
                //检查所有段是否已经下载，如果下载完成，做相应操作
                if(check_download_record(frameData))
                {
                    if(datfile_update(&update_header,&resetFlag,frameData)==FALSE)
                    {
                        respflag=0xFFFFFFFF;
                    }
					
                }
            }
    }

    set_pn_fn(pResp->frame, &pResp->pos, pn, fn);

    //应答确认数据

    //第i段标识
     pResp->frame[pResp->pos++] = respflag;
     pResp->frame[pResp->pos++] = respflag>>8;
     pResp->frame[pResp->pos++] = respflag>>16;
     pResp->frame[pResp->pos++] = respflag>>24;

    //if(!(pResp->is_cast))//非组地址下载时，需要应答
    { 
         //发送响应祯，结束=TRUE
      app_send_response_frame(pRequest,pResp,TRUE);

    }
    return resetFlag;

}

/*+++
 功能： 一类F14,文件传输未收到的数据
 参数：
        INT8U *buffer 数据缓冲区不小于130字节
返回值
    实际读取的长度
---*/
#ifndef __ONENET__
INT16U read_c1_f14(INT8U *buffer)
{
    INT16U sec_count;
	INT16U idx;
    INT16U group_count;
    INT16U size;

    mem_set(buffer,130,0xff);    
    //总段数
    fread_array_update_info(FLADDR_PROG_TOTAL_SEC_COUNT,(INT8U *)&sec_count,2);
    
    if((sec_count == 0xFFFF) || (sec_count == 0x00) || (sec_count>0x2800))
    {
        return 130;
    }

    group_count=sec_count/1024;
    if(sec_count % 1024 !=0)
    {
        group_count++;
    }
    //读取段下载标记
    for(idx=0;idx<group_count;idx++)
    {
        size=(idx+1)*1024;
        if(size>sec_count)
        {
            size=(1024-(size-sec_count))/8+1;
        }
        if(size>128)
        {
            size=128;
        }
        fread_array_update_info(FLADDR_PROG_DOWNLOAD_RECORD,buffer+2,size);
        if(!check_is_all_ch(buffer+2,size,0))
        {
            buffer[0]=idx;    
            buffer[1]=idx>>8;
            break;    
        }
    }

    return 130;
}
#else
INT16U read_c1_f14(INT8U *buffer)
{
    INT16U sec_count;
	INT16U idx;
    INT16U group_count;
    INT16U size;

    mem_set(buffer,258,0xff);
    sec_count = 0;   
    //总段数
    fread_array_update_info(FLADDR_PROG_TOTAL_SEC_COUNT,(INT8U *)&sec_count,2);
    
    if((sec_count == 0xFFFF) || (sec_count == 0x00) || (sec_count>0x2800))
    {
        return 130;
    }
    mem_cpy(buffer,(INT8U*)&sec_count,2);
    sec_count = sec_count/8;
	if(sec_count%8!=0)
	{
		sec_count++;
	}
	fread_array_update_info(FLADDR_PROG_DOWNLOAD_RECORD,buffer+2,sec_count);
	
    return sec_count+2;
}
#endif