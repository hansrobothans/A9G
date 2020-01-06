#include <string.h>
#include <stdio.h>
#include <api_os.h>
#include <api_event.h>
#include <api_network.h>
#include <api_debug.h>
#include <api_socket.h>

#define MAIN_TASK_STACK_SIZE    (2048 * 2)
#define MAIN_TASK_PRIORITY      0
#define MAIN_TASK_NAME          "Socket Test Task"

#define SECOND_TASK_STACK_SIZE    (2048 * 2)
#define SECOND_TASK_PRIORITY      1
#define SECOND_TASK_NAME          "Second Test Task"

static HANDLE network3TaskHandle = NULL;
static HANDLE secondTaskHandle = NULL;

void network_attach_activate_handle()
{
	uint8_t status; 		
	bool ret = Network_GetAttachStatus(&status);
	if(!ret){
		Trace(1,"get attach staus failed");
		return;
	}
	Trace(1,"attach status:%d",status);
	if(0 == status){
		ret = Network_StartAttach();
		if(!ret){
			Trace(1,"network attach failed");
			return;
		}
	}else{
		ret = Network_GetActiveStatus(&status);
		if(!ret){
			Trace(1,"get active staus failed");
			return;
		}
		
		Trace(1,"active status:%d",status);
		if(0 == status){
			Network_PDP_Context_t context = {
				.apn		="cmnet",
				.userName	= "",
				.userPasswd = ""
			};
			Network_StartActive(context);
		}
	}
}

static void network_deattached()
{
	Trace(1,"Network_StartDetach");
	Network_StartDetach();
}

//Network_StartDeactive(uint8_t contextID); 
//contextID���ʹ��??

static void network_deactived()
{
	Trace(1,"Network_StartDeactive");
	Network_StartDeactive(0);
}

void EventDispatch(API_Event_t* pEvent)
{
    switch(pEvent->id)
    {
        case API_EVENT_ID_NO_SIMCARD:
            Trace(1,"!!NO SIM CARD%d!!!!",pEvent->param1);
            break;
        
        case API_EVENT_ID_SIMCARD_DROP:
            Trace(1,"SIM CARD%d DROP",pEvent->param1);
            break;
        
        case API_EVENT_ID_SIGNAL_QUALITY:
            Trace(1,"signal quality:%d",pEvent->param1);
            break;
        
        case API_EVENT_ID_NETWORK_REGISTERED_HOME:
        case API_EVENT_ID_NETWORK_REGISTERED_ROAMING:			
            Trace(1,"network register success,id:%d",pEvent->id);
        	network_attach_activate_handle();
			break;
        case API_EVENT_ID_NETWORK_REGISTER_SEARCHING:
            Trace(1,"network register searching");
            break;
        
        case API_EVENT_ID_NETWORK_REGISTER_DENIED:
            Trace(1,"network register denied");
            break;

        case API_EVENT_ID_NETWORK_REGISTER_NO:
            Trace(1,"network register no");
            break;
        
        case API_EVENT_ID_NETWORK_DETACHED:
            Trace(1,"network detached");			
        	network_attach_activate_handle();
            break;
        
        case API_EVENT_ID_NETWORK_ATTACH_FAILED:
            Trace(1,"network attach failed");
            break;

        case API_EVENT_ID_NETWORK_ATTACHED:
            Trace(1,"network attach success");
            Network_PDP_Context_t context = {
                .apn        ="cmnet",
                .userName   = ""    ,
                .userPasswd = ""
            };
            Network_StartActive(context);
            break;

        case API_EVENT_ID_NETWORK_DEACTIVED:
            Trace(1,"network deactived");			
        	network_attach_activate_handle();
            break;
        
        case API_EVENT_ID_NETWORK_ACTIVATE_FAILED:
            Trace(1,"network activate failed");
            network_attach_activate_handle();
            break;
        
        case API_EVENT_ID_NETWORK_ACTIVATED:
            Trace(1,"network activate success");
			OS_Sleep(3000);
			//network_deattached();
            break;

        case API_EVENT_ID_NETWORK_GOT_TIME:
            //Trace(1,"network got time");
            break;

        default:
            break;
    }
}

//断一次网络
void SecondTask(void *pData)
{
    char ip[16];
    int i = 0;
    for(i=0;i<20;i++){
            OS_Sleep(1000);
        }
    network_deactived();
    network_deattached();
    while(1)
    {
        OS_Sleep(20000);
        Trace(1,"HELLO WORLD");
        network_attach_activate_handle();
    }
}

void network3_MainTask(void *pData)
{
    API_Event_t* event = NULL;
    secondTaskHandle = OS_CreateTask(SecondTask,
        NULL, NULL, SECOND_TASK_STACK_SIZE, SECOND_TASK_PRIORITY, 0, 0, SECOND_TASK_NAME);
    while(1)
    {
        if(OS_WaitEvent(network3TaskHandle, (void**)&event, OS_TIME_OUT_WAIT_FOREVER))
        {
            EventDispatch(event);
            OS_Free(event->pParam1);
            OS_Free(event->pParam2);
            OS_Free(event);
        }
    }
}


void network3_Main(void)
{
    network3TaskHandle = OS_CreateTask(network3_MainTask ,
        NULL, NULL, MAIN_TASK_STACK_SIZE, MAIN_TASK_PRIORITY, 0, 0, MAIN_TASK_NAME);
    OS_SetUserMainHandle(&network3TaskHandle);
}
