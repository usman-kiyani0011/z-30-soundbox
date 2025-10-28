#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lvgl/lvgl.h"
#include "uart_pub.h"
#include "tracedef.h"
//#include "pub/code/Unicode.h"
#include "libapi_xpos/inc/libapi_system.h"
#include "libapi_xpos/inc/mfsdk_util.h"
#include "libapi_xpos/inc/mfsdk_sys.h"
#include "libapi_xpos/inc/mfsdk_comm.h"
#include "libapi_xpos/inc/mfsdk_mem.h"

#include "page_main.h"



#define PKT_SIZE	128
int m_card_type = 0;
static st_qr_data ET389_qr_data = { 0 };

void uart_pub_check_sum_update(unsigned char* check_value, unsigned char* buff, int length)
{
	int i;
	for (i = 0; i < length; i++) {
		*check_value = *check_value ^ buff[i];
	}
}

static int _uart_send(char* buf, int count)
{
	int ret;
//	ret = Sys_uart_send(buf, count);
	ret = MfSdkCommUartSend(MFSDK_COMM_UART_COM26,(u8*)buf, (u32)count);
	return ret;
}

void uart_pub_pack_data(char* buff, char* key, char* val)
{
	if (strlen(buff) == 0) {
		sprintf(buff + strlen(buff), "%s=%s", key, val);
	}
	else {
		sprintf(buff + strlen(buff), "&%s=%s", key, val);
	}

}

static void uart_pub_pack_json(char* val)
{
	int size = strlen(val);
	char* buff = (char*)MfSdkMemMalloc(size + 256);
	int i = 0;
	int j = 0;
	char* item[50];
	int len[50];
	int count = 0;
	char* src = val;
	char key[32] = { 0 };
	char data[512] = { 0 };
	int rl = 0;

	memset(buff, 0, size + 256);

	item[0] = val;
	len[0] = 0;

	for (i = 0; i < size; i++) {
		if (*val == '&') {
			count++;
			item[count] = val + 1;
			len[count] = 0;
		}
		else {
			len[count] ++;
		}
		val++;
	}
	if (len[count] > 0) count++;

	strcpy(buff, "{");
	for (i = 0; i < count; i++) {

		memset(key, 0, sizeof(key));
		memset(data, 0, sizeof(data));

		for (j = 0; j < len[i]; j++) {
			if (*(item[i] + j) == '=') {
				rl = len[i] - j - 1;
				if (j > 0) memcpy(key, item[i], j);
				if (rl > 0)	memcpy(data, item[i] + j + 1, rl);
				sprintf(buff + strlen(buff), "\"%s\":\"%s\",", key, data);
				break;
			}
		}
	}

	buff[strlen(buff) - 1] = 0;
	strcat(buff, "}");
	memcpy(src, buff, strlen(buff));

	APP_TRACE("send buff:%s\r\n", src);

	MfSdkMemFree(buff);
}

//STX(0x4D46)  Data length	 Instruction number  Command number Serial number Response code Variable data	ETX(0x02)	LRC	
//2bytes		2 bytes			 1 byte				2 bytes			1 byte		 2 bytes	   variable		 1 byte	    1byte
int uart_pub_send_pkt(unsigned char* pdata, int len, st_pkt_info* pkt)
{
	unsigned char check_sum = 0;
	unsigned char head[10];
	unsigned char end[2];
	int size = 6 + len;
//	int iSendLen = 0;
//	int i = 0;
	unsigned char* buf;
	int count = sizeof(head) + len + sizeof(end);

	buf = (u8*)MfSdkMemMalloc(count);

	head[0] = STX_CODE_1;
	head[1] = STX_CODE_2;

	uart_pub_set_ll_len(&head[2], size);
	head[4] = 0x4F;
	head[5] = pkt->func / 256;
	head[6] = pkt->func % 256;
	head[7] = (pkt->seq + 1) & 0x7f;
	head[8] = pkt->ret / 10 + '0';
	head[9] = pkt->ret % 10 + '0';

	uart_pub_check_sum_update(&check_sum, head + 2, sizeof(head) - 2);

	if (len > 0) uart_pub_check_sum_update(&check_sum, pdata, len);

	end[0] = ETX_CODE;
	uart_pub_check_sum_update(&check_sum, end, 1);
	end[1] = check_sum;
	memcpy(buf, head, sizeof(head));
	memcpy(buf + sizeof(head), pdata, len);
	memcpy(buf + sizeof(head) + len, end, sizeof(end));
	_uart_send((char*)buf, count);
	APP_TRACE_BUFF_LOG(buf, count, "send to host:");

	MfSdkMemFree(buf);
	return 0;
}

