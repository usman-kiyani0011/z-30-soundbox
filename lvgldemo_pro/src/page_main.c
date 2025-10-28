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
#include "pages/pages.h"
#include "func_wifi.h"
#include "pub/queue_pub.h"
#include "libapi_xpos/inc/libapi_comm.h"
#include "libapi_xpos/inc/mfsdk_lvgl.h"
#include "libapi_xpos/inc/mfsdk_esim.h"
#include "libapi_xpos/inc/mfsdk_ver.h"

#include "pages/page_qr.h"
#include "pages/page_esim.h"
#include "./pub/queue_pub.h"
#include "./pages/page_status_bar.h"

static DATE_TIME_T m_stDateTime = { 0 };


static lv_obj_t* mainpage = NULL;
static lv_obj_t* img = NULL;
static lv_obj_t* page_state = NULL;

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

void ClearAmountTimer() 
{
	amt_time = MfSdkSysGetTick();//clear amount timer
}

static void main_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t key;

	if (e == LV_EVENT_KEY) 
	{		
		MfSdkLcdBackLight(MFSDK_LCD_ON);
		key = page_get_key();
		APP_TRACE("key1999===%d\r\n", key);
		
		if (key == MF_LV_KEY_FUNC_LONG_PRESS)//ok key
		{
			if(1 == MfSdkCommWifiChipExist())
			{
				while (MfSdkAudTtsState() == 1)//Broadcasting
				{
					MfSdkSysSleep(500);
				}
				
				PubMultiPlay((const s8 *)"cnet.mp3");
				network_page_win();			
			}
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
				ClearAmountTimer();//claer timer
				func_pay(get_mainpage(), amt);
			}		
		}
		else if (key == MF_LV_KEY_CANCEL_SHORT_PRESS)
		{
			AppFormatSegmentLedAmount(0LL);
		}
		else if (key == MF_LV_KEY_F1_SHORT_PRESS)
		{
			//if(MfSdkCommGetNetMode() != MFSDK_COMM_NET_ONLY_WIRELESS && 1 == wifi_get_exist())
			//	func_wifi_show_ex();
			MfSdkLvglNetMenu_T netmenu;
			netmenu.obj = NULL;
			netmenu.height = page_get_statusbar_height();
			netmenu.font = "lv_font_montserrat_24";
			MfSdkLvglNetMenuSetting(&netmenu);
		}
		else if (key == MF_LV_KEY_F1_LONG_PRESS)
		{
			if (MfSdkCommLinkState() == 0)
		    {
		        APP_TRACE("TransactionProc network fail\n");
		        page_text_show_mid(get_mainpage(), "TIP", "Network unstable", 3000);
		        PubMultiPlay((const s8*)"netf.mp3");				
		    }
			else if(!MfSdkEsimIsEnable())
			{
				page_text_show_mid(get_mainpage(), "TIP", "ESIM function \r\nis not enabled", 3000);
			}
			else
			{
				EsimMainPage(get_mainpage());
			}			
		}
	}
}


unsigned int tick_get_btn_play = 0;
int press_btn_flag = 0;

static void btn_play_deweight(lv_task_t* task)
{
	char s_getdata[DATA_SIZE + 1] = { 0 };

	if (fifo_checkEmpty() == false)
	{
		play_proc_with_fifo();
		ClearAmountTimer();
	}

	if (MfSdkSysTimerCheck(tick_get_btn_play) == 0 && press_btn_flag == 1) 
	{
		fifo_get_last_one(s_getdata);
		APP_TRACE("fifo_get s_getdata is %s", s_getdata);

		if (strcmp(s_getdata, TTS_BUTTON_M) == 0) 
		{
			playMbtn();
		}
		else if (strcmp(s_getdata, TTS_VOLUME_MIN) == 0) 
		{
			PubMultiPlay((const s8 *)TTS_VOLUME_MIN);
		}
		else if (strcmp(s_getdata, TTS_VOLUME_MAX) == 0) 
		{
			PubMultiPlay((const s8 *)TTS_VOLUME_MAX);
		}
		else if (strcmp(s_getdata, TTS_VOLUME_NOR) == 0) 
		{
			//PubMultiPlay((const s8 *)TTS_VOLUME_NOR);
		}
		press_btn_flag = 0;
	}
	return;
}

