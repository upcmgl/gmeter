#ifndef __APP_CONSTDEF_H__
#define __APP_CONSTDEF_H__


//应用层标志字
#define  APP_POWER_ONOFF_CY  0x01     //停上电事件依赖交采产生，否则终端自己产生

  #define COMMPORT_485_CY    0x01     //交采内表
  #define COMMPORT_485_REC   0x02     //抄表
  #define COMMPORT_485_CAS   0x03     //级联
  #define COMMPORT_PLC       0x1F     //载波设置端口，河南使用这个端口
  #define COMMPORT_PLC_REC   0x04     //载波抄表端口，集中器内部使用的端口
  
  #define COMMPORT_INFRA_RED   0xFE     //本地红外口
  #define COMMPORT_GPRS   0xFD     	 //本地GPRS口

#if defined  __PROVICE_JIANGSU__
     #define DATA_DEFAULT      0xEE
#else
     #define DATA_DEFAULT       0xFF
#endif

#define GB645_2007       30      //645-2007

#define EVENT_RECORD_SIZE  152

//WORK_FLAG 使用定义
#define  WORK_FLAG_SEQ_RSEQ        0
#define  WORK_FLAG_IS_FIRST_FRAME  1    //表示是否是多帧中的首帧
#define  WORK_FLAG_CHECK_FLAG      2
#define  WORK_FLAG_METER_READ      3
#define  WORK_FLAG_IS_CAST         4    //表示命令帧中的终端地址是否是广播地址
#define  WORK_FLAG_CMD_FLAG        5    //下行命令标志

#define MAX_SIZE_PLMSDTP  1900

#define PLMT_VER  "C3.0"

//
//消息验证码长度,  分为2字节和16字节
//
#if defined (__PWLEN_IS_2BYTE__)
#define PW_LEN  2
#elif defined (__PWLEN_IS_32BYTE__)
#define PW_LEN  32
#else
#define PW_LEN  16
#endif

#ifdef __RTUADDR_IS_6BYTE__
#define CFG_DEVID_LEN               6    //定义终端地址的长度
#else
#define CFG_DEVID_LEN               4    //定义终端地址的长度
#endif

//透明转发计算转发内容长度，需去除字节长度，PNFN、转发控制字等内容
#if defined __QGDW_FK2005__ || defined __PROVICE_NEIMENG__
#define RELAY_MINUS_LEN 9
#else
#define RELAY_MINUS_LEN 10
#endif

#define LIGHT   0
#define UNLIGHT 1

//透明模式定义
#define PLC_TRANSIT_MODE   0xAA
#define GPRS_TRANSIT_MODE  0xBB

#define PROTECT_TEMPERATURE       83      //保护温度
#define STARTPROTECT_TEMPERATURE  88  //启动保护的温度值，一定要高于PROTECT_TEMPERATURE，起到缓冲区作用

//Q/GDW上行协议分类定义
#define QGDW_2004        1        //负控2004
#define QGDW_376_1       2        //Q/GDW376.1
#define QGDW_2005        1       //负控2005，和原2004协议基本相同，协议标志也相同，但是目前的实现更完整
#define QGXD_FK2012      2       //广西配变协议2012
#define QNMDW_FK2012     3       //内蒙古2012版计量通讯规约


#define MAX_ADDGROUP_COUNT        8      //总加组最大数量
#define MAX_ADDGROUP_SPOTCOUNT    8      //总加组内测量点最大数量

#define MAX_COMMPORT_COUNT  5      //通信端口数量


#if defined __POWER_CTRL_I__  || defined __POWER_CTRL_II__
#define MAX_PULSE_PORT_COUNT 8
#else
#define MAX_PULSE_PORT_COUNT 2
#endif

//电表类别定义
#define MCLASS_COMMON      0x01     //普通电表  
#define MCLASS_CJQ         0x10     //采集器
#define MCLASS_ZJQ         0x20     //中继器




//转发执行结果定义
#define  TRANS_ERR         0x00   //不能执行转发、
#define  TRANS_TIMEOUT     0x01   //转发接收超时
#define  TRANS_RECV_ERR    0x02   //转发接收错误
#define  TRANS_RECV_ACK    0x03   //转发接收确
#define  TRANS_RECV_NAK    0x04   //转发接收否认
#define  TRANS_RECV_DATA   0x05   //转发接收数据




//#if defined(__PROVICE_ANHUI__) 
//#define PATCH_REC_DH_DAY               62      //补抄最大天数
//#else
//#define PATCH_REC_DH_DAY                3      //补抄最大天数
//#endif
//#define PATCH_REC_DH_NUM                6      //每次补抄几个日冻结就换表




//广播任务标志,可以有8个不同类别的任务。
#define  PLCTASK_CASTSET           0x01      //广播设置任务

//广播任务状态
#define PLCCASTTASK_NO            0x00     //广播任务取消或无任务
#define PLCCASTTASK_OK            0x01     //广播任务完成
#define PLCCASTTASK_EXEC          0x02     //广播任务未完成


////远程通道类别定义
//#define REMOTECOMM_GPRS    0x01      //GPRS,SMS
//#define REMOTECOMM_CDMA    0x02      //CDMA,SMS
//#define REMOTECOMM_PSTN    0x03      //PSTN
//#define REMOTECOMM_SMS     0x04      //GSM-SMS
//#define REMOTECOMM_NET     0x05      //TCP-IP
//#define REMOTECOMM_OTHER   0x06      //other

//远程通道命令
#define REMOTE_COMMAND_QUREY	        	0
#define REMOTE_COMMAND_RST			1
#define REMOTE_COMMAND_ON			2
#define REMOTE_COMMAND_OFF			3
#define REMOTE_COMMAND_ONLINE		        4
#define REMOTE_COMMAND_OFFLINE		        5
#define REMOTE_COMMAND_NOTECHG		        6