int uart_pub_send_pkt_json(st_pkt_info* pkt)
{
	char head[7] = { 0 };
	char end[3] = { 0 };
	char* buf;
	int count = 0;
	char cmd[3] = { 0 };
	char res[3] = { 0 };
	char tbuf[512] = { 0 };

	int mlength = sizeof(head) + sizeof(tbuf) + sizeof(end);		
	buf = (char*)MfSdkMemMalloc(mlength);

	head[0] = STX_CODE_1;
	head[1] = STX_CODE_2;

	sprintf(cmd, "%02d", pkt->func);
	uart_pub_pack_data(tbuf, "C", cmd);
	res[0] = pkt->ret / 10 + '0';
	res[1] = pkt->ret % 10 + '0';
	uart_pub_pack_data(tbuf, "R", res);

	uart_pub_pack_json(tbuf);
	uart_pub_set_len((unsigned char*)&head[2], strlen(tbuf));
	strcpy(end, ETX_CODE_json);
	memset(buf, 0, mlength/*sizeof(buf)*/);
	memcpy(buf, head, strlen(head));
	memcpy(buf + strlen(head), tbuf, strlen(tbuf));
	memcpy(buf + strlen(head) + strlen(tbuf), end, strlen(end));
	count = strlen(head) + strlen(tbuf) + strlen(end);
	_uart_send(buf, count);
	MfSdkMemFree(buf);
	return 0;
}

int uart_pub_pkt_check(unsigned char* pdata, int len, st_pkt_info* pkt)
{
	pkt->flag = pdata[0];
	pkt->func = pdata[1] * 256 + pdata[2];
	pkt->seq = pdata[3];
	pkt->pbuff = (char*)&pdata[4];
	pkt->len = len - 4;
	pkt->ret = 0;
	pkt->format = 0x02;		//mpos tlv
	if (pkt->flag != 0x2F && pkt->flag != 0x3F && pkt->flag != 0x4F) return -2;

	return 0;
}

int uart_pub_pkt_check_json(unsigned char* pdata, int len, st_pkt_info* pkt)
{
	cJSON* rootobj = 0;
	char temp[5] = { 0 };

	rootobj = (cJSON*)cJSON_Parse((const char *)pdata);
	if (NULL != rootobj)
	{
		uart_pub_get_json(rootobj, "C", temp, 2);
		APP_TRACE("[uart_pub_pkt_check_json][cmd:%s]\r\n" , temp);
		pkt->func = atoi(temp);
		pkt->pbuff = (char*)&pdata[0];
		pkt->pbuff[len] = 0;
		pkt->len = len;
		pkt->ret = 0;
		pkt->format = 0x01; //json
		cJSON_Delete(rootobj);
	}

	return 0;
}
void uart_pub_pack_str(unsigned char* pbuff, unsigned char* str, int size)
{
	int len;
	len = strlen((char*)str);
	if (len > size) len = size;
	memset(pbuff, 0x20, size);
	memcpy(pbuff, str, len);
}

int uart_pub_get_ll_len(unsigned char* pbuff)
{
	int len = 0;
	len = (pbuff[0] & 0xf0) / 16 * 1000 + (pbuff[0] & 0x0f) * 100;
	len += (pbuff[1] & 0xf0) / 16 * 10 + (pbuff[1] & 0x0f);
	return len;
}

