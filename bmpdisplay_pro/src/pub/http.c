#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include "libapi_xpos/inc/libapi_system.h"
#include "pub/common/misc/inc/mfmalloc.h"
#include "../tracedef.h"
#include "http.h"
#include "player_proc.h"
#include "libapi_xpos/inc/mfsdk_sys.h"
#include "libapi_xpos/inc/mfsdk_aud.h"
#include "libapi_xpos/inc/mfsdk_comm.h"
#include "libapi_xpos/inc/mfsdk_mem.h"
#include "libapi_xpos/inc/mfsdk_power.h"
#include "../page_main.h"
#include "../sdk_tts.h"
#include "../card/func_pay.h"
#include "../pages/page_http.h"
//#include "../pages/pages.h"
#define HTTP_OK 200



// Parsing the status code of http
static int http_StatusCode(char *heads)
{
    char *rescode = (char *)strstr(heads, " ");

    if ( rescode != 0 )
    {
        rescode += 1;
        return atoi(rescode);
    }
    // return -1;
    return 0;
}

#define HTTP_HOST                       "test-iot.morefun-et.com"
//#define HTTP_HOST			"sg-iot.morefun-et.com"
int HttpRequestFunc(char *urlpath, char *SendDate, char *RecData)
{
#define  RECV_LEN 1024

    int m_comm_sock = 0;
    int c_ret  = 0;
    int ret = 0;
    int port = 80;
    char *ip = HTTP_HOST;
    char *SendBuf = NULL;
    char *RecvUrl = NULL;
    int index = 0;
//    int recv_len = 0;

    if (MfSdkCommLinkState() == 0)
    {
        APP_TRACE("HttpRequestFunc network fail\n");
        return -1;
    }
    m_comm_sock = MfSdkCommSocketCreate(MFSDK_COMM_SOCKET_INDEX_1);

    c_ret = MfSdkCommSocketConnect(m_comm_sock, ip, port, 3000, NULL);

//	c_ret = comm_sock_connect_timeout(m_comm_sock, ip, port, 3000);
    APP_TRACE("m_comm_sock=%d, ret = %d, urlpath=%s\r\n", m_comm_sock, c_ret, urlpath);

    if( c_ret < 0 )
    {
        APP_TRACE("socket fail\n");
//		comm_sock_close(m_comm_sock);
        MfSdkCommSocketClose(m_comm_sock);
        return -1;
    }

    do
    {
        SendBuf = (char *)MfSdkMemMalloc(512);
        memset(SendBuf, 0, 512);

        index += sprintf(SendBuf + index, "POST %s?%s HTTP/1.1\r\n", urlpath, SendDate);
        index += sprintf(SendBuf + index, "Host: %s:%d\r\n", ip, port);
        //index += sprintf(SendBuf + index , "Content-Length:%d\r\n", strlen(data));
        //index += sprintf(SendBuf + index , "Content-Type: multipart/form-data\r\n");
        //index += sprintf(SendBuf + index , "Connection: keep-alive\r\n");
        //index += sprintf(SendBuf + index , "Accept: application/json\r\n");
        index += sprintf(SendBuf + index, "\r\n");

        APP_TRACE("send SendBuf=%s\n\n", SendBuf);
//		ret = comm_sock_send(m_comm_sock, SendBuf , strlen(SendBuf));	//Send http request
        ret = MfSdkCommSocketSend(m_comm_sock, (unsigned char*)SendBuf, strlen(SendBuf));

        if( ret < 0 )
        {
            APP_TRACE("send fail\n");
            ret = -1;
            break;
        }
        RecvUrl = (char *)MfSdkMemMalloc(RECV_LEN);
        memset(RecvUrl, 0, RECV_LEN);
//		ret = comm_sock_recv( m_comm_sock, (unsigned char *)RecvUrl, RECV_LEN , 1000);//Recv http
        ret = MfSdkCommSocketRecv(m_comm_sock, (unsigned char *)RecvUrl, RECV_LEN, 1000);          //Recv http

        if( ret <= 0 )
        {
            APP_TRACE("recv fail\n");
            ret = -1;
            break;
        }
        APP_TRACE("recv RecvUrl=%s\r\n", RecvUrl);

        if (HTTP_OK == http_StatusCode(RecvUrl))
        {
            char *p = strstr(RecvUrl, "\r\n\r\n");

            if(p != 0)          // Remove Header
            {
                ret = strlen(p + 4);
                memcpy(RecData, p + 4, ret);                    //	(-2) for taking in last \r\n
                RecData[strlen(p + 4)] = '\0';
                APP_TRACE("RecData = %s\r\n", RecData);
            }
        }
    }
    while(0);

    if( SendBuf != NULL)
    {
        MfSdkMemFree(SendBuf);
    }

    if( RecvUrl != NULL)
    {
        MfSdkMemFree(RecvUrl);
    }
//	comm_sock_close(m_comm_sock);
    MfSdkCommSocketClose(m_comm_sock);
    APP_TRACE("HttpRequestFunc ret = %d\r\n", ret);

    return ret;
}

