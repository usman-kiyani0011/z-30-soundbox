#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "http_sync.h"
#include "libapi_xpos\inc\libapi_comm.h"
#include "libapi_xpos\inc\libapi_file.h"
#include "libapi_xpos\inc\libapi_system.h"
#include "libapi_xpos\inc\mfsdk_comm.h"
#include "libapi_xpos\inc\mfsdk_fs.h"
#include "libapi_xpos\inc\mfsdk_sys.h"
#include "libapi_xpos\inc\mfsdk_tms.h"
#include "libapi_xpos\inc\mfsdk_util.h"

static int getfullname( const char *name, char *out ){
	return sprintf( out,"exdata\\%s",name);
}
static int download( const char *name )
{
	const char *baseurl = "http://sync.ghttps.com:8434/data/";
	char url[128];
	char file[32];

	sprintf( url,"%s%s",baseurl,name);
	getfullname(name, file);

	return MfSdkCommHttpDownload((s8*)url,(s8*)file,1,2);
//	return comm_http_download_try(url,file,1,2);
}

static int checkfile( const char *name,const char *md5str )
{
	char fullname[16];
	char outmd5[33];

	getfullname(name,fullname);
	memset(outmd5,0x00,sizeof(outmd5));
//	Sys_md5_File( fullname, outmd5);
	MfSdkUtilMd5File((const s8*)fullname,(u8*)outmd5);

	if ( strcmp(outmd5, md5str) == 0)
	{
		return 1;
	}
	return 0;

}


int http_sync()
{
	int nRet;

	MfSdkTmsAppBusy(MFSDK_TMS_APP_STATE_BUSY);
//	osl_set_app_busy(1);
	nRet = download( "list.txt");
	//nRet = 0;
	if ( nRet == 0)
	{
		int nFileHandle;
//		int nRet = UFile_OpenCreate("list.txt",FILE_PUBLIC,FILE_RDONLY,&nFileHandle,0);
		int nRet = MfSdkFsOpenCreate((const s8*)"list.txt",MFSDK_FS_PUBLIC,MFSDK_FS_RDONLY,&nFileHandle,0);
		if (nRet == MFSDK_FS_RET_OK)
		{
			char line[128] = {0};
//			nRet = UFile_ReadLine(nFileHandle, line  ,sizeof( line));
			nRet = MfSdkFsReadLine(nFileHandle,(s8*)line,sizeof(line));			
			while ( nRet == MFSDK_FS_RET_OK)
			{
				char *name = line;
				char *p = (char *)strchr( line,' ');
				if( p != 0 )
				{
					int ntrycount = 1;
					char *md5str = p+1;
					*p = 0;

					while ( checkfile(name, md5str ) != 1 )
					{	
						int nDownloadret;
						if ( ntrycount == 0)
						{
							break;
						}
						nDownloadret = download( name);
						ntrycount--;
						MFSDK_UNUSED(nDownloadret);
					}
				}
				memset(line,0,sizeof(line));
				nRet = MfSdkFsReadLine(nFileHandle,(s8*)line,sizeof(line));
			}
//			UFile_Close( nFileHandle);
			MfSdkFsClose(nFileHandle);
		}		
	}
//	osl_set_app_busy(0);
	MfSdkTmsAppBusy(MFSDK_TMS_APP_NOT_BUSY);
	return nRet;
}
