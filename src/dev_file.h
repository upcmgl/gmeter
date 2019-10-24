#ifndef __DEV_FILE_H__
#define __DEV_FILE_H__

#define  FILE_SUCCESS                  0x00
#define  FILE_ERROR_SIZE               0x01
#define  FILE_ERROR_UNKOWN             0x02
#define  FALSE_SECTOR_BITS             (4096)
/*
 * 存储规划
 * 存储按照sector来规划，一个sector为4K，整个flash为4Mbyte，一共1024个sector
 * 把存储按照sector分区使用，划分分区大小必须是整个sector
 * 
 */

//sector 范围 0~1023
//0~89  升级使用
//90~101 参数使用，使用双备份取最新机制，90~93，4个sector存放不初始化参数，参数长度2个sector（4000*2=8000），94~101,8个sector参数可初始化参数，参数长度4个sector（4000*4=16000）

/*升级部分，程序标识512byte，升级程序大小0x56200（0x200文件头信息+0x56000程序）（512字节头信息+344k程序）
 * 一共需要大小345KByte  87个sector
 * 预留3个sector
 * 升级部分一共需要90个sector
 * MX 三相表程序为512.5K，总共需要133个sector
 */
#define FLASH_SECTOR_UPDATE_START             0

#define FLASH_SECTOR_UPDATE_END               (129)

//升级数据 G55 第一个sector开始，前512字节存放Header，后面紧跟着代码。G55一共512K字节Flash，NorFlash中预留 129*4096=516K字节
#define FLASH_SECOTR_UPDATE_DATA_START        (0)
#define FLASH_SECOTR_UPDATE_DATA_EDN          (128)
//升级信息 G55 放到代码后面的一个sector
#define FLASH_SECTOR_UPDATE_INFO              (129)

#define FLADDR_PROG_INFO_START                (0UL)   //升级附属信息开始地址
#define FLADDR_PROG_INFO_FLAG                 FLADDR_PROG_INFO_START            //32=16+16,16字节标识，4字节升级次数，12字节预留信息
#define FLADDR_PROG_FILE_TYPE                 (FLADDR_PROG_INFO_FLAG+32)          //2,文件性质
#define FLADDR_PROG_TOTAL_SEC_COUNT           (FLADDR_PROG_FILE_TYPE+2)         //2字节，总段数
#define FLADDR_PROG_ONE_SEC_SIZE              (FLADDR_PROG_TOTAL_SEC_COUNT+2)  //2字节，每段长度
#define FLADDR_PROG_FILE_SIZE                 (FLADDR_PROG_ONE_SEC_SIZE+2)  //4字节，文件大小
#define FLADDR_PROG_FRAME_LEN                 (FLADDR_PROG_FILE_SIZE+4)        //2字节，帧长度
#define FLADDR_PROG_END_FRAME_LEN             (FLADDR_PROG_FRAME_LEN+2)        //2字节，最后一帧长度
#define FLADDR_UPDATE_TIME                    (FLADDR_PROG_END_FRAME_LEN+20)              //6,升级开始时间
#define FLADDR_PROG_DOWNLOAD_RECORD           (FLADDR_UPDATE_TIME+6)           //512字节，按位表示每段下载标志，1未下载，0已下载，可表示4096个段，如果一个段   1024字节可表示4M，512字节，可表示2M，256字节，1M

#define FLADDR_PROG_INFO_END                  (4095)  //升级信息结束地址


/*参数存储部分
 *存储规划，采用双备份取最新机制，每个sector4096，前96byte做管理用，4000字节做数据
 * 不可初始化参数8000,2个sector大小，实际占用4个sector
 * 可初始化参数16000,4个sector大小，实际占用8个sector
 */
#define FLASH_SECTOR_DATA_SIZE                4000UL   //flash一个sector存放的数据大小，一个sector物理上4096，前边96存放管理信息，后边4000做数据
#define FLASH_SECTOR_ERTU_PARAM_START         (130)

