#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "LineParser.h"

typedef struct process{ 
    cmdLine* cmd;          /* the parsed command line*/ 
    pid_t pid;             /* the process id that is running the command*/ 
    int status;            /* status of the process: RUNNING/SUSPENDED/TERMINATED */ 
    struct process *next;  /* next process in chain */ 
} process;

#define TERMINATED -1 
#define RUNNING 1 
#define SUSPENDED 0

#define HISTLEN 20
#define MAX_BUF 200

void execute(cmdLine *pCmdLine, int debug);
void executeWithPiping(cmdLine *pCmdLine, int debug);
void addProcess(process** process_list, cmdLine* cmd, pid_t pid);
void printProcessList(process** process_list);
void freeProcessList(process* process_list);
void updateProcessList(process **process_list);
void updateProcessStatus(process* process_list, int pid, int status);
void addHistory(char *input);
void printHistory();

process *process_list = NULL;

char history[HISTLEN][MAX_BUF] = {0};
int newest = -1;
int oldest = -1;

int main(int argc, char **argv){
    char cwd[PATH_MAX] = {0};
    char input[2048] = {0};
    int debug = 0;
    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "-d") == 0){
            debug = 1;
            break;
        }
    }
    while(1){
        getcwd(cwd, PATH_MAX);
        printf("%s$ ", cwd);    
        fgets(input, 2048, stdin);
        cmdLine *cmd = parseCmdLines(input);
        
        
        if(strcmp(cmd -> arguments[0], "!!") == 0){
            if(newest == -1){
                freeCmdLines(cmd);
                continue;
            }
            else{
                strncpy(input, history[newest], MAX_BUF);
                freeCmdLines(cmd);
                cmd = parseCmdLines(input);
            }
        }
        else if(cmd -> arguments[0][0] == '!'){
            int index = atoi((cmd -> arguments[0]) + 1);
            if(index >= HISTLEN || index < 0){
                printf("Invalid number\n");
                freeCmdLines(cmd);
                continue;
            }
            else if(history[index][0] == 0){
                printf("Invalid number\n");
                freeCmdLines(cmd);
                continue;
            }
            else{
                strncpy(input, history[index], MAX_BUF);
                freeCmdLines(cmd);
                cmd = parseCmdLines(input);
            }
        }
        if(strcmp(cmd -> arguments[0], "quit") == 0){
            freeCmdLines(cmd);
            addHistory(input);
            break;
        }
        else if(strcmp(cmd -> arguments[0], "cd") == 0){
            if(chdir(cmd -> arguments[1]) == -1){
                perror("cd");
            }
            addHistory(input);
            freeCmdLines(cmd);
        }
        else if(strcmp(cmd -> arguments[0], "alarm") == 0){
            int pid = atoi(cmd -> arguments[1]);
            if(kill(pid, SIGCONT) == -1){
                perror("alarm");
            }  
            addHistory(input);
            freeCmdLines(cmd);
            updateProcessStatus(process_list, pid, RUNNING);
        }
        else if(strcmp(cmd -> arguments[0], "blast") == 0){
            int pid = atoi(cmd -> arguments[1]);
            if(kill(pid, SIGINT) == -1){
                perror("blast");
            }
            addHistory(input);
            freeCmdLines(cmd);
            updateProcessStatus(process_list, pid, TERMINATED);
        }
        else if(strcmp(cmd -> arguments[0], "sleep") == 0){
            int pid = atoi(cmd -> arguments[1]);
            if(kill(pid, SIGTSTP) == -1){
                perror("suspend");
            }
            addHistory(input);
            freeCmdLines(cmd);
            updateProcessStatus(process_list, pid, SUSPENDED);
        }
        else if(strcmp(cmd -> arguments[0], "procs") == 0){
            printProcessList(&process_list);
            addHistory(input);
            freeCmdLines(cmd);
        }
        else if(strcmp(cmd -> arguments[0], "history") == 0){
            addHistory(input);
            printHistory();
        }
        
        else{
            addHistory(input);
            executeWithPiping(cmd, debug);
        }
    }
    freeProcessList(process_list);
    return 0;
}

void execute(cmdLine *pCmdLine, int debug){
    int pid = fork();
    if(pid == -1){
        perror("fork");
        _exit(1);
    }
    else if(pid == 0){
        if (pCmdLine -> inputRedirect) {
            freopen(pCmdLine->inputRedirect, "r", stdin);
        }
        if (pCmdLine -> outputRedirect) {
            freopen(pCmdLine->outputRedirect, "w", stdout);
        }

        if(execvp(pCmdLine -> arguments[0], pCmdLine -> arguments) == -1){
            perror("execute");
            _exit(1);
        }
    }
    else{
        addProcess(&process_list, pCmdLine, pid);
        if(debug == 1){
            fprintf(stderr, "PID : %d\n",pid);
            fprintf(stderr, "Executing command : %s\n", pCmdLine -> arguments[0]);
        }
        if(pCmdLine -> blocking){
            waitpid(pid, NULL, 0);
        }
    }
}

