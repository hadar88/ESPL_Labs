#include <stdio.h>
#include <stdlib.h>

struct fun_desc { 
    char *name; 
    char (*fun)(char); 
};
char* map(char *array, int array_length, char (*f) (char));
char my_get(char c); 
char cprt(char c); 
char encrypt(char c); 
char decrypt(char c); 
char xoprt(char c); 

int main(int argc, char **argv){
    char line[12] = {0};
    char *carray = calloc(5, sizeof(char));
    
    struct fun_desc menu[] = {{"Get string", my_get}, {"Print string", cprt}, {"Encrypt", encrypt}, {"Decrypt", decrypt}, {"Print Hex and Octal", xoprt}, {NULL, NULL}};
    int length = 0;
    while (menu[length].name != NULL){
        length++;
    }
    while(1){
        fprintf(stdout, "Select option from the following menu (ctrl^D for exit):\n");
        for(int i = 0; menu[i].name != NULL; i++){
            fprintf(stdout, "%i)  %s\n", i, menu[i].name);
        }
        if(fgets(line, 12, stdin) == NULL)
            break;
        int option = atoi(line);
        fprintf(stdout, "Option : %i\n\n", option);
        if(option >= 0 && option < length)
            fprintf(stdout, "Within bounds\n");
        else{
            fprintf(stdout, "Not within bounds\n");
            exit(0);
        }
        char *temp = carray;
        carray = map(carray, 5, menu[option].fun);
        free(temp);
        fprintf(stdout, "Done.\n\n");
    }
    free(carray);
    return 0;
}

char* map(char *array, int array_length, char (*f) (char)){
    char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
    for(int i = 0; i < array_length; i++){
        mapped_array[i] = f(array[i]);
    }
    return mapped_array;
}

char my_get(char c){
    return fgetc(stdin);
}
char cprt(char c){
    (c >= (char)0x20 && c <= (char)0x7E) ? printf("%c\n", c) : printf("%c\n", '.');
    return c;
}
char encrypt(char c){
    c = (c >= (char)0x20 && c <= (char)0x4E) ? c + (char)0x20 : c + (char)0x0;
    return c;
}
char decrypt(char c){
    c = (c >= (char)0x40 && c <= (char)0x7E) ? c - (char)0x20 : c;
    return c;
}
char xoprt(char c){
    printf("%x ", c);
    printf("%o\n", c);
    return c;
}
