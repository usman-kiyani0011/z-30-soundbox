#include "lvgl/lvgl.h"
#include <stdio.h>
#include "pages.h"
#include "../player_proc.h"
#include "http/AppHttpDemo.h"


static lv_obj_t* page_win = NULL;
static lv_obj_t* lab_title = NULL;
static lv_obj_t* lab_message = NULL;
static lv_obj_t* load_img = NULL;
static lv_task_t* image_task = NULL;

static char* m_title;
static st_comm_data* m_comm_data;
static int sock = -1;
static page_close_page_func m_page_close_page_func = 0;

static int g_angle = 0;
static int g_usetls = 0;
static int g_comm_busy = 0;

#define LV_EVENT_MESSAGE		(_LV_EVENT_LAST+1)
#define LV_EVENT_CLOSEPAGE		(LV_EVENT_MESSAGE+1)

int GetCommStatus(void)
{
	return g_comm_busy;
}

static void comm_close(MfSdkCommSocketIndex_E index)
{
	if(index >= 0)
	{
		if(1 == g_usetls)
		{
			MfSdkCommSslClose(index);
		}
		else
		{
			MfSdkCommSocketClose(index);
		}
	}
}

static void _comm_close_page(int ret)
{
	APP_TRACE("_comm_close_page ret = %d\r\n", ret);
	if(sock >= 0)
	{
		comm_close(sock);
		sock = -1;
	}
	if(NULL != image_task)
	{
		APP_TRACE("image_task delete\r\n");
        lv_task_del(image_task);
		image_task = 0;
	}

    if (page_win != NULL) 
	{
		if (NULL != lab_title)
		{
			lv_obj_del(lab_title);
			lab_title = 0;
		}
		if (NULL != lab_message)
		{
			lv_obj_del(lab_message);
			lab_message = 0;
		}
		if(NULL != load_img)
		{
			lv_obj_del(load_img);
			load_img = 0;
		}
		lv_free_png_file(LOADPNG);

		lv_obj_del(page_win);
        page_win = NULL;
		g_comm_busy = 0;
		if (m_page_close_page_func != 0) m_page_close_page_func(ret,NULL);
        m_page_close_page_func = 0;
    }

	return;
}

static void _comm_message_update(char* title, char* message)
{
	if(lab_title)
	{
		lv_label_set_text(lab_title, title);
	}
	if(lab_message)
	{
		lv_label_set_text(lab_message, message);
		lv_obj_align(lab_message, NULL, LV_ALIGN_CENTER, 0, 0);
	}
}

static void _comm_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t ret = -1;
	const void *data = lv_event_get_data();

	APP_TRACE("LV_EVENT = %d\r\n", e);
	if (e == LV_EVENT_MESSAGE)
	{
		APP_TRACE("data = %s\r\n", data);
		_comm_message_update(m_title, (char*)data);
	}
	else if (e == LV_EVENT_CLOSEPAGE)
	{
		memcpy(&ret, data, sizeof(uint32_t));
		APP_TRACE("LV_EVENT_CLOSEPAGE = %d\r\n", ret);
	 	_comm_close_page(ret);
	}
	return;
}

#if 0
static int _connect_func()
{
	char msg[64] = { 0 };
	int ret = 0;
	int num = 0;
 
	num = MfSdkSysTimerCheck(m_connect_tick) / 1000;

	if (num <= 0) 
	{
		ret = 1;
	}
	else if (m_exit_flag == 0) 
	{
		sprintf(msg, "Connecting(%d)", num);
        _comm_message_update(m_title, msg);
	}
	else 
	{
		sprintf(msg, "Canceling...");
        _comm_message_update(m_title, msg);
		ret = 2;
	}

	return ret;
}

static int _comm_link_func()
{
    int sec;
    char msg[64] = {0};
    sec = MfSdkSysGetTickDiff(tick_start) / 1000;
    sprintf(msg ,"net link:%d", sec + 1);
	lv_start_lock(1);
	_comm_message_update(m_title, msg);
	lv_start_lock(0);

	return 0;
}
#endif

s32 AppEventSend(lv_obj_t * obj, lv_event_t event, const void * data)
{
	lv_res_t res = LV_RES_INV;

	if(obj != NULL)
	{
		lv_start_lock(1);
		res = lv_event_send(obj,event,data);
		lv_start_lock(0);
	}
	return (res == LV_RES_OK) ? 0 : MFSDK_RET_FAILED;
}

