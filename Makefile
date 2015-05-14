ips: main.o getter.o table.o fin.o
	gcc $(CFLAGS) -o ips main.o getter.o table.o fin.o -lpcap -lm -lpthread

main.o: main.c 
	gcc $(CFLAGS) -c main.c -lpcap -lm -lpthread

table.o: table.h table.c
	gcc $(CFLAGS) -c table.c -lpthread

getter.o: getter.c getter.h
	gcc $(CFLAGS) -c getter.c -lpthread

fin.o: fin.c fin.h
	gcc $(CFLAGS) -c fin.c -lpthread

table-test: table.o
	gcc $(CFLAGS) -o test table.o -lpthread

clean:
	rm *.o ips
