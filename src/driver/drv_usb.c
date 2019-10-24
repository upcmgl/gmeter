/*
 * drv_usb.c
 *
 * Created: 2018/12/18 星期二 20:28:05
 *  Author: Administrator
 */ 

#include "asf.h"
#include "os.h"
#include "ff.h"
#include "dev_var.h"
#include "dev_driver.h"
#include "tops_fun_tools.h"
static volatile INT8U usb_plug, usb_unplug;
static volatile uhd_speed_t usb_speed_type;

//! To notify that the USB mode are switched automatically.
//! This is possible only when ID pin is available.
void ui_usb_mode_change(bool b_host_mode)
{
    UNUSED(b_host_mode);
}

//! To notify that the Vbus level has changed
//! Available only in USB hardware with Vbus monitoring.
void ui_usb_vbus_change(bool b_vbus_present)
{
    UNUSED(b_vbus_present);
}

//! To notify that a Vbus error has occurred
//! Available only in USB hardware with Vbus monitoring.
void ui_usb_vbus_error(void)
{
    
}

//! To notify that a device has been connected or disconnected.
void main_usb_connection_event(uhc_device_t * dev, bool b_present)
{
    if (!b_present) //unplug
    {
        usb_unplug = 0x1;
    }
    else//plug
    {
        usb_plug = 0x1;
    }
}

//! Called when a USB device or the host have wake up the USB line.
void ui_usb_wakeup_event(void)
{

}

//! Called for each received SOF each 1 ms
//! Note: Available in High and Full speed mode
void main_usb_sof_event(void)   
{

}

//! Called when a USB device configuration must be chosen.
//! Thus, the application can choose either a configuration number for this device
//! or a configuration number 0 to reject it.
//! If callback not defined the configuration 1 is chosen.
//#define UHC_DEVICE_CONF(dev)                uint8_t usb_device_conf(dev)

//! Called when a USB device enumeration is completed.

void ui_usb_enum_event(uhc_device_t *dev, uhc_enum_status_t status)
{
    usb_speed_type = dev->speed;
    switch (dev->speed) {
        case UHD_SPEED_HIGH:
            usb_plug = 0xA0;
        break;
        case UHD_SPEED_FULL:
            usb_plug = 0xA1;
        break;
        case UHD_SPEED_LOW:
            usb_plug = 0xA2;
        break;
        default:
            usb_plug = 0x0;
        break;
    }
}

BOOLEAN usb_file_process(void);
void LED_green_on(INT8U on);
FIL myFile;
INT16U read_usb_data(INT8U* filename,INT32U offset,INT8U* data,INT16U len)
{
	INT16U ret=0,count=0,pos=0,len_tmp=0;
	INT8U down_flag = 0,cnt=0;
	

	if(!len)
	{
		return 0;
	}
	if(filename==NULL)
	{
		filename="0:USB.dat";
	}	
	
    if(FR_OK != f_open_app(&myFile, filename, FA_READ))
	{
		if(FR_OK != f_open_app(&myFile, filename, FA_READ|FA_CREATE_NEW))
		{
			return 0;
		}
	}
	ret = f_lseek(&myFile,offset);
    if(ret != FR_OK)
    {
        return 0;
    }
	len_tmp = len;
    do
    {         
        ClrTaskWdt();
		if(len_tmp>512)
		{
			ret = f_read(&myFile, data+pos, 512, &count);
		}
		else
		{
			ret = f_read(&myFile, data+pos, len_tmp, &count);	
		}
        if(ret != FR_OK)
        {
            f_close(&myFile);
            return pos;
        } 
        len_tmp -= count;
		pos += count;

        //告警灯一个闪烁，表示U盘已经插入了。
        down_flag ++;
        if( (down_flag&0x0F) == 0x0F)      
        {
                //闪烁告警灯
            LED_comm_toggle();
        }
		if(pos==0) 
		{
			if(cnt++>10)
			{
				break;
			}
		}
    }while(len_tmp > 0);
	f_close(&myFile);	
	return pos;
}
INT16U app_read_usb_data(INT8U* filename,INT32U offset,INT8U* data,INT16U len)
{
	INT32U timer=0;
	
	timer = system_get_tick10ms();
	
	while(tpos_mutexRequest(&SIGNAL_SYSTEMINFO) == FALSE)
	{
		if((system_get_tick10ms()-timer) > 10*100)
		{
			return 0;
		}
		tpos_TASK_YIELD();
	}
	while(gSystemInfo.usb_read)
	{
		if((system_get_tick10ms() - timer) > 10*100)
		{
			//1分钟，放弃吧
			tpos_mutexFree(&SIGNAL_SYSTEMINFO);
			return 0;
		}
		DelayNmSec(10);

	}
	tpos_enterCriticalSection();
	gSystemInfo.filename = filename;
	gSystemInfo.offset = offset;
	gSystemInfo.data = data;
	gSystemInfo.len = len;
	gSystemInfo.usb_read=1;
	tpos_leaveCriticalSection();
	
	while(gSystemInfo.len)
	{
		if((system_get_tick10ms() - timer) > 10*100)
		{
			//1分钟，放弃吧
			gSystemInfo.filename = NULL;
			gSystemInfo.offset = 0;
			gSystemInfo.data = NULL;
			gSystemInfo.len = 0;
			gSystemInfo.usb_read = 0;
			tpos_mutexFree(&SIGNAL_SYSTEMINFO);
			return 0;
		}
		DelayNmSec(10);

	}
	return len;
}


