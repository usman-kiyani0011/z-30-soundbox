#pragma once
#include "lvgl/lvgl.h"
#include "func_wifi.h"
#define FONT_HZ_16 "lv_font_simfang_16"
#define FONT_HZ_24	"lv_font_simsun_24"
#define UART_EN		(0)//1: open uart 0: close uart


void MainPage();
lv_obj_t* get_imgpage();
lv_obj_t* get_mainpage();
void ClearAmountTimer();
void show_page_result(int ret,lv_obj_t* obj);
lv_obj_t* get_mainpage();


