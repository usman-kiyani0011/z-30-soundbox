#pragma once
#include <stdbool.h>
typedef struct {
	char** data;
	int front;
	int rear;
	bool isEmpty;
} CharPointerQueue;
#define DATA_SIZE (30)
#define PTR_SIZE sizeof(void *)
#define MSG_NUM_MAX (201)
#define FIFO_SIZE (MSG_NUM_MAX * PTR_SIZE)


int fifo_init();
void fifo_set_zero();
bool fifo_checkEmpty();
bool fifo_checkFull();
int fifo_put(unsigned char** data);
int fifo_get(char **outData);

int fifo_key_init();
void fifo_key_set_zero();
bool fifo_key_checkEmpty();
bool fifo_key_checkFull();
int fifo_key_put(unsigned char* data);
int fifo_key_get(char *outData);
int fifo_get_last_one(char* outData);

