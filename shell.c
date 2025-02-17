#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define PROMPT "$fb "
#define MAX_ARGS 64

void execute_command(char *cmd)
{
    char *args[MAX_ARGS];
    char *token;
    int i = 0;

    token = strtok(cmd, " \t");
    while (token != NULL && i < MAX_ARGS - 1)
    {
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;

    if (args[0] == NULL)
        return;

    if (strchr(args[0], '/') != NULL)
    {
        if (access(args[0], X_OK) == 0)
        {
            if (fork() == 0)
            {
                execve(args[0], args, NULL);
                perror("execve");
                exit(EXIT_FAILURE);
            }
            else
            {
                wait(NULL);
            }
        }
        else
        {
            fprintf(stderr, "Command not found: %s\n", args[0]);
        }
        return;
    }

    char *path = getenv("PATH");
    char *path_token = strtok(path, ":");
    char cmd_path[1024];
    int found = 0;

    while (path_token != NULL)
    {
        snprintf(cmd_path, sizeof(cmd_path), "%s/%s", path_token, args[0]);
        if (access(cmd_path, X_OK) == 0)
        {
            args[0] = cmd_path;
            found = 1;
            break;
        }
        path_token = strtok(NULL, ":");
    }

    if (!found)
    {
        fprintf(stderr, "Command not found: %s\n", args[0]);
        return;
    }

    if (fork() == 0)
    {
        execve(args[0], args, NULL);
        perror("execve");
        exit(EXIT_FAILURE);
    }
    else
    {
        wait(NULL);
    }
}

int main(void)
{
    char *line = NULL;
    size_t len = 0;
    int interactive = isatty(STDIN_FILENO);
    char *command;

    while (1)
    {
        if (interactive)
        {
            printf(PROMPT);
            fflush(stdout);
        }

        if (getline(&line, &len, stdin) == -1)
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
