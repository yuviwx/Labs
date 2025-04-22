.PHONY: all clean

CC = gcc
CFLAGS = -g -m32 -Wall

all: ../menu

../menu: menu.o
	$(CC) $(CFLAGS) -o $@ $^
	
menu.o: menu.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f ../menu menu.o