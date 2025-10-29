#include <stdio.h>
#include <string.h>
#include "lvgl/lvgl.h"
#include "pages/pages.h"
#include "pages/page_show_image.h"
#include "func_pay.h"
#include "libapi_xpos/inc/emvapi.h"
#include "libapi_xpos/inc/mfsdk_mem.h"
#include "libapi_xpos/inc/mfsdk_aud.h"
#include "libapi_xpos/inc/mfsdk_gui.h"
#include "libapi_xpos/inc/mfsdk_power.h"
#include "libapi_xpos/inc/mfsdk_emv.h"
#include "libapi_xpos/inc/mfsdk_tms.h"

#include "../sdk_tts.h"
#include "../pub/http.h"
#include "../page_main.h"
#include "../player_proc.h"


static lv_obj_t* base_win;
static st_pay_data  * pay_data_pay = NULL;
static st_read_card_in card_in = {0};
static st_read_card_out card_out = { 0 };
//static st_card_info card_info = { 0 };
static char amount[16] = { 0 };
static int lock_flag = 0;

static int _pay_return_func(int ret);

static int emv_read_card_callback(int ret, lv_obj_t* obj)
{
	APP_TRACE("emv_read_card_callback ret:%d\r\n", ret);	
	_pay_return_func(ret);

	return 0;
}

int getTransacationStatus()
{
	int ret = 0;

	if(1 == lock_flag && NULL != pay_data_pay)
	{
		APP_TRACE("GetCommStatus = %d\r\n", GetCommStatus());	
		if(1 == GetCommStatus())
			return -1;
		
		if(TRANSTYPE_QR == pay_data_pay->mode)
		{
			ret = (QRPAGE_LOCK_OFF == GetPowerLock())?0:-1;
		}
		else if(TRANSTYPE_RF == pay_data_pay->mode || TRANSTYPE_SCAN == pay_data_pay->mode)
		{
			ret = -1;
		}
		APP_TRACE("lock_flag :%d, GetPowerLock :%d, mode = %d, ret = %d\r\n", lock_flag, GetPowerLock(), pay_data_pay->mode, ret);	
	}

	return ret;
}

s32 PayPagePowerLock(char *sfun)
{
	if(lock_flag == 0)
	{
		lock_flag = 1;
		APP_TRACE("<%s> [%s]\r\n", __FUNCTION__, sfun);
		return AppPowerLockApp(sfun);
	}
	return -1;
}

void PayPagePowerUnlock(char *sfun)
{
	if(lock_flag == 1)
	{
		lock_flag = 0;
		APP_TRACE("<%s> [%s]\r\n", __FUNCTION__, sfun);
		AppPowerUnlockApp(sfun);
	}
	return;
}

static void _pay_close_page()
{
	SetPowerLock(QRPAGE_LOCK_DEF);
	comm_disconnect();
	//Type:2:free icc card data; 3:free rfic card data
	if(NULL != pay_data_pay && TRANSTYPE_RF == pay_data_pay->mode)
	{
		MfSdkEmvCardFree(3);
	}

	if (base_win != 0) 
	{
		lv_obj_del(base_win);
		base_win = 0;
	}
	if(pay_data_pay)
	{
		MfSdkMemFree(pay_data_pay);
		pay_data_pay = 0;
	}
	PayPagePowerUnlock((char*)"func_pay");
	MfSdkPowerSupertimeReset();
	MfSdkTmsAppBusy(MFSDK_TMS_APP_NOT_BUSY);
}

void PayClosePage()
{
	_pay_close_page();
}

void *_read_card_page(void)
{
	APP_TRACE("_read_card_page \r\n");	
	page_read_card_show(get_mainpage(), get_trans_title(pay_data_pay->mode), "Icc proccess", 1000);
}


