#include "readmeter_oop_data.h"
#include "../main_include.h"

//#define __DEBUG_
//BOOLEAN oad_search_in_hold_list( INT8U oad[4],tagREADITEM *read_item)
//{
	//INT8U  idx_list;
	//INT16U sub_idx;
	//INT16U sub_cnt;
	//tagREADITEM *item; 
	//for(idx_list=0;idx_list<sizeof(hold_data_list)/sizeof(READ_LIST_PROP);idx_list++)
	//{
		//sub_cnt = hold_data_list[idx_list].read_item_count;
		//item    = hold_data_list[idx_list].read_item_list;
		//for(sub_idx=0;sub_idx<sub_cnt;sub_idx++)
		//{
			////
			//if(item[sub_idx].oad == bin2_int32u(oad))
			//{
				////
				////*page_id = item[sub_idx].page_id;
				//mem_cpy((void *)read_item,(void *)&item[sub_idx],sizeof(tagREADITEM));
				//return TRUE;
			//}
		//}
	//}
//
	//return FALSE;
//}
extern BOOLEAN oad_search_in_hold_list(INT8U list_idx, INT8U oad[4], tagREADITEM *read_item);
extern BOOLEAN oad_search_in_cur_list(INT8U list_idx,INT8U oad[4],tagREADITEM *read_item);
BOOLEAN oad_search_in_list(INT8U type, INT8U list_idx,INT8U oad[4],tagREADITEM *read_item)
{
	//INT8U type;
	switch(type)
	{
		case READ_TYPE_HOLD://冻结
			return oad_search_in_hold_list(list_idx,oad,read_item);
		case READ_TYPE_CUR:
			/*  */
			return oad_search_in_cur_list(list_idx,oad,read_item);
			break;
		default:
			return FALSE;
	}
	return FALSE;
}

void clr_process_mask(READ_PARAMS *read_param)
{
	// 
	INT8U idx;
	INT8U tsk_idx = read_param->idx_tsk_attr;
	for(idx = 0;idx<read_param->norml_task_save_params.group_oad_params.oad_count;idx++)
	{
		clr_bit_value(gTaskPlanAttribute[tsk_idx].procs_mask, 8, 
					read_param->norml_task_save_params.group_oad_params.mask_idx[idx]);
	}
}
void save_grp_read_oad_params(GROUP_OAD_PARAMS *grp_oad_params,tagREADITEM read_item,INT8U mask_idx)
{
	INT8U pos = 0;

	/* test */
	INT32U master = 0x50040200;
	
	pos = grp_oad_params->oad_count;
	if(!grp_oad_params->oad_count)
	{
		// 存一次OAD ??
		//mem_cpy(grp_oad_params->master_oad.object_id,(void *)&master,4);
		grp_oad_params->page_cnt  = read_item.page_count;
		grp_oad_params->page_id   = read_item.page_id;
		grp_oad_params->save_size = read_item.block_data_len;
	}
	/* OAD 存储偏移  预期应答长度 
	 *掩码，用于存储的时候使用?? 
	 */
	mem_cpy_reverse(grp_oad_params->oad[pos],(void *)&read_item.oad, 4);
	mem_cpy(grp_oad_params->save_off[pos],(void *)&read_item.offset,2);
	set_bit_value(grp_oad_params->mask, 4, grp_oad_params->oad_count);
	/* 在 plan中的 mask 索引 */
	grp_oad_params->mask_idx[grp_oad_params->oad_count] = mask_idx;

	
	grp_oad_params->resp_byte_num[pos] = read_item.data_len;//read_item.item_count * read_item.data_len;
	
	grp_oad_params->oad_count++;
}


/*  获取方案OAD相关的参数 */
BOOLEAN get_plan_oad_params(READ_PARAMS *read_param,INT16U obis, INT8U index)
{
	/*  */
	INT8U process_mask[8];
	INT8U **oad;
	INT8U idx;
	INT8U max_idx;
	INT8U tsk_idx;
	INT16U page_id = 0xFFFF;
	INT16U tmp_pgid;
	GROUP_OAD_PARAMS *grp_oad_params;
	tagREADITEM read_item;
	INT8U type = READ_TYPE_CUR;/* 类型 冻结 实时 曲线?? */
	
	switch(obis)
	{
		case 0x5002: /* 分钟冻结 */
		case 0x5003: /* 小时冻结 */
		case 0x5004: /* 日冻结 */
		case 0x5005: /* 结算日冻结 */
		case 0x5006: /* 月冻结 */
		case 0x5007: /* 年冻结 */
			type = READ_TYPE_HOLD;
			break;
		default:
			
			break;
	}
	/* 先找到一个，然后找同类的，同类的，就是在相同page规划的空间
	 * 这样找到的抄读OAD不一定按照顺序，处理方案就是在存储的时候，重新组织数据，然后存储。
	 * 
	 */
	//idx            = read_param->norml_task_save_params.oad_idx;
	tsk_idx = read_param->idx_tsk_attr;
	if(tsk_idx > 10)
	{
		// TODO:
		
	}
	#if 0
	max_idx        = read_param->norml_task_save_params.max_oad_cnt;
	grp_oad_params = (GROUP_OAD_PARAMS *)&(read_param->norml_task_save_params.group_oad_params);
	mem_set(grp_oad_params->value,sizeof(GROUP_OAD_PARAMS),0x00);
	process_mask = read_param->norml_task_save_params.process_mask;
	#else
	max_idx        = gTaskPlanAttribute[tsk_idx].oadcnt;
	grp_oad_params = (GROUP_OAD_PARAMS *)&(read_param->norml_task_save_params.group_oad_params);
	mem_set(grp_oad_params->value,sizeof(GROUP_OAD_PARAMS),0x00);
	mem_cpy(process_mask, gTaskPlanAttribute[tsk_idx].procs_mask, 8);
	#endif
	//oad = read_param->norml_task_save_params.plan_oad;
	//oad     = gTaskPlanAttribute[tsk_idx].planOad;
	//page_id = 0;
	if(type == READ_TYPE_HOLD)
	{
		// 没必要
		//mem_cpy(grp_oad_params->master_oad.value,read_param->oad,4);
	}
	for(idx=0;idx< max_idx;idx++)  //max_idx 表示本方案中有多少个oad
	{
		if(get_bit_value(process_mask, 8, idx))
		{
			if(oad_search_in_list(type,index,gTaskPlanAttribute[tsk_idx].planOad[idx],&read_item))
			{
				if(page_id != 0xFFFF)/* 存在的话,查看是否page_offset 相同 */
				{
					/* PAGE_ID相同，把参数存起来 */
					if(page_id == read_item.page_id)
					{
						if(grp_oad_params->oad_count >= 32)
						{
							/* 不好的处理方案 不用了?? */
							clr_bit_value(process_mask, 8, idx);
							continue;
						}

						/* 数据存储 */
						save_grp_read_oad_params(grp_oad_params, read_item, idx);
						clr_bit_value(process_mask, 8, idx);
					}
				}
				else
				{
					
					page_id = read_item.page_id;
					/* 如果ID和上次不一样，则更新last_td */
					if(page_id != bin2_int16u(gTaskPlanAttribute[tsk_idx].page_id))
					{
						//set_cosem_datetime_s(read_param->last_td, gTaskPlanAttribute[idx].last_cycle);
						gTaskPlanAttribute[idx].read_num = 0;
						
					}
					/* 数据存储 */
					save_grp_read_oad_params(grp_oad_params, read_item, idx);
					/* 不考虑重复抄读，暂时清除 TODO: */
					clr_bit_value(process_mask, 8, idx);
				}
			}
			else
			{
				clr_bit_value(process_mask, 8, idx);
			}
		}
	}

	if(grp_oad_params->oad_count)
	{
		/* PAGE_ID */
		mem_cpy(gTaskPlanAttribute[tsk_idx].page_id,(void *)&page_id,2);
		return TRUE;
	}

	//找不到，就把 mask都清零
	mem_set(gTaskPlanAttribute[tsk_idx].procs_mask,8,0x00);
	return FALSE;
}