//不可初始化参数4个
#define FLASH_SECTOR_ERTU_PARAM_NOINIT_START  (FLASH_SECTOR_ERTU_PARAM_START)
#define FLASH_SECOTR_ERTU_PARAM_NOINIT_END    (FLASH_SECTOR_ERTU_PARAM_START+3)
#ifdef __OOP_G_METER__
#define FLASH_SECTOR_ERTU_PARAM_INIT_START  (FLASH_SECOTR_ERTU_PARAM_NOINIT_END+1) //
#define FLASH_SECTOR_ERTU_PARAM_INIT_END    (FLASH_SECTOR_ERTU_PARAM_INIT_START+11) // OOP存储数据越线了，所以扩了俩页，。
#else
#define FLASH_SECTOR_ERTU_PARAM_INIT_START  (FLASH_SECOTR_ERTU_PARAM_NOINIT_END+1) //
#define FLASH_SECTOR_ERTU_PARAM_INIT_END    (FLASH_SECTOR_ERTU_PARAM_INIT_START+7) //
#endif
#define FLASH_SECTOR_ERTU_PARAM_END           (FLASH_SECTOR_ERTU_PARAM_INIT_END)

#define FLASH_ERTU_PARAM_MAX_LENTH            ((FLASH_SECTOR_ERTU_PARAM_END+1-FLASH_SECTOR_ERTU_PARAM_START)/2*FLASH_SECTOR_DATA_SIZE) 
#ifdef __13761__
//三类数据ERC存储规划
#define FLASH_ERTU_EVENT_ERC_START			(FLASH_SECTOR_ERTU_PARAM_END+1)
#define FLASH_ERTU_EVENT_ERC_VIP_START      (FLASH_ERTU_EVENT_ERC_START) //重要文件10个sector
#define FLASH_ERTU_EVENT_ERC_VIP_END        (FLASH_ERTU_EVENT_ERC_VIP_START+9)
#define FLASH_ERTU_EVENT_ERC_GEN_START       (FLASH_ERTU_EVENT_ERC_VIP_END+1) //一般文件10个sector
#define FLASH_ERTU_EVENT_ERC_GEN_END         (FLASH_ERTU_EVENT_ERC_GEN_START+9)
#define FLASH_ERTU_EVENT_ERC_STORE_START    (FLASH_ERTU_EVENT_ERC_GEN_END+1)  //三类数据F330+10个sector。
#define FLASH_ERTU_EVENT_ERC_STORE_END      (FLASH_ERTU_EVENT_ERC_STORE_START+EVENT_ERC_SECTOR_MAX-1) //EVENT_ERC_SECTOR_MAX个
//#define FLASH_ERTU_EVENT_ERC_STORE_LENGTH    ((FLASH_ERTU_EVENT_ERC_STORE_END+1-FLASH_ERTU_EVENT_ERC_VIP_START)/2*FLASH_SECTOR_DATA_SIZE)
#endif

#ifdef __EDP__   //存储edp未上报成功的数据。
#define  FLASH_EDP_STORT_DATA_START            (500)
#define  FLASH_EDP_STORT_DATA_SECTOR_CNT        (10)
#define  FLASH_EDP_STORT_DATA_START_END        (FLASH_EDP_STORT_DATA_START+FLASH_EDP_STORT_DATA_SECTOR_CNT)
#define  FLASH_EDP_RAND_REPORT_STORE_START            (FLASH_EDP_STORT_DATA_START_END) //  从510开始存储随机间隔上报数据。
#define  FLASH_EDP_RAND_REPORT_STORE_CNT			(5)
#define  FLASH_EDP_RAND_REPORT_STORE_END           (FLASH_EDP_RAND_REPORT_STORE_START + FLASH_EDP_RAND_REPORT_STORE_CNT)
#endif 

#ifdef __OOP_G_METER__
#define PAGE_ID_DATA_BUFFER               (490)    //数据搬移缓存页
    #define PAGE_COUNT_DATA_BUFFER         1
/* 	电表数据存储起始页 */
#define FLASH_SECTOR_METER_DATA_START      500    //这里开始是电表数据区  

#define PAGE_ID_METER_DATA_START           FLASH_SECTOR_METER_DATA_START    
#define MAX_ONE_METER_DATA_PAGE_COUNT      475    //电表的数据占用的页数

