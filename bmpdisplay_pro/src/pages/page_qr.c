#include "lvgl/lvgl.h"
#include "page_pub.h"
#include "page_qr.h"
#include "pages.h"
#include "lvgl/lvgl/src/lv_lib_qrcode/lv_qrcode.h"
#include "libapi_xpos/inc/mfsdk_power.h"
#include "libapi_xpos/inc/mfsdk_gui.h"

#include "../player_proc.h"

//#define CANVAS_WIDTH  200
//#define CANVAS_HEIGHT  200
#define QR_HEIGHT 240

#define PAY_DEMO_BASE_COLOR 0xF3F4F6

static int m_timeover = 0;
//static page_timer_func m_timer_func = 0;
static lv_obj_t* page_win = NULL;
lv_obj_t* text_label2 = NULL;
static lv_task_t* task_qrcode = NULL;
static int exit_time = 0;
static lv_obj_t* paymentqr = NULL;
static page_close_page_func m_page_close_page_func = 0;


void _close_qrcode_page(int ret)
{
    if (ret == CARD_RET_TIMEOVER) {
        if (page_win != NULL) {
            if (text_label2)
                lv_label_set_text(text_label2,"timeout!");
            exit_time = 5000;
        }
    }
    else {
		if (page_win != 0) {	
			if(task_qrcode){
				lv_task_del(task_qrcode);
	    		task_qrcode = 0;
			}
			if(text_label2){
				lv_obj_del(text_label2);
		        text_label2 = 0;
			}
			if (paymentqr)
			{
				lv_qrcode_delete(paymentqr);
				paymentqr = 0;
			}
			lv_obj_del(page_win);
	        page_win = 0;
            if (m_page_close_page_func != 0) m_page_close_page_func(ret, NULL);
            m_page_close_page_func = 0;
			set_messageflag(0);
			MfSdkPowerUnlockApp();
	    }
	} 
    
}

static void _qrcard_task_func(lv_task_t* task)
{
//    int ret;
    /*ret = mf_rfid_tcl_open();
    APP_TRACE("mf_rfid_tcl_open ret = %d\r\n", ret);
    if (ret >= 0) {
		_close_qrcode_page(CARD_RET_RFIC);
        return;
    }*/
	if(1 == get_messageflag()){
		_close_qrcode_page(CARD_RET_CANCEL);
		return;
	}
    //power_supertime_reset();
    if (m_timeover > 0) {
        APP_TRACE("mf_rfid_tcl_open m_timeover = %d\r\n", m_timeover);
        m_timeover -= 1000;
        if (m_timeover <= 0) {
			_close_qrcode_page(CARD_RET_TIMEOVER);
        }
    }
    if (exit_time > 0) {
        APP_TRACE("mf_rfid_tcl_open exit_time = %d\r\n", exit_time);
        exit_time -= 1000;
        if (exit_time <= 0) {
			_close_qrcode_page(CARD_RET_CANCEL);
        }
    }

}

void close_qrcode_page(int ret)
{
	_close_qrcode_page(ret);
}

static void _page_code_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t key;

    if (e == LV_EVENT_KEY) {
        key = page_get_key(1);
		APP_TRACE("_page_code_event_cb ret == %d\r\n", key);
		if (key == MF_LV_KEY_CANCEL_SHORT_PRESS)
		{
			_close_qrcode_page(CARD_RET_CANCEL);
			MfSdkGuiLedAmount((char*)"0.00");
		}
    }
}

lv_obj_t* page_show_qrcode(lv_obj_t* parent, void *pfunc, char* title, char* tip, char* data, int timeout)
{
	int left = 10;
	int width = lv_obj_get_width(parent)-5*left;

	exit_time = 0;
	lv_obj_t* win = NULL;

	if(parent == NULL || strlen(data) <= 0){
		//APP_TRACE("parent is null!\r\n");
		return NULL;
	}
	
    m_timeover = timeout;
    m_page_close_page_func = (page_close_page_func)pfunc;
	MfSdkPowerLockApp((char*)"qr");
	MfSdkAudPlay((const s8 *)"pscode");
	win = lv_obj_create(parent, NULL);
	page_win = win;
	lv_obj_set_event_cb(win, _page_code_event_cb);
	lv_obj_set_style_local_bg_color(win, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(PAY_DEMO_BASE_COLOR));
	lv_obj_set_style_local_radius(win, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_size(win, lv_obj_get_width(parent), lv_obj_get_height(parent));

    lv_obj_t* text_label1 = lv_label_create(win, NULL);
    lv_label_set_text(text_label1, title);
    lv_obj_set_style_local_text_font(text_label1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_get_font("lv_font_montserrat_24"));
    lv_obj_set_style_local_text_color(text_label1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0));
   // lv_obj_align(text_label1, win, LV_ALIGN_IN_TOP_MID, 0, 20);
	if (LCD_IS_320_480)
	{
		lv_obj_align(text_label1, win, LV_ALIGN_IN_TOP_MID, 0, 60);
    }
	else
	{
		lv_obj_align(text_label1, win, LV_ALIGN_IN_TOP_MID, 0, 40);
	}

    text_label2 = lv_label_create(win, NULL);
    lv_label_set_text(text_label2, tip);
    lv_obj_set_style_local_text_font(text_label2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_get_font("lv_font_montserrat_24"));
    lv_obj_set_style_local_text_color(text_label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0));
    if (LCD_IS_320_480)
	{
    	lv_obj_align(text_label2, win, LV_ALIGN_IN_TOP_MID, 0, 420);
	}
	else
	{
		lv_obj_align(text_label2, win, LV_ALIGN_IN_TOP_MID, 0, 280);
	}

	page_group_set_obj(win);

	/*Create a QR code*/
	paymentqr = lv_qrcode_create(win, width, lv_color_hex3(0x0), lv_color_hex3(0xfff));
	lv_obj_align(paymentqr, win, LV_ALIGN_CENTER, 0, 10);//90

	/*Set data*/
	lv_qrcode_update(paymentqr, data, strlen(data));
	
    if(timeout>0)
	    task_qrcode = lv_task_create(_qrcard_task_func, 1000, LV_TASK_PRIO_MID, 0);
	
	return win;
}
