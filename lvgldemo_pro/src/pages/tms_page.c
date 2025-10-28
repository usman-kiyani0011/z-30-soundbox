#include "lvgl/lvgl.h"
#include "lvgl/src/lv_lib_png/lv_png.h"
#include "libapi_xpos/inc/mfsdk_tms.h"
#include "page_pub.h"
#include "tms_page.h"
#include "tms/inc/tms_pub.h"

#include "../page_main.h"

#define TMS_PROC_PAGE	"P:exdata/tmsproc.png"
#define TMS_TIP_SUCC	"P:exdata/tmssucc.png"
#define TMS_TIP_FAIL	"P:exdata/tmsfail.png"

#define FRONT_PROCESS_OFFSET	(60)
#define BEHIND_PROCESS_OFFSET	(8)

static lv_obj_t* frontparent = NULL;
static lv_obj_t* tms_front_label = NULL;
static lv_obj_t* tms_bar = NULL;
static lv_task_t *task_timer = NULL;
static lv_obj_t* res_img = NULL;
static lv_obj_t* tip = NULL;

static int g_update_manual = 0;
static int g_disptip = 0;
static int m_timeover = 0;


static char* TmsGetMessage(int type)
{
	static char* message = "";

	APP_TRACE("TmsGetMessage type = %d\r\n", type);
	if (type == TMS_NET_FAIL)
	{
		message = "Net Link Fail";
	}
	else if (type == TMS_SOCKET_FAIL)
	{
		message = "Connect Fail";
	}
	else if (type == TMS_LOGIN_FAIL)
	{
		message = "Server Login Fail";
	}
	else if (type == TMS_LOGIN_FORMAT_ERROR)
	{
		message = "Login Package Error";
	}
	else if (type == TMS_DEVICE_ERROR)
	{
		message = "Illegal Equipment";
	}
	else if (type == TMS_MD5_ERROR)
	{
		message = "MD5 Check Error";
	}
	else if (type == TMS_SIG_ERROR)
	{
		message = "Verification Fail";
	}
	else if (type == TMS_LOW_BATTERY)
	{
		message = "Low Power";
	}
	else if (type == TMS_UPDATE_FAIL)
	{
		message = "Update Fail";
	}
	else if (type == TMS_CONNECT_SERVER_FAIL)
	{
		message = "Connect Fail";
	}
	else if (type == TMS_SEND_SERVER_FAIL)
	{
		message = "Send Fail";
	}
	else if (type == TMS_RECV_SERVER_FAIL)
	{
		message = "Receive Fail";
	}
	else if (type == TMS_UPDATE_SUCCESS)
	{
		message = "Download Success\rRestarting Device";
	}
	else if (type == TMS_UPDATE_NO_TRIGERR)
	{
		message = "No need to update";
	}
	else if (type == TMS_CONNECT_SERVER)
	{
		message = "Connect Server";
	}
	else if (type == TMS_SEND_SERVER)
	{
		message = "Send Data";
	}
	else if (type == TMS_RECV_SERVER)
	{
		message =  "Receive Data";
	}
	else if (type == TMS_UPDATING)
	{
		message = "Downloading";
	}
	else
	{
		message = "Unknown Error";
	}

	APP_TRACE("TmsGetMessage message = %s\r\n", message);
	return message;
}

static void tms_proc_close_tip_page()
{
	APP_TRACE("tms_proc_close_tip_page\r\n");
	if (tip != NULL)
	{
		if (res_img != NULL)
		{
			lv_obj_del(res_img);
			res_img = NULL;
		}
		lv_obj_del(tip);
		tip = NULL;
	}
}

static void tms_proc_close_page()
{
	APP_TRACE("tms_proc_close_page\r\n");
	if (task_timer != NULL)
	{
        lv_task_del(task_timer);
		task_timer = NULL;
	}
	if (frontparent != NULL)
	{
		APP_TRACE("frontparent close\r\n");
		if (tms_front_label != NULL)
		{
			lv_obj_del(tms_front_label);
			tms_front_label = NULL;
		}
		if (tms_bar != NULL)
		{
			lv_obj_del(tms_bar);
			tms_bar = NULL;
		}
		tms_proc_close_tip_page();
		lv_obj_del(frontparent);
		frontparent = NULL;
	}

	lv_free_png_file(TMS_PROC_PAGE);
	lv_free_png_file(TMS_TIP_FAIL); 
	lv_free_png_file(TMS_PROC_PAGE);

	g_update_manual = 0;
	g_disptip = 0;
	m_timeover = 0;
}

