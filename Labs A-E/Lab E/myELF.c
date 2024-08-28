#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#define IN_MAX 256
#define MAX_FILES 2

typedef struct fun_desc {
  char *name;
  void (*fun)();
} fun_desc; 

typedef struct types {
    int type;
    char *name;
} types;

void toggleDebugMode(); //Done
void examineELFFile(); // Done
void printSectionNames(); // Done
void printSymbols(); // Done
void checkFilesForMerge(); // TODO:
void MergeELFFiles(); // TODO:
void quit(); //Done

void printMenu(fun_desc *menu);


char debug_mode = 0;
Elf32_Ehdr *header = NULL;
int file_descriptors[MAX_FILES] = {-1, -1};
void *mapped_files[MAX_FILES] = {NULL, NULL};
Elf32_Ehdr *elf_headers[MAX_FILES] = {NULL, NULL};
char fileNames[MAX_FILES][256] = {0};
int count_files = 0;


types section_types[] = {
    {0, "NULL"},
    {1, "PROGBITS"},
    {2, "SYMTAB"},
    {3, "STRTAB"},
    {4, "RELA"},
    {5, "HASH"},
    {6, "DYNAMIC"},
    {7, "NOTE"},
    {8, "NOBITS"},
    {9, "REL"},
    {10, "SHLIB"},
    {11, "DYNSYM"},
    {14, "INIT_ARRAY"},
    {15, "FINI_ARRAY"},
    {16, "PREINIT_ARRAY"},
    {17, "GROUP"},
    {18, "SYMTAB_SHNDX"},
    {19, "NUM"}
};

int main(int argc, char **argv) {
    
    fun_desc menu[] = { 
        {"Toggle Debug Mode", toggleDebugMode},
        {"Examine ELF File", examineELFFile},
        {"Print Section Names", printSectionNames},
        {"Print Symbols", printSymbols},
        {"Check Files For Merge", checkFilesForMerge},
        {"Merge ELF Files", MergeELFFiles},
        {"Quit", quit},
        {NULL, NULL}
    };

    unsigned option = -1, menuLen = sizeof(menu) / sizeof(menu[0]);

    char input[IN_MAX] = {0};

    while (1)
    {
        printMenu(menu);

        printf(">> ");

        fgets(input, IN_MAX, stdin);

        option = atoi(input);

        if (option < 0 || option >= menuLen)
        {
            fprintf(stderr, "invalid menu option!\n");
            continue;
        }

        menu[option].fun();
        puts(" ");
    }
    
    return 0;
}

void printMenu(fun_desc *menu){
    unsigned i = 0;

    puts("Choose action:");

    while (menu->fun)
    {
        printf("%d - %s\n", i++, menu->name);
        menu++;
    }
}

// Menu functions

void toggleDebugMode(){

    debug_mode = !debug_mode;

    printf("Debug mode is now %s\n", debug_mode ? "ON" : "OFF");
}

void examineELFFile(){
    if(count_files == MAX_FILES){
        puts("Can't open more files");
        return;
    }
    char fileName[IN_MAX] = {0};
    printf("Enter file name: ");
    fgets(fileName, IN_MAX, stdin);
    sscanf(fileName, "%s", fileName);

    int fd = open(fileName, O_RDONLY);// is it should be 3?

    if(fd < 0){
        perror("open failed");
        return;
    }

    size_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    void *map_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

    if(map_start == MAP_FAILED){
        perror("mmap failed");
        close(fd);
        return;
    }

    header = (Elf32_Ehdr *)map_start;
    
    if(header -> e_ident[0] != 0x7f || header -> e_ident[1] != 'E' || header -> e_ident[2] != 'L' || header -> e_ident[3] != 'F'){
        fprintf(stderr, "Not an ELF file\n");
        munmap(map_start, size);
        close(fd);
        return;
    }

    for (int i = 0; i < MAX_FILES; i++) {
        if (file_descriptors[i] == -1) {
            file_descriptors[i] = fd;
            mapped_files[i] = map_start;
            elf_headers[i] = header;
            strcpy(fileNames[i], fileName);
            break;
        }
    }

    count_files++;
    puts(" ");
    puts("ELF Header: ");
    printf("Magic: %c%c%c\n", header -> e_ident[1], header -> e_ident[2], header -> e_ident[3]);
    printf("Data: %s\n", header -> e_ident[5] == 1 ? "2's complement, little endian" : "2's complement, big endian");
    printf("Entry point: 0x%x\n", header -> e_entry);
    printf("Section header offset: %d\n", header->e_shoff);
    printf("Number of section headers: %d\n", header->e_shnum);
    printf("Size of section headers: %d\n", header->e_shentsize);
    printf("Program header offset: %d\n", header->e_phoff);
    printf("Number of program headers: %d\n", header->e_phnum);
    printf("Size of program headers: %d\n", header->e_phentsize);

    close(fd);
}

