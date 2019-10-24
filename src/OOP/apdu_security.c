#include "protocol_app_handler.h"
#include "protocol_app_const_def.h"
#include "object_describe.h"
#include "drv_esam_spi.h"
//#if defined __ENABLE_ESAM2__
#ifdef __ENABLE_ESAM2__

#define SID_MAC     0       //数据验证码
#define RN          1       //随机数
#define RN_MAC      2       //随机数+数据MAC
#define SID         3       //安全标识

#define RTU_DECODE_ERROR                   0x1001	/*终端解密错误*/
#define RTU_VERI_SIGN_ERROR                0x1002	/*终端验签失败*/
#define RTU_VERI_MAC_ERROR                 0x1003	/*终端MAC校验失败*/
#define RTU_DEVCOUNT_ERROR                 0x1004	/*会话计数器错误*/
#define LINK_CODE_UNIT_ERROR               0x1009   /*链路设备密码单元故障*/


#define RTU_LINK_SESSION_ERROR             0x3005	/*当前链路会话协商失败*/


void sal_send_error_resp(objRequest  *pRequest,objResponse *pResp,INT16U err_code);
void app_protocol_handler(objRequest* pRequest,objResponse *pResp);

INT16S esam_send_cmd(INT8U *cmd_buffer,INT16U cmd_len,INT8U *recv_buffer,INT16U recv_max_len,INT16U *err_code)
{
    INT16S len;
    INT8U headdata[4];

    //获取信号量
    //tpos_mutexPend(&SIGNAL_EASM);
    drv_esam_spi_WriteArray(cmd_buffer,cmd_len);
    len = drv_esam_spi_ReadArray(recv_buffer,recv_max_len,headdata,err_code);
    //释放信号量
    //tpos_mutexFree(&SIGNAL_EASM);

    return len;
}
/*+++
  功能： 主站发送获取 T-ESAM 信息命令

  参数：
         INT8U *buffer  缓冲区
  返回：
         数据长度
  描述：
      发送：803600FF0000
      返回：9000+LEN+ESAM序列号

 ---*/
INT16S esam_get_tesam_info(INT8U *buffer,INT16U max_len)
{
    INT16S len;
    INT16U pos;
    INT8U headdata[4];
    INT16U err_code;
    
    pos = 0;
    buffer[pos++] = 0x80;
    buffer[pos++] = 0x36;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0xFF;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    //获取信号量
    //tpos_mutexPend(&SIGNAL_EASM);
    drv_esam_spi_WriteArray(buffer,pos);
    mem_set(buffer,pos,0x00);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //释放信号量
    //tpos_mutexFree(&SIGNAL_EASM);

    return len;
}
//8002000060
INT16S esam_session_auth(INT8U *session_in,INT16U session_len,INT8U *buffer,INT16U max_len)
{
    INT16S len;
    INT16U pos;
    INT8U headdata[4];
    INT16U err_code;

    pos = 0;
    buffer[pos++] = 0x80;
    buffer[pos++] = 0x02;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x60;
    if(max_len-pos>0)
    {
        mem_cpy(buffer+pos,session_in,session_len);
        pos+=session_len;
    }
    //获取信号量
    //tpos_mutexPend(&SIGNAL_EASM);

    drv_esam_spi_WriteArray(buffer,pos);
    mem_set(buffer,pos,0x00);
    len = drv_esam_spi_ReadArray(buffer,max_len,headdata,&err_code);
    //tpos_mutexFree(&SIGNAL_EASM);
    return len;
}
INT16S req_object_attribute_esam(INT8U attri_id,INT8U index,INT8U *data,INT16U left_len)
{
    INT16U pos;
    INT16S len=0;
    INT8U esam_cmd[]={0x80,0x36,0x00,0x00,0x00,0x00};
    INT8U *text;
    INT16U err_code=0;
    INT8U chip_state;

    pos=0;
    switch(attri_id)
    {
    case 2: //ESAM序列号
		data[pos++]=1;
        data[pos++]=DT_OCTETSTRING;
        data[pos++]=8;
        esam_cmd[3]=0x02;
        mem_cpy(data+pos,esam_cmd,6);
        len=esam_send_cmd(data+pos,6,data+pos,32,&err_code);
        if(len!=8)
        {
            mem_set(data+pos,8,0x00);
        }
        pos+=8;
        break;
    case 3: //ESAM版本号
		data[pos++]=1;
        data[pos++]=DT_OCTETSTRING;
        data[pos++]=4;
        esam_cmd[3]=0x03;
        mem_cpy(data+pos,esam_cmd,6);
        len=esam_send_cmd(data+pos,6,data+pos,32,&err_code);
        if(len!=4)
        {
            mem_set(data+pos,4,0x00);
        }
        pos+=4;
        break;
    case 4: //对称密钥版本16字节
		data[pos++]=1;
        data[pos++]=DT_OCTETSTRING;
        data[pos++]=1;
        esam_cmd[3]=0x04;
        mem_cpy(data+pos,esam_cmd,6);
        len=esam_send_cmd(data+pos,6,data+pos,32,&err_code);
        data[pos-1]=len;
        pos+=len;
        break;
    case 5: //会话时效门限
		data[pos++]=1;
        data[pos++]=DT_DOUBLELONGUNSIGNED;
        esam_cmd[3]=0x06;
        mem_cpy(data+pos,esam_cmd,6);
        len=esam_send_cmd(data+pos,6,data+pos,32,&err_code);
        if(len!=4)
        {
            mem_set(data+pos,4,0x00);
        }
        pos+=4;
        break;
    case 6: //会话时效剩余时间
		data[pos++]=1;
        data[pos++]=DT_DOUBLELONGUNSIGNED;
        esam_cmd[3]=0x07;
        mem_cpy(data+pos,esam_cmd,6);
        len=esam_send_cmd(data+pos,6,data+pos,32,&err_code);
        if(len!=4)
        {
            mem_set(data+pos,4,0x00);
        }
        pos+=4;
        break;
    case 7: //当前计数器
		data[pos++]=1;
        data[pos++]=DT_STRUCTURE;
        data[pos++]=3;

        esam_cmd[3]=0x08;
        mem_cpy(data+pos,esam_cmd,6);
        len=esam_send_cmd(data+pos,6,data+pos,32,&err_code);
        if(len==12)
        {
            mem_cpy(g_esam_info.counter.single_addr_counter,data+pos,4);
            mem_cpy(g_esam_info.counter.report_counter,data+pos+4,4);
            mem_cpy(g_esam_info.counter.broadcast_no,data+pos+8,4);

            data[pos++]=DT_DOUBLELONGUNSIGNED;
            mem_cpy(data+pos,g_esam_info.counter.single_addr_counter,4);
            pos+=4;
            data[pos++]=DT_DOUBLELONGUNSIGNED;
            mem_cpy(data+pos,g_esam_info.counter.report_counter,4);
            pos+=4;
            data[pos++]=DT_DOUBLELONGUNSIGNED;
            mem_cpy(data+pos,g_esam_info.counter.broadcast_no,4);
            pos+=4;
        }

        if(len<=0)
        {
            data[pos++]=DT_DOUBLELONGUNSIGNED;
            mem_set(data+pos,4,0x00);
            pos+=4;
            data[pos++]=DT_DOUBLELONGUNSIGNED;
            mem_set(data+pos,4,0x00);
            pos+=4;
            data[pos++]=DT_DOUBLELONGUNSIGNED;
            mem_set(data+pos,4,0x00);
            pos+=4;
        }
        break;
    case 8: //证书版本
		data[pos++]=1;
        data[pos++]=DT_STRUCTURE;
        data[pos++]=2;

        esam_cmd[3]=0x05;
        mem_cpy(data+pos,esam_cmd,6);
        len=esam_send_cmd(data+pos,6,data+pos,32,&err_code);
        if(len==2)
        {
            g_esam_info.cert_ver[1]=data[pos+1];
            g_esam_info.cert_ver[0]=data[pos];
            data[pos++]=DT_OCTETSTRING;
            data[pos++]=1;
            data[pos++]=g_esam_info.cert_ver[1];
            data[pos++]=DT_OCTETSTRING;
            data[pos++]=1;
            data[pos++]=g_esam_info.cert_ver[0];
        }

        if(len!=2)
        {
            data[pos++]=DT_OCTETSTRING;
            data[pos++]=1;
            data[pos++]=0;
            data[pos++]=DT_OCTETSTRING;
            data[pos++]=1;
            data[pos++]=0;
        }
        break;
    case 9: //终端证书序列号
		data[pos++]=1;
        data[pos++]=DT_OCTETSTRING;
        data[pos++]=16;

        esam_cmd[3]=0x09;
        mem_cpy(data+pos,esam_cmd,6);
        len=esam_send_cmd(data+pos,6,data+pos,32,&err_code);
        if(len!=16)
        {
            mem_set(data+pos,16,0x00);
        }
        pos+=16;
        break;
    case 10: //终端证书
        text=data+pos+4;
        text[0]=0x80;
        text[1]=0x36;
        text[2]=0x00;
        text[3]=0x0B;
        text[4]=0x00;
        text[5]=0x00;
        len=esam_send_cmd(text,6,text,left_len-5,&err_code);
		data[pos++]=1;
        data[pos++]=DT_OCTETSTRING;
        if(len>0)
        {
            pos+=set_cosem_data_len(data+pos,len);
            mem_cpy(data+pos,text,len);
            pos+=len;
        }
        else
        {
            data[pos++]=0;
        }
        break;
    case 11: //主站证书序列号
		data[pos++]=1;
        data[pos++]=DT_OCTETSTRING;
        data[pos++]=16;
        esam_cmd[3]=0x0A;
        mem_cpy(data+pos,esam_cmd,6);
        len=esam_send_cmd(data+pos,6,data+pos,32,&err_code);
        if(len!=16)
        {
            mem_set(data+pos,16,0x00);
        }
        pos+=16;
        break;
    case 12: //主站证书
        text=data+pos+4;
        text[0]=0x80;
        text[1]=0x36;
        text[2]=0x00;
        text[3]=0x0C;
        text[4]=0x00;
        text[5]=0x00;
        len=esam_send_cmd(text,6,text,left_len-5,&err_code);
		data[pos++]=1;
        data[pos++]=DT_OCTETSTRING;
        if(len>0)
        {
            pos+=set_cosem_data_len(data+pos,len);
            mem_cpy(data+pos,text,len);
            pos+=len;
        }
        else
        {
            data[pos++]=0;
        }
        break;
    case 13: //ESAM安全存储对象列表
		data[pos++]=1;
        data[pos++]=DT_ARRAY;
        data[pos++]=0;
        break;
    default:
		data[pos++]=1;
        data[pos++]=0;
        break;
    }
    return pos;
}

