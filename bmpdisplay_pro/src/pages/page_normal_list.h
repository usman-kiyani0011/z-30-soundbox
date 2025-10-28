/**
 * @file page_normal_list.h
 * @author CHAR
 * @brief 
 * @date 2023-12-2
 * @copyright Fujian MoreFun Electronic Technology Co., Ltd.
 */
#ifndef __PAGE_NORMAL_LIST_H__
#define __PAGE_NORMAL_LIST_H__

#include "lvgl/lvgl.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

LIB_EXPORT int get_refresh_wifi_list_flag();
LIB_EXPORT void page_list_delete(lv_obj_t*list);
LIB_EXPORT lv_obj_t *page_list_focus_btn_index(int index);
LIB_EXPORT lv_obj_t* page_list_get_lv_list(lv_obj_t* page_win);
LIB_EXPORT void page_list_reset_seleted_index();
LIB_EXPORT int page_list_seleted_index();
LIB_EXPORT lv_obj_t* page_list_show(lv_obj_t* parent, void* pfunc, char* title, char* listitem[], int count, int index, int timeover);
LIB_EXPORT lv_obj_t* page_list_show_ex(lv_obj_t* parent, void* pfunc, char* title, char* listitem[], int count, int index, int timeover, int show_back);
LIB_EXPORT void set_refresh_wifi_list_flag(int refresh_flag);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __PAGE_NORMAL_LIST_H__ */
