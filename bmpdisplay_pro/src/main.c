#include "page_pub.h"
#include "tracedef.h"
#include <stdio.h>
#include "lvgl/lvgl.h"
#include "lvgl/src/lv_lib_png/lv_png.h"
#include "player_proc.h"
#include "libapi_xpos/inc/libapi_system.h"

#include "libapi_xpos/inc/mfsdk_define.h"
#include "libapi_xpos/inc/mfsdk_nfc.h"
#include "libapi_xpos/inc/mfsdk_lcd.h"
#include "libapi_xpos/inc/mfsdk_gui.h"
#include "libapi_xpos/inc/mfsdk_log.h"
#include "libapi_xpos/inc/mfsdk_fs.h"
#include "libapi_xpos/inc/mfsdk_tms.h"
#include "libapi_xpos/inc/mfsdk_aud.h"
#include "libapi_xpos/inc/mfsdk_comm.h"
#include "libapi_xpos/inc/mfsdk_sys.h"
#include "libapi_xpos/inc/mfsdk_emv.h"

#include "EntryPoint/lib_emvpub/inc/emv_interface.h"
#include "delfileinit.h"
#include "page_main.h"
#include "player_proc.h"
#include "sdk_readcard.h"
#include "./pub/queue_pub.h"
#include "./pub/http.h"

#define APP_VER                         "V1.5.7"
#define APP_VER_STR                     ((const s8*)"Q@#%Q@#%"APP_VER)
//#define LOG_DEBUG_MODE

static int   tts_system_play_fun(int type, void *data)
{
    switch(type)
    {
         case MFSDK_SYS_POWER_SHUTDOWN: // X_Power_ShutDown:
            {
                return 1;
            }

         case MFSDK_SYS_POWER_LOW: //X_Power_Low:
            {
                MfSdkAudPlay((const s8 *)"lowc.mp3");
                return 1;
            }

         case MFSDK_SYS_POWER_CHARGE://  X_Power_Charge:
            {
                MfSdkLcdBackLight(MFSDK_LCD_ON);
                return 1;
            }

         case MFSDK_SYS_POWER_OUT:// X_Power_Out:
            {
                return 1;
            }

         case MFSDK_SYS_NET_GPRS_MODE://X_Net_Gprs_Mode:
            {
                return 1;
            }

         case MFSDK_SYS_NET_WIFI_MODE://X_Net_Wifi_Mode:
            {
                MfSdkAudPlay((const s8 *)"wifi.mp3");
                return 1;
            }

         case MFSDK_SYS_NET_CONFIG_MODE://X_Net_Config_Mode:
            {
                MfSdkAudPlay((const s8 *)"cnet.mp3");
                return 1;
            }

         case MFSDK_SYS_WIFI_AIRKISS_CONFIG:// X_Wifi_AirKiss_Config:
            {
                return 1;
            }

         case MFSDK_SYS_WIFI_AP_CONFIG://X_Wifi_AP_Config:
            {
                return 1;
            }

         case MFSDK_SYS_WIFI_CONFIG_SUCCESS:// X_Wifi_Config_Success:
            {
                MfSdkAudPlay((const s8 *)"csuc.mp3");
                return 1;
            }

         case MFSDK_SYS_WIFI_CONFIG_FAIL:// X_Wifi_Config_Fail:
            {
                MfSdkAudPlay((const s8 *)"cfail.mp3");
                return 1;
            }

         case MFSDK_SYS_TMS_UPDATE_START://X_Tms_Update_Start:
            {
                MfSdkAudPlay((const s8 *)"updating.mp3");
                return 1;
            }

         case MFSDK_SYS_TMS_DOWN_FINISH_SUCC:// Tms_Down_Finish_Succ:
            {
                MfSdkAudPlay((const s8 *)"updasuc.mp3");

                while(MfSdkAudTtsState() == 1)
                {
                    MfSdkSysSleep(100);
                }

                return 1;
            }
    }

    return 0;
}

void app_lv_init()
{
    lv_drv_init();

    if(LCD_IS_320_480)
    {
        lv_load_png_file(MAINPNG_320X480);
    }
    else
    {
        lv_load_png_file(MAINPNG);
    }

}

static void  tms_download_fun(int size, int total_size)
{
    APP_TRACE("[%s] %d/%d \r\n", __FUNCTION__, size, total_size);
    int progress = 0;
    char tmp[10] = { 0 };
    progress = 100 * size / total_size;

    sprintf(tmp, "%d", progress);
    APP_TRACE("[%s]:[Progress bar:%s] \r\n", __FUNCTION__, tmp);
    MfSdkGuiLedAmount(tmp);
}

static void _app_work_task()
{
    MfSdkFsSetPath((const s8 *)"exdata");
	
    MfSdkSysTtsSystemSetFunc(tts_system_play_fun);
	
    MfSdkTmsSetProgressCallback(tms_download_fun);

	MfSdkSysTaskAppInit();

    //1:open log  0:close log
    MfSdkLogSoundSet(0);

    app_lv_init();
	
    if(MfSdkNfcInit() == MFSDK_NFC_RET_OK) { sdk_readcard_init(); }

    MfSdkCommMbedtlsInit(1, 0);

    MfSdkCommUartInit();

    delfile_init();
	
    orderCnt_lcd_init();
	
    fifo_init();
	
    MfSdkCommSetWifiName(0);
	
    fifo_init_ptr();
	
//    player_proc_init();

//    http_proc_init();

	AppInitUartRecvTask();
	
    page_main();
	
	MFSDK_UNUSED(HttpIdleflag);//suppressing warnings
}

void app_main(int p)
{
    MfSdkSysStart(APP_VER_STR, _app_work_task);
}

