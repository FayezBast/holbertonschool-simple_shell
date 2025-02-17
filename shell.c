#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define PROMPT "$fb "
#define MAX_ARGS 64

char *get_full_path(char *cmd)
{
    struct stat st;
    char *path = NULL;
    char *path_copy = NULL;
    char *dir = NULL;
    char *full_path = NULL;
    
    if (strchr(cmd, '/') != NULL)
    {
        if (stat(cmd, &st) == 0)
            return strdup(cmd);
        return NULL;
    }

    path = getenv("PATH");
    if (!path)
        return NULL;

    path_copy = strdup(path);
    if (!path_copy)
        return NULL;

    dir = strtok(path_copy, ":");
    while (dir != NULL)
    {
        full_path = malloc(strlen(dir) + strlen(cmd) + 2);
        if (!full_path)
        {
            free(path_copy);
            return NULL;
        }
        sprintf(full_path, "%s/%s", dir, cmd);
        
        if (stat(full_path, &st) == 0)
        {
            free(path_copy);
            return full_path;
        }
        
        free(full_path);
        dir = strtok(NULL, ":");
    }
    
    free(path_copy);
    return NULL;
}

void execute_command(char *cmd)
{
    char *args[MAX_ARGS];
    char *token;
    int i = 0;
    char *full_path;
    pid_t pid;

    while (*cmd == ' ' || *cmd == '\t')
        cmd++;

    if (*cmd == '\0')
        return;

    token = strtok(cmd, " \t");
    while (token != NULL && i < MAX_ARGS - 1)
    {
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;

    if (args[0] == NULL)
        return;

    full_path = get_full_path(args[0]);
    if (!full_path)
        return;

    pid = fork();
    if (pid == 0)
    {
        execve(full_path, args, NULL);
        perror("execve");
        free(full_path);
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        wait(NULL);
    }

    free(full_path);
}

int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    int interactive;
    char *command;

    interactive = isatty(STDIN_FILENO);

    while (1)
    {
        if (interactive)
        {
            printf(PROMPT);
            fflush(stdout);
        }

        nread = getline(&line, &len, stdin);
        if (nread == -1)
        {
            if (interactive)
                printf("\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';
        command = strtok(line, ";");
        while (command != NULL)
        {
            execute_command(command);
            command = strtok(NULL, ";");
        }
    }

    free(line);
    return 0;
}
