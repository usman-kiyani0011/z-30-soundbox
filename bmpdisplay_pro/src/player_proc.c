#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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
static unsigned char sendbuf[1024];
static unsigned char readbuf[1024];
static char cid[32];
static unsigned char s_status[32];
static MQTTClient *s_client = 0;
static int mqtt_con_state = 0;
static MQTTClient c;
static Network n;
static volatile int messageflag = 0;

static lv_obj_t* page_win = NULL;
static lv_obj_t* lab_title = NULL;
static lv_obj_t* lab_text = NULL;
static lv_task_t* task_meaasge = NULL;

#define COMM_HOST_IP                    "hostip"
#define COMM_HOST_PORT                          "port"
#define PRODUCT_KEY                     "productkey"
#define PRODUCT_SECRET                          "productsecret"
#define KEEPALIVE_TIME                          "keepalivetime"

#define SET_INI_SECTION           "set"
#define SET_INI_FILENAME  "exdata\\setting.ini"

static int m_nrecordCount = 0;
char OrderSn[256] = { 0 };

void orderCnt_lcd_init()
{
    char szcounter[16] = { 0 };

    m_nrecordCount = 0;
    sprintf(szcounter, "%03d", 0);

    MfSdkGuiLedCounter(szcounter);
    MfSdkGuiLedAmount("0.00");
}

int get_messageflag()
{
	MFSDK_UNUSED(HttpIdleflag);
    return messageflag;
}