//远程通道通信状态
#define REMOTE_CTS_GSMOFF		    	1	 //1:RESET 
#define REMOTE_CTS_GSMON		    	2	 //2:GSM POWER ON unreg 
#define REMOTE_CTS_GSMREG		    	3	 //3:gsm reg 
#define REMOTE_CTS_TCPLINK			4	 //4:tcp online
#define REMOTE_CTS_ONLINECOMMAND		5	 //5:tcponline command 

//liujing add
//底层状态栏显示内容
#define FOOT_SHOW_DATA                1
#define FOOT_SHOW_DATAANDFIRST        2
#define FOOT_SHOW_DATAANDMIDDLE       3
#define FOOT_SHOW_DATAANDLAST         4
#define FOOT_NOTSHOW_DATA             5

//集抄参数设置事务控制类别
#define TRANS_JCSET_MODIFY   0x01    //修改档案
#define TRANS_JCSET_DEL      0x02    //删除档案
#define TRANS_JCSET_ADD      0x04    //新增档案
#define TRANS_JCSET_CLR      0x08    //清除档案






//下行报文标志定义
#define MSA_BROADCAST  0x01  //主站广播
#define DEV_ASK        0x02  //设备模拟请求祯



//事件类别定义，定义在事件索引文件中
//check_flag
#define  EXCLUDE_FLAG    0x01
#define  SWCTRL_FLAG     0x02
#define  VIPEVENT_FLAG   0x04
#define  SENDFRAME_FLAG  0x08
#define  CURVEDATE_FLAG  0x10
#define  VALIDEVT_FLAG   0x20
#define  NO_REPORT_FLAG  0x40

//事件索引文件中，事件标识定义
#define  VIP_EVENT_FLAG         0x01     //重要事件标识
#define  VALID_EVENT_FLAG       0x02     //有效事件标识
#define  EVENT_REPORT_FLAG      0x04     //是否需上报标识
#define  EVENT_REPORT_SUC_FLAG  0x08     //是否上报成功标识

//任务上报标识定义
#define  TASK_REPORT_FLAG       0x01     //是否需上报,D0有效
#define  TASK_REPORT_IN_CYCLE   0x02     //任务周期内是否已上报


//文件传输的文件标识定义
#define PROG_FILE_FLAG    0x00  //主程序
#define DATA_FILE_FLAG    0x01    //浙江扩展占用主程序
#define ASC168_FILE_FLAG  0x02
#define HZ1616_FILE_FLAG  0x03
#define PROG_COMM_MODULE  0x10  //通讯模块程序
#define PROG_METER_MODULE 0xA1  //交采程序
#define PROG_G14_MODULE   0xA2  //G14程序
#define PROG_ROUTER_MODULE 0xA3  //路由


//新376.1文件传输文件属性
#define FILEPROP_FIR    0x00    //起始帧、中间帧
#define FILEPROP_FIN    0x01    //结束帧
//新376.1文件传输文件指令
#define FILECMD_FRAME    0x00    //报文方式
#define FILECMD_FTP      0x01    //FTP传输方式
#define FILECMD_GROUP    0x02    //启动组地址升级



//用户大类号定义
#define DEFAULT_USER_CLASS   0     //用户大类号缺省值
#define USER_CLASS_A         1     //A类，大型专变用户
#define USER_CLASS_B         2     //B类，中小型专变用户
#define USER_CLASS_C         3     //C类，三相一般工商业用户
#define USER_CLASS_D         4     //D类，单相一般工商业用户
#define USER_CLASS_E         5     //E类，居民用户
#define USER_CLASS_F         6     //F类，公用配变考核计量点

//用户小类号(电表分类号)定义
#define DEFAULT_METER_CLASS  0      //电表分类号缺省值
#define METER_CLASS_SP       1      //单相智能表
#define METER_CLASS_TP       2      //三相智能表
#define METER_CLASS_APM     15     //APM数据电表(下接水表，煤气表，APM)


//FCS计算使用

#define PPPINITFCS16 0xffff /* Initial FCS value */
#define PPPGOODFCS16 0xf0b8 /* Good final FCS value */





//设备特征字定义，最多12个位，用于device_flag变量
#define DEVICE_UPDATE_13MODE	       0x0001		//支持13版协议升级
#define DEVICE_UPDATE_ZHEJIANG	   	   0x0002		//支持浙江升级模式


//电表接收格式判断，借助波特率高4位传送
#define RECV_FORMAT_GB645           0x00
#define RECV_FORMAT_EDMI            0x10
#define RECV_FORMAT_HENGTONG        0x20    //恒通新、旧规约报文格式一样

// 错误编码
#define OK                      0
#define ERR                     1
#define ERR_NOANSWER            1     //中继命令没有返回
#define ERR_BADFRAME            2     //设置内容非法
#define ERR_AUTH                3     //密码权限不足
#define ERR_NOITEM              4     //无此项数据
#define ERR_NOROOM              4     //空间不足
#define ERR_EXPIRED             5     //命令时间失效
#define ERR_BADDST            0x11    //目标地址不存在
#define ERR_SENDFAIL          0x12    //发送失败
#define ERR_SMSTOOLONG        0x13    //短消息帧太长

#define ESAM_RESET_OK        0x02



#define POS_CTRL     6       //控制码
#define POS_RT       7       //终端行政区划代码 2字节
#define POS_UID      9       //终端地址        2字节
#ifdef __RTUADDR_IS_6BYTE__
    #define POS_MSA      13      //主站地址以及组地址标志
    #define POS_AFN      14      //应用层功能码
    #define POS_SEQ      15      //帧序列
    #define POS_DATA     16      //用户数据起始地址
