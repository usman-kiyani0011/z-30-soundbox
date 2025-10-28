#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "player_proc.h"
#include "mqtt_embed/MQTTClient/src/MQTTClient.h"
#include "tracedef.h"

#include "mbedtls/include/mbedtls/sha1.h"
#include "mbedtls/include/mbedtls/md.h"
#include "libapi_xpos/inc/libapi_comm.h"
#include "page_main.h"
#include "page_pub.h"
#include "pub/cJSON.h"

#include "pub/http.h"
#include "mqtt_wrapper.h"
#include "file.h"

#include "libapi_xpos/inc/libapi_util.h"
#include "libapi_xpos/inc/libapi_file.h"
#include "libapi_xpos/inc/util_tts.h"

#include "libapi_xpos/inc/mfsdk_define.h"
#include "libapi_xpos/inc/mfsdk_lcd.h"
#include "libapi_xpos/inc/mfsdk_power.h"
#include "libapi_xpos/inc/mfsdk_gui.h"
#include "libapi_xpos/inc/mfsdk_aud.h"
#include "libapi_xpos/inc/mfsdk_mem.h"
#include "libapi_xpos/inc/mfsdk_sys.h"
#include "libapi_xpos/inc/mfsdk_tms.h"
#include "libapi_xpos/inc/mfsdk_comm.h"
#include "libapi_xpos/inc/mfsdk_fs.h"
#include "libapi_xpos/inc/mfsdk_util.h"
#include "libapi_xpos/inc/mfsdk_nfc.h"

#include "sdk_tts.h"
#include "./card/func_pay.h"
#include "./pub/queue_pub.h"
#include "./pages/pages.h"
#include "./pages/page_scan.h"
#include "./pages/page_version.h"
static unsigned char sendbuf[1024] = {0};
static unsigned char readbuf[1024] = {0};
static MQTTClient *s_client = 0;
static int mqtt_con_state = 0;
static MQTTClient c;
static Network n;

typedef struct st_mqtt_config
{
	int plamtform;
	int keepAliveInterval;
	int mqttfeedback;
	int port;
	char ip[64];
	char hmac_key[17];
	char hmac_secret[17];
	char clientID[32];
	char username[32];
	char mqPassword[128];
	char topic[128];//make topic buffer big enough to receive
}st_mqtt_config;

st_mqtt_config g_mqttConfig;


#define SET_INI_SECTION           "set"
#define SET_INI_FILENAME  "exdata\\setting.ini"
#define DEFAULT_PRODUCT_KEY			"pFppbioOCKlo5c8E"
#define DEFAULT_PRODUCT_SECRET		"sj2AJl102397fQAV"

static int m_nrecordCount = 0;
static char *g_qrcode = NULL;

void orderCnt_lcd_init()
{
    char szcounter[16] = { 0 };

    m_nrecordCount = 0;
    sprintf(szcounter, "%03d", 0);

    MfSdkGuiLedCounter(szcounter);
	AppFormatSegmentLedAmount(0LL);
}

void set_counter_led()
{
	char szcounter[16] = { 0 };

	MfSdkLcdSegmentBackLight(MFSDK_LCD_ON);
	m_nrecordCount++;

	if (m_nrecordCount > 999)
	{
		m_nrecordCount = 1;
	}
	sprintf(szcounter, "%d", m_nrecordCount);
	MfSdkGuiLedCounter(szcounter);
}

void set_setting_str(const char* key, const char* val)
{
    MfSdkFsWriteProfileString((const s8 *) SET_INI_SECTION, (const s8 *) key, (const s8 *) val, (const s8 *)SET_INI_FILENAME);
}

void set_setting_int(const char *key, int val)
{
    MfSdkFsWriteProfileInt((const s8 *)SET_INI_SECTION, (const s8 *) key, val, (const s8*) SET_INI_FILENAME);
}

void get_setting_str(const char* key, char* val, int len)
{
    MfSdkFsReadProfileString((const s8 *)SET_INI_SECTION, (const s8 *)key, (s8 *)val, len, (const s8 *)"", (const s8 *)SET_INI_FILENAME);
}

int get_setting_int(const char* key)
{
    char value[32] = { 0 };

    get_setting_str(key, value, sizeof(value));
    return atoi(value);
}

