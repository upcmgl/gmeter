#ifndef __TASK_NB_ONENET_H__
#define __TASK_NB_ONENET_H__


void remote_send_app_frame(uint8_t *frame,uint16_t frame_len);
void task_1376_3_service(void);
int16_t remote_read_byte(void);
int16_t remote_read_byte2(void);
void force_gprs_power_off(void);
void update_gprs_param(void);
#endif