#else
#define POS_MSA      11      //主站地址以及组地址标志
#define POS_AFN      12      //应用层功能码
#define POS_SEQ      13      //帧序列
#define POS_DATA     14      //用户数据起始地址
#endif


#define    PN_RTUA          0x10     //终端
#define    PN_METER         0x20     //测量点
#define    PN_AGT           0x30     //总加组号
#define    PN_SIMULATE      0x40     //直流模拟量

#define    LOCK_HOUR        0x01     //时冻结
#define    LOCK_DAY         0x02     //日冻结
#define    LOCK_MONTH       0x03     //月冻结
#define    LOCK_METERDAY    0x04     //抄表日冻结
#define    LOCK_CURVE       0x05     //曲线

//事件类别
#define ERC1   1
#define ERC2   2
#define ERC3   3
#define ERC4   4
#define ERC5   5
#define ERC6   6
#define ERC7   7
#define ERC8   8
#define ERC9   9
#define ERC10  10
#define ERC11  11
#define ERC12  12
#define ERC13  13
#define ERC14  14
#define ERC15  15
#define ERC16  16
#define ERC17  17
#define ERC18  18
#define ERC19  19
#define ERC20  20
#define ERC21  21
#define ERC22  22
#define ERC23  23
#define ERC24  24
#define ERC25  25
#define ERC26  26
#define ERC27  27
#define ERC28  28
#define ERC29  29
#define ERC30  30
#define ERC31  31
#define ERC32  32
#define ERC33  33
#define ERC34  34
#define ERC35  35
#define ERC36  36 //浙江扩展为上报失败电表
#define ERC37  37 //以色列扩展
#define ERC38  38 //浙江扩展上报采集器未知电表
#define ERC39  39
#define ERC40  40 //线损报警
#define ERC41  41
#define ERC42  42
#define ERC43  43
#define ERC44  44 
#define ERC45  45
#define ERC49  49 //宽带载波使用的 设备掉电事件记录
#define ERC51  51
#define ERC52  52
#define ERC53  53
#define ERC54  54
#define ERC55  55
#define ERC56  56
#define ERC57  57
#define ERC58  58
#define ERC60  60
#define ERC61  61  //上海时钟偏差事件
#define ERC62  62  //探测抄读事件
#define ERC63  63  //跨台区电能表信息
#define ERC64  64  //江苏掉电事件

#define ERC65  65  //上海专变失压事件
#define ERC66  66  //上海专变电子开门报警器事件
#define ERC67  67
#define ERC68  68

//ESAM验证错误代码
#define HERR_OK            0  
#define HERR_SIGNATURE     1     //签名校验错误
#define HERR_CRYPTO        2     //密文校验错误
#define HERR_SYNMAC        3     //对称MAC验证失败

//===========应用层功能编码 AFN_XXX============================

#define AFN_ACK            0x00     //确认/否认 以及终端对所请求的数据不具备响应条件的否认回答
#define AFN_RESET          0x01     //复位命令
#define AFN_TEST           0x02     //链路接口检测,适用于交换网络传输信道
#define AFN_FEP            0x03     //中继站命令
#define AFN_SET            0x04     //设置参数
#define AFN_CTRL           0x05     //控制命令
#define AFN_AUTH           0x06     //密钥协商
#define AFN_ASK_SLAVE      0x08     //请求被级联终端主动上报
#define AFN_READCFG        0x09     //请求终端配置
#define AFN_QUERY          0x0A     //查询参数
#define AFN_READTASK       0x0B     //请求任务数据
#define AFN_READCUR        0x0C     //请求1类数据（实时数据）
#define AFN_READHIS        0x0D     //请求2类数据（历史数据） 
#define AFN_READEVENT      0x0E     //请求3类数据（事件数据）  
#define AFN_FTP            0x0F     //文件传输
#define AFN_REMOTE_UPDATE_NM      0x13     //内蒙 远程升级方式2
#define AFN_RELAY          0x10     //数据转发
#define AFN_PRODUCT_TEST   0x15     //生产检测扩展
#define AFN_ASK_SLAVE_NEXT 0x18     //继续请求从终端上报数据(作为从终端，应该等待该报文后才能继续发送数据)
#define AFN_PATCH_READHIS  0x1d     //补抄 2类数据
#define AFN_DEBUG          0xFE     //厂家调试





#define FUNC_MASK_CLIENT    0x80       //设置传送方向
#define FUNC_MASK_DENY      0x40       //设置否定标志

#define CTRLFUNC_SET_DIR        0x80       //设置传输方向位DIR=1    使用|
#define CTRLFUNC_CLR_DIR        0x7F       //设置传输方向位DIR=0    使用&
#define CTRLFUNC_CLR_PRM        0xBF       //设置启动标志位PRM=0    使用&
#define CTRLFUNC_SET_PRM        0x40       //设置启动标志位PRM=1    使用|
#define CTRLFUNC_CLR_ACD        0xDF       //设置帧要求访问位ACD=0  使用&
#define CTRLFUNC_SET_ACD        0x20       //设置帧要求访问位ACD=1  使用|
#define CTRLFUNC_CLR_FCB        0xDF       //设置帧计数位FCB=0      使用&
#define CTRLFUNC_SET_FCB        0x20       //设置帧计数位FCB=1      使用|
#define CTRLFUNC_CLR_FCV        0xEF       //设置帧计数有效位FCV=0      使用&
#define CTRLFUNC_SET_FCV        0x10       //设置帧计数有效位FCV=1      使用|






//功能码定义
//PRM=1 帧报文来自启动站,启动站：报文发起站，从动站：响应端。

//复位命令   发送/确认
#define PRM_CTRLFUNC_RESET  1

//用户数据   发送/无回答
#define PRM_CTRLFUNC_USERDATA  4

//链路测试   请求/响应帧     请求/响应：就是说该功能需要有从动站的响应报文。
#define PRM_CTRLFUNC_TEST    9

