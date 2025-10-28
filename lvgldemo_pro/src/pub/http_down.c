#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libapi_xpos/inc/mfsdk_sys.h"
#include "libapi_xpos/inc/mfsdk_aud.h"
#include "libapi_xpos/inc/mfsdk_comm.h"
#include "libapi_xpos/inc/mfsdk_mem.h"
#include "libapi_xpos/inc/mfsdk_fs.h"
#include "libapi_xpos/inc/mfsdk_log.h"
#include "libapi_xpos/inc/mfsdk_power.h"
#include "pub/common/misc/inc/mfmalloc.h"

typedef int (*http_download_progress)( int current, int total);

static http_download_progress s_progress = 0;

#define HTTPS_CA_CERT "" 
#define HTTPS_CLIENT_CERT ""
#define HTTPS_CLIENT_KEY ""

#define HTTP_SOCK_INDEX 	 (MFSDK_COMM_SOCKET_INDEX_3)
#define HTTP_SSL_SOCK_INDEX (MFSDK_COMM_SOCKET_INDEX_4)
#define HTTP_SSL_ENABLE()  (0)

#define HTTP_TRACE(format, ...) MfSdkLogLevel("app", MFSDK_LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)

#define HTTPSOCK (HTTP_SSL_ENABLE() ? HTTP_SSL_SOCK_INDEX : HTTP_SOCK_INDEX)

#ifndef NULL
#define NULL ((void *)0)
#endif
typedef struct http_uri_tag
{
    char             *full;                              /* full URL */
    char             *proto;                             /* protocol */
    char             *host;                              /* copy semantics */
    unsigned short port;
    char             *resource;                          /* copy semantics */
} http_uri;


typedef enum uri_parse_state_tag
{
    parse_state_read_host = 0,
    parse_state_read_port,
    parse_state_read_resource
} uri_parse_state;