#ifdef __PROVINCE_JIBEI_FUHEGANZHI__ //冀北负荷感知项目
#define  ELECTRICAL_EQUIPMENT_NUM 30
#define  SECTOR_ELECTRICAL_EQUIPMENT_MINUTE_DATA_START  250
#define  MAX_SECTOR_ELECTRICAL_EQUIPMENT_MINUTE_DATA      2   //对于冀北负荷感知项目，每个设备分20个sector，假设支持有10个设备
#define  SECTOR_ELECTRICAL_EQUIPMENT_MINUTE_DATA_END    SECTOR_ELECTRICAL_EQUIPMENT_MINUTE_DATA_START+MAX_SECTOR_ELECTRICAL_EQUIPMENT_MINUTE_DATA*ELECTRICAL_EQUIPMENT_NUM
#endif
/* 电表数据存储页具体定义 
 * ===============================================================================
 */
//电表数据存储规划，每个电表占用MAX_ONE_METER_DATA_PAGE_COUNT页--------------------------------------------------------------------------------------
//电表实时数据
#define PAGE_ID_CUR_DL_1                  0          //10页， ---500
    #define PAGE_COUNT_CUR_DL_1          2
#define PAGE_ID_CUR_DL_2            (PAGE_ID_CUR_DL_1 + PAGE_COUNT_CUR_DL_1)       //数据块按80字节计算，使用2页存储，按1分钟周期可使用18年
    #define PAGE_COUNT_CUR_DL_2          2
#define PAGE_ID_CUR_DL_3            (PAGE_ID_CUR_DL_2 + PAGE_COUNT_CUR_DL_2)
    #define PAGE_COUNT_CUR_DL_3          2
#define PAGE_ID_CUR_DL_4            (PAGE_ID_CUR_DL_3 + PAGE_COUNT_CUR_DL_3)
    #define PAGE_COUNT_CUR_DL_4          2
#define PAGE_ID_CUR_DL_5            (PAGE_ID_CUR_DL_4 + PAGE_COUNT_CUR_DL_4)
    #define PAGE_COUNT_CUR_DL_5          2
#define PAGE_ID_CUR_DL_6            (PAGE_ID_CUR_DL_5 + PAGE_COUNT_CUR_DL_5)
    #define PAGE_COUNT_CUR_DL_6          2
#define PAGE_ID_CUR_DL_7            (PAGE_ID_CUR_DL_6 + PAGE_COUNT_CUR_DL_6)
    #define PAGE_COUNT_CUR_DL_7          2

#define PAGE_ID_CUR_XL_1            (PAGE_ID_CUR_DL_7 + PAGE_COUNT_CUR_DL_7)   //14   ---514
    #define PAGE_COUNT_CUR_XL_1                2
#define PAGE_ID_CUR_XL_2            (PAGE_ID_CUR_XL_1 + PAGE_COUNT_CUR_XL_1)
    #define PAGE_COUNT_CUR_XL_2                2
#define PAGE_ID_CUR_XL_3            (PAGE_ID_CUR_XL_2 + PAGE_COUNT_CUR_XL_2)
    #define PAGE_COUNT_CUR_XL_3                2
#define PAGE_ID_CUR_XL_4            (PAGE_ID_CUR_XL_3 + PAGE_COUNT_CUR_XL_3)
    #define PAGE_COUNT_CUR_XL_4                2
#define PAGE_ID_CUR_XL_5            (PAGE_ID_CUR_XL_4 + PAGE_COUNT_CUR_XL_4)
    #define PAGE_COUNT_CUR_XL_5                2
#define PAGE_ID_CUR_XL_6            (PAGE_ID_CUR_XL_5 + PAGE_COUNT_CUR_XL_5)
    #define PAGE_COUNT_CUR_XL_6                2
#define PAGE_ID_CUR_XL_7            (PAGE_ID_CUR_XL_6 + PAGE_COUNT_CUR_XL_6)
    #define PAGE_COUNT_CUR_XL_7                2
#define PAGE_ID_CUR_XL_8            (PAGE_ID_CUR_XL_7 + PAGE_COUNT_CUR_XL_7)
    #define PAGE_COUNT_CUR_XL_8                2

#define PAGE_ID_VARIABLE_1            (PAGE_ID_CUR_XL_8 + PAGE_COUNT_CUR_XL_8)  //30   ----530
    #define PAGE_COUNT_VARIABLE_1           4
#define PAGE_ID_VARIABLE_2            (PAGE_ID_VARIABLE_1 + PAGE_COUNT_VARIABLE_1) //34
    #define PAGE_COUNT_VARIABLE_2           3