//请求1级数据   请求/响应帧
#define PRM_CTRLFUNC_CURDATA    10

//请求2级数据   请求/响应帧
#define PRM_CTRLFUNC_HISDATA    11


//PRM=0 帧报文来自从动站

//认可  确认
#define CTRLFUNC_CONFIRM  0

//用户数据  响应帧
#define CTRLFUNC_USERDATA  8

//否认：无所召唤的数据  响应帧
#define CTRLFUNC_NODATA   9

//链路状态   响应帧
#define CTRLFUNC_TEST    11





//帧时间标签有效位
#define MASK_TpV   0x80

//首帧标志位
#define MASK_FIR   0x40

//末帧标志位
#define MASK_FIN   0x20

//请求确认标志位
#define MASK_CON   0x10



//信息类
#define DT_F0   0x0000

#define DT_F1   0x0001
#define DT_F2   0x0002
#define DT_F3   0x0004
#define DT_F4   0x0008
#define DT_F5   0x0010
#define DT_F6   0x0020
#define DT_F7   0x0040
#define DT_F8   0x0080

#define DT_F9   0x0101
#define DT_F10  0x0102
#define DT_F11  0x0104
#define DT_F12  0x0108
#define DT_F13  0x0110
#define DT_F14  0x0120
#define DT_F15  0x0140
#define DT_F16  0x0180


#define DT_F17  0x0201
#define DT_F18  0x0202
#define DT_F19  0x0204
#define DT_F20  0x0208
#define DT_F21  0x0210
#define DT_F22  0x0220
#define DT_F23  0x0240
#define DT_F24  0x0280


#define DT_F25  0x0301
#define DT_F26  0x0302
#define DT_F27  0x0304
#define DT_F28  0x0308
#define DT_F29  0x0310
#define DT_F30  0x0320
#define DT_F31  0x0340
#define DT_F32  0x0380


#define DT_F33  0x0401
#define DT_F34  0x0402
#define DT_F35  0x0404
#define DT_F36  0x0408
#define DT_F37  0x0410
#define DT_F38  0x0420
#define DT_F39  0x0440
#define DT_F40  0x0480

#define DT_F41  0x0501
#define DT_F42  0x0502
#define DT_F43  0x0504
#define DT_F44  0x0508
#define DT_F45  0x0510
#define DT_F46  0x0520
#define DT_F47  0x0540
#define DT_F48  0x0580


#define DT_F49  0x0601
#define DT_F50  0x0602
#define DT_F51  0x0604
#define DT_F52  0x0608
#define DT_F53  0x0610
#define DT_F54  0x0620
#define DT_F55  0x0640
#define DT_F56  0x0680


#define DT_F57  0x0701
#define DT_F58  0x0702
#define DT_F59  0x0704
#define DT_F60  0x0708
#define DT_F61  0x0710
#define DT_F62  0x0720
#define DT_F63  0x0740
#define DT_F64  0x0780

#define DT_F65  0x0801
#define DT_F66  0x0802
#define DT_F67  0x0804
#define DT_F68  0x0808
#define DT_F69  0x0810
#define DT_F70  0x0820
#define DT_F71  0x0840
#define DT_F72  0x0880

#define DT_F657 0x0805
#define DT_F668 0x080A


#define DT_F73  0x0901
#define DT_F74  0x0902
#define DT_F75  0x0904
#define DT_F76  0x0908
#define DT_F77  0x0910
#define DT_F78  0x0920
#define DT_F79  0x0940
#define DT_F80  0x0980


#define DT_F81  0x0A01
#define DT_F82  0x0A02
#define DT_F83  0x0A04
#define DT_F84  0x0A08
#define DT_F85  0x0A10
#define DT_F86  0x0A20
#define DT_F87  0x0A40
#define DT_F88  0x0A80

#define DT_F89  0x0B01
#define DT_F90  0x0B02
#define DT_F91  0x0B04
#define DT_F92  0x0B08
#define DT_F93  0x0B10
#define DT_F94  0x0B20
#define DT_F95  0x0B40
#define DT_F96  0x0B80


#define DT_F97   0x0C01
#define DT_F98   0x0C02
#define DT_F99   0x0C04
#define DT_F100  0x0C08
#define DT_F101  0x0C10
#define DT_F102  0x0C20
#define DT_F103  0x0C40
#define DT_F104  0x0C80

#define DT_F105  0x0D01
#define DT_F106  0x0D02
#define DT_F107  0x0D04
#define DT_F108  0x0D08
#define DT_F109  0x0D10
#define DT_F110  0x0D20
#define DT_F111  0x0D40
#define DT_F112  0x0D80

#define DT_F113  0x0E01
#define DT_F114  0x0E02
#define DT_F115  0x0E04
#define DT_F116  0x0E08
#define DT_F117  0x0E10
#define DT_F118  0x0E20
#define DT_F119  0x0E40
#define DT_F120  0x0E80

#define DT_F121  0x0F01
#define DT_F122  0x0F02
#define DT_F123  0x0F04
#define DT_F124  0x0F08
#define DT_F125  0x0F10
#define DT_F126  0x0F20
#define DT_F127  0x0F40
#define DT_F128  0x0F80

#define DT_F129  0x1001
#define DT_F130  0x1002
#define DT_F131  0x1004
#define DT_F132  0x1008
#define DT_F133  0x1010
#define DT_F134  0x1020
#define DT_F135  0x1040
#define DT_F136  0x1080


#define DT_F137  0x1101
#define DT_F138  0x1102
#define DT_F139  0x1104
#define DT_F140  0x1108
#define DT_F141  0x1110
#define DT_F142  0x1120
#define DT_F143  0x1140
#define DT_F144  0x1180


