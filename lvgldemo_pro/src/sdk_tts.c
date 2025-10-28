#include <string.h>
#include <stdio.h>

#include "libapi_xpos/inc/util_tts.h"

#include "tracedef.h"
#include "libapi_xpos/inc/mfsdk_aud.h"
#include "libapi_xpos/inc/mfsdk_sys.h"
#include "player_proc.h"
#include "sdk_tts.h"

#define CRORE           10000000
#define LAKH            100000
#define THOUSAND        1000
#define HUNDRED         100

//#define MP3_FILE	"data\\1.mp3"
typedef enum
{
	LANG_DEF = 0,
	LANG_ENG,
	LANG_INDIA,
	LANG_MAX,
}Language_E;

static Language_E g_language = LANG_DEF;
static const char*s_langPath[LANG_MAX] = {"exdata", "lang1", "lang2"};

void SetLanguage(int lang)
{
	g_language = lang;
	set_setting_int(PLAY_LANGUAGE, lang);
	return;
}

int LanguageInit()
{
	g_language = get_setting_int(PLAY_LANGUAGE);
	APP_TRACE("LanguageInit = %d\r\n",g_language);
	return g_language;
}
	
int GetLanguage()
{
	APP_TRACE("GetLanguage = %d\r\n",g_language);
	return g_language;
}

static const char* GetLanguagePath(Language_E lang)
{
	const char *path = NULL;
	if(lang >= LANG_DEF && lang < LANG_MAX)
	{
		path = s_langPath[lang];
	}
	else
	{
		path = "exdata";
	}
	return path;
}

int PubMultiPlay(const s8 * sndfile)
{
	char filepath[64]={0};

	memset(filepath, 0, sizeof(filepath));
	sprintf(filepath, "%s\\%s", GetLanguagePath(g_language), sndfile);
	APP_TRACE("language = %d, filepath = %s\r\n",g_language, filepath);
	return MfSdkAudPlay((const s8 *)filepath );
}

// Play unit 0 = hundred 1 = thousand 2 = million 3= One hundred thousand 4=Ten million
static void sdk_tts_play_unit(int index)
{
	char filename[16]={0};
	memset(filename,0x00,sizeof(filename));

	sprintf(filename, "unit%d.mp3", index);//fix the play issue

	PubMultiPlay((const s8*)filename);	
}

// Play the numbers 0-19 20-90
 void sdk_tts_play_num(int num)
{
	char filename[16]={0};
	memset(filename,0x00,sizeof(filename));

	sprintf(filename, "num%d.mp3", num);
	APP_TRACE("tts_play_num filename = %s \r\n", filename);
	PubMultiPlay((const s8*)filename);
}

// Play less than a thousand
static void sdk_tts_play_lt(int amt)
{
	int m;
	m = amt / 100;
	if(m >0)
	{
		sdk_tts_play_num(m);
		sdk_tts_play_unit(0);		// hundred 
	}
	
	amt = amt % 100;
	if((amt > 20))
	{ //  greater than 20
		APP_TRACE("English Language and play the amount greater than 20\r\n");
		m = amt / 10;
		sdk_tts_play_num(m * 10);	// Tens first digits
		amt = amt % 10;
	}
	if(amt>0)
		sdk_tts_play_num(amt);   // Less than 20 direct play
}


void pub_tts_play_amt_india(int amt)
{
	int m;


	if(amt >= CRORE)	// //If it is more than Ten million,
	{
		m = amt / CRORE;
		sdk_tts_play_lt(m);	//  play the number of more than Ten million first
		sdk_tts_play_unit(3);	//plus the  "Crore"
		amt = amt % CRORE;		// Remove more than one million of the amount
	}

	if(amt >= LAKH)	// //If it is more than One hundred thousand,
	{
		m = amt / LAKH;
		sdk_tts_play_lt(m);	//  play the number of more than One hundred thousand first
		sdk_tts_play_unit(2);	//plus the LAKH
		amt = amt % LAKH;		// Remove more than One hundred thousand of the amount
	}

	// Determine if it is more than a thousand
	if(amt >= THOUSAND)
	{		
		m = amt / THOUSAND;
		sdk_tts_play_lt(m);		// play a part of a thousand or more
		sdk_tts_play_unit(1);	// play a thousand
		amt = amt % THOUSAND;		
	}

	sdk_tts_play_lt(amt);			// Play numbers below a thousand

	PubMultiPlay((const s8*)"Rupees.mp3");
      

}

