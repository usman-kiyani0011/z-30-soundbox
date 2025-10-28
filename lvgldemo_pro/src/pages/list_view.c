#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lvgl/lvgl.h"
#include "libapi_xpos/inc/mfsdk_mem.h"
#include "pages/pages.h"
#include "list_view.h"
#include "tracedef.h"
#include "lvgl/lvgl.h"

#define PAGE_MAX_BTN 5 

static lv_style_t s_StyleCont;
static lv_obj_t *g_page_list = NULL;
static lv_obj_t *g_page_list_win = NULL;
static lv_task_t *task_timeover = NULL;

static DetailList_t *s_pDetList = NULL;

static int s_DetListItem = 0;
static int s_curPage = 0;
static int s_totalPage = 0;
static int s_itemsOffset = 0;
static int s_curItmesNum = 0;
static int m_timeover = 0;

static ListViewDestroyCb pFunDestory = NULL;


static void ListViewContainerFree(void)
{
    lv_style_reset(&s_StyleCont);
}

static void ListViewInit(void)
{
	m_timeover = 0;
	s_totalPage = 0;
	s_curPage = 0;
	s_itemsOffset = 0;
	s_curItmesNum = 0;
	s_DetListItem = 0;
	s_pDetList = NULL;
	pFunDestory = NULL;
}

static void ListViewDestroy(void)
{
    ListViewContainerFree();
	if (task_timeover) 
	{
		lv_task_del(task_timeover);
		task_timeover = 0;
	}

	if(g_page_list_win != NULL)
	{
		lv_obj_del(g_page_list_win); 
	    g_page_list_win = NULL;	
	}

    if(pFunDestory != NULL) { pFunDestory(0); }
	pFunDestory = NULL;

	ListViewInit();
	AppPowerUnlockApp((char*)"ListView");
}

static void _listbtn_event_cb(lv_obj_t* obj, lv_event_t e)
{
	return;
}

static lv_obj_t* page_lab_style(lv_obj_t* parent, char* text, lv_font_t* font, lv_label_long_mode_t long_mode, uint32_t c)
{
    lv_obj_t* lab = lv_label_create(parent, 0);

    lv_label_set_long_mode(lab, long_mode);
    lv_label_set_text(lab, text);
    lv_obj_set_style_local_text_font(lab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, font);
    lv_obj_set_style_local_text_color(lab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(c));
	lv_obj_align(lab, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    return lab;
}

static lv_obj_t* _create_btn(lv_obj_t* parent, struct _detail_list list, int count)
{
    int cnt = 0;
    char buf[64] = { 0 };
    lv_obj_t* lab[5] = { 0 };
    lv_obj_t* btn = lv_list_add_btn(parent, NULL, NULL);

    lv_obj_set_event_cb(btn, _listbtn_event_cb);
    lv_btn_set_layout(btn, LV_LAYOUT_OFF);
	lv_obj_set_size(btn, lv_obj_get_width(parent), page_get_statusbar_height());
	lv_obj_set_style_local_text_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_obj_set_style_local_pad_ver(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 20);

    lv_style_reset(&s_StyleCont);
    lv_style_set_bg_opa(&s_StyleCont, LV_STATE_DEFAULT, 0);
    lv_style_set_bg_color(&s_StyleCont, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_border_width(&s_StyleCont, LV_STATE_DEFAULT, 1);
    lv_style_set_border_opa(&s_StyleCont, LV_STATE_DEFAULT, 0);
    lv_style_set_border_color(&s_StyleCont, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_radius(&s_StyleCont, LV_STATE_DEFAULT, 0);
    lv_style_set_shadow_width(&s_StyleCont, LV_STATE_DEFAULT, 0);
    lv_obj_add_style(btn, LV_CONT_PART_MAIN, &s_StyleCont);

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "Date: %s", list.date);
    lab[cnt] = page_lab_style(btn, buf, LV_FONT_16, LV_LABEL_LONG_EXPAND, 0x0);
    cnt++;

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "Amount: %0.2f", list.amount);
    lab[cnt] = page_lab_style(btn, buf, LV_FONT_16, LV_LABEL_LONG_EXPAND, 0x0);
    cnt++;
	
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "TxnId: %s", list.txnId);
    lab[cnt] = page_lab_style(btn, buf, LV_FONT_16, LV_LABEL_LONG_EXPAND, 0x0);
    cnt++;
	
    lab[cnt] = page_lab_style(btn, " ", LV_FONT_16, LV_LABEL_LONG_EXPAND, 0x0);

    lv_obj_set_style_local_pad_all(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_bg_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    return btn;
}

static void ListViewUpdate(int updatePage)
{
    if(g_page_list == NULL || updatePage >= s_totalPage || updatePage < 0) { return; }
	#ifndef WIN32
    lv_list_clean(g_page_list);
	#endif

    int offset = updatePage * PAGE_MAX_BTN;	
	int curItems = 0; 
	
	APP_TRACE("ListViewUpdate updatePage:%d\r\n",updatePage);
	
	if(updatePage == (s_totalPage-1)) 
	{ 
		curItems = PAGE_MAX_BTN - (s_totalPage * PAGE_MAX_BTN - s_DetListItem); 
	}
	else
	{
		curItems = PAGE_MAX_BTN;
	}
	s_itemsOffset = offset;
	s_curItmesNum = curItems;
	
	APP_TRACE("offset:%d,curItems:%d\r\n",offset,curItems);
    for (int i = 0; i < curItems; i++)
    {
        lv_obj_t*btn = _create_btn(g_page_list, s_pDetList[offset+i], i);

        if(i == 0)
        {
            lv_list_focus_btn(g_page_list, btn);
            lv_obj_set_style_local_text_color(btn, LV_LABEL_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_WHITE);
        }
    }
}
static void DisplayItemsCb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_KEY)
	{
		lv_obj_del(obj);
	}
}

