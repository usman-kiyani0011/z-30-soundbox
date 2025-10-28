/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#include "http_parser.h"
#include <stdlib.h>
//#include <assert.h>
#include <stdio.h>
#include <stdlib.h> /* rand */
#include <string.h>
#include <stdarg.h>

#include "../tracedef.h"
#include "libapi_xpos/inc/mfsdk_define.h"

static MFSDKBOOL bParsed = MFSDK_FALSE;

typedef struct 
{
	u8 bodyData[4*1024];
	s32 bodyDataLength;
}BodyDate_T;

static BodyDate_T s_BodyData;

int onMessageBegin(http_parser* pParser)
{
    APP_TRACE("@onMessageBegin call \n");
    bParsed = MFSDK_FALSE;
	memset(&s_BodyData ,0,sizeof(BodyDate_T));
    return 0;
}
int onHeaderComplete(http_parser* pParser)
{
    APP_TRACE("@onHeaderComplete call \n");
    return 0;
}
int onMessageComplete(http_parser* pParser)
{
    APP_TRACE("@onMessageComplete call \n");
    bParsed = MFSDK_TRUE;
    return 0;
}
int onURL(http_parser* pParser, const char *at, size_t length)
{
    APP_TRACE("@onURL call, length:[%d] \n", length);
    
    APP_TRACE_BUFF_TIP(at,length,"@onURL url");
    return 0;
}
int onStatus(http_parser* pParser, const char *at, size_t length)
{
    APP_TRACE("@onStatus call, length:[%d] \n", length);
    APP_TRACE_BUFF_TIP(at,length,"@onStatus");
    return 0;
}
int onHeaderField(http_parser* pParser, const char *at, size_t length)
{
    APP_TRACE("@onHeaderField call, length:[%d] \n", length);
    APP_TRACE_BUFF_TIP(at,length,"@onHeaderField");
    return 0;
}
int onHeaderValue(http_parser* pParser, const char *at, size_t length)
{
    APP_TRACE("@onHeaderValue call, length:[%d] \n", length);
    APP_TRACE_BUFF_TIP(at,length,"@onHeaderValue");
    return 0;
}
int onBody(http_parser* pParser, const char *at, size_t length)
{
    APP_TRACE("@onBody call, length:[%d] \n", length);
    
	strncat((char*)s_BodyData.bodyData,at,length);
	s_BodyData.bodyDataLength += length;


	APP_TRACE("@onBody recv:[%s] \n",s_BodyData.bodyData);
    return 0;
}

int chunk_header_cb(http_parser* p)
{
//	assert(p == parser);
//	int chunk_idx = messages[num_messages].num_chunks;
//	messages[num_messages].num_chunks++;
//	if (chunk_idx < MAX_CHUNKS) {
//		messages[num_messages].chunk_lengths[chunk_idx] = p->content_length;
//	}
 	APP_TRACE("@chunk_header_cb p->content_length:[%d] \r\n",p->content_length);
	return 0;
}
 
int chunk_complete_cb(http_parser* p)
{

APP_TRACE("@chunk_complete_cb \r\n");

//	assert(p == parser);
 
	/* Here we want to verify that each chunk_header_cb is matched by a
	* chunk_complete_cb, so not only should the total number of calls to
	* both callbacks be the same, but they also should be interleaved
	* properly */
//	assert(messages[num_messages].num_chunks == messages[num_messages].num_chunks_complete + 1);
// 
//	messages[num_messages].num_chunks_complete++;
	return 0;
}