#define PAGE_ID_VARIABLE_3            (PAGE_ID_VARIABLE_2 + PAGE_COUNT_VARIABLE_2)  //37
    #define PAGE_COUNT_VARIABLE_3           3

#define PAGE_ID_PARAMS_1               (PAGE_ID_VARIABLE_3 + PAGE_COUNT_VARIABLE_3) //40
    #define PAGE_COUNT_PARAMS_1             2

//电表冻结数据
#define PAGE_ID_HOLD_IDX                  (PAGE_ID_PARAMS_1 + PAGE_COUNT_PARAMS_1)      // 2页,冻结索引 //  ---542
    #define PAGE_COUNT_HOLD_IDX                   2                 // 2页，冻结索引

#define PAGE_ID_DAY_HOLD_1                (PAGE_ID_HOLD_IDX + PAGE_COUNT_HOLD_IDX)       //日冻结  //44
    #define PAGE_COUNT_DAY_HOLD_1                 3
#define PAGE_ID_DAY_HOLD_2                (PAGE_ID_DAY_HOLD_1 + PAGE_COUNT_DAY_HOLD_1)    //47
    #define PAGE_COUNT_DAY_HOLD_2                 8
#define PAGE_ID_DAY_HOLD_3                (PAGE_ID_DAY_HOLD_2 + PAGE_COUNT_DAY_HOLD_2)   //55
    #define PAGE_COUNT_DAY_HOLD_3                 5

// 月冻结数据 
#define PAGE_ID_MONTH_HOLD_1                (PAGE_ID_DAY_HOLD_3 + PAGE_COUNT_DAY_HOLD_3)   //60
    #define PAGE_COUNT_MONTH_HOLD_1               2
#define PAGE_ID_MONTH_HOLD_2                (PAGE_ID_MONTH_HOLD_1 + PAGE_COUNT_MONTH_HOLD_1) //62
    #define PAGE_COUNT_MONTH_HOLD_2               2
#define PAGE_ID_MONTH_HOLD_3                (PAGE_ID_MONTH_HOLD_2 + PAGE_COUNT_MONTH_HOLD_2)  //64
    #define PAGE_COUNT_MONTH_HOLD_3               2    

// 小时冻结     
#define PAGE_ID_HOUR_HOLD                 (PAGE_ID_MONTH_HOLD_3 + PAGE_COUNT_MONTH_HOLD_3)   //小时冻结 目前不抄读小时冻结，***暂时先空着***//-----566
    #define PAGE_COUNT_HOUR_HOLD                  3
//分钟冻结
#define PAGE_ID_MINUTE_HOLD               (PAGE_ID_HOUR_HOLD + PAGE_COUNT_HOUR_HOLD)   //15分钟冻结     按照7天  //69   ----569
    #define PAGE_COUNT_MINUTE_HOLD                21

#define PAGE_ID_ACCOUNT_DAY_HOLD_1        (PAGE_ID_MINUTE_HOLD + PAGE_COUNT_MINUTE_HOLD)     //结算日冻结  // 90
    #define PAGE_COUNT_ACCOUNT_DAY_HOLD_1          2
#define PAGE_ID_ACCOUNT_DAY_HOLD_2        (PAGE_ID_ACCOUNT_DAY_HOLD_1 + PAGE_COUNT_ACCOUNT_DAY_HOLD_1)   //92
    #define PAGE_COUNT_ACCOUNT_DAY_HOLD_2          2

#define PAGE_ID_TIME_ZONE_HOLD            (PAGE_ID_ACCOUNT_DAY_HOLD_2 + PAGE_COUNT_ACCOUNT_DAY_HOLD_2)  //时区表切换  //94
    #define PAGE_COUNT_TIME_ZONE_HOLD              2

#define PAGE_ID_DAY_SEG_HOLD              (PAGE_ID_TIME_ZONE_HOLD + PAGE_COUNT_TIME_ZONE_HOLD)  //日时段表切换   //94
    #define PAGE_COUNT_DAY_SEG_HOLD                2

#define PAGE_ID_FL_PRICE_HOLD             (PAGE_ID_DAY_SEG_HOLD + PAGE_COUNT_DAY_SEG_HOLD)   //费率电价切
    #define PAGE_COUNT_FL_PRICE_HOLD               2

