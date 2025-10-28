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
#define RFPNG "P:exdata/rf.png"
#define LOWPOWERPNG "P:exdata/lowpower.png"
#define SHUTDOWNRPNG "P:exdata/shutdown.png"

#define MAINPNG_320X480 "P:exdata/mainb.png"
#define LOGOPNG_320X480 "P:exdata/logob.png"
#define SUCCPNG_320X480 "P:exdata/succb.png"
#define LOWPOWERPNG_320X480 "P:exdata/lowpoweb.png"
#define SHUTDOWNRPNG_320X480 "P:exdata/shutdowb.png"

#define LCD_IS_320_480	(MFSDK_SYS_LCD_TYPE_320_480 == MfSdkSysGetLcdType())

int network_page_get_key();
int page_get_key(int flag);
void page_group_create();
void page_group_set_obj(lv_obj_t* obj);
void playMbtn();
void reset_tick_get_btn_play_time();

