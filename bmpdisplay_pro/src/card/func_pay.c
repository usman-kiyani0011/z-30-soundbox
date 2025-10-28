#include <stdio.h>
#include "lvgl/lvgl.h"
#include "pages/pages.h"
#include "func_pay.h"
#include "libapi_xpos/inc/emvapi.h"
#include "libapi_xpos/inc/mfsdk_mem.h"
#include "libapi_xpos/inc/mfsdk_aud.h"
#include "libapi_xpos/inc/mfsdk_gui.h"
#include "libapi_xpos/inc/mfsdk_power.h"
#include "libapi_xpos/inc/mfsdk_emv.h"

#include "../sdk_tts.h"
#include "../pub/http.h"
#include "../page_main.h"
#include "../player_proc.h"

#define TIMEOVER	60000
#define TIMEOVER_AUTO	500

enum 
{
	PAY_STEP_START,
	PAY_STEP_INPUT_AMOUNT,
	PAY_STEP_INPUT_CARD,
	PAY_STEP_READ_CARD,
	PAY_STEP_INPUT_PIN,
	PAY_STEP_EMV,
	PAY_STEP_EMV_RESULT_MSG,
	PAY_STEP_COMM,
	PAY_STEP_MSG,
};

typedef struct __st_pay_data
{
	int step;
	char amount[32];
	char pin[12];
	char cardnum[32];
	char track2[128];
	char track3[128];
	st_comm_data comm_data;
	char retmsg[256];
} st_pay_data;

typedef struct __st_ip_port
{
	int step;
	char ip[64];
	char port[5];
} st_ip_port;

static lv_obj_t* base_win;
static lv_obj_t* amount_win;
static lv_obj_t* card_win;
static lv_obj_t* pin_win;
static lv_obj_t* comm_win;

static int pay_step = 0;
static st_pay_data  * pay_data_pay = NULL;
static st_read_card_in card_in = {0};
static st_read_card_out card_out = { 0 };
static st_card_info card_info = { 0 };
static char amount[16] = { 0 };

static void _pay_close_page()
{
	if (base_win != 0) {
		lv_obj_del(base_win);
		base_win = 0;
		MfSdkMemFree(pay_data_pay);

		MfSdkPowerUnlockApp();
	}
	
}

void free_rfid_page()
{
	while ((get_page_comm_busyflag()==1))
	{
		MfSdkSysSleep(100);
	}
	//删除读卡界面
	free_readcard_page();
	free_message_page();

	_pay_close_page();
}

static void _pack_data()
{
	char* sbuff;

	memset(&pay_data_pay->comm_data, 0 ,sizeof(st_comm_data));
	sbuff = pay_data_pay->comm_data.sbuff;
	
	sprintf(sbuff + strlen(sbuff), "%s pay %s\r\n", "-----------", "-----------");
	sprintf(sbuff + strlen(sbuff), "amount=%s\r\n", pay_data_pay->amount);
	sprintf(sbuff + strlen(sbuff), "track2=%s\r\n", pay_data_pay->track2);
	sprintf(sbuff + strlen(sbuff), "track3=%s\r\n", pay_data_pay->track3);
	sprintf(sbuff + strlen(sbuff), "pin=%s\r\n", pay_data_pay->pin);
	pay_data_pay->comm_data.slen = strlen(sbuff);
	APP_TRACE("[%s][%s]\r\n",__FUNCTION__, sbuff);
}

static void _unpack_data()
{
	APP_TRACE_BUFF_TIP(pay_data_pay->comm_data.rbuff, pay_data_pay->comm_data.rlen, "recv:");
	if (pay_data_pay->comm_data.rlen > 0&& pay_data_pay->comm_data.rlen < 256) {
		strcpy(pay_data_pay->retmsg, pay_data_pay->comm_data.rbuff);
	}
	else {
		APP_TRACE("_unpack_data error length!");
	}
}

void rf_play_amt(long long money)
{
	MfSdkAudPlay((const s8 *)"pay.mp3");
	if (money >= 100)
	{
		sdk_tts_play_amt_india(money / 100);
		if (money % 100 != 0)
		{
			MfSdkAudPlay((const s8 *)"and.mp3");
			sdk_tts_play_amt_india_Paise(money % 100);
		}
	}
	else if (money % 100 != 0)
	{
		sdk_tts_play_amt_india_Paise(money % 100);
	}
}

