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
	int currentMode;
}network_param_t;
void network_page_win();
void destory_network_pagewin();