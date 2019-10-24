///////////////////////////////////////////////////////////////////////////////
//
//   onenet 协议处理  
//
///////////////////////////////////////////////////////////////////////////////

#include "../main_include.h"



typedef union
{
    INT16U value;
    struct{
        INT8U convert_type:8;//转换函数名
        INT8U type:4;//返回数据格式
        INT8U report_type:3;//上报类型 DAY_HOLD:001  CUR_DATA 010 MONTHHOLD 100
        INT8U ret:1;//备用
    };
}tagDataType; 

typedef struct{
    INT16U obj;
    INT8U instance;
    INT16U id;
    INT32U item;
    tagDataType data_type;//数据类型1:
    INT8U  len;
}tagResourceId;







const tagResourceId  RESOURCE_ID[] = 
{
//   object,    instance,id,                item,               type,                       len
{	3316,       0,      5700,               0x02010100,          CYCLE_DATA|FLOAT_DATA|V_FLOAT,   2}, //A相电压Ua
{	3316,       0,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|V_STRING,   1}, //A相电压单位 V
#ifndef __METER_SIMPLE__
{	3316,       1,      5700,               0x02010200,          CYCLE_DATA|FLOAT_DATA|V_FLOAT,   2}, //B相电压Ub
{	3316,       1,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|V_STRING,   1},//B相电压单位V
{	3316,       2,      5700,               0x02010300,          CYCLE_DATA|FLOAT_DATA|V_FLOAT,   2}, //C相电压Uc
{	3316,       2,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|V_STRING,   1},//C相电压单位V
#endif

{	3317,       0,      5700,               0x02020100,          CYCLE_DATA|FLOAT_DATA|A_FLOAT,   3}, //A相电流Ia  
{	3317,       0,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|A_STRING,   1}, //A相电流单位 A
#ifndef __METER_SIMPLE__
{	3317,       1,      5700,               0x02020200,          CYCLE_DATA|FLOAT_DATA|A_FLOAT,   3}, //B相电流Ib
{	3317,       1,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|A_STRING,   1},//B相电流单位A
{	3317,       2,      5700,               0x02020300,          CYCLE_DATA|FLOAT_DATA|A_FLOAT,   3}, //C相电流Ic
{	3317,       2,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|A_STRING,   1},//C相电流单位A
#endif
{	3317,       3,      5700,               0x02800001,          CYCLE_DATA|FLOAT_DATA|A_FLOAT,   3}, //零序电流I0 
{	3317,       3,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|A_STRING,   1},//零序电流单位A

{	3328,       0,      5700,               0x02030000,          CYCLE_DATA|FLOAT_DATA|P_FLOAT,   3}, //总有功功率 
{	3328,       0,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KW_STRING,   2}, //总有功功率 KW
{	3328,       1,      5700,               0x02030100,          CYCLE_DATA|FLOAT_DATA|P_FLOAT,   3}, //A相有功功率Pa  
{	3328,       1,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KW_STRING,   2}, //A相有功功率单位 KW
#ifndef __METER_SIMPLE__
{	3328,       2,      5700,               0x02030200,          CYCLE_DATA|FLOAT_DATA|P_FLOAT,   3}, //B相有功功率Pb
{	3328,       2,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KW_STRING,   2},//B相有功功率单位 KW
{	3328,       3,      5700,               0x02030300,          CYCLE_DATA|FLOAT_DATA|P_FLOAT,   3}, //C相有功功率Pc
{	3328,       3,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KW_STRING,   2},//C相有功功率单位 KW
#endif
#ifndef __METER_SIMPLE__
{	3328,       4,      5700,               0x02040000,          CYCLE_DATA|FLOAT_DATA|P_FLOAT,   3}, //总无功功率 
{	3328,       4,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KVar_STRING,   4}, //总无功功率 KW
{	3328,       5,      5700,               0x02040100,          CYCLE_DATA|FLOAT_DATA|P_FLOAT,   3}, //A相无功功率Qa  
{	3328,       5,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KVar_STRING,   4}, //A相无功功率单位 kvar
{	3328,       6,      5700,               0x02040200,          CYCLE_DATA|FLOAT_DATA|P_FLOAT,   3}, //B相无功功率Qb
{	3328,       6,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KVar_STRING,   4},//BA相无功功率单位 kvar
{	3328,       7,      5700,               0x02040300,          CYCLE_DATA|FLOAT_DATA|P_FLOAT,   3}, //C相无功功率Qc
{	3328,       7,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KVar_STRING,   4},//CA相无功功率单位 kvar
#endif

{	3329,       0,      5700,               0x02060000,          CYCLE_DATA|FLOAT_DATA|FACTOR_FLOAT,   4}, //总功率因数
{	3329,       1,      5700,               0x02060100,          CYCLE_DATA|FLOAT_DATA|FACTOR_FLOAT,   4}, //功率因数facA
#ifndef __METER_SIMPLE__
{	3329,       2,      5700,               0x02060200,          CYCLE_DATA|FLOAT_DATA|FACTOR_FLOAT,   4}, //功率因数facB  
{	3329,       3,      5700,               0x02060300,          CYCLE_DATA|FLOAT_DATA|FACTOR_FLOAT,   4}, //功率因数facC
#endif

{	3331,       0,      5805,               0x00010000,          CYCLE_DATA|FLOAT_DATA|KWH_FLOAT,   4}, // 当前正向有功电能示值
{	3331,       0,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KWH_STRING,   3}, //当前正向有功电能示值单位 KWh
#ifndef __METER_SIMPLE__
{	3331,       1,      5805,               0x00030000,          CYCLE_DATA|FLOAT_DATA|KWH_FLOAT_MINUS,   4}, // 当前正向无功电能示值-----》
{	3331,       1,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KVarH_STRING,   5}, //当前正向无功电能示值单位 Kvarh
#endif
{	3331,       2,      5805,               0x00020000,          CYCLE_DATA|FLOAT_DATA|KWH_FLOAT,   4}, // 当前反向有功电能示值
{	3331,       2,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KWH_STRING,   3}, //当前反向有功电能示值单位 KWh
#ifndef __METER_SIMPLE__
{	3331,       3,      5805,               0x00040000,          CYCLE_DATA|FLOAT_DATA|KWH_FLOAT_MINUS,   4}, // 当前反向无功电能示值----》
{	3331,       3,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KVarH_STRING,   5}, //当前反向无功电能示值单位 Kvarh
#endif
{	3331,       4,      5805,               0x05060101,          DAYHOLD_DATA|FLOAT_DATA|KWH_FLOAT,   4}, // (上1 次)日冻结正向有功总电能
{	3331,       4,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KWH_STRING,   3}, //(上1 次)日冻结正向有功总电能 KWh
{	3331,       5,      5805,               0x05060201,          DAYHOLD_DATA|FLOAT_DATA|KWH_FLOAT,   4}, // (上1 次)日冻结反向有功总电能
{	3331,       5,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KWH_STRING,   3}, //(上1 次)日冻结反向有功总电能 KWh
#ifndef __METER_SIMPLE__
{	3331,       6,      5805,               0x05060301,          DAYHOLD_DATA|FLOAT_DATA|KWH_FLOAT_MINUS,   4}, // (上1 次)日冻结正向无功总电能 ------》
{	3331,       6,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KVarH_STRING,   5}, //(上1 次)日冻结正向无功总电能 Kvarh
{	3331,       7,      5805,               0x05060401,          DAYHOLD_DATA|FLOAT_DATA|KWH_FLOAT_MINUS,   4}, // (上1 次)日冻结反向无功总电能 ------》
{	3331,       7,      5701,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|KVarH_STRING,   5}, //(上1 次)日冻结反向无功总电能 Kvarh
#endif
{	3331,       8,      5805,               0x00010001,          MONTHHOLD_DATA|FLOAT_DATA|KWH_FLOAT,   4}, //  月冻结正向有功总电能示值 ----》
{	3331,       8,      5701,               0xFFFFFFFF,          MONTHHOLD_DATA|STRING_DATA|KWH_STRING,   4}, //月冻结正向有功总电能示值 KWh

//扩展的3332系列
{   3332,       0,      5750,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|METER_INDEX,             	4}, //电能表资产编号
{   3332,       1,      5750,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|METER_TYPE,             	4}, //电能表类型
{   3332,       2,      5750,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|MODULE_IMEI,             	4}, //IMEI
{   3332,       3,      5750,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|MODULE_ICCID,             	4}, //ICCID
{   3332,       4,      5750,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|MODULE_IMSI,             	4}, //IMSI
{   3332,       5,      5750,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|MODULE_CSQ,             	4}, //信号强度等级
{   3332,       6,      5750,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|METER_ADDR,             	4}, //电表通信地址
{   3332,       7,      5750,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|NOTIFY_CYCLE,             	4}, //主动上报周期
{   3332,       8,      5750,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|SOFT_VERSION,             	4}, //厂家软件版本号
{   3332,       9,      5750,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|HARD_VERSION,             	4}, //厂家硬件版本号
{   3332,       10,     5750,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|VENDER_NO,             	4}, //厂家编号
{   3332,       11,     5750,               0xFFFFFFFF,          CYCLE_DATA|STRING_DATA|POWER_SUPPLY,   			4}, //当前供电状态
{   3332,       12,     5750,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|DATA_FLOW,             	4}, //数据流量
{   3332,       13,     5750,               0xFFFFFFFF,          STRING_DATA|REMOTE_UPDATE,             4}, //远程升级
{   3332,       14,     5750,               0xFFFFFFFF,          CYCLE_DATA|STRING_DATA|RSRP,           4}, //RSRP 接收功率
{   3332,       15,     5750,               0xFFFFFFFF,          CYCLE_DATA|STRING_DATA|SINR,           4}, //信噪比
{   3332,       16,     5750,               0xFFFFFFFF,          CYCLE_DATA|STRING_DATA|eNODEB,         4}, //小区号enodeB
{   3332,       17,     5750,               0xFFFFFFFF,          CYCLE_DATA|STRING_DATA|CELL_ID,        4}, //CELL ID

{   3333,       0,      5750,               0xFFFFFFFF,          DAYHOLD_DATA|STRING_DATA|ONENET_TIME,             	4}, //日期、星期（其中0代表星期天）及时间

{   3200,       0,      5500,               0xFFFFFFFF,          DAYHOLD_DATA|BOOL_DATA|SZDC_QY,             1},//
{   3200,       1,      5500,               0xFFFFFFFF,          DAYHOLD_DATA|BOOL_DATA|SD_TSD,             1},//
{   3200,       2,      5500,               0xFFFFFFFF,          DAYHOLD_DATA|BOOL_DATA|FZDY_TSD,             1},//
{   3200,       3,      5500,               0xFFFFFFFF,          DAYHOLD_DATA|BOOL_DATA|YGGL_FX,             1},//
#ifndef __METER_SIMPLE__
{   3200,       4,      5500,               0xFFFFFFFF,          DAYHOLD_DATA|BOOL_DATA|WGGL_FX,             1},//
#endif
{   3200,       5,      5500,               0xFFFFFFFF,          DAYHOLD_DATA|BOOL_DATA|A_GL,             1},//
#ifndef __METER_SIMPLE__
{   3200,       6,      5500,               0xFFFFFFFF,          DAYHOLD_DATA|BOOL_DATA|B_GL,             1},//
{   3200,       7,      5500,               0xFFFFFFFF,          DAYHOLD_DATA|BOOL_DATA|C_GL,             1},//
#endif
#ifndef __METER_SIMPLE__
{   3200,       8,      5500,               0xFFFFFFFF,          DAYHOLD_DATA|BOOL_DATA|A_DX,             1},//
{   3200,       9,      5500,               0xFFFFFFFF,          DAYHOLD_DATA|BOOL_DATA|B_DX,             1},//
{   3200,       10,     5500,               0xFFFFFFFF,          DAYHOLD_DATA|BOOL_DATA|C_DX,             1},//
#endif
};