/*
 * 功能：检查方案执行当前时间与周期抄读匹配性
 * cycle_time-当前时间，exec_time-本周期起始时间，cycle-TI，cycle_unit-周期单位
 * 判断方式，周期抄读时标增加TI后，判断当前时间是否在其范围之内
 */
BOOLEAN check_task_execute_cycle(INT8U cycle_time[7],INT8U exec_time[7],INT16U cycle, INT8U cycle_unit)
{
    INT8U  cur_exec_time[7]={0};
    
    mem_cpy(cur_exec_time, exec_time, 7);
    
    switch (cycle_unit)
    {
        case 0://秒，抄读时周期单位为秒的为支持，暂不支持,支持之后可以将注掉部分放开，需进行测试
            return FALSE;
            //datetime_add_seconds(cur_exec_time + YEAR,cur_exec_time + MONTH,cur_exec_time + DAY,cur_exec_time + HOUR,cur_exec_time + MINUTE,cur_exec_time + SECOND,cycle);
            break;
        case 1://分
            datetime_add_minutes(cur_exec_time + YEAR,cur_exec_time + MONTH,cur_exec_time + DAY,cur_exec_time + HOUR,cur_exec_time + MINUTE,cycle);
            break;
        case 2://时
            datetime_add_hours(cur_exec_time + YEAR,cur_exec_time + MONTH,cur_exec_time + DAY,cur_exec_time + HOUR,cycle);
            break;
        case 3://日
            date_add_days(cur_exec_time + YEAR,cur_exec_time + MONTH,cur_exec_time + DAY,cycle);
            break;
        case 4://月
            date_add_months(cur_exec_time + YEAR,cur_exec_time + MONTH,cycle);
            break;
        case 5://年,周期为年的目前抄读中不支持，此处也不做处理，避免出现问题,直接按照时标不正确处理
            return FALSE;
            break;
    }
    // 当前时间大于起始周期，且下一周期开始时间大于当前时间，不刷新 
    if((compare_string_reverse(cycle_time,exec_time,7) >= 0)&&(compare_string_reverse(cur_exec_time,cycle_time,7) > 0))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    
}



/* start_time 任务启动时间 第一次的时候，参数填充当前时间吧
 * 
 */
