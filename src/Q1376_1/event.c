/***************************************************************

    DL/T 1376.1  G表===ERC事件部分

    请求1类数据 0x0C



***************************************************************/
#include "app_const_3761.h"
#include "../main_include.h"
#include "app_3761.h"
#include "ErtuParam.h"
#include "event.h"
INT16U  format_check_event_record(INT8U *event_record);
INT16U fetch_event_index(INT32U *ec_list,INT16U *erc_list,INT16U list_count,INT8U erc_idx,INT8U erc_flag);
tagEVETN g_erc_EVENT;
/************************************************************************/
/* 重构事件ERC记录索引，
	@retVal  0,重构成功或不需要重构
	 @retVal -1，重构失败                                                */
/************************************************************************/
INT8S reconstructe_eventERC_index()
{
	INT8S res=-1;
	INT8U need_reconstructe=0;
	tagERC_96bits_region  ERC_96bits;
	INT8U sector_idx=0;
	INT8U free_sector_idx = 0;
	for(sector_idx=0;sector_idx<EVENT_ERC_SECTOR_MAX;sector_idx++)
	{
		fread_erc_params((sector_idx)*FALSE_SECTOR_BITS,&ERC_96bits,sizeof(tagERC_96bits_region));
		if(ERC_96bits.erc_no != 0xFF)
		{
			need_reconstructe = 1;  //不需要初始化，直接重构
		 	break;
		}
	}
	if(need_reconstructe==0x01)
	{
		for(sector_idx=0;sector_idx<EVENT_ERC_SECTOR_MAX;sector_idx++)
		{
			fread_erc_params((sector_idx)*FALSE_SECTOR_BITS,&ERC_96bits,sizeof(tagERC_96bits_region));
			if(ERC_96bits.erc_no != 0xFF)
			{
				g_erc_EVENT.ERC_F3.sector_index[ERC_96bits.erc_no] =ERC_96bits.erc_no;
				g_erc_EVENT.ERC_F3.erc_cnt[ERC_96bits.erc_no] =ERC_96bits.erc_cnt;
			}
			else     //free的sector
			{
				g_erc_EVENT.ERC_F3.sector_free_index[free_sector_idx++]=sector_idx;
			}
		}
	}
	else
	{
		for(sector_idx =0;sector_idx<G_METER_ERC_RECORD_MAX;sector_idx++)
		{
			ERC_96bits.erc_no  = sector_idx;
			ERC_96bits.erc_cnt = 0;
			fwrite_erc_params((sector_idx)*FALSE_SECTOR_BITS,&ERC_96bits,sizeof(tagERC_96bits_region));
			g_erc_EVENT.ERC_F3.sector_index[ERC_96bits.erc_no] =ERC_96bits.erc_no;
			g_erc_EVENT.ERC_F3.erc_cnt[ERC_96bits.erc_no] =ERC_96bits.erc_cnt;		
		}
		for(sector_idx=G_METER_ERC_RECORD_MAX;sector_idx<EVENT_ERC_SECTOR_MAX;sector_idx++)  //自由的也需要初始化
		{
			g_erc_EVENT.ERC_F3.sector_index[sector_idx] = sector_idx;
			g_erc_EVENT.ERC_F3.erc_cnt[sector_idx] =0;
		}
	}
	return 0;
}

