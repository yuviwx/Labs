.PHONY: all clean

all: task1 task4

task4: task4.c
	gcc -g -m32 -Wall -fno-pie -fno-stack-protector -o task4 task4.c

task1: task1.o
	gcc -g -m32 -Wall -o task1 task1.o

task1.o: task1.c
	gcc -g -m32 -Wall -c task1.c

clean:
	rm -f task1 task1.o task4