void executeWithPiping(cmdLine *pCmdLine, int debug){
    if(pCmdLine -> next){
        int pipefd[2];
        pid_t cpid1, cpid2;

        // Create a pipe.
        if(pipe(pipefd) == -1){
            perror("pipe");
            _exit(1);
        }
        // Fork a first child process (child1).
        if((cpid1 = fork()) == -1){
            perror("fork");
            _exit(1);
        }   
        else if(cpid1 == 0){// Child1 process.
            close(STDOUT_FILENO);
            if(dup(pipefd[1]) == -1){
                perror("dup2");
                _exit(1);
            }
            close(pipefd[0]);
            close(pipefd[1]);
            execute(pCmdLine, debug);
            _exit(0);
        }
        else{// Parent process.
            addProcess(&process_list, pCmdLine, cpid1);
            close(pipefd[1]);
            if((cpid2 = fork()) == -1){
                perror("fork");
                _exit(1);
            }
            else if(cpid2 == 0){// Child2 process.
                close(STDIN_FILENO);
                if(dup(pipefd[0]) == -1){
                    perror("dup");
                    _exit(1);
                }
                close(pipefd[0]);
                execute(pCmdLine -> next, debug);
                _exit(0);
            }
            else{// Parent process.
                addProcess(&process_list, pCmdLine -> next, cpid2);
                if(debug == 1){
                    fprintf(stderr, "PID : %d\n", cpid1);
                    fprintf(stderr, "Executing command : %s\n", pCmdLine -> arguments[0]);
                    fprintf(stderr, "PID : %d\n", cpid2);
                    fprintf(stderr, "Executing command : %s\n", pCmdLine -> next -> arguments[0]);
                }
                close(pipefd[0]);
                waitpid(cpid1, NULL, 0);
                waitpid(cpid2, NULL, 0);
            }
        }
    }
    else{
        execute(pCmdLine, debug);
    }
}

void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process *newProcess = (process *)calloc(1, sizeof(process));

    newProcess -> cmd = cmd;
    newProcess -> pid = pid;
    newProcess -> status = RUNNING;
    newProcess -> next = *process_list;

    *process_list = newProcess;
}

void printProcessList(process** process_list){
    updateProcessList(process_list);

    process *current = *process_list;
    process *prev = NULL;
    int index = 0;
    printf("Index    PID     Status       command\n");
    while(current != NULL){
        char *status = ((current -> status) == -1) ? "TERMINATED" : ((current -> status) == 1) ? "RUNNING" : "SUSPENDED";
        printf("%-9d%-8d%-13s", index, current -> pid, status);
        for(int i = 0; (current -> cmd -> arguments[i]) != NULL; i++){
            printf("%s ", current -> cmd -> arguments[i]);
        }
        printf("\n");

        if((current -> status) == TERMINATED){// checking if the process was terminated
            if(prev == NULL){
                *process_list = current -> next;
                if((current -> cmd -> idx) == 0){
                    freeCmdLines(current -> cmd);
                    free(current);
                }
                current = *process_list;
            }
            else{
                prev -> next = current -> next;
                if((current -> cmd -> idx) == 0){
                    freeCmdLines(current -> cmd);
                    free(current);
                }
                current = prev -> next;
            }
        }
        else{
            prev = current;
            current = current -> next;
        }
        index++;
    }
}

void freeProcessList(process* process_list) {
    process *current = process_list;
    process *next;
    while(current != NULL){
        next = current -> next;
        if((current -> cmd -> idx) == 0){
            freeCmdLines(current -> cmd);
            free(current);
        }
        current = next;
    }
}

void updateProcessList(process **process_list){
    process *current = *process_list;
    int status;
    pid_t result;
    while(current != NULL){
        result = waitpid(current -> pid, &status, WNOHANG | WUNTRACED);
        if(result == 0){
            current -> status = RUNNING;
        }
        else if(result != current -> pid){
            current -> status = TERMINATED;
        }
        else if(WIFEXITED(status) || WIFSIGNALED(status)){
            current -> status = TERMINATED;
        }
        else if(WIFSTOPPED(status)){
            current -> status = SUSPENDED;
        }
        else if(WIFCONTINUED(status)){
            current -> status = RUNNING;
        }
        else{
            current -> status = TERMINATED;
        }
        current = current -> next;
    }
}

void updateProcessStatus(process* process_list, int pid, int status){
    process *current = process_list;
    while(current != NULL){
        if((current -> pid) == pid){
            current -> status = status;
            break;
        }
        current = current -> next;
    }
}

void addHistory(char *input){
    if(newest == -1){
        newest = 0;
        oldest = 0;
    }
    else{
        newest = (newest + 1) % HISTLEN;
        if(newest == oldest){
            oldest = (oldest + 1) % HISTLEN;
        }
    }
    strncpy(history[newest], input, MAX_BUF);
}

void printHistory(){
    int i = oldest;
    while(i != newest){
        printf("%d %s", i, history[i]);
        i = (i + 1) % HISTLEN;
    }
    printf("%d %s", i, history[i]);
}