static int http_uri_parse(const char *a_string, http_uri *a_uri)
{
    /* Everyone chant... "we love state machines..." */
    uri_parse_state l_state = parse_state_read_host;
    char *l_start_string = NULL;
    char *l_end_string = NULL;
    char l_temp_port[6];

    /* init the array */
    memset(l_temp_port, 0, 6);

    /* check the parameters */
    if (a_string == NULL)
    {
        goto ec;
    }

    if (a_uri)
    {
        a_uri->full = (char *)STRDUP(a_string);
    }
    l_start_string = (char *)strchr(a_string, ':');

    /* check to make sure that there was a : in the string */
    if (!l_start_string)
    {
        goto ec;
    }

    if (a_uri)
    {
        a_uri->proto = (char *)MfSdkMemMalloc(l_start_string - a_string + 1);
        memcpy(a_uri->proto, a_string, (l_start_string - a_string));
        a_uri->proto[l_start_string - a_string] = '\0';
    }

    /* check to make sure it starts with "http://" */
    if (strncmp(l_start_string, "://", 3) != 0)
    {
        goto ec;
    }
    /* start at the beginning of the string */
    l_start_string = l_end_string = &l_start_string[3];

    while(*l_end_string)
    {
        if (l_state == parse_state_read_host)
        {
            if (*l_end_string == ':')
            {
                l_state = parse_state_read_port;

                if ((l_end_string - l_start_string) == 0)
                {
                    goto ec;
                }

                /* allocate space */
                if ((l_end_string - l_start_string) == 0)
                {
                    goto ec;
                }

                /* only do this if a uri was passed in */
                if (a_uri)
                {
                    a_uri->host = (char *)MfSdkMemMalloc(l_end_string - l_start_string + 1);
                    /* copy the data */
                    memcpy(a_uri->host, l_start_string, (l_end_string - l_start_string));
                    /* terminate */
                    a_uri->host[l_end_string - l_start_string] = '\0';
                }
                /* reset the counters */
                l_end_string++;
                l_start_string = l_end_string;
                continue;
            }
            else if (*l_end_string == '/')
            {
                l_state = parse_state_read_resource;

                if ((l_end_string - l_start_string) == 0)
                {
                    goto ec;
                }

                if (a_uri)
                {
                    a_uri->host = (char *)MfSdkMemMalloc(l_end_string - l_start_string + 1);
                    memcpy(a_uri->host, l_start_string, (l_end_string - l_start_string));
                    a_uri->host[l_end_string - l_start_string] = '\0';
                }
                l_start_string = l_end_string;
                continue;
            }
        }
        else if (l_state == parse_state_read_port)
        {
            if (*l_end_string == '/')
            {
                l_state = parse_state_read_resource;

                /* check to make sure we're not going to overflow */
                if (l_end_string - l_start_string > 5)
                {
                    goto ec;
                }

                /* check to make sure there was a port */
                if ((l_end_string - l_start_string) == 0)
                {
                    goto ec;
                }
                /* copy the port into a temp buffer */
                memcpy(l_temp_port, l_start_string, l_end_string - l_start_string);

                /* convert it. */
                if (a_uri)
                {
                    a_uri->port = atoi(l_temp_port);
                }
                l_start_string = l_end_string;
                continue;
            }
            else if (isdigit(*l_end_string) == 0)
            {
                /* check to make sure they are just digits */
                goto ec;
            }
        }
        /* next.. */
        l_end_string++;
        continue;
    }

    if (l_state == parse_state_read_host)
    {
        if ((l_end_string - l_start_string) == 0)
        {
            goto ec;
        }

        if (a_uri)
        {
            a_uri->host = (char *)MfSdkMemMalloc(l_end_string - l_start_string + 1);
            memcpy(a_uri->host, l_start_string, (l_end_string - l_start_string));
            a_uri->host[l_end_string - l_start_string] = '\0';
            /* for a "/" */
            a_uri->resource = (char *)STRDUP("/");
        }
    }
    else if (l_state == parse_state_read_port)
    {
        if (strlen(l_start_string) == 0)
        {
            /* oops.  that's not a valid number */
            goto ec;
        }

        if (a_uri)
        {
            a_uri->port = atoi(l_start_string);
            a_uri->resource = STRDUP("/");
        }
    }
    else if (l_state == parse_state_read_resource)
    {
        if (strlen(l_start_string) == 0)
        {
            if (a_uri)
            {
                a_uri->resource = STRDUP("/");
            }
        }
        else
        {
            if (a_uri)
            {
                a_uri->resource = STRDUP(l_start_string);
            }
        }
    }
    else
    {
        /* uhh...how did we get here? */
        goto ec;
    }
    return 0;

ec:
    return -1;
}

static http_uri *http_uri_new(void)
{
    http_uri *l_return = NULL;

    l_return = (http_uri *)MfSdkMemMalloc(sizeof(http_uri));
    l_return->full = NULL;
    l_return->proto = NULL;
    l_return->host = NULL;
    l_return->port = 80;
    l_return->resource = NULL;
    return l_return;
}

static void http_uri_destroy(http_uri *a_uri)
{
    if (a_uri->full)
    {
        MfSdkMemFree(a_uri->full);
        a_uri->full = NULL;
    }

    if (a_uri->proto)
    {
        MfSdkMemFree(a_uri->proto);
        a_uri->proto = NULL;
    }

    if (a_uri->host)
    {
        MfSdkMemFree(a_uri->host);
        a_uri->host = NULL;
    }

    if (a_uri->resource)
    {
        MfSdkMemFree(a_uri->resource);
        a_uri->resource = NULL;
    }
    MfSdkMemFree(a_uri);
}

static char *http_headvalue(char *heads, const char *head)
{
    char *pret = strstr(heads, head);

    if ( pret != 0 )
    {
        pret += strlen(head);
        pret += strlen(": ");
    }
    return pret;
}

static int http_ContentLength(char *heads)
{
    char *pContentLength = http_headvalue(heads, "Content-Length");

    if ( pContentLength != 0)
    {
        return atoi(pContentLength);
    }
    return -1;
}

static int http_ContentRange(char *heads)
{
    char *val = http_headvalue(heads, "Content-Range");

    if ( val != 0 )
    {
        val = strchr(val, '/');

        if ( val != 0 )
        {
            val++;
            return atoi(val);
        }
    }
    return -1;
}

