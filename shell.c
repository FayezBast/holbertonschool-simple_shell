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

char *read_command(void);
void execute_command(char **args, int cmd_count, char *program_name);
char *find_command(char *command);
int parse_command(char *command_line, char **args);
int handle_built_in(char **args);
char *get_path_from_environ(void);

/**
 * handle_built_in - Handles built-in commands
 * @args: Array of command and arguments
 * Return: 1 if command was a built-in, 0 if not
 */
int handle_built_in(char **args)
{
    if (!args || !args[0])
        return 0;

    if (strcmp(args[0], "exit") == 0)
    {
        exit(0);
    }

    return 0;
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
 * @cmd_count: Command counter for error messages
 * @program_name: Name of the shell program
 */
void execute_command(char **args, int cmd_count, char *program_name)
{
    pid_t pid;
    int status;
    char *command_path;
    char error_msg[100];

    if (!args || !args[0])
        return;

    if (handle_built_in(args))
        return;

    command_path = find_command(args[0]);
    if (command_path == NULL)
    {
        snprintf(error_msg, sizeof(error_msg), 
                "%s: %d: %s: not found\n", 
                program_name, cmd_count, args[0]);
        write(STDERR_FILENO, error_msg, strlen(error_msg));
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
        if (execve(command_path, args, environ) == -1)
        {
            snprintf(error_msg, sizeof(error_msg), 
                    "%s: %d: %s: not found\n", 
                    program_name, cmd_count, args[0]);
            write(STDERR_FILENO, error_msg, strlen(error_msg));
            free(command_path);
            exit(127);
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
int main(int argc, char **argv)
{
    char *command_line;
    char *args[MAX_ARGS];
    int interactive = isatty(STDIN_FILENO);
    int cmd_count = 1;
    char *program_name = (argc > 0) ? argv[0] : "./hsh";

    while (1)
    {
        if (interactive)
            write(STDOUT_FILENO, "#fb$ ", 5);

        command_line = read_command();
        if (command_line == NULL)
        {
            if (interactive)
                write(STDOUT_FILENO, "\n", 1);
            break;
        }

        if (strlen(command_line) > 0)
        {
            parse_command(command_line, args);
            execute_command(args, cmd_count, program_name);
            cmd_count++;
        }

        free(command_line);
    }

    return 0;
}
