#ifndef __LIST_VIEW_H__
#define __LIST_VIEW_H__
#include "lvgl/lvgl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef struct _detail_list
{
    char id[40 + 1];
    double amount;
    char txnId[40 + 1];
    char date[32 + 1];
}DetailList_t;

typedef void (*ListViewDestroyCb)(int ret);

void ListViewCreate(lv_obj_t *par, char *title, DetailList_t *pDetList, int size, ListViewDestroyCb destroy, int timeover);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __LIST_VIEW_H__ */