INT16U app_write_usb_data(INT8U* filename,INT32U offset,INT8U* data,INT16U len)
{
	INT32U timer=0;
	
	timer = system_get_tick10ms();
	    
	while(tpos_mutexRequest(&SIGNAL_SYSTEMINFO) == FALSE)
	{
		if((system_get_tick10ms()-timer) > 10*100)
		{
			return 0;
		}
		tpos_TASK_YIELD();
	}
	while(gSystemInfo.usb_write)
	{
		if((system_get_tick10ms() - timer) > 10*100)
		{
			//1分钟，放弃吧
			tpos_mutexFree(&SIGNAL_SYSTEMINFO);
			return 0;
		}
		DelayNmSec(10);

	}
	tpos_enterCriticalSection();
	gSystemInfo.filename = filename;
	gSystemInfo.offset = offset;
	gSystemInfo.data = data;
	gSystemInfo.len = len;
	gSystemInfo.usb_write = 1;
	tpos_leaveCriticalSection();
	
	while(gSystemInfo.usb_write)
	{
		if((system_get_tick10ms() - timer) > 10*100)
		{
			//1分钟，放弃吧
			gSystemInfo.filename = NULL;
			gSystemInfo.offset = 0;
			gSystemInfo.data = NULL;
			gSystemInfo.len = 0;
			gSystemInfo.usb_write = 0;			
			tpos_mutexFree(&SIGNAL_SYSTEMINFO);
			return 0;
		}
		DelayNmSec(10);

	}
	return len;
} 


INT16U write_usb_data(INT8U* filename,INT32U offset,INT8U* data,INT16U len)
{
	INT16U ret=0,count=0,pos=0;
	INT8U down_flag = 0,len_tmp=0,cnt=0;

	if(!len)
	{
		return 0;
	}
	if(filename==NULL)
	{
		filename="0:USB.dat";
	}
	
	if(FR_OK != f_open_app(&myFile, filename, FA_WRITE))
	{
		if(FR_OK != f_open_app(&myFile, filename, FA_WRITE|FA_CREATE_NEW))
		{
			return 0;
		}		
	}		
	ret = f_lseek(&myFile,offset);
    if(ret != FR_OK)
    {
        return 0;
    }
	len_tmp = len;
    do
    {         
        ClrTaskWdt();
		if(len>512)
		{
			ret = f_write(&myFile, data+pos, 512, &count);
		}
		else
		{
			ret = f_write(&myFile, data+pos, len_tmp, &count);	
		}
        if(ret != FR_OK)
        {
            f_close(&myFile);
            return pos;
        } 
        len_tmp -= count;
		pos += count;

        //告警灯一个闪烁，表示U盘已经插入了。
        down_flag ++;
        if( (down_flag&0x0F) == 0x0F)      
        {
                //闪烁告警灯
            LED_comm_toggle();
        } 
		if(pos==0) 
		{
			if(cnt++>10)
			{
				break;
			}
		}
    }while(len_tmp > 0);
	f_close(&myFile);	
	return pos;	
}



