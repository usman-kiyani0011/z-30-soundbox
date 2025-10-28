#pragma once
#include <stdbool.h>

enum 
{
	GPRS_MODE=0,
	WIFI_MODE
};
typedef struct
{
	bool ischecking_network;
	bool isgprs_network;
	int currentKey;
}network_param_t;
void network_page_win(char* title, char* fontname);
void destory_network_pagewin();