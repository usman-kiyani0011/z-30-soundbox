
#include "AppHttpDemo.h"
#include "pub/common/misc/inc/mfmalloc.h"

#include "libapi_xpos/inc/mfsdk_mem.h"
#include "libapi_xpos/inc/mfsdk_fs.h"
#include "libapi_xpos/inc/mfsdk_util.h"
#include "libapi_xpos/inc/mfsdk_log.h"
#include "libapi_xpos/inc/mfsdk_ped.h"
//#include "libapi_xpos/inc/mfsdk_rki.h"
#include "libapi_xpos/inc/mfsdk_sys.h"
#include "libapi_xpos/inc/mfsdk_emv.h"
#include "libapi_xpos/inc/mfsdk_util.h"
#include "libapi_xpos/inc/mfsdk_comm.h"

#define APP_TRACE(...) MfSdkLogLevel("app", MFSDK_LOG_LEVEL_TRACE, __VA_ARGS__)
#define APP_TRACE_BUFF_TIP(a, b, c) MfSdkLogTip("app", MFSDK_LOG_LEVEL_TRACE, a, b, c, 1);
enum WEBCLIENT_STATUS
{
    WEBCLIENT_OK,
    WEBCLIENT_ERROR,
    WEBCLIENT_TIMEOUT,
    WEBCLIENT_NOMEM,
    WEBCLIENT_NOSOCKET,
    WEBCLIENT_NOBUFFER,
    WEBCLIENT_CONNECT_FAILED,
    WEBCLIENT_DISCONNECT,
    WEBCLIENT_FILE_ERROR,
    WEBCLIENT_SOCKET_ERROR,
    WEBCLIENT_SOCKET_SIZE_ERROR,    //add by ygf
};


const char* AppHttpHeaderFieldsGet(char* buffer, s32 length, const char* fields)
{
    char* resp_buf = NULL;
    size_t resp_buf_len = 0;

    //    RT_ASSERT(session);
    //    RT_ASSERT(session->header->buffer);

    resp_buf = buffer;

    while (resp_buf_len < length)
    {
        if (strstr(resp_buf, fields))
        {
            char* mime_ptr = NULL;

            /* jump space */
            mime_ptr = strstr(resp_buf, ":");

            if (mime_ptr != NULL)
            {
                mime_ptr += 1;

                while (*mime_ptr && (*mime_ptr == ' ' || *mime_ptr == '\t'))
                {
                    mime_ptr++;
                }

                return mime_ptr;
            }
        }

        if (*resp_buf == '\0')
        {
            break;
        }
        resp_buf += strlen(resp_buf) + 1;
        resp_buf_len += strlen(resp_buf) + 1;
    }

    return NULL;
}

static int AppHttpRecv(AppHttpSession_T* session, char* buffer, size_t len, int flag)
{
    return MfSdkCommSocketRecv(session->socket, (u8*)buffer, len, session->recv_timeout_ms);
}

static int AppHttpReadLine(AppHttpSession_T* session, char* buffer, s32 size)
{
    int rc, count = 0;
    char ch = 0, last_ch = 0;

    //    RT_ASSERT(session);
    //    RT_ASSERT(buffer);
    /* Keep reading until we fill the buffer. */
    while (count < size)
    {
        rc = AppHttpRecv(session, &ch, 1, 0);

        if (rc <= 0) { return rc; }

        if (ch == '\n' && last_ch == '\r') { break; }
        buffer[count++] = ch;
        last_ch = ch;
    }

    if (count > size)
    {
        APP_TRACE("read line failed. The line data length is out of buffer size(%d)!", count);
        return -WEBCLIENT_ERROR;
    }
    return count;
}

