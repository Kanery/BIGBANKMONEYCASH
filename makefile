CC = gcc
CFLAGS = -ansi -pedantic -Wall -std=c99

all:bank server client

bank:	bank.o
	$(CC) -pthread $(CFLAGS) -o bank bank.o

bank.o:	bank.c bank.h
	$(CC) -pthread $(CFLAGS) -c bank.c 

server:	server.o
	$(CC) -pthread $(CFLAGS) -o server server.o

server.o: server.c
	$(CC) -pthread $(CFLAGS) -c server.c

client: client.o
	$(CC) -pthread $(CFLAGS) -o client client.o

client.o: client.c
	$(CC) -pthread $(CFLAGS) -c client.c

clean:
	rm *.o
	rm server client bank
