#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PROMPT "fb$ "
#define MAX_ARGS 64
#define BUFFER_SIZE 1024

void execute_command(char *cmd)
{
    char *args[MAX_ARGS];
    int i = 0;
    char *token;
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

    if (access(args[0], X_OK) != 0)
    {
        char *path = getenv("PATH");
        char buffer[BUFFER_SIZE];
        int found = 0;
        
        if (path != NULL)
        {
            char *path_copy = strdup(path);
            char *dir = strtok(path_copy, ":");
            
            while (dir != NULL && !found)
            {
                snprintf(buffer, BUFFER_SIZE, "%s/%s", dir, args[0]);
                if (access(buffer, X_OK) == 0)
                {
                    args[0] = buffer;
                    found = 1;
                }
                dir = strtok(NULL, ":");
            }
            free(path_copy);
        }
        
        if (!found && args[0][0] != '/' && args[0][0] != '.')
        {
            fprintf(stderr, "./hsh: 1: %s: not found\n", args[0]);
            return;
        }
    }
    
    pid = fork();
    if (pid == -1)
    {
        perror("Error:");
        return;
    }
    if (pid == 0)
    {
        if (execve(args[0], args, environ) == -1)
        {
            perror("Error:");
            exit(EXIT_FAILURE);
        }
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
    char *command;
    int interactive;
    
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
