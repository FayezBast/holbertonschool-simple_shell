#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>

#define PROMPT "$fb "
#define MAX_ARGS 64
#define MAX_PATH 1024

char *find_command(const char *command)
{
    struct stat st;
    char full_path[MAX_PATH];
    
    if (stat(command, &st) == 0)
        return strdup(command);
        
    return NULL;
}

void execute_command(char *cmd)
{
    char *args[MAX_ARGS];
    char *token;
    int i = 0;
    char *command_path;
    pid_t pid;

    token = strtok(cmd, " \t\n");
    while (token != NULL && i < MAX_ARGS - 1)
    {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;

    if (args[0] == NULL)
        return;

    command_path = find_command(args[0]);
    if (!command_path)
        return;

    pid = fork();
    if (pid == 0)
    {
        execve(command_path, args, NULL);
        perror("execve");
        free(command_path);
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        wait(NULL);
    }
    
    free(command_path);
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