void printSectionNames(){
    if(count_files == 0){
        puts("No files opened");
        return;
    }
    for(int i = 0; i < MAX_FILES; i++){
        if(file_descriptors[i] == -1){
            continue;
        }

        Elf32_Ehdr *header = elf_headers[i];
        
        puts(" ");
        printf("File %s \n", fileNames[i]);

        if(debug_mode){
            printf("(shstrndx = %d )\n", header -> e_shstrndx);
        }

        Elf32_Shdr *section_headers = (Elf32_Shdr *)(mapped_files[i] + header -> e_shoff);
        char *section_strtab = (char *)(mapped_files[i] + section_headers[elf_headers[i]->e_shstrndx].sh_offset);
        
        for(int j = 0; j < header -> e_shnum; j++){
            if(debug_mode){
                printf("(name offset = %d)\t", section_headers[j].sh_name);
            }
            
            if(section_headers[j].sh_type < 20){
                printf("[%d] %-15s\t %08x\t %06x\t %06x\t %s\n", j, &section_strtab[section_headers[j].sh_name], 
                section_headers[j].sh_addr
                ,section_headers[j].sh_offset, section_headers[j].sh_size, section_types[section_headers[j].sh_type].name);
            }
            else{  
                printf("[%d] %-15s\t %08x\t %06x\t %06x\t %x\n", j, &section_strtab[section_headers[j].sh_name], 
                section_headers[j].sh_addr
                ,section_headers[j].sh_offset, section_headers[j].sh_size, section_headers[j].sh_type); 
            }
            
        }
    }
}

void printSymbols(){
    if(count_files == 0){
        puts("No files opened");
        return;
    }
    for(int i = 0; i < MAX_FILES; i++){
        if(file_descriptors[i] == -1){
            continue;
        }

        Elf32_Ehdr *header = elf_headers[i];

        Elf32_Shdr *section_headers = (Elf32_Shdr *)(mapped_files[i] + header -> e_shoff);
        char *section_strtab = (char *)(mapped_files[i] + section_headers[elf_headers[i]->e_shstrndx].sh_offset);

        Elf32_Shdr *symtab = NULL;
        Elf32_Shdr *strtab = NULL;

        for(int j = 0; j < header -> e_shnum; j++){
            if(section_headers[j].sh_type == SHT_SYMTAB){
                symtab = &section_headers[j];
            }
            else if(section_headers[j].sh_type == SHT_STRTAB){
                strtab = &section_headers[j];
            }
        }

        if(symtab == NULL || strtab == NULL){
            puts("No symbols found");
            return;
        }

        Elf32_Sym *symbols = (Elf32_Sym *)(mapped_files[i] + symtab -> sh_offset);
        char *stringTable = (char *)(mapped_files[i] + strtab -> sh_offset);
        
        int symbols_count = symtab -> sh_size / sizeof(Elf32_Sym);
        printf("Number of symbols: %d\n", symbols_count);

        puts(" ");
        printf("File %s \n", fileNames[i]);

        if(debug_mode){
            printf("Symbol table size: %d, number of symbols: %d\n", symtab -> sh_size, symbols_count);
        }

        for(int j = 0; j < symbols_count; j++){
            printf("[%d]\t %08x\t ", j, symbols[j].st_value);

            if(symbols[j].st_shndx == SHN_UNDEF){
                printf("%s\t ", "UND");
            }
            else if(symbols[j].st_shndx == SHN_ABS){
                printf("%s\t ", "ABS");
            }
            else{
                printf("%d\t ", symbols[j].st_shndx);
            }

            if(symbols[j].st_shndx < header -> e_shnum){
                printf("%-10s\t ", &section_strtab[section_headers[symbols[j].st_shndx].sh_name]);
            }
            else{
                printf("%-10s\t ", "");
            }
            
            printf("%s\n", &stringTable[symbols[j].st_name]);
        }

    }
}