static int _pay_return_func(int ret);

static int emv_read_card_callback(int ret, lv_obj_t* obj)
{
	APP_TRACE("emv_read_card_callback ret:%d\r\n", ret);	
	_pay_return_func(ret);

	return 0;
}

void *_read_card_page(void)
{
	APP_TRACE("_read_card_page \r\n");	
	page_read_card_show(get_mainpage(), "pay", "Icc proccess", 1000);
}


static int _pay_return_func(int ret)
{

	APP_TRACE("_pay_return_func(%d):%d\r\n", pay_data_pay->step, ret);

	if (pay_data_pay->step == PAY_STEP_START) {		// ??
		
		APP_TRACE("PAY_STEP_START1\r\n");
		pay_data_pay->step = PAY_STEP_INPUT_CARD;
		_pay_return_func(0);
		APP_TRACE("PAY_STEP_START2\r\n");
	}
	else if (pay_data_pay->step == PAY_STEP_INPUT_CARD) {		// 读卡
		if (ret == PAGE_RET_CANCEL || ret == PAGE_RET_TIMEOVR) {
			_pay_close_page();			
		}
		else {
			page_card_showamt(base_win, _pay_return_func, "pay", 0 , pay_data_pay->amount, TIMEOVER, 0);
			pay_data_pay->step = PAY_STEP_READ_CARD;
		}
	}
	else if (pay_data_pay->step == PAY_STEP_READ_CARD) {	// 读卡返回
		APP_TRACE("ret = %d\r\n", ret);
		if (ret == CARD_RET_CANCEL || ret == CARD_RET_RFS) {			
			_pay_close_page();
		}
		else {
			if (ret == CARD_RET_RFIC) {
				MfSdkAudPlay((const s8 *)"volnor.mp3");
				MfSdkEmvSetReadPageCallback(_read_card_page);
				APP_TRACE("CARD_RET_RFIC\r\n");
				strcpy(card_in.amt, pay_data_pay->amount);
				APP_TRACE("CARD_RET_RFIC AMOUNT:%s\r\n", card_in.amt);
				card_in.card_mode = EMVCARD_RET_RFID;
				MfSdkEmvReadCardPage(base_win, emv_read_card_callback, &card_in, &card_out);
				pay_data_pay->step = PAY_STEP_EMV;
			}			
		}
	}
	else if (pay_data_pay->step == PAY_STEP_EMV) {	// EMV返回
		emvreadcardtiprelease();
		APP_TRACE("PAY_STEP_EMV ret = %d\r\n", ret);
		if (ret == PAGE_RET_CANCEL) {
			_pay_close_page();
		}
		else {

			if (ret == EMVAPI_RET_ARQC) {
				strcpy(pay_data_pay->retmsg, "arqc online");
			}
			else if (ret == EMVAPI_RET_TC) {
				strcpy(pay_data_pay->retmsg, "approve");
			}
			else if (ret == EMVAPI_RET_AAC) {
				strcpy(pay_data_pay->retmsg, "decline");
			}
			else if (ret == EMVAPI_RET_AAR) {
				strcpy(pay_data_pay->retmsg, "terminate");
			}
			else {
				strcpy(pay_data_pay->retmsg, "terminate");
			}

			if (ret == EMVAPI_RET_ARQC) {
				page_message_show_ex(base_win, _pay_return_func, "pay", pay_data_pay->retmsg, "", "confirm", 3000, 1);
				pay_data_pay->step = PAY_STEP_EMV_RESULT_MSG;
			}
			else if (ret == EMVAPI_RET_TC) {
				pay_data_pay->step = PAY_STEP_COMM;
				_pay_return_func(0);
			}
			else {
				APP_TRACE("PAY_STEP_EMV2\r\n");
				page_message_show_ex(base_win, _pay_return_func, "pay", pay_data_pay->retmsg, "", "confirm", 5000, 1);
				pay_data_pay->step = PAY_STEP_MSG;
			}
		}
	}
	else if (pay_data_pay->step == PAY_STEP_EMV_RESULT_MSG) {
		if (ret == PAGE_RET_CANCEL) {
			_pay_close_page();
		}
		else {
			pay_data_pay->comm_data.timeover = 10000;
			comm_win = page_comm_show_auto(base_win, _pay_return_func, "pay", &pay_data_pay->comm_data, 0);
			pay_data_pay->step = PAY_STEP_COMM;
		}
	}
	else if (pay_data_pay->step == PAY_STEP_COMM) {			// 通讯返回	
		APP_TRACE("PAY_STEP_COMM ret = %d\r\n", ret);
		page_message_show_ex(base_win, _pay_return_func, "pay", "request", "", "confirm", 1, 1);
		pay_data_pay->step = PAY_STEP_MSG;
	}
	else if (pay_data_pay->step == PAY_STEP_MSG) {			// 提示返回
		APP_TRACE("PAY_STEP_MSG ret = %d\r\n", ret);
		_pay_close_page();
		if(0 == strcmp(pay_data_pay->retmsg, "arqc online") || 0 == strcmp(pay_data_pay->retmsg, "approve"))
			SendTransResult();
	}
	
	return 0;

}


