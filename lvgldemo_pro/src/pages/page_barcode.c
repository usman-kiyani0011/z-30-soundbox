
#include "lvgl/lvgl.h"
#include "code128/zint.h"
#include "pages/pages.h"

//#define BAR_HEIGHT		80
//#define BAR_WIDTH		260

void show_barcode(lv_obj_t* canvas, char* data, lv_color_t* cbuf, int zoom, int height, int width, int top, int color)
{
	struct zint_symbol* my_symbol = ZBarcode_Create();
	int i, j;
	int w = 1;
	int left;
	char code[32] = {0};

	my_symbol->symbology = BARCODE_CODE128;
	ZBarcode_Encode(my_symbol, (unsigned char*)data, 0);

	//lv_canvas_set_buffer(canvas, cbuf, width, height, LV_IMG_CF_TRUE_COLOR);
	//lv_canvas_fill_bg(canvas, LV_COLOR_WHITE, LV_OPA_COVER);

	if (strlen(data) == 18) 
	{
		memcpy(code + 0, data + 0, 4);
		code[4] = 0x20;
		memcpy(code + 5, data + 4, 4);
		code[9] = 0x20;
		memcpy(code + 10, data + 8, 4);
		code[14] = 0x20;
		memcpy(code + 15, data + 12, 6);
	}
	else 
	{
		strcpy(code, data);
	}

	left = (width - my_symbol->width * zoom) / 2;

	for (i = 0; i < my_symbol->width; i++) 
	{
		if (module_is_set(my_symbol, 0, i)) 
		{
			for (j = 0; j < height; j++) 
			{
				for (w = 0; w < zoom; w++) 
				{
					cbuf[( (j + top) * width) +  i * zoom + left + w] = lv_color_hex(color);
				}
			}
		}
	}

	ZBarcode_Clear(my_symbol);
	ZBarcode_Delete(my_symbol);

	return;
}

lv_obj_t* page_barcode(lv_obj_t* parent, char* data, lv_color_t* cbuf, int zoom, int height, int width, int top, int color)
{
	if(NULL == cbuf) return;
	
	lv_obj_t* canvas = lv_canvas_create(parent, NULL);
	lv_obj_align(canvas, NULL, LV_ALIGN_CENTER, 0, -40);//90
	lv_obj_set_size(canvas, width, height);
	lv_obj_set_x(canvas, 30);
	
	lv_canvas_set_buffer(canvas, cbuf, width, height, LV_IMG_CF_TRUE_COLOR);
	lv_canvas_fill_bg(canvas, LV_COLOR_WHITE, LV_OPA_COVER);

	show_barcode(canvas, data, cbuf, 2, height, width, 0, 0);
	
	page_ShowTextOut(parent, data, LV_ALIGN_CENTER, 0, 90, LV_COLOR_BLACK, LV_FONT_24);

	return canvas;
}