int AppHttpHandleResponse(AppHttpSession_T* session)
{
    s32 rc = WEBCLIENT_OK;
    char* mime_buffer = NULL;
    char* mime_ptr = NULL;
    const char* transfer_encoding;
    s32 i = 0;
    s32 begin_ticket = MfSdkSysGetTick();

    //    RT_ASSERT(session);

    /* clean header buffer and size */
    memset(session->buffer, 0x00, session->size);

    /* We now need to read the header information */
    while (1)
    {
        if (MfSdkSysCheckTick(begin_ticket, session->recv_resp_timeout_ms))
        {
            return -WEBCLIENT_TIMEOUT;
        }
        mime_buffer = session->buffer + session->length;

        /* read a line from the header information. */
        rc = AppHttpReadLine(session, mime_buffer, session->size - session->length);

//        APP_TRACE("webclient_read_line:%d \r\n", rc);

        if (rc < 0) { break; }
        //        if (rc > 0) { begin_ticket = MfSdkSysGetTick(); }

//        APP_TRACE_BUFF_TIP(session->buffer, session->length, "session->buffer");

        /* End of headers is a blank line.  exit. */
        if (rc == 0)
        {
            break;
        }

        if ((rc == 1) && (mime_buffer[0] == '\r'))
        {
            mime_buffer[0] = '\0';
            break;
        }
        /* set terminal charater */
        mime_buffer[rc - 1] = '\0';

        session->length += rc;

        if (session->length >= session->size)
        {
            APP_TRACE("not enough header buffer size(%d)!", session->size);
            return -WEBCLIENT_NOMEM;
        }
    }

    /* get HTTP status code */
    mime_ptr = STRDUP(session->buffer);

    if (mime_ptr == NULL)
    {
        APP_TRACE("no memory for get http status code buffer!");
        return -WEBCLIENT_NOMEM;
    }

    if (strstr(mime_ptr, "HTTP/1."))
    {
        char* ptr = mime_ptr;

        ptr += strlen("HTTP/1.x");

        while (*ptr && (*ptr == ' ' || *ptr == '\t'))
        {
            ptr++;
        }

        /* Terminate string after status code */
        for (i = 0; ((ptr[i] != ' ') && (ptr[i] != '\t')); i++)
        {
            ;
        }

        ptr[i] = '\0';

        session->resp_status = (int)strtol(ptr, NULL, 10);
    }

    /* get content length */
    if (AppHttpHeaderFieldsGet(session->buffer, session->length, "Content-Length") != NULL)
    {
        session->content_length = atoi(AppHttpHeaderFieldsGet(session->buffer, session->length, "Content-Length"));
    }

    if (AppHttpHeaderFieldsGet(session->buffer, session->length, "content-length") != NULL)
    {
        session->content_length = atoi(AppHttpHeaderFieldsGet(session->buffer, session->length, "content-length"));
    }
    session->content_remainder = session->content_length ? (size_t)session->content_length : 0xFFFFFFFF;

    transfer_encoding = AppHttpHeaderFieldsGet(session->buffer, session->length, "Transfer-Encoding");

    if (transfer_encoding == 0)
    {
        transfer_encoding = AppHttpHeaderFieldsGet(session->buffer, session->length, "transfer-encoding");
    }

    if (transfer_encoding && strcmp(transfer_encoding, "chunked") == 0)
    {
        char line[32] = { 0 };
        /* chunk mode, we should get the first chunk size */
        AppHttpReadLine(session, line, sizeof(line));
        session->chunk_sz = strtol(line, NULL, 16);
//        APP_TRACE("session->chunk_sz:%d \r\n", session->chunk_sz);
        session->chunk_offset = 0;
    }

    if (mime_ptr)
    {
        FREE(mime_ptr);
    }

    if (rc < 0)
    {
        return rc;
    }
    return session->resp_status;
}

