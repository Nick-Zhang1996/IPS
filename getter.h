#ifndef getter_h
#define getter_h
#include <stdio.h>
#include <pcap.h>
#include <math.h>
#include <sys/time.h>
#include "table.h"
#include <pthread.h>
struct getter_args{
	pcap_t* handle;
};

extern pthread_t getter_id;

pthread_t getter_listen();

int getter_init(char* dev);
void* getter(void* arg);
int getter_finish(void* arg);
int getter_stop();

#endif
