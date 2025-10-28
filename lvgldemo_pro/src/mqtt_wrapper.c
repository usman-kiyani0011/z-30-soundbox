#include "tracedef.h"
#include "mqtt_wrapper.h"
#include "libapi_xpos/inc/mfsdk_sys.h"
#include "libapi_xpos/inc/mfsdk_comm.h"

static int mf_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	int ret = 0;
	ret = MfSdkCommSocketRecv(n->my_socket, buffer, len, timeout_ms);
	//APP_TRACE_BUFF_TIP(buffer, len, "mf_read:");
	return ret;
}

static int mf_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	//APP_TRACE_BUFF_TIP(buffer, len, "mf_write:");
    return MfSdkCommSocketSend(n->my_socket, buffer, len);
}

void NetworkInit(Network* n)
{
    n->mqttread = mf_read;
    n->mqttwrite = mf_write;
}

int NetworkConnect(Network* n, char *ip, int port)
{
    int ret = -1;
    int tick = -1;
	
	NetworkInit(n);

    APP_TRACE("NetworkConnect>>>>\r\n");
    ret = MfSdkCommNetLink(NULL, "", 5000);
	APP_TRACE("MfSdkCommNetLink:%d\r\n",ret);
    if(ret < 0) { return -1; }
    n->my_socket = MfSdkCommSocketCreate(MFSDK_COMM_SOCKET_INDEX_1);
    APP_TRACE("comm_sock_create>>>>%d\r\n", n->my_socket);
    tick = MfSdkSysGetTick();
    ret = MfSdkCommSocketConnect(n->my_socket, (s8*)ip, (s32)port, 20 * 1000, NULL);
    APP_TRACE("[Latency] MfSdkCommSocketConnect:%d\r\n", MfSdkSysGetTick()-tick);
    if ( ret != 0)
    {
        MfSdkCommSocketClose(n->my_socket);
		n->my_socket = -1;
    }
    return ret;
}

int NetworkConnect_ssl(Network* n, char *addr, int port, char *cacert, char *clientcert, char *clientkey)
{
    int ret = -1;
	
	NetworkInit(n);

    n->my_socket = MfSdkCommSocketCreate(MFSDK_COMM_SOCKET_INDEX_1);

    MfSdkCommSslAuthMode(n->my_socket, 1);
    MfSdkCommSslInit(n->my_socket, cacert, clientcert, clientkey, 1);
    ret = MfSdkCommSocketConnect(n->my_socket, (s8*)addr, port, 30*1000, (void*)0);

    if(ret != 0) 
	{
		MfSdkCommSocketClose(n->my_socket);
		n->my_socket = -1;
	}
    return ret;
}

void NetworkDisconnect(Network* n)
{
	if(n->my_socket >= 0) 
	{
		APP_TRACE("NetworkDisconnect my_socket = %d\r\n", n->my_socket);
		MfSdkCommSocketClose(n->my_socket); 
	}
	n->my_socket = -1;
}

