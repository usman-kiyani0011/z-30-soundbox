/**
 * @file page_scan.h
 * @author CHAR
 * @brief 
 * @date 2023-12-2
 * @copyright Fujian MoreFun Electronic Technology Co., Ltd.
 */
#ifndef __PAGE_SCAN_H__
#define __PAGE_SCAN_H__
#include "lvgl/lvgl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
typedef struct
{
	int maxLenth;
	char *scanRes;
}scancode_param_t;

LIB_EXPORT int ScanPage(lv_obj_t* parent, void* pfunc, char*amount, char *scancode, int maxlenth);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __PAGE_SCAN_H__ */
