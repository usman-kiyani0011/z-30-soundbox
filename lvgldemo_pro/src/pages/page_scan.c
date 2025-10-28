#include "lvgl/lvgl.h"
#include "pages.h"
#include "page_scan.h"
#include "tracedef.h"

#include "libapi_xpos/inc/mfsdk_gui.h"
#include "libapi_xpos/inc/mfsdk_power.h"

#include "../pub/http.h"
#include "../sdk_tts.h"

#define	SCANCODE_MAXLENTH (1024)

static lv_task_t* task_scan = NULL;
static lv_obj_t* scan_body = NULL;

static int ticks = 0;
static int m_nscanflag = 0;
static unsigned char *result = NULL;
static scancode_param_t scancode_param = { 0 };

static page_close_page_func m_page_close_page_func = 0;

static void _open_scaner()
{
	MfSdkQrScannerSetPreview(1);
	MfSdkQrScannerOpen();
}

static void _close_scaner()
{
	MfSdkQrScannerClose();
}

static void _scan_close_page(int ret)
{
	APP_TRACE("_scan_close_page\r\n");
	m_nscanflag = 0;
	if(task_scan)
	{
		lv_task_del(task_scan);
		task_scan = 0;
	}
	if (scan_body != 0) 
	{
		lv_obj_del(scan_body);
		scan_body = 0;
	}

	if(result)
	{
		MfSdkMemFree(result);
		result = NULL;
	}

	MfSdkLcdSetNormalDirection();
	MfSdkSysTaskAppSet(_close_scaner);
	MfSdkSysSleep(50);//wait for scanner close
	//enable refresh UI
	MfSdkLcdAutoFlush(MFSDK_FALSE);
	if (m_page_close_page_func != 0) m_page_close_page_func(ret,NULL);
	m_page_close_page_func = 0;
	//AppPowerUnlockApp((char*)"scan");
}

static void scan_event_cb(lv_obj_t* obj, lv_event_t event)
{
	uint32_t key;

	if (event == LV_EVENT_KEY)
	{
		MfSdkLcdBackLight(MFSDK_LCD_ON);
		key = pub_page_get_key();
		APP_TRACE("scan_event_cb key=%d\r\n",key);
		if (key == MF_LV_KEY_CANCEL_SHORT_PRESS)
		{
			_scan_close_page(PAGE_RET_CANCEL);
			AppFormatSegmentLedAmount(0LL);
			
		}
		else if (key == MF_LV_KEY_QUIT_LONG_PRESS)
		{
			MfSdkPowerOff();
		}
	}
}

static void _scan_task_func(lv_task_t* task)
{
	int  ret = 0;
	scancode_param_t *scan_result = (scancode_param_t *)task->user_data;

	if (m_nscanflag == 0 && NULL != result)
	{
		memset(result, 0x0, SCANCODE_MAXLENTH);
		APP_TRACE("osl_qrdecode_decode\r\n");
		ret = MfSdkQrDecode((s8*)result, SCANCODE_MAXLENTH);
		APP_TRACE("MfSdkQrDecode ret = %d\r\n", ret);
		if (ret > 0)
		{
			PubMultiPlay((const s8 *)TTS_VOLUME_NOR);
			APP_TRACE("######qrdecode %s\r\n", result);
			if(NULL != scan_result->scanRes && strlen((char*)result) < scan_result->maxLenth)
			{
				memcpy(scan_result->scanRes, result, ret);
				APP_TRACE("scan_result.scanRes[%d] %s\r\n", ret, scan_result->scanRes);
			}

			m_nscanflag = 1;
			_scan_close_page(PAGE_RET_CONFIRM);
		}
		else if(MfSdkSysTimerCheck(ticks) == 0) //timeout
		{
			_scan_close_page(PAGE_RET_TIMEOVR);
		}
	}
	
}

int ScanPage(lv_obj_t* parent, void* pfunc, char*amount, char*scancode, int maxlenth)
{
	result = MfSdkMemMalloc(SCANCODE_MAXLENTH);
	if(NULL == result)
	{
		APP_TRACE("result malloc error\r\n", result);
		return -1;
	}
	memset(result, 0x0, SCANCODE_MAXLENTH);

	memset(&scancode_param, 0, sizeof(scancode_param));
	scancode_param.maxLenth = maxlenth;
	scancode_param.scanRes = scancode;

	PubMultiPlay((const s8 *)"pshowqr.mp3");
	while (MfSdkAudTtsState() == 1)
	{
		MfSdkSysSleep(100);
	}
	MfSdkSysTaskAppSet(_open_scaner);

	if (scan_body != NULL)
	{
		lv_obj_del(scan_body);
	}
	//AppPowerLockApp((char*)"scan");
	m_page_close_page_func = (page_close_page_func)pfunc;
	scan_body = page_create_win(parent, scan_event_cb);
	
	ticks = MfSdkSysTimerOpen(120 * 1000);
	APP_TRACE("osl_scaner_open\r\n");
	task_scan = lv_task_create(_scan_task_func, 100, LV_TASK_PRIO_MID, &scancode_param);
	lv_task_ready(task_scan);

	//disable refresh UI
	MfSdkLcdAutoFlush(MFSDK_TRUE);
	return 0;
}

