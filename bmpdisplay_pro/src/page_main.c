#include "xdk_et.h"
#include "page_main.h"
#include "uart_pub.h"
#include "pub/messagedatastruct.h"

#include "tracedef.h"
#include "network_page.h"
#include "page_card.h"
#include "page_pub.h"
#include "player_proc.h"
#include "card/func_pay.h"
#include "pages/power_page.h"
#include "pages/pages.h"
#include "func_wifi.h"
#include "pub/queue_pub.h"
#include "libapi_xpos/inc/libapi_comm.h"

static DATE_TIME_T m_stDateTime = { 0 };
static int m_wifi_flag = 0;

static lv_obj_t* mainpage = NULL;
static lv_obj_t* img = NULL;
static lv_obj_t* page_state = NULL;
static lv_obj_t* page_state_top = NULL;

static lv_obj_t* lab_ver = NULL;
static lv_obj_t* lab_time = NULL;
static lv_obj_t* lab_year = NULL;
static int amt_time=0;

lv_obj_t* get_mainpage()
{
	return mainpage;
}

lv_obj_t* get_imgpage()
{
	return img;
}

int get_WifiFlag() 
{
	return m_wifi_flag;
}

static void mian_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t key;

	if (e == LV_EVENT_KEY) 
	{		
		MfSdkLcdBackLight(MFSDK_LCD_ON);
		key = page_get_key(0);
		APP_TRACE("key1999===%d\r\n", key);
		
		if (key == MF_LV_KEY_FUNC_LONG_PRESS)//ok key
		{
			while (MfSdkAudTtsState() == 1)//Broadcasting
			{
				MfSdkSysSleep(500);
			}
			
			m_wifi_flag = 1;
			MfSdkAudPlay((const s8 *)"cnet.mp3");
			network_page_win("Net Config",NULL);			
		}
		else if (key >= MF_LV_KEY_1_SHORT_PRESS && key <= MF_LV_KEY_0_TRIPLE_PRESS)
		{
			int keyValue = GetKeyValue(key);
			APP_TRACE("keyValue---------%x\t\n",keyValue);
			if(0 == GetKeyStatus(key))//short press
			{
				char amt[13] = {0};

				memset(amt ,0, sizeof(amt));
				sprintf(amt, "%c", keyValue);
				amt_time = MfSdkSysGetTick();//claer timer
				func_pay2(get_mainpage(), amt);
			}		
		}
		else if (key == MF_LV_KEY_CANCEL_SHORT_PRESS)
		{
			MfSdkGuiLedAmount((char*)"0.00");
		}
		else if (key == MF_LV_KEY_F1_SHORT_PRESS)
		{
			if(MfSdkCommGetNetMode() != MFSDK_COMM_NET_ONLY_WIRELESS)
				func_wifi_show_ex();
		}

	}
}


void show_page_result(int ret,lv_obj_t* obj)
{
	//show result
	message_close_page(0);
	message_close_imagepage(0);
	close_qrcode_page(0);
	if(LCD_IS_320_480)
		page_image_show(mainpage, SUCCPNG_320X480, 3000);
	else
		page_image_show(mainpage, SUCCPNG, 3000);
}

void show_page_qrcode(lv_obj_t* parent, char*amount, char*qrcode)
{
	char title[64]={0};
	
	//QR code display
	if(strlen(qrcode)>0){//QR code display
		sprintf(title, "Amount:%s", amount);
		message_close_page(0);
		message_close_imagepage(0);
		close_qrcode_page(0);
		page_show_qrcode(parent, NULL, title, "Please Scan", qrcode, 60000);
	}
}

lv_obj_t* show_cardpage_qrcode(lv_obj_t* parent, void *pfunc, char*amount, char*qrcode, int timeover)
{
	lv_obj_t* winpage = 0;
	char title[64]={0};

	APP_TRACE("show_cardpage_qrcode\r\n");
	if(0 == parent)
	{
		APP_TRACE("page_code NULL!!\r\n");
		return NULL;
	}
	//QR code display
	if(strlen(qrcode)>0){//QR code display
		sprintf(title, "Amount:%s", amount);
		message_close_page(0);
		message_close_imagepage(0);
		close_qrcode_page(0);
		APP_TRACE("page_code\r\n");
		winpage = page_show_qrcode(parent, pfunc, title, "Please Scan", qrcode, timeover);
	}
	return winpage;
}

unsigned int tick_get_btn_play = 0;
int press_btn_flag = 0;

static void btn_play_deweight(lv_task_t* task)
{
	{
		char s_getdata[DATA_SIZE + 1] = { 0 };
 
		if (MfSdkSysTimerCheck(tick_get_btn_play) == 0 && press_btn_flag == 1) {
			fifo_get_last_one(s_getdata);
			APP_TRACE("fifo_get s_getdata is %s", s_getdata);

			if (strcmp(s_getdata, TTS_BUTTON_M) == 0) {
				playMbtn();
			}
			else if (strcmp(s_getdata, TTS_VOLUME_MIN) == 0) {
				MfSdkAudPlay((const s8 *)TTS_VOLUME_MIN);
			}
			else if (strcmp(s_getdata, TTS_VOLUME_MAX) == 0) {
				MfSdkAudPlay((const s8 *)TTS_VOLUME_MAX);
			}
			else if (strcmp(s_getdata, TTS_VOLUME_NOR) == 0) {
				MfSdkAudPlay((const s8 *)TTS_VOLUME_NOR);
			}
			press_btn_flag = 0;
		}
	}
}