void uart_pub_set_ll_len(unsigned char* pbuff, int len)
{
	pbuff[0] = (len / 1000) * 16 + (len / 100 % 10);
	pbuff[1] = (len / 10 % 10) * 16 + (len % 10);
}

int uart_pub_get_len(unsigned char* pbuff)
{
	int len = 0;
	char temp[5] = { 0 };

	memcpy(temp, pbuff, 4);
	len = atoi(temp);

	return len;
}

void uart_pub_set_len(unsigned char* pbuff, int len)
{
	sprintf((char*)pbuff, "%04d", len);
}

int uart_pub_unpack_ll_data(unsigned char* pbuff, unsigned char* pdata, int size)
{
	int len;
	len = uart_pub_get_ll_len(pbuff);
	if (len > size) return 0;

	if (len > 0) {
		memcpy(pdata, pbuff + 2, len);
	}


	return len + 2;
}

unsigned int uart_pub_get_int_2(unsigned char* pbuff)
{
	unsigned int count = 0;
	count = pbuff[0] * 256 + pbuff[1];
	return count;
}

unsigned int uart_pub_get_int_4(unsigned char* pbuff)
{
	unsigned int count = 0;
	count = pbuff[0] * 0x1000000 + pbuff[1] * 0x10000 + pbuff[2] * 0x100 + pbuff[3];;
	return count;
}


void uart_pub_set_int_2(unsigned int count, unsigned char* pbuff)
{
	pbuff[0] = count % 0x100;
	pbuff[1] = count / 0x100 % 0x100;
}

void uart_pub_set_int_4(unsigned int count, unsigned char* pbuff)
{

	pbuff[3] = count % 0x100;
	pbuff[2] = count / 0x100 % 0x100;
	pbuff[1] = count / 0x10000 % 0x100;
	pbuff[0] = count / 0x1000000 % 0x100;
}


int uart_pub_atoi(char* pbuff, int len)
{
	char tmp[12];
	int data;
	if (len > 10) return 0;

	memcpy(tmp, pbuff, len);
	tmp[len] = 0;

	data = atoi(tmp);
	return data;
}

int uart_pub_pkt_tlv_bin(char* buff, int offset, int tag, char* val, int length)
{
	buff[offset++] = tag;
	buff[offset++] = length / 256;
	buff[offset++] = length % 256;
	memcpy(buff + offset, val, length);
	return (offset + length);
}

int uart_pub_pkt_tlv_int2(char* buff, int offset, int tag, int val)
{
	unsigned char bin[2];
	bin[0] = val / 256;
	bin[1] = val % 256;
	return uart_pub_pkt_tlv_bin(buff, offset, tag, (char*)bin, 2);
}

int uart_pub_pkt_tlv_int(char* buff, int offset, int tag, int val)
{
	unsigned char bin[1];
	bin[0] = val;
	return uart_pub_pkt_tlv_bin(buff, offset, tag, (char*)bin, 1);
}

int uart_pub_pkt_tlv_str(char* buff, int offset, int tag, char* val)
{
	return uart_pub_pkt_tlv_bin(buff, offset, tag, val, strlen(val));
}

int uart_pub_pkt_tlv_get(char* buff, int size, int tag1, char* val)
{
	int offset = 0;
	int tag2;
	int length;
	int ret = -1;
	unsigned char* pbuff = (unsigned char*)buff;
	while (offset < size) {
		tag2 = buff[offset++];
		length = buff[offset++] * 256;
		length += buff[offset++];

		if (tag1 == tag2) {
			if (length > 0) memcpy(val, buff + offset, length);
			val[length] = 0;
			ret = 0;
		}
		offset += length;
	}
	MFSDK_UNUSED(pbuff);
	return ret;
}