#ifdef WIN32
#define ATOLL   _atoi64
#else
#define ATOLL   atoll
#endif

void play_voice(long long money)
{
    set_counter_led();
    MfSdkAudPlay((const s8 *)"pay.mp3");
    //sdk_tts_play_amt_india(money);
    //if (moneylen <= 7)
    {
        if (money >= 100)
        {
            sdk_tts_play_amt_en(money / 100);
            MfSdkAudPlay((const s8 *)"dollar.mp3");

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
                MfSdkAudPlay((const s8 *)"cents.mp3");
            }
        }
    }
    //else
    //	pub_tts_play("success.mp3");
}

//http callback function
void _SendTransResult_callback(int ret)
{
    if (ret > 0)
    {
        page_text_show(get_mainpage(), (char*)"pay", (char*)"success", 1000);
        APP_TRACE("SendTransResult success!\r\n");
    }
    else
    {
        page_text_show(get_mainpage(), (char*)"pay", (char*)"request timeout", 1000);
        APP_TRACE("SendTransResult error!\r\n");
    }
}

#ifdef DEMO_OFFLINE
void SendTransResult()
{
//	power_lock_app("http");
    MfSdkPowerLockApp((char*)"http");
    show_page_result(0, 0);
    play_voice(ATOLL(get_amount()));
//	power_unlock_app();
    MfSdkPowerUnlockApp();
}

#else
void SendTransResult()
{
    char SendDate[128] = {0};
    char RecvDate[256] = {0};
    char *pQrcode = get_OrderSn();

    MfSdkPowerLockApp((char*)"http");
    APP_TRACE("pQrcode = %s\r\n", pQrcode);

    if(strlen(pQrcode) > 0)
    {
        memset(SendDate, 0x00, sizeof(SendDate));
        memset(RecvDate, 0x00, sizeof(RecvDate));

        sprintf(SendDate, "order_sn=%s", pQrcode);

        if (HttpRequestFunc((char*)"/api/Iotmsgtest/pushmsgMf", SendDate, RecvDate) > 0)
        {
            page_text_show(get_mainpage(), (char*)"pay", (char*)"success", 1000);
            APP_TRACE("SendTransResult success!\r\n");
        }
        else
        {
            page_text_show(get_mainpage(), (char*)"pay", (char*)"request timeout", 1000);
            APP_TRACE("SendTransResult error!\r\n");
        }
    }
    else
    {
        show_page_result(0, 0);
        play_voice(ATOLL((const char*)get_amount()));
    }
    MfSdkPowerUnlockApp();
}

#endif

