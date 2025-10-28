#pragma once
#include "mqtt_embed\MQTTClient\src\mf\MQTTMf.h"



void NetworkInit(Network* n);
int NetworkConnect(Network* n, char *ip, int port);
int NetworkConnect_ssl(Network* n, char *addr, int port, char *cacert ,char *clientcert , char *clientkey);
void NetworkDisconnect(Network* n);