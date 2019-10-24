
#ifndef CJSONFRAMEPACK_H
#define CJSONFRAMEPACK_H
#include "cJSON.h"
#include "../main_include.h"
#ifdef __cplusplus
extern "C" {
#endif

#define __SIMULATION__
//#define __BEIJING_ONENET_EDP__
//#define __ZHEJIANG_ONENET_EDP__
#ifdef __BEIJING_ONENET_EDP__
//设备配置信息
INT16U  cjsonDeviceConfigPack(INT8U * buf);
//事件告警信息
INT16U cjsonEventWarningPack(INT8U *buf,INT8U flag);
//固件升级信息
INT16U cjsonFirmwareUpdatePack(INT8U *buf,INT8U flag);
//实时电量信息
INT16U cjsonEnergyInfoPack(char dayhold,INT8U *buf);
//电能量信息数据块
INT16U cjsonEnergyBlockPack(INT8U *buf);
INT16U cjsonReadMeter(INT8U *buf);
#endif

#ifdef __ZHEJIANG_ONENET_EDP__
//设备配置信息
INT16U  cjsonDeviceConfigPack(INT8U * buf);
//事件告警信息
//INT16U cjsonEventWarningPack(INT8U *buf,INT8U flag);
INT16U cjsonEventWarningPack(INT8U *buf,INT8U flag,INT8U data);
//固件升级信息
INT16U cjsonFirmwareUpdatePack(INT8U *buf,INT8U flag);
//实时电量信息
INT16U cjsonEnergyInfoPack(char dayhold,INT8U *buf);
//电能量信息数据块
INT16U cjsonEnergyBlockPack(INT8U *buf);
INT16U cjsonReadMeter(INT8U *buf);
//月冻结数据
INT16U cjsonEnergyBlockPackMonth(INT8U *buf);

#endif

#ifdef __cplusplus
}
#endif

#endif