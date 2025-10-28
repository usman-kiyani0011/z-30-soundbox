#include "lvgl/lvgl.h"
#include <stdio.h>
#include "pages.h"
//#include "driver/mf_magtek.h"
#include "../player_proc.h"


static lv_obj_t* page_win = NULL;
static lv_obj_t* lab_title = NULL;
static lv_obj_t* lab_message = NULL;
static lv_obj_t* btn_canel = NULL;
static lv_obj_t* btn_confirm = NULL;
static char* m_title;
static st_comm_data* m_comm_data;
//static int m_size;
static int m_exit_flag = 0;
static int m_connect_tick = 0;
static unsigned int tick_start = 0;
static lv_task_t* task = NULL;
static volatile int page_comm_busyflag = 0;
static page_close_page_func m_page_close_page_func = 0;
int get_page_comm_busyflag()
{
    return page_comm_busyflag;
}
static void _comm_close_page(int ret)
{
    if (page_win != NULL) {
		lv_obj_del(page_win);
        page_win = NULL;
		if (m_page_close_page_func != 0) m_page_close_page_func(ret,NULL);
        m_page_close_page_func = 0;
    }
    page_comm_busyflag = 0;
}
void free_comm_page()
{
    _comm_close_page(COMM_RET_CANCEL);
}
static void _comm_btn_event_cb(struct _lv_obj_t* obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
       if (obj == btn_canel) {
           m_exit_flag = 1;
       }
       else if (obj == btn_confirm) {
       }
    }
}

static void _comm_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t key;

    if (e == LV_EVENT_KEY) {
        key = page_get_key(0);

		MFSDK_UNUSED(key);
#if 0
        if (key == MF_KEY_ESC){
            if (btn_canel != 0) {
                m_exit_flag = 1;
            }
        }
        else if (key == MF_KEY_ENTER) {
            if (btn_confirm != 0) {

            }
        }
#endif
    }
}

static void _comm_message_update(char* title, char* message)
{
	lv_label_set_text(lab_title, title);
	lv_label_set_text(lab_message, message);
}

static int _connect_func()
{
	char msg[64] = { 0 };
	int ret = 0;
	int num = 0;
 
	num = MfSdkSysTimerCheck(m_connect_tick) / 1000;

	if (num <= 0) {
		ret = 1;
	}
	else if (m_exit_flag == 0) {
		sprintf(msg, "Connecting(%d)", num);
        _comm_message_update(m_title, msg);
	}
	else {
		sprintf(msg, "Canceling...");
        _comm_message_update(m_title, msg);
		ret = 2;
	}


	return ret;
}


static int _comm_recv(int sock , char * buff, int len)
{
    unsigned int tick1;
    int ret;
    int num;
    char msg[64] = { 0 };
    int index = 0;

    tick1 = MfSdkSysTimerOpen(m_comm_data->timeover);
    while (MfSdkSysTimerCheck(tick1) > 0) {
		num = MfSdkSysTimerCheck(tick1) / 1000;
		num = num < 0 ? 0 : num;
		sprintf(msg, "%s(%d)", "recv", num);
        _comm_message_update(m_title, msg);

		ret = MfSdkCommSocketRecv(sock, (u8*)(buff + index), len - index, 700);
//        ret = comm_sock_recv(sock, (unsigned char*)(buff + index), len - index, 700);
        if (ret < 0) {
            index = -1;
            break;
        }
        else if (ret == 0) {

        }
        else {
            index += ret;
            if (buff[index - 1] == 0x0a && buff[index - 2] == 0x0d) {
                break;
            }
        }
    }

    return index;
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

static int _comm_task()
{   
	#if 0
    int ret = 0;
    int sock;
    char host[32] = {0};
    int port;

    tick_start = osl_GetTick();

    ret = comm_net_link(_comm_link_func, "", m_comm_data->timeover);

    if (ret != 0) {
		lv_start_lock(1);
        _comm_close_page(COMM_RET_FAIL_LINK);
		lv_start_lock(0);
        return 0;
    }

    sock = comm_sock_create(1);	//  Create sock

    m_connect_tick = Sys_TimerOpen(m_comm_data->timeover);
	lv_start_lock(1);
	_comm_message_update(m_title, "connect server...");
	lv_start_lock(0);



    char  *host_def = "120.40.104.194";
    read_profile_string("set", "host", host, sizeof(host), "120.40.104.194", "exdata\\app.ini");
    if (strcmp(host, host_def) == 0) {
        write_profile_string("set", "host", host_def, "exdata\\app.ini");
    }

    int port_def = 6602;
    port = read_profile_int("set", "port", port_def, "exdata\\app.ini");
    if (port == port_def) {
        write_profile_int("set", "port", port_def, "exdata\\app.ini");
    }

    
    ret = comm_sock_connect2(sock, host, port, _connect_func);
    if (ret != 0) {
        comm_sock_close(sock);
		lv_start_lock(1);
		_comm_close_page(COMM_RET_FAIL_CONECT);
		lv_start_lock(0);
		return 0;
    }

	lv_start_lock(1);
    _comm_message_update(m_title, "send data...");
	lv_start_lock(0);

    comm_sock_send(sock, m_comm_data->sbuff, m_comm_data->slen);

    ret = _comm_recv(sock, m_comm_data->rbuff, RECV_BUFF_SIZE);

    comm_sock_close(sock);

	lv_start_lock(1);
    if (ret <= 0) {
        _comm_close_page(COMM_RET_FAIL_RECV);
    }
    else {
        m_comm_data->rlen = ret;
        _comm_close_page(COMM_RET_SUCC);
    }
    lv_start_lock(0);
	#endif
	return 0;
}
static void back_navigation_event_cb(lv_obj_t* btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		if (!btn) return;
		m_exit_flag = 1;
	}
}