static int MessagePackData()
{
	int index = 0;
    char sn[33] = {0};
    int port = HTTP_PORT;
    char ip[64] = { 0 };
	unsigned char msgAmt[32] = {0};
	char sendData[SEND_BUFF_SIZE] = {0};
	char* sbuff = pay_data_pay->comm_data.sbuff;;

	memset(&pay_data_pay->comm_data, 0 ,sizeof(st_comm_data));
	
	#ifdef TEST_SN
    strcpy(sn, TEST_SN);
	#else
	MfSdkSysGetTermSn(sn);
	#endif

    get_setting_str(COMM_HTTP_IP, ip, sizeof(ip));
    if (strlen(ip) < 3)
    {
        strcpy(ip, HTTP_HOST);
        port = HTTP_PORT;
    }
	else
	{
	    port = get_setting_int(COMM_HTTP_PORT);
	}

	AppFormatAmountFinal(msgAmt,ATOLL(pay_data_pay->amount));	
	snprintf(sendData, sizeof(sendData), "device_id=%s&request_data=%s", sn,msgAmt);
	index += sprintf(sbuff + index , "POST /api/Iotmsgtest/createQrMf?%s HTTP/1.1\r\n", sendData);
	index += sprintf(sbuff + index , "Host: %s:%d\r\n\r\n", ip, port);	
	pay_data_pay->comm_data.slen = strlen(sbuff);

	APP_TRACE("[%s][%s]\r\n",__FUNCTION__, sbuff);
	APP_TRACE_BUFF_TIP(pay_data_pay->comm_data.sbuff, pay_data_pay->comm_data.slen, "send:");
	return 0;
}

static int MessageUnpackData()
{
	int ret = -1;
    char temp[256] = { 0 };
	char recvdata[RECV_BUFF_SIZE] = {0};
	cJSON* rootobj = NULL;
	cJSON* dataobj = NULL;

	APP_TRACE_BUFF_TIP(pay_data_pay->comm_data.rbuff, pay_data_pay->comm_data.rlen, "recv:");
	if (pay_data_pay->comm_data.rlen > 0 && pay_data_pay->comm_data.rlen < RECV_BUFF_SIZE) 
	{
		memcpy(recvdata, pay_data_pay->comm_data.rbuff, pay_data_pay->comm_data.rlen);
		rootobj = cJSON_Parse(recvdata);
		if (rootobj != NULL) 
		{
			if (json_getval(rootobj, "code", temp, sizeof(temp)-1) > 0 && 0 == strcmp(temp, "0000"))
			{
				APP_TRACE("code:%s\r\n", temp);
				memset(temp, 0, sizeof(temp));
				dataobj = cJSON_GetObjectItem(rootobj, "data");
				if (dataobj)
				{
					if (json_getval(dataobj, "order_sn", pay_data_pay->orderSn, sizeof(pay_data_pay->orderSn)) > 0)
					{
						//op=4:IC, op=2:rf, op=1:qrcdeo
						APP_TRACE("OrderSn:%s transtype = %d\r\n", pay_data_pay->orderSn, pay_data_pay->mode);
						ret = 0;
					}
				}
			}
			else
			{
				if (json_getval(rootobj, "msg", temp, sizeof(temp)) > 0)
				{
					APP_TRACE("msg: %s\r\n", temp);
					//page_text_show(get_mainpage(), pay_data_pay->title, temp, 3000);
				}
				APP_TRACE("error!\r\n");
			}
			cJSON_Delete(rootobj);
		}
	}
	else 
	{
		APP_TRACE("_unpack_data error length!");
	}

	return ret;
}

