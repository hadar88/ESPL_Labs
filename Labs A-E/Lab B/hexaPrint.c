#include <stdio.h>
#include <stdlib.h>

void PrintHex(char *buffer, int length);

int main(int argc, char **argv){
    FILE *file;
    if((file = fopen(argv[1], "r")) == NULL){//opening the file
        perror("Error");
        exit(1);
    }
    if(fseek(file, 0, SEEK_END) == -1){//checking the size of the file
        perror("Error");
        exit(1);
    }
    int size = ftell(file);
    if(size == -1){
        perror("Error");
        exit(1);
    }
    if(fseek(file, 0, SEEK_SET) == -1){
        perror("Error");
        exit(1);
    }
    char *buffer = malloc(size);//initializing the buffer
    if(fread(buffer, 1, size, file) != size){//reading the file
        perror("Error");
        exit(1);
    }
    PrintHex(buffer, size);
    free(buffer);
    fclose(file);
    return 0;
}

void PrintHex(char *buffer, int length){
    for(int i = 0; i < length; i++){
        printf("%x ", buffer[i]);
    }
    printf("\n");
}