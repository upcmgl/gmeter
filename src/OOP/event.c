/*
 * event.c
 *
 * Created: 2019-1-24 14:52:43
 *  Author: mgl
 */ 
#include "event.h"
void powerOnOff()
{
	volatile DateTime datetime;
	volatile INT8U  time[7],tmpMin,tmpSec;
	volatile INT8U tmp;
	INT32U  event_idx=0; //用于记录事件记录序号。
	INT8U idx;
	tpos_datetime(&datetime);
	//mem_cpy(&time,datetime.century*100+datetime.year,2);
	time[0]= (datetime.century*100+datetime.year)/256;
	time[1]= (datetime.century*100+datetime.year)%256;
	time[2]=datetime.month;
	time[3]=datetime.day;
	time[4]=datetime.hour;
	time[5]=datetime.minute;
	time[6]=datetime.second;

	if(dev_power_lost()&& (TerminalPowerOnOffState==1))//检测到掉电。
	{
		
		TerminalPowerOnOffState = 0;
		//fread_ertu_params(TERMINAL_POWER_ONOFF,&TerminalPowerOnOffEvent,sizeof(StrTerminalPowerOnOffEventBlock));
		if(TerminalPowerOnOffEvent.eventIndex>=15)
		{
			TerminalPowerOnOffEvent.eventIndex=15;
			for(idx=0;idx<14;idx++)
			{
				//TerminalPowerOnOffEvent.event[idx+1].eventIndex[4] = idx;
				mem_cpy(&(TerminalPowerOnOffEvent.event[idx]),&(TerminalPowerOnOffEvent.event[idx+1]),sizeof(StrTerminalPowerOnOff));
			}
			
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventIndex[0] = 0x06;
			//event_idx = &TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventIndex[1];
			mem_cpy_reverse((INT8U*)&event_idx,&TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventIndex[1],4);
			event_idx++;
			if(event_idx==0xFFFFFFFF)
			{
				event_idx =0;
			}
			mem_cpy_reverse(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventIndex[1]),(INT8U*)(&event_idx),4);//更改事件记录序号			
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].beginTime[0] = 0x1C;  //开始时间
			mem_cpy(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].beginTime[1]),time,7);

			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].endTime[0] = 0x00;  //结束时间  //
			mem_set(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].endTime[1]),7,0x00);
			
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventSource[0] = 0x16; //发生源
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventSource[1] = 0;
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].reportState[0] = 0x1;  //上报状态
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].reportState[1] = 0;
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventAttribute[0] = 0x04;
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventAttribute[1] = 0x08;
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventAttribute[2] = 0x80;
			
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].has_report = 0xAA;
		}
		else
		{
			//	TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventIndex = TerminalPowerOnOffEvent.eventIndex;
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventIndex[0] = 0x06;
			if(TerminalPowerOnOffEvent.eventIndex ==0)	//初次上电只能生成掉电事件了。
			{
				event_idx =1;
				mem_cpy_reverse(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventIndex[1]),(INT8U*)(&event_idx),4);//更改事件记录序号

			}
			else
			{
				mem_cpy_reverse((INT8U*)&event_idx,&TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventIndex[1],4);
				event_idx++;
				if(event_idx==0xFFFFFFFF)
				{
					event_idx =0;
				}
				mem_cpy_reverse(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventIndex[1]),(INT8U*)(&event_idx),4);//更改事件记录序号
			}

			//mem_set(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventIndex[1]),3,0x00);
			//TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventIndex[4] = TerminalPowerOnOffEvent.eventIndex;
			
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].beginTime[0] = 0x1C;  //开始时间
			mem_cpy(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].beginTime[1]),time,7);

			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].endTime[0] = 0x00;  //结束时间
			mem_set(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].endTime[1]),7,0x00);
			
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventSource[0] = 0x16; //发生源
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventSource[1] = 0;
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].reportState[0] = 0x1;  //上报状态
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].reportState[1] = 0;
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventAttribute[0] = 0x04;
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventAttribute[1] = 0x08;
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventAttribute[2] = 0x80;
			
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].has_report =0xAA;
			TerminalPowerOnOffEvent.eventIndex++;
			system_debug_info("event is store 0           ");
		}
		fwrite_ertu_params(TERMINAL_POWER_ONOFF,&TerminalPowerOnOffEvent,sizeof(StrTerminalPowerOnOffEventBlock));
		system_debug_info("event is store1           ");
			
	}
	else if(!dev_power_lost()&& (TerminalPowerOnOffState==0))	  //上电后。
	{
		tpos_datetime(&datetime);
		fread_ertu_params(TERMINAL_POWER_ONOFF,&TerminalPowerOnOffEvent,sizeof(StrTerminalPowerOnOffEventBlock));
		//tmpMin = TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].beginTime[6]; //得到上次停电时分钟数，用来判断上电有效没
		//tmpSec = TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].beginTime[7]; //得到上次停电时分钟数，用来判断上电有效没
		TerminalPowerOnOffState = 1;
//		tmp =TerminalPowerOnOffEvent.eventIndex;
		if((TerminalPowerOnOffEvent.eventIndex)>=15)
		{
			//tmp =0;
			TerminalPowerOnOffEvent.eventIndex=15;
			for(idx=0;idx<14;idx++)
			{
				//TerminalPowerOnOffEvent.event[idx+1].eventIndex[4] = idx;
				mem_cpy(&(TerminalPowerOnOffEvent.event[idx]),&(TerminalPowerOnOffEvent.event[idx+1]),sizeof(StrTerminalPowerOnOff));
			}
			tmpMin = TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].beginTime[6]; //得到上次停电时分钟数，用来判断上电有效没
			tmpSec = TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].beginTime[7]; //得到上次停电时分钟数，用来判断上电有效没
					
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventIndex[0] = 0x06;
			

			mem_cpy_reverse((INT8U*)&event_idx,&TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventIndex[1],4);
			event_idx++;
			if(event_idx==0xFFFFFFFF)
			{
				event_idx =0;
			}
			mem_cpy_reverse(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventIndex[1]),(INT8U*)(&event_idx),4);//更改事件记录序号
			
			//mem_set(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventIndex[1]),3,0x00);
			//TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventIndex[4] = TerminalPowerOnOffEvent.eventIndex;
			
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].beginTime[0] = 0x1C;  //开始时间
			mem_cpy(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].beginTime[1]),&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].beginTime[1]),7);

			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].endTime[0] = 0x1C;  //结束时间
			mem_cpy(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].endTime[1]),time,7);
			
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventSource[0] = 0x16; //发生源
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventSource[1] = 1;
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].reportState[0] = 0x1;  //上报状态
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].reportState[1] = 0;
			
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventAttribute[0] = 0x04;
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventAttribute[1] = 0x08;
			
			if((((datetime.minute -tmpMin)*60+datetime.second -tmpSec)>60) ||( ((tmpMin - datetime.minute)*60+tmpSec-datetime.second)>60))
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventAttribute[2] = 0xC0;
			else
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventAttribute[2] = 0x80;
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].has_report =0xAA;
		}
		else
		{
			tmpMin = TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].beginTime[6]; //得到上次停电时分钟数，用来判断上电有效没
			tmpSec = TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].beginTime[7]; //得到上次停电时分钟数，用来判断上电有效没			
			//	TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventIndex = TerminalPowerOnOffEvent.eventIndex;
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventIndex[0] = 0x06;

			mem_cpy_reverse((INT8U*)&event_idx,&TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].eventIndex[1],4);
			event_idx++;
			if(event_idx==0xFFFFFFFF)
			{
				event_idx =0;
			}
			mem_cpy_reverse(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventIndex[1]),(INT8U*)(&event_idx),4);//更改事件记录序号
			
			//mem_set(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventIndex[1]),3,0x00);
			//TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventIndex[4] = TerminalPowerOnOffEvent.eventIndex;
			
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].beginTime[0] = 0x1C;  //开始时间
			mem_cpy(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].beginTime[1]),&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex-1].beginTime[1]),7);

			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].endTime[0] = 0x1C;  //结束时间
			mem_cpy(&(TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].endTime[1]),time,7);
			
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventSource[0] = 0x16; //发生源
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventSource[1] = 1;
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].reportState[0] = 0x1;  //上报状态
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].reportState[1] = 0;
			
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventAttribute[0] = 0x04;
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventAttribute[1] = 0x08;
			
			if((((datetime.minute -tmpMin)*60+datetime.second -tmpSec)>60) ||( ((tmpMin - datetime.minute)*60+tmpSec-datetime.second)>60))
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventAttribute[2] = 0xC0;
			else
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].eventAttribute[2] = 0x80;
			
			TerminalPowerOnOffEvent.event[TerminalPowerOnOffEvent.eventIndex].has_report =0xAA;
			TerminalPowerOnOffEvent.eventIndex++;
		}
		fwrite_ertu_params(TERMINAL_POWER_ONOFF,&TerminalPowerOnOffEvent,sizeof(StrTerminalPowerOnOffEventBlock));
		system_debug_info("event is store2           ");
		
	}
}

