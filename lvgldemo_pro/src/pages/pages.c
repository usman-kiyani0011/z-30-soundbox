#include "lvgl/lvgl.h"
#include "pages.h"
#include "../page_pub.h"

static int applockcnt=0;

s32 AppPowerLockApp(char *sfun)
{
	applockcnt++;
	APP_TRACE("AppPowerlock [%s][locknct]=%d", sfun, applockcnt);
    MfSdkPowerLockApp(sfun == NULL ? "" : sfun);

    return 0;
}

void AppPowerUnlockApp(char *sfun)
{
	applockcnt--;
	APP_TRACE("AppPowerUnlockApp [%s][locknct]=%d", sfun, applockcnt);
    MfSdkPowerUnlockApp();
}


lv_obj_t* page_create_title(lv_obj_t *parent, char* title)
{
	lv_obj_t* lab_title = lv_label_create(parent, NULL);
	lv_label_set_long_mode(lab_title, LV_LABEL_LONG_BREAK);
	lv_obj_set_width(lab_title, lv_obj_get_width(parent));
	lv_obj_set_height(lab_title, page_get_title_height());
	lv_label_set_align(lab_title, LV_LABEL_ALIGN_CENTER);
	lv_label_set_text(lab_title, title);
	lv_obj_align(lab_title, parent, LV_ALIGN_IN_TOP_LEFT, 0, page_get_statusbar_height());	
	lv_obj_add_style(lab_title, LV_LABEL_PART_MAIN, page_style_get_title());

	lv_obj_set_style_local_radius(lab_title, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_opa(lab_title, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_bg_color(lab_title, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0055F6));
	return lab_title;
}

lv_obj_t * page_create_btn(lv_obj_t* parent, char* text, lv_event_cb_t btn_event_cb, int align)
{
	lv_obj_t* page_win;
	int in_align;

	page_win = (lv_obj_t *) parent;	
	lv_obj_t* btn = lv_btn_create(page_win, NULL);
	lv_obj_set_size(btn, 120, page_get_btn_height());
	
	if (align == PAGE_BTN_ALIGN_LEFT) 
	{
		in_align = LV_ALIGN_IN_BOTTOM_LEFT;
	}
	else 
	{
		in_align = LV_ALIGN_IN_BOTTOM_RIGHT;
	}
	
	lv_obj_align(btn, page_win, in_align, align == PAGE_BTN_ALIGN_LEFT ? 10 : -10, -5);	
	lv_obj_set_event_cb(btn, btn_event_cb);
	lv_obj_t* lab = lv_label_create(btn, NULL);
	lv_obj_set_width(lab, lv_obj_get_width(btn));
	lv_label_set_align(lab, LV_LABEL_ALIGN_CENTER);
	lv_label_set_text(lab, text);
	lv_obj_add_style(btn, LV_BTN_PART_MAIN, page_style_get_button());

	return  btn;
}

lv_obj_t* page_create_base(lv_obj_t* parent)
{
	lv_obj_t*  page_base = lv_obj_create(parent, NULL);
	lv_obj_set_size(page_base, lv_obj_get_width(parent), lv_obj_get_height(parent));
	lv_obj_set_style_local_radius(page_base, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	return page_base;
}

lv_obj_t* page_create_msg(lv_obj_t* parent , char * msg)
{

	lv_obj_t* page = lv_page_create(parent, NULL);
	lv_obj_set_style_local_radius(page, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_color(page, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_obj_set_size(page, lv_obj_get_width(parent), lv_obj_get_height(parent) - page_get_title_btn_height());
	lv_obj_align(page, parent, LV_ALIGN_IN_LEFT_MID, 0, page_get_title_height()+6);	
	lv_page_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_AUTO);
	lv_obj_add_style(page, LV_PAGE_PART_SCROLLBAR, page_style_get_page_sb());
	lv_obj_add_style(page, LV_PAGE_PART_EDGE_FLASH, page_style_get_page_edge());

	lv_obj_t * lab = lv_label_create(page, NULL);
	lv_obj_set_style_local_radius(lab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_text_color(lab, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_label_set_long_mode(lab, LV_LABEL_LONG_BREAK);
	lv_label_set_recolor(lab, true);
	APP_TRACE("msg = %s\r\n", msg);
	lv_label_set_text(lab, msg);
    lv_obj_align(lab, page, LV_ALIGN_IN_LEFT_MID, 0, 0);
	lv_label_set_align(lab, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_width(lab, lv_page_get_width_fit(page));
	lv_obj_set_user_data(lab, (lv_obj_user_data_t) page );
	lv_obj_t *scroll = lv_page_get_scrollable(page);	
	lv_obj_set_y(scroll, 0);
	return lab;
}

lv_obj_t* page_create_msg_mid(lv_obj_t* parent , char * msg)
{
	lv_obj_t* page = lv_page_create(parent, NULL);
	lv_obj_set_style_local_radius(page, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_color(page, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_obj_set_size(page, lv_obj_get_width(parent), lv_obj_get_height(parent) - page_get_title_btn_height());
	lv_obj_align(page, parent, LV_ALIGN_CENTER, 0, page_get_title_height()+6);	

	lv_obj_t * lab = lv_label_create(page, NULL);
	lv_obj_set_style_local_radius(lab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_text_color(lab, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_label_set_long_mode(lab, LV_LABEL_LONG_BREAK);
	//lv_label_set_recolor(lab, true);
	APP_TRACE("msg = %s\r\n", msg);
	lv_label_set_text(lab, msg);
	lv_label_set_align(lab, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_size(lab, lv_obj_get_width(parent), lv_obj_get_height(parent));
	lv_obj_align(lab, page, LV_ALIGN_CENTER, 0, -page_get_title_height());
	
	return lab;
}

lv_obj_t* page_create_msg_ex(lv_obj_t* parent , char * msg, lv_font_t* font)
{
	lv_obj_t* page = lv_page_create(parent, NULL);
	lv_obj_set_style_local_radius(page, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_color(page, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_obj_set_size(page, lv_obj_get_width(parent), lv_obj_get_height(parent) - page_get_title_btn_height());
	lv_obj_align(page, parent, LV_ALIGN_IN_LEFT_MID, 0, page_get_title_height()+6);
	
	lv_page_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_AUTO);
	lv_obj_add_style(page, LV_PAGE_PART_SCROLLBAR, page_style_get_page_sb());
	lv_obj_add_style(page, LV_PAGE_PART_EDGE_FLASH, page_style_get_page_edge());

	lv_obj_t * lab = lv_label_create(page, NULL);
	lv_obj_set_style_local_radius(lab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_text_color(lab, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_label_set_long_mode(lab, LV_LABEL_LONG_BREAK);
	lv_label_set_recolor(lab, true);
	
	APP_TRACE("msg = %s\r\n", msg);
	lv_label_set_text(lab, msg);
	lv_obj_set_style_local_text_font(lab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, font);
	//lv_obj_align(lab, page, LV_ALIGN_IN_LEFT_MID, 0, 0);
	lv_label_set_align(lab, LV_LABEL_ALIGN_LEFT);

	lv_obj_set_width(lab, lv_page_get_width_fit(page));
	lv_obj_set_user_data(lab, (lv_obj_user_data_t) page );

	return lab;
}
lv_obj_t* page_create_msg_align(lv_obj_t* parent , char * msg , lv_font_t* font, lv_label_align_t align)
{

	lv_obj_t* page = lv_page_create(parent, NULL);
	lv_obj_set_style_local_radius(page, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_color(page, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_obj_set_size(page, lv_obj_get_width(parent), lv_obj_get_height(parent) - page_get_title_btn_height());
	lv_obj_align(page, parent, LV_ALIGN_IN_LEFT_MID, 0, page_get_title_height()+6);	
	lv_page_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_AUTO);
	lv_obj_add_style(page, LV_PAGE_PART_SCROLLBAR, page_style_get_page_sb());
	lv_obj_add_style(page, LV_PAGE_PART_EDGE_FLASH, page_style_get_page_edge());

	lv_obj_t * lab = lv_label_create(page, NULL);
	lv_obj_set_style_local_radius(lab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_text_color(lab, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_label_set_long_mode(lab, LV_LABEL_LONG_BREAK);
	lv_label_set_recolor(lab, true);
	APP_TRACE("msg = %s\r\n", msg);
	lv_label_set_text(lab, msg);
	lv_obj_set_style_local_text_font(lab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, font);
    lv_obj_align(lab, page, LV_ALIGN_IN_LEFT_MID, 0, 0);
	lv_label_set_align(lab, align);
	lv_obj_set_width(lab, lv_page_get_width_fit(page));
	lv_obj_set_user_data(lab, (lv_obj_user_data_t) page );
	lv_obj_t *scroll = lv_page_get_scrollable(page);	
	lv_obj_set_y(scroll, 0);
	return lab;
}


lv_obj_t* page_create_win(lv_obj_t* parent, lv_event_cb_t _event_cb)
{
	lv_obj_t* win = lv_obj_create(parent, NULL);
	lv_obj_set_style_local_radius(win, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_color(win, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_obj_set_size(win, lv_obj_get_width(parent), lv_obj_get_height(parent));
	page_group_set_obj(win);
	if(_event_cb != 0) lv_obj_set_event_cb(win, _event_cb);
	return win;
}

lv_obj_t* page_create_navigation_back(lv_obj_t* parent, lv_event_cb_t _event_cb)
{
	lv_load_png_file(IMG_NAVIGATION_BACK);
    lv_obj_t* left_imgbtn = lv_img_create(parent, NULL);
    lv_img_set_src(left_imgbtn, IMG_NAVIGATION_BACK);
    lv_obj_set_click(left_imgbtn, true);
	lv_obj_set_user_data(left_imgbtn, (lv_obj_user_data_t)parent);
    lv_obj_set_event_cb(left_imgbtn, _event_cb);
    lv_obj_align(left_imgbtn, NULL, LV_ALIGN_IN_TOP_LEFT, 16, 6);
	return left_imgbtn;
}

static void app_apply_cb(lv_theme_t* th, lv_obj_t* obj, lv_theme_style_t name)
{
	lv_style_list_t* list;
	switch (name) 
	{
		case LV_THEME_LIST:
		list = lv_obj_get_style_list(obj, LV_LIST_PART_BG);
		_lv_style_list_add_style(list, page_style_get_list());
		break;
		default: break;
	}
}

static lv_theme_t custom_theme = {0};

void page_theme_init()
{
	/*Get the current theme (e.g. material). It will be the base of the custom theme.*/
	lv_theme_t* base_theme = lv_theme_get_act();
	/*Initialize a custom theme*/
	                      /*Declare a theme*/
	lv_theme_copy(&custom_theme, base_theme);               /*Initialize the custom theme from the base theme*/
	lv_theme_set_apply_cb(&custom_theme, app_apply_cb);  /*Set a custom theme apply callback*/
	lv_theme_set_base(&custom_theme, base_theme);            /*Set the base theme of the csutom theme*/
	
	lv_theme_set_act(&custom_theme);
}

void page_init()
{
	page_theme_init();
	page_group_create();
}

int page_get_btn_height()
{
	return 40;
}
int page_get_statusbar_height()
{
	return 30;
}

int page_get_title_height()
{
	return 40;
}

int page_get_title_btn_height()
{
	return /*page_get_btn_height() +*/ page_get_title_height();
}



static lv_task_t* task_arc = NULL;
static lv_obj_t* page_spinner = NULL;
static int16_t l_angle = 0;
static void arc_animation_task(lv_task_t* task)
{

	/* Increase the angle */
	l_angle += 10;
	if (l_angle > 360)
		l_angle -= 360;
	/* Set the new angle to the arc */
	lv_obj_t* arc = task->user_data;
	lv_arc_set_start_angle(arc, l_angle);
	lv_arc_set_end_angle(arc, l_angle + 45);
}

lv_obj_t* lv_ex_arc_2(lv_obj_t* parent)
{
	/* Create an arc */
	l_angle = 180;
	lv_obj_t* arc1 = lv_arc_create(parent, NULL);
	lv_arc_set_bg_start_angle(arc1, 0);
	lv_arc_set_bg_end_angle(arc1, 360);
	lv_arc_set_start_angle(arc1, l_angle + 0);
	lv_arc_set_end_angle(arc1, l_angle + 45);
	lv_arc_set_rotation(arc1, 90);
	//lv_arc_set_bg_color(arc1, lv_color_hex(0xAAAAAA));
	//lv_arc_set_color(arc1, lv_color_hex(0x3B86FF));
	//lv_arc_set_width(arc1, 15);
	lv_obj_align(arc1, NULL, LV_ALIGN_CENTER, 0, 0);
	/* Create a task to animate the arc */
	task_arc = lv_task_create(arc_animation_task, 50, LV_TASK_PRIO_MID, arc1);
	return arc1;
}
void _spinner_close_page()
{
	if (task_arc)
	{
		lv_task_del(task_arc);
		task_arc = 0;
	}
	if (page_spinner)
	{
		lv_obj_del(page_spinner);
		page_spinner = 0;
	}
}

lv_obj_t* create_spinner(lv_obj_t* parent, char* title, char* message) 
{
	page_spinner = page_create_win(parent, NULL);
	if (title != NULL) {
		page_create_title(page_spinner, title);
	}
	//lv_obj_t* lab_message = page_create_msg(page_spinner, message);
	page_ShowTextOut(page_spinner, message, LV_ALIGN_CENTER, 0, 90, LV_COLOR_BLACK, LV_FONT_24);
	lv_ex_arc_2(page_spinner);
	return page_spinner;
}