BOOLEAN  app_get_id_to_item(INT32U objId,INT32U insId,INT32U resId,tagResourceId *pLib)
{
    INT16U size=0,idx=0;
    
    size = sizeof(RESOURCE_ID)/sizeof(tagResourceId);
    
    for(idx=0;idx<size;idx++)
    {
        if((RESOURCE_ID[idx].obj==objId)&&((RESOURCE_ID[idx].instance)==insId)&&(RESOURCE_ID[idx].id == resId))
        {
            pLib->item = RESOURCE_ID[idx].item;
            pLib->data_type.type = RESOURCE_ID[idx].data_type.type;
            pLib->len  = RESOURCE_ID[idx].len;
            pLib->data_type.convert_type = RESOURCE_ID[idx].data_type.convert_type;
            return TRUE; 
        }
    }
    pLib->item = 0xFFFFFFFF;
    return FALSE;
}


/*********************************
 * frame  需要已转换string
 * len    string类型长度
 ********************************/
void app_readresp_onenet_frame(tagWirelessObj *Obj,INT8U* frame,INT16U len,INT8U need_quotation)
{
 	INT8U str[100]={0};   
    if(len)
    {
        if(need_quotation)
        {
            sprintf((char*)str,"AT+MIPLREADRSP=0,%ld,%d,%4d,%d,%ld,%d,%d,\"", Obj->mid,Obj->result,Obj->objid,Obj->insid,Obj->resid,Obj->type,len);
        }
        else
        {
            sprintf((char*)str,"AT+MIPLREADRSP=0,%ld,%d,%4d,%d,%ld,%d,%d,", Obj->mid,Obj->result,Obj->objid,Obj->insid,Obj->resid,Obj->type,len);        
        }
    }
    else
    {
        sprintf((char*)str,"AT+MIPLREADRSP=0,%ld,%d\r\n", Obj->mid,Obj->result);
    }

    Obj->send_len = str_cmdlen(str);    
    mem_cpy(Obj->send_buf+Obj->send_len,frame,len);
    mem_cpy(Obj->send_buf,str,str_cmdlen(str));
    Obj->send_len += len;
    
    if(need_quotation)
    {
        sprintf((char*)Obj->send_buf+Obj->send_len,"\",0,0\r\n");
        Obj->send_len+=7;
    }
    else
    {
        sprintf((char*)Obj->send_buf+Obj->send_len,",0,0\r\n");
        Obj->send_len+=6;
    }

    remote_send_app_frame(Obj->send_buf,Obj->send_len); 
}
INT8U app_data_convert(INT8U* data,INT8U type)
{
	float res=0;
	BOOLEAN valid_bcd;
	INT32U tmp=0,len=0;
	INT8U str1[60]={0},str2[100]={0},meter_no[6]={0};
	INT8U idx,minus=0;
	INT8U report_period[4];
	tagDatetime datetime;
	INT8U monthFlow[4];
	INT8U ch;//为了暂时存储int2bcd。
	tagEVENT event_for;
	
	if(type==0)
	{
		return str_cmdlen(data);
	}
	
	switch(type)
	{
		case KWH_FLOAT:
		res = bcd2u32(data,4,&valid_bcd);
		sprintf((char*)data,"%.2f",res/100);
		break;
		case KWH_STRING:
		sprintf((char*)data,"\"kWh\"");
		break;
		case KVarH_STRING:
		sprintf((char*)data,"\"kvarh\"");
		break;
		case V_STRING:
		sprintf((char*)data,"\"V\"");
		break;
		case V_FLOAT:
		res = bcd2u32(data,2,&valid_bcd);
		sprintf((char*)data,"%.1f",res/10);
		break;
		case A_FLOAT:
		minus = data[2]&0x80;
		data[2] = data[2]&0x7F;
		res = bcd2u32(data,3,&valid_bcd);
		if(minus)
		{
			sprintf((char*)data,"-%.3f",res/1000);
		}
		else
		{
			sprintf((char*)data,"%.3f",res/1000);
		}
		break;
		case A_STRING:
		sprintf((char*)data,"\"A\"");
		break;
		case P_FLOAT:
		minus = data[2]&0x80;
		data[2] = data[2]&0x7F;
		res = bcd2u32(data,3,&valid_bcd);
		if(minus)
		{
			sprintf((char*)data,"-%.4f",res/10000);
		}
		else
		{
			sprintf((char*)data,"%.4f",res/10000);
		}
		break;
		case KW_STRING:
		sprintf((char*)data,"\"kW\"");
		break;
		case KVar_STRING:
		sprintf((char*)data,"\"kvar\"");
		break;
		case FACTOR_FLOAT:
		minus = data[1]&0x80;
		data[1] = data[1]&0x7F;
		res = bcd2u32(data,2,&valid_bcd);
		if(minus)
		{
			sprintf((char*)data,"-%.3f",res/1000);
		}
		else
		{
			sprintf((char*)data,"%.3f",res/1000);
		}
		break;
		case  METER_INDEX: //电能表资产编号 0
		memset(str1,0x00,32);
		len = app_read_his_item(0x04000403,str1,str2,255,0,2000);//str1中存在着我们要读取的值；
		*data='"';
		if(len==22)
		{
			mem_cpy_reverse(data+1,str1,22);
		}
		else
		{
			mem_cpy_reverse(data+1,str1,32);
		}
		if(check_is_all_ch(data+1,22,0x00))
		{
			memset(data+1,0x30,22);//默认为全0资产号 不是null 适应浙江现场电表资产号未设置
		}
		*(data+23)='"';
		*(data+24)=0x00;
		break;
		case  METER_TYPE: //电能表类型  后续根据读数据判断
		#ifdef __METER_SIMPLE__
		sprintf((char*)data,"\"1\"");
		#else
		sprintf((char*)data,"\"3\"");
		#endif
		break;
		case  MODULE_IMEI: //IMEI 2  imei_info[15]; 2  +CN:9903682 869976030068952
		//gprs_send_cmd_wait_OK((uint8_t *)"AT+CGSN=1\r\n",str1,sizeof(str1),100);
		*data='"';
		for(idx =0;idx<15;idx++)
		{
			*(data+idx+1) = gSystemInfo.imei_info[idx];
		}
		*(data+idx+1) ='"';
		break;
		case  MODULE_ICCID: //ICCID 3  +NID982B510241 898602B7151630249417
		// gprs_send_cmd_wait_OK((uint8_t *)"AT+NCCID\r\n",str1,sizeof(str1),100);
		*data='"';
		for(idx =0;idx<20;idx++)
		{
			*(data+idx+1) = gSystemInfo.myccid_info[idx];
		}
		*(data+idx+1) ='"';
		break;
		case  MODULE_IMSI: //IMSI 4   46405341
		// gprs_send_cmd_wait_OK((uint8_t *)"AT+CIMI\r\n",str1,sizeof(str1),100);
		*data='"';
		for(idx =0;idx<15;idx++)
		{
			*(data+idx+1) = gSystemInfo.imsi_info[idx];
		}
		*(data+idx+1) ='"';
		break;
		case  MODULE_CSQ: //信号强度等级 gSystemInfo.csq  5
		*data='"';
		*(data+1)=gSystemInfo.csq/10+0x30;
		*(data+2)=gSystemInfo.csq- (*(data+1)-0x30)*10+0x30;
		*(data+3) ='"';
		// sprintf((char*)data,"%d",&(gSystemInfo.csq));
		break;
		case  METER_ADDR: //电表通信地址 6
		*data='"';
		mem_cpy_reverse(meter_no,gSystemInfo.meter_no,6);
		for(idx=0;idx<6;idx++)
		{
			*(data+idx*2+1)=(((meter_no[idx])>>4)&0x0f) +0x30;
			*(data+idx*2+2)=((meter_no[idx])&0x0f)  +0x30;
		}
		*(data+idx*2+1) ='"';
		break;
		case  NOTIFY_CYCLE: //主动上报周期7
		fread_ertu_params(EEADDR_TASK_REPORT_CYCLE,report_period,4);
		tmp = bcd2u32(report_period,4,&valid_bcd);
		if((valid_bcd==FALSE)||(tmp==0))
		{
			report_period[0]=0x60;
			report_period[1]=0x00;
			report_period[2]=0x00;
			report_period[3]=0x00;
			fwrite_ertu_params(EEADDR_TASK_REPORT_CYCLE,report_period,4);
		}
		mem_cpy_reverse(str1,report_period,4);
		hex2str(str1,4,data+1);
		*data='"';
		*(data+9) ='"';
		break;
		case  SOFT_VERSION: //厂家软件版本号8
		*data='"';
		*(data+1)=SOFT_VER_MAIN;
		*(data+2)=SOFT_VER_DOT;
		*(data+3) =SOFT_VER_MIN1;
		*(data+4) =SOFT_VER_MIN2;
		*(data+5) ='"';
		break;
		case  HARD_VERSION: //厂家硬件版本号9
		*data='"';
		*(data+1)=HARDWARE_VER_H;
		*(data+2)=HARDWARE_VER_DOT;
		*(data+3) =HARDWARE_VER_L1;
		*(data+4) =HARDWARE_VER_L2;
		*(data+5) ='"';
		break;
		case  VENDER_NO: //厂家编号 10
		*data='"'; *(data+1)='T'; *(data+2)='O'; *(data+3) = 'P';
		*(data+4) = 'S';*(data+5) = 'C';*(data+6) = 'O';
		*(data+7) = 'M';*(data+8) = 'M';
		*(data+9) ='"';
		break;
		case  ONENET_TIME: //ONENET_TIME 设置时间
		os_get_datetime(&datetime);
		*data ='"';
		ch = byte2BCD(datetime.year);
		*(data+1) = ((ch>>4)&0x0f)+'0';
		*(data+2) = ((ch)&0x0f)+'0';
		*(data+3) = '-';
		ch = byte2BCD(datetime.month);
		*(data+4) = ((ch>>4)&0x0f)+'0';
		*(data+5) = ((ch)&0x0f)+'0';
		*(data+6) = '-';
		ch = byte2BCD(datetime.day);
		*(data+7) = ((ch>>4)&0x0f)+'0';
		*(data+8) = ((ch)&0x0f)+'0';
		*(data+9) = ' ';
		ch = byte2BCD(datetime.weekday);
		*(data+10) = ((ch)&0x0f)+'0';
		*(data+11) = ' ';
		ch = byte2BCD(datetime.hour);
		*(data+12) = ((ch>>4)&0x0f)+'0';
		*(data+13) = ((ch)&0x0f)+'0';
		*(data+14) = ':';
		ch = byte2BCD(datetime.minute);
		*(data+15) = ((ch>>4)&0x0f)+'0';
		*(data+16) = ((ch)&0x0f)+'0';
		*(data+17) = ':';
		ch = byte2BCD(datetime.second);
		*(data+18) = ((ch>>4)&0x0f)+'0';
		*(data+19) = ((ch)&0x0f)+'0';
		*(data+20) = '"';
		break;
		case  POWER_SUPPLY: //当前供电状态11
		app_read_his_item(0x04000503,str1,str2,255,0,2000);//str2中存在着我们要读取的值；
		*data='"';
		if((str1[0]&0x06) == 0)
		{
			*(data+1) = '0';
			*(data+2) = '0';
		}
		else
		{
			*(data+1) = '0';
			*(data+2) = '1';
		}
		*(data+3)='"';
		break;
		case  DATA_FLOW: //数据流量12
		memcpy(&tmp,g_app_run_data.gprs_bytes.month_bytes,4);
		tmp = tmp/1024;
		ul2bcd(tmp,monthFlow,4);
		mem_cpy_reverse((INT8U*)&tmp,monthFlow,4);
		mem_cpy(monthFlow,(INT8U*)&tmp,4);
		hex2str(monthFlow,4, (data+1));
		*data='"';
		*(data+9) ='"';
		break;
		case REMOTE_UPDATE:
		*data='"';
		*(data+1)='1';
		*(data+2)='"';
		break;
		case SZDC_QY:
		fread_ertu_params(EEADDR_LAST_STATUS,str1,26);
		*data='"';
		if(str1[0]&0x04)
		{
			*(data+1)='1';
		}
		else
		{
			*(data+1)='0';
		}
		*(data+2)='"';
		break;
		case SD_TSD:
		case FZDY_TSD:
		case YGGL_FX:
		case WGGL_FX:
		case A_GL:
		case B_GL:
		case C_GL:
		case A_DX:
		case B_DX:
		case C_DX:
		fread_ertu_params(EEADDR_LAST_ENENT_TIMES+(type-27-1)*4,event_for.value,4);
		*data='"';
		if(event_for.state==0xAA)
		{
			*(data+1)='1';
		}
		else
		{
			*(data+1)='0';
		}
		*(data+2)='"';
		break;
		case KWH_FLOAT_MINUS://带符号位的电能示值
		minus = data[3]&0x80;
		data[3] = data[3]&0x7F;
		res = bcd2u32(data,4,&valid_bcd);
		if(minus)
		{
			sprintf((char*)data,"-%.2f",res/100);
		}
		else
		{
			sprintf((char*)data,"%.2f",res/100);
		}
		break;
		case RSRP:
		data[0]='"';
		mem_cpy(data+1,gSystemInfo.rsrp,4);
		data[str_cmdlen(data)]='"';
		break;
		case SINR:
		data[0]='"';
		mem_cpy(data+1,gSystemInfo.sinr,4);
		data[str_cmdlen(data)]='"';
		break;
		case eNODEB:
		data[0]='"';
		mem_cpy(data+1,gSystemInfo.cellid,4);
		data[str_cmdlen(data)]='"';
		break;
		case CELL_ID:
		data[0]='"';
		mem_cpy(data+1,gSystemInfo.cellid,8);
		data[str_cmdlen(data)]='"';
		break;
		case KWH_BCD2STRING:
		mem_cpy_reverse(str1,data,4);
		hex2str(str1,4,data);
		mem_cpy(str1,data,6);
		str1[6]='.';
		mem_cpy_reverse(str1+7,data+6,2);
		mem_cpy(data,str1,9);
		break;
		case V_BCD2STRING:
		mem_cpy_reverse(str1,data,2);
		hex2str(str1,2,data);
		mem_cpy(str1,data,3);
		str1[3]='.';
		mem_cpy_reverse(str1+4,data+3,1);
		mem_cpy(data,str1,5);
		break;
		case I_BCD2STRING://len=8
		minus = data[2]&0x80;
		data[2] = data[2]&0x7F;
		mem_cpy_reverse(str1,data,3);
		hex2str(str1,3,data);
		mem_cpy(str1,data,3);
		str1[3]='.';
		mem_cpy_reverse(str1+4,data+3,3);
		if(minus)
		{
			data[0]='-';
		}
		else
		{
			data[0]=' ';
		}
		mem_cpy(data+1,str1,7);
		break;
		case P_BCD2STRING://len=8
		minus = data[2]&0x80;
		data[2] = data[2]&0x7F;
		mem_cpy_reverse(str1,data,3);
		hex2str(str1,3,data);
		mem_cpy(str1,data,2);
		str1[2]='.';
		mem_cpy_reverse(str1+3,data+2,4);
		if(minus)
		{
			data[0]='-';
		}
		else
		{
			data[0]=' ';
		}
		mem_cpy(data+1,str1,7);
		break;
		case F_BCD2STRING://len=6
		minus = data[1]&0x80;
		data[1] = data[1]&0x7F;
		mem_cpy_reverse(str1,data,2);
		hex2str(str1,2,data);
		mem_cpy(str1,data,1);
		str1[1]='.';
		mem_cpy_reverse(str1+2,data+1,3);
		if(minus)
		{
			data[0]='-';
		}
		else
		{
			data[0]=' ';
		}
		mem_cpy(data+1,str1,5);
		break;
		default:
		break;
	}
	if(type==METER_INDEX)
	{
		return 24;
	}
	return str_cmdlen(data);
}