static int AppHttpNextChunk(AppHttpSession_T* session)
{
    char line[64];
    int length;

    //RT_ASSERT(session);
    memset(line, 0x00, sizeof(line));
    length = AppHttpReadLine(session, line, sizeof(line));
//    APP_TRACE_BUFF_TIP(line, length, "AppHttpNextChunk");

    if (length > 0)
    {
        //if (strcmp(line, "\r") == 0)
        if(line[0] == '\r')
        {
            length = AppHttpReadLine(session, line, sizeof(line));

            if (length <= 0)
            {
                if (session->socket >= 0)
                {
                    MfSdkCommSocketClose(session->socket);
                }
                session->socket = -1;
                return length;
            }
        }
    }
    else
    {
        if (session->socket >= 0)
        {
            MfSdkCommSocketClose(session->socket);
        }
        session->socket = -1;

        return length;
    }
    session->chunk_sz = strtol(line, NULL, 16);
    session->chunk_offset = 0;

    if (session->chunk_sz == 0)
    {
        /* end of chunks */

        if (session->socket >= 0)
        {
            MfSdkCommSocketClose(session->socket);
        }
        session->socket = -1;
    }
    return session->chunk_sz;
}

int AppHttpRead(AppHttpSession_T* session, unsigned char* buffer, size_t length)
{
    int bytes_read = 0;
    int total_read = 0;
    int left;

    //    RT_ASSERT(session);

    if (session->socket < 0)
    {
        return -WEBCLIENT_DISCONNECT;
    }

    if (length == 0)
    {
        return 0;
    }
//    APP_TRACE("session->chunk_sz:%d\r\n", session->chunk_sz);

    /* which is transfered as chunk mode */
    if (session->chunk_sz)
    {
        if (MfSdkSysCheckTick(session->begin_ticket, session->recv_resp_timeout_ms))
        {
            return -WEBCLIENT_TIMEOUT;
        }
//        APP_TRACE("xx length:%d\r\n", length);

        if ((int)length > (session->chunk_sz - session->chunk_offset))
        {
            length = session->chunk_sz - session->chunk_offset;
        }
//        APP_TRACE("length:%d\r\n", length);
        bytes_read = AppHttpRecv(session, (char*)buffer, length, 0);
//        APP_TRACE("AppHttpRecv bytes_read:%d\r\n", bytes_read);
        //APP_TRACE_BUFF_TIP(buffer, bytes_read,"chunk buffer");

        if (bytes_read <= 0)
        {
            if (session->socket >= 0)
            {
                MfSdkCommSocketClose(session->socket);
            }
            session->socket = -1;
            return 0;
        }
        session->chunk_offset += bytes_read;

        APP_TRACE("session->chunk_offset:%d,session->chunk_sz:%d\r\n", session->chunk_offset, session->chunk_sz);

        if (session->chunk_offset >= session->chunk_sz)
        {
            AppHttpNextChunk(session);
        }
        return bytes_read;
    }

    if (session->content_length > 0)
    {
        if (length > session->content_remainder)
        {
            length = session->content_remainder;
        }

        if (length == 0)
        {
            return 0;
        }
    }
    /*
     * Read until: there is an error, we've read "size" bytes or the remote
     * side has closed the connection.
     */
    left = length;

    do
    {
        bytes_read = AppHttpRecv(session, (char*)(buffer + total_read), left, 0);

        if (bytes_read <= 0)
        {
            APP_TRACE("receive data error(%d).", bytes_read);

            if (total_read)
            {
                break;
            }
            else
            {
                if (session->socket >= 0)
                {
                    MfSdkCommSocketClose(session->socket);
                }
                session->socket = -1;
                return 0;
            }
        }
        left -= bytes_read;
        total_read += bytes_read;
    }
    while (left);

    if (session->content_length > 0)
    {
        session->content_remainder -= total_read;
    }
    return total_read;
}

