#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "LineParser.h"


void execute(cmdLine *pCmdLine, int debug){
    int pid = fork();
    if(pid == -1){
        perror("fork error");
        _exit(1);
    }
    else if(pid == 0){
        if (pCmdLine->inputRedirect) {
            freopen(pCmdLine->inputRedirect, "r", stdin);
        }
        if (pCmdLine->outputRedirect) {
            freopen(pCmdLine->outputRedirect, "w", stdout);
        }

        if(execvp(pCmdLine -> arguments[0], pCmdLine -> arguments) == -1){
            perror("execute error");
            _exit(1);
        }
    }
    else{
        if(debug == 1){
            fprintf(stderr, "PID : %d\n",pid);
            fprintf(stderr, "Executing command : %s\n", pCmdLine -> arguments[0]);
        }
        if(pCmdLine -> blocking){
            waitpid(pid, NULL, 0);
        }
    }
    
}

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
        if(strcmp(cmd -> arguments[0], "quit") == 0){
            freeCmdLines(cmd);
            break;
        }
        else if(strcmp(cmd -> arguments[0], "cd") == 0){
            if(chdir(cmd -> arguments[1]) == -1){
                perror("cd error");
            }
        }
        else if(strcmp(cmd -> arguments[0], "alarm") == 0){
            int pid = atoi(cmd -> arguments[1]);
            if(kill(pid, SIGCONT) == -1){
                perror("alarm error");
            }  
        }
        else if(strcmp(cmd -> arguments[0], "blast") == 0){
            int pid = atoi(cmd -> arguments[1]);
            if(kill(pid, SIGINT) == -1){
                perror("blast error");
            }
        }
        else{
            execute(cmd, debug);
        }
        freeCmdLines(cmd);
    }
    return 0;
}