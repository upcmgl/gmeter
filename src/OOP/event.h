/*
 * event.h
 *
 * Created: 2019-1-24 14:52:56
 *  Author: mgl
 */ 


#ifndef EVENT_H_
#define EVENT_H_
#include "main_include.h"
typedef struct {
	INT8U eventIndex[5];
	INT8U beginTime[8];
	INT8U endTime[8];
	INT8U eventSource[2];
	INT8U reportState[2];
	INT8U eventAttribute[3];
	INT8U has_report;
}StrTerminalPowerOnOff;
//终端停上电数据块
typedef struct {
	INT8U eventIndex;
	StrTerminalPowerOnOff  event[15];
}StrTerminalPowerOnOffEventBlock;
StrTerminalPowerOnOffEventBlock    TerminalPowerOnOffEvent;

INT8U  TerminalPowerOnOffState;


void powerOnOff();
void reportOnOffEvent(INT8U event_report_idx);
#endif /* EVENT_H_ */