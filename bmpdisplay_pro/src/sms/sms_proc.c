#include <stdint.h>
#include <stdlib.h>
#include "../tracedef.h"
#include "sms_proc.h"


ql_task_t sms_task = NULL;
ql_sem_t  sms_init_sem = NULL;
ql_sem_t  sms_list_sem = NULL;

void user_sms_event_callback(uint8_t nSim, int event_id, void *ctx)
{
	switch(event_id)
	{
		case QL_SMS_INIT_OK_IND:
		{
			APP_TRACE("QL_SMS_INIT_OK_IND\r\n");
			ql_rtos_semaphore_release(sms_init_sem);
			break;
		}
		case QL_SMS_NEW_MSG_IND:
		{
			uint16_t msg_len = 512;
			char *text = malloc(msg_len);
			ql_sms_new_s *msg = (ql_sms_new_s *)ctx;
			APP_TRACE("sim=%d, index=%d, storage memory=%d\r\n", nSim, msg->index, msg->mem);
			int ret = ql_sms_read_msg(0,msg->index, text, msg_len, TEXT);
			APP_TRACE("ql_sms_read_msg = %d\r\n", ret);
			if(QL_SMS_SUCCESS == ret){
				APP_TRACE("read msg OK, msg=%s\r\n", text);
			}else{
				APP_TRACE("read sms FAIL %d\r\n", ret);
			}
			free(text);
			break;
		}
		case QL_SMS_LIST_IND:
		{
#if 1
			ql_sms_msg_s *msg = (ql_sms_msg_s *)ctx;
			APP_TRACE("sim=%d,index=%d, msg = %s\r\n",nSim, msg->index, msg->buf);
#endif
			break;
		}
        case QL_SMS_LIST_EX_IND:
        {
            ql_sms_recv_s *msg = (ql_sms_recv_s *)ctx;
            APP_TRACE("index=%d,os=%s,tooa=%u,status=%d,fo=0x%x,dcs=0x%x,scst=%d/%d/%d %d:%d:%d±%d,uid=%u,total=%u,seg=%u,dataLen=%d,data=%s\r\n",
                msg->index,msg->oa,msg->tooa,msg->status,msg->fo,msg->dcs,
                msg->scts.uYear,msg->scts.uMonth,msg->scts.uDay,msg->scts.uHour,msg->scts.uMinute,msg->scts.uSecond,msg->scts.iZone,
                msg->uid,msg->msg_total,msg->msg_seg,msg->dataLen,msg->data);
            break;
        }
		case QL_SMS_LIST_END_IND:
		{
			APP_TRACE("QL_SMS_LIST_END_IND\r\n");
			ql_rtos_semaphore_release(sms_list_sem);
			break;
		}
		case QL_SMS_MEM_FULL_IND:
		{
			ql_sms_new_s *msg = (ql_sms_new_s *)ctx;
			APP_TRACE("QL_SMS_MEM_FULL_IND sim=%d, memory=%d\r\n",nSim,msg->mem);
			break;
		}
		default :
			break;
	}
}


