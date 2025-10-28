#include "pages/pages.h"
#include "page_pub.h"
#include "EntryPoint/lib_emvpub/inc/emv_interface.h"
#include "libapi_xpos/inc/emvapi.h"
//#include "page_input_psw.h"
#include "./card/func_pay.h"
#include "libapi_xpos/inc/mfsdk_icc.h"
#include "libapi_xpos/inc/mfsdk_nfc.h"
#include "libapi_xpos/inc/mfsdk_emv.h"
#include "./tracedef.h"
#include "./page_main.h"
#include "sdk_tts.h"

#define MSG_SELECT_APP	MFSDK_EMV_MSG_SELECT_APP
#define MSG_ONLINE_PIN	MFSDK_EMV_MSG_ONLINE_PIN
#define MSG_OFFLINE_PIN	MFSDK_EMV_MSG_OFFLINE_PIN
#define MSG_OFFLINE_PIN_RETRY_COUNT	MFSDK_EMV_MSG_OFFLINE_PIN_RETRY_COUNT
#define MSG_DISPLAY_CARD_NO	MFSDK_EMV_MSG_DISPLAY_CARD_NO
#define MSG_SET_PUREAMT_BEFOREGPO	MFSDK_EMV_MSG_SET_PUREAMT_BEFOREGPO

#if 0
static MFSDKBOOL g_bOnlinePin = MFSDK_FALSE;

void AppOnlinePinInit(void)
{
	g_bOnlinePin = MFSDK_FALSE;
}
MFSDKBOOL AppOnlinePinGetFlag(void)
{
	return g_bOnlinePin;
}

void AppOnlinePinSetFlag(MFSDKBOOL onlinePin)
{
	g_bOnlinePin = onlinePin;
}

static int input_pin_ret = 999;
static int input_pin_return_func(int ret, lv_obj_t * obj)
{
    APP_TRACE("input_pin_return_func:%d", ret);
    input_pin_ret = ret;
	return input_pin_ret;
}
static int AppInputOnlinePinEx(char* psCardNo, char* psAmt, char* psCiphertext)
{
    char sPin[12 + 1] = { 0 };
    int nPinLen = 0;
    int i = 0;
	lv_obj_t* emv_page_win = NULL;
	APP_TRACE("AppInputOnlinePinEx\r\n");

    //return SUCC;
    for (i = 0; i < 50; i++)
    {
        if (psCardNo[i] == 'F' || psCardNo[i] == 'f') { psCardNo[i] = 0; }

        if (psCardNo[i] == 0) { break; }
    }
	
	emv_page_win = app_emv_get_page_win();
	if(NULL == emv_page_win)
		emv_page_win = emv_get_page_win();

    input_pin_ret = 999;
    //page_input_show_ex(emv_get_page_win(), input_pin_return_func, "input PIN:(4-12)", sPin, 4, 12, PAGE_INPUT_MODE_PWD, 30000, 1);
	LV_THREAD_LOCK(page_input_show_psw(emv_page_win, input_pin_return_func, get_card_title(), "input PIN:(4-12)", sPin, 4, 12, PAGE_INPUT_MODE_PWD, 60000));

    while (input_pin_ret == 999)
    {
        MfSdkSysSleep(50);
    }
	APP_TRACE("AppInputOnlinePinEx input_pin_ret:%d\r\n",input_pin_ret);
    if (PAGE_RET_CANCEL == input_pin_ret)    /*INPUT_NUM_RET_QUIT*/
    {
        APP_TRACE("canceled\r\n");
        PubMultiPlay((const s8*)TTS_VOLUME_BEEP);
        LV_THREAD_LOCK(page_text_show(get_subpage(), get_card_title(), "Transaction cancel", 3000));
        return -7;
    }
    else if (PAGE_RET_TIMEOVR == input_pin_ret)    /*INPUT_NUM_RET_TIMEOVER*/
    {
        APP_TRACE("input timeover\r\n");
        PubMultiPlay((const s8*)TTS_VOLUME_BEEP);
        LV_THREAD_LOCK(page_text_show(get_subpage(), get_card_title(), "Transaction cancel", 3000));
        return -4;
    }
    nPinLen = input_pin_ret;
	APP_TRACE("AppInputOnlinePinEx nPinLen:%d\r\n",nPinLen);
    if (nPinLen == -18)
    {
        return -18; // bypass
    }
    else if (nPinLen < 0)
    {
        return nPinLen;
    }
	if(strlen(sPin) > 0) { strcpy(psCiphertext,sPin); }
	else { return -18; }
	
	AppOnlinePinSetFlag(MFSDK_TRUE);
    return 0;
}