#define PAGE_ID_LADDER_HOLD               (PAGE_ID_FL_PRICE_HOLD + PAGE_COUNT_FL_PRICE_HOLD)  //阶梯切换
    #define PAGE_COUNT_LADDER_HOLD                 2

//EVENT数据定义
#define PAGE_ID_EVENT_READ_STATE                 (PAGE_ID_LADDER_HOLD + PAGE_COUNT_LADDER_HOLD)       //2页,3320,事件的可读性
    #define PAGE_COUNT_EVENT_READ_STATE                  2                 //2页,3320,事件的可读性

#define PAGE_ID_EVENT_IDX                        (PAGE_ID_EVENT_READ_STATE + PAGE_COUNT_EVENT_READ_STATE)       //2页，冻结索引
    #define PAGE_COUNT_EVENT_IDX                         2                 //2页，冻结索引

#define PAGE_ID_EVENT_CUR_DATA_1                 (PAGE_ID_EVENT_IDX + PAGE_COUNT_EVENT_IDX)
    #define PAGE_COUNT_EVENT_CUR_DATA_1                  2
#define PAGE_ID_EVENT_CUR_DATA_2                 (PAGE_ID_EVENT_CUR_DATA_1 + PAGE_COUNT_EVENT_CUR_DATA_1)
    #define PAGE_COUNT_EVENT_CUR_DATA_2                  2

//失压、欠压、过压、断相为一组
#define PAGE_ID_EVENT_3000_REOCRD_DATA           (PAGE_ID_EVENT_CUR_DATA_2 + PAGE_COUNT_EVENT_CUR_DATA_2)                   //失压 *3
    #define PAGE_COUNT_EVENT_3000_REOCRD_DATA          2
#define PAGE_ID_EVENT_3001_REOCRD_DATA           (PAGE_ID_EVENT_3000_REOCRD_DATA + PAGE_COUNT_EVENT_3000_REOCRD_DATA*3)     //欠压 *3
    #define PAGE_COUNT_EVENT_3001_REOCRD_DATA          2
#define PAGE_ID_EVENT_3002_REOCRD_DATA           (PAGE_ID_EVENT_3001_REOCRD_DATA + PAGE_COUNT_EVENT_3001_REOCRD_DATA*3)     //过压 *3
    #define PAGE_COUNT_EVENT_3002_REOCRD_DATA          2
#define PAGE_ID_EVENT_3003_REOCRD_DATA           (PAGE_ID_EVENT_3002_REOCRD_DATA + PAGE_COUNT_EVENT_3002_REOCRD_DATA*3)     //断相 *3
    #define PAGE_COUNT_EVENT_3003_REOCRD_DATA          2

//失流、过流、断流为一组
#define PAGE_ID_EVENT_3004_REOCRD_DATA           (PAGE_ID_EVENT_3003_REOCRD_DATA + PAGE_COUNT_EVENT_3003_REOCRD_DATA*3)     //失流 *3
    #define PAGE_COUNT_EVENT_3004_REOCRD_DATA          2
#define PAGE_ID_EVENT_3005_REOCRD_DATA           (PAGE_ID_EVENT_3004_REOCRD_DATA + PAGE_COUNT_EVENT_3004_REOCRD_DATA*3)     //过流 *3
    #define PAGE_COUNT_EVENT_3005_REOCRD_DATA          2
#define PAGE_ID_EVENT_3006_REOCRD_DATA           (PAGE_ID_EVENT_3005_REOCRD_DATA + PAGE_COUNT_EVENT_3005_REOCRD_DATA*3)     //断流 *3
    #define PAGE_COUNT_EVENT_3006_REOCRD_DATA          2

#define PAGE_ID_EVENT_3007_REOCRD_DATA           (PAGE_ID_EVENT_3006_REOCRD_DATA + PAGE_COUNT_EVENT_3006_REOCRD_DATA*3)     //潮流反向事件
    #define PAGE_COUNT_EVENT_3007_REOCRD_DATA          2

//有功需量超限、无功需量超限为一组
#define PAGE_ID_EVENT_3009_REOCRD_DATA           (PAGE_ID_EVENT_3007_REOCRD_DATA + PAGE_COUNT_EVENT_3007_REOCRD_DATA)     //有功需量超限事件
    #define PAGE_COUNT_EVENT_3009_REOCRD_DATA          2
