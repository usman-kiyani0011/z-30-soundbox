#include "lvgl/lvgl.h"
#include <stdio.h>
#include "pages.h"
#include "page_pub.h"
#include "libapi_xpos/inc/mfsdk_gui.h"
#include "libapi_xpos/inc/mfsdk_nfc.h"
#include "../sdk_tts.h"

#define CHECK_TIMER		(200)

static lv_obj_t* emv_page_win = NULL;


static lv_obj_t* page_win = NULL;
static lv_obj_t* lab_title = NULL;
static lv_obj_t* lab_buff = NULL;
static lv_task_t* task_card = NULL;

static lv_obj_t* img = NULL;
static int m_timeover = 0; 

static int exit_time = 0;

static lv_task_t* task_card_disp = NULL;

static page_close_page_func m_page_close_page_func = 0;

void card_close_subpage()
{
    lv_free_png_file(RFPNG);
    lv_free_png_file(RFPNG2);
    lv_free_png_file(RFPNG3);
    if (task_card_disp) 
	{
        lv_task_del(task_card_disp);
        task_card_disp = NULL;
    }
}

void emvreadcardtiprelease(void)
{
	 if (emv_page_win != NULL)
	 {
		lv_obj_del(emv_page_win);		
	 }
	 emv_page_win = NULL;
}

static void _card_close_page(int ret)
{
    if (ret == CARD_RET_TIMEOVER) 
	{
        if (page_win != NULL) 
		{
            if (lab_buff)
                lv_label_set_text(lab_buff,"read card timeout!");
            exit_time = 3000;
        }
    }
    else 
	{
		if (task_card) 
		{
			lv_task_del(task_card);
			task_card = NULL;
		}
        if (page_win != NULL) 
		{
			if (lab_title != NULL) 
			{
	            lv_obj_del(lab_title);
	            lab_title = NULL;
			}
			if (lab_buff != NULL) 
			{
	            lv_obj_del(lab_buff);
	            lab_buff = NULL;
			}
            lv_obj_del(page_win);
            page_win = NULL;

			if (ret == CARD_RET_CANCEL || ret == CARD_RET_TIMEOVER)
			{
				MfSdkNfcClose();
			}
			card_close_subpage();
            if (m_page_close_page_func != 0) m_page_close_page_func(ret, NULL);
            m_page_close_page_func = 0;
        }
    }
}
void free_readcard_page()
{
    _card_close_page(CARD_RET_CANCEL);
}

static void _card_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t key;

    if (e == LV_EVENT_KEY) 
	{
        key = page_get_key();
		APP_TRACE("_card_event_cb ret == %d\r\n", key);
		if (key == MF_LV_KEY_CANCEL_SHORT_PRESS)
		{
	        _card_close_page(CARD_RET_CANCEL);
//            MfSdkGuiLedAmount((char*)"0.00");
			AppFormatSegmentLedAmount(0LL);

		}
    }
}

static void _card_disp_task_func(lv_task_t* task)
{
    static int imgShowChange = 0;

    switch (imgShowChange)
    {
	    case 0:
	        lv_img_set_src(img, RFPNG2);
	        break;
	    case 1:
	        lv_img_set_src(img, RFPNG3);
	        break;
	    case 2:
	        lv_img_set_src(img, RFPNG);
	        break;
	    default:
	        break;
    }

    if (imgShowChange == 2)
        imgShowChange = 0;
    else
        imgShowChange++;
}

void _card_task_func(lv_task_t* task)
{
    int ret;
	int tick1 = MfSdkSysGetTick();

    ret = MfSdkNfcDetect();
	APP_TRACE("mf_rfid_tcl_open[%d] ret = %d\r\n", MfSdkSysGetTick()-tick1, ret);
    if (ret >= 0) 
	{
        _card_close_page(CARD_RET_RFIC);
        return;
    }
	
    if (m_timeover > 0) 
	{
        APP_TRACE("mf_rfid_tcl_open m_timeover = %d\r\n", m_timeover);
        m_timeover -= CHECK_TIMER;
        if (m_timeover <= 0) 
		{
            _card_close_page(CARD_RET_TIMEOVER);
        }
    }
    if (exit_time > 0) 
	{
        APP_TRACE("mf_rfid_tcl_open exit_time = %d\r\n", exit_time);
        exit_time -= CHECK_TIMER;
        if (exit_time <= 0) 
		{
            _card_close_page(CARD_RET_CANCEL);
        }
    }

}

