/**
 * @file page_input.h
 * @author CHAR
 * @brief 
 * @date 2023-12-1
 * @copyright Fujian MoreFun Electronic Technology Co., Ltd.
 */
#ifndef __PAGE_INPUT_H__
#define __PAGE_INPUT_H__
#include <lvgl/lvgl.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

LIB_EXPORT int GetKeyStatus(int key);
LIB_EXPORT int GetKeyValue(int key);
LIB_EXPORT lv_obj_t* page_input_show(lv_obj_t* parent , void* pfunc, char * title, char * buff, int maxnum, int mode, int timeover);
LIB_EXPORT lv_obj_t* page_input_show_ex(lv_obj_t* parent , void* pfunc, char * title, char * buff, int minnum, int maxnum, int mode, int timeover,int show_back);
LIB_EXPORT lv_obj_t* page_input_show_fix(lv_obj_t* parent , void* pfunc, char * title, char * buff, int lenth, int mode, int timeover, int show_back);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __PAGE_INPUT_H__ */
