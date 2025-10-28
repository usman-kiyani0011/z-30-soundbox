#include "tracedef.h"
#include "mqtt_wrapper.h"
#include "libapi_xpos/inc/libapi_system.h"
#include "libapi_xpos/inc/mfsdk_comm.h"

static int mf_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
    return MfSdkCommSocketRecv(n->my_socket, buffer, len, timeout_ms);
}

static int mf_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
    return MfSdkCommSocketSend(n->my_socket, buffer, len);
}

void NetworkInit(Network* n)
{
    n->mqttread = mf_read;
    n->mqttwrite = mf_write;
}

int NetworkConnect(Network* n, char *ip, int port)
{
    int ret;

    APP_TRACE("NetworkConnect>>>>\r\n");
    ret = MfSdkCommNetLink("", "", 5000);
	APP_TRACE("MfSdkCommNetLink:%d\r\n",ret);
    if(ret < 0) { return -1; }
    n->my_socket = MfSdkCommSocketCreate(2);
    APP_TRACE("comm_sock_create>>>>%d\r\n", n->my_socket);
    ret = MfSdkCommSocketConnect(n->my_socket, (s8*)ip, port, 20 * 1000, NULL);
    if ( ret != 0)
    {
        MfSdkCommSocketClose(n->my_socket);
    }
    return ret;
}

int NetworkConnect_ssl(Network* n, char *addr, int port, char *cacert, char *clientcert, char *clientkey)
{
    int ret = -1;

    n->my_socket = MfSdkCommSocketCreate(1);

    MfSdkCommSslAuthMode(n->my_socket, 1);
    MfSdkCommSslInit(n->my_socket, cacert, clientcert, clientkey, 1);
    ret = MfSdkCommSslConnect(n->my_socket, addr, port, (void*)0);

    if(ret != 0) { MfSdkCommSocketClose(n->my_socket); }
    return ret;
}

void NetworkDisconnect(Network* n)
{
    MfSdkCommSocketClose(n->my_socket);
}