void set_messageflag(int value)
{
    messageflag = value;
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

extern void sdk_tts_play_amt_india(int amt);
extern void pub_tts_play_amt(int amt);

static int s_power_resume = 0;
/**
 * @brief mqtt Thread wake-up power
 * @param ret
 * @return
 */
static int mqtt_power_resume_proc(int ret)
{
    if (ret == 4)
    {
        s_power_resume = 1;
    }
    return 0;
}

static int _getval(const char *data, const char *key, char *outbuff, int size)
{
    char temp[256] = {0};
    char *pstart, *pend;

    sprintf(temp, "\"%s\":", key);

    pstart = (char *) strstr(data, temp);

    if ( pstart != 0)
    {
        int vallen;

        pstart += strlen(temp);
        pend =  (char *)strstr(pstart, ",");

        if ( pend == 0 )
        {
            pend = (char *) strstr(data, "}");
        }

        //delete "
        if ( *pstart == '\"' &&  *(pend - 1) == '\"' )
        {
            pstart++;
            pend--;
        }
        vallen = pend - pstart;
        memcpy(outbuff, pstart, vallen);
        outbuff[vallen] = 0;
        return vallen;
    }
    return 0;
}

static int json_getval(cJSON* rootobj, const char* key, char* outbuff, int size)
{
    cJSON* obj = NULL;

    if (rootobj == NULL)
    {
        APP_TRACE("cJSON_Parse error\n");
        return -1;
    }
    obj = cJSON_GetObjectItem(rootobj, key);

    if (obj != NULL)
    {
        memset(outbuff, 0, size);

        if (obj->valuestring == NULL)
        {
            sprintf(outbuff, "%d", obj->valueint);
        }
        else
        {
            sprintf(outbuff, "%s", obj->valuestring);
        }
        return strlen(outbuff);
    }
    else
    {
        APP_TRACE("cJSON_GetObjectItem error\n");
        return -1;
    }
    return 0;
}

void set_counter_led()
{
    char szcounter[16] = { 0 };

    if(MfSdkSysDevIs(MFSDK_SYS_DEV_MODEL_ET389))
    {

        MfSdkLcdSegmentBackLight(MFSDK_LCD_ON);
        m_nrecordCount++;

        if (m_nrecordCount > 999)
        {
            m_nrecordCount = 1;
        }
        sprintf(szcounter, "%d", m_nrecordCount);
        MfSdkGuiLedCounter(szcounter);
    }
}

static void voice_play_func(cJSON* rootobj)
{
    long long money = 0;
    int broadcast_type = -1;
    char temp[256] = {0};
    char strmoney[128] = {0};

    if ( json_getval(rootobj, "broadcast_type", temp, sizeof(temp)) >= 0
         && json_getval(rootobj, "money", strmoney, sizeof(strmoney)) >= 0 )
    {
        broadcast_type = atoi(temp);
        money = MfSdkUtilStr2Longlong((const s8 *) strmoney);

        if(money <= 0) { return; }

        if (broadcast_type == 10 || broadcast_type == 1)
        {
            show_page_result(0, NULL);
            set_counter_led();
        }
        APP_TRACE("======== play begin: %d ======== \r\n", MfSdkSysGetTick());

        APP_TRACE("broadcast_type=%d\r\n", broadcast_type);

        if(broadcast_type == 10)
        {
            char sttstext[256] = {0};
            messageflag = 1;

            if(json_getval(rootobj, "ttstext", sttstext, sizeof(sttstext)) >= 0)
            {
                APP_TRACE("osl_tts_play_2\r\n")
                MfSdkAudTtsPlay((s8*)sttstext);

                if(MfSdkSysDevIs(MFSDK_SYS_DEV_MODEL_ET389))
                {
                    if ( strlen(strmoney) <= 10 )
                    {
                        APP_TRACE("strmoney=%s\r\n", strmoney);
                        MfSdkGuiLedAmount(strmoney);
                    }
                }
            }
        }
        else if (broadcast_type == 11)
        {
            messageflag = 1;

            if (strlen(strmoney) <= 10)
            {
                APP_TRACE("strmoney=%s\r\n", strmoney);
                MfSdkGuiLedAmount(strmoney);
            }
            APP_TRACE("strmoney=%s money=%lld\r\n", strmoney, money);
            memset(temp, 0x00, sizeof(temp));
            sprintf(temp, "%lld", money);
            free_rfid_page();
            memset(OrderSn, 0, sizeof(OrderSn));
            func_pay(get_mainpage(), temp);
        }
        else if(broadcast_type == 1)
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
            if (MfSdkSysGetHardwareVer() >= 500)
            {
                numShowLen = 7;
            }
            else
            {
                numShowLen = 6;
            }
            APP_TRACE("strmoney=%s    numShowLen=%d\r\n", strmoney, numShowLen);
            APP_TRACE("play money=%lld\r\n", money);

            if(MfSdkSysDevIs(MFSDK_SYS_DEV_MODEL_ET389) && moneyshowlen <= numShowLen)
            {
                MfSdkGuiLedAmount(strmoney);
            }

            MfSdkAudPlay((const s8*)"pay.mp3");
            if (moneylen <= 7)
            {
                if (money >= 100)
                {
                    sdk_tts_play_amt_en(money / 100);
                    MfSdkAudPlay((const s8*)"dollar.mp3");

                    if (money % 100 != 0)
                    {
                        MfSdkAudPlay((const s8 *)"and.mp3");
                        sdk_tts_play_amt_en(money % 100);
                        MfSdkAudPlay((const s8 *)"cents.mp3");
                    }
                }
                else
                {
                    if (money % 100 != 0)
                    {
                        sdk_tts_play_amt_en(money % 100);
                        MfSdkAudPlay((const s8*)"cents.mp3");
                    }
                }
            }
            else
            {
                MfSdkAudPlay((const s8*)"success.mp3");
            }
        }
        APP_TRACE("======== play end: %d ======== \r\n", MfSdkSysGetTick());
    }
}

void on_user_update(const char *payload, int len)
{
    char temp[256] = {0};
    cJSON* rootobj = NULL;

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

    if(json_getval(rootobj, "order_sn", temp, sizeof(temp)) >= 0 )
    {    //payload = 0x02fec42c "{"money":"1.23","request_id":"2022081910050555","order_sn":"2022081910050102","datetime":"20220819143957","ctime":1660891197}"
         //QR code display
        char qrcode[256] = {0};
        memset(qrcode, 0x00, sizeof(qrcode));

        if(strlen(temp) < sizeof(qrcode))
        {
            strcpy(qrcode, temp);
        }
        memset(temp, 0x00, sizeof(temp));

        if(json_getval(rootobj, "money", temp, sizeof(temp)) >= 0 \
			&& MfSdkUtilStr2Longlong((const s8*)temp) > 0)
        {
            show_page_qrcode(get_mainpage(), temp, qrcode);
        }
    }
    else
    {
        voice_play_func(rootobj);
    }
    cJSON_Delete(rootobj);

    while ( MfSdkAudTtsState() == 1)
    {
        MfSdkSysSleep(100);
    }

}

