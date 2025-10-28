#include "lvgl/lvgl.h"
#include "tracedef.h"

#include "page_pub.h"
#include "AppPub/inc/struct_tlv.h"
#include "libapi_xpos/inc/mfsdk_aud.h"
#include "libapi_xpos/inc/mfsdk_mem.h"
#include "libapi_xpos/inc/mfsdk_log.h"
#include "libapi_xpos/inc/mfsdk_ver.h"
#include "libapi_xpos/inc/mfsdk_lvgl.h"
#include "libapi_xpos/inc/mfsdk_esim.h"

#include "../sdk_tts.h"
#include "../pages/page_normal_list.h"
#include "page_esim.h"
#include "../page_main.h"
#include "../pages/page_message.h"
#include "./card/func_pay.h"

static lv_task_t* lv_esim_task = NULL;
static lv_task_t* lv_esim_task_tick = NULL;

static lv_obj_t* page_esim_win = NULL;
static lv_obj_t* label_msg = NULL;
static lv_obj_t* label_tick = NULL;
//static lv_obj_t* label_bt = NULL;

static int m_reboot_flag = 0;
static int m_close_flag = 2;
static unsigned int g_tick = 0;

static int m_state = -1000000;
static int g_nmenu = -1;


typedef struct
{
    int status;
    char *inform;
}AppEsimStatus_T;

