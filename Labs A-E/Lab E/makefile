# All Targets
all: myELF

myELF: myELF.o
	gcc -m32 -g -Wall -o myELF myELF.o

myELF.o: myELF.c
	gcc -m32 -g -Wall -c -o myELF.o myELF.c

# Other Targets
clean:
	rm -f myELF myELF.o