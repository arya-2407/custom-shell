#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   // For getcwd(), getlogin(), and gethostname()
#include <limits.h>   // For PATH_MAX, which defines max path length

#define MAX_INPUT 1024
#define MAX_ARGS 64

// Function to get the formatted shell prompt: username@hostname:cwd>
void get_prompt(char *prompt, size_t size) {
    char hostname[HOST_NAME_MAX + 1];  // Buffer for the hostname
    char cwd[PATH_MAX];                // Buffer for the current working directory
    char *username = getlogin();       // Get the username

    // Get the hostname
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("gethostname");
        strcpy(hostname, "unknown");   // Fallback if gethostname fails
    }

    // Get the current working directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        strcpy(cwd, "/unknown");       // Fallback if getcwd fails
    }

    // Format the prompt string: username@hostname:cwd>
    snprintf(prompt, size, "%s@%s:%s> ", username, hostname, cwd);
}

// Function to read user input
void read_input(char *buffer) {
    char prompt[1024];  // Buffer to store the shell prompt

    // Get the formatted shell prompt
    get_prompt(prompt, sizeof(prompt));

    // Display the custom shell prompt
    printf("%s", prompt);

    // Read user input
    fgets(buffer, MAX_INPUT, stdin);
    buffer[strlen(buffer) - 1] = '\0';  // Remove the newline at the end
}

// Function to split input into command and arguments
void parse_input(char *input, char **args) {
    char *token;
    int i = 0;
    token = strtok(input, " ");
    while (token != NULL && i < MAX_ARGS) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;  // Null-terminate the list of arguments
}

// Function to execute a command using fork and exec
void execute_command(char **args) {
    pid_t pid = fork();
    if (pid == 0) {
        // In child process
        if (execvp(args[0], args) == -1) {
            perror("execvp");
        }
        exit(EXIT_FAILURE);  // If execvp fails
    } else if (pid < 0) {
        // Fork failed
        perror("fork");
    } else {
        // In parent process
        wait(NULL);  // Wait for the child to finish
    }
}

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    while (1) {
        // Read input
        read_input(input);

        // Exit shell if the user types "exit"
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Parse the input into command and arguments
        parse_input(input, args);

        // Execute the command
        execute_command(args);
    }

    return 0;
}