/*
INT8U app_data_convert(INT8U* data,INT8U type)
{
    float res=0;
    BOOLEAN valid_bcd;
    INT32U tmp=0,len=0;
    INT8U str1[60]={0},str2[100]={0},meter_no[6]={0};
    INT8U idx,minus=0;
    INT8U report_period[4];
    tagDatetime datetime;
    INT8U monthFlow[4];
    INT8U ch;//为了暂时存储int2bcd。
    tagEVENT event_for;
    
    if(type==0)
    {
        return str_cmdlen(data);
    }
    
    switch(type)
    {
        case KWH_FLOAT:
            res = bcd2u32(data,4,&valid_bcd);
            sprintf((char*)data,"%.2f",res/100); 
            break;
        case KWH_STRING:
            sprintf((char*)data,"\"kWh\""); 
            break;  
        case KVarH_STRING:
            sprintf((char*)data,"\"kvarh\""); 
            break;
        case V_STRING:
            sprintf((char*)data,"\"V\""); 
            break;
        case V_FLOAT:
             res = bcd2u32(data,2,&valid_bcd);
            sprintf((char*)data,"%.1f",res/10); 
            break;
        case A_FLOAT:
        	minus = data[2]&0x80;
        	data[2] = data[2]&0x7F;
            res = bcd2u32(data,3,&valid_bcd);
            if(minus)    
            {
            	sprintf((char*)data,"-%.3f",res/1000);             
            } 
            else
            {    
            sprintf((char*)data,"%.3f",res/1000); 
            }
            break;
        case A_STRING:
            sprintf((char*)data,"\"A\""); 
            break;
        case P_FLOAT:
            minus = data[2]&0x80;
        	data[2] = data[2]&0x7F;
            res = bcd2u32(data,3,&valid_bcd);
            if(minus)   
            {
            	sprintf((char*)data,"-%.4f",res/10000);             
            }
            else
            {            
            sprintf((char*)data,"%.4f",res/10000); 
            }
            break;
        case KW_STRING:
            sprintf((char*)data,"\"kW\""); 
            break;
        case KVar_STRING:
            sprintf((char*)data,"\"kvar\""); 
            break;
        case FACTOR_FLOAT:
        	minus = data[1]&0x80;
        	data[1] = data[1]&0x7F;        
            res = bcd2u32(data,2,&valid_bcd);
            if(minus)
            {
            	sprintf((char*)data,"-%.3f",res/1000);
            }
            else
            {
            sprintf((char*)data,"%.3f",res/1000); 
            } 
            break;
        case  METER_INDEX: //电能表资产编号 0
            memset(str1,0x00,32);
            len = app_read_his_item(0x04000403,str1,str2,255,0,2000);//str1中存在着我们要读取的值；
                *data='"';
            if(len==22)
            {
            	mem_cpy_reverse(data+1,str1,22);	
            }
            else
            {
            	mem_cpy_reverse(data+1,str1,32);
            }
            if(check_is_all_ch(data+1,22,0x00))
            {
            	memset(data+1,0x30,22);//默认为全0资产号 不是null 适应浙江现场电表资产号未设置
            }
            *(data+23)='"';
            *(data+24)=0x00;
            break;
        case  METER_TYPE: //电能表类型  后续根据读数据判断
        	#ifdef __METER_SIMPLE__
            sprintf((char*)data,"\"1\""); 
            #else
            sprintf((char*)data,"\"3\""); 
            #endif
            break;
        case  MODULE_IMEI: //IMEI 2  imei_info[15]; 2  +CN:9903682 869976030068952
            //gprs_send_cmd_wait_OK((uint8_t *)"AT+CGSN=1\r\n",str1,sizeof(str1),100);
            *data='"';
            for(idx =0;idx<15;idx++)
            {
               *(data+idx+1) = gSystemInfo.imei_info[idx];
            }
            *(data+idx+1) ='"';
            break;
        case  MODULE_ICCID: //ICCID 3  +NID982B510241 898602B7151630249417
           // gprs_send_cmd_wait_OK((uint8_t *)"AT+NCCID\r\n",str1,sizeof(str1),100);
            *data='"';
            for(idx =0;idx<20;idx++)
            {
                *(data+idx+1) = gSystemInfo.myccid_info[idx];
            }
            *(data+idx+1) ='"';
            break;
        case  MODULE_IMSI: //IMSI 4   46405341
           // gprs_send_cmd_wait_OK((uint8_t *)"AT+CIMI\r\n",str1,sizeof(str1),100);
            *data='"';
            for(idx =0;idx<15;idx++)
            {
               *(data+idx+1) = gSystemInfo.imsi_info[idx];
            }
            *(data+idx+1) ='"';
            break;
        case  MODULE_CSQ: //信号强度等级 gSystemInfo.csq  5
             *data='"';
             *(data+1)=gSystemInfo.csq/10+0x30;
             *(data+2)=gSystemInfo.csq- (*(data+1)-0x30)*10+0x30;
             *(data+3) ='"';
           // sprintf((char*)data,"%d",&(gSystemInfo.csq));
            break;
        case  METER_ADDR: //电表通信地址 6
             *data='"';
             mem_cpy_reverse(meter_no,gSystemInfo.meter_no,6);
             for(idx=0;idx<6;idx++)
             {
                 *(data+idx*2+1)=(((meter_no[idx])>>4)&0x0f) +0x30;
                 *(data+idx*2+2)=((meter_no[idx])&0x0f)  +0x30;
             }
             *(data+idx*2+1) ='"';
            break;
        case  NOTIFY_CYCLE: //主动上报周期7
            fread_ertu_params(EEADDR_TASK_REPORT_CYCLE,report_period,4);
            tmp = bcd2u32(report_period,4,&valid_bcd);
    		if((valid_bcd==FALSE)||(tmp==0))
    		{
        		report_period[0]=0x60;
        		report_period[1]=0x00;
        		report_period[2]=0x00;
        		report_period[3]=0x00;
        		fwrite_ertu_params(EEADDR_TASK_REPORT_CYCLE,report_period,4);
    		}
             mem_cpy_reverse(str1,report_period,4);
             hex2str(str1,4,data+1);
             *data='"';
             *(data+9) ='"';
            break;
        case  SOFT_VERSION: //厂家软件版本号8
             *data='"';
             *(data+1)=SOFT_VER_MAIN;
             *(data+2)=SOFT_VER_DOT;
             *(data+3) =SOFT_VER_MIN1;
             *(data+4) =SOFT_VER_MIN2;
             *(data+5) ='"';
            break;
        case  HARD_VERSION: //厂家硬件版本号9
             *data='"';
             *(data+1)=HARDWARE_VER_H;
             *(data+2)=HARDWARE_VER_DOT;
             *(data+3) =HARDWARE_VER_L1;
             *(data+4) =HARDWARE_VER_L2;
             *(data+5) ='"';
            break;
        case  VENDER_NO: //厂家编号 10
            *data='"'; *(data+1)='T'; *(data+2)='O'; *(data+3) = 'P';
            *(data+4) = 'S';*(data+5) = 'C';*(data+6) = 'O';
            *(data+7) = 'M';*(data+8) = 'M';
            *(data+9) ='"';
            break;
        case  ONENET_TIME: //ONENET_TIME 设置时间 
            os_get_datetime(&datetime);
            *data ='"';
            ch = byte2BCD(datetime.year);
            *(data+1) = ((ch>>4)&0x0f)+'0';
            *(data+2) = ((ch)&0x0f)+'0';  
            *(data+3) = '-'; 
            ch = byte2BCD(datetime.month);
            *(data+4) = ((ch>>4)&0x0f)+'0';
            *(data+5) = ((ch)&0x0f)+'0';
            *(data+6) = '-';
            ch = byte2BCD(datetime.day);
            *(data+7) = ((ch>>4)&0x0f)+'0';
            *(data+8) = ((ch)&0x0f)+'0';
            *(data+9) = ' ';
            ch = byte2BCD(datetime.weekday);
            *(data+10) = ((ch)&0x0f)+'0';
            *(data+11) = ' ';
            ch = byte2BCD(datetime.hour);
            *(data+12) = ((ch>>4)&0x0f)+'0';
            *(data+13) = ((ch)&0x0f)+'0';
            *(data+14) = ':';
            ch = byte2BCD(datetime.minute);
            *(data+15) = ((ch>>4)&0x0f)+'0';
            *(data+16) = ((ch)&0x0f)+'0';
            *(data+17) = ':';
            ch = byte2BCD(datetime.second);
            *(data+18) = ((ch>>4)&0x0f)+'0';
            *(data+19) = ((ch)&0x0f)+'0';
            *(data+20) = '"';
            break;     
        case  POWER_SUPPLY: //当前供电状态11
            app_read_his_item(0x04000503,str1,str2,255,0,2000);//str2中存在着我们要读取的值；
            *data='"';
            if((str1[0]&0x06) == 0)
            {
                *(data+1) = '0';
                *(data+2) = '0';
            }
            else
            {
                *(data+1) = '0';
                *(data+2) = '1';
            }  
            *(data+3)='"';
            break;
        case  DATA_FLOW: //数据流量12
            memcpy(&tmp,g_app_run_data.gprs_bytes.month_bytes,4);
            tmp = tmp/1024;
            ul2bcd(tmp,monthFlow,4);
            mem_cpy_reverse(&tmp,monthFlow,4);
            mem_cpy(monthFlow,&tmp,4);
            hex2str(monthFlow,4, (data+1));
             *data='"';
             *(data+9) ='"';
            break;
        case REMOTE_UPDATE:
            *data='"';
            *(data+1)='1';
            *(data+2)='"';
            break;
        case SZDC_QY:
            fread_ertu_params(EEADDR_LAST_STATUS,str1,26);
            *data='"';
            if(str1[0]&0x04)
            {
                *(data+1)='1';
            }
            else
            {
                *(data+1)='0';            
            }
            *(data+2)='"';            
            break;
        case SD_TSD:
        case FZDY_TSD:
        case YGGL_FX:
        case WGGL_FX:  
        case A_GL:
        case B_GL:
        case C_GL:
        case A_DX:   
        case B_DX:   
        case C_DX:
            fread_ertu_params(EEADDR_LAST_ENENT_TIMES+(type-27-1)*4,event_for.value,4);
             *data='"';
            if(event_for.state==0xAA)
            {
                *(data+1)='1';
            }
            else
            {
                *(data+1)='0';            
            }
            *(data+2)='"'; 
            break;
        case KWH_FLOAT_MINUS://带符号位的电能示值
            minus = data[3]&0x80;
        	data[3] = data[3]&0x7F;
            res = bcd2u32(data,4,&valid_bcd);
            if(minus)
            {
            	sprintf((char*)data,"-%.2f",res/100);             
            }
            else
            {
            	sprintf((char*)data,"%.2f",res/100); 
            }
            break;
       case RSRP:
       		data[0]='"';
	   		mem_cpy(data+1,gSystemInfo.rsrp,4);
	   		data[str_cmdlen(data)]='"';
       		break;     
       case SINR:
       		data[0]='"';       
	   		mem_cpy(data+1,gSystemInfo.sinr,4);
	   		data[str_cmdlen(data)]='"';
       		break;     
       case eNODEB:
        	data[0]='"';      
	   		mem_cpy(data+1,gSystemInfo.cellid,4);
       		data[str_cmdlen(data)]='"';	   		
       		break; 		
       case CELL_ID:
       		data[0]='"';
	   		mem_cpy(data+1,gSystemInfo.cellid,8);
       		data[str_cmdlen(data)]='"';
       		break; 				
        default:
           break;
    }
    if(type==METER_INDEX)
    {
    	return 24;
    }
    return str_cmdlen(data);
}
*/
void  app_protocol_handler_onenet(void)
{
    INT16S idx,idx2;
    INT8U res_len=0;
    tagWirelessObj *Obj;
    tagResourceId source_id,*pLib;
    Obj = &RequestOnenet;
    pLib = &source_id;
    INT8U  report_cycle[4];
    INT8U str[8],str2[8];
    INT8U cs,pos; //校验值
    uint8_t meter_send_buffer[50];
    INT8U  resp[200]={0};
    objRequest* Request;
    
    if(Obj->state != REQUEST_STATE_PROCESSING)
    {
        return;
    }
    read_c1_time_out = 3000;
    memset(Obj->send_buf,0x00,SIZE_OF_APP_PROTOCOL_FRAME);
    if(Obj->msg_type == ONENET_READ)
    {
        if(app_get_id_to_item(Obj->objid,Obj->insid,Obj->resid,pLib))
        {
            if(pLib->item!=0xFFFFFFFF)
            {
                //电表数据
                res_len = app_read_his_item(pLib->item,Obj->send_buf,Obj->recv_buf,255,NULL,read_c1_time_out);
            }
            else
            {
                //内部数据
            }

            Obj->send_ptr = Obj->send_buf;
            Obj->state = REQUEST_STATE_PROCESSED;
            Obj->type = pLib->data_type.type; 
            if((res_len!=0)||(pLib->item==0xFFFFFFFF))
            {
                Obj->send_len = app_data_convert(Obj->send_buf,pLib->data_type.convert_type);
            }
            Obj->result = (Obj->send_len)? 1:0;
            app_readresp_onenet_frame(Obj,Obj->send_buf,Obj->send_len,0);      
        }
    }
    else if(Obj->msg_type == ONENET_WRITE)
    {
        Obj->state = REQUEST_STATE_PROCESSED;
        //写数据
        if(app_get_id_to_item(Obj->objid,Obj->insid,Obj->resid,pLib))
        {
            if(pLib->data_type.convert_type == NOTIFY_CYCLE)
            {
                for(idx =0;idx<8;idx++)
                {
                    if(Obj->recv_buf[idx]==0)
                        break;
                }
                idx--;
                for(idx2 =7;idx2>=0;idx2--)
                {
                    if(idx>=0)
                    {
                        str[idx2] = Obj->recv_buf[idx];
                        idx--;
                    }
                    else
                        str[idx2]='0';        
                }
                str2hex(str,8,str2); //首先变为4个字节
                mem_cpy_reverse(report_cycle, str2,4);//然后把这4个字节翻转。
                fwrite_ertu_params(EEADDR_TASK_REPORT_CYCLE,report_cycle,4);
                
	      		mem_set(resp,200,0x00);
	      		sprintf((char*)resp,"AT+MIPLWRITERSP=0,%ld,2\r\n", Obj->mid);
	      		remote_send_app_frame(resp,str_cmdlen(resp));
            }
            if(pLib->data_type.convert_type == ONENET_TIME)
            {
                //os_datetime;
                for(idx=0;idx<14;)
                {
                    str[idx/2] = (Obj->recv_buf[idx]-'0')*16+(Obj->recv_buf[idx+1]-'0');
                    idx+=2;
                }
                //组年月日报文
                idx = 0; //组报文初始位置
                meter_send_buffer[idx++] =0x68;
                meter_send_buffer[idx++] =gSystemInfo.meter_no[0];meter_send_buffer[idx++] =gSystemInfo.meter_no[1];meter_send_buffer[idx++] =gSystemInfo.meter_no[2];
                meter_send_buffer[idx++] =gSystemInfo.meter_no[3];meter_send_buffer[idx++] =gSystemInfo.meter_no[4];meter_send_buffer[idx++] =gSystemInfo.meter_no[5];
                meter_send_buffer[idx++] = 0x68;
                meter_send_buffer[idx++] = 0x14;
                meter_send_buffer[idx++] = 0x10;
                meter_send_buffer[idx++] = 0x34;
                meter_send_buffer[idx++] = 0x34;
                meter_send_buffer[idx++] = 0x33;
                meter_send_buffer[idx++] = 0x37;
                meter_send_buffer[idx++] =0x35;meter_send_buffer[idx++] =0x89;meter_send_buffer[idx++] =0x67;meter_send_buffer[idx++] =0x45;
                meter_send_buffer[idx++] =0xAB;meter_send_buffer[idx++] =0x89;meter_send_buffer[idx++] =0x67;meter_send_buffer[idx++] =0x45;
                meter_send_buffer[idx++] =str[3] +0x33;meter_send_buffer[idx++] =str[2] +0x33;
                meter_send_buffer[idx++] =str[1] +0x33;meter_send_buffer[idx++] = str[0] +0x33;
               // for(idx=22;idx<pos;idx++) frame[idx]+=0x33;
                cs = 0;
                for(pos =0;pos<idx;pos++) cs += meter_send_buffer[pos];
                meter_send_buffer[idx++] = cs;
                meter_send_buffer[idx++] =0x16;
                app_trans_send_meter_frame(meter_send_buffer,28, resp,200,50);
                
                //写入时分秒
                idx = 0; //组报文初始位置
                meter_send_buffer[idx++] =0x68;
                meter_send_buffer[idx++] =gSystemInfo.meter_no[0];meter_send_buffer[idx++] =gSystemInfo.meter_no[1];meter_send_buffer[idx++] =gSystemInfo.meter_no[2];
                meter_send_buffer[idx++] =gSystemInfo.meter_no[3];meter_send_buffer[idx++] =gSystemInfo.meter_no[4];meter_send_buffer[idx++] =gSystemInfo.meter_no[5];
                meter_send_buffer[idx++] = 0x68;
                meter_send_buffer[idx++] = 0x14;
                meter_send_buffer[idx++] = 0x0F;
                meter_send_buffer[idx++] = 0x35;
                meter_send_buffer[idx++] = 0x34;
                meter_send_buffer[idx++] = 0x33;
                meter_send_buffer[idx++] = 0x37;
                meter_send_buffer[idx++] =0x35;meter_send_buffer[idx++] =0x89;meter_send_buffer[idx++] =0x67;meter_send_buffer[idx++] =0x45;
                meter_send_buffer[idx++] =0xAB;meter_send_buffer[idx++] =0x89;meter_send_buffer[idx++] =0x67;meter_send_buffer[idx++] =0x45;
                meter_send_buffer[idx++] =str[6] +0x33;meter_send_buffer[idx++] =str[5] +0x33;
                meter_send_buffer[idx++] =str[4] +0x33;
               // for(idx=22;idx<pos;idx++) frame[idx]+=0x33;
                cs = 0;
                for(pos =0;pos<idx;pos++) cs += meter_send_buffer[pos];
                meter_send_buffer[idx++] = cs;
                meter_send_buffer[idx++] =0x16;
                app_trans_send_meter_frame(meter_send_buffer,27, resp,200,50);
                
        mem_set(resp,200,0x00);
        sprintf((char*)resp,"AT+MIPLWRITERSP=0,%ld,2\r\n", Obj->mid);
        remote_send_app_frame(resp,str_cmdlen(resp));
                
            }

        //透传通道
        if(pLib->data_type.convert_type == REMOTE_UPDATE)
        {
            if(Obj->recv_buf[0]=='"')   //如果下发的升级报文包括在“”之内，那么走if路径 否则走else路径
            {
                for(idx = 0;idx <Obj->len -2;)
                {
                    if(Obj->recv_buf[idx+1] < 'A')
                        RequestRemote.frame[idx/2] = (Obj->recv_buf[idx+1]-'0')<<4;
                    else
                        RequestRemote.frame[idx/2] = (Obj->recv_buf[idx+1]-'A'+0xA)<<4;

                    if(Obj->recv_buf[idx+2] < 'A')
                        RequestRemote.frame[idx/2] = (Obj->recv_buf[idx+2]-'0') +RequestRemote.frame[idx/2] ;
                    else
                        RequestRemote.frame[idx/2] = (Obj->recv_buf[idx+2]-'A'+0xA) +RequestRemote.frame[idx/2];
                    idx = idx +2;
                }
            }
            else
            {
                for(idx = 0;idx <Obj->len;)
                    {
                        if(Obj->recv_buf[idx] < 'A')
                            RequestRemote.frame[idx/2] = (Obj->recv_buf[idx]-'0')<<4;
                        else
                            RequestRemote.frame[idx/2] = (Obj->recv_buf[idx]-'A'+0xA)<<4;

                        if(Obj->recv_buf[idx+1] < 'A')
                            RequestRemote.frame[idx/2] = (Obj->recv_buf[idx+1]-'0') +RequestRemote.frame[idx/2] ;
                        else
                            RequestRemote.frame[idx/2] = (Obj->recv_buf[idx+1]-'A'+0xA) +RequestRemote.frame[idx/2];
                        idx = idx +2;
                    }
            }

           
        Request = (objRequest*)&RequestRemote;
        Request->frame_len = get_1376_1_frame_len(Request->frame,MAX_FRAME_HEADER_SIZE);
        Request->cs  = init_1376_1_frame_checksum(Request->frame,Request->frame_len,Request->frame_len+1); //要保证接受的数据大于预先设计传输的字节。
        if(is_1376_1_frame_ready(Request->frame,Request->frame_len,Request->frame_len+1,&(Request->cs)))
        {
            //置协议标识
            Request->cur_protocol.value = 0;
            Request->cur_protocol.GDW1376 = 1;
            Request->recv_time_in_10ms = system_get_tick10ms();
            Request->state = REQUEST_STATE_WAIT_PROCESS;
            Request->recv_success_flag = 1;
        }         
        else
        {
            Request->cur_protocol.GDW1376 = 0;
        }  
      } 
//      		mem_set(resp,200,0x00);
//      		sprintf((char*)resp,"AT+MIPLWRITERSP=0,%ld,2\r\n", Obj->mid);
//      		remote_send_app_frame(resp,str_cmdlen(resp));
    }
    }
    else if(Obj->msg_type == ONENET_EXECUTE)
    {
    
    }

}