void sms_demo_task(void * param)
{
	char addr[20] = {0};
	int ret = 0;
	uint8_t nSim = 0;
	APP_TRACE("enter\r\n");
	ql_sms_callback_register(user_sms_event_callback);
	
	//wait sms ok
	if(ql_rtos_semaphore_wait(sms_init_sem, QL_WAIT_FOREVER)){
		APP_TRACE("Waiting for SMS init timeout\r\n");
	}
	ret = ql_sms_get_center_address(nSim, addr, sizeof(addr));
	if(QL_SMS_SUCCESS == ret){
		APP_TRACE("ql_sms_get_center_address OK, addr=%s\r\n",addr);
	}else{
		APP_TRACE("ql_sms_get_center_address FAIL %d\r\n", ret);
	}
    
	ql_sms_set_code_mode(QL_CS_GSM);
    
#if 0

    //Send English text message
	ret = ql_sms_send_msg(nSim,"+8610000","~!@#$%^&*()_+<>?:{}|", GSM);
	if(QL_SMS_SUCCESS == ret){
		QL_SMS_LOG("ql_sms_send_msg OK");
	}else{
		QL_SMS_LOG("ql_sms_send_msg FAIL %d", ret);
	}
    


	//Send messages in Chinese and English. (Need use UTF8 encoding to open sms_demo.c for chinese.)
	ret = ql_sms_send_msg(nSim,"+8610000","hello,你好", UCS2);
	if(QL_SMS_SUCCESS == ret){
		QL_SMS_LOG("ql_sms_send_msg OK");
	}else{
		QL_SMS_LOG("ql_sms_send_msg FAIL %d", ret);
	}
#endif
	
	//Get how many SMS messages can be stored in the SIM card in total and how much storage is used
#if 0
	ql_sms_stor_info_s stor_info;
	ret = ql_sms_get_storage_info(nSim,&stor_info);
	if(QL_SMS_SUCCESS == ret){
		QL_SMS_LOG("ql_sms_get_storage_info OK");
		QL_SMS_LOG("SM used=%u,SM total=%u,SM unread=%u,ME used=%u,ME total=%u,ME unread=%u, newSmsStorId=%u",
			stor_info.usedSlotSM,stor_info.totalSlotSM,stor_info.unReadRecordsSM,
			stor_info.usedSlotME,stor_info.totalSlotME,stor_info.unReadRecordsME,
			stor_info.newSmsStorId);
	}else{
		QL_SMS_LOG("ql_sms_get_storage_info FAIL %d", ret);
	}
#endif
while(1)
{
	APP_TRACE("======sms_demo_task=====\r\n");
    //The first parameter specifies that SMS messages are read from SM
    ql_sms_set_storage(nSim,SM,SM,SM);
	//Read one messages in SIM
#if 0
	uint16_t msg_len = 512;
	ql_sms_mem_info_t sms_mem = {0};
	ql_sms_recv_s *sms_recv = NULL;
	
	char *msg = malloc(msg_len);
	if(msg == NULL){
		APP_TRACE("malloc ql_sms_msg_s fail\r\n");
		//goto exit;
	}
	memset(msg ,0 ,msg_len);

	ql_sms_get_storage(nSim, &sms_mem);
	APP_TRACE("mem1=%d, mem2=%d, mem3=%d\r\n", sms_mem.mem1, sms_mem.mem2, sms_mem.mem3);
    
	//Read SMS messages as text
	ret = ql_sms_read_msg(nSim,2, msg, msg_len, TEXT);
	APP_TRACE("ql_sms_read_msg = %d\r\n", ret);
	if(QL_SMS_SUCCESS == ret){
		APP_TRACE("read msg OK, msg=%s\r\n", msg);
	}else{
		APP_TRACE("read sms FAIL %d\r\n", ret);
	}
	
	//Read SMS messages as pdu
	memset(msg ,0 ,msg_len);
	ret = ql_sms_read_msg(nSim,2, msg, msg_len, PDU);
	if(QL_SMS_SUCCESS == ret){
		APP_TRACE("read msg OK, msg=%s", msg);
	}else{
		APP_TRACE("read sms FAIL %d\r\n", ret);
	}
    if(msg)free(msg);

    //Read SMS messages as text
    sms_recv = (ql_sms_recv_s *)calloc(1,sizeof(ql_sms_recv_s));
    if(sms_recv == NULL)
    {
        APP_TRACE("calloc FAIL\r\n");
        //goto exit;
    }
	ret = ql_sms_read_msg_ex(nSim,2, TEXT,sms_recv);
	if(QL_SMS_SUCCESS == ret){
        APP_TRACE("index=%d,os=%s,tooa=%u,status=%d,fo=0x%x,dcs=0x%x,scst=%d/%d/%d %d:%d:%d±%d,uid=%u,total=%u,seg=%u,dataLen=%d,data=%s\r\n",
            sms_recv->index,sms_recv->oa,sms_recv->tooa,sms_recv->status,sms_recv->fo,sms_recv->dcs,
            sms_recv->scts.uYear,sms_recv->scts.uMonth,sms_recv->scts.uDay,sms_recv->scts.uHour,sms_recv->scts.uMinute,sms_recv->scts.uSecond,sms_recv->scts.iZone,
            sms_recv->uid,sms_recv->msg_total,sms_recv->msg_seg,sms_recv->dataLen,sms_recv->data);

	}else{
		APP_TRACE("read sms FAIL %d\r\n", ret);
	}
	if(sms_recv)free(sms_recv);

	ql_rtos_task_sleep_ms(100);
#endif
}
	//Read all message in SIM
#if 0
	ql_sms_set_storage(nSim,SM,SM,SM);//set sms storage as SIM.
	ret = ql_sms_read_msg_list(nSim, TEXT);
	if(QL_SMS_SUCCESS == ret){
		if(ql_rtos_semaphore_wait(sms_list_sem, QL_WAIT_FOREVER)){
			QL_SMS_LOG("sms_list_sem time out");
		}
	}else{
		QL_SMS_LOG("get msg list FAIL %d", ret);
	}
#endif
	
	//Delete message.
#if 0
	if(QL_SMS_SUCCESS == ql_sms_delete_msg_ex(nSim, 0, QL_SMS_DEL_ALL)){
		QL_SMS_LOG("delete msg OK");
	}else{
		QL_SMS_LOG("delete sms FAIL");
	}
#endif
// 	goto exit;
// exit:

// 	ql_rtos_task_delete(NULL);
}


QlOSStatus ql_sms_app_init(void)
{
	QlOSStatus err = QL_OSI_SUCCESS;
	APP_TRACE("ql_sms_app_init \r\n");
	err = ql_rtos_task_create(&sms_task, 4096, APP_PRIORITY_NORMAL, "QsmsApp", sms_demo_task, NULL, 2);
	if(err != QL_OSI_SUCCESS)
	{
		APP_TRACE("sms_task created failed, ret = 0x%x\r\n", err);
	}
	
	err = ql_rtos_semaphore_create(&sms_init_sem, 0);
	if(err != QL_OSI_SUCCESS)
	{
		APP_TRACE("sms_init_sem created failed, ret = 0x%x\r\n", err);
	}

	err = ql_rtos_semaphore_create(&sms_list_sem, 0);
	if(err != QL_OSI_SUCCESS)
	{
		APP_TRACE("sms_init_sem created failed, ret = 0x%x\r\n", err);
	}

	return err;
}