static void tms_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t key;

	if (e == LV_EVENT_KEY)
	{
		key = page_get_key();

		if (key == MF_LV_KEY_CANCEL_SHORT_PRESS || key == MF_LV_KEY_OK_SHORT_PRESS)
		{
			tms_proc_close_page();
		}
	}
}

static void update_tms_progress(int progress)
{
	char buff[10] = { 0 };

	tms_proc_close_tip_page();
	
	sprintf(buff, "%d%%", progress);
	if(tms_front_label != NULL)
	{
		APP_TRACE("front process: %s\r\n", buff);
		lv_label_set_text(tms_front_label, buff);
	}
	if(tms_bar != NULL)
	{
		lv_bar_set_value(tms_bar, progress, LV_ANIM_OFF);
	}
}

#if 0
static int tms_size = -1;
#define TMS_SIZE	200
void tms_test_func()
{
	int progress = 0;
	if(tms_size == -1)
	{
		app_tms_page_show(0);
		tms_size = 0;
	}

	if(tms_size<=TMS_SIZE &&tms_size >=0)
	{
	    progress = 100 * tms_size / TMS_SIZE;
		update_tms_progress(progress);
		tms_size+=10;
	}
}
#endif

static lv_obj_t* TmsPageCreate()
{

	APP_TRACE("%s behind\r\n", __FUNCTION__);
	
	//mainpage_create_title(behindparent, "TMS UPDATE");

	//lv_load_png_file(TMS_TIP_SUCC);
	//lv_load_png_file(TMS_TIP_FAIL);
	lv_load_png_file(TMS_PROC_PAGE);
	
	APP_TRACE("%s front\r\n", __FUNCTION__);
	lv_obj_t * parent = get_mainpage();//lv_scr_act();

	frontparent = lv_img_create(parent, NULL);
	lv_img_set_src(frontparent, TMS_PROC_PAGE);
	lv_img_set_auto_size(frontparent, true);
	lv_obj_align(frontparent, NULL, LV_ALIGN_CENTER, 0, 0);

	tms_front_label = page_ShowTextOut(frontparent, "", LV_ALIGN_CENTER, 0, FRONT_PROCESS_OFFSET, LV_COLOR_BLACK, LV_FONT_24);

	return frontparent;
}

void app_tms_page_show(int progress)
{
	char buff[10] = { 0 };

    APP_TRACE("[%s]\r\n", __FUNCTION__);
	if(NULL == frontparent)
	{
		TmsPageCreate();
	}
	sprintf(buff, "%d%%", progress);

	page_ShowTextOut(frontparent, "TMS Upgrading", LV_ALIGN_CENTER, 0, 20, LV_COLOR_BLACK, LV_FONT_16);
	lv_label_set_text(tms_front_label, buff);
	lv_obj_align(tms_front_label, NULL, LV_ALIGN_CENTER, 0, FRONT_PROCESS_OFFSET);

	tms_bar = lv_bar_create(frontparent, NULL);
	lv_bar_set_range(tms_bar, 0, 100);
	lv_obj_set_size(tms_bar, lv_obj_get_width(frontparent)-60, 10);
	lv_obj_align(tms_bar, NULL, LV_ALIGN_CENTER, 0, 100);

	page_ShowTextOut(frontparent, "Please wait...", LV_ALIGN_CENTER, 0, 140, LV_COLOR_BLACK, LV_FONT_24);
	
	return;
}

