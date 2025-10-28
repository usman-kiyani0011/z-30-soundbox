#pragma once
#include "lvgl/lvgl.h"
typedef struct __st_card_info {
	char title[32];
	char pan[32];
	char amt[32];
	char expdate[32];
}st_card_info;

void func_pay(lv_obj_t * parent, char* amt);
void func_pay2(lv_obj_t * parent, char* amt);
void free_rfid_page();
char* get_amount();
void rf_play_amt(long long money);