void  process_read_event(objRequest *pRequest,objResponse *pResp)
{
INT32U ec_list[EVENT_MAX_SAVE_COUNT*4];
    INT16U erc_list[EVENT_MAX_SAVE_COUNT*2];
    INT16U erc_count = 0;
    INT16U count;
    INT16U pn,fn;
    INT16U itemDataLen;
    INT16U pos_event_end_index;
    INT16U event_file_id;
    INT32U offset;
    INT8U event_begin_index,event_index,ec_index;
    INT8U event_end_index;
    INT8U event_record[EVENT_RECORD_SIZE];
    BOOLEAN hasData;


    ClrTaskWdt();

    //如果不是主动上报,写入FLASH的情况
    if(pResp->channel!=CHANNEL_FLASH)
    {
        //设置控制码：DIR=1,PRM=0, FUNC=8  响应报文，用户数据
        pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_USERDATA;
    }

//    idx = 0;
    //下行报文数据单元标识
    pn=bin2_int16u(pRequest->frame + pRequest->pos);
    fn=bin2_int16u(pRequest->frame + pRequest->pos +2);
    pRequest->pos+=4;
    event_index =  0;

   //检查数据单元标识
    if(  (pn != DA_P0) ||  ( (fn != DT_F1) && (fn != DT_F2) && (fn != DT_F3)) )
    {
        //不能识别的转发任务，发送否定报文
        app_send_NAK(pRequest,pResp);
        return;
    }


    //请求事件的起始和结束指针
    event_begin_index = pRequest->frame[pRequest->pos];  //Pm,F3时为请求事件代码erc
    event_end_index  = pRequest->frame[pRequest->pos+1]; // Pn,F3时为请求事件数量

    //首帧标志置1
    pResp->work_flag[WORK_FLAG_IS_FIRST_FRAME]= TRUE;

    /////////////////////////////////
    //    开始组织响应帧
    /////////////////////////////////

    //提取事件需要临时缓冲区1.5K，由于请求3类数据数据占用缓冲区较小，目前request对象缓存区大小为1900字节，因此可以借用该缓冲区
    if(SIZE_OF_APP_PROTOCOL_FRAME -pRequest->frame_len<256*4+256*2+4)
    {
            app_send_NAK(pRequest,pResp);
           return;
    }

   //设置事件记录数据的起始地址
    //if(fn==DT_F1)
    //{
         //重要事件
         //ec = g_event.ec1;
    //}
    //else if(fn==DT_F2)
    //{
         //一般事件
         //ec = g_event.ec2;
    //}
    //else 
    if(fn==DT_F3)
    {
         //按事件代码查询
         mem_set(ec_list,EVENT_MAX_SAVE_COUNT*4,0xFF);
         mem_set(erc_list,EVENT_MAX_SAVE_COUNT*2,0xFF);
		 if(event_end_index==0)//为请求该事件代码ERC的所有事件
		 {
			event_end_index =15;
		 }
         erc_count=fetch_event_index(ec_list,erc_list,15,event_begin_index,EVENT_TYPE_ALL);
    }

    RESP_READ_EVENT:

    pResp->pos = POS_DATA;
    //设置数据单元标识
    set_pn_fn(pResp->frame, &pResp->pos, pn, fn);
    count = 0;
    hasData = FALSE;
    //F3和F1F2处理不同
    if(fn!=DT_F3)
    {
        //响应帧数据
        pResp->frame[pResp->pos++] = g_event.ec1;
        pResp->frame[pResp->pos++] = g_event.ec2;
        pResp->frame[pResp->pos++] = event_begin_index;  //Pm

        //设置响应帧中的事件记录结束指针
        pos_event_end_index = pResp->pos;

        pResp->frame[pResp->pos++] = event_end_index;    //Pn

        event_index  = event_begin_index;

    }
    else
    {
        if(event_end_index>erc_count)//请求数据是否超过最大事件数量
        {
            event_end_index=erc_count;
        }
        pResp->frame[pResp->pos++] = event_end_index;
        //event_index  = 0;
        pos_event_end_index = pResp->pos;
        if(erc_count==0)//没有可请求的事件
        {
			//   clear_event_status(pRequest->channel);
               //发送响应祯，结束=TRUE
               app_send_response_frame(pRequest,pResp,TRUE);

              return;
        }
   }

    //循环读取事件记录，直到事件记录结束指针或无效的事件记录
    do
    {
           //按最大尺寸读取事件记录
        if(fn!=DT_F3)
        {
       //     event_file_id=(fn==DT_F1)?FILEID_EVENT_VIP:FILEID_EVENT_GEN;
            offset =  EVENT_RECORD_SIZE * event_index+EVENT_HEAD_LEN;
        }
        else
        {
            ec_index = erc_count-event_end_index+event_index;
            if(ec_index>=EVENT_MAX_SAVE_COUNT)
            {
              ec_index=0;
            }
 //           event_file_id= FILEID_EVENT;
            event_file_id+= erc_list[ec_index]>>8;
            offset =  EVENT_RECORD_SIZE * ((erc_list[ec_index] & 0xFF)-1)+EVENT_HEAD_LEN;
        }
  //      fread_array(event_file_id,offset,event_record,EVENT_RECORD_SIZE);

        //判断事件记录的有效性
//        if((event_record[0] > MAX_ERC_NO ) || (event_record[1] > EVENT_RECORD_SIZE) || (event_record[0]==0) )
        {
            // 结束!
            //设置最终结束的事件记录指针
            if(fn!=DT_F3)
            {
                #ifdef INVALID_EVENT_RETURN_INVALID
                //四川博高要求，读不到事件时，事件起始指针和结束指针均返回EE
                if(event_begin_index == event_index)
                {
//                     pos = pos_event_end_index-1;
                     pResp->frame[pos_event_end_index-1] = event_begin_index;
                     pResp->frame[pos_event_end_index]   = event_end_index;
                     for(index=event_begin_index;index<event_end_index;index++)
                     {
                         pResp->frame[pResp->pos++]   = 0x00;
                         pResp->frame[pResp->pos++]   = 0x00;
                     }
                }
                else
                {
                     //事件指针指向下一个
                     event_index ++;
                     if(!hasData)
                     {
                         pResp->frame[pos_event_end_index-1] = event_index;
                     }
                     continue;
                }

               #else
                //设置最终结束的事件记录指针
                 //事件指针指向下一个
                 event_index ++;
                 if(!hasData)
                 {
                     pResp->frame[pos_event_end_index-1] = event_index;
                 }
                 continue;
               #endif
            }
            //发送响应祯
 //           clear_event_status(pRequest->channel);
            //send_response_frame(resp,pos,&first_frame_flag,1);
            //发送响应祯，结束=TRUE
            app_send_response_frame(pRequest,pResp,TRUE);


            return;
        }

        //检查并修正事件数据记录格式。
        itemDataLen = format_check_event_record(event_record);
        count++;
        //判断是否超过最大用户数据有效长度
        //如超过，则进行多帧发送
        if(pResp->pos + itemDataLen > pResp->max_reply_pos)
        {
            //设置最终结束的事件记录指针
            if(fn!=DT_F3)
            {
                 pResp->frame[pos_event_end_index] = event_index;
            }
            else
            {
                 pResp->frame[pos_event_end_index-1] = count;
                 count=0;

            }
            //发送响应祯

            if(FALSE == app_send_response_frame(pRequest,pResp,FALSE))
            {
              return;
            }
            //下帧的起始事件指针
            event_begin_index = event_index;
//            pResp = pRequest->pResp;
            //继续组帧
            goto RESP_READ_EVENT;
        }
          #ifdef __SHANXI_READ_BPLC_NETWORK_INFO__
          switch(event_record[0])
          {
          case ERC60:
          case ERC61:
          case ERC62:
          case ERC63:
               event_record[1] --;  break; /*这4个扩展的事件，长度域是两个字节。存储事件的时候，长度只算一个字节，第二个算到内容里面了，所以多一个字节，到这里减去。
                                             如果存储的时候就用实际长度，主动上报直接提取一个字节长度，会导致主动上报时处理复杂，*/
          }
          #endif

        mem_cpy(pResp->frame+pResp->pos,event_record,itemDataLen);
        //更新响应帧数据位置指针
        pResp->pos += itemDataLen;
        hasData =TRUE;
          //事件指针指向下一个
        event_index ++;
        if(fn!=DT_F3)
        {
             pResp->frame[pos_event_end_index] = event_index;
        }
        else
        {
             pResp->frame[pos_event_end_index-1] = count;
        }
    } while(event_index != event_end_index);

    #ifdef __PROVICE_SICHUAN__
    if(pResp->frame[pos_event_end_index-1] == pResp->frame[pos_event_end_index])
    {
        app_send_NAK(pRequest,pResp);
    }
    else
    {
      //发送响应祯
 //     clear_event_status(pRequest->channel);
     //发送响应祯，结束=TRUE
     app_send_response_frame(pRequest,pResp,TRUE);

    }
    #else
    //发送响应祯
//	clear_event_status(pRequest->channel);
   //发送响应祯，结束=TRUE
    app_send_response_frame(pRequest,pResp,TRUE);

    #endif

}
/*+++
  功能： 检查事件记录的格式
  参数：
         INT8U  event_record;
  返回：
         INT16U 事件记录长度
  描述：

---*/
INT16U  format_check_event_record(INT8U *event_record)
{
  INT8U  dlen;

    dlen = 0;
  switch(event_record[0])
  {
  case ERC1:      dlen = 14;  break;
  case ERC2:      dlen = 6;   break;
  case ERC4:      dlen = 7;   break;
  case ERC5:      dlen = 10;  break;
  case ERC6:      dlen = 14;  break;
  case ERC7:      dlen = 16;  break;
  case ERC8:      dlen =  8;  break;
  case ERC9:      dlen = 28;  break;
  case ERC10:     dlen = 28;  break;
  case ERC11:     dlen = 24;  break;
  case ERC12:     dlen = 7;   break;
  case ERC13:     dlen = 8;   break;
  #ifdef __PROVICE_JIANGSU__
  case ERC64:       dlen =93;   break;
  #endif
#if defined (__QGDW_376_2009_PROTOCOL__) || defined (__QGDW_FK2005__)
  case ERC14:     dlen = 10;  break;
#else
//#if defined(__CHECK_POWEROFF_NEW__)
  case ERC14:     dlen = 11;  break;
//#else
//  case ERC14:     dlen = 10;  break;
//#endif
#endif
  case ERC15:     dlen = 49;  break;
  case ERC16:     dlen =  9;  break;
  case ERC17:     dlen = 27;  break;
  case ERC18:     dlen = 16;  break;
  case ERC19:     dlen = 31;  break;
  case ERC20:     dlen = 22;  break;
  case ERC21:     dlen =  6;  break;
  case ERC23:     dlen = 16;  break;
  case ERC24:     dlen = 14;  break;
  case ERC25:     dlen = 17;  break;
  case ERC26:     dlen = 14;  break;
  case ERC27:     dlen = 17;  break;
  case ERC28:     dlen = 18;  break;
  case ERC29:     dlen = 18;  break;
  case ERC30:     dlen = 13;  break;
  case ERC31:     dlen = 21;  break;
  case ERC32:     dlen = 13;  break;
  case ERC33:     dlen = 35;  break;
  case ERC34:     dlen =  8;  break;
  //#ifdef __ERC_37__
  //case ERC37:     dlen = 70;  break;
  //#endif
  //#ifdef  __COUNTRY_ISRAEL__
  //case ERC37:     dlen = 11;  break;    //以色列扩展电表报警事件
  //#endif
  //#ifdef __ERC_38__
  //case ERC38:     dlen = 70;  break;
  //#endif
  //#ifdef __PLC_REC_CJQ_CYCLE_ERC_40__
  //case ERC40:     dlen = 16;  break;
  //#else
  //case ERC40:     dlen =  9;  break;
  //#endif
  case ERC42:     dlen = 19;  break;
  case ERC43:     dlen = 19;  break;
  case ERC44:     dlen = 15;  break;
  case ERC65:		dlen = 21;  break;
  case ERC66:		dlen = 6;  break;  
  //--------------非定长事件记录----------------
  case ERC3:
  case ERC22:
  case ERC35:   
  case ERC36:
  case ERC37:
  case ERC38:
  case ERC39:
  case ERC40:
  case ERC41:  
  case ERC45: 
  case ERC51:
  case ERC52:
  case ERC53:
  case ERC62:
  case ERC63:
#ifndef __PROVICE_JIANGSU__
  case ERC64:
#endif
  case ERC67:
  case ERC68:
         dlen = event_record[1];
         break;
  }

  #ifdef __QGDW_FK2005__
  switch(event_record[0])
  {
  case ERC2:      dlen = 6;   break;
  case ERC8:      dlen = 7;   break;
  case ERC9:      dlen = 24;  break;
  case ERC10:     dlen = 24;  break;
  case ERC11:     dlen = 23;  break;
  case ERC12:     dlen = 6;   break;
  case ERC13:     dlen = 7;   break;
  case ERC14:	  dlen = 10;  break;
  case ERC15:     dlen = 48;  break;
  case ERC17:     dlen = 23;  break;
  case ERC18:     dlen = 15;  break;
  case ERC24:     dlen = 13;  break;
  case ERC25:     dlen = 13;  break;
  case ERC26:     dlen = 13;  break;
  case ERC27:     dlen = 16;  break;
  case ERC28:     dlen = 17;  break;
  case ERC29:     dlen = 17;  break;
  case ERC30:     dlen = 12;  break;
  }
  #endif
  #ifdef __QGXD_FK2012__
  switch(event_record[0])
  {
  case ERC19:     dlen = 30;  break;
  case ERC20:     dlen =  8;  break;
  case ERC39:     dlen = 20;  break;
  case ERC40:     dlen = 13;  break;
  case ERC42:     dlen = 34;  break;
  } 
  #endif
  #ifdef __QCSG_698__
  switch(event_record[0])
  {
  case ERC8:      dlen =  8;  break;
  case ERC9:      dlen = 25;  break;
  case ERC10:     dlen = 25;  break;
  case ERC11:     dlen = 24;  break;
  case ERC12:     dlen = 7;   break;
  case ERC13:     dlen = 8;   break;
  case ERC15:     dlen = 49;  break;
  case ERC17:     dlen = 24;  break;
  case ERC18:     dlen = 16;  break;
  case ERC24:     dlen = 14;  break;
  case ERC25:     dlen = 14;  break;
  case ERC26:     dlen = 14;  break;
  case ERC27:     dlen = 17;  break;
  case ERC28:     dlen = 18;  break;
  case ERC29:     dlen = 18;  break;
  case ERC30:     dlen = 13;  break;
  } 
  #endif
  //重新设定事件长度，避免事件长度错误。
  event_record[1] = dlen;

  return dlen+2;
}
/*+++
  功能：
        INT8U erc_flag   0-一般事件  1-重要事件 2-全部

  描述：
---*/
INT16U fetch_event_index(INT32U *ec_list,INT16U *erc_list,INT16U list_count,INT8U erc_idx,INT8U erc_flag)
{
    INT32U ec,offset,temp_ec;
    INT16U file_id,count,list_idx,loop;
    INT8U idx,flag;
	INT8U buf[8];

    file_id=erc_idx-1;
//    if(!file_exist(file_id)) return 0;

    for(idx=0;idx<EVENT_MAX_SAVE_COUNT;idx++)
    {
        offset =  idx * EVENT_RECORD_SIZE + FLASH_SECTOR_96BITS_LEAD;
        fread_erc_params(file_id*FALSE_SECTOR_BITS +offset,buf,EVENT_POS_ERC_NO+1);  //读偏移量offset处的数据块，仅仅读到事件长度。
		if(buf[EVENT_POS_ERC_NO]!=erc_idx) continue;
	    ec=bin2_int32u(buf);
        if(ec==INVALID_EC_SEQ)continue;
        flag=buf[EVENT_POS_RPT_FLAG] & 0x01;
        if((flag!=erc_flag) && (erc_flag!=2))continue;

        for(list_idx=0;list_idx<list_count;list_idx++)
        {
            temp_ec=ec_list[list_idx];
            if(ec>=temp_ec)continue;  //这里不会发生开始。
            for(loop=list_count-1;loop>=list_idx+1;loop--)
            {
                ec_list[loop]=ec_list[loop-1];
                erc_list[loop]=erc_list[loop-1];
            }
            ec_list[list_idx]=ec;
            erc_list[list_idx]=((erc_idx-1)<<8)| (idx+1);
            break;
        }
    }
    count=0;
    for(list_idx=0;list_idx<list_count;list_idx++)
    {
        if(erc_list[list_idx]==0xFFFF)continue;
        if((erc_list[list_idx]>>8)!=erc_idx-1)continue;
        count++;

    }
    return count;

}
/*+++
  功能：检查终端事件配置的有效性和重要性
  描述： D0~D63对应事件代码ERC1~ERC64所定义的事件

  返回： VIPEVENT_FLAG   重要事件
         VALIDEVT_FLAG   有效事件 
         0 不需要处理
---*/
INT8U  check_event_prop(INT8U event_code)
{

   INT8U bitpos;
   INT8U event_set[16];


   #if defined __PROVICE_SHANDONG__
   if(event_code == ERC14)  return  VIPEVENT_FLAG;
   #endif


   //1. 读取参数   F9:终端事件记录配置设置
   //   重要事件设置
   fread_ertu_params(EEADDR_SET_F9,event_set,16);

   //2.计算当前事件的位串位置
   bitpos = event_code-1;

   //3.提起当前事件的重要性,如果是重要事件，不用检查有效性标志
   if(get_bit_value(event_set+8,64,bitpos) )
   {
        return  VIPEVENT_FLAG;
   }
   //4.否则，提取当前事件的有效性
   if( get_bit_value(event_set,64,bitpos) ) return VALIDEVT_FLAG;
   return 0;
}