void task_usb( void)
{
    FATFS fs; // Re-use fs for LUNs to reduce memory footprint
    //FIL file;
    FILINFO file_info;
    volatile FRESULT ret;
    UINT *count;
    volatile INT8U i;
    INT8U reset_flag = 0;

    while(1)
    {	
        if(usb_plug == 0)
        {
            tpos_yield();
            continue;
        }   
        i = 0;
        reset_flag = 0;
        //usb_plug = 0;
        usb_unplug = 0;
        while(1)
        {
            ret = f_mount(LUN_ID_USB, &fs);
            if(ret != FR_OK)
            {
                i++;
                if(i > 20)
                {
                    usb_plug = 0;
                    //goto F_EXIT;
                }
                msleep(20);
            }
            else
            {
                msleep(100);//wait steady
                reset_flag = usb_file_process();
                break;
            }            
        }
////////////////////////////
        //while((usb_plug==0xa1)&&(reset_flag !=1)&&(usb_unplug==0))
        //{
//#ifdef DEBUG
	//system_debug_info("=============in usb task s gmeter==============");
//#endif
			//gSystemInfo.usb_plug =1;
	    ////    ret = f_mount(LUN_ID_USB, &fs);
	        //if(ret != FR_OK)
	        //{
		        //i++;
		        //if(i > 20)
		        //{
			        //usb_plug = 0;
			        ////goto F_EXIT;
		        //}
		        //msleep(20);
	        //}
	        //else
	        //{			
		        //msleep(100);//wait steady
		        //usb_file_process_log();
		        //
	        //}
        //}
		//if(usb_unplug ==1)
		//{		
			//usb_plug=0;
			//output_flash_sector.flash_out_flag =0;
			//LED_green_on(0);
			//gSystemInfo.usb_plug =0;
		//}
///////////////////////////		
        if(reset_flag)
        {
			
            while(usb_unplug == 0)
            {
                msleep(100);
                LED_green_on(1);//绿灯长亮
				LED_local_on(1);
            }
			sys_reset();
        }
        else
		{
		
			if(gSystemInfo.usb_read == 1)
			{
				if(gSystemInfo.len == read_usb_data(gSystemInfo.filename,gSystemInfo.offset,gSystemInfo.data,gSystemInfo.len))
				{
					tpos_enterCriticalSection();
					gSystemInfo.len=0;
					gSystemInfo.offset=0;
					gSystemInfo.usb_read=0;
					tpos_leaveCriticalSection();
				}			
			}
			else if(gSystemInfo.usb_write == 1)
			{
				if(gSystemInfo.len == write_usb_data(gSystemInfo.filename,gSystemInfo.offset,gSystemInfo.data,gSystemInfo.len))
				{
					tpos_enterCriticalSection();
					gSystemInfo.len=0;
					gSystemInfo.offset=0;
					gSystemInfo.usb_write=0;
					tpos_leaveCriticalSection();
				}
			}
			else
			{
			}
		}
    //F_EXIT:
        //usb_plug = 0;
        //f_close(&file); 
                   
    }//while(1)

}

FRESULT f_open_app(
	FIL *fp,			/* Pointer to the blank file object */
	const TCHAR *path,	/* Pointer to the file name */
	BYTE mode			/* Access mode and file open mode flags */)
{
    INT8U cnt=0;
    FRESULT res=FR_NOT_READY;
    
    while(cnt < 10)
    {
        res = f_open(fp, path, mode);
        if(res != FR_NOT_READY)
        {
            return  res;
        }  
        cnt++;
        msleep(100);   
    }  
    return res;              
}