static int http_StatusCode(char *heads)
{
    char *rescode = strstr(heads, " ");

    if ( rescode != 0 )
    {
        rescode += 1;
        return atoi(rescode);
    }
    else
    {
        HTTP_TRACE("http_StatusCode error %s\r\n", heads);
    }
    return -1;
}

typedef struct _tag_httpdownload
{
    http_uri *urlbase;      //请求链接
    http_uri *urllocation;     //重定向地址
    const char *fullpathfilename;      //下载地址
    int iscontinue;
    int fdsize;    //文件已下载长度
    int ContentRange;     //请求文件长度
    int downloadsize;    //本次已下载大小
}httpdownload;

#define RECVTIMEOUT 30000
#define RECVBUFFSIZE 4096


//////////////////////////////////////////////////////////////////////////
//下载临时flash区
#ifdef WIN32
#define EXT_FLASH_TMS_AREA_ADDR                 (0x0)
#else

#endif

static void inittotemp()
{
#ifdef USTEMP
    mf_flash_erase(EXT_FLASH_TMS_AREA_ADDR, 512 * 1024);
#endif
}

static int writetotemp(int index, unsigned char * buff, int datasize)
{
#ifdef USTEMP
    unsigned int write_addr =  EXT_FLASH_TMS_AREA_ADDR + index;
    mf_flash_write((unsigned char *)buff, write_addr, datasize);
    return datasize;

#else
    return 0;

#endif
}

static int readtemp(int index, void * buff, int datasize)
{
#ifdef USTEMP
    unsigned int read_addr =  EXT_FLASH_TMS_AREA_ADDR + index;
    mf_flash_read((unsigned char *)buff, read_addr, datasize);
    return datasize;

#else
    return 0;

#endif
}

static int flushtofile(httpdownload *ret)
{
#ifdef USTEMP
    int count = 0;
    unsigned int tick1;

    if ( ret->downloadsize > 0 )
    {    //已下载大小
        int downloadfd = FILE_OPEN(ret->fullpathfilename, FILE_WRITE_FLAG, FILE_WRITE_MODE);

        if (  downloadfd == FILE_OPEN_FAIL  )
        {
            downloadfd = FILE_OPEN(ret->fullpathfilename, FILE_CREAT_FLAG, FILE_WRITE_MODE);
        }

        if ( downloadfd != FILE_OPEN_FAIL )
        {
            void *buff = MfSdkMemMalloc(4096);
            int readindex =  0;

            FILE_LSEEK(downloadfd, 0, SEEK_END);

            HTTP_TRACE("flushtofile1:%d,%d\r\n", ret->downloadsize, MfSdkSysGetTick());
            tick1 = MfSdkSysGetTick();//osl_GetTick(); 

            while ( readindex < ret->downloadsize )
            {
                int readlen = (ret->downloadsize - readindex) > 4096 ? 4096 : ret->downloadsize - readindex;
                readtemp(readindex, buff, readlen);
                mf_file_write2(downloadfd, buff, readlen, 0);

                readindex += readlen;
                count++;

                if(count % 25 == 0) { osl_Sleep(100); }
            }

            HTTP_TRACE("flushtofile2:%d\r\n", MfSdkSysGetTick() - tick1);

            MfSdkMemFree(buff);

            FILE_CLOSE(downloadfd);
        }
        ret->fdsize += ret->downloadsize;
        ret->downloadsize = 0;
    }
#endif
    return 0;
}

static int closedownfile(httpdownload *ret)
{
    flushtofile(ret);
    return 0;
}

static int craetenewdownfile(httpdownload *ret)
{
    closedownfile(ret);
//    FILE_DELETE(ret->fullpathfilename);
	MfSdkFsUnlink((const s8 *)ret->fullpathfilename);
    ret->fdsize = 0;
    return 0;
}

static httpdownload * http_download_create(const char *url, const char *filename, int iscontinue)
{
    httpdownload *ret = (httpdownload *)MfSdkMemMalloc(sizeof(httpdownload) );

    ret->urlbase =  http_uri_new();
    http_uri_parse(url, ret->urlbase);
    ret->urllocation = NULL;
    ret->fullpathfilename = STRDUP(filename);
    ret->iscontinue = iscontinue;
    ret->downloadsize = 0;

    inittotemp();

    if ( iscontinue )
    {
        ret->fdsize = MfSdkFsGetFileLength(filename); //File_GetFileLength(filename);

        if ( ret->fdsize < 0)
        {
            craetenewdownfile(ret);
        }
    }
    else
    {
        craetenewdownfile(ret);
    }
    return ret;
}