/*+++
  功能：ERC3 参数变更记录
  描述：
---*/

void  event_params_set(INT8U *event)
{
   INT8U event_flag;

   //1. 判断事件属性，检查是否需要处理,0不需要处理

//   event_flag = check_event_prop(ERC3);
//   if(!event_flag ) return;

   event[EVENT_POS_ERC_NO] = ERC3;
   //2. 记录事件数据时间
   set_event_datetime(event+EVENT_POS_ERC_TIME);

   //2. 记录事件数据,已经有了
    save_event_record(event,event_flag);

}


void update_ERC_F3_sector(INT8U *event,INT8U length)
{
	INT8U erc_idx;
	INT8U randsector;
	INT8U new_erc_store_sector =0,old_erc_store_sector = 0;
	INT8U old_erc_store_cnt = 0;
	INT8U event_record[EVENT_RECORD_SIZE];
	INT8U sector=0,move_idx = 0;
	INT32U move_addr = 0;
	erc_idx =event[EVENT_POS_ERC_NO];
	tagERC_96bits_region erc_96bits;
	//从空闲块中随机选出一个块用来转存当前的erc。
	randsector =rand()%(EVENT_ERC_SECTOR_MAX-G_METER_ERC_RECORD_MAX);
	system_debug_info("the rand num is ");
	system_debug_data(&randsector,1);
	system_debug_info("\n");
	new_erc_store_sector = g_erc_EVENT.ERC_F3.sector_free_index[randsector];
	old_erc_store_sector = g_erc_EVENT.ERC_F3.sector_index[erc_idx];
	old_erc_store_cnt = g_erc_EVENT.ERC_F3.erc_cnt[erc_idx];
	nor_flash_erase_page(new_erc_store_sector+FLASH_ERTU_EVENT_ERC_STORE_START);
	if( old_erc_store_cnt == EVENT_MAX_SAVE_COUNT)
	{
			
		for(move_idx=1;move_idx<EVENT_MAX_SAVE_COUNT;move_idx++)
		{
			sector = old_erc_store_sector;
			move_addr =sector*FLASH_SECTOR_SIZE+FLASH_SECTOR_96BITS_LEAD+move_idx*EVENT_RECORD_SIZE;
			fread_erc_params(move_addr,event_record,EVENT_RECORD_SIZE);
			sector = new_erc_store_sector;
			system_debug_data(event_record,EVENT_RECORD_SIZE);
			move_addr =sector*FLASH_SECTOR_SIZE+FLASH_SECTOR_96BITS_LEAD+(move_idx-1)*EVENT_RECORD_SIZE;
			fwrite_erc_params(move_addr,event_record,EVENT_RECORD_SIZE);
		}
		sector = new_erc_store_sector;
		move_addr =sector*FLASH_SECTOR_SIZE+FLASH_SECTOR_96BITS_LEAD+(EVENT_MAX_SAVE_COUNT-1)*EVENT_RECORD_SIZE;
		fwrite_erc_params(move_addr,event,EVENT_RECORD_SIZE);
		erc_96bits.erc_no = erc_idx;
		erc_96bits.erc_cnt = EVENT_MAX_SAVE_COUNT;
		move_addr =sector*FLASH_SECTOR_SIZE;
		fwrite_erc_params(move_addr,(INT8U*)&erc_96bits,sizeof(tagERC_96bits_region));
		g_erc_EVENT.ERC_F3.sector_index[erc_idx] = new_erc_store_sector;
		g_erc_EVENT.ERC_F3.sector_free_index[randsector] = old_erc_store_sector;
		nor_flash_erase_page(old_erc_store_sector+FLASH_ERTU_EVENT_ERC_STORE_START);
			
	}
	else
	{
		for(move_idx=1;move_idx<old_erc_store_cnt;move_idx++)
		{
			sector = old_erc_store_sector;
			move_addr =sector*FLASH_SECTOR_SIZE+FLASH_SECTOR_96BITS_LEAD+move_idx*EVENT_RECORD_SIZE;
			system_debug_data(&move_addr,4);
			fread_erc_params(move_addr,event_record,EVENT_RECORD_SIZE);
			system_debug_data(event_record,EVENT_RECORD_SIZE);
			sector = new_erc_store_sector;
			move_addr =sector*FLASH_SECTOR_SIZE+FLASH_SECTOR_96BITS_LEAD+(move_idx-1)*EVENT_RECORD_SIZE;
			fwrite_erc_params(move_addr,event_record,EVENT_RECORD_SIZE);
		}
		sector = new_erc_store_sector;
		move_addr =sector*FLASH_SECTOR_SIZE+FLASH_SECTOR_96BITS_LEAD+(old_erc_store_cnt-1)*EVENT_RECORD_SIZE;
		fwrite_erc_params(move_addr,event,EVENT_RECORD_SIZE);
		erc_96bits.erc_no = erc_idx;
		erc_96bits.erc_cnt = old_erc_store_cnt + 1 ;
		move_addr =sector*FLASH_SECTOR_SIZE;
		fwrite_erc_params(move_addr,(INT8U*)&erc_96bits,sizeof(tagERC_96bits_region));
		g_erc_EVENT.ERC_F3.sector_index[erc_idx] = new_erc_store_sector;
		g_erc_EVENT.ERC_F3.erc_cnt[erc_idx] = erc_96bits.erc_cnt;
		g_erc_EVENT.ERC_F3.sector_free_index[randsector] = old_erc_store_sector;
		nor_flash_erase_page(old_erc_store_sector+FLASH_ERTU_EVENT_ERC_STORE_START);
		
	}
	
	
}

