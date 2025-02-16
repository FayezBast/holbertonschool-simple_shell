#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PROMPT "#fayezshell$ "

int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    pid_t pid;
    int status;

    while (1)
    {
        printf(PROMPT);
        fflush(stdout);

        nread = getline(&line, &len, stdin);
        if (nread == -1)  // Handle Ctrl+D (EOF)
        {
            printf("\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';  // Remove newline character

        if (strlen(line) == 0)  // Ignore empty input
            continue;

        pid = fork();
        if (pid == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0)  // Child process
        {
            char *args[2];   // Fixed-size array
            args[0] = line;
            args[1] = NULL;

            if (execve(line, args, NULL) == -1)
            {
                perror("./shell");
                exit(EXIT_FAILURE);
            }
        }
        else  // Parent process
        {
            wait(&status);
        }
    }

    free(line);
    return 0;
}