#define DT_F145  0x1201
#define DT_F146  0x1202
#define DT_F147  0x1204
#define DT_F148  0x1208
#define DT_F149  0x1210
#define DT_F150  0x1220
#define DT_F151  0x1240
#define DT_F152  0x1280

#define DT_F153  0x1301
#define DT_F154  0x1302
#define DT_F155  0x1304
#define DT_F156  0x1308
#define DT_F157  0x1310
#define DT_F158  0x1320
#define DT_F159  0x1340
#define DT_F160  0x1380


#define DT_F161  0x1401
#define DT_F162  0x1402
#define DT_F163  0x1404
#define DT_F164  0x1408
#define DT_F165  0x1410
#define DT_F166  0x1420
#define DT_F167  0x1440
#define DT_F168  0x1480

#define DT_F169  0x1501
#define DT_F170  0x1502
#define DT_F171  0x1504
#define DT_F172  0x1508
#define DT_F173  0x1510
#define DT_F174  0x1520
#define DT_F175  0x1540
#define DT_F176  0x1580


#define DT_F177  0x1601
#define DT_F178  0x1602
#define DT_F179  0x1604
#define DT_F180  0x1608
#define DT_F181  0x1610
#define DT_F182  0x1620
#define DT_F183  0x1640
#define DT_F184  0x1680

#define DT_F185  0x1701
#define DT_F186  0x1702
#define DT_F187  0x1704
#define DT_F188  0x1708
#define DT_F189  0x1710
#define DT_F190  0x1720
#define DT_F191  0x1740
#define DT_F192  0x1780

#define DT_F193  0x1801
#define DT_F194  0x1802
#define DT_F195  0x1804
#define DT_F196  0x1808
#define DT_F197  0x1810
#define DT_F198  0x1820
#define DT_F199  0x1840
#define DT_F200  0x1880


#define DT_F201  0x1901
#define DT_F202  0x1902
#define DT_F203  0x1904
#define DT_F204  0x1908
#define DT_F205  0x1910
#define DT_F206  0x1920
#define DT_F207  0x1940
#define DT_F208  0x1980



#define DT_F209  0x1A01
#define DT_F210  0x1A02
#define DT_F211  0x1A04
#define DT_F212  0x1A08
#define DT_F213  0x1A10
#define DT_F214  0x1A20
#define DT_F215  0x1A40
#define DT_F216  0x1A80



#define DT_F217  0x1B01
#define DT_F218  0x1B02
#define DT_F219  0x1B04
#define DT_F220  0x1B08
#define DT_F221  0x1B10
#define DT_F222  0x1B20
#define DT_F223  0x1B40
#define DT_F224  0x1B80


#define DT_F225  0x1C01
#define DT_F226  0x1C02
#define DT_F227  0x1C04
#define DT_F228  0x1C08
#define DT_F229  0x1C10
#define DT_F230  0x1C20
#define DT_F231  0x1C40
#define DT_F232  0x1C80

#define DT_F233  0x1D01
#define DT_F234  0x1D02
#define DT_F235  0x1D04
#define DT_F236  0x1D08
#define DT_F237  0x1D10
#define DT_F238  0x1D20
#define DT_F239  0x1D40
#define DT_F240  0x1D80

#define DT_F241  0x1E01
#define DT_F242  0x1E02
#define DT_F243  0x1E04
#define DT_F244  0x1E08
#define DT_F245  0x1E10
#define DT_F246  0x1E20
#define DT_F247  0x1E40
#define DT_F248  0x1E80

#define DT_F249  0x1F01
#define DT_F250  0x1F02
#define DT_F251  0x1F04
#define DT_F252  0x1F08
#define DT_F253  0x1F10
#define DT_F254  0x1F20
#define DT_F255  0x1F40
#define DT_F256  0x1F80

#define DT_F296  0x2480
#define DT_F297  0x2501
#define DT_F298  0x2502
#define DT_F299  0x2504
#define DT_F300  0x2508
#define DT_F301  0x2510
#define DT_F302  0x2520
#define DT_F303  0x2540
#define DT_F304  0x2580

#define DT_F305  0x2601
#define DT_F306  0x2602
#define DT_F307  0x2604
#define DT_F308  0x2608
#define DT_F309  0x2610
#define DT_F310  0x2620
#define DT_F311  0x2640
#define DT_F312  0x2680

#define DT_F313  0x2701
#define DT_F314  0x2702
#define DT_F315  0x2704
#define DT_F316  0x2708
#define DT_F317  0x2710
#define DT_F318  0x2720
#define DT_F319  0x2740
#define DT_F320  0x2780

#define DT_F321  0x2801
#define DT_F322  0x2802
#define DT_F324  0x2808
#define DT_F325  0x2810

#define DT_F400  0x3180
#define DT_F401  0x3201
#define DT_F402  0x3202

#define DT_F701  0x5710
#define DT_F702  0x5720

#define DT_F796  0x6308
#define DT_F797  0x6310
#define DT_F798  0x6320
#define DT_F799  0x6340

#define DT_F801  0x6401
#define DT_F802  0x6402

#define DT_F2040  0xFE80

#define DT_F2048  0xFF80
//信息点
#define DA_P0  0x0000

#define DA_P1  0x0101
#define DA_P2  0x0102
#define DA_P3  0x0104
#define DA_P4  0x0108
#define DA_P5  0x0110
#define DA_P6  0x0120
#define DA_P7  0x0140
#define DA_P8  0x0180


#define DA_P9   0x0201
#define DA_P10  0x0202
#define DA_P11  0x0204
#define DA_P12  0x0208
#define DA_P13  0x0210
#define DA_P14  0x0220
#define DA_P15  0x0240
#define DA_P16  0x0280

#define DA_P17  0x0401
#define DA_P18  0x0402
#define DA_P19  0x0404
#define DA_P20  0x0408
#define DA_P21  0x0410
#define DA_P22  0x0420
#define DA_P23  0x0440
#define DA_P24  0x0480