void ListViewDisplayItems(lv_obj_t *par , DetailList_t *pDet)
{
	char buf[128] = {0};
    lv_obj_t * mbox1 = lv_msgbox_create(par, NULL);
	page_group_set_obj(mbox1);
    snprintf(buf, sizeof(buf), "%s\r\nAmount: %0.2f", pDet->date, pDet->amount);
    lv_msgbox_set_text(mbox1, buf);
//    lv_msgbox_add_btns(mbox1, btns);
    lv_obj_set_width(mbox1, lv_obj_get_width(par));
    lv_obj_set_event_cb(mbox1, DisplayItemsCb);
    lv_obj_align(mbox1, NULL, LV_ALIGN_CENTER, 0, 0); /*Align to the corner*/
}

static void ListViewEventCb(lv_obj_t* obj, lv_event_t e)
{
    int key = -1;
    lv_obj_t *btn = NULL;

    APP_TRACE("e:%d\r\n", e);

    APP_TRACE("MF_LV_KEY_CANCEL_SHORT_PRESS:%d\r\n", MF_LV_KEY_CANCEL_SHORT_PRESS);
    APP_TRACE("MF_LV_KEY_7_SHORT_PRESS:%d\r\n", MF_LV_KEY_7_SHORT_PRESS);

    if (e == LV_EVENT_KEY)
    {
        key = page_get_key(0);

        if(key == MF_LV_KEY_CANCEL_SHORT_PRESS)
        {
            ListViewDestroy();
        }
        else if (key == MF_LV_KEY_F2_SHORT_PRESS)
        {
			btn = lv_list_get_btn_selected(g_page_list);
			if (btn != 0) 
			{
				btn = lv_list_get_prev_btn(g_page_list, btn);
				if (btn != 0) 
				{
					lv_obj_set_style_local_text_color(btn, LV_LABEL_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_WHITE);
					lv_list_focus_btn(g_page_list, btn);
					lv_list_get_btn_index(g_page_list, btn);
				}
			}
			#if 0
        	APP_TRACE("MF_LV_KEY_9_SHORT_PRESS s_curPage:%d\r\n",s_curPage);
            if(s_curPage > 0) { ListViewUpdate(--s_curPage); }
			if(s_curPage < 0) { s_curPage = 0; }
			#endif
        }
        else if (key == MF_LV_KEY_F1_SHORT_PRESS)
        {
			btn = lv_list_get_btn_selected(g_page_list);
			if (btn != 0) 
			{
				btn = lv_list_get_next_btn(g_page_list, btn);
				if (btn != 0) 
				{
					lv_obj_set_style_local_text_color(btn, LV_LABEL_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_WHITE);
					lv_list_focus_btn(g_page_list, btn);
					lv_list_get_btn_index(g_page_list, btn);
				}
			}
			#if 0
        	APP_TRACE("MF_LV_KEY_7_SHORT_PRESS s_curPage:%d\r\n",s_curPage);			
            if(s_totalPage == 0 || s_curPage == s_totalPage)
            {
                //TODO Tip the last page
            }
            else
            {
                ListViewUpdate(++s_curPage);
				if(s_curPage >= s_totalPage) { s_curPage = s_totalPage - 1; }
            }
			#endif
        }		
		else if (key >= MF_LV_KEY_1_SHORT_PRESS && key <= MF_LV_KEY_6_SHORT_PRESS)
		{
			int keyValue = GetKeyValue(key) - 0x30 - 1;
			APP_TRACE("keyValue:%d\r\n",keyValue);			
			if(keyValue <  s_curItmesNum)
			{
				DetailList_t *pDet = &s_pDetList[s_itemsOffset + keyValue];
				//APP_TRACE("pDet->amt:%s\r\n",pDet->amt);

				ListViewDisplayItems(obj,pDet);
			}			
		}
		if (key == MF_LV_KEY_OK_SHORT_PRESS) 
		{
			APP_TRACE("OK key:%d\r\n", key);
			btn = lv_list_get_btn_selected(g_page_list);
			int index = lv_list_get_btn_index(g_page_list, btn);
			DetailList_t *pDet = &s_pDetList[s_itemsOffset + index];
			ListViewDisplayItems(obj,pDet);
			//_menu_list_btn_click(g_page_list, btn);
		}
    }
}