int AppHttpPraseMain (void)
{
	http_parser httpParser;
    http_parser_settings httpSettings;

    // 初使化解析器及回调函数
    http_parser_init(&httpParser, HTTP_RESPONSE);
    http_parser_settings_init(&httpSettings);
    httpSettings.on_message_begin = onMessageBegin;
    httpSettings.on_headers_complete = onHeaderComplete;
    httpSettings.on_message_complete = onMessageComplete;
    httpSettings.on_url = onURL;
    httpSettings.on_status = onStatus;
    httpSettings.on_header_field = onHeaderField;
    httpSettings.on_header_value = onHeaderValue;
    httpSettings.on_body = onBody;
	httpSettings.on_chunk_header = chunk_header_cb;
  	httpSettings.on_chunk_complete = chunk_complete_cb;

	char strHttpRes[3*1024] = {0};

	strcat(strHttpRes,"HTTP/1.1 200 OK\r\n");
	strcat(strHttpRes,"Cache-Control: private\r\n");
	strcat(strHttpRes,"Content-Type: application/json); charset=utf-8\r\n");
	strcat(strHttpRes,"Server: Microsoft-IIS/8.0\r\n");
	strcat(strHttpRes,"X-AspNetMvc-Version: 5.2\r\n");
	strcat(strHttpRes,"X-AspNet-Version: 4.0.30319\r\n");
	strcat(strHttpRes,"X-Powered-By: ASP.NET\r\n");
	strcat(strHttpRes,"Date: Sat, 24 Oct 2020 02:45:29 GMT\r\n");
	strcat(strHttpRes,"Content-Length: 30\r\n");
	strcat(strHttpRes,"\r\n");
	strcat(strHttpRes,"{\"State\":\"Success\",\"Msg\":\"OK\"}");
	
    // 解析响应
    int nParseBytes = http_parser_execute(&httpParser, &httpSettings, strHttpRes, strlen(strHttpRes));
	
    APP_TRACE("http_parser_execute => parsebytes:[%d] parseok:[%d] \n", nParseBytes, bParsed);


	APP_TRACE("status_code: [%d] \n", httpParser.status_code);

    // 解析成功，打印解析结果
    if (bParsed)
    {
        APP_TRACE("s_BodyData.bodyData: [%s] \n", s_BodyData.bodyData);
    }

	APP_TRACE("*******************************************************\r\n");
	
	http_parser_init(&httpParser, HTTP_RESPONSE);
	http_parser_settings_init(&httpSettings);
	httpSettings.on_message_begin = onMessageBegin;
	httpSettings.on_headers_complete = onHeaderComplete;
	httpSettings.on_message_complete = onMessageComplete;
	httpSettings.on_url = onURL;
	httpSettings.on_status = onStatus;
	httpSettings.on_header_field = onHeaderField;
	httpSettings.on_header_value = onHeaderValue;
	httpSettings.on_body = onBody;
	httpSettings.on_chunk_header = chunk_header_cb;
	httpSettings.on_chunk_complete = chunk_complete_cb;

	memset(strHttpRes,0,sizeof(strHttpRes));
	strcat(strHttpRes,"HTTP/1.1 200 OK\r\n");
	
	strcat(strHttpRes,"Content-Type: text/plain\r\n");
	strcat(strHttpRes,"Transfer-Encoding: chunked\r\n");
	strcat(strHttpRes,"\r\n"); 
	strcat(strHttpRes,"1\r\n");
	strcat(strHttpRes,"T\r\n");
	strcat(strHttpRes,"1\r\n");
	strcat(strHttpRes,"y\r\n");
	strcat(strHttpRes,"1\r\n");
	strcat(strHttpRes,"x\r\n");
	strcat(strHttpRes,"0\r\n");
	strcat(strHttpRes,"\r\n");


	nParseBytes = http_parser_execute(&httpParser, &httpSettings, strHttpRes, strlen(strHttpRes));
	
	APP_TRACE("http_parser_execute => parsebytes:[%d] parseok:[%s] \n", nParseBytes, bParsed? "success" : "fail");
	
	APP_TRACE("status_code: [%d] \n", httpParser.status_code);

	// 解析成功，打印解析结果
	if (bParsed)
	{
		APP_TRACE("s_BodyData.bodyData: [%s] \n", s_BodyData.bodyData);
	}

	
	return 0;
}
