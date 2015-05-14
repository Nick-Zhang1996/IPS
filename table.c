//RSSI table manager

#include "table.h"
// this should be in global region
struct rssi_table table[MAX_ENTRY];
struct timeval present;
pthread_mutex_t table_mutex=PTHREAD_MUTEX_INITIALIZER;

int initialize(){
	
	pthread_mutex_lock(&table_mutex);
	memset(&present,0,sizeof(struct timeval));
	memset(table,0,sizeof(struct rssi_table)*MAX_ENTRY);
	pthread_mutex_unlock(&table_mutex);
	return 0;
}

//return index if this bssid is already recorded,-1 if not
int get_index(uint8_t *bssid){
	int i;
	for(i=0;i<50;i++){
		if(table[i].used==1&&memcmp(bssid,&table[i].bssid,6)==0){
			return i;
		}
	}
	
	return -1;
}

//return 1 if this bssid is blocked(mobile phone AP)
int isinblocklist(uint8_t *bssid){
	return 0;
}
//create a new entry
int create(uint8_t *bssid){
	int i,index;
	//DEBUG
	printf("creating entry for");
	for (i=0;i<5;i++){
		printf("%X:",*(bssid+i));
	}
	printf("%X\n",*(bssid+5));


	for (i=0;i<50;i++){
		if(table[i].used==0){
			table[i].used=1;
			memcpy(table[i].bssid,bssid,6);
			return i;
		}
	}
	//no available space,should inc table array size
	printf("create entry failed!\n");
	return -1;
}
//update one signal
int update(uint8_t *bssid,uint8_t signal,struct timeval ts){
	int index=get_index(bssid);
	if(index==-1){
		index=create(bssid);
	}

	pthread_mutex_lock(&table_mutex);
	table[index].rssi[table[index].p++]=signal;
	table[index].p%=10;
	table[index].ts=ts;
	present=ts;
	pthread_mutex_unlock(&table_mutex);
	return 0;
}
//get weighed rssi,0 if outdated,-1 if no such entry
float get_rssi(uint8_t *bssid){
	int index=get_index(bssid);
	if(index==-1){return -1;}
	return get_rssi_index(index);

}
//get rssi based on index,return -1 on no entry
//this function does not check table[index].used
float get_rssi_index(int index){
	float calc=0.0;
	pthread_mutex_lock(&table_mutex);
	if(present.tv_sec-table[index].ts.tv_sec>VALID_INTERVAL){
		return 0;
	}
	int i;
	for(i=0;i<10;i++){
		calc+=table[index].rssi[i];
	}
	pthread_mutex_unlock(&table_mutex);
	return calc/10;
}
//get rssi based on pointer
float get_rssi_raw(struct rssi_table* p){
	pthread_mutex_lock(&table_mutex);
	if(present.tv_sec-p->ts.tv_sec>VALID_INTERVAL){
		return 0;
	}
	if(p->used==0){return -1;}
	float calc=0.0;
	int i;
	for(i=0;i<10;i++){
		calc+=p->rssi[i];
	}
	pthread_mutex_unlock(&table_mutex);
	return calc/10;
}
//create an fingerprint 
//tester
/*
int main(){

	initialize();
	struct timeval ts;
	ts.tv_sec=10;
	ts.tv_usec=0;
	uint8_t mybssid[6]={0xf4,0xec,0x38,0x3f,0x7b,0x7c};	
	uint8_t urbssid[6]={0x33,0xec,0x38,0x3f,0x7b,0x7c};	
	uint8_t hsbssid[6]={0x33,0xec,0x34,0x3f,0x7b,0x7c};	
	int i;
	int ran[13]={1,2,3,4,5,6,7,8,9,10,11,12,13};
	for(i=0;i<13;i++){
		update(mybssid,ran[i],ts);
	}
	
	printf("updated mybssid with 13 values,should give 8.5\n");
	printf("get_rssi(mybssid)=%.2f\n",get_rssi(mybssid));
	
	printf("get_rssi(urssid),should give -1(no entry)\n");
	printf("get_rssi(urbssid)=%.2f\n",get_rssi(urbssid));

	ts.tv_sec+=4;
	for(i=0;i<13;i++){
		update(urbssid,ran[i]+1,ts);
	}

	printf("updated urbssid with 13 values,should give 9.5\n");
	printf("get_rssi(urbssid)=%.2f\n",get_rssi(urbssid));
	
	printf("get_rssi(myssid),should give 0(outdated)\n");
	printf("get_rssi(mybssid)=%.2f\n",get_rssi(mybssid));
	return 0;
}
*/