static int http_GetContentLength(char *heads)
{
    const char *keystr = "Content-Length";
    char *pContentLength = strstr(heads, keystr);

    if ( pContentLength != 0)
    {
        pContentLength += strlen(keystr);
        pContentLength += strlen(": ");
        return atoi(pContentLength);
    }
    return -1;
}

static int _comm_recv(int sock , char * buff, int len)
{
    unsigned int tick1 = 0;
    int ret = 0;
    int num = 0;
    int index = 0;
    char msg[64] = { 0 };
	m_comm_data->timeover = 60*1000;

    tick1 = MfSdkSysTimerOpen(m_comm_data->timeover);
    while (MfSdkSysTimerCheck(tick1) > 0) 
	{
		num = MfSdkSysTimerCheck(tick1) / 1000;
		num = num < 0 ? 0 : num;
		snprintf(msg, sizeof(msg), "Please wait(%ds)", num);
        //_comm_message_update(m_title, msg);
		AppEventSend(page_win, LV_EVENT_MESSAGE, msg);

		ret = MfSdkCommSocketRecv(sock, (u8*)(buff + index), len - index, 700);
		APP_TRACE("_comm_recv ret = %d\r\n", ret);
        if (ret < 0) 
		{
            index = -1;
            break;
        }
        else if (ret == 0) 
		{

        }
        else 
		{
            index += ret;
            if (buff[index - 1] == 0x0a && buff[index - 2] == 0x0d) 
			{
                break;
            }
			else
			{
				int ContentLength = http_GetContentLength(buff);
				APP_TRACE("ContentLength = %d\r\n", ContentLength);
				if(ContentLength > 0)
				{
					char *p = strstr(buff, "\r\n\r\n");
					if(NULL != p && ContentLength == strlen(p+4))
					{
						break;
					}
				}
			}
        }
    }
	APP_TRACE("_comm_recv index = %d\r\n", index);
    return index;
}

static int comm_connect(int index)
{
    int ret = -1;
    int port = HTTP_PORT;
    char ip[64] = { 0 };

    get_setting_str(COMM_HTTP_IP, ip, sizeof(ip));
    if (strlen(ip) < 3)
    {
        strcpy(ip, HTTP_HOST);
        port = HTTP_PORT;
    }
	else
	{
	    port = get_setting_int(COMM_HTTP_PORT);
	}
	
    sock = MfSdkCommSocketCreate(index);	//  Create sock
    APP_TRACE("sock = %d\r\n", sock);

    APP_TRACE("[%s] ip = %s, port = %d\r\n",  __FUNCTION__, ip, port);		
	if(1 == g_usetls)
	{
	    MfSdkCommSslInit(sock, NULL, NULL, NULL, 0);
	    ret = MfSdkCommSslConnect(sock, (s8*)ip, (s32)port, NULL);
	    APP_TRACE("MfSdkCommSslConnect = %d\r\n", ret);
	}
	else
	{
		ret = MfSdkCommSocketConnect(sock, (s8 *)ip, (s32)port, 20*1000, NULL);
	    APP_TRACE("MfSdkCommSocketConnect = %d\r\n", ret);
	}
    if( ret < 0 )
    {
        APP_TRACE("connect fail\n");
        comm_close(sock);
		sock = -1;
    }

	return ret;
}

void comm_disconnect(void)
{
	#if COMM_PRECONNECT
	APP_TRACE("[%s] sock = %d\r\n", __FUNCTION__, sock);
	if(sock >= 0)
	{
		comm_close(sock);
		sock = -1;
	}
	#endif
}

int comm_preconnect()
{
	int ret = -1;
	#if COMM_PRECONNECT
	APP_TRACE("[%s] \r\n", __FUNCTION__);
	ret = comm_connect(MFSDK_COMM_SOCKET_INDEX_3);
	if(0 != ret)
	{
		comm_disconnect();
	}
	#else
	ret = 0;
	#endif
	return ret;
}

