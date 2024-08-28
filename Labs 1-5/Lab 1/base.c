#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char my_get(char c); 

char cprt(char c); 
 
char encrypt(char c); 

char decrypt(char c); 
 
char xoprt(char c); 

char* map(char *array, int array_length, char (*f) (char)){
    char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
    /* TODO: Complete during task 2.a */
    for(int i = 0; i < array_length; i++){
        mapped_array[i] = f(array[i]);
    }
    return mapped_array;
}
 
int main(int argc, char **argv){
    /* TODO: Test your code */
    int base_len = 5; 
    char arr1[base_len]; 
    char* arr2 = map(arr1, base_len, my_get); 
    char* arr3 = map(arr2, base_len, cprt); 
    char* arr4 = map(arr3, base_len, xoprt); 
    char* arr5 = map(arr4, base_len, encrypt); 
    free(arr2); 
    free(arr3); 
    free(arr4); 
    free(arr5);
    return 0;
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
