#include "table.h"
#include "getter.h"
#include "fin.h"
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
uint8_t flag_exit=0;
void ctrl_c(int num){
	//print statistic results
	struct pcap_stat stat;
	pcap_stats(getter_handle,&stat);
	printf("received %d packages\n",stat.ps_recv);
	printf("%d dropped for lack of buffer space\n",stat.ps_drop);
	printf("%d dropped by network interface or driver\n",stat.ps_ifdrop);
	pcap_breakloop();
	flag_exit=1;
	return;

}
//delay time milliseconds
void delay(const long time){
	struct timespec delaytime;
	delaytime.tv_sec=time/1000;
	delaytime.tv_nsec=(time%1000)*1000000;
	nanosleep(&delaytime,NULL);
	return;
}

//test functions
//
//create a fin file
int create_test(){
//create a new fin file
	fin_new("test1.fin");
	while(1){
		printf("input any number to record this position:\n0 to exit");
		int temp;
		scanf("%d\n",&temp);
		printf("temp=%d\n",temp);
		if(temp==0){
			fin_end();
		} else {
			table_to_file();
		}
	}

	return 0;
}
int main(int argc,char** argv){
	int rval=0;
	//initialize rssi table 
	initialize();
	//prepare pcap
	rval=getter_init("mon0");
	struct getter_args g_arg;
	if (rval==-1){
		return 1;
	} else{
		//keep listening and updating
		getter_id=getter_listen();
		if(getter_id==-1){
			return 1;
		}
	}	
		
		
	//print signal using ncurses
//below are tester 
//find my AP
/*
	u_char nicksbssid[6]={0xf4,0xec,0x38,0x3f,0x7b,0x7c};	
	int i=0;
	float nicksrssi;
	while(1){
		nicksrssi=get_rssi(nicksbssid);
		if(nicksrssi==0||nicksrssi==-1){
			//printf("getrssifail\nnickrssi=%.0f\n",nicksrssi);

			//WARNING: VERY BUGGY!!!
			//getter_finish(&g_arg);
			//return 1;
		}else{
			printf("HEY I find our AP!!!!%.2f\n",nicksrssi);
		}
		delay(100);
	}
	return 0;
	*/
	create_test();
	while(1){
		if(flag_exit){
			return;
		}
	}
	return 0;

}