static int AppInputPinEx(char* psCardNo, char* psAmt, char cMsgType, char* psPin) /*输入密码界面函数指针*/
{
#if 1
    char *msg = 0;
    int ret = 0;
    char password[16] = {0};

    memset(password, 0, sizeof(password));
    APP_TRACE("s_InputPin msg = %s\r\n", msg);

    //get_samt(msg, line2);
    //ret = input_password_page_contact("sale", line2 , password , 12,  90000, cMsgType);
    if(cMsgType == OFFLINE_PIN_AGAIN)
    {
        msg = "input offline PIN l";
    }
    else if(cMsgType == OFFLINE_PIN_LAST)
    {
        msg = "input offline PIN 2";
    }
    else if(cMsgType == OFFLINE_PIN_NOMAL)
    {
        msg = "input offline PIN";
    }
    else
    {
        msg = "input Online PIN:";
    }
    lv_start_lock(1);

    if(emv_get_page_win())
    {
        //page_message_show(emv_get_page_win(), disp_card_return_func, "Sale", msg, "cancel", "confirm", OK_TIME_OVER);
        //comm_page_input_show_ex(emv_get_page_win(), input_pin_return_func, "Sale", msg, password, 4, 12, PAGE_INPUT_MODE_PWD, OK_TIME_OVER, 1);
        page_input_show_ex(emv_get_page_win(), input_pin_return_func, "Sale", password, 4, 12, PAGE_INPUT_MODE_PWD, OK_TIME_OVER, 0);
    }
    else
    {
        APP_TRACE("emv_get_page_win error is null");
        input_pin_ret = PAGE_RET_CONFIRM;
    }
    lv_start_lock(0);

    while (input_pin_ret == 999)
    {
        MfSdkSysSleep(100);
    }

    ret = input_pin_ret;

    if (-1 == ret)  /*INPUT_NUM_RET_QUIT*/
    {
        APP_TRACE("canceled\r\n");
        //gui_warning_show(emv_get_page_win(), 0, "sale", "P:data/warning_bg.png", "canceled", "comfirm", 10000);
        //page_proc_msg_show("sale","canceled" , 10000 , 1 , 1);
        return -7;
    }
    else if (-2 == ret)  /*INPUT_NUM_RET_TIMEOVER*/
    {
        APP_TRACE("input timeover\r\n");
        //gui_warning_show(emv_get_page_win(), 0, "sale", "P:data/warning_bg.png", "input timeover", "comfirm", 10000);
        //page_proc_msg_show("sale","input timeover" , 10000 , 1 , 1);
        return -4;
    }

    if (strlen(password) > 0)
    {
        //readCardTip();
        memcpy(psPin, password, strlen(password));
        return strlen(password);
    }
    else        //不输pin
    {
        return SUCC;
    }
#else
    return SUCC;

#endif
}

