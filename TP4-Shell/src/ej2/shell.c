#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h> 

#define MAX_COMMANDS 200

void split_args_preserving_quotes(char *input, char **args, int *arg_count) {
    char *p = input;
    *arg_count = 0;

    while (*p) {
        // saltar espacios en blanco
        while (*p && isspace((unsigned char)*p)) {
            *p++ = '\0';
        }
        if (!*p) break;

        if (*p == '"') {
            // argumento entre comillas
            p++;
            args[(*arg_count)++] = p;
            while (*p && *p != '"') p++;
            if (*p == '"') {
                *p = '\0';
                p++;
            }
        } else {
            // argumento simple
            args[(*arg_count)++] = p;
            while (*p && !isspace((unsigned char)*p)) p++;
            if (*p) {
                *p = '\0';
                p++;
            }
        }
    }

    // terminar array
    args[*arg_count] = NULL;
}


int main() {

    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count = 0;

    while (1) 
    {
        printf("Shell> ");
        
        /*Reads a line of input from the user from the standard input (stdin) and stores it in the variable command */
        fgets(command, sizeof(command), stdin);
        
        /* Removes the newline character (\n) from the end of the string stored in command, if present. 
           This is done by replacing the newline character with the null character ('\0').
           The strcspn() function returns the length of the initial segment of command that consists of 
           characters not in the string specified in the second argument ("\n" in this case). */
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0) break;

        /* Tokenizes the command string using the pipe character (|) as a delimiter using the strtok() function. 
           Each resulting token is stored in the commands[] array. 
           The strtok() function breaks the command string into tokens (substrings) separated by the pipe character |. 
           In each iteration of the while loop, strtok() returns the next token found in command. 
           The tokens are stored in the commands[] array, and command_count is incremented to keep track of the number of tokens found. */
        char *token = strtok(command, "|");
        while (token != NULL) 
        {
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }

        /* You should start programming from here... */
        for (int i = 0; i < command_count; i++) 
        {
            printf("Command %d: %s\n", i, commands[i]);
        }

        int pipes[command_count - 1][2];

        for (int i = 0; i < command_count - 1; i++) {
            if (pipe(pipes[i]) < 0) {
                perror("pipe");
                exit(1);
            }
        }

        for (int i = 0; i < command_count; i++) {
            pid_t pid = fork();
            if (pid == 0) {
                // Redireccionar entrada si no soy el primero
                if (i > 0) {
                    dup2(pipes[i - 1][0], STDIN_FILENO);
                }
                // Redireccionar salida si no soy el último
                if (i < command_count - 1) {
                    dup2(pipes[i][1], STDOUT_FILENO);
                }

                // Cerrar todos los pipes
                for (int j = 0; j < command_count - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                // Parseo de argumentos (respetando comillas)
                char *args[64];
                int arg_count = 0;
                split_args_preserving_quotes(commands[i], args, &arg_count);

                execvp(args[0], args);
                perror("execvp");
                exit(1);
            }
        }

        // Cerrar todos los pipes en el padre
        for (int i = 0; i < command_count - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        // Esperar a todos los hijos
        for (int i = 0; i < command_count; i++) {
            wait(NULL);
        }

        // Resetear contador
        command_count = 0;
    }
    return 0;
}