int MqttPlatformInit()
{
	char cid[32] = {0};

	memset(&g_mqttConfig, 0x00, sizeof(st_mqtt_config));

    //Default 0
    g_mqttConfig.plamtform = MfSdkFsReadProfileInt((const s8*)SET_INI_SECTION, (const s8*)MQTT_PLATFORM, 0, (const s8*)SET_INI_FILENAME);
	APP_TRACE("get g_plamtform:%d\r\n", g_mqttConfig.plamtform);

	//set ping interval
	g_mqttConfig.keepAliveInterval = get_keepAliveInterval();

    g_mqttConfig.port = MQTT_PORT;
    get_setting_str(COMM_HOST_IP, g_mqttConfig.ip, sizeof(g_mqttConfig.ip));
    if (strlen(g_mqttConfig.ip) < 3)
    {
        strcpy(g_mqttConfig.ip, MQTT_HOST);
    }
	else
	{
	    g_mqttConfig.port = get_setting_int(COMM_HOST_PORT);
	}
	APP_TRACE("ip:%s, port:%d\r\n", g_mqttConfig.ip, g_mqttConfig.port);

    get_setting_str(PRODUCT_KEY, (char*)g_mqttConfig.hmac_key, sizeof(g_mqttConfig.hmac_key));
    if (strlen((char*)g_mqttConfig.hmac_key) < 3)
    {
        strcpy((char*)g_mqttConfig.hmac_key, DEFAULT_PRODUCT_KEY);
    }

    get_setting_str(PRODUCT_SECRET, (char*)g_mqttConfig.hmac_secret, sizeof(g_mqttConfig.hmac_secret));
    if (strlen((char*)g_mqttConfig.hmac_secret) < 3)
    {
        strcpy((char*)g_mqttConfig.hmac_secret, DEFAULT_PRODUCT_SECRET);
    }

	memset(cid, 0, sizeof(cid));
	#ifdef TEST_SN
    strcpy(cid, TEST_SN);
	#else
	MfSdkSysGetTerminalSn((s8*)cid, sizeof(cid));
	#endif
	
    get_setting_str(CLIENT_ID, (char*)g_mqttConfig.clientID, sizeof(g_mqttConfig.clientID));
    if (strlen((char*)g_mqttConfig.clientID) < 3)
    {
		strcpy((char*)g_mqttConfig.clientID, cid);
    }

    get_setting_str(USER_NAME, (char*)g_mqttConfig.username, sizeof(g_mqttConfig.username));
    if (strlen((char*)g_mqttConfig.username) < 3)
    {
		strcpy((char*)g_mqttConfig.username, cid);
    }
	APP_TRACE("username:%s, clientID:%s\r\n", g_mqttConfig.username, g_mqttConfig.clientID);

    get_setting_str(MQTT_TOPIC, (char*)g_mqttConfig.topic, sizeof(g_mqttConfig.topic));
    if (strlen((char*)g_mqttConfig.topic) < 3)
    {
		snprintf(g_mqttConfig.topic, sizeof(g_mqttConfig.topic), "/ota/%s/%s/update", g_mqttConfig.hmac_key, g_mqttConfig.clientID);
    }
	APP_TRACE("topic:%s\r\n", g_mqttConfig.topic);

	g_mqttConfig.mqttfeedback = get_setting_int(MQTT_FEEDBACK);

    return 0;
}


static int s_power_resume = 0;
/**
 * @brief mqtt Thread wake-up power
 * @param ret
 * @return
 */
int mqtt_power_resume_proc(int ret)
{
	if (ret == 4)
	{
		APP_TRACE("%s\r\n", __FUNCTION__);
		s_power_resume = 1;
	}
	return 0;
}

int json_getval(cJSON* rootobj, const char* key, char* outbuff, int size)
{
	int ret = -1;
    cJSON* obj = NULL;

    if (rootobj == NULL)
    {
        APP_TRACE("cJSON_Parse error\r\n");
        return -1;
    }
    obj = cJSON_GetObjectItem(rootobj, key);

    if (obj != NULL)
    {
        memset(outbuff, 0, size);

        if (obj->valuestring == NULL)
        {
            snprintf(outbuff, size, "%d", obj->valueint);
            ret = strlen(outbuff);
        }
        else
        {
            if (strlen(obj->valuestring) > size)
            {
                APP_TRACE("json lenth error\n");
                ret = -2;
            }
			else
			{
	            snprintf(outbuff, size,"%s", obj->valuestring);
                ret = strlen(outbuff);
			}
        }
    }
    else
    {
        APP_TRACE("cJSON_GetObjectItem error,key = %s\r\n", key);
        ret = -1;
    }
    return ret;
}