#define PAGE_ID_EVENT_300A_REOCRD_DATA           (PAGE_ID_EVENT_3009_REOCRD_DATA + PAGE_COUNT_EVENT_3009_REOCRD_DATA)     //有功需量超限事件
    #define PAGE_COUNT_EVENT_300A_REOCRD_DATA          2
#define PAGE_ID_EVENT_300B_REOCRD_DATA           (PAGE_ID_EVENT_300A_REOCRD_DATA + PAGE_COUNT_EVENT_300A_REOCRD_DATA)     //无功需量超限事件 * 4
    #define PAGE_COUNT_EVENT_300B_REOCRD_DATA          2

#define PAGE_ID_EVENT_300C_REOCRD_DATA           (PAGE_ID_EVENT_300B_REOCRD_DATA + PAGE_COUNT_EVENT_300B_REOCRD_DATA*4)   //总功率因数超下限事件
    #define PAGE_COUNT_EVENT_300C_REOCRD_DATA          2

#define PAGE_ID_EVENT_300D_REOCRD_DATA           (PAGE_ID_EVENT_300C_REOCRD_DATA + PAGE_COUNT_EVENT_300C_REOCRD_DATA)     //全失压事件
    #define PAGE_COUNT_EVENT_300D_REOCRD_DATA          2

//电压逆相序、电流逆相序、过载为一组
#define PAGE_ID_EVENT_300F_REOCRD_DATA           (PAGE_ID_EVENT_300D_REOCRD_DATA + PAGE_COUNT_EVENT_300D_REOCRD_DATA)     //电压逆相序
    #define PAGE_COUNT_EVENT_300F_REOCRD_DATA          2
#define PAGE_ID_EVENT_3010_REOCRD_DATA           (PAGE_ID_EVENT_300F_REOCRD_DATA + PAGE_COUNT_EVENT_300F_REOCRD_DATA)       //电流逆相序
    #define PAGE_COUNT_EVENT_3010_REOCRD_DATA          2
#define PAGE_ID_EVENT_3008_REOCRD_DATA           (PAGE_ID_EVENT_3010_REOCRD_DATA + PAGE_COUNT_EVENT_3010_REOCRD_DATA)       //过载  *3
    #define PAGE_COUNT_EVENT_3008_REOCRD_DATA          2

//辅助电源失电、掉电为一组
#define PAGE_ID_EVENT_300E_REOCRD_DATA           (PAGE_ID_EVENT_3008_REOCRD_DATA + PAGE_COUNT_EVENT_3008_REOCRD_DATA*3)     //辅助电源失电事件
    #define PAGE_COUNT_EVENT_300E_REOCRD_DATA          2
#define PAGE_ID_EVENT_3011_REOCRD_DATA           (PAGE_ID_EVENT_300E_REOCRD_DATA + PAGE_COUNT_EVENT_300E_REOCRD_DATA)     //掉电事件
    #define PAGE_COUNT_EVENT_3011_REOCRD_DATA          2

#define PAGE_ID_EVENT_3012_REOCRD_DATA           (PAGE_ID_EVENT_3011_REOCRD_DATA + PAGE_COUNT_EVENT_3011_REOCRD_DATA)     //电能表编程事件
    #define PAGE_COUNT_EVENT_3012_REOCRD_DATA          2

#define PAGE_ID_EVENT_3013_REOCRD_DATA           (PAGE_ID_EVENT_3012_REOCRD_DATA + PAGE_COUNT_EVENT_3012_REOCRD_DATA)     //电能表清零事件
    #define PAGE_COUNT_EVENT_3013_REOCRD_DATA          2

#define PAGE_ID_EVENT_3014_REOCRD_DATA           (PAGE_ID_EVENT_3013_REOCRD_DATA + PAGE_COUNT_EVENT_3013_REOCRD_DATA)     //电能表需量清零事件
    #define PAGE_COUNT_EVENT_3014_REOCRD_DATA          2

#define PAGE_ID_EVENT_3015_REOCRD_DATA           (PAGE_ID_EVENT_3014_REOCRD_DATA + PAGE_COUNT_EVENT_3014_REOCRD_DATA)     //电能表事件清零事件
    #define PAGE_COUNT_EVENT_3015_REOCRD_DATA          2
	