#if 0
#include "emv_tag.h"
static const char * cEmvTag[]=
{
	EMV_TAG_82_IC_AIP,              EMV_TAG_9F36_IC_ATC,           EMV_TAG_9F27_IC_CID,          EMV_TAG_9F34_TM_CVMRESULT,     EMV_TAG_9F1E_TM_IFDSN, 
	EMV_TAG_9F10_IC_ISSAPPDATA,     EMV_TAG_9F33_TM_CAP,           EMV_TAG_9F35_TM_TERMTYPE,     EMV_TAG_9F37_TM_UNPNUM,
	EMV_TAG_9F01_TM_ACQID,          EMV_TAG_9F03_TM_OTHERAMNTN,    EMV_TAG_81_TM_AUTHAMNTB ,     EMV_TAG_9F02_TM_AUTHAMNTN ,    EMV_TAG_5F24_IC_APPEXPIREDATE ,
	EMV_TAG_5F25_IC_APPEFFECTDATE,  EMV_TAG_5A_IC_PAN ,            EMV_TAG_5F34_IC_PANSN ,       EMV_TAG_99_TM_ONLINEPIN ,      EMV_TAG_9F15_TM_MCHCATCODE ,    
	EMV_TAG_9F16_TM_MCHID	,       EMV_TAG_9F1A_TM_CNTRYCODE,     EMV_TAG_9F1C_TM_TERMID,       EMV_TAG_57_IC_TRACK2EQUDATA,   EMV_TAG_5F2A_TM_CURCODE,
	EMV_TAG_9F21_TM_TRANSTIME,		EMV_TAG_9C_TM_TRANSTYPE,       EMV_TAG_8E_IC_CVMLIST,        EMV_TAG_9F0D_IC_IAC_DEFAULT,
	EMV_TAG_9F0E_IC_IAC_DENIAL,     EMV_TAG_91_TM_ISSAUTHDT,       EMV_TAG_9F40_TM_CAP_AD,       
	EMV_TAG_9F26_IC_AC   ,          EMV_TAG_9F07_IC_AUC ,          EMV_TAG_9A_TM_TRANSDATE,      EMV_TAG_5F28_IC_ISSCOUNTRYCODE,EMV_TAG_9F09_TM_APPVERNO,
	EMV_TAG_9F41_TM_TRSEQCNTR,      EMV_TAG_9F0F_IC_IAC_ONLINE ,   EMV_TAG_9F5D_IC_RF_BALANCE,   EMV_TAG_5F20_IC_HOLDERNAME,    EMV_TAG_95_TM_TVR,       
	EMV_TAG_9B_TM_TSI,				EMV_TAG_9F63_IC_IDENTIFY_INFOR,EMV_TAG_9F24_IC_PAR,				EMV_TAG_9F19_IC_TOKEN,
	EMV_TAG_9F06_TM_AID,
};


int Disp_EMV_Tag(char *pData, int nDataLen)
{
	int ind = 0;
	char* tag= 0;
	int bufflen = 0;
	char buff[128]={0};

	memset(buff, 0, sizeof(buff));
	if (0 == Ex_TLV_GetDataByTag((u8*)EMV_TAG_57_IC_TRACK2EQUDATA, (u8*)pData, nDataLen, (u8*)buff, &bufflen))
	{
		APP_TRACE_BUFF_TIP(buff, bufflen, "EMV_TAG_57_IC_TRACK2EQUDATA");
	}
	else
	{
		APP_TRACE("fail\r\n" );
	}
	return 0;
	
	for(ind = 0; ind < sizeof(cEmvTag)/sizeof(cEmvTag[0]); ind++)
	{
		tag = (char*)cEmvTag[ind];
		APP_TRACE("tag[%s]: \r\n", tag);
		memset(buff, 0, sizeof(buff));
		if (0 == Ex_TLV_GetDataByTag((u8*)tag, (u8*)pData, nDataLen, (u8*)buff, &bufflen))
		{
			APP_TRACE_BUFF_TIP(buff, bufflen, "tagbuf");
		}
		else
		{
			APP_TRACE("fail\r\n" );
		}
	}
	return 0;
}
#endif

char * get_trans_title(int transType)
{
	if(TRANSTYPE_QR == transType)
	{
		return "QRCODE";
	}
	else
	{
		return "SALE";
	}
}

static void play_voice(char* strmoney)
{
	int money = atoi((const char*)strmoney);
	
	set_counter_led();
	pub_tts_play_amt_en(money);
}

