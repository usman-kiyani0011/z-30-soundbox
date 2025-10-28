#include"network_page.h"
//#include "driver/mf_supply_power.h"
//#include "driver/mf_misc.h"
//#include "driver/mf_rtc.h"
//#include "pub/osl/inc/osl_time.h"
#include "page_main.h"
#include "lvgl/lvgl.h"
#include "lvgl/src/lv_lib_png/lv_png.h"
//#include "pub/osl/inc/osl_BaseParam.h"
#include "./tracedef.h"
//#include "libapi_xpos/inc/libapi_comm.h"
#include "libapi_xpos/inc/mfsdk_aud.h"
#include "libapi_xpos/inc/mfsdk_sys.h"
#include "libapi_xpos/inc/mfsdk_comm.h"
#include "libapi_xpos/inc/mfsdk_power.h"
#include "page_pub.h"
#define IMG_MODE_NORMA "P:exdata/mnormal.png"
#define IMG_MODE_SELECT  "P:exdata/selected.png"

//network interface pointer
static lv_obj_t* networkwin;
static network_param_t network_param = { 0 };
static lv_obj_t* network_page;
static lv_obj_t* gprs_obj;
static lv_obj_t* wifi_obj;
static lv_task_t* ptask = NULL;

int start_time=0;

void tts_sleep()
{
	while (MfSdkAudTtsState() == 1)//Broadcasting
	{
		MfSdkSysSleep(500);
	}
	MfSdkSysSleep(1000);
}

//create radio button
lv_obj_t* create_cb_btn(lv_obj_t* parent, const char* labtitle, const char* filename,bool isSplit)
{
	lv_obj_t* check_obj = lv_obj_create(parent, NULL);
	lv_obj_clean_style_list(check_obj, LV_OBJ_PART_MAIN);
	lv_obj_set_width(check_obj, lv_page_get_width_fit(parent));
	lv_obj_set_height(check_obj,50);
	

	lv_obj_t* gprs_lab = lv_label_create(check_obj, NULL);
	lv_label_set_text(gprs_lab, labtitle);
	lv_obj_align(gprs_lab, check_obj, LV_ALIGN_IN_LEFT_MID, 8, 1);

	lv_obj_t* check_btn = lv_imgbtn_create(check_obj, NULL);
	lv_imgbtn_set_src(check_btn, LV_BTN_STATE_RELEASED, filename);

	lv_obj_align(check_btn, check_obj, LV_ALIGN_IN_RIGHT_MID, -4, 0);

	if (isSplit)
	{
		lv_obj_t* sper_label = lv_label_create(check_obj, NULL);
		
		lv_label_set_recolor(sper_label, true);
		lv_label_set_text(sper_label, "#CFCFCF ____________________________");
		lv_obj_set_height(sper_label, 1);
		lv_obj_align(sper_label, check_obj, LV_ALIGN_IN_BOTTOM_MID, 0,5);

	}
	

	return check_obj;

}


/**
 * @brief check_network_task_func
 * @param task
 */
static void check_network_task_func(lv_task_t* task)
{
	
	int ret = -1;
	static bool is_wifi_start_config = false;

	if (MfSdkSysCheckTick(start_time, 60000*2) == 1)
	{
		if (network_param.currentKey == MF_LV_KEY_DOWN_SHORT_PRESS)
		MfSdkAudPlay((const s8 *)"cfail");
		tts_sleep();
		
//		power_reset();
		MfSdkPowerReset();
	}
	//Check the distribution network
	if (network_param.ischecking_network \
		|| (MfSdkCommGetNetSelect() == MFSDK_COMM_NET_ONLY_WIFI \
		&& is_wifi_start_config == false))
	{
		if (is_wifi_start_config == false)
		{
//			wifi_start_config();
			MfSdkCommWifiStartConfig();
		}
		is_wifi_start_config = true;
//		ret = wifi_check_state();
		ret = MfSdkCommWifiCheckState();
		if (ret == 1)
		{
			MfSdkAudPlay((const s8 *)"csuc");
			tts_sleep();
//			power_reset();
			MfSdkPowerReset();			
		}
	}
	//gprs mode
	if (network_param.isgprs_network)
	{
		is_wifi_start_config = false;
//		osl_set_4G_mode();
		MfSdkCommSet4gMode();
		MfSdkAudPlay((const s8 *)"gprs");
		network_param.ischecking_network = false;
		network_param.isgprs_network = false;
	}

}
void destory_network_pagewin()
{
	//return
	lv_obj_del(networkwin);
	networkwin = NULL;
	lv_free_png_file(IMG_MODE_NORMA);
	lv_free_png_file(IMG_MODE_SELECT);
}

/**
 * @brief networkpage_event_cb
 * @param obj
 * @param e
 */