s32 AppHttpRecvResponseContent(AppHttpSession_T* session, u8* recv_buf, s32 recv_buf_len)
{
    s32 ret = 0;
    s32 read_len = 0;
    s32 bytes_read = 0;
    s32 content_pos = 0;
    s32 content_length = session->content_length;

    if (content_length < 0)
    {
        APP_TRACE("webclient GET request type is chunked.\n");

        do
        {
            if (MfSdkSysCheckTick(session->begin_ticket, session->recv_resp_timeout_ms))
            {
                return -WEBCLIENT_TIMEOUT;
            }

            if (session->chunk_sz >= 0 && session->chunk_offset >= 0)
            {
                if (content_pos + session->chunk_sz - session->chunk_offset >= recv_buf_len - 1)
                {
                    ret = -WEBCLIENT_NOMEM; break;
                }
            }
            bytes_read = AppHttpRead(session, recv_buf + content_pos, recv_buf_len - content_pos);
            APP_TRACE("AppHttpRecvResponseContent bytes_read:%d \r\n", bytes_read);

            if (bytes_read > 0) { content_pos += bytes_read;  }

            if (bytes_read <= 0 || session->chunk_sz <= 0) 
			{ 
				if(session->chunk_sz > 0) { ret = -WEBCLIENT_ERROR; }
				break;  
			}			
        }while (1);
    }
    else
    {
        do
        {
            if (MfSdkSysCheckTick(session->begin_ticket, session->recv_resp_timeout_ms))
            {
                return -WEBCLIENT_TIMEOUT;
            }
            read_len = content_length - content_pos > recv_buf_len ? recv_buf_len : content_length - content_pos;

            if ((content_pos + read_len) >= recv_buf_len - 1) { ret = -WEBCLIENT_NOMEM; break;  }
            bytes_read = AppHttpRead(session, recv_buf + content_pos, read_len);
			APP_TRACE("AppHttpRecvResponseContent bytes_read:%d \r\n", bytes_read);
            if (bytes_read <= 0) 
			{ 
				if(content_pos < content_length) 
				{ 
					ret = -WEBCLIENT_ERROR;
					APP_TRACE("AppHttpRecvResponseContent content_pos:%d < content_length : %d \r\n", content_pos,content_length);
				}			
				break;  
			}
            content_pos += bytes_read;
        }
        while (content_pos < content_length);
    }
    return ret == 0 ? content_pos : ret;
}

#define BAIDU_HOST  "www.baidu.com"
#define GOOGLE_HOST "www.google.com"

#define BAIDU_URL  "www.baidu.com"
#define GOOGLE_URL "www.google.com"

#define CONTENT_MAX_LENGTH 400 * 1024

#include "http_parser.h"

