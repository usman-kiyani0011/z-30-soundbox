#pragma once
#define COMM_CONFIG_FILE  "comm.dat"

#define ENG_LANGUAGE         0
#define HIN_LANGUAGE         1

// ============ Comm Setting ID's ======
#define CARD_PAY_IP_URL_ID          110
#define CARD_PAY_PORT_ID            111
#define SERVICE_URL_ID     112
#define SERVICE_PORT_ID    113
#define MQTT_URL_ID     114
#define MQTT_PORT_ID    115
#define APN_ID          116
#define LANG_ID         117

#define MESSAGEID_MAX	20
#define MESSAGEID_SIZE	64

typedef struct st_control_config
{
	int isMqttSoundAllowed;
	int isSmsSoundAllowed;
	
}st_control_config;

typedef struct st_last_txn
{
	char payload[256];
	char currency[8];
	
}st_last_txn;


typedef struct comm_config
{
    char ip1[100];
    char port1[8];
    char ip2[100];
    char port2[8];
    char mqttUrl[100];
    char mqttPort[8];
    char apn[40];
    int defaultLang;
}comm_config;

void messageId_ini();
int checkFileExist(char *fileName);
int check_messageId(char* msgid);
void add_messageId(char* msgid);
int save_messageId();