int uart_pub_get_json(cJSON* rootobj, char* key, char* val, int size)
{
	int ret = -1;
	cJSON* itemobj = (cJSON*)cJSON_GetObjectItem(rootobj, key);

	size--;
	strcpy(val, "");
	if (NULL != itemobj) {
		char* pdata = itemobj->valuestring;
		APP_TRACE("[%s][%d][valuestring:%s]",__FUNCTION__,__LINE__, itemobj->valuestring);
		int len = strlen(pdata);
		char* p_ucs = 0;
		char* p_asc = 0;

		p_ucs = (char*)MfSdkMemMalloc(len + 1);
		p_asc = (char*)MfSdkMemMalloc(len + 1);
		memset(p_ucs, 0, len + 1);
		memset(p_asc, 0, len + 1);

		strcpy(p_ucs, pdata);
		ret = MfSdkUtilUtf8str2Astr(p_ucs, strlen(p_ucs),(u8*)p_asc, len + 1);
		if (ret >= strlen(p_ucs)) { //Conversion failed
			strcpy(val, p_ucs);
			MfSdkMemFree(p_ucs);
			MfSdkMemFree(p_asc);
			return 1;
		}

		if (strlen(p_asc) < size) {
			strcpy(val, p_asc);
		}
		else {
			memcpy(val, p_asc, size);
			val[size] = 0;
		}


		MfSdkMemFree(p_ucs);
		MfSdkMemFree(p_asc);
		ret = 1;
	}

	return ret;
}

#define MAX_UART_BUF		512
#define RECV_BUFF_SIZE		512

static unsigned int _uart_recv(unsigned char* buff, int size)
{
	return MfSdkCommUartRecv(MFSDK_COMM_UART_COM26,buff,(u32)size,10);
}

int comm_pub_get_ll_len(unsigned char* pbuff)
{
	int len = 0;
	len = (pbuff[0] & 0xf0) / 16 * 1000 + (pbuff[0] & 0x0f) * 100;
	len += (pbuff[1] & 0xf0) / 16 * 10 + (pbuff[1] & 0x0f);
	return len;
}

void custom_show_QR_proc(st_pkt_info* pkt_info)
{
	int data_len = 0;
	unsigned char state[1];
	unsigned char* pbuff = (u8*)pkt_info->pbuff;
	static unsigned char* qrdata = NULL;
	data_len = comm_pub_get_ll_len(pbuff + 1);
	if (NULL != qrdata)
	{
		MfSdkMemFree(qrdata);
		qrdata = NULL;
	}
	qrdata = (u8*)MfSdkMemMalloc(data_len + 1);
	memset(qrdata, 0x00, data_len + 1);
	memcpy(qrdata, pbuff + 3, data_len);
	APP_TRACE("qr data[%d]:%s", data_len, qrdata);
	//page_code();
	show_page_qrcode(get_mainpage(),"",(char*)qrdata);
	state[0] = 0x00;
	uart_pub_send_pkt(state, 1, pkt_info);
}
void uart_set_qrdata(st_pkt_info* pkt_info)
{
//	unsigned char state[1];
	int n = 0;
	cJSON* rootobj = 0;
	char* temp = (char*)MfSdkMemMalloc(1024);
	char szSha1Data[21] = { 0 };
	char szDate[40] = { 0 };

	MFSDK_UNUSED(n);
	MFSDK_UNUSED(szSha1Data);

	APP_TRACE("[%s][%d]", __FUNCTION__, __LINE__);
//	Sys_GetDateTime(szDate);
	MfSdkSysGetDateTime((u8*)szDate);

	rootobj = cJSON_Parse(pkt_info->pbuff);
	if (NULL != rootobj)
	{
		memset(&ET389_qr_data, 0, sizeof(ET389_qr_data));
		uart_pub_get_json(rootobj, "A", temp, 16);
		strcpy(ET389_qr_data.amt, temp);
		APP_TRACE("[%s][%d][amt:%s]", __FUNCTION__, __LINE__, ET389_qr_data.amt);

		memset(temp, 0, 1024);
		uart_pub_get_json(rootobj, "D", temp, 1024);
		strcpy(ET389_qr_data.qrdata, temp);
		APP_TRACE("[%s][%d][qrdata:%s]", __FUNCTION__, __LINE__, ET389_qr_data.qrdata);

		cJSON_Delete(rootobj);
	}
	MfSdkMemFree(temp);

//	state[0] = 0x00;
	uart_pub_send_pkt_json(pkt_info);
}
//STX(0x4D46)  Data length	 Instruction number  Command number Serial number Response code Variable data	ETX(0x02)	LRC	
// 2bytes		2 bytes			 1 byte				2 bytes			1 byte		 2 bytes	   variable		 1 byte	    1byte
int uart_func_proc(unsigned char* data, int len, int flag)
{
	int ret = -1;
	st_pkt_info pkt_info;

	if (flag == PROC_MPOS_FORMAT)
		ret = uart_pub_pkt_check(data, len, &pkt_info);
	else if (flag == PROC_JSON_FORMAT)
		ret = uart_pub_pkt_check_json(data, len, &pkt_info);

	APP_TRACE("ret:%d----func:%04x", ret, pkt_info.func);

	if (ret != 0)
		return ret;
	if (pkt_info.func == CUSTOM_SHOW_QR)      //show QR code
	{
		custom_show_QR_proc(&pkt_info);
	}
	else if (MPOS_FUNC_SET_QR == pkt_info.func)
	{
		uart_set_qrdata(&pkt_info);
		show_page_qrcode(get_mainpage(), ET389_qr_data.amt, ET389_qr_data.qrdata);
	}
	return 0;
}