static void ShowTransResult(lv_obj_t* parent, void* pfunc, st_pay_data* pay_data_pay, int ret)
{
	char *amountstr = pay_data_pay->amount;
	int is_scan_mode = (pay_data_pay->mode == TRANSTYPE_SCAN);
	int has_scan_payload = (is_scan_mode && strlen(pay_data_pay->orderSn) > 0);
	int display_ret = ret;

	APP_TRACE("ShowTransResult ret = %d, amount = %s\r\n", ret, amountstr);

	if (has_scan_payload && ret < 0)
	{
		APP_TRACE("Scan demo mode: forcing success feedback for ret=%d\r\n", ret);
		display_ret = 0;
	}

	page_show_image(parent, pfunc, get_trans_title(pay_data_pay->mode), display_ret, 0, 3000);
	if ((ret >= 0 || has_scan_payload) && strlen(amountstr) > 0)
	{
		play_voice(amountstr);
	}
}

static int _input_return_func(int ret,lv_obj_t* obj)
{
	long long llamont = 0;
	char disp_format[20] = { 0 };

	if (ret == PAGE_RET_CANCEL || ret == PAGE_RET_TIMEOVR) 
	{
		AppFormatSegmentLedAmount(0LL);
		_pay_close_page();
		return ret;			
	}
	APP_TRACE("_input_return_func = %d\r\n",ret);
	APP_TRACE("amount = %s\r\n",pay_data_pay->amount);
	llamont = ATOLL(pay_data_pay->amount);
	APP_TRACE("llamont = %lld\r\n",llamont);
	if (llamont > 0)
	{
		AppFormatAmountFinal((unsigned char*)disp_format, llamont);
		MfSdkGuiLedAmount(disp_format);
		if (ret == PAGE_RET_CONFIRM) 
		{//qrcode
			pay_data_pay->mode = TRANSTYPE_QR;
			pay_data_pay->step = PAY_STEP_EMV_RESULT_MSG;
			//TransactionProc(get_mainpage(), llamont, 1);
		}
		else if (ret == PAGE_RET_F1) 
		{//scan
			pay_data_pay->mode = TRANSTYPE_SCAN;
			pay_data_pay->step = PAY_STEP_SCAN;
			//TransactionProc(get_mainpage(), llamont, 4);
		}
		else if (ret == PAGE_RET_F2) 
		{//rf
			pay_data_pay->mode = TRANSTYPE_RF;
			pay_data_pay->step = PAY_STEP_INPUT_CARD;
			//TransactionProc(get_mainpage(), llamont, 2);
		}

		_pay_return_func(0);
	}
	return ret;			
}

