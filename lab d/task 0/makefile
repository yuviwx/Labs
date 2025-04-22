all: task0

task0: task0.o
	gcc -m32 -o task0 task0.o

task0.o: task0.s
	nasm -f elf32 -g task0.s -o task0.o

.PHONY: all clean

clean:
	rm -f *.o task0