//for http test
#define APP_SOCK_INDEX  1
int myhost_test()
{
#define  RECV_LEN 1024

    int m_comm_sock = 0;
    int c_ret  = 0;
    int ret = 0;
    int port = 443;
    char *ip = "iidyllicsoft.com";
    char *SendBuf = NULL;
    char *RecvUrl = NULL;
    int index = 0;
//    int recv_len = 0;
    char sn[33] = {0};
    char data[128] = {0};
//    char temp[256] = {0};

    APP_TRACE("myhost_test\r\n");
    //m_comm_sock = mf_sock_create();
    m_comm_sock = MfSdkCommSocketCreate(APP_SOCK_INDEX);    //create socket
//	comm_ssl_init(m_comm_sock ,NULL,NULL,NULL,0);
    MfSdkCommSslInit(m_comm_sock, NULL, NULL, NULL, 0);
    //c_ret = mf_sock_connect(m_comm_sock , ip , port);
//	c_ret = comm_ssl_connect(m_comm_sock, ip, port);	//Connect to http server
    c_ret = MfSdkCommSslConnect(m_comm_sock, ip, port, NULL);
    APP_TRACE("===============comm_ssl_connect c_ret = %d\r\n", c_ret);

    if( c_ret < 0 )
    {
        APP_TRACE("socket fail\n");
        MfSdkCommSslClose(m_comm_sock);
        return -1;
    }
#ifdef WIN32
    strcpy(sn, "66220601359001");
#else
//	Sys_GetTermSn(sn);
    MfSdkSysGetTermSn((s8*)sn);
#endif

    do
    {
        sprintf(data, "sn=%s&money=%s&seq_no=%s", sn, "1", "20230418191563");
        SendBuf = (char *)MfSdkMemMalloc(512);
        memset(SendBuf, 0, 512);
        index += sprintf(SendBuf + index, "GET %s/callback/sandbox/dynamic/qr/api.php?%s HTTP/1.1\r\n", ip, data);
        index += sprintf(SendBuf + index, "Host: %s:%d\r\n", ip, port);
        //index += sprintf(SendBuf + index , "Content-Length:%d\r\n", msgl);
        //index += sprintf(SendBuf + index , "User-Agent: Fiddler\r\n");
        //index += sprintf(SendBuf + index , "Accept: application/json\r\n");
        index += sprintf(SendBuf + index, "\r\n");


        APP_TRACE("send SendBuf=%s\n\n", SendBuf);
        //ret = mf_sock_send(m_comm_sock, SendBuf , strlen(SendBuf));
//		ret = comm_sock_send(m_comm_sock , SendBuf ,  strlen(SendBuf));//Send http request
        ret = MfSdkCommSocketSend(m_comm_sock, (u8*)SendBuf, strlen(SendBuf));      //Send http request

        if( ret < 0 )
        {
            APP_TRACE("send fail\n");
            ret = -1;
            break;
        }
        RecvUrl = (char *)MfSdkMemMalloc(RECV_LEN);
        memset(RecvUrl, 0, RECV_LEN);
        //ret = mf_sock_recv( m_comm_sock, RecvUrl, RECV_LEN , 700);
//		ret = comm_sock_recv( m_comm_sock, (unsigned char *)(RecvUrl), RECV_LEN, 3000 );//Recv http
        ret = MfSdkCommSocketRecv(m_comm_sock, (u8*)RecvUrl, RECV_LEN, 3000);

        if( ret < 0 )
        {
            APP_TRACE("recv fail\n");
            ret = -1;
            break;
        }
        char *p = NULL;
        int nRespCode = http_StatusCode(RecvUrl);

        if (HTTP_OK == nRespCode)
        {
            p = strstr(RecvUrl, "\r\n\r\n");

            if(p != NULL)               // Remove Header
            {
                memset(data, 0, sizeof(data));
                memcpy(data, p + 4, strlen(p + 4));
                data[strlen(p + 4)] = '\0';
            }
        }
    }
    while(0);

    if( SendBuf != NULL)
    {
        MfSdkMemFree(SendBuf);
    }

    if( RecvUrl != NULL)
    {
        MfSdkMemFree(RecvUrl);
    }
    //mf_sock_close(m_comm_sock);
//	comm_ssl_close(m_comm_sock);

    MfSdkCommSslClose(m_comm_sock);

    return ret;
}

void _HttpCommStop()
{
    //HttpPtr.isHttpExist = 0;
    HttpRequestFree();
}

