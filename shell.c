#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PROMPT "#fyzshell$ "

int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    pid_t pid;
    int status;

    while (1)
    {
        printf(PROMPT);  // Display prompt
        fflush(stdout);

        nread = getline(&line, &len, stdin);  // Read user input
        if (nread == -1)  // Handle Ctrl+D (EOF)
        {
            printf("\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';  // Remove newline character

        if (strlen(line) == 0)  // Ignore empty input
            continue;

        pid = fork();  // Create a child process
        if (pid == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0)  // Child process
        {
            char *args[] = {line, NULL};
            if (execve(line, args, NULL) == -1)  // Execute command
            {
                perror("./shell");
                exit(EXIT_FAILURE);
            }
        }
        else  // Parent process
        {
            wait(&status);  // Wait for the child to finish
        }
    }

    free(line);
    return 0;
}
