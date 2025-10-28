#include "lvgl/lvgl.h"
#include <stdio.h>
#include "pages.h"
#include "page_main.h"
#include "lvgl/lvgl/src/lv_font/lv_font.h"


static lv_obj_t* page_win = NULL;
static lv_obj_t* lab_title = NULL;
static lv_obj_t* lab_tip = NULL;
static lv_obj_t* lab_buff = NULL;
static lv_obj_t* lab_add = NULL;
static lv_obj_t* btn_canel = NULL;
static lv_obj_t* btn_confirm = NULL;
static lv_task_t* task_input = NULL;
static lv_obj_t* input_body = NULL;
static lv_style_t style_amount_line;

static char* m_buff = NULL;
static char m_addbuff[32] = {0};
static char m_totalamt[16] = {0};
static long long lltotalamt = 0;
static int m_maxnum = 0;
static int m_minnum = 0;
static int m_mode = 0;
static int m_timeover = 0;
static int m_timeovr_reset = 0;
static int m_limit = 0;
static int g_nadd = 0;
static int g_buff_lenth = 0;

static page_close_page_func m_page_close_page_func = 0;


static const char* keybuff[11] = {
	"0","1","2abcABC","3defDEF","4ghiGHI",
	"5jklJKL","6mnoMNO","7pqrsPQRS","8tuvTUV","9wxyzWXYZ",
	".,'?!\"-*@/\\:_;+&%#$=()<>^"
};

static char _input_newchar(char oldchar)
{
	int i, j;
	int size;
	char ch = oldchar;
	for (i = 0; i < 11; i++) {
		size = strlen(keybuff[i]);
		for (j = 0; j < size; j++) {
			if (oldchar == keybuff[i][j]) {
				j = (j + 1) % size;
				ch = keybuff[i][j];
				return ch;
			}
		}
	}
	return oldchar;
}

static int GetAmontLine()
{
	
	return LINE_HEIGNT;
}

static int GetTotalAmontLine()
{
	int offset = 0;

	if (LCD_IS_320_480)
	{
		offset = 2*LINE_HEIGNT;
	}
	else
	{
		offset = 1*LINE_HEIGNT;
	}
	
	return GetAmontLine()-offset;
}

static void amountFormat(char *amountStr, char*amountFormat)
{
	AppFormatAmountFinal((unsigned char*)amountFormat,ATOLL(amountStr));
}

int GetKeyStatus(int key)
{
	return (key-MF_LV_KEY_QUIT_SHORT_PRESS)%4;
}

int GetKeyValue(int key)
{
	int keyvalue = 0;
	if( key >= MF_LV_KEY_0_SHORT_PRESS && key <= MF_LV_KEY_0_TRIPLE_PRESS) 
		keyvalue = '0';
	else if( key >= MF_LV_KEY_1_SHORT_PRESS && key <= MF_LV_KEY_9_TRIPLE_PRESS)
		keyvalue = (key-MF_LV_KEY_1_SHORT_PRESS)/4+'1';
	return keyvalue;
}

static int checkAmountLimit()
{
	int ret = 0;
	long long lltotalamt_temp = lltotalamt;
	char disp_temp[16]={0};
	
	lltotalamt_temp += ATOLL(m_buff);
	sprintf(disp_temp, "%lld", lltotalamt_temp);
	if(lltotalamt_temp <= 0)
	{
		return -1;
	}
	if(strlen(disp_temp)>m_maxnum)
	{
		m_limit = 1;//Exceed the limit
	    lv_label_set_text(lab_buff, "Exceed the limit");
		lv_obj_align(lab_buff, NULL, LV_ALIGN_CENTER, 0, GetAmontLine());
		snprintf(disp_temp, sizeof(disp_temp), "%.*s", m_maxnum, "-----------");
		MfSdkGuiLedAmount(disp_temp);
		PubMultiPlay((const s8 *)"exdlimit.mp3");
		ret = -1;
	}
	return ret;
}

