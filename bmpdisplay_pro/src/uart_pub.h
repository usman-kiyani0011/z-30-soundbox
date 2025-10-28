#pragma once
#include "pub/cJSON.h"

typedef struct __st_pkt_info {
	int flag;
	int func;
	int seq;
	char* pbuff;
	int len;
	int ret;
	int format;
}st_pkt_info;

typedef struct __st_qr_data {
	st_pkt_info pkt_info;

	char amt[20];	  //The transaction amount
	char qrdata[1024]; //qr data
	char text[64];	//Payment success message
	char showTime;
}st_qr_data;

#ifdef WIN32
#define MPOS_PORT_COM	11
#else
#define MPOS_PORT_COM	APP_COM
#endif

#define	STX_CODE_1 			0x4D
#define	STX_CODE_2 			0x46
#define	ETX_CODE			0x02
#define	ETX_CODE_json			"ED"

#define PROC_JSON_FORMAT			0x01
#define PROC_MPOS_FORMAT			0x02


#define	MPOS_FUNC_RESET			0x01
#define	MPOS_FUNC_SET_TIME		0x02
#define	MPOS_FUNC_SET_QR		0x03
#define	MPOS_FUNC_SET_TEXT		0x04
#define	MPOS_FUNC_SET_VOICE		0x05
#define	MPOS_FUNC_RF_POWER		0x06
#define	MPOS_FUNC_RF_EXCHANGE	0x07

#define CUSTOM_SHOW_QR              0x800C  //show QR

void uart_pub_check_sum_update(unsigned char* check_value, unsigned char* buff, int length);
int uart_pub_send_pkt(unsigned char* pdata, int len, st_pkt_info* pkt);
int uart_pub_pkt_check(unsigned char* pdata, int len, st_pkt_info* pkt);
int uart_pub_pkt_check_json(unsigned char* pdata, int len, st_pkt_info* pkt);
void uart_pub_pack_str(unsigned char* pbuff, unsigned char* str, int size);
int uart_pub_get_json(cJSON* rootobj, char* key, char* val, int size);

int uart_pub_get_ll_len(unsigned char* pbuff);
void uart_pub_set_ll_len(unsigned char* pbuff, int len);
int uart_pub_get_len(unsigned char* pbuff);
void uart_pub_set_len(unsigned char* pbuff, int len);
int uart_pub_unpack_ll_data(unsigned char* pbuff, unsigned char* pdata, int size);

unsigned int uart_pub_get_int_2(unsigned char* pbuff);
unsigned int uart_pub_get_int_4(unsigned char* pbuff);
void uart_pub_set_int_2(unsigned int count, unsigned char* pbuff);
void uart_pub_set_int_4(unsigned int count, unsigned char* pbuff);

int uart_pub_atoi(char* pbuff, int len);

int uart_pub_pkt_tlv_bin(char* buff, int offset, int tag, char* val, int length);
int uart_pub_pkt_tlv_int2(char* buff, int offset, int tag, int val);
int uart_pub_pkt_tlv_int(char* buff, int offset, int tag, int val);
int uart_pub_pkt_tlv_str(char* buff, int offset, int tag, char* val);

int uart_pub_pkt_tlv_get(char* buff, int size, int tag1, char* val);

void uart_proc();
