#pragma once
//#include "xdk_et.h"
#include <stdarg.h>
#include "lvgl/lvgl.h"
#include "lvgl/src/lv_lib_png/lv_png.h"
#include "libapi_xpos/inc/mfsdk_log.h"
#include "libapi_xpos/inc/mfsdk_qr.h"
#include "libapi_xpos/inc/mfsdk_lcd.h"


#define TRACE(...) 			MfSdkLogLevel("app",MFSDK_LOG_LEVEL_TRACE, __VA_ARGS__);

#define APP_TRACE(...) 		MfSdkLogLevel("app",MFSDK_LOG_LEVEL_TRACE, __VA_ARGS__);
#define APP_TRACE_FILE(...) MfSdkLogLevel("app",MFSDK_LOG_LEVEL_FILE, __VA_ARGS__);
#define APP_TRACE_BUFF(a,b) MfSdkLogHexBuff("app",MFSDK_LOG_LEVEL_TRACE,a,b);

#define APP_TRACE_BUFF_TIP(a,b,c) 	MfSdkLogTip("app",MFSDK_LOG_LEVEL_TRACE,a,b , c ,1 );

#define APP_TRACE_BUFF_LOG(a,b,c) 	MfSdkLogTip("app",MFSDK_LOG_LEVEL_TRACE,a,b , c ,0);	

#define TIMEBEGIN { int _start_value_ = MfSdkSysGetTick();
#define TIMEEND( k )  MfSdkLogLevel("app",MFSDK_LOG_LEVEL_TRACE, k##" %d\r\n" , MfSdkSysGetTick()-_start_value_ ); }

#define APP_TRACE_FORMAT(fmt,...)		\
{MfSdkLogLevel("app",MFSDK_LOG_LEVEL_TRACE, "[APP][%s][%d] ", __FUNCTION__, __LINE__); \
MfSdkLogLevel("app",MFSDK_LOG_LEVEL_TRACE, __VA_ARGS__);}