static int mqtt_publish_feedback(const char* request_id, const char* money)
{
	int res = 0;
	int topic_len = 0;
	const char* fmt = "/ota/%s/%s/user/log";
	char* topic = NULL;
	char payload[128] = { 0 };
	MQTTMessage msg;
	char deviceName[32] = { 0 };
    char* hmac_key = DEFAULT_PRODUCT_KEY;

    if (MfSdkCommLinkState() != 1)
    {
        APP_TRACE("LinkState error");
        return -1;
	}

	if (NULL == s_client || MQTTIsConnected(s_client) != 1)
	{
		APP_TRACE("mqtt_publish error");
		return -2;
	}

	strcpy(deviceName, g_mqttConfig.clientID);
	
	topic_len = strlen(fmt) + strlen((char*)hmac_key) + strlen(deviceName) + 1;
	topic = MfSdkMemMalloc(topic_len);
	if (topic == NULL) 
	{
		APP_TRACE("memory not enough");
		return -1;
	}

	memset(topic, 0, topic_len);
	snprintf(topic, topic_len, fmt, (char*)hmac_key, deviceName);

	sprintf(payload, "{\"request_id\":\"%s\",\"money\":\"%s\"}", request_id, money);
	APP_TRACE("Payload send log : %s", payload);

	memset(&msg, 0x00, sizeof(msg));
	msg.payloadlen = strlen(payload);
	msg.payload = (void*)payload;

	res = MQTTPublish(s_client, topic, &msg);//0, topic, IOTX_MQTT_QOS1, payload, strlen(payload));
	if (res < 0) 
	{
		APP_TRACE("publish failed, res = %d", res);
	}

	MfSdkMemFree(topic);
	topic = NULL;
	return res;
}

void voice_play_func(cJSON* rootobj)
{
	long long money = 0;
	int broadcast_type = -1;
	char temp[256] = {0};
	char strmoney[128] = {0};

	if ( json_getval(rootobj, "broadcast_type", temp, sizeof(temp)) >= 0
		&& json_getval(rootobj, "money", strmoney, sizeof(strmoney)) >= 0 )
	{
		//payload example:
		//"{"broadcast_type":1,"money":"1.23","request_id":"2022081951495648","datetime":"20220819144051","ctime":1660891251}"

		money = MfSdkUtilStr2Longlong((const s8 *) strmoney);
		if(money <= 0) { return; }
		
		broadcast_type = atoi(temp);
		APP_TRACE("broadcast_type=%d\r\n",broadcast_type);
		if (broadcast_type == 1)
		{
			if(QRPAGE_LOCK_ON == GetPowerLock())
			{
				AppPowerUnlockApp("qr");
				SetPowerLock(QRPAGE_LOCK_DEF);
			}
			PayClosePage();
			close_qrcode_page(0);
			page_show_image(get_mainpage(), NULL, 0, 0, 0, 3000);
			set_counter_led();
		}
		
		APP_TRACE("======== play begin: %d ======== \r\n", MfSdkSysGetTick());
		APP_TRACE("broadcast_type=%d\r\n", broadcast_type);

		if(broadcast_type == 1)
		{
			char *dot;
			int moneylen = 0;
			int moneyshowlen = 0;
			int numShowLen = 0;

			dot = strstr(strmoney, ".");
			if (dot != 0)
			{
				moneylen = dot - strmoney;
				moneyshowlen = strlen(strmoney) - 1;
			}
			else
			{
				moneylen = strlen(strmoney);
				moneyshowlen = strlen(strmoney);
			}

			//Judge the number of broken code screen
			numShowLen = MfSdkSysGetSegmentLcdDisplayMaxLength();
			APP_TRACE("strmoney=%s    numShowLen=%d\r\n", strmoney, numShowLen);
			APP_TRACE("play money=%lld\r\n", money);

			if(moneyshowlen <= numShowLen)
			{
				MfSdkGuiLedAmount(strmoney);
			}
			else
			{
				snprintf(strmoney, sizeof(strmoney), "%.*s", numShowLen, "---------");
				MfSdkGuiLedAmount(strmoney);
			}

			if (moneylen <= 7)
			{
				pub_tts_play_amt_en(money);
			}
			else
			{
				PubMultiPlay((const s8*)"pay.mp3");
				PubMultiPlay((const s8*)"success.mp3");
			}
		}

		APP_TRACE("======== play end: %d ======== \r\n", MfSdkSysGetTick());
	}
	return;
}