void uart_proc()
{
//	int i;
	int recv_count;
	int data_len = 0;
	unsigned char check_sum = 0;
	unsigned char m_recv_buff[RECV_BUFF_SIZE + 1];
	unsigned char recv_tmp_buff[RECV_BUFF_SIZE + 1];

	while (1)
	{
		recv_count = 0;
		check_sum = 0;
		memset(recv_tmp_buff, 0x00, sizeof(recv_tmp_buff));
		recv_count = _uart_recv(recv_tmp_buff, MAX_UART_BUF);
		APP_TRACE("_uart_recv ret=%d", recv_count);
		APP_TRACE_BUFF_TIP(recv_tmp_buff, recv_count, "recv from host");
		memset(m_recv_buff, 0x00, sizeof(m_recv_buff));
		memcpy(m_recv_buff, recv_tmp_buff, sizeof(m_recv_buff));
		if (m_recv_buff[0] == STX_CODE_1 && m_recv_buff[1] == STX_CODE_2)	//mf protocol
		{
			APP_TRACE("STX CODE PASS");
			uart_pub_check_sum_update(&check_sum, m_recv_buff + 2, recv_count - 3);
			if (m_recv_buff[recv_count - 2] == ETX_CODE && m_recv_buff[recv_count - 1] == check_sum)	// mpos tlv protocol
			{
				APP_TRACE("CHECK CODE PASS");
				data_len = uart_pub_get_ll_len(&m_recv_buff[2]) + 2;
				APP_TRACE("[uart_pub_get_ll_len][data_len=%d]", data_len);
				if (data_len > RECV_BUFF_SIZE - 5)
					break;
				uart_func_proc(m_recv_buff + 4, recv_count - 6, PROC_MPOS_FORMAT);
			}
			else //json protocol
			{
				APP_TRACE("CHECK JSON CODE PASS");
				data_len = uart_pub_get_len(&m_recv_buff[2]);
				APP_TRACE("[uart_pub_get_len][data_len=%d]", data_len);
				if (data_len > RECV_BUFF_SIZE - 5)
					break;
				uart_func_proc(m_recv_buff + 6, recv_count - 6, PROC_JSON_FORMAT);
			}
		}
		break;
	}

	return;
}