static void update_time_task_func(lv_task_t* task)
{
	char str[32] = { 0 };
	MfSdkSysTime_T stDateTime = { 0 };
	MfSdkSysGetTime(&stDateTime);
	char sztime[32] = { 0 };

	if (m_stDateTime.nSecond == 0 || stDateTime.nHour != m_stDateTime.nHour || m_stDateTime.nMinute != stDateTime.nMinute)
	{
//		sprintf(sztime, "%02d:%02d", stDateTime.nHour, stDateTime.nMinute);
//		memcpy(&m_stDateTime, &stDateTime, sizeof(DATE_TIME_T));
//		sprintf(str, "%04d-%02d-%02d", stDateTime.nYear, stDateTime.nMonth, stDateTime.nDay);
//		lv_label_set_text(lab_time, sztime);
//		lv_label_set_text(lab_year, str);
		MfSdkGuiLedTime(sztime);
	}

	if (fifo_checkEmpty_ptr() == false)
	{
		play_proc_with_fifo();
		amt_time = MfSdkSysGetTick();
	}
	
	if (MfSdkSysCheckTick(amt_time, 60000) == 1)
	{
		MfSdkGuiLedAmount((char*)"0.00");
		amt_time = MfSdkSysGetTick();
	}

}

void AppDispBmp(lv_img_dsc_t* dsc)
{
	if(img != NULL) { lv_img_set_src(img, dsc); }

//	if(img != NULL)
//	{
//		lv_obj_del(img);
//		img = NULL;
//	}
//	
//	img = lv_img_create(mainpage, NULL);
//	lv_img_set_src(img, dsc);
}

static void page_main_show()
{

	MfSdkSysTime_T stDateTime = { 0 };
	char strTime[64] = { 0 };
	char str[64] = { 0 };
	char *app_ver = strstr((char*)MfSdkVerGetAppVer(), "V");
	int wifi_power_page_high = 0;
	static int user_data = 10;

	mainpage = lv_scr_act();
	
	img = lv_img_create(mainpage, NULL);
	lv_obj_set_size(img, lv_obj_get_width(mainpage), lv_obj_get_height(mainpage));

	if (LCD_IS_320_480)
	{
		lv_img_set_src(img, MAINPNG_320X480);
		wifi_power_page_high = 10;
	}
	else
		lv_img_set_src(img, MAINPNG);

	
	//Status bar
	page_state = lv_layer_top();
	lv_obj_reset_style_list(page_state, LV_OBJ_PART_MAIN);
	lv_obj_set_size(page_state, lv_obj_get_width(mainpage),/* 30 + wifi_power_page_high*/0);
	lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_obj_set_style_local_bg_opa(page_state, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
	lv_obj_set_style_local_bg_color(page_state, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);//lv_color_hex(0x0055F6)

//	if(MfSdkCommGetNetMode() == MODEL_MODE_ONLY_WIFI)
//		state_wifi_page_init(page_state, LV_ALIGN_IN_TOP_LEFT, 10, 2+wifi_power_page_high);
//	else
//		state_atc_page_init(page_state, LV_ALIGN_IN_TOP_LEFT, 10, 2+wifi_power_page_high);
//	state_power_page_init(page_state, LV_ALIGN_IN_TOP_RIGHT, -10, 4+wifi_power_page_high);
	MfSdkSysGetTime(&stDateTime);
	sprintf(strTime,"%02d:%02d",stDateTime.nHour,stDateTime.nMinute);
	
	//ver label
	lab_ver = lv_label_create(mainpage, NULL);
	lv_label_set_long_mode(lab_ver, LV_LABEL_LONG_EXPAND);
	lv_label_set_recolor(lab_ver, true);
	lv_label_set_text(lab_ver, "");
	lv_obj_align(lab_ver, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -110);
	lv_obj_set_style_local_text_color(lab_ver, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
	lv_obj_set_width(lab_ver, lv_obj_get_width(mainpage) - 10);

	//time label
	lab_time = lv_label_create(mainpage, NULL);
	lv_label_set_long_mode(lab_time, LV_LABEL_LONG_EXPAND);
	lv_label_set_recolor(lab_time, true);
	lv_label_set_text(lab_time, "");
	lv_obj_align(lab_time, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -50);
	lv_obj_set_style_local_text_color(lab_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
	lv_obj_set_width(lab_time, lv_obj_get_width(mainpage) - 10);

	sprintf(str, "%04d-%02d-%02d", stDateTime.nYear, stDateTime.nMonth, stDateTime.nDay);
	lab_year = lv_label_create(mainpage, NULL);
	lv_label_set_long_mode(lab_year, LV_LABEL_LONG_EXPAND);
	lv_label_set_recolor(lab_year, true);
	lv_label_set_text(lab_year, "");
	lv_obj_align(lab_year, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -20);
	lv_obj_set_style_local_text_color(lab_year, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
	lv_obj_set_width(lab_year, lv_obj_get_width(mainpage) - 10);

	amt_time = MfSdkSysGetTick();
	lv_task_create(update_time_task_func, 1000, LV_TASK_PRIO_MID, &user_data);
	lv_task_create(btn_play_deweight, 100, LV_TASK_PRIO_MID, 0);
	
	 page_group_set_obj(lab_time);
	 lv_obj_set_event_cb(lab_time, mian_event_cb);
	 APP_TRACE("page_main_show3\r\n");



}

void play_start()
{
	MfSdkAudPlay((const s8 *)"welc");
}

void page_main()
{	
	MfSdkKbKeySetParam(MF_KEY_OK, 500, 2000, 0);
	MfSdkKbKeySetParam(MF_KEY_QUIT, 500, 2000, 0);
	MfSdkKbKeySetParam(MF_KEY_UP, 500, 2000, 0);
	MfSdkKbKeySetParam(MF_KEY_DOWN, 500, 2000, 0);

	page_group_create();
	page_main_show();
	MfSdkPowerPageCb(app_power_page_show);

	play_start();//tts initialization
	lv_start();
}