int voicePlayZipperInfo(cJSON *rootobj)
{
	int broadcast_type = -1;
	char temp[256] = {0};
	char strmoney[128] = {0};

	memset(temp,0x00,sizeof(temp));

	if ( json_getval(rootobj,"broadcast_type" , temp, sizeof(temp)) > 0 )
	{
		broadcast_type = atoi(temp);
		APP_TRACE("broadcast_type=%d\r\n",broadcast_type);
	
		if (broadcast_type == 2)
		{
			cJSON *array = cJSON_GetObjectItem(rootobj, "voiceZipperInfo");
			if (array)
			{
				int cnt = cJSON_GetArraySize(array);
				APP_TRACE("Array cnt: %d\r\n", cnt);
				for (int i=0; i<cnt; i++)
				{
					cJSON *arraytemp = cJSON_GetArrayItem(array, i);
					if (arraytemp && cJSON_IsObject(arraytemp))
					{
						cJSON *name = cJSON_GetObjectItemCaseSensitive(arraytemp, "name");
						if (name)
						{
							char filename[128] = {0};
							snprintf(filename, sizeof(filename), "%s.mp3", name->valuestring);
							APP_TRACE("Array value: %s\r\n", name->valuestring);
							PubMultiPlay((const s8 *)filename);
						}
					}
				}
			}
		}
	}
	return 0;
}

void on_user_update(const char *payload, int len)
{
	int qrCodeLenth = 0;
	char temp[256] = {0};
	cJSON* rootobj = NULL;
    cJSON* obj = NULL;

	APP_TRACE("[%s]\r\n", __FUNCTION__);
	rootobj = cJSON_Parse(payload);

	if (rootobj == NULL)
	{
		APP_TRACE("cJSON_Parse error\r\n");
		return;
	}

	if (json_getval(rootobj, "tms_action", temp, sizeof(temp)) >= 0 )
	{
		if ( strcmp(temp, "1") == 0 )       //tms heartbeat
		{
			MfSdkTmsHeartBeat();
		}
	}

	obj = cJSON_GetObjectItem(rootobj, "order_sn");
    if (obj != NULL && obj->valuestring != NULL && strlen(obj->valuestring) > 0)
	{
		qrCodeLenth = strlen(obj->valuestring)+1;
		if(NULL != g_qrcode)
		{
			MfSdkMemFree(g_qrcode);
			g_qrcode = NULL;
		}

		g_qrcode = MfSdkMemMalloc(qrCodeLenth);
		if(NULL == g_qrcode)
		{
		    cJSON_Delete(rootobj);
	        APP_TRACE("g_qrcode Malloc error\r\n");
	        return;
		}

		memset(g_qrcode, 0, qrCodeLenth);
        snprintf(g_qrcode, qrCodeLenth, "%s", obj->valuestring);
		
		//QR code display
		//payload = 0x02fec42c "{"money":"1.23","request_id":"2022081910050555","order_sn":"2022081910050102","datetime":"20220819143957","ctime":1660891197}"
        memset(temp, 0x00, sizeof(temp));
        if(json_getval(rootobj, "money", temp, sizeof(temp)) >= 0 \
			&& MfSdkUtilStr2Longlong((const s8*)temp) > 0)
		{ 
			ShowSetting_func(0, 0);
			show_page_qrcode(get_mainpage(), NULL, temp, g_qrcode);
		}
	}
	else
	{
		ShowSetting_func(0, 0);
		AppPowerLockApp((char*)"play_proc_highprio");
		voice_play_func(rootobj);
		AppPowerUnlockApp((char*)"play_proc_highprio");
	}
	cJSON_Delete(rootobj);

	save_messageId();
}

int play_proc_with_fifo()
{
	char* payload = NULL;
	int payloadlen = 0;

	if (MfSdkAudTtsState() == 1)
	{
		return -1;
	}

	if (getTransacationStatus() == -1)
	{
		return -2;
	}

	fifo_get(&payload);
	APP_TRACE("payload = 0x%p\r\n", payload);
	APP_TRACE("payload : %s\r\n", payload);

	if (payload != NULL)
	{
		payloadlen = strlen(payload);
		//AppPowerLockApp((char*)"play_proc_highprio");

		if (payloadlen > 0)
		{
			MfSdkLcdBackLight(MFSDK_LCD_ON);
			on_user_update(payload, payloadlen);
		}

		MfSdkMemFree(payload);
		payload = NULL;
		//AppPowerUnlockApp((char*)"play_proc_highprio");
		return 1;
	}
	else
	{
		return 0;
	}
}

