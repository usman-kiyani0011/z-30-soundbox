#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libapi_xpos/inc/mfsdk_define.h"

typedef struct
{
    char* buffer;
    s32 size;
    s32 length;
    s32 socket; //real socket
    s32 resp_status;
    s32 recv_timeout_ms;
    s32 begin_ticket;
    s32 recv_resp_timeout_ms;

    s32 chunk_sz;
    s32 chunk_offset;

    s32 content_length;
    u32 content_remainder;           /* remainder of content length */
}AppHttpSession_T;

int AppHttpHandleResponse(AppHttpSession_T* session);
s32 AppHttpRecvResponseContent(AppHttpSession_T* session, u8* recv_buf, s32 recv_buf_len);

