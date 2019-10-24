#include "main_include.h"
#include "stdio.h"
#include "string.h"

INT8U flash_swap_buf[256];
//存储文件函数，每个部分写独立的函数接口，判断自己使用的sector范围
void delete_update_file(void)
{
    INT16U idx;
    
    for(idx=FLASH_SECTOR_UPDATE_START;idx<=FLASH_SECTOR_UPDATE_END;idx++)
    {
        nor_flash_erase_page(idx);
    }
}
INT16U fread_array_update_info(INT16U addr,INT8U *buf,INT16U len)
{
    if((addr < FLADDR_PROG_INFO_START)||(addr > FLADDR_PROG_INFO_END))
    {
#ifdef __DEBUG    
    __builtin_software_breakpoint();
    /* If we are in debug mode, cause a software breakpoint in the debugger */
#endif
        mem_set(buf,len,0);
        return FILE_ERROR_SIZE;
    }
    if((addr+len) > FLADDR_PROG_INFO_END)
    {
#ifdef __DEBUG    
    __builtin_software_breakpoint();
    /* If we are in debug mode, cause a software breakpoint in the debugger */
#endif
        mem_set(buf,len,0);
        return FILE_ERROR_SIZE;
    }
    if(len != nor_flash_read_data(FLASH_SECTOR_UPDATE_INFO,addr,buf,len))
    {
#ifdef __DEBUG    
    __builtin_software_breakpoint();
    /* If we are in debug mode, cause a software breakpoint in the debugger */
#endif
        return FILE_ERROR_UNKOWN;
    }
    return FILE_SUCCESS;
}
INT16U fwrite_array_update_info(INT16U addr,INT8U *buf,INT16U len)
{
    if((addr < FLADDR_PROG_INFO_START)||(addr > FLADDR_PROG_INFO_END))
    {
#ifdef __DEBUG    
    __builtin_software_breakpoint();
    /* If we are in debug mode, cause a software breakpoint in the debugger */
#endif
        return FILE_ERROR_SIZE;
    }
    if((addr+len) > FLADDR_PROG_INFO_END)
    {
#ifdef __DEBUG    
    __builtin_software_breakpoint();
    /* If we are in debug mode, cause a software breakpoint in the debugger */
#endif
        return FILE_ERROR_SIZE;
    }
    if(len != nor_flash_write_data(FLASH_SECTOR_UPDATE_INFO,addr,buf,len))
    {
#ifdef __DEBUG    
    __builtin_software_breakpoint();
    /* If we are in debug mode, cause a software breakpoint in the debugger */
#endif
        return FILE_ERROR_UNKOWN;
    }
    return FILE_SUCCESS;
}
INT16U fread_array_update_data(INT32U addr,INT8U *buf,INT16U len)
{
    INT16U sector;
    INT16U offset;
    
    sector = addr/FLASH_SECTOR_SIZE;
    offset = addr%FLASH_SECTOR_SIZE;
    sector += FLASH_SECOTR_UPDATE_DATA_START;
    if((sector < FLASH_SECOTR_UPDATE_DATA_START)||(sector > FLASH_SECOTR_UPDATE_DATA_EDN))
    {
#ifdef __DEBUG    
    __builtin_software_breakpoint();
    /* If we are in debug mode, cause a software breakpoint in the debugger */
#endif
        mem_set(buf,len,0);
        return FILE_ERROR_SIZE;
    }
    if(len != nor_flash_read_data(sector,offset,buf,len))
    {
#ifdef __DEBUG    
    __builtin_software_breakpoint();
    /* If we are in debug mode, cause a software breakpoint in the debugger */
#endif
        return FILE_ERROR_UNKOWN;
    }
    return FILE_SUCCESS;
}
INT16U fwrite_array_update_data(INT32U addr,INT8U *buf,INT16U len)
{
    INT16U sector;
    INT16U offset;
    
    sector = addr/FLASH_SECTOR_SIZE;
    offset = addr%FLASH_SECTOR_SIZE;
    sector += FLASH_SECOTR_UPDATE_DATA_START;
    if((sector < FLASH_SECOTR_UPDATE_DATA_START)||(sector > FLASH_SECOTR_UPDATE_DATA_EDN))
    {
#ifdef __DEBUG    
    __builtin_software_breakpoint();
    /* If we are in debug mode, cause a software breakpoint in the debugger */
#endif
        return FILE_ERROR_SIZE;
    }
    if(len != nor_flash_write_data(sector,offset,buf,len))
    {
#ifdef __DEBUG    
    __builtin_software_breakpoint();
    /* If we are in debug mode, cause a software breakpoint in the debugger */
#endif
        return FILE_ERROR_UNKOWN;
    }
    return FILE_SUCCESS;
}

