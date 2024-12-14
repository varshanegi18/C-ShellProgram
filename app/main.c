#include <stdio.h>
#include <string.h>

int main() {
    char input[100];
    char i;
    while (1) {
        printf("$ ");
        fflush(stdout);

        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0'; // Remove newline character
        if(strcmp(input,"exit 0") ==0){
        break;
        }
        if (strncmp(input,"type ", strlen("type")) ==0){
          printf("%s is a shell builtin\n", input+5);
          continue;
        }
        if (strncmp(input, "echo ", strlen("echo")) == 0){
            printf ("%s\n",  input + 5);
            continue;
        }  
        printf("%s: command not found\n", input);
    }

    return 0;
}