int play_proc_with_fifo()
{
    char* payload = NULL;
    int payloadlen = 0;

    payload = fifo_get_ptr();
    APP_TRACE("payload = 0x%p\r\n", payload);
    APP_TRACE("payload : %s\r\n", payload);

    if (payload != NULL)
    {
        payloadlen = strlen(payload);
        MfSdkPowerLockApp((char*)"play_proc_highprio");

        if (payloadlen > 0)
        {
            MfSdkLcdBackLight(MFSDK_LCD_ON);
            on_user_update(payload, payloadlen);
        }

        MfSdkMemFree(payload);
        MfSdkPowerUnlockApp();
        return 1;
    }
    else
    {
        return 0;
    }
}

int play_proc_default_msg(MESSAGE * pmsg)
{
    APP_TRACE("play_proc_default_msg=%d\r\n", pmsg->MessageId);

    if (pmsg->MessageId == XM_MESSAGEARRIVED)
    {
        unsigned char *payload = (unsigned char *)pmsg->WParam;
        unsigned int payloadlen = (unsigned int)pmsg->LParam;;

        MfSdkPowerLockApp((char*)"default_msg");

        if ( payloadlen > 0)
        {
            MfSdkLcdBackLight(MFSDK_LCD_ON);
            on_user_update((const char*)payload, payloadlen);
        }
        MfSdkMemFree(payload);
        MfSdkPowerUnlockApp();
        return 1;
    }
    return 0;
}

static void mqtt_comm_messageArrived(MessageData* md)
{
    MQTTMessage* m = md->message;

    if (m->payloadlen > 0)
    {
        char* arMesg = MfSdkMemMalloc(m->payloadlen + 1);        //free after it's played
        memset(arMesg, 0x00, m->payloadlen + 1);
        memcpy(arMesg, m->payload, m->payloadlen);
        arMesg[m->payloadlen] = 0;
        APP_TRACE("messageArrived(%d):\r\n r:%s\r\n", m->payloadlen, arMesg);
        APP_TRACE("======== user_update_message_arrive: %d ========\r\n", MfSdkSysGetTick());
        //xgui_PostMessage(XM_MESSAGEARRIVED, (unsigned int)arMesg, m->payloadlen);//send to main proc to play
        fifo_put_ptr((char*)arMesg);
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

    sprintf((char*)s_status, "Subscribe..");
    rc = MQTTSubscribeWithResults(c, topic, subsqos, mqtt_comm_messageArrived, &suback);
    APP_TRACE("rc from sub:%d>>%s\r\n", rc, topic);

    if ( rc == 0)
    {
        sprintf((char*)s_status, "Recving..");
    }
    else
    {
        sprintf((char*)s_status, "Subscribe failed,%d", rc);
    }
    return rc;
}

static int HMACcalculate(char *in, char* key, char * out)
{
    unsigned char md[32];     //32 bytes
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
    }
    APP_TRACE("set keepAliveInterval:%d\r\n", keepAliveInterval);
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
//#define	AWSMQTT_URL		"a2d911mzqj2e50-ats.iot.ap-south-1.amazonaws.com"
static char m_topic[128] = {0}; //make topic buffer big enough to receive
static char mqPassword[128] = { 0 };
#ifdef AWSMQTT_URL
static int AWS_mqtt_comm_run()
{
    int rc = -1;
    char ip[64] = { 0 };
    int port = 8883;

    get_setting_str(COMM_HOST_IP, ip, sizeof(ip));
    port = get_setting_int(COMM_HOST_PORT);

    if (strlen(ip) < 3)
    {
        strcpy(ip, AWSMQTT_URL);
        port = 8883;
    }
    MQTTPacket_connectData data1 = MQTTPacket_connectData_initializer;
    strcpy((char*)s_status, "Connecting..");

    APP_TRACE("Connecting\r\n");

    NetworkInit(&n);
    //Please use the certificate files of the device you created on the AWS platform
    rc = NetworkConnect_ssl(&n, ip, port, "xxxx\\ca.pem", "xxxx\\cli.crt", "xxxx\\pri.key");
    APP_TRACE("rc from net:%d\r\n", rc);

    if ( rc == 0 )
    {
        MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
        memset(&c, 0x00, sizeof(MQTTClient));
        MQTTClientInit(&c, &n, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));
        c.defaultMessageHandler = mqtt_comm_messageArrived;
        MfSdkSysGetTermSn((s8*)cid);

        /*data.willFlag = 0;
           data.MQTTVersion = 4;*/
        data.keepAliveInterval = get_keepAliveInterval();
        data.cleansession = 0;
        data.clientID.cstring = cid;
        data.username.cstring = cid;
        data.password.cstring = "";
        //topic: "/ota/<sn>/update"
        sprintf(m_topic, "/ota/%s/update", cid);
        APP_TRACE("m_topic:%s\r\n", m_topic);

        rc = MQTTConnect(&c, &data);

        s_client = &c;
        APP_TRACE("rc from connect:%d\r\n", rc);

        if ( rc == 0 )
        {
            rc = mqtt_subscribe_yield(&c, m_topic);
        }
    }
    return rc;
}