static void _timeover_task_func(lv_task_t* task)
{
	if (m_timeover > 0) 
	{
		m_timeover -= 1000;
		if (m_timeover <= 0) 
		{
			APP_TRACE("[%s] _timeover_task_func end:%d\r\n", __FUNCTION__, m_timeover);
			ListViewDestroy();
		}
	}
}

/**
 * @brief
 * 
 * @param[in] lv_obj_t *par
 * @param[in] char *title
 * @param[in] DetailList_t *pDetList
 * @param[in] int size
 * @param[in] ListViewDestroyCb destroy
 */
void ListViewCreate(lv_obj_t *par, char *title, DetailList_t *pDetList, int size, ListViewDestroyCb destroy, int timeover)
{
	ListViewInit();//init
	
    pFunDestory = destroy;
    s_pDetList = pDetList;
    s_DetListItem = size;
	m_timeover = timeover;
	
    //s_curPage = 0;
    s_totalPage = (s_DetListItem + (PAGE_MAX_BTN - 1)) / PAGE_MAX_BTN;
	APP_TRACE("s_totalPage:%d\r\n",s_totalPage);
	AppPowerLockApp((char*)"ListView");

    g_page_list_win = page_create_win(par, ListViewEventCb);
    g_page_list = lv_list_create(g_page_list_win, NULL);
    page_create_title(g_page_list_win, title);

    lv_obj_set_style_local_pad_all(g_page_list, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 5);
    lv_obj_set_size(g_page_list, lv_obj_get_width(g_page_list_win), lv_obj_get_height(g_page_list_win) - 30 - page_get_title_height());
    lv_obj_align(g_page_list, g_page_list_win, LV_ALIGN_CENTER, 0, 30);
	lv_obj_set_style_local_bg_color(g_page_list, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

	ListViewUpdate(s_curPage);
	if(timeover>0)
		task_timeover = lv_task_create(_timeover_task_func, 1000, LV_TASK_PRIO_MID, 0);
}