#define PAGE_ID_EVENT_3016_REOCRD_DATA           (PAGE_ID_EVENT_3015_REOCRD_DATA + PAGE_COUNT_EVENT_3015_REOCRD_DATA)     //电能表校时事件
    #define PAGE_COUNT_EVENT_3016_REOCRD_DATA          2
	
#define PAGE_ID_EVENT_3017_REOCRD_DATA           (PAGE_ID_EVENT_3016_REOCRD_DATA + PAGE_COUNT_EVENT_3016_REOCRD_DATA)     //电能表时段编程事件
    #define PAGE_COUNT_EVENT_3017_REOCRD_DATA          2

#define PAGE_ID_EVENT_3018_REOCRD_DATA           (PAGE_ID_EVENT_3017_REOCRD_DATA + PAGE_COUNT_EVENT_3017_REOCRD_DATA)     //电能表时区编程事件
    #define PAGE_COUNT_EVENT_3018_REOCRD_DATA          2

#define PAGE_ID_EVENT_3019_REOCRD_DATA           (PAGE_ID_EVENT_3018_REOCRD_DATA + PAGE_COUNT_EVENT_3018_REOCRD_DATA)     //周休日编程事件
    #define PAGE_COUNT_EVENT_3019_REOCRD_DATA          2

#define PAGE_ID_EVENT_301A_REOCRD_DATA           (PAGE_ID_EVENT_3019_REOCRD_DATA + PAGE_COUNT_EVENT_3019_REOCRD_DATA)     //结算日编程事件
    #define PAGE_COUNT_EVENT_301A_REOCRD_DATA          2

//开表盖、开端钮盒为一组
#define PAGE_ID_EVENT_301B_REOCRD_DATA           (PAGE_ID_EVENT_301A_REOCRD_DATA + PAGE_COUNT_EVENT_301A_REOCRD_DATA)     //开表盖事件
    #define PAGE_COUNT_EVENT_301B_REOCRD_DATA          2
#define PAGE_ID_EVENT_301C_REOCRD_DATA           (PAGE_ID_EVENT_301B_REOCRD_DATA + PAGE_COUNT_EVENT_301B_REOCRD_DATA)     //开端纽盒事件
    #define PAGE_COUNT_EVENT_301C_REOCRD_DATA          2
	
#define PAGE_ID_EVENT_301D_REOCRD_DATA           (PAGE_ID_EVENT_301C_REOCRD_DATA + PAGE_COUNT_EVENT_301C_REOCRD_DATA)     //电压不平衡
    #define PAGE_COUNT_EVENT_301D_REOCRD_DATA          2
	
//电流不平衡、电流严重不平衡为一组	
#define PAGE_ID_EVENT_301E_REOCRD_DATA           (PAGE_ID_EVENT_301D_REOCRD_DATA + PAGE_COUNT_EVENT_301D_REOCRD_DATA)     //电流不平衡
    #define PAGE_COUNT_EVENT_301E_REOCRD_DATA          2
#define PAGE_ID_EVENT_302D_REOCRD_DATA           (PAGE_ID_EVENT_301E_REOCRD_DATA + PAGE_COUNT_EVENT_301E_REOCRD_DATA)     //电流严重不平衡
    #define PAGE_COUNT_EVENT_302D_REOCRD_DATA          2

//跳闸、合闸为一组
#define PAGE_ID_EVENT_301F_REOCRD_DATA           (PAGE_ID_EVENT_302D_REOCRD_DATA + PAGE_COUNT_EVENT_302D_REOCRD_DATA)     //跳闸事件
    #define PAGE_COUNT_EVENT_301F_REOCRD_DATA          2
#define PAGE_ID_EVENT_3020_REOCRD_DATA           (PAGE_ID_EVENT_301F_REOCRD_DATA + PAGE_COUNT_EVENT_301F_REOCRD_DATA)     //合闸事件
    #define PAGE_COUNT_EVENT_3020_REOCRD_DATA          2

#define PAGE_ID_EVENT_3021_REOCRD_DATA           (PAGE_ID_EVENT_3020_REOCRD_DATA + PAGE_COUNT_EVENT_3020_REOCRD_DATA)     //电能表节假日编程事件
    #define PAGE_COUNT_EVENT_3021_REOCRD_DATA          2