int AppInputPin(char* psCardNo, char* psAmt, char cMsgType, char* psPin)        /*输入密码界面函数指针*/
{
    char *msg = 0;
    int ret = 0;
    char password[16] = {0};

    memset(password, 0, sizeof(password));
    APP_TRACE("s_InputPin msg = %s\r\n", msg);
    if(cMsgType == OFFLINE_PIN_AGAIN)
    {
        msg = "input offline PIN l";
    }
    else if(cMsgType == OFFLINE_PIN_LAST)
    {
        msg = "input offline PIN 2";
    }
    else if(cMsgType == OFFLINE_PIN_NOMAL)
    {
        msg = "input offline PIN";
    }
    else
    {
        msg = "input Online PIN:";
    }
    lv_start_lock(1);

    if(emv_get_page_win())
    {
        page_input_show_ex(emv_get_page_win(), input_pin_return_func, "Sale", password, 4, 12, PAGE_INPUT_MODE_PWD, OK_TIME_OVER, 0);
    }
    else
    {
        APP_TRACE("emv_get_page_win error is null");
        input_pin_ret = PAGE_RET_CONFIRM;
    }
    lv_start_lock(0);

    while (input_pin_ret == 999)
    {
        MfSdkSysSleep(100);
    }

    ret = input_pin_ret;

    if (-1 == ret)  /*INPUT_NUM_RET_QUIT*/
    {
        APP_TRACE("canceled\r\n");
        return -7;
    }
    else if (-2 == ret)  /*INPUT_NUM_RET_TIMEOVER*/
    {
        APP_TRACE("input timeover\r\n");
        return -4;
    }

    if (strlen(password) > 0)
    {
        memcpy(psPin, password, strlen(password));
        return strlen(password);
    }
    else        //不输pin
    {
        return SUCC;
    }
}
int AppInputOnlinePin(char* psCardNo, char* psAmt,char* psPin)        /*输入密码界面函数指针*/
{
#if 0 /*Modify by CHAR at 2023.12.25  17:15 */
    char *msg = 0;
    int ret = 0;
    char password[16] = {0};

    memset(password, 0, sizeof(password));
    APP_TRACE("s_InputPin msg = %s\r\n", msg);
    msg = "input Online PIN:";
    lv_start_lock(1);

    if(emv_get_page_win())
    {
        page_input_show_ex(emv_get_page_win(), input_pin_return_func, "Sale", password, 4, 12, PAGE_INPUT_MODE_PWD, OK_TIME_OVER, 0);
    }
    else
    {
        APP_TRACE("emv_get_page_win error is null");
        input_pin_ret = PAGE_RET_CONFIRM;
    }
    lv_start_lock(0);

    while (input_pin_ret == 999)
    {
        MfSdkSysSleep(100);
    }

    ret = input_pin_ret;

    if (-1 == ret)  /*INPUT_NUM_RET_QUIT*/
    {
        APP_TRACE("canceled\r\n");
        return -7;
    }
    else if (-2 == ret)  /*INPUT_NUM_RET_TIMEOVER*/
    {
        APP_TRACE("input timeover\r\n");
        return -4;
    }

    if (strlen(password) > 0)
    {
        memcpy(psPin, password, strlen(password));
//        return strlen(password);
    }
//    else        //不输pin
//    {
        return SUCC;
//    }
#else
	AppOnlinePinSetFlag(MFSDK_TRUE);
	APP_TRACE("AppInputOnlinePin:%d\r\n",AppOnlinePinGetFlag());

	memcpy(psPin, "888888", 6);
	return SUCC;
#endif /* if 0 */
}
static int disp_card_ret = 999;

static int disp_card_return_func(int ret, lv_obj_t* obj)
{
    APP_TRACE("disp_card_return_func:%d", ret);
    disp_card_ret = ret;
	if(ret != PAGE_RET_CONFIRM)
	{
		if(GetEmvPageWinTip())
			lv_obj_set_hidden(GetEmvPageWinTip(), 1);
		//if(MfSdkEmvGetPageWinTip())
		//	lv_obj_set_hidden(MfSdkEmvGetPageWinTip(), 1);
	}
    return disp_card_ret;
}

