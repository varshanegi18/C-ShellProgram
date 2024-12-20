#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>  // For access() function to check if file is executable
#include <sys/types.h>
#include <sys/wait.h>  // For waitpid()
#include <sys/stat.h>  // for stat() function

#define BUFFER_SIZE 1024  // Define a buffer size

// Function to parse input with single quotes
void parse_input(char *input, char *args[]) {
    char *token;
    int i = 0;
    int len = strlen(input);
    int in_single_quote = 0;
    char temp[BUFFER_SIZE];
    int temp_index = 0;

    // Iterate through the input
    for (int j = 0; j < len; j++) {
        if (input[j] == '\'' && !in_single_quote) {
            // Start of a quoted string
            in_single_quote = 1;
            continue; // Skip the opening quote
        } else if (input[j] == '\'' && in_single_quote) {
            // End of a quoted string
            in_single_quote = 0;
            temp[temp_index] = '\0';
            args[i++] = strdup(temp); // Add quoted string as an argument
            temp_index = 0;
            continue; // Skip the closing quote
        } else if (in_single_quote) {
            // Collect characters inside quotes
            temp[temp_index++] = input[j];
        } else if (input[j] == ' ' || input[j] == '\t') {
            // Handle space or tab outside quotes (end of argument)
            if (temp_index > 0) {
                temp[temp_index] = '\0';
                args[i++] = strdup(temp);
                temp_index = 0;
            }
        } else {
            // Collect normal characters outside quotes
            temp[temp_index++] = input[j];
        }
    }

    // Add the last argument if any
    if (temp_index > 0) {
        temp[temp_index] = '\0';
        args[i++] = strdup(temp);
    }

    args[i] = NULL;  // Null-terminate the arguments array
}

