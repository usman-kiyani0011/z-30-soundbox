#pragma once
#include "lvgl/lvgl.h"
#include "pub/message.h"
#include "pub/taskdef.h"
#include "sdk_tts.h"
#include "pub/cJSON.h"

//#define TEST_SN         "202302060000002"

#define SINGAPORE_SERVER	(1)//server in Singapore

//#define	AWSMQTT_URL		"a2d911mzqj2e50-ats.iot.ap-south-1.amazonaws.com"

#if SINGAPORE_SERVER
#define MQTT_HOST		"sg-mqtt.funicom.com.cn"
#define MQTT_PORT       32517

#define HTTP_HOST		"sg-iot.morefun-et.com"
#define HTTP_PORT		80
#else
#define MQTT_HOST       "test-mqtt.funicom.com.cn"
#define MQTT_PORT       32517

#define HTTP_HOST		"test-iot.morefun-et.com"
#define HTTP_PORT		80
#endif

#define XM_MESSAGEARRIVED  (XM_USERFIRST + 1)
#define TASK_PRIO (_APP_TASK_MIN_PRIO + 1)

#define MQTT_PLATFORM			"mqtt"
#define COMM_HOST_IP			"hostip"
#define COMM_HOST_PORT			"port"
#define COMM_HTTP_IP			"httpip"
#define COMM_HTTP_PORT			"httpport"
#define PRODUCT_KEY				"productkey"
#define PRODUCT_SECRET			"productsecret"
#define CLIENT_ID				"clientid"
#define USER_NAME				"username"
#define MQTT_TOPIC				"mqtttopic"
#define KEEPALIVE_TIME			"keepalivetime"
#define PLAY_LANGUAGE			"laguage"
#define MQTT_FEEDBACK			"mgttfeedback"

int json_getval(cJSON* rootobj, const char* key, char* outbuff, int size);
int get_keepAliveInterval();
int get_mqtt_con_state();
int get_setting_int(const char* key);
void get_setting_str(const char* key, char* val, int len);
void mqtt_play_state(int nstate);
int mqtt_publish(char* payload);
int mqtt_publish_message(char* payload);
void on_user_update(const char *payload, int len);
void orderCnt_lcd_init();
int player_proc_init();
int play_proc_with_fifo();
void set_counter_led();
int set_keepAliveInterval(int keepAliveInterval);
void set_setting_int(const char *key, int val);
void set_setting_str(const char* key, const char* val);
int mqtt_power_resume_proc(int ret);
int MqttPlatformInit();
void voice_play_func(cJSON* rootobj);