/*********************************
 * frame  需要已转换string
 * len    string类型长度
 ********************************/
void app_report_onenet_frame(tagWirelessObj *Obj,INT8U* frame,INT16U len,INT8U need_quotation)
{
    
//    if(Obj->type == ((STRING&0x0F00)>>8)) 
    if(need_quotation)
    {
        sprintf((char*)Obj->send_buf,"AT+MIPLNOTIFY=0,0,%4d,%d,%ld,%d,%d,\"",Obj->objid,Obj->insid,Obj->resid,Obj->type,len);
    }
    else
    {
        sprintf((char*)Obj->send_buf,"AT+MIPLNOTIFY=0,0,%4d,%d,%ld,%d,%d,",Obj->objid,Obj->insid,Obj->resid,Obj->type,len);        
    }  
    Obj->send_len = str_cmdlen(Obj->send_buf);    

    mem_cpy(Obj->send_buf+Obj->send_len,frame,len);
    Obj->send_len += len;
    
    if(need_quotation)
    {
        sprintf((char*)Obj->send_buf+Obj->send_len,"\",0,0\r\n");
        Obj->send_len+=7;
    }
    else
    {
        sprintf((char*)Obj->send_buf+Obj->send_len,",0,0\r\n");
        Obj->send_len+=8;
    }

    remote_send_app_frame(Obj->send_buf,Obj->send_len); 
}

