#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void handler(int sig) {
    printf("A received signal from B\n");
}

int main() {
    pid_t pid;
    pid = fork();
    int status;
    int parent_pid = getpid();
    signal(SIGUSR1, handler);

    if (pid < 0) {
        printf("Fork error\n");
    }
    else if (pid == 0){
        char str[10];
        sprintf(str, "%d", parent_pid - 1);
        char *cmd[] = {"./progB", str,NULL};
        execvp(cmd[0], cmd);
    }
    else if (pid > 0) {
        pause();
        sleep(1);
        kill(pid, SIGTERM);
        wait(&status);
    }
    return 0;
}