lv_obj_t* page_card_show(lv_obj_t* parent , void * pfunc , char * title, void * trackinfo, int timeover,int show_back)
{
    m_timeover = timeover;
    m_page_close_page_func = (page_close_page_func)pfunc;

    if (page_win != NULL)
    {
        lv_obj_del(page_win);
    }
    page_win = page_create_win(parent, _card_event_cb);
    lv_obj_set_size(page_win, lv_obj_get_width(parent), lv_obj_get_height(parent));
    lab_title = page_create_title(page_win, title);

    lab_buff = page_create_msg(page_win, "Please tap card");
    task_card = lv_task_create(_card_task_func, 100, LV_TASK_PRIO_HIGHEST, 0);

    return page_win;
}

lv_obj_t* page_read_card_show(lv_obj_t* parent, char* title, char* text, int timeout)
{
	if (parent == NULL) 
	{
		return NULL;
	}
	APP_TRACE("page_read_card_show page_create_win 00 emv_page_win=%p \r\n",emv_page_win);

	if (emv_page_win != NULL) 
	{
		APP_TRACE("page_read_card_show page_create_win 01 emv_page_win=%p \r\n",emv_page_win);
		lv_obj_del(emv_page_win);
		emv_page_win = NULL;
	}
	
	APP_TRACE("page_read_card_show page_create_win \r\n");
	emv_page_win = page_create_win(parent, NULL);
	lv_obj_set_size(emv_page_win, lv_obj_get_width(parent), lv_obj_get_height(parent));

	page_create_title(emv_page_win, title);
	page_create_msg(emv_page_win, text);

    return emv_page_win;
}

lv_obj_t* page_card_showamt(lv_obj_t* parent, void* pfunc, char* title, void* trackinfo,char* amtstr, int timeover, int show_back)
{
	char ret = -1;
    char msg[100] = { 0 };
    long long  amt = 0;

    amt = atoll (amtstr);
	APP_TRACE("page_card_showamt \r\n");
    APP_TRACE("[%s] = %s\r\n", __FUNCTION__, amtstr);
    APP_TRACE("[%s] = %lld\r\n", __FUNCTION__, amt);
    PubMultiPlay((const s8 *)"pwcard.mp3");

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
    int ret1, ret2, ret3;
    ret1 = lv_load_png_file(RFPNG);
    ret2 = lv_load_png_file(RFPNG2);
    ret3 = lv_load_png_file(RFPNG3);
    APP_TRACE("ret1=%d , ret2=%d , ret3=%d \r\n", ret1, ret2,ret3);
    if(1 == ret1 && 1 == ret2 && 1 == ret3)
	{
		AppFormatAmountFinal((unsigned char*)msg, amt);
        img = lv_img_create(page_win, NULL);
        lv_img_set_src(img, RFPNG);
		if (!LCD_IS_320_480)
			lv_img_set_zoom(img, 128);
        lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, 40);
        page_ShowTextOut(page_win, "Amount:", LV_ALIGN_IN_TOP_MID, 0, 90, LV_COLOR_BLACK, LV_FONT_24);
        page_ShowTextOut(page_win, msg, LV_ALIGN_IN_TOP_MID, 0, 120, LV_COLOR_RED, LV_FONT_24);
        page_ShowTextOut(page_win, "Please tap card", LV_ALIGN_IN_BOTTOM_MID, 0, -20, LV_COLOR_BLACK, LV_FONT_24);
        task_card_disp = lv_task_create(_card_disp_task_func, 1000, LV_TASK_PRIO_MID, 0);

    }
	else
	{
		char amtDisp[32] = { 0 };
		sprintf(msg, "Please tap card\r\nAmount:\r\n%s",AppFormatAmountFinal((unsigned char*)amtDisp, amt));
	    lab_buff = page_create_msg(page_win, msg);
	}

    task_card = lv_task_create(_card_task_func, CHECK_TIMER, LV_TASK_PRIO_HIGHEST, 0);

    return page_win;
}