static void mqtt_comm_messageArrived(MessageData* md)
{
	char amount[16 + 1] = { 0 };
	char messageId[MESSAGEID_SIZE + 1] = { 0 };
	cJSON* rootobj = NULL;
	MQTTMessage* m = md->message;

	if (m->payloadlen > 0)
	{
		if(fifo_checkFull()) 
		{
		    APP_TRACE("fifo full !\n");
			return;
		}

		char* arMesg = MfSdkMemMalloc(m->payloadlen + 1);//free after it's played
		if (NULL == arMesg)
		{
			APP_TRACE("msg malloc fail !\n");
			return;
		}
		
		memset(amount, 0, sizeof(amount));
		memset(messageId, 0, sizeof(messageId));
		memset(arMesg, 0x00, m->payloadlen + 1);
		memcpy(arMesg, m->payload, m->payloadlen);
		arMesg[m->payloadlen] = 0;
		APP_TRACE("messageArrived(%d):\r\n r:%s\r\n", m->payloadlen, arMesg);
		APP_TRACE("======== user_update_message_arrive: %d ========\r\n", MfSdkSysGetTick());

		rootobj = cJSON_Parse(arMesg);
		if (rootobj == NULL) 
		{
			APP_TRACE("cJSON_Parse error\r\n");
			MfSdkMemFree(arMesg);
			return;
		}

		if (json_getval(rootobj, "request_id", messageId, sizeof(messageId)) > 0)
		{
			APP_TRACE("request_id: %s", messageId);
			if (0 == check_messageId(messageId)) 
			{
				fifo_put((char*)&arMesg);
				add_messageId(messageId);
				/*	message feedback to test platform.
					Test environment functionality, 
					please make changes according to the 
					requirements of the connected mqtt platform 
					@Linwei 2024.04.11 19:12
				*/
				if (1 == g_mqttConfig.mqttfeedback && json_getval(rootobj, "money", amount, sizeof(amount)) > 0)
				{
					mqtt_publish_feedback(messageId, amount);
				}
			}
			else 
			{
				APP_TRACE("Duplicate order number, discard msg");
				MfSdkMemFree(arMesg);
				arMesg = NULL;
			}
		}
		else
		{
			MfSdkMemFree(arMesg);
			arMesg = NULL;
		}

		cJSON_Delete(rootobj);
	}
	else
	{
		APP_TRACE("messageArrived is null\r\n");
	}
}

static int mqtt_subscribe_yield(MQTTClient *c, char *topic)
{
	int rc = -1;
	int subsqos = 1;
	MQTTSubackData suback;

	//sprintf((char*)s_status, "Subscribe..");
	APP_TRACE("%s\r\n", __FUNCTION__);
	rc = MQTTSubscribeWithResults(c, topic, subsqos, mqtt_comm_messageArrived, &suback);
	APP_TRACE("rc from sub:%d>>%s\r\n", rc, topic);

	return rc;
}

static int HMACcalculate(char *in, char* key, char * out)
{
	unsigned char md[32] = {0};     //32 bytes
	const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

	mbedtls_md_hmac(md_info,(const u8*)key, strlen(key),(const u8*)in, strlen(in), md);
	MfSdkUtilHex2Asc(md, 64, 0, (u8*)out);

	return 0;
}

int set_keepAliveInterval(int keepAliveInterval)
{
	//Default heartbeat interval 60s
	if (keepAliveInterval > 0)
	{
		MfSdkFsWriteProfileInt((const s8*)SET_INI_SECTION, (const s8*)KEEPALIVE_TIME, keepAliveInterval, (const s8*)SET_INI_FILENAME);
		APP_TRACE("set keepAliveInterval:%d\r\n", keepAliveInterval);
	}
    return keepAliveInterval;
}

int get_keepAliveInterval()
{
	int keepAliveInterval = 0;

	//Default heartbeat interval 300s
	keepAliveInterval = MfSdkFsReadProfileInt((const s8*)SET_INI_SECTION, (const s8*)KEEPALIVE_TIME, 300, (const s8*)SET_INI_FILENAME);
	APP_TRACE("get keepAliveInterval:%d\r\n", keepAliveInterval);
	return keepAliveInterval;
}