void reportOnOffEvent(INT8U event_report_idx)
{
	INT8U buffer[200] ={0};
    INT8U pos = 0;
    tagDatetime datetime;
    INT8U head_len = 0;
    os_get_datetime(&datetime);
    buffer[pos++]=0x68;
    buffer[pos++]=0x00;
    buffer[pos++]=0x00;
    buffer[pos++]=CTRLFUNC_DIR_SET & CTRLFUNC_PRM_CLR |  CTRL_FUNC_USERDATA;
    buffer[pos++]=gSystemInfo.devid_len-1;
    mem_cpy_reverse(buffer + OOP_POS_ADDR+1,gSystemInfo.ertu_devid,gSystemInfo.devid_len); //这里我设置表号来读取
    // mem_cpy_reverse(pResp->frame+OOP_POS_ADDR+1,gSystemInfo.meter_no,gSystemInfo.devid_len);
    pos+=gSystemInfo.devid_len;
    buffer[pos++]=0x10;//主站地址
    pos+=2;
    buffer[pos++] =0x88;
    buffer[pos++] =0x02;
    buffer[pos++] =0x00;
    buffer[pos++] =0x01;
    mem_cpy(buffer+pos,"\x31\x06\x02\x0",4);
    pos+=4;
    buffer[pos++] =0x05;
    mem_cpy(buffer+pos,"\x0\x20\x22\x2\x0",5);
    pos+=5;
    mem_cpy(buffer+pos,"\x0\x20\x1E\x2\x0",5);
    pos+=5;
    mem_cpy(buffer+pos,"\x0\x20\x20\x2\x0",5);
    pos+=5;
    mem_cpy(buffer+pos,"\x0\x20\x24\x2\x0",5);
    pos+=5;	
    mem_cpy(buffer+pos,"\x0\x33\x09\x2\x6",5);
    pos+=5;
    buffer[pos++] = 0x01;
	buffer[pos++] = 0x01;
	
	buffer[pos++] = 0x06;
	mem_cpy(buffer+pos,&TerminalPowerOnOffEvent.event[event_report_idx].eventIndex[1],4);
    pos+=4;
	

    buffer[pos++] = 0x1C;
	mem_cpy(buffer+ pos,&(TerminalPowerOnOffEvent.event[event_report_idx].beginTime[1]),7);
	pos+=7;

	if(TerminalPowerOnOffEvent.event[event_report_idx].eventSource[1])	  //结束时间 ，上电才有结束时间，掉电没有结束时间。
	{
		buffer[pos++] = 0x1C;
		mem_cpy(buffer+ pos,&(TerminalPowerOnOffEvent.event[event_report_idx].endTime[1]),7);
		pos+=7;
	}
	else
	{
		buffer[pos++] =0;
	}


	buffer[pos++] =0x16;
	buffer[pos++] =TerminalPowerOnOffEvent.event[event_report_idx].eventSource[1];	//发生源。		 0 停电，1上电
	buffer[pos++] =0x04;
	buffer[pos++] =0x08;
	buffer[pos++] =TerminalPowerOnOffEvent.event[event_report_idx].eventAttribute[2];  //发生是否有效。
	

    buffer[pos++] =0x00;
    buffer[pos++] =0x00;
    
    buffer[OOP_POS_LEN] =  pos-1+2;
    buffer[OOP_POS_LEN+1] = (pos-1+2) >> 8;
    head_len= 12;
    fcs16(buffer+OOP_POS_LEN,head_len-1);
    //计算FCS校验位
    fcs16(buffer+OOP_POS_LEN,pos-1);
    pos += 2;
    buffer[pos++] = 0x16;
		
	mem_cpy(ResponseApp.frame,buffer,pos);
	ResponseApp.frame_len =pos;
	ResponseApp.pos =pos;
	ResponseApp.channel = PROTOCOL_CHANNEL_DEBUG;
	app_send_ReplyFrame(&ResponseApp);
	ResponseApp.channel = PROTOCOL_CHANNEL_REMOTE;
	app_send_ReplyFrame(&ResponseApp);
}