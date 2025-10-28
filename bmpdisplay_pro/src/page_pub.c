#include "lvgl/lvgl.h"
#include "page_pub.h"
#include "page_main.h"
#include "tracedef.h"
#include "pub/queue_pub.h"
#include "./player_proc.h"
#include "libapi_xpos/inc/mfsdk_lvgl.h"

//#include "pub/common/misc/inc/mfmalloc.h"

#define MAX_PAGE_NUM    20

typedef int (*page_close_func)(int ret);

typedef struct __st_page_stack_data
{
    lv_obj_t* page;
    page_close_func close_func;
}st_page_stack_data;

static st_page_stack_data page_stack_data[MAX_PAGE_NUM];

static int page_stack_index = 0;

static lv_group_t* group = 0;
//static int gui_init_flag = 0;
//static int group_init_flag = 0;

static lv_obj_t* page_win = NULL;
static lv_obj_t* ddlist = NULL;
int g_edit_value = 0;

//LV_IMG_DECLARE(img_screen);






void page_group_set_obj(lv_obj_t* obj)
{
//    lv_group_add_obj(group, obj);
//    lv_group_focus_obj(obj);

	MfSdkLvglGroupSet(obj);

	MFSDK_UNUSED(ddlist);
	MFSDK_UNUSED(page_win);
//	MFSDK_UNUSED(gui_init_flag);
	MFSDK_UNUSED(page_stack_index);
	MFSDK_UNUSED(page_stack_data);
}

void page_group_create()
{
#if 0 /*Modify by CHAR at 2023.12.18  10:30 */
    if (group_init_flag == 1) { return; }
    group_init_flag = 1;

    group = lv_group_create();
    lv_indev_t* cur_drv = NULL;

    for (;; )
    {
        cur_drv = lv_indev_get_next(cur_drv);

        if (!cur_drv)
        {
            break;
        }

        if (cur_drv->driver.type == LV_INDEV_TYPE_KEYPAD)
        {
            lv_indev_set_group(cur_drv, group);
        }
    }

    return;
#else
	MfSdkLvglGuiInit();
//	group = gui_get_group();
#endif /* if 0 */
}

static int play_net_mode()
{
    if (MfSdkCommGetNetMode() == MFSDK_COMM_NET_ONLY_WIRELESS)     //gprs
    {
        MfSdkAudPlay((const s8 *)"gprs.mp3");
    }
    else
    {
        MfSdkAudPlay((const s8 *)"wifimode.mp3");
    }
    return 0;
}

int network_page_get_key()
{
    uint32_t key;
    const void* d = lv_event_get_data();

    memcpy(&key, d, sizeof(uint32_t));
    APP_TRACE("network_page_get_key key = %d\r\n", key);

    return key;
}

extern unsigned int tick_get_btn_play;
extern int press_btn_flag;
extern int get_mqtt_con_state();



void playMbtn()
{
	MfSdkAudPlayBatteryLevel();
	
    play_net_mode();

    MfSdkSysSleep(1000);

    if (MfSdkCommLinkState() == 0)
    {
		MfSdkAudPlay((const s8 *)"netf.mp3");
    }
    else
    {
        MfSdkAudPlay((const s8 *)"nets.mp3");
    }
    APP_TRACE("net_func_link_state = %d\r\n", MfSdkCommLinkState());
    mqtt_play_state(get_mqtt_con_state());
}

void reset_tick_get_btn_play_time()
{
    tick_get_btn_play = MfSdkSysTimerOpen(500);
}

int page_get_key(int flag)
{
    int volume = 0;
    uint32_t key;
    char s_play_name[DATA_SIZE + 1] = { 0 };
    const void* d = lv_event_get_data();

    memcpy(&key, d, sizeof(uint32_t));
    APP_TRACE("page_get_key key = %d\r\n", key);

    if (key == MF_LV_KEY_QUIT_LONG_PRESS)
    {
        mqtt_publish("0");
        MfSdkAudPlay((const s8 *)"shut");

        while (MfSdkAudTtsState() == 1)        //Broadcasting
        {
            MfSdkSysSleep(500);
        }

        APP_TRACE("Shutdown---------\t\n");
        MfSdkSysSleep(1000);
//        Sys_power_off();
		MfSdkPowerOff();
    }
    else if (key == MF_LV_KEY_DOWN_SHORT_PRESS)
    {
        press_btn_flag = 1;
        reset_tick_get_btn_play_time();
        volume = MfSdkAudGetVolume();//Sys_GetTtsVolume();

        if (volume < 5)
        {
            volume += 1;
//            Sys_SetTtsVolume_Running(volume);            //Set the voice used for the current broadcast
			MfSdkAudSetVolumeRunning(volume);
			//pub_tts_play(TTS_VOLUME_NOR);
            strcpy(s_play_name, TTS_VOLUME_NOR);
        }
        else
        {
            //pub_tts_play(TTS_VOLUME_MAX);
            strcpy(s_play_name, TTS_VOLUME_MAX);
        }
        APP_TRACE("volume:%d\r\n", volume);
        fifo_set_zero();
        fifo_put((unsigned char*)s_play_name, DATA_SIZE);
        //paint_flag =1;
    }
    else if (key == MF_LV_KEY_UP_SHORT_PRESS)
    {
        press_btn_flag = 1;
        reset_tick_get_btn_play_time();
        volume = MfSdkAudGetVolume();//Sys_GetTtsVolume();

        if (volume > 0)
        {
            volume -= 1;
//            Sys_SetTtsVolume_Running(volume);            //Set the voice used for the current broadcast
			MfSdkAudSetVolumeRunning(volume);
//pub_tts_play(TTS_VOLUME_NOR);
            strcpy(s_play_name, TTS_VOLUME_NOR);
        }
        else
        {
            //pub_tts_play(TTS_VOLUME_MIN);
            strcpy(s_play_name, TTS_VOLUME_MIN);
        }
        APP_TRACE("volume:%d\r\n", volume);
        //paint_flag =1;
        fifo_set_zero();
        fifo_put((unsigned char*)s_play_name, DATA_SIZE);
    }
    else if (key == MF_LV_KEY_FUNC_SHORT_PRESS)
    {
//        Sys_Version();
		MfSdkSysVersion();
        press_btn_flag = 1;
        reset_tick_get_btn_play_time();

        fifo_set_zero();
        strcpy(s_play_name, TTS_BUTTON_M);
        fifo_put((unsigned char*)s_play_name, DATA_SIZE);
    }
    else
    {
        if ((flag == 1 && key == MF_LV_KEY_CANCEL_SHORT_PRESS) ||
            (flag == 0 && key != 0 && key % 4 == 0 && ((key >= MF_LV_KEY_F1_SHORT_PRESS && key <= MF_LV_KEY_0_SHORT_PRESS) || key == MF_LV_KEY_OK_SHORT_PRESS)))
        {
            MfSdkAudPlay((const s8 *)TTS_VOLUME_NOR);
        }
        /*press_btn_flag = 1;
           reset_tick_get_btn_play_time();

           fifo_set_zero();
           strcpy(s_play_name, TTS_VOLUME_NOR);
           fifo_put((unsigned char*)s_play_name, DATA_SIZE);*/
    }
    return key;
}

