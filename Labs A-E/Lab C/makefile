# All Targets
all: myshell mypipeline looper

# Tool invocations
myshell: LineParser.o myshell.o 
	gcc -m32 -g -Wall -o myshell LineParser.o myshell.o 

LineParser.o: LineParser.c 
	gcc -m32 -g -Wall -c -o LineParser.o LineParser.c

myshell.o: myshell.c
	gcc -m32 -g -Wall -c -o myshell.o myshell.c

mypipeline: mypipeline.o
	gcc -m32 -g -Wall -o mypipeline mypipeline.o

mypipeline.o: mypipeline.c
	gcc -m32 -g -Wall -c -o mypipeline.o mypipeline.c

looper: looper.o
	gcc -m32 -g -Wall -o looper looper.o

looper.o: looper.c
	gcc -m32 -g -Wall -c -o looper.o looper.c
# Other Targets
clean:
	rm -f mypipeline myshell looper *.o