static void _input_close_page(int ret)
{
	//AppPowerUnlockApp((char*)"input");
	if (task_input != 0) 
	{
		lv_task_del(task_input);
		task_input = 0;
	}
	if (page_win != 0) 
	{
		if(ret == PAGE_RET_CONFIRM||ret == PAGE_RET_F1||ret == PAGE_RET_F2)
		{
			lltotalamt+=ATOLL(m_buff);
			sprintf(m_totalamt, "%lld", lltotalamt);
		}
		if (m_buff != 0)
		{
			memset(m_buff, 0, g_buff_lenth);
			strcpy(m_buff, m_totalamt);
		}
		if(lab_title != 0)
		{
			lv_obj_del(lab_title);
			lab_title = 0;
		}
		if(lab_tip != 0)
		{
			lv_obj_del(lab_tip);
			lab_tip = 0;
		}
		if(lab_buff != 0)
		{
			lv_obj_del(lab_buff);
			lab_buff = 0;
		}
		if(lab_add != 0)
		{
			lv_obj_del(lab_add);
			lab_add = 0;
		}
		lv_obj_del(page_win);
		page_win = 0;
		m_buff = NULL;

		if (m_page_close_page_func != 0) m_page_close_page_func(ret,input_body);
		m_page_close_page_func = 0;
	}
	
}

static void _input_btn_event_cb(struct _lv_obj_t* obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) 
	{
		if (obj == btn_canel) 
		{
            _input_close_page(PAGE_RET_CANCEL);
		}
		else if (obj == btn_confirm) 
		{
            _input_close_page(PAGE_RET_CONFIRM);
		}
	}
}

static void _input_set_text()
{
    char buff[256] = { 0 };
    int size;

    if (m_mode == PAGE_INPUT_MODE_PWD) 
	{
		size = strlen(m_buff);
		if (size > 0) memset(buff, '*', size);
		m_buff[size] = 0;
		lv_label_set_text(lab_buff, buff);
    }
    else 
	{
		if (strlen(m_addbuff) > 0)
		{
			lv_label_set_text(lab_add, m_addbuff);
			lv_obj_align(lab_add, NULL, LV_ALIGN_CENTER, 0, GetTotalAmontLine());
		}
		amountFormat(m_buff, buff);
		
        lv_label_set_text(lab_buff, buff);
		lv_obj_align(lab_buff, NULL, LV_ALIGN_CENTER, 0, GetAmontLine());
    }
    
}

static void showAmountLed(char *amtstr)
{
	char amountDisp[16] = {0};
	if(NULL == amtstr) return;
	memset(amountDisp, 0, sizeof(amountDisp));
	AppFormatAmountFinal((unsigned char*)amountDisp,ATOLL(amtstr));
	APP_TRACE("amountDisp = %s\r\n", amountDisp);
	MfSdkGuiLedAmount((char*)amountDisp);
}

static void AmtStringRemoveHighZero(char* str, int lenth) 
{
    long long amt = atoi(str);
	memset(str, 0, lenth);
	sprintf(str, "%lld", amt);
}

