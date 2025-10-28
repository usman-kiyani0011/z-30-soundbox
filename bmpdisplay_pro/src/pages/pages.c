#include "lvgl/lvgl.h"
#include "pages.h"
#include "../page_pub.h"

lv_obj_t* page_create_title(lv_obj_t *parent, char* title)
{
	lv_obj_t* lab_title = lv_label_create(parent, NULL);
	lv_label_set_long_mode(lab_title, LV_LABEL_LONG_CROP);
	lv_obj_set_width(lab_title, lv_obj_get_width(parent));
	lv_obj_set_height(lab_title, page_get_title_height());
	lv_label_set_align(lab_title, LV_LABEL_ALIGN_CENTER);
	lv_label_set_text(lab_title, title);
	lv_obj_align(lab_title, parent, LV_ALIGN_IN_TOP_LEFT, 0, 30);	
	lv_obj_add_style(lab_title, LV_LABEL_PART_MAIN, page_style_get_title());

	lv_obj_set_style_local_radius(lab_title, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	#ifndef WIN32
	lv_obj_set_style_local_bg_opa(lab_title, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
	#endif
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
	
	if (align == PAGE_BTN_ALIGN_LEFT) {
		in_align = LV_ALIGN_IN_BOTTOM_LEFT;
	}
	else {
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
	lv_obj_set_size(page, lv_obj_get_width(parent), lv_obj_get_height(parent) - page_get_title_btn_height());
	lv_obj_align(page, parent, LV_ALIGN_IN_LEFT_MID, 0, page_get_title_height());	
	lv_page_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_AUTO);
	lv_obj_add_style(page, LV_PAGE_PART_SCROLLBAR, page_style_get_page_sb());
	lv_obj_add_style(page, LV_PAGE_PART_EDGE_FLASH, page_style_get_page_edge());

	lv_obj_t * lab = lv_label_create(page, NULL);
	lv_obj_set_style_local_radius(lab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_label_set_long_mode(lab, LV_LABEL_LONG_BREAK);
	lv_label_set_recolor(lab, true);
	lv_label_set_text(lab, msg);
	lv_label_set_align(lab, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_width(lab, lv_page_get_width_fit(page) - 20);
	lv_obj_set_user_data(lab, (lv_obj_user_data_t) page );

	return lab;
}

lv_obj_t* page_create_win(lv_obj_t* parent, lv_event_cb_t _event_cb)
{
	lv_obj_t* win = lv_obj_create(parent, NULL);
	lv_obj_set_style_local_radius(win, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
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
	switch (name) {
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

int page_get_title_height()
{
	return 50;
}

int page_get_title_btn_height()
{
	return /*page_get_btn_height() +*/ page_get_title_height();
}