int main() {
    char input[200];  // Increased size to handle longer commands
    char cwd[BUFFER_SIZE];  // Statically allocate a buffer to store the current working directory
    
    // List of built-in commands
    char *builtin[] = {"echo", "exit", "type", "pwd", "cd"};
    
    while (1) {
        printf("$ ");
        fflush(stdout);

        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0'; // Remove newline character

        // Handle 'type' command separately
        if (strncmp(input, "type ", 5) == 0) {
            char *command = input + 5; // Get the command after 'type '
            if (command[0] == '\0') {
                printf("type: missing operand\n");
                continue; // Skip further processing
            }
            int found_builtin = 0;
            
            // Check if the command is a shell builtin
            for (int i = 0; i < sizeof(builtin) / sizeof(builtin[0]); i++) {
                if (strcmp(builtin[i], command) == 0) {
                    printf("%s is a shell builtin\n", command);
                    found_builtin = 1;
                    break;
                }
            }
            
            // If not a builtin, check in PATH for executable
            if (!found_builtin) {
                // Get the PATH environment variable
                char *path_env = getenv("PATH");
                if (path_env) {
                    // Copy PATH to avoid modifying the original
                    char path_copy[200];
                    strncpy(path_copy, path_env, sizeof(path_copy));
                    path_copy[sizeof(path_copy) - 1] = '\0'; // Ensure null termination

                    char *path = strtok(path_copy, ":");
                    while (path != NULL) {
                        // Create the full path for the command
                        char full_path[200];
                        snprintf(full_path, sizeof(full_path), "%s/%s", path, command);
                        
                        // Check if the command is executable in this directory
                        if (access(full_path, X_OK) == 0) {
                            printf("%s is %s\n", command, full_path);
                            found_builtin = 1;
                            break;
                        }
                        
                        path = strtok(NULL, ":"); // Move to the next directory
                    }
                }
            }

            // If command was not found anywhere, print not found
            if (!found_builtin) {
                printf("%s: not found\n", command);
            }
            continue; // Skip further processing for 'type' command
        }

        // Handle 'echo' command
        if (strncmp(input, "echo ", strlen("echo")) == 0) {
            // Tokenize the input and handle single quotes
            char *args[100];
            parse_input(input + 5, args);  // Skip 'echo ' and parse the rest

            // Echo each argument (printed as they are, including spaces within quotes)
            for (int i = 0; args[i] != NULL; i++) {
                printf("%s", args[i]);
                if (args[i + 1] != NULL) {
                    printf(" ");
                }
            }
            printf("\n");
            continue;
        }

        // Handle 'pwd' command
        if (strncmp(input, "pwd", strlen("pwd")) == 0) {
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);  // Print the current working directory
                continue;
            }
        }

        // Handle 'cd' command (both absolute and relative paths)
        if (strncmp(input, "cd ", strlen("cd ")) == 0) {
            char *path = input + 3;  // Skip "cd " and get the path

            // Handle the case of empty or no path
            if (path[0] == '\0') {
                fprintf(stderr, "cd: missing operand\n");
                continue;
            }

            // Handle '~' as home directory
            if (path[0] == '~') {
                // Check if path starts with '~', replace it with HOME env variable
                const char *home_dir = getenv("HOME");
                if (home_dir == NULL) {
                    fprintf(stderr, "cd: HOME environment variable is not set\n");
                    continue;
                }

                // Skip the '~' character and append the rest of the path if there is more
                if (path[1] != '\0') {
                    char *full_path = (char *)malloc(strlen(home_dir) + strlen(path));
                    if (full_path == NULL) {
                        fprintf(stderr, "cd: memory allocation failed\n");
                        continue;
                    }

                    snprintf(full_path, strlen(home_dir) + strlen(path), "%s%s", home_dir, path + 1);

                    struct stat path_stat;
                    // Check if the directory exists and is a directory
                    if (stat(full_path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
                        if (chdir(full_path) != 0) {
                            perror("cd");
                        }
                    } else {
                        fprintf(stderr, "cd: %s: No such file or directory\n", full_path);
                    }

                    free(full_path);
                } else {
                    // If '~' is alone, change to home directory
                    if (chdir(home_dir) != 0) {
                        perror("cd");
                    }
                }
            } else {
                // Handle absolute path (starts with '/')
                if (path[0] == '/') {
                    struct stat path_stat;
                    // Check if the path exists and is a directory
                    if (stat(path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
                        // Change the directory using chdir()
                        if (chdir(path) != 0) {
                            perror("cd");
                        }
                    } else {
                        // Print error message if directory doesn't exist
                        fprintf(stderr, "cd: %s: No such file or directory\n", path);
                    }
                } else { 
                    // Handle relative path
                    if (getcwd(cwd, sizeof(cwd)) != NULL) {
                        // Dynamically calculate the required buffer size for the concatenated path
                        size_t full_path_len = strlen(cwd) + strlen(path) + 2; // +2 for '/' and '\0'
                        if (full_path_len > sizeof(cwd)) {
                            fprintf(stderr, "cd: path is too long\n");
                            continue;
                        }

                        // Allocate memory for full_path dynamically based on the required size
                        char *full_path = (char *)malloc(full_path_len);
                        if (full_path == NULL) {
                            fprintf(stderr, "cd: memory allocation failed\n");
                            continue;
                        }

                        snprintf(full_path, full_path_len, "%s/%s", cwd, path);  // Combine current dir with the relative path

                        struct stat path_stat;
                        // Check if the relative path exists and is a directory
                        if (stat(full_path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
                            // Change the directory using chdir()
                            if (chdir(full_path) != 0) {
                                perror("cd");
                            }
                        } else {
                            // Print error message if directory doesn't exist
                            fprintf(stderr, "cd: %s: No such file or directory\n", full_path);
                        }

                        // Free the dynamically allocated memory
                        free(full_path);
                    } else {
                        perror("getcwd");
                    }
                }
            }
            continue; // Skip further processing for 'cd' command
        }

        // Exit command
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Tokenize input into command and arguments
        char *args[100];
        parse_input(input, args);  // Parse input with handling for single quotes

        // If command is empty, prompt again
        if (args[0] == NULL) {
            continue;
        }

        // If it's a built-in command, handle it
        int is_builtin = 0;
        for (int i = 0; i < sizeof(builtin) / sizeof(builtin[0]); i++) {
            if (strcmp(args[0], builtin[i]) == 0) {
                is_builtin = 1;
                break;
            }
        }

        if (is_builtin) {
            // Handle built-in commands like exit, echo, etc.
            if (strcmp(args[0], "exit") == 0) {
                break;
            } else if (strcmp(args[0], "echo") == 0) {
                printf("%s\n", args[1]);  // Simple echo
            }
        } else {
            // Run the external command
            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                // Check if the command is in the PATH
                char *path_env = getenv("PATH");
                char path_copy[200];
                strncpy(path_copy, path_env, sizeof(path_copy));

                char *path = strtok(path_copy, ":");
                while (path != NULL) {
                    char full_path[200];
                    snprintf(full_path, sizeof(full_path), "%s/%s", path, args[0]);
                    if (access(full_path, X_OK) == 0) {
                        execv(full_path, args);  // Execute the external command
                        perror("execv failed");  // If execv fails
                        exit(1);
                    }
                    path = strtok(NULL, ":");
                }

                // If we reach here, the command was not found
                fprintf(stderr, "%s: command not found\n", args[0]);
                exit(1);
            } else if (pid > 0) {
                // Parent process waits for the child to finish
                waitpid(pid, NULL, 0);
            } else {
                perror("fork failed");
            }
        }
    }

    return 0;
}