//双备份切换相关接口
INT8U _file_flash_safe_write(INT16U sector,INT16U offset,INT8U *data,INT16U len)
{
    INT32U version1;
    INT32U version2;
    INT32U version_new;
    INT16U sector_old;
    INT16U sector_new;
    INT16U cur_offset;
    INT16U idx;
    INT8U  tmp[4];
    tagDatetime datetime;
    
    offset += 96;
    
    nor_flash_read_data(sector,0,tmp,4);
    mem_cpy((INT8U*)&version1,tmp,4);
    nor_flash_read_data(sector+1,0,tmp,4);
    mem_cpy((INT8U*)&version2,tmp,4);
    
    
    if(    ((version2 > version1)&&(version2 != 0xFFFFFFFF))
         ||(version1 == 0xFFFFFFFF))
    {
        //后边的sector是新的，那么更新旧的
        //sector_op不变
        sector_old = sector;
        sector_new = sector+1;
        version_new = version2;
    }
    else
    {
        //前边的sector是新的，更新后边的
        sector_old = sector+1;
        sector_new = sector;
        version_new = version1;
    }
    //万一4字节的version循环到了0xFFFFFFFE，要采取措施
    if(version_new == 0xFFFFFFFE)
    {
        //策略，把小值写为0，大值写为一个非0的最小数，这里使用覆盖写
        mem_set(tmp,4,0);
        nor_flash_write_data(sector_old,0,tmp,4);
        mem_cpy(tmp,(INT8U*)&version_new,4);
        for(idx=32;idx>0;idx++)
        {
            if(get_bit_value(tmp,4,idx))
            {
                clr_bit_value(tmp,4,idx);
                if(check_is_all_ch(tmp,4,0))
                {
                    break;
                }
                mem_cpy((INT8U*)&version_new,tmp,4);
            }
        }
        mem_cpy(tmp,(INT8U*)&version_new,4);
        nor_flash_write_data(sector_new,0,tmp,4);
    }
    //验证能否重写，可以重写就重写
    {
        //对于参数来说，寿命足够，暂不考虑重写了，后期考虑增加sector校验，如果有校验，就不能重写
    }
    version_new ++;
    nor_flash_erase_page(sector_old);
    for(cur_offset=0;cur_offset<FLASH_SECTOR_SIZE;cur_offset+=256)
    {
        nor_flash_read_data(sector_new,cur_offset,flash_swap_buf,256);
        if(cur_offset == 0)
        {
            mem_set(flash_swap_buf,96,0xFF);
        }
        for(idx=cur_offset;idx<(cur_offset+256);idx++)
        {
            if((idx >= offset)&&(idx<(offset+len)))
            {
                flash_swap_buf[idx-cur_offset] = *data;
                data ++;
            }
        }
        nor_flash_write_data(sector_old,cur_offset,flash_swap_buf,256);
    }
    os_get_datetime(&datetime);
    mem_cpy(flash_swap_buf,(INT8U*)&version_new,4);
    flash_swap_buf[4] = datetime.second;
    flash_swap_buf[5] = datetime.minute;
    flash_swap_buf[6] = datetime.hour;
    flash_swap_buf[7] = datetime.day;
    flash_swap_buf[8] = datetime.month;
    flash_swap_buf[9] = datetime.year;
    nor_flash_write_data(sector_old,0,flash_swap_buf,4);
    return FILE_SUCCESS;
}
INT8U _file_flash_safe_read(INT16U sector,INT16U offset,INT8U *data,INT16U len)
{
    INT32U version1;
    INT32U version2;
    INT16U sector_new;
    INT8U  tmp[4];
    
    offset += 96;
    
    nor_flash_read_data(sector,0,tmp,4);
    mem_cpy((INT8U*)&version1,tmp,4);
    nor_flash_read_data(sector+1,0,tmp,4);
    mem_cpy((INT8U*)&version2,tmp,4);
    
    if(    ((version2 > version1)&&(version2 != 0xFFFFFFFF))
         ||(version1 == 0xFFFFFFFF))
    {
        //后边的sector是新的，那么更新旧的
        sector_new = sector+1;
    }
    else
    {
        //前边的sector是新的，更新后边的
        sector_new = sector;
    }
    nor_flash_read_data(sector_new,offset,data,len);
    return FILE_SUCCESS;
}
//终端参数相关操作接口
INT8U  fwrite_ertu_params(INT32U addr,INT8U *data,INT16U len)
{
    INT16U sector;
    INT16U offset;
    INT16U write_len;
    
    if(   (addr >= FLASH_ERTU_PARAM_MAX_LENTH)
        ||((addr+len)>=FLASH_ERTU_PARAM_MAX_LENTH) )
    {
        return FILE_ERROR_SIZE;
    }

    tpos_mutexPend(&SIGNAL_FILE_OP);
    while(len > 0)
    {
        sector = addr / FLASH_SECTOR_DATA_SIZE;
        offset = addr % FLASH_SECTOR_DATA_SIZE;
        if((len+offset) > FLASH_SECTOR_DATA_SIZE)
        {
            write_len = FLASH_SECTOR_DATA_SIZE-offset;
        }
        else
        {
            write_len = len;
        }
        sector *= 2;
        sector += FLASH_SECTOR_ERTU_PARAM_START;
        _file_flash_safe_write(sector,offset,data,write_len);        
        len -= write_len;
        addr += write_len;
    }
    tpos_mutexFree(&SIGNAL_FILE_OP);
    return FILE_SUCCESS;
}
INT8U  fread_ertu_params(INT32U addr,INT8U *data,INT16U len)
{
    INT16U sector;
    INT16U offset;
    INT16U write_len;
    
    if(   (addr >= FLASH_ERTU_PARAM_MAX_LENTH)
        ||((addr+len)>=FLASH_ERTU_PARAM_MAX_LENTH) )
    {
        return FILE_ERROR_SIZE;
    }

    tpos_mutexPend(&SIGNAL_FILE_OP);
    while(len > 0)
    {
        sector = addr / FLASH_SECTOR_DATA_SIZE;
        offset = addr % FLASH_SECTOR_DATA_SIZE;
        if((len+offset) > FLASH_SECTOR_DATA_SIZE)
        {
            write_len = FLASH_SECTOR_DATA_SIZE-offset;
        }
        else
        {
            write_len = len;
        }
        sector *= 2;
        sector += FLASH_SECTOR_ERTU_PARAM_START;
        _file_flash_safe_read(sector,offset,data,write_len);   
		data +=write_len;     
        len -= write_len;
        addr += write_len;
    }
    tpos_mutexFree(&SIGNAL_FILE_OP);
    return FILE_SUCCESS;
}
#ifdef __13761__
//三类数据ERC相关操作接口,不使用双备份，之所以再封装一下就是考虑到需要这个操作flash的信号量
INT8U fwrite_erc_params(INT32U addr,INT8U *data,INT16U len)
{
	INT16U sector;
	INT16U offset;
	
	tpos_mutexPend(&SIGNAL_FILE_OP);
	
	sector = addr / FLASH_SECTOR_SIZE;
	offset = addr % FLASH_SECTOR_SIZE;
	sector += FLASH_ERTU_EVENT_ERC_START;
	nor_flash_write_data(sector,offset,data,len);

	tpos_mutexFree(&SIGNAL_FILE_OP);
	return FILE_SUCCESS;
}
INT8U fread_erc_params(INT32U addr,INT8U *data,INT16U len)
{
	INT16U sector;
	INT16U offset;

	tpos_mutexPend(&SIGNAL_FILE_OP);
	
	sector = addr / FLASH_SECTOR_SIZE;
	offset = addr % FLASH_SECTOR_SIZE;
	sector += FLASH_ERTU_EVENT_ERC_START;
	nor_flash_read_data(sector,offset,data,len);

	tpos_mutexFree(&SIGNAL_FILE_OP);
	return FILE_SUCCESS;
}
#endif


























