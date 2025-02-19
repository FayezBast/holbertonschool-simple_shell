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

int last_status = 0;

char *read_command(void);
void execute_command(char **args, int cmd_count, char *program_name);
char *find_command(char *command);
int parse_command(char *command_line, char **args);
int handle_built_in(char **args, char *program_name);
char *get_path_from_environ(void);
/**
 * handle_built_in - Handles built-in commands
 * @args: Array of command and arguments
 * @program_name: Name of the shell program
 * Return: 1 if command was a built-in, 0 if not
 */
int handle_built_in(char **args, char *program_name)
{
    if (!args || !args[0])
        return 0;

    if (strcmp(args[0], "exit") == 0)
    {
        int status = last_status; 

        if (args[1])
        {
            char *endptr;
            long val = strtol(args[1], &endptr, 10);

            if (*endptr != '\0' || val < 0 || val > 255)
            {
                fprintf(stderr, "%s: 1: exit: Illegal number: %s\n", program_name, args[1]);
                last_status = 2;
                return 1;
            }
            status = (int)val;
        }

        exit(status);
    }

    return 0;
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
    struct stat buffer;
    if (strchr(command, '/') != NULL)
    {
        if (stat(command, &buffer) == 0)
            return strdup(command);
        return NULL;
    }
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
    char *token = strtok(command_line, " \t");
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

    if (!args || !args[0])
        return;

    if (handle_built_in(args, program_name))
        return;

    command_path = find_command(args[0]);
    if (command_path == NULL)
    {
        fprintf(stderr, "%s: %d: %s: not found\n", program_name, cmd_count, args[0]);
        last_status = 127;
        return;
    }

    pid = fork();
    if (pid == -1)
    {
        perror("Error:");
        free(command_path);
        last_status = 1;
        return;
    }
    
    if (pid == 0)
    {
        if (execve(command_path, args, environ) == -1)
        {
            fprintf(stderr, "%s: %d: %s: not found\n", program_name, cmd_count, args[0]);
            free(command_path);
            exit(127);
        }
    }
    else
    {
        wait(&status);
        if (WIFEXITED(status))
            last_status = WEXITSTATUS(status);
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

    return last_status;
}