//=============================================================================
//  操作结果定义
//=============================================================================
#define  EXEC_SUCESS            0x00   //操作成功
#define  EXEC_BADDATA           0x01   //数据出错
#define  EXEC_BADPWD            0x02   //密码错
#define  EXEC_INVALID           0x03   //无有效数据
#define  EXEC_BADVER            0x04   //终端版本变更
#define  EXEC_BUSY              0xC0   //忙

#define  EXEC_ROUTER_NORESP     0xF1   //路由器无响应



//默认费率 天津要求没有数据时费率不能写0
#if defined(__PROVICE_TIANJIN__)
    #define DEFAULT_FL       0x01        
#elif defined (__PROVICE_ANHUI__) || defined(__PROVICE_JIANGSU__) || defined(__PROVICE_HEILONGJIANG__) 
    #define DEFAULT_FL       0x04
#else
    #define DEFAULT_FL       0x00
#endif

//浙江捞表参数
#define PLC_NET_TIME1        10   //60   //15         //捞表时间15分钟
#define PLC_NET_TIME2        20         //捞表时间120分钟
#define PLC_NET_INTERVAL1    3          //间隔1分钟
#define PLC_NET_INTERVAL2    10         //间隔10分钟
#define PLC_NET_INTERVAL3    17         //间隔17分钟
#define PLC_NET_INTERVAL4    240        //间隔240分钟


//控制类别  __POWER_CTRL__     //负控
#define ERTU_CTRLTYPE_TIMESEG    1        //时段控
#define ERTU_CTRLTYPE_RELAX      2        //厂休控
#define ERTU_CTRLTYPE_STOPWORK   3        //报停控
#define ERTU_CTRLTYPE_FLOATDOWN  4        //下浮控
#define ERTU_CTRLTYPE_BUYFEE     5        //购电费控
#define ERTU_CTRLTYPE_MONTHFEE   6        //月电费控
#define ERTU_CTRLTYPE_REMOTE     7        //遥控
#define ERTU_CTRLTYPE_PROTECT    8        //保电
#define ERTU_BUYFEE_ALARM        9        //催费告警

#define ERTU_INACT               0x01    //投入
#define ERTU_WARNING             0x02    //告警
#define ERTU_SWITCH_OFF          0x04    //跳闸
#define ERTU_DISMISS             0x08    //解除
#define ERTU_SWITCH_ON           0x10    //合闸

#define LED_SW1_RED     0x0001
#define LED_SW2_RED     0x0002
#define LED_SW3_RED     0x0004
#define LED_SW4_RED     0x0008
#define LED_PCTRL   0x0010
#define LED_FCTRL   0x0020
#define LED_PROTECT 0x0040

#define LED_SW1_GREEN 0x0100
#define LED_SW2_GREEN 0x0200
#define LED_SW3_GREEN 0x0400
#define LED_SW4_GREEN 0x0800


//被动激活模式增加2011.3.6
#define ONLINE_ALLTIME						0x01				//永久在线
#define ONLINE_ACTIVE					        0x02				//被动激活
#define ONLINE_TIME 							0x03				//时段在线

//数据抄读时，填写的默认值
#ifdef __PROVICE_JIANGSU__
#define REC_DATA_IS_DENY					0xEE				//抄读回否认
#define REC_DATA_IS_DEFAULT					0xEF				//抄读不回来,也就是默认填写
#else
#define REC_DATA_IS_DENY					0xEE				//抄读回否认
#define REC_DATA_IS_DEFAULT					0xEE				//抄读不回来,也就是默认填写
#endif  //__PROVICE_JIANGSU__

#define SMS_ALLOW_REPORT					0x01	//使能短信发送上报
#define SMS_ALLOW_TASK_REPORT				0x02    //允许上报任务
#define SMS_REPORT_EVENT					0x04    //当前上报事件
#define SMS_REPORT_TASK						0x08    //当前上报任务
#define SMS_SAVE_DEBUG_INFO				0x10	  //存储短信调试信息,来自F221
#define SMS_EVENT_USE						0x20    //存储短信事件上报标志，来自F221		
#define SMS_TASK_USE						0x40    //存储短信事件上报标志，来自F221		
#define SMS_USE								0x80    //使能短信功能，来自F221
#define ETH_CONNECT_SUCCESS_TIME			20     //预设以太网连接成时间
//#define HASH_FUN_COUNT                 16     //哈希算法个数，用于根据表号生成存储位置

//GPRS 拨号失败状态
#define GPRS_NORMAL					        0		//正常
#define GPRS_NOMODEL					        1		//无模块
#define GPRS_NOCARD						     2		//无卡
#define GPRS_NONET						     3		//无网络
#define GPRS_DIALFAIL					     4		//拨号失败
#define GPRS_CONNECTFAIL				     5		//未连接

//显示刷新区域
#define LCD_HEADER					        1		//状态栏
#define LCD_WORKAREA					        2		//工作区域
#define LCD_FOOTER						     4		//提示栏

//GPRS状态
#define	GPRS_NONE       					0x0000 
#define	GPRS_SOCKET_OPEN					0x0001 
#define	GPRS_SOCKET_CLOSE					0x0002
#define  GPRS_SOCKET_CONNECTED              0x0004
#define	GPRS_CHECK_LOGIN					0x0008 
#define	GPRS_LOGON							0x0010 
#define	GPRS_PPP_OPEN						0x0020 
#define	GPRS_PPP_CLOSE						0x0040

//ETH状态
#define	ETH_NONE       					    0x0000 
#define	ETH_SOCKET_OPEN					    0x0001 
#define	ETH_SOCKET_CLOSE					0x0002 
#define  ETH_SOCKET_CONNECTED               0x0004
#define	ETH_CHECK_LOGIN					    0x0008 
#define	ETH_LOGON							0x0010 