void check_read_cycle_one_task()//BOOLEAN first)
{
	INT8U idx;
	INT8U sub_idx;
	INT8U unit;//单位 
	INT8U num; // 数值
	INT8U cur_time[7];
	INT8U date_time_cur[8];
	INT8U check_val = 0x00; // 00 OR  ff 
	DateTime dt;
	BOOLEAN is_valid = FALSE;
	INT8U   acq_unit = 0;
	INT16U  acq_val  = 0;
	tpos_datetime(&dt);
	//if(first)
	{
		mem_cpy(cur_time,dt.value,6);
		cur_time[6] = dt.century;
		#ifdef __DEBUG_
		system_debug_data(cur_time,7);
		#endif
		mem_cpy(date_time_cur, cur_time, 7);
	}
	for(idx=0;idx<MAX_TASK_COUNT;idx++)
	{
		is_valid = FALSE;
		if(gTaskPlanAttribute[idx].task_id != 0xFF)
		{
			/*  */
			unit = gTaskPlanAttribute[idx].run_unit;
			num  = gTaskPlanAttribute[idx].run_frq;
			if(num >0)
			{
				switch(unit)
				{
					case 0://秒
						break;
					case 1: // 分
						if(!check_task_execute_cycle(date_time_cur, gTaskPlanAttribute[idx].cycle_date, num, unit)  //当前时间
                           && !(check_is_all_ch(gTaskPlanAttribute[idx].last_cycle, 7, check_val)))
						{
							mem_cpy(gTaskPlanAttribute[idx].cycle_date, date_time_cur, 7);
							gTaskPlanAttribute[idx].cycle_date[SECOND] = 0;
							gTaskPlanAttribute[idx].cycle_date[MINUTE] = (INT8U)(date_time_cur[MINUTE] / num) * num;
							mem_cpy(gTaskPlanAttribute[idx].last_cycle, gTaskPlanAttribute[idx].cycle_date, 7);
							datetime_minus_minutes(gTaskPlanAttribute[idx].last_cycle + YEAR,
							gTaskPlanAttribute[idx].last_cycle + MONTH,
							gTaskPlanAttribute[idx].last_cycle + DAY,
							gTaskPlanAttribute[idx].last_cycle + HOUR,
							gTaskPlanAttribute[idx].last_cycle + MINUTE,
							num);
							is_valid = TRUE;
						}
						//第一次全FF还是全00，再说吧
						else if(check_is_all_ch(gTaskPlanAttribute[idx].last_cycle, 7, check_val))
						{
							mem_cpy(gTaskPlanAttribute[idx].cycle_date, date_time_cur, 7);
							gTaskPlanAttribute[idx].cycle_date[SECOND] = 0;
							gTaskPlanAttribute[idx].cycle_date[MINUTE] = (INT8U)(date_time_cur[MINUTE] / num) * num;
							mem_cpy(gTaskPlanAttribute[idx].last_cycle, gTaskPlanAttribute[idx].cycle_date, 7);
							datetime_minus_minutes(gTaskPlanAttribute[idx].last_cycle + YEAR,
							gTaskPlanAttribute[idx].last_cycle + MONTH,
							gTaskPlanAttribute[idx].last_cycle + DAY,
							gTaskPlanAttribute[idx].last_cycle + HOUR,
							gTaskPlanAttribute[idx].last_cycle + MINUTE,
							num);
							if(check_task_execute_cycle(date_time_cur, gTaskPlanAttribute[idx].cycle_date, num, unit))
							{
								is_valid = TRUE;
							}
						}
						break;
					case 2:// 小时
						if(!check_task_execute_cycle(date_time_cur, gTaskPlanAttribute[idx].cycle_date, num, unit)
                           && !(check_is_all_ch(gTaskPlanAttribute[idx].last_cycle, 7, check_val)))
						{
							mem_cpy(gTaskPlanAttribute[idx].cycle_date, date_time_cur, 7);
							gTaskPlanAttribute[idx].cycle_date[SECOND] = 0;
							gTaskPlanAttribute[idx].cycle_date[MINUTE] = 0;
							gTaskPlanAttribute[idx].cycle_date[HOUR] = (INT8U)(date_time_cur[HOUR] / num) * num;
							mem_cpy(gTaskPlanAttribute[idx].last_cycle, gTaskPlanAttribute[idx].cycle_date, 7);
							datetime_minus_hours(gTaskPlanAttribute[idx].last_cycle + YEAR,
							gTaskPlanAttribute[idx].last_cycle + MONTH,
							gTaskPlanAttribute[idx].last_cycle + DAY,
							gTaskPlanAttribute[idx].last_cycle + HOUR,
							num);
							is_valid = TRUE;
						}
						//第一次全FF还是全00，再说吧
						else if(check_is_all_ch(gTaskPlanAttribute[idx].last_cycle, 7, check_val))
						{
							mem_cpy(gTaskPlanAttribute[idx].cycle_date, date_time_cur, 7);
							gTaskPlanAttribute[idx].cycle_date[SECOND] = 0;
							gTaskPlanAttribute[idx].cycle_date[MINUTE] = 0;
							gTaskPlanAttribute[idx].cycle_date[HOUR] = (INT8U)(date_time_cur[HOUR] / num) * num;
							mem_cpy(gTaskPlanAttribute[idx].last_cycle, gTaskPlanAttribute[idx].cycle_date, 7);
							datetime_minus_hours(gTaskPlanAttribute[idx].last_cycle + YEAR,
							gTaskPlanAttribute[idx].last_cycle + MONTH,
							gTaskPlanAttribute[idx].last_cycle + DAY,
							gTaskPlanAttribute[idx].last_cycle + HOUR,
							num);
							if(check_task_execute_cycle(date_time_cur, gTaskPlanAttribute[idx].cycle_date, num, unit))
							{
								is_valid = TRUE;
							}
						}
						break;
					case 3:// 日
						if(!check_task_execute_cycle(date_time_cur, gTaskPlanAttribute[idx].cycle_date, num, unit)
                           && !(check_is_all_ch(gTaskPlanAttribute[idx].last_cycle, 7, check_val)))
						{
							mem_cpy(gTaskPlanAttribute[idx].cycle_date, date_time_cur, 7);
							gTaskPlanAttribute[idx].cycle_date[SECOND] = 0;
							gTaskPlanAttribute[idx].cycle_date[MINUTE] = 0;
							gTaskPlanAttribute[idx].cycle_date[HOUR]   = 0;
							gTaskPlanAttribute[idx].cycle_date[DAY] = (INT8U)(date_time_cur[DAY] / num) * num;
							mem_cpy(gTaskPlanAttribute[idx].last_cycle, gTaskPlanAttribute[idx].cycle_date, 7);
							date_minus_days(gTaskPlanAttribute[idx].last_cycle + YEAR,
							gTaskPlanAttribute[idx].last_cycle + MONTH,
							gTaskPlanAttribute[idx].last_cycle + DAY,
							num);
							is_valid = TRUE;
						}
						//第一次全FF还是全00，再说吧
						else if(check_is_all_ch(gTaskPlanAttribute[idx].last_cycle, 7, check_val))
						{
							mem_cpy(gTaskPlanAttribute[idx].cycle_date, date_time_cur, 7);
							gTaskPlanAttribute[idx].cycle_date[SECOND] = 0;
							gTaskPlanAttribute[idx].cycle_date[MINUTE] = 0;
							gTaskPlanAttribute[idx].cycle_date[HOUR]   = 0;
							gTaskPlanAttribute[idx].cycle_date[DAY] = (INT8U)(date_time_cur[DAY] / num) * num;
							mem_cpy(gTaskPlanAttribute[idx].last_cycle, gTaskPlanAttribute[idx].cycle_date, 7);
							date_minus_days(gTaskPlanAttribute[idx].last_cycle + YEAR,
							gTaskPlanAttribute[idx].last_cycle + MONTH,
							gTaskPlanAttribute[idx].last_cycle + DAY,
							num);
							if(check_task_execute_cycle(date_time_cur, gTaskPlanAttribute[idx].cycle_date, num, unit))
							{
								is_valid = TRUE;
							}
						}
						break;
					case 4:// 月
						if(!check_task_execute_cycle(date_time_cur, gTaskPlanAttribute[idx].cycle_date, num, unit)
                           && !(check_is_all_ch(gTaskPlanAttribute[idx].last_cycle, 7, check_val)))
						{
							mem_cpy(gTaskPlanAttribute[idx].cycle_date, date_time_cur, 7);
							gTaskPlanAttribute[idx].cycle_date[SECOND] = 0;
							gTaskPlanAttribute[idx].cycle_date[MINUTE] = 0;
							gTaskPlanAttribute[idx].cycle_date[HOUR]   = 0;
							gTaskPlanAttribute[idx].cycle_date[DAY]    = 1;
							//gTaskPlanAttribute[idx].cycle_date[MINUTE] = (INT8U)(date_time_cur[MINUTE] / num) * num;
							mem_cpy(gTaskPlanAttribute[idx].last_cycle, gTaskPlanAttribute[idx].cycle_date, 7);
							date_minus_months(gTaskPlanAttribute[idx].last_cycle + YEAR,
							gTaskPlanAttribute[idx].last_cycle + MONTH,
							num);
							is_valid = TRUE;
						}
						//第一次全FF还是全00，再说吧
						else if(check_is_all_ch(gTaskPlanAttribute[idx].last_cycle, 7, check_val))
						{
							mem_cpy(gTaskPlanAttribute[idx].cycle_date, date_time_cur, 7);
							gTaskPlanAttribute[idx].cycle_date[SECOND] = 0;
							gTaskPlanAttribute[idx].cycle_date[MINUTE] = 0;
							gTaskPlanAttribute[idx].cycle_date[HOUR]   = 0;
							gTaskPlanAttribute[idx].cycle_date[DAY]    = 1;
							gTaskPlanAttribute[idx].cycle_date[MONTH]  = (INT8U)(date_time_cur[MONTH] / num) * num;
							mem_cpy(gTaskPlanAttribute[idx].last_cycle, gTaskPlanAttribute[idx].cycle_date, 7);
							date_minus_months(gTaskPlanAttribute[idx].last_cycle + YEAR,
							gTaskPlanAttribute[idx].last_cycle + MONTH,
							num);
							if(check_task_execute_cycle(date_time_cur, gTaskPlanAttribute[idx].cycle_date, num, unit))
							{
								is_valid = TRUE;
							}
						}
						break;
					default:
						break;
				}
				if(is_valid)
				{
					mem_set(gTaskPlanAttribute[idx].mask,8,0x00);
					mem_set(gTaskPlanAttribute[idx].procs_mask,8,0x00);
					
					for(sub_idx=0;sub_idx<gTaskPlanAttribute[idx].oadcnt;sub_idx++)
					{
						set_bit_value(gTaskPlanAttribute[idx].mask, 8, sub_idx);
					}
					mem_cpy(gTaskPlanAttribute[idx].procs_mask,gTaskPlanAttribute[idx].mask,8);

					// 处理ctrl flag
					gTaskPlanAttribute[idx].ctl_flg.in_cycle  = 1;
					gTaskPlanAttribute[idx].ctl_flg.new_cycle = 1;

					/* 采集类型 */ 
					switch(gTaskPlanAttribute[idx].acq_type)
					{
						#if 0
						case 3:// TI
							/* 单位  */
							
							acq_unit = gTaskPlanAttribute[idx].acq_content[0];
							acq_val  = cosem_bin2_int16u(gTaskPlanAttribute[idx].acq_content + 1);
							if(gTaskPlanAttribute[idx].run_unit < acq_unit)
							 ||(acq_val == 0)
							{
								/* 只采集一个点 */
								gTaskPlanAttribute[idx].point_num = 1;
							}
							else
							{
								/* 处理下吧 */
								gTaskPlanAttribute[idx].point_num = get_point_num(acq_unit,acq_val,
								                                           gTaskPlanAttribute[idx].run_unit,
								                                           gTaskPlanAttribute[idx].run_frq);
							}
							break;
						#endif
						default:
							gTaskPlanAttribute[idx].point_num = 1;
							break;
					}
					
				}
			}
		}
	}
	
}

/* OOP数据抄读
 * make frame ->send -> wait resp ->???
 * 
 */
