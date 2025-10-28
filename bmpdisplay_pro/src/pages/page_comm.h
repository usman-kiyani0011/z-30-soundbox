/**
 * @file page_comm.h
 * @author CHAR
 * @brief 
 * @date 2023-12-1
 * @copyright Fujian MoreFun Electronic Technology Co., Ltd.
 */
#ifndef __PAGE_COMM_H__
#define __PAGE_COMM_H__
#include "lvgl/lvgl.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

LIB_EXPORT void free_comm_page();
LIB_EXPORT int get_page_comm_busyflag();
LIB_EXPORT lv_obj_t* page_comm_show(lv_obj_t* parent , void * pfunc , char * title, st_comm_data* comm_data,int show_back);
LIB_EXPORT lv_obj_t* page_comm_show_auto(lv_obj_t* parent, void* pfunc, char* title, st_comm_data* comm_data,int show_back);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __PAGE_COMM_H__ */
