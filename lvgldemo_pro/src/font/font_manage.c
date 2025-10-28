
#include "libapi_xpos/inc/mfsdk_fs.h"
#include "libapi_xpos/inc/mfsdk_mem.h"

#include "font_manage.h"
#include "../tracedef.h"

#define FONT_NAME			"font"

static uint8_t *g_font_buf = NULL;
static int g_font_select = 0;
static s8 g_fontpath[64] = {0};
static s8 g_fontinipath[64] = {0};
static s8 g_fontName[16] = {0};


static const char* disp_msg_def[] = 
{
	"ÁáÉéÍíÓóÚúÜü",
	"monto",
	""
};

static const char* disp_msg_def2[] = 
{
	"welcom",
	"amount",
	""
};

#define MSGMAXCOUNT (sizeof(disp_msg_def) / sizeof(disp_msg_def[0]))

char* GetDispMessage(int index)
{
	
	if(index < 0 || index >= MSGMAXCOUNT)
		return NULL;

	if(1 == g_font_select)
		return disp_msg_def[index];
	else
		return disp_msg_def2[index];
}


char* GetDispMessageFromCfg(int index)
{
	char key[16] = {0};
	static char text[128] = {0};
	
	if(index < 0 || index >= MSGMAXCOUNT)
		return NULL;

	memset(text, 0, sizeof(text));
	snprintf(key, sizeof(key), "text%d", index);
    MfSdkFsReadProfileString((const s8 *)"text", (const s8 *)key, (s8 *)text, sizeof(text), (const s8 *)"", (const s8 *)g_fontinipath);
	APP_TRACE("key = %s, text = %s\r\n", key, text);
	return text;
}

lv_font_t* GetMultiFont()
{
	LV_FONT_DECLARE(multifont);
	if(1 == g_font_select)
		return &multifont;
	else
		return lv_get_font("lv_font_montserrat_24");
}

static int FontReadBinFile(const s8* name ,char *buffer, s32 size)
{
	int length = MFSDK_RET_FAILED;
	int fd = MfSdkFsOpen(name, MFSDK_FS_FLAG_READ,MFSDK_FS_MODE_READ);
	
	APP_TRACE("File = %s, ret :%d\r\n",name, fd);
	if(fd >= 0)
	{
		MfSdkFsLseek(fd ,0, 0);
		length = MfSdkFsRead(fd, buffer, size);
		MfSdkFsClose(fd);
	}
	
	APP_TRACE("FontReadBinFile length :%d\r\n",length);
	return length;
}

uint8_t * AppFontReadBin(int offset)
{
	int length = -1;
	char *fontdata = NULL;
	const char *fontpath = g_fontpath;

	if(g_font_buf == NULL)
	{
		length = MfSdkFsGetFileLength(fontpath);
		//APP_TRACE("[%s] length :%d\r\n", fontpath, length);
		if(length > 0)
		{
			g_font_buf = MfSdkMemMalloc(length+1);
			//APP_TRACE("g_font_buf :%p\r\n",g_font_buf);
			if(g_font_buf != NULL)
			{
				memset(g_font_buf, 0 , length+1);				
				if(FontReadBinFile((const s8*)fontpath,(char*)g_font_buf ,length) != length)
				{
					MfSdkMemFree(g_font_buf);
					g_font_buf = NULL;
				}
			}
		}
	}
	
	if (g_font_buf != NULL)
		fontdata = g_font_buf+offset;

	APP_TRACE("fontdata :%p\r\n",fontdata);
	return fontdata;
}

int MultiFontInit()
{
	g_font_select = 1;
	memset(g_fontName, 0, sizeof(g_fontName));
	memset(g_fontpath, 0, sizeof(g_fontpath));
	memset(g_fontinipath, 0, sizeof(g_fontinipath));
    get_setting_str(FONT_NAME, g_fontName, sizeof(g_fontName));
	snprintf(g_fontpath, sizeof(g_fontpath), "exdata\\%s.bin", g_fontName);
	snprintf(g_fontinipath, sizeof(g_fontpath), "exdata\\%s.ini", g_fontName);
	return 0;
}

