/**
 * @file page_style.h
 * @author CHAR
 * @brief 
 * @date 2023-12-1
 * @copyright Fujian MoreFun Electronic Technology Co., Ltd.
 */
#ifndef __PAGE_STYLE_H__
#define __PAGE_STYLE_H__
#include "lvgl/lvgl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

LIB_EXPORT lv_style_t* page_style_get_button();
LIB_EXPORT lv_style_t* page_style_get_list();
LIB_EXPORT lv_style_t* page_style_get_msg();
LIB_EXPORT lv_style_t* page_style_get_page_edge();
LIB_EXPORT lv_style_t* page_style_get_page_sb();
LIB_EXPORT lv_style_t* page_style_get_title();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __PAGE_STYLE_H__ */
