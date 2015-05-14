//structures and functions to manipulate a fingerprint file
//a fingerprint file contans entries, which are made up of REC_NUM records
//each record is a struct fin_record and contains one single BSSID and RSSI(caled)
//each entry represents a position
//up to now ,entries and records are of consistent length
#ifndef fin_h
#define fin_h
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include "table.h"
#include "getter.h"

#define REC_NUM 10
//header struct
struct fin_header {
	uint16_t entry_count;
};
//one record for a rssi
struct fin_record{
	uint8_t used;
	uint8_t bssid[6];
	float rssi;
};

//one entry for a given posision
struct fin_entry{
	uint16_t number;
	struct fin_record records[REC_NUM];
};
//current fingerprint file fd
extern int fin_fd;
//current entries in total
extern size_t entry_count;
//create a new fingerprint file
//return file descripter on success,-1 on error
int fin_new(char* filename);

//close a fingerprint file
int fin_end();

//add one entry to the file
int fin_app(struct fin_entry* this_entry);
int fin_flush();
//get offset to entry "number"
//note this does not check overflows
off_t fin_get_entry(int num);

//get pointer to entry "number"'s "subno" entry
off_t fin_get_record(int num,int subno);

//create a fin_record based on one entry of rssi_table
int rssi_table_to_fin_record(struct fin_record* dest,struct rssi_table* from);
	
//compare function for qsort(), based on rssi
int cmp_rssi(const void* c1,const void* c2);

//sort rssi_table on rssi, then create a fin_entry based on the top-signalled items
//caller need to fill in used field,and maintain global variable entry_count
int table_to_fin_entry(struct fin_entry* dest);

//create a fin entry for current table
int table_to_file();
//***find an entry based on current rssi table***
//this is extremely difficult!massive improvement needed!!!
#endif
