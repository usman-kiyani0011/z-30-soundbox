#include "sdk_readcard.h"
#include "EntryPoint/lib_emvpub/inc/emv_interface.h"
//#include "pub/osl/inc/osl_log.h"
#include "tracedef.h"
#include "libapi_xpos/inc/mfsdk_util.h"
#include "libapi_xpos/inc/mfsdk_emv.h"
#include "libapi_xpos/inc/mfsdk_fs.h"

#define EMV_PARAMS_BUFF_MAX (2*1024)

#define EMV_PARAMS_FILE (const s8*)"xxxx\\emvpar.ini"
#define EMV_DEF_VALUE (const s8*)""

#define EMV_SECTION_CTRL (const s8*)"ctrlparams"
#define EMV_KEY_CTRL_AID (const s8*)"aidclear"
#define EMV_KEY_CTRL_CAPK (const s8*)"capkclear"


#define EMV_KEY_CONFIG (const s8*)"params"

#define EMV_SECTION_CONFIG (const s8*)"emvconfig"
#define EMV_SECTION_AID (const s8*)"aids"
#define EMV_SECTION_CAPK (const s8*)"capks"


static s32 AppLoadEmvParams(const s8* filename, const s8*sec, const s8*key)
{
	s8 temp[EMV_PARAMS_BUFF_MAX] = {0};
	s8 buf[EMV_PARAMS_BUFF_MAX/2] = {0};
	char format[128] = {0};
	s32 index = 0;
	int ret = SUCC;
	do{
		snprintf(format,sizeof(format),"%s%d",key,index);
		memset(temp,0,sizeof(temp));
		MfSdkFsReadProfileString(sec,(const s8*)format,temp,sizeof(temp),EMV_DEF_VALUE,filename);
		index++;
		TRACE("%s,%s : %s\r\n",sec ,format, temp);
		
		if(strlen((char*)temp) == 0) { break; }
		
		memset(buf,0,sizeof(buf));
		MfSdkUtilAsc2Bcd(temp,buf,strlen((char*)temp));

		if(strlen((char*)EMV_SECTION_AID) == strlen((char*)sec))
		{
			ret = MfSdkEmvSetAid((u8*)buf, strlen((char*)temp) / 2);
            TRACE("MfSdkEmvSetAid:%d\r\n", ret);
			if(ret != SUCC) { return ret; }
		}
		else
		{
			ret = MfSdkEmvSetCapk((u8*)buf, strlen((char*)temp) / 2);
            TRACE("MfSdkEmvSetCapk:%d\r\n", ret);
			if(ret != SUCC) { return ret; }
		}
	}while(strlen((char*)temp) > 0);

	return SUCC;
}

static int AppEmvLoadEmvTerminalParams(void)
{
	s8 temp[EMV_PARAMS_BUFF_MAX] = {0};
	s8 buf[EMV_PARAMS_BUFF_MAX/2] = {0};
	s32 ret = SUCC;
	
	memset(temp,0,sizeof(temp));
	MfSdkFsReadProfileString(EMV_SECTION_CONFIG,EMV_KEY_CONFIG,temp,sizeof(temp),EMV_DEF_VALUE,EMV_PARAMS_FILE);
	if(strlen((char*)temp) > 0)
	{
		memset(buf,0,sizeof(buf));
		MfSdkUtilAsc2Bcd(temp,buf,strlen((char*)temp));
		APP_TRACE_BUFF_TIP(buf,strlen((char*)temp)/2,(char*)EMV_KEY_CONFIG);
		ret = MfSdkEmvTerminalConfigInit((u8*)buf,strlen((char*)temp)/2);
		TRACE("MfSdkEmvTerminalConfigInit :%d\r\n",ret);
	}
	
	return ret;
}
static void AppEmvParamsInit(void)
{
	s32 flag = MfSdkFsReadProfileInt(EMV_SECTION_CTRL,EMV_KEY_CTRL_AID,0,EMV_PARAMS_FILE);
	if(flag != 0) { MfSdkEmvClearAllAid(); }
	TRACE("EMV_KEY_CTRL_AID flag:%d\r\n",flag);
	flag = MfSdkFsReadProfileInt(EMV_SECTION_CTRL,EMV_KEY_CTRL_CAPK,0,EMV_PARAMS_FILE);
	if(flag != 0) { MfSdkEmvDeleteAllCapk(); }
	TRACE("EMV_KEY_CTRL_CAPK flag:%d\r\n",flag);
}

int sdk_readcard_init(void)
{
	MfSdkEmvKernelInit();
	
	s32 x = MfSdkFsCheckPath(EMV_PARAMS_FILE);
	TRACE("MfSdkFsCheckPath x:%d\r\n",x);
	if(x == MFSDK_FS_RET_OK)
	{
		AppEmvParamsInit();
		
		x = AppEmvLoadEmvTerminalParams();
		if(x != SUCC) { return FAIL; }
		
		x = AppLoadEmvParams(EMV_PARAMS_FILE,EMV_SECTION_AID, (const s8*)"aid");	
		if(x != SUCC) { return FAIL; }
		
		x = AppLoadEmvParams(EMV_PARAMS_FILE,EMV_SECTION_CAPK, (const s8*)"capk");	
		if(x != SUCC) { return FAIL; }
		
		MfSdkFsUnlink(EMV_PARAMS_FILE);
	}

    TRACE("MfSdkEmvGetAidNum:%d\r\n", MfSdkEmvGetAidNum());
    TRACE("MfSdkEmvGetCapkNum:%d\r\n", MfSdkEmvGetCapkNum());

    return SUCC;
}

