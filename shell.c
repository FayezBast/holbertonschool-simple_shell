#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024
#define MAX_ARGS 64
extern char **environ;

/**
 * display_prompt - Displays the shell prompt
 */
void display_prompt(void)
{
    printf("#fb$ ");
    fflush(stdout);
}

/**
 * read_command - Reads a command from standard input
 * Return: The command string or NULL on EOF or error
 */
char *read_command(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    read = getline(&line, &len, stdin);
    if (read == -1)
    {
        free(line);
        return NULL;
    }

    if (line[read - 1] == '\n')
        line[read - 1] = '\0';

    return line;
}

/**
 * get_path_from_environ - Get PATH value from environ
 * Return: Copy of PATH value or NULL if not found
 */
char *get_path_from_environ(void)
{
    int i;
    char *path_copy = NULL;
    const char *path_prefix = "PATH=";
    size_t prefix_len = strlen(path_prefix);

    for (i = 0; environ[i] != NULL; i++)
    {
        if (strncmp(environ[i], path_prefix, prefix_len) == 0)
        {
            path_copy = strdup(environ[i] + prefix_len);
            return path_copy;
        }
    }
    return NULL;
}

/**
 * find_command - Searches for command in PATH
 * @command: Command to find
 * Return: Full path of command if found, NULL if not found
 */
char *find_command(char *command)
{
    char *path, *path_copy, *path_token, *file_path;
    int command_length, directory_length;
    struct stat buffer;

    if (strchr(command, '/') != NULL)
    {
        if (stat(command, &buffer) == 0)
            return strdup(command);
        return NULL;
    }

    path = get_path_from_environ();
    if (!path)
        return NULL;

    path_copy = strdup(path);
    command_length = strlen(command);
    path_token = strtok(path_copy, ":");

    while (path_token != NULL)
    {
        directory_length = strlen(path_token);
        file_path = malloc(directory_length + command_length + 2);
        strcpy(file_path, path_token);
        strcat(file_path, "/");
        strcat(file_path, command);

        if (stat(file_path, &buffer) == 0)
        {
            free(path_copy);
            free(path);
            return file_path;
        }
        free(file_path);
        path_token = strtok(NULL, ":");
    }

    free(path_copy);
    free(path);
    return NULL;
}

/**
 * parse_command - Splits command line into command and arguments
 * @command_line: The command line to parse
 * @args: Array to store the command and arguments
 * Return: Number of arguments
 */
int parse_command(char *command_line, char **args)
{
    int count = 0;
    char *token;

    token = strtok(command_line, " \t");
    while (token != NULL && count < MAX_ARGS - 1)
    {
        args[count] = token;
        count++;
        token = strtok(NULL, " \t");
    }
    args[count] = NULL;
    return count;
}

/**
 * execute_command - Executes the given command with arguments
 * @args: Array of command and arguments
 */
void execute_command(char **args)
{
    pid_t pid;
    int status;
    char *command_path;

    if (args[0] == NULL)
        return;

    command_path = find_command(args[0]);
    if (command_path == NULL)
    {
        printf("./shell: No such file or directory\n");
        return;
    }

    pid = fork();
    if (pid == -1)
    {
        perror("Error:");
        free(command_path);
        return;
    }
    
    if (pid == 0)
    {
        args[0] = command_path;
        if (execve(command_path, args, environ) == -1)
        {
            perror("./shell");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        wait(&status);
        free(command_path);
    }
}

/**
 * main - Simple shell main function
 * Return: Always 0
 */
int main(void)
{
    char *command_line;
    char *args[MAX_ARGS];
    int interactive = isatty(STDIN_FILENO);

    while (1)
    {
        if (interactive)
            display_prompt();

        command_line = read_command();
        if (command_line == NULL)
        {
            if (interactive)
                printf("\n");
            break;
        }

        if (strlen(command_line) > 0)
        {
            parse_command(command_line, args);
            execute_command(args);
        }

        free(command_line);
    }

    return 0;
}
