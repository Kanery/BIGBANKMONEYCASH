CC = gcc
CFLAGS = -pedantic -Wall 

all:bank server client

bank:	bank.o
	$(CC) -lpthread $(CFLAGS) -o bank bank.o

bank.o:	bank.c bank.h
	$(CC) -lpthread $(CFLAGS) -c bank.c 

server:	server.o
	$(CC) -lpthread $(CFLAGS) -o server server.o

server.o: server.c
	$(CC) -lpthread $(CFLAGS) -c server.c

client: client.o
	$(CC) -lpthread $(CFLAGS) -o client client.o

client.o: client.c
	$(CC) -lpthread $(CFLAGS) -c client.c

clean:
	rm *.o
	rm server client bank
