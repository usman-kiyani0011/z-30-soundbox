#pragma once
#include "lvgl/lvgl.h"
lv_obj_t* wifilist_img;
int func_wifi_info(lv_obj_t* parent);
int func_wifi_show(lv_obj_t* parent);
void func_wifi_show_ex();
lv_obj_t* get_wifilist_img();
lv_obj_t* set_wifilist_img(lv_obj_t* wifilist_img_in);
