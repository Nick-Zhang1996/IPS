//structures and functions to manipulate a fingerprint file
//a fingerprint file contans entries, which are made up of REC_NUM records
//each record is a struct fin_record and contains one single BSSID and RSSI(caled)
//each entry represents a position
//up to now ,entries and records are of consistent length
#include "fin.h"

//current fingerprint file fd
int fin_fd;
//current entries in total
size_t entry_count;
//create a new fingerprint file
//return file descripter on success,-1 on error
int fin_new(char* filename){
	mode_t mode=S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;
	int fd=open(filename,O_RDWR|O_CREAT|O_EXCL,mode);
	if (fd==-1){
		perror("create fin:");
		return -1;
	}
//may add a header,dont know what to include
//
	fin_fd=fd;
	entry_count=0;
	return fd;
	
}

//close a fingerprint file
int fin_end(){
	lseek(fin_fd,0,SEEK_SET);
	struct fin_header header;
	header.entry_count=entry_count;
	write(fin_fd,&header,sizeof(struct fin_header));
	close(fin_fd);
	printf("file closed,written %d entries\n",entry_count);
	return 0;
}

//add one entry to the file
int fin_app(struct fin_entry* this_entry){
	size_t written=write(fin_fd,this_entry,sizeof(struct fin_entry));
	fin_flush();
	if(written!=sizeof(struct fin_entry)){
		printf("error in writing\n");
		return -1;
	}
	else {
		printf("added one entry successfully!\n");
		return 0;
	}
}	

//flush
int fin_flush(){
	int rval=fsync(fin_fd);
	if (rval!=0){
		perror("fin_flush");
		return rval;
	} else {
		return 0;
	}
}
//get offset to entry "number"
//note this does not check overflows
off_t fin_get_entry(int num){
	return sizeof(struct fin_header)+sizeof(struct fin_entry)*num;
}
//get pointer to entry "number"'s "subno" entry
off_t fin_get_record(int num,int subno){
	return fin_get_entry(num)+2+subno*sizeof(struct fin_record);
}
//create a fin_record based on one entry of rssi_table
int rssi_table_to_fin_record(struct fin_record* dest,struct rssi_table* from){
	dest->used=1;
	memcpy(dest->bssid,from->bssid,6);
	float calced=0.0;
	int i;
	for(i=0;i<10;i++){
		calced+=from->rssi[i];
	}
	calced/=10;
	dest->rssi=calced;
	return 0;
}
	
//compare function for qsort(), based on rssi
int cmp_rssi(const void* c1,const void* c2){
	struct rssi_table* a1=(struct rssi_table*)c1;
	struct rssi_table* a2=(struct rssi_table*)c2;
	if ( get_rssi_raw(a1)>get_rssi_raw(a2) ){
		return 1;
	} else {
		return -1;
	}
}
//sort rssi_table on rssi, then create a fin_entry based on the top-signalled items
//caller need to fill in used field,and maintain global variable entry_count
int table_to_fin_entry(struct fin_entry* dest){
	printf("trying to lock table\n");
	pthread_mutex_lock(&table_mutex);
	//sort whole rssi table on rssi
	printf("sorting...\n");
	qsort(table,MAX_ENTRY,sizeof(struct rssi_table),cmp_rssi);
	int i;
	printf("copying data\n");
	for (i=0;i<REC_NUM;i++){
		dest->records[i].used=table[i].used;
		if(dest->records[i].used==0){break;}
		memcpy(dest->records[i].bssid,table[i].bssid,6);
		dest->records[i].rssi=get_rssi_index(i);
	}

	pthread_mutex_unlock(&table_mutex);
	printf("fin_entry created successfully\n");
	return 0;
}


//create a fin entry for current table
int table_to_file(){
	int rval=0;
	//pause listening
	printf("stopping listening...\n");
	getter_stop();
	//create fin_entry
	printf("creating entry..\n");
	
	struct fin_entry thisEntry;
	thisEntry.number=entry_count++;
	table_to_fin_entry(&thisEntry);
	//write to file
	printf("writing to file...\n");
	fin_app(&thisEntry);
	//flush the file
	rval=fin_flush();
	if(rval!=0){return -1;}
	//resume listening
	printf("resume listening...\n");
	getter_id=getter_listen();
	return 0;
}
//***find an entry based on current rssi table***
//score one entry
float fin_score(struct fin_entry* src,int index){
	float score=0.0;
	struct fin_entry retrieved;
	lseek(fin_fd,sizeof(struct fin_header)+index*sizeof(struct fin_entry),SEEK_SET);
	read(fin_fd,&retrieved,sizeof(struct fin_entry));
	int i,j;
	for(i=0;i<REC_NUM;i++){
		for(j=0;j<REC_NUM;j++){
			if(retrieved.records[i].used==1){
				if(memcmp(retrieved.records[i].bssid,src->records[j].bssid,6)==0){
					//this algorithm should be improved
					score+=abs(retrieved.records[i].rssi-src->records[j].rssi);
					break;
				}
			}
		}
		//this is penalty for not having a certain record;
		score+=10;
	}

	//the lower, the batter
	return score;

}
//
//massive improvement needed!!!
//return number of entry,which indicates a position
//possible improvement: return multiple candidates
int locate(struct fin_entry* sig){

	//evaluate all entries,record their scores
	float* scores=malloc(sizeof(float)*entry_count);
	int i=0;
	for(i=0;i<entry_count;i++){
		scores[i]=fin_score(sig,i);
	}
	//compare score to find a winner
	float min=scores[0];
	int min_index=0;
	for(i=1;i<entry_count;i++){
		if(scores[i]<min){
			min_index=i;
			min=scores[i];
		}
	}

	return min_index;
}




