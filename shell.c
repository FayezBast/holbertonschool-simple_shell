#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h> // For isspace()
#include <errno.h> // For errno

#define PROMPT "$fb "
#define MAX_ARGS 64
#define MAX_PATH_LEN 1024

char *trim_whitespace(char *str)
{
    while (isspace(*str)) str++;
    if (*str == '\0') return str;

    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    end[1] = '\0';
    return str;
}

char *find_command(const char *cmd)
{
    if (cmd == NULL) return NULL;

    if (cmd[0] == '/' || (cmd[0] == '.' && (cmd[1] == '/' || (cmd[1] == '.' && cmd[2] == '/'))))
    {
        if (access(cmd, X_OK) == 0) return strdup(cmd);
        return NULL;
    }

    char *path = getenv("PATH");
    if (path == NULL) return NULL;

    char *path_copy = strdup(path);
    if (path_copy == NULL) return NULL;

    char *dir = strtok(path_copy, ":");
    while (dir != NULL)
    {
        char full_path[MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);

        if (access(full_path, X_OK) == 0)
        {
            free(path_copy);
            return strdup(full_path);
        }

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

    cmd = trim_whitespace(cmd);
    if (cmd[0] == '\0') return;

    token = strtok(cmd, " \t");
    while (token != NULL && i < MAX_ARGS - 1)
    {
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;

    if (args[0] == NULL) return;

    char *command_path = find_command(args[0]);
    if (command_path == NULL)
    {
        fprintf(stderr, "%s: command not found\n", args[0]);
        return;
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        execve(command_path, args, environ);
        perror("execve");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        wait(NULL);
    }
    else
    {
        perror("fork");
    }

    free(command_path);
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
