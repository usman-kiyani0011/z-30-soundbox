/**
 * @file page_list.h
 *
 */
#pragma once

#ifndef PAGE_LIST_H
#define PAGE_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"
/**
 * 创建可以选中列表
 * @param parent : 父视图
 * @param title : 标题
 * @param title_height: 标题 高度
 * @param edge_flash: 边缘半圆弧样式
 * @param img_body:背景图
 * @return : 创建的列表
 */
	
lv_obj_t* page_list_create(lv_obj_t* parent, char* title,int title_height, bool edge_flash, lv_obj_t* img_body);
/**
 * 删除选中列表
 * @param list 指针
 */
void page_list_clean(lv_obj_t * list);

/**
 * 选中状态设置
 * @param 列表指针
 * @param track_id : 索引值 0...N
 * @param state : 是否选中
 */
void page_list_btn_check(const lv_obj_t* list,uint32_t track_id, bool state);
/**
 * 获取选中按钮在列表的索引值
 * @param 列表指针
 * @param 列表选中按钮的指针
 * @return 列表索引值 -1 ：不属于该列表 列表索引:0...N
 */
int32_t page_list_get_btn_index(const lv_obj_t * list, const lv_obj_t * btn);
/**
 * 删除列表的行
 * @param 列表指针
 * @param 要删除列表索引值
 * @return 删除成功、失败
 */
bool page_list_remove(const lv_obj_t* list, uint16_t index);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*PAGE_LIST_H*/