void AppHttpTest(char* url)
{
    s32 index = 0;
	s32 port = 80;
	MFSDKBOOL tls = MFSDK_FALSE;
	u8* recv_buf = NULL;
    char SendBuf[1024] = { 0 };
    char RecBuf[1024 * 4] = { 0 };
	char host[256] = {0};
    AppHttpSession_T session;

	struct http_parser_url u;
    memset(&u, 0, sizeof(u));

    if(http_parser_parse_url(url, strlen(url), 0, &u))
	{
		APP_TRACE("http_parser_parse_url failed\r\n");
		return ;
	}

	if (u.field_set & (1 << UF_SCHEMA)) 
	{
        APP_TRACE("Scheme: %.*s\n", u.field_data[UF_SCHEMA].len, url + u.field_data[UF_SCHEMA].off);
		if(u.field_data[UF_SCHEMA].len == 5 && !memcmp(url + u.field_data[UF_SCHEMA].off , "https" , 5))
		{
			tls = MFSDK_TRUE;
		}		
    }
	if(tls){ port = (u.port == 0) ? 443 : u.port; }
	else{ port = (u.port == 0) ? 80 : u.port; }

	if (u.field_set & (1 << UF_HOST)) 
	{
        APP_TRACE("Host: %.*s\n", u.field_data[UF_HOST].len, url + u.field_data[UF_HOST].off);
		snprintf(host ,sizeof(host) , "%.*s" , u.field_data[UF_HOST].len, url + u.field_data[UF_HOST].off);
    }
	else
	{
		APP_TRACE("url get host failed\r\n");
		return ;
	}

	if (u.field_set & (1 << UF_PATH)) {
        APP_TRACE("UF_PATH: %.*s\n", u.field_data[UF_PATH].len, url + u.field_data[UF_PATH].off);
    }

	APP_TRACE("host:%s\r\n",host);
	
    memset(&session, 0, sizeof(AppHttpSession_T));

    session.buffer = RecBuf;
    session.size = sizeof(RecBuf);
    session.recv_timeout_ms = 10 * 1000;
    session.recv_resp_timeout_ms = 30 * 1000;
    session.content_length = -1;

    session.socket = MfSdkCommSocketCreate(MFSDK_COMM_SOCKET_INDEX_3);

    index += sprintf(SendBuf + index, "GET %.*s HTTP/1.1\r\n",u.field_data[UF_PATH].len, url + u.field_data[UF_PATH].off);
    index += sprintf(SendBuf + index, "Host: %s\r\n",host);
    // index += sprintf(SendBuf + index, "%s\r\n", "Content-Type: text/plain");
    index += sprintf(SendBuf + index, "%s\r\n", "Accept: */*");
    index += sprintf(SendBuf + index, "%s\r\n", "Connection: keep-alive");
    index += sprintf(SendBuf + index, "\r\n");

	if(tls)
	{
		MfSdkCommSslAuthMode(session.socket, 1);
		MfSdkCommSslInit(session.socket, "", "", "", 1);
	}

    do{
	    s32 ret = MfSdkCommSocketConnect(session.socket, (s8*)host, port, 10000, (void*)0);
		if(ret != 0)
		{ 
			APP_TRACE("MfSdkCommSocketConnect failed:%d\r\n", ret);
			break; 
		}

	    ret = MfSdkCommSocketSend(session.socket, (u8*)SendBuf, strlen(SendBuf));

		if(ret != strlen(SendBuf)) 
		{ 
			APP_TRACE("MfSdkCommSocketSend failed:%d\r\n", ret);
			break;
		}

	    if( (recv_buf = (u8*)MfSdkMemMalloc(CONTENT_MAX_LENGTH)) == NULL )
	    {
			APP_TRACE("MfSdkMemMalloc failed \r\n");
			break;
		}
		
	    memset(recv_buf, 0, CONTENT_MAX_LENGTH);
	    session.begin_ticket = MfSdkSysGetTick();
	    ret = AppHttpHandleResponse(&session); //GET status code

	    if (ret != 200)
	    {
	      	APP_TRACE("AppHttpHandleResponse status code : %d \r\n" , ret);  
			break;
	    }
		ret = AppHttpRecvResponseContent(&session, recv_buf, CONTENT_MAX_LENGTH);
		APP_TRACE("AppHttpRecvResponseContent retrun : %d \r\n" , ret);
		if(ret > 0) { APP_TRACE_BUFF_TIP(recv_buf, ret, "recv_buf");  }
	       
    }while(0);

	if (session.socket >= 0) { MfSdkCommSocketClose(session.socket); }

	if(recv_buf != NULL) { MfSdkMemFree(recv_buf); }
}


