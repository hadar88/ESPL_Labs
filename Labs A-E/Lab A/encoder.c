#include <stdio.h>
#include <stdbool.h>
int length(char *c);


int main(int argc, char **argv){
    bool debug = true;

    char *key = NULL;
    char operation =' ';

    FILE *infile = stdin;
    FILE *outfile = stdout;

    for(int i = 1; i < argc; i++){
        if(debug){
            fprintf(stderr, "%s\n", argv[i]);
        }
        if(argv[i][1] == 'D' && argv[i][0] == '-'){
            debug = false;
        }
        else if(argv[i][1] == 'D' && argv[i][0] == '+'){
            debug = true;
        }

        else if(argv[i][1] == 'e' && argv[i][0] == '+'){
            key = argv[i] + 2;
            operation = '+';
        }
        else if(argv[i][1] == 'e' && argv[i][0] == '-'){
            key = argv[i] + 2;
            operation = '-';
        }
        else if(argv[i][1] == 'I' && argv[i][0] == '-'){
            char *name = argv[i] + 2;
            infile = fopen(name, "r");
            if(infile == NULL){
                fprintf(stderr, "Can't open the input file: %s\n", name);
            }
        }
        else if(argv[i][1] == 'O' && argv[i][0] == '-'){
            char *name = argv[i] + 2;
            outfile = fopen(name, "a");
            if(outfile == NULL){
                fprintf(stderr, "Can't open the input file: %s\n", name);
            }
        }
    }

    if(key != NULL){
        int key_place = 0;
        int key_length = length(key);
        int input;
        while((input = fgetc(infile)) != EOF){
            if((input <= '9' && input >= '0') || (input <= 'z' && input >= 'a')){
                if(operation == '+'){
                    int c = key[key_place] - '0';
                    input += c;
                    if(((input - c) <= 'z' && (input - c) >= 'a') && input > 'z'){
                        input -= 26;
                    }
                    else if(((input - c) <= '9' && (input - c) >= '0') && input > '9'){
                        input -= 10;
                    }
                }
                else if(operation == '-'){
                    int c = key[key_place] - '0';
                    input -= c;
                    if(((input + c) <= '9' && (input + c) >= '0') && input < '0'){
                        input += 10;
                    }
                    else if(((input + c) <= 'z' && (input + c) >= 'a') && input < 'a'){
                        input += 26;
                    }
                }
            }
            key_place++;
            if(key_place == key_length)
                key_place = 0;
            fputc(input, outfile);
        }
    }
    else{
        int input;
        while((input = fgetc(infile)) != EOF){
            fputc(input, outfile);
        }
    }
    fclose(infile);
    fclose(outfile);
    return 0;
}

int length(char *c){
    int lenght = 0;
    while(c[lenght] != '\0'){
        lenght++;
    }
    return lenght;
}


