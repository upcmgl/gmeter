#include "cjsonFramePack.h"
//#include "stdio.h"
#include "heap.h"

#ifdef __BEIJING_ONENET_EDP__

//设备配置信息
INT16U  cjsonDeviceConfigPack(INT8U * buf)
{
	cJSON *root, *datestreamsArray, *datestreamsContent, *datapoint, *datapointCotent, *value;
	char *out;
	INT16U resLen =0;
	INT8U tmpData[30];
	INT8S str_server_ip[20];
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "datastreams", datestreamsArray = cJSON_CreateArray());
	cJSON_AddItemToArray(datestreamsArray, datestreamsContent = cJSON_CreateObject());
	cJSON_AddStringToObject(datestreamsContent, "id", "event");
	cJSON_AddItemToObject(datestreamsContent, "datapoints", datapoint = cJSON_CreateArray());
	cJSON_AddItemToArray(datapoint, datapointCotent = cJSON_CreateObject());
	cJSON_AddItemToObject(datapointCotent, "value", value = cJSON_CreateObject());
	 mem_set(tmpData,30, 0x00);
	 sprintf(tmpData,"%s",gSystemInfo.managementNum);
	cJSON_AddStringToObject(value, "meterNum", tmpData); ///"1111111111111111111111");
	mem_set(tmpData,30, 0x00);
	sprintf(tmpData, "%.20s", gSystemInfo.myccid_info);
	cJSON_AddStringToObject(value, "iccid",tmpData); // "898602B8231700769443");
	cJSON_AddNumberToObject(value, "interval", gSystemInfo.edp_report_interval);
	mem_set(tmpData,30, 0x00);
	sprintf(tmpData, "%.15s", gSystemInfo.imei_info);
	cJSON_AddStringToObject(value, "imei", tmpData); //"861477030064636");
	if(FILEFLAG_TYPE == FILEFLAG_TGP_GMTER_G55)
	{
		cJSON_AddNumberToObject(value, "meterType", 301);   //301代表三相，101代表单相。
	}
	else if(FILEFLAG_TYPE == FILEFLAG_SGP_GMTER_G55)
	{
		cJSON_AddNumberToObject(value, "meterType", 101);   //301代表三相，101代表单相。
	}	
     ip2str(gSystemInfo.ppp_ip, str_server_ip);
     mem_set(tmpData,30, 0x00);
     sprintf(tmpData, "%s", str_server_ip);	
	cJSON_AddStringToObject(value, "ip", tmpData); //"10.146.238.6");
	cJSON_AddNumberToObject(value, "moduleType", 2); //通信模块的类型（4G设置为1，NB设置为2，2G设置为3）
	mem_set(tmpData,30, 0x00);
	sprintf(tmpData, "%s", edp_version);
	cJSON_AddStringToObject(value, "version", tmpData);

	out = cJSON_PrintUnformatted(root);
	//printf("%s", out);
	cJSON_Delete(root);
	mem_cpy(buf,out,resLen = strlen(out));
	vPortFree(out);
	//	cJSON_Delete(out);
	return resLen;

}
//组过流告警字符串
void makeOverFlow(INT8U *buf,INT8U len)
{
	INT8U str1[200],str2[200],i =0,bufPos =0,length =0;
	INT32U AphaseOverFlowCnt =0,BphaseOverFlowCnt =0,CphaseOverFlowCnt =0;
	INT32U AphaseOverFlowTime =0,BphaseOverFlowTime =0,CphaseOverFlowTime =0;
	INT8U AphaseOverFlowTimeOccur[6] = {0},BphaseOverFlowTimeOccur[6] = {0},CphaseOverFlowTimeOccur[6] = {0};
////////////////////////////////////读A相数据//////////////////////////////////////////////////////////////		
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF); 
	length =app_read_his_item(0x19010001,str1,str2,255,0,2000);  //A相过流总次数 抄表的结果应该放在str1里。
	if(length)
	{
		AphaseOverFlowCnt = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
		gSystemInfo.edp_over_flow_A = AphaseOverFlowCnt;
		fwrite_ertu_params(EEADDR_OVERFLOW_WARNING_A,(INT8U*)(&gSystemInfo.edp_over_flow_A),4);			
	}
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x19010002,str1,str2,255,0,2000);  //A相过流总时间 抄表的结果应该放在str1里。
	if(length)
		AphaseOverFlowTime = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x19010002,str1,str2,255,0,2000);  //A相发生过流时刻 抄表的结果应该放在str1里。
	if(length)
	{
		for(i =0;i<6;i++)
		{
			AphaseOverFlowTimeOccur[i] =str1[i];
		}
	}	
////////////////////////////////////读B相数据//////////////////////////////////////////////////////////////
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x19020001,str1,str2,255,0,2000);  //A相过流总次数 抄表的结果应该放在str1里。
	if(length)
	{
		BphaseOverFlowCnt = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
		gSystemInfo.edp_over_flow_B = BphaseOverFlowCnt;
		fwrite_ertu_params(EEADDR_OVERFLOW_WARNING_B,(INT8U*)(&gSystemInfo.edp_over_flow_B),4);		
	}
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length = app_read_his_item(0x19020002,str1,str2,255,0,2000);  //A相过流总时间 抄表的结果应该放在str1里。
	if(length)
		BphaseOverFlowTime = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length = app_read_his_item(0x19020002,str1,str2,255,0,2000);  //A相发生过流时刻 抄表的结果应该放在str1里。
	if(length)
	{
		for(i =0;i<6;i++)
		{
			BphaseOverFlowTimeOccur[i] =str1[i];
		}
	}	

	////////////////////////////////////读C相数据//////////////////////////////////////////////////////////////
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length = app_read_his_item(0x19030001,str1,str2,255,0,2000);  //A相过流总次数 抄表的结果应该放在str1里。
	if(length)
	{
			CphaseOverFlowCnt = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
			gSystemInfo.edp_over_flow_C =CphaseOverFlowCnt;
			fwrite_ertu_params(EEADDR_OVERFLOW_WARNING_C,(INT8U*)(&gSystemInfo.edp_over_flow_C),4);			
	}
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x19030002,str1,str2,255,0,2000);  //A相过流总时间 抄表的结果应该放在str1里。
	if(length)
		CphaseOverFlowTime = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x19030002,str1,str2,255,0,2000);  //A相发生过流时刻 抄表的结果应该放在str1里。
	if(length)
	{
		for(i =0;i<6;i++)
		{
			CphaseOverFlowTimeOccur[i] =str1[i];
		}
	}
//	*(buf+bufPos++) ='0';*(buf+bufPos++) ='$';*(buf+bufPos++) ='A';*(buf+bufPos++) ='&';sprintf(buf,"%d",AphaseOverFlowCnt);
	sprintf(buf+strlen(buf),"0$",NULL);
	sprintf(buf+strlen(buf),"A&%d&%d",AphaseOverFlowCnt,AphaseOverFlowTime);
	if(AphaseOverFlowTimeOccur[5]!=0x00)
		sprintf(buf+strlen(buf),"&%d%d-%d%d-%d%d %d%d:%d%d:%d%d",AphaseOverFlowTimeOccur[5]/16,AphaseOverFlowTimeOccur[5]%16,AphaseOverFlowTimeOccur[4]/16,(AphaseOverFlowTimeOccur[4]%16),AphaseOverFlowTimeOccur[3]/16,(AphaseOverFlowTimeOccur[3]%16),AphaseOverFlowTimeOccur[2]/16,(AphaseOverFlowTimeOccur[2]%16),AphaseOverFlowTimeOccur[1]/16,(AphaseOverFlowTimeOccur[1]%16),AphaseOverFlowTimeOccur[0]/16,(AphaseOverFlowTimeOccur[0]%16));
	sprintf(buf+strlen(buf),";",NULL);
	sprintf(buf+strlen(buf),"B&%d&%d",BphaseOverFlowCnt,BphaseOverFlowTime);
	if(BphaseOverFlowTimeOccur[5]!=0x00)
		sprintf(buf+strlen(buf),"&%d%d-%d%d-%d%d %d%d:%d%d:%d%d",BphaseOverFlowTimeOccur[5]/16,BphaseOverFlowTimeOccur[5]%16,BphaseOverFlowTimeOccur[4]/16,(BphaseOverFlowTimeOccur[4]%16),BphaseOverFlowTimeOccur[3]/16,(BphaseOverFlowTimeOccur[3]%16),BphaseOverFlowTimeOccur[2]/16,(BphaseOverFlowTimeOccur[2]%16),BphaseOverFlowTimeOccur[1]/16,(BphaseOverFlowTimeOccur[1]%16),BphaseOverFlowTimeOccur[0]/16,(BphaseOverFlowTimeOccur[0]%16));
	sprintf(buf+strlen(buf),";",NULL);
	sprintf(buf+strlen(buf),"C&%d&%d",CphaseOverFlowCnt,CphaseOverFlowTime);
	if(CphaseOverFlowTimeOccur[5]!=0x00)
		sprintf(buf+strlen(buf),"&%d%d-%d%d-%d%d %d%d:%d%d:%d%d",CphaseOverFlowTimeOccur[5]/16,CphaseOverFlowTimeOccur[5]%16,CphaseOverFlowTimeOccur[4]/16,(CphaseOverFlowTimeOccur[4]%16),CphaseOverFlowTimeOccur[3]/16,(CphaseOverFlowTimeOccur[3]%16),CphaseOverFlowTimeOccur[2]/16,(CphaseOverFlowTimeOccur[2]%16),CphaseOverFlowTimeOccur[1]/16,(CphaseOverFlowTimeOccur[1]%16),CphaseOverFlowTimeOccur[0]/16,(CphaseOverFlowTimeOccur[0]%16));
	len =strlen(buf);
	
}
//组开表盖告警字符串
void makeOpenMeter(INT8U *buf,INT8U len)
{
	INT8U str1[200],str2[200],i =0,bufPos =0,length =0;
	INT16U openCnt =0;
	INT8U openTimeOccur[6] = {0};
	////////////////////////////////////读A相数据//////////////////////////////////////////////////////////////
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x03300D00,str1,str2,255,0,2000);  //开盖总次数 抄表的结果应该放在str1里。
	if(length)
	{
			openCnt = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
			gSystemInfo.edp_open_meter = openCnt;
			fwrite_ertu_params(EEADDR_OPENMETER_WARNING,(INT8U*)(&gSystemInfo.edp_open_meter),4);			
	}
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x03300D01,str1,str2,255,0,2000);  //上一次开盖时间 抄表的结果应该放在str1里。
	if(length)
	{
		for(i =0;i<6;i++)
		{
			openTimeOccur[i] =str1[i];
		}
	}
	sprintf(buf+strlen(buf),"1$",NULL);
	sprintf(buf+strlen(buf),"%d&",openCnt);
	sprintf(buf+strlen(buf),"&%d%d-%d%d-%d%d %d%d:%d%d:%d%d",openTimeOccur[5]/16,openTimeOccur[5]%16,openTimeOccur[4]/16,(openTimeOccur[4]%16),openTimeOccur[3]/16,(openTimeOccur[3]%16),openTimeOccur[2]/16,(openTimeOccur[2]%16),openTimeOccur[1]/16,(openTimeOccur[1]%16),openTimeOccur[0]/16,(openTimeOccur[0]%16));
	len =strlen(buf);
}
//组电表清零告警字符串
void makeCleanMeter(INT8U *buf,INT8U len)
{
	INT8U str1[200],str2[200],i =0,bufPos =0,length =0;
	INT16U cleanCnt =0;
	INT8U cleanTimeOccur[6] = {0};
	////////////////////////////////////读A相数据//////////////////////////////////////////////////////////////
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x03300100,str1,str2,255,0,2000);  //开盖总次数 抄表的结果应该放在str1里。
	if(length)
	{
		cleanCnt = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
		gSystemInfo.edp_clean_meter =cleanCnt;
		fwrite_ertu_params(EEADDR_CLEANNMETER_WARNING,(INT8U*)(&gSystemInfo.edp_clean_meter),4);		
	}
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x03300101,str1,str2,255,0,2000);  //上一次开盖时间 抄表的结果应该放在str1里。
	if(length)
	{
		for(i =0;i<6;i++)
		{
			cleanTimeOccur[i] =str1[i];
		}
	}
	sprintf(buf+strlen(buf),"2$",NULL);
	sprintf(buf+strlen(buf),"%d&",cleanCnt);
	sprintf(buf+strlen(buf),"&%d%d-%d%d-%d%d %d%d:%d%d:%d%d",cleanTimeOccur[5]/16,cleanTimeOccur[5]%16,cleanTimeOccur[4]/16,(cleanTimeOccur[4]%16),cleanTimeOccur[3]/16,(cleanTimeOccur[3]%16),cleanTimeOccur[2]/16,(cleanTimeOccur[2]%16),cleanTimeOccur[1]/16,(cleanTimeOccur[1]%16),cleanTimeOccur[0]/16,(cleanTimeOccur[0]%16));
	len =strlen(buf);
}
//事件告警信息
INT16U cjsonEventWarningPack(INT8U *buf,INT8U flag)
{
	cJSON *root, *datestreamsArray, *datestreamsContent, *datapoint, *datapointCotent, *value;
	char *out;
	INT16U resLen =0;
	INT8U buftmp[200] ={0},len =0;
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "datastreams", datestreamsArray = cJSON_CreateArray());
	cJSON_AddItemToArray(datestreamsArray, datestreamsContent = cJSON_CreateObject());
	cJSON_AddStringToObject(datestreamsContent, "id", "event");
	cJSON_AddItemToObject(datestreamsContent, "datapoints", datapoint = cJSON_CreateArray());
	cJSON_AddItemToArray(datapoint, datapointCotent = cJSON_CreateObject());
	cJSON_AddItemToObject(datapointCotent, "value", value = cJSON_CreateObject());
	switch(flag)
	{
		case 1:  //过流告警
			makeOverFlow(buftmp,len);
			break;
		case 2:  //开表盖告警
			makeOpenMeter(buftmp,len);
			break;
		case 3:   // 电能表清零告警
			makeCleanMeter(buftmp,len);
			break;
		default:
			break;
	}
