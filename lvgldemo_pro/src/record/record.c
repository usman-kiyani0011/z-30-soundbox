#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include "pub/osl/inc/osl_filedef.h"
#include "record.h"
#include "../player_proc.h"
#include "../pub/cJSON.h"
#include "../tracedef.h"
#include "libapi_xpos/inc/mfsdk_mem.h"
#include "libapi_xpos/inc/mfsdk_fs.h"
#include "libapi_xpos/inc/mfsdk_gui.h"
#include "../page_main.h"
#include "../player_proc.h"
#include "../card/func_pay.h"

typedef struct  _st_order_info 
{
	int len;
	char msg_payload[256 + 1];
}st_order_info;

#define ORDER_INFO_FILE   "record.ini"
#define ARR_INFO_MAX	  5
#define INFO_SIZE		  sizeof(st_order_info)

static int mIndex = 0;    //total index
static int mShowIndex = 0;//show index
static int mNoDataFlag = 0;
static char *order_info = NULL;


//clear order record
void ClearOrderRecord()
{
	APP_TRACE("ClearOrderRecord\r\n");
	mIndex = 0;    //total index
	mShowIndex = 0;//show index
	mNoDataFlag = 1;  
	memset(order_info, 0x00, INFO_SIZE*ARR_INFO_MAX+1);
	MfSdkFsUnlink((const s8*)ORDER_INFO_FILE);
}

//load order record
void LoadOrderRecord()
{
	int read_bytes = 0;
	int hfile = MfSdkFsOpen((const s8*)ORDER_INFO_FILE, MFSDK_FS_FLAG_WRITE, MFSDK_FS_MODE_READ);
	int nlen =0;

	APP_TRACE("LoadOrderRecord\r\n");
	order_info = MfSdkMemMalloc(INFO_SIZE * ARR_INFO_MAX + 1);
	if(NULL == order_info)
	{
		APP_TRACE("LoadOrderRecord error!\r\n");
		return;
	}
	memset(order_info,0x00,INFO_SIZE*ARR_INFO_MAX+1);

	if (hfile >= MFSDK_FS_RET_OK)
	{
		MfSdkFsLseek(hfile, 0, MFSDK_FS_SEEK_SET);
		nlen = MfSdkFsLseek(hfile, 0, MFSDK_FS_SEEK_END);
		if(nlen>INFO_SIZE*ARR_INFO_MAX+4)
		nlen=INFO_SIZE*ARR_INFO_MAX+4;
		MfSdkFsLseek(hfile, 0, MFSDK_FS_SEEK_SET);
		MfSdkFsRead(hfile,(char*)&mIndex,4);
		read_bytes = MfSdkFsRead(hfile, (char*)order_info, INFO_SIZE*ARR_INFO_MAX);

		APP_TRACE("read_bytes = %d\r\n", read_bytes);
		if (read_bytes < 4)
		{
			mNoDataFlag = 1;
			//no_info_show();
			APP_TRACE("no data\r\n");
		}
		MfSdkFsClose(hfile);		
	}
	
	RecordUpdataIndex(0);
}

static void AddOrderRecordMax(st_order_info *OrderInfo)
{
	char *arr_info_tmp = MfSdkMemMalloc(ARR_INFO_MAX * INFO_SIZE+1);
	if(NULL == arr_info_tmp)
		return;

	if (NULL != order_info)
	{
		APP_TRACE("SaveOrderRecordMax[%d] %s\r\n", OrderInfo->len, OrderInfo->msg_payload);
		memset(arr_info_tmp, 0, ARR_INFO_MAX * INFO_SIZE+1);
		memcpy(arr_info_tmp, order_info+INFO_SIZE, (ARR_INFO_MAX - 1)*INFO_SIZE );
		memcpy(arr_info_tmp+(ARR_INFO_MAX-1)*INFO_SIZE, OrderInfo, INFO_SIZE);
		memset(order_info, 0x00, ARR_INFO_MAX*INFO_SIZE);
		memcpy(order_info, arr_info_tmp, ARR_INFO_MAX*INFO_SIZE);
	}
	MfSdkMemFree(arr_info_tmp);
}

static int AddOrderInfo(st_order_info *OrderInfo)
{
//	int i =0;
	int hfile =-1;
	int index_tmp = mIndex;

	if(index_tmp+1 > ARR_INFO_MAX)
	{
		AddOrderRecordMax(OrderInfo);
		mIndex = ARR_INFO_MAX;
		mShowIndex = ARR_INFO_MAX - 1;
	}
	else
	{
		memcpy(order_info+mIndex*INFO_SIZE,OrderInfo,INFO_SIZE);
		mIndex++;
		mShowIndex = mIndex - 1;
	}
	MfSdkFsUnlink((const s8 *) ORDER_INFO_FILE);
	hfile = MfSdkFsOpen((const s8*)ORDER_INFO_FILE, MFSDK_FS_FLAG_WRITE, MFSDK_FS_MODE_READ);

	if (hfile >= MFSDK_FS_RET_OK)
	{
		MfSdkFsLseek(hfile, 0, MFSDK_FS_SEEK_SET);
		MfSdkFsWrite(hfile, (char*)&mIndex, 4);
		MfSdkFsWrite(hfile, (char*)order_info, INFO_SIZE*ARR_INFO_MAX);
		MfSdkFsClose(hfile);
		mNoDataFlag = 0;
		APP_TRACE("add order succ\r\n");
	}
	return 0;
}
 