#else
#define MQTT_HOST       "test-mqtt.funicom.com.cn"
//#define MQTT_HOST	"sg-mqtt.funicom.com.cn"
#define MQTT_PORT       32517
static int mqtt_comm_run()
{
    int rc = -1;
    char ip[64] = { 0 };
    int port = MQTT_PORT;
    unsigned char hmac_key[17] = { 0 };     //"pyh5Dq0e6KN7fpxY";
    unsigned char hmac_secret[17] = { 0 };    //"s8XRrGiOfhNV4LlE";
    char hmac_payload[128] = {0};    //"clientId{202210310000001}.{202210310000001}202210310000001pX1s883V8ytT35Kx2524608000000";

    get_setting_str(COMM_HOST_IP, ip, sizeof(ip));
    port = get_setting_int(COMM_HOST_PORT);

    if (strlen(ip) < 3)
    {
        strcpy(ip, MQTT_HOST);
        port = MQTT_PORT;
    }
    strcpy((char*)s_status, "Connecting..");

    APP_TRACE("Connecting\r\n");

	APP_TRACE("port:%d\r\n",port);
	APP_TRACE("ip:%s\r\n",ip);

    NetworkInit(&n);
    rc = NetworkConnect(&n, ip, port);
    APP_TRACE("rc from net:%d\r\n", rc);

    if ( rc == 0 )
    {
        char willTopic[256] = { 0 };
        MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
        memset(&c, 0x00, sizeof(MQTTClient));
        MQTTClientInit(&c, &n, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));
        c.defaultMessageHandler = mqtt_comm_messageArrived;

        get_setting_str(PRODUCT_KEY,(char*)hmac_key, sizeof(hmac_key));

        if (strlen((char*)hmac_key) < 3)
        {
            strcpy((char*)hmac_key, "pFppbioOCKlo5c8E");
        }
        get_setting_str(PRODUCT_SECRET, (char*)hmac_secret, sizeof(hmac_secret));

        if (strlen((char*)hmac_secret) < 3)
        {
            strcpy((char*)hmac_secret, "sj2AJl102397fQAV");
        }
#ifdef WIN32
        strcpy(cid, "202302060000002");
#else
		MfSdkSysGetTermSn((s8*)cid);
#endif

        memset(hmac_payload, 0, sizeof(hmac_payload));
        sprintf(hmac_payload, "clientId{%s}.{%s}%s%s%s", cid, cid, cid, hmac_key, "2524608000000");
        HMACcalculate(hmac_payload,(char*)hmac_secret, mqPassword);

        data.keepAliveInterval = get_keepAliveInterval();
        data.cleansession = 0;
        data.clientID.cstring = cid;
        data.username.cstring = cid;
        data.password.cstring = mqPassword;
        sprintf(m_topic, "/ota/%s/%s/update", hmac_key, cid);
        APP_TRACE("m_topic:%s\r\n", m_topic);

        data.willFlag = 1;
        data.will.topicName.cstring = willTopic;
        data.will.message.cstring = "0";
        data.will.retained = 1;
        data.will.qos = 0;
        sprintf(willTopic, "/ota/device/connection/%s/state ", cid);

        rc = MQTTConnect(&c, &data);

        s_client = &c;
        APP_TRACE("rc from connect:%d\r\n", rc);

        if ( rc == 0 )
        {
            rc = mqtt_subscribe_yield(&c, m_topic);          //"MoreFunt3st");
        }
    }
    return rc;
}

#endif // AWSMQTT_URL



void mqtt_play_state(int nstate)
{
	MfSdkAudPlay((const s8*)(nstate? "sers.mp3" : "serf.mp3"));
}

int get_mqtt_con_state()
{
    return mqtt_con_state;
}