static void _comm_task(lv_task_t *pTask)
{
    int ret = -1;
    int comm_ret = -1;

	g_usetls = 0;
    APP_TRACE("[%s]\r\n",__FUNCTION__);
	
	do{
		if(sock < 0)
		{
			ret = comm_connect(MFSDK_COMM_SOCKET_INDEX_3);
		    if (0 != ret) 
			{
		        APP_TRACE("socket fail\n");
				comm_ret = COMM_RET_FAIL_CONECT;
				break;
		    }
		}

		//AppEventSend(page_win, LV_EVENT_MESSAGE, "Send data...");
		ret = MfSdkCommSocketSend(sock, (u8*)m_comm_data->sbuff, m_comm_data->slen);
		APP_TRACE("SocketSend[%d] ret= %d\r\n", m_comm_data->slen, ret);
		APP_TRACE_BUFF_TIP(m_comm_data->sbuff, m_comm_data->slen, "send:");
		if( ret < 0 )
		{
			APP_TRACE("send fail ret= %d\r\n", ret);
			comm_ret = COMM_RET_FAIL_SEND;
			break;
		}

		AppEventSend(page_win, LV_EVENT_MESSAGE, "Recv data...");

		char RecBuf[1024 * 4] = { 0 };
		AppHttpSession_T session;
		memset(&session, 0, sizeof(AppHttpSession_T));

		session.buffer = RecBuf;
		session.size = sizeof(RecBuf);
		session.recv_timeout_ms = 3 * 1000;
		session.recv_resp_timeout_ms = 30 * 1000;
		session.content_length = -1;
		session.socket = sock;
		session.begin_ticket = MfSdkSysGetTick();

		ret = AppHttpHandleResponse(&session); //GET status code
		if (ret != 200)
		{
			APP_TRACE("AppHttpHandleResponse status code : %d \r\n" , ret);  
			break;
		}

		session.begin_ticket = MfSdkSysGetTick();
		ret = AppHttpRecvResponseContent(&session, m_comm_data->rbuff, RECV_BUFF_SIZE);
		APP_TRACE("AppHttpRecvResponseContent retrun : %d \r\n" , ret);
		if(ret > 0) 
		{
			m_comm_data->rlen = ret;
			comm_ret = COMM_RET_SUCC;
			APP_TRACE_BUFF_TIP(m_comm_data->rbuff, ret, "recv_buf"); 
		}
	}while(0);
	
	APP_TRACE("[%s][comm_net_link ret:%d]\r\n",__FUNCTION__, comm_ret);
	AppEventSend(page_win, LV_EVENT_CLOSEPAGE, &comm_ret);
	
    return;
}

static void load_task_func(lv_task_t *pTask)
{
	g_angle += 400;
	if (g_angle > 3600)
	{
		g_angle = 0;
	}
	lv_img_set_angle(load_img, g_angle);
	return;
}

static void _comm_task_show(lv_task_t *pTask)
{
	int ret = COMM_RET_SUCC;
	
	AppEventSend(page_win, LV_EVENT_MESSAGE, "Connect server...");
	MfSdkSysSleep(800);
	AppEventSend(page_win, LV_EVENT_MESSAGE, "Send data...");
	MfSdkSysSleep(800);
	AppEventSend(page_win, LV_EVENT_MESSAGE, "Recv data...");
	MfSdkSysSleep(800);
	AppEventSend(page_win, LV_EVENT_CLOSEPAGE, &ret);
	
    return;
}

lv_obj_t* page_comm_show_auto(lv_obj_t* parent, void* pfunc, char* title, st_comm_data* comm_data,int mode)
{
    char countdown[32] = { 0 };
	m_comm_data = comm_data;
	m_title = title;
	g_angle = 0;
	g_comm_busy = 1;

	m_page_close_page_func = (page_close_page_func)pfunc;

	page_win = page_create_win(parent, _comm_event_cb);
	lab_title = page_create_title(page_win, title);
	page_ShowTextOut(page_win, "Connecting...", LV_ALIGN_CENTER, 0, -30, LV_COLOR_BLACK, LV_FONT_24);
    sprintf(countdown, "Please wait(%ds)", m_comm_data->timeover / 1000);
	lab_message = page_ShowTextOut(page_win, countdown, LV_ALIGN_CENTER, 0, 0, LV_COLOR_BLACK, LV_FONT_24);
	#if 0
	lv_load_png_file(LOADPNG);
	load_img = lv_img_create(page_win, NULL);
	lv_img_set_src(load_img, LOADPNG);
	lv_obj_align(load_img, NULL, LV_ALIGN_CENTER, 0, -30);//80
	
	image_task = lv_task_create(load_task_func, 1000, LV_TASK_PRIO_LOW, 0);		
	#endif

	if(1 == mode)
	{
		MfSdkSysTaskAppSet(_comm_task_show);
	}
	else if(2 == mode)
	{
		MfSdkSysTaskAppSet(_comm_task);
	}

	return page_win;
}

