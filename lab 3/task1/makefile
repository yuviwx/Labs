all: task1a

task1a: start.o util.o
	ld -m elf_i386 start.o util.o -o task1a

start.o: start.s
	nasm -f elf32 start.s -o start.o

util.o: util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

.PHONY: all clean
clean:
	rm -f *.o task1a