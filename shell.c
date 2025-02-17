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
    char *path = getenv("PATH");
    char *path_copy, *token, *full_path;
    size_t command_len;

    if (!path)
        return NULL;

    path_copy = strdup(path);
    if (!path_copy)
        return NULL;

    token = strtok(path_copy, ":");
    command_len = strlen(command);

    while (token)
    {
        full_path = malloc(strlen(token) + command_len + 2);
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

void execute_command(char *command, char **args)
{
    char *full_path;
    pid_t pid;
    int status;

    full_path = find_command(command);
    if (!full_path)
    {
        fprintf(stderr, "%s: command not found\n", command);
        return;
    }

    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        free(full_path);
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        if (execve(full_path, args, environ) == -1)
        {
            perror("execve");
            free(full_path);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
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
    char *command;
    char *args[MAX_ARGS];
    int i;

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
            i = 0;
            args[i] = strtok(command, " ");
            while (args[i] != NULL && i < MAX_ARGS - 1)
            {
                i++;
                args[i] = strtok(NULL, " ");
            }
            args[i] = NULL;

            if (args[0] != NULL)
                execute_command(args[0], args);

            command = strtok(NULL, ";");
        }
    }

    free(line);
    return 0;
}

