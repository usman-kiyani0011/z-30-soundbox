/**
 * @file page_http.h
 * @author CHAR
 * @brief 
 * @date 2023-12-1
 * @copyright Fujian MoreFun Electronic Technology Co., Ltd.
 */
#ifndef __PAGE_HTTP_H__
#define __PAGE_HTTP_H__
#include "lvgl/lvgl.h"
#include "pages.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

LIB_EXPORT void free_http_message_page();
LIB_EXPORT int get_http_page_end_flag();
LIB_EXPORT void page_http_message_set_timer_func(void* pfunc);
LIB_EXPORT lv_obj_t* page_http_message_show(lv_obj_t* parent , void * pfunc , char * title, char * message ,char * leftbtn, char *rightbtn, int timeover);
LIB_EXPORT lv_obj_t* page_http_message_show_ex(lv_obj_t* parent, void* pfunc, char* title, char* message, char* leftbtn, char* rightbtn, int timeover,int show_back);
LIB_EXPORT void set_http_tts_play_func(tts_play_func pfunc, unsigned char *payload, unsigned int payloadlen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __PAGE_HTTP_H__ */
