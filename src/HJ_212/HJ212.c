#include "HJ212.h"
#include "main_include.h"
unsigned int CRC16_Checkout(const unsigned char *puchMsg, unsigned int usDataLen)
{
	unsigned int i, j, crc_reg, check;
	crc_reg = 0xFFFF;
	for (i = 0; i < usDataLen; i++)
	{
		crc_reg = (crc_reg >> 8) ^ puchMsg[i];
		for (j = 0; j < 8; j++)
		{
			check = crc_reg & 0x0001;
			crc_reg >>= 1;
			if (check == 0x0001)
			{
				crc_reg ^= 0xA001;
			}
		}
	}
	return crc_reg;
}
void hex2dec(INT16U data, INT8U *buf)
{
	INT8U pos = 0;
	*(buf + pos) = data / 1000;
	data -= *(buf + pos) * 1000;
	pos++;
	*(buf + pos) = data / 100;
	data -= *(buf + pos) * 100;
	pos++;
	*(buf + pos) = data / 10;
	data -= *(buf + pos) * 10;
	pos++;
	*(buf + pos) = data;
	for (pos = 0; pos < 4; pos++)
	{
		*(buf + pos) += '0';
	}

}
void hex2ascii(INT16U data, INT8U *buf)
{
	INT8U tmp;
	tmp = (*((INT8U*)&data + 1) >> 4) & 0x0F;
	if (tmp > 9)
	{
		*buf = (tmp - 0xA) + 'A';
	}
	else
	{
		*buf = tmp + '0';
	}
	tmp = (*((INT8U*)&data + 1)) & 0x0F;
	if (tmp > 9)
	{
		*buf = (tmp - 0xA) + 'A';
	}
	else
	{
		*buf = tmp + '0';
	}
	tmp = (*((INT8U*)&data) >> 4) & 0x0F;
	if (tmp > 9)
	{
		*buf = (tmp - 0xA) + 'A';
	}
	else
	{
		*buf = tmp + '0';
	}
	tmp = (*((INT8U*)&data)) & 0x0F;
	if (tmp > 9)
	{
		*buf = (tmp - 0xA) + 'A';
	}
	else
	{
		*buf = tmp + '0';
	}
}

