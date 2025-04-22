all: task0a task0b

task0b: printString.o
	ld -m elf_i386 printString.o -o task0b

printString.o: printString.s
	nasm -f elf32 printString.s -o printString.o

task0a: start.o main.o util.o
	ld -m elf_i386 start.o main.o util.o -o task0a

start.o: start.s
	nasm -f elf32 start.s -o start.o

util.o: util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

main.o: main.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o

.PHONY: all clean
clean:
	rm -f *.o task0a task0b