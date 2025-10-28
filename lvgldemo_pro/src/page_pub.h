#pragma once
#include "lvgl/lvgl.h"
#include "libapi_xpos/inc/mfsdk_sys.h"
#include "libapi_xpos/inc/mfsdk_lcd.h"
#include "libapi_xpos/inc/mfsdk_aud.h"
#include "libapi_xpos/inc/mfsdk_comm.h"
#include "libapi_xpos/inc/mfsdk_power.h"


#define MAINPNG "P:exdata/main.png"
#define LOGOPNG "P:exdata/logo.png"
#define SUCCPNG "P:exdata/succ.png"
#define FAILPNG "P:exdata/fail.png"
#define RFPNG "P:exdata/rf.png"
#define RFPNG2 "P:exdata/rf2.png"
#define RFPNG3 "P:exdata/rf3.png"
#define LOWPOWERPNG "P:exdata/lowpower.png"
#define SHUTDOWNRPNG "P:exdata/shutdown.png"
#define LOADPNG "P:exdata/load.png"
#define QRLOGO "P:exdata/qrlogo.png" 

#define MAINPNG_320X480 "P:exdata/mainb.png"
#define LOGOPNG_320X480 "P:exdata/logob.png"
#define SUCCPNG_320X480 "P:exdata/succb.png"
#define FAILPNG_320X480 "P:exdata/failb.png"
#define LOWPOWERPNG_320X480 "P:exdata/lowpoweb.png"
#define SHUTDOWNRPNG_320X480 "P:exdata/shutdowb.png"

#define LCD_IS_320_480	(MFSDK_SYS_LCD_TYPE_320_480 == MfSdkSysGetLcdType())

#define TTS_VOLUME_MIN			"volmin.mp3"		
#define TTS_VOLUME_MAX			"volmax.mp3"		
#define TTS_VOLUME_NOR			"volnor.mp3"
#define TTS_BUTTON_M	        "buttonM"		// 

typedef int (*pFuncPagePubCallback)(int ret, lv_obj_t* obj);

lv_obj_t* page_ShowTextOut(lv_obj_t* parent, char* str, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, lv_color_t color, lv_font_t* font);
lv_obj_t*  page_ShowText_Font(lv_obj_t * parent, char *str, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, lv_color_t color, lv_font_t* font);
lv_obj_t*  page_ShowText_MultiFont(lv_obj_t * parent, int index, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, lv_color_t color);
int pub_page_get_key();
int page_get_key();
void page_netmenu_key_proc(int key);
void page_group_create();
void page_group_set_obj(lv_obj_t* obj);
void playMbtn();
void reset_tick_get_btn_play_time();
lv_obj_t* page_text_show(lv_obj_t* parent, char* title, char* text, int timeout);
lv_obj_t* page_text_show_mid(lv_obj_t* parent, char* title, char* text, int timeout);
lv_obj_t* page_text_show_callback(lv_obj_t* parent, char* title, char* text, int timeout,pFuncPagePubCallback pFuncCallback);