void AppHttpChunkedTest(char* url)
{
	s32 index = 0;
	s32 port = 80;
	MFSDKBOOL tls = MFSDK_FALSE;
	char host[256] = {0};
    char SendBuf[1024] = { 0 };
    char RecBuf[1024 * 4] = { 0 };
	u8 *recv_buf = NULL;

    AppHttpSession_T session;
    
    memset(RecBuf,0,sizeof(RecBuf));
    memset(SendBuf, 0, sizeof(SendBuf));
    memset(&session, 0, sizeof(AppHttpSession_T));

	struct http_parser_url u;
    memset(&u, 0, sizeof(u));

    if(http_parser_parse_url(url, strlen(url), 0, &u))
	{
		APP_TRACE("http_parser_parse_url failed\r\n");
		return ;
	}

	if (u.field_set & (1 << UF_SCHEMA)) 
	{
        APP_TRACE("Scheme: %.*s\n", u.field_data[UF_SCHEMA].len, url + u.field_data[UF_SCHEMA].off);
		if(u.field_data[UF_SCHEMA].len == 5 && !memcmp(url + u.field_data[UF_SCHEMA].off , "https" , 5))
		{
			tls = MFSDK_TRUE;
		}		
    }
	if(tls){ port = (u.port == 0) ? 443 : u.port; }
	else{ port = (u.port == 0) ? 80 : u.port; }

	if (u.field_set & (1 << UF_HOST)) 
	{
        APP_TRACE("Host: %.*s\n", u.field_data[UF_HOST].len, url + u.field_data[UF_HOST].off);
		snprintf(host ,sizeof(host) , "%.*s" , u.field_data[UF_HOST].len, url + u.field_data[UF_HOST].off);
    }
	else
	{
		APP_TRACE("url get host failed\r\n");
		return ;
	}

	if (u.field_set & (1 << UF_PATH)) {
        APP_TRACE("UF_PATH: %.*s\n", u.field_data[UF_PATH].len, url + u.field_data[UF_PATH].off);
    }

	APP_TRACE("host:%s\r\n",host);	

    session.buffer = RecBuf;
    session.size = sizeof(RecBuf);
    session.recv_timeout_ms = 10 * 1000;
    session.recv_resp_timeout_ms = 30 * 1000;
    session.content_length = -1;

    session.socket = MfSdkCommSocketCreate(MFSDK_COMM_SOCKET_INDEX_3);

	index += sprintf(SendBuf + index, "GET %.*s HTTP/1.1\r\n",u.field_data[UF_PATH].len, url + u.field_data[UF_PATH].off);
	index += sprintf(SendBuf + index, "Host: %s\r\n", host);
	// index += sprintf(SendBuf + index, "%s\r\n", "Content-Type: text/plain");
	index += sprintf(SendBuf + index, "%s\r\n", "Accept: */*");
	index += sprintf(SendBuf + index, "%s\r\n", "Connection: keep-alive");
	index += sprintf(SendBuf + index, "\r\n");

	if(tls)
	{
		MfSdkCommSslAuthMode(session.socket, 1);
		MfSdkCommSslInit(session.socket, "", "", "", 1);
	}
	do{
		s32 ret = MfSdkCommSocketConnect(session.socket,(s8*)host, port, 10000, (void*)0);

		if(ret != 0)
		{ 
			APP_TRACE("MfSdkCommSocketConnect failed:%d\r\n", ret);
			break; 
		}

		ret = MfSdkCommSocketSend(session.socket, (u8*)SendBuf, strlen(SendBuf));
		if(ret != strlen(SendBuf)) 
		{ 
			APP_TRACE("MfSdkCommSocketSend failed:%d\r\n", ret);
			break;
		}
		
		session.begin_ticket = MfSdkSysGetTick();
	    ret = AppHttpHandleResponse(&session); //GET status code 
	    if(ret != 200) 
	    {
	    	APP_TRACE("AppHttpHandleResponse status code : %d \r\n" , ret);  
			break;
	    }

	    recv_buf = (u8*)MfSdkMemMalloc(CONTENT_MAX_LENGTH);
	    memset(recv_buf, 0, CONTENT_MAX_LENGTH);
	    ret = AppHttpRecvResponseContent(&session,recv_buf, CONTENT_MAX_LENGTH);
		APP_TRACE("AppHttpRecvResponseContent retrun : %d \r\n" , ret);
	}while(0);

    if (session.socket >= 0) { MfSdkCommSocketClose(session.socket);  }
	if(recv_buf != NULL) { MfSdkMemFree(recv_buf); }
}