extern void read_meter_fixed_time_interval_hold_data(INT16U obis,READ_PARAMS *read_param);
void read_oop_data(READ_PARAMS *read_params)
{
	/*  */
	METER_DOCUMENT meter_doc;
    INT16U  obis = 0;
	mem_cpy((void *)&meter_doc,(void *)&gmeter_doc,sizeof(METER_DOCUMENT));

	// mast oad obis 
	obis = cosem_bin2_int16u(read_params->oad.object_id); //主OAD
	switch(obis)
	{
		

		case 0x5002: /* 分钟冻结 */
		case 0x5003: /* 小时冻结 */
		case 0x5004: /* 日冻结 */
		case 0x5005: /* 结算日冻结 */
		case 0x5006: /* 月冻结 */
		case 0x5007: /* 年冻结 */
			read_meter_fixed_time_interval_hold_data(obis,read_params);
			break;
		default:
			read_meter_cur_data(obis,read_params);
			break;
			//read_meter_fixed_time_interval_hold_data();

	}
}
INT8U calculate_point_num(READ_PARAMS *read_params,INT8U ti[3])
{
	INT16U obis;
	INT8U td[7];
	INT8U num = 1;
	mem_cpy(td,read_params->last_td,7);

	obis = cosem_bin2_int16u(read_params->oad.object_id);
	if( (obis == 0x5002) || (obis == 0x5003) )
	{
		if( (ti[0] > 4) || (cosem_bin2_int16u(ti+1) == 0) )
		{
			return 1;
		}
		while(1)
		{
			cosem_datetime_add_ti(td, ti);
			if(compare_string(td,read_params->hold_td,7) < 0 )
			{
				num++;
			}
			else
			{
				break;
			}
		}

		if(num > 96)
		{
			num = 96;
		}
		return num;
	}

	return 1;
	
}
void prepare_read_oop_data_cycle(READ_PARAMS *read_params)  //准备循环
{
	/* */
	static INT8U last_minute[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	DateTime sys_datetime;
	INT8U idx;
	BOOLEAN read_flg;
	INT16U obis = 0;
	OBJECT_ATTR_DESC1   oad;
	tpos_datetime(&sys_datetime);

	if( (last_minute[MINUTE]!=sys_datetime.value[MINUTE])||(last_minute[HOUR]!=sys_datetime.value[HOUR])
      ||(last_minute[DAY]!=sys_datetime.value[DAY])||(last_minute[MONTH]!=sys_datetime.value[MONTH])
      ||(last_minute[YEAR]!=sys_datetime.value[YEAR]) )
    {
    	/* 每分钟刷新一次 */
    	last_minute[MINUTE] = sys_datetime.value[MINUTE];
        last_minute[HOUR]   = sys_datetime.value[HOUR];
        last_minute[DAY]    = sys_datetime.value[DAY];
        last_minute[MONTH]  = sys_datetime.value[MONTH];
        last_minute[YEAR]   = sys_datetime.value[YEAR];
        check_read_cycle_one_task();
    }

    for(idx=0;idx<MAX_TASK_COUNT;idx++)
    {
    	read_flg = FALSE;
    	if(gTaskPlanAttribute[idx].task_id != 0x00)
    	{
			/*  */
			mem_cpy(oad.value,gTaskPlanAttribute[idx].master_oad,4);
			obis = cosem_bin2_int16u(oad.object_id);

			if( (0 == sys_datetime.value[HOUR]) && (0 == sys_datetime.value[MINUTE]) )
			{
				if(sys_datetime.value[SECOND] < 30)
				{
					/* 日冻结 */
					if( (0x5004 == obis) || (0x5005 == obis) || (0x5006 == obis) )
					{
						continue;
					}
				}
			}

    		/*  */
    		if(gTaskPlanAttribute[idx].ctl_flg.new_cycle)  //该方案进入了新周期需要读上个周期的数据
    		{
    			/* clr */
    			mem_set(read_params->value,sizeof(READ_PARAMS),0x00);
    			// id ;  方案所在全局中索引,有无意思 呵呵 ; 
    			//master_oad:ctrl冻结曲线等  acq_type:采集类型
    			read_params->task_idx[0] = gTaskPlanAttribute[idx].task_id;
    			read_params->idx_tsk_attr = idx;
    			mem_cpy(read_params->oad.value,gTaskPlanAttribute[idx].master_oad,4);
    			read_params->acq_type = gTaskPlanAttribute[idx].acq_type;
				//set_cosem_datetime_s(read_params->hold_td,);
				
				/* 直接转换??? */
				set_cosem_datetime_s(read_params->hold_td, gTaskPlanAttribute[idx].cycle_date);
				set_cosem_datetime_s(read_params->last_td, gTaskPlanAttribute[idx].last_cycle);

				//计算点数
				gTaskPlanAttribute[idx].point_num = calculate_point_num(read_params,gTaskPlanAttribute[idx].acq_content);
				/* clr */
				mem_set(gTaskPlanAttribute[idx].page_id,2,0x00);
				
				
				// clr
    			gTaskPlanAttribute[idx].ctl_flg.new_cycle = 0;/*  */
    			read_flg = TRUE;
    		}
    		else if(!check_is_all_ch(gTaskPlanAttribute[idx].procs_mask, 8, 0x00))
    		{
    			/*
    			 * 还存在掩码没处理完成，才可以看看是否需要抄读
    			 * 加载上周期
    			 */
    			if(gTaskPlanAttribute[idx].task_id != read_params->task_idx[0])
    			{
    				// id ;  方案所在全局中索引, 呵呵 ; 
    				//master_oad:ctrl冻结曲线等  acq_type:采集类型
    				read_params->task_idx[0]  = gTaskPlanAttribute[idx].task_id;
    				read_params->idx_tsk_attr = idx;
    				mem_cpy(read_params->oad.value,gTaskPlanAttribute[idx].master_oad,4);
    				read_params->acq_type = gTaskPlanAttribute[idx].acq_type;
    				/* 直接转换??? */
					set_cosem_datetime_s(read_params->hold_td, gTaskPlanAttribute[idx].cycle_date); //
					set_cosem_datetime_s(read_params->last_td, gTaskPlanAttribute[idx].last_cycle);  //这俩时间是啥意思？？？？
					//计算点数
					gTaskPlanAttribute[idx].point_num = calculate_point_num(read_params,gTaskPlanAttribute[idx].acq_content);
    			}
    			/* reload procs_mask */
    			//mem_cpy(read_params->normal_task_oad_mask,gTaskPlanAttribute[idx].procs_mask,4);
    			read_flg = TRUE;
    		}
    		else
    		{
    			continue;
    		}
    	}
    	if(read_flg)
    	{
    		/* 抄读完成，然后返回 */
			read_oop_data(read_params);

			/* 更新procs_mask */
    		return ;
    	}
    }
	
	
}

#define FLASH_BUFFER_SIZE   500
INT8U FLASH_BUFFER[FLASH_BUFFER_SIZE];




//#define DATA_HEADER_SIZE   sizeof(DATA_HEADER) 
#define MAX_PAGE_SIZE FLASH_SECTOR_SIZE   // 4096  

/* 事务处理  暂时不处理 TODO: */
typedef union{
    INT8U value[4];
    struct{
        INT8U transaction_flag;
        INT8U page_id[2];
        INT8U idx_data[2];
        INT8U block_size[2];
    }elmt;
}TRANSACTION_FLAG;

void read_flash_data(INT8U meter_no[6],INT16U data_offset,INT8U* data,INT16U data_len,INT16U save_size,INT16U page_id,INT16U page_count)
{
    PAGE_HEADER *page_header;
    INT32U addr;
    INT16U save_count,head_count;
    INT16U data_addr;
    INT16U idx_data,idx_page;
    INT16U idx_data_last,idx_page_last;
    INT16U sector;
    INT8U head_data_last;//,head_data;
    BOOLEAN is_find;

	// 要加上数据头，为了读取的时候检索使用 
	save_size   += DATA_HEADER_SIZE;
	data_offset += DATA_HEADER_SIZE;
	
    mem_set(data,data_len,0xFF);
    if (save_size > FLASH_BUFFER_SIZE) return;
    if ((data_offset + data_len) >= FLASH_BUFFER_SIZE) return;  //超了

    page_header = (PAGE_HEADER*)FLASH_BUFFER;

    save_count = MAX_PAGE_SIZE/save_size;
    head_count = save_count/8;
    if (save_count%8) head_count++;
    save_count = (MAX_PAGE_SIZE-head_count-6)/save_size;   //一页上存几个数据块,加6个字节的电表地址
    head_count = save_count/8;           //页头标志占用字节数
    if (save_count%8) head_count++;
    head_count += 6;  //加6个字节的电表地址
    if ((head_count+save_size*save_count) > MAX_PAGE_SIZE) return;   //分配错误
    data_addr = head_count;  //每页数据的起始地址

    tpos_mutexPend(&SIGNAL_FLASH);
    
    //寻找最后一次写得位置；
    is_find = FALSE;
    idx_data_last = save_count;
    idx_page_last = 0xFFFF;
    head_data_last = 0x00;
    for(idx_page=0;idx_page<page_count;idx_page++)
    {
        //addr = (page_id+idx_page)*MAX_PAGE_SIZE;
        sector = (page_id+idx_page);
        nor_flash_read_data(sector,0,page_header->meter_mo,head_count);
        if (meter_no)
        {
            if (check_is_all_ch(page_header->meter_mo,head_count,0xFF) == FALSE)
            {
                if (compare_string(page_header->meter_mo,meter_no,6) != 0)
                {
                    mem_set(page_header->meter_mo,head_count,0xFF);
                }
            }
        }
        else
        {
            if (check_is_all_ch(page_header->meter_mo,6,0xFF) == FALSE)
            {
                mem_set(page_header->meter_mo,head_count,0xFF);
            }
        }

        for(idx_data=0;idx_data<save_count;idx_data++)
        {
            if(get_bit_value(page_header->use_flag,head_count-6,idx_data))
            {
                is_find = TRUE;
                //head_data = page_header->use_flag[idx_data/8];
                break;
            }
            else
            {
                idx_data_last = idx_data;
                idx_page_last = idx_page;
                head_data_last = page_header->use_flag[idx_data_last/8];
            }
        }
        if (is_find) break;
    }
    
    if (idx_data_last >= save_count)
    {
        idx_data_last = save_count - 1;
        idx_page_last = page_count - 1;
        //addr = (page_id+idx_page_last)*MAX_PAGE_SIZE;
        sector = (page_id+idx_page_last);
        nor_flash_read_data(sector,0,page_header->meter_mo,head_count);
        head_data_last = page_header->use_flag[idx_data_last/8];
        if ((meter_no) && (compare_string(page_header->meter_mo,meter_no,6) != 0)) head_data_last = 0xFF;
        if ((!meter_no) && (check_is_all_ch(page_header->meter_mo,6,0xFF) == FALSE)) head_data_last = 0xFF;
    }

    if (is_find)
    {
        if ((head_data_last & (0x01<<(idx_data_last%8))) == 0)
        {
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,200,"*** read_flash_data : page_id = %04d , idx_page = %04d , idx_data = %04d , save_count = %04d",page_id,idx_page,idx_data,save_count);
            debug_println(info);
            #endif
            //addr = (page_id+idx_page_last)*MAX_PAGE_SIZE;
            addr   = data_addr;
            addr  += idx_data_last * save_size;
            //addr  += data_offset;
            sector = (page_id+idx_page_last);
            //nor_flash_read_data(sector,addr,data,data_len);
            nor_flash_read_data(sector,addr,FLASH_BUFFER,save_size);

            mem_cpy(data,FLASH_BUFFER,DATA_HEADER_SIZE);
            mem_cpy(data+DATA_HEADER_SIZE,FLASH_BUFFER+data_offset,data_len -DATA_HEADER_SIZE);  //小心长度
        }
    }

    tpos_mutexFree(&SIGNAL_FLASH);
}