#define PAGE_ID_EVENT_3022_REOCRD_DATA           (PAGE_ID_EVENT_3021_REOCRD_DATA + PAGE_COUNT_EVENT_3021_REOCRD_DATA)     //电能表有功组合方式编程事件
    #define PAGE_COUNT_EVENT_3022_REOCRD_DATA          2

#define PAGE_ID_EVENT_3023_REOCRD_DATA           (PAGE_ID_EVENT_3022_REOCRD_DATA + PAGE_COUNT_EVENT_3022_REOCRD_DATA)     //电能表无功组合方式编程事件
    #define PAGE_COUNT_EVENT_3023_REOCRD_DATA          2

#define PAGE_ID_EVENT_3024_REOCRD_DATA           (PAGE_ID_EVENT_3023_REOCRD_DATA + PAGE_COUNT_EVENT_3023_REOCRD_DATA)     //电能表费率参数编程事件
    #define PAGE_COUNT_EVENT_3024_REOCRD_DATA          2

#define PAGE_ID_EVENT_3025_REOCRD_DATA           (PAGE_ID_EVENT_3024_REOCRD_DATA + PAGE_COUNT_EVENT_3024_REOCRD_DATA)     //电能表阶梯表编程事件
    #define PAGE_COUNT_EVENT_3025_REOCRD_DATA          2

#define PAGE_ID_EVENT_3026_REOCRD_DATA           (PAGE_ID_EVENT_3025_REOCRD_DATA + PAGE_COUNT_EVENT_3025_REOCRD_DATA)     //电能表密钥更新事件
    #define PAGE_COUNT_EVENT_3026_REOCRD_DATA          2

#define PAGE_ID_EVENT_3028_REOCRD_DATA           (PAGE_ID_EVENT_3026_REOCRD_DATA + PAGE_COUNT_EVENT_3026_REOCRD_DATA)     //电能表购电记录
    #define PAGE_COUNT_EVENT_3028_REOCRD_DATA          2

#define PAGE_ID_EVENT_302A_REOCRD_DATA           (PAGE_ID_EVENT_3028_REOCRD_DATA + PAGE_COUNT_EVENT_3028_REOCRD_DATA)     //恒定磁场干扰事件
    #define PAGE_COUNT_EVENT_302A_REOCRD_DATA          2

#define PAGE_ID_EVENT_302B_REOCRD_DATA           (PAGE_ID_EVENT_302A_REOCRD_DATA + PAGE_COUNT_EVENT_302A_REOCRD_DATA)     //负荷开关误动作事件
    #define PAGE_COUNT_EVENT_302B_REOCRD_DATA          2

#define PAGE_ID_EVENT_302C_REOCRD_DATA           (PAGE_ID_EVENT_302B_REOCRD_DATA + PAGE_COUNT_EVENT_302B_REOCRD_DATA)     //电源异常事件
    #define PAGE_COUNT_EVENT_302C_REOCRD_DATA          2


#define PAGE_ID_METER_DATA_END                   (PAGE_ID_EVENT_302C_REOCRD_DATA + PAGE_COUNT_EVENT_302C_REOCRD_DATA)     //电表数据结束page_id 现在使用了220页了

#endif

/*  
 * ===============================================================================
 */
//升级相关操作接口
void delete_update_file(void);
INT16U fread_array_update_info(INT16U addr,INT8U *buf,INT16U len);
INT16U fwrite_array_update_info(INT16U addr,INT8U *buf,INT16U len);
INT16U fread_array_update_data(INT32U addr,INT8U *buf,INT16U len);
INT16U fwrite_array_update_data(INT32U addr,INT8U *buf,INT16U len);

//终端参数相关操作接口
INT8U  fwrite_ertu_params(INT32U addr,INT8U *data,INT16U len);
INT8U  fread_ertu_params(INT32U addr,INT8U *data,INT16U len);
#ifdef __13761__
//三类数据ERC相关操作接口
INT8U fwrite_erc_params(INT32U addr,INT8U *data,INT16U len);
INT8U fread_erc_params(INT32U addr,INT8U *data,INT16U len);
#endif
#endif