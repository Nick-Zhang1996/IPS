#ifndef table_h
#define table_h
//RSSI table manager
//this header includes essential structures and functions to store,manipulate and access the RSSI information of various BSSIDs
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#define VALID_INTERVAL 3
#define MAX_ENTRY 500
struct rssi_table{
	uint8_t used;//0-empty,1-used
	uint8_t bssid[6];
	uint8_t rssi[10];//most-recent 10 rssi
	uint8_t p;//tracks the one to be replaced
	struct timeval ts;//timestamp !!!potential overflow
};

extern struct rssi_table table[MAX_ENTRY];
extern struct timeval present;
extern pthread_mutex_t table_mutex;
int initialize();
//return 1 if this bssid is already recorded,0 if not
int get_index(uint8_t *bssid);
//return 1 if this bssid is blocked(mobile phone AP)
int isinblocklist(uint8_t *bssid);
//create a new entry
int create(uint8_t *bssid);
//update one signal
int update(uint8_t *bssid,uint8_t signal,struct timeval ts);
//get weighed rssi,0 if outdated,-1 if no such entry
float get_rssi(uint8_t *bssid);
//get rssi based on index,return -1 on no entry
//makesure index is used
float get_rssi_index(int index);
//get rssi based on pointer,return -1 on no entry
float get_rssi_raw(struct rssi_table* p);



#endif
