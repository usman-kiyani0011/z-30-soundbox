//#include <xdk_et.h>
#include <stdio.h>
#include <stdlib.h>
#include "tracedef.h"
#include "player_proc.h"
#include "xdk_filedef.h"
#include "file.h"
#include "libapi_xpos/inc/def.h"
#include "libapi_xpos/inc/mfsdk_fs.h"


#define CTRL_FILE_NAME  "ctrl.dat"
#define LAST_FILE_NAME  "lst.dat"

struct st_control_config g_st_control_config;
struct st_last_txn g_st_last_txn;

struct comm_config g_commConfig;
void readControlConfig()
{
	int fd=0;
    memset(&g_st_control_config,0x00,sizeof(struct st_control_config));
	APP_TRACE("Inside to save the updateControlConfig \r\n");
//    fd = FILE_OPEN(CTRL_FILE_NAME, FILE_READ_FLAG,FILE_READ_MODE);
	fd = MfSdkFsOpen((const s8*)CTRL_FILE_NAME,MFSDK_FS_FLAG_READ,MFSDK_FS_MODE_READ);
    if(fd != MFSDK_FS_RET_OPEN_FAIL)
	{
//		FILE_LSEEK(fd , 0, 0);
		MfSdkFsLseek(fd ,0, 0);
//		FILE_READ(fd, &g_st_control_config, sizeof(st_control_config));
		MfSdkFsRead(fd, (char*)&g_st_control_config, sizeof(st_control_config));
//		FILE_CLOSE(fd);				  // Close the file
		MfSdkFsClose(fd);
	}
	else{
		APP_TRACE("File Open Failed");
	}
}

void updateControlConfig()
{
	int fd=0;
	memset(&g_st_control_config,0x00,sizeof(struct st_control_config));
	APP_TRACE("Inside to save the updateControlConfig \r\n");
    fd = MfSdkFsOpen((const s8*)CTRL_FILE_NAME, MFSDK_FS_FLAG_WRITE,MFSDK_FS_MODE_WRITE);
    if(fd == MFSDK_FS_RET_OPEN_FAIL)
    {
		fd = MfSdkFsOpen((const s8*)CTRL_FILE_NAME, MFSDK_FS_FLAG_CREAT,MFSDK_FS_MODE_WRITE);
    }
    if(fd != MFSDK_FS_RET_OPEN_FAIL)
	{
		MfSdkFsLseek(fd , 0, 0);
		MfSdkFsWrite(fd, (char*)&g_st_control_config, sizeof(st_control_config));
		MfSdkFsClose(fd);				  // Close the file
	}
	else{
          APP_TRACE("File Open Failed");
	}
	readControlConfig();
}

void saveLastTxn()
{
	int fd=0;
	APP_TRACE("Inside to save the saveLast transactions \r\n");
	fd = MfSdkFsOpen((const s8*)LAST_FILE_NAME, MFSDK_FS_FLAG_WRITE,MFSDK_FS_MODE_WRITE);
	if(fd == MFSDK_FS_RET_OPEN_FAIL)
	{
		fd = MfSdkFsOpen((const s8*)LAST_FILE_NAME, MFSDK_FS_FLAG_CREAT,MFSDK_FS_MODE_WRITE);
	}
	if(fd != MFSDK_FS_RET_OPEN_FAIL)
	{
		APP_TRACE("File Open Success\r\n");
		MfSdkFsLseek(fd , 0, 0);
		MfSdkFsWrite(fd, (char*)&g_st_last_txn, sizeof(struct st_last_txn));
		MfSdkFsClose(fd);				  // Close the file
	}
	else{
		APP_TRACE("File Open again Failed\r\n");
	}
}

void readLastTxn()
{
	int fd=0;
	memset(&g_st_last_txn,0x00,sizeof(struct st_last_txn));
	APP_TRACE("Inside to readLastTxn transactions \r\n");
	fd = MfSdkFsOpen((const s8*)LAST_FILE_NAME, MFSDK_FS_FLAG_READ,MFSDK_FS_MODE_READ);
	if(fd != MFSDK_FS_RET_OPEN_FAIL)
	{
		APP_TRACE("File Open Success\r\n");
		MfSdkFsLseek(fd , 0, 0);
		MfSdkFsRead(fd, (char*)&g_st_last_txn, sizeof(struct st_last_txn));
		MfSdkFsClose(fd);				  // Close the file
	}
	else{
		APP_TRACE("File Open again Failed\r\n");
	}
}

