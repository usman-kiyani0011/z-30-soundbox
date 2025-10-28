#ifndef __SMS_PROC_H__
#define __SMS_PROC_H__
#include "ql_api_common.h"
#include "ql_api_osi.h"
#include "ql_api_sms.h"

#if 0
typedef void * ql_task_t;
typedef void* ql_sem_t;

typedef enum
{
	QL_SMS_INIT_OK_IND = 1 | (QL_COMPONENT_SMS << 16),
	QL_SMS_NEW_MSG_IND,
	QL_SMS_LIST_IND,
	QL_SMS_LIST_END_IND,
	QL_SMS_LIST_EX_IND,
	QL_SMS_MEM_FULL_IND,
}ql_sms_event_id_e;

typedef enum
{
	ME = 1, //Mobile Equipment message storage
	SM = 2,//SIM message storage
}ql_sms_stor_e;

typedef struct
{
    ql_sms_stor_e mem1; //messages to be read and deleted from this memory storage.
    ql_sms_stor_e mem2; //messages will be written and sent to this memory storage.
    ql_sms_stor_e mem3; //received messages will be placed in this memory storage if routing to PC is not set.
}ql_sms_mem_info_t;
#endif
/***************************  task priority defination   ******************************/
typedef enum
{
	APP_PRIORITY_IDLE = 1, // reserved
    APP_PRIORITY_LOW = 4,
    APP_PRIORITY_BELOW_NORMAL = 8,
    APP_PRIORITY_NORMAL = 12,
    APP_PRIORITY_ABOVE_NORMAL = 16,
    APP_PRIORITY_HIGH = 25,
    APP_PRIORITY_REALTIME = 30  
}APP_ThreadPriority_e;



/****************************  error code about osi    ***************************/
typedef enum
{
	QL_OSI_SUCCESS             =     0,
	
	QL_OSI_TASK_PARAM_INVALID  =	 1 | (QL_COMPONENT_OSI << 16),
	QL_OSI_TASK_CREATE_FAIL,   
	QL_OSI_NO_MEMORY,
	QL_OSI_TASK_DELETE_FAIL,
	QL_OSI_TASK_PRIO_INVALID,
	QL_OSI_TASK_NAME_INVALID,
	QL_OSI_TASK_EVENT_COUNT_INVALID,
	QL_OSI_INVALID_TASK_REF,
	QL_OSI_TASK_CNT_REACH_MAX,
	QL_OSI_TASK_BIND_EVENT_FAIL,
	QL_OSI_TASK_UNBIND_EVENT_FAIL,
	QL_OSI_TASK_GET_REF_FAIL,
	QL_OSI_TASK_GET_PRIO_FAIL,
	QL_OSI_TASK_SET_PRIO_FAIL,
	QL_OSI_TASK_GET_STATUS_FAIL,
	QL_OSI_TASK_HAS_BINDED_TIMER,

	QL_OSI_QUEUE_CREATE_FAIL	=   50 | (QL_COMPONENT_OSI << 16), 
	QL_OSI_QUEUE_DELETE_FAIL,
	QL_OSI_QUEUE_IS_FULL,
	QL_OSI_QUEUE_RELEASE_FAIL,
	QL_OSI_QUEUE_RECEIVE_FAIL,
	QL_OSI_QUEUE_GET_CNT_FAIL,
	QL_OSI_QUEUE__FAIL,

	QL_OSI_SEMA_CREATE_FAILE    =  100 | (QL_COMPONENT_OSI << 16), 
	QL_OSI_SEMA_DELETE_FAIL,
	QL_OSI_SEMA_IS_FULL,
	QL_OSI_SEMA_RELEASE_FAIL,
	QL_OSI_SEMA_GET_FAIL,
	QL_OSI_SEMA__FAIL,

	QL_OSI_MUTEX_CREATE_FAIL	=  150 | (QL_COMPONENT_OSI << 16), 
	QL_OSI_MUTEX_DELETE_FAIL,
	QL_OSI_MUTEX_LOCK_FAIL,
	QL_OSI_MUTEX_UNLOCK_FAIL,

	QL_OSI_EVENT_SEND_FAIL		=  200 | (QL_COMPONENT_OSI << 16),
	QL_OSI_EVENT_GET_FAIL,
	QL_OSI_EVENT_REGISTER_FAIL,

	QL_OSI_TIMER_CREATE_FAIL	=  250 | (QL_COMPONENT_OSI << 16),
	QL_OSI_TIMER_START_FAIL,
	QL_OSI_TIMER_STOP_FAIL,
	QL_OSI_TIMER_DELETE_FAIL,
	QL_OSI_TIMER_BIND_TASK_FAIL,

	QL_OSI_SWDOG_REGISTER_FAIL  =  300 | (QL_COMPONENT_OSI << 16),
	QL_OSI_SWDOG_UNREGISTER_FAIL,
	QL_OSI_SWDOG_FEED_DOG_FAIL,
	QL_OSI_SWDOG_ENABLE_FAIL,
	QL_OSI_SWDOG_DISABLE_FAIL,

}osi_errcode_e;
#endif