static httpdownload * http_download_destroy(httpdownload *d)
{
    closedownfile(d);

    if ( d->urlbase != NULL )
    {
        http_uri_destroy(d->urlbase);
        d->urlbase = NULL;
    }

    if ( d->urllocation != NULL )
    {
        http_uri_destroy(d->urllocation);
        d->urllocation = NULL;
    }

    if ( d->fullpathfilename != NULL )
    {
        MfSdkMemFree((void *)d->fullpathfilename);
        d->fullpathfilename = NULL;
    }
    MfSdkMemFree(d);
    return 0;
}

static http_uri *http_download_getconnecturi(httpdownload *d)
{
    if ( d->urllocation != NULL )
    {
        return d->urllocation;
    }
    else
    {
        return d->urlbase;
    }
}

static int http_download_getreqbuff(httpdownload *d, char *reqbuff)
{
    return sprintf(reqbuff, "GET %s HTTP/1.1\r\nHost: %s:%d\r\nRange: bytes=%d- \r\nConnection: keep-alive\r\n\r\n", http_download_getconnecturi(d)->resource, d->urlbase->host, d->urlbase->port, d->fdsize);
}

static int writetofile(httpdownload *d, const char *data, int datasize, int sumlen)
{
    int progressret = 0;
    int ret = -1;


#ifdef USTEMP
    d->downloadsize += writetotemp(d->downloadsize, (unsigned char * )data, datasize);
#ifdef WIN32

    if ( d->downloadsize > 10240 )
    {
        flushtofile(d);
    }
#endif

#else

    if ( datasize > 0 )
    {    //已下载大小
        int write_bytes = 0;
		int downloadfd = MfSdkFsOpen((const s8*)d->fullpathfilename,MFSDK_FS_FLAG_WRITE,MFSDK_FS_MODE_WRITE);

        if ( downloadfd == MFSDK_FS_RET_OPEN_FAIL )
        {
			downloadfd = MfSdkFsOpen((const s8*)d->fullpathfilename,MFSDK_FS_FLAG_CREAT,MFSDK_FS_MODE_WRITE);
        }

        if (MFSDK_FS_RET_OPEN_FAIL != downloadfd)
        {
			int len = MfSdkFsLseek(downloadfd, 0, SEEK_END);
            HTTP_TRACE("writetofile lseek=%d %d %d\r\n", len, sumlen, sumlen - len);
			write_bytes = MfSdkFsWrite(downloadfd, (char *) data,datasize);
			
            MfSdkFsClose(downloadfd);//FILE_CLOSE(downloadfd);

            if(write_bytes != datasize)
            {
                HTTP_TRACE("writetofile error %s", d->fullpathfilename);
            }
        }
        d->fdsize += datasize;
        d->downloadsize = 0;
    }
#endif

    if ( s_progress != 0)
    {
        progressret = s_progress(d->fdsize + d->downloadsize, d->ContentRange);
    }
	MFSDK_UNUSED(progressret);
    return ret;
}

static int http_download_recvtofile(int s, int len, httpdownload * info)
{
    unsigned char *szTemp = (unsigned char *)MfSdkMemMalloc(RECVBUFFSIZE);
    int nTry = 0;
    int sumlen = 0;
    int nTryCount = HTTP_SSL_ENABLE() ? 300 : 5;

    while(sumlen < len && nTry < nTryCount)
    {
        int nret = -1;

        if(HTTP_SSL_ENABLE())
        {
			nret = MfSdkCommSslRecv(s, (char*)szTemp, RECVBUFFSIZE);
			//nret = MfSdkCommSocketRecv(s, (char*)szTemp, RECVBUFFSIZE, 15000);
        }
        else
        {
			nret = MfSdkCommSocketRecv(s, szTemp, RECVBUFFSIZE, 3000);
        }

        if ( nret > 0 )
        {
            unsigned int tick1 = MfSdkSysGetTick();
            writetofile(info, (const char *)szTemp, nret, sumlen);
            HTTP_TRACE("write(%d):%d\r\n", nret, MfSdkSysGetTick() - tick1);
            sumlen += nret;
            nTry = 0;
        }
        else
        {
            nTry++;
            HTTP_TRACE("http_download_recvtofile: %d/%d nTry = %d\r\n", sumlen, len, nTry);
        }
    }

    HTTP_TRACE("http_download_recvtofile---: %d/%d nTry = %d\r\n", sumlen, len, nTry);
    MfSdkMemFree(szTemp);
    return sumlen;
}

