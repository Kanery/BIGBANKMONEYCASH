CC = gcc
CFLAGS = -pedantic -Wall 

all:server client

bank.o:	bank.c bank.h
	$(CC) -lpthread $(CFLAGS) -c -g bank.c 

server:	server.o bank.o
	$(CC) -lpthread $(CFLAGS) -o server server.o 

server.o: server.c
	$(CC) -lpthread $(CFLAGS) -c -g server.c

client: client.o 
	$(CC) -lpthread $(CFLAGS) -o client client.o

client.o: client.c
	$(CC) -lpthread $(CFLAGS) -c client.c

clean:
	rm *.o
	rm server client
