#include "network_page.h"
#include "page_main.h"
#include "lvgl/lvgl.h"
#include "lvgl/src/lv_lib_png/lv_png.h"
#include "tracedef.h"
#include "libapi_xpos/inc/mfsdk_sys.h"
#include "libapi_xpos/inc/util_tts.h"
#include "libapi_xpos/inc/mfsdk_aud.h"
#include "libapi_xpos/inc/mfsdk_power.h"
#include "libapi_xpos/inc/mfsdk_comm.h"
#include "page_pub.h"
#include "pages/pages.h"

#define IMG_MODE_NORMA			"P:exdata/mnormal.png"
#define IMG_MODE_SELECT			"P:exdata/selected.png"

//network interface pointer
static network_param_t network_param = { 0 };
static lv_obj_t* networkwin = NULL;
static lv_obj_t* network_page = NULL;
static lv_obj_t* gprs_obj = NULL;
static lv_obj_t* wifi_obj = NULL;
static lv_task_t* ptask = NULL;

static s32 tick_timeout = 15;
static int start_time=0;
static bool is_wifi_start_config = false;

static void tts_sleep()
{
	while (MfSdkAudTtsState() == 1)//Broadcasting
	{
		MfSdkSysSleep(500);
	}
	//MfSdkSysSleep(1000);
}

static void setNetworkTimer(int timeout)
{
	tick_timeout = timeout;
	start_time = MfSdkSysGetTick();
}

void destory_network_pagewin()
{
	APP_TRACE("============close wifi configer============%d\r\n", network_param.currentMode);
	MfSdkCommConfigReset();
	if (network_param.currentMode == WIFI_MODE)
	{
		APP_TRACE("MfSdkCommWifiRestart!\r\n");
		MfSdkCommWifiRestart();
	}

	if(ptask != NULL)
	{
		lv_task_del(ptask);
		ptask = NULL;
	}

	if(networkwin != NULL)
	{
		if(gprs_obj != NULL)
		{
			lv_obj_del(gprs_obj);
			gprs_obj = NULL;
		}
		if(wifi_obj != NULL)
		{
			lv_obj_del(wifi_obj);
			wifi_obj = NULL;
		}
		if(network_page != NULL)
		{
			lv_obj_del(network_page);
			network_page = NULL;
		}
		lv_obj_del(networkwin);
		networkwin = NULL;
	}
	start_time = 0;
	memset(&network_param, 0, sizeof(network_param_t));
	lv_free_png_file(IMG_MODE_NORMA);
	lv_free_png_file(IMG_MODE_SELECT);
	AppPowerUnlockApp((char*)"netconfig");
}

//create radio button
static lv_obj_t* create_cb_btn(lv_obj_t* parent, const char* labtitle, const char* filename,bool isSplit)
{
	lv_obj_t* check_obj = lv_obj_create(parent, NULL);
	lv_obj_clean_style_list(check_obj, LV_OBJ_PART_MAIN);
	lv_obj_set_width(check_obj, lv_page_get_width_fit(parent));
	lv_obj_set_height(check_obj,50);
	

	lv_obj_t* gprs_lab = lv_label_create(check_obj, NULL);
	lv_label_set_text(gprs_lab, labtitle);
	lv_obj_align(gprs_lab, check_obj, LV_ALIGN_IN_LEFT_MID, 16, 0);

	lv_obj_t* check_btn = lv_imgbtn_create(check_obj, NULL);
	lv_imgbtn_set_src(check_btn, LV_BTN_STATE_RELEASED, filename);
	lv_obj_align(check_btn, check_obj, LV_ALIGN_IN_RIGHT_MID, -16, 0);

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

	APP_TRACE("============CommLinkState[%d]============\r\n", MfSdkCommLinkState());
	if (MfSdkSysCheckTick(start_time, tick_timeout*1000) == 1)
	{
		if (network_param.currentMode == WIFI_MODE)
		{
			PubMultiPlay((const s8 *)"cfail.mp3");
			tts_sleep();
		}
//		MfSdkPowerReset();
		destory_network_pagewin();
	}
	//Check the distribution network
	if (network_param.ischecking_network == true
		|| MfSdkCommGetNetSelect() == MFSDK_COMM_NET_ONLY_WIFI)
	{
		if (is_wifi_start_config == false)
		{
			APP_TRACE("============StartConfig============\r\n");
			MfSdkCommWifiStartConfig();
			is_wifi_start_config = true;
		}
		
		ret = MfSdkCommWifiCheckState();
		APP_TRACE("wifi config check state:%d\r\n",ret);
		if (ret == 1)
		{
			PubMultiPlay((const s8 *)"csuc.mp3");
			tts_sleep();
//			MfSdkPowerReset();	
			destory_network_pagewin();
		}
		else if (ret == -1)
		{
			PubMultiPlay("cfail.mp3");
			tts_sleep();
			//MfSdkPowerReset();
			destory_network_pagewin();
		}
	}
	//set gprs mode
	if (true == network_param.isgprs_network)
	{
		if(false == network_param.ischecking_network)
		{
			APP_TRACE("============close wifi configer============\r\n");
			MfSdkCommConfigReset();
			MfSdkCommWifiStopConfig();
		}
		is_wifi_start_config = false;
		MfSdkCommSet4gMode();
		network_param.ischecking_network = false;
		network_param.isgprs_network = false;
	}

}

