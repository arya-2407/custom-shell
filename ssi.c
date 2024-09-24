#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>   // For PATH_MAX
#include <sys/wait.h>

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

// Built-in function to change directory
void change_directory(char *path) {
    char *home_dir;

    if (path == NULL || strcmp(path, "~") == 0) {
        // If no path is provided or path is "~", go to the home directory
        home_dir = getenv("HOME");  // Get the home directory from $HOME
        if (home_dir == NULL) {
            perror("getenv");
            return;
        }
        path = home_dir;
    }

    // Use chdir to change the directory
    if (chdir(path) != 0) {
        perror("chdir");  // Print error if chdir fails
    }
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

        // Parse the input into command and arguments
        parse_input(input, args);

        // Handle the "cd" command separately
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] != NULL && args[2] != NULL) {
                // Ignore additional arguments beyond the first one
                printf("cd: too many arguments\n");
            } else {
                // Change directory
                change_directory(args[1]);
            }
        }
        // Exit the shell if the user types "exit"
        else if (strcmp(args[0], "exit") == 0) {
            break;
        } else {
            // Execute the command if it's not "cd"
            execute_command(args);
        }
    }

    return 0;
}
