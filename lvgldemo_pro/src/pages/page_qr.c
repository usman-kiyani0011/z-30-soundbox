#include <stdio.h>
#include "lvgl/lvgl.h"
#include "page_pub.h"
#include "page_qr.h"
#include "pages.h"
#include "lvgl/lvgl/src/lv_lib_qrcode/lv_qrcode.h"
#include "libapi_xpos/inc/mfsdk_power.h"
#include "libapi_xpos/inc/mfsdk_gui.h"

#include "../player_proc.h"

#define QR_HEIGHT 240

#define BAR_HEIGHT		100

#define PAY_DEMO_BASE_COLOR 0xF3F4F6

static int g_PowerLock = QRPAGE_LOCK_DEF;
static int m_timeover = 0;
//static page_timer_func m_timer_func = 0;
static lv_obj_t* page_win = NULL;
lv_obj_t* text_label2 = NULL;
static lv_task_t* task_qrcode = NULL;
static int exit_time = 0;
static lv_obj_t* paymentqr = NULL;
static lv_obj_t* canvas = NULL;
static lv_color_t* cbuf = NULL;
static page_close_page_func m_page_close_page_func = 0;

void SetPowerLock (int value)
{
	g_PowerLock = value;
	APP_TRACE("[%s] g_PowerLock = %d\r\n", __FUNCTION__, g_PowerLock);
	return;
}

int GetPowerLock (void)
{
	APP_TRACE("[%s] g_PowerLock = %d\r\n", __FUNCTION__, g_PowerLock);
	return g_PowerLock;
}

void LockStatusCheck()
{
	APP_TRACE("[%s] g_PowerLock = %d\r\n", __FUNCTION__, g_PowerLock);
	if(QRPAGE_LOCK_ON == g_PowerLock)
	{
		AppPowerUnlockApp("qr");
		AppPowerLockApp((char*)"qr");
	}
	else if(QRPAGE_LOCK_DEF == g_PowerLock)
	{
		AppPowerLockApp((char*)"qr");
		SetPowerLock(QRPAGE_LOCK_ON);
	}

}
void close_qrcode_page(int ret)
{
    if (ret == CARD_RET_TIMEOVER) 
	{
        if (page_win != NULL) 
		{
            if (text_label2)
                lv_label_set_text(text_label2,"timeout!");
            exit_time = 5000;
        }
    }
    else 
	{
		if(task_qrcode)
		{
			lv_task_del(task_qrcode);
    		task_qrcode = 0;
		}
		if (page_win != 0) 
		{	
			if(text_label2)
			{
				lv_obj_del(text_label2);
		        text_label2 = 0;
			}
			if (paymentqr)
			{
				lv_qrcode_delete(paymentqr);
				paymentqr = 0;
			}
			if (canvas)
			{
				lv_obj_del(canvas);
				canvas = 0;
			}
			if (cbuf)
			{
				MfSdkMemFree(cbuf);
				cbuf = 0;
			}
			lv_obj_del(page_win);
	        page_win = 0;
			//AppPowerUnlockApp((char*)"qr");
	    }
		lv_free_png_file(QRLOGO);
	} 
    
}

void _close_qrcode_page(int ret)
{
	APP_TRACE("_close_qrcode_page\r\n");
	close_qrcode_page(ret);

	if (CARD_RET_TIMEOVER != ret)
	{
		if(QRPAGE_LOCK_ON == GetPowerLock())
		{
			AppPowerUnlockApp("qr");
		}
		SetPowerLock(QRPAGE_LOCK_DEF);

		if (m_page_close_page_func != 0) m_page_close_page_func(ret, NULL);
		m_page_close_page_func = 0;
	}
}

static void _qrcard_task_func(lv_task_t* task)
{
    //power_supertime_reset();
    if (m_timeover > 0) 
	{
        APP_TRACE("mf_rfid_tcl_open m_timeover = %d\r\n", m_timeover);
        m_timeover -= 1000;
        if (m_timeover <= 0) 
		{
			_close_qrcode_page(CARD_RET_TIMEOVER);
        }
    }
    if (exit_time > 0) 
	{
        APP_TRACE("mf_rfid_tcl_open exit_time = %d\r\n", exit_time);
        exit_time -= 1000;
        if (exit_time <= 0) 
		{
			_close_qrcode_page(CARD_RET_CANCEL);
        }
    }

}

