#pragma once
#include "lvgl/lvgl.h"
#include "pub/message.h"
#include "pub/taskdef.h"
#define RFCARD	1
//#define DEMO_OFFLINE	1//show txn offline

#define XM_MESSAGEARRIVED  XM_USERFIRST + 1
#define TASK_PRIO (_APP_TASK_MIN_PRIO + 1)

int get_keepAliveInterval();
int get_messageflag();
int get_mqtt_con_state();
char* get_OrderSn();
int get_setting_int(const char* key);
void get_setting_str(const char* key, char* val, int len);
void mqtt_play_state(int nstate);
int mqtt_publish(char* payload);
void on_user_update(const char *payload, int len);
void orderCnt_lcd_init();
lv_obj_t* page_text_show(lv_obj_t* parent, char* title, char* text, int timeout);
int player_proc_init();
int play_proc_default_msg(MESSAGE * pmsg);
int play_proc_with_fifo();
void set_counter_led();
int set_keepAliveInterval(int keepAliveInterval);
void set_messageflag(int value);
void set_setting_int(const char *key, int val);
void set_setting_str(const char* key, const char* val);
void TransactionProc(lv_obj_t* obj, long long llamont, int op);
void TransactionProc(lv_obj_t* obj, long long llamont, int op);
void _TransactionProcCallbackFunction(int ret, char* recvdata);

