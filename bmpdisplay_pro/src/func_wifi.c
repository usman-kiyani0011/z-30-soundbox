//#include "lvgl/lvgl.h"
#include "pages/pages.h"
#include "func_wifi.h"
//#include "tracedef.h"
#include "libapi_xpos/inc/libapi_comm.h"
//#include "libapi_xpos/inc/libapi_system.h"
#include "libapi_xpos/inc/mfsdk_sys.h"
#include "libapi_xpos/inc/mfsdk_comm.h"
#define CONNECTED	MFSDK_COMM_WIFI_STATE_CONNECT
#define CONNECT_BEGIN	MFSDK_COMM_WIFI_STATE_CONNECT_START
#define CONNECTING	MFSDK_COMM_WIFI_STATE_CONNECT_PROCESS
#define MAX_AP_COUNT	10

#define _WIFI_TASK_PRIO		(4)
#define _APP_TASK_SIZE		4096
static  unsigned int  pTaskStk[_APP_TASK_SIZE];

static lv_obj_t* scan_win = NULL;
//static lv_obj_t* list_win = NULL;
//static lv_obj_t* input_win = NULL;
static lv_obj_t* link_win = NULL;
static lv_obj_t* m_parent = NULL;
//static int listecn[40] = { 0 };

static st_wifi_ap_list wifi_ap_list[MAX_AP_COUNT+1] = { 0 };
static int wifi_ap_count = 0;
static int select_index = 0;
static int input_ret = 0;
static char wifi_pwd[128] = { 0 };
static int wifi_process = 0;
static int exit_process = 0;
static lv_obj_t* mbox_wifi = NULL;
//static lv_obj_t* mbox_bg = NULL;
//static lv_task_t* task = NULL;

static void exit_scan_page()
{
	exit_process = 1;
	m_parent = NULL;
	link_win = NULL;
	scan_win = NULL;
	mbox_wifi = NULL;
	//list_win = NULL;
	//input_win = NULL;
	//wifi_set_scan(0);
	comm_list_ap_quit();
	if (wifilist_img != NULL)
	{
		lv_obj_del(wifilist_img);
		wifilist_img = NULL;
	}
}

static int _wifi_connect_result_func(int ret,lv_obj_t* obj)
{
	APP_TRACE("_wifi_connect_result_func:%d\r\n", ret);
	if (ret <= 0) {
		exit_scan_page();
	}
	return 1;
}


static int _wifi_connecting_func(int ret,lv_obj_t* obj)
{
	APP_TRACE("_wifi_connecting_func:%d\r\n", ret);
	if (ret <= 0) {
		exit_scan_page();
	}
	return 1;
}

static int _wifi_input_password_func(int ret,lv_obj_t* obj)
{
	APP_TRACE("_wifi_input_password_func:%d\r\n", ret);
	if (ret < 0) {
		exit_scan_page();
	}
	input_ret = ret;
	return 1;
}

static int page_return_wifi_saved_func(int ret,lv_obj_t* obj)
{
	APP_TRACE("page_return_wifi_saved_func:%d\r\n", ret);
	exit_scan_page();
	return 1;
}