void num2ascii(INT8U *buf,INT16U num)
{
	if (num / 1000 > 0)
	{
		*buf = num / 1000 + '0';
		num -= (*buf - '0') * 1000;
		num2ascii(buf+1, num);
	}
	else if (num / 100 > 0)
	{
		*buf = num / 100 + '0';
		num -= (*buf - '0') * 100;
		num2ascii(buf+1, num);
	}
	else if (num / 10 > 0)
	{
		*buf = num / 10 + '0';
		num -= (*buf-'0') * 10;
		num2ascii(buf + 1, num);
	}
	else
	{
		*buf = num + '0';
	}
}
/**
*构造请求编码QN，也就是时间戳
*/
void make212_QN(INT8U *QN, tagDatetime dt)
{
	INT8U pos = 0;
	INT16U ms = 0;
	memcpy(QN+pos, "QN=", 3);
	pos = strlen(QN);
	num2ascii(QN+pos, dt.century);
	pos = strlen(QN);
	num2ascii(QN + pos, dt.year);
	pos = strlen(QN);
	if (dt.month < 10)
	{
		*(QN + pos) = '0';
		pos++;
	}
	num2ascii(QN + pos, dt.month);
	pos = strlen(QN);
	num2ascii(QN + pos, dt.day);
	pos = strlen(QN);
	num2ascii(QN + pos, dt.hour);
	pos = strlen(QN);
	num2ascii(QN + pos, dt.minute);
	pos = strlen(QN);
	num2ascii(QN + pos, dt.second);
	ms = (dt.msecond_h << 8) + dt.msecond_l;
	pos = strlen(QN);
	num2ascii(QN + pos, ms);
}
void make212_ST(INT8U *buf, INT8U st)
{
	INT8U pos = 0;
	memcpy(buf + pos, "ST=", 3);
	pos = strlen(buf);
	hex2ascii(buf + pos, st);
}
void make212_CN(INT8U *buf, INT16U cn)
{
	INT8U pos = 0;
	memcpy(buf + pos, "CN=", 3);
	pos = strlen(buf);
	hex2ascii(buf + pos, cn);
//	sprintf();
}
//void make212_passwd(INT8U *buf, INT8U *passwd);
//void make212_mn(INT8U *buf, INT8U * mn);
//void make212_flag(INT8U *buf, INT8U flag);
//void make212_pnum(INT8U *buf, INT16U pnum);
//void make212_pno(INT8U *buf, INT16U pno);
/**
*  构建数据段的数据
*  |***********************************************************************************************************************|
*  | 请求编码(QN）|系统编码(ST)|命令编码(CN)|访问密码|设备唯一标识(MN)|拆分包及应答标志(FLAG)|总包数(PNUM）|包号(PNO)|指令参数(CP)|
*  |***********************************************************************************************************************|
*  return 构造的数据段的长度
*/
INT16U  make_data_seg(INT8U* buf,const INT16U ST,const INT16U CN,const INT8U *passwd,const  INT8U* MN, INT8U Flag, INT16U PNUM, INT16U PNO,const INT8U* CP)
{
	INT16U  unit_size = 0,pos =0;
	INT8U flag_size = 0,flag_loop=0,flag_value=0;
	tagDatetime dt;
	//dt.century = 20;
	//dt.year = 16;
	//dt.month = 8;
	//dt.day = 1;
	//dt.hour = 8;
	//dt.minute = 58;
	//dt.second = 57;
	//dt.msecond_h = 0;
	//dt.msecond_l = 0xDF;
	tpos_datetime(&dt);	   //时间中没有世纪这个时间。
	/***QN***/
	sprintf(buf + pos, "QN=%02d%02d%02d%02d%02d%02d%02d%03d;", 20, dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second, (dt.msecond_h<<8)+dt.msecond_l);
	pos = strlen(buf);
	///***ST****/
	sprintf(buf + pos, "ST=%02d;", ST);
	pos = strlen(buf);
	///***CN****/
	sprintf(buf + pos, "CN=%04d;", CN);
	pos = strlen(buf);
	///***passWD****/
	sprintf(buf + pos, "PW=");
	pos = strlen(buf);
	unit_size = strlen(passwd);
	memcpy(buf + pos, passwd, unit_size);
	pos += unit_size;
	sprintf(buf + pos, ";");
	pos = strlen(buf);
	///***MN****/
	sprintf(buf + pos, "MN=");
	pos = strlen(buf);
	unit_size = strlen(MN);
	memcpy(buf + pos, MN, unit_size);
	pos += unit_size;
	sprintf(buf + pos, ";");
	pos = strlen(buf);
	///***FLAG****/
	sprintf(buf + pos, "Flag=%d;", Flag);
	pos = strlen(buf);
	//////计算Flag中实际有用的长度
	if (flag_value && 0x20 == 0x20) //有后续包
	{
		/***PNUM****/
		sprintf(buf + pos, "PNUM=%d;", PNUM);
		pos = strlen(buf);
		/***PNO****/
		sprintf(buf + pos, "PNO=%d;", PNO);
		pos = strlen(buf);
	}
	///***CP****/
	sprintf(buf + pos, "CP=&&", Flag);
	pos = strlen(buf);
	unit_size = strlen(CP);
	memcpy(buf + pos, CP, unit_size);
	pos += unit_size;
	sprintf(buf + pos, "&&", Flag);
	pos = strlen(buf);
	return pos;
}
void makeHJ212Frame(INT8U *buf, const INT8U *data_seg, INT16U data_seg_len, INT16U *frameLen)
{
	INT16U pos = 0, crcseg;
	INT8U i = 0;
	crcseg = CRC16_Checkout(data_seg, data_seg_len);  //因为crc校验使用的是数据段，所以要提前计算。
	sprintf(buf, "##%04d", data_seg_len);
	pos += 6;
	memcpy(buf + pos, data_seg, data_seg_len);
	pos += data_seg_len;
	sprintf(buf+pos, "%04X", crcseg);
	pos += 4;
	*(buf + pos) = 0x0D;
	pos++;
	*(buf + pos) = 0x0A;
	pos++;
	*frameLen = pos;
}

