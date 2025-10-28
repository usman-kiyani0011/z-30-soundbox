#include "page_list.h"


/**********************
 *  STATIC VARIABLES
 **********************/
//static lv_font_t* font_small;
//static lv_font_t* font_medium;
static lv_style_t style_scrollbar;
static lv_style_t style_page_edge;
//static lv_style_t style_list;

static bool page_list_is_list_btn(lv_obj_t* list_btn)
{
	lv_obj_type_t type;

	lv_obj_get_type(list_btn, &type);
	uint8_t cnt;
	for (cnt = 0; cnt < LV_MAX_ANCESTOR_NUM; cnt++) {
		if (type.type[cnt] == NULL) break;
		if (!strcmp(type.type[cnt], "lv_obj")) return true;
	}
	return false;
}
static lv_obj_t* page_list_get_next_btn(const lv_obj_t* list, lv_obj_t* prev_btn)
{
	/* Not a good practice but user can add/create objects to the lists manually.
	 * When getting the next button try to be sure that it is at least a button */

	lv_obj_t* btn;
	lv_obj_t* scrl = lv_page_get_scrollable(list);

	btn = lv_obj_get_child_back(scrl, prev_btn);
	if (btn == NULL) return NULL;

	while (page_list_is_list_btn(btn) == false) {
		btn = lv_obj_get_child_back(scrl, btn);
		if (btn == NULL) break;
	}

	return btn;
}

/**
 * 创建可以选中列表
 * @param parent : 父视图
 * @param title : 标题
 * @param title_height: 标题 高度
 * @param edge_flash: 边缘半圆弧样式
 * @param img_body:背景图
 * @return : 创建的列表
 */
lv_obj_t* page_list_create(lv_obj_t* parent, char* title, int title_height, bool edge_flash, lv_obj_t *img_body)
{
	lv_obj_t* list = NULL;
	int height = 0;
	static lv_style_t style_title_lab;
	if (title) {
		height = title_height;
	}

	lv_style_init(&style_scrollbar);
	lv_style_set_size(&style_scrollbar, LV_STATE_DEFAULT, 1);
	lv_style_set_bg_opa(&style_scrollbar, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_style_set_bg_color(&style_scrollbar, LV_STATE_DEFAULT, lv_color_hex3(0xeee));
	lv_style_set_radius(&style_scrollbar, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
	lv_style_set_pad_right(&style_scrollbar, LV_STATE_DEFAULT, 0);
	

	if (img_body == NULL) {
		parent = parent;
	}

	if (height&&title) {
		lv_style_init(&style_title_lab);
		lv_style_set_text_color(&style_title_lab, LV_STATE_DEFAULT, lv_color_hex(0xffffff));

		lv_obj_t* title_obj = lv_label_create(img_body, NULL);
		lv_obj_clean_style_list(title_obj, LV_LABEL_PART_MAIN);
		lv_label_set_text(title_obj, title);
		lv_obj_set_size(title_obj, lv_obj_get_width(img_body), height);
		lv_obj_align(title_obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

		lv_obj_add_style(title_obj, LV_LABEL_PART_MAIN, &style_title_lab);
	}

	/*Create an empty white main container*/
	list = lv_page_create(parent, NULL);
	lv_obj_set_size(list, LV_HOR_RES, LV_VER_RES - height);
	lv_obj_set_y(list, height);
	lv_obj_clean_style_list(list, LV_PAGE_PART_BG);
	lv_obj_clean_style_list(list, LV_PAGE_PART_SCROLLABLE);
	lv_obj_clean_style_list(list, LV_PAGE_PART_SCROLLBAR);
	lv_page_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_DRAG);
	lv_obj_add_style(list, LV_PAGE_PART_SCROLLBAR, &style_scrollbar);

	lv_page_set_scrl_layout(list, LV_LAYOUT_COLUMN_MID);

	lv_obj_set_user_data(list, (lv_obj_user_data_t) img_body);
	

	if (edge_flash) {
		lv_page_set_edge_flash(list, true);//使能边缘半圆弧动画效果
		//边缘半圆弧样式
		lv_style_init(&style_page_edge);
		lv_style_set_bg_color(&style_page_edge, LV_STATE_DEFAULT, lv_color_hex(0x00ff00));
		lv_style_set_bg_opa(&style_page_edge, LV_STATE_DEFAULT, LV_OPA_30);
		lv_obj_add_style(list, LV_PAGE_PART_EDGE_FLASH, &style_page_edge);
	}

	return list;
}
/**
 * 删除选中列表
 * @param list 指针
 */
void page_list_clean(lv_obj_t * list)
{
	if (!list) return;
	lv_obj_t* img_body = (lv_obj_t *)lv_obj_get_user_data(list);
	if(img_body)
		lv_obj_del(img_body);
	lv_obj_del(list);
}

/**
 * 选中状态设置
 * @param track_id : 索引值 0...N
 * @param state : 是否选中
 */
void page_list_btn_check(const lv_obj_t* list,uint32_t track_id, bool state)
{
	lv_obj_t* btn = lv_obj_get_child_back(lv_page_get_scrl((lv_obj_t*)list), NULL);
	uint32_t i = 0;
	while (btn) {
		if (i == track_id) break;
		i++;
		btn = lv_obj_get_child_back(lv_page_get_scrl((lv_obj_t*)list), btn);
	}

	if (btn) {
		lv_obj_t* icon = lv_obj_get_child_back(btn, NULL);

		if (state) {
			lv_imgbtn_set_state(icon, LV_BTN_STATE_CHECKED_RELEASED);
			lv_obj_add_state(btn, LV_STATE_CHECKED);
		}
		else {
			lv_imgbtn_set_state(icon, LV_BTN_STATE_RELEASED);
			lv_obj_clear_state(btn, LV_STATE_CHECKED);
		}
	}

}
/**
 * 获取选中按钮在列表的索引值
 * @param 列表指针
 * @param 列表选中按钮的指针
 * @return 列表索引值 -1 ：不属于该列表 列表索引:0...N
 */
int32_t page_list_get_btn_index(const lv_obj_t * list, const lv_obj_t * btn)
{
	int i = 0;
	lv_obj_t* child = lv_obj_get_child_back(lv_page_get_scrl((lv_obj_t*)list), NULL);
	while (child) {
		if ((lv_obj_t*)btn == child) {
			return i;
		}
		i++;
		child = lv_obj_get_child_back(lv_page_get_scrl((lv_obj_t*)list), child);
	}
	return -1;
}

/**
 * 删除列表的行
 * @param 列表指针
 * @param 要删除列表索引值
 * @return 删除成功、失败
 */
bool page_list_remove(const lv_obj_t* list, uint16_t index)
{
	uint16_t count = 0;
	lv_obj_t* e = page_list_get_next_btn(list, NULL);
	while (e != NULL) {
		if (count == index) {
			lv_obj_del(e);
			return true;
		}
		e = page_list_get_next_btn(list, e);
		count++;
	}
	return false;
}