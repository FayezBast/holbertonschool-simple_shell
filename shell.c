#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024
#define MAX_ARG_COUNT 100

void display_prompt(void) {
    printf("#fb$ ");
}

void read_command(char *buffer) {
    if (fgets(buffer, MAX_CMD_LEN, stdin) == NULL) {
        if (feof(stdin)) {
            printf("\n");
            exit(0);
        } else {
            perror("fgets");
            exit(1);
        }
    }

    buffer[strcspn(buffer, "\n")] = '\0';
}

void parse_command(char *command, char **argv) {
    int i = 0;

    argv[i] = strtok(command, " ");
    while (argv[i] != NULL && i < MAX_ARG_COUNT - 1) {
        argv[++i] = strtok(NULL, " ");
    }
    argv[i] = NULL;
}

void execute_command(char **argv) {
    pid_t pid;
    int status;

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        if (execve(argv[0], argv, environ) == -1) {
            perror(argv[0]);
            exit(1);
        }
    } else {
        do {
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
                exit(1);
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

int main(void) {
    char command[MAX_CMD_LEN];
    char *argv[MAX_ARG_COUNT];

    while (1) {
        display_prompt();
        read_command(command);

        if (strlen(command) == 0) {
            continue;
        }

        parse_command(command, argv);
        execute_command(argv);
    }

    return 0;
}
