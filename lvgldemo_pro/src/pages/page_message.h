/**
 * @file page_message.h
 * @author CHAR
 * @brief 
 * @date 2023-12-2
 * @copyright Fujian MoreFun Electronic Technology Co., Ltd.
 */
#ifndef __PAGE_MESSAGE_H__
#define __PAGE_MESSAGE_H__
#include "pages.h"
#include "lvgl/lvgl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

LIB_EXPORT void free_message_page();
LIB_EXPORT lv_obj_t* page_message_img_show(lv_obj_t* parent , char *path, void * pfunc , char * title, char * message ,char * leftbtn, char *rightbtn, int timeover);
LIB_EXPORT void page_message_set_timer_func(void* pfunc);
LIB_EXPORT lv_obj_t* page_message_show(lv_obj_t* parent , void * pfunc , char * title, char * message ,char * leftbtn, char *rightbtn, int timeover);
LIB_EXPORT lv_obj_t* page_message_show_ex(lv_obj_t* parent, void* pfunc, char* title, char* message, char* leftbtn, char* rightbtn, int timeover,int show_back);
LIB_EXPORT void set_tts_play_func(tts_play_func pfunc, unsigned char *payload, unsigned int payloadlen);
LIB_EXPORT void wifi_message_close_page();
LIB_EXPORT lv_obj_t* page_message_show_align(lv_obj_t* parent, void* pfunc, char* title, char* message, char* leftbtn, char* rightbtn, int timeover,int align);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __PAGE_MESSAGE_H__ */
