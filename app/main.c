#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>  // For access() function to check if file is executable
#include <sys/types.h>
#include <sys/wait.h>  // For waitpid()
#include <sys/stat.h>  // for stat() function

#define BUFFER_SIZE 1024  // Define a buffer size

// Updated Function to parse input with both single and double quotes
void parse_input(char *input, char *args[]) {
    char temp[BUFFER_SIZE];
    int temp_index = 0;
    int in_single_quote = 0, in_double_quote = 0;
    int i = 0;

    for (int j = 0; input[j] != '\0'; j++) {
        if (input[j] == '\'' && !in_double_quote) {
            // Toggle single-quote mode if not inside double quotes
            in_single_quote = !in_single_quote;
        } else if (input[j] == '"' && !in_single_quote) {
            // Toggle double-quote mode if not inside single quotes
            in_double_quote = !in_double_quote;
        } else if ((input[j] == ' ' || input[j] == '\t') && !in_single_quote && !in_double_quote) {
            // End of argument outside quotes
            if (temp_index > 0) {
                temp[temp_index] = '\0';
                args[i++] = strdup(temp);
                temp_index = 0;
            }
        } else if (input[j] == '\\' && in_double_quote) {
            // Handle escaped characters inside double quotes
            if (input[j + 1] == '"' || input[j + 1] == '$' || input[j + 1] == '\\') {
                temp[temp_index++] = input[++j]; // Skip the backslash and use the next char
            } else {
                temp[temp_index++] = input[j]; // Treat as normal character
            }
        } else {
            // Collect characters
            temp[temp_index++] = input[j];
        }
    }

    // Add the last argument if any
    if (temp_index > 0) {
        temp[temp_index] = '\0';
        args[i++] = strdup(temp);
    }

    args[i] = NULL; // Null-terminate the argument list
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
            // Tokenize the input and handle quotes
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
                const char *home_dir = getenv("HOME");
                if (home_dir == NULL) {
                    fprintf(stderr, "cd: HOME environment variable is not set\n");
                    continue;
                }

                char *full_path = malloc(strlen(home_dir) + strlen(path));
                if (!full_path) {
                    fprintf(stderr, "cd: memory allocation failed\n");
                    continue;
                }

                snprintf(full_path, strlen(home_dir) + strlen(path), "%s%s", home_dir, path + 1);

                struct stat path_stat;
                if (stat(full_path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
                    if (chdir(full_path) != 0) {
                        perror("cd");
                    }
                } else {
                    fprintf(stderr, "cd: %s: No such file or directory\n", full_path);
                }

                free(full_path);
            } else if (path[0] == '/') {
                struct stat path_stat;
                if (stat(path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
                    if (chdir(path) != 0) {
                        perror("cd");
                    }
                } else {
                    fprintf(stderr, "cd: %s: No such file or directory\n", path);
                }
            } else {
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    size_t full_path_len = strlen(cwd) + strlen(path) + 2;
                    char *full_path = malloc(full_path_len);
                    if (!full_path) {
                        fprintf(stderr, "cd: memory allocation failed\n");
                        continue;
                    }

                    snprintf(full_path, full_path_len, "%s/%s", cwd, path);

                    struct stat path_stat;
                    if (stat(full_path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
                        if (chdir(full_path) != 0) {
                            perror("cd");
                        }
                    } else {
                        fprintf(stderr, "cd: %s: No such file or directory\n", full_path);
                    }

                    free(full_path);
                } else {
                    perror("getcwd");
                }
            }
            continue;
        }

        // Exit command
        if (strcmp(input, "exit 0") == 0) {
            break;
        }

        // Tokenize input into command and arguments
        char *args[100];
        parse_input(input, args);  // Parse input with handling for quotes

        if (args[0] == NULL) {
            continue;
        }

        // Run the external command
        pid_t pid = fork();
        if (pid == 0) {
            char *path_env = getenv("PATH");
            char path_copy[200];
            strncpy(path_copy, path_env, sizeof(path_copy));

            char *path = strtok(path_copy, ":");
            while (path != NULL) {
                char full_path[200];
                snprintf(full_path, sizeof(full_path), "%s/%s", path, args[0]);
                if (access(full_path, X_OK) == 0) {
                    execv(full_path, args);
                    perror("execv failed");
                    exit(1);
                }
                path = strtok(NULL, ":");
            }

            fprintf(stderr, "%s: command not found\n", args[0]);
            exit(1);
        } else if (pid > 0) {
            waitpid(pid, NULL, 0);
        } else {
            perror("fork failed");
        }
    }

    return 0;
}

