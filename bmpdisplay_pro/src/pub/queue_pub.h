#pragma once
#include <stdbool.h>
typedef struct {
	char** data;
	int front;
	int rear;
	bool isEmpty;
} CharPointerQueue;
#define FIFO_SIZE 2048
#define DATA_SIZE 50


int fifo_init();
void fifo_set_zero();
int fifo_put(unsigned char* data, int n_len);

CharPointerQueue* get_tts_queue();
CharPointerQueue* get_iotrevert_queue();
void fifo_init_ptr();
int  fifo_get_last_one(char* outData);
bool _fifo_put_ptr(CharPointerQueue* queue, char* item);
bool fifo_put_ptr(char* item);

char* _fifo_get_ptr(CharPointerQueue* queue);
char* fifo_get_ptr();

bool _fifo_checkEmpty_ptr(CharPointerQueue* queue);
bool fifo_checkEmpty_ptr();

void _fifo_setEmpty_ptr(CharPointerQueue* queue);
void fifo_setEmpty_ptr();

void _fifo_cleanup_ptr(CharPointerQueue* queue);
void fifo_cleanup_ptr();