CC = gcc
CFLAGS = -ansi -pedantic -Wall -std=c99

all:bank

bank:	bank.o
	$(CC) -pthread $(CFLAGS) -o bank bank.o

bank.o:	bank.c bank.h
	$(CC) -pthread $(CFLAGS) -c bank.c bank.h

server:	server.c
	$(CC) -pthread $(CFLAGS) -o server server.c

client: client.c
	$(CC) -pthread $(CFLAGS) -o client client.c

	
