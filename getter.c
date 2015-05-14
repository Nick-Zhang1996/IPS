//getter, include function to catch packages and update RSSI table
#include "getter.h"
pcap_t* getter_handle;
void print_len(u_char *args, const struct pcap_pkthdr *header,const u_char *packet){
	fprintf(stderr,"%d\n",header->len);
}
unsigned char buffer[1100];
int buffer_c=0;
pthread_t getter_id;
float stat(){
	float sum=0;
	int i;
	for (i=0;i<buffer_c;i++){
		sum+=buffer[i];
	}
	float avg=sum/buffer_c;
	float helper=0;
	for (i=0;i<buffer_c;i++){
		helper+=(buffer[i]-avg)*(buffer[i]-avg);
	}

	float dev=sqrt(helper/buffer_c);
	printf("%d match,average: %5.3f standard deviation: %5.3f",buffer_c,avg,dev);
}
uint32_t count=0;
void count_mine(u_char *args, const struct pcap_pkthdr *header,const u_char *packet){
	u_char mybssid[6]={0xf4,0xec,0x38,0x3f,0x7b,0x7c};	
	//fprintf(stderr,"%u\n",++count);	
	int i;
	/*
	for(i=36;i<42;i++){
		fprintf(stderr,"%x:",*(packet+i));
	}
	fprintf(stderr,"RSSI %u ",*(packet+22));
	*/
	if(memcmp(mybssid,packet+36,6)==0){
		fprintf(stderr,"match\n");
		//record RSSI
		buffer[buffer_c++]=*(packet+22);
		}
	
	//fprintf(stderr,"\n");
}

//important stuff begins here
//initialize a pcap session on given dev, return -1 upon error 
int getter_init(char* dev){
	int verbose=1;
	char errbuf[PCAP_ERRBUF_SIZE];

	if (dev==NULL) {
		dev=pcap_lookupdev(errbuf);
		if (dev==NULL) {
			printf("cannot find default device: %s\n",errbuf);
			return -1;
		} else
			printf("use default interface: %s\n",dev);
	} else {
		printf("use interface:%s\n",dev);
	}


	pcap_t *handle;
	//sniff
	handle=pcap_open_live(dev,BUFSIZ,1,1000,errbuf);
	//DEBUG
	printf("handle created%d",handle);
	if (handle==NULL) {
		printf("Can't open device %s %s\n",dev,errbuf);
		return -1;
	} else {
		if (verbose) {
			printf("open device %s successfully\n",dev);
		}
	}
/*
	if (pcap_datalink(handle)!=DLT_EN10MB) {
		printf("Device %s does not provide Ethernet headers--not supported\n",dev);
		return 2;
	}
*/

	//set buffersize
	int rval=pcap_set_buffer_size(handle,4000);
	//apply filter to sniffing
	struct bpf_program fp;
	char filter_exp[]="subtype beacon";
	//char filter_exp[]="";
	if (pcap_compile(handle,&fp,filter_exp,0,0)==-1){
		printf("can't parse filter %s:%s\n",filter_exp,pcap_geterr(handle));
		return -1;
	}

	if(pcap_setfilter(handle,&fp)==-1){
		printf("can't install filter %s:%s\n",filter_exp,pcap_geterr(handle));
		return -1;
	}
	pcap_freecode(&fp);

	printf("pcap ready\n");
	getter_handle=handle;
	return 0;
}
void getter_update(u_char *args, const struct pcap_pkthdr *header,const u_char *packet){
	u_char mybssid[6];
	memcpy(mybssid,packet+36,6);
	uint8_t rssi=*(packet+22);
	//DEBUG
	//printf(".");

	update(mybssid,rssi,header->ts);
	return;
}

void* getter(void* arg){
	//DEBUG
	printf("listening...\n");

	//pcap_t* handle=((struct getter_args*)arg)->handle;
	pcap_t* handle=getter_handle;
	printf("handle when listening%d",handle);
	pcap_loop(handle,-1,getter_update,NULL);

	//pcap_loop(handle,-1,count_mine,NULL);
	printf("listening stopped\n");
	//pcap_perror(handle,"pcap:");
	return 0;
}

pthread_t getter_listen(){
	pthread_t myid;
	int rval=pthread_create(&myid,NULL,getter,NULL);

	if (rval!=0){perror("fail creating getter thread"); return -1;}
	return myid;
}

int getter_finish(void* arg){
	//pcap_t* handle=((struct getter_args*)arg)->handle;
	pcap_t* handle=getter_handle;
	pcap_close(handle);
	return 0;
}

int getter_stop(){
	pcap_breakloop(getter_handle);
	//wait until listening is truly stopped
	pthread_join(getter_id,NULL);
	//now we return
	return 0;
}
