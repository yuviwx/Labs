.PHONY: all clean

CC = gcc
CFLAGS = -g -m32 -Wall

all: ../base

../base: base.o
	$(CC) $(CFLAGS) -o $@ $^

base.o: base.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f ../base base.o