void func_pay(lv_obj_t * parent, char* amt)
{
	MFSDK_UNUSED(HttpIdleflag);
	MFSDK_UNUSED(base_win);
	MFSDK_UNUSED(amount_win);
	MFSDK_UNUSED(card_win);
	MFSDK_UNUSED(pin_win);
	MFSDK_UNUSED(comm_win);
	MFSDK_UNUSED(pay_step);
	MFSDK_UNUSED(&card_info);

	//set_rf_leisure_flag(0);
	if(NULL != base_win)
		_pay_close_page();
	base_win = page_create_base(parent);
	pay_data_pay = MfSdkMemMalloc(sizeof(st_pay_data));
	memset(pay_data_pay, 0x00, sizeof(st_pay_data));
	pay_data_pay->step = PAY_STEP_START;
	if (NULL != amt) {
		char amountFormat[16] = {0};
		sprintf(amountFormat , "%0.2f", ATOLL(amt)/100.0);
		MfSdkGuiLedAmount(amountFormat);
		strcpy(pay_data_pay->amount, amt);

		memset(amount, 0, sizeof(amount));
		strcpy(amount, amt);
	}

	MfSdkPowerLockApp((char*)"rf");
	_pay_return_func(0);
}

char* get_amount()
{
	return amount;
}
static int _input_return_func(int ret,lv_obj_t* obj)
{
	long long llamont = 0;
	if (ret == PAGE_RET_CANCEL || ret == PAGE_RET_TIMEOVR) {
//		xgui_aumont_led("0.00");
		MfSdkGuiLedAmount("0.00");
		return ret;			
	}
	APP_TRACE("_input_return_func = %d\r\n",ret);
	APP_TRACE("amount = %s\r\n",amount);
	llamont = ATOLL(amount);
	APP_TRACE("llamont = %lld\r\n",llamont);
	if (llamont > 0)
	{
		if (ret == PAGE_RET_CONFIRM) {//qrcode
			TransactionProc(get_mainpage(), llamont, 1);
		}
		else if (ret == PAGE_RET_F1) {//scan
			TransactionProc(get_mainpage(), llamont, 4);
		}
		else if (ret == PAGE_RET_F2) {//rf
			TransactionProc(get_mainpage(), llamont, 2);
			//func_pay(get_mainpage(), amount);
		}
	}
	return ret;			
}

void func_pay2(lv_obj_t * parent, char* amt)
{
	char amountFormat[16] = {0};
	memset(amount, 0, sizeof(amount));
	if (NULL != amt) {
		strcat(amount, amt);
		sprintf(amountFormat , "%0.2f", ATOLL(amt)/100.0);
		MfSdkGuiLedAmount(amountFormat);
	}
	page_input_show_ex(parent, _input_return_func, "amount", amount, 0, 7, PAGE_INPUT_MODE_AMOUNT, 60*1000,0);
	
}


