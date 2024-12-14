#include <stdio.h>
#include <string.h>

int main() {
    char input[100];
    
    while (1) {
        printf("$ ");
        fflush(stdout);

        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0'; // Remove newline character
        if(strcmp(input,"exit 0") ==0){
        break;
        }
        if (strncmp(input,"echo ",8) ==0){
          printf ("%s\n", 8);
        }        
        printf("%s: command not found\n", input);
    }

    return 0;
}
