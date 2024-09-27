#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>   // For PATH_MAX
#include <sys/wait.h>

#define MAX_ARG_INPUT 1024
#define MAX_ARGS 64

// =Linked List struct for bglist
typedef struct bg_process {
    pid_t pid;               
    char command[MAX_ARG_INPUT];  
    struct bg_process *next;  
} bg_process;

// Head of the linked list for background processes
bg_process *bg_list = NULL;

// Function to add a process to the bg_list
void add_bg_process(pid_t pid, char *command) {
    bg_process *new_process = (bg_process *)malloc(sizeof(bg_process));
    new_process->pid = pid;
    strcpy(new_process->command, command);
    new_process->next = bg_list;
    bg_list = new_process;
}

// Function to remove a process from the bg_list
void remove_bg_process(pid_t pid) {
    bg_process *current = bg_list;
    bg_process *prev = NULL;
    while (current != NULL) {
        if (current->pid == pid) {
            if (prev == NULL) {
                bg_list = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// Function to display the list of background jobs
void bglist() {
    bg_process *current = bg_list;
    int count = 0;
    while (current != NULL) {
        printf("%d: %s\n", current->pid, current->command);
        current = current->next;
        count++;
    }
    printf("Total Background jobs: %d\n", count);
}

// Function to check and remove terminated background processes
void check_terminated_bg_processes() {
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {

        bg_process *current = bg_list;

        // Find the terminated process in the list
        while (current != NULL) {
            if (current->pid == pid) {
                printf("%d: %s has terminated.\n", pid, current->command);
                break;
            }
            current = current->next;
        }

    
        remove_bg_process(pid);
    }
}


// Function to get the formatted shell prompt: username@hostname:cwd>
void get_prompt(char *prompt, size_t size) {
    char hostname[HOST_NAME_MAX + 1];
    char cwd[PATH_MAX];
    char *username = getlogin();

    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("gethostname");
        strcpy(hostname, "unknown");
    }
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        strcpy(cwd, "/unknown");
    }
    snprintf(prompt, size, "%s@%s:%s> ", username, hostname, cwd);
}

// Built-in function to change directory
void change_directory(char *path) {
    char *home_dir;
    if (path == NULL || strcmp(path, "~") == 0) {
        home_dir = getenv("HOME");
        if (home_dir == NULL) {
            perror("getenv");
            return;
        }
        path = home_dir;
    }
    if (chdir(path) != 0) {
        perror("chdir");
    }
}

// Function to read user input
void read_input(char *buffer) {
    char prompt[1024];
    get_prompt(prompt, sizeof(prompt));
    printf("%s", prompt);
    fgets(buffer, MAX_ARG_INPUT, stdin);
    buffer[strlen(buffer) - 1] = '\0';
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
    args[i] = NULL;
}

// Function to execute a command using fork and exec
void execute_command(char **args, int background) {
    pid_t pid = fork();
    if (pid == 0) {
        // In the child process (background process)
        if (background) {
            freopen("/dev/null", "w", stdout);
            freopen("/dev/null", "w", stderr);
        }

        if (execvp(args[0], args) == -1) {
            perror("execvp");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Fork failed
        perror("fork");
    } else {
        if (background) {
            // Add the background process to the list
            char command[MAX_ARG_INPUT] = "";
            for (int i = 0; args[i] != NULL; i++) {
                strcat(command, args[i]);
                strcat(command, " ");
            }
            add_bg_process(pid, command);
            printf("Started background process %d: %s\n", pid, command);
        } else {
            wait(NULL);
        }
    }
}

int main() {
    char input[MAX_ARG_INPUT];
    char *args[MAX_ARGS];

    while (1) {
        check_terminated_bg_processes();

        // Read input
        read_input(input);

        // Parse the input into command and arguments
        parse_input(input, args);

        // Handle the "cd" command separately
        if (strcmp(args[0], "cd") == 0) {
            change_directory(args[1]);
        }
        // Handle the "exit" command
        else if (strcmp(args[0], "exit") == 0) {
            break;
        }
        // Handle the "bglist" command
        else if (strcmp(args[0], "bglist") == 0) {
            bglist();
        }
        // Handle "bg" command
        else if (strcmp(args[0], "bg") == 0) {
            if (args[1] != NULL) {
                execute_command(&args[1], 1);  // Execute command in background
            } else {
                printf("bg: Missing command\n");
            }
        }
        // Execute other commands normally
        else {
            execute_command(args, 0);
        }
    }

    return 0;
}
