#ifndef __PIC32MX_LLVC_NEIMENG_H__
#define __PIC32MX_LLVC_NEIMENG_H__
//参数文件定义
#define     NMADDR_SET_F63               0                            //18432, 配置低压居民电表参数,一个电表18个字节*1024=18432=18k
#define     NMADDR_SET_F90          NMADDR_SET_F63+18432             //10，抄表数据量选配参数
#define     NMADDR_SET_F91          NMADDR_SET_F90+10                //11，第二通讯通道工作方式配置及两个通讯通道的启用参数
#define     NMADDR_SET_F96          NMADDR_SET_F91+11                //5120，设置电表中继约束
#define     NMADDR_SET_F105         NMADDR_SET_F96+5120              //1025，终端管理的中继器地址参数
#define     NMADDR_SET_F106         NMADDR_SET_F105+1025             //704，终端电能表中继路由配置参数
#define     NMADDR_SET_F107         NMADDR_SET_F106+704             //11264,居民电表中继路由配置参数
#define     NMADDR_SET_F85          NMADDR_SET_F106+11264           //512,电表有效性/和重点用户标志


//一类数据扩展文件定义
#define     NMADDR_C1_RECSTATUS     0                               //3968,抄表状态，1块表保存31天，1024/8*31=3968,一个字节8个表状态

#define    NM_MAX_METER_COUNT       1024                            //内蒙古目前支持的最大电表数量
#define    ADD_METER_COUNT          10                              //电表序号映射到配置序号时需要加64    
typedef union{
  INT8U value[40];
  struct{
    INT8U  ipport_1[6];
    INT8U  ipport_2[6];
    INT8U  gate_ipport[6];
    INT8U  proxy_ipport[6];
    INT8U  apn[16];
  }d;
  struct{
    INT8U  ipport_1[6];
    INT8U  ipport_2[6];
    INT8U  apn[16];
  }n;
}tagSET_F3_nm;
//参数F63：配置低压居民电表参数
typedef union{
   INT8U  values[18];
   struct{
     INT8U  meter_idx[2];       //电表序号
     INT8U  meter_type;              //电表类型01H：单费率485分表 02H：单费率载波分表 03H：单费率485台区总表
                                     //04H：多费率485分表05H：多费率载波分表 06H：多费率台区总表07H：单费率脉冲分表08H：
                                     //单费率脉冲总表09H：多费率脉冲分表  0AH：多费率脉冲总表
     INT8U  meter_addr[6];     //电表地址 ,6个BCD码
     INT8U  meter_const[2];     //电表常数：1-65535
     INT8U  rtu_no[2];          //所属采集器地址1-65535（若为总表，由集中器自身的485接口直接抄收，该值为0，对于载波表，该值为0）
     INT8U  rtu_port;           //所属采集器端子号1-32（对于脉冲电表，必须标明该表属于哪个采集器的哪个端子号，对于485电表可以不使用，对于载波表可以不用）
     INT8U  ctrl_flag;          //重点户、拉合闸标志、停用标志、规约类型
     INT8U  data_item;          //采集数据项
     INT8U  read_interval;      //采集间隔 ：以小时为单位
     INT8U  meter_bytes;        //表位数
   };
}tagSET_F63;

typedef union{
   INT8U  value;
   struct {
    INT8U protocol:3;
    INT8U use_flag:2;
    INT8U remote_ctrl:2;
    INT8U is_vip:1;
   };
}tag_F63_CTRL_FLAG;



void update_rec_meter_status_neimeng(INT8U day,INT16U meter_idx);
void init_rec_meter_status_neimeng(BOOLEAN is_all,INT8U cur_day);
INT16U  get_gb645_data(INT8U *gb645,INT16U frameLen,BOOLEAN is_33);
BOOLEAN query_meter_info_neimeng(INT8U *data,INT16U *itemDataLen,INT16U max_len);
INT8U set_ertu_meters_neimeng(INT8U *buffer,INT8U *params,INT16U *itemDataLen);
void update_f85_use_flag(INT16U meter_idx,BOOLEAN valid_flag);
void update_f85_vip_flag(INT16U meter_idx,BOOLEAN valid_flag);
INT16U plc_readRelay_645meter(INT8U *resp,INT16U meter_idx,INT16U item);
void update_meter_use_flag(INT16U meter_idx,INT8U is_use);
void update_vip_param(void);
INT8U set_ertu_F85_nm(INT8U *itemData,INT16U *itemDataLen);
INT8U set_ertu_meters_F96(INT8U *buffer,INT8U *itemData,INT16U *itemDataLen);
BOOLEAN query_ertu_meters_F96(INT8U *Reply,INT16U *itemDataLen,QUERY_CTRL *query_ctrl);
void calc_seg_time();
INT16U read_c1_f10(INT8U *data,READ_HIS *readC1);
INT16U read_c1_f11(INT8U *data,READ_HIS *readC1);
INT8U get_vip_meter_idx(INT16U meter_idx);


INT16U set_spot_params(INT16U spot_idx,INT32U eeAddr,INT8U *params,INT16U itemDataLen);
//F3:设置终端主站参数
INT8U set_ertu_msa_info_nm(INT8U *itemData,INT16U *itemDataLen);
INT16U query_nm_msa_info(INT8U *data);

//F7:抄表日参数
INT8U set_ertu_recday_info_nm(INT8U *itemData,INT16U *itemDataLen);
INT16U query_nm_recday_info(INT8U *data);


//F10:终端电能表/交流采样装置配置参数
INT16U query_nm_meter_info(INT8U *data,INT16U max_len);
INT8U set_ertu_recinterval_info_nm(INT8U *itemData,INT16U *itemDataLen,INT8U port_type);
INT16U query_nm_recinterval_info(INT8U *data,INT8U port_type);
INT8U set_ertu_recstarttime_info_nm(INT8U *itemData,INT16U *itemDataLen);
INT16U query_nm_recstarttime_info(INT8U *data);
INT16U query_nm_ertu_F85(INT8U *data,QUERY_CTRL *query_ctrl);
void set_ertu_f9(INT8U *itemData);
INT16U query_ertu_f27(INT16U pn_idx,INT8U *itemData);
INT16U set_ertu_f27(INT16U pn_idx,INT8U *itemData);
INT16U  set_spot_params_nm(INT16U spot_idx,INT16U fn,INT8U *params,INT16U itemDataLen);
INT16U get_F27F28_C1_nm(INT8U *data,C1F27 *cur_F27,C1F28 *cur_F28);
INT8U set_ertu_F60_nm(INT8U *params,INT16U *itemDataLen);
INT16U query_ertu_F60_nm(INT8U *data);
INT8U set_ertu_F21_nm(INT8U *params,INT16U *itemDataLen);
INT16U query_ertu_F21_nm(INT8U *data);
INT8U set_ertu_F22_nm(INT8U *params,INT16U *itemDataLen);
INT16U query_ertu_F22_nm(INT8U *data);
INT16U set_ertu_F25_nm(INT16U spot_idx,INT8U *params);
INT16U query_ertu_F25_nm(INT16U spot_idx,INT8U *data);
INT16U set_ertu_F26_nm(INT16U spot_idx,INT8U *params);
INT16U query_ertu_F26_nm(INT16U spot_idx,INT8U *data);
INT8U set_ertu_F81_nm(INT8U *params,INT16U *itemDataLen);
INT16U query_ertu_F81_nm(INT8U *data);
INT16U query_nm_F14(INT8U *Reply);
void default_param_neimeng(void);
#endif