void esam_get_info(void)
{
    uint8_t buffer[30]={0};
    uint8_t len;

    mem_set((INT8U*)&g_esam_info,sizeof(ESAM_INFO),0xFF);
    gSystemInfo.esam_type=0;
    mem_set(gSystemInfo.ESAM_DEVID,8,0xFF);
    gSystemInfo.ESAM_CERT_STATE = 0xFF;
    len = esam_get_esam_id(buffer,20);
    if(len == 8) 
    {
        mem_cpy_reverse(gSystemInfo.ESAM_DEVID,buffer,8);
        //gSystemInfo.esam_type=13;//13规范esam
        len = esam_get_chip_state(buffer,20);
        if(len == 1) gSystemInfo.ESAM_CERT_STATE = buffer[0];
    }
    if(gSystemInfo.esam_type==0)
    {
        len = tesam_get_esam_id(buffer,20);
        if(len == 8)
        {
            mem_cpy_reverse(gSystemInfo.ESAM_DEVID,buffer,8);
            //gSystemInfo.esam_type=ESAM_OOP;//698esam
            len = tesam_get_chip_state(buffer,30);
            if(len == 16)
            {
                if(check_is_all_ch(buffer,16,0x00))
                {
                    gSystemInfo.ESAM_CERT_STATE=0x00;
                }
                else
                {
                    gSystemInfo.ESAM_CERT_STATE = 0x01;
                }
            }
        }
    }
    //if(gSystemInfo.esam_type==ESAM_OOP)//698esam
    {
        //提高esam spi通讯速率
        drv_esam_spi_high_Initialize();
    }
}
void update_esam_chip_state(void)
{
    uint8_t buffer[30]={0};
    uint8_t len;

    len = tesam_get_chip_state(buffer,30);
    if(len == 16)
    {
        if(check_is_all_ch(buffer,16,0x00))
        {
            gSystemInfo.ESAM_CERT_STATE=0x00;
        }
        else
        {
            gSystemInfo.ESAM_CERT_STATE = 0x01;
        }
    }
}
void exec_esam_method(OBJECT_ATTR_DESC *oad,INT8U* param,objResponse *pResp)
{
    INT8U sid[4],mac[4];
    INT16U pos,val16;
    INT16S len;
    INT16U  text_len,attach_len;
    INT8U *text,*attach_info,head_size;
    INT16U err_code;

    pos=0;
    text=NULL;
    attach_info=NULL;
    mem_set(sid,4,0x00);
    mem_set(mac,4,0x00);

    switch(oad->attribute_id.value)
    {
    case 3: //ESAM读取
            pos++;//SID TYPE
            mem_cpy(sid,param+pos,4);
            pos+=4;
            attach_len=get_cosem_data_len(param+pos,&head_size);
            pos+=head_size;
            attach_info=param+pos;
            pos+=attach_len;
            
            text=pResp->frame+pResp->pos+8;
            mem_cpy(text,sid,4);
            mem_cpy(text+4,attach_info,attach_len);
            len=esam_send_cmd(text,4+attach_len,text,pResp->max_reply_pos-pResp->pos-8,&err_code);
            if(len>0)
            {
                pResp->frame[pResp->pos++] = DAR_SECCESS;
                pResp->frame[pResp->pos++] = 1;
                pResp->frame[pResp->pos++] = DT_OCTETSTRING;
                pResp->pos+=set_cosem_data_len(pResp->frame+pResp->pos,len);
                mem_cpy(pResp->frame+pResp->pos,text,len);
                pResp->pos+=len;
            }
            else
            {
                pResp->frame[pResp->pos++] = DAR_OTHER_REASON;
                pResp->frame[pResp->pos++] = 1;//Data  OPTIONAL
                pResp->frame[pResp->pos++] = DT_OCTETSTRING;
                pResp->frame[pResp->pos++] = 2;
                cosem_int162_bin(err_code,pResp->frame+pResp->pos);// esam 返回错误代码
                pResp->pos+=2;
                
            }
        break;
    case 4://数据更新（参数）
        if((param[pos]==DT_STRUCTURE) && (param[pos+1]==2))
        {
            pos+=2;
            text_len=get_cosem_data_len(param+pos,&head_size);
            pos+=head_size;
            text=param+pos;
            pos+=text_len;

            pos++;//SID TYPE
            mem_cpy(sid,param+pos,4);
            pos+=4;
            attach_len=get_cosem_data_len(param+pos,&head_size);
            pos+=head_size;
            attach_info=param+pos;
            pos+=attach_len;
            pos++;
            mem_cpy(mac,param+pos,4);
            pos+=4;
            
            //发送： sid1+AttachData1+Endata1
            mem_cpy(pResp->frame+pResp->pos+8,sid,4);
            mem_cpy(pResp->frame+pResp->pos+8+4,attach_info,attach_len);
            mem_cpy(pResp->frame+pResp->pos+8+4+attach_len,text,text_len);
            mem_cpy(pResp->frame+pResp->pos+8+4+attach_len+text_len,mac,4);
            len=esam_send_cmd(pResp->frame+pResp->pos+8,4+attach_len+text_len+4,pResp->frame+pResp->pos+8,pResp->max_reply_pos-pResp->pos-8,&err_code);
            if(len==0)
            {
                pResp->frame[pResp->pos++] = DAR_SECCESS;
                
            }
            else
            {
                pResp->frame[pResp->pos++] = DAR_OTHER_REASON;
                pResp->frame[pResp->pos++] =  1;//Data  OPTIONAL
                pResp->frame[pResp->pos++] = DT_OCTETSTRING;
                pResp->frame[pResp->pos++] = 2;
                cosem_int162_bin(err_code,pResp->frame+pResp->pos);// esam 返回错误代码
                pResp->pos+=2;
            }
        }
        else
        {
             pResp->frame[pResp->pos++] = DAR_OTHER_REASON;
            pResp->frame[pResp->pos++] = 0;
        }
        break;
    case 5: //协商失效
        pResp->frame[pResp->pos++] = DAR_SECCESS;
        pResp->frame[pResp->pos++] = 1;
        pResp->frame[pResp->pos++] = DT_DATETIME_S;
        sys_datetime2cosem_datetime( pResp->frame+pResp->pos);
        pResp->pos+=7;
        break;
    case 7: //更新秘钥
        if((param[pos]==DT_STRUCTURE) && (param[pos+1]==2))
        {
            pos+=3;//DT_STRUCTURE、2、octet-string
            //密文
            text_len=get_cosem_data_len(param+pos,&head_size);
            pos+=head_size;
            text=param+pos;
            pos+=text_len;
            //SID
            pos++;//SID data type
            mem_cpy(sid,param+pos,4);
            pos+=4;
            //附加域
            attach_len=get_cosem_data_len(param+pos,&head_size);
            pos+=head_size;
            attach_info=param+pos;
            pos+=attach_len;
            //MAC
            pos++;//mac data type
            mem_cpy(mac,param+pos,4);
            pos+=4;
            
            //发送： sid1+AttachData1+Endata1
            mem_cpy(pResp->frame+pResp->pos+8,sid,4);
            mem_cpy(pResp->frame+pResp->pos+8+4,attach_info,attach_len);
            mem_cpy(pResp->frame+pResp->pos+8+4+attach_len,text,text_len);
            mem_cpy(pResp->frame+pResp->pos+8+4+attach_len+text_len,mac,4);
            len=esam_send_cmd(pResp->frame+pResp->pos+8,4+attach_len+text_len+4,pResp->frame+pResp->pos+8,pResp->max_reply_pos-pResp->pos-8,&err_code);
            if(len==0)
            {
                pResp->frame[pResp->pos++] = DAR_SECCESS;
                update_esam_chip_state();

            }
            else
            {
                pResp->frame[pResp->pos++] = DAR_OTHER_REASON;
                pResp->frame[pResp->pos++] =  1;//Data  OPTIONAL
                pResp->frame[pResp->pos++] = DT_OCTETSTRING;
                pResp->frame[pResp->pos++] = 2;
                cosem_int162_bin(err_code,pResp->frame+pResp->pos);// esam 返回错误代码
                pResp->pos+=2;
                
            }
        }
        else
        {
             pResp->frame[pResp->pos++] = DAR_OTHER_REASON;
            pResp->frame[pResp->pos++] = 0;
        }
        break;
    case 8: //证书更新
        if((param[pos]==DT_STRUCTURE) && (param[pos+1]==2))
        {
            pos+=3;
            text_len=get_cosem_data_len(param+pos,&head_size);
            pos+=head_size;
            text=param+pos;
            pos+=text_len;

            pos++;//SID TYPE
            mem_cpy(sid,param+pos,4);
            pos+=4;
            attach_len=get_cosem_data_len(param+pos,&head_size);
            pos+=head_size;
            attach_info=param+pos;
            pos+=attach_len;
            //发送： sid1+AttachData1+Endata1
            mem_cpy(pResp->frame+pResp->pos+8,sid,4);
            mem_cpy(pResp->frame+pResp->pos+8+4,attach_info,attach_len);
            mem_cpy(pResp->frame+pResp->pos+8+4+attach_len,text,text_len);
            len=esam_send_cmd(pResp->frame+pResp->pos+8,4+attach_len+text_len,pResp->frame+pResp->pos+8,pResp->max_reply_pos-pResp->pos-8,&err_code);
            if(len==0)
            {
                pResp->frame[pResp->pos++] = DAR_SECCESS;
                update_esam_chip_state();
            }
            else
            {
                pResp->frame[pResp->pos++] = DAR_OTHER_REASON;
                pResp->frame[pResp->pos++] =  1;//Data  OPTIONAL
                pResp->frame[pResp->pos++] = DT_OCTETSTRING;
                pResp->frame[pResp->pos++] = 2;
                cosem_int162_bin(err_code,pResp->frame+pResp->pos);// esam 返回错误代码
                pResp->pos+=2;                
            }
        }
        else
        {
             pResp->frame[pResp->pos++] = DAR_OTHER_REASON;
             pResp->frame[pResp->pos++] = 0;
        }
        break;
    case 9://设置协商时效（参数）
        if((param[pos]==DT_STRUCTURE) && (param[pos+1]==2))
        {
            pos+=2;
            text_len=get_cosem_data_len(param+pos,&head_size);
            pos+=head_size;
            text=param+pos;
            pos+=text_len;

            pos++;//SID TYPE
            mem_cpy(sid,param+pos,4);
            pos+=4;
            attach_len=get_cosem_data_len(param+pos,&head_size);
            pos+=head_size;
            attach_info=param+pos;
            pos+=attach_len;
            //发送： sid1+AttachData1+Endata1
            mem_cpy(pResp->frame+pResp->pos+8,sid,4);
            mem_cpy(pResp->frame+pResp->pos+8+4,attach_info,attach_len);
            mem_cpy(pResp->frame+pResp->pos+8+4+attach_len,text,text_len);
            len=esam_send_cmd(pResp->frame+pResp->pos+8,4+attach_len+text_len,pResp->frame+pResp->pos+8,pResp->max_reply_pos-pResp->pos-8,&err_code);
            if(len==0)
            {
                pResp->frame[pResp->pos++] = DAR_SECCESS;
            }
            else
            {
                pResp->frame[pResp->pos++] = DAR_OTHER_REASON;
                pResp->frame[pResp->pos++] =  1;//Data  OPTIONAL
                pResp->frame[pResp->pos++] = DT_OCTETSTRING;
                pResp->frame[pResp->pos++] = 2;
                cosem_int162_bin(err_code,pResp->frame+pResp->pos);// esam 返回错误代码
                pResp->pos+=2;                
            }
        }
        else
        {
             pResp->frame[pResp->pos++] = DAR_OTHER_REASON;
             pResp->frame[pResp->pos++] = 0;
        }
        break;
    }
}
void process_client_apdu_security_error(objRequest *pRequest,objResponse *pResp)
{
    
    pResp->frame[pResp->pos++]=APDU_SECURITY_RESPONSE;
    pResp->frame[pResp->pos++]=2;//错误
    pResp->frame[pResp->pos++]=DAR_ESAM_VERIFY_FAIL;
    app_send_response_frame(pRequest,pResp,TRUE); 
}
void process_client_apdu_security(objRequest *pRequest,objResponse *pResp)
{

    INT16U pos,mac_len,text_len;
    INT8U  text_type,auth_type,cast_len;
    INT8U *text,sid[8],mac[4],head_size;
    INT16S len;
    INT16U err_code;

    pos=pRequest->userdata_pos+1;//跳过 apdu tag

    mem_set(sid,8,0x00);
    mem_set(mac,4,0x00);
    //提取apdu明文或密文
    text_type=pRequest->frame[pos++];
    text_len=get_cosem_data_len(pRequest->frame+pos,&head_size);
    pos+=head_size;
    text=pRequest->frame+pos;
    pos+=text_len;

    //数据验证信息
    auth_type=pRequest->frame[pos++];
    pRequest->security_info[0]=text_type;
    pRequest->security_info[1]=SECURITYRESULT_OK;
    pRequest->security_info[2]=auth_type;
    switch(auth_type)
    {
        case SID_MAC://数据验证码
            cast_len=0;
            //4字节SID
            mem_cpy(sid,pRequest->frame+pos,4);
            pos+=4;
            //附加域
            head_size=pRequest->frame[pos++];
            if(head_size==2)
            {
                mem_cpy(sid+4,pRequest->frame+pos,2);
            }
            else if(head_size==4)
            {
                mem_cpy(sid+4,pRequest->frame+pos,4);
            }
            pos+=head_size;
            //mac
            mac_len=pRequest->frame[pos++];
            if(mac_len==4)
            {
                mem_cpy(mac,pRequest->frame+pos,4);
            }
            mem_cpy_right(pRequest->frame+pRequest->userdata_pos+cast_len+6,text,text_len);
            mem_cpy(pRequest->frame+pRequest->userdata_pos,sid,6);
            mem_cpy(pRequest->frame+pRequest->userdata_pos+cast_len+6+text_len,mac,4);
            len=esam_send_cmd(pRequest->frame+pRequest->userdata_pos,text_len+10+cast_len,pRequest->frame+pRequest->userdata_pos,SIZE_OF_APP_PROTOCOL_FRAME-pRequest->userdata_pos,&err_code);
            if(len>0)
            {
                pRequest->frame_len=pRequest->userdata_pos+len+3;
                pRequest->frame[OOP_POS_LEN]=(pRequest->frame_len-2);
                pRequest->frame[OOP_POS_LEN+1]=(pRequest->frame_len-2)>>8;
                fcs16(pRequest->frame+OOP_POS_LEN,(pRequest->frame[OOP_POS_ADDR]&0x0F)+6);
                fcs16(pRequest->frame+OOP_POS_LEN,pRequest->frame_len-4);
                pRequest->frame[pRequest->frame_len-1] = 0x16;
                pRequest->is_security=TRUE;
                app_protocol_handler(pRequest,pResp);
            }
            else
            {
                pRequest->security_info[1]=SECURITYRESULT_NONSYMMETRY_ERR;
                pRequest->security_info[0]=SECURITYRESULT_NONSYMMETRY_ERR;
                process_client_apdu_security_error(pRequest,pResp);
            }
            break;
        case RN:    //随机数
            head_size=pRequest->frame[pos++];
            mem_cpy(pRequest->frame+pRequest->userdata_pos,text,text_len);
            mem_cpy(pRequest->security_info+3,pRequest->frame+pos,head_size);
            pRequest->frame_len=pRequest->userdata_pos+text_len+3;
            pRequest->frame[OOP_POS_LEN]=(pRequest->frame_len-2);
            pRequest->frame[OOP_POS_LEN+1]=(pRequest->frame_len-2)>>8;
            fcs16(pRequest->frame+OOP_POS_LEN,(pRequest->frame[OOP_POS_ADDR]&0x0F)+6);
            fcs16(pRequest->frame+OOP_POS_LEN,pRequest->frame_len-4);
            pRequest->frame[pRequest->frame_len-1] = 0x16;
            pRequest->is_security=TRUE;
            app_protocol_handler(pRequest,pResp);
            break;
        case RN_MAC://随机数+数据MAC
                process_client_apdu_security_error(pRequest,pResp);
            break;
        case SID:  //安全标识
            //4字节SID
            mem_cpy(sid,pRequest->frame+pos,4);
            pos+=4;
            //附加域
            head_size=pRequest->frame[pos++];
            if(head_size==2)
            {
                mem_cpy(sid+4,pRequest->frame+pos,2);
            }
            else if(head_size==4)
            {
                mem_cpy(sid+4,pRequest->frame+pos,4);
            }
            pos+=head_size;
            mem_cpy_right(pRequest->frame+pRequest->userdata_pos+6,text,text_len);
            mem_cpy(pRequest->frame+pRequest->userdata_pos,sid,6);
            len=esam_send_cmd(pRequest->frame+pRequest->userdata_pos,text_len+6,pRequest->frame+pRequest->userdata_pos,SIZE_OF_APP_PROTOCOL_FRAME-pRequest->userdata_pos,&err_code);
            if(len>0)
            {
                pRequest->frame_len=pRequest->userdata_pos+len+3;
                pRequest->frame[OOP_POS_LEN]=(pRequest->frame_len-2);
                pRequest->frame[OOP_POS_LEN+1]=(pRequest->frame_len-2)>>8;
                fcs16(pRequest->frame+OOP_POS_LEN,(pRequest->frame[OOP_POS_ADDR]&0x0F)+6);
                fcs16(pRequest->frame+OOP_POS_LEN,pRequest->frame_len-4);
                pRequest->frame[pRequest->frame_len-1] = 0x16;
                pRequest->is_security=TRUE;
                app_protocol_handler(pRequest,pResp);
            }
            else
            {
                pRequest->security_info[1]=SECURITYRESULT_NONSYMMETRY_ERR;
                pRequest->security_info[0]=SECURITYRESULT_NONSYMMETRY_ERR;
                process_client_apdu_security_error(pRequest,pResp);
            }
            break;
        default:
            process_client_apdu_security_error(pRequest,pResp);
            break;            
    }
}

