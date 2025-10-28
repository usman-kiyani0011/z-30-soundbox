/**
 * @file page_rfcard.h
 * @author CHAR
 * @brief 
 * @date 2023-12-2
 * @copyright Fujian MoreFun Electronic Technology Co., Ltd.
 */
#ifndef __PAGE_RFCARD_H__
#define __PAGE_RFCARD_H__
#include "lvgl/lvgl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

LIB_EXPORT void _card_task_func(lv_task_t* task);
LIB_EXPORT void free_readcard_page();
LIB_EXPORT lv_obj_t* page_card_show(lv_obj_t* parent , void * pfunc , char * title, void * trackinfo, int timeover,int show_back);
LIB_EXPORT lv_obj_t* page_card_showamt(lv_obj_t* parent, void* pfunc, char* title, void* trackinfo,char* amtstr, int timeover, int show_back);
LIB_EXPORT lv_obj_t* page_read_card_show(lv_obj_t* parent, char* title, char* text, int timeout);
LIB_EXPORT void emvreadcardtiprelease(void);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __PAGE_RFCARD_H__ */