//Connect to AWS Please enable AWSMQTT_URL. Create your AWS account from the website: https://aws.amazon.com
static int AWS_mqtt_comm_run()
{
	int rc = -1;
	char ip[64] = { 0 };
	int port = 8883;

	get_setting_str(COMM_HOST_IP, ip, sizeof(ip));
	if (strlen(ip) < 3)
	{
		#ifdef AWSMQTT_URL
		strcpy(ip, AWSMQTT_URL);
		#endif
		port = 8883;
	}
	else
	{
		port = get_setting_int(COMM_HOST_PORT);
	}

	APP_TRACE("Connecting\r\n");

	//Please use the certificate files of the device you created on the AWS platform(refer to te manual "AWS configuration guidance.docx")
	//ca.pem: AmazonRootCA1.pem, cil.crt:xxx-certificate.pem.crt, pri.key:xxx-private.pem.key
	rc = NetworkConnect_ssl(&n, ip, port, "xxxx\\ca.pem", "xxxx\\cli.crt", "xxxx\\pri.key");
	APP_TRACE("rc from net:%d\r\n", rc);

	if (rc == 0)
	{
		MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
		memset(&c, 0x00, sizeof(MQTTClient));
		MQTTClientInit(&c, &n, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));
		c.defaultMessageHandler = mqtt_comm_messageArrived;

		data.cleansession = 0;
		data.keepAliveInterval = g_mqttConfig.keepAliveInterval;
		data.clientID.cstring = g_mqttConfig.clientID;
		data.username.cstring = g_mqttConfig.username;
		data.password.cstring = "";

		rc = MQTTConnect(&c, &data);
		APP_TRACE("rc from connect:%d\r\n", rc);

		if (rc == 0)
		{
			s_client = &c;
			//topic: "/ota/<sn>/update"
			memset(g_mqttConfig.topic, 0, sizeof(g_mqttConfig.topic));
			snprintf(g_mqttConfig.topic, sizeof(g_mqttConfig.topic), "/ota/%s/update", g_mqttConfig.clientID);
			APP_TRACE("m_topic:%s\r\n", g_mqttConfig.topic);
			rc = mqtt_subscribe_yield(&c, g_mqttConfig.topic);
		}
	}
	return rc;
}

static int mqtt_comm_run()
{
	int tick = -1;
	int rc = -1;
	char hmac_payload[256] = {0};

	APP_TRACE("Connecting\r\n");
	//Connect to mqtt broker
	//rc = NetworkConnect(&n, g_mqttConfig.ip, g_mqttConfig.port);
	rc = NetworkConnect_ssl(&n, "202c8d1e67214669b19c4341947837d8.s1.eu.hivemq.cloud", 8883, "", "", "");
	APP_TRACE("rc from net:%d\r\n", rc);
    if ( rc != 0 ) 
	{
        APP_TRACE("Network connection failed: %d\n", rc);
        return rc;
    }
	
	//Initialize MQTT client
	memset(&c, 0x00, sizeof(MQTTClient));
	MQTTClientInit(&c, &n, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));
	c.defaultMessageHandler = mqtt_comm_messageArrived;

	//Calculate mqtt password
	memset(hmac_payload, 0, sizeof(hmac_payload));
	snprintf(hmac_payload, sizeof(hmac_payload), "clientId{%s}.{%s}%s%s%s", 
	g_mqttConfig.clientID, g_mqttConfig.clientID, g_mqttConfig.clientID, g_mqttConfig.hmac_key, "2524608000000");
	HMACcalculate(hmac_payload, (char*)g_mqttConfig.hmac_secret, g_mqttConfig.mqPassword);

	//Set up mqtt connection data
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.cleansession = 0;
	data.keepAliveInterval = g_mqttConfig.keepAliveInterval;
	data.clientID.cstring = g_mqttConfig.clientID;
	data.username.cstring = "mydev";//g_mqttConfig.username;
	data.password.cstring = "Ab123456";//g_mqttConfig.mqPassword;

	//Connect to MQTT broker
	tick = MfSdkSysGetTick();
	rc = MQTTConnect(&c, &data);
	APP_TRACE("[Latency] MQTTConnect:%d\r\n", MfSdkSysGetTick()-tick);
	APP_TRACE("rc from connect:%d\r\n", rc);
	if ( rc != 0 ) 
	{
		APP_TRACE("MQTT connection failed: %d\n", rc);
		return rc;
	}
	
	s_client = &c;
	//Subscribe mqtt topic
	memset(g_mqttConfig.topic, 0, sizeof(g_mqttConfig.topic));
	strcpy(g_mqttConfig.topic, "/update");
	APP_TRACE("mqtt topic:%s\r\n", g_mqttConfig.topic);
	rc = mqtt_subscribe_yield(&c, g_mqttConfig.topic);

	return rc;
}



