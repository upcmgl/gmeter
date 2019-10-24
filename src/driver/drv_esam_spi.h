/*
 * drv_esam_spi.h
 *
 * Created: 2018/12/19 星期三 11:50:23
 *  Author: Administrator
 */ 


#ifndef DRV_ESAM_SPI_H_
#define DRV_ESAM_SPI_H_
#include "os.h"

#define ESAM_1376  1
#define ESAM_OOP   2

uint16_t esam_get_esam_id(uint8_t *buffer,uint16_t max_len);
uint16_t esam_get_offline_counter(uint8_t *buffer,uint16_t max_len);
uint16_t esam_get_chip_state(uint8_t *buffer,uint16_t max_len);
uint16_t esam_get_key_version(uint8_t *buffer,uint16_t max_len);
uint16_t esam_get_cert_id(uint8_t *buffer,uint16_t max_len);
uint16_t esam_get_public_key(uint8_t *buffer,uint16_t max_len);
uint16_t esam_session_init(uint8_t *data,uint8_t *buffer,uint16_t max_len);
uint16_t esam_session_key_consult(uint8_t *data,uint8_t *buffer,uint16_t max_len);
bool esam_symmetric_key_update(uint8_t *data,uint8_t *buffer,uint16_t max_len);
bool esam_ca_certificate_update(uint8_t *data,uint8_t *buffer,uint16_t max_len);
uint16_t esam_internal_auth(uint8_t *data,uint8_t *buffer,uint16_t max_len);
bool esam_external_auth(uint8_t *data,uint8_t *buffer,uint16_t max_len);
uint16_t esam_get_random_num(uint8_t randonlen,uint8_t *buffer,uint16_t max_len);
bool esam_cert_state_change(uint8_t *data,uint8_t *buffer,uint16_t max_len);
bool esam_set_offline_counter(uint8_t *data,uint8_t *buffer,uint16_t max_len);
bool esam_change_data_authorize(uint8_t *data,uint8_t *buffer,uint16_t max_len);
bool esam_verify_mac2(uint8_t kid,uint8_t *data,uint16_t datalen,uint8_t *buffer,uint16_t max_len,uint8_t mac[4]);
bool esam_group_broadcast(uint8_t kid1,uint8_t kid2,uint16_t group_id,uint8_t *data,uint16_t datalen,uint8_t *buffer,uint16_t max_len,uint8_t mac[4]);
uint16_t esam_identity_authentication(uint8_t P2,uint8_t *data,uint8_t datalen,uint8_t *buffer,uint16_t max_len);
uint16_t esam_set_time(uint8_t cur_datatime[7],uint8_t task_data[40],uint8_t meter_er[32],uint8_t meter_rdn[4],uint8_t *buffer,uint16_t max_len);
void esam_get_info(void);
int16_t esam_get_tesam_info(uint8_t *buffer,uint16_t max_len);
uint16_t tesam_get_esam_id(uint8_t *buffer,uint16_t max_len);
uint16_t tesam_get_chip_state(uint8_t *buffer,uint16_t max_len);
void drv_esam_spi_high_Initialize(void);

int16_t drv_esam_spi_ReadArray(uint8_t* data, uint16_t max_len,uint8_t* headdata,INT16U *err_code);
bool drv_esam_spi_WriteArray(uint8_t* data, uint16_t size);

#endif /* DRV_ESAM_SPI_H_ */