
#include "libapi_xpos/inc/libapi_comm.h"
#include "page_status_bar.h"
#include "libapi_xpos/inc/mfsdk_comm.h"
#include "libapi_xpos/inc/mfsdk_sys.h"

static lv_obj_t* img_wifi;
static int wifi_signal_index = -1;

/*wifi*/
static char *wifi_get_img(int index)
{

	if (index == 1)	return "P:exdata/wifi1.png";
	if (index == 2)	return "P:exdata/wifi2.png";
	if (index == 3)	return "P:exdata/wifi3.png";
	if (index == 4)	return "P:exdata/wifi4.png";
	return "P:exdata/wifi0.png";
}

void state_wifi_page_init(void* state, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs)
{
	lv_obj_t* page_state = (lv_obj_t*)state;

	lv_load_png_file(wifi_get_img(0));
	lv_load_png_file(wifi_get_img(1));
	lv_load_png_file(wifi_get_img(2));
	lv_load_png_file(wifi_get_img(3));
	lv_load_png_file(wifi_get_img(4));

	img_wifi = lv_img_create(page_state, NULL);
	lv_img_set_src(img_wifi, wifi_get_img(0));
	lv_obj_align(img_wifi, NULL, align, x_ofs, y_ofs);
	if(MfSdkCommGetNetMode() != MFSDK_COMM_NET_ONLY_WIFI || 0 == MfSdkCommWifiChipExist())
	{	
		lv_obj_set_hidden(img_wifi, 1);
	}

	return;
}


static lv_obj_t* img_atc;
static int atc_signal_index = -1;

/*gprs*/
static char *atc_get_img(int index)
{

	if (index == 1)	return "P:exdata/signal1.png";
	if (index == 2)	return "P:exdata/signal2.png";
	if (index == 3)	return "P:exdata/signal3.png";
	if (index == 4)	return "P:exdata/signal4.png";
	if (index == 5)	return "P:exdata/signal5.png";
	return "P:exdata/signal0.png";
}

void state_atc_page_init(void* state, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs)
{
	lv_obj_t* page_state = (lv_obj_t*)state;

	lv_load_png_file(atc_get_img(0));
	lv_load_png_file(atc_get_img(1));
	lv_load_png_file(atc_get_img(2));
	lv_load_png_file(atc_get_img(3));
	lv_load_png_file(atc_get_img(4));
	lv_load_png_file(atc_get_img(5));

	img_atc = lv_img_create(page_state, NULL);
	lv_img_set_src(img_atc, atc_get_img(0));
	lv_obj_align(img_atc, NULL, align, x_ofs, y_ofs);
	if(MfSdkCommGetNetMode() != MFSDK_COMM_NET_ONLY_WIRELESS)
	{	
		lv_obj_set_hidden(img_atc, 1);
	}

	return;
}

static void signal_task_func(lv_task_t* task)
{
	int index = 0;
	
	if(MfSdkCommGetNetMode() != MFSDK_COMM_NET_ONLY_WIRELESS)
	{	
		lv_obj_set_hidden(img_atc, 1);	
		lv_obj_set_hidden(img_wifi, 0);		
		index = MfSdkCommWifiGetSignal();
		APP_TRACE("MfSdkCommWifiGetSignal = %d\r\n", index);
		if (wifi_signal_index == -1 || wifi_signal_index != index)
		{
			APP_TRACE("wifi_get_img:%s\r\n",wifi_get_img(index));
			lv_img_set_src(img_wifi, wifi_get_img(index));
			wifi_signal_index = index;
		}
	}
	else
	{
		lv_obj_set_hidden(img_atc, 0);		
		lv_obj_set_hidden(img_wifi, 1);		
		index = MfSdkCommGsmGetSignal();
		APP_TRACE("MfSdkCommGsmGetSignal = %d\r\n", index);
		if (atc_signal_index == -1 || atc_signal_index != index)
		{
			APP_TRACE("atc_get_img:%s\r\n",atc_get_img(index));
			lv_img_set_src(img_atc, atc_get_img(index));
			atc_signal_index = index;
		}
	}
	return;
}

void state_signal_page_init(void* state, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs)
{
	state_wifi_page_init(state, align, x_ofs, y_ofs);
	state_atc_page_init(state, align, x_ofs, y_ofs);

	lv_task_create(signal_task_func, 1000, LV_TASK_PRIO_MID, NULL);
	return;
}

/*power*/
static lv_obj_t* img_power = NULL;
static int power_index = -1;
static int power_status = -1;

static char* get_power_img(int index, int status)
{
	if(0 == status)
	{
		if (index == 1)	return "P:exdata/power1.png";
		if (index == 2)	return "P:exdata/power2.png";
		if (index == 3)	return "P:exdata/power3.png";
		if (index == 4)	return "P:exdata/power4.png";
		if (index == 5)	return "P:exdata/power5.png";

		return  "P:exdata/power0.png";
	}
	else
	{
		if (index == 1)	return "P:exdata/power11.png";
		if (index == 2)	return "P:exdata/power12.png";
		if (index == 3)	return "P:exdata/power13.png";
		if (index == 4)	return "P:exdata/power14.png";
		//if (index == 5)	return "P:exdata/power15.png";

		return  "P:exdata/power10.png";
	}
}

static void power_task_func(lv_task_t* task)
{
	int index = 0;
	int status = 0;
	MfSdkBatterAttr_T batterySts;

	memset(&batterySts, 0, sizeof(MfSdkBatterAttr_T));
	status = MfSdkSysGetBatterStatus(&batterySts);
	index = batterySts.voltage_level -1;
	if (index < 0 || index > 5){
		index = 0;
	}


	APP_TRACE("power_task_func index:%d, status:%d", index, status);
	if (power_index != index || power_status != status) 
	{
		power_index = index;
		power_status = status;
		lv_img_set_src(img_power, get_power_img(index, status));
	}
	return;
}

void state_power_page_init(lv_obj_t* state, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs)
{
	lv_obj_t* page_state = state;
	int index = 0;
	int status = 0;
	MfSdkBatterAttr_T batterySts;

	memset(&batterySts, 0, sizeof(MfSdkBatterAttr_T));
	status = MfSdkSysGetBatterStatus(&batterySts);
	index = batterySts.voltage_level -1;
	if (index < 0 || index > 5){
		index = 0;
	}

	lv_load_png_file(get_power_img(0, 0));
	lv_load_png_file(get_power_img(1, 0));
	lv_load_png_file(get_power_img(2, 0));
	lv_load_png_file(get_power_img(3, 0));
	lv_load_png_file(get_power_img(4, 0));
	lv_load_png_file(get_power_img(5, 0));
	
	lv_load_png_file(get_power_img(0, 1));
	lv_load_png_file(get_power_img(1, 1));
	lv_load_png_file(get_power_img(2, 1));
	lv_load_png_file(get_power_img(3, 1));
	lv_load_png_file(get_power_img(4, 1));

	img_power = lv_img_create(page_state, NULL);
	lv_obj_set_width(img_power, 44);
	lv_img_set_src(img_power, get_power_img(index, status));
	lv_obj_align(img_power, NULL, align, x_ofs, y_ofs);
	lv_task_create(power_task_func, 1000, LV_TASK_PRIO_MID, NULL);
	return;
}

