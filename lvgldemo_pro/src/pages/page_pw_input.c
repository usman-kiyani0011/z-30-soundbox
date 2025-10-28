#include <stdio.h>
#include "lvgl/lvgl.h"
#include "tracedef.h"
#include "pages.h"
//#include "comm_pages.h"

static lv_obj_t* page_win = NULL;
static lv_obj_t* textarea = NULL;
static lv_obj_t* white_bkg = NULL;
static lv_obj_t* time_count = NULL;

static char* m_title;
static char* m_buff;
static int m_maxnum;
//static int m_mode;
static int m_timeover;
static int m_timeovr_reset;
static int max_timeover;
static lv_task_t* task_input = NULL;
static lv_task_t* task_timeover = NULL;
static page_close_page_func m_page_close_page_func = 0;
static const char* keybuff[11] = {
	"0 ","1","2abcABC","3defDEF","4ghiGHI",
	"5jklJKL","6mnoMNO","7pqrsPQRS","8tuvTUV","9wxyzWXYZ",
	".,'?!\"-*@/\\:_;+&%#$=()<>"
};


static void _input_close_page(int ret)
{
	if (page_win != 0) {
		lv_obj_del(page_win);
		page_win = 0;
		if (task_input) {
			lv_task_del(task_input);
			task_input = 0;
		}
		if (task_timeover) {
			lv_task_del(task_timeover);
			task_timeover = 0;
		}
		if (m_page_close_page_func != 0) m_page_close_page_func(ret, NULL);
	}

}
static char _input_newchar(char oldchar)
{
	int i, j;
	int size;
	char ch = oldchar;
	for (i = 0; i < 11; i++) {
		size = strlen(keybuff[i]);
		for (j = 0; j < size; j++) {
			if (oldchar == keybuff[i][j]) {
				j = (j + 1) % size;
				ch = keybuff[i][j];
				return ch;
			}
		}
	}
	return oldchar;
}
static void _wifi_input_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t key;
	char a[2] = { 0 };
	char buff[64] = { 0 };
	char old_char, new_char;
	char time_set[32] = { 0 };
	if (e == LV_EVENT_KEY) 
	{
		m_timeover = m_timeovr_reset;
		key = page_get_key();
		lv_task_reset(task_input);
		snprintf(time_set, sizeof(time_set), "(%02ds)", m_timeover / 1000);
		lv_label_set_text(time_count, time_set);
		if (key == MF_LV_KEY_CANCEL_SHORT_PRESS) 
		{
			_input_close_page(PAGE_RET_CANCEL);
		}
		if (key == MF_LV_KEY_OK_SHORT_PRESS)
		{
			strcpy(m_buff, lv_textarea_get_text(textarea));
			_input_close_page(PAGE_RET_CONFIRM);
		}
		if (key >= MF_LV_KEY_1_SHORT_PRESS && key <= MF_LV_KEY_0_TRIPLE_PRESS)
		{
			APP_TRACE("key == %d\r\n", key);
			sprintf(a, "%c", GetKeyValue(key));
			lv_textarea_add_text(textarea, a);
		}
		else if (key == MF_LV_KEY_BACKSPACE_SHORT_PRESS)
		{

			lv_textarea_del_char(textarea);

		}
		else if (key == MF_LV_KEY_F1_SHORT_PRESS)
		{

			lv_textarea_add_text(textarea, ".");

		}
		else if (key == MF_LV_KEY_F2_SHORT_PRESS)
		{
			strcpy(buff, lv_textarea_get_text(textarea));
			old_char = buff[strlen(buff) - 1];
			new_char = _input_newchar(old_char);
			lv_textarea_del_char(textarea);
			sprintf(a, "%c", new_char);
			lv_textarea_add_text(textarea, a);
		}
	}
}
static void back_navigation_event_cb(lv_obj_t* btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		_input_close_page(PAGE_RET_CANCEL);
	}
}
static void _input_task_func(lv_task_t* task)
{
	char time_set[32] = { 0 };
	if (m_timeover > 0) {
		m_timeover -= 1000;

		snprintf(time_set, sizeof(time_set), "(%02ds)", m_timeover / 1000);
		lv_label_set_text(time_count, time_set);

		if (m_timeover <= 0) {
			_input_close_page(PAGE_RET_TIMEOVR);
		}
	}
}
static void _timeover_task_func(lv_task_t* task)
{

	if (max_timeover > 0) {
		max_timeover -= 1000;
		if (max_timeover <= 0) {
			APP_TRACE("[%s] _timeover_task_func end:%d\r\n", __FUNCTION__, max_timeover);
			_input_close_page(PAGE_RET_TIMEOVR);
		}
	}
}
#define INTERVAL		10
static lv_obj_t* mf_create_wifi_input(lv_obj_t* parent, char* msg)
{
	//backgroud
	white_bkg = lv_obj_create(parent, NULL);
	lv_obj_set_style_local_radius(white_bkg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_color(white_bkg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_border_width(white_bkg, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_border_color(white_bkg, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_size(white_bkg, lv_obj_get_width(parent), lv_obj_get_height(parent)/2+30);
	lv_obj_align(white_bkg, 0, LV_ALIGN_CENTER, 0, 0);

	//input
	textarea = lv_textarea_create(white_bkg, NULL);
	lv_obj_set_width(textarea, lv_obj_get_width(white_bkg) - INTERVAL*2);
	lv_obj_set_height(textarea, 40);
	//lv_textarea_set_pwd_mode(textarea, true);
	lv_textarea_set_placeholder_text(textarea, "");
	lv_textarea_set_text_align(textarea, LV_LABEL_ALIGN_LEFT);
	lv_textarea_set_text(textarea, msg);
	lv_textarea_set_max_length(textarea, m_maxnum);
	lv_textarea_set_cursor_hidden(textarea, false);
	lv_obj_set_style_local_text_color(textarea, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_obj_set_style_local_text_font(textarea, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_FONT_24);//changyong_18
	lv_obj_align(textarea, white_bkg, LV_ALIGN_IN_TOP_LEFT, INTERVAL, 100);

	//text:Password
	page_ShowTextOut(white_bkg, "Password:", LV_ALIGN_IN_TOP_LEFT, INTERVAL, 30, LV_COLOR_BLACK, LV_FONT_24);

	//underline
	lv_obj_t* line = lv_obj_create(white_bkg, NULL);
	lv_obj_set_size(line, lv_obj_get_width(parent), 2);
	lv_obj_set_style_local_bg_color(line, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x87CEFA));
	lv_obj_set_style_local_border_color(line, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x87CEFA));
	lv_obj_align(line, white_bkg, LV_ALIGN_IN_TOP_LEFT, INTERVAL, 145);

	return textarea;
}

lv_obj_t* _mf_wifiinput_show(lv_obj_t* parent, void* pfunc, char* title, char* buff, int maxnum,int timeover)
{
	char timeset[32] = { 0 };
	m_title = title;
	m_buff = buff;
	m_maxnum = maxnum;
	m_timeover = timeover;
	m_timeovr_reset = timeover;
	max_timeover = 0;
	m_page_close_page_func = (page_close_page_func)pfunc;
	APP_TRACE("_mf_wifiinput_showr\r\n");
	page_win = page_create_win(parent, _wifi_input_event_cb);
	lv_obj_set_style_local_bg_color(page_win, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(PAY_DEMO_BASE_COLOR));
	mf_create_wifi_input(page_win, buff);

	//·µ»Ø¼ü
	//comm_creat_topleft_backbtn(page_win, "WI-FI", back_navigation_event_cb);
	lv_obj_t* wifiname = lv_label_create(white_bkg, NULL);
	lv_label_set_long_mode(wifiname, LV_LABEL_LONG_DOT);
    lv_obj_set_style_local_text_font(wifiname, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_FONT_24);
	lv_obj_set_style_local_text_color(wifiname, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_label_set_text(wifiname, title);
	lv_obj_set_width(wifiname, lv_obj_get_width(white_bkg)-20);
	lv_obj_align(wifiname, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

	snprintf(timeset, sizeof(timeset), "(%02ds)", timeover / 1000);
	time_count = page_ShowTextOut(white_bkg, timeset, LV_ALIGN_IN_BOTTOM_MID, 0, 0, LV_COLOR_BLACK, LV_FONT_24);

	if(timeover>0)
		task_input = lv_task_create(_input_task_func, 1000, LV_TASK_PRIO_MID, 0);
	APP_TRACE("[%s] _timeover_task_func start:%d\r\n", __FUNCTION__, max_timeover);
	if(max_timeover>0)
		task_timeover = lv_task_create(_timeover_task_func, 1000, LV_TASK_PRIO_HIGHEST, 0);
	return page_win;
}