int AppEmvDispCard(char* sDispPan)
{
#if 1
    char msg[64] = { 0 };
    int ret;
    int get_show_pan = 1;
	lv_obj_t* emv_page_win;
    APP_TRACE("s_DispCard>>>>\r\n");
    //flag_lock = 1;

    memset(msg, 0x00, sizeof(msg));

    if (get_show_pan == 1)
    {
        sprintf(msg, "%s", sDispPan);
        APP_TRACE("msg = %s\r\n", msg);
    }
    else if (get_show_pan == 2)
    {
        char acPan[19 + 1] = { 0 };
        memset(acPan, 0x00, sizeof(acPan));

        memcpy(acPan, sDispPan, strlen(sDispPan));
        memset(acPan + 6, '*', strlen(sDispPan) - 10);

        sprintf(msg, "%s", acPan);
    }
    else
    {
        return -7;
    }
    //ret = xgui_messagebox_show("PLS confirm", msg, "cancel", "confirm", OP_TIME_OVER);
    //EMV_iReadingCardDisp(1);

    disp_card_ret = 999;

    lv_start_lock(1);
	emv_page_win = app_emv_get_page_win();
    APP_TRACE("app_emv_get_page_win = %p >>>>\r\n", emv_page_win);
	if(NULL == emv_page_win)
	{
		emv_page_win = emv_get_page_win();
    	APP_TRACE("emv_get_page_win = %p >>>>\r\n", emv_page_win);
	}

    if(emv_page_win)
    {
        APP_TRACE("disp_card = %s\r\n", msg);
        page_message_show_ex(emv_page_win, disp_card_return_func, get_card_title(), msg, 0, "confirm", OK_TIME_OVER, 3);
        //page_message_img_show(emv_get_page_win(), CARDPNG_320X480, disp_card_return_func, get_card_title(), msg, 0, "confirm", OK_TIME_OVER, 1);
    }
    else
    {
        APP_TRACE("emv_get_page_win error is null");
        disp_card_ret = PAGE_RET_CONFIRM;
    }
    lv_start_lock(0);

    APP_TRACE("s_DispCard end %d\r\n", disp_card_ret);

    while (disp_card_ret == 999)
    {
//		osl_Sleep(100);
        MfSdkSysSleep(100);
    }

    APP_TRACE("disp_card_ret = %d\r\n", disp_card_ret);

    ret = disp_card_ret;

    if (ret == PAGE_RET_CONFIRM)
    {
        return 0;
    }
    else
    {
        return -7;
    }
#else
    memset(cardnum, 0, sizeof(cardnum));
    strcat(cardnum, sDispPan);
    APP_TRACE("sDispPan = %s>>>>\r\n", sDispPan);
#endif
    return 0;
}
static int g_appEmvSelectAppRet = 999;

static int AppEMvSelectAppCallback(int ret, lv_obj_t* obj)
{
    APP_TRACE("AppEMvSelectAppCallback:%d", ret);
    g_appEmvSelectAppRet = ret;
    return g_appEmvSelectAppRet;
}

/**
 * @brief
 * 
 * @param[in] char *listitem[]
 * @param[in] s32 count
 * @param[in] s32 timeOver
 * @return 
 */
static s32 AppEMvSelectApp(char *listitem[], s32 count , s32 timeOver)
{
	int iRet = -7;
	lv_obj_t* emv_page_win = NULL;
	
	g_appEmvSelectAppRet = 999;
	
	lv_start_lock(1);
	emv_page_win = app_emv_get_page_win();
	if(NULL == emv_page_win)
		emv_page_win = MfSdkEmvGetPageWin();

	page_list_show_ex(emv_page_win,AppEMvSelectAppCallback,get_card_title(),listitem,count,0,timeOver,0);
	lv_start_lock(0);

	APP_TRACE("g_appEmvSelectAppRet end %d\r\n", g_appEmvSelectAppRet);

	while(g_appEmvSelectAppRet == 999)
	{
		MfSdkSysSleep(100);
	}
	
	if(PAGE_RET_TIMEOVR== g_appEmvSelectAppRet) { iRet = -17; }
	else if(CARD_RET_CANCEL == g_appEmvSelectAppRet) { iRet = -7; }
	else if(g_appEmvSelectAppRet < 0) { iRet = -7; }
	else { iRet = g_appEmvSelectAppRet; }

	APP_TRACE("AppEMvSelectApp iRet = %d\r\n", iRet);

	return iRet;
}

/**
 * @brief
 * 
 * @param[in] u8* Indata
 * @param[in] s32 InLen
 * @param[out] u8* pOutData
 * @param[in] s32 iOutDataLength
 * @return > 0 amount length
 * @return 0
 */