//一个数据块的最大尺寸是256字节，不能超过256字节
void write_flash_data(HEADER_TD hdr_td,INT8U meter_no[6],INT16U data_offset,INT8U* data,INT16U data_len,INT16U save_size,INT16U page_id,INT16U page_count)
{
    PAGE_HEADER *page_header;
    INT32U addr;
    INT16U save_count,head_count;
    INT16U data_addr,idx_page_erase;
    INT16U idx_data,idx_page;
    INT16U idx_data_last,idx_page_last;
    INT16U sector;
    INT8U head_data_last;//,head_data;
    BOOLEAN is_find,is_write;

	// 要加上数据头，为了读取的时候检索使用 
	save_size   += DATA_HEADER_SIZE;
	data_offset += DATA_HEADER_SIZE;
//	tmp += DATA_HEADER_SIZE;
    if (save_size > FLASH_BUFFER_SIZE) return;
    if ((data_offset + data_len) >= FLASH_BUFFER_SIZE) return;  //写超了


    page_header = (PAGE_HEADER*)FLASH_BUFFER;

    save_count = MAX_PAGE_SIZE/save_size;
    head_count = save_count/8;
    if (save_count%8) head_count++;
    save_count = (MAX_PAGE_SIZE-head_count-6)/save_size;   //一页上存几个数据块,加6个字节的电表地址
    head_count = save_count/8;           //页头标志占用字节数
    if (save_count%8) head_count++;
    head_count += 6;  //加6个字节的电表地址
    if ((head_count+save_size*save_count) > MAX_PAGE_SIZE) 
    {
    	// TODO: log_info????
    	return;   //分配错误
    }
    data_addr = head_count;  //每页数据的起始地址

    tpos_mutexPend(&SIGNAL_FLASH);

    //寻找最后一次写得位置；
    is_find = FALSE;
    idx_data_last = save_count;
    idx_page_last = 0xFFFF;
    head_data_last = 0x00;
    for(idx_page=0;idx_page<page_count;idx_page++)
    {
        //addr = (page_id+idx_page)*MAX_PAGE_SIZE;
        sector = (page_id+idx_page);
        nor_flash_read_data(sector,0,page_header->meter_mo,head_count);
        if (meter_no)
        {
            if (check_is_all_ch(page_header->meter_mo,head_count,0xFF) == FALSE)
            {
                if (compare_string(page_header->meter_mo,meter_no,6) != 0)
                {
                    nor_flash_erase_page(sector);//(page_id+idx_page)*MAX_PAGE_SIZE);
                    #ifdef __SOFT_SIMULATOR__
                    snprintf(info,200,"*** erase_page : page_id = %04d , idx_page = %04d",page_id,idx_page_erase);
                    debug_println(info);
                    #endif
                    mem_set(page_header->meter_mo,head_count,0xFF);
                }
            }
        }
        else
        {
            if (check_is_all_ch(page_header->meter_mo,6,0xFF) == FALSE)
            {
                //if_nor_flash_se((page_id+idx_page)*MAX_PAGE_SIZE);
                nor_flash_erase_page(sector);
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,200,"*** erase_page : page_id = %04d , idx_page = %04d",page_id,idx_page_erase);
                debug_println(info);
                #endif
                mem_set(page_header->meter_mo,head_count,0xFF);
            }
        }
        for(idx_data=0;idx_data<save_count;idx_data++)
        {
            if(get_bit_value(page_header->use_flag,head_count-6,idx_data))
            {
                is_find = TRUE;
                //head_data = page_header->use_flag[idx_data/8];
                break;
            }
            else
            {
                idx_data_last = idx_data;
                idx_page_last = idx_page;
                head_data_last = page_header->use_flag[idx_data_last/8];
            }
        }
        if (is_find) break;
    }
    
    if (idx_data_last >= save_count)
    {
        idx_data_last = save_count - 1;
        idx_page_last = page_count - 1;
        //addr = (page_id+idx_page_last)*MAX_PAGE_SIZE;
        sector = (page_id+idx_page_last);
        nor_flash_read_data(sector,0,page_header->meter_mo,head_count);
        head_data_last = page_header->use_flag[idx_data_last/8];
        if ((meter_no) && (compare_string(page_header->meter_mo,meter_no,6) != 0)) head_data_last = 0xFF;
        if ((!meter_no) && (check_is_all_ch(page_header->meter_mo,6,0xFF) == FALSE)) head_data_last = 0xFF;
    }

    is_write = TRUE; //需要写数据
    if (is_find)
    {
        if ((head_data_last & (0x01<<(idx_data_last%8))) == 0)
        {
            //addr = (page_id+idx_page_last)*MAX_PAGE_SIZE;
            addr  = data_addr;
            addr += idx_data_last * save_size;
			sector = (page_id+idx_page_last);
            nor_flash_read_data(sector,addr,FLASH_BUFFER,save_size);
system_debug_data(&data_offset,4);
//            if ((compare_string(FLASH_BUFFER+data_offset,data,data_len) != 0))
			if ((compare_string(FLASH_BUFFER+data_offset,data,data_len) != 0)||(compare_string(hdr_td.tsk_start_time,FLASH_BUFFER+5,7) != 0))			
            {
                if (check_is_all_ch(FLASH_BUFFER+data_offset,data_len,0xFF) == FALSE)   //需要换一个新的位置
                {
                    if (idx_data >= (save_count - 1))  //写本页的最后一位置时，擦除下个页
                    {
                        idx_page_erase = idx_page;
                        idx_page_erase++;
                        if (idx_page_erase >= page_count) idx_page_erase = 0;
                        nor_flash_erase_page((page_id+idx_page_erase));//*MAX_PAGE_SIZE);
                        #ifdef __SOFT_SIMULATOR__
                        snprintf(info,200,"*** erase_page : page_id = %04d , idx_page = %04d",page_id,idx_page_erase);
                        debug_println(info);
                        #endif
                    }
                }
                else  //使用旧的位置
                {
                    //head_data = head_data_last;
                    idx_page = idx_page_last;
                    idx_data = idx_data_last;
                    idx_data_last = 0xFFFF;
                }
            }
            else is_write = FALSE;  //一模一样，不用写了
        }
        else
        {
            idx_data_last = 0xFFFF;  
        }
    }
    else
    {
        idx_data_last = 0xFFFF;
        idx_page = 0;
        idx_data = 0;
        //head_data = 0xFF;
        nor_flash_erase_page((page_id+idx_page));//*MAX_PAGE_SIZE);
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,200,"*** Warning : erase_page  page_id = %04d , idx_page = %04d",page_id,idx_page_erase);
        debug_println(info);
        #endif
    }

    if (is_write)
    {
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,200,"*** write_flash_data : page_id = %04d , idx_page = %04d , idx_data = %04d , save_count = %04d",page_id,idx_page,idx_data,save_count);
        debug_println(info);
        #endif
        //addr = (page_id+idx_page)*MAX_PAGE_SIZE;
        addr   = data_addr;
        addr  += idx_data * save_size;
		sector = (page_id+idx_page);

		mem_set(FLASH_BUFFER,4,0xFF);
		mem_cpy(FLASH_BUFFER+4,hdr_td.tsk_start_time,24);
        if (idx_data_last < save_count)  //需要搬移数据块，保留之前数据，并把新数据copy过来，然后写入
        {
            mem_cpy(FLASH_BUFFER+data_offset,data,data_len);
            nor_flash_write_data(sector,addr,FLASH_BUFFER,save_size);
        }
        else //直接写入
        {
			mem_cpy(FLASH_BUFFER+data_offset,data,data_len);
            nor_flash_write_data(sector,addr,FLASH_BUFFER,save_size);
//          nor_flash_write_data(sector,addr+data_offset,data,data_len);
        }
        //写use_flag
        //addr = (page_id+idx_page)*MAX_PAGE_SIZE;
        nor_flash_read_data(sector,0,page_header->meter_mo,head_count);
        if (meter_no) mem_cpy(page_header->meter_mo,meter_no,6);
        clr_bit_value(page_header->use_flag,head_count-6,idx_data);
        nor_flash_write_data(sector,0,page_header->meter_mo,head_count);
    }
	

		system_debug_data(&data_offset,4);
