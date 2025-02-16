#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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
    ssize_t nread;
    int interactive = isatty(STDIN_FILENO);
    char *command;

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