static void pub_tts_play_amt_india_paise(int amt)
{
	if (amt <= 0)
	{
		return;
	}

	sdk_tts_play_lt(amt);
	PubMultiPlay((const s8*)"Paise.mp3");
}



void sdk_tts_play_amt_en(int amt)
{
	int m;

	if(amt >= 1000000)	// //If it is more than one million,
	{
		m = amt / 1000000;
		sdk_tts_play_lt(m);	//  play the number of more than one million first
		sdk_tts_play_unit(2);	//plus the million
		amt = amt % 1000000;		// Remove more than one million of the amount
	}


	// Determine if it is more than a thousand
	if(amt >= 1000)
	{		
		m = amt / 1000;
		sdk_tts_play_lt(m);		// play a part of a thousand or more
		sdk_tts_play_unit(1);	// play a thousand
		amt = amt % 1000;		
	}
	
	sdk_tts_play_lt(amt);			// Play numbers below a thousand

}

void pub_tts_play_amt_en(int amt)
{
	int rupees = amt / 100;
	int paise = amt % 100;

	PubMultiPlay((const s8*)"pay.mp3");

	if (rupees > 0)
	{
		pub_tts_play_amt_india(rupees);

		if (paise > 0)
		{
			PubMultiPlay((const s8*)"and.mp3");
			pub_tts_play_amt_india_paise(paise);
		}
	}
	else if (paise > 0)
	{
		pub_tts_play_amt_india_paise(paise);
	}
}
				
#define UNIT_MILLION 1000000
#define UNIT_THOUSAND 1000
#define UNIT_HUNDRED 100

#define UNIT_INDEX_MILLION 2
#define UNIT_INDEX_THOUSAND 1
#define UNIT_INDEX_HUNDRED 0


static void sdk_tts_play_unit_idr(int index)
{
	char filename[16]={0};
	memset(filename,0x00,sizeof(filename));

	sprintf(filename, "unit%d.mp3", index);//fix the play issue

	PubMultiPlay((const s8*)filename);	
}


static void sdk_tts_play_lt_idr(int amt)
{
	int m = 0;
	
	m = amt / 100;
	
	if(m > 0)
	{
		if(m == 1){ PubMultiPlay((const s8*)"se.mp3");}
		else { sdk_tts_play_num(m);}
		sdk_tts_play_unit_idr(UNIT_INDEX_HUNDRED);		// hundred 
	}	
	amt = amt % 100;

	if(amt > 20) //greater than 20
	{ 
	 	APP_TRACE("play the amount greater than 20\r\n");
		m = amt / 10;
		sdk_tts_play_num(m * 10);	// Tens first digits
		amt = amt % 10;
	}
	
    if(amt>0) { sdk_tts_play_num(amt); }   // Less than 20 direct play          
}

void pub_tts_play_amt_idr(int amt)
{
	int m = 0;
	
	if(amt >= UNIT_MILLION)// If it is more than one million,
	{	
		m = amt / UNIT_MILLION;
//		if(m == 1) { MfSdkAudPlay((const s8*)"num1.mp3"); }
//		else { sdk_tts_play_lt_idr(m);}	//  play the number of more than one million first
		sdk_tts_play_lt_idr(m);
		sdk_tts_play_unit_idr(UNIT_INDEX_MILLION);	//plus the million
		amt = amt % UNIT_MILLION;		// Remove more than one million of the amount
	}


	// Determine if it is more than a thousand
	if(amt >= UNIT_THOUSAND)
	{		
		m = amt / UNIT_THOUSAND;
		if(m == 1){ PubMultiPlay((const s8*)"se.mp3"); }
		else { sdk_tts_play_lt_idr(m); }		// play a part of a thousand or more
		sdk_tts_play_unit_idr(UNIT_INDEX_THOUSAND);	// play a thousand
		amt = amt % UNIT_THOUSAND;		
	}
	
	sdk_tts_play_lt_idr(amt);			// Play numbers below a thousand
}

