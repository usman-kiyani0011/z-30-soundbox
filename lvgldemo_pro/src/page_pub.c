#include "lvgl/lvgl.h"
#include "pages/pages.h"
#include "page_pub.h"
#include "page_main.h"
#include "tracedef.h"
#include "pub/queue_pub.h"
#include "./player_proc.h"
#include "libapi_xpos/inc/mfsdk_lvgl.h"


static pFuncPagePubCallback g_pFuncCallback = NULL;

#define MAX_PAGE_NUM    20

typedef int (*page_close_func)(int ret);

typedef struct __st_page_stack_data
{
    lv_obj_t* page;
    page_close_func close_func;
}st_page_stack_data;

static lv_obj_t* page_win = NULL;
static lv_task_t* task_meaasge = NULL;




void page_group_set_obj(lv_obj_t* obj)
{
	MfSdkLvglGroupSet(obj);
}

void page_group_create()
{
	MfSdkLvglGuiInit();
}

static int play_net_mode()
{
    if (MfSdkCommGetNetMode() == MFSDK_COMM_NET_ONLY_WIRELESS)     //gprs
    {
        PubMultiPlay((const s8 *)"gprs.mp3");
    }
    else
    {
        PubMultiPlay((const s8 *)"wifimode.mp3");
    }
    return 0;
}

int pub_page_get_key()
{
    uint32_t key = -1;
    const void* d = lv_event_get_data();

    memcpy(&key, d, sizeof(uint32_t));
    APP_TRACE("pub_page_get_key key = %d\r\n", key);

    return key;
}

extern unsigned int tick_get_btn_play;
extern int press_btn_flag;
extern int get_mqtt_con_state();

void AppTestPlayVoi(void)
{
	unsigned char temp[32] = {0};
	unsigned char tempFile[32] = {0};

	int j = 0;
	do
	{
		memset(temp,0,sizeof(temp));
		memset(tempFile,0,sizeof(tempFile));
		snprintf(temp,sizeof(temp),"f%d",j++);
		APP_TRACE("temp:%s\r\n",temp);
		read_profile_string("voit", temp, tempFile, sizeof(tempFile)-1, "", "exdata\\voit.ini");
		APP_TRACE("tempFile:%s\r\n",tempFile);
		if( strlen(tempFile) > 0 ) { pub_tts_play(tempFile); }
	}while(strlen((char*)tempFile) > 0);
}

void playMbtn()
{
    int netlinkstate = MfSdkCommLinkState();

	AppPlayBatteryLevel();
	//MfSdkAudPlayBatteryLevel();
	
    play_net_mode();

    MfSdkSysSleep(1000);

    APP_TRACE("net_func_link_state = %d\r\n", netlinkstate);
	if(0 == netlinkstate)
	{
		PubMultiPlay((const s8*)"netf.mp3");
	}
	else
	{		
		mqtt_play_state(get_mqtt_con_state());
	}
	AppTestPlayVoi();
	return;
}

void reset_tick_get_btn_play_time()
{
    tick_get_btn_play = MfSdkSysTimerOpen(500);
}