static int _wifi_list_select_func(int ret,lv_obj_t* obj)
{

	APP_TRACE("_wifi_list_select_func:%d,mbox_wifi:%x\r\n", ret,mbox_wifi);
	if (mbox_wifi == NULL) {
		APP_TRACE("ret:%d, state = %d\r\n", ret, MfSdkCommWifiGetLinkState());
		APP_TRACE("wifi_ap_list[ret].ssid = %s\r\n", wifi_ap_list[ret].ssid);
		APP_TRACE("wifi_get_ssid() = %s\r\n", MfSdkCommWifiGetSsid());
		//选中已连接并且有网络
		if((ret>=0)&&(strcmp(wifi_ap_list[ret].ssid,(char*)MfSdkCommWifiGetSsid()) == 0) && MfSdkCommWifiGetLinkState()== CONNECTED){
			APP_TRACE("wifi_get_ssid():%s,comm_wifi_get_link_state():%d",(char*)MfSdkCommWifiGetSsid(),MfSdkCommWifiGetLinkState());
			#if 1
			if (mbox_wifi == NULL)
			{
				char msg_buff[100] = { 0 };
				//static const char* btn_str[] = { "OK",  "" };
				//mbox_bg = lv_obj_create(lv_scr_act(), NULL);
				//lv_obj_set_style_local_bg_opa(mbox_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
				//lv_obj_set_size(mbox_bg, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
			    //mbox_wifi = lv_msgbox_create(lv_scr_act(), NULL);
				sprintf(msg_buff, "Already linked WIFI:%s", (char*)MfSdkCommWifiGetSsid());
				#if 0
				lv_msgbox_set_text(mbox_wifi, msg_buff);
				lv_msgbox_add_btns(mbox_wifi, btn_str);
				lv_obj_set_user_data(mbox_wifi, (lv_obj_user_data_t)wifi_get_ssid());
				lv_obj_align(mbox_wifi, NULL, LV_ALIGN_CENTER, 0, 0);
				//lv_obj_set_event_cb(mbox_wifi, msgbox_event_handler1);
				//lv_msgbox_set_anim_time(mbox_wifi, 100);
				page_group_set_obj(mbox_wifi);
				#endif
				page_message_show_ex(obj, page_return_wifi_saved_func, "wifi set", msg_buff, "", "OK", 3*1000, 0);
			}
			return 0;
			#endif
		}
		if (ret < 0) {
			APP_TRACE("exit_scan_page\r\n");
			exit_scan_page();
		}
		select_index = ret;
		APP_TRACE("select_index = %d\r\n",select_index);
		//wifi_set_scan(0);//停止扫描防止更新扫描链表
		comm_list_ap_quit();
		return 1;
	}
	else {
		APP_TRACE("here is mbox\r\n");
		return 0;
	}
}
#if 0
static void msgbox_event_handler(lv_obj_t *obj, lv_event_t evt)
{
    char btn_id = -1;
    if(evt == LV_EVENT_DELETE && obj) {
        
    } else if(evt == LV_EVENT_VALUE_CHANGED) {
        /* A button was clicked */
		btn_id = lv_msgbox_get_active_btn(obj);
		if (btn_id == 0) //ok
		{ 
			char* ssid = (char *)lv_obj_get_user_data(obj);
			APP_TRACE("lv_obj_get_user_data :%s",ssid);
			if(strcmp(ssid,wifi_get_ssid())==0&&comm_wifi_get_link_state()>0){//已连接并且是当前wifi			
				comm_wifi_unlink_ap(ssid);
				wifi_remove_ap(ssid);
			}else{
				wifi_remove_ap(ssid);
			}
		}
		
		page_list_reset_select();

		page_list_focus_btn_index(page_list_seleted_index());
		
		//马上关闭消息对话框
		//lv_obj_del(mbox_bg);
		//lv_msgbox_stop_auto_close(mbox_wifi);
		lv_msgbox_start_auto_close(mbox_wifi, 0);
		mbox_wifi = NULL;
		mbox_bg = NULL;		
		
    }
}
static int _wifi_list_longpress_func(int ret)
{
	APP_TRACE("_wifi_list_longpress_func:%d,wifi_ap_count:%d\r\n", ret,wifi_ap_count);
	if(ret>=0 && ret<wifi_ap_count){
		char* ssid = wifi_ap_list[ret].ssid;
		APP_TRACE("_wifi_list_longpress_func ssid:%s",ssid);
		if (ssid) {
			if(mbox_wifi == NULL){
				char msg_buff[1024] = {0};
				static const char* btn_str[] = { "OK", "Cancel", "" };
				//mbox_bg = lv_obj_create(lv_scr_act(), NULL);				
				//lv_obj_set_style_local_bg_opa(mbox_bg,LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
				//lv_obj_set_size(mbox_bg, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
			    mbox_wifi = lv_msgbox_create(lv_scr_act(), NULL);
				sprintf(msg_buff,"Remove %s ap",ssid);
			    lv_msgbox_set_text(mbox_wifi, msg_buff);
			    lv_msgbox_add_btns(mbox_wifi, btn_str);
				lv_obj_set_user_data(mbox_wifi, (lv_obj_user_data_t)ssid);
			    lv_obj_align(mbox_wifi, NULL, LV_ALIGN_CENTER, 0, 0);
	    		lv_obj_set_event_cb(mbox_wifi, msgbox_event_handler);
				//lv_msgbox_set_anim_time(mbox_wifi, 100);

				page_group_set_obj(mbox_wifi);
			}
		}
	}
	
	return 0;
}
#endif
static lv_obj_t * _wifi_list_show(lv_obj_t* parent,int select_index)
{
	char* listitem[40] = {0};
	
	st_wifi_ap_list *wifi_net_list =  0 ;
	lv_obj_t* ret;

	int i = 0,jj = 0,bFound = 0,wifi_net_count = 0;
	int max_wifi_count = MAX_AP_COUNT;
	
	wifi_net_list =(st_wifi_ap_list *) malloc(sizeof(st_wifi_ap_list) * max_wifi_count);
	memset(wifi_net_list, 0x00, sizeof(st_wifi_ap_list) * max_wifi_count);
	for (i = 0; i < max_wifi_count; i++) {
		listitem[i] = malloc(256);
	}
	APP_TRACE("_wifi_list_show parent:%d,select_index:%d",parent,select_index);
	//wifi_net_count = wifi_list_network(wifi_net_list);
	APP_TRACE("_wifi_list_show wifi_net_count:%d",wifi_net_count);
	for (i = 0; i < wifi_ap_count; i++) 
	{
		#if 0
		if (wifi_ap_list[i].ecn == 0) {
			sprintf(listitem[i], "%s(OPEN)(%d)", wifi_ap_list[i].ssid, wifi_ap_list[i].rssi);
		}else if (wifi_ap_list[i].ecn == 1) {
			sprintf(listitem[i], "%s(WEP)(%d)", wifi_ap_list[i].ssid, wifi_ap_list[i].rssi);
		}else {
			sprintf(listitem[i], "%s(PSK)(%d)", wifi_ap_list[i].ssid, wifi_ap_list[i].rssi);
		}
		#endif
		APP_TRACE("wifi_net_list[%d] = %s",i,wifi_net_list[i].ssid);
		sprintf(listitem[i], "%s(%d)", wifi_ap_list[i].ssid, wifi_ap_list[i].rssi);
		APP_TRACE("list_show:%d.%s\r\n", i, listitem[i]);
		bFound = 0;
		for(jj = 0;jj<wifi_net_count;jj++){
			APP_TRACE("_wifi_list_show wifi_net_list ssid:%s,wifi_ap_list ssid:%s",wifi_net_list[jj].ssid,wifi_ap_list[i].ssid);
			if(strcmp(wifi_net_list[jj].ssid,wifi_ap_list[i].ssid) == 0)
			{
				bFound = 1;
				break;
			}
		}
		if(bFound){
			sprintf(listitem[i], "%s(SAVE)", listitem[i]);
			//APP_TRACE("found liststr[i]:%s", listitem[i]);
		}
		if((MfSdkCommWifiGetLinkState() == CONNECTED)&&(strcmp(wifi_ap_list[i].ssid,(char*)MfSdkCommWifiGetSsid()) == 0)){
			sprintf(listitem[i], "%s *", listitem[i]);
			APP_TRACE("current connected wifi :%s",wifi_ap_list[i].ssid);
		}
	
		//listitem[i] = liststr[i];
	}
	
	//ret = page_list_show_extend(m_parent, _wifi_list_select_func, _wifi_list_longpress_func, "wifi list", listitem, wifi_ap_count, select_index < wifi_ap_count?select_index:wifi_ap_count, 0, 1);
	ret = page_list_show_ex(m_parent, _wifi_list_select_func, "wifi list", listitem, wifi_ap_count, select_index < wifi_ap_count?select_index:wifi_ap_count , 60*1000 , 0);

	for (i = 0; i < max_wifi_count; i++) {
		free(listitem[i]);
	}
	free(wifi_net_list);
	APP_TRACE("wifi list show end %x\r\n", ret);
	return ret;
}