static void networkpage_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t key = -1;
//	int ret = -1;
	if (e == LV_EVENT_KEY)
	{
//		mf_auxlcd_backlight(1);
		MfSdkLcdBackLight(MFSDK_LCD_ON);
		key = network_page_get_key();
		if (key == MF_LV_KEY_UP_SHORT_PRESS)//up key
		{
			lv_obj_del(gprs_obj);
			lv_obj_del(wifi_obj);
			gprs_obj = create_cb_btn(network_page, "1.4G", IMG_MODE_SELECT,true);
			wifi_obj = create_cb_btn(network_page, "2.WIFI", IMG_MODE_NORMA,false);
			network_param.isgprs_network = true;
			network_param.ischecking_network = false;
			network_param.currentKey = MF_LV_KEY_UP_SHORT_PRESS;
			
		}
		else if (key == MF_LV_KEY_DOWN_SHORT_PRESS)//down key
		{
			lv_obj_del(gprs_obj);
			lv_obj_del(wifi_obj);
			gprs_obj = create_cb_btn(network_page, "1.4G", IMG_MODE_NORMA,true);
			wifi_obj = create_cb_btn(network_page, "2.WIFI", IMG_MODE_SELECT,false);
			lv_obj_invalidate(gprs_obj);
			lv_obj_invalidate(wifi_obj);
			network_param.currentKey = MF_LV_KEY_DOWN_SHORT_PRESS;

			MfSdkAudPlay((const s8 *)"wifi");
			
			//wifi configure network
			if (network_param.ischecking_network)
				return;
			network_param.ischecking_network = true;
			network_param.isgprs_network = false;
		}
		else if (key == MF_LV_KEY_QUIT_SHORT_PRESS)//return key
		{
//			power_reset();
			MfSdkPowerReset();
		}
		else if (key == MF_LV_KEY_OK_SHORT_PRESS)
		{

		}
	}
}



void network_page_win(char* title, char* fontname)
{
	static lv_style_t style;
	lv_style_init(&style);

	lv_obj_t* parent = lv_scr_act();
	networkwin = lv_obj_create(parent, NULL);
	lv_obj_set_size(networkwin, lv_obj_get_width(parent), lv_obj_get_height(parent));


	lv_obj_t* lab_title = lv_label_create(networkwin, NULL);
	lv_label_set_long_mode(lab_title, LV_LABEL_LONG_BREAK);
	lv_obj_set_width(lab_title, lv_obj_get_width(parent));
	lv_obj_set_height(lab_title, 20);
	lv_obj_clean_style_list(lab_title, LV_LABEL_PART_MAIN);
	lv_label_set_align(lab_title, LV_LABEL_ALIGN_CENTER);


	if (fontname != 0)
	{
		//	lv_obj_set_style_local_text_font(lab_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_get_font(fontname));
	}
	lv_label_set_text(lab_title, title);
	lv_obj_align(lab_title, networkwin, LV_ALIGN_IN_TOP_MID, 0, 0);
	lv_style_set_text_color(&style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_obj_add_style(lab_title, LV_LABEL_PART_MAIN, &style);



	network_page = lv_page_create(networkwin, NULL);
	lv_obj_set_size(network_page, LV_HOR_RES, LV_VER_RES - lv_obj_get_height(lab_title));
	//set page position
	lv_obj_set_pos(network_page, 0, lv_obj_get_height(lab_title) + 20);
	//lv_obj_set_y(network_page, lv_obj_get_height(lab_title)+15);
	lv_obj_clean_style_list(network_page, LV_PAGE_PART_BG);
	lv_obj_clean_style_list(network_page, LV_PAGE_PART_SCROLLABLE);
	lv_obj_clean_style_list(network_page, LV_PAGE_PART_SCROLLBAR);
	lv_page_set_scrollbar_mode(network_page, LV_SCROLLBAR_MODE_DRAG);
	lv_page_set_scrl_layout(network_page, LV_LAYOUT_COLUMN_LEFT);

	static lv_style_t style_page_bg;
	lv_style_init(&style_page_bg);
	lv_style_set_bg_color(&style_page_bg, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_style_set_bg_opa(&style_page_bg, LV_STATE_DEFAULT, LV_OPA_20);
	lv_obj_add_style(network_page, LV_PAGE_PART_BG, &style_page_bg);
	lv_obj_set_event_cb(network_page, networkpage_event_cb);
	lvgl_group_set_obj(network_page);


	static lv_style_t style_label_bg;
	lv_style_init(&style_label_bg);
	lv_style_set_bg_color(&style_label_bg, LV_STATE_DEFAULT, LV_COLOR_RED);
	lv_style_set_bg_opa(&style_label_bg, LV_STATE_DEFAULT, LV_OPA_20);


	lv_load_png_file(IMG_MODE_NORMA);
	lv_load_png_file(IMG_MODE_SELECT);

	if (MfSdkCommGetNetSelect() == MFSDK_COMM_NET_ONLY_WIRELESS)
	{
		gprs_obj = create_cb_btn(network_page, "1.4G", IMG_MODE_SELECT,true);	
		wifi_obj = create_cb_btn(network_page, "2.WIFI", IMG_MODE_NORMA,false);
	}
	else
	{
		gprs_obj = create_cb_btn(network_page, "1.4G", IMG_MODE_NORMA,true);
		wifi_obj = create_cb_btn(network_page, "2.WIFI", IMG_MODE_SELECT,false);

	}
	
	network_param.ischecking_network = false;

	if (ptask == NULL)
	{
		start_time = MfSdkSysGetTick();
		ptask = lv_task_create(check_network_task_func, 500, LV_TASK_PRIO_MID, &network_param);	
	}

}