static s32 AppEmvGetAmount(u8* Indata,s32 InLen , u8* pOutData , s32 iOutDataLength)
{
	s32 length = 0;
	u8 temp[32] = {0};

	memset(temp ,0 ,sizeof(temp));
	length = MfSdkUtilTlvGetDataByTag((u8*)"\x9F\x02",Indata,InLen,temp, sizeof(temp));
	
	if(length <= 0 || length != 6){ return 0; }

	if(iOutDataLength < 2*length) { return 0; }

	MfSdkUtilBcd2Asc((s8*)temp,(s8*)pOutData, length*2);

	return length*2;
	
}
/**
 * @brief
 * 
 * @param[in] u8* Indata
 * @param[in] s32 InLen
 * @param[out] u8* pOutData
 * @param[in] s32 iOutDataLength
 * @return > 0 card No. length
 * @return 0
 */

static s32 AppEmvGetCardNo(u8* Indata,s32 InLen , u8* pOutData , s32 iOutDataLength)
{
	s32 i = 0;
	s32 length = 0;
	u8 temp[32] = {0};

	MFSDK_COND_RET(Indata == NULL || InLen <= 0 || pOutData == NULL, 0);
	
	memset(temp ,0 ,sizeof(temp));
	length = MfSdkUtilTlvGetDataByTag((u8*)"\x5A",Indata,InLen,temp,sizeof(temp));
	
	if(length <= 0){ return 0; }
	
	if(iOutDataLength < 2*length) { return 0; }

	MfSdkUtilBcd2Asc((s8*)temp,(s8*)pOutData,length*2);

	for(i = 0; i < length*2 ; i++)
	{
		if(pOutData[i] == 'F' || pOutData[i] == 'f')
		{
			pOutData[i] = 0;
			break;
		}
	}

	return i;
}

#include "AppPub/inc/struct_tlv.h"
#define MAX_AID_NAME_LENGTH 16
typedef struct CandidateAids
{
	u8 value[MAX_AID_NAME_LENGTH*2+1];
	u8 length;
}AppCandidateAids_T;

int AppAidsParse(u8* Indata,s32 InLen , AppCandidateAids_T *aids, int size)
{
	int num = 0;
	int i61Num = 0;
	int iCandidate = 0;
	pTlvData pTlv = NULL;
	pTlvData ptlv61 = NULL;
		
	Ex_TLV_Unpack_Data_EX(&pTlv,Indata,InLen,&num);
	APP_TRACE("num = %d\r\n",num);
	for(int i = 0 ; i < num ; i++)
	{
		APP_TRACE("pTlv[%d].tag.data[0]:0x%02x\r\n",i,pTlv[i].tag.data[0]);
		if( pTlv[i].tag.nLen == 1 && pTlv[i].tag.data[0] == 0x61 )
		{
			i61Num = 0;
			Ex_TLV_Unpack_Data(&ptlv61, pTlv[i].val.data,pTlv[i].val.nLen,&i61Num);
			APP_TRACE("i61Num = %d\r\n",i61Num);
			for(int j = 0 ; j < i61Num && iCandidate < size; j++)
			{
				if( ptlv61[j].tag.nLen == 1 && ptlv61[j].tag.data[0] == 0x4f )
				{
					APP_TRACE_BUFF_TIP(ptlv61[j].val.data,ptlv61[j].val.nLen,"AID");
					int ilen = ptlv61[j].val.nLen > MAX_AID_NAME_LENGTH ? MAX_AID_NAME_LENGTH : ptlv61[j].val.nLen;				
					MfSdkUtilBcd2Asc((s8*)ptlv61[j].val.data,(s8*)aids[iCandidate].value,ilen*2);
					aids[iCandidate].length = ilen*2;
					iCandidate++;
					break;
				}
			}
			MfSdkMemFree(ptlv61);
			ptlv61 = NULL;
		}
	}
	MfSdkMemFree(pTlv);
	APP_TRACE("iCandidate:%d\r\n",iCandidate);
	return iCandidate;
}
#endif
/**
 * @brief
 * 
 * @param[] s32 MsgType
 * @param[] u8* Indata
 * @param[] s32 InLen
 * @param[] u8* OutData
 * @param[] s32* Outlen
 * @return 
 */
