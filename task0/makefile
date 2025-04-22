# .PHONY: is a special target that tells make that these targets are not files and should be executed.
# Shortcuts: $@ - target name, 
#			 $^ - all dependencies, 
#			 $< - first dependency.

.PHONY: all clean

CC = gcc
CFLAGS = -g -m32 -Wall

all: ../count-words

../count-words: count-words.o
	$(CC) $(CFLAGS) -o $@ $^ 
							 
count-words.o: count-words.c
	$(CC) $(CFLAGS) -c $<	 

clean:
	rm -f ../count-words count-words.o
