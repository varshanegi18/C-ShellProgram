/*#include <stdio.h>
#include <string.h>

int main() {
    char input[100];
    char builtin[][15] = {"echo", "cat", "exit", "type"};
    char PATH[][20]= {"ls","clear","cd","mkdir","pwd"},
    char *arguments = input;
    
    while (1) {
        printf("$ ");
        fflush(stdout);

        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0'; // Remove newline character
        
        // Handle 'type' command separately
        if (strncmp(input, "type ", 5) == 0) {
            char *command = input + 5; // Get the command after 'type '
            int found_builtin = 0;
            
            
            // Check if the command is a shell builtin
            for (int i = 0; i < sizeof(builtin) / sizeof(builtin[0]); i++) {
                if (strcmp(builtin[i], command) == 0) {
                    printf("%s is a shell builtin\n", command);
                    found_builtin = 1;
                    break;
                }
            }
            // Check if the command is a shell builtin
            for (int i = 0; i < sizeof(builtin) / sizeof(builtin[0]); i++) {
                if (strcmp(builtin[i], command) == 0) {
                    printf("%s is a shell builtin\n", command);
                    found_builtin = 1;
                    break;
                }
            }
            if (!found_builtin) {
                printf("%s: not found\n", command);
            }
            continue; // Skip further processing for 'type' command
        }

        // Handle other built-in commands
        for (int i = 0; i < sizeof(builtin) / sizeof(builtin[0]); i++) {
            if (strcmp(builtin[i], input) == 0) {
                printf("%s is a shell builtin\n", builtin[i]);
                break;
            }
        }

        // Handle unrecognized commands
        if (strncmp(input, "echo ", strlen("echo")) == 0) {
            printf("%s\n", input + 5);
            continue;
        }

        if(strcmp(input,"exit 0") ==0){
        break;
        }

        printf("%s: command not found\n", input);
    }

    return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>  // For access() function to check if file is executable

int main() {
    char input[100];
    // List of built-in commands
    char *builtin[] = {"echo", "exit", "type"};
    
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
            // Skip any leading spaces after 'echo '
            char *arg = input + 5;
            while (*arg == ' ') arg++;  // Skip leading spaces
            printf("%s\n", arg); // Print the argument after 'echo'
            continue;
        }

        // Exit command
        if (strcmp(input, "exit 0") == 0) {
            break;
        }

        // If command is not recognized, print not found
        printf("%s: command not found\n", strtok(input, "\n"));
    }

    return 0;
}*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>  // For access() function to check if file is executable
#include <sys/types.h>
#include <sys/wait.h>  // For waitpid()

int main() {
    char input[200];  // Increased size to handle longer commands

    // List of built-in commands
    char *builtin[] = {"echo", "exit", "type"};
    
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
            // Skip any leading spaces after 'echo '
            char *arg = input + 5;
            while (*arg == ' ') arg++;  // Skip leading spaces
            printf("%s\n", arg); // Print the argument after 'echo'
            continue;
        }

        // Exit command
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Tokenize input into command and arguments
        char *args[100];
        int i = 0;
        char *token = strtok(input, " ");
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL; // Null-terminate the arguments array

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
            // For now, we just handle the built-in commands directly
            if (strcmp(args[0], "exit") == 0) {
                break;
            } else if (strcmp(args[0], "echo") == 0) {
                printf("%s\n", args[1]);  // Simple echo
            } else if (strcmp(args[0], "type") == 0) {
                // The 'type' command is already handled above, so skip here
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

      