void checkFilesForMerge() {
    if (count_files < 2) {
        puts("Not enough files opened");
        return;
    }

    Elf32_Shdr *symtab1 = NULL, *symtab2 = NULL;
    Elf32_Shdr *strtab1 = NULL, *strtab2 = NULL;

    for (int i = 0; i < MAX_FILES; i++) {
        if (file_descriptors[i] == -1) {
            continue;
        }
        Elf32_Ehdr *header = elf_headers[i];
        Elf32_Shdr *section_headers = (Elf32_Shdr *)(mapped_files[i] + header->e_shoff);
        int symtab_count = 0;

        for (int j = 0; j < header->e_shnum; j++) {
            if (section_headers[j].sh_type == SHT_SYMTAB) {
                symtab_count++;
                if (symtab_count == 1) {
                    if (i == 0 && symtab1 == NULL) {
                        symtab1 = &section_headers[j];
                    } else if (i == 1 && symtab2 == NULL) {
                        symtab2 = &section_headers[j];
                    }
                }
            } 
            else if (section_headers[j].sh_type == SHT_STRTAB) {
                if (i == 0 && strtab1 == NULL && symtab_count == 1) {
                    strtab1 = &section_headers[j];
                } else if (i == 1 && strtab2 == NULL && symtab_count == 1) {
                    strtab2 = &section_headers[j];
                }
            }
        }

        if (symtab_count != 1) {
            printf("Feature not supported\n");
            return;
        }
    }

    if (symtab1 == NULL || symtab2 == NULL || strtab1 == NULL || strtab2 == NULL) {
        printf("Feature not supported\n");
        return;
    }

    Elf32_Sym *symbols1 = (Elf32_Sym *)(mapped_files[0] + symtab1->sh_offset);
    Elf32_Sym *symbols2 = (Elf32_Sym *)(mapped_files[1] + symtab2->sh_offset);
    char *stringTable1 = (char *)(mapped_files[0] + strtab1->sh_offset);
    char *stringTable2 = (char *)(mapped_files[1] + strtab2->sh_offset);

    int symbols_count1 = symtab1->sh_size / sizeof(Elf32_Sym);
    int symbols_count2 = symtab2->sh_size / sizeof(Elf32_Sym);

    for (int i = 1; i < symbols_count1; i++) {
        Elf32_Sym *sym = &symbols1[i];
        char *name = &stringTable1[sym->st_name];
        int found = 0;

        if (sym->st_shndx == SHN_UNDEF) {
            for (int j = 1; j < symbols_count2; j++) {
                Elf32_Sym *sym2 = &symbols2[j];
                char *name2 = &stringTable2[sym2->st_name];
                if (strcmp(name, "") != 0 && strcmp(name, name2) == 0) {
                    found = 1;
                    if (sym2->st_shndx == SHN_UNDEF) {
                        printf("Symbol %s undefined\n", name);
                    }
                }
            }
            if (!found) {
                printf("Symbol %s undefined\n", name);
            }
        } else {
            for (int j = 1; j < symbols_count2; j++) {
                Elf32_Sym *sym2 = &symbols2[j];
                char *name2 = &stringTable2[sym2->st_name];
                if (strcmp(name, "") != 0 && strcmp(name, name2) == 0 && sym2->st_shndx != SHN_UNDEF) {
                    printf("Symbol %s multiply defined\n", name);
                }
            }
        }
    }
}


void MergeELFFiles(){
    puts("Not implementes yet\n");
}

void quit(){ 

    for (int i = 0; i < MAX_FILES; i++) {
        if (file_descriptors[i] != -1) {
            munmap(mapped_files[i], lseek(file_descriptors[i], 0, SEEK_END));
            close(file_descriptors[i]);
        }
    }

    exit(0);
}