static int PubKeyRespProc(int key)
{
    int volume = 0;
    char s_play_name[DATA_SIZE + 1] = { 0 };

    if (key == MF_LV_KEY_QUIT_LONG_PRESS)
    {
        PubMultiPlay((const s8 *)"shut.mp3");

        while (MfSdkAudTtsState() == 1)        //Broadcasting
        {
            MfSdkSysSleep(500);
        }

        APP_TRACE("Shutdown---------\t\n");
        MfSdkSysSleep(1000);
		MfSdkPowerOff();
    }
    else if (key == MF_LV_KEY_DOWN_SHORT_PRESS)
    {
        press_btn_flag = 1;
        reset_tick_get_btn_play_time();
        volume = MfSdkAudGetVolume();

        if (volume < 5)
        {
            volume += 1;
			if(0 == volume)
			{
				MfSdkKbSetKeySound(1);
		        page_text_show(get_mainpage(), "VOLUME", "Mute Off", 1000);
			}
			MfSdkAudSetVolumeRunning(volume);            //Set the voice used for the current broadcast
            strcpy(s_play_name, TTS_VOLUME_NOR);
        }
        else
        {
            strcpy(s_play_name, TTS_VOLUME_MAX);
        }
        APP_TRACE("volume:%d\r\n", volume);
        fifo_key_set_zero();
        fifo_key_put((unsigned char*)s_play_name);
    }
    else if (key == MF_LV_KEY_UP_SHORT_PRESS)
    {
        press_btn_flag = 1;
        reset_tick_get_btn_play_time();
        volume = MfSdkAudGetVolume();

		//volume = -1, Mute
        if (volume > 0)
        {
            volume -= 1;
			MfSdkAudSetVolumeRunning(volume);            //Set the voice used for the current broadcast
			if (volume > 0)
			{
            	strcpy(s_play_name, TTS_VOLUME_NOR);
			}
			else
			{
	            strcpy(s_play_name, TTS_VOLUME_MIN);
			}
			APP_TRACE("volume:%d\r\n", volume);
	        fifo_key_set_zero();
	        fifo_key_put((unsigned char*)s_play_name);	
        }
        else if (0 == volume)
        {
			MfSdkKbSetKeySound(0);
			MfSdkAudSetVolumeRunning(-1);
	        page_text_show(get_mainpage(), "VOLUME", "Mute ON", 1000);
        }
    }
    else if (key == MF_LV_KEY_FUNC_SHORT_PRESS)
    {
		MfSdkSysVersion();
		APP_TRACE("Free Spcace [%d]\r\n", MfSdkFsGetFreeSpace(""));
		APP_TRACE("Total Spcace [%d]\r\n", MfSdkFsGetTotalSpace(""));

		if(MFSDK_TRUE == MfSdkLcdExistScreen())
			AppShowVersion("VERSION");

        press_btn_flag = 1;
        reset_tick_get_btn_play_time();

        fifo_key_set_zero();
        strcpy(s_play_name, TTS_BUTTON_M);
        fifo_key_put((unsigned char*)s_play_name);
    }
    else
    {
        /*if ((flag == 1 && key == MF_LV_KEY_CANCEL_SHORT_PRESS) ||
            (flag == 0 && key != 0 && key % 4 == 0 && ((key >= MF_LV_KEY_F1_SHORT_PRESS && key <= MF_LV_KEY_0_SHORT_PRESS) || key == MF_LV_KEY_OK_SHORT_PRESS)))
        {
            PubMultiPlay((const s8 *)TTS_VOLUME_NOR);
        }*/
    }
    return key;
}

int page_get_key()
{
	uint32_t key = -1;
	const void* d = lv_event_get_data();

	memcpy(&key, d, sizeof(uint32_t));
	APP_TRACE("page_get_key key = %d\r\n", key);

	PubKeyRespProc(key);
	return key;
}

void page_netmenu_key_proc(int key)
{
	if(key >= MF_LV_KEY_QUIT_SHORT_PRESS)
	{
		PubKeyRespProc(key);
	}
}

lv_obj_t* page_ShowTextOut(lv_obj_t* parent, char* str, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, lv_color_t color, lv_font_t* font)
{
    if (NULL == parent) return 0;

	APP_TRACE("ShowTextOut = %s \r\n", str);
    lv_obj_t* lab = lv_label_create(parent, NULL);
    lv_label_set_long_mode(lab, LV_LABEL_LONG_EXPAND);
    //lv_label_set_recolor(lab, true);
    lv_label_set_text(lab, str);
    lv_obj_set_style_local_bg_color(lab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_text_font(lab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, font);
    lv_obj_set_style_local_text_color(lab, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color);
    lv_obj_set_width(lab, lv_obj_get_width(parent));
    lv_obj_align(lab, NULL, align, x_ofs, y_ofs);

    return lab;
}


static void _message_close_text_page(lv_task_t* task)
{
    if (page_win != 0)
    {
        if (task_meaasge)
        {
            lv_task_del(task_meaasge);
            task_meaasge = 0;
        }
        lv_obj_del(page_win);
        page_win = 0;

		if(g_pFuncCallback != NULL){ g_pFuncCallback(0, NULL);}
    }
	g_pFuncCallback = NULL;
}

static void _textpage_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t key;

	if (e == LV_EVENT_KEY) 
	{
		key = page_get_key(); 
		APP_TRACE("_card_event_cb ret == %d\r\n", key);
		if (key == MF_LV_KEY_CANCEL_SHORT_PRESS) 
		{
			_message_close_text_page(NULL);
		}
		else if (key == MF_LV_KEY_OK_SHORT_PRESS) 
		{
			_message_close_text_page(NULL);
		}
	}
}