#define	REMOTE_MODEL_NONE					0x1F 
#define	REMOTE_MODEL_AT					    0x0F 
#define	REMOTE_MODEL_NOTAT					0x07 

//上报补抄日冻结每帧数据个数
#define ENCRYPT_NONE  0
#define ENCRYPT_TOPS1  1
#define ENCRYPT_TOPS2  2

//电压监视功能
#define MAX_V_MONITOR_COUNT                  10       //电压监视列表最大值或最小值的个数
#define VIP_V_MONITOR_CYCLE                  1        //重点电压监视测量点抄读电压的周期，单位：小时

#define LOG_RECORD_MASK1				0x8000		  //日志记录，最高位1，表示记录的附加信息，记录内容由前一个日志决定，目的是区分日志类别还是附加信息

//退出交采编程时间
#if defined(__PROVICE_JILIN__) || defined(__PROVICE_JILIN_FK__)
#define CYMETER_NOTE_TIME                    480       //退出交采编程时间
#else
#define CYMETER_NOTE_TIME                    240        //退出交采编程时间
#endif

#define PORT_FUNC_RS485              0x01     //抄表485口
#define PORT_FUNC_CYMETER            0x02     //交采485口
#define PORT_FUNC_CASCADE            0x03     //级联485口

#define GPRS_TO_RS232                       0x01       //GPRS转串口
#define PPP_TO_RS232                        0x02       //PPP转串口
#define RS485_TO_RS232                      0x04       //485转串口
#define CYMETER_TO_RS232                    0x08       //交采转串口
#define INFRA_TO_RS232                      0x10       //红外口转串口
#define PLC_TO_RS232                        0x20       //载波口转串口
#define RS232_TO_RS4852                     0x40       //串口转级联口
#define RS232_TO_CYMETER                    0x80       //串口转交采口


#define DEBUG_3761							0x01	   //保存386.1报文
#define DEBUG_CYMETER						0x02	   //保存交采报文

#define FAST_IDX_ADD_ROUTER_FLAG   0x8000        //是否添加路由标志
#define FAST_IDX_RTU_FLAG          0x4000        //采集器标志    //__zy_set_meter__
#define FAST_IDX_METER_FLAG        0x2000        //电表序号中采集终端下电表标志位
#define FAST_IDX_DZC_METER_FLAG    0x1000        //是否为采集终端下水气热表
#define FAST_IDX_MASK              0x0FFF        //电表序号掩码

#define CYCLE_REC_MODE_CONCENTRATOR        1     //集中器主导的周期抄表模式
#define CYCLE_REC_MODE_ROUTER              2     //路由主导的周期抄表模式
#define CYCLE_REC_MODE_ALL                 3     //两种都支持
#define CYCLE_REC_MODE_PARALLEL            10     //集中器主导的并行抄读模式

#define CAST_DELAY_UNIT_1        1   //1:1s  2:0.1s
#define CAST_DELAY_UNIT_2        2   //1:1s  2:0.1s


/*集中器重启*/
#define  HARD_RESET     0x01                 /*硬件重启*/
#define  DATA_RESET     0x02                 /*数据区初始化*/
#define  PARAMS_RESET   0x03                 /*参数区初始化*/
#define  CY_CLEAR       0x04                 /*交采清零*/

#define CT  0x01
#define PT  0x02


#define CONST_CFG_DEVID				1UL			//6	终端地址
#define CONST_CFG_VENDOR			2UL			//4	厂商代码
#define CONST_VENDOR_CHIP			3UL			//4	终端厂家（山西）
#define CONST_VENDOR_CHIP2			4UL			//2芯片厂家（山西）
#define CONST_SHANGHAI_VENDOR_FLAG	5UL			//2	版本标识（上海）
#define CONST_SOFT_VER				6UL			//4+1	软件版本号
#define CONST_SOFT_DATE				7UL			//3+1	软件版本发布日期
#define CONST_CAPACITY				8UL			//11+1	容量信息码
#define CONST_PROTOCOL_VER			9UL			//4+1	协议版本号
#define CONST_HARDWARE_VER			10UL		//4+1	硬件版本号
#define CONST_HARDWARE_DATE			11UL		//3+1	硬件版本发布日期
#define CONST_ERTU_SWITCH           12UL             //4+1  终端的编译开关
#define CONST_LOG_PARAM             13UL               //2+1，日志级别及输出方式
#define CONST_DISPLAY_PWD           14UL         //8，显示密码
#define CONST_RS232_BAUDRATE        15UL         //4，串口波特率
#define CONST_FRAME_MONITOR         16UL         //2，报文监控
#define CONST_RS232_REPORT          17UL         //1，串口上报,AA-普通串口 BB-GPRS串口	其它-禁用串口上报
#define CONST_GPRS_BAUDRATE         18UL         //4，GPRS串口波特率,默认115200
#define CONST_INFRA_BAUDRATE        19UL         //4，红外口波特率
#define CONST_MNG485_BAUDRATE       20UL         //4，485维护口波特率
#define CONST_GPRS_LOCK		        21UL         //1，GPRS锁频
#define CONST_PROVINCE_FEATURE      22UL         //8,省份特征，3字节省份+2字节区域+3字节备用
#define CONST_REMOTE_TYPE           23UL         //1,远程通道选择,AA-GPRS BB-以太网 其他-自适应
#define CONST_MES                   24UL         //16,主板MES号，板级检测时设置，永久保留
#define CONST_ERTU_PLMS_SWITCH      25UL         //4，终端的参数控制字
#define CONST_230M_CHANNEL          29UL         //1，I型专变230频道，数值范围1~16
#define CONST_GPRS_AUTH_TYPE        33UL         //1，鉴权类型，55-pap，AA-chap，其他默认
#define CONST_DZC_METER_SWTICH      34UL         //3，水气热表协议切换，3个字节BCD分别是水气热表协议号
#define CONST_DZC_CJT_SWTICH        35UL         //2，与转换器通讯协议切换
#define CONST_DZC_485_BAUDRATE      36UL         //4，四表485与转换器通讯波特率
#define CONST_ERTU_SWITCH_HUBEI_F801  37UL       //1湖北F801功能切换
#define CONST_CONFIG_ONENET         45UL         //1=2 1字节 00/FF 为公网 其他为专网
#define CONST_SWITCH_PARAM          25UL         //4 默认参数配置开关 D0：冀北环保645心跳帧 上行645协议
#define CONST_DJB_VER               99UL         //蓝牙版本信息，16+1
//0~99 为系统使用，所有工程统一定义
//100以上为应用程序使用，这里只考虑376.1的
#define CONST_POWERON_PROTECT_TIME  100UL        //1376.1工程使用，四川提出，上电时自动保电时间，单位15min，0和FF为无效
//事务狗ID定义，系统使用 0~999，应用层使用1000~9999
//这里定义应用层的事务狗，从1000开始