//		system_debug_data(&p,4);
    tpos_mutexFree(&SIGNAL_FLASH);
}

//save_size的最大尺寸是(FLASH_BUFFER_SIZE - DATA_HEADER_SIZE)字节，不能超过FLASH_BUFFER_SIZE字节
//数据的开始存储了DATA_HEADER结构,共占用DATA_HEADER_SIZE个字节
void read_flash_data_from_save_idx(INT32U save_idx,INT16U data_offset,INT8U* data,INT16U data_len,INT16U save_size,INT16U page_id,INT16U page_count)
{
    INT32U addr,idx_data;
    INT16U save_count,max_count;
    INT16U idx_page;
    INT16U sector = 0;

    mem_set(data,data_len,0xFF);

    if ((save_size + DATA_HEADER_SIZE) > FLASH_BUFFER_SIZE) return;
    if ((data_offset + DATA_HEADER_SIZE + data_len) >= FLASH_BUFFER_SIZE) return;  //写超了

    save_count = (MAX_PAGE_SIZE - sizeof(TRANSACTION_FLAG)) / (save_size + DATA_HEADER_SIZE); //每页上的块数
    max_count = save_count * page_count;           //一共存的块数
    idx_data = save_idx % max_count;
    idx_page = idx_data / save_count;              //在哪一页上
    idx_data = idx_data % save_count;              //在页内的第几个位置
	
    tpos_mutexPend(&SIGNAL_FLASH);

    sector = (page_id+idx_page);
    addr   = idx_data * (save_size + DATA_HEADER_SIZE);
    #ifdef __DEBUG_
	system_debug_data((void *)&sector,2);
	system_debug_data((void *)&addr,2);
	#endif
    nor_flash_read_data(sector,addr,FLASH_BUFFER,save_size+DATA_HEADER_SIZE);
    if (compare_string((INT8U*)&save_idx,FLASH_BUFFER,4) == 0) 
    {

        mem_cpy(data,FLASH_BUFFER,DATA_HEADER_SIZE);  //时间在此,添加上时间
		mem_cpy(data+DATA_HEADER_SIZE,FLASH_BUFFER+data_offset+DATA_HEADER_SIZE,data_len - DATA_HEADER_SIZE);//具体的数据内容后延一个偏移
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,200,"*** read_flash_data_from_save_idx : page_id = %04d , idx_page = %04d , idx_data = %04d , save_count = %04d , save_idx = %d",page_id,idx_page,idx_data,save_count,save_idx);
        debug_println(info);
        #endif
    }

    tpos_mutexFree(&SIGNAL_FLASH);
}