static void update_time_task_func(lv_task_t* task)
{
	char str[32] = { 0 };
	char sztime[32] = { 0 };
	MfSdkSysTime_T stDateTime = { 0 };
	MfSdkSysGetTime(&stDateTime);

	if (m_stDateTime.nSecond == 0 || stDateTime.nHour != m_stDateTime.nHour || m_stDateTime.nMinute != stDateTime.nMinute)
	{
		sprintf(sztime, "%02d:%02d", stDateTime.nHour, stDateTime.nMinute);
		memcpy(&m_stDateTime, &stDateTime, sizeof(MfSdkSysTime_T));
		sprintf(str, "%04d-%02d-%02d", stDateTime.nYear, stDateTime.nMonth, stDateTime.nDay);
		if(NULL != lab_time)
			lv_label_set_text(lab_time, sztime);
		if(NULL != lab_year)
			lv_label_set_text(lab_year, str);
		MfSdkGuiLedTime(sztime);
	}
	
	if (MfSdkSysCheckTick(amt_time, 60000) == 1)
	{
		AppFormatSegmentLedAmount(0LL);
		ClearAmountTimer();
	}

	#if UART_EN
	uart_proc();
	#endif
	return;
}

static void page_state_show()
{	
	if(NULL != page_state) return;

	page_state = lv_layer_top();
	lv_obj_reset_style_list(page_state, LV_OBJ_PART_MAIN);
	lv_obj_set_size(page_state, lv_obj_get_width(page_state), page_get_statusbar_height());
	lv_obj_set_style_local_bg_opa(page_state, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
	lv_obj_set_style_local_bg_color(page_state, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREY);
	state_signal_page_init(page_state, LV_ALIGN_IN_TOP_LEFT, 10, 2);
	state_power_page_init(page_state, LV_ALIGN_IN_TOP_RIGHT, -10, 4);

	return;
}

static void page_main_show()
{
	lv_color_t color = LV_COLOR_BLACK;
	MfSdkSysTime_T stDateTime = { 0 };
	char strTime[64] = { 0 };
	char str[64] = { 0 };
	char app_ver[64] = { 0 };

	LV_FONT_DECLARE(font_size36);
	ClearAmountTimer();
	APP_TRACE("lcdtype = %d\r\n", MfSdkSysGetLcdType());
	
	mainpage = lv_scr_act();
	lv_obj_set_style_local_bg_color(mainpage, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

	img = lv_img_create(mainpage, NULL);
	lv_obj_set_size(img, lv_obj_get_width(mainpage), lv_obj_get_height(mainpage));

	MfSdkSysGetTime(&stDateTime);
	sprintf(strTime,"%02d:%02d",stDateTime.nHour,stDateTime.nMinute);
	sprintf(str, "%04d-%02d-%02d", stDateTime.nYear, stDateTime.nMonth, stDateTime.nDay);
	sprintf(app_ver, "(Ver: %s)", strstr((char *)MfSdkVerGetAppVer(), "V"));

	if (LCD_IS_320_480)
	{
		lv_load_png_file(MAINPNG_320X480);
		lv_img_set_src(img, MAINPNG_320X480);
		//time label
		lab_time = page_ShowText_Font(mainpage, strTime, LV_ALIGN_IN_TOP_MID, 0, 60, color, &font_size36);

		//data label
		lab_year = page_ShowTextOut(mainpage, str, LV_ALIGN_IN_TOP_MID, 0, 130, color, LV_FONT_16);

		//ver label
		page_ShowTextOut(mainpage, app_ver, LV_ALIGN_IN_BOTTOM_MID, 0, -40, color, LV_FONT_12);
	}
	else
	{
		lv_load_png_file(MAINPNG);
		lv_img_set_src(img, MAINPNG);
		//time label
		lab_time = page_ShowTextOut(mainpage, strTime, LV_ALIGN_IN_TOP_MID, 0, 40, color, LV_FONT_32);

		//data label
		lab_year = page_ShowTextOut(mainpage, str, LV_ALIGN_IN_TOP_MID, 0, 80, color, LV_FONT_16);

		//ver label
		page_ShowTextOut(mainpage, app_ver, LV_ALIGN_IN_BOTTOM_MID, 0, -10, color, LV_FONT_12);
	}



	lv_task_create(update_time_task_func, 1000, LV_TASK_PRIO_MID, 0);
	lv_task_create(btn_play_deweight, 50, LV_TASK_PRIO_MID, 0);

	page_group_set_obj(mainpage);
	lv_obj_set_event_cb(mainpage, main_event_cb);

	return;
}

static void play_start()
{
	PubMultiPlay((const s8 *)"welc.mp3");
}

void MainPage()
{	
	APP_TRACE("MainPage\r\n");

	page_group_create();

	//status bar
	page_state_show();
	page_main_show();

	play_start();//tts initialization
	lv_start();
}

