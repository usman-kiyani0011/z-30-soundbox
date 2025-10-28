#pragma once
#include "lvgl/lvgl.h"
#include "../tracedef.h"

void state_wifi_page_init(void* state, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs);
void state_atc_page_init(void* state, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs);
void state_power_page_init(lv_obj_t* state, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs);
void state_signal_page_init(void* state, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs);