//save_size的最大尺寸是(FLASH_BUFFER_SIZE - DATA_HEADER_SIZE)字节，不能超过FLASH_BUFFER_SIZE字节
//数据的开始存储了DATA_HEADER结构体,共占用DATA_HEADER_SIZE个字节
void write_flash_data_from_save_idx(HEADER_TD hdr_td,INT32U save_idx,INT16U data_offset,INT8U* data,INT16U data_len,INT16U save_size,INT16U page_id,INT16U page_count)
{
    INT16U addr,idx_data,idx;
    INT16U save_count,max_count;
    INT16U idx_page;
    INT16U sector = 0;
    INT16U tmp_len;
    INT8U erase_type;
    TRANSACTION_FLAG transaction_flag;

    erase_type = 0;
    if ((save_size + DATA_HEADER_SIZE) > FLASH_BUFFER_SIZE) return;
    if ((data_offset + DATA_HEADER_SIZE + data_len) >= FLASH_BUFFER_SIZE) return;  //写超了

    save_count = (MAX_PAGE_SIZE - sizeof(TRANSACTION_FLAG)) / (save_size + DATA_HEADER_SIZE); //每页上的块数
    max_count = save_count * page_count;           //一共存的块数，也就是存多少次数据的概念 count 
    idx_data = save_idx % max_count; // saveidx 较大，取余后，落在0 ~ max_cont-1范围内
    idx_page = idx_data / save_count;              //在哪一页上
    idx_data = idx_data % save_count;              //在页内的第几个位置
	
    tpos_mutexPend(&SIGNAL_FLASH);

	/*  */
	sector = page_id+idx_page;
    addr   = idx_data * (save_size + DATA_HEADER_SIZE);
    nor_flash_read_data(sector,addr,FLASH_BUFFER,save_size+DATA_HEADER_SIZE);
    if ((check_is_all_ch(FLASH_BUFFER,save_size+DATA_HEADER_SIZE,0xFF) == FALSE)  //写过了
      && (compare_string((INT8U*)&save_idx,FLASH_BUFFER,4) != 0) )
    {
        erase_type = 0xAA; //整块擦除（将整个记录块处理）
    }
    else if ((check_is_all_ch(FLASH_BUFFER+DATA_HEADER_SIZE+data_offset,data_len,0xFF) == FALSE) 
          && (compare_string(FLASH_BUFFER+DATA_HEADER_SIZE+data_offset,data,data_len) != 0))
    {
        erase_type = 0x55; //块内擦除
    }
    
    /* 长度信息 */
    tmp_len = (save_size + DATA_HEADER_SIZE);
    if (erase_type) 
    {
        if (idx_data > 0) //将数据搬移至缓存页，说明需要先搬移数据，然后擦除 然后再搬回数据
        {
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,200,"*** erase_page : data_buffer_page_id = %04d",PAGE_ID_DATA_BUFFER);
            debug_println(info);
            #endif
            nor_flash_erase_page(PAGE_ID_DATA_BUFFER);  //擦除缓存页
            //addr_tmp = (page_id+idx_page) * MAX_PAGE_SIZE;
            
            for(idx=0;idx<idx_data;idx++)
            {
                nor_flash_read_data(sector,idx*tmp_len,FLASH_BUFFER,tmp_len);
                if (check_is_all_ch(FLASH_BUFFER,tmp_len,0xFF) == FALSE)
                {
                    nor_flash_write_data(PAGE_ID_DATA_BUFFER,idx*tmp_len,FLASH_BUFFER,tmp_len);
                }
            }
            
            if (erase_type == 0x55) //块内擦除，将本块也搬过去
            {
                nor_flash_read_data(sector,idx_data*tmp_len,FLASH_BUFFER,tmp_len);
                if (check_is_all_ch(FLASH_BUFFER,tmp_len,0xFF) == FALSE)
                {
                    nor_flash_write_data(PAGE_ID_DATA_BUFFER,idx_data*tmp_len,FLASH_BUFFER,tmp_len);
                }
            }

            //启动事务
            transaction_flag.elmt.transaction_flag = 0xE7;
            transaction_flag.elmt.page_id[0] = (page_id+idx_page);
            transaction_flag.elmt.page_id[1] = (page_id+idx_page)>>8;
            transaction_flag.elmt.idx_data[0] = idx_data;
            transaction_flag.elmt.idx_data[1] = idx_data>>8;
            transaction_flag.elmt.block_size[0] = save_size+10;
            transaction_flag.elmt.block_size[1] = (save_size+10)>>8;
            nor_flash_write_data(PAGE_ID_DATA_BUFFER,MAX_PAGE_SIZE-sizeof(TRANSACTION_FLAG),transaction_flag.value,sizeof(TRANSACTION_FLAG));
        }
        
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,200,"*** erase_page : page_id = %04d , idx_page = %04d",page_id,idx_page);
        debug_println(info);
        #endif
        nor_flash_erase_page((page_id+idx_page));  //擦除页
        
        if (idx_data > 0) //将数据从缓存页搬回
        {
            //更新事务
            transaction_flag.elmt.transaction_flag = 0xC3;
            nor_flash_write_data(PAGE_ID_DATA_BUFFER,MAX_PAGE_SIZE-sizeof(TRANSACTION_FLAG),transaction_flag.value,sizeof(TRANSACTION_FLAG));

            //addr_tmp = (page_id+idx_page) * MAX_PAGE_SIZE;
            for(idx=0;idx<idx_data;idx++)
            {
                nor_flash_read_data(PAGE_ID_DATA_BUFFER,idx*tmp_len,FLASH_BUFFER,tmp_len);
                if (check_is_all_ch(FLASH_BUFFER,tmp_len,0xFF) == FALSE)
                {
                    nor_flash_write_data(sector,idx*tmp_len,FLASH_BUFFER,tmp_len);
                }
            }
            
            //更新事务  事务完成
            transaction_flag.elmt.transaction_flag = 0x00;
            nor_flash_write_data(PAGE_ID_DATA_BUFFER,MAX_PAGE_SIZE-sizeof(TRANSACTION_FLAG),transaction_flag.value,sizeof(TRANSACTION_FLAG));
            
            if (erase_type == 0x55) //块内擦除，将本块读出来
            {
                nor_flash_read_data(PAGE_ID_DATA_BUFFER,idx_data*tmp_len,FLASH_BUFFER,tmp_len);
            }
        }
        
        if (erase_type == 0x55)
        {
            mem_set(FLASH_BUFFER+DATA_HEADER_SIZE+data_offset,FLASH_BUFFER_SIZE-(DATA_HEADER_SIZE+data_offset),0xFF);
        }
        else
        {
            mem_set(FLASH_BUFFER,FLASH_BUFFER_SIZE,0xFF);
        }
    }
    #ifdef __DEBUG_
	system_debug_data((void *)&sector,2);
	system_debug_data((void *)&addr,2);
	#endif
    if ((compare_string(FLASH_BUFFER+DATA_HEADER_SIZE+data_offset,data,data_len) != 0))//&&(compare_string(FLASH_BUFFER+4,hdr_td.tsk_start_time,24))!=0) //如果数据不对那么重新写
    {
        mem_cpy(FLASH_BUFFER,(INT8U*)&save_idx,4);
        /*  */
        mem_cpy(FLASH_BUFFER+4,hdr_td.tsk_start_time,24);
        
        mem_cpy(FLASH_BUFFER+DATA_HEADER_SIZE+data_offset,data,data_len);
		#ifdef __DEBUG_
		system_debug_data((void *)&sector,2);
		system_debug_data((void *)&addr,2);
		#endif

        nor_flash_write_data(sector,addr,FLASH_BUFFER,save_size+DATA_HEADER_SIZE);
	    }

    tpos_mutexFree(&SIGNAL_FLASH);
}
void save_oop_data(INT8U *buf,INT16U buf_len)
{
	
}



void save_normal_list_data(READ_PARAMS *read_param,INT8U *frame,INT16U frame_len)
{
	/*  */
	GROUP_OAD_PARAMS *params;
	INT8U *buf;
	INT32U save_idx;
	INT16U pos = 0;
	INT16U expect_len = 0;
	INT16U data_len = 0;
	INT16U data_offset = 0;
	INT16U offset = 0;
	INT16U save_len = 0;
	INT16U cp_len;
	INT8U  col_cnt = 0;
	INT8U  idx_col;
	INT8U  hold_time[7] = {0};
	HEADER_TD hdr_td;
	DateTime cur_dt;
	METER_DOCUMENT meter_doc;
	
    mem_cpy((void *)&meter_doc,(void *)&gmeter_doc,sizeof(METER_DOCUMENT));
	
	tpos_datetime(&cur_dt);
	

	
	params = &(read_param->norml_task_save_params.group_oad_params);
	buf = read_param->send_frame;/* 借用 不行再改掉 */

	
	pos = get_frame_header_length_gb_oop(frame);
	pos += 1; /* get response */
	pos += 2;  //apdu和piid
	col_cnt = frame[pos++];
	#ifdef __DEBUG_
	system_debug_data(frame+pos,frame_len-pos);
	#endif
	mem_set(buf,300,0xFF);
	/* 比对oad并拷贝对应数据 */
    for(idx_col=0;idx_col<col_cnt;idx_col++)
    {
		if(compare_string(frame+pos,params->oad[idx_col],4)!=0)
	    {
	        return;
	    }
	    pos += 4;
		if(frame[pos++] != 1)   //20 05 02 00 " 00 03 "   20 06 02 00 " 00 03 "   这里如果值不存在，那么其实包含一个错误信息，故有两个字节。 
		{
			pos+=1; 
			continue;
		}
		/* 获取长度信息 */
		data_len = get_object_attribute_item_len(frame+pos,frame[pos]);
		expect_len = params->resp_byte_num[idx_col];
		offset     = bin2_int16u(params->save_off[idx_col]);
        if(data_len == 1)
        {
        	/* 没有数据 */
        	buf[offset] = 0xFF;
        }
        else// 处理正常
        {
        	//
        	if(data_len >(expect_len-1))
        	{
        		cp_len = expect_len - 1 ;
        	}
        	else
        	{
        		cp_len = data_len;
        	}
        	buf[offset] = 1;//有数据
        	mem_cpy(buf+offset+1,frame+pos,cp_len);
        }
        if( save_len <= (offset+expect_len) )
        {
        	save_len = offset + expect_len;
        }
		pos += data_len;
//	    offset+=data_len;
    }
    //
    #ifdef __DEBUG_
	system_debug_data(buf, save_len);
	#endif
	get_datetime_from_cosem(read_param->hold_td, hold_time);
	save_idx = getPassedDays(2000+hold_time[YEAR],hold_time[MONTH],hold_time[DAY]);

	hdr_td.tsk_start_time[0] = DT_DATETIME_S;
	hdr_td.save_time[0]      = DT_DATETIME_S;
	hdr_td.success_time[0]   = DT_DATETIME_S;
	mem_cpy(hdr_td.tsk_start_time+1,read_param->hold_td,7);
	set_cosem_datetime_s(hdr_td.success_time+1, cur_dt.value);
	mem_cpy(hdr_td.save_time+1,read_param->hold_td,7);


	write_flash_data(hdr_td,meter_doc.meter_no, data_offset, buf, save_len, 
	                 params->save_size, read_param->start_page+params->page_id, params->page_cnt);
}
/* 
 * 冻结时间  hold_flag
 */
