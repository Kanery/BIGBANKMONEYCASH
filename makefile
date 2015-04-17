CC = gcc
CFLAGS = -pedantic -Wall 

all:server client

server:	server.o bank.o
	$(CC) -lpthread $(CFLAGS) -o server server.o bank.o

server.o: server.c bank.h
	$(CC) -lpthread $(CFLAGS) -c server.c 

client: client.o bank.o
	$(CC) -lpthread $(CFLAGS) -o client client.o bank.o

client.o: client.c bank.h
	$(CC) -lpthread $(CFLAGS) -c client.c

bank.o:	bank.c bank.h
	$(CC) -lpthread $(CFLAGS) -c bank.c 

clean:
	rm *.o
	rm server client
