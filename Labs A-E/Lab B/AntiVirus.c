#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct virus { 
    unsigned short SigSize; 
    char virusName[16]; 
    unsigned char* sig; 
} virus;

typedef struct link link;
struct link { 
    link *nextVirus; 
    virus *vir; 
};

struct fun_desc { 
    char *name; 
    void (*fun)(); 
};

void OpenFile();
void SetSigFileName();
virus* readVirus(FILE* file);
void printVirus(virus* virus);
void list_print(link *virus_list, FILE *file);
link* list_append(link* virus_list, virus* data);
void list_free(link *virus_list);

void loadSignatures();
void printSignatures();
void detectViruses();
void fixFile();
void quit();
void detect_virus(char *buffer, unsigned int size, link *virus_list);
void neutralize_virus(char *fileName, int signatureOffset);

char fileName[16] = "signatures-L";
FILE *file;
unsigned char *magicnumber;
int isNull = 1;
link *virus_list = NULL;
link *copy = NULL;
int run = 1;
char *virusFileName;

int main(int argc, char **argv){
    virusFileName = argv[1];
    struct fun_desc menu[] = {{"Set signatures file name", SetSigFileName}, {"Load signatures", loadSignatures}, {"Print signatures", printSignatures}, {"Detect viruses", detectViruses}, {"Fix file", fixFile}, {"Quit", quit}, {NULL, NULL}};
    char line[12] = {0};
    
    while(run){
        fprintf(stdout, "Select option from the following menu (ctrl^D for exit):\n");
        for(int i = 0; menu[i].name != NULL; i++){
            fprintf(stdout, "%i) %s\n", i, menu[i].name);
        }
        if(fgets(line, 12, stdin) == NULL)
            break;
        int option = atoi(line);
        if(option >= 0 && option < 6){
            menu[option].fun();
        }
    }
    return 0;
}

/*This function opens the file with the given name,
 and sets the file pointer file to point to it.*/
void OpenFile(){
    if((file = fopen(fileName, "r")) == NULL){
        perror("Error");
        exit(1);
    }
}

/*This function queries the user for a new signature file name,
 and sets the signature file name accordingly.*/
void SetSigFileName(){
    printf("Please enter a new signature file name:\n");
    if(fgets(fileName, sizeof(fileName), stdin)){
        fileName[strlen(fileName) - 1] = '\0';
        isNull = 1;
    }
    else{
        perror("Error");
        exit(1);
    }
}

/*This function receives a file pointer and 
returns a virus* that represents the next virus in the file.*/
virus* readVirus(FILE* file){
    virus* virus = (struct virus*)malloc(sizeof(struct virus));
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;

    if(isNull){
        magicnumber = (unsigned char*)malloc(4);
        fread(magicnumber, 4, 1, file);
        isNull = 0;
    }
    char temp[18];
    fread(temp, 18, 1, file);
    if(strcmp((const char*)magicnumber, "VIRL") == 0){
        SigSize = (temp[1] << 8) | temp[0];
    }
    else if(strcmp((const char*)magicnumber, "VIRB") == 0){
        SigSize = (temp[0] << 8) | temp[1];
    }
    else{
        printf("Error: The file is not a virus signature file\n");
        exit(1);
    }
    virus -> SigSize = SigSize;

    strcpy(virusName, temp + 2);
    strcpy(virus -> virusName, virusName);

    sig = (unsigned char*)malloc(SigSize);
    fread(sig, SigSize, 1, file);
    virus -> sig = sig;
    return virus;
}
//The function prints the virus data to stdout.
void printVirus(virus* virus){
    printf("Name: %s\n", virus -> virusName);
    printf("Size: %d\n", virus -> SigSize);
    printf("Signature:\n");
    for (int i = 0; i < virus->SigSize; ++i) {
        printf("%X ", virus -> sig[i]);
    }
    printf("\n");
    printf("\n");
}

