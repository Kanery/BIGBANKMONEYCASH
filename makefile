CC = gcc
CFLAGS = -ansi -pedantic -Wall -std=c99

all:bank

bank:	bank.o
	$(CC) -pthread $(CFLAGS) -o 