#define CHANNEL_REMOTE  0x01
#define CHANNEL_RS232   0x02
//#define CHANNEL_INFRA   0x04
#define CHANNEL_FLASH   0x08
#define CHANNEL_STACK   0x10
#define CHANNEL_ETH     0x20
#define CHANNEL_SMS     0x40
#define CHANNEL_NONE    0x80
//以上通道不够用，由于没有按位使用，因此增加新通道，不按位定义
#define CHANNEL_GPRS_SERVER    0x81
#define CHANNEL_GPRS_CLIENT    0x82
#define CHANNEL_ETH_SERVER    0x83
#define CHANNEL_ETH_CLIENT     0x84
#define CHANNEL_BEIDOU        0x85

#define  TIME_OUT_C1    6000  //单位 10ms
#define  MAX_METER_COUNT  2000
/*************
 * 转换函数定义
 **************/
//转换函数
#define RATE                    0x0F
#define REC_DATE                0x10	//  终端时间-> 376.1 A.15
#define REC_DATE_SEC            0x11
#define MeterPowerStatus_3761FMT 0x12 //376.1 F161 电能表通断电状态
#define DXSJ_COMPLEMENT_3761FMT 0x13               //根据ABC相计算 断相总次数等
#define Datetime2FMT15          0x14   //YYWMMDDHHMMSS -> YYWMMDDHHMM
#define SetPhaseV               0x15   //默认电压相角0 120 240
#define A3toA2                  0x16   //3字节变成2字节
#define Datetime09toFMT15       0x17   //09规约的时标5字节
#define I3toI2                  0x18   //电流的格式转换
#define RATE_1                  0x19   //费率1
#define DELEWEEK                0x20   //去除星期
#define JSJL_HSJ                0x21   //校时记录后时间
#define JSJL_QSJ                0x22   //校时记录前时间
#define SYDL_ZDZ_2_376          0x23   //江苏剩余电流最大值 数据顺序倒换
#define DLSJK_FORMAT3to4        0x24   //江苏断路器电流数据转换
#define BbreakerIto376          0x25   //江苏剩余电流块电流数据转换
#define CbreakerIto376          0x26   //江苏剩余电流块电流数据转换
#define GetSYDL                 0x27   //江苏剩余电流块提取电流数据
#define GET_REC_DATE            0x28   //江苏曲线时间
#define GET_RQ                  0x29   //日期星期取出日期
#define AbreakerIto376          0x30   //江苏剩余电流块电流数据转换
#define Energy_3761FMT14_BLOCK  0x31   //正向有功数据块转换
#define METER_STATUS            0x32   //电表状态字变位




//内部数据 宏定义
#define ZDCB_SJ_XN              0x80000001
#define FL_S_XN                 0x80000002
#define XB_CS_XN                0x80000003
#define XSDJ_SJ_SB_XN           0x80000004
#define RDJ_SJ_SB_XN            0x80000005
#define YDJ_SJ_SB_XN            0x80000006
#define QX_SJ_SB_XN             0x80000007
#define DBCD_XX_XN              0x80000008
#define DBYX_ZTZ_BWBZ_XN        0x80000009
#define ZBB_DY_XJ_XN            0x8000000A
#define DX_BC_XN                0x8000000B
#define YM_JS_DNL_XN            0x8000000C
#define ABCN_I_CURVE            0x8000000E     //ABCN相电流曲线，需转换，内蒙电流为2字节  
#define ABC_DUANXIANG_COUNT     0x8000000F     //ABC相断相次数，需转换，内蒙要求为2字节
#define FK2005_XSDJ_SJ_MD_XN    0x80000010      //负控2005小时冻结，时间与密度占一字节
#define METER_ZH_CURVE          0x80000011      //电能表综合数据曲线，需转换电流
#define ZXYG_REC_STATE_CURVE    0x80000012      //正向有功总及抄表状态信息曲线，根据正向有功总的值手动添加状态值
#define JILIN_CURVE             0x80000013      //吉林增加半点值
#define CHECK_IS_HAVE_EE        0x80000014      //判断是否含有EE 只要有无效数据 fn覆盖为全EE （四川需求） 
#define DXB_XLQL_EE             0x80000015      //判断是否含有EE 只要有无效数据 fn覆盖为全EE （黑龙江台体）




#define NOT_BCD                 0x8000
#define NOT_FL_DATA             0x01
#define NOT_SINGLE              0x08 //单相表不支持的数据


#define LEN_ERTU_CFG_VERINFO        41

#define HUANBAO_JIBEI          00000001 //冀北环保局645协议上行 特殊登陆心跳帧



#endif




