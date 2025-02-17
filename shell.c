#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#define PROMPT "$fb "
#define MAX_ARGS 64
#define MAX_PATH 1024

char *find_command(const char *command) {
    struct stat st;
    char *path_copy, *dir, *full_path;
    char *path = getenv("PATH");

    if (strchr(command, '/') != NULL) {
        if (stat(command, &st) == 0 && (st.st_mode & S_IXUSR)) {
            return strdup(command);
        }
        return NULL;
    }

    if (!path || !*path) {
        return NULL;
    }

    path_copy = strdup(path);
    if (!path_copy) {
        return NULL;
    }

    full_path = malloc(MAX_PATH);
    if (!full_path) {
        free(path_copy);
        return NULL;
    }

    dir = strtok(path_copy, ":");
    while (dir) {
        snprintf(full_path, MAX_PATH, "%s/%s", dir, command);
        if (stat(full_path, &st) == 0 && (st.st_mode & S_IXUSR)) {
            free(path_copy);
            return full_path;
        }
        dir = strtok(NULL, ":");
    }

    free(path_copy);
    free(full_path);
    return NULL;
}

void trim_whitespace(char *str) {
    char *end;
    
    while (isspace(*str)) str++;
    
    if (*str == 0) return;
    
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    
    *(end + 1) = '\0';
}

void execute_command(char *cmd) {
    char *args[MAX_ARGS];
    char *token;
    int i = 0;
    
    if (!cmd || !*cmd) return;
    
    
    trim_whitespace(cmd);
    if (!*cmd) return;
    
    token = strtok(cmd, " \t");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;
    
    if (args[0] == NULL) return;
    
    char *command_path = find_command(args[0]);
    if (!command_path) {
        fprintf(stderr, "%s: No such file or directory\n", args[0]);
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        char *envp[] = {NULL};
        execve(command_path, args, envp);
        perror("execve");
        free(command_path);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        perror("fork");
    }
    
    free(command_path);
}

int main(void) {
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    int interactive = isatty(STDIN_FILENO);
    char *command;

    while (1) {
        if (interactive) {
            printf(PROMPT);
            fflush(stdout);
        }

        nread = getline(&line, &len, stdin);
        if (nread == -1) {
            if (interactive)
                printf("\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';
        command = strtok(line, ";");
        
        while (command != NULL) {
            execute_command(command);
            command = strtok(NULL, ";");
        }
    }

    free(line);
    return 0;
}
