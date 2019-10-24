#ifndef __TASK_PROTOCOL_SERVICE_H__
#define __TASK_PROTOCOL_SERVICE_H__

void task_protocol_service(void);
void task_protocol_handler(void);

void app_send_ReplyFrame(objResponse *Response);
#endif