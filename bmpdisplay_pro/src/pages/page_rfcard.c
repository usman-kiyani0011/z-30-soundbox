#include "lvgl/lvgl.h"
#include <stdio.h>
#include "pages.h"
#include "page_pub.h"
#include "libapi_xpos/inc/mfsdk_gui.h"
#include "libapi_xpos/inc/mfsdk_nfc.h"

#define IC_CARD_SOCKET	0

static lv_obj_t* page_win = NULL;
static lv_obj_t* lab_title = NULL;
static lv_obj_t* lab_buff = NULL;
static lv_obj_t* btn_canel = NULL;
static lv_obj_t* btn_confirm = NULL;
static lv_task_t* task_card = NULL;
static int m_timeover = 0; 
static int exit_time = 0;

static page_close_page_func m_page_close_page_func = 0;
static lv_obj_t* emv_page_win = NULL;
static lv_obj_t* emv_lab_title = NULL;
static lv_obj_t* emv_lab_buff = NULL;
static lv_task_t* emv_task_card = NULL;

void emvreadcardtiprelease(void)
{
	 if (emv_page_win != NULL)
	 {
		lv_obj_del(emv_page_win);		
	 }
	 emv_page_win = NULL;
}

lv_obj_t* page_read_card_show(lv_obj_t* parent, char* title, char* text, int timeout)
{
	if (parent == NULL) {
		return NULL;
	}

	if (emv_page_win != NULL) {
		lv_obj_del(emv_page_win);
		emv_page_win = NULL;
	}
	
	APP_TRACE("page_read_card_show page_create_win \r\n");
	emv_page_win = page_create_win(parent, NULL);
	lv_obj_set_size(emv_page_win, lv_obj_get_width(parent), lv_obj_get_height(parent));

	emv_lab_title = page_create_title(emv_page_win, title);
	emv_lab_buff = page_create_msg(emv_page_win, text);

    return emv_page_win;
}

static void _card_close_page(int ret)
{
    if (ret == CARD_RET_TIMEOVER) {
        if (page_win != NULL) {
            if (lab_buff)
                lv_label_set_text(lab_buff,"read card timeout!");
            exit_time = 5000;
        }
    }
    else {
        if (page_win != NULL) {
			if (lab_title != NULL) {
	            lv_obj_del(lab_title);
	            lab_title = NULL;
			}
			if (lab_buff != NULL) {
	            lv_obj_del(lab_buff);
	            lab_buff = NULL;
			}
            lv_obj_del(page_win);
            page_win = NULL;
            if (task_card) {
                lv_task_del(task_card);
                task_card = NULL;
            }
			lv_free_png_file(RFPNG);
            if (m_page_close_page_func != 0) m_page_close_page_func(ret, NULL);
            m_page_close_page_func = 0;
        }
    }
}
void free_readcard_page()
{
    _card_close_page(CARD_RET_CANCEL);
}
static void _card_btn_event_cb(struct _lv_obj_t* obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
       if (obj == btn_canel) {
           _card_close_page(CARD_RET_CANCEL);
       }
       else if (obj == btn_confirm) {
           //_card_close_page(PAGE_RET_CONFIRM);
       }

    }
}

static void _card_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t key;

    if (e == LV_EVENT_KEY) {
        key = page_get_key(1);
		APP_TRACE("_card_event_cb ret == %d\r\n", key);
		if (key == MF_LV_KEY_CANCEL_SHORT_PRESS){
	        _card_close_page(CARD_RET_CANCEL);
            MfSdkGuiLedAmount((char*)"0.00");
		}
#if 0
        if (key == MF_KEY_ESC){
            //if (btn_canel != 0) {
                _card_close_page(CARD_RET_CANCEL);
            //}
        }
        else if (key == MF_KEY_ENTER) {
           // if (btn_confirm != 0) {
                //_card_close_page(CARD_RET_CONFIRM);
           // }
        }
#ifdef WIN32
        else if (key == MF_KEY_1) {
			strcpy(m_trackinfo->b_chars.chars, "6223676590001877823=49125209092761000");
			strcpy(m_trackinfo->c_chars.chars, "996223676590001877823=1561560300030000196013000000210000049121=000000000000=000000000000=00000000");
            _card_close_page(CARD_RET_MAGTEK);
        }
		else if (key == MF_KEY_2) {
            _card_close_page(CARD_RET_ICC);
		}
		else if (key == MF_KEY_3) {
            _card_close_page(CARD_RET_RFIC);
		}
#endif 
#endif
    }
}