static AppEsimStatus_T g_appEsimStatus[] =
{
	{MFSDK_ESIM_STATE_FUN_DISABLE,					"ESIM function\r\nis not enabled"}, //ESIM功能未开启
	{MFSDK_ESIM_STATE_BT_CONNECT_FAIL,				"BT not connected"},                                //蓝牙未连接
	{MFSDK_ESIM_STATE_BT_FORMAT_ERROR,				"BT receiving error"},                    //蓝牙接收错误
	{MFSDK_ESIM_STATE_NO_EXIST,						"ESIM card\r\ndoes not exist"},
	{MFSDK_ESIM_STATE_NET_FAIL,						"Network Exception"},       //网络异常
	{MFSDK_ESIM_STATE_NET_ECCID_CONNECT_FAIL,		"Connect server failed"},           //连接服务器失败(请求eccid mf平台)
	{MFSDK_ESIM_STATE_NET_ECID_SEND_FAIL,			"Failed to send data"},     //发送数据失败
	{MFSDK_ESIM_STATE_NET_ECID_RECV_FAIL,			"Failed to recv data"},     //接收复失败
	{MFSDK_ESIM_STATE_NET_ICCID_CONNECT_FAIL,		"ICCID Failed\r\nto connect"},     //连接服务器失败(请求iccid 平台运营商)
	{MFSDK_ESIM_STATE_NET_ICCID_SEND_FAIL,			"ICCID Failed\r\nto send data"},           //发送数据失败
	{MFSDK_ESIM_STATE_NET_ICCID_RECV_FAIL,			"ICCID Failed\r\nto recv data"},           //接收复失败
	{MFSDK_ESIM_STATE_NET_NOTIFY_CONNECT_FAIL,		"NT Connect\r\nserver failed"},            //连接服务器失败(上报通知 平台运营商)
	{MFSDK_ESIM_STATE_NET_NOTIFY_SEND_FAIL,			"NT Failed to send"},       //发送数据失败
	{MFSDK_ESIM_STATE_NET_NOTIFY_RECV_FAIL,			"NT Failed to recv"},       //接收复失败
	{MFSDK_ESIM_STATE_APDU_ERROR,					"APDU failed"},     //发送报文失败
	{MFSDK_ESIM_STATE_ICCID_ENABLE_FAIL,			"ICCID Active\r\nconfiguration failed"},                   //iccid 配置文件启用失败
	{MFSDK_ESIM_STATE_ICCID_DISABLE_FAIL,			"ICCID Disable\r\nconfiguration failed"},          //iccid禁用文件失败
	{MFSDK_ESIM_STATE_ICCID_DEL_FAIL,				"ICCID Delete\r\nconfiguration failed"},           //iccid删除配置文件失败
	{MFSDK_ESIM_STATE_ICCID_NULL,					"ICCID recv NULL"},         //返回iccid 为空
	{MFSDK_ESIM_STATE_ICCID_STATE_ERROR,			"ICCID state error"},       //返回iccid 状态错误
	{MFSDK_ESIM_STATE_ECCID_FORMAT_ERROR,			"ECID Format error"},       //返回数据错误
	{MFSDK_ESIM_STATE_ECCID_ERROR,					"ECID Error"},      //返回激活码为空 或状态不匹配
	{MFSDK_ESIM_STATE_SMDP_RETURN_ERROR,			"SMDP error"}, //平台SMDP返回错误
	{MFSDK_ESIM_STATE_SUCCED,						"ESIM Successful"},             //ESIM成功激活或启用ICCID成功
	{MFSDK_ESIM_STATE_ACTIVEED,						"ESIM Actived"},            //ESIM已激活	结束
	{MFSDK_ESIM_STATE_ECID_RSP_READY,				"Wating to\r\nreceive"},                   //等待接收蓝牙或网络返回的激活码
	{MFSDK_ESIM_STATE_BT_ECID_RSP_READY,			"BT Wating for\r\nconnection"},                            //蓝牙等待连接
	{MFSDK_ESIM_STATE_BT_ECID_RSP_REV,				"BT Recv..."},              //蓝牙连接成功 等待接收数据
	{MFSDK_ESIM_STATE_BT_CONNECT_END,				"BT Connected"},                    //ble已连接 等待接收ECCID数据
	{MFSDK_ESIM_STATE_BT_ECID_RSP_END,				"ECID End"},        //成功收到ECID 未写卡
	{MFSDK_ESIM_STATE_NET_ECID_RSP_CONNECT,			"ECID Connect"},                            //mf 平台等待连接
	{MFSDK_ESIM_STATE_NET_ECID_RSP_SEND,			"ECID Send"},                       //请求激活码
	{MFSDK_ESIM_STATE_NET_ECID_RSP_RECV,			"ECID Recv..."},           //等待接收激活码
	{MFSDK_ESIM_STATE_NET_ECID_RSP_END,				"ECID End"},           //收到平台截屏返回
	{MFSDK_ESIM_STATE_NET_ICCID_RSP_CONNECT,		"ICCID Connect..."},                                //运营商平台
	{MFSDK_ESIM_STATE_NET_ICCID_RSP_SEND,			"ICCID Send"},                      //请求ICID请求
	{MFSDK_ESIM_STATE_NET_ICCID_RSP_RECV,			"ICCID Recv..."},            //等待接收ICID中
	{MFSDK_ESIM_STATE_NET_ICCID_RSP_END,			"ICCID End"},               //激活码写卡成功 未设置ICCID
	{MFSDK_ESIM_STATE_NET_NOTIFY_RSP_CONNECT,		"NT Connect..."},                           //运营商平台
	{MFSDK_ESIM_STATE_NET_NOTIFY_RSP_SEND,			"NT Send"},                         //请求ICID请求
	{MFSDK_ESIM_STATE_NET_NOTIFY_RSP_RECV,			"NT Recv..."},             //等待接收ICID中
	{MFSDK_ESIM_STATE_HEART_COM,					"Heart..."},                //后台正在通讯中
	{MFSDK_ESIM_STATE_HANDLE_ENABLE,				"Enable ESIM"},             //启用ESIM卡...
	{MFSDK_ESIM_STATE_HANDLE_DISABLE,				"Disable ESIM"},                    //禁用ESIM卡...
	{MFSDK_ESIM_STATE_HANDLE_DELETE,				"Delete\nconfiguration"},                    //删除卡配置文件...
	{MFSDK_ESIM_STATE_NOTIFY_UPLOAD,				"Upload..."},               //状态信息上报中
	{MFSDK_ESIM_STATE_ICCID_STATE_UPLOAD,			"Upload to MF..."}, //状态上报中(mf平台)
	{MFSDK_ESIM_STATE_FINISH,						"Finish"},
};

