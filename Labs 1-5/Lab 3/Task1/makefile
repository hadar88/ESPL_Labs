# All Targets
all: task1 

# Tool invocations
task1: start.o util.o
	ld -m elf_i386 start.o util.o -o task1

start.o: start.s
	nasm -f elf32 start.s -o start.o
	
util.o: util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
# Other Targets
clean:
	rm -f task1 *.o