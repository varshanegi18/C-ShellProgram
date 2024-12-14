#include <stdio.h>
#include <string.h>

int main() {
    char input[100];
    char builtin[][15] = {"echo", "cat", "exit", "type"};
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