int mqtt_publish(char* payload)
{
    int res = 0;
    const char* fmt = "/ota/device/connection/%s/state";
    char* topic = NULL;
    int topic_len = 0;
    char deviceName[32] = { 0 };
    MQTTMessage msg;

#ifdef WIN32
    strcpy(deviceName, "202302060000001");
#else
    MfSdkSysGetTerminalSn((s8*)deviceName, sizeof(deviceName));
#endif

    if (MQTTIsConnected(s_client) != 1)
    {
        APP_TRACE("mqtt_publish error");
        return -2;
    }
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
        MfSdkMemFree(topic);
        return -1;
    }
    MfSdkMemFree(topic);
    return 0;
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
    static int mqtt_con_fail_state_flag = 0;
    static int mqtt_con_success_state_flag = 0;
	MFSDK_UNUSED(mqtt_con_fail_state_flag);

    APP_TRACE("mqtt_comm_task\r\n");
    memset(ticks, 0x00, sizeof(int) * ticks_count);

	MfSdkPowerTaskInit(TASK_PRIO);
//    power_task_init(TASK_PRIO);
    //Check the network for 60 seconds after startup. No network broadcast is abnormal
    ticks[net_state_player] = MfSdkSysTimerOpen(60 * 1000);    //Query network connection status
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
            //		    APP_TRACE("newnetstate================%d\r\n", netlinkstate);
            if (netlinkstate == 1)
            {
                //Network connection succeeded
                //	Sys_TtsPlay("nets.mp3");
                dev_open_flag = 1;
                APP_TRACE("netlinkstate success\r\n");
                //The network successfully starts connecting to the service immediately
                network_state = 1;

                if (ticks[iot_init_player] == 0)
                {
                    ticks[iot_init_player] = MfSdkSysTimerOpen(60 * 1000);                    //Query network connection status
                }
            }
            else
            {
                network_state = -1;
            }
        }
        else        //60s timeout
        {
            if (dev_open_flag == 0)
            {
                dev_open_flag = 1;
                if (MfSdkCommGetNetMode() == MFSDK_COMM_NET_ONLY_WIRELESS \
					&& /*atc_cpin() != 2*/ MfSdkCommAtcCpin(0) != MFSDK_COMM_ATCCPIN_NORMAL)//Judge whether there is a sim card
                {
					MfSdkAudPlay((const s8*)"nonsim.mp3");
                    APP_TRACE("nonsim\r\n");
                }
                else
                {
					MfSdkAudPlay((const s8*)"netf.mp3");
                    APP_TRACE("netf\r\n");
                }
            }
            ticks[net_state_player] = MfSdkSysTimerOpen(60 * 1000);//MfSdkSysTimerOpen(60 * 1000);            //Reset the timer			
        }

        if (network_state == 1)
        {
#ifdef AWSMQTT_URL
            ret = AWS_mqtt_comm_run();
#else
            ret = mqtt_comm_run();
#endif

            //			APP_TRACE("mqtt_comm_run ret=%d\r\n",ret);
            if (ret)
            {
                ticks[net_state_player] = MfSdkSysTimerOpen(60 * 1000);                //Reset the timer

                if (MfSdkSysTimerCheck(ticks[iot_init_player]) == 0)                //timeout
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
            else            //connect success
            {
                mqtt_publish("1");

                if (mqtt_con_success_state_flag == 0)
                {
                    mqtt_con_success_state_flag = 1;
                    mqtt_play_state(1);
                    APP_TRACE("mqtt connect success\r\n");
                }
                mqtt_con_state = 1;

                while ((s_client != 0) && MQTTIsConnected(s_client))
                {
                    if (MfSdkSysAppIsLock() == 1)                     //that there are other tasks
                    {
                        APP_TRACE("IOT_MQTT_Yield MfSdkSysAppIsLock() == 1 break\r\n");
                        break;
                    }

                    //Sleep Wake
                    if (s_power_resume == 1)
                    {
                        MQTTStartPing(s_client);
                        s_power_resume = 0;
                    }
                    ret = MQTTYield(s_client, 1000);                    //Send heartbeat packet

                    if(ret)
                    {
                        mqtt_con_state = 0;
                        APP_TRACE("MQTTYield break, ret= %d\r\n", ret);
                        break;
                    }

                    if (MQTTIsConnected(s_client))
                    {
                        if (MQTT_Keepalive_Probes(s_client) == 0)
                        {
                            MfSdkPowerTaskSuspend(TASK_PRIO, 500);
                        }
                        APP_TRACE("mqtt running......\r\n");
                    }
                }

                if(NULL != s_client)
                {
                    MQTTDisconnect(s_client);
                    APP_TRACE("mqtt Disconnect......\r\n");
                    s_client = 0;
                }
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

    memset(cid, 0x00, sizeof(cid));
    sprintf((char*)s_status, "Loading..");

	MfSdkSysSleep(1000);
	MfSdkPowerResumeProc(mqtt_power_resume_proc);
    nerr = MfSdkSysTaskCreate(mqtt_comm_task, 0, (s8*)&(pTaskStk[0]), _APP_TASK_SIZE);
    APP_TRACE("player_proc_init %d ====== %d\r\n", TASK_PRIO, nerr);
    return 0;
}

char* get_OrderSn()
{
    return OrderSn;
}

static void _message_close_text_page(lv_task_t* task)
{
    if (page_win != 0)
    {
        if (task_meaasge)
        {
            lv_task_del(task_meaasge);
            task_meaasge = 0;
        }

        if (lab_text)
        {
            lv_obj_del(lab_text);
            lab_text = 0;
        }

        if (lab_title)
        {
            lv_obj_del(lab_title);
            lab_title = 0;
        }
        lv_obj_del(page_win);
        page_win = 0;
    }
}

lv_obj_t* page_text_show(lv_obj_t* parent, char* title, char* text, int timeout)
{
    if (parent == NULL)
    {
        return NULL;
    }

    if (page_win != 0)
    {
        lv_obj_del(page_win);
        page_win = 0;
    }
    page_win = lv_img_create(parent, NULL);
    lv_obj_set_size(page_win, lv_obj_get_width(parent), lv_obj_get_height(parent));
    lab_title = page_create_title(page_win, title);
    lab_text = page_create_msg(page_win, text);

    if (timeout > 0)
    {
        task_meaasge = lv_task_create(_message_close_text_page, timeout, LV_TASK_PRIO_MID, 0);
    }
    return page_win;
}

static int op_callback = 0;
static long long llamont_callback = 0;;
void _TransactionProcCallbackFunction(int ret, char* recvdata)
{
    if (ret == HTTP_PROC_RUN )
    {
        return;
    }
    else if(ret == HTTP_PROC_FAIL)
    {
        lv_start_lock(1);
        page_text_show(get_mainpage(), "", "request timeout", 3000);
        APP_TRACE("HttpRequestFunc error!\r\n");
        lv_start_lock(0);
        return;
    }
    else if (ret == HTTP_PROC_SUCC)    //Process the received data
    {
        char amountStr[16] = { 0 };
        char temp[256] = { 0 };
        char sn[33] = { 0 };
        sprintf(amountStr, "%0.2f", llamont_callback / 100.0);
		MFSDK_UNUSED(sn);
        if (_getval(recvdata, "code", temp, sizeof(temp)) > 0 && 0 == strcmp(temp, "0000"))
        {
            APP_TRACE("code:%s\r\n", temp);

            if (_getval(recvdata, "order_sn", OrderSn, sizeof(OrderSn)) > 0)
            {
                //op=3:rf and qrcode, op=2:rf, op=1:qrcdeo
                APP_TRACE("OrderSn:%s op = %d\r\n", OrderSn, op_callback);

                if (1 == MfSdkNfcIsProbe() && op_callback == 2)
                {
                    char amt[64] = { 0 };

                    memset(amt, 0x00, sizeof(amt));
                    sprintf(amt, "%lld", llamont_callback);
                    lv_start_lock(1);
                    func_pay(get_mainpage(), amt);
                    lv_start_lock(0);
                }
                else if (op_callback == 4)
                {
                    //Determine if the camera is present
                    lv_start_lock(1);
                    ScanPage(get_mainpage(), amountStr);
                    lv_start_lock(0);
                }
                else
                {
                    lv_start_lock(1);
                    show_cardpage_qrcode(get_mainpage(), NULL, amountStr, OrderSn, 60 * 1000);
                    lv_start_lock(0);
                }
            }
        }
        else
        {
            APP_TRACE("error!\r\n");
        }
    }
}

#ifdef DEMO_OFFLINE
void TransactionProc(lv_obj_t* obj, long long llamont, int op)
{
    char amountStr[16] = {0};
    char sn[33] = {0};

    memset(OrderSn, 0, sizeof(OrderSn));
    sprintf(amountStr, "%0.2f", llamont / 100.0);

    if (strlen(amountStr) <= 8)
    {
		MfSdkGuiLedAmount((char*)amountStr);
    }
    APP_TRACE("[%s] AMT:%s\r\n", __FUNCTION__, amountStr);

#ifdef WIN32
    strcpy(OrderSn, "202302060000002");
#else
    MfSdkSysGetTermSn((s8*)OrderSn);
#endif

    //op=3:rf and qrcode, op=2:rf, op=1:qrcdeo
    APP_TRACE("OrderSn:%s op = %d\r\n", sn, op);

    if(1 == MfSdkNfcIsProbe() && op == 2)
    {
        char amt[64] = {0};

        memset(amt, 0x00, sizeof(amt));
        sprintf(amt, "%d", llamont);
        func_pay(obj, amt);
    }
    else if(op == 4)
    {
        //Determine if the camera is present
        ScanPage(obj, amountStr);
    }
    else
    {
        show_cardpage_qrcode(obj, NULL, amountStr, OrderSn, 60 * 1000);
    }
}

#else
void TransactionProc(lv_obj_t* obj, long long llamont, int op)
{
    char amountStr[16] = {0};
    char SendDate[128] = {0};
    char RecvDate[256] = {0};
    char temp[256] = {0};
    char sn[33] = {0};

    memset(OrderSn, 0, sizeof(OrderSn));
    sprintf(amountStr, "%0.2f", llamont / 100.0);

    if (strlen(amountStr) <= 8)
    {
        MfSdkGuiLedAmount((char*)amountStr);
    }
    APP_TRACE("[%s] AMT:%s\r\n", __FUNCTION__, amountStr);

    memset(temp, 0x00, sizeof(temp));
    memset(SendDate, 0x00, sizeof(SendDate));
    memset(RecvDate, 0x00, sizeof(RecvDate));
#ifdef WIN32
    strcpy(sn, "202302060000002");
#else
    MfSdkSysGetTermSn((s8*)sn);
#endif
    sprintf(SendDate, "device_id=%s&request_data=%s", sn, amountStr);

    if (MfSdkCommLinkState() == 0)
    {
        APP_TRACE("TransactionProc network fail\n");
        page_text_show(obj, "", "Network unstable", 3000);
        MfSdkAudPlay((const s8 *)"netf.mp3");
        return;
    }
    else if (get_mqtt_con_state() == 0)
    {
        APP_TRACE("TransactionProc connect fail\n");
        page_text_show(obj, "", "Connect fail", 3000);
        mqtt_play_state(0);
        return;
    }
#if 1
    op_callback = op;
    llamont_callback = llamont;
    HttpRequestSet("/api/Iotmsgtest/createQrMf", SendDate, _TransactionProcCallbackFunction);
#endif
#if 0

    if(HttpRequestFunc("/api/Iotmsgtest/createQrMf", SendDate, RecvDate) > 0)
    {
        if( _getval(RecvDate, "code", temp, sizeof(temp)) > 0 && 0 == strcmp(temp, "0000"))
        {
            APP_TRACE("code:%s\r\n", temp);

            if( _getval(RecvDate, "order_sn", OrderSn, sizeof(OrderSn)) > 0 )
            {
                //op=3:rf and qrcode, op=2:rf, op=1:qrcdeo
                APP_TRACE("OrderSn:%s op = %d\r\n", OrderSn, op);

                if(1 == mf_rfid_probe() && op == 2)
                {
                    char amt[64] = {0};

                    memset(amt, 0x00, sizeof(amt));
                    sprintf(amt, "%d", llamont);
                    //free_rfid_page();
                    func_pay(obj, amt);
                }
                else if(op == 4)
                {
                    //Determine if the camera is present
                    //if (mf_camera_present() == 1)
                    ScanPage(obj, amountStr);
                    //else
                    //page_text_show(obj, "", "Camera not exist", 3000);
                }
                else
                {
                    //show_page_qrcode(obj, amountStr, OrderSn);
                    show_cardpage_qrcode(obj, NULL, amountStr, OrderSn, 60 * 1000);
                }
            }
        }
        else
        {
            APP_TRACE("error!\r\n");
        }
    }
    else
    {
        page_text_show(obj, "", "request timeout", 3000);
        APP_TRACE("HttpRequestFunc error!\r\n");
    }
#endif
}

#endif