/**
 * @brief networkpage_event_cb
 * @param obj
 * @param e
 */
static void networkpage_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t key = -1;

	if (e == LV_EVENT_KEY)
	{
		MfSdkLcdBackLight(MFSDK_LCD_ON);
		key = pub_page_get_key();
		if (key == MF_LV_KEY_UP_SHORT_PRESS || key == MF_LV_KEY_F2_SHORT_PRESS)//GPRS_MODE
		{
			lv_obj_del(gprs_obj);
			lv_obj_del(wifi_obj);
			gprs_obj = create_cb_btn(network_page, "1. 4G", IMG_MODE_SELECT,false);
			wifi_obj = create_cb_btn(network_page, "2. WIFI", IMG_MODE_NORMA,false);
			lv_obj_invalidate(gprs_obj);
			lv_obj_invalidate(wifi_obj);

			PubMultiPlay((const s8 *)"gprs.mp3");
			//tts_sleep();
			setNetworkTimer(5);

			network_param.currentMode = GPRS_MODE;
			network_param.isgprs_network = true;
			network_param.ischecking_network = false;
			
		}
		else if (key == MF_LV_KEY_DOWN_SHORT_PRESS || key == MF_LV_KEY_F1_SHORT_PRESS)//WIFI_MODE
		{
			lv_obj_del(gprs_obj);
			lv_obj_del(wifi_obj);
			gprs_obj = create_cb_btn(network_page, "1. 4G", IMG_MODE_NORMA,false);
			wifi_obj = create_cb_btn(network_page, "2. WIFI", IMG_MODE_SELECT,false);
			lv_obj_invalidate(gprs_obj);
			lv_obj_invalidate(wifi_obj);

			PubMultiPlay((const s8 *)"wifi.mp3");
			//tts_sleep();
			setNetworkTimer(120);

			//wifi configure network
			network_param.currentMode = WIFI_MODE;
			if (false == network_param.ischecking_network)
			{
				network_param.ischecking_network = true;
				network_param.isgprs_network = false;		
			}
		}
		else if (key == MF_LV_KEY_QUIT_SHORT_PRESS || key == MF_LV_KEY_OK_SHORT_PRESS)
		{
//			MfSdkPowerReset();
			destory_network_pagewin();
		}
	}
}


void network_page_win()
{
	int timeout = 0;
	char *title = NULL;

	if(LCD_IS_320_480)
	{
		title = "NETWORK SETUP";
	}
	else
	{
		title = "NET SETUP";
	}

	AppPowerLockApp((char*)"netconfig");
	lv_obj_t* parent = lv_scr_act();
	networkwin = lv_obj_create(parent, NULL);
	lv_obj_set_size(networkwin, lv_obj_get_width(parent), lv_obj_get_height(parent));

    lv_obj_t* lab_title = page_create_title(networkwin, title);


	network_page = lv_page_create(networkwin, NULL);
	lv_obj_set_size(network_page, LV_HOR_RES, LV_VER_RES - lv_obj_get_height(lab_title));
	//set page position
	lv_obj_set_pos(network_page, 0, lv_obj_get_height(lab_title) + 40);
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


	lv_load_png_file(IMG_MODE_NORMA);
	lv_load_png_file(IMG_MODE_SELECT);
	memset(&network_param, 0, sizeof(network_param_t));
	is_wifi_start_config = false;
	
	if (MfSdkCommGetNetSelect() == MFSDK_COMM_NET_ONLY_WIRELESS)
	{
		gprs_obj = create_cb_btn(network_page, "1. 4G", IMG_MODE_SELECT,false);	
		wifi_obj = create_cb_btn(network_page, "2. WIFI", IMG_MODE_NORMA,false);
		network_param.ischecking_network = false;
		network_param.currentMode = GPRS_MODE;
		timeout = 15;
	}
	else
	{
		gprs_obj = create_cb_btn(network_page, "1. 4G", IMG_MODE_NORMA,false);
		wifi_obj = create_cb_btn(network_page, "2. WIFI", IMG_MODE_SELECT,false);
		network_param.ischecking_network = true;
		network_param.currentMode = WIFI_MODE;
		timeout = 120;
	}
	

	if (ptask == NULL)
	{
		setNetworkTimer(timeout);
		//start_time = MfSdkSysGetTick();
		ptask = lv_task_create(check_network_task_func, 500, LV_TASK_PRIO_MID, &network_param);	
	}

}