//	cJSON_AddStringToObject(value, "topwarn", "2$1&17-04-01 13:05:47;B&0&0;C&0&0;");
	cJSON_AddStringToObject(value, "topwarn", buftmp);


	out = cJSON_PrintUnformatted(root);
	mem_cpy(buf,out,resLen = strlen(out));
	cJSON_Delete(root);
	vPortFree(out);
	return resLen;
}
//固件升级信息
INT16U cjsonFirmwareUpdatePack(INT8U *buf,INT8U flag)
{
	cJSON *root, *datestreamsArray, *datestreamsContent, *datapoint, *datapointCotent, *value;
	char *out;
	INT16U resLen = 0;
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "datastreams", datestreamsArray = cJSON_CreateArray());
	cJSON_AddItemToArray(datestreamsArray, datestreamsContent = cJSON_CreateObject());
	cJSON_AddStringToObject(datestreamsContent, "id", "event");
	cJSON_AddItemToObject(datestreamsContent, "datapoints", datapoint = cJSON_CreateArray());
	cJSON_AddItemToArray(datapoint, datapointCotent = cJSON_CreateObject());
	cJSON_AddItemToObject(datapointCotent, "value", value = cJSON_CreateObject());
	cJSON_AddNumberToObject(value, "fota", flag);

	out = cJSON_PrintUnformatted(root);
	mem_cpy(buf,out,resLen = strlen(out));
	cJSON_Delete(root);
	vPortFree(out);
	return resLen;
}
//组报文透传抄表数据。
INT16U cjsonReadMeter(INT8U *buf)
{
	cJSON *root, *datestreamsArray, *datestreamsContent, *datapoint, *datapointCotent, *value;
	char *out;
	INT16U resLen =0;
	INT8U buftmp[10] ={0},len =0,i=0;
	INT8U str1[200]={0},str2[400]={0};	
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "datastreams", datestreamsArray = cJSON_CreateArray());
	cJSON_AddItemToArray(datestreamsArray, datestreamsContent = cJSON_CreateObject());
	cJSON_AddStringToObject(datestreamsContent, "id", "event");
	cJSON_AddItemToObject(datestreamsContent, "datapoints", datapoint = cJSON_CreateArray());
	cJSON_AddItemToArray(datapoint, datapointCotent = cJSON_CreateObject());
	cJSON_AddItemToObject(datapointCotent, "value", value = cJSON_CreateObject());
	//cJSON_AddStringToObject(value, "topwarn", "2$1&17-04-01 13:05:47;B&0&0;C&0&0;");
	sprintf(buftmp,"%08x",cmd645);
	cJSON_AddStringToObject(value, "cmd", buftmp);
	
	len = app_read_his_item(cmd645,str1,str2,255,0,2000);//str1中存在着我们要读取的值；组合有功。
	mem_set(str2,400,0x00);
	for(i =0;i<len;i++)
	{
		if(((str1[i]>>4)&0x0F)>=0 &&((str1[i]>>4)&0x0F)<=9)
			str2[2*i] = ((str1[i]>>4)&0x0F) +'0';
		else if(((str1[i]>>4)&0x0F)>=10 &&((str1[i]>>4)&0x0F)<=15)
			str2[2*i] = ((str1[i]>>4)&0x0F) +'A';
			
		if(((str1[i])&0x0F)>=0 &&((str1[i])&0x0F)<=9)
			str2[2*i+1] = ((str1[i])&0x0F) +'0';
		else if(((str1[i])&0x0F)>=10 &&((str1[i])&0x0F)<=15)
			str2[2*i+1] = ((str1[i])&0x0F) +'A';
	}
	cJSON_AddStringToObject(value, "echo", str2);
	
	out = cJSON_PrintUnformatted(root);
	mem_cpy(buf,out,resLen = strlen(out));
	cJSON_Delete(root);
	vPortFree(out);
	return resLen;
}
//实时电量信息
INT16U cjsonEnergyInfoPack(char dayhold,INT8U *buf)
{
	cJSON *root, *datestreamsArray, *datestreamsContent, *datapoint, *datapointCotent, *value, *energy;
	char *out;
	INT8U tmpData[30] ={0},length =0;
    INT8U str1[100],str2[100];	
	INT32U energy_int[2];	
	tagDatetime  datetime;
	INT16U resLen =0;
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "datastreams", datestreamsArray = cJSON_CreateArray());
	cJSON_AddItemToArray(datestreamsArray, datestreamsContent = cJSON_CreateObject());
	cJSON_AddStringToObject(datestreamsContent, "id", "value");
	cJSON_AddItemToObject(datestreamsContent, "datapoints", datapoint = cJSON_CreateArray());
	cJSON_AddItemToArray(datapoint, datapointCotent = cJSON_CreateObject());
	cJSON_AddItemToObject(datapointCotent, "value", value = cJSON_CreateObject());
	//mem_set(tmpData,30, 0x00);
	//sprintf(tmpData,"%d",gSystemInfo.csq);	
	//cJSON_AddStringToObject(value, "SignalLevel", tmpData);
	cJSON_AddNumberToObject(value, "SignalLevel", gSystemInfo.csq);
	cJSON_AddItemToObject(value, "UserData", energy = cJSON_CreateObject());
	mem_set(tmpData,30, 0x00);
	sprintf(tmpData,"%s",gSystemInfo.managementNum);
	cJSON_AddStringToObject(energy, "meterNum", tmpData); ///"1111111111111111111111");
	
	os_get_datetime(&datetime);
	 //	str = "\"17-08-02 10:52:01\",";
	sprintf(tmpData,"%d-%d%d-%d%d %d%d:%d%d:%d%d",datetime.year,datetime.month/10,(datetime.month%10),datetime.day/10,(datetime.day%10),datetime.hour/10,(datetime.hour%10),datetime.minute/10,(datetime.minute%10),datetime.second/10,(datetime.second%10));
	cJSON_AddStringToObject(energy, "time", tmpData);
	{
		if(dayhold ==0)
		{
			length =app_read_his_item(0x0000FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；组合有功。
		}
		else if(dayhold==1)
		{
			length =app_read_his_item(0x05060101,str1,str2,255,0,2000);//str1中存在着我们要读取的值；日冻结正向有功。
		}
		if(length>0)
		{
			energy_int[0] = (BCD2byte(str1[3])*10000) +(BCD2byte(str1[2])*100)+(BCD2byte(str1[1]));
			energy_int[1] = BCD2byte(str1[0]);	 	
			mem_set(tmpData,30, 0x00);
			sprintf(tmpData,"%d.%02d",energy_int[0],energy_int[1]);
			cJSON_AddStringToObject(energy, "kwh", tmpData);

		}
		else
		{
			 cJSON_AddStringToObject(energy, "kwh", "FFFFFF.FF");
		}

	}	
		cJSON_AddNumberToObject(energy, "frozen", dayhold);
	

	out = cJSON_PrintUnformatted(root);
	
	mem_cpy(buf,out,resLen = strlen(out));
	cJSON_Delete(root);
	vPortFree(out);
	return resLen;

}
//电能量信息数据块
INT16U cjsonEnergyBlockPack(INT8U *buf)
{
	cJSON *root, *datestreamsArray, *datestreamsContent, *datapoint, *datapointCotent, *value, *energy;
	char *out;
	INT16U resLen =0,len;
    INT8U str1[100],str2[100];
    INT16U vol[6];  //转换用于电压
    INT16S cur[6];  //转换用于电流
    INT16S insP[8];  //瞬时有功  总+ABC
    INT16S insQ[8];  //瞬时无功   总+ABC
    INT16S pwrFactor[8];  //功率因数。 总+ABC	
	INT8U tmpData[50] ={0};
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "datastreams", datestreamsArray = cJSON_CreateArray());
	cJSON_AddItemToArray(datestreamsArray, datestreamsContent = cJSON_CreateObject());
	cJSON_AddStringToObject(datestreamsContent, "id", "value");
	cJSON_AddItemToObject(datestreamsContent, "datapoints", datapoint = cJSON_CreateArray());
	cJSON_AddItemToArray(datapoint, datapointCotent = cJSON_CreateObject());
	cJSON_AddItemToObject(datapointCotent, "value", value = cJSON_CreateObject());
	//mem_set(tmpData,30, 0x00);
	//sprintf(tmpData,"%d",gSystemInfo.csq);
	//cJSON_AddStringToObject(value, "SignalLevel", tmpData);
	cJSON_AddNumberToObject(value, "SignalLevel", gSystemInfo.csq);
	cJSON_AddItemToObject(value, "UserData", energy = cJSON_CreateObject());
	if(FILEFLAG_TYPE == FILEFLAG_TGP_GMTER_G55)
	{ 
	{
		mem_set(tmpData,50, 0x00);
		mem_set(str1,100, 0x00);
	    len =app_read_his_item(0x0202FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；电流数据块。
	    if (len>0)
	   {
		    cur[0] = (BCD2byte(str1[2]&0x7F)*10) +(str1[1]>>4);
		    cur[1] = (str1[1]&0x0F)*100 +BCD2byte(str1[0]);
		    if(str1[2]&0x80)
		    {
			    sprintf(tmpData+strlen(tmpData),"-%d.%03d",cur[0],cur[1]);
		    }
		    else
		    {
			    sprintf(tmpData+strlen(tmpData),"%d.%03d",cur[0],cur[1]);
		    }
		    cur[2] = (BCD2byte(str1[5]&0x7F)*10) +(str1[4]>>4);
		    cur[3] = (str1[4]&0x0F)*100 +BCD2byte(str1[3]);
		    if(str1[5]&0x80)
		    {
			    sprintf(tmpData+strlen(tmpData),"$-%d.%03d",cur[2],cur[3]);
		    }
		    else
		    {
			    sprintf(tmpData+strlen(tmpData),"$%d.%03d",cur[2],cur[3]);
		    }
		    cur[4] = (BCD2byte(str1[8]&0x7F)*10) +(str1[7]>>4);
		    cur[5] = (str1[7]&0x0F)*100 +BCD2byte(str1[6]);
		    if(str1[8]&0x80)
		    {
			    sprintf(tmpData+strlen(tmpData),"$-%d.%03d",cur[4],cur[5]);
		    }
		    else
		    {
			    sprintf(tmpData+strlen(tmpData),"$%d.%03d",cur[4],cur[5]);
		    }
	   }
	   else
	   {
		   sprintf(tmpData,"FFF.FFF$FFF.FFF$FFF.FFF");
	   }
    }	
	//mem_set(tmpData,30, 0x00);		
    //sprintf(tmpData,"%d.%03d$%d.%03d$%d.%03d",cur[0],cur[1],cur[2],cur[3],cur[4],cur[5]);
	cJSON_AddStringToObject(energy, "cur", tmpData); //"000.000$FFF.FFF$FFF.FFF");
}
else if(FILEFLAG_TYPE == FILEFLAG_SGP_GMTER_G55)
{
	{
		mem_set(tmpData,50, 0x00);
		mem_set(str1,100, 0x00);
		len =app_read_his_item(0x0202FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；电流数据块。
		if(len>0)
		{
			cur[0] = (BCD2byte(str1[2]&0x7F)*10) +(str1[1]>>4);
			cur[1] = (str1[1]&0x0F)*100 +BCD2byte(str1[0]);
			if(str1[2]&0x80)
			{
				sprintf(tmpData+strlen(tmpData),"-%d.%03d$FFF.FFF$FFF.FFF",cur[0],cur[1]);
			}
			else
			{
				sprintf(tmpData+strlen(tmpData),"%d.%03d$FFF.FFF$FFF.FFF",cur[0],cur[1]);
			}
		}
		else
		{
			sprintf(tmpData,"FFF.FFF$FFF.FFF$FFF.FFF");
		}
	
	}
	//mem_set(tmpData,30, 0x00);
	//sprintf(tmpData,"%d.%03d$FFF.FFF$FFF.FFF",cur[0],cur[1]);
	cJSON_AddStringToObject(energy, "cur", tmpData); //"000.000$FFF.FFF$FFF.FFF");
}
if(FILEFLAG_TYPE == FILEFLAG_TGP_GMTER_G55)
{
    {
		mem_set(tmpData,50, 0x00);
		mem_set(str1,100, 0x00);
	    len =app_read_his_item(0x0201FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；电压数据块。
		if(len>0)
		{
			vol[0] =(BCD2byte(str1[1])*10) +(str1[0]>>4);vol[1] =str1[0]&0x0F;
			vol[2] =(BCD2byte(str1[3])*10) +(str1[2]>>4);vol[3] =str1[2]&0x0F;
			vol[4] =(BCD2byte(str1[5])*10) +(str1[4]>>4);vol[5] =str1[4]&0x0F;
			sprintf(tmpData,"%d.%d$%d.%d$%d.%d",vol[0],vol[1],vol[2],vol[3],vol[4],vol[5]);	
		}
		else
		{
			 sprintf(tmpData,"FFF.F$FFF.F$FFF.F");
		}
    }
	cJSON_AddStringToObject(energy, "vol", tmpData); //"219.4$FFF.F$FFF.F");
}
else if(FILEFLAG_TYPE == FILEFLAG_SGP_GMTER_G55)
{
	 {	 
		 mem_set(tmpData,50, 0x00);
		 mem_set(str1,100, 0x00);
		len = app_read_his_item(0x0201FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；电压数据块。
		if(len>0)
		{
			 vol[0] =(BCD2byte(str1[1])*10) +(str1[0]>>4);vol[1] =str1[0]&0x0F;
			 sprintf(tmpData,"%d.%d$FFF.F$FFF.F",vol[0],vol[1]);
		}
		else
		{
			 sprintf(tmpData,"FFF.F$FFF.F$FFF.F");
		}
	 }
	 cJSON_AddStringToObject(energy, "vol", tmpData); //"219.4$FFF.F$FFF.F");
}
if(FILEFLAG_TYPE == FILEFLAG_TGP_GMTER_G55)
{
    {
		mem_set(tmpData,50, 0x00);
		mem_set(str1,100, 0x00);
	    len =app_read_his_item(0x0203FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；瞬时有功功率。
		if(len>0)
		{
			insP[0] = BCD2byte(str1[2]&0x7F);
			insP[1] = BCD2byte(str1[1])*100+BCD2byte(str1[0]);
			if(str1[2]&0x80)
			{
				sprintf(tmpData+strlen(tmpData),"-%d.%04d",insP[0],insP[1]);
			}
			else
			{
				sprintf(tmpData+strlen(tmpData),"%d.%04d",insP[0],insP[1]);
			}
			insP[2] = BCD2byte(str1[5]&0x7F);
			insP[3] = BCD2byte(str1[4])*100+BCD2byte(str1[3]);
			if(str1[5]&0x80)
			{
				sprintf(tmpData+strlen(tmpData),"$-%d.%04d",insP[2],insP[3]);
			}
			else
			{
				sprintf(tmpData+strlen(tmpData),"$%d.%04d",insP[2],insP[3]);
			}
			insP[4] = BCD2byte(str1[8]&0x7F);
			insP[5] = BCD2byte(str1[7])*100+BCD2byte(str1[6]);
			if(str1[8]&0x80)
			{
				sprintf(tmpData+strlen(tmpData),"$-%d.%04d",insP[4],insP[5]);
			}
			else
			{
				sprintf(tmpData+strlen(tmpData),"$%d.%04d",insP[4],insP[5]);
			}
			insP[6] = BCD2byte(str1[11]&0x7F);
			insP[7] = BCD2byte(str1[10])*100+BCD2byte(str1[9]);
			if(str1[11]&0x80)
			{
				sprintf(tmpData+strlen(tmpData),"$-%d.%04d",insP[6],insP[7]);
			}
			else
			{
				sprintf(tmpData+strlen(tmpData),"$%d.%04d",insP[6],insP[7]);
			}
		}
		else
		{
			sprintf(tmpData, "FF.FFFF$FF.FFFF$FF.FFFF$FF.FFFF");
		}
    }
	//mem_set(tmpData,30, 0x00);	
    //sprintf(tmpData, "%d.%04d$%d.%04d$%d.%04d$%d.%04d",insP[0],insP[1],insP[2],insP[3],insP[4],insP[5],insP[6],insP[7]);	
	cJSON_AddStringToObject(energy, "insP", tmpData); //"00.0000$00.0000$FF.FFFF$FF.FFFF");
}
else if(FILEFLAG_TYPE == FILEFLAG_SGP_GMTER_G55)
{
	 {
		 mem_set(tmpData,50, 0x00);
		 mem_set(str1,100, 0x00);
		 len =app_read_his_item(0x0203FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；瞬时有功功率。
		 if(len>0)
		 {
			insP[0] = BCD2byte(str1[2]&0x7F);
			insP[1] = BCD2byte(str1[1])*100+BCD2byte(str1[0]);
			if(str1[2]&0x80)
			{
				sprintf(tmpData, "-%d.%04d",insP[0],insP[1]);
			}
			else
			{
				sprintf(tmpData, "%d.%04d",insP[0],insP[1]);
			}
			insP[2] = BCD2byte(str1[5]&0x7F);
			insP[3] = BCD2byte(str1[4])*100+BCD2byte(str1[3]);
			if(str1[5]&0x80)
			{
				sprintf(tmpData+strlen(tmpData), "$-%d.%04d$FF.FFFF$FF.FFFF",insP[2],insP[3]);
			}
			else
			{
				sprintf(tmpData+strlen(tmpData), "$%d.%04d$FF.FFFF$FF.FFFF",insP[2],insP[3]);
			} 
		 }
		 else
		 {
			 sprintf(tmpData, "FF.FFFF$FF.FFFF$FF.FFFF$FF.FFFF");
		 }
		  
	 }
	 //mem_set(tmpData,30, 0x00);
	 //sprintf(tmpData, "%d.%04d$%d.%04d$FF.FFFF$FF.FFFF",insP[0],insP[1],insP[2],insP[3]);
	 cJSON_AddStringToObject(energy, "insP", tmpData); //"00.0000$00.0000$FF.FFFF$FF.FFFF");
}
if(FILEFLAG_TYPE == FILEFLAG_TGP_GMTER_G55)
{
    {
		mem_set(tmpData,50, 0x00);	
		mem_set(str1,100, 0x00);
	    len =app_read_his_item(0x0204FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；瞬时无功功率。
		if(len>0)
		{
			insQ[0] = BCD2byte(str1[2]&0x7F);
			insQ[1] = BCD2byte(str1[1])*100+BCD2byte(str1[0]);
			if(str1[2]&0x80)
			{
				sprintf(tmpData+strlen(tmpData),"-%d.%04d",insQ[0],insQ[1]);
			}
			else
			{
				sprintf(tmpData+strlen(tmpData),"%d.%04d",insQ[0],insQ[1]);
			}
			insQ[2] = BCD2byte(str1[5]&0x7F);
			insQ[3] = BCD2byte(str1[4])*100+BCD2byte(str1[3]);
			if(str1[5]&0x80)
			{
				sprintf(tmpData+strlen(tmpData),"$-%d.%04d",insQ[2],insQ[3]);
			}
			else
			{
				sprintf(tmpData+strlen(tmpData),"$%d.%04d",insQ[2],insQ[3]);
			}
			insQ[4] = BCD2byte(str1[8]&0x7F);
			insQ[5] = BCD2byte(str1[7])*100+BCD2byte(str1[6]);
			if(str1[8]&0x80)
			{
				sprintf(tmpData+strlen(tmpData),"$-%d.%04d",insQ[4],insQ[5]);
			}
			else
			{
				sprintf(tmpData+strlen(tmpData),"$%d.%04d",insQ[4],insQ[5]);
			}
			insQ[6] = BCD2byte(str1[11]&0x7F);
			insQ[7] = BCD2byte(str1[10])*100+BCD2byte(str1[9]);
			if(str1[11]&0x80)
			{
				sprintf(tmpData+strlen(tmpData),"$-%d.%04d",insQ[6],insQ[7]);
			}
			else
			{
				sprintf(tmpData+strlen(tmpData),"$%d.%04d",insQ[6],insQ[7]);
			}
		}
		else
		{
			sprintf(tmpData, "FF.FFFF$FF.FFFF$FF.FFFF$FF.FFFF");
		}
    }
	//mem_set(tmpData,30, 0x00);		
    //sprintf(tmpData, "%d.%04d$%d.%04d$%d.%04d$%d.%04d",insQ[0],insQ[1],insQ[2],insQ[3],insQ[4],insQ[5],insQ[6],insQ[7]);	
	cJSON_AddStringToObject(energy, "insQ", tmpData); //"00.0000$00.0000$FF.FFFF$FF.FFFF");
}
else if(FILEFLAG_TYPE == FILEFLAG_SGP_GMTER_G55)
{
	{
		mem_set(tmpData,50, 0x00);
		mem_set(str1,100, 0x00);
		len =app_read_his_item(0x0204FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；瞬时无功功率。
		if(len>0)
		{
			insQ[0] = BCD2byte(str1[2]&0x7F);		
			insQ[1] = BCD2byte(str1[1])*100+BCD2byte(str1[0]);
			if(str1[2]&0x80)
			{
				sprintf(tmpData+strlen(tmpData),"-%d.%04d",insQ[0],insQ[1]);
			}
			else
			{
				sprintf(tmpData+strlen(tmpData),"%d.%04d",insQ[0],insQ[1]);
			}		
			insQ[2] = BCD2byte(str1[5]&0x7F);
			insQ[3] = BCD2byte(str1[4])*100+BCD2byte(str1[3]);
			if(str1[5]&0x80)
			{
				sprintf(tmpData+strlen(tmpData), "$-%d.%04d$FF.FFFF$FF.FFFF",insQ[2],insQ[3]);
			}
			else
			{
				sprintf(tmpData+strlen(tmpData), "$%d.%04d$FF.FFFF$FF.FFFF",insQ[2],insQ[3]);
			}		
		}
		else
		{
			sprintf(tmpData, "FF.FFFF$FF.FFFF$FF.FFFF$FF.FFFF");
		}	
	}
	////mem_set(tmpData,30, 0x00);
	//sprintf(tmpData, "%d.%04d$%d.%04d$FF.FFFF$FF.FFFF",insQ[0],insQ[1],insQ[2],insQ[3]);
	cJSON_AddStringToObject(energy, "insQ", tmpData); //"00.0000$00.0000$FF.FFFF$FF.FFFF");
}
if(FILEFLAG_TYPE == FILEFLAG_TGP_GMTER_G55)
{
    {
		mem_set(tmpData,50, 0x00);
		mem_set(str1,100, 0x00);
	    len =app_read_his_item(0x0206FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；功率因数。
		if(len >0)
		{
			 pwrFactor[0] = BCD2byte((str1[1]&0x7f)>>4);
			 pwrFactor[1] = BCD2byte(str1[1]&0x0F)*100+BCD2byte(str1[0]);
			 if(str1[1]&0x80)
			 {
				 sprintf(tmpData+strlen(tmpData),"-%d.%03d",pwrFactor[0],pwrFactor[1]);
			 }
			 else
			 {
				 sprintf(tmpData+strlen(tmpData),"%d.%03d",pwrFactor[0],pwrFactor[1]);
			 }
			 pwrFactor[2] = BCD2byte((str1[3]&0x7f)>>4);
			 pwrFactor[3] = BCD2byte(str1[3]&0x0F)*100+BCD2byte(str1[2]);
			 if(str1[3]&0x80)
			 {
				 sprintf(tmpData+strlen(tmpData),"$-%d.%03d",pwrFactor[2],pwrFactor[3]);
			 }
			 else
			 {
				 sprintf(tmpData+strlen(tmpData),"$%d.%03d",pwrFactor[2],pwrFactor[3]);
			 }
			 pwrFactor[4] = BCD2byte((str1[5]&0x7F)>>4);
			 pwrFactor[5] = BCD2byte(str1[5]&0x0F)*100+BCD2byte(str1[4]);
			 if(str1[5]&0x80)
			 {
				 sprintf(tmpData+strlen(tmpData),"$-%d.%03d",pwrFactor[4],pwrFactor[5]);
			 }
			 else
			 {
				 sprintf(tmpData+strlen(tmpData),"$%d.%03d",pwrFactor[4],pwrFactor[5]);
			 }
			 pwrFactor[6] = BCD2byte((str1[7]&0x7F)>>4);
			 pwrFactor[7] = BCD2byte(str1[7]&0x0F)*100+BCD2byte(str1[6]);
			 if(str1[7]&0x80)
			 {
				 sprintf(tmpData+strlen(tmpData),"$-%d.%03d",pwrFactor[6],pwrFactor[7]);
			 }
			 else
			 {
				 sprintf(tmpData+strlen(tmpData),"$%d.%03d",pwrFactor[6],pwrFactor[7]);
			 }
		}
		else
		{
			sprintf(tmpData, "F.FFF$F.FFF$F.FFF$F.FFF");
		}
	   
    }
	//mem_set(tmpData,30, 0x00);	
    //sprintf(tmpData, "%d.%03d$%d.%03d$%d.%03d$%d.%03d", pwrFactor[0], pwrFactor[1],pwrFactor[2], pwrFactor[3], pwrFactor[4], pwrFactor[5],pwrFactor[6], pwrFactor[7]);	
	cJSON_AddStringToObject(energy, "pwrF", tmpData); //"1.000$1.000$F.FFF$F.FFF");
}
else if(FILEFLAG_TYPE == FILEFLAG_SGP_GMTER_G55)
{
	{
		mem_set(tmpData,50, 0x00);
		mem_set(str1,100, 0x00);
		len =app_read_his_item(0x0206FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；功率因数。
		if(len>0)
		{
			pwrFactor[0] = BCD2byte((str1[1]&0x7f)>>4);
			pwrFactor[1] = BCD2byte(str1[1]&0x0F)*100+BCD2byte(str1[0]);
			if(str1[1]&0x80)
			{
				sprintf(tmpData+strlen(tmpData),"-%d.%03d",pwrFactor[0],pwrFactor[1]);
			}
			else
			{
				sprintf(tmpData+strlen(tmpData),"%d.%03d",pwrFactor[0],pwrFactor[1]);
			}
			pwrFactor[2] = BCD2byte((str1[3]&0x7F)>>4);
			pwrFactor[3] = BCD2byte(str1[3]&0x0F)*100+BCD2byte(str1[2]);
			if(str1[3]&0x80)
			{
				sprintf(tmpData+strlen(tmpData), "$-%d.%03d$F.FFF$F.FFF", pwrFactor[2], pwrFactor[3]);
			}
			else
			{
				sprintf(tmpData+strlen(tmpData), "$%d.%03d$F.FFF$F.FFF", pwrFactor[2], pwrFactor[3]);
			}
		}
		else
		{
			sprintf(tmpData, "F.FFF$F.FFF$F.FFF$F.FFF");
		}
			
	}
	//mem_set(tmpData,30, 0x00);
	//sprintf(tmpData, "%d.%03d$%d.%03d$F.FFF$F.FFF", pwrFactor[0], pwrFactor[1],pwrFactor[2], pwrFactor[3]);
	cJSON_AddStringToObject(energy, "pwrF", tmpData); //"1.000$1.000$F.FFF$F.FFF");
}
	out = cJSON_PrintUnformatted(root);
	mem_cpy(buf,out,resLen = strlen(out));
	cJSON_Delete(root);
	vPortFree(out);
	return resLen;
}
#endif // __BEIJING_ONENET_EDP__
#ifdef __ZHEJIANG_ONENET_EDP__
//设备配置信息
INT16U  cjsonDeviceConfigPack(INT8U * buf)
{
	cJSON *root, *datestreamsArray, *datestreamsContent, *datapoint, *datapointCotent, *value;
	DateTime datetime;
	INT32U month_bytes=0;
	INT8U str1[200],str2[200],i =0,bufPos =0,length =0,supplyState=0;
	char *out;
	INT16U resLen =0;
	INT8U tmpData[30],idx =0;
	INT8S str_server_ip[20];
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "datastreams", datestreamsArray = cJSON_CreateArray());
	cJSON_AddItemToArray(datestreamsArray, datestreamsContent = cJSON_CreateObject());
	cJSON_AddStringToObject(datestreamsContent, "id", "event");
	cJSON_AddItemToObject(datestreamsContent, "datapoints", datapoint = cJSON_CreateArray());
	cJSON_AddItemToArray(datapoint, datapointCotent = cJSON_CreateObject());
	cJSON_AddItemToObject(datapointCotent, "value", value = cJSON_CreateObject());
	mem_set(tmpData,30, 0x00);
	sprintf(tmpData,"%s",gSystemInfo.managementNum);
	cJSON_AddStringToObject(value, "meterNum", tmpData); ///"1111111111111111111111");
	mem_set(tmpData,30, 0x00);
	sprintf(tmpData, "%.20s", gSystemInfo.myccid_info);
	cJSON_AddStringToObject(value, "iccid",tmpData); // "898602B8231700769443");
	cJSON_AddNumberToObject(value, "interval", gSystemInfo.edp_report_interval);
	mem_set(tmpData,30, 0x00);
	sprintf(tmpData, "%.15s", gSystemInfo.imei_info);
	cJSON_AddStringToObject(value, "imei", tmpData); //"861477030064636");
	mem_set(tmpData,30, 0x00);
	sprintf(tmpData, "%.15s", gSystemInfo.imsi_info);
	cJSON_AddStringToObject(value, "imsi", tmpData); //"861477030064636");
	cJSON_AddNumberToObject(value, "SignalLevel", gSystemInfo.csq);
	mem_set(tmpData,30, 0x00);
	for(idx =0;idx<6;idx++)  //这里要注意表地址的反的，需要显示的时候翻转过来。
	{
		tmpData[2*idx] =(gSystemInfo.meter_no[5-idx]>>4)+ '0';
		tmpData[2*idx+1] =(gSystemInfo.meter_no[5-idx]&0x0f )+'0';
	}
	//sprintf(tmpData, "%x", gSystemInfo.meter_no);
	cJSON_AddStringToObject(value, "meterAddr", tmpData); 
	if(FILEFLAG_TYPE == FILEFLAG_TGP_GMTER_G55)
	{
		cJSON_AddNumberToObject(value, "meterType", 3);   //301代表三相，101代表单相。
	}
	else if(FILEFLAG_TYPE == FILEFLAG_SGP_GMTER_G55)
	{
		cJSON_AddNumberToObject(value, "meterType", 1);   //301代表三相，101代表单相。
	}
	////ip2str(gSystemInfo.ppp_ip, str_server_ip);
	////mem_set(tmpData,30, 0x00);
	////sprintf(tmpData, "%s", str_server_ip);
	////cJSON_AddStringToObject(value, "ip", tmpData); //"10.146.238.6");
	//cJSON_AddNumberToObject(value, "moduleType", 1); //通信模块的类型（4G设置为1，NB设置为2，2G设置为3）
	//mem_set(tmpData,30, 0x00);
	//sprintf(tmpData, "%s", edp_version);
	//cJSON_AddStringToObject(value, "version", tmpData);
	cJSON_AddStringToObject(value, "manuNum", "TPOS");
	os_get_datetime(&datetime);
	//	str = "\"17-08-02 10:52:01\",";
	sprintf(tmpData,"%d-%d%d-%d%d %d%d:%d%d:%d%d",datetime.year,datetime.month/10,(datetime.month%10),datetime.day/10,(datetime.day%10),datetime.hour/10,(datetime.hour%10),datetime.minute/10,(datetime.minute%10),datetime.second/10,(datetime.second%10));
	cJSON_AddStringToObject(value, "time", tmpData);	
	//
	//
	mem_set(tmpData,30, 0x00);
	sprintf(tmpData, "%s", edp_version);
	cJSON_AddStringToObject(value, "softVersion", tmpData);  //软件版本
	cJSON_AddStringToObject(value, "hardVersion", "1.01");  //硬件版本
	cJSON_AddStringToObject(value, "manuNum", "TOPSCOMM");  //硬件版本
	mem_cpy(&month_bytes,g_app_run_data.gprs_bytes.day_bytes,4);
	cJSON_AddNumberToObject(value,"flow",month_bytes/1024);
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x04000503,str1,str2,255,0,2000);  //A相过流总次数 抄表的结果应该放在str1里。
	if(length)
	{
		supplyState=(str1[0]>>1)&0x03;
	}	
	cJSON_AddNumberToObject(value,"supplyState",supplyState);	//供电方式
	out = cJSON_PrintUnformatted(root);
	//printf("%s", out);
	cJSON_Delete(root);
	mem_cpy(buf,out,resLen = strlen(out));
	vPortFree(out);
	//	cJSON_Delete(out);
	return resLen;

}
//组过流告警字符串
void makeOverFlow(INT8U *buf,INT8U len)
{
	INT8U str1[200],str2[200],i =0,bufPos =0,length =0;
	INT32U AphaseOverFlowCnt =0,BphaseOverFlowCnt =0,CphaseOverFlowCnt =0;
	INT32U AphaseOverFlowTime =0,BphaseOverFlowTime =0,CphaseOverFlowTime =0;
	INT8U AphaseOverFlowTimeOccur[6] = {0},BphaseOverFlowTimeOccur[6] = {0},CphaseOverFlowTimeOccur[6] = {0};
	////////////////////////////////////读A相数据//////////////////////////////////////////////////////////////
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x19010001,str1,str2,255,0,2000);  //A相过流总次数 抄表的结果应该放在str1里。
	if(length)
	{
		AphaseOverFlowCnt = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
		gSystemInfo.edp_over_flow_A = AphaseOverFlowCnt;
		fwrite_ertu_params(EEADDR_OVERFLOW_WARNING_A,(INT8U*)(&gSystemInfo.edp_over_flow_A),4);
	}
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x19010002,str1,str2,255,0,2000);  //A相过流总时间 抄表的结果应该放在str1里。
	if(length)
	AphaseOverFlowTime = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x19010002,str1,str2,255,0,2000);  //A相发生过流时刻 抄表的结果应该放在str1里。
	if(length)
	{
		for(i =0;i<6;i++)
		{
			AphaseOverFlowTimeOccur[i] =str1[i];
		}
	}
	////////////////////////////////////读B相数据//////////////////////////////////////////////////////////////
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x19020001,str1,str2,255,0,2000);  //B相过流总次数 抄表的结果应该放在str1里。
	if(length)
	{
		BphaseOverFlowCnt = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
		gSystemInfo.edp_over_flow_B = BphaseOverFlowCnt;
		fwrite_ertu_params(EEADDR_OVERFLOW_WARNING_B,(INT8U*)(&gSystemInfo.edp_over_flow_B),4);
	}
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length = app_read_his_item(0x19020002,str1,str2,255,0,2000);  //B相过流总时间 抄表的结果应该放在str1里。
	if(length)
	BphaseOverFlowTime = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length = app_read_his_item(0x19020002,str1,str2,255,0,2000);  //B相发生过流时刻 抄表的结果应该放在str1里。
	if(length)
	{
		for(i =0;i<6;i++)
		{
			BphaseOverFlowTimeOccur[i] =str1[i];
		}
	}

	////////////////////////////////////读C相数据//////////////////////////////////////////////////////////////
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length = app_read_his_item(0x19030001,str1,str2,255,0,2000);  //C相过流总次数 抄表的结果应该放在str1里。
	if(length)
	{
		CphaseOverFlowCnt = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
		gSystemInfo.edp_over_flow_C =CphaseOverFlowCnt;
		fwrite_ertu_params(EEADDR_OVERFLOW_WARNING_C,(INT8U*)(&gSystemInfo.edp_over_flow_C),4);
	}
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x19030002,str1,str2,255,0,2000);  //C相过流总时间 抄表的结果应该放在str1里。
	if(length)
	CphaseOverFlowTime = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x19030002,str1,str2,255,0,2000);  //C相发生过流时刻 抄表的结果应该放在str1里。
	if(length)
	{
		for(i =0;i<6;i++)
		{
			CphaseOverFlowTimeOccur[i] =str1[i];
		}
	}
	//	*(buf+bufPos++) ='0';*(buf+bufPos++) ='$';*(buf+bufPos++) ='A';*(buf+bufPos++) ='&';sprintf(buf,"%d",AphaseOverFlowCnt);
	sprintf(buf+strlen(buf),"0$",NULL);
	sprintf(buf+strlen(buf),"A&%d&%d",AphaseOverFlowCnt,AphaseOverFlowTime);
	if(AphaseOverFlowTimeOccur[5]!=0x00)
	sprintf(buf+strlen(buf),"&%d%d-%d%d-%d%d %d%d:%d%d:%d%d",AphaseOverFlowTimeOccur[5]/16,AphaseOverFlowTimeOccur[5]%16,AphaseOverFlowTimeOccur[4]/16,(AphaseOverFlowTimeOccur[4]%16),AphaseOverFlowTimeOccur[3]/16,(AphaseOverFlowTimeOccur[3]%16),AphaseOverFlowTimeOccur[2]/16,(AphaseOverFlowTimeOccur[2]%16),AphaseOverFlowTimeOccur[1]/16,(AphaseOverFlowTimeOccur[1]%16),AphaseOverFlowTimeOccur[0]/16,(AphaseOverFlowTimeOccur[0]%16));
	sprintf(buf+strlen(buf),";",NULL);
	sprintf(buf+strlen(buf),"B&%d&%d",BphaseOverFlowCnt,BphaseOverFlowTime);
	if(BphaseOverFlowTimeOccur[5]!=0x00)
	sprintf(buf+strlen(buf),"&%d%d-%d%d-%d%d %d%d:%d%d:%d%d",BphaseOverFlowTimeOccur[5]/16,BphaseOverFlowTimeOccur[5]%16,BphaseOverFlowTimeOccur[4]/16,(BphaseOverFlowTimeOccur[4]%16),BphaseOverFlowTimeOccur[3]/16,(BphaseOverFlowTimeOccur[3]%16),BphaseOverFlowTimeOccur[2]/16,(BphaseOverFlowTimeOccur[2]%16),BphaseOverFlowTimeOccur[1]/16,(BphaseOverFlowTimeOccur[1]%16),BphaseOverFlowTimeOccur[0]/16,(BphaseOverFlowTimeOccur[0]%16));
	sprintf(buf+strlen(buf),";",NULL);
	sprintf(buf+strlen(buf),"C&%d&%d",CphaseOverFlowCnt,CphaseOverFlowTime);
	if(CphaseOverFlowTimeOccur[5]!=0x00)
	sprintf(buf+strlen(buf),"&%d%d-%d%d-%d%d %d%d:%d%d:%d%d",CphaseOverFlowTimeOccur[5]/16,CphaseOverFlowTimeOccur[5]%16,CphaseOverFlowTimeOccur[4]/16,(CphaseOverFlowTimeOccur[4]%16),CphaseOverFlowTimeOccur[3]/16,(CphaseOverFlowTimeOccur[3]%16),CphaseOverFlowTimeOccur[2]/16,(CphaseOverFlowTimeOccur[2]%16),CphaseOverFlowTimeOccur[1]/16,(CphaseOverFlowTimeOccur[1]%16),CphaseOverFlowTimeOccur[0]/16,(CphaseOverFlowTimeOccur[0]%16));
	len =strlen(buf);
	
}
//组开表盖告警字符串
void makeOpenMeter(INT8U *buf,INT8U len)
{
	INT8U str1[200],str2[200],i =0,bufPos =0,length =0;
	INT16U openCnt =0;
	INT8U openTimeOccur[6] = {0};
	////////////////////////////////////读A相数据//////////////////////////////////////////////////////////////
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x03300D00,str1,str2,255,0,2000);  //开盖总次数 抄表的结果应该放在str1里。
	if(length)
	{
		openCnt = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
		gSystemInfo.edp_open_meter = openCnt;
		fwrite_ertu_params(EEADDR_OPENMETER_WARNING,(INT8U*)(&gSystemInfo.edp_open_meter),4);
	}
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x03300D01,str1,str2,255,0,2000);  //上一次开盖时间 抄表的结果应该放在str1里。
	if(length)
	{
		for(i =0;i<6;i++)
		{
			openTimeOccur[i] =str1[i];
		}
	}
	sprintf(buf+strlen(buf),"1$",NULL);
	sprintf(buf+strlen(buf),"%d&",openCnt);
	sprintf(buf+strlen(buf),"&%d%d-%d%d-%d%d %d%d:%d%d:%d%d",openTimeOccur[5]/16,openTimeOccur[5]%16,openTimeOccur[4]/16,(openTimeOccur[4]%16),openTimeOccur[3]/16,(openTimeOccur[3]%16),openTimeOccur[2]/16,(openTimeOccur[2]%16),openTimeOccur[1]/16,(openTimeOccur[1]%16),openTimeOccur[0]/16,(openTimeOccur[0]%16));
	len =strlen(buf);
}
//组电表清零告警字符串
void makeCleanMeter(INT8U *buf,INT8U len)
{
	INT8U str1[200],str2[200],i =0,bufPos =0,length =0;
	INT16U cleanCnt =0;
	INT8U cleanTimeOccur[6] = {0};
	////////////////////////////////////读A相数据//////////////////////////////////////////////////////////////
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x03300100,str1,str2,255,0,2000);  //开盖总次数 抄表的结果应该放在str1里。
	if(length)
	{
		cleanCnt = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
		gSystemInfo.edp_clean_meter =cleanCnt;
		fwrite_ertu_params(EEADDR_CLEANNMETER_WARNING,(INT8U*)(&gSystemInfo.edp_clean_meter),4);
	}
	mem_set(str1,200,0xFF);
	mem_set(str2,200,0xFF);
	length =app_read_his_item(0x03300101,str1,str2,255,0,2000);  //上一次开盖时间 抄表的结果应该放在str1里。
	if(length)
	{
		for(i =0;i<6;i++)
		{
			cleanTimeOccur[i] =str1[i];
		}
	}
	sprintf(buf+strlen(buf),"2$",NULL);
	sprintf(buf+strlen(buf),"%d&",cleanCnt);
	sprintf(buf+strlen(buf),"&%d%d-%d%d-%d%d %d%d:%d%d:%d%d",cleanTimeOccur[5]/16,cleanTimeOccur[5]%16,cleanTimeOccur[4]/16,(cleanTimeOccur[4]%16),cleanTimeOccur[3]/16,(cleanTimeOccur[3]%16),cleanTimeOccur[2]/16,(cleanTimeOccur[2]%16),cleanTimeOccur[1]/16,(cleanTimeOccur[1]%16),cleanTimeOccur[0]/16,(cleanTimeOccur[0]%16));
	len =strlen(buf);
}
//事件告警信息
INT16U cjsonEventWarningPack(INT8U *buf,INT8U flag,INT8U data)
{
	cJSON *root, *datestreamsArray, *datestreamsContent, *datapoint, *datapointCotent, *value;
	char *out;
	INT16U resLen =0;
	INT8U buftmp[200] ={0},len =0;
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "datastreams", datestreamsArray = cJSON_CreateArray());
	cJSON_AddItemToArray(datestreamsArray, datestreamsContent = cJSON_CreateObject());
	cJSON_AddStringToObject(datestreamsContent, "id", "event");
	cJSON_AddItemToObject(datestreamsContent, "datapoints", datapoint = cJSON_CreateArray());
	cJSON_AddItemToArray(datapoint, datapointCotent = cJSON_CreateObject());
	cJSON_AddItemToObject(datapointCotent, "value", value = cJSON_CreateObject());
	switch(flag)
	{
		case 0:	    //时钟电池欠压事件
			cJSON_AddNumberToObject(value, "DCQY", data);
			break;			
		case 1:  //市电停上电事件
			cJSON_AddNumberToObject(value, "SDTSD", data);
			break;
		case 2:  //停上电事件（辅助电源）
			cJSON_AddNumberToObject(value, "FZDYTSD", data);
			break;
		case 3:  //有功功率方向  ,这里指的是A相
			cJSON_AddNumberToObject(value, "YGGLFX", data);
			break;
		case 4:  //无功功率方向	 这次暂时不用
			cJSON_AddNumberToObject(value, "WGGLFX", data);
			break;	
		case 5:  //A相过流
			cJSON_AddNumberToObject(value, "AXGL", data);
			break;
		case 6:  //A相过流
			cJSON_AddNumberToObject(value, "BXGL", data);
			break;
		case 7:  //A相过流
			cJSON_AddNumberToObject(value, "CXGL", data);
			break;	
		case 8:  //A相过流
			cJSON_AddNumberToObject(value, "AXDX", data);
			break;
		case 9:  //A相过流
			cJSON_AddNumberToObject(value, "BXDX", data);
			break;
		case 10:  //A相过流
			cJSON_AddNumberToObject(value, "CXDX", data);
			break;
		default:
			break;
	}
	//	cJSON_AddStringToObject(value, "topwarn", "2$1&17-04-01 13:05:47;B&0&0;C&0&0;");
	//cJSON_AddStringToObject(value, "topwarn", buftmp);


	out = cJSON_PrintUnformatted(root);
	mem_cpy(buf,out,resLen = strlen(out));
	cJSON_Delete(root);
	vPortFree(out);
	return resLen;
}
//固件升级信息
INT16U cjsonFirmwareUpdatePack(INT8U *buf,INT8U flag)
{
	cJSON *root, *datestreamsArray, *datestreamsContent, *datapoint, *datapointCotent, *value;
	char *out;
	INT16U resLen = 0;
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "datastreams", datestreamsArray = cJSON_CreateArray());
	cJSON_AddItemToArray(datestreamsArray, datestreamsContent = cJSON_CreateObject());
	cJSON_AddStringToObject(datestreamsContent, "id", "event");
	cJSON_AddItemToObject(datestreamsContent, "datapoints", datapoint = cJSON_CreateArray());
	cJSON_AddItemToArray(datapoint, datapointCotent = cJSON_CreateObject());
	cJSON_AddItemToObject(datapointCotent, "value", value = cJSON_CreateObject());
	cJSON_AddNumberToObject(value, "fota", flag);

	out = cJSON_PrintUnformatted(root);
	mem_cpy(buf,out,resLen = strlen(out));
	cJSON_Delete(root);
	vPortFree(out);
	return resLen;
}
//组报文透传抄表数据。
INT16U cjsonReadMeter(INT8U *buf)
{
	cJSON *root, *datestreamsArray, *datestreamsContent, *datapoint, *datapointCotent, *value;
	char *out;
	INT16U resLen =0;
	INT8U buftmp[10] ={0},len =0,i=0;
	INT8U str1[200]={0},str2[400]={0};
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "datastreams", datestreamsArray = cJSON_CreateArray());
	cJSON_AddItemToArray(datestreamsArray, datestreamsContent = cJSON_CreateObject());
	cJSON_AddStringToObject(datestreamsContent, "id", "event");
	cJSON_AddItemToObject(datestreamsContent, "datapoints", datapoint = cJSON_CreateArray());
	cJSON_AddItemToArray(datapoint, datapointCotent = cJSON_CreateObject());
	cJSON_AddItemToObject(datapointCotent, "value", value = cJSON_CreateObject());
	//cJSON_AddStringToObject(value, "topwarn", "2$1&17-04-01 13:05:47;B&0&0;C&0&0;");
	sprintf(buftmp,"%08x",cmd645);
	cJSON_AddStringToObject(value, "cmd", buftmp);
	
	len = app_read_his_item(cmd645,str1,str2,255,0,2000);//str1中存在着我们要读取的值；组合有功。
	mem_set(str2,400,0x00);
	for(i =0;i<len;i++)
	{
		if(((str1[i]>>4)&0x0F)>=0 &&((str1[i]>>4)&0x0F)<=9)
		str2[2*i] = ((str1[i]>>4)&0x0F) +'0';
		else if(((str1[i]>>4)&0x0F)>=10 &&((str1[i]>>4)&0x0F)<=15)
		str2[2*i] = ((str1[i]>>4)&0x0F) +'A';
		
		if(((str1[i])&0x0F)>=0 &&((str1[i])&0x0F)<=9)
		str2[2*i+1] = ((str1[i])&0x0F) +'0';
		else if(((str1[i])&0x0F)>=10 &&((str1[i])&0x0F)<=15)
		str2[2*i+1] = ((str1[i])&0x0F) +'A';
	}
	cJSON_AddStringToObject(value, "echo", str2);
	
	out = cJSON_PrintUnformatted(root);
	mem_cpy(buf,out,resLen = strlen(out));
	cJSON_Delete(root);
	vPortFree(out);
	return resLen;
}
//实时电量信息
INT16U cjsonEnergyInfoPack(char dayhold,INT8U *buf)
{
	cJSON *root, *datestreamsArray, *datestreamsContent, *datapoint, *datapointCotent, *value, *energy;
	char *out;
	INT32U month_bytes=0;
	INT8U tmpData[30] ={0},length=0;
	INT8U str1[100],str2[100];
	INT32U DQZXYG[2],DQZXWG[2],DQFXYG[2],DQFXWG[2],RDJZXYG[2],RDJFXYG[2],RDJZXWG[2],RDJFXWG[2],YDJZXYG[2];
	tagDatetime  datetime;
	INT16U resLen =0;
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "datastreams", datestreamsArray = cJSON_CreateArray());
	cJSON_AddItemToArray(datestreamsArray, datestreamsContent = cJSON_CreateObject());
	cJSON_AddStringToObject(datestreamsContent, "id", "value");
	cJSON_AddItemToObject(datestreamsContent, "datapoints", datapoint = cJSON_CreateArray());
	cJSON_AddItemToArray(datapoint, datapointCotent = cJSON_CreateObject());
	cJSON_AddItemToObject(datapointCotent, "value", value = cJSON_CreateObject());
	//mem_set(tmpData,30, 0x00);
	//sprintf(tmpData,"%d",gSystemInfo.csq);
	//cJSON_AddStringToObject(value, "SignalLevel", tmpData);
	cJSON_AddNumberToObject(value, "SignalLevel", gSystemInfo.csq);
	cJSON_AddItemToObject(value, "UserData", energy = cJSON_CreateObject());
	mem_set(tmpData,30, 0x00);
	sprintf(tmpData,"%s",gSystemInfo.managementNum);
	cJSON_AddStringToObject(energy, "meterNum", tmpData); ///"1111111111111111111111");
	
	os_get_datetime(&datetime);
	//	str = "\"17-08-02 10:52:01\",";
	sprintf(tmpData,"%d-%d%d-%d%d %d%d:%d%d:%d%d",datetime.year,datetime.month/10,(datetime.month%10),datetime.day/10,(datetime.day%10),datetime.hour/10,(datetime.hour%10),datetime.minute/10,(datetime.minute%10),datetime.second/10,(datetime.second%10));
	cJSON_AddStringToObject(energy, "time", tmpData);
	{
		if(dayhold ==0)
		{
			length =app_read_his_item(0x00010000,str1,str2,255,0,2000);//str1中存在着我们要读取的值；当前正向有功电能示值
			if(length>0)
			{
				mem_set(tmpData,30, 0x00);
				DQZXYG[0] = (BCD2byte(str1[3])*10000) +(BCD2byte(str1[2])*100)+(BCD2byte(str1[1]));
				DQZXYG[1] = BCD2byte(str1[0]);
				sprintf(tmpData,"%d.%02d",DQZXYG[0],DQZXYG[1]);
				cJSON_AddStringToObject(energy, "DQZXYG", tmpData);			
			}
			else
			{
				cJSON_AddStringToObject(energy, "DQZXYG", "FF.FF");
			}
			
			length =app_read_his_item(0x00030000,str1,str2,255,0,2000);//str1中存在着我们要读取的值；当前正向无功电能示值
			if(length>0)
			{
				mem_set(tmpData,30, 0x00);
				DQZXWG[0] = (BCD2byte(str1[3])*10000) +(BCD2byte(str1[2])*100)+(BCD2byte(str1[1]));
				DQZXWG[1] = BCD2byte(str1[0]);
				sprintf(tmpData,"%d.%02d",DQZXWG[0],DQZXWG[1]);
				cJSON_AddStringToObject(energy, "DQZXWG", tmpData);
			}
			else
			{
				cJSON_AddStringToObject(energy, "DQZXWG", "FF.FF");
			}			
			length =app_read_his_item(0x00020000,str1,str2,255,0,2000);//str1中存在着我们要读取的值；当前反向有功电能示值
			if(length>0)
			{
				mem_set(tmpData,30, 0x00);
				DQFXYG[0] = (BCD2byte(str1[3])*10000) +(BCD2byte(str1[2])*100)+(BCD2byte(str1[1]));
				DQFXYG[1] = BCD2byte(str1[0]);
				sprintf(tmpData,"%d.%02d",DQFXYG[0],DQFXYG[1]);
				cJSON_AddStringToObject(energy, "DQFXYG", tmpData);
			}
			else
			{
				cJSON_AddStringToObject(energy, "DQFXYG", "FF.FF");
			}			
			length =app_read_his_item(0x00040000,str1,str2,255,0,2000);//str1中存在着我们要读取的值；当前反向无功电能示值
			if(length>0)
			{
				mem_set(tmpData,30, 0x00);
				DQFXWG[0] = (BCD2byte(str1[3])*10000) +(BCD2byte(str1[2])*100)+(BCD2byte(str1[1]));
				DQFXWG[1] = BCD2byte(str1[0]);
				sprintf(tmpData,"%d.%02d",DQFXWG[0],DQFXWG[1]);
				cJSON_AddStringToObject(energy, "DQFXWG", tmpData);
			}
			else
			{
				cJSON_AddStringToObject(energy, "DQFXWG", "FF.FF");
			}										
		}
		else if(dayhold==1)
		{	
			length =app_read_his_item(0x05060101,str1,str2,255,0,2000);//str1中存在着我们要读取的值；(上1 次)日冻结正向有功总电能
			if(length>0)
			{
				mem_set(tmpData,30, 0x00);
				RDJZXYG[0] = (BCD2byte(str1[3])*10000) +(BCD2byte(str1[2])*100)+(BCD2byte(str1[1]));
				RDJZXYG[1] = BCD2byte(str1[0]);
				sprintf(tmpData,"%d.%02d",RDJZXYG[0],RDJZXYG[1]);
				cJSON_AddStringToObject(energy, "RDJZXYG", tmpData);
			}
			else
			{
				cJSON_AddStringToObject(energy, "RDJZXYG", "FF.FF");
			}			

			length =app_read_his_item(0x05060201,str1,str2,255,0,2000);//str1中存在着我们要读取的值；(上1 次)日冻结反向有功总电能
			if(length>0)
			{
				mem_set(tmpData,30, 0x00);
				RDJFXYG[0] = (BCD2byte(str1[3])*10000) +(BCD2byte(str1[2])*100)+(BCD2byte(str1[1]));
				RDJFXYG[1] = BCD2byte(str1[0]);
				sprintf(tmpData,"%d.%02d",RDJFXYG[0],RDJFXYG[1]);
				cJSON_AddStringToObject(energy, "RDJFXYG", tmpData);
			}
			else
			{
				cJSON_AddStringToObject(energy, "RDJFXYG", "FF.FF");
			}			
			length =app_read_his_item(0x05060301,str1,str2,255,0,2000);//str1中存在着我们要读取的值；(上1 次)日冻结正向无功总电能
			if(length>0)
			{
				mem_set(tmpData,30, 0x00);
				RDJZXWG[0] = (BCD2byte(str1[3])*10000) +(BCD2byte(str1[2])*100)+(BCD2byte(str1[1]));
				RDJZXWG[1] = BCD2byte(str1[0]);
				sprintf(tmpData,"%d.%02d",RDJZXWG[0],RDJZXWG[1]);
				cJSON_AddStringToObject(energy, "RDJZXWG", tmpData);
			}
			else
			{
				cJSON_AddStringToObject(energy, "RDJZXWG", "FF.FF");
			}
			length =app_read_his_item(0x05060401,str1,str2,255,0,2000);//str1中存在着我们要读取的值；(上1 次)日冻结反向无功总电能
			if(length>0)
			{
				mem_set(tmpData,30, 0x00);
				RDJFXWG[0] = (BCD2byte(str1[3])*10000) +(BCD2byte(str1[2])*100)+(BCD2byte(str1[1]));
				RDJFXWG[1] = BCD2byte(str1[0]);
				sprintf(tmpData,"%d.%02d",RDJFXWG[0],RDJFXWG[1]);
				cJSON_AddStringToObject(energy, "RDJFXWG", tmpData);
			}
			else
			{
				cJSON_AddStringToObject(energy, "RDJFXWG", "FF.FF");
			}			
							
		}

	}
	mem_cpy(&month_bytes,g_app_run_data.gprs_bytes.day_bytes,4);
	cJSON_AddNumberToObject(energy,"flow",month_bytes/1024);
	cJSON_AddNumberToObject(energy, "frozen", dayhold);
	

	out = cJSON_PrintUnformatted(root);
	
	mem_cpy(buf,out,resLen = strlen(out));
	cJSON_Delete(root);
	vPortFree(out);
	return resLen;
}
//月冻结数据
INT16U cjsonEnergyBlockPackMonth(INT8U *buf)
{
	cJSON *root, *datestreamsArray, *datestreamsContent, *datapoint, *datapointCotent, *value, *energy;
	char *out;
	INT32U month_bytes=0;
	INT8U tmpData[30] ={0},length=0;
	INT8U str1[100],str2[100];
	INT32U YDJZXYG[2];
	tagDatetime  datetime;
	INT16U resLen =0;
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "datastreams", datestreamsArray = cJSON_CreateArray());
	cJSON_AddItemToArray(datestreamsArray, datestreamsContent = cJSON_CreateObject());
	cJSON_AddStringToObject(datestreamsContent, "id", "value");
	cJSON_AddItemToObject(datestreamsContent, "datapoints", datapoint = cJSON_CreateArray());
	cJSON_AddItemToArray(datapoint, datapointCotent = cJSON_CreateObject());
	cJSON_AddItemToObject(datapointCotent, "value", value = cJSON_CreateObject());
 
	cJSON_AddNumberToObject(value, "SignalLevel", gSystemInfo.csq);
	cJSON_AddItemToObject(value, "UserData", energy = cJSON_CreateObject());
	mem_set(tmpData,30, 0x00);
	sprintf(tmpData,"%s",gSystemInfo.managementNum);
	cJSON_AddStringToObject(energy, "meterNum", tmpData); ///"1111111111111111111111");
	
	os_get_datetime(&datetime);
	//	str = "\"17-08-02 10:52:01\",";
	sprintf(tmpData,"%d-%d%d-%d%d %d%d:%d%d:%d%d",datetime.year,datetime.month/10,(datetime.month%10),datetime.day/10,(datetime.day%10),datetime.hour/10,(datetime.hour%10),datetime.minute/10,(datetime.minute%10),datetime.second/10,(datetime.second%10));
	cJSON_AddStringToObject(energy, "time", tmpData);
	length =app_read_his_item(0x00010001,str1,str2,255,0,2000);//str1中存在着我们要读取的值；月冻结正向有功总电能示值
	if(length>0)
	{
		mem_set(tmpData,30, 0x00);
		YDJZXYG[0] = (BCD2byte(str1[3])*10000) +(BCD2byte(str1[2])*100)+(BCD2byte(str1[1]));
		YDJZXYG[1] = BCD2byte(str1[0]);
		sprintf(tmpData,"%d.%02d",YDJZXYG[0],YDJZXYG[1]);
		cJSON_AddStringToObject(energy, "YDJZXYG", tmpData);
	}
	else
	{
		cJSON_AddStringToObject(energy, "YDJZXYG", "FF.FF");
	}
	
	out = cJSON_PrintUnformatted(root);
	mem_cpy(buf,out,resLen = strlen(out));
	cJSON_Delete(root);
	vPortFree(out);
	return resLen;	
		
}
//电能量信息数据块
INT16U cjsonEnergyBlockPack(INT8U *buf)
{
	cJSON *root, *datestreamsArray, *datestreamsContent, *datapoint, *datapointCotent, *value, *energy;
	char *out;
	INT16U resLen =0,len;
	INT8U str1[100],str2[100];
	INT16U vol[6];  //转换用于电压
	INT16S cur[6];  //转换用于电流
	INT16S zero_cur[2];  //转换用于零线电流
	INT16S insP[8];  //瞬时有功  总+ABC
	INT16S insQ[8];  //瞬时无功   总+ABC
	INT16S pwrFactor[8];  //功率因数。 总+ABC
	INT8U tmpData[50] ={0};
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "datastreams", datestreamsArray = cJSON_CreateArray());
	cJSON_AddItemToArray(datestreamsArray, datestreamsContent = cJSON_CreateObject());
	cJSON_AddStringToObject(datestreamsContent, "id", "value");
	cJSON_AddItemToObject(datestreamsContent, "datapoints", datapoint = cJSON_CreateArray());
	cJSON_AddItemToArray(datapoint, datapointCotent = cJSON_CreateObject());
	cJSON_AddItemToObject(datapointCotent, "value", value = cJSON_CreateObject());
	//mem_set(tmpData,30, 0x00);
	//sprintf(tmpData,"%d",gSystemInfo.csq);
	//cJSON_AddStringToObject(value, "SignalLevel", tmpData);
	cJSON_AddNumberToObject(value, "SignalLevel", gSystemInfo.csq);
	cJSON_AddItemToObject(value, "UserData", energy = cJSON_CreateObject());
	if(FILEFLAG_TYPE == FILEFLAG_TGP_GMTER_G55)
	{
		{
			mem_set(tmpData,50, 0x00);
			mem_set(str1,100, 0x00);
			len =app_read_his_item(0x0202FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；电流数据块。
			if (len>0)
			{
				cur[0] = (BCD2byte(str1[2]&0x7F)*10) +(str1[1]>>4);
				cur[1] = (str1[1]&0x0F)*100 +BCD2byte(str1[0]);
				if(str1[2]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"-%d.%03d",cur[0],cur[1]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"%d.%03d",cur[0],cur[1]);
				}
				cur[2] = (BCD2byte(str1[5]&0x7F)*10) +(str1[4]>>4);
				cur[3] = (str1[4]&0x0F)*100 +BCD2byte(str1[3]);
				if(str1[5]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"$-%d.%03d",cur[2],cur[3]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"$%d.%03d",cur[2],cur[3]);
				}
				cur[4] = (BCD2byte(str1[8]&0x7F)*10) +(str1[7]>>4);
				cur[5] = (str1[7]&0x0F)*100 +BCD2byte(str1[6]);
				if(str1[8]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"$-%d.%03d",cur[4],cur[5]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"$%d.%03d",cur[4],cur[5]);
				}
			}
			else
			{
				sprintf(tmpData,"FFF.FFF$FFF.FFF$FFF.FFF");
			}
		}
		//mem_set(tmpData,30, 0x00);
		//sprintf(tmpData,"%d.%03d$%d.%03d$%d.%03d",cur[0],cur[1],cur[2],cur[3],cur[4],cur[5]);
		cJSON_AddStringToObject(energy, "cur", tmpData); //"000.000$FFF.FFF$FFF.FFF");
	}
	else if(FILEFLAG_TYPE == FILEFLAG_SGP_GMTER_G55)
	{
		{
			mem_set(tmpData,50, 0x00);
			mem_set(str1,100, 0x00);
			len =app_read_his_item(0x0202FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；电流数据块。
			if(len>0)
			{
				cur[0] = (BCD2byte(str1[2]&0x7F)*10) +(str1[1]>>4);
				cur[1] = (str1[1]&0x0F)*100 +BCD2byte(str1[0]);
				if(str1[2]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"-%d.%03d$FFF.FFF$FFF.FFF",cur[0],cur[1]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"%d.%03d$FFF.FFF$FFF.FFF",cur[0],cur[1]);
				}
			}
			else
			{
				sprintf(tmpData,"FFF.FFF$FFF.FFF$FFF.FFF");
			}
			
		}
		//mem_set(tmpData,30, 0x00);
		//sprintf(tmpData,"%d.%03d$FFF.FFF$FFF.FFF",cur[0],cur[1]);
		cJSON_AddStringToObject(energy, "cur", tmpData); //"000.000$FFF.FFF$FFF.FFF");
	}
	if(FILEFLAG_TYPE == FILEFLAG_TGP_GMTER_G55)
	{
		{
			mem_set(tmpData,50, 0x00);
			mem_set(str1,100, 0x00);
			len =app_read_his_item(0x0201FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；电压数据块。
			if(len>0)
			{
				vol[0] =(BCD2byte(str1[1])*10) +(str1[0]>>4);vol[1] =str1[0]&0x0F;
				vol[2] =(BCD2byte(str1[3])*10) +(str1[2]>>4);vol[3] =str1[2]&0x0F;
				vol[4] =(BCD2byte(str1[5])*10) +(str1[4]>>4);vol[5] =str1[4]&0x0F;
				sprintf(tmpData,"%d.%d$%d.%d$%d.%d",vol[0],vol[1],vol[2],vol[3],vol[4],vol[5]);
			}
			else
			{
				sprintf(tmpData,"FFF.F$FFF.F$FFF.F");
			}
		}
		cJSON_AddStringToObject(energy, "vol", tmpData); //"219.4$FFF.F$FFF.F");
	}
	else if(FILEFLAG_TYPE == FILEFLAG_SGP_GMTER_G55)
	{
		{
			mem_set(tmpData,50, 0x00);
			mem_set(str1,100, 0x00);
			len = app_read_his_item(0x0201FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；电压数据块。
			if(len>0)
			{
				vol[0] =(BCD2byte(str1[1])*10) +(str1[0]>>4);vol[1] =str1[0]&0x0F;
				sprintf(tmpData,"%d.%d$FFF.F$FFF.F",vol[0],vol[1]);
			}
			else
			{
				sprintf(tmpData,"FFF.F$FFF.F$FFF.F");
			}
		}
		cJSON_AddStringToObject(energy, "vol", tmpData); //"219.4$FFF.F$FFF.F");
	}
	if(FILEFLAG_TYPE == FILEFLAG_TGP_GMTER_G55)
	{
		{
			mem_set(tmpData,50, 0x00);
			mem_set(str1,100, 0x00);
			len =app_read_his_item(0x0203FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；瞬时有功功率。
			if(len>0)
			{
				insP[0] = BCD2byte(str1[2]&0x7F);
				insP[1] = BCD2byte(str1[1])*100+BCD2byte(str1[0]);
				if(str1[2]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"-%d.%04d",insP[0],insP[1]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"%d.%04d",insP[0],insP[1]);
				}
				insP[2] = BCD2byte(str1[5]&0x7F);
				insP[3] = BCD2byte(str1[4])*100+BCD2byte(str1[3]);
				if(str1[5]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"$-%d.%04d",insP[2],insP[3]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"$%d.%04d",insP[2],insP[3]);
				}
				insP[4] = BCD2byte(str1[8]&0x7F);
				insP[5] = BCD2byte(str1[7])*100+BCD2byte(str1[6]);
				if(str1[8]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"$-%d.%04d",insP[4],insP[5]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"$%d.%04d",insP[4],insP[5]);
				}
				insP[6] = BCD2byte(str1[11]&0x7F);
				insP[7] = BCD2byte(str1[10])*100+BCD2byte(str1[9]);
				if(str1[11]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"$-%d.%04d",insP[6],insP[7]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"$%d.%04d",insP[6],insP[7]);
				}
			}
			else
			{
				sprintf(tmpData, "FF.FFFF$FF.FFFF$FF.FFFF$FF.FFFF");
			}
		}
		//mem_set(tmpData,30, 0x00);
		//sprintf(tmpData, "%d.%04d$%d.%04d$%d.%04d$%d.%04d",insP[0],insP[1],insP[2],insP[3],insP[4],insP[5],insP[6],insP[7]);
		cJSON_AddStringToObject(energy, "insP", tmpData); //"00.0000$00.0000$FF.FFFF$FF.FFFF");
	}
	else if(FILEFLAG_TYPE == FILEFLAG_SGP_GMTER_G55)
	{
		{
			mem_set(tmpData,50, 0x00);
			mem_set(str1,100, 0x00);
			len =app_read_his_item(0x0203FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；瞬时有功功率。
			if(len>0)
			{
				insP[0] = BCD2byte(str1[2]&0x7F);
				insP[1] = BCD2byte(str1[1])*100+BCD2byte(str1[0]);
				if(str1[2]&0x80)
				{
					sprintf(tmpData, "-%d.%04d",insP[0],insP[1]);
				}
				else
				{
					sprintf(tmpData, "%d.%04d",insP[0],insP[1]);
				}
				insP[2] = BCD2byte(str1[5]&0x7F);
				insP[3] = BCD2byte(str1[4])*100+BCD2byte(str1[3]);
				if(str1[5]&0x80)
				{
					sprintf(tmpData+strlen(tmpData), "$-%d.%04d$FF.FFFF$FF.FFFF",insP[2],insP[3]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData), "$%d.%04d$FF.FFFF$FF.FFFF",insP[2],insP[3]);
				}
			}
			else
			{
				sprintf(tmpData, "FF.FFFF$FF.FFFF$FF.FFFF$FF.FFFF");
			}
			
		}
		//mem_set(tmpData,30, 0x00);
		//sprintf(tmpData, "%d.%04d$%d.%04d$FF.FFFF$FF.FFFF",insP[0],insP[1],insP[2],insP[3]);
		cJSON_AddStringToObject(energy, "insP", tmpData); //"00.0000$00.0000$FF.FFFF$FF.FFFF");
	}
	if(FILEFLAG_TYPE == FILEFLAG_TGP_GMTER_G55)
	{
		{
			mem_set(tmpData,50, 0x00);
			mem_set(str1,100, 0x00);
			len =app_read_his_item(0x0204FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；瞬时无功功率。
			if(len>0)
			{
				insQ[0] = BCD2byte(str1[2]&0x7F);
				insQ[1] = BCD2byte(str1[1])*100+BCD2byte(str1[0]);
				if(str1[2]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"-%d.%04d",insQ[0],insQ[1]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"%d.%04d",insQ[0],insQ[1]);
				}
				insQ[2] = BCD2byte(str1[5]&0x7F);
				insQ[3] = BCD2byte(str1[4])*100+BCD2byte(str1[3]);
				if(str1[5]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"$-%d.%04d",insQ[2],insQ[3]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"$%d.%04d",insQ[2],insQ[3]);
				}
				insQ[4] = BCD2byte(str1[8]&0x7F);
				insQ[5] = BCD2byte(str1[7])*100+BCD2byte(str1[6]);
				if(str1[8]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"$-%d.%04d",insQ[4],insQ[5]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"$%d.%04d",insQ[4],insQ[5]);
				}
				insQ[6] = BCD2byte(str1[11]&0x7F);
				insQ[7] = BCD2byte(str1[10])*100+BCD2byte(str1[9]);
				if(str1[11]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"$-%d.%04d",insQ[6],insQ[7]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"$%d.%04d",insQ[6],insQ[7]);
				}
			}
			else
			{
				sprintf(tmpData, "FF.FFFF$FF.FFFF$FF.FFFF$FF.FFFF");
			}
		}
		//mem_set(tmpData,30, 0x00);
		//sprintf(tmpData, "%d.%04d$%d.%04d$%d.%04d$%d.%04d",insQ[0],insQ[1],insQ[2],insQ[3],insQ[4],insQ[5],insQ[6],insQ[7]);
		cJSON_AddStringToObject(energy, "insQ", tmpData); //"00.0000$00.0000$FF.FFFF$FF.FFFF");
	}
	else if(FILEFLAG_TYPE == FILEFLAG_SGP_GMTER_G55)
	{
		{
			mem_set(tmpData,50, 0x00);
			mem_set(str1,100, 0x00);
			len =app_read_his_item(0x0204FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；瞬时无功功率。
			if(len>0)
			{
				insQ[0] = BCD2byte(str1[2]&0x7F);
				insQ[1] = BCD2byte(str1[1])*100+BCD2byte(str1[0]);
				if(str1[2]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"-%d.%04d",insQ[0],insQ[1]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"%d.%04d",insQ[0],insQ[1]);
				}
				insQ[2] = BCD2byte(str1[5]&0x7F);
				insQ[3] = BCD2byte(str1[4])*100+BCD2byte(str1[3]);
				if(str1[5]&0x80)
				{
					sprintf(tmpData+strlen(tmpData), "$-%d.%04d$FF.FFFF$FF.FFFF",insQ[2],insQ[3]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData), "$%d.%04d$FF.FFFF$FF.FFFF",insQ[2],insQ[3]);
				}
			}
			else
			{
				sprintf(tmpData, "FF.FFFF$FF.FFFF$FF.FFFF$FF.FFFF");
			}
		}
		////mem_set(tmpData,30, 0x00);
		//sprintf(tmpData, "%d.%04d$%d.%04d$FF.FFFF$FF.FFFF",insQ[0],insQ[1],insQ[2],insQ[3]);
		cJSON_AddStringToObject(energy, "insQ", tmpData); //"00.0000$00.0000$FF.FFFF$FF.FFFF");
	}
	if(FILEFLAG_TYPE == FILEFLAG_TGP_GMTER_G55)
	{
		{
			mem_set(tmpData,50, 0x00);
			mem_set(str1,100, 0x00);
			len =app_read_his_item(0x0206FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；功率因数。
			if(len >0)
			{
				pwrFactor[0] = BCD2byte((str1[1]&0x7f)>>4);
				pwrFactor[1] = BCD2byte(str1[1]&0x0F)*100+BCD2byte(str1[0]);
				if(str1[1]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"-%d.%03d",pwrFactor[0],pwrFactor[1]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"%d.%03d",pwrFactor[0],pwrFactor[1]);
				}
				pwrFactor[2] = BCD2byte((str1[3]&0x7f)>>4);
				pwrFactor[3] = BCD2byte(str1[3]&0x0F)*100+BCD2byte(str1[2]);
				if(str1[3]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"$-%d.%03d",pwrFactor[2],pwrFactor[3]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"$%d.%03d",pwrFactor[2],pwrFactor[3]);
				}
				pwrFactor[4] = BCD2byte((str1[5]&0x7F)>>4);
				pwrFactor[5] = BCD2byte(str1[5]&0x0F)*100+BCD2byte(str1[4]);
				if(str1[5]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"$-%d.%03d",pwrFactor[4],pwrFactor[5]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"$%d.%03d",pwrFactor[4],pwrFactor[5]);
				}
				pwrFactor[6] = BCD2byte((str1[7]&0x7F)>>4);
				pwrFactor[7] = BCD2byte(str1[7]&0x0F)*100+BCD2byte(str1[6]);
				if(str1[7]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"$-%d.%03d",pwrFactor[6],pwrFactor[7]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"$%d.%03d",pwrFactor[6],pwrFactor[7]);
				}
			}
			else
			{
				sprintf(tmpData, "F.FFF$F.FFF$F.FFF$F.FFF");
			}
			
		}
		//mem_set(tmpData,30, 0x00);
		//sprintf(tmpData, "%d.%03d$%d.%03d$%d.%03d$%d.%03d", pwrFactor[0], pwrFactor[1],pwrFactor[2], pwrFactor[3], pwrFactor[4], pwrFactor[5],pwrFactor[6], pwrFactor[7]);
		cJSON_AddStringToObject(energy, "pwrF", tmpData); //"1.000$1.000$F.FFF$F.FFF");
	}
	else if(FILEFLAG_TYPE == FILEFLAG_SGP_GMTER_G55)
	{
		{
			mem_set(tmpData,50, 0x00);
			mem_set(str1,100, 0x00);
			len =app_read_his_item(0x0206FF00,str1,str2,255,0,2000);//str1中存在着我们要读取的值；功率因数。
			if(len>0)
			{
				pwrFactor[0] = BCD2byte((str1[1]&0x7f)>>4);
				pwrFactor[1] = BCD2byte(str1[1]&0x0F)*100+BCD2byte(str1[0]);
				if(str1[1]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"-%d.%03d",pwrFactor[0],pwrFactor[1]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"%d.%03d",pwrFactor[0],pwrFactor[1]);
				}
				pwrFactor[2] = BCD2byte((str1[3]&0x7F)>>4);
				pwrFactor[3] = BCD2byte(str1[3]&0x0F)*100+BCD2byte(str1[2]);
				if(str1[3]&0x80)
				{
					sprintf(tmpData+strlen(tmpData), "$-%d.%03d$F.FFF$F.FFF", pwrFactor[2], pwrFactor[3]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData), "$%d.%03d$F.FFF$F.FFF", pwrFactor[2], pwrFactor[3]);
				}
			}
			else
			{
				sprintf(tmpData, "F.FFF$F.FFF$F.FFF$F.FFF");
			}
			
		}
		//mem_set(tmpData,30, 0x00);
		//sprintf(tmpData, "%d.%03d$%d.%03d$F.FFF$F.FFF", pwrFactor[0], pwrFactor[1],pwrFactor[2], pwrFactor[3]);
		cJSON_AddStringToObject(energy, "pwrF", tmpData); //"1.000$1.000$F.FFF$F.FFF");
	}
	if(FILEFLAG_TYPE == FILEFLAG_TGP_GMTER_G55)
	{
		{
			mem_set(tmpData,50, 0x00);
			mem_set(str1,100, 0x00);
			len =app_read_his_item(0x02800001,str1,str2,255,0,2000);//str1中存在着我们要读取的值；零线电流。
			if(len >0)
			{
				zero_cur[0] = (BCD2byte(str1[2]&0x7F)*10) +(str1[1]>>4);
				zero_cur[1] = (str1[1]&0x0F)*100 +BCD2byte(str1[0]);
				if(str1[2]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"-%d.%03d",zero_cur[0],zero_cur[1]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"%d.%03d",zero_cur[0],zero_cur[1]);
				}
			}
			else
			{
				sprintf(tmpData, "FFF.FFF");
			}
		
		}
		cJSON_AddStringToObject(energy, "curZero", tmpData); 
	}
	else if(FILEFLAG_TYPE == FILEFLAG_SGP_GMTER_G55)
	{
		{
			mem_set(tmpData,50, 0x00);
			mem_set(str1,100, 0x00);
			len =app_read_his_item(0x02800001,str1,str2,255,0,2000);//str1中存在着我们要读取的值；零线电流。
			if(len >0)
			{
				zero_cur[0] = (BCD2byte(str1[2]&0x7F)*10) +(str1[1]>>4);
				zero_cur[1] = (str1[1]&0x0F)*100 +BCD2byte(str1[0]);
				if(str1[2]&0x80)
				{
					sprintf(tmpData+strlen(tmpData),"-%d.%03d",zero_cur[0],zero_cur[1]);
				}
				else
				{
					sprintf(tmpData+strlen(tmpData),"%d.%03d",zero_cur[0],zero_cur[1]);
				}
			}
			else
			{
				sprintf(tmpData, "FFF.FFF");
			}
				
		}
		cJSON_AddStringToObject(energy, "curZero", tmpData);
	}	
	out = cJSON_PrintUnformatted(root);
	mem_cpy(buf,out,resLen = strlen(out));
	cJSON_Delete(root);
	vPortFree(out);
	return resLen;
}

#endif // DEBUG