/* Print the data of every link in list to the given stream. Each item 
followed by a newline character. */
void list_print(link *virus_list, FILE *file){
    while (virus_list != NULL){
        fprintf(file, "Virus name: %s\n", virus_list -> vir -> virusName);
        fprintf(file, "Virus size: %d\n", virus_list -> vir -> SigSize);
        fprintf(file, "signature:\n");
        for (int i = 0; i < virus_list -> vir -> SigSize; ++i) {
            printf("%X ", virus_list -> vir -> sig[i]);
        }
        printf("\n");
        printf("\n");
        virus_list = virus_list -> nextVirus;
    }
}

/* Add a new link with the given data at the beginning of the list and 
return a pointer to the list (i.e., the first link in the list).*/
link* list_append(link* virus_list, virus* data){
    if(virus_list == NULL){
        link *new = (link*)malloc(sizeof(link));
        new -> vir = data;
        virus_list = new;
        return virus_list;
    }
    else{
        link *new = (link*)malloc(sizeof(link));
        new -> vir = data;
        new -> nextVirus = virus_list;
        virus_list = new;
        return virus_list;
    }
}

/* Free the memory allocated by the list. */
void list_free(link *virus_list){
    link *temp;
    while(virus_list != NULL){
        temp = virus_list;
        virus_list = virus_list -> nextVirus;
        free(temp -> vir -> sig);
        free(temp -> vir);
        free(temp);
    }
}

void loadSignatures(){
    OpenFile();
    while (!feof(file)){
        virus *temp = readVirus(file);
        if(feof(file))
            break;
        virus_list = list_append(virus_list, temp);
        copy = virus_list;
    }
    fclose(file);
}

void printSignatures(){
    virus *vir = copy -> vir;
    copy = copy -> nextVirus;
    if(copy != NULL){
        printSignatures();
    }
    printVirus(vir);
}

void detectViruses(){ 
    FILE *VirusFile;
    char buffer[10 << 10] ={0};
    if((VirusFile = fopen(virusFileName, "r")) == NULL){
        perror("Error");
        exit(1);
    }
    int fileSize = fread(buffer, 1, 10 << 10, VirusFile);
    fclose(VirusFile);
    detect_virus(buffer, fileSize, virus_list);
}

void fixFile(){
    FILE *VirusFile;
    char buffer[10 << 10] ={0};
    if((VirusFile = fopen(virusFileName, "r")) == NULL){
        perror("Error");
        exit(1);
    }
    int fileSize = fread(buffer, 1, 10 << 10, VirusFile);
    fclose(VirusFile);
    link *temp = virus_list;
    while(virus_list != NULL){
        for(int i = 0; i < fileSize; i++){
            if(memcmp(virus_list -> vir -> sig, &buffer[i], virus_list -> vir -> SigSize) == 0){
                neutralize_virus(virusFileName, i);
            }
        }
        virus_list = virus_list->nextVirus;
    }
    virus_list = temp;
}

void quit(){
    run = 0;
    list_free(virus_list);
    free(magicnumber);
}

void detect_virus(char *buffer, unsigned int size, link *virus_list){
    
    while(virus_list != NULL){
        for(int i = 0; i < size; i++){
            if(memcmp(virus_list -> vir -> sig, &buffer[i], virus_list -> vir -> SigSize) == 0){
                printf("Location : %i\n", i);
                printf("Name : %s\n", virus_list -> vir -> virusName);
                printf("Size : %i\n", virus_list -> vir -> SigSize);
                printf("\n");
            }
        }
        virus_list = virus_list->nextVirus;
    }
}

void neutralize_virus(char *fileName, int signatureOffset){
    FILE *VirusFile;
    unsigned char change = 0xC3;
    if((VirusFile = fopen(fileName, "r+b")) == NULL){ 
        perror("Error opening file");
        exit(1);
    }
    fseek(VirusFile, signatureOffset, SEEK_SET);
    fwrite(&change, 1, 1, VirusFile);   
    fclose(VirusFile);
}


