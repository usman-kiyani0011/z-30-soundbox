#pragma once
#include "lvgl/lvgl.h"
#include "func_wifi.h"
#define FONT_HZ_16 "lv_font_simfang_16"
#define FONT_HZ_24	"lv_font_simsun_24"

#define TTS_VOLUME_MIN			"volmin.mp3"		
#define TTS_VOLUME_MAX			"volmax.mp3"		
#define TTS_VOLUME_NOR			"volnor.mp3"
#define TTS_BUTTON_M	        "buttonM"		// 

void page_main();
lv_obj_t* get_imgpage();
lv_obj_t* get_mainpage();
void show_page_result(int ret,lv_obj_t* obj);
void show_page_qrcode(lv_obj_t* parent, char*amount, char*qrcode);
lv_obj_t* show_cardpage_qrcode(lv_obj_t* parent, void *pfunc, char*amount, char*qrcode, int timeover);