//上报前先检查ONENET在线状态 不在线需要重新拨号 
BOOLEAN check_update_lifetime(void)
{
    INT8U frame[50]={0};
    static INT8U err=0,step=0;  

    tpos_mutexPend(&SIGNAL_SYSTEMINFO);
    if(step==0)
    {
        if(gSystemInfo.check_update_tick==0)
        {
            mem_set(frame,50,0x00);
            sprintf(frame,"AT+MIPLUPDATE=0,4200,0\r\n");
            remote_send_app_frame(frame,str_cmdlen(frame));
            gSystemInfo.check_update_tick = system_get_tick10ms();
            step++;
        }
        else
        {
            gSystemInfo.check_update_tick=0;//
        }
    }
    else if(step==1)
    {
        if(gSystemInfo.check_update_tick==0)//收到正确响应
        {
            tpos_mutexFree(&SIGNAL_SYSTEMINFO);
            step=0;  
            err=0;            
            return TRUE;
        }
        else if(second_elapsed(gSystemInfo.check_update_tick)>10)//超时
        {
            if(err++>1)
            {
                gSystemInfo.tcp_link=0;//掉线重连
                gSystemInfo.login_status=0;
                step=0;  
                err=0;
            }
            else
            {
                gSystemInfo.check_update_tick=0;//重发
                step=0;  
            }                
        }
    }
    tpos_mutexFree(&SIGNAL_SYSTEMINFO);
    return FALSE;
}
void task_data_report_onenet(void)
{
    INT32U passed_sec=0,last_res=0;
    INT32U rpt_cycle=0,diff=0;
    static INT32U last_tick=0,last_update=0;
    INT16U res_len=0;
    INT8U idx1=0,buffer[5]={0},debug[30]={0};
    BOOLEAN is_valid=FALSE;
    tagDatetime datetime;
    static INT8U first_run=0,power_off=0,hour=0;
    tagResourceId *pLib;
    tagWirelessObj *Obj;
    
    Obj = &ReportOnenet;
    if(gSystemInfo.tcp_link==0)
    {
        return;
    }
    if(gSystemInfo.login_status!=1)
    {
        return;
    }    
    if(second_elapsed(last_tick) < 2)
    {
        return;
    }
    else
    {
        last_tick = system_get_tick10ms();
    }
    
    if((Obj->state==REQUEST_STATE_IDLE)||(Obj->state==REQUEST_STATE_PROCESSED))
    {
    }
    else
    {
        return;
    }
    //升级时使用离线命令需要上报才能触发 进入升级状态后 每30秒上报一帧数据持续触发
    if((gSystemInfo.update ==1)&&(second_elapsed(last_update) > 30))
    {
       	mem_set(Obj,sizeof(tagWirelessObj),0x00);
       	sprintf(Obj->send_buf,"AT+MIPLNOTIFY=0,0,3332,13,5750,1,1,\"0\",0,0\r\n");
       	remote_send_app_frame(Obj->send_buf,str_cmdlen(Obj->send_buf));
       	last_update = system_get_tick10ms();    	
    }
    os_get_datetime(&datetime);
    if(datetime.hour!=hour)
    {
//        mem_set(Obj,sizeof(tagWirelessObj),0x00);
//        sprintf(Obj->send_buf,"AT+MIPLOPEN?\r\n");//task_nb_onenet 检查结果
//       	remote_send_app_frame(Obj->send_buf,str_cmdlen(Obj->send_buf));
//        
//        mem_set(Obj,sizeof(tagWirelessObj),0x00);
//        sprintf(Obj->send_buf,"AT+MIPLUPDATE=0,4200,0\r\n");
//       	remote_send_app_frame(Obj->send_buf,str_cmdlen(Obj->send_buf));
        if(check_update_lifetime())
        {
            hour=datetime.hour;
        }
        else
            return;
    }
    
    //预留1分钟电表冻结时间
    if((datetime.hour==0)&&(datetime.minute < 1))
    {
        return;
    }
    
    //task.rpt_flag 控制是否需要上报 
    fread_ertu_params(EEADDR_TASK_REPORT_CYCLE,buffer,4);
    rpt_cycle = bcd2u32(buffer,4,&is_valid);
    if((is_valid==FALSE)||(rpt_cycle==0))
    {
        rpt_cycle = 60;
        buffer[0]=0x60;
        buffer[1]=0x00;
        buffer[2]=0x00;
        buffer[3]=0x00;
        fwrite_ertu_params(EEADDR_TASK_REPORT_CYCLE,buffer,4);
    }
    
    //日月冻结按照日月间隔存储时间，周期数据使用内存变量存储，不写flash，上电会重新上报周期数据
    //主要周期间隔可设置，减少写次数
    if(first_run==0)
    {
        task.ack_idx = 0;
        task.rpt_idx = 0;
        task.rpt_flag = 0;
    	first_run = 1;	
    }

    fread_ertu_params(EEADDR_TASK_REPORT,task.value,sizeof(tagDatetime)*2);  
    if(task.rpt_idx>(sizeof(RESOURCE_ID)/sizeof(tagResourceId)))
    {
        mem_set(task.value,sizeof(TASK_INFO),0x00);
        return;
    }
    
    if((task.rpt_flag==0)||(task.rpt_flag==0xFF))
    {
        if((rpt_cycle == 0xFFFFFFFF)||(rpt_cycle == 0))
        {
            rpt_cycle = 60;
        }
        passed_sec =  diff_sec_between_dt(&datetime,&task.cycle_report_time);
        if(datetime.day != task.dayhold_report_time.day)
        {
            task.rpt_flag |= DAYHOLD_DATA;
            gSystemInfo.allow_charge=0;//发送前关闭充电
        }
        if(datetime.month != task.monthhold_report_time.month)
        {
            task.rpt_flag |= MONTHHOLD_DATA;
            gSystemInfo.allow_charge=0;//发送前关闭充电
        }        
        if(passed_sec > rpt_cycle*60)
        {
            task.rpt_flag |= CYCLE_DATA;
            gSystemInfo.allow_charge=0;//发送前关闭充电
        }
    }
    else
    {
        task.rpt_idx++;
    }
            
    //未到上报周期
    if((task.rpt_flag==0)||(task.rpt_flag==0xFF))
    {
        return;
    }

    if((gSystemInfo.csq==99)||(gSystemInfo.csq==0))
    {
        gSystemInfo.tcp_link=0;
        gSystemInfo.login_status = 0;
        return;
    }
    
    mem_set(Obj,sizeof(tagWirelessObj),0x00);
    for(;task.rpt_idx<(sizeof(RESOURCE_ID)/sizeof(tagResourceId));task.rpt_idx++)
    {
        pLib = &(RESOURCE_ID)[task.rpt_idx];
        if(pLib->data_type.report_type == 0)
        {
            continue;
        }
        if((task.rpt_flag & pLib->data_type.value)==0)
        {
            continue;
        }
        if(pLib->item!=0xFFFFFFFF)
        {
            //考虑重发最多2次
            for(idx1=0;idx1<2;idx1++)
            {
                read_c1_time_out = 1000;
                res_len = app_read_his_item(pLib->item,Obj->recv_buf,Obj->send_buf,255,NULL,1000);
                if(res_len)
                {
                    break;
                }
            }
        }
        //ack_id 使用表内序号作为上报标志 按位计算偏移地址
        Obj->ackid = task.rpt_idx;
        res_len = app_data_convert(Obj->recv_buf,pLib->data_type.convert_type);
        Obj->insid = pLib->instance;
        Obj->objid = pLib->obj;
        Obj->resid = pLib->id;
        Obj->type = pLib->data_type.type;
        Obj->mid = 0;
        Obj->result = 1;
        app_report_onenet_frame(Obj,Obj->recv_buf,res_len,0);
        if(task.rpt_idx<(sizeof(RESOURCE_ID)/sizeof(tagResourceId)))
        {
            #ifdef __DEBUG_INFO__
            system_debug_info("rpt_idx:");
            debug[0]=task.rpt_idx&0xF0+'0';
            debug[1]=task.rpt_idx&0x0F+'0';
            system_debug_data(debug,2);
            #endif
            return;
        }
    }
    #ifdef __DEBUG_INFO__
    system_debug_info("report end!");
    #endif
    if(task.rpt_idx==sizeof(RESOURCE_ID)/sizeof(tagResourceId))
    {    
        //全部上报完成初始化变量 更新上报时间
        os_get_datetime(&datetime);
        if(task.rpt_flag & DAYHOLD_DATA)
        {
            mem_cpy(task.dayhold_report_time.value,datetime.value,sizeof(tagDatetime));
            //记录上报时间为整点 防止上报过程占用的时间累加
            task.dayhold_report_time.hour = 0;
            task.dayhold_report_time.minute = 0;
            task.dayhold_report_time.second = 0;
            fwrite_ertu_params(EEADDR_TASK_REPORT,task.value,sizeof(TASK_INFO));
            #ifdef __DEBUG_INFO__
            system_debug_info("clear dayhold flag");
            #endif
        }
        if(task.rpt_flag & MONTHHOLD_DATA)
        {
            mem_cpy(task.monthhold_report_time.value,datetime.value,sizeof(tagDatetime));
            //记录上报时间为整点 防止上报过程占用的时间累加
            task.monthhold_report_time.day = 1;
            task.monthhold_report_time.hour = 0;
            task.monthhold_report_time.minute = 0;
            task.monthhold_report_time.second = 0;
            fwrite_ertu_params(EEADDR_TASK_REPORT,task.value,sizeof(TASK_INFO));
            #ifdef __DEBUG_INFO__
            system_debug_info("clear monthhold flag");
            #endif
        }        
        if(task.rpt_flag & CYCLE_DATA)
        {
            mem_cpy(task.cycle_report_time.value,datetime.value,sizeof(tagDatetime));
            //记录上报时间为整分钟 防止上报过程占用的时间累加
            datetime.hour=0;
            datetime.minute=0;
            datetime.second=0;
            diff = diff_sec_between_dt(&datetime,&task.cycle_report_time);
            diff = diff % (rpt_cycle*60);
            datetime_minus_seconds(&task.cycle_report_time.year, &task.cycle_report_time.month, &task.cycle_report_time.day, &task.cycle_report_time.hour, &task.cycle_report_time.minute, &task.cycle_report_time.second, diff);
        	#ifdef __DEBUG_INFO__
        	system_debug_info("update！");
        	system_debug_info("clear cycle");
        	#endif
        } 
        task.ack_idx = 0;
        task.rpt_idx = 0;
        task.rpt_flag = 0;
        gSystemInfo.allow_charge=1;
    }
    else 
    {
        mem_set(task.value,sizeof(TASK_INFO),0x00);
    }
}

