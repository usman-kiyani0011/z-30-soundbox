#include <string.h>
#include <stdio.h>

#include "libapi_xpos/inc/util_tts.h"

#include "tracedef.h"
#include "libapi_xpos/inc/mfsdk_aud.h"


#define CRORE           10000000
#define LAKH            100000
#define THOUSAND        1000
#define HUNDRED         100

//#define MP3_FILE	"data\\1.mp3"

// Play unit 0 = hundred 1 = thousand 2 = million 3= One hundred thousand 4=Ten million
static void sdk_tts_play_unit(int index)
{
	char filename[16]={0};
	memset(filename,0x00,sizeof(filename));

	sprintf(filename, "hunit%d.mp3", index);//fix the play issue

	MfSdkAudPlay((const s8*)filename);	
}

// Play the numbers 0-19 20-90
 void sdk_tts_play_num(int num)
{
	char filename[16]={0};
	memset(filename,0x00,sizeof(filename));

	APP_TRACE("Hindi Number will be played \r\n");
	sprintf(filename, "num%d.mp3", num);

	MfSdkAudPlay((const s8*)filename);
}

// Play less than a thousand
static void sdk_tts_play_lt(int amt)
{
	int m;
	m = amt / 100;
	if(m >0){
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


void sdk_tts_play_amt_india(int amt)
{
	int m;


	if(amt >= CRORE){	// //If it is more than Ten million,
		m = amt / CRORE;
		sdk_tts_play_lt(m);	//  play the number of more than Ten million first
		sdk_tts_play_unit(3);	//plus the  "Crore"
		amt = amt % CRORE;		// Remove more than one million of the amount
	}

	if(amt >= LAKH){	// //If it is more than One hundred thousand,
		m = amt / LAKH;
		sdk_tts_play_lt(m);	//  play the number of more than One hundred thousand first
		sdk_tts_play_unit(2);	//plus the LAKH
		amt = amt % LAKH;		// Remove more than One hundred thousand of the amount
	}

	// Determine if it is more than a thousand
	if(amt >= THOUSAND){		
		m = amt / THOUSAND;
		sdk_tts_play_lt(m);		// play a part of a thousand or more
		sdk_tts_play_unit(1);	// play a thousand
		amt = amt % THOUSAND;		
	}

	sdk_tts_play_lt(amt);			// Play numbers below a thousand

	MfSdkAudPlay((const s8*)"Rupees.mp3");
      

}



void sdk_tts_play_amt_en(int amt)
{
	int m;


	
	if(amt >= 1000000){	// //If it is more than one million,
		m = amt / 1000000;
		sdk_tts_play_lt(m);	//  play the number of more than one million first
		sdk_tts_play_unit(2);	//plus the million
		amt = amt % 1000000;		// Remove more than one million of the amount
	}


	// Determine if it is more than a thousand
	if(amt >= 1000){		
		m = amt / 1000;
		sdk_tts_play_lt(m);		// play a part of a thousand or more
		sdk_tts_play_unit(1);	// play a thousand
		amt = amt % 1000;		
	}
	
	sdk_tts_play_lt(amt);			// Play numbers below a thousand

}

void sdk_tts_play_amt_india_Paise(int amt)
{
	sdk_tts_play_lt(amt);			// Play numbers below one hundred

	MfSdkAudPlay((const s8*)"Paise.mp3");
      
}

