.PHONY: all clean

CC = gcc
CFLAGS = -g -m32 -Wall

all: ../addresses

../addresses: addresses.o
	$(CC) $(CFLAGS) -o $@ $^

addresses.o: addresses.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f ../addresses addresses.o