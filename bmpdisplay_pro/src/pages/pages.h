#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lvgl/lvgl.h"
#include "lvgl/src/lv_lib_png/lv_png.h"
#include "lvglwrap/inc/page_image.h"
#include "../tracedef.h"
#include "page_input.h"
#include "page_style.h"
#include "libapi_xpos/inc/mfsdk_aud.h"
#include "libapi_xpos/inc/mfsdk_qr.h"
#include "libapi_xpos/inc/mfsdk_lcd.h"
#include "libapi_xpos/inc/mfsdk_sys.h"
#include "libapi_xpos/inc/mfsdk_mem.h"
#include "libapi_xpos/inc/mfsdk_qr.h"
#include "libapi_xpos/inc/mfsdk_comm.h"
#include "libapi_xpos/inc/mfsdk_power.h"
#include "libapi_xpos/inc/mfsdk_gui.h"
#include "libapi_xpos/inc/mfsdk_fs.h"
#include "libapi_xpos/inc/mfsdk_kb.h"
#include "libapi_xpos/inc/mfsdk_nfc.h"
#include "libapi_xpos/inc/mfsdk_util.h"

#include "page_pub.h"
#include "page_rfcard.h"
#include "page_normal_list.h"
#include "page_qr.h"

//#include "page_list.h"
//#include "comm_pages/comm_pages.h"
#define PAY_DEMO_BASE_COLOR 0xF3F4F6

#ifdef WIN32
#define ATOLL	_atoi64
#else
#define ATOLL	atoll
#endif

enum {
	MF_KEY_QUIT,
	MF_KEY_OK,
	MF_KEY_UP,
	MF_KEY_DOWN,
	MF_KEY_LEFT,
	MF_KEY_RIGHT,
	MF_KEY_TOTAL,
	MF_KEY_BACKSPACE,
	MF_KEY_0=48,
	MF_KEY_1,
	MF_KEY_2,
	MF_KEY_3,
	MF_KEY_4,
	MF_KEY_5,
	MF_KEY_6,
	MF_KEY_7,
	MF_KEY_8,
	MF_KEY_9,
	MF_KEY_XING,
	MF_KEY_JING,
	MF_KEY_F1,
	MF_KEY_F2,
	MF_KEY_CANCEL,
	MF_KEY_ADD,
	MF_KEY_FUNC,
};

enum {
	PAGE_BTN_ALIGN_LEFT,
	PAGE_BTN_ALIGN_RIGHT,
};

enum {
	PAGE_INPUT_MODE_NUM,
	PAGE_INPUT_MODE_ALPHABET,
	PAGE_INPUT_MODE_IP,
	PAGE_INPUT_MODE_PWD,
	PAGE_INPUT_MODE_AMOUNT,
};

enum {
	PAGE_RET_TIMEOVR = -2,
	PAGE_RET_CANCEL = -1,
	PAGE_RET_CONFIRM = 0,
	PAGE_RET_F1 = 1,
	PAGE_RET_F2 = 2,
};

enum {
	COMM_RET_FAIL_LINK = -2,
	COMM_RET_FAIL_CONECT = -3,
	COMM_RET_FAIL_RECV = -4,
	COMM_RET_FAIL_PACK = -5,
	COMM_RET_FAIL_SEND = -6,
	COMM_RET_CANCEL = -1,
	COMM_RET_SUCC = 0,
};

enum {
	ENCRYPT_PIN_ERR1 = -18,
	ENCRYPT_PIN_ERR2 = -19,
	ENCRYPT_MAC_ERR1 = -20,
	ENCRYPT_DATA_ERR1 = -31,
	ENCRYPT_DATA_ERR2 = -32,
	ENCRYPT_DATA_ERR3 = -33,
};

enum {
	CARD_RET_TIMEOVER = -2,
	CARD_RET_CANCEL = -1,
	CARD_RET_CONFIRM,
	CARD_RET_MAGTEK,
	CARD_RET_ICC,
	CARD_RET_RFIC,
	CARD_RET_RFS,
};

enum {
	WIFI_EXIT = -3,
	WIFI_BACK = -2,
	WIFI_CONFIRM = 0,
};
#define PAY_DEMO_BASE_CLOR 0xF3F4F6

#define IMG_SETTING_MORE	    "P:exdata/rarrow.png" // > 更多箭头图标
#define IMG_SETTING_BACK		"P:exdata/larrow.png"

#define IMG_SETTING_ADD	             "P:exdata/add.png"
#define IMG_SETTING_REDUCE		    "P:exdata/reduce.png"

#define APP_STATE_HEIGHT 32
#define APP_NAV_HEIGHT   66


#define LIST_INTERVAL    20
#define CELL_HEIGHT      76
#define CELL_WIDTH       440

#define SEND_BUFF_SIZE	1024
#define RECV_BUFF_SIZE	1024

#define OK_TIME_OVER	30000
#define FAIL_TIME_OVER	0
#define SHOW_TIME_OVER	15000
#define PSW_TIMEOUT		15000
#define OP_TIME_OVER	15000

typedef struct __st_comm_data{
	char sbuff[SEND_BUFF_SIZE];
	int slen;
	char rbuff[RECV_BUFF_SIZE];
	int rlen;
	int timeover;
}st_comm_data;

#define IMG_NAVIGATION_BACK "P:exdata/navback.png"

typedef int (*page_close_page_func)(int ret,lv_obj_t* obj);
typedef void (*tts_play_func)( const char *payload,int len);
typedef int (*page_timer_func)(lv_obj_t* obj_win, lv_obj_t* obj_msg);


lv_obj_t* page_card_show(lv_obj_t* parent, void* pfunc, char* title, void* trackinfo, int timeover,int show_back);
lv_obj_t* page_card_showamt(lv_obj_t* parent, void* pfunc, char* title, void* trackinfo, char* amtstr, int timeover, int show_back);
void page_init();
lv_obj_t* page_create_btn(lv_obj_t* parent, char* text, lv_event_cb_t btn_event_cb, int align);
lv_obj_t* page_create_title(lv_obj_t* parent, char* title);
lv_obj_t* page_create_base(lv_obj_t* parent);
lv_obj_t* page_create_msg(lv_obj_t* parent, char* msg);
lv_obj_t* page_create_navigation_back(lv_obj_t* parent, lv_event_cb_t _event_cb);
int page_get_btn_height();
int page_get_title_height();
int page_get_title_btn_height();
lv_obj_t* page_create_win(lv_obj_t* parent, lv_event_cb_t _event_cb);
void set_tts_play_func(tts_play_func pfunc, unsigned char *payload, unsigned int payloadlen);
int GetKeyStatus(int key);
int GetKeyValue(int key);

#include "page_comm.h"
#include "page_message.h"
#include "page_pw_input.h"