void _card_task_func(lv_task_t* task)
{
    //struct magtek_track_info track_info = {0};
    int ret;
#if 0
    ret = mf_magtek_read(m_trackinfo);
    APP_TRACE("mf_magtek_read ret = %d\r\n", ret);

    if (ret != 0) {
        _card_close_page(CARD_RET_MAGTEK);
        return;
    }
#endif
    ret = MfSdkNfcDetect();
    APP_TRACE("mf_rfid_tcl_open ret = %d\r\n", ret);
    if (ret >= 0) {
        _card_close_page(CARD_RET_RFIC);
        return;
    }
#if 0
	icc_open(IC_CARD_SOCKET);
	ret = icc_present(IC_CARD_SOCKET);
	icc_close(IC_CARD_SOCKET);
    APP_TRACE("icc_present ret = %d\r\n", ret);
	if (ret  == 1) {
		_card_close_page(CARD_RET_ICC);
		return;
	}
#endif
    //power_supertime_reset();
    if (m_timeover > 0) {
        APP_TRACE("mf_rfid_tcl_open m_timeover = %d\r\n", m_timeover);
        m_timeover -= 1000;
        if (m_timeover <= 0) {
            _card_close_page(CARD_RET_TIMEOVER);
        }
    }
    if (exit_time > 0) {
        APP_TRACE("mf_rfid_tcl_open exit_time = %d\r\n", exit_time);
        exit_time -= 1000;
        if (exit_time <= 0) {
            _card_close_page(CARD_RET_CANCEL);
        }
    }

}
static void back_navigation_event_cb(lv_obj_t* btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		if (!btn) return;
		_card_close_page(CARD_RET_CANCEL);
	}
}

lv_obj_t* page_card_show(lv_obj_t* parent , void * pfunc , char * title, void * trackinfo, int timeover,int show_back)
{
    //m_trackinfo = (struct magtek_track_info *)trackinfo;
    m_timeover = timeover;
    m_page_close_page_func = (page_close_page_func)pfunc;

    if (page_win != NULL)
    {
        lv_obj_del(page_win);
    }
    page_win = page_create_win(parent, _card_event_cb);
    lv_obj_set_size(page_win, lv_obj_get_width(parent), lv_obj_get_height(parent));
    lab_title = page_create_title(page_win, title);

#if 0
	if(show_back)
		page_create_navigation_back(page_win,back_navigation_event_cb);
	
	btn_canel = page_create_btn(page_win, "cancel", _card_btn_event_cb, PAGE_BTN_ALIGN_LEFT);
#endif
    lab_buff = page_create_msg(page_win, "Please tap card");
    task_card = lv_task_create(_card_task_func, 100, LV_TASK_PRIO_HIGHEST, 0);

    return page_win;
}
lv_obj_t* page_card_showamt(lv_obj_t* parent, void* pfunc, char* title, void* trackinfo,char* amtstr, int timeover, int show_back)
{
	char ret = -1;
    char msg[100] = { 0 };
    long long  amt = 0;

    amt = atoll (amtstr);
    APP_TRACE("[%s] = %s\r\n", __FUNCTION__, amtstr);
    APP_TRACE("[%s] = %lld\r\n", __FUNCTION__, amt);
    MfSdkAudPlay((const s8 *)"pwcard");

    m_timeover = timeover;
    exit_time = 0;
    m_page_close_page_func = (page_close_page_func)pfunc;

    if (page_win != NULL)
    {
        lv_obj_del(page_win);
    }
    page_win = page_create_win(parent, _card_event_cb);
    lv_obj_set_size(page_win, lv_obj_get_width(parent), lv_obj_get_height(parent));
    lab_title = page_create_title(page_win, title);

    ret = lv_load_png_file(RFPNG);
	if(1 == ret)
	{
		sprintf(msg, "Amount:%0.2f", amt/100.00);
	    lab_buff = page_create_msg(page_win, msg);
        lv_obj_t* img = lv_img_create(page_win, NULL);
        lv_img_set_src(img, RFPNG);
        lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, 40);
	}
	else
	{
	    sprintf(msg, "Please tap card\r\nAmount:\r\n%0.2f", amt/100.00);
	    lab_buff = page_create_msg(page_win, msg);
	}

    task_card = lv_task_create(_card_task_func, 1000, LV_TASK_PRIO_HIGHEST, 0);

    return page_win;
}
