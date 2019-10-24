#ifndef __FRAME_RECV_SERVICE_H__
#define __FRAME_RECV_SERVICE_H__
#include "main_include.h"

INT8U  is_1376_1_frame_header(INT8U *frame, INT16U recv_len);
INT8U is_gb645_frame_header(INT8U *frame,INT16U recv_len);
INT16U  get_1376_1_frame_len(INT8U *frame, INT16U recv_len);
INT16U  get_gb645_frame_len(INT8U *frame, INT16U recv_len);
INT16U  get_gboop_frame_len(INT8U *frame, INT16U recv_len);
INT16U init_1376_1_frame_checksum(INT8U *frame, INT16U frame_len, INT16U recv_len);
INT16U init_gb645_frame_checksum(INT8U *frame, INT16U frame_len, INT16U recv_len);
INT8U  is_1376_1_frame_ready(INT8U *frame, INT16U frame_len, INT16U recv_len,INT16U *cs);
INT8U  is_gb645_frame_ready(INT8U *frame, INT16U frame_len, INT16U recv_len,INT16U *cs);
INT8U is_oop_protocol_frame_ready(INT8U *frame, INT16U frame_len, INT16U recv_len,INT16U *cs);
INT8U check_frame_header_gb_oop_up(INT8U* frame);
INT8U check_frame_header_gb_oop_down(INT8U* frame);
INT8U* trim_oopframe_FE(INT8U* frame);
INT8U get_oopframe_addr_length(INT8U* frame);
INT16U get_frame_header_length_gb_oop(INT8U* frame);

#endif