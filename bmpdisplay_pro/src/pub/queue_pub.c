#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "queue_pub.h"
//#include "pub/common/misc/inc/fifo_func.h"
#include "../tracedef.h"
//#include "pub/common/misc/inc/mfmalloc.h"

#include "libapi_xpos/inc/mfsdk_fifo.h"
#include "libapi_xpos/inc/mfsdk_mem.h"

static MfSdkFifoData_T play_fifo;

//static unsigned int tick_btn_press_last = 0;
//static int first_set_tick = 0;


int fifo_init()
{
    int ret = 0;

    ret = MfSdkFifoCreate(&play_fifo, FIFO_SIZE);

    if (ret)
    {
        MfSdkFifoInit(&play_fifo);
    }
    return 0;
}

void fifo_set_zero()
{
    MfSdkFifoInit(&play_fifo);
}

int fifo_put(unsigned char* data, int n_len)
{
    int putLen = 0;

    putLen =  MfSdkFifoPut(&play_fifo, data, n_len);

    APP_TRACE("fifo_put should write len == %d, --- actual len = %d", n_len, putLen);

    if(putLen >= n_len)
    {
        return 1;
    }
    return 0;
}

int  fifo_get_last_one(char* outData)
{
    int outLen = 0;
    int fifo_len = 0;
    int last_data_offset = 0;

    fifo_len = MfSdkGetFifoNum(&play_fifo);
    APP_TRACE("fifo_len get len is %d", fifo_len);

    if(fifo_len > 0)
    {
        MfSdkFifoData_T* pFifo = &play_fifo;
        //char tmp[100] = { 0 };
        char *DataTmp;
        DataTmp = MfSdkMemMalloc(fifo_len + 1);
        memset(DataTmp, 0x00, fifo_len + 1);

        outLen = MfSdkFifoGet(pFifo, (unsigned char*)DataTmp, fifo_len);
        APP_TRACE("fifo_out actual read len = %d", outLen);

        if (outLen > 0)
        {
            //取队列最后一个
            last_data_offset = (fifo_len / DATA_SIZE) - 1;
            //memcpy(outData, DataTmp+ last_data_offset* DATA_SIZE, DATA_SIZE);
            strcpy(outData, DataTmp + last_data_offset * DATA_SIZE);
            MfSdkMemFree(DataTmp);
            return 1;
        }
        MfSdkMemFree(DataTmp);
    }
    return 0;
}

#define MAX_QUEUE_SIZE 20


CharPointerQueue queue_ptr;
CharPointerQueue queue_ptr_tts;
CharPointerQueue queue_ptr_iot;
CharPointerQueue* get_tts_queue()
{
    return &queue_ptr_tts;
}

CharPointerQueue* get_iotrevert_queue()
{
    return &queue_ptr_iot;
}

// 初始化队列
void _fifo_init_ptr(CharPointerQueue* queue)
{
    queue->data = (char**)MfSdkMemMalloc(MAX_QUEUE_SIZE * sizeof(char*));
    queue->front = 0;
    queue->rear = -1;
    queue->isEmpty = true;
}

void fifo_init_ptr()
{
    _fifo_init_ptr(&queue_ptr);
    _fifo_init_ptr(&queue_ptr_tts);
    _fifo_init_ptr(&queue_ptr_iot);
}

// 将指针放入队列
bool _fifo_put_ptr(CharPointerQueue* queue, char* item)
{
    if (queue->rear == MAX_QUEUE_SIZE - 1)
    {
        APP_TRACE("The queue is full and cannot continue adding elements.\n");
        return false;
    }
    queue->rear++;
    queue->data[queue->rear] = item;
    queue->isEmpty = false;
    APP_TRACE("_fifo_put_ptr success\n");
    return true;
}

bool fifo_put_ptr(char* item)
{
    APP_TRACE("fifo_put_ptr.\n");
    return _fifo_put_ptr(&queue_ptr, item);
}

// 从队列中取出指针
char* _fifo_get_ptr(CharPointerQueue* queue)
{
    if (queue->isEmpty)
    {
        APP_TRACE("The queue is empty and cannot retrieve elements.\n");
        return NULL;
    }
    char* item = queue->data[queue->front];
    queue->front++;

    if (queue->front > queue->rear)
    {
        queue->front = 0;
        queue->rear = -1;
        queue->isEmpty = true;
    }
    APP_TRACE("_fifo_get_ptr success\n");
    return item;
}

char* fifo_get_ptr()
{
    return _fifo_get_ptr(&queue_ptr);
}

// 检查队列是否为空
bool _fifo_checkEmpty_ptr(CharPointerQueue* queue)
{
    APP_TRACE("_fifo_checkEmpty_ptr = %d", queue->isEmpty);
    return queue->isEmpty;
}

bool fifo_checkEmpty_ptr()
{
    return _fifo_checkEmpty_ptr(&queue_ptr);
}

// 设置队列为空
void _fifo_setEmpty_ptr(CharPointerQueue* queue)
{
    queue->front = 0;
    queue->rear = -1;
    queue->isEmpty = true;
}

void fifo_setEmpty_ptr()
{
    _fifo_setEmpty_ptr(&queue_ptr);
}

// 释放队列内存
void _fifo_cleanup_ptr(CharPointerQueue* queue)
{
    MfSdkMemFree(queue->data);
    queue->data = NULL;
    queue->front = 0;
    queue->rear = -1;
    queue->isEmpty = true;
}

void fifo_cleanup_ptr()
{
    _fifo_cleanup_ptr(&queue_ptr);
}

