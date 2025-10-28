#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "queue_pub.h"
#include "../tracedef.h"
#include "libapi_xpos/inc/mfsdk_fifo.h"
#include "libapi_xpos/inc/mfsdk_mem.h"

static MfSdkFifoData_T play_fifo;
static MfSdkFifoData_T key_fifo;


int fifo_init()
{
	int ret = 0;

	ret = MfSdkFifoCreate(&play_fifo, FIFO_SIZE);

	if (ret)
	{
		MfSdkFifoInit(&play_fifo);
	}
	fifo_key_init();
	return 0;
}

void fifo_set_zero()
{
	MfSdkFifoInit(&play_fifo);
}

bool fifo_checkEmpty()
{
	return MfSdkFifoIsEmpty(&play_fifo);
}

bool fifo_checkFull()
{
	return MfSdkFifoCheckFreeSpace(&play_fifo, PTR_SIZE);
}

int fifo_put(unsigned char** data)
{
	int putLen = 0;

	putLen =  MfSdkFifoPut(&play_fifo, (u8*)data, PTR_SIZE);

	APP_TRACE("fifo_put --- actual len = %d", putLen);

	if(putLen >= PTR_SIZE)
	{
		return 1;
	}
	return 0;
}

int fifo_putdata(char *outData, int lenth)
{
	int putLen =  MfSdkFifoPut(&play_fifo, (u8*)outData, lenth);
	return putLen;
}

int fifo_getdata(char *outData, int lenth)
{
	int outLen = MfSdkFifoGet(&play_fifo, (u8*)outData, lenth);
	return outLen;
}

int fifo_get(char **outData)
{
	int ret = 0;
	int outLen = 0;
	int fifo_len = 0;

	fifo_len = MfSdkGetFifoNum(&play_fifo);
	APP_TRACE("play fifo num is %d", fifo_len);

	if(fifo_len >= PTR_SIZE)
	{
		outLen = MfSdkFifoGet(&play_fifo, (u8*)outData, PTR_SIZE);
		APP_TRACE("play fifo_out actual read len = %d", outLen);
		if(outLen == PTR_SIZE)
			ret = 1;
	}
	return ret;
}

int fifo_key_init()
{
	int ret = 0;

	ret = MfSdkFifoCreate(&key_fifo, DATA_SIZE*50);

	if (ret)
	{
		MfSdkFifoInit(&key_fifo);
	}

	return 0;
}

void fifo_key_set_zero()
{
	MfSdkFifoInit(&key_fifo);
}

bool fifo_key_checkEmpty()
{
	return MfSdkFifoIsEmpty(&key_fifo);
}

bool fifo_key_checkFull()
{    
	return MfSdkFifoCheckFreeSpace(&key_fifo, DATA_SIZE);
}

int fifo_key_put(unsigned char* data)
{
	int putLen = 0;

	if(strlen((char*)data) > DATA_SIZE)
		return -1;

	putLen =  MfSdkFifoPut(&key_fifo, (u8*)data, DATA_SIZE);

	APP_TRACE("fifo_put --- actual len = %d", putLen);

	if(putLen >= DATA_SIZE)
	{
		return 1;
	}
	return 0;
}

int fifo_key_get(char *outData)
{
	int outLen = 0;
	int fifo_len = 0;
	
	fifo_len = MfSdkGetFifoNum(&key_fifo)/DATA_SIZE;
	APP_TRACE("fifo_len get len is %d", fifo_len);

	if(fifo_len > 0)
	{
		outLen = MfSdkFifoGet(&key_fifo, (u8*)outData, DATA_SIZE);
		APP_TRACE("fifo_out actual read len = %d", outLen);
	}
	return outLen;
}

int  fifo_get_last_one(char* outData)
{
	int ret = 0;
	int outLen = 0;
	int fifo_len = 0;

	fifo_len = MfSdkGetFifoNum(&key_fifo);
	APP_TRACE("fifo_len get len is %d", fifo_len);

	if(fifo_len > 0)
	{
		char *DataTmp = MfSdkMemMalloc(fifo_len + 1);
		memset(DataTmp, 0x00, fifo_len + 1);

		outLen = MfSdkFifoGet(&key_fifo, (unsigned char*)DataTmp, fifo_len);
		APP_TRACE("fifo_out actual read len = %d", outLen);

		if (outLen > 0)
		{
			//取队列最后一个
			strcpy(outData, DataTmp + fifo_len - DATA_SIZE);
			ret = 1;
		}
		MfSdkMemFree(DataTmp);
	}
	return ret;
}


