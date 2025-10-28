#include "lvgl/lvgl.h"
#include "pages.h"
#include "tracedef.h"

#include "libapi_xpos/inc/mfsdk_gui.h"
#include "libapi_xpos/inc/mfsdk_power.h"

#include "../pub/http.h"


static lv_task_t* task_card = NULL;
static lv_obj_t* scan_body = NULL;
static lv_obj_t* lab_text = NULL;
static lv_obj_t* lab_code = NULL;
//static lv_obj_t* lbt_scan = NULL;
//static lv_obj_t* lbt_cancel = NULL;
static  int m_nscanflag = 0;
static unsigned char result[256] = { 0 };
static int ticks = 0;

static int scan_get_key()
{
	uint32_t key;
	const void* d;
	d = lv_event_get_data();
	memcpy(&key, d, sizeof(uint32_t));
	return key;
}

static lv_obj_t * btn_create(lv_obj_t* parent, const char* text)
{
	lv_obj_t* label;
	lv_obj_t* btn1 = lv_btn_create(parent, NULL);
	label = lv_label_create(btn1, NULL);
	//lv_obj_set_style_local_text_font(btn1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_16);
	lv_label_set_text(label, text);
	lv_obj_set_size(btn1, 120, 30);	
	
	return btn1;
}

static void _scan_close_page()
{
	APP_TRACE("_scan_close_page\r\n");
//	osl_scaner_close();
	MfSdkQrScannerClose();

	m_nscanflag = 0;
	if (scan_body != 0) {
		if(task_card){
			lv_task_del(task_card);
    		task_card = 0;
		}
		if(lab_text){
			lv_obj_del(lab_text);
    		lab_text = 0;
		}
		if(lab_code){
			lv_obj_del(lab_code);
    		lab_code = 0;
		}
		lv_obj_del(scan_body);
		scan_body = 0;
	}
	//power_unlock_app();
}

static void scan_event_cb(lv_obj_t* obj, lv_event_t event)
{
	uint32_t key;

	if (event == LV_EVENT_KEY)
	{
		MfSdkLcdBackLight(MFSDK_LCD_ON);
		key = scan_get_key();
		APP_TRACE("scan_event_cb key=%d\r\n",key);
		if (key == MF_LV_KEY_CANCEL_SHORT_PRESS)
		{
			MfSdkAudPlay((const s8 *)"volnor.mp3");
			_scan_close_page();
			MfSdkGuiLedAmount("0.00");
			
		}
		else if (key == MF_LV_KEY_QUIT_LONG_PRESS)
		{
//			Sys_power_off();
			MfSdkPowerOff();
		}
	}
}

static void _scan_task_func(lv_task_t* task)
{
	int  ret = 0;

	if (m_nscanflag == 0)
	{
		memset(result, 0x0, sizeof(result));
		APP_TRACE("osl_qrdecode_decode\r\n");
//		ret = osl_qrdecode_decode(result, sizeof(result));
		ret = MfSdkQrDecode((s8*)result, sizeof(result));
		APP_TRACE("MfSdkQrDecode ret = %d\r\n", ret);
		if (ret > 0)
		{
			MfSdkAudPlay((const s8 *)"volnor.mp3");
			//lv_label_set_text(lab_text, "success");
			//lv_label_set_text(lab_code, result);
			APP_TRACE("######qrdecode %s\r\n", result);

			m_nscanflag = 1;
			_scan_close_page();
			SendTransResult();
		}
		else if(MfSdkSysTimerCheck(ticks) == 0) //timeout
		{
			_scan_close_page();
		}
	}
	
}

int ScanPage(lv_obj_t* parent, char*amount)
{
//	char title[32] = {0};
	if (scan_body != NULL)
	{
		lv_obj_del(scan_body);
	}
	//power_lock_app("scan");
	//lv_obj_t* parent = get_mainpage();
	scan_body = lv_obj_create(parent, NULL);

	lv_obj_set_size(scan_body, lv_obj_get_width(parent), lv_obj_get_height(parent));
	lv_obj_align(scan_body, parent, LV_ALIGN_IN_TOP_LEFT, 0, 0);
#if 0
	lab_text = lv_label_create(scan_body, NULL);
	lv_label_set_long_mode(lab_text, LV_LABEL_LONG_BREAK);
	lv_label_set_recolor(lab_text, true);
	sprintf(title, "Amount (PKR): %s", amount);
	lv_label_set_text(lab_text, title);
	lv_label_set_align(lab_text, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_width(lab_text, lv_obj_get_width(parent) - 10);

	lab_code = lv_label_create(scan_body, NULL);
	lv_obj_align(lab_code, parent, LV_ALIGN_IN_TOP_LEFT, 0, 50);
	lv_label_set_long_mode(lab_code, LV_LABEL_LONG_BREAK);
	lv_label_set_recolor(lab_code, true);
	lv_label_set_text(lab_code, "Please Scan");
	lv_label_set_align(lab_code, LV_LABEL_ALIGN_LEFT);
	lv_obj_set_width(lab_code, lv_obj_get_width(parent) - 70);
#endif
	//lbt_cancel = btn_create(scanTest_body, "CANCEL");
	//lv_obj_align(lbt_cancel, parent, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
	
	MfSdkAudPlay((const s8 *)"pshowqr");
	while (MfSdkAudTtsState() == 1)
	{
		MfSdkSysSleep(100);
	}

	MfSdkQrScannerOpen();
	
	//osl_scaner_open();
	ticks = MfSdkSysTimerOpen(120 * 1000);
	APP_TRACE("osl_scaner_open\r\n");
	task_card = lv_task_create(_scan_task_func, 100, LV_TASK_PRIO_MID, 0);
	//lvgl_group_set_obj(scan_body);
	page_group_set_obj(scan_body);
	lv_obj_set_event_cb(scan_body, scan_event_cb);
	//lv_obj_set_style_local_text_font(lab, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_24);
	MFSDK_UNUSED(HttpIdleflag);
	return 0;
}
