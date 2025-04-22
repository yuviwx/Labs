# $@ is the target, $^ is all the dependencies, $< is the first dependency
.PHONY: all clean

CC = gcc
CFLAGS = -g -m32 -Wall

all: myshell mypipeline mypipe looper

myshell: myshell.o LineParser.o
	$(CC) $(CFLAGS) -o $@ $^ 

mypipeline: mypipeline.c
	$(CC) $(CFLAGS) -o $@ $<

myshell.o: myshell.c
	$(CC) $(CFLAGS) -c $<
	
LineParser.o: LineParser.c
	$(CC) $(CFLAGS) -c $<

looper: looper.c
	$(CC) $(CFLAGS) -o $@ $<

mypipe: mypipe.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f myshell mypipeline mypipe looper myshell.o LineParser.o