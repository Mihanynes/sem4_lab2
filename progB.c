#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_ARGS 1024
#define MAX_COMMANDS 16

pid_t this_pid;

void sigterm_handler(int sig) {
    kill(-this_pid, SIGTERM);
    printf("B is terminated by A\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    char *commands[MAX_COMMANDS][MAX_ARGS];
    char input_buffer[1024];
    int num_commands = 0;
    this_pid = getpid();
    signal(SIGTERM, sigterm_handler);

    printf("Input commands: ");
    fgets(input_buffer, sizeof(input_buffer), stdin);

    if (argc > 1) {
        int parent_pid = atoi(argv[1]);
        if (parent_pid <= 0) {
            printf("Atoi error\n");
        }
        else {
            kill(parent_pid, SIGUSR1);
        }
    }

    commands[num_commands][0] = strtok(input_buffer, "|");
    while (commands[num_commands][0] != NULL) {
        num_commands++;
        commands[num_commands][0] = strtok(NULL, "|");
    }

    for (int i = 0; i < num_commands; i++) {
        char *args = strtok(commands[i][0], " \n");
        int arg_count = 0;
        while (args != NULL) {
            commands[i][arg_count] = args;
            arg_count++;
            args = strtok(NULL, " \n");
        }
    }

    int pipefds[num_commands - 1][2];
    for (int i = 0; i < num_commands - 1; i++) {
        pipe(pipefds[i]);
    }

    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            printf("Fork error\n");
        }
        else if (pid == 0) {
            if (i == 0) {
                if (dup2(pipefds[i][1], STDOUT_FILENO) < 0) {
                    printf("Unable to duplicate file descriptor.");
                    exit(EXIT_FAILURE);
                }
            } else if (i == num_commands - 1) {
                if (dup2(pipefds[i - 1][0], STDIN_FILENO) < 0) {
                    printf("Unable to duplicate file descriptor.");
                    exit(EXIT_FAILURE);
                }
            } else {
                if (dup2(pipefds[i - 1][0], STDIN_FILENO) < 0 ||
                    dup2(pipefds[i][1], STDOUT_FILENO) < 0)  {
                    printf("Unable to duplicate file descriptor.");
                    exit(EXIT_FAILURE);
                }
            }
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipefds[j][0]);
                close(pipefds[j][1]);
            }
            execvp(commands[i][0], commands[i]);
            exit(0);
        }
    }

    for (int j = 0; j < num_commands - 1; j++) {
        close(pipefds[j][0]);
        close(pipefds[j][1]);
    }
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
    return 0;
}