#include "libapi_xpos/inc/mfsdk_ver.h"
#include "page_version.h"
#include "pages.h"
#include "page_message.h"
#include "player_proc.h"

static lv_obj_t* page_version = NULL;

int AppShowVersion_func(int ret, lv_obj_t* obj)
{
	if (page_version != NULL)
	{
		lv_obj_del(page_version);
		page_version = 0;
		AppPowerUnlockApp("page_ver");
	}
	if (0 == ret)
	{
		ShowSetting("SETTING");
	}
	return ret;
}

void AppShowVersion(char *title)
{
	char msg[512] = {0};
	char deviceName[64] = {0};
	const s8 *appVer = NULL;
	const s8 *dataVer = NULL;
	const s8 *sysversion = NULL;
	const s8 *halVer = NULL;

	if (page_version != NULL)
	{
		return;
	}

	AppPowerLockApp("page_ver");
	lv_obj_t * parent = lv_scr_act();
	page_version = lv_obj_create(parent, NULL);
	lv_obj_set_size(page_version, lv_obj_get_width(parent), lv_obj_get_height(parent));

	memset(msg, 0, sizeof(msg));
	appVer = MfSdkVerGetAppVer();
	dataVer = MfSdkVerGetDataVersion();
	sysversion = MfSdkVerMfOsVersion();
	halVer = MfSdkVerGetDriverVer();
	MfSdkSysGetTerminalSn((s8*)deviceName,sizeof(deviceName));
	//MfSdkSysHeap_T HeapInfor;
	//MfSdkSysGetHeapInformation(&HeapInfor);
	//APP_TRACE("total_size  = %d\r\n", HeapInfor.total_size);
	//APP_TRACE("avail_size  = %d\r\n", HeapInfor.avail_size);

	snprintf(msg + strlen(msg), sizeof(msg), "#808080 SN: \n%s\n", deviceName);
	snprintf(msg + strlen(msg), sizeof(msg), "#808080 AppVer: \n%5.5s-%s\n", appVer, strstr((char *)appVer,"V"));
	snprintf(msg + strlen(msg), sizeof(msg), "#808080 SysVer: \n%s\n", sysversion);
	snprintf(msg + strlen(msg), sizeof(msg), "#808080 Driver: \n%s\n", halVer);
	snprintf(msg + strlen(msg), sizeof(msg), "#808080 DataVer: \n%s\n", dataVer);
	//snprintf(msg + strlen(msg), sizeof(msg), "#808080 total_size: \n%d\n", HeapInfor.total_size);
	//snprintf(msg + strlen(msg), sizeof(msg), "#808080 avail_size: \n%d\n", HeapInfor.avail_size);

	APP_TRACE("AppShowVersion:\r\n%s", msg);
	page_message_show(page_version, AppShowVersion_func, title, msg, "", "", 60*1000);

	return;
}

int ShowSetting_func(int ret, lv_obj_t* obj)
{
	free_message_page();
	if (page_version != NULL)
	{
		lv_obj_del(page_version);
		page_version = 0;
		AppPowerUnlockApp("page_setting");
	}
	return ret;
}

void ShowSetting(char *title)
{
	char msg[512] = {0};
    char ip[64] = { 0 };
    int port = MQTT_PORT;	
    char httpip[64] = { 0 };
    int httpport = HTTP_PORT;	

	if (page_version != NULL)
	{
		return;
	}

	AppPowerLockApp("page_setting");
	lv_obj_t * parent = lv_scr_act();
	page_version = lv_obj_create(parent, NULL);
	lv_obj_set_size(page_version, lv_obj_get_width(parent), lv_obj_get_height(parent));

    get_setting_str(COMM_HOST_IP, ip, sizeof(ip));
    if (strlen(ip) < 3)
    {
        strcpy(ip, MQTT_HOST);
        port = MQTT_PORT;
    }
	else
	{
	    port = get_setting_int(COMM_HOST_PORT);
	}
	
    get_setting_str(COMM_HTTP_IP, httpip, sizeof(httpip));
    if (strlen(httpip) < 3)
    {
        strcpy(httpip, HTTP_HOST);
        httpport = HTTP_PORT;
    }
	else
	{
	    httpport = get_setting_int(COMM_HTTP_PORT);
	}

	MfSdkBatterAttr_T batterAttr = {0};
	memset(&batterAttr , 0, sizeof(MfSdkBatterAttr_T));
	MfSdkSysGetBatterStatus(&batterAttr);

	char *mode = (MfSdkCommGetNetMode() == MFSDK_COMM_NET_ONLY_WIRELESS) ? "GPRS" : "WIFI";
	char *status = NULL;
	if (0 == MfSdkCommLinkState())
	{
		status = "Network Fail";
	}
	else
	{
    	status = (1 == get_mqtt_con_state()) ? "Connect Success" : "Connect Fail";
	}

	snprintf(msg + strlen(msg), sizeof(msg), "#808080 Mqtt: \n%s:%d\n", ip, port);
	snprintf(msg + strlen(msg), sizeof(msg), "#808080 Http: \n%s:%d\n", httpip, httpport);
	snprintf(msg + strlen(msg), sizeof(msg), "Keepalive: %d\n", get_keepAliveInterval());
	snprintf(msg + strlen(msg), sizeof(msg), "Battery Capacity: %d\n", batterAttr.capacity);
	snprintf(msg + strlen(msg), sizeof(msg), "Mode: %s\n", mode);
	snprintf(msg + strlen(msg), sizeof(msg), "Status: \n%s\n", status);

	APP_TRACE("ShowSetting:\r\n%s", msg);
	page_message_show(page_version, ShowSetting_func, title, msg, "", "", 60*1000);

	return;
}