int HttpRequestSet(char* urlpath, char* SendDate, HttpCallbackFunction callbackfunction)
{
    if (HttpPtr.isHttpExist == 1)
    {
        callbackfunction(HTTP_PROC_RUN, NULL);
    }
    //free_http_message_page();
    page_http_message_show(get_mainpage(), _HttpCommStop, (char*)"Tip", (char*)"http communication,pls wait...", (char*)"", (char*)"", 60 * 1000);
    char* urlpath_ptr = (char*)MfSdkMemMalloc(strlen(urlpath) + 1);
    char* SendDate_ptr = (char*)MfSdkMemMalloc(strlen(SendDate) + 1);
    char* RecData_ptr = (char*)MfSdkMemMalloc(HTTP_BLOCK + 1);
    memset(urlpath_ptr, 0, strlen(urlpath) + 1);
    memset(SendDate_ptr, 0, strlen(SendDate) + 1);
    memset(RecData_ptr, 0, HTTP_BLOCK + 1);
    //call callbackfunction when http fisish
    strcpy(urlpath_ptr, urlpath);
    strcpy(SendDate_ptr, SendDate);
    HttpPtr.urlpath_ptr = urlpath_ptr;
    HttpPtr.SendDate_ptr = SendDate_ptr;
    HttpPtr.RecData_ptr = RecData_ptr;
    HttpPtr.callbackfunction = callbackfunction;
    HttpPtr.isHttpExist = 1;

    return 0;
}

int HttpRequestFree()
{
    HttpPtr.callbackfunction = NULL;
    HttpPtr.isHttpExist = 0;

    if (HttpPtr.urlpath_ptr)
    {
        MfSdkMemFree(HttpPtr.urlpath_ptr);
        HttpPtr.urlpath_ptr = NULL;
    }

    if (HttpPtr.SendDate_ptr)
    {
        MfSdkMemFree(HttpPtr.SendDate_ptr);
        HttpPtr.SendDate_ptr = NULL;
    }

    if (HttpPtr.RecData_ptr)
    {
        MfSdkMemFree(HttpPtr.RecData_ptr);
        HttpPtr.RecData_ptr = NULL;
    }
    return 0;
}

static void http_proc_task(void* p)
{
//    int ret = 0;
//    char* request_id = NULL;

//	power_task_init(HTTP_TASK_PRIO);

    MfSdkPowerTaskInit(HTTP_TASK_PRIO);

    while (1)
    {
        if (/*app2_is_lock()*/ MfSdkSysAppIsLock() == 1)
        {
            MfSdkSysSleep(1000);
            continue;
        }

        if (HttpPtr.isHttpExist == 1)
        {
            HttpIdleflag = 1;
            int ret = 0;
            ret = HttpRequestFunc(HttpPtr.urlpath_ptr, HttpPtr.SendDate_ptr, HttpPtr.RecData_ptr);

            if (HttpPtr.isHttpExist == 0)            //Actively close
            {
            }
            else if (ret <= 0)
            {
                HttpPtr.callbackfunction(HTTP_PROC_FAIL, NULL);
            }
            else
            {
                HttpPtr.callbackfunction(HTTP_PROC_SUCC, HttpPtr.RecData_ptr);
            }
            free_http_message_page();
            HttpRequestFree();
            HttpIdleflag = 0;
			MFSDK_UNUSED(HttpIdleflag);
        }

        if (HttpIdleflag == 0)        //Device http proc idle
        {
//			power_task_suspend(HTTP_TASK_PRIO, 500);

            MfSdkPowerTaskSuspend(HTTP_TASK_PRIO, 500);
        }
        MfSdkSysDelay(1000);
    }
}

#define _APP_TASK_SIZE 4096
static unsigned int pTaskStk1[_APP_TASK_SIZE];
int http_proc_init()
{
    int nerr = 0;

    nerr = MfSdkSysTaskCreate(http_proc_task, 1, (s8*)&(pTaskStk1[0]), _APP_TASK_SIZE);
    APP_TRACE("iot_feedback_proc_init %d ====== %d\r\n", HTTP_TASK_PRIO, nerr);
    return 0;
}

