#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv){
    int fd[2];
    char *msg = "hello";
    char buffer[8];

    if(pipe(fd) == -1){
        perror("Error");
        _exit(1);
    }
    int pid = fork();
    if(pid == -1){
        perror("Error");
        _exit(1);
    }
    if(pid == 0){//child procces
        close(fd[0]);
        write(fd[1], msg, strlen(msg) + 1);
        close(fd[1]);
    }
    else{//parent process
        close(fd[1]);
        read(fd[0], buffer, sizeof(buffer));
        printf("%s\n", buffer);
        close(fd[0]);
    }
    return 0;
}

