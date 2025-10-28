/**
 * @file page_comm.h
 * @author CHAR
 * @brief 
 * @date 2023-12-1
 * @copyright Fujian MoreFun Electronic Technology Co., Ltd.
 */
#ifndef __PAGE_COMM_H__
#define __PAGE_COMM_H__
#include "pages.h"
#include "lvgl/lvgl.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define	COMM_PRECONNECT		(0)
void comm_disconnect(void);
int comm_preconnect();
int GetCommStatus(void);
LIB_EXPORT lv_obj_t* page_comm_show_auto(lv_obj_t* parent, void* pfunc, char* title, st_comm_data* comm_data,int mode);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __PAGE_COMM_H__ */