INT16U security_encode_apdu(objRequest *pRequest,objResponse *pResp)
{
    INT16U pos,idx,apdu_len;
    INT8U cmd[]={0x80,0x1C,0x00,0x11,0x00,0x00};
    INT8U mac[4];
    INT8U data[16];
    INT16U err_code;
    INT16S len;

    mem_set(mac,4,0x00);
    mem_set(data,16,0x00);
    pos=pResp->userdata_pos;

    len=pResp->pos-pResp->userdata_pos;
    apdu_len=len;
    idx=pResp->userdata_pos+10;

    if(pRequest->security_info[0]==0)//明文
    {
        if(pRequest->security_info[2]==RN_MAC)//上报使用
        {
            cmd[0]=0x80;
            cmd[1]=0x14;
            cmd[2]=0x01;
            cmd[3]=0x03;
        }
        else if(pRequest->security_info[2]==RN)           //读取数据
        {
            cmd[0]=0x80;
            cmd[1]=0x0E;
            cmd[2]=0x40;
            cmd[3]=0x02;
            len+=16;
            idx+=16;
        }
        else    //明文+MAC,设置参数
        {

        }
    }
    else if(pRequest->security_info[0]==1)//密文
    {
        cmd[3]=0x96;
        if(pRequest->security_info[2]==SID_MAC)
        {
            cmd[3]=0x97;
        }
    }
    else
    {
        pResp->frame[pos++]=APDU_SECURITY_RESPONSE;
        pResp->frame[pos++]=2;
        pResp->frame[pos++]=pRequest->security_info[1];
        return pos;
    }
    cosem_int162_bin(len,cmd+4);
    mem_cpy_right(pResp->frame+idx,pResp->frame+pResp->userdata_pos,len);
    if((pRequest->security_info[0]==0) && (pRequest->security_info[2]==RN))
    {
        mem_cpy(pResp->frame+pResp->userdata_pos+10,pRequest->security_info+3,16);
    }
    mem_cpy(pResp->frame+pResp->userdata_pos+4,cmd,6);

    if((pRequest->security_info[0]==0) && (pRequest->security_info[2]==RN_MAC))//上报使用
    {
        mem_cpy(pRequest->frame+pRequest->userdata_pos,pResp->frame+idx,apdu_len);
        len=esam_send_cmd(pResp->frame+pResp->userdata_pos+4,len+6,pResp->frame+pResp->userdata_pos+4,SIZE_OF_APP_PROTOCOL_FRAME-pResp->userdata_pos-4,&err_code);
    }
    else
    {
        len=esam_send_cmd(pResp->frame+pResp->userdata_pos+4,len+6,pResp->frame+pResp->userdata_pos+4,SIZE_OF_APP_PROTOCOL_FRAME-pResp->userdata_pos-4,&err_code);
    }
    if(len>0)
    {
        pResp->frame[pos++]=APDU_SECURITY_RESPONSE;
        pResp->frame[pos++]=pRequest->security_info[0];
        if(pRequest->security_info[2]==SID_MAC)
        {
            pos+=set_cosem_data_len(pResp->frame+pos,len-4);
            mem_cpy(pResp->frame+pos,pResp->frame+pResp->userdata_pos+4,len);
            pos+=len;
            mem_cpy(mac,pResp->frame+pos-4,4);
            pos-=4;
            pResp->frame[pos++]=1;//mac OPIION
            pResp->frame[pos++]=0;//mac choice
            pResp->frame[pos++]=4;//mac len
            mem_cpy(pResp->frame+pos,mac,4);
            pos+=4;
        }
        else if(pRequest->security_info[2]==RN)//明文+MAC
        {
            mem_cpy(mac,pResp->frame+pResp->userdata_pos+4,4);
            pos+=set_cosem_data_len(pResp->frame+pos,apdu_len);
            mem_cpy(pResp->frame+pos,pResp->frame+idx,apdu_len);
            pos+=apdu_len;
            pResp->frame[pos++]=1;//mac OPIION
            pResp->frame[pos++]=0;//mac choice
            pResp->frame[pos++]=4;//mac len
            mem_cpy(pResp->frame+pos,mac,4);
            pos+=4;
        }
        else if(pRequest->security_info[2]==RN_MAC)//随机数+数据MAC,上报使用
        {
            pResp->frame[pos-2]=APDU_SECURITY_REQUEST;
            mem_cpy(data,pResp->frame+pResp->userdata_pos+4,12);
            mem_cpy(mac,pResp->frame+pResp->userdata_pos+16,4);
            pos+=set_cosem_data_len(pResp->frame+pos,apdu_len);
            mem_cpy(pResp->frame+pos,pRequest->frame+pRequest->userdata_pos,apdu_len);
            pos+=apdu_len;
            pResp->frame[pos++]=2;//随机数+数据MAC
            pResp->frame[pos++]=12;//RN len
            mem_cpy(pResp->frame+pos,data,12);
            pos+=12;
            pResp->frame[pos++]=4;//mac len
            mem_cpy(pResp->frame+pos,mac,4);
            pos+=4;
        }
        else
        {
            pos+=set_cosem_data_len(pResp->frame+pos,len);
            mem_cpy(pResp->frame+pos,pResp->frame+pResp->userdata_pos+4,len);
            pos+=len;
            pResp->frame[pos++]=0;//mac option
        }
    }
    else
    {
        pResp->frame[pos++]=APDU_SECURITY_RESPONSE;
        pResp->frame[pos++]=2;
        pResp->frame[pos++]=pRequest->security_info[1];
    }
    return pos;

}
//INT16U encode_readmeter_frame(INT8U *frame,INT16U len,INT8U rand[16])
//{
    //INT16S size,header_size;
    //INT16U userdata_pos,pos;
    //INT8U buffer[64],header[4];
    //INT16U err_code;