lv_obj_t* page_comm_show(lv_obj_t* parent , void * pfunc , char * title, st_comm_data* comm_data,int show_back)
{

    m_comm_data = comm_data;
    m_title = title;

    m_page_close_page_func = (page_close_page_func)pfunc;

    page_win = page_create_win(parent, _comm_event_cb);
    lab_title = page_create_title(page_win, title);
	lab_message = page_create_msg(page_win, "net link....");

	if(show_back)
		page_create_navigation_back(page_win,back_navigation_event_cb);

    //Sys_TaskCreate(_comm_task, 0, 0, 0);

	//btn_canel = page_create_btn(page_win, "cancel", _comm_btn_event_cb, PAGE_BTN_ALIGN_LEFT);
   
    return page_win;
}

#define SET_INI_SECTION           "set"
#define SET_INI_FILENAME  "exdata\\setting.ini"
static void _comm_task_auto(lv_task_t *pTask)
{
    int ret = -1;
    int sock = 0;
    int index = 0;
    char host[32] = { 0 };
    int port = 80;

    APP_TRACE("[%s]\r\n",__FUNCTION__);
    _comm_close_page(COMM_RET_SUCC);
	return ;
    //strcpy(host, "test-iot.morefun-et.com");
//    read_profile_string(SET_INI_SECTION, "host", host, sizeof(host), "test-iot.morefun-et.com", SET_INI_FILENAME);
//	port = read_profile_int(SET_INI_SECTION, "port", 80, SET_INI_FILENAME);

	MfSdkFsReadProfileString((const s8*)SET_INI_SECTION,(const s8*)"host",(s8*)host, sizeof(host),(const s8*)"test-iot.morefun-et.com",(const s8*)SET_INI_FILENAME);
	port = MfSdkFsReadProfileInt((const s8*)SET_INI_SECTION,(const s8*)"port", 80,(const s8*)SET_INI_FILENAME);

    sock = MfSdkCommSocketCreate(1);	//  Create sock
    //m_connect_tick = Sys_TimerOpen(m_comm_data->timeover);
	do{
	    _comm_message_update(m_title, "connect server...");
	    APP_TRACE("[%s] ip = %s, port = %d\r\n",  __FUNCTION__, host, port);		
//	    ret = comm_sock_connect(sock, host, port);
		ret = MfSdkCommSocketConnect(sock,host,port,20*1000,NULL);
	    if (ret != 0) {
	        APP_TRACE("socket fail\n");
	        _comm_close_page(COMM_RET_FAIL_CONECT);
			break;
	    }

	    _comm_message_update(m_title, "send data...");
		index += sprintf(m_comm_data->sbuff + index , "POST api/Iotmsgtest/pushmsgMf?order_sn=%s HTTP/1.1\r\n", get_OrderSn());
		index += sprintf(m_comm_data->sbuff + index , "Host: %s:%d\r\n\r\n", host, port);
		m_comm_data->slen = strlen(m_comm_data->sbuff);
//	    ret = comm_sock_send(sock, m_comm_data->sbuff, m_comm_data->slen);
		ret = MfSdkCommSocketSend(sock, (u8*)m_comm_data->sbuff, m_comm_data->slen);
		if( ret < 0 )
		{
			APP_TRACE("send fail ret= %d\r\n", ret);
	        _comm_close_page(COMM_RET_FAIL_SEND);
			ret = -1;
			break;
		}

	    ret = _comm_recv(sock, m_comm_data->rbuff, RECV_BUFF_SIZE);
		if( ret <= 0 )
		{
			APP_TRACE("recv fail\n");
	        _comm_close_page(COMM_RET_FAIL_RECV);
			ret = -1;
			break;
		}

	    char *rescode = (char *)strstr(m_comm_data->rbuff ," ");
	    if ( rescode != 0 && 200 == atoi(rescode+1))
		{
	        m_comm_data->rlen = ret;
	        _comm_close_page(COMM_RET_SUCC);
		}
		else
		{
			APP_TRACE("resp code fail\n");
	        _comm_close_page(COMM_RET_FAIL_RECV);
		}
	}while(0);
	
//    comm_sock_close(sock);
	MfSdkCommSocketClose(sock);
    APP_TRACE("[%s][comm_net_link ret:%d]\r\n",__FUNCTION__, ret);

    return;
}

lv_obj_t* page_comm_show_auto(lv_obj_t* parent, void* pfunc, char* title, st_comm_data* comm_data,int show_back)
{
    page_comm_busyflag = 1;
    m_comm_data = comm_data;
    m_title = title;

    m_page_close_page_func = (page_close_page_func)pfunc;

    page_win = page_create_win(parent, _comm_event_cb);
    lab_title = page_create_title(page_win, title);
	if(show_back)
		page_create_navigation_back(page_win,back_navigation_event_cb);
    lab_message = page_create_msg(page_win, "link....");

    //Sys_TaskCreate(_comm_task_auto, 0, 0, 0);

    //btn_canel = page_create_btn(page_win, "cancel", _comm_btn_event_cb, PAGE_BTN_ALIGN_LEFT);
    task = lv_task_create(_comm_task_auto, 1000, LV_TASK_PRIO_LOWEST, NULL);
#ifndef WIN32
    lv_task_ready(task);
    lv_task_once(task);
#endif
    return page_win;
}
