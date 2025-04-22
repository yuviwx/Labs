CFLAGS = -m32 -g -Wall

.PHONY: all clean
all: encoder

encoder: encoder.o 	
	gcc $(CFLAGS) -o encoder encoder.o

encoder.o: encoder.c
	gcc $(CFLAGS) -c encoder.c

clean: 	
	rm -f encoder.o encoder
