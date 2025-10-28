#include "lvgl/lvgl.h"
#include "./pages/pages.h"
#include "page_card.h"
#include "tracedef.h"
#include "page_pub.h"

static lv_obj_t* page_M1 = NULL;
static lv_obj_t* lab_title = NULL;
static lv_obj_t* lab_tip = NULL;
static lv_obj_t* lab_tip2 = NULL;
static lv_obj_t* lab_tip3 = NULL;
static lv_task_t* M1card_task = NULL;


static void _card_close_page(int ret)
{
	APP_TRACE("_card_close_page close ret = %d",ret);
    if (M1card_task) 
	{
        lv_task_del(M1card_task);
        M1card_task = NULL;
    }
	
    if (page_M1 != NULL)
    {
	    if (lab_title != NULL)
	    {
	        lv_obj_del(lab_title);
	        lab_title = NULL;
	    }
	    if (lab_tip != NULL)
	    {
	        lv_obj_del(lab_tip);
	        lab_tip = NULL;
	    }
	    if (lab_tip2 != NULL)
	    {
	        lv_obj_del(lab_tip2);
	        lab_tip2 = NULL;
	    }
	    if (lab_tip3 != NULL)
	    {
	        lv_obj_del(lab_tip3);
	        lab_tip3 = NULL;
	    }
        lv_obj_del(page_M1);
        page_M1 = NULL;

		lv_free_png_file(RFPNG);
        //if (m_page_close_page_func != 0) m_page_close_page_func(ret, NULL);
    }
}

static void _card_event_cb(lv_obj_t* obj, lv_event_t e)
{
	uint32_t key;

    if (e == LV_EVENT_KEY) 
	{
        key = page_get_key();

        if (key == MF_LV_KEY_OK_SHORT_PRESS)
		{
            _card_close_page(-1);
        }
    }
}

static void M1CardTask_func(lv_task_t *pTask)
{
	int rc;
	unsigned char uid[16];
	int uidlen = 0;
	int cmd = 0;
	int sector = 0;
	int block = 0;
	char tempbuf[64] = {0};
	unsigned char databuff[16] = {0};
	int datalen = 0;

	rc = MfSdkNfcM1Open();//Find card
	APP_TRACE("mf_rfid_mfcl_open rc: %d\r\n",rc);
	if(rc >= 0) 
	{
		uidlen = MfSdkNfcGetUid(uid);//get card sn
		if(uidlen >=0)
		{
			MfSdkUtilHex2Asc(uid ,8, 0, (u8*)tempbuf);
			APP_TRACE("CardID: %s\r\n",tempbuf);
			MfSdkNfcM1SetKey((u8*)"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF");			
			cmd = 0x60;//0x60:A key authentication;0x61:B key authentication
			sector = 15;//0--15	
			rc = MfSdkNfcM1Auth(cmd, sector);//Authenticate sector
			if(rc == 0)
			{
				APP_TRACE("mf_rfid_mfcl_auth: %d\r\n",rc);
				block = 4*15+0;//0--2, fourth blcok is key data  
				rc = MfSdkNfcM1Read(block, databuff, &datalen);//Reads the data for the specified block
				APP_TRACE("mf_rfid_mfcl_read: %d\r\n",rc);
				if(rc == 0 && datalen > 0)
				{
					//MfSdkUtilHex2Asc(databuff , datalen*2 , 0, (u8*)tempbuf );
					APP_TRACE("Card info[%d]: %s\r\n",datalen, databuff);
					lv_label_set_text(lab_tip, "CardID:");
					lv_label_set_text(lab_tip2, (char*)databuff);
				}
				else
				{
					APP_TRACE("read data fail");
					lv_label_set_text(lab_tip, "");
					lv_label_set_text(lab_tip2, "read data fail");
				}
			}
			else
			{
				lv_label_set_text(lab_tip, "");
				lv_label_set_text(lab_tip2, "authentication fail");
			}
		}
		else
		{
			//xgui_messagebox_show("rf m1" , "rf get cardid fail", "" , "confirm" , 0);
		}
	}
	MfSdkNfcM1Close();
	return ;

}

void M1Card_Proc()
{
	int user_data = 10;
	lv_obj_t *img;
    page_M1 = page_create_win(get_mainpage(), _card_event_cb);

	img = lv_img_create(page_M1, NULL);
	lv_obj_set_size(img, lv_obj_get_width(page_M1), lv_obj_get_height(page_M1));
	lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_MID, 50, 90);
	lv_load_png_file(RFPNG);
	lv_img_set_src(img, RFPNG);

	lab_title = page_ShowTextOut(page_M1, "M1 card", LV_ALIGN_IN_TOP_MID, 0, 0, LV_COLOR_BLACK, LV_FONT_24);
	lab_tip = page_ShowTextOut(page_M1, "Please press your", LV_ALIGN_IN_TOP_MID, 0, 180, LV_COLOR_BLACK, LV_FONT_24);
	lab_tip2 = page_ShowTextOut(page_M1, "M1 card", LV_ALIGN_IN_TOP_MID, 0, 210, LV_COLOR_BLACK, LV_FONT_24);
	lab_tip3 = page_ShowTextOut(page_M1, "press M key exit", LV_ALIGN_IN_BOTTOM_MID, 0, -20, LV_COLOR_BLACK, LV_FONT_24);

	M1card_task = lv_task_create(M1CardTask_func, 1000, LV_TASK_PRIO_MID, &user_data);
}