//
    //buffer[0]=0x80;
    //buffer[1]=0x04;
    //buffer[2]=0x00;
    //buffer[3]=0x10;
    //buffer[4]=0x00;
    //buffer[5]=0x00;
    //size=esam_send_cmd(buffer,6,buffer,64,&err_code);
    //if(size!=16)return 0;
    //mem_cpy(rand,buffer,16);
    //userdata_pos=get_oopframe_addr_length(frame)+6;
    //if(len<userdata_pos+3) return 0;
    //len-=userdata_pos+3;
    //header_size=set_cosem_data_len(header,len);
    //mem_cpy_right(frame+userdata_pos+header_size+2,frame+userdata_pos,len);
    //frame[userdata_pos]=0x10;
    //frame[userdata_pos+1]=0x00;
    //mem_cpy(frame+userdata_pos+2,header,header_size);
    //pos=userdata_pos+header_size+len+2;
    //frame[pos++]=0x01;//随机数
    //frame[pos++]=size;
    //mem_cpy(frame+pos,buffer,size);
    //pos+=size;
    //
    //frame[OOP_POS_LEN] = (pos+2-1);
    //frame[OOP_POS_LEN+1] = ((pos+2-1)>>8);
    ////计算HCS校验位
    //fcs16(frame+OOP_POS_LEN,11);
    ////计算FCS校验位
    //fcs16(frame+OOP_POS_LEN,pos-1);
    //pos += 2;
    //frame[pos++]=0x16;
    //return pos;
