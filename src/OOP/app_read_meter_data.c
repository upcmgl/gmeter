#include "../main_include.h"
INT8U transIndex =0;
void init_separate_frame_info(INT8U apdu_service_code)
{
    //需要先初始化分帧信息，防止有了新的请求还保留上次分帧信息
    mem_set(separate_frame_mem.index_id,6,0x00);
    mem_set(separate_frame_mem.idx_start,8,0x00);
    separate_frame_mem.more_frame = FALSE;
    separate_frame_mem.is_frist   = TRUE;
    separate_frame_mem.prepare_param = TRUE;
    separate_frame_mem.apdu_service_code=apdu_service_code;//
    separate_frame_mem.frame_id=0;
}

INT16U make_read_oop_data_frame(INT8U *frame,RSD *rsd,INT8U *begin_time,INT8U *end_time,INT8U *last_count,INT8U *rcsd,INT8U *data_type,INT8U *end_flag,BOOLEAN *hold_flag,INT8U *hold_oad)
{
    INT8U idx,csd_cnt,road_count,oad_count,col_idx,col_cnt,choice_flag=0;
    INT8U master_oad[4]={0},buffer[50]={0},ti[4]={0};
    INT8U *col_cnt_pos;
    BOOLEAN is_event=FALSE;
    INT16U obis,pos,frame_pos;

    transIndex =0;  //因为是全局变量，所以要清一次0
    pos=0;csd_cnt=rcsd[pos++];
    road_count=0;
    oad_count=0;
    frame_pos=0;
    for(idx=0;idx<csd_cnt;idx++)
    {
        if(rcsd[pos++]==0)
        {
            obis=cosem_bin2_int16u(rcsd+pos);
            switch(obis)
            {
            case OBIS_COMM_ADDR:
            case OBIS_SERVER_ADDR:
            case OBIS_ACQ_BEGIN_TD:
            case OBIS_ACQ_END_TD:
            case OBIS_ACQ_SAVE_TD:
                pos+=4;
                break;
            default:
                oad_count++;
                mem_cpy(frame+frame_pos+1,rcsd+pos,4);
                frame[0]=oad_count;		
                frame_pos+=4;
                pos+=4;
                break;
            }
        }
        else
        {
            road_count++;
            mem_cpy(master_oad,rcsd+pos,4);
            obis=cosem_bin2_int16u(master_oad);
            switch(obis)
            {
            case OBIS_MINUTE_HOLD:
                ti[0]=DT_TI;
                ti[1]=1;
                ti[2]=rsd->choice_6_7_8.time_interval[1];
                ti[3]=rsd->choice_6_7_8.time_interval[2];
                break;
            case OBIS_HOUR_HOLD:
                ti[0]=DT_TI;
                ti[1]=2;
                ti[2]=0;
                ti[3]=1;
                break;
            case OBIS_DAY_HOLD:
                ti[0]=DT_TI;
                ti[1]=3;
                ti[2]=0;
                ti[3]=1;
                break;
            case OBIS_MONTH_HOLD:
                ti[0]=DT_TI;
                ti[1]=4;
                ti[2]=0;
                ti[3]=1;
                break;
            default:
                if((obis>=0x3000)&&(obis<0x3030))
                {
                    is_event=TRUE;
                }
                else
                {
                    return 0;
                }
                break;
            }

            pos+=4;
            col_cnt=rcsd[pos++];
            col_cnt_pos=frame+frame_pos;
            frame_pos++;
            *hold_flag=FALSE;
            for(col_idx=0;col_idx<col_cnt;col_idx++)
            {
                frame[frame_pos++]=0;
                if(is_event)
                {
                    mem_cpy(hold_oad,"\x20\x1E\x02\x00",4);
                }
                else
                {
                    mem_cpy(hold_oad,"\x20\x21\x02\x00",4);
                }
                if(compare_string(rcsd+pos,hold_oad,4)==0)
                {
                    *hold_flag=TRUE;
                }
				//由于反向无功需量没有，我们这里用正向无功需量的值替代
				if(((*(rcsd+pos))==0x10) &&((*(rcsd+pos+1))==0x40))
				{
					transIndex = col_idx+1; //指示无功需量在哪个位置。
					mem_cpy(frame+frame_pos,rcsd+pos,4);
					*(frame+frame_pos+1) = 0x30;
				}
                else
				
					mem_cpy(frame+frame_pos,rcsd+pos,4); 
                frame_pos+=4;
                pos+=4;
            }
            if(!*hold_flag)
            {
                mem_cpy_right(col_cnt_pos+6,col_cnt_pos+1,frame+frame_pos-col_cnt_pos-1);
                col_cnt_pos[1]=0;
                mem_cpy(col_cnt_pos+2,hold_oad,4);
                frame_pos+=5;
                col_cnt++;
            }
            *col_cnt_pos=col_cnt;
        }
    }
    if(road_count>1)
    {
        return 0;
    }
    if(road_count==0)
    {
        if(oad_count>0)
        {
            frame_pos++;
            *data_type=0;   //实时的
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if(oad_count==0)
        {
            if(is_event)
            {
                *data_type=2;  //事件类
            }
            else
            {
                *data_type=1;  //记录类
            }
        }
        else
        {
            return 0;
        }
    }

    buffer[OOP_POS_BEGIN] = 0x68;
    buffer[OOP_POS_LEN]   = 0x00;
    buffer[OOP_POS_LEN+1] = 0x00;
    buffer[OOP_POS_CTRL]  = 0x43;//CTRLFUNC_PRM_SET|CTRLFUNC_FC_REQ_RESP
    pos = OOP_POS_ADDR;
    buffer[pos++] = 0x05 | (0x01<<6);      //地址类型
    mem_set(buffer+pos,6,0xAA);

    pos += 6;
    buffer[pos++] = 0x10;                  //客户机地址CA 以前是0x22
    buffer[pos++] = 0x00;                  //HCS
    buffer[pos++] = 0x00;                  //
    buffer[pos++] = 0x05;                  //APDU-读取
    if(*data_type==0)
    {
        buffer[pos++] = 0x02;                  //choice-普通读取
        buffer[pos++] = 0x00;                  //PIID
        mem_cpy_right(frame+pos,frame,frame_pos);
        mem_cpy(frame,buffer,pos);
        pos+=frame_pos;
        frame[pos++] = 0x00;                 //tp
        frame[OOP_POS_LEN] = (pos+2-1);
        frame[OOP_POS_LEN+1] = ((pos+2-1)>>8);
        fcs16(frame+OOP_POS_LEN,11);
        fcs16(frame+OOP_POS_LEN,pos-1);
        pos += 2;
        frame[pos++]=0x16;
        mem_cpy_right(frame+4,frame,pos);
        mem_set(frame,4,0xFE);
        pos += 4;
        *end_flag=TRUE;
        return pos;
    }

    if(*data_type==1)
    {
        buffer[pos++] = 0x03;                  //choice-记录读取读取
        buffer[pos++] = 0x00;                  //PIID
        mem_cpy(buffer+pos,master_oad,4);
        pos+=4;
        switch(rsd->choice)
        {
        case 4:
        case 5:
            buffer[pos++]=1;
            choice_flag=1;
            break;
        case 6:
        case 7:
        case 8:
            buffer[pos++]=2;
            choice_flag=2;
            break;
        case 10:    //budong
            buffer[pos++]=9;
            buffer[pos++]=*last_count;
            mem_cpy_right(frame+pos,frame,frame_pos);
            mem_cpy(frame,buffer,pos);
            pos+=frame_pos;
            frame[pos++] = 0x00;                 //tp
            frame[OOP_POS_LEN] = (pos+2-1);
            frame[OOP_POS_LEN+1] = ((pos+2-1)>>8);
            fcs16(frame+OOP_POS_LEN,11);
            fcs16(frame+OOP_POS_LEN,pos-1);
            pos += 2;
            frame[pos++]=0x16;
            mem_cpy_right(frame+4,frame,pos);
            mem_set(frame,4,0xFE);
            pos += 4;
            (*last_count)--;
            if((*last_count)==0)
            {
                *end_flag=TRUE;
            }
            return pos;
        default:
            return 0;
        }
        
        mem_cpy(buffer+pos,"\x20\x21\x2\x0",4);
        pos+=4;
        buffer[pos++]=DT_DATETIME_S;
		if((INT16U)master_oad == 0x5004)
		{
			
		}
        mem_cpy(buffer+pos,begin_time,7);   //
        pos+=7;
        if(choice_flag==2)
        {
            buffer[pos++]=DT_DATETIME_S;
            cosem_datetime_add_ti(begin_time,ti+1);
            mem_cpy(buffer+pos,begin_time,7);
            pos+=7;
            mem_cpy(buffer+pos,ti,4);
            pos+=4;
        }

        mem_cpy_right(frame+pos,frame,frame_pos);
        mem_cpy(frame,buffer,pos);
        pos+=frame_pos;
        frame[pos++] = 0x00;                 //tp
        frame[OOP_POS_LEN] = (pos+2-1);
        frame[OOP_POS_LEN+1] = ((pos+2-1)>>8);
        fcs16(frame+OOP_POS_LEN,11);
        fcs16(frame+OOP_POS_LEN,pos-1);
        pos += 2;
        frame[pos++]=0x16;
        mem_cpy_right(frame+4,frame,pos);
        mem_set(frame,4,0xFE);
        pos += 4;
        if(choice_flag==1)
        {
            *end_flag=TRUE;
        }
        else
        {
            if(compare_string(begin_time,end_time,7)>=0)
            {
                *end_flag=TRUE;
            }
        }
        return pos;
    }
    if(*data_type==2)
    {
        buffer[pos++] = 0x03;                  //choice-记录读取读取
        buffer[pos++] = 0x00;                  //PIID
        mem_cpy(buffer+pos,master_oad,4);
        pos+=4;
        buffer[pos++]=9;
        buffer[pos++]=*last_count;
        mem_cpy_right(frame+pos,frame,frame_pos);
        mem_cpy(frame,buffer,pos);
        pos+=frame_pos;
        frame[pos++] = 0x00;                 //tp
        frame[OOP_POS_LEN] = (pos+2-1);
        frame[OOP_POS_LEN+1] = ((pos+2-1)>>8);
        fcs16(frame+OOP_POS_LEN,11);
        fcs16(frame+OOP_POS_LEN,pos-1);
        pos += 2;
        frame[pos++]=0x16;
        mem_cpy_right(frame+4,frame,pos);
        mem_set(frame,4,0xFE);
        pos += 4;
        (*last_count)--;
        if((*last_count)==0)
        {
            *end_flag=TRUE;
        }
        return pos;
    }
    return 0;
}
INT16U save_read_data_OOP_from_flash(INT8U* frame,INT16U frame_len,objResponse *pResp,INT8U *rcsd,BOOLEAN hold_flag,INT8U *hold_oad,COSEM_DATETIME_S *hold_time,INT16U left_len,BOOLEAN *is_more_frame,INT8U *tmp_td,INT8U *task_start_time_frq,INT8U* day_hold_flag)
{
	OBJECT_ATTR_DESC *oad;
	INT8U *data,*resp,*master_oad;
	INT16U oad_id,pos=0,data_len,data_pos,csd_pos,td_pos;
	volatile INT16U resp_pos;
	INT8U count_col,idx_col,idx,count,find_idx;
	COSEM_DATETIME_S ok_time;
	BOOLEAN add_hold_time_oad,is_find;
	INT8U task_start_time[7];  //用来计算采集启动时标
	INT16U  DayHoldFlag;   //为测试日冻结设置的flag。
	INT8U  hold_type_ornot =0;
	add_hold_time_oad = FALSE;
	resp=pResp->frame+pResp->pos;    //resp为返回给主站的报文位置
	resp_pos=0;
//	pos = get_frame_header_length_gb_oop(frame);   //frame原来请求电表回应帧
	data_len = 0;

	hold_time->tag = DT_DATETIME_S;
	ok_time.tag=DT_DATETIME_S;
	sys_datetime2cosem_datetime(ok_time.value);
	
	idx = 0;
	csd_pos=0;
	for (idx=0;idx<rcsd[0];idx++)
	{
		if(rcsd[1+5*idx]==1)
		{
			csd_pos=2+5*idx;
			csd_pos+=4;
			count=rcsd[csd_pos++];   //有多少个oad电表回的
#ifdef	__PROVINCE_JIBEI_FUHEGANZHI__
	
#endif			
			hold_type_ornot =1;
			break;
		}
	}


	
	for (idx=0;idx<rcsd[0];idx++)
	{
		oad_id = (rcsd[idx*5+2]<<8)+rcsd[idx*5+3];
		switch(oad_id)
		{
			case OBIS_COMM_ADDR: //电表地址
				resp[resp_pos++]=0x09;
				resp[resp_pos++]=0x06;
				mem_cpy_reverse(resp+resp_pos,gSystemInfo.meter_no,6);
				//mem_cpy(resp+resp_pos,gSystemInfo.ertu_devid,6); //也可以是表地址
				resp_pos += 6;
				break;
			case OBIS_SERVER_ADDR: //目标服务器地址
			if(resp_pos+sizeof(COSEM_TSA)+5<left_len)
			{
				resp[resp_pos++]=DT_TSA;
				resp[resp_pos++]=7;
				resp[resp_pos++]=5;
				//#ifdef __PROVICE_SHANDONG__
				//mem_cpy(resp+resp_pos,gSystemInfo.ertu_devid,6);
				//#else
				//mem_cpy(resp+resp_pos,gSystemInfo.ertu_devid,6); //也可以是表地址
				mem_cpy_reverse(resp+resp_pos,gSystemInfo.meter_no,6);
				resp_pos += 6;
			}
			else
			{
				*is_more_frame=TRUE;
			}
			break;
			case 0x6040: //采集启动时标
			{
				*(resp+resp_pos) = DT_DATETIME_S;
				mem_cpy(resp+resp_pos+1,task_start_time_frq,sizeof(COSEM_DATETIME_S)-1);
				resp_pos+=sizeof(COSEM_DATETIME_S);
				break;
			}
			case 0x6042: //采集存储时标
			if(resp_pos+sizeof(COSEM_DATETIME_S)+5<left_len)
			{
				*(resp+resp_pos) = DT_DATETIME_S;
			//	mem_cpy(resp+resp_pos+1,tmp_td,sizeof(COSEM_DATETIME_S)-1);
				mem_cpy(resp+resp_pos+1,task_start_time_frq,sizeof(COSEM_DATETIME_S)-1);
				if(*day_hold_flag) //如果是日冻结。需要修改采集存储时标根据方案内容
				{
					change_day_hold_serach_time(resp+resp_pos+1,2);
				}
				resp_pos+=sizeof(COSEM_DATETIME_S);
			}
			else
			{
				*is_more_frame=TRUE;
			}
			break;
			case 0x6041: //采集成功时标
			if(resp_pos+sizeof(COSEM_DATETIME_S)+5<left_len)
			{
				*(resp+resp_pos) = DT_DATETIME_S;
				mem_cpy(resp+resp_pos+1,task_start_time_frq,sizeof(COSEM_DATETIME_S)-1);
				resp_pos+=sizeof(COSEM_DATETIME_S);
			}
			else
			{
				*is_more_frame=TRUE;
			}
			break;
			default:
			if(hold_type_ornot ==1)  //冻结 类
			{
				data_pos=0;
				resp[resp_pos++]=DT_ARRAY;
				resp[resp_pos++]=count;
				count_col =count;
				for (idx_col = 0;idx_col < count_col;idx_col++)
				{
			 		data_len=get_object_attribute_item_len(frame+pos,frame[pos]);
					//if((!hold_flag)&&(idx_col==find_idx))
					//{
						//pos+=data_len;
						//continue;
					//}
	//				data_len = 1;  //为了测试
					if(resp_pos+data_len+5<left_len)
					{
						//if(*(frame+pos) == 0x1c)
						//{
							//mem_cpy(frame+pos,&(hold_time->tag),sizeof(COSEM_DATETIME_S)); //冻结时间这里要减1分钟，使用上面减好的时间。
						//}
						mem_cpy(resp+resp_pos,frame+pos,data_len);

						//*(resp+resp_pos) =0x00;//为了测试,提供作假数据
						resp_pos+=data_len;
						pos+=data_len;
					}
					else
					{
						*is_more_frame=TRUE;
						break;
					}
				}
			}
			else  //实时类
			{
				data_len=get_object_attribute_item_len(frame+pos,frame[pos]);
				//if((!hold_flag)&&(idx_col==find_idx))
				//{
					//pos+=data_len;
					//continue;
				//}
//				data_len = 1;  //为了测试
				if(resp_pos+data_len+5<left_len)
				{
					//if(*(frame+pos) == 0x1c)
					//{
						//mem_cpy(frame+pos,&(hold_time->tag),sizeof(COSEM_DATETIME_S)); //冻结时间这里要减1分钟，使用上面减好的时间。
					//}
					mem_cpy(resp+resp_pos,frame+pos,data_len);

					//*(resp+resp_pos) =0x00;//为了测试,提供作假数据
					resp_pos+=data_len;
					pos+=data_len;
				}
				else
				{
					*is_more_frame=TRUE;
					break;
				}
			}
			break;
		}
		if(*is_more_frame)
		{
			break;
		}
	}
	
	return resp_pos;
	
	
	
/*	
	if (frame[pos++] == SERVER_APDU_GET)
	{
		if (frame[pos] == GET_RESPONSE_NORMAL_LIST) //APDU
		{
			pos += 2;  //apdu和piid
			count_col = frame[pos++];
			data=frame+pos;
			sys_datetime2cosem_datetime(hold_time->value);
			hold_time->value[4]=0;
			hold_time->value[5]=0;
			hold_time->value[6]=0;

			for (idx=0;idx<rcsd[0];idx++)
			{
				oad_id = (rcsd[idx*5+2]<<8)+rcsd[idx*5+3];
				switch(oad_id)
				{
					case OBIS_COMM_ADDR: //电表地址
					case OBIS_SERVER_ADDR: //目标服务器地址
					resp[resp_pos++]=DT_TSA;
					resp[resp_pos++]=7;
					resp[resp_pos++]=5;
					#ifdef __PROVICE_SHANDONG__
					mem_cpy(resp+resp_pos,gSystemInfo.ertu_devid,6);
					#else
					mem_cpy_reverse(resp+resp_pos,gSystemInfo.meter_no,6);
					#endif
					resp_pos += 6;
					break;
					case 0x6040: //采集启动时间

					case 0x6042: //采集存储时标  //andu
					mem_cpy(resp+resp_pos,&(hold_time->tag),sizeof(COSEM_DATETIME_S));
					resp_pos+=sizeof(COSEM_DATETIME_S);
					break;
					case 0x6041: //采集到时间
					mem_cpy(resp+resp_pos,&(ok_time.tag),sizeof(COSEM_DATETIME_S));
					resp_pos+=sizeof(COSEM_DATETIME_S);
					break;
					default:
					data_pos=0;
					is_find=FALSE;
					for (idx_col = 0;idx_col < count_col;idx_col++)
					{
						oad = (OBJECT_ATTR_DESC*)(data+data_pos);
						data_pos += sizeof(OBJECT_ATTR_DESC);
						if (compare_string(oad->value,rcsd+idx*5+2,sizeof(OBJECT_ATTR_DESC)) == 0)
						{
							is_find=TRUE;
							if(data[data_pos++]==1)
							{
								data_len=get_object_attribute_item_len(data+data_pos,data[data_pos]);
								mem_cpy(resp+resp_pos,data+data_pos,data_len);
								resp_pos+=data_len;
							}
							else
							{
								resp[resp_pos++]=DT_NULL;
							}
							break;
						}
						else
						{
							if(data[data_pos++]==1)
							{
								data_len=get_object_attribute_item_len(data+data_pos,data[data_pos]);
								data_pos+=data_len;
							}
							else
							{
								data_pos+=2;
							}
						}
					}
					if(!is_find)
					{
						resp[resp_pos++]=DT_NULL;
					}
					break;
				}
			}
		}
		else if ((frame[pos] == GET_RESPONSE_RECORD))
		{
			pos += 2;
			master_oad = frame+pos;
			pos += 4;
			count_col = frame[pos++];

			//比较列头
			idx = 0;
			csd_pos=0;
			for (idx=0;idx<rcsd[0];idx++)
			{
				if(rcsd[1+5*idx]==1)
				{
					csd_pos=2+5*idx;
					break;
				}
			}
			if(compare_string(rcsd+csd_pos,master_oad,4)!=0)
			{
				return 0;
			}
			csd_pos+=4;
			count=rcsd[csd_pos++];   //有多少个oad电表回的
			if((hold_flag)&&(count!=count_col)) return 0;
			if((!hold_flag)&&((count+1)!=count_col))    return 0;
			find_idx=0;
			for(idx_col=0;idx_col<count;idx_col++)
			{
				if(hold_flag)
				{
					if(compare_string(frame+pos+idx_col*5+1,rcsd+csd_pos+idx_col*4,4)!=0)
					{
						return 0;
					}
					if(compare_string(frame+pos+idx_col*5+1,hold_oad,4)==0)
					{
						find_idx=idx_col;
					}
				}
				else
				{
					if(compare_string(frame+pos+(idx_col+1)*5+1,rcsd+csd_pos+idx_col*4,4)!=0)
					{
						return 0;
					}
				}
			}
			pos+=5*count_col;
			//处理电表数据
			if (frame[pos++] != 1)   //是否有记录数据
			{
				return 0;
			}
			if (frame[pos++] == 0)   //记录数据条数不能为0
			{
				return 0;
			}
			td_pos=pos;
			for (idx_col = 0;idx_col < count_col;idx_col++)
			{
				data_len=get_object_attribute_item_len(frame+td_pos,frame[td_pos]);
				if(idx_col==find_idx)
				{
					if(frame[td_pos]==DT_DATETIME_S)
					{
						mem_cpy(hold_time->value,frame+td_pos+1,7);
						if(compare_string(hold_oad,"\x20\x1E\x02\x00",4)==0)
						{
							// hold_time->value[4]=0;
							// hold_time->value[5]=0;
							// hold_time->value[6]=0;
						}
						break;
					}
					else
					{
						return 0;
					}
				}
				td_pos+=data_len;
			}
			//为日冻结减数。
			DayHoldFlag = (rcsd[(rcsd[0]-1)*5+2]<<8) + rcsd[(rcsd[0]-1)*5+3];
			if((DayHoldFlag == 0x5004)&&(gSystemInfo.dayHodeMode ==0x3))//日冻结类数据
			{
				datetime_minus_seconds(&(hold_time->value[1]),&(hold_time->value[2]),&(hold_time->value[3]),&(hold_time->value[4]),&(hold_time->value[5]),&(hold_time->value[6]),60);
			}
			for (idx=0;idx<rcsd[0];idx++)
			{
				oad_id = (rcsd[idx*5+2]<<8)+rcsd[idx*5+3];
				switch(oad_id)
				{
					case OBIS_COMM_ADDR: //电表地址
					case OBIS_SERVER_ADDR: //目标服务器地址
					if(resp_pos+sizeof(COSEM_TSA)+5<left_len)
					{
						resp[resp_pos++]=DT_TSA;
						resp[resp_pos++]=7;
						resp[resp_pos++]=5;
						#ifdef __PROVICE_SHANDONG__
						mem_cpy(resp+resp_pos,gSystemInfo.ertu_devid,6);
						#else
						mem_cpy_reverse(resp+resp_pos,gSystemInfo.meter_no,6);
						#endif
						resp_pos += 6;
					}
					else
					{
						*is_more_frame=TRUE;
					}
					break;
					case 0x6040: //采集启动时间
					//修改
					//if(DayHoldFlag == 0x5004)//日冻结类数据
					{
						//hold_time->value[5] = hold_time->value[5]-1;
						//datetime_minus_seconds(&(hold_time->value[1]),&(hold_time->value[2]),&(hold_time->value[3]),&(hold_time->value[4]),&(hold_time->value[5]),&(hold_time->value[6]),60);
						mem_cpy(resp+resp_pos,&(hold_time->tag),sizeof(COSEM_DATETIME_S));
						resp_pos+=sizeof(COSEM_DATETIME_S);
						break;
					}
					case 0x6042: //采集存储时标
					if(resp_pos+sizeof(COSEM_DATETIME_S)+5<left_len)
					{
						//if(DayHoldFlag == 0x5004)//日冻结类数据
						{
							//datetime_add_seconds(&(hold_time->value[1]),&(hold_time->value[2]),&(hold_time->value[3]),&(hold_time->value[4]),&(hold_time->value[5]),&(hold_time->value[6]),60);
						}
						mem_cpy(resp+resp_pos,&(hold_time->tag),sizeof(COSEM_DATETIME_S));
						resp_pos+=sizeof(COSEM_DATETIME_S);
					}
					else
					{
						*is_more_frame=TRUE;
					}
					break;
					case 0x6041: //采集到时间
					if(resp_pos+sizeof(COSEM_DATETIME_S)+5<left_len)
					{
						mem_cpy(resp+resp_pos,&(ok_time.tag),sizeof(COSEM_DATETIME_S));
						resp_pos+=sizeof(COSEM_DATETIME_S);
					}
					else
					{
						*is_more_frame=TRUE;
					}
					break;
					default:
					data_pos=0;
					resp[resp_pos++]=DT_ARRAY;
					resp[resp_pos++]=count;

					for (idx_col = 0;idx_col < count_col;idx_col++)
					{
						data_len=get_object_attribute_item_len(frame+pos,frame[pos]);
						if((!hold_flag)&&(idx_col==find_idx))
						{
							pos+=data_len;
							continue;
						}
						if(resp_pos+data_len+5<left_len)
						{
							if(*(frame+pos) == 0x1c)
							{
								mem_cpy(frame+pos,&(hold_time->tag),sizeof(COSEM_DATETIME_S)); //冻结时间这里要减1分钟，使用上面减好的时间。
							}
							mem_cpy(resp+resp_pos,frame+pos,data_len);
							resp_pos+=data_len;
							pos+=data_len;
						}
						else
						{
							*is_more_frame=TRUE;
							break;
						}
					}
					break;
				}
				if(*is_more_frame)
				{
					break;
				}
			}
		}
		else  //TODO可能是分帧，目前还不支持
		{
			return 0;
		}
	}
	else//链路层分帧，或报文格式不正确
	{

	}
	return resp_pos;
	*/
}
INT16U save_read_data_OOP(INT8U* frame,INT16U frame_len,objResponse *pResp,INT8U *rcsd,BOOLEAN hold_flag,INT8U *hold_oad,COSEM_DATETIME_S *hold_time,INT16U left_len,BOOLEAN *is_more_frame)
{
	OBJECT_ATTR_DESC *oad;
	INT8U *data,*resp,*master_oad;
	INT16U oad_id,pos,data_len,data_pos,csd_pos,td_pos;
	volatile INT16U resp_pos;
	INT8U count_col,idx_col,idx,count,find_idx;
    COSEM_DATETIME_S ok_time;
	BOOLEAN add_hold_time_oad,is_find;
    INT16U  DayHoldFlag;   //为测试日冻结设置的flag。
	add_hold_time_oad = FALSE;
    resp=pResp->frame+pResp->pos;    //resp为返回给主站的报文位置
    resp_pos=0;
    pos = get_frame_header_length_gb_oop(frame);   //frame原来请求电表回应帧
	data_len = 0;

	hold_time->tag = DT_DATETIME_S;
    ok_time.tag=DT_DATETIME_S;
    sys_datetime2cosem_datetime(ok_time.value);
	if (frame[pos++] == SERVER_APDU_GET)
	{
		if (frame[pos] == GET_RESPONSE_NORMAL_LIST) //APDU
		{
			pos += 2;  //apdu和piid
			count_col = frame[pos++];
            data=frame+pos;
            sys_datetime2cosem_datetime(hold_time->value);
            hold_time->value[4]=0;
            hold_time->value[5]=0;
            hold_time->value[6]=0;

            for (idx=0;idx<rcsd[0];idx++)
            {
                oad_id = (rcsd[idx*5+2]<<8)+rcsd[idx*5+3];
                switch(oad_id)
                {
                case OBIS_COMM_ADDR: //电表地址
                case OBIS_SERVER_ADDR: //目标服务器地址
                    resp[resp_pos++]=DT_TSA;
                    resp[resp_pos++]=7;
                    resp[resp_pos++]=5;
#ifdef __PROVICE_SHANDONG__
					mem_cpy(resp+resp_pos,gSystemInfo.ertu_devid,6);
#else				
                    mem_cpy_reverse(resp+resp_pos,gSystemInfo.meter_no,6);
#endif	
                    resp_pos += 6;
                    break;
                case 0x6040: //采集启动时间

                case 0x6042: //采集存储时标  //andu
                    mem_cpy(resp+resp_pos,&(hold_time->tag),sizeof(COSEM_DATETIME_S));
                    resp_pos+=sizeof(COSEM_DATETIME_S);
                    break;
                case 0x6041: //采集到时间
                    mem_cpy(resp+resp_pos,&(ok_time.tag),sizeof(COSEM_DATETIME_S));
                    resp_pos+=sizeof(COSEM_DATETIME_S);
                    break;
                default:
                    data_pos=0;
                    is_find=FALSE;
                    for (idx_col = 0;idx_col < count_col;idx_col++)
                    {
                        oad = (OBJECT_ATTR_DESC*)(data+data_pos);
#ifdef __OOP_GMETER_ERTU__
						if((oad->value[0]==0x46)&&(oad->value[1]==0x00)&&(oad->value[2]==0x03))
						{
							need_to_seperate =1;
							oad->value[3] =0x00;
						}
#endif						
                        data_pos += sizeof(OBJECT_ATTR_DESC);
                        if (compare_string(oad->value,rcsd+idx*5+2,sizeof(OBJECT_ATTR_DESC)) == 0)
                        {
                            is_find=TRUE;
                            if(data[data_pos++]==1)
                            {
                                data_len=get_object_attribute_item_len(data+data_pos,data[data_pos]);
                                mem_cpy(resp+resp_pos,data+data_pos,data_len);
                                resp_pos+=data_len;
                            }
                            else
                            {
                                resp[resp_pos++]=DT_NULL;
                            }
                            break;
                        }
                        else
                        {
                            if(data[data_pos++]==1)
                            {
                                data_len=get_object_attribute_item_len(data+data_pos,data[data_pos]);
                                data_pos+=data_len;
                            }
                            else
                            {
                                data_pos+=2;
                            }
                        }
                    }
                    if(!is_find)
                    {
                        resp[resp_pos++]=DT_NULL;
                    }
                    break;
                }
            }
		}
		else if ((frame[pos] == GET_RESPONSE_RECORD))
		{
			pos += 2;
            master_oad = frame+pos;
			pos += 4;
			count_col = frame[pos++];

			//比较列头
			idx = 0;
            csd_pos=0;
			for (idx=0;idx<rcsd[0];idx++)
			{
                if(rcsd[1+5*idx]==1)
                {
                    csd_pos=2+5*idx;
                    break;
                }
			}
            if(compare_string(rcsd+csd_pos,master_oad,4)!=0)
            {
                return 0;
            }
            csd_pos+=4;
            count=rcsd[csd_pos++];   //有多少个oad电表回的
            if((hold_flag)&&(count!=count_col)) return 0;
            if((!hold_flag)&&((count+1)!=count_col))    return 0;
            find_idx=0;
            for(idx_col=0;idx_col<count;idx_col++)
            {
                if(hold_flag)
                {
                    if(compare_string(frame+pos+idx_col*5+1,rcsd+csd_pos+idx_col*4,4)!=0)
                    {
                        return 0;
                    }
                    if(compare_string(frame+pos+idx_col*5+1,hold_oad,4)==0)
                    {
                        find_idx=idx_col;
                    }
                }
                else
                {
                    if(compare_string(frame+pos+(idx_col+1)*5+1,rcsd+csd_pos+idx_col*4,4)!=0)
                    {
                        return 0;
                    }
                }
            }
            pos+=5*count_col;
			//处理电表数据
			if (frame[pos++] != 1)   //是否有记录数据
			{
                return 0;
            }
            if (frame[pos++] == 0)   //记录数据条数不能为0
            {
                return 0;
            }
            td_pos=pos;
            for (idx_col = 0;idx_col < count_col;idx_col++)
            {
                data_len=get_object_attribute_item_len(frame+td_pos,frame[td_pos]);
                if(idx_col==find_idx)
                {
                    if(frame[td_pos]==DT_DATETIME_S)
                    {
                        mem_cpy(hold_time->value,frame+td_pos+1,7);
                        if(compare_string(hold_oad,"\x20\x1E\x02\x00",4)==0)
                        {
                           // hold_time->value[4]=0;
                           // hold_time->value[5]=0;
                           // hold_time->value[6]=0;
                        }
                        break;
                    }
                    else
                    {
                        return 0;
                    }
                }
                td_pos+=data_len;
            }
			//为日冻结减数。
			DayHoldFlag = (rcsd[(rcsd[0]-1)*5+2]<<8) + rcsd[(rcsd[0]-1)*5+3];
			if((DayHoldFlag == 0x5004)&&(gSystemInfo.dayHodeMode ==0x3))//日冻结类数据
			{
			datetime_minus_seconds(&(hold_time->value[1]),&(hold_time->value[2]),&(hold_time->value[3]),&(hold_time->value[4]),&(hold_time->value[5]),&(hold_time->value[6]),60);
			}
            for (idx=0;idx<rcsd[0];idx++)
            {
                oad_id = (rcsd[idx*5+2]<<8)+rcsd[idx*5+3];
                switch(oad_id)
                {
                case OBIS_COMM_ADDR: //电表地址
                case OBIS_SERVER_ADDR: //目标服务器地址
                    if(resp_pos+sizeof(COSEM_TSA)+5<left_len)
                    {
                        resp[resp_pos++]=DT_TSA;
                        resp[resp_pos++]=7;
                        resp[resp_pos++]=5;
#ifdef __PROVICE_SHANDONG__
						mem_cpy(resp+resp_pos,gSystemInfo.ertu_devid,6);
#else
						mem_cpy_reverse(resp+resp_pos,gSystemInfo.meter_no,6);
#endif
                        resp_pos += 6;
                    }
                    else
                    {
                        *is_more_frame=TRUE;
                    }
                    break;
                case 0x6040: //采集启动时间
				    //修改
					//if(DayHoldFlag == 0x5004)//日冻结类数据
					{
				    //hold_time->value[5] = hold_time->value[5]-1;
					//datetime_minus_seconds(&(hold_time->value[1]),&(hold_time->value[2]),&(hold_time->value[3]),&(hold_time->value[4]),&(hold_time->value[5]),&(hold_time->value[6]),60);
				    mem_cpy(resp+resp_pos,&(hold_time->tag),sizeof(COSEM_DATETIME_S));
				    resp_pos+=sizeof(COSEM_DATETIME_S);
				    break;				
					}			
                case 0x6042: //采集存储时标
                    if(resp_pos+sizeof(COSEM_DATETIME_S)+5<left_len)
                    {
						//if(DayHoldFlag == 0x5004)//日冻结类数据
						{
							//datetime_add_seconds(&(hold_time->value[1]),&(hold_time->value[2]),&(hold_time->value[3]),&(hold_time->value[4]),&(hold_time->value[5]),&(hold_time->value[6]),60);
						}
						mem_cpy(resp+resp_pos,&(hold_time->tag),sizeof(COSEM_DATETIME_S));
                        resp_pos+=sizeof(COSEM_DATETIME_S);
                    }
                    else
                    {
                        *is_more_frame=TRUE;
                    }
                    break;
                case 0x6041: //采集到时间
                    if(resp_pos+sizeof(COSEM_DATETIME_S)+5<left_len)
                    {
                        mem_cpy(resp+resp_pos,&(ok_time.tag),sizeof(COSEM_DATETIME_S));
                        resp_pos+=sizeof(COSEM_DATETIME_S);
                    }
                    else
                    {
                        *is_more_frame=TRUE;
                    }
                    break;
                default:
                    data_pos=0;
                    resp[resp_pos++]=DT_ARRAY;
                    resp[resp_pos++]=count;

                    for (idx_col = 0;idx_col < count_col;idx_col++)
                    {
                        data_len=get_object_attribute_item_len(frame+pos,frame[pos]);
                        if((!hold_flag)&&(idx_col==find_idx))
                        {
                            pos+=data_len;
                            continue;
                        }
                        if(resp_pos+data_len+5<left_len)
                        {
							if(*(frame+pos) == 0x1c)
							{
								mem_cpy(frame+pos,&(hold_time->tag),sizeof(COSEM_DATETIME_S)); //冻结时间这里要减1分钟，使用上面减好的时间。
							}
                            mem_cpy(resp+resp_pos,frame+pos,data_len);
                            resp_pos+=data_len;
                            pos+=data_len;
                        }
                        else
                        {
                            *is_more_frame=TRUE;
                            break;
                        }
                    }
                    break;
                }
                if(*is_more_frame)
                {
                    break;
                }
            }
		}
		else  //TODO可能是分帧，目前还不支持
		{
            return 0;
		}
	}
	else//链路层分帧，或报文格式不正确
	{

	}
	return resp_pos;
}
//调整下一下循环开始时间。
INT8U adjust_next_begin_time(INT8U *time_begin,RSD *rsd,INT8U *rcsd,INT8U *end_time,INT8U *end_flag)
{
	INT8U idx,csd_cnt,road_count,oad_count,col_idx,col_cnt,choice_flag=0;
	INT8U master_oad[4]={0},buffer[50]={0},ti[4]={0};
	INT8U *col_cnt_pos;
	BOOLEAN is_event=FALSE;
	INT16U obis,pos,frame_pos;


	transIndex =0;  //因为是全局变量，所以要清一次0
	pos=0;csd_cnt=rcsd[pos++];
	road_count=0;
	oad_count=0;
	frame_pos=0;
	for(idx=0;idx<csd_cnt;idx++)
	{
		if(rcsd[pos++]==0)
		{
			obis=cosem_bin2_int16u(rcsd+pos);
			switch(obis)
			{
				case OBIS_COMM_ADDR:
				case OBIS_SERVER_ADDR:
				case OBIS_ACQ_BEGIN_TD:
				case OBIS_ACQ_END_TD:
				case OBIS_ACQ_SAVE_TD:
				pos+=4;
				break;
				default:

#ifdef __PROVINCE_JIBEI_FUHEGANZHI__
				if((*(rcsd+pos)==0x46) && (*(rcsd+pos +1)==0x00) )
				{
					ti[0]=DT_TI;
					ti[1]=1;
					ti[2]=rsd->choice_6_7_8.time_interval[1];
					ti[3]=rsd->choice_6_7_8.time_interval[2];	
				}
#endif		
				oad_count++;
				//mem_cpy(frame+frame_pos+1,rcsd+pos,4);
				//frame[0]=oad_count;
				frame_pos+=4;
				pos+=4;				
						
				break;
			}
		}
		else
		{
			road_count++;
			mem_cpy(master_oad,rcsd+pos,4);			
			obis=cosem_bin2_int16u(master_oad);
	
			switch(obis)
			{
				case OBIS_MINUTE_HOLD:
				ti[0]=DT_TI;
				ti[1]=1;
				ti[2]=rsd->choice_6_7_8.time_interval[1];
				ti[3]=rsd->choice_6_7_8.time_interval[2];
				break;
				case OBIS_HOUR_HOLD:
				ti[0]=DT_TI;
				ti[1]=2;
				ti[2]=0;
				ti[3]=1;
				break;
				case OBIS_DAY_HOLD:
				ti[0]=DT_TI;
				ti[1]=3;
				ti[2]=0;
				ti[3]=1;
				break;
				case OBIS_MONTH_HOLD:
				ti[0]=DT_TI;
				ti[1]=4;
				ti[2]=0;
				ti[3]=1;
				break;
				default:
				if((obis>=0x3000)&&(obis<0x3030))
				{
					is_event=TRUE;
				}
				else
				{
					return 0;
				}
				break;
			}
		}
	}
	cosem_datetime_add_ti(time_begin,ti+1);
	if(compare_string(time_begin,end_time,7)>=0)
    {
        *end_flag=TRUE;
    }
	return 1;

}
void change_day_hold_serach_time(INT8U *time,INT8U flag)
{
	INT8U idx;
	INT8U oad[4];
	for(idx=0;idx<MAX_TASK_COUNT;idx++)
	{
		
		if(compare_string(gTaskPlanAttribute[idx].master_oad,"\x50\x04\x02\x00",4)==0) //日冻结
		{
			switch(gTaskPlanAttribute[idx].store_time_flag)
			{
				case 3:      //相对于上日23点59分
					if(flag ==1)
					{
						datetime_add_seconds(time+1,time+2,time+3,time+4,time+5,time+6,60);
					}
					else if(flag ==2)
					{
						datetime_minus_seconds(time+1,time+2,time+3,time+4,time+5,time+6,60);
					}					
					break;
			}
		}
	}
}
#ifdef __PROVINCE_JIBEI_FUHEGANZHI__
	INT8U loadData[1200];
#endif 
INT8U read_acq_plan_data_from_flash(objRequest *pRequest,objResponse *pResp,RSD *rsd,INT8U *rcsd,INT16U left_len,SEPARATE_FRAME *separate_frame)
{
	INT8U *req,*resp,*data;
	INT8U data_type,count,last_count=0,tmp_count;
	INT8U time_begin[7]={0},time_end[7]={0},tmp_td[7]={0},hold_oad[4]={0};
	volatile INT16U req_len,resp_len,resp_pos;
	BOOLEAN end_flag=FALSE,hold_flag=FALSE,is_more_frame=FALSE;
	COSEM_DATETIME_S hold_time;
	INT32U *master_oad =NULL,*oad = NULL;  //这些oad指针都指向了rcsd序列
	INT8U oad_cnt=0,rcsd_cnt=0,rcsd_find_idx =0,rcsd_idx =0,oad_idx = 0;
	INT8U DayHoldFlag = 0;  //暂时存储日冻结标志位
	INT8U task_start_time_frq[7]; //任务周期启动时间。
	INT8U is_cur_data_req = 0;  //是请求实时数据
	if(left_len<300)
	{
		return 0;
	}
#ifdef __PROVINCE_JIBEI_FUHEGANZHI__
		INT8U tmp_master_oad[4] ={0x50,0x02,0x02,0x00};
#endif	
	frame_receive++; //采集监控报文收发统计
	left_len-=300;  //原来是300
	req=pRequest->frame+sizeof(pRequest->frame)-800; //原来是300
	resp=pResp->frame+sizeof(pResp->frame)-300;  //原来是是300
#ifdef __PROVINCE_JIBEI_FUHEGANZHI__
	resp = loadData;
#endif 
	count=0;
	switch(rsd->choice)
	{
		case 4:
		case 5:
		//DayHoldFlag = (rcsd[(rcsd[0]-1)*5+2]<<8) + rcsd[(rcsd[0]-1)*5+3];
		//if((DayHoldFlag == 0x5004)&&(gSystemInfo.dayHodeMode ==0x3))
		//{
			//datetime_add_seconds(rsd->choice_4_5.acq_time+1,rsd->choice_4_5.acq_time+2,rsd->choice_4_5.acq_time+3,rsd->choice_4_5.acq_time+4,rsd->choice_4_5.acq_time+5,rsd->choice_4_5.acq_time+6,60);
		//}
		change_day_hold_serach_time(rsd->choice_4_5.acq_time,1);
		end_flag = true;
		DayHoldFlag = 1;
		mem_cpy(time_begin,rsd->choice_4_5.acq_time,7);
		last_count=10;
		break;
		case 6:
		case 7:
		case 8:
		if(separate_frame_mem.frame_id==0)
		{
			mem_cpy(time_begin,rsd->choice_6_7_8.acq_time_begin,7);  //开始时间
			last_count=10;
		}
		else
		{
			mem_cpy(time_begin,separate_frame_mem.param.a.start_td,7);
		}
		mem_cpy(time_end,rsd->choice_6_7_8.acq_time_end,7);   //结束时间
		break;
		case 10:
		if(separate_frame_mem.frame_id==0)
		{
			last_count=rsd->choice_10.last_count;
		}
		else
		{
			last_count=separate_frame_mem.param.a.last_count;
		}
		data_type =2;
		break;
		default:
		return 0;
	}
	//这段代码用来修改masteroad 和oad指针指向 ,要注意这里oad顺序是相反的，内存中是50020200的。
	{
		rcsd_find_idx =0;
		rcsd_cnt = rcsd[rcsd_find_idx++];
		for(rcsd_idx =0;rcsd_idx<rcsd_cnt;rcsd_idx++)
		{
			if(rcsd[rcsd_find_idx] == 0x01)
			{
				master_oad =rcsd +rcsd_find_idx+1;
				rcsd_find_idx+=5;		
				oad_cnt = rcsd[rcsd_find_idx++];
				oad =rcsd + rcsd_find_idx;
				break;
			}
			else
			{
				if(compare_string(rcsd+rcsd_find_idx,"\x00\x40\x01\x02\00",5)==0||compare_string(rcsd+rcsd_find_idx,"\x00\x60\x40\x02\00",5)==0||compare_string(rcsd+rcsd_find_idx,"\x00\x20\x2A\x02\00",5)==0||
					compare_string(rcsd+rcsd_find_idx,"\x00\x60\x41\x02\00",5)==0||compare_string(rcsd+rcsd_find_idx,"\x00\x60\x42\x02\00",5)==0)
				{
					rcsd_find_idx+=5;
				}
				else
				{
#ifdef __PROVINCE_JIBEI_FUHEGANZHI__
					if(*(rcsd+rcsd_find_idx+1)==0x46)  //非侵入式用户感知用46开头。
					{
						master_oad = tmp_master_oad;
					}
#endif					

					oad_cnt = rcsd_cnt -rcsd_idx;
					oad =rcsd + rcsd_find_idx;
					break;
				}
			}
		}

	}
	while(true)
	{
		mem_cpy(tmp_td,time_begin,7);
		tmp_count=last_count;
//		req_len=make_read_oop_data_frame(req,rsd,time_begin,time_end,&last_count,rcsd,&data_type,&end_flag,&hold_flag,hold_oad);
		req_len = adjust_next_begin_time(time_begin,rsd,rcsd,time_end,&end_flag);  //改变了下次要读取点的时间。
		if(req_len>0)
		{
//			resp_len=app_trans_send_meter_frame(req,req_len,resp,800,5000);  //原来是300；
			//这里用来检索数据，也把数据放到resp这个缓冲区里去。
			resp_len =make_meter_data_from_flash(master_oad,oad,oad_cnt,tmp_td,resp,task_start_time_frq,&DayHoldFlag,&data_type,&is_cur_data_req);  
			if(resp_len>0)
			{
//				resp_pos=save_read_data_OOP(resp,resp_len,pResp,rcsd,hold_flag,hold_oad,&hold_time,left_len,&is_more_frame);
				resp_pos=save_read_data_OOP_from_flash(resp,resp_len,pResp,rcsd,hold_flag,hold_oad,&hold_time,left_len,&is_more_frame,tmp_td,&task_start_time_frq,&DayHoldFlag);
				mem_cpy(hold_time.value,time_begin,7);
				if(is_more_frame)
				{
					separate_frame->more_frame=TRUE;
					separate_frame->frame_id++;
					separate_frame->param.a.choice=rsd->choice;
					separate_frame->param.a.last_count=tmp_count;
					mem_cpy(separate_frame->param.a.start_td,tmp_td,7);
					return count;
				}
				if(resp_pos>0)
				{
					if(data_type==2)
					{
						if((rsd->choice==4)||(rsd->choice==5))
						{
							if(compare_string(time_begin,hold_time.value,7)!=0)
							{
								if(end_flag)
								{
									return count;
								}
								continue;
							}
							else
							{
								pResp->pos+=resp_pos;
								count++;
								return count;
							}
						}
						if((rsd->choice==6)||(rsd->choice==7)||(rsd->choice==8))
						{
							if(compare_string(time_begin,hold_time.value,7)>0)
							{
								if(end_flag)
								{
									return count;
								}
								continue;
							}
							if(compare_string(time_end,hold_time.value,7)<=0)
							{
								pResp->pos+=resp_pos;
								count++;
								return count;
							}
							if(is_cur_data_req ==1)  //rsd通过方法5,6,7 来请求实时数据。
							{
								pResp->pos+=resp_pos;
								count++;
								return count;
							}
						}
						if((rsd->choice==10))
						{
							pResp->pos+=resp_pos;
							count++;
							return count;
						}
					}
					pResp->pos+=resp_pos;
					left_len-=resp_pos;
					count++;
				}
				else
				{
				   //break;   //以前是注掉的
				}
			}
		}
		else
		{
			break;
		}
		if(end_flag)
		{
			return count;
		}
	}
	return 0;
}

INT8U read_acq_plan_data_from_meter(objRequest *pRequest,objResponse *pResp,RSD *rsd,INT8U *rcsd,INT16U left_len,SEPARATE_FRAME *separate_frame)
{
    INT8U *req,*resp,*data;
    INT8U data_type,count,last_count=0,tmp_count;
    INT8U time_begin[7]={0},time_end[7]={0},tmp_td[7]={0},hold_oad[4]={0};
    volatile INT16U req_len,resp_len,resp_pos;
    BOOLEAN end_flag=FALSE,hold_flag=FALSE,is_more_frame=FALSE;
    COSEM_DATETIME_S hold_time;
	INT16U DayHoldFlag;  //暂时存储日冻结标志位
#ifdef __OOP_GMETER_ERTU__ 	     //电表部非侵入式终端
	INT8U meter_cnt =0,meter_index=0,frame_no =0;
	INT8U seprate_link=0;
#endif	
    if(left_len<300)
    {
        return 0;
    }
	frame_receive++; //采集监控报文收发统计
    left_len-=800;  //原来是300
    req=pRequest->frame+sizeof(pRequest->frame)-800; //原来是300
    resp=pResp->frame+sizeof(pResp->frame)-800;  //原来是是300
    count=0;
    switch(rsd->choice)
    {
    case 4:
    case 5:
		DayHoldFlag = (rcsd[(rcsd[0]-1)*5+2]<<8) + rcsd[(rcsd[0]-1)*5+3];
		if((DayHoldFlag == 0x5004)&&(gSystemInfo.dayHodeMode ==0x3))
		{
			datetime_add_seconds(rsd->choice_4_5.acq_time+1,rsd->choice_4_5.acq_time+2,rsd->choice_4_5.acq_time+3,rsd->choice_4_5.acq_time+4,rsd->choice_4_5.acq_time+5,rsd->choice_4_5.acq_time+6,60);
		}
        mem_cpy(time_begin,rsd->choice_4_5.acq_time,7);
        last_count=10;
        break;
    case 6:
    case 7:
    case 8:
        if(separate_frame_mem.frame_id==0)
        {
            mem_cpy(time_begin,rsd->choice_6_7_8.acq_time_begin,7);
            last_count=10;
#ifdef __OOP_GMETER_ERTU__ 	     //read_meter_MS_set
		  meter_cnt =rsd->choice_6_7_8.meter_set.count;
		  mem_set(read_meter_MS_set,72,0x00);
		  mem_set(read_meter_MS_set_mask,12,0x00);
		  for(meter_index=0;meter_index<meter_cnt;meter_index++)
		  {
			  mem_cpy(&read_meter_MS_set[meter_index*6],((INT8U*)&(rsd->choice_6_7_8.meter_set.choice))+2+2+(meter_index*2),6);
			  read_meter_MS_set_mask[meter_index] =0x01;
		  }
#endif			
        }
        else
        {
            mem_cpy(time_begin,separate_frame_mem.param.a.start_td,7);
        }
        mem_cpy(time_end,rsd->choice_6_7_8.acq_time_end,7);

        break;
    case 10:
        if(separate_frame_mem.frame_id==0)
        {
            last_count=rsd->choice_10.last_count;
        }
        else
        {
            last_count=separate_frame_mem.param.a.last_count;
        }
        break;
    default:
        return 0;
    }
    while(true)
    {
        mem_cpy(tmp_td,time_begin,7);
        tmp_count=last_count;
        req_len=make_read_oop_data_frame(req,rsd,time_begin,time_end,&last_count,rcsd,&data_type,&end_flag,&hold_flag,hold_oad);
        if(req_len>0)
        {
            resp_len=app_trans_send_meter_frame(req,req_len,resp,800,8000);  //原来是300；
#ifdef __OOP_GMETER_ERTU__ 	  //分帧处理
			seprate_frame_len =0;
			mem_set(seprate_frame_array,1000,0x00);
			if(resp_len >0)
			{
				while((seprate_link=is_link_seperate(resp,&resp_len,&frame_no))&&(seprate_frame_len<800))
				{
					req_len = make_read_ertu_seprate_frame(req,&frame_no);
					if(req_len>0)
					{
						 resp_len =app_trans_send_meter_frame(req,req_len,resp,800,500);
					}
				}	
			}
	
#endif			
            if(resp_len>0)
            {
                resp_pos=save_read_data_OOP(resp,resp_len,pResp,rcsd,hold_flag,hold_oad,&hold_time,left_len,&is_more_frame);
                if(is_more_frame)
                {
                    separate_frame->more_frame=TRUE;
                    separate_frame->frame_id++;
                    separate_frame->param.a.choice=rsd->choice;
                    separate_frame->param.a.last_count=tmp_count;
                    mem_cpy(separate_frame->param.a.start_td,tmp_td,7);
                    return count;
                }
                if(resp_pos>0)
                {
                    if(data_type==2)
                    {
                        if((rsd->choice==4)||(rsd->choice==5))
                        {
                            if(compare_string(time_begin,hold_time.value,7)!=0)
                            {
                                if(end_flag)
                                {
                                    return count;
                                }
                                continue;
                            }
                            else
                            {
                                pResp->pos+=resp_pos;
                                count++;
                                return count;
                            }
                        }
                        if((rsd->choice==6)||(rsd->choice==7)||(rsd->choice==8))
                        {
                            if(compare_string(time_begin,hold_time.value,7)>0)
                            {
                                if(end_flag)
                                {
                                    return count;
                                }
                                continue;
                            }
                            if(compare_string(time_end,hold_time.value,7)<=0)
                            {
                                return count;
                            }
                        }
                    }
                    pResp->pos+=resp_pos;
                    left_len-=resp_pos;
                    count++;
#ifdef __OOP_GMETER_ERTU__
				if(need_to_seperate ==1)
				{
					separate_frame->more_frame=TRUE;
					separate_frame->frame_id++;
					separate_frame->param.a.choice=rsd->choice;
					separate_frame->param.a.last_count=tmp_count;
					mem_cpy(separate_frame->param.a.start_td,tmp_td,7);
					return count;
				}		
#endif						
                }
                else
                {
                    //break;
                }
            }
        }
        else
        {
            break;
        }
        if(end_flag)
        {
            return count;
        }
    }
    return 0;
}

INT8U process_prepare_separate_frame_info(objRequest *pRequest,objResponse *pResp,SEPARATE_FRAME *separate_frame,INT16U frame_id)
{
    void app_prepare_request(objRequest *pRequest);
    INT8U result;
    
    result=DAR_SECCESS;
    if(separate_frame->apdu_service_code==GET_REQUEST_RECORD)
    {
        pResp->frame[pResp->pos++] = 2;         //记录型对象属性
        pResp->frame[pResp->pos++]=1;           //对象个数
    }
    else if(separate_frame->apdu_service_code==GET_REQUEST_NORMAL)
    {
        pResp->frame[pResp->pos++] = 1;         //对象属性
        pResp->frame[pResp->pos++]=1;           //对象个数
    }
    else if(separate_frame->apdu_service_code==GET_REQUEST_NORMAL_LIST)
    {
        pResp->frame[pResp->pos++] = 1;         //对象属性
        pResp->frame[pResp->pos++]=0;           //对象个数
    }
    else
    {
        result=DAR_OTHER_REASON;
        return result;
    }

    if(separate_frame->more_frame)
    {
        if (separate_frame->frame_id  == frame_id)
        {
            separate_frame->more_frame = FALSE;
            mem_cpy(pRequest->frame,separate_frame->frame,separate_frame->frame_len);
            app_prepare_request(pRequest);
            (void)check_timetag(pRequest);
            pRequest->pos=separate_frame->pos;
        }
        else
        {			
            result=DAR_LONG_GET_ABORTED;
        }
    }
    else
    {
        result=DAR_NO_LONG_GET_IN_PROGRESS;
    }
    return result;
}

void process_get_meter_attribute(objRequest *pRequest,objResponse *pResp)
{
    INT8U check_frame_header_gb_oop(INT8U* frame);
    INT8U *req;
    INT8U *resp,*oad_list,*server_addr;
    INT8U server_type,server_addr_len,idx,oad_count;
    INT8U meter_no[6]={0};
    INT16U max_resp_len,pos,pos_hcs,pos_count,pos_def,user_pos,err_pos;
    INT16U len,count;
    INT32U time_out;

    req=pRequest->frame+pRequest->frame_len;
    pos=0;

    //1.服务器地址处理
    server_addr_len = pRequest->frame[pRequest->pos]-1;
    server_type=pRequest->frame[pRequest->pos+1];
    server_addr=pRequest->frame+pRequest->pos+2;
    //2.根据主站报文，构造电表报文      
    req[pos++]=0x68;
    req[pos++]=0x00;//长度
    req[pos++]=0x00;//长度
    req[pos++]=0x43;//控制字
    req[pos++]=server_type;
    mem_cpy_reverse(req+pos,server_addr,server_addr_len);
    mem_cpy(pResp->frame+pResp->pos,pRequest->frame+pRequest->pos,server_addr_len+2);//应答服务器地址
    pResp->pos+=server_addr_len+2;
    pRequest->pos+=server_addr_len+2;
    pos+=server_addr_len;
    req[pos++]=0x00;//客户端地址
    pos_hcs= pos;
    pos+=2;

    time_out=(pRequest->frame[pRequest->pos]<<8)+pRequest->frame[pRequest->pos+1];
    time_out*=100;
    //超时时间
    pRequest->pos+=2;
    count=pRequest->frame[pRequest->pos++];
    oad_list = pRequest->frame+pRequest->pos;
    oad_count = count;
    req[pos++]=CLIENT_APDU_GET;
    req[pos++]=GET_REQUEST_NORMAL_LIST;
    req[pos++]= pRequest->frame[pRequest->userdata_pos+2];//PIID
    pos_count=pos;
    req[pos++]=0;//个数
    pos_def=pResp->pos;
    pResp->frame[pos_def++]=count;
    for(idx=0;idx<count;idx++)
    {
        //OAD
        mem_cpy(req+pos,pRequest->frame+pRequest->pos,4);
        //默认填充应答结果
        mem_cpy(pResp->frame+pos_def,pRequest->frame+pRequest->pos,4);//应答OAD及结果
        pos_def+=4;
        pResp->frame[pos_def++]=0;//无结果
        err_pos = pos_def;
        pResp->frame[pos_def++]=DAR_TIME_OUT;
        pRequest->pos+=4;
        pos+=4;
        req[pos_count]++;
    }

    req[pos++] = 0;//时间标签
    len = pos + 2 - 1;

    req[OOP_POS_LEN] = len;
    req[OOP_POS_LEN+1] = len >> 8;
    //计算HCS
    fcs16(req+OOP_POS_LEN,pos_hcs-1);
    //计算FCS
    fcs16(req+OOP_POS_LEN,pos-1);
    pos += 2;
    req[pos++] = 0x16;

    //3.使用表计监控转发电表报文
    //检查表计地址是否正确
    mem_cpy_reverse(meter_no,server_addr,6);
    if((server_addr_len==6)&&(compare_string(meter_no,gSystemInfo.meter_no,6)==0))
    {
        resp=pResp->frame+pos_def;
        max_resp_len=SIZE_OF_APP_PROTOCOL_FRAME-pos_def;            
        len=app_trans_send_meter_frame(req,pos,resp,max_resp_len,time_out);
        
        if((len>0) && check_frame_header_gb_oop(resp))
        {
            //解析响应报文，获取DAR
            resp=trim_oopframe_FE(resp);
            user_pos=get_frame_header_length_gb_oop(resp);
            count=resp[user_pos+3];
            pResp->frame[pResp->pos++]=count;
            user_pos+=4;
            for(idx=0;idx<count;idx++)
            {
                mem_cpy(pResp->frame+pResp->pos,resp+user_pos,5);//应答OAD
                user_pos+=5;
                pResp->pos+=5;
                if(resp[user_pos-1]==1)
                {
                    len=get_object_attribute_item_len(resp+user_pos,resp[user_pos]);
                    if(len>0)
                    {
                        mem_cpy(pResp->frame+pResp->pos,resp+user_pos,len);//应答DATA
                    }
                    user_pos+=len;
                    pResp->pos+=len;
                }
                else
                {
                    pResp->frame[pResp->pos++]=resp[user_pos++];
                }
            }
        }
        else
        {
            pResp->pos=pos_def;
        }
    }
    else
    {
        for(idx=0;idx<oad_count;idx++)
        {
            pResp->frame[err_pos-idx*6] = DAR_ADDRESS_EXCEPT;
        }
        pResp->pos=pos_def;
    }    
}

void process_get_meter_record(objRequest *pRequest,objResponse *pResp)
{
    INT8U check_frame_header_gb_oop(INT8U* frame);
    INT8U *req,*resp,*server_addr;
    INT8U server_type,server_addr_len,idx;
    INT8U meter_no[6];
    INT16U max_resp_len,pos,pos_hcs,pos_def,user_pos;
    INT16U len,count,col,col_count,err_pos,result_pos;
    INT32U time_out;

    req=pRequest->frame+pRequest->frame_len;
    pos=0;
    mem_set(meter_no,16,0x00);
    
    //1.代理请求的超时时间  long-unsigned
    time_out=(pRequest->frame[pRequest->pos]<<8)+pRequest->frame[pRequest->pos+1];
    time_out*=100;
    //超时时间
    pRequest->pos+=2;

    //2.服务器地址处理
    server_addr_len = pRequest->frame[pRequest->pos]-1;
    server_type=pRequest->frame[pRequest->pos+1];
    server_addr=pRequest->frame+pRequest->pos+2;

    //3.根据主站报文，构造电表报文
    req[pos++]=0x68;
    req[pos++]=0x00;//长度
    req[pos++]=0x00;//长度
    req[pos++]=0x43;//控制字
    req[pos++]=server_type;
    mem_cpy(req+pos,meter_no,server_addr_len);
    mem_cpy(pResp->frame+pResp->pos,pRequest->frame+pRequest->pos,server_addr_len+2);//应答服务器地址
    pResp->pos+=server_addr_len+2;
    pRequest->pos+=server_addr_len+2;
    pos+=server_addr_len;
    req[pos++]=0x00;//客户端地址
    pos_hcs= pos;
    pos+=2;
    //APDU部分
    req[pos++]=CLIENT_APDU_GET;
    req[pos++]=GET_REQUEST_RECORD;
    req[pos++]= pRequest->frame[pRequest->userdata_pos+2];//PIID
    pos_def=pResp->pos;
    result_pos = pResp->pos;
    //4.对象属性描述符      OAD
    mem_cpy(req+pos,pRequest->frame+pRequest->pos,4);
    mem_cpy(pResp->frame+pos_def,pRequest->frame+pRequest->pos,4);//默认填充应答OAD及结果
    pRequest->pos+=4;
    pos+=4;
    pos_def+=4;

    //5.RSD
    len=get_oop_data_type_len(pRequest->frame+pRequest->pos,DT_RSD);
    mem_cpy(req+pos,pRequest->frame+pRequest->pos,len);
    pos+=len;
    pRequest->pos+=len;

    //6.RCSD
    count=pRequest->frame[pRequest->pos];
    req[pos++]=pRequest->frame[pRequest->pos];
    pResp->frame[pos_def++]=pRequest->frame[pRequest->pos++];
    for(idx=0;idx<count;idx++)
    {
        len=get_oop_data_type_len(pRequest->frame+pRequest->pos,DT_CSD);
        mem_cpy(req+pos,pRequest->frame+pRequest->pos,len);//填充请求CSD
        mem_cpy(pResp->frame+pos_def,pRequest->frame+pRequest->pos,len);//默认填充RCSD
        pRequest->pos+=len;
        pos+=len;
        pos_def+=len;
    }
    //填充默认结果
    pResp->frame[pos_def++]=0;
    err_pos = pos_def;
    pResp->frame[pos_def++]=DAR_TIME_OUT;

    req[pos++] = 0;//时间标签
    len = pos + 2 - 1;

    req[OOP_POS_LEN] = len;
    req[OOP_POS_LEN+1] = len >> 8;
    //计算HCS
    fcs16(req+OOP_POS_LEN,pos_hcs-1);
    //计算FCS
    fcs16(req+OOP_POS_LEN,pos-1);
    pos += 2;
    req[pos++] = 0x16;

    //3.使用表计监控转发电表报文

    //检查表计地址是否正确
    mem_cpy_reverse(meter_no,server_addr,6);
    if((server_addr_len==6)&&(compare_string(server_addr,meter_no,6)==0))
    {
        resp=pResp->frame+pos_def;
        max_resp_len=SIZE_OF_APP_PROTOCOL_FRAME-pos_def;

        len=app_trans_send_meter_frame(req,pos,resp,max_resp_len,time_out);
        
        if((len>0) && check_frame_header_gb_oop(resp))
        {
            //解析响应报文，获取DAR
            resp=trim_oopframe_FE(resp);
            user_pos=get_frame_header_length_gb_oop(resp);
            user_pos+=3;
            mem_cpy(pResp->frame+result_pos,resp+user_pos,4);//应答OAD
            user_pos+=4;
            result_pos+=4;
            
            col_count=resp[user_pos];
            if(col_count == 0)//说明回复报文存在异常，电表老版本存在此问题，这里做兼容修正
            {
                user_pos++;
                pResp->frame[err_pos] = resp[user_pos];
                pResp->pos=pos_def;
                return;
            }
            pResp->frame[result_pos++]=resp[user_pos++];
            for(col=0;col<col_count;col++)
            {
                len=get_oop_data_type_len(resp+user_pos,DT_CSD);
                mem_cpy(pResp->frame+result_pos,resp+user_pos,len);//填充CSD
                result_pos+=len;
                user_pos+=len;
            }
            if(resp[user_pos] == 0)
            {
                pResp->frame[result_pos++] = resp[user_pos++];
                pResp->frame[result_pos++] = resp[user_pos++];
                pResp->pos=result_pos;
            }
            else
            {
                pResp->frame[result_pos++]=resp[user_pos++];
                count=resp[user_pos++];
                pResp->frame[result_pos++]=count;
                for(idx=0;idx<count;idx++)
                {
                    for(col=0;col<col_count;col++)
                    {
                        len=get_object_attribute_item_len(resp+user_pos,resp[user_pos]);
                            mem_cpy(pResp->frame+result_pos,resp+user_pos,len);//填data
                        user_pos+=len;
                            result_pos+=len;
                    }
                }
                pResp->pos=result_pos;
            }
        }
        else
        {
            pResp->pos=pos_def;
        }
    }
    else
    {
        pResp->frame[err_pos] = DAR_ADDRESS_EXCEPT;
        pResp->pos=pos_def;
    }

}