static int NoneWifiModeCallback(int ret, lv_obj_t* obj);

#if 0
s32 MfSdkSysGetBtName(char *blename , s32 length)
{
	strcpy(blename, "BTNAME");
	return 0;
}
MfSdkEsimRet_E MfSdkEsimActiveFun(MfSdkEsimCommMode_E mode)
{
    return 0;
}
MfSdkEsimState_E MfSdkEsimGetState(void)
{
    return 0;
}
s32 MfSdkEsimResetProc(pFuncMfSdkEsimPowerResetCb fun)
{
    return 0;
}
MFSDKBOOL MfSdkEsimIsEnable(void)
{
    return MFSDK_TRUE;
}
#endif

static s32 ESimPageGetBTName(char *name, s32 namesize)
{
    char szblename[32 + 1] = { 0 };

    MfSdkSysGetBtName(szblename,sizeof(szblename));
    snprintf(name, namesize, "BT:%s", strlen(szblename) ? szblename : "mf-test");

    return strlen(name);
}

static void _page_esim_close(int ret)
{
    if (lv_esim_task != NULL)
    {
        lv_task_del(lv_esim_task);
        lv_esim_task = NULL;
    }

    if(lv_esim_task_tick != NULL)
    {
        lv_task_del(lv_esim_task_tick);
        lv_esim_task_tick = NULL;
    }

    if (page_esim_win != 0)
    {
	    if (label_msg != 0)
	    {
	        lv_obj_del(label_msg);
	        label_msg = 0;
	    }
	    if (label_tick != 0)
	    {
	        lv_obj_del(label_tick);
	        label_tick = 0;
	    }

        lv_obj_del(page_esim_win);
        page_esim_win = 0;
        APP_TRACE("ocr page close0 =%x\r\n", page_esim_win);
    }
    AppPowerUnlockApp((char*)"ESIM");
    g_tick = 0;
}

static void page_esim_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t key = -1;

	if (e == LV_EVENT_KEY)
	{
		key = pub_page_get_key();
		if (key == MF_LV_KEY_QUIT_LONG_PRESS)
		{
			PubMultiPlay((const s8 *)"shut.mp3");
			while (MfSdkAudTtsState() == 1)                             //Broadcasting
			{
				MfSdkSysSleep(500);
			}

			APP_TRACE("Shutdown---------\t\n");
			MfSdkSysSleep(1000);
			MfSdkPowerOff();
		}
		else if (key == MF_LV_KEY_CANCEL_SHORT_PRESS)
		{
			if (MfSdkEsimGetState() <= MFSDK_ESIM_STATE_BT_CONNECT_END)
			{
				m_close_flag = 1;
				lv_label_set_text(label_msg, "Exiting\r\nplease wait");
				MfSdkEsimActiveFun(MFSDK_ESIM_FUN_NULL);
			}
			else
			{
				APP_TRACE("comm busy\r\n");
				lv_label_set_text(label_msg, "Busying\r\nplease wait");
			}
		}
	}
}

/**
 * @brief app1界面定时刷新任务
 * @param task
 */
