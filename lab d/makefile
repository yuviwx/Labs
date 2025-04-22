all: task1

task1: task1.o
	gcc -m32 -o task1 task1.o

task1.o: task1.s
	nasm -f elf32 -g task1.s -o task1.o

.PHONY: all clean

clean:
	rm -f *.o task1
