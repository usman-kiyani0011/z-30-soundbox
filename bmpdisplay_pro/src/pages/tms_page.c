#include "lvgl/lvgl.h"
#include "lvgl/src/lv_lib_png/lv_png.h"
#include "power_page.h"
#include "page_pub.h"
//#include "pub/power/power_key.h"
#include "xdk_et.h"
#include "tracedef.h"

//static int g_stime = 0;
static lv_obj_t* power_body = NULL;
//static lv_obj_t* lab_tip = NULL;
//static lv_task_t* uptime = NULL;
lv_obj_t* tms_label = NULL;

void _tms_proc_close_page(int ret)
{

}

static void _tms_proc_page_event_cb(lv_obj_t* obj, lv_event_t e)
{
#if 0
	int key;

	if (e == LV_EVENT_KEY)
	{
		mf_auxlcd_backlight(1);
        key = page_get_key(0);
		
        //if (key == MF_LV_KEY_OK_SHORT_PRESS || key == MF_LV_KEY_QUIT_SHORT_PRESS)
		{
			_tms_proc_close_page(0);
        }
	}
#endif
}
void _update_tms_progress(char *msg)
{
	char buff[10] = { 0 };
	sprintf(buff, "%s%%", msg);
	if(tms_label != NULL)
		lv_label_set_text(tms_label, buff);
}
#define TMS_PROC_PAGE	"P:exdata/tmsproc.png"
void app_tms_page_show(char * msg)
{
	lv_obj_t * parent = lv_scr_act();

	if (power_body != NULL)
	{
		lv_obj_del(power_body);
		power_body = NULL;
	}

	power_body = lv_img_create(parent, NULL);
	lv_load_png_file(TMS_PROC_PAGE);
	lv_img_set_src(power_body, TMS_PROC_PAGE);
#ifndef WIN32
	lv_img_set_auto_size(power_body, true);//
#endif // !WIN32
	lv_obj_align(power_body, NULL, LV_ALIGN_CENTER, 0, 0);
	page_group_set_obj(power_body);
	lv_obj_set_event_cb(power_body, NULL);
	char buff[10] = { 0 };
	sprintf(buff, "%s%%", msg);
	tms_label = lv_label_create(power_body, NULL);
	lv_label_set_text(tms_label, buff);
	lv_obj_set_style_local_text_color(tms_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_size(tms_label, 60, 30);
	lv_obj_align(tms_label, power_body, LV_ALIGN_CENTER, 0, 40);
	lv_obj_set_style_local_text_font(tms_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_get_font("lv_font_montserrat_32"));

	lv_obj_t* tms_label_tip1 = lv_label_create(power_body, NULL);
	lv_label_set_text(tms_label_tip1, "The device is undergoing tms upgrade,");
	lv_obj_set_style_local_text_color(tms_label_tip1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_size(tms_label_tip1, 60, 30);
	lv_obj_align(tms_label_tip1, tms_label, LV_ALIGN_CENTER, 0, 40);
	lv_obj_set_style_local_text_font(tms_label_tip1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_get_font("lv_font_montserrat_16"));
	//The device is undergoing tms upgrade, please wait. .
	lv_obj_t* tms_label_tip2 = lv_label_create(power_body, NULL);
	lv_label_set_text(tms_label_tip2, "please wait...");
	lv_obj_set_style_local_text_color(tms_label_tip2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_size(tms_label_tip2, 60, 30);
	lv_obj_align(tms_label_tip2, tms_label_tip1, LV_ALIGN_CENTER, 0, 40);
	lv_obj_set_style_local_text_font(tms_label_tip1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_get_font("lv_font_montserrat_16"));
	return;
}


