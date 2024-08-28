# All Targets
all: encoder

# Tool invocations
# Executable "main" depends on the files main.o, numbers.o and add.o.
encoder: encoder.o

	gcc -m32 -g -Wall -o encoder encoder.o

# Depends on the source and header files
encoder.o: encoder.c

	gcc -g -Wall -m32 -c -o encoder.o encoder.c

# Other Targets
clean:
	rm -f encoder *.o