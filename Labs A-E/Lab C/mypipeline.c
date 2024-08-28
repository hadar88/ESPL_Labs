#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv){
    int pipefd[2];
    pid_t cpid1, cpid2;

    // Create a pipe.
    if(pipe(pipefd) == -1){ 
        perror("pipe");
        exit(1);
    }

    fprintf(stderr, "(parent_process>forking...)\n");

    // Fork a first child process (child1).
    if((cpid1 = fork()) == -1){
        perror("fork");
        exit(1);
    }
    else if(cpid1 == 0){// Child1 process.
        fprintf(stderr, "(parent_process>created process with id: %d)\n", cpid1);
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe...)\n");
        close(STDOUT_FILENO);// Close the standard output.

        if(dup(pipefd[1]) == -1){
            perror("dup");
            exit(1);
        }

        close(pipefd[1]);
        close(pipefd[0]);
        fprintf(stderr, "(child1>going to execute cmd: ls -l)\n");
        char *args1[] = {"ls", "-l", NULL};
        if(execvp(args1[0], args1) == -1){
            perror("execvp");
            exit(1);
        }
    }
    else{// Parent process.
        fprintf(stderr, "(parent_process>closing the write end of the pipe...)\n");
        close(pipefd[1]);
        fprintf(stderr, "(parent_process>forking...)\n");
        if((cpid2 = fork()) == -1){// Error
            perror("fork");
            exit(1);
        }
        else if(cpid2 == 0){// Child2 process.
        fprintf(stderr, "(parent_process>created process with id: %d)\n", cpid2);
            fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe...)\n");
            close(STDIN_FILENO);// Close the standard input.

            if(dup(pipefd[0]) == -1){
                perror("dup");
                exit(1);
            }

            close(pipefd[0]);
            fprintf(stderr, "(child2>going to execute cmd: tail -n 2)\n");
            char *args2[] = {"tail", "-n", "2", NULL};
            if(execvp(args2[0], args2) == -1){
                perror("execvp");
                exit(1);
            }
        }
        else{// Parent process.
            fprintf(stderr, "(parent_process>closing the read end of the pipe...)\n");
            close(pipefd[0]);
            fprintf(stderr, "(parent_process>waiting for child processes to terminate...)\n");
            waitpid(cpid1, NULL, 0);
            waitpid(cpid2, NULL, 0);
            fprintf(stderr, "(parent_process>exiting...)\n");
        }
    }
    return 0;
}