void AppPlayBatteryLevel(void)
{
	int ret = -1;
	int capacity = 0;
	char radiobuf[16] = { 0 };
	MfSdkBatterAttr_T batterAttr = {0};

	memset(&batterAttr , 0, sizeof(MfSdkBatterAttr_T));
	ret = MfSdkSysGetBatterStatus(&batterAttr);
	if (ret >= 0) { capacity = batterAttr.capacity; }

	APP_TRACE("AppPlayBatteryLevel= %d  %d \r\n", capacity, batterAttr.voltage_level);
	MfSdkAudPlay("battcap.mp3");
	if (capacity >0 && capacity <20)
	{
		if (capacity == 10)
		{
			strcpy(radiobuf, "num10.mp3");
			MfSdkAudPlay(radiobuf);
		}
		else
		{
			sprintf(radiobuf, "num%d.mp3", capacity);
			MfSdkAudPlay(radiobuf);
		}
		
	}
	else if (capacity >= 20 && (capacity<=100))
	{
		memset(radiobuf, 0, sizeof(radiobuf));
		if ((capacity==20)|| (capacity == 30) || (capacity == 40) || (capacity == 50) || (capacity == 60)
			|| (capacity == 70) || (capacity == 80) || (capacity == 90))
		{
			
			sprintf(radiobuf, "num%d.mp3", capacity);
			MfSdkAudPlay(radiobuf);
		}
		else if (capacity == 100)
		{
			memset(radiobuf, 0, sizeof(radiobuf));
			strcpy(radiobuf, "num1.mp3");
			MfSdkAudPlay(radiobuf);
			memset(radiobuf, 0, sizeof(radiobuf));
			strcpy(radiobuf, "unit0.mp3");
			MfSdkAudPlay(radiobuf);
		}
		else
		{
			int tensnum = capacity / 10;
			int remainder = capacity % 10;
			memset(radiobuf, 0, sizeof(radiobuf));
			sprintf(radiobuf, "num%d0.mp3", tensnum);
			MfSdkAudPlay(radiobuf);
			memset(radiobuf, 0, sizeof(radiobuf));
			sprintf(radiobuf, "num%d.mp3", remainder);
			MfSdkAudPlay(radiobuf);
		}
		
	}
}
/**
 * @brief
 * 
 */
void AppPlayBatteryIndonesia(void)
{
	MfSdkBatterAttr_T batterAttr ;
	int ret;
	int capacity = 0;
	char radiobuf[16] = { 0 };

	memset(&batterAttr , 0, sizeof(MfSdkBatterAttr_T));
	ret = MfSdkSysGetBatterStatus(&batterAttr);
	if (ret >= 0) { capacity = batterAttr.capacity; }

	APP_TRACE("AppPlayBattery:%d\r\n",capacity);

	PubMultiPlay((const s8*)"battcap.mp3");
	if (capacity > 0 && capacity < 20)
	{
		sprintf(radiobuf, "num%d.mp3", capacity);
		PubMultiPlay((const s8*)radiobuf);	
	}
	else if (capacity >= 20 && (capacity <= 100))
	{
		memset(radiobuf, 0, sizeof(radiobuf));
		if (capacity == 100)
		{
			PubMultiPlay((const s8*)"se.mp3");
			sdk_tts_play_unit_idr(UNIT_INDEX_HUNDRED);
		}
		else
		{
			int tensnum = capacity / 10;
			int remainder = capacity % 10;

			APP_TRACE("tensnum:%d,remainder:%d\r\n",tensnum,remainder);
			if(tensnum > 0)
			{
				memset(radiobuf, 0, sizeof(radiobuf));
				sprintf(radiobuf, "num%d0.mp3", tensnum);
				PubMultiPlay((const s8*)radiobuf);
			}
			if(remainder > 0)
			{
				memset(radiobuf, 0, sizeof(radiobuf));
				sprintf(radiobuf, "num%d.mp3", remainder);
				PubMultiPlay((const s8*)radiobuf);
			}
		}
	}
}