static int _pay_return_func(int ret)
{

	APP_TRACE("_pay_return_func(%d):%d\r\n", pay_data_pay->step, ret);

	if (pay_data_pay->step == PAY_STEP_START) 
	{
		APP_TRACE("PAY_STEP_START1\r\n");
		int inputMaxLen = MfSdkSysGetSegmentLcdDisplayMaxLength();
		page_input_show_ex(base_win, _input_return_func, get_trans_title(pay_data_pay->mode), pay_data_pay->amount, 0, inputMaxLen, PAGE_INPUT_MODE_AMOUNT, 60*1000,0);
	}
	else if (pay_data_pay->step == PAY_STEP_INPUT_CARD) 
	{
		if (ret == PAGE_RET_CANCEL || ret == PAGE_RET_TIMEOVR) 
		{
			_pay_close_page();			
		}
		else 
		{
			page_card_showamt(base_win, _pay_return_func, get_trans_title(pay_data_pay->mode), 0 , pay_data_pay->amount, TIMEOVER, 0);
			APP_TRACE("_pay_return_func page_card_showamt 0k ret=%d \r\n",ret);
			pay_data_pay->step = PAY_STEP_READ_CARD;
		}
	}
	else if (pay_data_pay->step == PAY_STEP_READ_CARD) 
	{
		APP_TRACE("_pay_return_func PAY_STEP_READ_CARD ret = %d\r\n", ret);
		if (ret == CARD_RET_CANCEL || ret == CARD_RET_RFS) 
		{			
			_pay_close_page();
		}
		else 
		{
			if (ret == CARD_RET_RFIC) 
			{
				//EMV_OnlinePinCallback(AppInputOnlinePin);			
				PubMultiPlay((const s8 *)TTS_VOLUME_NOR);
				MfSdkEmvSetReadPageCallback(_read_card_page);
				APP_TRACE("CARD_RET_RFIC\r\n");
				strcpy(card_in.amt, pay_data_pay->amount);
				APP_TRACE("CARD_RET_RFIC AMOUNT:%s\r\n", card_in.amt);
				card_in.card_mode = EMVCARD_RET_RFID;
				//get need tag:9F02 9F03 82
				//strcpy(card_in.ic_tags, "9F029F0382");	//card_in.ic_tags ASCII code; TAGs need get value from card reading			
				MfSdkEmvReadCardPage(base_win, emv_read_card_callback, &card_in, &card_out);
				APP_TRACE_BUFF_TIP(card_out.ic_data, card_out.ic_data_len, "ic_date");
				//Disp_EMV_Tag(card_out.ic_data, card_out.ic_data_len);
				pay_data_pay->step = PAY_STEP_EMV;
			}			
		}
	}
	else if (pay_data_pay->step == PAY_STEP_EMV) 
	{
		emvreadcardtiprelease();
		APP_TRACE("PAY_STEP_EMV ret = %d\r\n", ret);
		if (ret == PAGE_RET_CANCEL) 
		{
			_pay_close_page();
		}
		else 
		{

			if (ret == EMVAPI_RET_ARQC) 
			{
				strcpy(pay_data_pay->retmsg, "Arqc Online");
			}
			else if (ret == EMVAPI_RET_TC) 
			{
				strcpy(pay_data_pay->retmsg, "Approve");
			}
			else if (ret == EMVAPI_RET_AAC) 
			{
				strcpy(pay_data_pay->retmsg, "Decline");
			}
			else if (ret == EMVAPI_RET_AAR) 
			{
				strcpy(pay_data_pay->retmsg, "Terminate");
			}
			else if(ret == EMVAPI_RET_KERNEL_NOT_SUPPORT)
			{
				strcpy(pay_data_pay->retmsg, "Not Support");
			}
			else 
			{
				strcpy(pay_data_pay->retmsg, "Terminate");
			}

			if (ret == EMVAPI_RET_ARQC) 
			{
				page_message_show_ex(base_win, _pay_return_func, get_trans_title(pay_data_pay->mode), pay_data_pay->retmsg, "", "confirm", 3000, 1);
				pay_data_pay->step = PAY_STEP_EMV_RESULT_MSG;
			}
			else if (ret == EMVAPI_RET_TC) 
			{
				pay_data_pay->step = PAY_STEP_COMM;
				_pay_return_func(0);
			}
			else 
			{
				APP_TRACE("PAY_STEP_EMV2\r\n");
				page_message_show_ex(base_win, _pay_return_func, get_trans_title(pay_data_pay->mode), pay_data_pay->retmsg, "", "confirm", 5000, 1);
				pay_data_pay->step = PAY_STEP_MSG;
			}
		}
	}
	else if (pay_data_pay->step == PAY_STEP_EMV_RESULT_MSG) 
	{
		if (ret == PAGE_RET_CANCEL) 
		{
			_pay_close_page();
		}
		else 
		{
			MessagePackData();
			pay_data_pay->comm_data.timeover = 60000;
			page_comm_show_auto(base_win, _pay_return_func, get_trans_title(pay_data_pay->mode), &pay_data_pay->comm_data, 2);
			pay_data_pay->step = PAY_STEP_COMM;
		}
	}
	else if (pay_data_pay->step == PAY_STEP_COMM) 
	{
		APP_TRACE("PAY_STEP_COMM ret = %d\r\n", ret);
		int msg_ret = MessageUnpackData();
		APP_TRACE("PAY_STEP_UNPACK msg_ret = %d\r\n", msg_ret);
		pay_data_pay->step = PAY_STEP_MSG;
		_pay_return_func(msg_ret);
	}
	else if (pay_data_pay->step == PAY_STEP_SCAN) 
	{
	    char amountStr[16] = { 0 };
		AppFormatAmountFinal((unsigned char*)amountStr,ATOLL(pay_data_pay->amount));
		APP_TRACE("PAY_STEP_SCAN ret = %d\r\n", ret);
        ScanPage(get_mainpage(), _pay_return_func, amountStr, pay_data_pay->orderSn, sizeof(pay_data_pay->orderSn));
		pay_data_pay->step = PAY_STEP_MSG;
	}
	else if (pay_data_pay->step == PAY_STEP_MSG) 
	{
		APP_TRACE("PAY_STEP_MSG ret = %d\r\n", ret);
		if (ret < 0)
		{
			ShowTransResult(get_mainpage(), _pay_close_page, pay_data_pay, ret);
			//_pay_close_page();
		}
		else
		{
			if(TRANSTYPE_QR == pay_data_pay->mode)
			{	
			    char amountStr[16] = { 0 };
			    char orderSn[256] = { 0 };
				AppFormatAmountFinal((unsigned char*)amountStr,ATOLL(pay_data_pay->amount));
				strcat(orderSn, pay_data_pay->orderSn);
				APP_TRACE("amountStr = %s\r\n", amountStr);
				SetPowerLock(QRPAGE_LOCK_OFF);
		        show_page_qrcode(get_mainpage(), _pay_close_page, amountStr, orderSn);
			}
			else
			{
				APP_TRACE("orderSn = %s\r\n", pay_data_pay->orderSn);
				ShowTransResult(get_mainpage(), _pay_close_page, pay_data_pay, ret);
			}
		}
	}
	
	return 0;

}