static void _page_code_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t key;

    if (e == LV_EVENT_KEY) 
	{
        key = page_get_key();
		APP_TRACE("_page_code_event_cb ret == %d\r\n", key);
		if (key == MF_LV_KEY_CANCEL_SHORT_PRESS)
		{
			_close_qrcode_page(CARD_RET_CANCEL);
//			MfSdkGuiLedAmount((char*)"0.00");
			AppFormatSegmentLedAmount(0LL);
		}
    }
}

lv_obj_t* page_show_qrcode(lv_obj_t* parent, void *pfunc, char* title, char* tip, char* data, int timeout)
{
	int left = 30;
	int width = lv_obj_get_width(parent)-2*left;

	exit_time = 0;
	lv_obj_t* win = NULL;
	lv_obj_t* img = NULL;

	if(parent == NULL || strlen(data) <= 0)
	{
		APP_TRACE("parent is null!\r\n");
		return NULL;
	}
	close_qrcode_page(0);

    m_timeover = timeout;
    m_page_close_page_func = (page_close_page_func)pfunc;
	
	LockStatusCheck();
	PubMultiPlay((const s8 *)"pscode.mp3");
	win = lv_obj_create(parent, NULL);
	page_win = win;
	lv_obj_set_event_cb(win, _page_code_event_cb);
	lv_obj_set_style_local_bg_color(win, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(PAY_DEMO_BASE_COLOR));
	lv_obj_set_style_local_radius(win, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_size(win, lv_obj_get_width(parent), lv_obj_get_height(parent));

	page_create_title(win, "QRCODE");
	if (LCD_IS_320_480)
	{
		page_ShowTextOut(page_win, "Amount:", LV_ALIGN_IN_TOP_MID, 0, 80, LV_COLOR_BLACK, LV_FONT_24);
		page_ShowTextOut(page_win, title, LV_ALIGN_IN_TOP_MID, 0, 110, LV_COLOR_RED, LV_FONT_24);
    }
	else
	{
		page_ShowTextOut(page_win, "Amount:", LV_ALIGN_IN_TOP_MID, 0, 60, LV_COLOR_BLACK, LV_FONT_24);
		page_ShowTextOut(page_win, title, LV_ALIGN_IN_TOP_MID, 0, 80, LV_COLOR_RED, LV_FONT_24);
	}
	text_label2 = page_ShowTextOut(page_win, tip, LV_ALIGN_IN_BOTTOM_MID, 0, -5, LV_COLOR_BLACK, LV_FONT_24);

	page_group_set_obj(win);

	/*Create a QR code*/
	paymentqr = lv_qrcode_create(win, width, lv_color_hex3(0x0), lv_color_hex3(0xfff));
	lv_obj_align(paymentqr, win, LV_ALIGN_CENTER, 0, 40);//90

	/*Set data*/
	lv_qrcode_update(paymentqr, data, strlen(data));

	/*Create QR logo*/
	if(MFSDK_FS_RET_OK == MfSdkFsCheckPath((const s8 *)"exdata/qrlogo.png"))
	{
		img = lv_img_create(paymentqr, NULL);
		lv_obj_set_size(img, lv_obj_get_width(paymentqr), lv_obj_get_height(paymentqr));
		lv_load_png_file(QRLOGO);
		lv_img_set_src(img, QRLOGO);
		lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, 0);
	}
#if 0
	/*Create a Barcode*/
	cbuf = (char*)MfSdkMemMalloc(sizeof(lv_color_t) * width * BAR_HEIGHT);
	memset(cbuf, 0x00, sizeof(lv_color_t) * width * BAR_HEIGHT);
	canvas = page_barcode(page_win, data, cbuf, 2, BAR_HEIGHT, width, 0, 0);
#endif
    if(timeout>0)
	    task_qrcode = lv_task_create(_qrcard_task_func, 1000, LV_TASK_PRIO_MID, 0);
	
	return win;
}

void show_page_qrcode(lv_obj_t* parent, void *pfunc, char*amount, char*qrcode)
{	
	//QR code display
	if(NULL != parent && strlen(qrcode)>0)
	{
		char title[64] = {0};

		message_close_imagepage(0);
		if (amount != NULL && amount[0] != '\0')
		{
			snprintf(title, sizeof(title), "Amount (PKR): %s", amount);
		}
		page_show_qrcode(parent, pfunc, (title[0] != '\0') ? title : amount, "Please Scan", qrcode, 60000);
	}
	
	return;
}