static void _wifi_task(void* task)
{
	int ret = 0,ii = 0;
	char msg[128] = { 0 };
	char current_ssid[128] = {0};
	unsigned int tick1;
	unsigned int tick_list;
	int count = 0;
	lv_obj_t *wifi_list_page = NULL;
	st_wifi_ap_list tmp = { 0 };

	MFSDK_UNUSED(tick_list);
	MFSDK_UNUSED(ii);

	memset(&wifi_ap_list, 0x00, sizeof(st_wifi_ap_list)*(MAX_AP_COUNT+1));
	wifi_ap_count = comm_wifi_list_ap(wifi_ap_list);
	APP_TRACE("comm_wifi_list_ap:%d\r\n", wifi_ap_count);
	
	//comm_wifi_clear_list_ap_num();
	//comm_wifi_set_scan(2);
	lv_start_lock(1);
	page_list_reset_seleted_index();
	lv_start_lock(0);

	tick1 = MfSdkSysGetTick();

	
	while (MfSdkSysCheckTick(tick1, 30000) == 0) {		// wait scan
		memset(&wifi_ap_list, 0x00, sizeof(st_wifi_ap_list)*(MAX_AP_COUNT+1));
		wifi_ap_count = comm_wifi_list_ap(wifi_ap_list);
		APP_TRACE("process wifi_ap_count %d\r\n",wifi_ap_count);
		if(wifi_ap_count > 0) {
			APP_TRACE("process wifi_ap_count %d,0 ssid:%s\r\n",wifi_ap_count,wifi_ap_list[0].ssid);
			break;
		}
		if (exit_process) {
			APP_TRACE("exit_process comm_wifi_list_ap exit\r\n");
			wifi_process = 0;
			MfSdkSysTaskDel(_WIFI_TASK_PRIO);
			return;
		}
		MfSdkSysSleep(500);
		
	}

	lv_start_lock(1);
	APP_TRACE("process scan_win end %x,exit_process:%d\r\n",scan_win,exit_process);
	if(scan_win&&(exit_process == 0)){
		APP_TRACE("scan_win:%x", scan_win);
		lv_obj_del(scan_win);
		scan_win = NULL;
	}
	lv_start_lock(0);

	if (wifi_ap_count <= 0){ 
		APP_TRACE("exit_process wifi_ap_count <= 0");
		wifi_process = 0;
		MfSdkSysTaskDel(_WIFI_TASK_PRIO);
		return;
	}
	lv_start_lock(1);
	select_index = 999;
	APP_TRACE("process m_parent end %x,exit_process:%d\r\n",m_parent,exit_process);
	if (m_parent && exit_process == 0&&wifi_ap_count>0)
	{
		#if 1
		for (int i = 0; i < wifi_ap_count - 1; i++)
		{
			for (int j = 0; j < wifi_ap_count - 1 - i; j++)
			{
				if (wifi_ap_list[j].rssi < wifi_ap_list[j + 1].rssi)
				{
					tmp = wifi_ap_list[j + 1];
					wifi_ap_list[j + 1] = wifi_ap_list[j];
					wifi_ap_list[j] = tmp;
				}
			}
		}
		#endif
		wifi_list_page = _wifi_list_show(m_parent,0);
	}	
	lv_start_lock(0);

	tick_list = MfSdkSysGetTick();

	set_refresh_wifi_list_flag(0);

	while (select_index == 999){		// wait select ap
		APP_TRACE("process select_index %d\r\n",select_index);
		if (exit_process) {
			APP_TRACE("exit_process wait select ap exit");
			wifi_process = 0;
			MfSdkSysTaskDel(_WIFI_TASK_PRIO);
			return;
		}
		MfSdkSysSleep(100);
		//if(osl_CheckTimeover(tick_list,2000) == 1){
		if (get_refresh_wifi_list_flag() == 1) {
			page_message_show_ex(m_parent, 0, "wifi list", "scan...", "", "", 0, 0);
			if(count&&(count%8 == 0)){
				APP_TRACE("====wifi_set_scan====");			
				//wifi_set_scan(1);
			}			
			APP_TRACE("comm_wifi_list_ap again");
			memset(&wifi_ap_list, 0x00, sizeof(st_wifi_ap_list)*(MAX_AP_COUNT+1));
			wifi_ap_count = comm_wifi_list_ap(wifi_ap_list);
			#if 1 
			for (int i = 0; i < wifi_ap_count - 1; i++)
			{
				for (int j = 0; j < wifi_ap_count - 1 - i; j++)
				{
					if (wifi_ap_list[j].rssi < wifi_ap_list[j + 1].rssi)
					{
						tmp = wifi_ap_list[j + 1];
						wifi_ap_list[j + 1] = wifi_ap_list[j];
						wifi_ap_list[j] = tmp;
					}
				}
			}
			#endif
			wifi_message_close_page();
			lv_start_lock(1);
			APP_TRACE("process select_index end %x,exit_process:%d,select_index:%d,wifi_ap_count:%d\r\n",m_parent,exit_process, select_index,wifi_ap_count);
			if (m_parent && exit_process == 0&&(select_index == 999)&&wifi_ap_count>0) {
				int sel_index = page_list_seleted_index();
				APP_TRACE("sel_index = %d\r\n",sel_index);
				lv_obj_t *scrl = lv_page_get_scrollable(page_list_get_lv_list(wifi_list_page));
				//lv_coord_t scrl_y = lv_obj_get_y(scrl);
				if(wifi_list_page){
					APP_TRACE("page_list_delete1\r\n");
					page_list_delete(wifi_list_page);
				}
				APP_TRACE("_wifi_list_show m_parent:%d,select_index:%d",m_parent,sel_index);
				wifi_list_page = _wifi_list_show(m_parent,sel_index);
				APP_TRACE("wifi_list_page:%x",wifi_list_page);
				scrl = lv_page_get_scrollable(page_list_get_lv_list(wifi_list_page));
				#if 0
				lv_obj_set_y(scrl,scrl_y);
				if(mbox_wifi)
					page_group_set_obj(mbox_wifi);
				focuse_power_page();
				#endif
				MFSDK_UNUSED(scrl);
			}	
			lv_start_lock(0);
			tick_list = MfSdkSysGetTick();
			count++;
			set_refresh_wifi_list_flag(0);
		}
	}
	APP_TRACE("xxxx select_index = %d\r\n",select_index);
	if (select_index < 0) {		// cancel
		APP_TRACE("exit_process cancel select ap exit");
		wifi_process = 0;
		MfSdkSysTaskDel(_WIFI_TASK_PRIO);
		return;
	}
	else
	{
		//page_list_delete(wifi_list_page);
	}

	memset(wifi_pwd, 0x00, sizeof(wifi_pwd));	

	//waiting connect result 
	memset(current_ssid,0,sizeof(current_ssid));
	strcpy(current_ssid,wifi_ap_list[select_index].ssid);
	#if 0
	/*
	1.已经saved状态的，选择该AP，直接连接（不要弹输密！）
	2.若1中AP端改密导致连接失败，提示连接失败，并直接弹输密。
	3.无saved状态的，选择该AP，正常弹输密。
	4.remove saved状态的ap后，选择该AP，正常弹输密。
	*/
	int found_saved = 0;
	memset(&wifi_ap_list, 0x00, sizeof(st_wifi_ap_list)*(MAX_AP_COUNT+1));
	//wifi_ap_count = comm_wifi_list_ap(wifi_ap_list);
	APP_TRACE("==wifi_ap_count == %d\r\n", wifi_ap_count);
	for(ii = 0;ii<wifi_ap_count;ii++)
	{
		if(strcmp(wifi_ap_list[ii].ssid,current_ssid) == 0){
			found_saved = 1;
			break;
		}
	}
	if(found_saved == 1){
		//memset(wifi_pwd,0,sizeof(wifi_pwd));
		//已保存直接连接
		memset(&tmp, 0, sizeof(tmp));
		memcpy(&tmp, &(wifi_ap_list[select_index]),sizeof(st_wifi_ap_list));
		ret =  comm_wifi_link_ap(&tmp, wifi_pwd);
		APP_TRACE("comm_wifi_link_ap ret == %d\r\n",ret);
		lv_start_lock(1);
		APP_TRACE("process link_win end %d,m_parent:%d,exit_process:%d\r\n",link_win,m_parent,exit_process);
		if (m_parent && exit_process == 0) {
			link_win = page_message_show_ex(m_parent, _wifi_connecting_func, "wifi set", "connecting...", "", "", 0, 0);
		}
		lv_start_lock(0);
		tick1 = osl_GetTick();
		while (osl_CheckTimeover(tick1,10000) == 0 && ((comm_wifi_get_link_state() == CONNECT_BEGIN || comm_wifi_get_link_state() == CONNECTING) || strcmp(current_ssid,wifi_get_ssid()))) {		//连接等待状态
			APP_TRACE("comm_wifi_link_ap state=%d\r\n",comm_wifi_get_link_state());
			if (exit_process) {
				APP_TRACE("exit_process save wifi link");
				wifi_process = 0;
				return;
			}
			Sys_Sleep(100);
		}
		
		lv_start_lock(1);
		APP_TRACE("wait scan end link_win end %d,m_parent:%d,exit_process:%d\r\n",link_win,m_parent,exit_process);
		if(link_win&& exit_process == 0){
			APP_TRACE("link_win:%x", link_win);
			lv_obj_del(link_win);
			link_win = NULL;
		}
		lv_start_lock(0);			
		if(comm_wifi_get_link_state() == CONNECTED){//提示连接成功		
			lv_start_lock(1);
			APP_TRACE("connection end m_parent:%d,exit_process:%d\r\n",m_parent,exit_process);
			if (m_parent && exit_process == 0) {
				page_message_show_ex(m_parent, _wifi_connect_result_func, "wifi set", "connection successful", "cancel", "confirm", 0, 0);
			}
			lv_start_lock(0);
			wifi_process = 0;
			//wifi_set_scan(0);
			OSTaskDel(_WIFI_TASK_PRIO);
			return;
		}else{//提示连接失败，直接输入密码
			lv_start_lock(1);
			char msg_buff[1024] = {0};
			static const char* btn_str[] = { "OK", "" };
		    lv_obj_t *mbox_tip = lv_msgbox_create(lv_scr_act(), NULL);
			sprintf(msg_buff,"connect %s failed",current_ssid);
		    lv_msgbox_set_text(mbox_tip, msg_buff);
		    lv_msgbox_add_btns(mbox_tip, btn_str);
		    lv_obj_align(mbox_tip, NULL, LV_ALIGN_CENTER, 0, 0);
			lv_msgbox_start_auto_close(mbox_tip,1000);
			lv_start_lock(0);
		}
	}
	#endif
	lv_start_lock(1);	
	input_ret = 999;
	APP_TRACE("process input_win end m_parent:%d,exit_process:%d\r\n",m_parent,exit_process);
	if (m_parent && exit_process == 0) {
		APP_TRACE("current_ssid = %s\r\n",current_ssid);
		_mf_wifiinput_show(m_parent,_wifi_input_password_func, current_ssid, wifi_pwd,sizeof(wifi_pwd),60000);
		//input_win = page_input_show_ex(m_parent, _wifi_input_password_func, "input password", wifi_pwd, 16, PAGE_INPUT_MODE_ALPHABET, 60000, 0);
	}	
	
	lv_start_lock(0);

	MfSdkPowerLockApp((char*)"_wifi_task");
	while (input_ret == 999) {		// 等待输入密码
		APP_TRACE("process input_ret %d\r\n",input_ret);
		if (exit_process) {
			APP_TRACE("exit_process input password cancel exit");
			wifi_process = 0;
			MfSdkPowerUnlockApp();
			MfSdkSysTaskDel(_WIFI_TASK_PRIO);
			return;
		}
		MfSdkSysSleep(100);
	}

	if (input_ret < 0) {		// 取消退出
		APP_TRACE("exit_process input password exit");
		wifi_process = 0;
		MfSdkPowerUnlockApp();
		MfSdkSysTaskDel(_WIFI_TASK_PRIO);
		return;
	}
	

	lv_start_lock(1);
	APP_TRACE("process link_win end %d,m_parent:%d,exit_process:%d\r\n",link_win,m_parent,exit_process);
	if (m_parent && exit_process == 0) {
		link_win = page_message_show_ex(m_parent, _wifi_connecting_func, "wifi set", "connecting...", "", "", 0, 1);
	}
	lv_start_lock(0);
	
	APP_TRACE("comm_wifi_get_link_state:%d\r\n", MfSdkCommWifiGetLinkState());
	if (MfSdkCommWifiGetLinkState() == CONNECTED) {
	//	comm_wifi_unlink_ap();
	}

	APP_TRACE("current_ssid:%s wifi_pwd:%s\r\n", current_ssid, wifi_pwd);
	memset(&tmp, 0, sizeof(tmp));
	memcpy(&tmp, &(wifi_ap_list[select_index]),sizeof(st_wifi_ap_list));
	ret =  comm_wifi_link_ap(&tmp, wifi_pwd);
	APP_TRACE("comm_wifi_link_ap ret:%d\r\n", ret);
	ret = -1;
	tick1 = MfSdkSysGetTick();
	while (MfSdkSysCheckTick(tick1, 30000) == 0) {		// wait scan
		APP_TRACE("process wait scan state:%d wifi_get_ssid:%s,ssid:%s \r\n",MfSdkCommWifiGetLinkState(),(char*)MfSdkCommWifiGetSsid(),current_ssid);
		if (exit_process) {
			APP_TRACE("exit_process wait scan exit");
			wifi_process = 0;
			MfSdkPowerUnlockApp();
			MfSdkSysTaskDel(_WIFI_TASK_PRIO);
			return;
		}			
		MfSdkSysSleep(500);
		APP_TRACE("wifi_get_ssid = %s, current_ssid = %s\r\n",(char*)MfSdkCommWifiGetSsid(),current_ssid);
		if (MfSdkCommWifiGetLinkState() == CONNECTED && strcmp((char*)MfSdkCommWifiGetSsid(), current_ssid) == 0) {
			APP_TRACE("wifi_get_ssid = %s\r\n",(char*)MfSdkCommWifiGetSsid());
			ret = 0;
			break;
		}
	}
	
	lv_start_lock(1);
	APP_TRACE("wait scan end link_win end %d,m_parent:%d,exit_process:%d\r\n",link_win,m_parent,exit_process);
	if(link_win&& exit_process == 0){
		APP_TRACE("link_win:%x", link_win);
		lv_obj_del(link_win);
		link_win = NULL;
	}
	lv_start_lock(0);

	sprintf(msg, "connection:%s", (ret == 0) ? "successful" : "failed");

	lv_start_lock(1);
	APP_TRACE("connection end m_parent:%d,exit_process:%d\r\n",m_parent,exit_process);
	if (m_parent && exit_process == 0) {
		APP_TRACE("msg:%s\r\n",msg);
		page_message_show_ex(m_parent, _wifi_connect_result_func, "wifi set", msg, "", "OK", 5*1000, 0);
		if(ret == 0)
			MfSdkAudPlay((const s8 *)"csuc");
		else
			MfSdkAudPlay((const s8 *)"cfail");
	}
	lv_start_lock(0);
	wifi_process = 0;
	//wifi_set_scan(0);
	comm_list_ap_quit();
	APP_TRACE("exit_process normal exit wifi_process:%d",wifi_process);
	MfSdkPowerUnlockApp();
//	OSTaskDel(_WIFI_TASK_PRIO);

	MfSdkSysTaskDel(_WIFI_TASK_PRIO);
	
	return;
}
static int page_return_wifi_func(int ret,lv_obj_t* obj)
{
	APP_TRACE("page_return_wifi_func:%d\r\n", ret);
	//if(ret == -1 || ret == 0){
		exit_scan_page();
	//}
	return 1;
}