static void _input_win_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t key;
    int size;

    if (e == LV_EVENT_KEY) 
	{
		ClearAmountTimer();
		m_timeover = m_timeovr_reset;
        key = page_get_key();
        size = strlen(m_buff);

		APP_TRACE("m_buff = %s\r\n", m_buff);
		APP_TRACE("m_maxnum = %d\r\n", m_maxnum);
		APP_TRACE("size = %d\r\n", size);
		APP_TRACE("key = %x\r\n", key);
        if (key >= MF_LV_KEY_1_SHORT_PRESS && key <= MF_LV_KEY_0_TRIPLE_PRESS)
		{
			int keyValue = GetKeyValue(key);
			APP_TRACE("keyValue == %d\r\n", keyValue);
            if (size < m_maxnum && 0 == GetKeyStatus(key) && 0 == m_limit) 
			{
				if(0 == g_nadd)
				{
               		sprintf(m_buff, "%s%c", m_buff, keyValue);
				}
				else
				{
					sprintf(m_buff, "%c", keyValue);
				}
				AmtStringRemoveHighZero(m_buff, strlen(m_buff));
				if(strlen(m_totalamt)>0 && ATOLL(m_totalamt)>0)
				{
					unsigned char buffer1[32] = {0};
					unsigned char buffer2[32] = {0};
					AppFormatAmountFinal(buffer1,ATOLL(m_totalamt));
					AppFormatAmountFinal(buffer2,ATOLL(m_buff));					
					sprintf(m_addbuff, "%s+%s", buffer1, buffer2);
				}
                _input_set_text();
				showAmountLed(m_buff);
            }
        } 
        else if (key == MF_LV_KEY_ADD_SHORT_PRESS)
		{
			if(0 == checkAmountLimit())
			{
				char disp[16]={0};

				g_nadd = 0;
				lltotalamt += ATOLL(m_buff);
				memset(m_buff, 0, g_buff_lenth);
				APP_TRACE("lltotalamt = %lld\r\n", lltotalamt);
				sprintf(m_totalamt, "%lld", lltotalamt);

				unsigned char buffer1[32] = {0};
				AppFormatAmountFinal(buffer1,lltotalamt);
				sprintf(m_addbuff, "%s+", buffer1);

	            amountFormat(m_totalamt, disp);
				lv_label_set_text(lab_add, m_addbuff);
				lv_obj_align(lab_add, NULL, LV_ALIGN_CENTER, 0, GetTotalAmontLine());
			    lv_label_set_text(lab_buff, disp);
				MfSdkGuiLedAmount((char*)disp);
			}
        }
		else if (key == MF_LV_KEY_BACKSPACE_SHORT_PRESS) 
		{
			if (size > 0) 
			{
				if(1 == m_limit)
					m_limit = 0;
				m_buff[size - 1] = 0;
				sprintf(m_totalamt, "%lld", lltotalamt);
				if(strlen(m_totalamt)>0 && ATOLL(m_totalamt)>0)
				{
					unsigned char buffer1[32] = {0};
					unsigned char buffer2[32] = {0};
					AppFormatAmountFinal(buffer1,ATOLL(m_totalamt));
					AppFormatAmountFinal(buffer2,ATOLL(m_buff));					
					sprintf(m_addbuff, "%s+%s", buffer1,buffer2);
				}
				_input_set_text();
				showAmountLed(m_buff);
			}
		}
		else if (key == MF_LV_KEY_F1_SHORT_PRESS) 
		{//scan
			if (m_mode == PAGE_INPUT_MODE_ALPHABET || m_mode == PAGE_INPUT_MODE_IP/* || m_mode == PAGE_INPUT_MODE_AMOUNT*/)
			{
				if (size < m_maxnum) 
				{
					sprintf(m_buff, "%s%c", m_buff, '.');
					_input_set_text();
				}
			}
			else
			{
				if(size >= m_minnum && 0 == checkAmountLimit())
		            _input_close_page(PAGE_RET_F1);
			}
		}
		else if (key == MF_LV_KEY_F2_SHORT_PRESS) 
		{//rf
			if(m_mode == PAGE_INPUT_MODE_ALPHABET)
			{
				if (size > 0) 
				{
					m_buff[size - 1] = _input_newchar(m_buff[size - 1]);
					_input_set_text();
				}
			}
			else if(m_mode == PAGE_INPUT_MODE_PWD)
			{
	            if (size < m_maxnum) 
				{
	                sprintf(m_buff, "%s%c", m_buff, '#');
	                _input_set_text();
	            }
			}
			else
			{
				if(size >= m_minnum && 0 == checkAmountLimit())
		            _input_close_page(PAGE_RET_F2);
			}
		}
		else if (key == MF_LV_KEY_OK_SHORT_PRESS) 
		{//qrcode
			if(size >= m_minnum && 0 == checkAmountLimit())
	            _input_close_page(PAGE_RET_CONFIRM);
		}
        else if (key == MF_LV_KEY_CANCEL_SHORT_PRESS)
		{
			APP_TRACE("PAGE_RET_CANCEL\r\n");
            _input_close_page(PAGE_RET_CANCEL);
        }
    }
}