s32 AppEmvFuncCallback(s32 MsgType,u8* Indata,s32 InLen,u8* OutData,s32* Outlen)
{
	s32 iRet = -7; //-7 默认是取消 CHAR 2023.12.28 9:30
	u8 cardNo[32 + 1] = {0};
	u8 amount[12+1] = {0};

	APP_TRACE("AppEmvFuncCallback MsgType:%d\r\n",MsgType);
	
	switch (MsgType)
	{
		#if 0
		case MSG_SELECT_APP: 
			APP_TRACE_BUFF_TIP(Indata,InLen,"AIDSINFO");
			AppCandidateAids_T aids[16];
			char *listitem[17];
			memset(aids , 0, sizeof(aids));
			for(int i = 0 ; i < sizeof(listitem)/sizeof(listitem[0]); i++)
			{
				listitem[i] = NULL;
			}
			int num = AppAidsParse(Indata,InLen,aids,sizeof(aids)/sizeof(aids[0]));

			for(int i = 0 ; i < num; i++) { listitem[i] = (char *)aids[i].value; }
		
			iRet = AppEMvSelectApp(listitem,num,60*1000);
			if(iRet >= 0) { *OutData = (u8)(iRet+1); }	
			iRet = (iRet >= 0) ? 0 : iRet;
			break;
			
		case MSG_ONLINE_PIN: 
			LV_THREAD_LOCK(card_close_subpage());
			AppEmvGetAmount(Indata,InLen,amount,sizeof(amount)-1);
			AppEmvGetCardNo(Indata,InLen,cardNo,sizeof(cardNo)-1);

			APP_TRACE("amount:%s\r\n",amount);
			APP_TRACE("cardNo:%s\r\n",cardNo);
		
			iRet = AppInputOnlinePinEx((char*)cardNo,(char*)amount,(char*)OutData);

			APP_TRACE("AppInputOnlinePinEx iRet:%d\r\n",iRet);
			APP_TRACE("AppInputOnlinePinEx strlen(OutData):%d\r\n",strlen((char*)OutData));
			
			if(iRet == 0 && Outlen != NULL) { *Outlen = strlen((char*)OutData); }
			
			break;
			
		case MSG_OFFLINE_PIN: 
			AppEmvGetAmount(Indata,InLen,amount,sizeof(amount)-1);
			AppEmvGetCardNo(Indata,InLen,cardNo,sizeof(cardNo)-1);
		
			iRet = AppInputPinEx((char*)cardNo,(char*)amount,0,(char*)OutData);
			APP_TRACE("AppInputPinEx iRet:%d\r\n",iRet);
			APP_TRACE("AppInputPinEx strlen(OutData):%d\r\n",strlen((char*)OutData));
			
			if(iRet > 0 && Outlen != NULL) { *Outlen = strlen((char*)OutData); }
			
			iRet = iRet > 0 ? 0 : iRet;			
			break;
		
		case MSG_OFFLINE_PIN_RETRY_COUNT: 
			if(InLen == 1)
			{ 
				APP_TRACE("MSG_OFFLINE_PIN_RETRY_COUNT Indata[0]:%d\r\n",Indata[0]);
			}
			iRet = 0;
			break;
			
		case MSG_DISPLAY_CARD_NO: 
			APP_TRACE("MFSDK_EMV_MSG_DISPLAY_CARD_NO cardNo:%s\r\n",cardNo);
			iRet = AppEmvGetCardNo(Indata,InLen,cardNo,sizeof(cardNo)-1);
			if(iRet > 0) { iRet = AppEmvDispCard((char*)cardNo); }			
			break;
		case MSG_SET_PUREAMT_BEFOREGPO: 
			APP_TRACE("EMV_INS_SET_PUREAMT_BEFOREGPO\r\n");
			* Outlen = 9;
			memcpy(OutData, "\x9F\x02\x06\x00\x00\x00\x00\x00\x01", 9);
			break;
		#endif

		default: break;
	}

	APP_TRACE("AppEmvFuncCallback iRet:%d\r\n",iRet);
	return iRet;
}
void AppInitEmvCallback(void)
{
	MfSdkEmvSetCallBackFunction(AppEmvFuncCallback);
}


