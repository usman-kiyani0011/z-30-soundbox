#pragma once

#ifdef __cplusplus
extern "C"{
#endif
#include "pub/pub.h"
#include "lvgl/lvgl.h"

LIB_EXPORT void message_close_show_image(int ret);
LIB_EXPORT lv_obj_t* page_show_image(lv_obj_t * parent, void* pfunc, char *title, int result, char*msg, int timeout);

#ifdef __cplusplus
}
#endif