//void uart_deug_info(char * info)
//{
//    while(meter_uart_is_idle()==0);
//    meter_uart_send_buf((INT8U*)info,strlen(info));
//    while(meter_uart_is_idle()==0);
//}
//void uart_deug_data(INT8U *data,INT16U len)
//{
//    while(meter_uart_is_idle()==0);
//    meter_uart_send_buf(data,len);
//    while(meter_uart_is_idle()==0);
//}
//
//INT8U test_data[4096];
//char debug_info[100];
//void flash_test(void)
//{
//    INT16U sector_idx;
//    INT16U idx;
//    
//    tpos_sleep(500);
//    uart_deug_info("flash test start!");
//    tpos_sleep(500);
//    ClrTaskWdt();
//    uart_deug_info("flash erase test start!");
//    tpos_sleep(500);
//    for(sector_idx=0;sector_idx<1024;sector_idx++)
//    {
//        ClrTaskWdt();
//        nor_flash_erase_page(sector_idx);
//        nor_flash_read_data(sector_idx,0,test_data,4096);
//        for(idx=0;idx<4096;idx++)
//        {
//            if(test_data[idx]!= 0xFF)
//            {
//                sprintf(debug_info,"erase test fail sector-%d",sector_idx);
//                uart_deug_info(debug_info);
//                Nop();
//                break;
//            }
//        }
//        if(idx>=4096)
//        {
//                sprintf(debug_info,"erase test good sector-%d",sector_idx);
//                uart_deug_info(debug_info);
//                Nop();
//        }
//        tpos_sleep(50);
//    }
//    tpos_sleep(500);
//    uart_deug_info("flash write test start!");
//    tpos_sleep(500);
//    for(sector_idx=0;sector_idx<1024;sector_idx++)
//    {
//        ClrTaskWdt();
////        nor_flash_erase_page(sector_idx);
//        
//        for(idx=0;idx<4096;idx+=2)
//        {
//            test_data[idx] = sector_idx&0xFF;
//            test_data[idx+1] = sector_idx>>8;
//        }
//        nor_flash_write_data(sector_idx,0,test_data,4096);
//        nor_flash_read_data(sector_idx,0,test_data,4096);
//        for(idx=0;idx<4096;idx+=2)
//        {
//            if( (test_data[idx]!= (sector_idx&0xFF))
//               ||(test_data[idx+1]!= (sector_idx>>8))  )
//            {
//                sprintf(debug_info,"write test fail sector-%d",sector_idx);
//                uart_deug_info(debug_info);
//                Nop();
//                break;
//            }
//        }
//        if(idx>=4096)
//        {
//                sprintf(debug_info,"write test good sector-%d",sector_idx);
//                uart_deug_info(debug_info);
//                Nop();
//        }
//        tpos_sleep(50);
//    }
//    tpos_sleep(500);
//    uart_deug_info("flash write check test start!");
//    tpos_sleep(500);
//    for(sector_idx=0;sector_idx<1024;sector_idx++)
//    {
//        ClrTaskWdt();
//        nor_flash_read_data(sector_idx,0,test_data,4096);
//        for(idx=0;idx<4096;idx+=2)
//        {
//            if( (test_data[idx]!= (sector_idx&0xFF))
//               ||(test_data[idx+1]!= (sector_idx>>8))  )
//            {
//                sprintf(debug_info,"write check test fail sector-%d",sector_idx);
//                uart_deug_info(debug_info);
//                Nop();
//                break;
//            }
//        }
//        if(idx>=4096)
//        {
//                sprintf(debug_info,"write check test good sector-%d",sector_idx);
//                uart_deug_info(debug_info);
//                Nop();
//        }
//        tpos_sleep(50);
//    }
//    uart_deug_info("flash test complete!");
//}