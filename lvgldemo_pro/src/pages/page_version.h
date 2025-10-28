#pragma once
#include "lvgl/lvgl.h"
#include "../tracedef.h"

void AppShowVersion(char *title);
void ShowSetting(char *title);
int AppShowVersion_func(int ret, lv_obj_t* obj);
int ShowSetting_func(int ret, lv_obj_t* obj);

