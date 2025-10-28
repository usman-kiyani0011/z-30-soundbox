#include "lvgl/lvgl.h"
#include "pages.h"


#define STYLE_STATE_ALL	  (LV_STATE_DEFAULT | LV_STATE_FOCUSED | LV_STATE_EDITED | LV_STATE_HOVERED | LV_STATE_PRESSED  )

static lv_style_t style_page_sb = {0};
static lv_style_t style_page_edge = {0};
static lv_style_t style_title = { 0 };
static lv_style_t style_msg = { 0 };
static lv_style_t style_button = { 0 };
static lv_style_t style_list = { 0 };



lv_style_t* page_style_get_list()
{
	lv_style_t* style = &style_list;
	if (style->map == 0) {
		lv_style_init(style);
		lv_style_set_border_side(style, LV_STATE_DEFAULT, LV_BORDER_SIDE_NONE);
	}
	return style;
}


lv_style_t* page_style_get_button()
{
	lv_style_t* style = &style_button;
	if (style->map == 0) {
		lv_style_init(style);
		//lv_style_set_bg_color(style, LV_STATE_DEFAULT, LV_COLOR_RED);
		//lv_style_set_bg_opa(style, LV_STATE_DEFAULT, LV_OPA_30);
	}
	return style;
}


lv_style_t* page_style_get_page_edge()
{
	lv_style_t* style = &style_page_edge;
	if (style->map == 0) {
		lv_style_init(style);
		//边缘半圆弧样式
		lv_style_set_bg_color(style, LV_STATE_DEFAULT, LV_COLOR_RED);
		lv_style_set_bg_opa(style, LV_STATE_DEFAULT, LV_OPA_30);
		
			
			
	}
	return style;
}

lv_style_t* page_style_get_page_sb()
{
	lv_style_t* style = &style_page_sb;
	if (style->map == 0) {
		lv_style_init(style);
		lv_style_set_bg_color(style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		lv_style_set_radius(style, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
		lv_style_set_bg_opa(style, LV_STATE_DEFAULT, LV_OPA_50);
		lv_style_set_pad_right(style, LV_STATE_DEFAULT, 3);//垂直滚动条边距
		lv_style_set_pad_bottom(style, LV_STATE_DEFAULT, 3);//水平滚动条边距
	}
	return style;
}

lv_style_t* page_style_get_title()
{
	lv_style_t* style = &style_title;
	if (style->map == 0) {
		lv_style_init(style);
		#ifdef WIN32
		lv_style_set_text_font(style, LV_STATE_DEFAULT, lv_get_font("lv_font_montserrat_24"));
		#else
		lv_style_set_text_font(style, LV_STATE_DEFAULT, lv_get_font("lv_font_montserrat_32"));
		#endif
		lv_style_set_text_color(style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
#if 0
		lv_style_set_bg_opa(style, LV_STATE_DEFAULT, LV_OPA_COVER);
		lv_style_set_bg_color(style, LV_STATE_DEFAULT, LV_COLOR_SILVER);
		//lv_style_set_bg_grad_color(style, LV_STATE_DEFAULT, LV_COLOR_BLUE);
		lv_style_set_bg_grad_dir(style, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
		//lv_style_set_pad_top(style, LV_STATE_DEFAULT, 20);
		//lv_style_set_value_ofs_x(style, LV_STATE_DEFAULT, 30);
		//lv_style_set_text_letter_space(style, LV_STATE_DEFAULT, 5);
		//lv_style_set_text_line_space(tyle, LV_STATE_DEFAULT, 20);
#endif

		lv_style_set_pad_top(style, LV_STATE_DEFAULT, 8);
		//lv_style_set_pad_bottom(style, LV_STATE_DEFAULT, 10);
	}
	return style;
}

lv_style_t* page_style_get_msg()
{
	lv_style_t* style = &style_msg;
	if (style->map == 0) {
		lv_style_init(style);
		//lv_style_set_text_font(style, LV_STATE_DEFAULT, lv_get_font("lv_font_montserrat_24"));
		//lv_style_set_text_color(style, LV_STATE_DEFAULT, LV_COLOR_BLUE);
		//lv_style_set_bg_opa(style, LV_STATE_DEFAULT, LV_OPA_COVER);
		//lv_style_set_bg_color(style, LV_STATE_DEFAULT, LV_COLOR_SILVER);
		//lv_style_set_bg_grad_color(style, LV_STATE_DEFAULT, LV_COLOR_BLUE);
		//lv_style_set_bg_grad_dir(style, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
	}
	return style;
}