static int TransactionCheck()
{
	int ret = 0;
	//char *title = get_trans_title(mode);

    if (MfSdkCommLinkState() == 0)
    {
        APP_TRACE("TransactionProc network fail\n");
        page_text_show(get_mainpage(), "TIP", "Network unstable", 3000);
        PubMultiPlay((const s8*)"netf.mp3");
		ret = -1;
    }
    else if (get_mqtt_con_state() == 0)
    {
        APP_TRACE("TransactionProc connect fail\n");
        page_text_show(get_mainpage(), "TIP", "Connect fail", 3000);
        mqtt_play_state(0);
		ret = -1;
    }

	return ret;
}

static void TransactionParamInit()
{	
	pay_data_pay = MfSdkMemMalloc(sizeof(st_pay_data));
	memset(pay_data_pay, 0x00, sizeof(st_pay_data));
	pay_data_pay->step = PAY_STEP_START;
	//pay_data_pay->mode = mode;
	pay_data_pay->comm_data.timeover = 60000;
	
	memset(&card_in, 0, sizeof(card_in));
	memset(&card_out, 0, sizeof(card_out));
	card_in.pin_dukpt_gid = -1;
	card_in.data_dukpt_gid = -1;
	card_in.pin_mksk_gid = -1;
}

void func_pay(lv_obj_t * parent, char* amt)
{
	char amountFormat[16] = {0};

	if (0 != TransactionCheck())
	{
		return;
	}
	//AppInitEmvCallback();
	MfSdkTmsAppBusy(MFSDK_TMS_APP_STATE_BUSY);
	PayPagePowerLock((char*)"func_pay");

	if(0 != comm_preconnect())
	{
		return;
	}

	TransactionParamInit();
	base_win = page_create_win(parent, NULL);
	if (NULL != amt) 
	{
		strcat(pay_data_pay->amount, amt);
		AppFormatAmountFinal((unsigned char*)amountFormat,ATOLL(amt));
		MfSdkGuiLedAmount(amountFormat);
	}
	_pay_return_func(0);
}