void parse_save_oop_data(READ_PARAMS *read_param,BOOLEAN hold_flag, INT8U *frame,INT16U frame_len)
{
	GROUP_OAD_PARAMS *params;
	INT8U *master_oad;
	INT8U *buf;
	INT32U save_idx;
	INT16U pos = 0;
	INT16U obis;
	INT16U data_len = 0;
	INT16U idx_col;
	INT16U offset = 0;
	INT16U data_offset;
	INT16U expect_len = 0;
	INT8U  type;
	INT8U  col_cnt;
	//INT8U  hold_flag; //冻结标志???
	INT8U  find_idx = 0;
	INT8U  cp_len = 0;
	INT8U  save_len = 0;
	INT8U  minus_val = 0;
	INT8U  hold_time[7] = {0};
	INT8U  hold_oad[4]  = {0x20,0x21,0x02,0x00}; // 冻结时标OAD
	HEADER_TD hdr_td;
	DateTime cur_dt;

 	tpos_datetime(&cur_dt);
	
	///* 方案任务变化，等待超过某个时间30S吧 */
	//if( (gSystemInfo.plan_task_chg) && (diff_sec_between_dt(&cur_dt, &gSystemInfo.dt) > 30) )
	//{
		//gSystemInfo.plan_task_chg = 0;
		//rebuilt_task_plan_index();
		//check_read_cycle_one_task();
	//}
	
	params = &(read_param->norml_task_save_params.group_oad_params);
	buf = read_param->send_frame;/* 借用 不行再改掉 */
	pos = get_frame_header_length_gb_oop(frame);
	
	if(frame[pos++] == SERVER_APDU_GET)
	{
		type = frame[pos];// APDU
		switch(type)
		{
			case GET_RESPONSE_NORMAL:
				break;
		    case GET_RESPONSE_NORMAL_LIST:
		    	save_normal_list_data(read_param,frame,frame_len);
				break;
			case GET_RESPONSE_RECORD:
				pos += 2;  //apdu和piid
				master_oad = frame+pos;
				pos += 4;
				col_cnt = frame[pos++];// 列 数量 

				/* 比较发送的OAD和接收OAD是否一致 
				 * 接收OAD 00+4字节的OAD=5B
				 */
				find_idx=0xFF; 
				for(idx_col=0;idx_col<col_cnt;idx_col++)
	            {
	            	if(hold_flag)
	            	{
		            	if(idx_col == 0)  //
		            	{
		            		/* 找到冻结时标  后续使用 */
		                    if(compare_string(frame+pos+idx_col*5+1,hold_oad,4)==0)
		                    {
		                        find_idx = idx_col;
		                    }
		            	}
		            	else
		            	{
		                    if(compare_string(frame+pos+(idx_col)*5+1,params->oad[idx_col-1],4)!=0)
		                    {
		                        goto err;
		                    }
	                    }
                    }
                    else
                    {
                    	if(compare_string(frame+pos+(idx_col)*5+1,params->oad[idx_col],4)!=0)
	                    {
	                        goto err;
	                    }
                    }
                    
	            }
				pos+=5*col_cnt;//到数据区
				if (frame[pos++] != 1)//记录数据
				{
	                goto err;
	            }
	            if (frame[pos++] == 0)/* 暂时处理一条吧 */
	            {
	                return 0;
	            }

	            if(hold_flag)
	            {
	            	minus_val = 1;
	            }
				// 先从hold_td(任务启动时间) 获取并转换hold_time
				// 处理没有冻结时间的情况 TODO:::::
	            get_datetime_from_cosem(read_param->hold_td, hold_time);
	            // 处理多条记录  曲线??	            
	            //for()
	            {
		            mem_set(buf,300,0xFF);
		            save_len = 0;
		            for(idx_col=0;idx_col<col_cnt;idx_col++)
		            {
			            /* 获取长度信息 */
			            data_len = get_object_attribute_item_len(frame+pos,frame[pos]);
						if(find_idx == idx_col)
						{
							/* TODO: */
							if(frame[pos]==DT_DATETIME_S)
							{
								/* 获取时间 */
								//mem_cpy(hold_time,frame+pos+1,7);
								get_datetime_from_cosem(frame+pos+1, hold_time);
							}
							pos += data_len;
							continue;
						}
						// -1 mark
						expect_len = params->resp_byte_num[idx_col-minus_val];
						offset     = bin2_int16u(params->save_off[idx_col-minus_val]);
			            if(data_len == 1)
			            {
			            	/* 没有数据 */
			            	buf[offset] = 0xFF;
			            }
			            else// 处理正常
			            {
			            	//
			            	if(data_len >(expect_len-1))
			            	{
			            		cp_len = expect_len - 1 ;
			            	}
			            	else
			            	{
			            		cp_len = data_len;
			            	}
			            	buf[offset] = 1;//有数据
			            	mem_cpy(buf+offset+1,frame+pos,cp_len);
			            }
			            if( save_len <= (offset+expect_len) )
			            {
			            	save_len = offset + expect_len;
			            }
			            pos += data_len;
		            }
					#ifdef __DEBUG_
					system_debug_data(buf, save_len);
					#endif
					//save_oop_data();
					save_idx = getPassedDays(2000+hold_time[YEAR],hold_time[MONTH],hold_time[DAY]);
					obis = cosem_bin2_int16u(read_param->oad.object_id);
					switch(obis)
					{
					case 0x5002: /* 分钟 */
						save_idx *= 96;
						save_idx += hold_time[HOUR] *4;
						save_idx += hold_time[MINUTE] /15;
						break;
					case 0x5003: /* HOUR */
						save_idx *= 24;
						save_idx += hold_time[HOUR];
						break;
					case 0x5005: /* 月 */
						save_idx += hold_time[MONTH];
						break;
					default:
						break;
					}
					//暂定为0 吧 这里基于一帧数据包含规划page的所有数据,所以没有偏移
					// TODO:
					data_offset = 0;

					hdr_td.tsk_start_time[0] = DT_DATETIME_S;
					hdr_td.save_time[0]      = DT_DATETIME_S;
					hdr_td.success_time[0]   = DT_DATETIME_S;
					mem_cpy(hdr_td.tsk_start_time+1,read_param->hold_td,7);
					set_cosem_datetime_s(hdr_td.success_time+1, cur_dt.value);
					switch(gTaskPlanAttribute[read_param->idx_tsk_attr].store_time_flag)
					{
					case 1: // 任务开始时间
						mem_cpy(hdr_td.save_time+1,read_param->hold_td,7);
						break;
					case 2: // 相对于当日 0 点
						mem_set(cur_dt.value,DAY,0x00);
						set_cosem_datetime_s(hdr_td.save_time+1, cur_dt.value);
						break;
					case 3: // 相对于上日 23:59:00
						mem_set(cur_dt.value,DAY,0x00);
						date_minus_days(cur_dt.value+YEAR, cur_dt.value+MONTH, cur_dt.value+DAY, 1);
						cur_dt.value[HOUR]   = 23;
						cur_dt.value[MINUTE] = 59;
						set_cosem_datetime_s(hdr_td.save_time+1, cur_dt.value);
						break;
					case 4: // 相对于上日 00:00:00
						mem_set(cur_dt.value,DAY,0x00);
						date_minus_days(cur_dt.value+YEAR, cur_dt.value+MONTH, cur_dt.value+DAY, 1);
						set_cosem_datetime_s(hdr_td.save_time+1, cur_dt.value);
						break;	
					case 5: // 相对于当月1日 00:00:00
						mem_set(cur_dt.value,MONTH,0x00);
						cur_dt.value[DAY]  = 1;
						set_cosem_datetime_s(hdr_td.save_time+1, cur_dt.value);
						break;
					case 6: // 数据冻结时标
						mem_cpy(hdr_td.save_time+1,read_param->hold_td,7);
						break;
					default:
						mem_cpy(hdr_td.save_time+1,read_param->hold_td,7);
						break;
						
					}
					write_flash_data_from_save_idx(hdr_td, save_idx, data_offset, buf, save_len, 
					                               params->save_size, read_param->start_page+params->page_id, params->page_cnt);
					#ifdef __DEBUG_
					read_flash_data_from_save_idx(save_idx, data_offset, read_param->recv_frame, save_len, 
					                               params->save_size, read_param->start_page+params->page_id, params->page_cnt);
					system_debug_data(read_param->recv_frame, save_len);
					#endif
	            }
				break;
		}
		return;
	}
	//else /* err ?? */
	{
		//clear or todo:
	}
err:
	return;
}