int defaultCommConfig()
{
	int ret = 0;
//	int fp;
	int fd=0;
	memset(&g_commConfig,0x00,sizeof(struct comm_config));
	memcpy(g_commConfig.apn,"",3);
    strcpy(g_commConfig.ip1,"");
	strcpy(g_commConfig.port1,"");
	strcpy(g_commConfig.ip2,"");
	strcpy(g_commConfig.port2,"");
	strcpy(g_commConfig.mqttUrl,"");
	memcpy(g_commConfig.mqttPort,"",4);  
    g_commConfig.defaultLang = ENG_LANGUAGE;
       
	fd = MfSdkFsOpen((const s8*)COMM_CONFIG_FILE, MFSDK_FS_FLAG_WRITE,MFSDK_FS_MODE_WRITE);
	if(fd == MFSDK_FS_RET_OPEN_FAIL)
	{
		fd = MfSdkFsOpen((const s8*)COMM_CONFIG_FILE, MFSDK_FS_FLAG_CREAT,MFSDK_FS_MODE_WRITE);
	}
	if(fd != MFSDK_FS_RET_OPEN_FAIL)
	{
		APP_TRACE("File Open Success\r\n");
		MfSdkFsLseek(fd , 0, 0);
		MfSdkFsWrite(fd, (char*)&g_commConfig, sizeof(struct comm_config));
		MfSdkFsClose(fd);				  // Close the file
	}
	else{
		APP_TRACE("File Open again Failed\r\n");
	}
	return ret;
}

void readCommConfigFile(int id, char *readData)
{
	int fp;
	fp = MfSdkFsOpenCreate((const s8*)COMM_CONFIG_FILE, FILE_PRIVATE, FILE_RDONLY, &fp, 0);
	if(fp == MFSDK_FS_RET_OK)
	{
		APP_TRACE("File Open Success\r\n");
		MfSdkFsLseek(fp , 0, MFSDK_FS_SEEK_SET);
        MfSdkFsRead(fp, (char*)&g_commConfig, sizeof(comm_config));
		MfSdkFsClose(fp);		  // Close the file
	}
	else{
		APP_TRACE("File Open again Failed\r\n");
	}
	if( fp == MFSDK_FS_RET_OK){
		switch (id)
		{
		case APN_ID:
			strcpy(readData,g_commConfig.apn);
			break;
		case CARD_PAY_IP_URL_ID:
			strcpy(readData,g_commConfig.ip1);
			break;
		case CARD_PAY_PORT_ID:
			strcpy(readData,g_commConfig.port1);
			break;
		case SERVICE_URL_ID:
			strcpy(readData,g_commConfig.ip2);
			break;
		case SERVICE_PORT_ID:
			strcpy(readData,g_commConfig.port2);
			break;
		case MQTT_URL_ID:
			strcpy(readData,g_commConfig.mqttUrl);
			break;
		case MQTT_PORT_ID:
			strcpy(readData,g_commConfig.mqttPort);
			break;
		case LANG_ID:
			sprintf(readData,"%d",g_commConfig.defaultLang);
			break;
		default:
			break;
		}
		APP_TRACE("===== Read comm Config file data === \r\n");
		APP_TRACE("APN :%s: Lang =:%d:\r\n",g_commConfig.apn,g_commConfig.defaultLang);
		APP_TRACE("IP 1 = %s, port 1= %s\r\n",g_commConfig.ip1,g_commConfig.port1);
		APP_TRACE("IP 2 = %s, port 2= %s\r\n",g_commConfig.ip2,g_commConfig.port2);
		APP_TRACE("mqttUrl = %s, port = %s\r\n",g_commConfig.mqttUrl,g_commConfig.mqttPort);

		APP_TRACE("===== End file data === \r\n");
	}
    else
    {
        //gui_messagebox_show( "Comm Config" , "File open or create fail" , "" , "confirm" , 0);
    }
}