static void _input_task_func(lv_task_t* task)
{
	if (m_timeover > 0) 
	{
		m_timeover -= 1000;
		if (m_timeover <= 0) 
		{
			_input_close_page(PAGE_RET_TIMEOVR);
		}
	}
}
static void back_navigation_event_cb(lv_obj_t* btn, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) 
	{        		
		if(!btn) return ;
		APP_TRACE("back_navigation_event_cb obj:%p",btn);
		 _input_close_page(PAGE_RET_CANCEL);
    }
}
lv_obj_t* page_input_show_ex(lv_obj_t* parent , void* pfunc, char * title, char * buff, int minnum, int maxnum, int mode, int timeover,int show_back)
{
	int offset = 0;
	lv_font_t* font = LV_FONT_24;
	char amtini[20]={0};
	m_timeover = timeover;
	m_timeovr_reset = timeover;
	input_body = parent;

    m_buff = buff;
	g_buff_lenth = sizeof(buff);
    m_maxnum = maxnum;
	m_minnum = minnum;
    m_mode = mode;
    m_page_close_page_func = (page_close_page_func)pfunc;
	lltotalamt = 0;
	m_limit = 0;
	memset(m_totalamt, 0, sizeof(m_totalamt));
	memset(m_addbuff, 0, sizeof(m_addbuff));

	//AppPowerLockApp((char*)"input");
	page_win = page_create_win(parent, _input_win_event_cb);
    lab_title = page_create_title(page_win, title);

	if (LCD_IS_320_480)
	{
		offset = 1;
		font = LV_FONT_32;
	}

	//Pls Input Amount
	page_ShowTextOut(page_win, "Pls Input Amount", LV_ALIGN_IN_TOP_MID, 0, (3+offset)*LINE_HEIGNT, APP_COLOR_FONT_TIP, LV_FONT_24);
	//total amount
	lab_add = page_ShowTextOut(page_win, "", LV_ALIGN_CENTER, 0, GetTotalAmontLine(), LV_COLOR_BLACK, LV_FONT_24);
	//input amount
	AppFormatAmountFinal((unsigned char*)amtini,ATOLL(m_buff));

	lab_buff = page_ShowTextOut(page_win, amtini, LV_ALIGN_CENTER, 0, GetAmontLine(), LV_COLOR_RED, font);

	//underline
	#if 0
	lv_obj_t* underLine = lv_obj_create(page_win, NULL);
	lv_obj_set_size(underLine, lv_obj_get_width(page_win)-20, 2);
	lv_obj_set_style_local_bg_color(underLine, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, APP_COLOR_FONT_TIP);
	lv_obj_set_style_local_border_color(underLine, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, APP_COLOR_FONT_TIP);
	lv_obj_align(underLine, page_win, LV_ALIGN_CENTER, 0, GetAmontLine()+LINE_HEIGNT/2);
	#endif
	
	//tip
	page_ShowTextOut(page_win, "F1 to scan", LV_ALIGN_IN_BOTTOM_MID, 0, -(offset+2)*LINE_HEIGNT, LV_COLOR_BLACK, LV_FONT_24);
	page_ShowTextOut(page_win, "F2 to TAP", LV_ALIGN_IN_BOTTOM_MID, 0, -(offset+1)*LINE_HEIGNT, LV_COLOR_BLACK, LV_FONT_24);
	page_ShowTextOut(page_win, "OK to show QR", LV_ALIGN_IN_BOTTOM_MID, 0, -(offset)*LINE_HEIGNT, LV_COLOR_BLACK, LV_FONT_24);

	task_input = lv_task_create(_input_task_func, 1000, LV_TASK_PRIO_MID, 0);

    return page_win;
}
lv_obj_t* page_input_show(lv_obj_t* parent , void* pfunc, char * title, char * buff, int maxnum, int mode, int timeover)
{
	return page_input_show_ex(parent,pfunc,title,buff,0,maxnum,mode,timeover,0);
}
lv_obj_t* page_input_show_fix(lv_obj_t* parent , void* pfunc, char * title, char * buff, int lenth, int mode, int timeover, int show_back)
{
	return page_input_show_ex(parent,pfunc,title,buff,lenth,lenth,mode,timeover,show_back);
}