static int http_download_recvbody(int sock, char *szTemp, int nRecvLen, httpdownload *info)
{
    int hret = -1;
    char *bodystart = strstr(szTemp, "\r\n\r\n") + 4;

    if ( bodystart != (char *)4 )
    {
        int nrescode = http_StatusCode(szTemp);
        HTTP_TRACE("http rescode: %d\r\n", nrescode);

        if ( nrescode != -1 )
        {
            info->ContentRange = http_ContentRange(szTemp);

            if ( nrescode == 200 || nrescode == 206 )
            {
                if ( nrescode == 200 )
                {                //服务器不支持断点续传
                    craetenewdownfile(info);
                }
                {
                    //获取的部分文件内容
                    int firstlen = nRecvLen - (bodystart - szTemp);
                    int ContentLength = http_ContentLength(szTemp);

                    HTTP_TRACE("Content-Length: %d\r\n", ContentLength);

                    if ( ContentLength >  0)
                    {
                        if ( firstlen > 0)
                        {
                            writetofile(info, bodystart, firstlen, firstlen);
                        }
                        //接收剩下的内容
                        ContentLength -= firstlen;

                        if ( ContentLength > 0 )
                        {
                            hret = http_download_recvtofile(sock, ContentLength, info);

                            if (hret == ContentLength )
                            {
                                hret = 0;
                            }
                            else
                            {
                                HTTP_TRACE("recvtofileth\r\n: error");
                                hret = -2;
                            }
                        }
                        else
                        {
                            hret = 0;
                        }
                    }
                }
            }
            else if (  nrescode == 416 )
            {            //超出请求范围
                if ( info->fdsize + info->downloadsize >= info->ContentRange )
                {                //下载完成
                    hret = 0;
                }
            }
            else if ( nrescode == 302 )
            {            //重定向
                const char *newurlstr = http_headvalue(szTemp, "Location");

                if ( newurlstr != NULL )
                {
                    char *d = strstr(newurlstr, "\r\n");

                    if ( d != NULL )
                    {
                        *d = '\0';
                        HTTP_TRACE("Location %s\r\n", newurlstr);

                        if ( info->urllocation != NULL )
                        {
                            http_uri_destroy(info->urllocation);
                            info->urllocation = NULL;
                        }
                        info->urllocation = http_uri_new();
                        http_uri_parse(newurlstr, info->urllocation);

                        hret = 302;
                    }
                }
            }
            else
            {
                HTTP_TRACE("http rescode error : %d\r\n", nrescode);
            }
        }
    }
    return hret;
}