int func_wifi_show(lv_obj_t* parent)
{
	APP_TRACE("============func_wifi_show wifi_process:%d==============",wifi_process);
	mbox_wifi = NULL;
	
	if(wifi_process == 0&&parent){
		lv_obj_set_hidden(parent, false);
		page_group_set_obj(parent);
		m_parent = parent;
		scan_win = page_message_show_ex(m_parent, page_return_wifi_func, "wifi list", "scan...", "", "", 0, 0);
		wifi_process = 1;
		exit_process = 0;
		MfSdkPowerLockApp((char*)"func_wifi_show");
		MfSdkSysTaskCreate(_wifi_task, _WIFI_TASK_PRIO,(s8*)&(pTaskStk[0]),_APP_TASK_SIZE);
	}else{
		//exit_process = 1;
	}
	return 0;
}

int func_wifi_info(lv_obj_t* parent)
{
	char msg[512] = { 0 };

	if (MfSdkCommWifiGetLinkState() != CONNECTED) {
		sprintf(msg + strlen(msg), "not connect\r\n");
	}
	else {
		sprintf(msg + strlen(msg), "ssid:%s\r\n", (char*)MfSdkCommWifiGetSsid());
		sprintf(msg + strlen(msg), "rssi:%d(%d)\r\n", MfSdkCommWifiGetSignal(), MfSdkCommWifiGetRssi());
		sprintf(msg + strlen(msg), "channel:%d\r\n", MfSdkCommWifiGetChannel());
		sprintf(msg + strlen(msg), "ap mac:%s\r\n",(char*) MfSdkCommWifiGetApMac());
		sprintf(msg + strlen(msg), "ip:%s\r\n", (char*)MfSdkCommWifiGetLocalip());
	}
	
	sprintf(msg + strlen(msg), "mac:%s\r\n",(char*)MfSdkCommWifiGetLocalMac());

	//printf("func_wifi_info1:%s\r\n", msg);
	page_message_show_ex(parent, 0, "WIFI info", msg, "", "OK", 0,0);
	//printf("func_wifi_info2\r\n");
	return 0;
}
lv_obj_t* wifilist_img = NULL;
lv_obj_t* get_wifilist_img()
{
	return wifilist_img;
}
lv_obj_t* set_wifilist_img(lv_obj_t* wifilist_img_in)
{
	wifilist_img = wifilist_img_in;
	return wifilist_img;
}
void func_wifi_show_ex()
{
	if (wifilist_img != NULL)
	{
		lv_obj_clean(wifilist_img);
		lv_obj_del(wifilist_img);
	}
	wifilist_img = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_size(wifilist_img, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
	
	lv_obj_set_hidden(wifilist_img, true);
	func_wifi_show(wifilist_img);
}