void mqtt_play_state(int nstate)
{
	PubMultiPlay((const s8*)(nstate? "sers.mp3" : "serf.mp3"));
}

int get_mqtt_con_state()
{
	return mqtt_con_state;
}

int mqtt_publish_message(char* payload)
{
	int             res = 0;
	const char* fmt = "/ota/%s/%s/update";
	char* topic = NULL;
	char* hmac_key = DEFAULT_PRODUCT_KEY;
	int             topic_len = 0;
	char deviceName[32] = { 0 };
	MQTTMessage msg;

    if (MfSdkCommLinkState() != 1)
    {
        APP_TRACE("LinkState error");
        return -1;
	}

	if (NULL == s_client || MQTTIsConnected(s_client) != 1)
	{
		APP_TRACE("mqtt_publish error");
		return -2;
	}

	strcpy(deviceName, g_mqttConfig.clientID);
	topic_len = strlen(fmt) + strlen(hmac_key) + strlen(deviceName) + 1;
	topic = (char*)MfSdkMemMalloc(topic_len);
	if (topic == NULL) 
	{
		APP_TRACE("memory not enough");
		return -1;
	}
	memset(topic, 0, topic_len);
	snprintf(topic, topic_len, fmt, hmac_key, deviceName);

	memset(&msg, 0x00, sizeof(msg));
	msg.payloadlen = strlen(payload);
	msg.payload = (void*)payload;

	res = MQTTPublish(s_client, topic, &msg);//0, topic, IOTX_MQTT_QOS1, payload, strlen(payload));
	if (res < 0) 
	{
		APP_TRACE("publish failed, res = %d", res);
	}
	APP_TRACE("topic = %s", topic);
	MfSdkMemFree(topic);
	topic = NULL;
	return res;
}

int mqtt_publish(char* payload)
{
	int res = 0;
	const char* fmt = "/ota/device/connection/%s/state";
	char* topic = NULL;
	int topic_len = 0;
	char deviceName[32] = { 0 };
	MQTTMessage msg;

	if (MfSdkCommLinkState() != 1)
	{
		APP_TRACE("LinkState error");
		return -1;
	}

	if (NULL == s_client || MQTTIsConnected(s_client) != 1)
	{
		APP_TRACE("mqtt_publish error");
		return -2;
	}

	strcpy(deviceName, g_mqttConfig.clientID);
	topic_len = strlen(fmt) + strlen(deviceName) + 1;
	topic = MfSdkMemMalloc(topic_len);

	if (topic == NULL)
	{
		APP_TRACE("memory not enough");
		return -1;
	}
	memset(topic, 0, topic_len);
	snprintf(topic, topic_len, fmt, deviceName);

	memset(&msg, 0x00, sizeof(msg));
	msg.payloadlen = strlen(payload);
	msg.payload = (void*)payload;

	res = MQTTPublish(s_client, topic, &msg);    //0, topic, IOTX_MQTT_QOS1, payload, strlen(payload));
	if (res < 0)
	{
		APP_TRACE("publish failed, res = %d", res);
	}
	
	MfSdkMemFree(topic);
	topic = NULL;
	return res;
}



static void MqttSendPinProc(MQTTClient *pclient)
{
	int ret = 0;
	while ((pclient != 0) && MQTTIsConnected(pclient))
	{
		if (MfSdkSysAppIsLock() == 1)                     //that there are other tasks
		{
			APP_TRACE("IOT_MQTT_Yield MfSdkSysAppIsLock() == 1 break\r\n");
			break;
		}

		//Sleep Wake
		if (s_power_resume == 1)
		{
			APP_TRACE("MQTTStartPing... \r\n");
			MQTTStartPing(pclient);
			APP_TRACE("======PING======[%d]\r\n",MfSdkSysGetTick());
			s_power_resume = 0;
		}

		ret = MQTTYield(pclient, 1000);                    //Send heartbeat packet
		if(ret)
		{
			mqtt_con_state = 0;
			APP_TRACE("MQTTYield break, ret= %d\r\n", ret);
			break;
		}

		if (MQTTIsConnected(pclient))
		{
			if (MQTT_Keepalive_Probes(pclient) == 0)
			{
				MfSdkPowerTaskSuspend(TASK_PRIO, 500);
			}
			APP_TRACE("mqtt running......\r\n");
		}
	}

	if(NULL != pclient)
	{
		MQTTDisconnect(pclient);
		APP_TRACE("mqtt Disconnect......\r\n");
		pclient = 0;
	}
	mqtt_con_state = 0;

}