static int http_download_procfile(httpdownload *info)
{
    int hret = -1;
    int sock = HTTPSOCK;

#ifndef WIN32
    sock = HTTPSOCK;
#endif

    sock = HTTP_SSL_ENABLE() ? HTTP_SSL_SOCK_INDEX : HTTP_SOCK_INDEX;

	sock = MfSdkCommSocketCreate(sock);
	
    if ( sock >= 0 )
    {
        http_uri *uritem =  http_download_getconnecturi(info);
        int nconnect = -1;

        if(HTTP_SSL_ENABLE())
        {
			MfSdkCommSslInit(sock, HTTPS_CA_CERT, HTTPS_CLIENT_CERT,HTTPS_CLIENT_KEY,0);
			nconnect  = MfSdkCommSslConnect(sock, uritem->host, uritem->port, NULL);
        }
        else
        {
			nconnect = MfSdkCommSocketConnect(sock, (s8*)uritem->host, uritem->port, 20 * 1000, NULL);
        }

        if ( nconnect >= 0 )
        {
            char *szTemp = (char *)MfSdkMemMalloc(RECVBUFFSIZE + 1);
            int ret = http_download_getreqbuff(info, szTemp);             //,"GET %s HTTP/1.1\r\nHost: %s:%d\r\nRange: bytes=%d- \r\nConnection: keep-alive\r\n\r\n" , huri->resource,huri->host,huri->port,fdsize);
            int sendret = -1;

            if(HTTP_SSL_ENABLE())
            {
				sendret = MfSdkCommSslSend(sock, szTemp, ret);
                sendret = ret;
            }
            else
            {
				sendret = MfSdkCommSocketSend(sock, (unsigned char *)szTemp, ret);
            }
            HTTP_TRACE("mf_sock_send =%d,reqbuff=%s", sendret, szTemp);

            if ( sendret == ret )
            {
                char *bodystart = 0;
                int nRecvLen = 0;

                unsigned int nstarttick = MfSdkSysGetTick();

                while (bodystart == 0 && MfSdkSysCheckTick(nstarttick, RECVTIMEOUT) == 0)
                {
                    int nret = -1;

                    if(HTTP_SSL_ENABLE())
                    {
						nret = MfSdkCommSslRecv(sock, (char *)szTemp + nRecvLen, RECVBUFFSIZE - nRecvLen);
                    }
                    else
                    {
						nret = MfSdkCommSocketRecv(sock, (u8 *)szTemp + nRecvLen, RECVBUFFSIZE - nRecvLen, 1000);
                    }
                    HTTP_TRACE("http_download_procfile=%d\r\n", nret);

                    if ( nret > 0 )
                    {
                        nRecvLen += nret;
                        szTemp[nRecvLen] = '\0';
                        bodystart = strstr(szTemp, "\r\n\r\n");
                        nstarttick = MfSdkSysGetTick();
                    }
                }

                if(nRecvLen > 0 && bodystart != 0)
                {
                    hret = http_download_recvbody(sock, szTemp, nRecvLen, info);
                }
            }
            MfSdkMemFree(szTemp);
        }
        MfSdkCommSocketClose(sock);
    }
    return hret;
}

static int _http_download(const char *url, const char *fullpathfilename, int iscontinue)
{
    int ret = 0;
    httpdownload *info = http_download_create(url, fullpathfilename, iscontinue);

    MfSdkPowerLockApp("_http_download");
    ret = http_download_procfile(info);

    if ( ret == 302 )
    {
        ret = http_download_procfile(info);
    }
    MfSdkPowerUnlockApp();

    http_download_destroy(info);

    return ret;
}

static int _http_download_(const char *url, const char *fullpathfilename, int iscontinue)
{
    int ret = -1;
    int nTry = 0;

    MfSdkCommSocketFifoResize(HTTPSOCK, 8192 * 2);

    while ( nTry < 5 && ret != 0 )
    {
        ret = _http_download(url, fullpathfilename, nTry == 0 ? iscontinue : 1);
        nTry++;
    }

    MfSdkCommSocketFifoResize(HTTPSOCK, 2048);
    return ret;
}

/*http下载，增加重试参数*/

int app_http_download_retry(const char *url, const char *fullpathfilename, int iscontinue, int nRetry)
{
    int ret = -1;
    int i;

    MfSdkCommSocketFifoResize(HTTPSOCK, 8192 * 2);

    if(nRetry < 1) { nRetry = 1; }

    for(i = 0; i < nRetry && ret != 0; i++)
    {
        ret = _http_download_(url, fullpathfilename, i == 0 ? iscontinue : 1);
    }

    MfSdkCommSocketFifoResize(HTTPSOCK, 2048);
    return ret;
}

void AppHttpDownloadTest(){

	int x = app_http_download_retry("https://sgb.oss.morefun-et.com/tms/2024-05-29/17169980156401855638.bin","exdata\\tempota.bin", 0, 5);
	HTTP_TRACE("app_http_download_retry:%d\r\n",x );
}



int http_download_progress_set( http_download_progress progressfun )
{
	s_progress = progressfun;

	return 0;
}

