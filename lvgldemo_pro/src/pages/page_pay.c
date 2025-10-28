#include "lvgl/lvgl.h"
#include "page_pub.h"
#include "page_qr.h"
#include "pages.h"
#include "lvgl/lvgl/src/lv_lib_qrcode/lv_qrcode.h"
#include "libapi_xpos/inc/mfsdk_power.h"
#include "libapi_xpos/inc/mfsdk_gui.h"

#include "../player_proc.h"


static int m_timeover = 0;
//static page_timer_func m_timer_func = 0;
static lv_obj_t* page_win = NULL;
static lv_obj_t* text_label2 = NULL;
static lv_task_t* task_qrcode = NULL;
static int exit_time = 0;
static lv_obj_t* paymentqr = NULL;
static page_close_page_func m_page_close_page_func = 0;

static  int m_scanflag = 0;
static unsigned char result[256] = { 0 };
static char *amountstr = 0;

static void _open_scaner()
{
	MfSdkSysSleep(200);
	MfSdkQrScannerSetPreview(0);
	MfSdkQrScannerOpen();
}

static void _close_scaner()
{
	MfSdkQrScannerOpen();
}

void _close_pay_page(int ret)
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
			lv_obj_del(page_win);
	        page_win = 0;

			m_scanflag = 0;
			MfSdkSysTaskAppSet(_close_scaner);
			if (ret == CARD_RET_CANCEL || ret == CARD_RET_TIMEOVER)
			{
				MfSdkNfcClose();
			}

			if (ret == CARD_RET_CONFIRM)
			{
				page_show_image(get_mainpage(), NULL, "SALE", ret, 0, 3000);
				play_voice(amountstr);
			}
			amountstr = 0;
		    m_timeover = 0;
		    exit_time = 0;
            if (m_page_close_page_func != 0) m_page_close_page_func(ret, NULL);
            m_page_close_page_func = 0;
			//AppPowerUnlockApp((char*)"qr");
	    }
	} 
    
}

static void _pay_task_func(lv_task_t* task)
{
    int ret = -1;

    ret = MfSdkNfcDetect();
    APP_TRACE("mf_rfid_tcl_open ret = %d\r\n", ret);
    if (ret >= 0) 
	{
        _close_pay_page(PAGE_RET_CONFIRM);
        return;
    }

    APP_TRACE("m_scanflag = %d\r\n", m_scanflag);
	if (m_scanflag == 0)
	{
		memset(result, 0x0, sizeof(result));
		APP_TRACE("osl_qrdecode_decode\r\n");
		ret = MfSdkQrDecode((s8*)result, sizeof(result));
		APP_TRACE("MfSdkQrDecode ret = %d\r\n", ret);
		if (ret > 0)
		{
			PubMultiPlay((const s8 *)TTS_VOLUME_NOR);
			APP_TRACE("######qrdecode %s\r\n", result);

			m_scanflag = 1;
			_close_pay_page(PAGE_RET_CONFIRM);
		}
	}

    //power_supertime_reset();
    if (m_timeover > 0) 
	{
        APP_TRACE("mf_rfid_tcl_open m_timeover = %d\r\n", m_timeover);
        m_timeover -= 1000;
        if (m_timeover <= 0) 
		{
			_close_pay_page(CARD_RET_TIMEOVER);
        }
    }
    if (exit_time > 0) 
	{
        APP_TRACE("mf_rfid_tcl_open exit_time = %d\r\n", exit_time);
        exit_time -= 1000;
        if (exit_time <= 0) 
		{
			_close_pay_page(CARD_RET_CANCEL);
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
			_close_pay_page(CARD_RET_CANCEL);
			MfSdkGuiLedAmount((char*)"0.00");
		}
    }
}

lv_obj_t* page_show_pay(lv_obj_t* parent, void *pfunc, char* amtStr, char* tip, char* data, int timeout)
{
	int left = 30;
	int width = lv_obj_get_width(parent)-2*left;
	char amountStr[32] = {0};

	m_scanflag = 0;
	exit_time = 0;
	lv_obj_t* win = NULL;

	if(parent == NULL || strlen(data) <= 0)
	{
		APP_TRACE("parent is null!\r\n");
		return NULL;
	}

	amountstr = amtStr;
    m_timeover = timeout;
    m_page_close_page_func = (page_close_page_func)pfunc;
	//AppPowerLockApp((char*)"qr");
	//PubMultiPlay((const s8 *)"pscode.mp3");
	MfSdkSysTaskAppSet(_open_scaner);
	win = lv_obj_create(parent, NULL);
	page_win = win;
	lv_obj_set_event_cb(win, _page_code_event_cb);
	lv_obj_set_style_local_bg_color(win, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xF3F4F6));
	lv_obj_set_style_local_radius(win, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_size(win, lv_obj_get_width(parent), lv_obj_get_height(parent));

	page_create_title(win, "QRCODE");
    sprintf(amountStr, "%0.2f", ATOLL(amtStr) / 100.0);
	if (LCD_IS_320_480)
	{
		page_ShowTextOut(page_win, "Amount:", LV_ALIGN_IN_TOP_MID, 0, 80, LV_COLOR_BLACK, LV_FONT_24);
		page_ShowTextOut(page_win, amountStr, LV_ALIGN_IN_TOP_MID, 0, 110, LV_COLOR_RED, LV_FONT_24);
    }
	else
	{
		page_ShowTextOut(page_win, "Amount:", LV_ALIGN_IN_TOP_MID, 0, 60, LV_COLOR_BLACK, LV_FONT_24);
		page_ShowTextOut(page_win, amountStr, LV_ALIGN_IN_TOP_MID, 0, 80, LV_COLOR_RED, LV_FONT_24);
	}
	text_label2 = page_ShowTextOut(page_win, tip, LV_ALIGN_IN_BOTTOM_MID, 0, -5, LV_COLOR_BLACK, LV_FONT_24);

	page_group_set_obj(win);

	/*Create a QR code*/
	paymentqr = lv_qrcode_create(win, width, lv_color_hex3(0x0), lv_color_hex3(0xfff));
	lv_obj_align(paymentqr, win, LV_ALIGN_CENTER, 0, 40);//90

	/*Set data*/
	lv_qrcode_update(paymentqr, data, strlen(data));
	
    if(timeout>0)
	    task_qrcode = lv_task_create(_pay_task_func, 1000, LV_TASK_PRIO_MID, 0);
	
	return win;
}