lv_obj_t* page_text_show(lv_obj_t* parent, char* title, char* text, int timeout)
{
    if (parent == NULL)
    {
        return NULL;
    }

	if (task_meaasge) 
	{
		lv_task_del(task_meaasge);
		task_meaasge = 0;
	}

    if (page_win != 0)
    {
        lv_obj_del(page_win);
        page_win = 0;
	}
    page_win = page_create_win(parent, _textpage_event_cb);
    lv_obj_set_size(page_win, lv_obj_get_width(parent), lv_obj_get_height(parent));
    page_create_title(page_win, title);
    page_create_msg(page_win, text);

    if (timeout > 0)
    {
        task_meaasge = lv_task_create(_message_close_text_page, timeout, LV_TASK_PRIO_MID, 0);
    }
    return page_win;
}

lv_obj_t* page_text_show_mid(lv_obj_t* parent, char* title, char* text, int timeout)
{
    if (parent == NULL)
    {
        return NULL;
    }

	if (task_meaasge) 
	{
		lv_task_del(task_meaasge);
		task_meaasge = 0;
	}

    if (page_win != 0)
    {
        lv_obj_del(page_win);
        page_win = 0;
	}
    page_win = page_create_win(parent, _textpage_event_cb);
    lv_obj_set_size(page_win, lv_obj_get_width(parent), lv_obj_get_height(parent));
    page_create_title(page_win, title);
    page_create_msg_mid(page_win, text);

    if (timeout > 0)
    {
        task_meaasge = lv_task_create(_message_close_text_page, timeout, LV_TASK_PRIO_MID, 0);
    }
    return page_win;
}

lv_obj_t* page_text_show_callback(lv_obj_t* parent, char* title, char* text, int timeout,pFuncPagePubCallback pFuncCallback)
{
	g_pFuncCallback = pFuncCallback;
	page_text_show(parent, title, text, timeout);
}


//api use for specific font library
lv_obj_t*  page_ShowText_Font(lv_obj_t * parent, char *str, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, lv_color_t color, lv_font_t* font)
{
	static lv_style_t style;
	if(NULL == parent) return 0;

	lv_obj_t *lab = lv_label_create(parent, NULL);

	lv_style_reset(&style);
	lv_style_init(&style);
	lv_style_set_text_font(&style, LV_STATE_DEFAULT, font);
	lv_obj_add_style(lab, LV_LABEL_PART_MAIN, &style);

	lv_label_set_long_mode(lab, LV_LABEL_LONG_EXPAND);
	lv_label_set_text(lab, str);
	lv_obj_set_style_local_bg_color(lab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_obj_set_style_local_text_font(lab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, font);
	lv_obj_set_style_local_text_color(lab, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color);
	lv_obj_set_width(lab, lv_obj_get_width(parent));
	lv_obj_align(lab, NULL, align, x_ofs, y_ofs);

	return lab;
}

lv_obj_t*  page_ShowText_MultiFont(lv_obj_t * parent, int index, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, lv_color_t color)
{
	static lv_style_t style;
	lv_font_t *font = GetMultiFont();
	//char *str = GetDispMessage(index);
	char *str = GetDispMessageFromCfg(index);
	
	if(NULL == parent || NULL == str || NULL == font) return 0;

	lv_style_reset(&style);
	lv_style_init(&style);
	lv_style_set_text_font(&style, LV_STATE_DEFAULT, font);

	lv_obj_t *lab = lv_label_create(parent, NULL);
	lv_obj_add_style(lab, LV_LABEL_PART_MAIN, &style);
	lv_label_set_long_mode(lab, LV_LABEL_LONG_EXPAND);
	lv_label_set_text(lab, str);
	lv_obj_set_style_local_bg_color(lab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_obj_set_style_local_text_font(lab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, font);
	lv_obj_set_style_local_text_color(lab, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color);
	lv_obj_set_width(lab, lv_obj_get_width(parent));
	lv_obj_align(lab, NULL, align, x_ofs, y_ofs);

	return lab;
}