//
//}

INT16U encode_readmeter_frame(INT8U *frame,INT16U len)
{
    INT16S size=16,header_size;
    INT16U userdata_pos,pos;
    INT8U buffer[64],header[4];
    INT16U err_code;

//    rs232_debug_info(frame,len);

    buffer[0]=0x80;
    buffer[1]=0x04;
    buffer[2]=0x00;
    buffer[3]=0x10;
    buffer[4]=0x00;
    buffer[5]=0x00;
//    size=esam_send_cmd(buffer,6,buffer,64,&err_code);
//    if(size!=16)return 0;
//    mem_cpy(rand,buffer,16);
//    rs232_debug_info("\xA4\xA6",2);
    userdata_pos=get_oopframe_addr_length(frame)+6;
    if(len<userdata_pos+3) return 0;
//    rs232_debug_info("\xA4\xA7",2);
    len-=userdata_pos+3;
    header_size=set_cosem_data_len(header,len);
    mem_cpy_right(frame+userdata_pos+header_size+2,frame+userdata_pos,len);
    frame[userdata_pos]=0x10;
    frame[userdata_pos+1]=0x00;
    mem_cpy(frame+userdata_pos+2,header,header_size);
    pos=userdata_pos+header_size+len+2;
    frame[pos++]=0x01;//随机数
    frame[pos++]=size;
    mem_set(frame+pos,size,0x01);
    pos+=size;
    
    frame[OOP_POS_LEN] = (pos+2-1);
    frame[OOP_POS_LEN+1] = ((pos+2-1)>>8);
    //计算HCS校验位
    fcs16(frame+OOP_POS_LEN,11);
    //计算FCS校验位
    fcs16(frame+OOP_POS_LEN,pos-1);
    pos += 2;
    frame[pos++]=0x16;
//    rs232_debug_info(frame,pos);
    return pos;

}
//INT16U decode_readmeter_frame(INT8U *frame,INT16U len,INT8U rand[16])
INT16U decode_readmeter_frame(INT8U *frame,INT16U len)
{
    INT16S size;
    INT16U userdata_pos,pos,data_pos,text_len,idx,frame_len;
    INT8U buffer[64],meter_no[16],head_size;
    INT16U err_code;

    //去FE
    for(idx=0;idx<8;idx++)
    {
        if(*frame==0xFE)
        {
            frame++;
            len--;
        }
    }
    mem_set(meter_no,8,0x00);
    size=get_oopframe_addr_length(frame);

    if(size==0)return 0;
    mem_cpy(meter_no+2,frame+OOP_POS_ADDR+1,6);


    userdata_pos=get_oopframe_addr_length(frame)+6;
    
    if(frame[userdata_pos]!=0x90)return 0;//非安全传输报文不处理
    if(frame[userdata_pos+1]!=0x00)return 0;//不是明文，目前没做处理

    frame_len = 0;
    frame_len += frame[1];
    frame_len += (frame[2]&0x3F) << 8;
    text_len=get_cosem_data_len(frame+userdata_pos+2,&head_size);//获取报文长度
    if((text_len > (frame_len - userdata_pos - 7) )) return 0;//判断收到的报文是否正确
//    mem_cpy(frame+userdata_pos+2+head_size+text_len,frame+userdata_pos+2+head_size+text_len+3,4);//将MAC放在数据末尾，原数据中间有3个字节01 00 04

//    len=text_len+4;
//
//    data_pos=userdata_pos+6+8+16;//cmd+esam id+rand
//    mem_cpy_right(frame+data_pos,frame+userdata_pos+2+head_size,len);
//
//    len+=8+16;
//
//    pos=userdata_pos;
//    frame[pos++]=0x80;
//    frame[pos++]=0x0E;
//    frame[pos++]=0x48;
//    frame[pos++]=0x87;
//    frame[pos++]=len>>8;
//    frame[pos++]=len;
//    mem_cpy(frame+pos,meter_no,8);
//    pos+=8;
//
//    mem_cpy(frame+pos,rand,16);
////    pos+=16;
//
////    pos+=len;
//    
//    //rs232_debug_info(frame+userdata_pos,len+6,buffer,64);
////    size=esam_send_cmd(frame+userdata_pos,len+6,buffer,64,&err_code);
//    if(size<0)return 0;
    data_pos = userdata_pos+2+head_size ;
    mem_cpy(frame+userdata_pos,frame+data_pos,text_len);
    pos=userdata_pos+ text_len;

    frame[OOP_POS_LEN] = (pos+2-1);
    frame[OOP_POS_LEN+1] = ((pos+2-1)>>8);
    //计算HCS校验位
    fcs16(frame+OOP_POS_LEN,11);
    //计算FCS校验位
    fcs16(frame+OOP_POS_LEN,pos-1);
    pos += 2;
    frame[pos++]=0x16;

//    rs232_debug_info(frame,pos);
    return pos;
}
#endif

