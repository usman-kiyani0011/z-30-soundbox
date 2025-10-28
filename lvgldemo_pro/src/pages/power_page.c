#include <stdint.h>
#include "lvgl/lvgl.h"
#include "lvgl/lvgl/src/lv_lib_png/lv_png.h"
#include "lvgl/lvgl/src/lv_misc/lv_task.h"

#include "power_page.h"
#include "../page_pub.h"
//#include "pub/power/power_key.h"
#include "xdk_et.h"
#include "tracedef.h"
#include "../page_main.h"
#include <stddef.h>
#include "libapi_xpos/inc/mfsdk_define.h"

static int g_stime = 0;
static lv_obj_t* power_body = NULL;
//static lv_obj_t* lab_tip = NULL;
static lv_task_t* uptime = NULL;

static void _power_close_page(int ret)
{
    if (power_body != NULL)
    {
        lv_obj_del(power_body);
        power_body = NULL;
        if (uptime != 0)
	        lv_task_del(uptime);
        uptime = 0;
		if(LCD_IS_320_480)
		{
			lv_free_png_file(LOWPOWERPNG_320X480);
			lv_free_png_file(SHUTDOWNRPNG_320X480);
		}
		else
		{
			lv_free_png_file(LOWPOWERPNG);
			lv_free_png_file(SHUTDOWNRPNG);
		}

        //if (m_page_close_page_func != 0) m_page_close_page_func(ret, NULL);
    }
}

static void _power_page_event_cb(lv_obj_t* obj, lv_event_t e)
{
	int key;

	if (e == LV_EVENT_KEY)
	{
		MfSdkLcdBackLight(MFSDK_LCD_ON);
        key = page_get_key();
		
        //if (key == MF_LV_KEY_OK_SHORT_PRESS || key == MF_LV_KEY_QUIT_SHORT_PRESS)
		{
            _power_close_page(0);
        }
	}
	MFSDK_UNUSED(key);
}

static void update_time(lv_task_t* btn)
{
    g_stime--;
    if (g_stime == 0)
    {
        _power_close_page(0);
    }
    
}

void app_power_page_show(int msg)
{
	lv_obj_t * parent = get_imgpage();
	
	if(NULL == parent)
		parent = lv_scr_act();

	if (power_body != NULL)
	{
		lv_obj_del(power_body);
		power_body = NULL;
	}

	power_body = lv_img_create(parent, NULL);
	lv_obj_set_size(power_body, lv_obj_get_width(parent), lv_obj_get_height(parent));

	if(msg==POWER_LOWER_TYPE)
	{
		if(LCD_IS_320_480)
		{
			lv_load_png_file(LOWPOWERPNG_320X480);
			lv_img_set_src(power_body, LOWPOWERPNG_320X480);
		}
		else
		{
			lv_load_png_file(LOWPOWERPNG);
			lv_img_set_src(power_body, LOWPOWERPNG);
		}
		lv_obj_align(power_body, NULL, LV_ALIGN_CENTER, 0, 0);
		
		g_stime = 10;//time out 10s
		if(g_stime>0)
		    uptime = lv_task_create(update_time, 1000, LV_TASK_PRIO_LOW, NULL);
	}
	else if(msg==Power_SHUTDOWN_TYPE)
	{
		if(LCD_IS_320_480)
		{
			lv_load_png_file(SHUTDOWNRPNG_320X480);
			lv_img_set_src(power_body, SHUTDOWNRPNG_320X480);
		}
		else
		{
			lv_load_png_file(SHUTDOWNRPNG);
			lv_img_set_src(power_body, SHUTDOWNRPNG);
		}
		lv_obj_align(power_body, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	}
	return;
}