INT8S HJ212_ertu_request_time()
{
	INT16U 	 frameLen=0,data_seg_len =0;
	if(HJ212_handle_state!=0)
	{
		return -1;
	}
	else
	{
		mem_set(HJ212_request_buf,1000,0x00);
		data_seg_len = make_data_seg(HJ212_request_buf+10,32,1013,"123456","000000000000000000000011", 5, NULL, NULL,"PolId=w01018");
		makeHJ212Frame(HJ212_request_buf, HJ212_request_buf+10,  data_seg_len, &frameLen);
		HJ212_handle_state =1;
		return frameLen;
	}
}
/************************************************************************/
/* 判断HJ212协议的帧是不是接受完毕                                        */
/************************************************************************/
INT8U is_HJ212_frame_ready(const INT8U* frame,INT16U frame_len)
{
	INT16U pos=0,crcseg =0;
	INT8U tmp_buf[5];
	pos=6; //从QN开始计算校验值。
	crcseg = CRC16_Checkout(frame+pos, frame_len);  //因为crc校验使用的是数据段，所以要提前计算。
	sprintf(tmp_buf,"%X",crcseg);
	pos += frame_len;
	if(mem_is_same(frame+pos,tmp_buf,4))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
/************************************************************************/
/* 参数命令                                                                     */
/************************************************************************/
static void handle_params_command(objRequest* pRequest,objResponse *pResp)
{
	uint8_t update_ertu_datetime(uint8_t force_flag);	
	
	INT16U frameLen=0,data_seg_len =0;
	INT16U pos=0;
	volatile INT8U time_set[7],time_loop=0;
	INT8U str[50]={0},idx=0,cs=0,pos1=0,tmp[50]={0};
	tagDatetime dt;	
	INT8U passwd[6];	
	if(gSystemHJ212.CN == 1012)	//主站设置设备时间
	{
		if((pos=str_find(pRequest->frame,pRequest->frame_len,"SystemTime=",11))!=-1)
		{
			pos+=11;
			if((*(pRequest->frame+pos)=='2')&&(*(pRequest->frame+pos+1)=='0'))
			{
				pos+=2;
				for(time_loop=0;time_loop<6;time_loop++)
				{
					time_set[time_loop] =  byte2BCD((*(pRequest->frame+pos)-'0')*10+(*(pRequest->frame+pos+1)-'0')); //年
					pos+=2;					
				}
				time_set[6]=weekDay(BCD2byte(time_set[0]),BCD2byte(time_set[1]),BCD2byte(time_set[2]));//week

				idx = 0; //组报文初始位置
				str[idx++] =0x68;
				str[idx++] =gSystemInfo.meter_no[0];str[idx++] =gSystemInfo.meter_no[1];str[idx++] =gSystemInfo.meter_no[2];
				str[idx++] =gSystemInfo.meter_no[3];str[idx++] =gSystemInfo.meter_no[4];str[idx++] =gSystemInfo.meter_no[5];
				str[idx++] = 0x68;
				str[idx++] = 0x14;
				str[idx++] = 0x10;
				str[idx++] = 0x34;
				str[idx++] = 0x34;
				str[idx++] = 0x33;
				str[idx++] = 0x37;
				str[idx++] =0x35;str[idx++] =0x89;str[idx++] =0x67;str[idx++] =0x45;
				str[idx++] =0xAB;str[idx++] =0x89;str[idx++] =0x67;str[idx++] =0x45;
				str[idx++] =time_set[6] +0x33;str[idx++] = time_set[2] +0x33;
				str[idx++] =time_set[1] +0x33;str[idx++] = time_set[0] +0x33;
				cs = 0;
				for(pos1 =0;pos1<idx;pos1++) cs += str[pos1];
				str[idx++] = cs;
				str[idx++] =0x16;
				app_trans_send_meter_frame(str,28, tmp,200,100);
							
				idx = 0; //组报文初始位置
				str[idx++] =0x68;
				str[idx++] =gSystemInfo.meter_no[0];str[idx++] =gSystemInfo.meter_no[1];str[idx++] =gSystemInfo.meter_no[2];
				str[idx++] =gSystemInfo.meter_no[3];str[idx++] =gSystemInfo.meter_no[4];str[idx++] =gSystemInfo.meter_no[5];
				str[idx++] = 0x68;
				str[idx++] = 0x14;
				str[idx++] = 0x0F;
				str[idx++] = 0x35;
				str[idx++] = 0x34;
				str[idx++] = 0x33;
				str[idx++] = 0x37;
				str[idx++] =0x35;str[idx++] =0x89;str[idx++] =0x67;str[idx++] =0x45;
				str[idx++] =0xAB;str[idx++] =0x89;str[idx++] =0x67;str[idx++] =0x45;
				str[idx++] = time_set[5] +0x33;
				str[idx++] = time_set[4] +0x33;
				str[idx++] = time_set[3] +0x33;
				cs = 0;
				for(pos1 =0;pos1<idx;pos1++) cs += str[pos1];
				str[idx++] = cs;
				str[idx++] =0x16;
				app_trans_send_meter_frame(str,27, tmp,200,100);	
									
				update_ertu_datetime(1);
			}	
		}	
		mem_set(HJ212_request_buf,1000,0x00);
		data_seg_len = make_data_seg(HJ212_request_buf+10,91,9011,gSystemInfo.passwd,"000000000000000000000011", 4, NULL, NULL,"QnRtn=1");
		makeHJ212Frame(HJ212_request_buf, HJ212_request_buf+10,  data_seg_len, &frameLen);
		mem_cpy(pResp->frame,HJ212_request_buf,frameLen);
		pResp->frame_len =frameLen;
		app_send_ReplyFrame(pResp);	
		DelayNmSec(2000);
		mem_set(HJ212_request_buf,1000,0x00);
		data_seg_len = make_data_seg(HJ212_request_buf+10,91,9012,gSystemInfo.passwd,"000000000000000000000011", 4, NULL, NULL,"ExeRtn=1");
		makeHJ212Frame(HJ212_request_buf, HJ212_request_buf+10,  data_seg_len, &frameLen);
		mem_cpy(pResp->frame,HJ212_request_buf,frameLen);
		pResp->frame_len =frameLen;
		app_send_ReplyFrame(pResp);		
	}
	if(gSystemHJ212.CN == 1011)//主站提取现场机时间
	{
		mem_set(HJ212_request_buf,1000,0x00);
		data_seg_len = make_data_seg(HJ212_request_buf+10,91,9011,gSystemInfo.passwd,"000000000000000000000011", 4, NULL, NULL,"QnRtn=1");
		makeHJ212Frame(HJ212_request_buf, HJ212_request_buf+10,  data_seg_len, &frameLen);
		mem_cpy(pResp->frame,HJ212_request_buf,frameLen);
		pResp->frame_len =frameLen;
		app_send_ReplyFrame(pResp);	
		DelayNmSec(2000);
		mem_set(HJ212_request_buf,1000,0x00);
		mem_set(str,50,0);
		tpos_datetime(&dt);	   //时间中没有世纪这个时间。
		/***QN***/
		sprintf(str, "SystemTime=%02d%02d%02d%02d%02d%02d%02d", 20, dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
		data_seg_len = make_data_seg(HJ212_request_buf+10,21,1011,gSystemInfo.passwd,"000000000000000000000011", 4, NULL, NULL,str);
		makeHJ212Frame(HJ212_request_buf, HJ212_request_buf+10,  data_seg_len, &frameLen);
		mem_cpy(pResp->frame,HJ212_request_buf,frameLen);
		pResp->frame_len =frameLen;
		app_send_ReplyFrame(pResp);	
		DelayNmSec(2000);		
		mem_set(HJ212_request_buf,1000,0x00);
		data_seg_len = make_data_seg(HJ212_request_buf+10,91,9012,gSystemInfo.passwd,"000000000000000000000011", 4, NULL, NULL,"ExeRtn=1");
		makeHJ212Frame(HJ212_request_buf, HJ212_request_buf+10,  data_seg_len, &frameLen);
		mem_cpy(pResp->frame,HJ212_request_buf,frameLen);
		pResp->frame_len =frameLen;
		app_send_ReplyFrame(pResp);		
		
	}
	if(gSystemHJ212.CN== 1072)	  //设置现场机访问密码
	{
		if((pos=str_find(pRequest->frame,pRequest->frame_len,"NewPW=",6))!=-1)
		{
			pos+=6;
			mem_cpy(passwd,pRequest->frame+pos,6);
			fwrite_ertu_params(EEADDR_HJ212_PASSWD,passwd,6);
			mem_cpy(gSystemInfo.passwd,passwd,6);
		}
		
		mem_set(HJ212_request_buf,1000,0x00);
		data_seg_len = make_data_seg(HJ212_request_buf+10,91,9011,gSystemInfo.passwd,"000000000000000000000011", 4, NULL, NULL,"QnRtn=1");
		makeHJ212Frame(HJ212_request_buf, HJ212_request_buf+10,  data_seg_len, &frameLen);
		mem_cpy(pResp->frame,HJ212_request_buf,frameLen);
		pResp->frame_len =frameLen;
		app_send_ReplyFrame(pResp);
		DelayNmSec(2000);
		mem_set(HJ212_request_buf,1000,0x00);
		data_seg_len = make_data_seg(HJ212_request_buf+10,91,9012,gSystemInfo.passwd,"000000000000000000000011", 4, NULL, NULL,"ExeRtn=1");
		makeHJ212Frame(HJ212_request_buf, HJ212_request_buf+10,  data_seg_len, &frameLen);
		mem_cpy(pResp->frame,HJ212_request_buf,frameLen);
		pResp->frame_len =frameLen;
		app_send_ReplyFrame(pResp);		
	}
}
void app_protocol_handler_HJ212(objRequest* pRequest,objResponse *pResp)
{
	INT16U CN=0,ST =0;
	INT16S pos=0;
	INT8U tmp[10]={0};
	//判断passwd；
	//判断MN；
	if((pos=str_find(pRequest->frame,pRequest->frame_len,"ST=",3))!=-1)
	{
		mem_set(tmp,10,0x00);
		mem_cpy(tmp,pRequest->frame+pos+3,2);
		ST = atoi(tmp);
		gSystemHJ212.ST =ST;
	}
	if((pos=str_find(pRequest->frame,pRequest->frame_len,"CN=",3))!=-1)
	{
		mem_set(tmp,10,0x00);
		mem_cpy(tmp,pRequest->frame+pos+3,4);
		CN = atoi(tmp);
		gSystemHJ212.CN = CN;
	}
	if((CN>=1000)&&(CN<=1010))	 //初始化命令
	{
		
	}
	else if((CN>=1011)&&(CN<=1999))	 //参数命令
	{
		 handle_params_command( pRequest,pResp);
	}
	else if(CN>=2011&&CN<=2012)	 //实时数据
	{
			
	}
	else if(CN>=2021&&CN<=2022)	 //设备状态
	{
		
	}
	else if(CN>=2031&&CN<=2041)	 //日数据
	{
			
	}
	else if(CN>=2051&&CN<=2051)	 //分钟数据
	{
		
	}	
	else if(CN>=2061&&CN<=2061)	 //小时数据
	{
		
	}	
	else if(CN>=2081&&CN<=2999)	 //上传数采仪开机时间，其他数据
	{
		
	}	
	else if(CN>=3011&&CN<=3999)	 //控制命令
	{
		
	}
	else if(CN>=9011&&CN<=9999)	 //交互命令
	{
		
	}		
}
#ifdef __XINHE212__	   //冀北新禾主站联调
INT8U data_field[1000];
INT16U report_212_monitor_xinhe()
{
	//首先将数据都读回来
	INT16U voltage_A[2]={0},voltage_B[2]={0},voltage_C[2]={0};
	INT16S current_A[2]={0},current_B[2]={0},current_C[2]={0};	   //低字节是高位
	INT16S factor[2]={0};
	INT16S yg_power[2]={0},wg_power[2]={0};	   //总有无功功率
	INT8U yg_energy[2]={0},wg_energy[2]={0};   //总有无功电能量。
	INT8U len=0;
	INT8U str1[100],str2[100];
	mem_set(str1,100, 0x00);
	tagDatetime dt;	
	INT16U pos=0;
	INT8U mean_point=1;
	INT16U 	 frameLen=0,data_seg_len =0;
	uint32_t timer;	
	len =app_read_his_item(0x0202FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；电流数据块。
	if (len>0)
	{
		current_A[0] = (BCD2byte(str1[2]&0x7F)*10) +(str1[1]>>4);
		current_A[1] = (str1[1]&0x0F)*100 +BCD2byte(str1[0]);
		if(str1[2]&0x80)
		{
			current_A[0] =-current_A[0];
		}
		
		current_B[0] = (BCD2byte(str1[5]&0x7F)*10) +(str1[4]>>4);
		current_B[1] = (str1[4]&0x0F)*100 +BCD2byte(str1[3]);
		if(str1[2]&0x80)
		{
			current_B[0] =-current_B[0];
		}
		
		current_C[0] = (BCD2byte(str1[8]&0x7F)*10) +(str1[7]>>4);
		current_C[1] = (str1[7]&0x0F)*100 +BCD2byte(str1[6]);
		if(str1[8]&0x80)
		{
			current_C[0] =-current_C[0];
		}				
				
	}

	mem_set(str1,100, 0x00);
	len =app_read_his_item(0x0201FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；电压数据块。
	if (len>0)
	{
		voltage_A[0] =(BCD2byte(str1[1])*10) +(str1[0]>>4);voltage_A[1] =str1[0]&0x0F;
		voltage_B[0] =(BCD2byte(str1[3])*10) +(str1[2]>>4);voltage_B[1] =str1[2]&0x0F;
		voltage_C[0] =(BCD2byte(str1[5])*10) +(str1[4]>>4);voltage_C[1] =str1[4]&0x0F;
		
	}
	mem_set(str1,100, 0x00);
	len =app_read_his_item(0x02030000,str1,str2,255,0,2000);//str1中存在着我们要读取的值；瞬时总有功功率。
	if(len>0)
	{
			yg_power[0] = BCD2byte(str1[2]&0x7F);
			yg_power[1] = BCD2byte(str1[1])*100+BCD2byte(str1[0]);		
	}
	mem_set(str1,100, 0x00);
	len =app_read_his_item(0x02040000,str1,str2,255,0,2000);//str1中存在着我们要读取的值；瞬时总无功功率。
	if(len>0)
	{
			wg_power[0] = BCD2byte(str1[2]&0x7F);
			wg_power[1] = BCD2byte(str1[1])*100+BCD2byte(str1[0]);		
	}
	mem_set(str1,100, 0x00);
	len =app_read_his_item(0x02060000,str1,str2,255,0,2000);//str1中存在着我们要读取的值；总功率因数。
	if(len>0)
	{
			 factor[0] = BCD2byte((str1[1]&0x7f)>>4);
			 factor[1] = BCD2byte(str1[1]&0x0F)*100+BCD2byte(str1[0]);		
	}
	mem_set(str1,100, 0x00);
	len =app_read_his_item(0x00010000,str1,str2,255,0,2000);//str1中存在着我们要读取的值；正向有功总电能。
	if(len>0)
	{
			yg_energy[0] = (BCD2byte(str1[3])*10000) +(BCD2byte(str1[2])*100)+(BCD2byte(str1[1]));
			yg_energy[1] = BCD2byte(str1[0]);		
	}
	mem_set(str1,100, 0x00);
	len =app_read_his_item(0x00030000,str1,str2,255,0,2000);//str1中存在着我们要读取的值；正向无功总电能。
	if(len>0)
	{
			wg_energy[0] = (BCD2byte(str1[3])*10000) +(BCD2byte(str1[2])*100)+(BCD2byte(str1[1]));
			wg_energy[1] = BCD2byte(str1[0]);		
	}				

	//将读回来的数据组成212报文
	if(FILEFLAG_TYPE==FILEFLAG_TGP_GMTER_G55)	   //三相表
	{
		pos=0;
		tpos_datetime(&dt);	   //时间中没有世纪这个时间。
		/***QN***/
		sprintf(data_field + pos, "DataTime=%02d%02d%02d%02d%02d%02d%02d%03d;", 20, dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second, (dt.msecond_h<<8)+dt.msecond_l);
		pos=strlen(data_field);
		sprintf(data_field + pos, "tn%02d01-Rtd=%d.%d,tn%02d01-Flag=N;",mean_point,current_A[0],current_A[1],mean_point);	//A相电流
		pos=strlen(data_field);
		sprintf(data_field + pos, "tn%02d02-Rtd=%d.%d,tn%02d02-Flag=N;",mean_point, current_B[0],current_B[1],mean_point);	//B相电流
		pos=strlen(data_field);	
		sprintf(data_field + pos, "tn%02d03-Rtd=%d.%d,tn%02d03-Flag=N;",mean_point, current_C[0],current_C[1],mean_point);	//C相电流
		pos=strlen(data_field);	
		sprintf(data_field + pos, "tn%02d05-Rtd=%d.%d,tn%02d05-Flag=N;",mean_point, yg_power[0],yg_power[1],mean_point);	//总有功功率
		pos=strlen(data_field);
		sprintf(data_field + pos, "tn%02d06-Rtd=%d.%d,tn%02d06-Flag=N;",mean_point, wg_power[0],wg_power[1],mean_point);	//总无功功率
		pos=strlen(data_field);
		sprintf(data_field + pos, "tn%02d07-Rtd=%d.%d,tn%02d07-Flag=N;",mean_point, yg_energy[0],yg_energy[1],mean_point);	//总有功电能量
		pos=strlen(data_field);
		sprintf(data_field + pos, "tn%02d08-Rtd=%d.%d,tn%02d08-Flag=N;",mean_point, wg_energy[0],wg_energy[1],mean_point);	//总无功电能量
		pos=strlen(data_field);	
		sprintf(data_field + pos, "tn%02d09-Rtd=%d.%d,tn%02d09-Flag=N;",mean_point, factor[0],factor[1],mean_point);	//功率因数
		pos=strlen(data_field);	
		sprintf(data_field + pos, "tn%02d10-Rtd=%d.%d,tn%02d10-Flag=N;",mean_point, voltage_A[0],voltage_A[1],mean_point);	//A相电压
		pos=strlen(data_field);
		sprintf(data_field + pos, "tn%02d11-Rtd=%d.%d,tn%02d11-Flag=N;",mean_point, voltage_B[0],voltage_B[1],mean_point);	//B相电压
		pos=strlen(data_field);		
		sprintf(data_field + pos, "tn%02d12-Rtd=%d.%d,tn%02d12-Flag=N;",mean_point,voltage_C[0],voltage_C[1],mean_point);	//C相电压
		pos=strlen(data_field);								

		mem_set(HJ212_request_buf,1000,0x00);
		data_seg_len = make_data_seg(HJ212_request_buf+10,80,2011,"123456","000000000000000000000011", 5, NULL, NULL,data_field);
		makeHJ212Frame(HJ212_request_buf, HJ212_request_buf+10,  data_seg_len, &frameLen);
		return frameLen;
		//timer =	 system_get_tick10ms();
		////while(ResponseApp.frame_len>0)
		////{
			////if((system_get_tick10ms()-timer) > 200)
			////{
				////return;
			////}
			////DelayNmSec(100);
		////}
		//ResponseApp.channel = PROTOCOL_CHANNEL_DEBUG;
		//mem_cpy(ResponseApp.frame,HJ212_request_buf,frameLen);
		//ResponseApp.frame_len =frameLen;
		//app_send_ReplyFrame(&ResponseApp);					
	}
	if(FILEFLAG_TYPE==FILEFLAG_SGP_GMTER_G55)	   //单相表
	{
		
	}	
	
}
//上报停上电事件
INT16U 	report_212_poweroff_xinhe()
{
	INT16U pos=0;
	tagDatetime dt;	
	INT8U mean_point=1;
	INT16U 	 frameLen=0,data_seg_len =0;	
	mem_set(data_field,1000,0x00);
	tpos_datetime(&dt);	   //时间中没有世纪这个时间。
	/***QN***/
	sprintf(data_field + pos, "DataTime=%02d%02d%02d%02d%02d%02d%02d%03d;", 20, dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second, (dt.msecond_h<<8)+dt.msecond_l);
	pos=strlen(data_field);
	sprintf(data_field+pos,"tn%02d50-Rtd=1,tn%02d-Flag=N",mean_point,mean_point);
	mem_set(HJ212_request_buf,1000,0x00);
	data_seg_len = make_data_seg(HJ212_request_buf+10,80,2011,"123456","000000000000000000000011", 4, NULL, NULL,data_field);
	makeHJ212Frame(HJ212_request_buf, HJ212_request_buf+10,  data_seg_len, &frameLen);
	return frameLen;	
}
#endif