/*+++
  功能： 保存事件数据
  描述：
---*/
void save_event_record(INT8U *event,INT8U event_flag)
{
     INT32U offset,ec;
     INT32U *ec_list;
     INT16U idx,erc_count;
     INT16U event_file_id;
     INT16U *erc_list;
//   INT8U  erc_type;

    //记录当前事件

    //验证事件代码是否超出允许最大事件定义
    if(event[EVENT_POS_ERC_NO]>MAX_ERC_NO)return;
     tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
     tpos_mutexPend(&SIGNAL_EVENT);
     ec_list=(INT32U*)g_temp_buffer;
     erc_list=(INT16U*)(g_temp_buffer+60*4);
    g_event.alarm_event = event[EVENT_POS_ERC_NO];
    //当前待写入事件文件
    event_file_id=FLASH_ERTU_EVENT_ERC_STORE_START+event[EVENT_POS_ERC_NO]-1;  //找到待存储的文件号。
    //读取当前事件计数器
    ec = bin2_int32u(g_event.event_ec);
    if(ec==INVALID_EC_SEQ)ec=1;
    else ec++;
    int32u2_bin(ec,g_event.event_ec);

    //fwrite_array(FILEID_RUN_DATA,FLADDR_GLOBAL_EVENT+STRUCT_OFFSET(tagGlobalEVENT,event_ec),g_event.event_ec,4);


    mem_set(ec_list,60,0xFF);  //多用了？？
    mem_set(erc_list,30,0xFF);
    erc_count=fetch_event_index(ec_list,erc_list,EVENT_MAX_SAVE_COUNT,event[EVENT_POS_ERC_NO],EVENT_TYPE_ALL);  //啥意思????
    if(erc_count>=EVENT_MAX_SAVE_COUNT)
    {
        idx=erc_list[0] & 0xFF;//如果15个位置存满了，则覆盖最早存储位置
    }
    else
    {
        idx=erc_list[erc_count] & 0xFF;
    }
    if((idx & 0xFF)==0xFF)//未获取到有效地址，则使用默认地址
    {
        idx = erc_count;
    }
    else
    {
        idx--;
    }
    //1. 计算保存地址
    offset =  idx * EVENT_RECORD_SIZE;

    //fread_array(FILEID_EVENT_INDEX,(event[EVENT_POS_ERC_NO]-1)*15+idx,&erc_type,1);
    if(event_flag & VIPEVENT_FLAG)
    {
        //1.3 重要事件计数器加1
        event[EVENT_POS_EC]=g_event.ec1;

    }
    else
    {
        //1.2 一般事件计数器加1
        event[EVENT_POS_EC]=g_event.ec2;
    }
    //2. 保存事件记录
    mem_cpy(event,g_event.event_ec,4);
    event[EVENT_POS_RPT_FLAG]=0;
    if(event_flag & VIPEVENT_FLAG)
    {
//        event[EVENT_POS_RPT_FLAG] |=MASK_EVENT_FLAG_VIP;
    }
	
//将事件分类存储的F3中去	
    update_ERC_F3_sector(event,event[EVENT_POS_ERC_LEN]+EVENT_HEAD_LEN+2);
//    fwrite_array(event_file_id,offset,event,event[EVENT_POS_ERC_LEN]+EVENT_HEAD_LEN+2);


	
    if(event_flag & VIPEVENT_FLAG)
    {
        //1.3 重要事件计数器加1
//        fwrite_array(FILEID_EVENT_VIP,g_event.ec1*EVENT_RECORD_SIZE,event,event[EVENT_POS_ERC_LEN]+EVENT_HEAD_LEN+2);
        g_event.ec1++;
        //set_bit_value(&erc_type,8,0); //设置重要事件索引
        //set_bit_value(&erc_type,8,2); //设置事件需上报索引

    }
    else
    {
//        fwrite_array(FILEID_EVENT_GEN,g_event.ec2*EVENT_RECORD_SIZE,event,event[EVENT_POS_ERC_LEN]+EVENT_HEAD_LEN+2);
        g_event.ec2++;
        //set_bit_value(&erc_type,8,1);////设置一般事件索引
    }
    if((event_flag & VIPEVENT_FLAG) && !(event_flag & NO_REPORT_FLAG))
    {
        //1.4 设置请求访问标志

      g_event.report_flag[event[EVENT_POS_EC]]=event[EVENT_POS_ERC_NO];
      g_event.acd =  TRUE;

    }
    //保存事件索引
    //fwrite_array(FILEID_EVENT_INDEX,(event[EVENT_POS_ERC_NO]-1)*15+idx,&erc_type,1);

    //保存EC计数器和事件标志位
    set_bit_value(g_event.ercflag+(event[EVENT_POS_ERC_NO]-1)/8,8,(event[EVENT_POS_ERC_NO]-1) % 8);
//    fwrite_array(FILEID_RUN_DATA,FLADDR_GLOBAL_EVENT,(INT8U*)&g_event,sizeof(tagGlobalEVENT));
     tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
     tpos_mutexFree(&SIGNAL_EVENT);


}