void  TmsDownloadFunc(int size, int total_size)
{
    APP_TRACE("[%s] %d/%d \r\n", __FUNCTION__, size, total_size);
    int progress = 100 * size / total_size;
    char tmp[10] = { 0 };

    sprintf(tmp, "%d", progress);
    APP_TRACE("[%s]:[Progress bar:%s] \r\n", __FUNCTION__, tmp);
    MfSdkGuiLedAmount(tmp);
	//lv_start_lock(1);
	//update_tms_progress(progress);
	//lv_start_lock(0);
}

static void TmsClosePage(int timeover)
{
	m_timeover = timeover;
	APP_TRACE("g_disptip = %d \r\n", g_disptip);
	while (m_timeover && 1 == g_disptip)
	{
		m_timeover-=1000;
		MfSdkSysSleep(1000);
		APP_TRACE("m_timeover = %d \r\n", m_timeover);
	}
	tms_proc_close_page();
}

static void TmsResultPage(int type, int timeover)
{
	if (type != TMS_UPDATE_NO_TRIGERR || g_update_manual == 1)
	{
		APP_TRACE("set g_disptip\r\n");
		g_disptip = 1;
	}
	
	tip = page_create_win(frontparent, tms_event_cb);
	res_img = lv_img_create(tip, NULL);
	if (type == TMS_UPDATE_SUCCESS || type == TMS_UPDATE_NO_TRIGERR)
	{
		lv_load_png_file(TMS_TIP_SUCC);
		lv_img_set_src(res_img, TMS_TIP_SUCC);
	}
	else
	{
		lv_load_png_file(TMS_TIP_FAIL);
		lv_img_set_src(res_img, TMS_TIP_FAIL);
	}
	lv_img_set_auto_size(res_img, true);
	lv_obj_align(res_img, NULL, LV_ALIGN_CENTER, 0,0);
	page_ShowTextOut(res_img, TmsGetMessage(type), LV_ALIGN_CENTER, 0, 30, LV_COLOR_BLACK, LV_FONT_24);

	MfSdkSysSleep(100);
	
	if(tms_front_label != NULL)
	{
		lv_label_set_text(tms_front_label, "");
	}
	
	if(timeover>0)
	{
	    task_timer = lv_task_create(tms_proc_close_page, timeover, LV_TASK_PRIO_MID, 0);
	}

	return;
}

void  TmsShowResultFunc(int type)
{

	lv_start_lock(1);
	APP_TRACE("TmsShowResultFunc: type = %d update = %d\r\n", type, g_update_manual);
	
	tms_proc_close_tip_page();
	//not display communication process
	if ((type == TMS_CONNECT_SERVER || type == TMS_SEND_SERVER || type == TMS_RECV_SERVER || type == TMS_UPDATE_NO_TRIGERR
		|| type == TMS_CONNECT_SERVER_FAIL || type == TMS_SEND_SERVER_FAIL || type == TMS_RECV_SERVER_FAIL) && 0 == g_update_manual)
	{
		APP_TRACE("not display type = %d\r\n", type);
	}
	else
	{
		if(NULL == frontparent)
		{
			TmsPageCreate();
		}
		
		if (type == TMS_CONNECT_SERVER || type == TMS_SEND_SERVER || type == TMS_RECV_SERVER) 
		{
			if(tms_front_label != NULL && 1 == g_update_manual)
			{
				lv_label_set_text(tms_front_label, TmsGetMessage(type));
				lv_obj_align(tms_front_label, NULL, LV_ALIGN_CENTER, 0, FRONT_PROCESS_OFFSET);
			}
		}
		else if (type == TMS_FINISH)
		{
			TmsClosePage(3000);
		}
		else
		{
			APP_TRACE("type = %d, update = %d\r\n", type, g_update_manual);		
			TmsResultPage(type, 0);
		}
	}
	
	lv_start_lock(0);
}

void TmsUpdateFunc()
{	
	if(0 == g_update_manual)
	{
	    if (MfSdkCommLinkState() != 1)
	    {
	        APP_TRACE("TransactionProc connect fail\n");
			TmsPageCreate();

			TmsResultPage(TMS_NET_FAIL, 3000);
	    }
		else
		{
			g_update_manual = 1;
			MfSdkTmsHeartBeat();
		}
	}
}