static void mqtt_comm_task(void* p)
{
	int ret = 0;

	enum
	{
		net_state_player,
		iot_init_player,
		ticks_count
	};
	int ticks[ticks_count];


	static int dev_open_flag = 0;

	int network_state = -1;
	static int mqtt_con_success_state_flag = 0;

	APP_TRACE("mqtt_comm_task\r\n");
	memset(ticks, 0x00, sizeof(int) * ticks_count);

	MfSdkPowerTaskInit(TASK_PRIO);
	//Check the network for 60 seconds after startup. No network broadcast is abnormal
	ticks[net_state_player] = MfSdkSysTimerOpen(60 * 1000);//Query network connection status
	ticks[iot_init_player] = 0;

	while (1)
	{
		if (MfSdkSysAppIsLock() == 1)
		{
			MfSdkSysSleep(1000);
			continue;
		}

		//Waiting network..
		if (MfSdkSysTimerCheck(ticks[net_state_player]))
		{
			int netlinkstate = MfSdkCommLinkState();
			APP_TRACE("newnetstate================%d\r\n", netlinkstate);
			if (netlinkstate == 1)
			{
				//Network connection succeeded
				dev_open_flag = 1;
				APP_TRACE("netlinkstate success\r\n");
				//The network successfully starts connecting to the service immediately
				network_state = 1;

				if (ticks[iot_init_player] == 0)
				{
					ticks[iot_init_player] = MfSdkSysTimerOpen(60 * 1000);//Query network connection status
				}
			}
			else
			{
				network_state = -1;
			}
		}
		else//60s timeout
		{
			if (dev_open_flag == 0)
			{
				dev_open_flag = 1;
				if (MfSdkCommGetNetMode() == MFSDK_COMM_NET_ONLY_WIRELESS \
					&& MfSdkCommAtcCpin(0) != MFSDK_COMM_ATCCPIN_NORMAL)//Judge whether there is a sim card
				{
					PubMultiPlay((const s8*)"nonsim.mp3");
					APP_TRACE("nonsim\r\n");
				}
				else
				{
					PubMultiPlay((const s8*)"netf.mp3");
					APP_TRACE("netf\r\n");
				}
			}
			ticks[net_state_player] = MfSdkSysTimerOpen(60 * 1000);//Reset the timer
		}

		if (network_state == 1)
		{
			#ifdef AWSMQTT_URL
			ret = AWS_mqtt_comm_run();
			#else
			if(1 == g_mqttConfig.plamtform)
				ret = AWS_mqtt_comm_run();
			else
				ret = mqtt_comm_run();
			#endif
			APP_TRACE("mqtt_comm_run[%d] ret=%d\r\n", g_mqttConfig.plamtform, ret);
			if (ret)
			{
				ticks[net_state_player] = MfSdkSysTimerOpen(60 * 1000);//Reset the timer

				if (MfSdkSysTimerCheck(ticks[iot_init_player]) == 0)//timeout
				{
					if (mqtt_con_success_state_flag == 0)
					{
						mqtt_con_success_state_flag = 1;
						mqtt_play_state(0);
						APP_TRACE("mqtt connect fail\r\n");
					}
					ticks[iot_init_player] = 0;
				}
				mqtt_con_state = 0;
			}
			else//connect success
			{
				if (mqtt_con_success_state_flag == 0)
				{
					mqtt_con_success_state_flag = 1;
					mqtt_play_state(1);
					APP_TRACE("mqtt connect success\r\n");
				}
				mqtt_con_state = 1;

				MqttSendPinProc(s_client);
			}
		}
		//Wait one second and try again
		NetworkDisconnect(&n);
		MfSdkSysDelay(1000);
		network_state = -1;
	}
}

#define _APP_TASK_SIZE 4096

static unsigned int pTaskStk[_APP_TASK_SIZE];


int player_proc_init()
{
	int nerr = 0;

	nerr = MfSdkSysTaskCreate(mqtt_comm_task, 0, (s8*)&(pTaskStk[0]), _APP_TASK_SIZE);
	APP_TRACE("player_proc_init %d ====== %d\r\n", TASK_PRIO, nerr);
	return 0;
}

