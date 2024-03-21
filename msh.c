#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"
#define MAX_COMMAND_SIZE 128
#define MAX_NUM_ARGUMENTS 11 // Increase this to support up to 10 arguments + command

int main()
{
    char *command_string = (char *)malloc(MAX_COMMAND_SIZE);

    while (1)
    {
        // Print shell prompt
        printf("msh> ");

        // Read input command from the user
        while (!fgets(command_string, MAX_COMMAND_SIZE, stdin));

        char *token[MAX_NUM_ARGUMENTS];
        for (int i = 0; i < MAX_NUM_ARGUMENTS; i++)
        {
            token[i] = NULL;
        }

        int token_count = 0;
        char *argument_ptr = NULL;
        char *working_string = strdup(command_string);
        char *head_ptr = working_string;

        // Tokenize the input command
        while (((argument_ptr = strsep(&working_string, WHITESPACE)) != NULL) &&
               (token_count < MAX_NUM_ARGUMENTS))
        {
            token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
            if (strlen(token[token_count]) == 0)
            {
                token[token_count] = NULL;
            }
            token_count++;
        }

        if (token[0] == NULL)
        {
            continue; // Empty command, prompt again
        }
        else if ((strcmp(token[0], "exit") == 0) || (strcmp(token[0], "quit") == 0))
        {
            exit(0); // Exit the shell
        }
        else if (strcmp(token[0], "cd") == 0)
        {
            chdir(token[1]); // Change directory
        }
        else
        {
            // Fork a child process
            pid_t pid = fork();

            if (pid == -1)
            {
                perror("fork");
                exit(1);
            }
            else if (pid == 0)
            {
                // Child process

                // Search for the command in /bin, /usr/bin, /usr/local/bin, and CWD
                char *paths[] = {"/bin/", "/usr/bin/", "/usr/local/bin/", ""}; // Add more paths if needed

                for (int i = 0; i < sizeof(paths) / sizeof(paths[0]); i++)
                {
                    char command_path[MAX_COMMAND_SIZE];
                    snprintf(command_path, sizeof(command_path), "%s%s", paths[i], token[0]);
                    execvp(command_path, token);
                }

                // If the command is not found in any path, print an error message
                fprintf(stderr, "Command not found: %s\n", token[0]);
                exit(1);
            }
            else
            {
                // Parent process
                int status;
                wait(&status); // Wait for the child to finish
            }
        }

        // Cleanup allocated memory
        for (int i = 0; i < MAX_NUM_ARGUMENTS; i++)
        {
            if (token[i] != NULL)
            {
                free(token[i]);
            }
        }

        free(head_ptr);
    }

    free(command_string);

    return 0;
}