void SaveOrderRecord(char *s_data, int len)
{
	st_order_info *info_tmp = NULL;

	if(len < INFO_SIZE)
	{
		info_tmp = (st_order_info *)MfSdkMemMalloc(INFO_SIZE);
		if(NULL == info_tmp) return;

		info_tmp->len = len;
		memcpy(info_tmp->msg_payload, s_data, len);
		AddOrderInfo(info_tmp);
		MfSdkMemFree(info_tmp);
	}
}

static void RecordShowAmount(int index)
{
	char s_tmp[8 + 1] = { 0 };
	char s_amt[16 + 1] = { 0 };
	cJSON* rootobj = NULL;

	st_order_info* msg = MfSdkMemMalloc(INFO_SIZE);
	if(NULL == msg) return;

	memset(msg, 0, INFO_SIZE);
	memcpy(msg, order_info+index*INFO_SIZE, INFO_SIZE);

	rootobj = cJSON_Parse(msg->msg_payload);
	if (rootobj != NULL) 
	{
		if (json_getval(rootobj, "money", s_amt, sizeof(s_amt)-1) > 0)
		{
			//clear amount timer
			ClearAmountTimer();
			MfSdkGuiLedAmount(s_amt);

			//show record index
			sprintf(s_tmp, "%d", index + 1);
			MfSdkGuiLedCounter(s_tmp);
		}
		cJSON_Delete(rootobj);
	}
	MfSdkMemFree(msg);
}

void RecordUpdataIndex(int up_data)
{
	int n_ret = 0;

	APP_TRACE("RecordUpdataIndex mNoDataFlag = %d\r\n", mNoDataFlag);
	if (mNoDataFlag != 0)
	{
		return ;
	}
	//up_data  0£ºcurrent record£» -1£ºprev rec£» +1£ºnext rec
	if (up_data == 0)
	{
		if (mIndex >= ARR_INFO_MAX)
		{
			mIndex = ARR_INFO_MAX;
			mShowIndex = ARR_INFO_MAX - 1;
		}
		else
		{
			mShowIndex = mIndex - 1;
		}
	}
	else 
	{
		n_ret = mShowIndex + up_data;
		APP_TRACE("mShowIndex = %d, n_ret = %d", mShowIndex, n_ret);
		if (n_ret >= ARR_INFO_MAX)
		{
			APP_TRACE("SHOW_INDEX IS MAX");
			mShowIndex = ARR_INFO_MAX - 1;
		}
		else if (n_ret < 0)
		{
			APP_TRACE("SHOW_INDEX IS MIN");
			mShowIndex = 0;
		}
		else 
		{
			cJSON* rootobj = NULL;
			char s_paytype[10 + 1] = { 0 };
			st_order_info *msg=MfSdkMemMalloc(INFO_SIZE);
			if(NULL == msg) return;

			memset(msg, 0, INFO_SIZE);
			memcpy(msg, order_info+n_ret*INFO_SIZE, INFO_SIZE);

			rootobj = cJSON_Parse(msg->msg_payload);
			if (rootobj != NULL) 
			{
				if (json_getval(rootobj, "broadcast_type", s_paytype, sizeof(s_paytype)-1) > 0)
				{
					if (atoi(s_paytype) > 0) 
					{
						mShowIndex = n_ret;
					}
				}
				cJSON_Delete(rootobj);
			}
			MfSdkMemFree(msg);
		}
		RecordShowAmount(mShowIndex);
	}
}
 
void ReplayRecord(int up_data)
{
	cJSON* rootobj = NULL;
	st_order_info *msg = MfSdkMemMalloc(INFO_SIZE);

	if(NULL == msg) return;

	RecordUpdataIndex(up_data);
	memset(msg, 0, INFO_SIZE);
	memcpy(msg, order_info+mShowIndex*INFO_SIZE, INFO_SIZE);

	rootobj = cJSON_Parse(msg->msg_payload);
	if (rootobj != NULL) 
	{
		APP_TRACE("ReplayRecord: %s\r\n", msg->msg_payload);
		voice_play_func(rootobj);
		cJSON_Delete(rootobj);
	}
	else
	{
		APP_TRACE("cJSON_Parse error\r\n");
	}
	
	MfSdkMemFree(msg);
	return;
}