static void update_msg_task_func(lv_task_t* task)
{
    int i = 0;
	int status = 0;
    char sztemp[128 + 1] = { 0 };
    APP_TRACE("update_msg_task_func:%d\r\n", m_close_flag);

	if (m_close_flag == 2)
	{
		_page_esim_close(0);
		page_list_close_page(0);
		EsimMainPage(get_mainpage());
	}
	if (m_close_flag == 0)
    {
        status = MfSdkEsimGetState();
        APP_TRACE("esim_get_state:%d\r\n", status);

        if (m_state != status)
        {
            m_state = MfSdkEsimGetState();
			if(999 == m_state) return;

            for(i = 0; i < sizeof(g_appEsimStatus) / sizeof(g_appEsimStatus[0]); i++)
            {
                if(g_appEsimStatus[i].status == m_state)
                {
                    sprintf(sztemp, "%s\r\n(status code:%d)", g_appEsimStatus[i].inform, m_state);
                    break;
                }
            }

            if(i >= sizeof(g_appEsimStatus) / sizeof(g_appEsimStatus[0]))
            {
                sprintf(sztemp, "ESIM status code:\r\n%d", m_state);
            }
            
			if( m_state < MFSDK_ESIM_STATE_SUCCED)
			{
				snprintf(sztemp + strlen(sztemp), sizeof(sztemp) - strlen(sztemp),"\r\nPress cancel exit!");
			}
			else if ( m_state <= MFSDK_ESIM_STATE_ACTIVEED || 1 == m_reboot_flag )
			{
				m_reboot_flag = 0;
				snprintf(sztemp + strlen(sztemp), sizeof(sztemp) - strlen(sztemp),"\r\nPress cancel restart");
			}
			lv_label_set_text(label_msg, sztemp);
        }
    }
}

static int esimpage_power_reset(int nstate)
{
    APP_TRACE("app nstate=%d\r\n", nstate);

	if (nstate == 0)
    {
		m_close_flag = 0;
		m_reboot_flag = 1;
		MfSdkSysSleep(2000);
        MfSdkPowerReset();
    }
    else
    {
		m_close_flag = 2;
    }
    return 0;
}

static void update_tick_task_func(lv_task_t* task)
{
    char str[128] = { 0 };

    if(label_tick != NULL)
    {
        sprintf(str, "(%ds)", ++g_tick);
        lv_label_set_text(label_tick, str);
		lv_obj_align(label_tick, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -6);
    }
}

