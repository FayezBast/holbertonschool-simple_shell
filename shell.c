#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 1024
#define MAX_ARG_COUNT 100

extern char **environ;

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

int command_exists(char *command) {
    struct stat st;
    char *path;
    char *path_copy;
    char *dir;
    
    if (stat(command, &st) == 0) {
        return 1;
    }

    path = getenv("PATH");
    if (!path) {
        return 0;
    }

    path_copy = strdup(path);
    dir = strtok(path_copy, ":");

    while (dir != NULL) {
        char full_path[MAX_CMD_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);

        if (stat(full_path, &st) == 0) {
            free(path_copy);
            return 1;
        }

        dir = strtok(NULL, ":");
    }

    free(path_copy);
    return 0;
}

void execute_command(char **argv) {
    pid_t pid;
    int status;

    if (!command_exists(argv[0])) {
        fprintf(stderr, "%s: command not found\n", argv[0]);
        return;
    }

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

void print_env(void) {
    char **env;

    for (env = environ; *env != 0; env++) {
        printf("%s\n", *env);
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

        if (strcmp(argv[0], "exit") == 0) {
            exit(0);
        } else if (strcmp(argv[0], "env") == 0) {
            print_env();
        } else {
            execute_command(argv);
        }
    }

    return 0;
}