//void process_client_apdu_connect(objRequest *pRequest,objResponse *pResp)
//{
//    extern const INT8U PROTOCOL_CONFORMANCE[];
//    extern const INT8U FUNCTION_CONFORMANCE[];
//    extern const INT8U dev_ver_info_default[];
//    extern const INT8U dev_ver_info_3_02[];
//
//    INT8U get_protocol_conformance_count();
//    INT8U get_function_conformance_count();
//    INT8U load_link_password(INT8U *password,INT8U max_len);
//
//    INT16U pos,idx,p_count,f_count,count,ciphertext_len,signature_len;
//    INT16S len;
//    INT32U protocol;
//    INT8U bit_info[16];
//    INT8U connect_mode,pwd_len,pwd_len2,head_size;
//    INT8U password[34];
//    INT8U *ciphertext,*signature,*buffer;
//    INT8U *dev_ver_info;
//
//    if(gSystemInfo.hardware_version == 0x02)
//    {
//        dev_ver_info = (INT8U*)dev_ver_info_3_02;
//    }
//    else
//    {
//        dev_ver_info = (INT8U*)dev_ver_info_default;
//    }
//    
//    mem_set(password,34,0x00);
//
//    //设置控制码：DIR=1,PRM=0, FUNC=8  响应报文，用户数据
//    pResp->frame[OOP_POS_CTRL] = CTRLFUNC_DIR_SET | CTRLFUNC_PRM_SET |  CTRL_FUNC_USERDATA;
//    pResp->frame[pResp->pos++] =  SERVER_APDU_CONNECT; //填充应答服务类型
//    pResp->frame[pResp->pos++] =  pRequest->frame[pRequest->userdata_pos+1];        //填充应答服务参数
//
//    pos=pRequest->userdata_pos+1;
//
//    pos++;//PIID
//    //厂商版本信息
//    mem_cpy(pResp->frame+pResp->pos,(INT8U*)dev_ver_info+4,4);
//    pResp->pos+=4;
//    mem_cpy(pResp->frame+pResp->pos,(INT8U*)dev_ver_info+10,4);
//    pResp->pos+=4;
//    mem_cpy(pResp->frame+pResp->pos,(INT8U*)dev_ver_info+16,6);
//    pResp->pos+=6;
//    mem_cpy(pResp->frame+pResp->pos,(INT8U*)dev_ver_info+24,4);
//    pResp->pos+=4;
//    mem_cpy(pResp->frame+pResp->pos,(INT8U*)dev_ver_info+30,6);
//    pResp->pos+=6;
//    mem_cpy(pResp->frame+pResp->pos,(INT8U*)dev_ver_info+38,8);
//    pResp->pos+=8;
//
//    //协议版本号
//    protocol=cosem_bcd2_int16u(pRequest->frame+pos);
//    pos+=2;
//
//    if(protocol!=PROTOCOL_VER)
//    {
//        //协议不相同，目前不做处理，只按自己协议处理
//    }
//    cosem_int162_bcd(PROTOCOL_VER,pResp->frame+pResp->pos);
//    pResp->pos+=2;
//   //协议一致性块,取交集
//    count=64;
//    p_count=get_protocol_conformance_count();
//    mem_set(bit_info,16,0x00);
//    for(idx=0;idx<p_count;idx++)
//    {
//        if((PROTOCOL_CONFORMANCE[idx]<count) && get_oop_bit_value(pRequest->frame+pos,8,PROTOCOL_CONFORMANCE[idx]))
//        {
//            set_oop_bit_value(bit_info,8,PROTOCOL_CONFORMANCE[idx]);
//        }
//        else
//        {
//            clr_oop_bit_value(bit_info,8,PROTOCOL_CONFORMANCE[idx]);
//        }
//    }
//    pos+=count/8;
//
//    mem_cpy(pResp->frame+pResp->pos,bit_info,8);
//    pResp->pos+=8;
//
//   //功能一致性块,取交集
//    count=128;
//    f_count=get_function_conformance_count();
//    mem_set(bit_info,16,0x00);
//    for(idx=0;idx<f_count;idx++)
//    {
//        if((FUNCTION_CONFORMANCE[idx]<count) && get_oop_bit_value(pRequest->frame+pos,8,FUNCTION_CONFORMANCE[idx]))
//        {
//            set_oop_bit_value(bit_info,8,FUNCTION_CONFORMANCE[idx]);
//        }
//        else
//        {
//            clr_oop_bit_value(bit_info,8,FUNCTION_CONFORMANCE[idx]);
//        }
//    }
//    pos+=count/8;
//
//    mem_cpy(pResp->frame+pResp->pos,bit_info,16);
//    pResp->pos+=16;
//
//   //客户机发送帧最大尺寸
//    g_app_link_info.client_send_frame_size =cosem_bin2_int16u(pRequest->frame+pos);
//    pos+=2;
//     if(g_app_link_info.client_send_frame_size>MAX_SEND_FRAME_SIZE)
//     {
//        g_app_link_info.client_send_frame_size=MAX_SEND_FRAME_SIZE;
//     }
//    cosem_int162_bin(g_app_link_info.client_send_frame_size,pResp->frame+pResp->pos);
//    pResp->pos+=2;
//   //客户机接收帧最大尺寸
//    g_app_link_info.client_recv_frame_size =cosem_bin2_int16u(pRequest->frame+pos);
//    if(g_app_link_info.client_recv_frame_size>MAX_RECV_FRAME_SIZE)
//    {
//        g_app_link_info.client_recv_frame_size=MAX_RECV_FRAME_SIZE;
//    }
//    if(pRequest->pChannel!=NULL)
//    {
//        ((objProtocolChannel *)pRequest->pChannel)->max_send_frame_size = g_app_link_info.client_recv_frame_size;//最大的发送帧最大尺寸
//    }
//    pos+=2;
//    cosem_int162_bin(g_app_link_info.client_send_frame_size,pResp->frame+pResp->pos);
//    pResp->pos+=2;
//
//   //客户机接收最大尺寸帧个数
//   g_app_link_info.client_recv_frame_count=pRequest->frame[pos++];
//   if(g_app_link_info.client_recv_frame_count>MAX_RECV_FRAME_COUNT)
//   {
//    g_app_link_info.client_recv_frame_count=MAX_RECV_FRAME_COUNT;
//   }
//   pResp->frame[pResp->pos++]=g_app_link_info.client_recv_frame_count;
//
//   //客户机最大可处理APDU尺寸
//    g_app_link_info.client_apdu_max_size =cosem_bin2_int16u(pRequest->frame+pos);
//    if(pRequest->pChannel!=NULL)
//    {
//         ((objProtocolChannel *)pRequest->pChannel)->max_apdu_size = cosem_bin2_int16u(pRequest->frame+pos); //最大的apdu，终端组织发送报文时，apdu的最大尺寸
//    }   
//    pos+=2;
//    if(g_app_link_info.client_apdu_max_size>MAX_APDU_SIZE)
//    {
//        g_app_link_info.client_apdu_max_size=MAX_APDU_SIZE;
//    }
//   cosem_int162_bin(g_app_link_info.client_apdu_max_size,pResp->frame+pResp->pos);
//   pResp->pos+=2;
//
//
//   //期望的应用连接超时时间
//   g_app_link_info.link_time_out=cosem_bin2_int32u(pRequest->frame+pos);
//    if(pRequest->pChannel!=NULL)
//    {   
//        ((objProtocolChannel *)pRequest->pChannel)->link_start_timer = system_get_tick10ms();
//        ((objProtocolChannel *)pRequest->pChannel)->link_time_out = cosem_bin2_int32u(pRequest->frame+pos); //期望的应用连接超时时间
//    }
//   pos+=4;
//   cosem_int32u2_bin(g_app_link_info.link_time_out,pResp->frame+pResp->pos);
//   pResp->pos+=4;
//   //安全认证请求对象
//   connect_mode=pRequest->frame[pos++];
//   //pResp->frame[pResp->pos++]=2;
//   switch(connect_mode)
//   {
//    case 0://公共连接    [0] NullSecurity，
//        pResp->frame[pResp->pos++]=SECURITYRESULT_OK;
//        pResp->frame[pResp->pos++]=0;
//        break;
//    case 1://一般密码    [1] PasswordSecurity，
//        //读取终端密码
//        pwd_len2=read_custom_param(CONST_LINK_PWD,password);//load_link_password(password,34);
//        pwd_len=pRequest->frame[pos++];
//        if((pwd_len2==pwd_len) && (compare_string(pRequest->frame+pos,password,pwd_len)==0))
//        {
//            pResp->frame[pResp->pos++]=SECURITYRESULT_OK;
//        }
//        else
//        {
//            pResp->frame[pResp->pos++]=SECURITYRESULT_PWD_ERR;
//        }
//        pResp->frame[pResp->pos++]=0;
//        break;
//    #if defined __ENABLE_ESAM2__
//    case 2://对称加密    [2] SymmetrySecurity
//        ciphertext_len=get_cosem_data_len(pRequest->frame+pos,&head_size);
//        pos+=head_size;
//        ciphertext=pRequest->frame+pos;
//        pos+=ciphertext_len;
//        signature_len=get_cosem_data_len(pRequest->frame+pos,&head_size);
//        pos+=head_size;
//        signature=pRequest->frame+pos;
////        pos+=signature_len;
//        mem_cpy(ciphertext+ciphertext_len,signature,signature_len);
//
//
//        pResp->frame[pResp->pos++]=SECURITYRESULT_OK;//默认认证成功
//        pResp->frame[pResp->pos++]=1;//默认认证成功
//        buffer=pResp->frame+pResp->pos+1;
//        len=esam_session_auth(ciphertext,ciphertext_len+signature_len,buffer,pResp->max_reply_pos-pResp->frame_len-1);
//        if(len<112)
//        {
//            pResp->frame[pResp->pos-2]=SECURITYRESULT_NONSYMMETRY_ERR;
//            pResp->frame[pResp->pos-1]=0;
//        }
//        else
//        {
//            pResp->frame[pResp->pos++]=len-64;
//            pResp->pos+=len-64;
//            mem_cpy_right(pResp->frame+pResp->pos+1,pResp->frame+pResp->pos,64);
//            pResp->frame[pResp->pos++]=64;
//            pResp->pos+=64;
//        }
//        break;
//    case 3://数字签名    [3] SignatureSecurity
//        ciphertext_len=get_cosem_data_len(pRequest->frame+pos,&head_size);
//        pos+=head_size;
//        ciphertext=pRequest->frame+pos;
//        pos+=ciphertext_len;
//        
//        signature_len=get_cosem_data_len(pRequest->frame+pos,&head_size);
//        pos+=head_size;
//        signature=pRequest->frame+pos;
////        pos+=signature_len;
//        
//        pResp->frame[pResp->pos++]=SECURITYRESULT_OK;//默认认证成功
//        pResp->frame[pResp->pos++]=1;//默认认证成功
//        mem_cpy(ciphertext+ciphertext_len,signature,signature_len);
//        buffer=pResp->frame+pResp->pos+1;
//        len=esam_session_auth(ciphertext,ciphertext_len+signature_len,buffer,pResp->max_reply_pos-pResp->pos-1);
//        if(len<112)
//        {
//            pResp->frame[pResp->pos-2]=SECURITYRESULT_NONSYMMETRY_ERR;
//            pResp->frame[pResp->pos-1]=0;
//        }
//        else
//        {
//            pResp->frame[pResp->pos++]=len-64;
//            pResp->pos+=len-64;
//            mem_cpy_right(pResp->frame+pResp->pos+1,pResp->frame+pResp->pos,64);
//            pResp->frame[pResp->pos++]=64;
//            pResp->pos+=64;
//        }
//        break;
//    #endif
//    default:
//        pResp->frame[pResp->pos++]=SECURITYRESULT_PWD_ERR;
//        pResp->frame[pResp->pos++]=0;
//   }
////   rs232_debug_info(pResp->frame,pResp->pos);
//   app_send_response_frame(pRequest,pResp,TRUE);
//
//}
//void process_client_apdu_disconnect(objRequest *pRequest,objResponse *pResp)
//{
//    //设置控制码：DIR=1,PRM=0, FUNC=8  响应报文，用户数据
//    pResp->frame[OOP_POS_CTRL] = CTRLFUNC_DIR_SET | CTRLFUNC_PRM_SET |  CTRL_FUNC_USERDATA;
//    pResp->frame[pResp->pos++] =  SERVER_APDU_RELEASE_RESP; //填充应答服务类型
//    pResp->frame[pResp->pos++] =  pRequest->frame[pRequest->userdata_pos+1];        //填充应答服务参数
//    pResp->frame[pResp->pos++] =  0;//断开成功
//    
//   //客户机发送帧最大尺寸
//    g_app_link_info.client_send_frame_size=MAX_SEND_FRAME_SIZE;
//    if(pRequest->pChannel!=NULL)
//    {
//        ((objProtocolChannel *)pRequest->pChannel)->max_send_frame_size = g_app_link_info.client_send_frame_size;
//    }
//     //客户机接收帧最大尺寸
//    g_app_link_info.client_recv_frame_size =MAX_RECV_FRAME_SIZE;
//   //客户机接收最大尺寸帧个数
//    g_app_link_info.client_recv_frame_count=MAX_RECV_FRAME_COUNT;
//
//   //客户机最大可处理APDU尺寸
//    g_app_link_info.client_apdu_max_size =MAX_APDU_SIZE;
//    if(pRequest->pChannel!=NULL)
//    {
//         ((objProtocolChannel *)pRequest->pChannel)->max_apdu_size = g_app_link_info.client_apdu_max_size; //最大的apdu，终端组织发送报文时，apdu的最大尺寸
//    }
//   //期望的应用连接超时时间
//    g_app_link_info.link_time_out=0;
//    if(pRequest->pChannel!=NULL)
//    {
//        ((objProtocolChannel *)pRequest->pChannel)->link_time_out = g_app_link_info.link_time_out; //期望的应用连接超时时间
//    }
//    app_setFrameLenThenSendFrame(pRequest,pResp);
//
//}
BOOLEAN check_safe_config(INT16U obis,INT8U operate,INT8U *security_info)
{
    INT16U config_word;
    INT8U safe_word,check_safe_word;
    BOOLEAN ret;

    if((security_info[0]==1) )//密文
    {
        if((security_info[2]==0) || (security_info[2]==2))//MAC
        {
            check_safe_word=0x01;
        }
        else
        {
            check_safe_word=0x02;
        }
    }
    else
    {
        if((security_info[2]==0)|| (security_info[2]==1) || (security_info[2]==2))//MAC
        {
            check_safe_word=0x04;
        }
        else
        {
            check_safe_word=0x08;
        }
    }
    ret=FALSE;
    //获取安全模式字
    config_word=get_esam_safe_config(obis);
    switch(operate)
    {
        case OP_GET:
            safe_word=(config_word>>12) & 0x0F;
            if(safe_word==0)
            {
                safe_word=0x08;
            }
            ret=check_safe_word<=safe_word;
            break;
        case OP_SET:
            safe_word=(config_word>>8) & 0x0F;;
            if(safe_word==0)
            {
                safe_word=0x08;
            }
            ret=check_safe_word<=safe_word;
            break;
        case OP_ACTION:
            safe_word=(config_word>>4) & 0x0F;
            if(safe_word==0)
            {
                safe_word=0x08;
            }
            ret=check_safe_word<=safe_word;
            break;
        case OP_PROXY://终端本身，不存在代理操作终端的情况，这里目前没意义
            safe_word=config_word & 0x0F;
            ret=check_safe_word<=safe_word;
            break;
        default:
            ret = TRUE;
    }
    if(get_system_flag(SYS_ESAM,SYS_FLAG_BASE))
    {
        return ret;
    }
    return TRUE;
}