void page_esim_show(lv_obj_t* parent, int nmenu)
{
	int ret = -1;
    char *title = 0;

    m_close_flag = 0;
    g_tick = 0;
    m_state = -1000000;
	g_nmenu = nmenu;

	ret = MfSdkEsimActiveFun(nmenu);
    if ( ret == -1)     //网络连接失败无法进入激活流程
    {
        page_text_show_mid(parent, "TIP", "Network unstable", 3000);
        PubMultiPlay((const s8*)"netf.mp3");
        return;
    }
	else if ( ret == -2 )
	{
        page_text_show_mid(parent, "TIP", "ESIM function\r\nis not enabled", 3000);
		return;
	}
	else if ( ret == -3 )
	{
        page_text_show_mid(parent, "TIP", "Reporting ESIM\r\nnotification\r\nmessage\r\nplease try again\r\nlater", 3000);
		return;
	}
    AppPowerLockApp((char*)"ESIM");
    MfSdkEsimResetProc(esimpage_power_reset);

	page_esim_win = page_create_win(parent, page_esim_event_cb);
	title = (MFSDK_ESIM_BT_MENU_ACTIVE == nmenu)?"BLUE ACTIVE":"NET ACTIVE";
	page_create_title(page_esim_win, title);

	if(MFSDK_ESIM_BT_MENU_ACTIVE == nmenu)
	{
		char szblename[64 + 1] = { 0 };
		ESimPageGetBTName(szblename , sizeof(szblename) - 1);
		page_ShowTextOut(page_esim_win, szblename, LV_ALIGN_IN_TOP_MID, 0, page_get_statusbar_height() + 50, LV_COLOR_BLACK, LV_FONT_24);
	}

    label_msg = lv_label_create(page_esim_win, NULL);
    lv_label_set_long_mode(label_msg, LV_LABEL_LONG_BREAK);
    lv_label_set_text(label_msg, "Waiting to receive ESIM\r\n registration code ");
    lv_obj_set_width(label_msg, lv_obj_get_width(page_esim_win));
    lv_label_set_align(label_msg, LV_LABEL_ALIGN_CENTER);     // 文本对齐方式
    lv_obj_align(label_msg, 0, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_text_color(label_msg, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	//label_msg = page_ShowTextOut(page_esim_win, str, LV_ALIGN_CENTER, 0, 0, LV_COLOR_BLACK, LV_FONT_24);

	label_tick = page_ShowTextOut(page_esim_win, " ", LV_ALIGN_IN_BOTTOM_MID, 0, -6, LV_COLOR_BLACK, LV_FONT_24);

    lv_esim_task = lv_task_create(update_msg_task_func, 100, LV_TASK_PRIO_MID, NULL);
    lv_esim_task_tick = lv_task_create(update_tick_task_func, 1000, LV_TASK_PRIO_MID, NULL);
}

static lv_obj_t *page_version = NULL;
static int ShowVersion_func(int ret, lv_obj_t* obj)
{
    if (page_version != NULL)
    {
        lv_obj_del(page_version);
        page_version = 0;
    }
	AppPowerUnlockApp("ShowVersion");
	page_list_close_page(0);
    EsimMainPage(get_mainpage());
    return ret;
}

static void ShowVersion(char *title)
{
    char msg[512] = {0};
    char deviceName[64] = {0};
    char szblename[32 + 1] = { 0 };
    const s8 *appVer = MfSdkVerGetAppVer();
    const s8 *dataVer = MfSdkVerGetDataVersion();
    const s8 *libversion = MfSdkVerMfOsVersion();
    //const s8 *spVer = MfSdkVerGetSpVer();
    const s8 *halVer = MfSdkVerGetDriverVer();
    const s8 *pICCID = MfSdkCommAtcIccid(0);

	AppPowerLockApp("ShowVersion");

    if (page_version != NULL)
    {
        lv_obj_clean(page_version);
        lv_obj_del(page_version);
    }
    lv_obj_t * parent = get_mainpage();
    page_version = lv_obj_create(parent, NULL);
    lv_obj_set_size(page_version, lv_obj_get_width(parent), lv_obj_get_height(parent));

    memset(msg, 0, sizeof(msg));
    MfSdkSysGetTerminalSn((s8*)deviceName, sizeof(deviceName));
    MfSdkSysGetBtName(szblename,sizeof(szblename));

    snprintf(msg, sizeof(msg), "SN:%s\r\nAppVer:\r\n%5.5s-%s\r\nSysVer:\r\n%s\r\nDriver:\r\n%s\r\nDataVer:\r\n%s\r\n",
             deviceName, appVer, strstr((char *)appVer, "V"), libversion, halVer, dataVer);

    snprintf(msg + strlen(msg), sizeof(msg) - strlen(msg), "BT:%s\r\n", strlen(szblename) ? szblename : "mf-test");

    snprintf(msg + strlen(msg), sizeof(msg) - strlen(msg), "ICCID:\r\n%s\r\n", pICCID == NULL  ? "" : (char*)pICCID);

    page_message_show_align(page_version, ShowVersion_func, title, msg, "", "", 60 * 1000, LV_LABEL_ALIGN_LEFT);

    return;
}

static char* g_esimlistitem[] = {"BLUE ACTIVE", "NET ACTIVE", "INFOR"};

static int esim_menu_list_select_func(int ret, lv_obj_t* obj)
{
	APP_TRACE("esim_menu_list_select_func:%d\r\n", ret);

	switch (ret)
	{
		case 0:
			if(!MfSdkCommLinkState())
			{
				page_text_show_mid(get_mainpage(), "TIP", "Pls check network", 3000);
			}
			else
			{
				page_esim_show(get_mainpage(), MFSDK_ESIM_BT_MENU_ACTIVE);
			}
			break;

		case 1: 
			page_esim_show(get_mainpage(), MFSDK_ESIM_NET_MENU_ACTIVE); 
			break;

		case 2: 
			ShowVersion("INFOR"); 
			break;

		default: 
			break;
	}

	return ret;
}

void EsimMainPage(lv_obj_t* parent)
{
	page_list_show(parent, esim_menu_list_select_func, "ESIM", g_esimlistitem, sizeof(g_esimlistitem) / sizeof(g_esimlistitem[0]), 0, 60 * 1000);
}

