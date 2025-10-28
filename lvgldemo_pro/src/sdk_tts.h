/**
 * @file sdk_tts.h
 * @author CHAR
 * @brief 
 * @date 2023-11-20
 * @copyright Fujian MoreFun Electronic Technology Co., Ltd.
 */
#ifndef __SDK_TTS_H__
#define __SDK_TTS_H__
#include "libapi_xpos/inc/mfsdk_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

LIB_EXPORT void sdk_tts_play_amt_en(int amt);
LIB_EXPORT void pub_tts_play_amt_india(int amt);
LIB_EXPORT  void sdk_tts_play_num(int num);
void pub_tts_play_amt_en(int amt);
int LanguageInit();
void SetLanguage(int lang);
int GetLanguage();
int PubMultiPlay(const s8 * sndfile);

void AppPlayBatteryLevel(void);
void AppPlayBatteryIndonesia(void);
void pub_tts_play_amt_idr(int amt);
void pub_tts_play_amt(int amt);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SDK_TTS_H__ */
