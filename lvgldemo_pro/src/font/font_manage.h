#ifndef __FONT_MANAGE_H__
#define __FONT_MANAGE_H__

#include "lvgl/lvgl.h"

//typedef struct _st_disp_message_def
//{
//
//}st_disp_message_def;

enum DSIPMESSAGE
{
	DSIP_MSG_WELCOME = 0,
	DSIP_MSG_AMOUNT,
	DSIP_MSG_QRCODE,
	DSIP_MSG_MAX,
};

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

lv_font_t* GetMultiFont();
char* GetDispMessage(int index);
char* GetDispMessageFromCfg(int index);
int MultiFontInit();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __FONT_MANAGE_H__ */

