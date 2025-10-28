#pragma once

#ifdef __cplusplus
extern "C"{
#endif
#include "pub/pub.h"
#include "lvgl/lvgl.h"
#include "pub/qrencode/QrEncode.h"

LIB_EXPORT void close_qrcode_page(int ret);
LIB_EXPORT lv_obj_t* page_show_qrcode(lv_obj_t* parent, void *pfunc, char* title, char* tip, char* data, int timeout);


#ifdef __cplusplus
}
#endif
