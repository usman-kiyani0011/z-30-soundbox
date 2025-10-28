#pragma once

#ifdef __cplusplus
extern "C"{
#endif
#include "pub/pub.h"
#include "lvgl/lvgl.h"
#include "pub/qrencode/QrEncode.h"

typedef enum 
{
	QRPAGE_LOCK_DEF = -1,
	QRPAGE_LOCK_ON = 0,
	QRPAGE_LOCK_OFF = 1,
} QRPAGE_LOCK_CTRL;

LIB_EXPORT void close_qrcode_page(int ret);
LIB_EXPORT lv_obj_t* page_show_qrcode(lv_obj_t* parent, void *pfunc, char* title, char* tip, char* data, int timeout);
void show_page_qrcode(lv_obj_t* parent, void *pfunc, char*amount, char*qrcode);
void SetPowerLock (int value);
int GetPowerLock (void);


#ifdef __cplusplus
}
#endif
