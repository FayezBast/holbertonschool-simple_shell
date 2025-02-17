#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PROMPT "$fb "
#define MAX_ARGS 64

extern char **environ;

char *find_command(char *command)
{
    if (strchr(command, '/') != NULL)
    {
        if (access(command, X_OK) == 0)
            return strdup(command);
        return NULL;
    }
    
    char *path = getenv("PATH");
    if (!path)
        return NULL;

    char *path_copy = strdup(path);
    if (!path_copy)
        return NULL;

    char *token = strtok(path_copy, ":");
    while (token)
    {
        char *full_path = malloc(strlen(token) + strlen(command) + 2);
        if (!full_path)
        {
            free(path_copy);
            return NULL;
        }
        sprintf(full_path, "%s/%s", token, command);
        if (access(full_path, X_OK) == 0)
        {
            free(path_copy);
            return full_path;
        }
        free(full_path);
        token = strtok(NULL, ":");
    }
    free(path_copy);
    return NULL;
}

void execute_command(char *command)
{
    char *args[MAX_ARGS];
    int i = 0;
    char *token = strtok(command, " ");
    while (token && i < MAX_ARGS - 1)
    {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
    
    if (!args[0])
        return;
    
    char *full_path = find_command(args[0]);
    if (!full_path)
    {
        fprintf(stderr, "%s: command not found\n", args[0]);
        return;
    }
    
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        free(full_path);
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        execve(full_path, args, environ);
        perror("execve");
        free(full_path);
        exit(EXIT_FAILURE);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
    }
    free(full_path);
}

int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    int interactive = isatty(STDIN_FILENO);
    
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

        char *command = strtok(line, ";");
        while (command != NULL)
        {
            execute_command(command);
            command = strtok(NULL, ";");
        }
    }
    free(line);
    return 0;
}

