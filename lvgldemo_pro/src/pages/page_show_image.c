#include "lvgl/lvgl.h"
#include "page_pub.h"
#include "page_show_image.h"
#include "pages/pages.h"
//#include "tracedef.h"

static int m_timeover;
static int m_timeovr_reset;
//static page_timer_func m_timer_func = 0;
static lv_obj_t* page_win = NULL;
static lv_obj_t* page_img = NULL;
static lv_task_t* task_meaasge=NULL;
static char *imag_path=NULL;
static page_close_page_func m_page_close_page_func = 0;

static void _message_close_page(int ret)
{
    if (page_win != 0) 
	{	
		if(NULL != page_img)
		{
			lv_obj_del(page_img);
			page_img = 0;
		}
		lv_obj_del(page_win);
        page_win = 0;
		if(imag_path!=NULL)
		{
			lv_free_png_file(imag_path);
			imag_path=NULL;
		}
    }
	if(task_meaasge)
	{
		lv_task_del(task_meaasge);
		task_meaasge = 0;
	}
	if (m_page_close_page_func != 0) 
		m_page_close_page_func(0,NULL);
	m_page_close_page_func = 0;

}

static void _message_task_func(lv_task_t* task)
{
	if (m_timeover > 0) 
	{
		m_timeover -= 1000;
		if (m_timeover <= 0) 
		{
            _message_close_page(PAGE_RET_TIMEOVR);
            return;
		}
	}
}

void message_close_show_image(int ret)
{
	_message_close_page(ret);
}

lv_obj_t* page_show_image(lv_obj_t * parent, void* pfunc, char *title, int result, char*msg, int timeout)
{
	//char *tip = 0;
	lv_obj_t* img_obj = 0;

	if(parent)
		img_obj = parent;
	
	if(img_obj == NULL)
	{
		APP_TRACE("parent is null!\r\n");
		return NULL;
	}

	_message_close_page(0);
    m_page_close_page_func = (page_close_page_func)pfunc;
    m_timeover = timeout;
    m_timeovr_reset = timeout;
	
	if(0 == result)
	{
		if(LCD_IS_320_480)
			imag_path = SUCCPNG_320X480;
		else
			imag_path = SUCCPNG;
		//tip = "Payment Success";
	}
	else
	{
		if(LCD_IS_320_480)
			imag_path = FAILPNG_320X480;
		else
			imag_path = FAILPNG;
		//tip = "Payment Fail";
	}
	
    page_win = page_create_win(img_obj, NULL);
	page_img = lv_img_create(img_obj, NULL);
	lv_obj_set_size(page_win, lv_obj_get_width(img_obj), lv_obj_get_height(img_obj));
	lv_load_png_file(imag_path);
	lv_img_set_src(page_img, imag_path);
	lv_obj_align(page_img, NULL, LV_ALIGN_CENTER, 0, -30);

	if(NULL != msg)
	{
		page_ShowTextOut(page_img, msg, LV_ALIGN_CENTER, 0, 120, LV_COLOR_BLACK, LV_FONT_24);
	}

    if(timeout>0)
	    task_meaasge = lv_task_create(_message_task_func, 1000, LV_TASK_PRIO_MID, 0);

	return page_win;
}

lv_obj_t* page_show_image_path(lv_obj_t * parent, void* pfunc, char *title, char *path, char*msg, int timeout)
{
	lv_obj_t* img_obj = 0;

	if(parent)
		img_obj = parent;
	
	if(img_obj == NULL)
	{
		APP_TRACE("parent is null!\r\n");
		return NULL;
	}

	_message_close_page(0);
    m_page_close_page_func = (page_close_page_func)pfunc;
    m_timeover = timeout;
    m_timeovr_reset = timeout;

	if(path)
	{
		imag_path = path;
	}
	
    page_win = page_create_win(img_obj, NULL);
	page_create_title(page_win, title);
	page_img = lv_img_create(img_obj, NULL);
	lv_obj_set_size(page_win, lv_obj_get_width(img_obj), lv_obj_get_height(img_obj));
	lv_load_png_file(imag_path);
	lv_img_set_src(page_img, imag_path);
	lv_obj_align(page_img, NULL, LV_ALIGN_CENTER, 0, 0);

	if(NULL != msg)
	{
		page_ShowTextOut(page_win, msg, LV_ALIGN_CENTER, 0, 120, LV_COLOR_BLACK, LV_FONT_24);
	}

    if(timeout>0)
	    task_meaasge = lv_task_create(_message_task_func, 1000, LV_TASK_PRIO_MID, 0);

	return page_win;
}