void AppHttpDemoTest()
{
	AppHttpTest("http://www.baidu.com/");
	AppHttpTest("http://www.google.com/");	
	AppHttpChunkedTest("http://www.httpwatch.com/httpgallery/chunked/chunkedimage.aspx");
}


#include "../pub/cJSON.h"
void AppFingPayQrHttp(void)
{

    int index = 0;
    char SendBuf[1024] = { 0 };
    char RecBuf[1024 * 2] = { 0 };
    u8* recv_buf = NULL;
    AppHttpSession_T session;

    memset(RecBuf, 0, sizeof(RecBuf)); // http header
    memset(SendBuf, 0, sizeof(SendBuf));
    memset(&session, 0, sizeof(AppHttpSession_T));

    session.buffer = RecBuf;
    session.size = sizeof(RecBuf);
    session.recv_timeout_ms = 10 * 1000;
    session.recv_resp_timeout_ms = 30 * 1000;
    session.content_length = -1;

    index += sprintf(SendBuf + index, "POST /fpcardwebserviceMqtt/api/microatm/server/qrcode2 HTTP/1.1\r\n");//pciuat.tapits.in
    index += sprintf(SendBuf + index, "Host: %s\r\n", "pciuat.tapits.in");
    index += sprintf(SendBuf + index, "%s\r\n", "Content-Type: text/plain");
    index += sprintf(SendBuf + index, "%s\r\n", "Accept: application/json");
    index += sprintf(SendBuf + index, "%s\r\n", "Connection: keep-alive");
    index += sprintf(SendBuf + index, "%s\r\n\r\n", "Content-Length: 5");
    index += sprintf(SendBuf + index, "%s", "Sweta");
    index += sprintf(SendBuf + index, "\r\n\r\n");


    session.socket = MfSdkCommSocketCreate(MFSDK_COMM_SOCKET_INDEX_3);

    MfSdkCommSslAuthMode(session.socket, 1);
    MfSdkCommSslInit(session.socket, "", "", "", 1);
    s32 ret = MfSdkCommSocketConnect(session.socket, "pciuat.tapits.in", 443, 10000, (void*)0);

    ret = MfSdkCommSocketSend(session.socket, (u8*)SendBuf, strlen(SendBuf));

    do {
        session.begin_ticket = MfSdkSysGetTick();
        ret = AppHttpHandleResponse(&session); //GET status code 
        if (ret != 200)
        {
            APP_TRACE("AppHttpHandleResponse status code : %d \r\n", ret);
            break;
        }

        recv_buf = (u8*)MfSdkMemMalloc(10 * 1024);
        memset(recv_buf, 0, 10 * 1024);
        ret = AppHttpRecvResponseContent(&session, recv_buf, 10 * 1024);
        APP_TRACE("AppHttpRecvResponseContent retrun : %d \r\n", ret);
    } while (0);
   // {"status":true, "message" : "upi://pay?pa=9014938219-2@axl&pn=Swetha&tr=MNOQFMSY0000000008954C5B51492750000&am=100&cu=INR&mc=7322", "data" : null, "statusCode" : 200}

    if (ret > 0)
    {
        APP_TRACE("%s\r\n", recv_buf);
        cJSON* rootobj = cJSON_Parse(recv_buf);
        if (rootobj != NULL) 
        {
            s32 statusCode = 0;
            cJSON *obj = cJSON_GetObjectItem(rootobj, "statusCode");            
            if (obj != NULL) {
                if (obj->valuestring == NULL) 
                {
                    statusCode = obj->valueint;
                }
                else
                {
                    //string
                }            
            }
            cJSON_Delete(rootobj);
        }
    }

    if (session.socket >= 0) { MfSdkCommSocketClose(session.socket); }
    if (recv_buf != NULL) { MfSdkMemFree(recv_buf); }
}

