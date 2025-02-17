#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_ARGS 64
#define PATH_DELIM ":"

/**
 * find_command - searches for the command in the directories listed in PATH
 * @cmd: the command to search for
 * Return: the full path of the command if found, NULL otherwise
 */
char *find_command(char *cmd)
{
    char *path = getenv("PATH");
    if (!path)
        return NULL;

    char *path_copy = strdup(path);
    char *dir = strtok(path_copy, PATH_DELIM);
    char *cmd_path = NULL;
    struct stat st;

    while (dir)
    {
        cmd_path = malloc(strlen(dir) + strlen(cmd) + 2);
        if (!cmd_path)
        {
            perror("malloc");
            free(path_copy);
            return NULL;
        }
        
        sprintf(cmd_path, "%s/%s", dir, cmd);
        
        if (stat(cmd_path, &st) == 0 && (st.st_mode & S_IXUSR))
        {
            free(path_copy);
            return cmd_path;
        }

        free(cmd_path);
        dir = strtok(NULL, PATH_DELIM);
    }

    free(path_copy);
    return NULL;
}

/**
 * execute_command - forks and executes the given command
 * @cmd: the command to execute
 */
void execute_command(char *cmd)
{
    char *args[MAX_ARGS];
    char *token;
    int i = 0;
    pid_t pid;

    token = strtok(cmd, " \t");
    while (token != NULL && i < MAX_ARGS - 1)
    {
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;

    if (args[0] == NULL)
        return;

    char *cmd_path = find_command(args[0]);
    if (cmd_path == NULL)
    {
        perror("Command not found");
        return;
    }

    pid = fork();
    if (pid == -1)
    {
        perror("Fork failed");
        free(cmd_path);
        return;
    }

    if (pid == 0)
    {
        execve(cmd_path, args, NULL);
        perror("Execve failed");
        free(cmd_path);
        exit(EXIT_FAILURE);
    }
    else
    {
        wait(NULL);
        free(cmd_path);
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
            printf(":) ");
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
