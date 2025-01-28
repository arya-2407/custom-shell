Features Implemented:

1. Custom Shell Prompt 
    The shell displays a prompt in the format:
    username@hostname:current_working_directory>
    Used getcwd(), getlogin() and gethostname()

2. Basic Command Execution
The shell can execute standard Unix commands (like ls and cat).
The commands are run by creating a child process using fork(). The child process then uses execvp() to execute the given command.

3. Change Directory (cd command)
The shell includes a built-in cd command to change the current working directory.
If the user provides no argument, or ~ is passed, it defaults to the home directory.

4. Background Execution (bg command)
Commands can be run in the background using the "bg".
The shell does not wait for background jobs to complete, allowing the user to continue entering commands.

5. Background Process Listing (bglist command)
The bglist command displays a list of all active background jobs, including their process ID (PID) and the command that is being executed.

6. Background Job Termination Notification
When a background process terminates, the shell automatically prints a message to the user in the format:
[process id]: [command] has terminated.
This ensures the user is aware when a background job has completed.

7. Handling Input
The shell accepts and processes user input via the read_input() function, allowing users to input commands and execute them.
Input is parsed into the command and its arguments using parse_input(). 
Used functions like strtok().

8. Exiting the Shell (exit command)
The shell can be terminated by typing the exit command  