void updateCommConfig(int id, char *writeData)
{
	// communication parameter will be updated
	int ret = -1;
	int fp;
	ret = MfSdkFsOpenCreate((const s8*)COMM_CONFIG_FILE, FILE_PRIVATE, FILE_RDWR, &fp, 0);
    if( ret == UFILE_SUCCESS)
    {
		MfSdkFsLseek(fp, 0, 0);	// seek 0 
		//memset(&g_commConfig,0x00,sizeof(comm_config));
        MfSdkFsRead(fp, (char*)&g_commConfig, sizeof(comm_config));                      // read 1 record
		switch (id)
		{
		case APN_ID:
			memset(g_commConfig.apn,0x00,sizeof(g_commConfig.apn));
			strcpy(g_commConfig.apn,writeData);
                        
			break;
		case CARD_PAY_IP_URL_ID:
			memset(g_commConfig.ip1,0x00,sizeof(g_commConfig.ip1));
			strcpy(g_commConfig.ip1,writeData);
			break;
		case CARD_PAY_PORT_ID:
			memset(g_commConfig.port1,0x00,sizeof(g_commConfig.port1));
			strcpy(g_commConfig.port1,writeData);
			break;
		case SERVICE_URL_ID:
			memset(g_commConfig.ip2,0x00,sizeof(g_commConfig.ip2));
			strcpy(g_commConfig.ip2,writeData);
			break;
		case SERVICE_PORT_ID:
			memset(g_commConfig.port2,0x00,sizeof(g_commConfig.port2));
			strcpy(g_commConfig.port2,writeData);
			break;
		case MQTT_URL_ID:
			memset(g_commConfig.mqttUrl,0x00,sizeof(g_commConfig.mqttUrl));
			strcpy(g_commConfig.mqttUrl,writeData);
			break;
		case MQTT_PORT_ID:
			memset(g_commConfig.mqttPort,0x00,sizeof(g_commConfig.mqttPort));
			strcpy(g_commConfig.mqttPort,writeData);
			break;
         case LANG_ID:
            APP_TRACE("Going to updated the Language :%s:\r\n",writeData);
            g_commConfig.defaultLang = atoi(writeData);
            break;
		default:
			break;
		}
		MfSdkFsLseek(fp, 0, 0);	// seek 0 
        ret = MfSdkFsWrite(fp, (char*)&g_commConfig, sizeof(comm_config));         // write to the file
		APP_TRACE("New Written inComm config file Size = :%d: \r\n",ret);
		MfSdkFsClose(fp);				// Close the file
		//memset(&g_commConfig,0x00,sizeof(comm_config));
		APP_TRACE("Sizeof comm file is :%d:\r\n",sizeof(comm_config));
		//g_commConfig = l_commConfig;
		APP_TRACE("===== Comm config file data === \r\n");
		APP_TRACE("APN is :%s: Play Language is :%d: \r\n",g_commConfig.apn,g_commConfig.defaultLang);
		APP_TRACE("IP 1 = %s, port 1= %s\r\n",g_commConfig.ip1,g_commConfig.port1);
		APP_TRACE("IP 2 = %s, port 2= %s\r\n",g_commConfig.ip2,g_commConfig.port2);
		APP_TRACE("mqttUrl = %s, port = %s\r\n",g_commConfig.mqttUrl,g_commConfig.mqttPort);
		APP_TRACE("===== End file data === \r\n");
//		readCommConfigFile(0,"");
	}
    else
    {
		//gui_messagebox_show( "Comm Config" , "File open or create fail" , "" , "confirm" , 0);
	}
}

int checkFileExist(char *fileName)
{
    int ret = -1;

//	int fp;
//	char buff[100];
//    ret = UFile_Check(fileName, FILE_PRIVATE);

	ret = MfSdkFsCheck((const s8 *) fileName, FILE_PRIVATE);

    if(ret == MFSDK_FS_RET_NOEXIST)
    {
        APP_TRACE("\n files %s not Exist\r\n", fileName);
        ret = -1;
    }
    else if(ret == MFSDK_FS_RET_OK)
    {
        APP_TRACE("\n files %s Exist\r\n", fileName);
        ret = 0;
    }
    else
    {
        APP_TRACE("\n files parameter Error %s \r\n", fileName);
    }
    return ret;
}

void setAPN(char *apn)
{
    updateCommConfig(APN_ID, apn);
}

