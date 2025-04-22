# $@ is the target, $^ is all the dependencies, $< is the first dependency
.PHONY: all clean

CC = gcc
CFLAGS = -g -m32 -Wall

all: myshell looper mypipe

myshell: myshell.o LineParser.o
	$(CC) $(CFLAGS) -o $@ $^ 

looper: looper.c
	$(CC) $(CFLAGS) -o $@ $<

mypipe: mypipe.c
	$(CC) $(CFLAGS) -o $@ $<
	
myshell.o: myshell.c
	$(CC) $(CFLAGS) -c $<
	
LineParser.o: LineParser.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f myshell looper mypipe myshell.o LineParser.o