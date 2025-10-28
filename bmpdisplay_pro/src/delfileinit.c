#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "xdk_et.h"
#include "tracedef.h"
//#include "libapi_xpos/inc/libapi_file.h"
#include "libapi_xpos/inc/mfsdk_fs.h"

/****************************************************************************************
exdata\\delfile.ini:
[del]
num=4
file_1=data//111.mp3
file_2=data//222.mp3
file_3=exdata//333.mp3
file_4=lang1//444.mp3
****************************************************************************************/

#define DELMP3_INI_FILENAME 	(const s8*)"exdata\\delfile.ini"
#define DELMP3_INI_SECTION		(const s8*)"del"

#ifdef WIN32
int UFile_Unlink(char* filename)
{
	int r = mf_file_unlink(filename);
	return UFILE_SUCCESS;
}
#endif
void delfile_init()
{
	int num = 0;
	int i = 0;
	char temp[20] = { 0 };
	char key[20] = { 0 };
	char val[20] = { 0 };

	MfSdkFsReadProfileString(DELMP3_INI_SECTION,(const s8*)"num", (s8*)temp, sizeof(temp),(const s8*)"",DELMP3_INI_FILENAME);

//	UFile_read_profile_string(DELMP3_INI_SECTION, "num", temp, sizeof(temp), "", DELMP3_INI_FILENAME);
	APP_TRACE("[%s][%d][strlen=%d]", __FUNCTION__, __LINE__, strlen(temp));
	if (strlen(temp) == 0)
	{
		return;
	}
	num = atoi(temp);
	APP_TRACE("[%s][%d][num=%d]",__FUNCTION__,__LINE__, num);

	for (i = 0; i < num; i++)
	{
		memset(key, 0, sizeof(key));
		memset(val, 0, sizeof(val));
		sprintf(key, "file_%d", i + 1);
		MfSdkFsReadProfileString(DELMP3_INI_SECTION,(const s8*)key, (s8*)val, sizeof(val),(s8*)"", DELMP3_INI_FILENAME);
		APP_TRACE("[%s][%d][%s = %s]", __FUNCTION__, __LINE__, key, val);
//		UFile_Unlink(val);
		MfSdkFsUnlink((const s8 *)val);
	}
//	UFile_Unlink(DELMP3_INI_FILENAME);
	MfSdkFsUnlink((const s8 *)DELMP3_INI_FILENAME);

	APP_TRACE("[%s][%d][success!!!]", __FUNCTION__, __LINE__);
}