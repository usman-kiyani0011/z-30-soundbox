#ifndef __EMVCALLBACK_PAGE_H__
#define __EMVCALLBACK_PAGE_H__
enum{
	UPAY_RET_CANCEL = -7,
	UPAY_RET_TIME_OVER = -4,
	UPAY_RET_TERMINATE,

	UPAY_RET_OK = 0,
	UPAY_RET_KEYIN,
	UPAY_RET_MAGTEK,
	UPAY_RET_ICC,
	UPAY_RET_RFID,
	UPAY_RET_FALLBACK_MAGTEK,
	UPAY_RET_FALLBACK_KEYIN,

};

//#define OFFLINE_PIN_NOMAL		0x03	/**<��ʾ�����ѻ�PIN*/
//#define OFFLINE_PIN_AGAIN		0x02	/**<��ʾ�ٴ������ѻ�PIN*/
//#define OFFLINE_PIN_LAST		0x01	/**<���һ�������ѻ�PIN*/
//#define	ONLINE_PIN			0x00
//	
//
//int SetEmvFunc();
//int SetEmvRfFunc();
//void set_emv_amnt(char* amt);
//void set_emv_other_amnt(char* amt);
//void EMV_iReadingCardDisp(int iTranMode);
//char *get_CardNum();
MFSDKBOOL AppOnlinePinGetFlag(void);

#endif
